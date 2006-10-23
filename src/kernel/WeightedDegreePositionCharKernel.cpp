/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2006 Soeren Sonnenburg
 * Written (W) 1999-2006 Gunnar Raetsch
 * Written (W) 2006 Alexander Zien
 * Copyright (C) 1999-2006 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include "lib/common.h"
#include "lib/io.h"
#include "kernel/WeightedDegreePositionCharKernel.h"
#include "features/Features.h"
#include "features/CharFeatures.h"

//#define NEWSTUFF

#ifdef NEWSTUFF
#include <ctype.h>

const int ProtSimThresh = 0 ;
const char ProtSimStr[21]="ARNDCQEGHILKMFPSTWYV" ;
const int ProtSimMat[20][20]=
{{  100, 22, 26, 24, 21, 38, 26, 32, 21, 27, 27, 36, 41, 19, 19, 54, 44, 10, 21, 44},
 {22,  100, 27, 21,  9, 48, 24, 13, 27, 16, 17, 60, 27, 12, 13, 29, 29,  6, 14, 20},
 {26, 27,  100, 42, 12, 42, 28, 24, 27, 16, 16, 41, 25, 14, 12, 40, 39,  7, 16, 21},
 {24, 21, 42, 100, 10, 37, 40, 18, 20, 14, 13, 34, 21, 12, 14, 33, 31,  5, 13, 18},
 {21,  9, 12, 10, 100, 14,  8,  9,  9, 19, 19, 13, 24, 13,  7, 19, 22,  6, 11, 25},
 {38, 48, 42, 37, 14, 100, 62, 24, 39, 24, 25, 62, 42, 19, 19, 50, 42, 10, 23, 31},
 {26, 24, 28, 40,  8, 62, 100, 14, 27, 13, 13, 48, 26, 12, 13, 37, 30,  6, 14, 19},
 {32, 13, 24, 18,  9, 24, 14, 100, 13, 10, 13, 23, 18, 11, 10, 28, 22,  6, 11, 16},
 {21, 27, 27, 20,  9, 39, 27, 13, 100, 14, 17, 30, 25, 18, 10, 28, 24,  8, 34, 18},
 {27, 16, 16, 14, 19, 24, 13, 10, 14, 100, 60, 23, 70, 32, 10, 23, 35, 11, 23, 89},
 {27, 17, 16, 13, 19, 25, 13, 13, 17, 60, 100, 24, 89, 41, 12, 24, 31, 14, 25, 68},
 {36, 60, 41, 34, 13, 62, 48, 23, 30, 23, 24, 100, 36, 17, 21, 42, 40,  9, 19, 29},
 {41, 27, 25, 21, 24, 42, 26, 18, 25, 70, 89, 36, 100, 47, 17, 37, 45, 16, 31, 80},
 {19, 12, 14, 12, 13, 19, 12, 11, 18, 32, 41, 17, 47, 100,  9, 20, 24, 17, 58, 37},
 {19, 13, 12, 14,  7, 19, 13, 10, 10, 10, 12, 21, 17,  9, 100, 21, 17,  4,  8, 15},
 {54, 29, 40, 33, 19, 50, 37, 28, 28, 23, 24, 42, 37, 20, 21, 100, 56,  9, 22, 34},
 {44, 29, 39, 31, 22, 42, 30, 22, 24, 35, 31, 40, 45, 24, 17, 56, 100,  9, 23, 48},
 {10,  6,  7,  5,  6, 10,  6,  6,  8, 11, 14,  9, 16, 17,  4,  9,  9, 100, 22, 13},
 {21, 14, 16, 13, 11, 23, 14, 11, 34, 23, 25, 19, 31, 58,  8, 22, 23, 22, 100, 27},
 {44, 20, 21, 18, 25, 31, 19, 16, 18, 89, 68, 29, 80, 37, 15, 34, 48, 13, 27, 100}};

int ProtSim[128][128] ;

#endif

CWeightedDegreePositionCharKernel::CWeightedDegreePositionCharKernel(LONG size, DREAL* w, INT d, 
								     INT max_mismatch_, INT * shift_, 
								     INT shift_len_, bool use_norm,
								     INT mkl_stepsize_)
    : CSimpleKernel<CHAR>(size),weights(NULL),position_weights(NULL),position_mask(NULL), counts(NULL),
      weights_buffer(NULL), mkl_stepsize(mkl_stepsize_), degree(d), 
      max_mismatch(max_mismatch_), seq_length(0), 
      sqrtdiag_lhs(NULL), sqrtdiag_rhs(NULL), initialized(false),
      use_normalization(use_norm)
{
#ifdef NEWSTUFF
    fprintf(stderr, "initializing protein similarity table -- experimental\n") ;
    for (int i=0; i<128; i++)
	for (int j=0; j<128; j++)
	    ProtSim[i][j]=0 ;
    for (int i=0; i<20; i++)
	for (int j=0; j<20; j++)
	{
	    ProtSim[(int)ProtSimStr[i]][(int)ProtSimStr[j]]=ProtSimMat[i][j] ;
	    ProtSim[(int)tolower(ProtSimStr[i])][(int)tolower(ProtSimStr[j])]=ProtSimMat[i][j] ;
	    ProtSim[(int)tolower(ProtSimStr[i])][(int)ProtSimStr[j]]=ProtSimMat[i][j] ;
	    ProtSim[(int)ProtSimStr[i]][(int)tolower(ProtSimStr[j])]=ProtSimMat[i][j] ;
	}
#endif 

    properties |= KP_LINADD | KP_KERNCOMBINATION | KP_BATCHEVALUATION;
    lhs=NULL;
    rhs=NULL;

    weights=new DREAL[d*(1+max_mismatch)];
    counts = new INT[d*(1+max_mismatch)];

    ASSERT(weights!=NULL);
    for (INT i=0; i<d*(1+max_mismatch); i++)
	weights[i]=w[i];

    shift_len = shift_len_ ;
    shift = new INT[shift_len] ;
    max_shift = 0 ;
	
    for (INT i=0; i<shift_len; i++)
    {
	shift[i] = shift_[i] ;
	if (shift[i]>max_shift)
	    max_shift = shift[i] ;
    } ;
    ASSERT(max_shift>=0 && max_shift<=shift_len) ;

#ifdef USE_TREEMEM
    TreeMemPtrMax=1024*1024/sizeof(struct Trie) ;
    TreeMemPtr=0 ;
    TreeMem = (struct Trie*)malloc(TreeMemPtrMax*sizeof(struct Trie)) ;
#endif

    length=0 ;
    trees=NULL ;
    tree_initialized=false ;
}

CWeightedDegreePositionCharKernel::~CWeightedDegreePositionCharKernel() 
{
    cleanup();

    delete[] shift;
    shift = NULL;

    delete[] counts;
    counts = NULL;

    delete[] weights ;
    weights=NULL ;

    delete[] position_weights ;
    position_weights=NULL ;

    delete[] position_mask ;
    position_mask=NULL ;

    delete[] weights_buffer ;
    weights_buffer = NULL ;

#ifdef USE_TREEMEM
    free(TreeMem) ;
#endif
}

void CWeightedDegreePositionCharKernel::remove_lhs() 
{ 
    delete_optimization();

#ifdef USE_SVMLIGHT
    if (lhs)
	cache_reset() ;
#endif

    if (sqrtdiag_lhs != sqrtdiag_rhs)
	delete[] sqrtdiag_rhs;
    delete[] sqrtdiag_lhs;

    lhs = NULL ; 
    rhs = NULL ; 
    initialized = false ;
    sqrtdiag_lhs = NULL ;
    sqrtdiag_rhs = NULL ;
	
    if (trees!=NULL)
    {
	for (INT i=0; i<seq_length; i++)
	{
	    delete trees[i];
	    trees[i]=NULL;
	}
	delete[] trees ;
	trees=NULL ;
    } ;

} ;

