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

void bl2_cntl_init( void )
{
	// Level-1
	bl2_scalv_cntl_init();
	bl2_packv_cntl_init();
	bl2_unpackv_cntl_init();

	// Level-1m
	bl2_scalm_cntl_init();
	bl2_packm_cntl_init();
	bl2_unpackm_cntl_init();

	// Level-2
	bl2_gemv_cntl_init();
	bl2_ger_cntl_init();
	bl2_hemv_cntl_init();
	bl2_her_cntl_init();
	bl2_her2_cntl_init();
	bl2_trmv_cntl_init();
	bl2_trsv_cntl_init();

	// Level-3
	bl2_gemm_cntl_init();
	bl2_hemm_cntl_init();
	bl2_herk_cntl_init();
	bl2_her2k_cntl_init();
	bl2_trmm_cntl_init();
	bl2_trsm_cntl_init();
}

void bl2_cntl_finalize( void )
{
	// Level-1
	bl2_scalv_cntl_finalize();
	bl2_packv_cntl_finalize();
	bl2_unpackv_cntl_finalize();

	// Level-1m
	bl2_scalm_cntl_finalize();
	bl2_packm_cntl_finalize();
	bl2_unpackm_cntl_finalize();

	// Level-2
	bl2_gemv_cntl_finalize();
	bl2_ger_cntl_finalize();
	bl2_hemv_cntl_finalize();
	bl2_her_cntl_finalize();
	bl2_her2_cntl_finalize();
	bl2_trmv_cntl_finalize();
	bl2_trsv_cntl_finalize();

	// Level-3
	bl2_gemm_cntl_finalize();
	bl2_hemm_cntl_finalize();
	bl2_herk_cntl_finalize();
	bl2_her2k_cntl_finalize();
	bl2_trmm_cntl_finalize();
	bl2_trsm_cntl_finalize();
}

