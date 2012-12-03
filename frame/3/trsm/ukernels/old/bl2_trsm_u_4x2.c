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

void bl2_strsm_u_4x2(
                      float* restrict a11,
                      float* restrict b11,
                      float* restrict bd11,
                      float* restrict c11, inc_t rs_c, inc_t cs_c
                    )
{
	bl2_check_error_code( BLIS_NOT_YET_IMPLEMENTED );
}

void bl2_dtrsm_u_4x2(
                      double* restrict  a11,
                      double* restrict  b11,
                      double* restrict  bd11,
                      double* restrict  c11, inc_t rs_c, inc_t cs_c
                    )
{
	const  dim_t rs_a   = 1;
	const  dim_t cs_a   = 4;

	const  dim_t rs_b   = 2;
	const  dim_t cs_b   = 1;

	const  dim_t NDUP   = 2;
	const  dim_t inc_bd = cs_b*NDUP;

	double beta00, beta01;
	double beta10, beta11;
	double beta20, beta21;
	double beta30, beta31;

	double alpha00, alpha01, alpha02, alpha03;
	double          alpha11, alpha12, alpha13;
	double                   alpha22, alpha23;
	double                            alpha33;


	beta00 = *(b11 + 0*rs_b + 0*cs_b);
	beta01 = *(b11 + 0*rs_b + 1*cs_b);
	beta10 = *(b11 + 1*rs_b + 0*cs_b);
	beta11 = *(b11 + 1*rs_b + 1*cs_b);
	beta20 = *(b11 + 2*rs_b + 0*cs_b);
	beta21 = *(b11 + 2*rs_b + 1*cs_b);
	beta30 = *(b11 + 3*rs_b + 0*cs_b);
	beta31 = *(b11 + 3*rs_b + 1*cs_b);


	// iteration 0

	alpha33 = *(a11 + 3*rs_a + 3*cs_a);

	beta30 -= 0.0;
	beta31 -= 0.0;

	beta30 *= alpha33;
	beta31 *= alpha33;

	*(b11 + 3*rs_b + 0*cs_b) = beta30;
	*(b11 + 3*rs_b + 1*cs_b) = beta31;
	*(c11 + 3*rs_c + 0*cs_c) = beta30;
	*(c11 + 3*rs_c + 1*cs_c) = beta31;


	// iteration 1

	alpha22 = *(a11 + 2*rs_a + 2*cs_a);
	alpha23 = *(a11 + 2*rs_a + 3*cs_a);

	beta20 -= alpha23 * beta30;
	beta21 -= alpha23 * beta31;

	beta20 *= alpha22;
	beta21 *= alpha22;

	*(b11 + 2*rs_b + 0*cs_b) = beta20;
	*(b11 + 2*rs_b + 1*cs_b) = beta21;
	*(c11 + 2*rs_c + 0*cs_c) = beta20;
	*(c11 + 2*rs_c + 1*cs_c) = beta21;


	// iteration 2

	alpha11 = *(a11 + 1*rs_a + 1*cs_a);
	alpha12 = *(a11 + 1*rs_a + 2*cs_a);
	alpha13 = *(a11 + 1*rs_a + 3*cs_a);

	beta10 -= alpha12 * beta20 +
	          alpha13 * beta30;
	beta11 -= alpha12 * beta21 +
	          alpha13 * beta31;

	beta10 *= alpha11;
	beta11 *= alpha11;

	*(b11 + 1*rs_b + 0*cs_b) = beta10;
	*(b11 + 1*rs_b + 1*cs_b) = beta11;
	*(c11 + 1*rs_c + 0*cs_c) = beta10;
	*(c11 + 1*rs_c + 1*cs_c) = beta11;


	// iteration 3

	alpha00 = *(a11 + 0*rs_a + 0*cs_a);
	alpha01 = *(a11 + 0*rs_a + 1*cs_a);
	alpha02 = *(a11 + 0*rs_a + 2*cs_a);
	alpha03 = *(a11 + 0*rs_a + 3*cs_a);

	beta00 -= alpha01 * beta10 +
	          alpha02 * beta20 +
	          alpha03 * beta30;
	beta01 -= alpha01 * beta11 +
	          alpha02 * beta21 +
	          alpha03 * beta31;

	beta00 *= alpha00;
	beta01 *= alpha00;

	*(b11 + 0*rs_b + 0*cs_b) = beta00;
	*(b11 + 0*rs_b + 1*cs_b) = beta01;
	*(c11 + 0*rs_c + 0*cs_c) = beta00;
	*(c11 + 0*rs_c + 1*cs_c) = beta01;
	

	// update bd

	*(bd11 + 0*inc_bd + 0*cs_b) = beta00;
	*(bd11 + 0*inc_bd + 1*cs_b) = beta00;
	*(bd11 + 1*inc_bd + 0*cs_b) = beta01;
	*(bd11 + 1*inc_bd + 1*cs_b) = beta01;
	*(bd11 + 2*inc_bd + 0*cs_b) = beta10;
	*(bd11 + 2*inc_bd + 1*cs_b) = beta10;
	*(bd11 + 3*inc_bd + 0*cs_b) = beta11;
	*(bd11 + 3*inc_bd + 1*cs_b) = beta11;
	*(bd11 + 4*inc_bd + 0*cs_b) = beta20;
	*(bd11 + 4*inc_bd + 1*cs_b) = beta20;
	*(bd11 + 5*inc_bd + 0*cs_b) = beta21;
	*(bd11 + 5*inc_bd + 1*cs_b) = beta21;
	*(bd11 + 6*inc_bd + 0*cs_b) = beta30;
	*(bd11 + 6*inc_bd + 1*cs_b) = beta30;
	*(bd11 + 7*inc_bd + 0*cs_b) = beta31;
	*(bd11 + 7*inc_bd + 1*cs_b) = beta31;

}

void bl2_ctrsm_u_4x2(
                      scomplex* restrict a11,
                      scomplex* restrict b11,
                      scomplex* restrict bd11,
                      scomplex* restrict c11, inc_t rs_c, inc_t cs_c
                    )
{
	bl2_check_error_code( BLIS_NOT_YET_IMPLEMENTED );
}

void bl2_ztrsm_u_4x2(
                      dcomplex* restrict a11,
                      dcomplex* restrict b11,
                      dcomplex* restrict bd11,
                      dcomplex* restrict c11, inc_t rs_c, inc_t cs_c
                    )
{
	bl2_check_error_code( BLIS_NOT_YET_IMPLEMENTED );
}