void CWeightedDegreePositionCharKernel::remove_rhs()
{
#ifdef USE_SVMLIGHT
    if (rhs)
	cache_reset() ;
#endif

    if (sqrtdiag_lhs != sqrtdiag_rhs)
	delete[] sqrtdiag_rhs;
    sqrtdiag_rhs = sqrtdiag_lhs ;
    rhs = lhs ;
}

  
bool CWeightedDegreePositionCharKernel::init(CFeatures* l, CFeatures* r, bool do_init)
{
    INT lhs_changed = (lhs!=l) ;
    INT rhs_changed = (rhs!=r) ;

    CIO::message(M_DEBUG, "lhs_changed: %i\n", lhs_changed) ;
    CIO::message(M_DEBUG, "rhs_changed: %i\n", rhs_changed) ;

	ASSERT(l && ((((CCharFeatures*) l)->get_alphabet()->get_alphabet()==DNA) || 
				 (((CCharFeatures*) l)->get_alphabet()->get_alphabet()==RNA)));
	ASSERT(r && ((((CCharFeatures*) r)->get_alphabet()->get_alphabet()==DNA) || 
				 (((CCharFeatures*) r)->get_alphabet()->get_alphabet()==RNA)));

    delete[] position_mask ;
    position_mask = NULL ;
	
    if (lhs_changed) 
    {
	INT alen ;
	bool afree ;
	CHAR* avec=((CCharFeatures*) l)->get_feature_vector(0, alen, afree);
		
	if (trees)
	{
	    delete_tree() ;
	    for (INT i=0; i<seq_length; i++)
	    {
		delete trees[i] ;
		trees[i]=NULL ;
	    }
	    delete[] trees ;
	    trees=NULL ;
	}
		
#ifdef OSF1
	trees=new (struct Trie**)[alen] ;		
#else
	trees=new struct Trie*[alen] ;		
#endif
	for (INT i=0; i<alen; i++)
	{
	    trees[i]=new struct Trie ;
	    trees[i]->weight=0 ;
	    for (INT j=0; j<4; j++)
		trees[i]->children[j]=NO_CHILD ;
	} 
	seq_length = alen ;
	((CCharFeatures*) l)->free_feature_vector(avec, 0, afree);
    } 

    bool result=CSimpleKernel<CHAR>::init(l,r,do_init);
    initialized = false ;
    INT i;

    CIO::message(M_DEBUG, "use normalization:%d\n", (use_normalization) ? 1 : 0);

    if (use_normalization)
    {
	if (rhs_changed)
	{
	    if (sqrtdiag_lhs != sqrtdiag_rhs)
		delete[] sqrtdiag_rhs;
	    sqrtdiag_rhs=NULL ;
	}
	if (lhs_changed)
	{
	    delete[] sqrtdiag_lhs;
	    sqrtdiag_lhs=NULL ;
	    sqrtdiag_lhs= new DREAL[lhs->get_num_vectors()];
	    ASSERT(sqrtdiag_lhs) ;
	    for (i=0; i<lhs->get_num_vectors(); i++)
		sqrtdiag_lhs[i]=1;
	}

	if (l==r)
	    sqrtdiag_rhs=sqrtdiag_lhs;
	else if (rhs_changed)
	{
	    sqrtdiag_rhs= new DREAL[rhs->get_num_vectors()];
	    ASSERT(sqrtdiag_rhs) ;

	    for (i=0; i<rhs->get_num_vectors(); i++)
		sqrtdiag_rhs[i]=1;
	}

	ASSERT(sqrtdiag_lhs);
	ASSERT(sqrtdiag_rhs);

	if (lhs_changed)
	{
	    this->lhs=(CCharFeatures*) l;
	    this->rhs=(CCharFeatures*) l;

	    //compute normalize to 1 values
	    for (i=0; i<lhs->get_num_vectors(); i++)
	    {
		sqrtdiag_lhs[i]=sqrt(compute(i,i));

		//trap divide by zero exception
		if (sqrtdiag_lhs[i]==0)
		    sqrtdiag_lhs[i]=1e-16;
	    }
	}

	// if lhs is different from rhs (train/test data)
	// compute also the normalization for rhs
	if ((sqrtdiag_lhs!=sqrtdiag_rhs) & rhs_changed)
	{
	    this->lhs=(CCharFeatures*) r;
	    this->rhs=(CCharFeatures*) r;

	    //compute normalize to 1 values
	    for (i=0; i<rhs->get_num_vectors(); i++)
	    {
		sqrtdiag_rhs[i]=sqrt(compute(i,i));

		//trap divide by zero exception
		if (sqrtdiag_rhs[i]==0)
		    sqrtdiag_rhs[i]=1e-16;
	    }
	}
    }
	
    this->lhs=(CCharFeatures*) l;
    this->rhs=(CCharFeatures*) r;

    initialized = true ;
    return result;
}

void CWeightedDegreePositionCharKernel::cleanup()
{
    delete_optimization();

    if (sqrtdiag_lhs != sqrtdiag_rhs)
	delete[] sqrtdiag_rhs;
    sqrtdiag_rhs = NULL;

    delete[] sqrtdiag_lhs;
    sqrtdiag_lhs = NULL;

    if (trees!=NULL)
    {
	for (INT i=0; i<seq_length; i++)
	{
	    delete trees[i];
	    trees[i]=NULL;
	}
	delete[] trees;
	trees=NULL;
    }

    lhs = NULL;
    rhs = NULL;

    seq_length = 0;
    initialized = false;
    tree_initialized = false;
}

bool CWeightedDegreePositionCharKernel::load_init(FILE* src)
{
    return false;
}

bool CWeightedDegreePositionCharKernel::save_init(FILE* dest)
{
    return false;
}

bool CWeightedDegreePositionCharKernel::init_optimization(INT count, INT * IDX, DREAL * alphas, INT tree_num, INT upto_tree)
{
    if (upto_tree<0)
	upto_tree=tree_num;

    if (max_mismatch!=0)
    {
	CIO::message(M_ERROR, "CWeightedDegreePositionCharKernel optimization not implemented for mismatch!=0\n") ;
	return false ;
    }

    delete_optimization();

    CIO::message(M_DEBUG, "initializing CWeightedDegreePositionCharKernel optimization\n") ;
    int i=0;
    for (i=0; i<count; i++)
    {
	if (tree_num<0)
	{
	    if ( (i % (count/10+1)) == 0)
		CIO::progress(i,0,count);
	    add_example_to_tree(IDX[i], alphas[i]);
	}
	else
	{
	    for (INT t=tree_num; t<=upto_tree; t++)
		add_example_to_single_tree(IDX[i], alphas[i], t);
	}
    }

    if (tree_num<0)
	CIO::message(M_DEBUG, "done.           \n");
	
    set_is_initialized(true) ;
    return true ;
}

bool CWeightedDegreePositionCharKernel::delete_optimization() 
{ 

    CIO::message(M_DEBUG, "deleting CWeightedDegreePositionCharKernel optimization\n");

    if (get_is_initialized())
    {
	delete_tree(NULL); 
	set_is_initialized(false);
	return true;
    }
	
    return false;
}

