/*

   BLIS    
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of The University of Texas at Austin nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

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

#include "blis.h"
#include "blix.h"

// Function pointer type for datatype-specific functions.
typedef void (*gemm_fp)
     (
       pack_t  schema_a,
       pack_t  schema_b,
       dim_t   m,
       dim_t   n,
       dim_t   k,
       void*   alpha,
       void*   a, inc_t cs_a, inc_t is_a,
                  dim_t pd_a, inc_t ps_a,
       void*   b, inc_t rs_b, inc_t is_b,
                  dim_t pd_b, inc_t ps_b,
       void*   beta,
       void*   c, inc_t rs_c, inc_t cs_c,
       cntx_t* cntx,
       dim_t ir_num_threads, dim_t ir_thread_id, \
       dim_t jr_num_threads, dim_t jr_thread_id \
     );

// Function pointer array for datatype-specific functions.
static gemm_fp ftypes[BLIS_NUM_FP_TYPES] =
{
    PASTECH2(blx_,s,gemm_ker_var2),
    PASTECH2(blx_,c,gemm_ker_var2),
    PASTECH2(blx_,d,gemm_ker_var2),
    PASTECH2(blx_,z,gemm_ker_var2)
};

typedef struct
{
    gemm_fp f;
    pack_t  schema_a;
    pack_t  schema_b;
    dim_t   m;
    dim_t   n;
    dim_t   k;
    void*   alpha;
    void*   a; inc_t cs_a; inc_t is_a;
               dim_t pd_a; inc_t ps_a;
    void*   b; inc_t rs_b; inc_t is_b;
               dim_t pd_b; inc_t ps_b;
    void*   beta;
    void*   c; inc_t rs_c; inc_t cs_c;
    cntx_t* cntx;
    dim_t ir_num_threads;
    dim_t jr_num_threads;
} gemm_params;

static void blx_gemm_ker_var2_thread( tci_comm* comm,
                                      uint64_t tid,
                                      uint64_t unused,
                                      void* param_ )
{
    gemm_params* param = param_;

    dim_t ir_thread_id = tid % param->ir_num_threads;
    dim_t jr_thread_id = tid / param->ir_num_threads;

    param->f( param->schema_a,
              param->schema_b,
              param->m,
              param->n,
              param->k,
              param->alpha,
              param->a, param->cs_a, param->is_a,
                        param->pd_a, param->ps_a,
              param->b, param->rs_b, param->is_b,
                        param->pd_b, param->ps_b,
              param->beta,
              param->c, param->rs_c, param->cs_c,
              param->cntx,
              param->ir_num_threads, ir_thread_id,
              param->jr_num_threads, jr_thread_id );
}

void blx_gemm_ker_var2
     (
       obj_t*  a,
       obj_t*  b,
       obj_t*  c,
       cntx_t* cntx,
       cntl_t* cntl,
       thrinfo_t* thread
     )
{
	num_t     dt_exec   = bli_obj_exec_dt( c );

	gemm_params param;

	param.schema_a  = bli_obj_pack_schema( a );
	param.schema_b  = bli_obj_pack_schema( b );

	param.m         = bli_obj_length( c );
	param.n         = bli_obj_width( c );
	param.k         = bli_obj_width( a );

	param.a         = bli_obj_buffer_at_off( a );
	param.cs_a      = bli_obj_col_stride( a );
	param.is_a      = bli_obj_imag_stride( a );
	param.pd_a      = bli_obj_panel_dim( a );
	param.ps_a      = bli_obj_panel_stride( a );

	param.b         = bli_obj_buffer_at_off( b );
	param.rs_b      = bli_obj_row_stride( b );
	param.is_b      = bli_obj_imag_stride( b );
	param.pd_b      = bli_obj_panel_dim( b );
	param.ps_b      = bli_obj_panel_stride( b );

	param.c         = bli_obj_buffer_at_off( c );
	param.rs_c      = bli_obj_row_stride( c );
	param.cs_c      = bli_obj_col_stride( c );

	param.cntx      = cntx;

    param.ir_num_threads = thread->sub_node->comm->nthread;
    param.jr_num_threads = thread->sub_node->comm->ngang;

	obj_t     scalar_a;
	obj_t     scalar_b;

	// Detach and multiply the scalars attached to A and B.
	bli_obj_scalar_detach( a, &scalar_a );
	bli_obj_scalar_detach( b, &scalar_b );
	bli_mulsc( &scalar_a, &scalar_b );

	// Grab the addresses of the internal scalar buffers for the scalar
	// merged above and the scalar attached to C.
	param.alpha = bli_obj_internal_scalar_buffer( &scalar_b );
	param.beta  = bli_obj_internal_scalar_buffer( c );

	// Index into the type combination array to extract the correct
	// function pointer.
	param.f = ftypes[dt_exec];

	tci_comm* comm = thread->comm;
	tci_range range = {comm->nthread, 1};

	// Invoke the function.
    tci_comm_distribute_over_threads( comm,
                                      range,
                                      blx_gemm_ker_var2_thread,
                                      &param );
}


#undef  GENTFUNC
#define GENTFUNC( ctype, ch, varname ) \
\
void PASTECH2(blx_,ch,varname) \
     ( \
       pack_t  schema_a, \
       pack_t  schema_b, \
       dim_t   m, \
       dim_t   n, \
       dim_t   k, \
       void*   alpha, \
       void*   a, inc_t cs_a, inc_t is_a, \
                  dim_t pd_a, inc_t ps_a, \
       void*   b, inc_t rs_b, inc_t is_b, \
                  dim_t pd_b, inc_t ps_b, \
       void*   beta, \
       void*   c, inc_t rs_c, inc_t cs_c, \
       cntx_t* cntx, \
       dim_t ir_num_threads, dim_t ir_thread_id, \
       dim_t jr_num_threads, dim_t jr_thread_id \
     ) \
{ \
	const num_t     dt         = PASTEMAC(ch,type); \
\
	/* Alias some constants to simpler names. */ \
	const dim_t     MR         = pd_a; \
	const dim_t     NR         = pd_b; \
	/*const dim_t     PACKMR     = cs_a;*/ \
	/*const dim_t     PACKNR     = rs_b;*/ \
