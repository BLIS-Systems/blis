/*

   BLIS    
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2012, The University of Texas

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of The University of Texas nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "blis2.h"

#define FUNCPTR_T unpackv_fp

typedef void (*FUNCPTR_T)( obj_t*     p,
                           obj_t*     a,
                           unpackv_t* cntl );

static FUNCPTR_T vars[1][3] =
{
	// unblocked            optimized unblocked    blocked
	{ bl2_unpackv_unb_var1, NULL,                  NULL }
};

void bl2_unpackv_int( obj_t*     p,
                      obj_t*     a,
                      unpackv_t* cntl )
{
	// The unpackv operation consists of an optional casting post-process.
	// (This post-process is analogous to the cast pre-process in packv.)
	// Here are the following possible ways unpackv can execute:
	//  1. unpack and cast: Unpack to a temporary vector c and then cast
	//     c to a.
	//  2. unpack only: Unpack directly to vector a since typecasting is
	//     not needed.
	//  3. cast only: Not yet supported / not used.
	//  4. no-op: The control tree directs us to skip the unpack operation
	//     entirely. No action is taken.

	obj_t     c;

	varnum_t  n;
	impl_t    i;
	FUNCPTR_T f;

	// Check parameters.
	if ( bl2_error_checking_is_enabled() )
		bl2_unpackv_check( p, a, cntl );

	// Sanity check; A should never have a zero dimension. If we must support
	// it, then we should fold it into the next alias-and-early-exit block.
	if ( bl2_obj_has_zero_dim( *a ) ) bl2_abort();

	// First check if we are to skip this operation because the control tree
	// is NULL, and if so, simply return.
	if ( cntl_is_noop( cntl ) )
	{
		return;
	}

	// If p was aliased to a during the pack stage (because it was already
	// in an acceptable packed/contiguous format), then no unpack is actually
	// necessary, so we return.
	if ( bl2_obj_is_alias_of( *p, *a ) )
	{
		return;
	}

	// Now, if we are not skipping the unpack operation, then the only
	// question left is whether we are to typecast vector a after unpacking.
	if ( bl2_obj_datatype( *p ) != bl2_obj_datatype( *a ) )
		bl2_abort();
/*
	if ( bl2_obj_datatype( *p ) != bl2_obj_datatype( *a ) )
	{
		// Initialize an object c for the intermediate typecast vector.
		bl2_unpackv_init_cast( p,
		                       a,
		                       &c );
	}
	else
*/
	{
		// If no cast is needed, then aliasing object c to the original
		// vector serves as a minor optimization. This causes the unpackv
		// implementation to unpack directly into vector a.
		bl2_obj_alias_to( *a, c );
	}

	// Now we are ready to proceed with the unpacking.

	// Extract the variant number and implementation type.
	n = cntl_var_num( cntl );
	i = cntl_impl_type( cntl );

	// Index into the variant array to extract the correct function pointer.
	f = vars[n][i];

	// Invoke the variant.
	f( p,
	   &c,
	   cntl );

	// Now, if necessary, we cast the contents of c to vector a. If casting
	// was not necessary, then we are done because the call to the unpackv
	// implementation would have unpacked directly to vector a.
	if ( bl2_obj_datatype( *p ) != bl2_obj_datatype( *a ) )
	{
		// Copy/typecast vector c to vector a.
		// NOTE: Here, we use copynzv instead of copym because, in the cases
		// where we are unpacking/typecasting a real vector c to a complex
		// vector a, we want to touch only the real components of a, rather
		// than also set the imaginary components to zero. This comes about
		// because of the fact that, if we are unpacking real-to-complex,
		// then it is because all of the computation occurred in the real
		// domain, and so we would want to leave whatever imaginary values
		// there are in vector a untouched. Notice that for unpackings that
		// entail complex-to-complex data movements, the copynzv operation
		// behaves exactly as copym, so no use cases are lost (at least none
		// that I can think of).
		bl2_copynzv( &c,
		             a );
	}
}

void bl2_unpackv_init_cast( obj_t*  p,
                            obj_t*  a,
                            obj_t*  c )
{
	// The idea here is that we want to create an object c that is identical
	// to object a, except that:
	//  (1) the storage datatype of c is equal to the target datatype of a,
	//      with the element size of c adjusted accordingly,
	//  (2) object c is marked as being stored in a standard, contiguous
	//      format (ie: a column vector),
	//  (3) the view offset of c is reset to (0,0), and
	//  (4) object c's main buffer is set to a new memory region acquired
	//      from the memory manager, or extracted from p if a mem entry is
	//      already available. (After acquring a mem entry from the memory
	//      manager, it is cached within p for quick access later on.)

	num_t dt_targ_a    = bl2_obj_target_datatype( *a );
	dim_t dim_a        = bl2_obj_vector_dim( *a );
	siz_t elem_size_c  = bl2_datatype_size( dt_targ_a );

	// We begin by copying the basic fields of a.
	bl2_obj_alias_to( *a, *c );

	// Update datatype and element size fields.
	bl2_obj_set_datatype( dt_targ_a, *c );
	bl2_obj_set_elem_size( elem_size_c, *c );

	// Update the strides and dimensions. We set the increments to reflect a
	// column-stored vector. Note that the column stride is set to dim(a),
	// though it should never be used because there is no second column to
	// index into (and therefore it also does not need to be aligned).
	bl2_obj_set_dims( dim_a, 1, *c );
	bl2_obj_set_incs( 1, dim_a, *c );

	// Reset the view offsets to (0,0).
	bl2_obj_set_offs( 0, 0, *c );

	// Check the mem_t entry of p associated with the cast buffer. If it is
	// NULL, then acquire memory sufficient to hold the object data and cache
	// it to p. (Otherwise, if it is non-NULL, then memory has already been
	// acquired from the memory manager and cached.) We then set the main
	// buffer of c to the cached address of the cast memory.
	bl2_obj_set_buffer_with_cached_cast_mem( *p, *c );
}