DREAL CWeightedDegreePositionCharKernel::compute_with_mismatch(CHAR* avec, INT alen, CHAR* bvec, INT blen) 
{
    DREAL sum0=0 ;
    DREAL *sum1=new DREAL[max_shift] ;
    for (INT i=0; i<max_shift; i++)
	sum1[i]=0 ;
	
    // no shift
    for (INT i=0; i<alen; i++)
    {
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;

	INT mismatches=0;
	DREAL sumi = 0.0 ;
	for (INT j=0; (j<degree) && (i+j<alen); j++)
	{
	    if (avec[i+j]!=bvec[i+j])
	    {
		mismatches++ ;
		if (mismatches>max_mismatch)
		    break ;
	    } ;
	    sumi += weights[j+degree*mismatches];
	}
	if (position_weights!=NULL)
	    sum0 += position_weights[i]*sumi ;
	else
	    sum0 += sumi ;
    } ;
	
    for (INT i=0; i<alen; i++)
    {
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	for (INT k=1; (k<=shift[i]) && (i+k<alen); k++)
	{
	    DREAL sumi = 0.0 ;
	    // shift in sequence a
	    INT mismatches=0;
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		if (avec[i+j+k]!=bvec[i+j])
		{
		    mismatches++ ;
		    if (mismatches>max_mismatch)
			break ;
		} ;
		sumi += weights[j+degree*mismatches];
	    }
	    // shift in sequence b
	    mismatches=0;
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		if (avec[i+j]!=bvec[i+j+k])
		{
		    mismatches++ ;
		    if (mismatches>max_mismatch)
			break ;
		} ;
		sumi += weights[j+degree*mismatches];
	    }
	    if (position_weights!=NULL)
		sum1[k-1] += position_weights[i]*sumi ;
	    else
		sum1[k-1] += sumi ;
	} ;
    }

    DREAL result = sum0 ;
    for (INT i=0; i<max_shift; i++)
	result += sum1[i]/(2*(i+1)) ;

    delete[] sum1 ;
    return result ;
}

#ifdef NEWSTUFF

DREAL CWeightedDegreePositionCharKernel::compute_without_mismatch(CHAR* avec, INT alen, CHAR* bvec, INT blen) 
{
    DREAL sum0=0 ;
    DREAL *sum1=new DREAL[max_shift] ;
    for (INT i=0; i<max_shift; i++)
	sum1[i]=0 ;
	
    // no shift
    for (INT i=0; i<alen; i++)
    {
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	DREAL sumi = 0.0 ;
	for (INT j=0; (j<degree) && (i+j<alen); j++)
	{
	    int sim = ProtSim[(int)avec[i+j]][(int)bvec[i+j]] ;
	    if (sim<ProtSimThresh)
		break ;
	    sumi += weights[j]*((double)sim) ;
	}
	sumi/=100.0 ;
	if (position_weights!=NULL)
	    sum0 += position_weights[i]*sumi ;
	else
	    sum0 += sumi ;
    } ;
	
    for (INT i=0; i<alen; i++)
    {
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	for (INT k=1; (k<=shift[i]) && (i+k<alen); k++)
	{
	    DREAL sumi = 0.0 ;
	    // shift in sequence a
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		int sim = ProtSim[(int)avec[i+j+k]][(int)bvec[i+j]] ;
		if (sim<ProtSimThresh)
		    break ;
		sumi += weights[j]*((double)sim) ;
	    }
	    // shift in sequence b
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		int sim = ProtSim[(int)avec[i+j]][(int)bvec[i+j+k]] ;
		if (sim<ProtSimThresh)
		    break ;
		sumi += weights[j]*((double)sim) ;
	    }
	    sumi/=100.0 ;
	    if (position_weights!=NULL)
		sum1[k-1] += position_weights[i]*sumi ;
	    else
		sum1[k-1] += sumi ;
	} ;
    }

    DREAL result = sum0 ;
    for (INT i=0; i<max_shift; i++)
	result += sum1[i]/(2*(i+1)) ;

    delete[] sum1 ;
    return result ;
}

#else

DREAL CWeightedDegreePositionCharKernel::compute_without_mismatch(CHAR* avec, INT alen, CHAR* bvec, INT blen) 
{
    DREAL sum0=0 ;
    DREAL *sum1=new DREAL[max_shift] ;
    for (INT i=0; i<max_shift; i++)
	sum1[i]=0 ;
	
    // no shift
    for (INT i=0; i<alen; i++)
    {
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	DREAL sumi = 0.0 ;
	for (INT j=0; (j<degree) && (i+j<alen); j++)
	{
	    if (avec[i+j]!=bvec[i+j])
		break ;
	    sumi += weights[j];
	}
	if (position_weights!=NULL)
	    sum0 += position_weights[i]*sumi ;
	else
	    sum0 += sumi ;
    } ;
	
    for (INT i=0; i<alen; i++)
    {
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	for (INT k=1; (k<=shift[i]) && (i+k<alen); k++)
	{
	    DREAL sumi = 0.0 ;
	    // shift in sequence a
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		if (avec[i+j+k]!=bvec[i+j])
		    break ;
		sumi += weights[j];
	    }
	    // shift in sequence b
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		if (avec[i+j]!=bvec[i+j+k])
		    break ;
		sumi += weights[j];
	    }
	    if (position_weights!=NULL)
		sum1[k-1] += position_weights[i]*sumi ;
	    else
		sum1[k-1] += sumi ;
	} ;
    }

    DREAL result = sum0 ;
    for (INT i=0; i<max_shift; i++)
	result += sum1[i]/(2*(i+1)) ;

    delete[] sum1 ;
    return result ;
}

#endif

DREAL CWeightedDegreePositionCharKernel::compute_without_mismatch_matrix(CHAR* avec, INT alen, CHAR* bvec, INT blen) 
{
    DREAL sum0=0 ;
    DREAL *sum1=new DREAL[max_shift] ;
    for (INT i=0; i<max_shift; i++)
	sum1[i]=0 ;
	
    if (!position_mask)
    {		
	position_mask = new bool[alen] ;
	for (INT i=0; i<alen; i++)
	{
	    position_mask[i]=false ;
			
	    for (INT j=0; j<degree; j++)
		if (weights[i*degree+j]!=0.0)
		{
		    position_mask[i]=true ;
		    break ;
		}
	}
    }
	
    // no shift
    for (INT i=0; i<alen; i++)
    {
	if (!position_mask[i])
	    continue ;
		
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	DREAL sumi = 0.0 ;
	for (INT j=0; (j<degree) && (i+j<alen); j++)
	{
	    if (avec[i+j]!=bvec[i+j])
		break ;
	    sumi += weights[i*degree+j];
	}
	if (position_weights!=NULL)
	    sum0 += position_weights[i]*sumi ;
	else
	    sum0 += sumi ;
    } ;
	
    for (INT i=0; i<alen; i++)
    {
	if (!position_mask[i])
	    continue ;		
	if ((position_weights!=NULL) && (position_weights[i]==0.0))
	    continue ;
	for (INT k=1; (k<=shift[i]) && (i+k<alen); k++)
	{
	    DREAL sumi = 0.0 ;
	    // shift in sequence a
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		if (avec[i+j+k]!=bvec[i+j])
		    break ;
		sumi += weights[i*degree+j];
	    }
	    // shift in sequence b
	    for (INT j=0; (j<degree) && (i+j+k<alen); j++)
	    {
		if (avec[i+j]!=bvec[i+j+k])
		    break ;
		sumi += weights[i*degree+j];
	    }
	    if (position_weights!=NULL)
		sum1[k-1] += position_weights[i]*sumi ;
	    else
		sum1[k-1] += sumi ;
	} ;
    }

    DREAL result = sum0 ;
    for (INT i=0; i<max_shift; i++)
	result += sum1[i]/(2*(i+1)) ;

    delete[] sum1 ;
    return result ;
}