\
	/* Query the context for the micro-kernel address and cast it to its
	   function pointer type. */ \
	PASTECH(ch,gemm_ukr_ft) \
	                gemm_ukr   = bli_cntx_get_l3_vir_ukr_dt( dt, BLIS_GEMM_UKR, cntx ); \
\
	/* Temporary C buffer for edge cases. Note that the strides of this
	   temporary buffer are set so that they match the storage of the
	   original C matrix. For example, if C is column-stored, ct will be
	   column-stored as well. */ \
	ctype           ct[ BLIS_STACK_BUF_MAX_SIZE \
	                    / sizeof( ctype ) ] \
	                    __attribute__((aligned(BLIS_STACK_BUF_ALIGN_SIZE))); \
	const bool_t    col_pref    = bli_cntx_l3_vir_ukr_prefers_cols_dt( dt, BLIS_GEMM_UKR, cntx ); \
	const inc_t     rs_ct       = ( col_pref ? 1 : NR ); \
	const inc_t     cs_ct       = ( col_pref ? MR : 1 ); \
\
	ctype* restrict zero       = PASTEMAC(ch,0); \
	ctype* restrict a_cast     = a; \
	ctype* restrict b_cast     = b; \
	ctype* restrict c_cast     = c; \
	ctype* restrict alpha_cast = alpha; \
	ctype* restrict beta_cast  = beta; \
	ctype* restrict b1; \
	ctype* restrict c1; \
\
	dim_t           m_iter, m_left; \
	dim_t           n_iter, n_left; \
	dim_t           i, j; \
	dim_t           m_cur; \
	dim_t           n_cur; \
	inc_t           rstep_a; \
	inc_t           cstep_b; \
	inc_t           rstep_c, cstep_c; \
	auxinfo_t       aux; \
\
	/*
	   Assumptions/assertions:
	     rs_a == 1
	     cs_a == PACKMR
	     pd_a == MR
	     ps_a == stride to next micro-panel of A
	     rs_b == PACKNR
	     cs_b == 1
	     pd_b == NR
	     ps_b == stride to next micro-panel of B
	     rs_c == (no assumptions)
	     cs_c == (no assumptions)
	*/ \
\
	/* If any dimension is zero, return immediately. */ \
	if ( bli_zero_dim3( m, n, k ) ) return; \
\
	/* Clear the temporary C buffer in case it has any infs or NaNs. */ \
	PASTEMAC(ch,set0s_mxn)( MR, NR, \
	                        ct, rs_ct, cs_ct ); \
\
	/* Compute number of primary and leftover components of the m and n
	   dimensions. */ \
	n_iter = n / NR; \
	n_left = n % NR; \
\
	m_iter = m / MR; \
	m_left = m % MR; \
\
	if ( n_left ) ++n_iter; \
	if ( m_left ) ++m_iter; \
\
	/* Determine some increments used to step through A, B, and C. */ \
	rstep_a = ps_a; \
\
	cstep_b = ps_b; \
