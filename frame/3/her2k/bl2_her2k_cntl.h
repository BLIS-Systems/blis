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

struct her2k_s
{
	impl_t             impl_type;
	varnum_t           var_num;
	blksz_t*           b;
	blksz_t*           b_aux;
	struct scalm_s*    sub_scalm;
	struct packm_s*    sub_packm_a;
	struct packm_s*    sub_packm_b;
	struct packm_s*    sub_packm_c;
	struct her2k_s*    sub_her2k;
	struct herk_s*     sub_herk;
	struct unpackm_s*  sub_unpackm_c;
};
typedef struct her2k_s her2k_t;

#define cntl_sub_her2k( cntl )      cntl->sub_her2k

void     bl2_her2k_cntl_init( void );
void     bl2_her2k_cntl_finalize( void );
her2k_t* bl2_her2k_cntl_obj_create( impl_t       impl_type,
                                    varnum_t     var_num,
                                    blksz_t*     b,
                                    blksz_t*     b_aux,
                                    scalm_t*     sub_scalm,
                                    packm_t*     sub_pack_a,
                                    packm_t*     sub_pack_b,
                                    packm_t*     sub_pack_c,
                                    her2k_t*     sub_her2k,
                                    herk_t*      sub_herk,
                                    unpackm_t*   sub_unpack_c );