DREAL CWeightedDegreePositionCharKernel::compute(INT idx_a, INT idx_b)
{
    INT alen, blen;
    bool afree, bfree;

    CHAR* avec=((CCharFeatures*) lhs)->get_feature_vector(idx_a, alen, afree);
    CHAR* bvec=((CCharFeatures*) rhs)->get_feature_vector(idx_b, blen, bfree);

    // can only deal with strings of same length
    ASSERT(alen == blen);
    ASSERT(shift_len == alen) ;

    DREAL sqrt_a=1;
    DREAL sqrt_b=1;

    if (initialized && use_normalization)
    {
	sqrt_a=sqrtdiag_lhs[idx_a];
	sqrt_b=sqrtdiag_rhs[idx_b];
    }

    DREAL sqrt_both=sqrt_a*sqrt_b;

    DREAL result = 0 ;
    if (max_mismatch > 0)
	result = compute_with_mismatch(avec, alen, bvec, blen) ;
    else if (length==0)
	result = compute_without_mismatch(avec, alen, bvec, blen) ;
    else
	result = compute_without_mismatch_matrix(avec, alen, bvec, blen) ;
  
    ((CCharFeatures*) lhs)->free_feature_vector(avec, idx_a, afree);
    ((CCharFeatures*) rhs)->free_feature_vector(bvec, idx_b, bfree);
  
    result/=sqrt_both;
  
    return result ;
}

void CWeightedDegreePositionCharKernel::add_example_to_tree(INT idx, DREAL alpha)
{
    INT len ;
    bool free ;
    CHAR* char_vec=((CCharFeatures*) lhs)->get_feature_vector(idx, len, free);
    ASSERT(max_mismatch==0) ;
    INT *vec = new INT[len] ;

    if (use_normalization)
	alpha /=  sqrtdiag_lhs[idx] ;

    for (INT i=0; i<len; i++)
	vec[i]=((CCharFeatures*) lhs)->get_alphabet()->remap_to_bin(char_vec[i]);

    for (INT i=0; i<len; i++)
    {
	INT max_s=-1;

	if (opt_type==SLOWBUTMEMEFFICIENT)
	    max_s=0;
	else if (opt_type==FASTBUTMEMHUNGRY)
	    max_s=shift[i];
	else
	    CIO::message(M_ERROR, "unknown optimization type\n");

	for (INT s=max_s; s>=0; s--)
	{
	    struct Trie *tree = trees[i] ;

	    for (INT j=0; (i+j+s<len); j++)
	    {
		if ((j<degree-1) && (tree->children[vec[i+j+s]]!=NO_CHILD))
		{
#ifdef USE_TREEMEM
		    tree=&TreeMem[tree->children[vec[i+j+s]]] ;
#else
		    tree=tree->children[vec[i+j+s]] ;
#endif
		    tree->weight += (s==0) ? (alpha) : (alpha/(2*s));
		}
		else if (j==degree-1)
		{
		    tree->child_weights[vec[i+j+s]] += (s==0) ? (alpha) : (alpha/(2*s));
		    break ;
		}
		else
		{
#ifdef USE_TREEMEM
		    tree->children[vec[i+j+s]]=TreeMemPtr++;
		    INT tmp = tree->children[vec[i+j+s]] ;
		    check_treemem() ;
		    tree=&TreeMem[tmp] ;
#else
		    tree->children[vec[i+j+s]]=new struct Trie ;
		    ASSERT(tree->children[vec[i+j+s]]!=NULL) ;
		    tree=tree->children[vec[i+j+s]] ;
#endif
		    if (j==degree-2)
		    {
			for (INT k=0; k<4; k++)
			    tree->child_weights[k]=0 ;
		    }
		    else
		    {
			for (INT k=0; k<4; k++)
			    tree->children[k]=NO_CHILD;
		    }
		    tree->weight = (s==0) ? (alpha) : (alpha/(2*s));
		}
	    }

	    if ((s==0) || (i+s>=len))
		continue;

	    tree = trees[i+s] ;

	    for (INT j=0; (i+j<len); j++)
	    {
		if ((j<degree-1) && (tree->children[vec[i+j]]!=NO_CHILD))
		{
#ifdef USE_TREEMEM
		    tree=&TreeMem[tree->children[vec[i+j]]] ;
#else
		    tree=tree->children[vec[i+j]] ;
#endif
		    tree->weight +=  alpha/(2*s);
		}
		else if (j==degree-1)
		{
		    tree->child_weights[vec[i+j]] += alpha/(2*s);
		    break ;
		}
		else
		{
#ifdef USE_TREEMEM
		    tree->children[vec[i+j]]=TreeMemPtr++;
		    INT tmp = tree->children[vec[i+j]] ;
		    check_treemem() ;
		    tree=&TreeMem[tmp] ;
#else
		    tree->children[vec[i+j]]=new struct Trie ;
		    ASSERT(tree->children[vec[i+j]]!=NULL) ;
		    tree=tree->children[vec[i+j]] ;
#endif
		    if (j==degree-2)
		    {
			for (INT k=0; k<4; k++)
			    tree->child_weights[k]=0 ;
		    }
		    else
		    {
			for (INT k=0; k<4; k++)
			    tree->children[k]=NO_CHILD;
		    }
		    tree->weight = alpha/(2*s);
		}
	    }
	}
    }

    ((CCharFeatures*) lhs)->free_feature_vector(char_vec, idx, free);
    delete[] vec ;
    tree_initialized=true ;
}

void CWeightedDegreePositionCharKernel::add_example_to_single_tree(INT idx, DREAL alpha, INT tree_num) 
{
    INT len ;
    bool free ;
    CHAR* char_vec=((CCharFeatures*) lhs)->get_feature_vector(idx, len, free);
    ASSERT(max_mismatch==0) ;
    INT *vec = new INT[len] ;

    if (use_normalization)
	alpha /=  sqrtdiag_lhs[idx] ;

    INT max_s=-1;
	
    if (opt_type==SLOWBUTMEMEFFICIENT)
		max_s=0;
    else if (opt_type==FASTBUTMEMHUNGRY)
		max_s=shift[tree_num];
    else
		CIO::message(M_ERROR, "unknown optimization type\n");
	
    for (INT i=CMath::max(0,tree_num-max_shift); i<CMath::min(len,tree_num+degree+max_shift); i++)
		vec[i]=((CCharFeatures*) lhs)->get_alphabet()->remap_to_bin(char_vec[i]);
	
    for (INT s=max_s; s>=0; s--)
    {
		struct Trie *tree = trees[tree_num] ;
		
		if (tree_num == tree_num)
		{
			for (INT j=0; (tree_num+j+s<len); j++)
			{
				if ((j<degree-1) && (tree->children[vec[tree_num+j+s]]!=NO_CHILD))
				{
#ifdef USE_TREEMEM
					tree=&TreeMem[tree->children[vec[tree_num+j+s]]] ;
#else
					tree=tree->children[vec[tree_num+j+s]] ;
#endif
					tree->weight += (s==0) ? (alpha) : (alpha/(2*s));
				}
				else if (j==degree-1)
				{
					tree->child_weights[vec[tree_num+j+s]] += (s==0) ? (alpha) : (alpha/(2*s));
					break ;
				}
				else
				{
#ifdef USE_TREEMEM
					tree->children[vec[tree_num+j+s]]=TreeMemPtr++;
					INT tmp = tree->children[vec[tree_num+j+s]] ;
					check_treemem() ;
					tree=&TreeMem[tmp] ;
#else
					tree->children[vec[tree_num+j+s]]=new struct Trie ;
					ASSERT(tree->children[vec[tree_num+j+s]]!=NULL) ;
					tree=tree->children[vec[tree_num+j+s]] ;
#endif
					if (j==degree-2)
					{
						for (INT k=0; k<4; k++)
							tree->child_weights[k]=0 ;
					}
					else
					{
						for (INT k=0; k<4; k++)
							tree->children[k]=NO_CHILD;
					}
					tree->weight = (s==0) ? (alpha) : (alpha/(2*s));
				}
			}
		}
    }
	
    if (opt_type==FASTBUTMEMHUNGRY)
    {
		for (INT i=CMath::max(0,tree_num-max_shift); i<CMath::min(len,tree_num+max_shift+1); i++)
		{
			INT s=tree_num-i;
			
			if (i+s<len && s>=1 && s<=shift[i])
			{
				struct Trie *tree = trees[tree_num] ;
				
				for (INT j=0; (i+j<len); j++)
				{
					if ((j<degree-1) && (tree->children[vec[i+j]]!=NO_CHILD))
					{
#ifdef USE_TREEMEM
						tree=&TreeMem[tree->children[vec[i+j]]] ;
#else
						tree=tree->children[vec[i+j]] ;
#endif
						tree->weight +=  alpha/(2*s);
					}
					else if (j==degree-1)
					{
						tree->child_weights[vec[i+j]] += alpha/(2*s);
						break ;
					}
					else
					{
#ifdef USE_TREEMEM
						tree->children[vec[i+j]]=TreeMemPtr++;
						INT tmp = tree->children[vec[i+j]] ;
						check_treemem() ;
						tree=&TreeMem[tmp] ;
#else
						tree->children[vec[i+j]]=new struct Trie ;
						ASSERT(tree->children[vec[i+j]]!=NULL) ;
						tree=tree->children[vec[i+j]] ;
#endif
						if (j==degree-2)
						{
							for (INT k=0; k<4; k++)
								tree->child_weights[k]=0 ;
						}
						else
						{
							for (INT k=0; k<4; k++)
								tree->children[k]=NO_CHILD;
						}
						tree->weight = alpha/(2*s);
					}
				}
			}
		}
    }
	
    ((CCharFeatures*) lhs)->free_feature_vector(char_vec, idx, free);
    delete[] vec ;
    tree_initialized=true ;
}