\
	rstep_c = rs_c * MR; \
	cstep_c = cs_c * NR; \
\
	/* Save the pack schemas of A and B to the auxinfo_t object. */ \
	bli_auxinfo_set_schema_a( schema_a, &aux ); \
	bli_auxinfo_set_schema_b( schema_b, &aux ); \
\
	/* Save the imaginary stride of A and B to the auxinfo_t object. */ \
	bli_auxinfo_set_is_a( is_a, &aux ); \
	bli_auxinfo_set_is_b( is_b, &aux ); \
\
	thrinfo_t* caucus    = bli_thrinfo_sub_node( thread ); \
	dim_t jr_num_threads = bli_thread_n_way( thread ); \
	dim_t jr_thread_id   = bli_thread_work_id( thread ); \
	dim_t ir_num_threads = bli_thread_n_way( caucus ); \
	dim_t ir_thread_id   = bli_thread_work_id( caucus ); \
\
	/* Loop over the n dimension (NR columns at a time). */ \
	for ( j = jr_thread_id; j < n_iter; j += jr_num_threads ) \
	{ \
		ctype* restrict a1; \
		ctype* restrict c11; \
		ctype* restrict b2; \
\
		b1 = b_cast + j * cstep_b; \
		c1 = c_cast + j * cstep_c; \
\
		n_cur = ( bli_is_not_edge_f( j, n_iter, n_left ) ? NR : n_left ); \
\
		/* Initialize our next panel of B to be the current panel of B. */ \
		b2 = b1; \
\
		/* Loop over the m dimension (MR rows at a time). */ \
		for ( i = ir_thread_id; i < m_iter; i += ir_num_threads ) \
		{ \
			ctype* restrict a2; \
\
			a1  = a_cast + i * rstep_a; \
			c11 = c1     + i * rstep_c; \
\
			m_cur = ( bli_is_not_edge_f( i, m_iter, m_left ) ? MR : m_left ); \
\
			/* Compute the addresses of the next panels of A and B. */ \
			a2 = bli_gemm_get_next_a_upanel( caucus, a1, rstep_a ); \
			if ( bli_is_last_iter( i, m_iter, ir_thread_id, ir_num_threads ) ) \
			{ \
				a2 = a_cast; \
				b2 = bli_gemm_get_next_b_upanel( thread, b1, cstep_b ); \
				if ( bli_is_last_iter( j, n_iter, jr_thread_id, jr_num_threads ) ) \
					b2 = b_cast; \
			} \
\
			/* Save addresses of next panels of A and B to the auxinfo_t
			   object. */ \
			bli_auxinfo_set_next_a( a2, &aux ); \
			bli_auxinfo_set_next_b( b2, &aux ); \
\
			/* Handle interior and edge cases separately. */ \
			if ( m_cur == MR && n_cur == NR ) \
			{ \
				/* Invoke the gemm micro-kernel. */ \
				gemm_ukr \
				( \
				  k, \
				  alpha_cast, \
				  a1, \
				  b1, \
				  beta_cast, \
				  c11, rs_c, cs_c, \
				  &aux, \
				  cntx  \
				); \
			} \
			else \
			{ \
				/* Invoke the gemm micro-kernel. */ \
				gemm_ukr \
				( \
				  k, \
				  alpha_cast, \
				  a1, \
				  b1, \
				  zero, \
				  ct, rs_ct, cs_ct, \
				  &aux, \
				  cntx  \
				); \
\
				/* Scale the bottom edge of C and add the result from above. */ \
				PASTEMAC(ch,xpbys_mxn)( m_cur, n_cur, \
				                        ct,  rs_ct, cs_ct, \
				                        beta_cast, \
				                        c11, rs_c,  cs_c ); \
			} \
		} \
	} \
\
/*
PASTEMAC(ch,fprintm)( stdout, "gemm_ker_var2: b1", k, NR, b1, NR, 1, "%4.1f", "" ); \
PASTEMAC(ch,fprintm)( stdout, "gemm_ker_var2: a1", MR, k, a1, 1, MR, "%4.1f", "" ); \
PASTEMAC(ch,fprintm)( stdout, "gemm_ker_var2: c after", m_cur, n_cur, c11, rs_c, cs_c, "%4.1f", "" ); \
*/ \
}

#if 0
GENTFUNC( float,    s, gemm_ker_var2 )
GENTFUNC( double,   d, gemm_ker_var2 )
GENTFUNC( scomplex, c, gemm_ker_var2 )
GENTFUNC( dcomplex, z, gemm_ker_var2 )
#else
INSERT_GENTFUNC_BASIC0( gemm_ker_var2 )
#endif

