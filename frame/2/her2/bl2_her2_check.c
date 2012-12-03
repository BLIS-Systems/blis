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

void bl2_her2_basic_check( obj_t*   alpha,
                           obj_t*   x,
                           obj_t*   y,
                           obj_t*   c )
{
	err_t e_val;

	// Check object datatypes.

	e_val = bl2_check_noninteger_object( alpha );
	bl2_check_error_code( e_val );

	e_val = bl2_check_floating_object( x );
	bl2_check_error_code( e_val );

	e_val = bl2_check_floating_object( y );
	bl2_check_error_code( e_val );

	e_val = bl2_check_floating_object( c );
	bl2_check_error_code( e_val );

	// Check object dimensions.

	e_val = bl2_check_scalar_object( alpha );
	bl2_check_error_code( e_val );

	e_val = bl2_check_vector_object( x );
	bl2_check_error_code( e_val );

	e_val = bl2_check_vector_object( y );
	bl2_check_error_code( e_val );

	e_val = bl2_check_square_object( c );
	bl2_check_error_code( e_val );

	e_val = bl2_check_object_length_equals( c, bl2_obj_vector_dim( *x ) );
	bl2_check_error_code( e_val );

	e_val = bl2_check_object_length_equals( c, bl2_obj_vector_dim( *y ) );
	bl2_check_error_code( e_val );
}

void bl2_her2_check( obj_t*   alpha,
                     obj_t*   x,
                     obj_t*   y,
                     obj_t*   c )
{
	err_t e_val;

	// Check basic properties of the operation.

	bl2_her2_basic_check( alpha, x, y, c );

	// Check matrix structure.

	e_val = bl2_check_hermitian_object( c );
	bl2_check_error_code( e_val );
}

void bl2_her2_int_check( conj_t   conjh,
                         obj_t*   alpha,
                         obj_t*   x,
                         obj_t*   y,
                         obj_t*   c,
                         her2_t*  cntl )
{
	err_t e_val;

	// Check basic properties of the operation.

	bl2_her2_basic_check( alpha, x, y, c );

	// Check matrix structure.

	if ( bl2_is_conj( conjh ) )
	{
		e_val = bl2_check_hermitian_object( c );
		bl2_check_error_code( e_val );
	}
	else
	{
		e_val = bl2_check_symmetric_object( c );
		bl2_check_error_code( e_val );
	}

	// Check control tree pointer

	e_val = bl2_check_valid_cntl( ( void* )cntl );
	bl2_check_error_code( e_val );
}