DREAL CWeightedDegreePositionCharKernel::compute_by_tree(INT idx)
{
    DREAL sum = 0 ;
    INT len ;
    bool free ;
    CHAR* char_vec=((CCharFeatures*) rhs)->get_feature_vector(idx, len, free);
    ASSERT(max_mismatch==0) ;
    INT *vec = new INT[len] ;

    for (INT i=0; i<len; i++)
	vec[i]=((CCharFeatures*) lhs)->get_alphabet()->remap_to_bin(char_vec[i]);

    for (INT i=0; i<len; i++)
	sum += compute_by_tree_helper(vec, len, i, i, i) ;

    if (opt_type==SLOWBUTMEMEFFICIENT)
    {
	for (INT i=0; i<len; i++)
	{
	    for (INT k=1; (k<=shift[i]) && (i+k<len); k++)
	    {
		sum+=compute_by_tree_helper(vec, len, i, i+k, i)/(2*k) ;
		sum+=compute_by_tree_helper(vec, len, i+k, i, i)/(2*k) ;
	    }
	}
    }

    ((CCharFeatures*) rhs)->free_feature_vector(char_vec, idx, free);
    delete[] vec ;

    if (use_normalization)
	return sum/sqrtdiag_rhs[idx];
    else
	return sum;
}

void CWeightedDegreePositionCharKernel::compute_by_tree(INT idx, DREAL* LevelContrib)
{
    INT len ;
    bool free ;
    CHAR* char_vec=((CCharFeatures*) rhs)->get_feature_vector(idx, len, free);
    ASSERT(max_mismatch==0) ;
    INT *vec = new INT[len] ;

    for (INT i=0; i<len; i++)
	vec[i]=((CCharFeatures*) lhs)->get_alphabet()->remap_to_bin(char_vec[i]);

    DREAL factor = 1.0 ;

    if (use_normalization)
	factor = 1.0/sqrtdiag_rhs[idx] ;

    for (INT i=0; i<len; i++)
	compute_by_tree_helper(vec, len, i, i, i, LevelContrib, factor) ;

    if (opt_type==SLOWBUTMEMEFFICIENT)
    {
	for (INT i=0; i<len; i++)
	    for (INT k=1; (k<=shift[i]) && (i+k<len); k++)
	    {
		compute_by_tree_helper(vec, len, i, i+k, i, LevelContrib, factor/(2*k)) ;
		compute_by_tree_helper(vec, len, i+k, i, i, LevelContrib, factor/(2*k)) ;
	    }
    }

    ((CCharFeatures*) rhs)->free_feature_vector(char_vec, idx, free);
    delete[] vec ;
}

DREAL CWeightedDegreePositionCharKernel::compute_abs_weights_tree(struct Trie* p_tree, INT depth) 
{
    DREAL ret=0 ;

    if (p_tree==NULL)
	return 0 ;
		
    if (depth==degree-2)
    {
	ret+=(p_tree->weight) ;

	for (INT k=0; k<4; k++)
	    ret+=(p_tree->child_weights[k]) ;
		
	return ret ;
    }

    ret+=(p_tree->weight) ;

    for (INT i=0; i<4; i++)
    {
	if (p_tree->children[i]!=NO_CHILD)
	{
#ifdef USE_TREEMEM
	    ret += compute_abs_weights_tree(&TreeMem[p_tree->children[i]], depth+1)  ;
#else
	    ret += compute_abs_weights_tree(p_tree->children[i], depth+1)  ;
#endif
	}
    }

    return ret ;
}

DREAL *CWeightedDegreePositionCharKernel::compute_abs_weights(int &len) 
{
    DREAL * sum=new DREAL[seq_length*4] ;
    for (INT i=0; i<seq_length*4; i++)
	sum[i]=0 ;
    len=seq_length ;
	
    for (INT i=0; i<seq_length; i++)
    {
	struct Trie *tree = trees[i] ;
	ASSERT(tree!=NULL) ;
	for (INT k=0; k<4; k++)
#ifdef USE_TREEMEM
	    sum[i*4+k]=compute_abs_weights_tree(&TreeMem[tree->children[k]], 0) ;
#else
	sum[i*4+k]=compute_abs_weights_tree(tree->children[k], 0) ;
#endif
    }

    return sum ;
}

void CWeightedDegreePositionCharKernel::delete_tree(struct Trie * p_tree, INT depth)
{
    if (p_tree==NULL)
    {
	if (trees==NULL)
	    return;

	for (INT i=0; i<seq_length; i++)
	{
#ifndef USE_TREEMEM
	    delete_tree(trees[i], depth+1);
#endif
	    for (INT k=0; k<4; k++)
		trees[i]->children[k]=NO_CHILD;
	}

	tree_initialized=false;
#ifdef USE_TREEMEM
	TreeMemPtr=0;
#endif
	return;
    }

    if (depth==degree-2)
	return;
	
    for (INT i=0; i<4; i++)
    {
	if (p_tree->children[i]!=NO_CHILD)
	{
#ifndef USE_TREEMEM
	    delete_tree(p_tree->children[i], depth+1);
	    delete p_tree->children[i];
#endif
	    p_tree->children[i]=NO_CHILD;
	} 
	p_tree->weight=0;
    }
} 

bool CWeightedDegreePositionCharKernel::set_weights(DREAL* ws, INT d, INT len)
{
    CIO::message(M_DEBUG, "degree = %i  d=%i\n", degree, d) ;
    degree = d ;
    length=len;
	
    if (len <= 0)
	len=1;
	
    delete[] weights;
    weights=new DREAL[d*len];

    delete[] position_mask ;
    position_mask=NULL ;
	
    if (weights)
    {
	for (int i=0; i<degree*len; i++)
	    weights[i]=ws[i];
	return true;
    }
    else
	return false;
}

