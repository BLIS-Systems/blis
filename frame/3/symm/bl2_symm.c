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

extern gemm_t* hemm_cntl;

//
// Define object-based interface.
//
void bl2_symm( side_t  side,
               obj_t*  alpha,
               obj_t*  a,
               obj_t*  b,
               obj_t*  beta,
               obj_t*  c )
{
	gemm_t* cntl;
	obj_t   alpha_local;
	obj_t   beta_local;
	obj_t   a_local;
	obj_t   b_local;
	obj_t   c_local;
	num_t   dt_targ_a;
	num_t   dt_targ_b;
	num_t   dt_targ_c;
	num_t   dt_alpha;
	num_t   dt_beta;

	// Check parameters.
	if ( bl2_error_checking_is_enabled() )
		bl2_symm_check( side, alpha, a, b, beta, c );

	// If alpha is zero, scale by beta and return.
	if ( bl2_obj_scalar_equals( alpha, &BLIS_ZERO ) )
	{
		bl2_scalm( beta, c );
		return;
	}

	// For now, assume the storage datatypes are the desired target
	// datatypes.
	dt_targ_a = bl2_obj_datatype( *a );
	dt_targ_b = bl2_obj_datatype( *b );
	dt_targ_c = bl2_obj_datatype( *c );

	// Alias A and B in case we need to induce the right side case.
	bl2_obj_alias_to( *a, a_local );
	bl2_obj_alias_to( *b, b_local );
	bl2_obj_alias_to( *c, c_local );

	// We implement symm in terms of gemm. But in order to do so we must make
	// sure matrix A is on the correct side for our gemm kernel. We assume
	// gemm is implemented with a block-panel kernel, thus, we will only
	// directly support the BLIS_LEFT case. We handle the BLIS_RIGHT case by
	// transposing the operation. Since A is Hermitian, we can mark it for
	// conjugation instead of transpostion (though transposition should be
	// correctly handled as well).
	if ( bl2_is_right( side ) )
	{
		bl2_obj_toggle_conj( a_local );
		bl2_obj_toggle_trans( b_local );
		bl2_obj_toggle_trans( c_local );
	}

	// Create an object to hold a copy-cast of alpha. Notice that we use
	// the target datatype of matrix A.
	dt_alpha = dt_targ_a;
	bl2_obj_init_scalar_copy_of( dt_alpha,
	                             BLIS_NO_CONJUGATE,
	                             alpha,
	                             &alpha_local );

	// Create an object to hold a copy-cast of beta. Notice that we use
	// the datatype of C.
	dt_beta = bl2_obj_datatype( *c );
	bl2_obj_init_scalar_copy_of( dt_beta,
	                             BLIS_NO_CONJUGATE,
	                             beta,
	                             &beta_local );

	// Choose the control tree. We can just use hemm since the algorithm
	// is nearly identical to that of symm.
	cntl = hemm_cntl;

	// Invoke the internal back-end.
	bl2_gemm_int( &alpha_local,
	              &a_local,
	              &b_local,
	              &beta_local,
	              &c_local,
	              cntl );
}

//
// Define BLAS-like interfaces with homogeneous-typed operands.
//
#undef  GENTFUNC
#define GENTFUNC( ctype, ch, opname, varname ) \
\
void PASTEMAC(ch,opname)( \
                          side_t  side, \
                          uplo_t  uploa, \
                          conj_t  conja, \
                          trans_t transb, \
                          dim_t   m, \
                          dim_t   n, \
                          ctype*  alpha, \
                          ctype*  a, inc_t rs_a, inc_t cs_a, \
                          ctype*  b, inc_t rs_b, inc_t cs_b, \
                          ctype*  beta, \
                          ctype*  c, inc_t rs_c, inc_t cs_c  \
                        ) \
{ \
    const num_t dt = PASTEMAC(ch,type); \
\
    obj_t       alphao, ao, bo, betao, co; \
\
	dim_t       m_a, n_a; \
	dim_t       m_b, n_b; \
\
	bl2_set_dims_with_side(  side,   m, n, m_a, n_a ); \
	bl2_set_dims_with_trans( transb, m, n, m_b, n_b ); \
\
    bl2_obj_create_scalar_with_attached_buffer( dt, alpha, &alphao ); \
    bl2_obj_create_scalar_with_attached_buffer( dt, beta,  &betao  ); \
\
	bl2_obj_create_with_attached_buffer( dt, m_a, n_a, a, rs_a, cs_a, &ao ); \
	bl2_obj_create_with_attached_buffer( dt, m_b, n_b, b, rs_b, cs_b, &bo ); \
	bl2_obj_create_with_attached_buffer( dt, m,   n,   c, rs_c, cs_c, &co ); \
\
    bl2_obj_set_uplo( uploa, ao ); \
    bl2_obj_set_conj( conja, ao ); \
	bl2_obj_set_conjtrans( transb, bo ); \
\
    bl2_obj_set_struc( BLIS_SYMMETRIC, ao ); \
\
	PASTEMAC0(opname)( side, \
                       &alphao, \
                       &ao, \
                       &bo, \
                       &betao, \
                       &co ); \
}

INSERT_GENTFUNC_BASIC( symm, symm )


//
// Define BLAS-like interfaces with homogeneous-typed operands.
//
#undef  GENTFUNC3U12
#define GENTFUNC3U12( ctype_a, ctype_b, ctype_c, ctype_ab, cha, chb, chc, chab, opname, varname ) \
\
void PASTEMAC3(cha,chb,chc,opname)( \
                                    side_t    side, \
                                    uplo_t    uploa, \
                                    conj_t    conja, \
                                    trans_t   transb, \
                                    dim_t     m, \
                                    dim_t     n, \
                                    ctype_ab* alpha, \
                                    ctype_a*  a, inc_t rs_a, inc_t cs_a, \
                                    ctype_b*  b, inc_t rs_b, inc_t cs_b, \
                                    ctype_c*  beta, \
                                    ctype_c*  c, inc_t rs_c, inc_t cs_c  \
                                  ) \
{ \
	bl2_check_error_code( BLIS_NOT_YET_IMPLEMENTED ); \
}

INSERT_GENTFUNC3U12_BASIC( symm, symm )

#ifdef BLIS_ENABLE_MIXED_DOMAIN_SUPPORT
INSERT_GENTFUNC3U12_MIX_D( symm, symm )
#endif

#ifdef BLIS_ENABLE_MIXED_PRECISION_SUPPORT
INSERT_GENTFUNC3U12_MIX_P( symm, symm )
#endif