bool CWeightedDegreePositionCharKernel::set_position_weights(DREAL* pws, INT len)
{
    if (len==0)
    {
	delete[] position_weights ;
	position_weights = NULL ;
    }
    if (seq_length==0)
	seq_length = len ;

    if (seq_length!=len) 
    {
	CIO::message(M_ERROR, "seq_length = %i, position_weights_length=%i\n", seq_length, len) ;
	return false ;
    }
    delete[] position_weights;
    position_weights=new DREAL[len];
	
    if (position_weights)
    {
	for (int i=0; i<len; i++)
	    position_weights[i]=pws[i];
	return true;
    }
    else
	return false;
}


DREAL* CWeightedDegreePositionCharKernel::compute_batch(INT& num_vec, DREAL* result, INT num_suppvec, INT* IDX, DREAL* alphas, DREAL factor)
{
    ASSERT(get_rhs());
    num_vec=get_rhs()->get_num_vectors();
    ASSERT(num_vec>0);
    INT num_feat=((CCharFeatures*) get_rhs())->get_num_features();
    ASSERT(num_feat>0);
	
    if (!result)
    {
		result= new DREAL[num_vec];
		ASSERT(result);
		memset(result, 0, sizeof(DREAL)*num_vec);
    }
	
    INT* vec= new INT[num_feat];
	
    EOptimizationType opt_type_backup=get_optimization_type();
    set_optimization_type(FASTBUTMEMHUNGRY);
	
    for (INT j=0; j<num_feat; j++)
    {
		init_optimization(num_suppvec, IDX, alphas, j);
		
		for (INT i=0; i<num_vec; i++)
		{
			INT len=0;
			bool freevec;
			CHAR* char_vec=((CCharFeatures*) rhs)->get_feature_vector(i, len, freevec);
			for (INT k=CMath::max(0,j-max_shift); k<CMath::min(len,j+degree+max_shift); k++)
				vec[k]=((CCharFeatures*) lhs)->get_alphabet()->remap_to_bin(char_vec[k]);
			
			if (use_normalization)
				result[i] += factor*compute_by_tree_helper(vec, len, j, j, j)/sqrtdiag_rhs[i];
			else
				result[i] += factor*compute_by_tree_helper(vec, len, j, j, j);
			
			((CCharFeatures*) rhs)->free_feature_vector(char_vec, i, freevec);
		}
		CIO::progress(j,0,num_feat);
    }
    set_optimization_type(opt_type_backup);
	
    delete[] vec;
	
    return result;
}

void CWeightedDegreePositionCharKernel::compute_scoring_helper(struct Trie* tree, INT i, INT j, DREAL weight, INT d, INT max_degree, INT num_feat, INT num_sym, INT sym_offset, INT offs, DREAL* result)
{
    if (i+j<num_feat)
    {
	DREAL decay=1.0; //no decay by default
	//if (j>d)
	//	decay=pow(0.5,j); //marginalize out lower order matches

	if (j<degree-1)
	{
	    for (INT k=0; k<num_sym; k++)
	    {
		if (tree->children[k]!=NO_CHILD)
		{
#ifdef USE_TREEMEM
		    struct Trie* child=&TreeMem[tree->children[k]];
#else
		    struct Trie* child=tree->children[k];
#endif
		    //continue recursion if not yet at max_degree, else add to result
		    if (d<max_degree-1)
			compute_scoring_helper(child, i, j+1, weight+decay*child->weight, d+1, max_degree, num_feat, num_sym, sym_offset, num_sym*offs+k, result);
		    else
			result[sym_offset*(i+j-max_degree+1)+num_sym*offs+k] += weight+decay*child->weight;

		    ////do recursion starting from this position
		    if (d==0)
			compute_scoring_helper(child, i, j+1, 0.0, 0, max_degree, num_feat, num_sym, sym_offset, offs, result);
		}
	    }
	}
	else if (j==degree-1)
	{
	    for (INT k=0; k<num_sym; k++)
	    {
		//continue recursion if not yet at max_degree, else add to result
		if (d<max_degree-1 && i<num_feat-1)
		    compute_scoring_helper(trees[i+1], i+1, 0, weight+decay*tree->child_weights[k], d+1, max_degree, num_feat, num_sym, sym_offset, num_sym*offs+k, result);
		else
		    result[sym_offset*(i+j-max_degree+1)+num_sym*offs+k] += weight+decay*tree->child_weights[k];
	    }
	}
    }
}


CWeightedDegreePositionCharKernel::Trie* CWeightedDegreePositionCharKernel::get_tree_at_position(INT i)
{
    const INT num_feat=((CCharFeatures*) get_rhs())->get_num_features();
    ASSERT(num_feat>0 && i<num_feat);
    return trees[i];
}


void CWeightedDegreePositionCharKernel::count( const DREAL w, const INT depth, const struct TreeParseInfo info, const INT p, INT* x, const INT k )
{
    ASSERT( fabs(w) < 1e10 );
    ASSERT( x[depth] >= 0 );
    ASSERT( x[depth+1] < 0 );
    if ( depth < k ) {
	return;
    }
    //ASSERT( info.margFactors[ depth-k ] == pow( 0.25, depth-k ) );
    const INT nofKmers = info.nofsKmers[k];
    const DREAL margWeight =  w * info.margFactors[ depth-k ];
    const INT m_a = depth - k + 1;
    const INT m_b = info.num_feat - p;
    const INT m = ( m_a < m_b ) ? m_a : m_b;
    // all proper k-substrings
    const INT offset0 = nofKmers * p;
    register INT i;
    register INT offset;
    offset = offset0;
    for( i = 0; i < m; ++i ) {
        const INT y = info.substrs[i+k+1];
	info.C_k[ y + offset ] += margWeight;
	offset += nofKmers;
    }
    if( depth > k ) {
	// k-prefix
	const INT offsR = info.substrs[k+1] + offset0;
	info.R_k[offsR] += margWeight;
	// k-suffix
	if( p+depth-k < info.num_feat ) {
  	    const INT offsL = info.substrs[depth+1] + nofKmers * (p+depth-k);
	    info.L_k[offsL] += margWeight; 
	}
    }
    //    # N.x = substring represented by N
    //    # N.d = length of N.x
    //    # N.s = starting position of N.x
    //    # N.w = weight for feature represented by N
    //    if( N.d >= k )
    //      margContrib = w / 4^(N.d-k)
    //      for i = 1 to (N.d-k+1)
    //        y = N.x[i:(i+k-1)]  # overlapped k-mer
    //        C_k[ N.s+i-1, y ] += margContrib
    //      end;
    //      if( N.d > k )
    //        L_k[ N.s+N.d-k, N.x[N.d-k+(1:k)] ] += margContrib  # j-suffix of N.x
    //        R_k[ N.s,       N.x[1:k]         ] += margContrib  # j-prefix of N.x
    //      end;
    //    end;
}


void CWeightedDegreePositionCharKernel::traverse( const struct Trie* tree, const INT p, struct TreeParseInfo info, const INT depth, INT* const x, const INT k )
{
    const INT num_sym = info.num_sym;
    const INT y0 = info.y0;
    const INT y1 = (k==0) ? 0 : y0 - ( (depth<k) ? 0 : info.nofsKmers[k-1] * x[depth-k] );
    //const INT temp = info.substrs[depth]*num_sym - ( (depth<=k) ? 0 : info.nofsKmers[k] * x[depth-k-1] );
    //if( !( info.y0 == temp ) ) {
    //  printf( "\n temp=%d y0=%d k=%d depth=%d \n", temp, info.y0, k, depth );
    //}
    //ASSERT( info.y0 == temp );
    INT sym;
    ASSERT( depth < degree );
    //ASSERT( 0 <= info.substrs[depth] && info.substrs[depth] < info.nofsKmers[k] );
    if (depth<degree-1)
    {
	for( sym=0; sym<num_sym; ++sym ) {
#ifdef USE_TREEMEM
	    const INT childNum = tree->children[ sym ];
	    if( childNum != NO_CHILD ) {
	        struct Trie* child = &TreeMem[ childNum ];
#else
	    struct Trie* child = tree->children[ sym ];
	    if( child != NO_CHILD ) {
#endif
		x[depth] = sym;
		info.substrs[depth+1] = y0 + sym;
		info.y0 = (k==0) ? 0 : (y1+sym)*num_sym;
		//ASSERT( info.y0 == ( info.substrs[depth+1]*num_sym - ( (depth<k) ? 0 : info.nofsKmers[k] * x[depth-k] ) ) );
		count( child->weight, depth, info, p, x, k );
		traverse( child, p, info, depth+1, x, k );
		x[depth] = -1;
	    }
	}
    }
    else if( depth == degree-1 )
    {
        for( sym=0; sym<num_sym; ++sym ) {
	    const DREAL w = tree->child_weights[ sym ];
	    if( w != 0.0 ) {
		x[depth] = sym;
		info.substrs[depth+1] = y0 + sym;
		info.y0 = (k==0) ? 0 : (y1+sym)*num_sym;
		//ASSERT( info.y0 == ( info.substrs[depth+1]*num_sym - ( (depth<k) ? 0 : info.nofsKmers[k] * x[depth-k] ) ) );
		count( w, depth, info, p, x, k );
		x[depth] = -1;
	    }
	}
    }
    //info.substrs[depth+1] = -1;
    //info.y0 = temp;
}


DREAL* CWeightedDegreePositionCharKernel::compute_scoring(INT max_degree, INT& num_feat, INT& num_sym, DREAL* result, INT num_suppvec, INT* IDX, DREAL* alphas)
{
    num_feat=((CCharFeatures*) get_rhs())->get_num_features();
    ASSERT(num_feat>0);
    ASSERT(((CCharFeatures*) get_rhs())->get_alphabet()->get_alphabet() == DNA);
    num_sym=4; //for now works only w/ DNA

    // === variables
    INT* nofsKmers = new INT[ max_degree ];
    DREAL** C = new DREAL*[ max_degree ];
    DREAL** L = new DREAL*[ max_degree ];
    DREAL** R = new DREAL*[ max_degree ];
    INT i;
    INT k;

    // --- return table
    INT bigtabSize = 0;
    for( k = 0; k < max_degree; ++k ) {
	nofsKmers[k] = (INT) pow( num_sym, k+1 );
        const INT tabSize = nofsKmers[k] * num_feat;
        bigtabSize += tabSize;
    }
    result= new DREAL[ bigtabSize ];

    // --- auxilliary tables
    INT tabOffs=0;
    for( k = 0; k < max_degree; ++k )
    {
	const INT tabSize = nofsKmers[k] * num_feat;
	C[k] = &result[tabOffs];
	L[k] = new DREAL[ tabSize ];
	R[k] = new DREAL[ tabSize ];
	tabOffs+=tabSize;
	for(i = 0; i < tabSize; i++ )
	{
	    C[k][i] = 0.0;
	    L[k][i] = 0.0;
	    R[k][i] = 0.0;
	}
    }

    // --- tree parsing info
    DREAL* margFactors = new DREAL[ degree ];
    INT* x = new INT[ degree+1 ];
    INT* substrs = new INT[ degree+1 ];
    // - fill arrays
    margFactors[0] = 1.0;
    substrs[0] = 0;
    for( k=1; k < degree; ++k ) {
      margFactors[k] = 0.25 * margFactors[k-1];
      substrs[k] = -1;
    }
    substrs[degree] = -1;
    // - fill struct
    struct TreeParseInfo info;
    info.num_sym = num_sym;
    info.num_feat = num_feat;
    info.p = -1;
    info.k = -1;
    info.nofsKmers = nofsKmers;
    info.margFactors = margFactors;
    info.x = x;
    info.substrs = substrs;
    info.y0 = 0;
    info.C_k = NULL;
    info.L_k = NULL;
    info.R_k = NULL;

    // === main loop
    i = 0; // total progress
    for( k = 0; k < max_degree; ++k )
    {
	const INT nofKmers = nofsKmers[ k ];
	info.C_k = C[k];
	info.L_k = L[k];
	info.R_k = R[k];

	// --- run over all trees
	for(INT p = 0; p < num_feat; ++p )
	{
	    init_optimization( num_suppvec, IDX, alphas, p );
	    const Trie* const tree = get_tree_at_position( p );
	    for(INT j = 0; j < degree+1; j++ ) {
		x[j] = -1;
	    }
	    traverse( tree, p, info, 0, x, k );
	    CIO::progress(i++,0,num_feat*max_degree);
	}

	// --- add partial overlap scores
	if( k > 0 ) {
	    const INT j = k - 1;
	    const INT nofJmers = (INT) pow( num_sym, j+1 );
	    for(INT p = 0; p < num_feat; ++p ) {
		const INT offsetJ = nofJmers * p;
		const INT offsetJ1 = nofJmers * (p+1);
		const INT offsetK = nofKmers * p;
		INT y;
		INT sym;
		for( y = 0; y < nofJmers; ++y ) {
		    for( sym = 0; sym < num_sym; ++sym ) {
			const INT y_sym = num_sym*y + sym;
			const INT sym_y = nofJmers*sym + y;
			ASSERT( 0 <= y_sym && y_sym < nofKmers );
			ASSERT( 0 <= sym_y && sym_y < nofKmers );
			C[k][ y_sym + offsetK ] += L[j][ y + offsetJ ];
			if( p < num_feat-1 ) {
			  C[k][ sym_y + offsetK ] += R[j][ y + offsetJ1 ];
			}
		    }
		}
	    }
	}
        //   if( k > 1 )
        //     j = k-1
        //     for all positions p
        //       for all j-mers y
        //          for n in {A,C,G,T}
        //            C_k[ p, [y,n] ] += L_j[ p, y ]
        //            C_k[ p, [n,y] ] += R_j[ p+1, y ]
        //          end;
        //       end;
        //     end;
        //   end;
    }

    // === return a vector
    num_feat=1;
    num_sym = bigtabSize;
    // --- clean up
    delete[] nofsKmers;
    delete[] margFactors;
    delete[] substrs;
    delete[] x;
    delete[] C;
    for( k = 0; k < max_degree; ++k ) {
      delete L[k];
      delete R[k];
    }
    delete[] L;
    delete[] R;
    return result;
}




/*
void CWeightedDegreePositionCharKernel::count( const DREAL w, const INT p, const INT depth, INT* x, const INT k, DREAL* C_k, DREAL* L_k, DREAL* R_k )
{
    //ASSERT( fabs(w) < 1e10 );
    ASSERT( x[depth] >= 0 );
    ASSERT( x[depth+1] < 0 );
    if ( depth < k ) {
	return;
    }
    const INT num_feat=((CCharFeatures*) get_rhs())->get_num_features();
    const INT num_sym = 4;
    const INT nofKmers = (INT) pow( num_sym, k+1 );
    const DREAL margWeight =  w * pow( 0.25, depth-k );
    INT i;
    INT j;
    for( i = 0; i < depth-k+1; ++i )
    {
	INT y = 0;  // k-mer substring at position i
	for(j = 0; j < k+1; j++) {
	    ASSERT( 0 <= x[i+j] && x[i+j] < num_sym );
	    y = y*num_sym + x[i+j];
	}
	if (p+i<num_feat)
		C_k[ y + nofKmers*(p+i) ] += margWeight;
    }
    if( depth > k )
    {
	INT yL = 0;  // k-suffix
	INT yR = 0;  // k-prefix
	for( j = 0; j < k+1; j++ ) {
	    yL = yL*num_sym + x[depth-k+j];
	    yR = yR*num_sym + x[j];
	}
	ASSERT( 0 <= yL && yL < nofKmers );
	ASSERT( 0 <= yR && yR < nofKmers );
	const INT offsL = yL + nofKmers * (p+depth-k);
	const INT offsR = yR + nofKmers * p;
	if( p+depth-k < num_feat ) {
	  L_k[offsL] += margWeight; 
	}
	R_k[offsR] += margWeight;
    }
    //    # N.x = substring represented by N
    //    # N.d = length of N.x
    //    # N.s = starting position of N.x
    //    # N.w = weight for feature represented by N
    //    if( N.d >= k )
    //      margContrib = w / 4^(N.d-k)
    //      for i = 1 to (N.d-k+1)
    //        y = N.x[i:(i+k-1)]  # overlapped k-mer
    //        C_k[ N.s+i-1, y ] += margContrib
    //      end;
    //      if( N.d > k )
    //        L_k[ N.s+N.d-k, N.x[N.d-k+(1:k)] ] += margContrib  # j-suffix of N.x
    //        R_k[ N.s,       N.x[1:k]         ] += margContrib  # j-prefix of N.x
    //      end;
    //    end;
}


void CWeightedDegreePositionCharKernel::traverse( const struct Trie* tree, const INT p, const INT depth, INT* const x, const INT k, DREAL* C_k, DREAL* L_k, DREAL* R_k )
{
    const INT num_sym = 4;
    INT sym;
    if (depth<degree-1)
    {
	for (sym=0; sym<num_sym; ++sym)
	{
	    if (tree->children[sym]!=NO_CHILD)
	    {
#ifdef USE_TREEMEM
		struct Trie* child=&TreeMem[tree->children[sym]];
#else
		struct Trie* child=tree->children[sym];
#endif
		x[depth] = sym;
		count( child->weight, p, depth, x, k, C_k, L_k, R_k );
		traverse( child, p, depth+1, x, k, C_k, L_k, R_k );
		x[depth] = -1;
	    }
	}
    }
    else if (depth==degree-1)
    {
	for (sym=0; sym<num_sym; ++sym)
	{
	    if( tree->child_weights[sym] != 0 ) {
		x[depth] = sym;
		count( tree->child_weights[sym], p, depth, x, k, C_k, L_k, R_k );
		x[depth] = -1;
	    }
	}
    }
}


DREAL* CWeightedDegreePositionCharKernel::compute_scoring(INT max_degree, INT& num_feat, INT& num_sym, DREAL* result, INT num_suppvec, INT* IDX, DREAL* alphas)
{
    num_feat=((CCharFeatures*) get_rhs())->get_num_features();
    ASSERT(num_feat>0);
    ASSERT(((CCharFeatures*) get_rhs())->get_alphabet()->get_alphabet() == DNA);
    num_sym=4; //for now works only w/ DNA

    DREAL** C = new DREAL*[max_degree];
    DREAL** L = new DREAL*[max_degree];
    DREAL** R = new DREAL*[max_degree];
    INT* x = new INT[degree+1];
    INT i,k;

	INT bigtabSize = 0;
    for( k = 0; k < max_degree; ++k )
    {
		const INT nofKmers = (INT) pow( num_sym, k+1 );
		const INT tabSize = nofKmers * num_feat;
		bigtabSize+=tabSize;
	}
	result= new DREAL[bigtabSize];

    // init table
	INT tabOffs=0;
    for( k = 0; k < max_degree; ++k )
    {
	const INT nofKmers = (INT) pow( num_sym, k+1 );
	const INT tabSize = nofKmers * num_feat;
	C[k] = &result[tabOffs];
	L[k] = new DREAL[ tabSize ];
	R[k] = new DREAL[ tabSize ];
	tabOffs+=tabSize;

	for(i = 0; i < tabSize; i++ )
	{
	    C[k][i] = 0.0;
	    L[k][i] = 0.0;
	    R[k][i] = 0.0;
	}
    }

	i=0; // total progress
    // main loop
    for( k = 0; k < max_degree; ++k )
    {
	const INT nofKmers = (INT) pow( num_sym, k+1 );

	// --- run over all trees
	for(INT p = 0; p < num_feat; ++p )
	{
	    init_optimization( num_suppvec, IDX, alphas, p );
	    const Trie* const tree = get_tree_at_position( p );
	    for(INT j = 0; j < degree+1; j++ ) {
		x[j] = -1;
	    }
	    traverse( tree, p, 0, x, k, C[k], L[k], R[k] );

		CIO::progress(i++,0,num_feat*max_degree);
	}

	// --- add partial overlap scores
	if( k > 0 ) {
	    const INT j = k - 1;
	    const INT nofJmers = (INT) pow( num_sym, j+1 );
	    for(INT p = 0; p < num_feat; ++p ) {
		const INT offsetJ = nofJmers * p;
		const INT offsetJ1 = nofJmers * (p+1);
		const INT offsetK = nofKmers * p;
		INT y;
		INT sym;
		for( y = 0; y < nofJmers; ++y ) {
		    for( sym = 0; sym < num_sym; ++sym ) {
			const INT y_sym = num_sym*y + sym;
			const INT sym_y = nofJmers*sym + y;
			ASSERT( 0 <= y_sym && y_sym < nofKmers );
			ASSERT( 0 <= sym_y && sym_y < nofKmers );
			C[k][ y_sym + offsetK ] += L[j][ y + offsetJ ];
			if( p < num_feat-1 ) {
			  C[k][ sym_y + offsetK ] += R[j][ y + offsetJ1 ];
			}
		    }
		}
	    }
	}
        //   if( k > 1 )
        //     j = k-1
        //     for all positions p
        //       for all j-mers y
        //          for n in {A,C,G,T}
        //            C_k[ p, [y,n] ] += L_j[ p, y ]
        //            C_k[ p, [n,y] ] += R_j[ p+1, y ]
        //          end;
        //       end;
        //     end;
        //   end;
	
    }

	//return a vector
	num_feat=1;
    num_sym = bigtabSize;

    delete[] C;

    for( k = 0; k < max_degree; ++k )
    {
		delete L[k];
		delete R[k];
	}
    delete[] L;
    delete[] R;
    return result;
}
*/




/*
  DREAL* CWeightedDegreePositionCharKernel::compute_scoring(INT max_degree, INT& num_feat, INT& num_sym, DREAL* result, INT num_suppvec, INT* IDX, DREAL* weights)
  {
  num_feat=((CCharFeatures*) get_rhs())->get_num_features();
  ASSERT(num_feat>0);
  ASSERT(((CCharFeatures*) get_rhs())->get_alphabet()->get_alphabet() == DNA);
  num_sym=4; //for now works only w/ DNA
  INT sym_offset=(INT) pow(num_sym,max_degree);

  if (!result)
  {
  INT buflen=(INT) num_feat*sym_offset;
  result= new DREAL[buflen];
  ASSERT(result);
  memset(result, 0, sizeof(DREAL)*buflen);
  }

  for (INT i=0; i<num_feat; i++)
  {
  init_optimization(num_suppvec, IDX, weights, i, CMath::min(num_feat-1,i+1));

  struct Trie* tree = trees[i];
  ASSERT(tree!=NULL);
  compute_scoring_helper(tree, i, 0, 0.0, 0, max_degree, num_feat, num_sym, sym_offset, 0, result);
  CIO::progress(i,0,num_feat);
  }
  num_sym=sym_offset;

  return result;
  }
*/

