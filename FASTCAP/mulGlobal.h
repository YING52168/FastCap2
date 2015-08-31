/*
Copyright (C) 2003 by the Board of Trustees of Massachusetts Institute of
Technology, hereafter designated as the Copyright Owners.
 
License to use, copy, modify, sell and/or distribute this software and
its documentation for any purpose is hereby granted without royalty,
subject to the following terms and conditions:
 
1.  The above copyright notice and this permission notice must
appear in all copies of the software and related documentation.
 
2.  The names of the Copyright Owners may not be used in advertising or
publicity pertaining to distribution of the software without the specific,
prior written permission of the Copyright Owners.
 
3.  THE SOFTWARE IS PROVIDED "AS-IS" AND THE COPYRIGHT OWNERS MAKE NO
REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, BY WAY OF EXAMPLE, BUT NOT
LIMITATION.  THE COPYRIGHT OWNERS MAKE NO REPRESENTATIONS OR WARRANTIES OF
MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE
SOFTWARE WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS TRADEMARKS OR OTHER
RIGHTS. THE COPYRIGHT OWNERS SHALL NOT BE LIABLE FOR ANY LIABILITY OR DAMAGES
WITH RESPECT TO ANY CLAIM BY LICENSEE OR ANY THIRD PARTY ON ACCOUNT OF, OR
ARISING FROM THE LICENSE, OR ANY SUBLICENSE OR USE OF THE SOFTWARE OR ANY
SERVICE OR SUPPORT.
 
LICENSEE shall indemnify, hold harmless and defend the Copyright Owners and
their trustees, officers, employees, students and agents against any and all
claims arising out of the exercise of any rights under this Agreement,
including, without limiting the generality of the foregoing, against any
damages, losses or liabilities whatsoever with respect to death or injury to
person or damage to property arising from or out of the possession, use, or
operation of Software or Licensed Program(s) by LICENSEE or its customers.

*/


/* for NWS-3860 compatability */
#ifdef NEWS
/*
** Memory management functions (from stdlib.h (recursive incl => unusable))
*/
extern char *   calloc();
extern char *   malloc();
extern char *   realloc();
#else
#include <malloc.h>
#endif /* end if NEWS */
#include <stdio.h>
#include <math.h>

/* fastcap data structures */
#include "mulStruct.h"

/* execution time macros */
#include "resusage.h"

/* time variables/structs */
#ifndef _TIME_                  /* if not on a Sun4 */
#ifndef NEWS                    /* if not on a NWS-38XX */
#include <time.h>
#endif
#endif

// function prototype, needed for 64 bit version (otherwise ualloc() is considered a function
// returning 'int', not 'char *'; this causes the addresses to be cut to 32 bits
char *ualloc(unsigned int);

#define VERSION 2.0

/*********************************************************************** 
  macros for allocation with checks for NULL pntrs and 0 byte requests
  - also keep an allocated memory count
  - CALLOC() is used when the memory must be zeroed
    its core should be either calloc() or ualloc() (not as fast but
    more space efficient, no free list - uses sbrk() and never frees)
  - MALLOC() used when memory can be anything
    core should be malloc() or ualloc()
***********************************************************************/
//#define CALCORE(NUM, TYPE) calloc((unsigned)(NUM),sizeof(TYPE)) /* Enrico */
#define CALCORE(NUM, TYPE) ualloc((unsigned)(NUM)*sizeof(TYPE))  /* Enrico */
//#define MALCORE malloc /* Enrico */
#define MALCORE ualloc  /* Enrico */

/* counts of memory usage by multipole matrix type */
extern long memcount;
extern long memQ2M;
extern long memQ2L;
extern long memQ2P;
extern long memL2L;
extern long memM2M;
extern long memM2L;
extern long memM2P;
extern long memL2P;
extern long memQ2PD;
extern long memMSC;

/* types of memory usage by multipole matrix type */
#define AQ2M 0
#define AQ2L 1
#define AQ2P 2
#define AL2L 3
#define AM2M 4
#define AM2L 5
#define AM2P 6
#define AL2P 7
#define AQ2PD 8
#define AMSC 9

#define DUMPALLOCSIZ                                                   \
{                                                                      \
  (void)viewprintf(stderr,                                                \
		"Total Memory Allocated: %d kilobytes (brk = 0x%x)\n", \
		memcount/1024, sbrk(0));                               \
}

#define CALLOC(PNTR, NUM, TYPE, FLAG, MTYP)                                 \
{                                                                           \
     if((NUM)*sizeof(TYPE)==0)                                              \
       (void)viewprintf(stderr,                                                \
		     "zero element request in file `%s' at line %d\n",      \
		     __FILE__, __LINE__);	                            \
     else if(((PNTR)=(TYPE*)CALCORE(NUM, TYPE))==NULL) {                    \
       (void)viewprintf(stderr,                                                \
	 "\nfastcap: out of memory in file `%s' at line %d\n",              \
	       __FILE__, __LINE__);                                         \
       (void)viewprintf(stderr, " (NULL pointer on %d byte request)\n",        \
		     (NUM)*sizeof(TYPE));                                   \
       DUMPALLOCSIZ;                                                        \
       DUMPRSS;                                                             \
       (void)fflush(stderr);                                                \
       (void)fflush(stdout);                                                \
       if(FLAG == ON) FCExit(FC_OUT_OF_MEMORY);                                              \
     }                                                                      \
     else {                                                                 \
       memcount += ((NUM)*sizeof(TYPE));                                    \
       if(MTYP == AQ2M) memQ2M += ((NUM)*sizeof(TYPE));                     \
       else if(MTYP == AQ2L) memQ2L += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AQ2P) memQ2P += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AL2L) memL2L += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AM2M) memM2M += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AM2L) memM2L += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AM2P) memM2P += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AL2P) memL2P += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AQ2PD) memQ2PD += ((NUM)*sizeof(TYPE));              \
       else if(MTYP == AMSC) memMSC += ((NUM)*sizeof(TYPE));                \
       else {                                                               \
         (void)viewprintf(stderr, "CALLOC: unknown memory type %d\n", MTYP);   \
         FCExit(FC_GENERIC_ERROR);                                                           \
       }                                                                    \
     }                                                                      \
}

#define MALLOC(PNTR, NUM, TYPE, FLAG, MTYP)                                  \
{                                                                            \
     if((NUM)*sizeof(TYPE)==0)			                             \
       (void)viewprintf(stderr,                                                \
		     "zero element request in file `%s' at line %d\n",      \
		     __FILE__, __LINE__);	                            \
     else if(((PNTR)=(TYPE*)MALCORE((unsigned)((NUM)*sizeof(TYPE))))==NULL) { \
       (void)viewprintf(stderr,                                                 \
	 "\nfastcap: out of memory in file `%s' at line %d\n",               \
	       __FILE__, __LINE__);                                          \
       (void)viewprintf(stderr, " (NULL pointer on %d byte request)\n",         \
		     (NUM)*sizeof(TYPE));                                    \
       DUMPALLOCSIZ;                                                         \
       DUMPRSS;                                                              \
       (void)fflush(stderr);                                                 \
       (void)fflush(stdout);                                                 \
       if(FLAG == ON) FCExit(FC_OUT_OF_MEMORY);                                               \
     }                                                                       \
     else {                                                                 \
       memcount += ((NUM)*sizeof(TYPE));                                    \
       if(MTYP == AQ2M) memQ2M += ((NUM)*sizeof(TYPE));                     \
       else if(MTYP == AQ2L) memQ2L += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AQ2P) memQ2P += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AL2L) memL2L += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AM2M) memM2M += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AM2L) memM2L += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AM2P) memM2P += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AL2P) memL2P += ((NUM)*sizeof(TYPE));                \
       else if(MTYP == AQ2PD) memQ2PD += ((NUM)*sizeof(TYPE));              \
       else if(MTYP == AMSC) memMSC += ((NUM)*sizeof(TYPE));                \
       else {                                                               \
         (void)viewprintf(stderr, "MALLOC: unknown memory type %d\n", MTYP);   \
         FCExit(FC_GENERIC_ERROR);                                                           \
       }                                                                    \
     }                                                                      \
}

/*****************************************************************************

misc. global macros

*****************************************************************************/
#define NOT !
#define  ABORT()						      \
{   (void)fflush(stdout);					      \
    (void)viewprintf(stderr, "FastCap: panic in file `%s' at line %d.\n",\
	    __FILE__, __LINE__);				      \
    (void)fflush(stderr);					      \
    abort();							      \
}

#define ASSERT(condition) if(NOT(condition)) ABORT()

#define INNER(pap,p,ap,size) for(pap=0.0,i=1; i<=size; i++) pap += p[i]*ap[i];

#ifndef MAX
#define MAX(A,B)  ( (A) > (B) ? (A) : (B) )
#endif

#ifndef MIN
#define MIN(A,B)  ( (A) > (B) ? (B) : (A) )
#endif

#define ABS(A) ( ( (A) > 0 ) ? (A) : (-(A)) )

#define VCOPY(A, B) A[0] = B[0]; A[1] = B[1]; A[2] = B[2];

#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0

#define LAST 2
#define ALL 2

#ifndef M_PI
/* pi constant included here since won't be in ANSI C */
#define M_PI       3.1415926535897931160E0  /*Hex  2^ 1 * 1.921FB54442D18 */
#endif

#define E_0 8.854187818E-12	/* epsilon0 +- .000000071E-12 F/m */
#define FPIEPS 4.0*M_PI*E_0	/* 4 pi times the dielectric permittivity,
				   free-space permittivity is the default,
				   units are F/m - all dimensions in meters */

/* flags in chkList() in mulDisplay.c (chks direct, local or eval cube lsts) */
#define DIRECT 0
#define LOCAL 1
#define EVAL 3

/* types of surfaces */
#define CONDTR 0		/* conductor surfaces */
#define DIELEC 1		/* dielectric interface surface */
#define BOTH 3			/* dielectric i/f w/very thin cond on it */

/* used in input routines */
#define MAXCON 10000		/* assumes never more conductors than this */

/* used in ps file dump */
#define OPEN 0			/* open ps file, print hdr, ignore row/col */
#define CLOSE 1			/* print trailer, close ps file */
#define UPDATE 2		/* => add 2 dots for this row and col */

/* divided difference distances, see electric.c */
#define HPOS (1e-6*cur_panel->max_diag) /* h in positive normal dir */
#define HNEG HPOS		/* h for divided difference in neg nrml dir */

/* level set mode, see placeq, mulSetup.c and input.c */
#define ONELES 2		/* => auto set levs to 1 up fr fully exact */

/* expansion moment index generating macros (see mulMulti.c, mulLocal.c) */
#define CINDEX(N, M) ( (M) + ((N)*((N)+1))/2 )
#define SINDEX(N, M, CTERMS) ( (CTERMS) + (M) + ((N)*((N)+1))/2 - ((N)+1) )

/* used in get_kill_num_list and routines it calls */
#define NOTUNI -1
#define NOTFND -2

/***********************************************************************
 
  configuration and debug flags

***********************************************************************/

/* types of downward/eval passes */
#define NOLOCL 0	       	/* multipoles evaluated directly - no locals */
#define NOSHFT 1		/* multis to locals w/o local2local shifts */
#define GRENGD 3		/* full Greengard downward pass/eval */

/* types of iterative methods (values of ITRTYP below) */
#define GCR 0			/* GCR with single (not block) vector iters */
#define GMRES 1                 /* GMRES with vector iterates */

/* types of finite elements (NOTE: only const. chg den. panels implemented) */
#define CONST 0			/* constant charge density on panels */
#define AFFINE 1
#define QUADRA 2

/* types of weighted residuals methods (NOTE: only collocation implemented) */
#define COLLOC 0		/* point collocation */
#define SUBDOM 1		/* subdomain collocation */
#define GALKIN 2		/* Galerkin */

/* types of preconditioners. */
#define NONE 0
#define BD 1                    /* Block diagonal (not set up for dielecs). */
#define OL 2                    /* OverLap */

/* Discretization Configuration */
#define WRMETH COLLOC		/* weighted res meth type (COLLOC only now) */
#define ELTYPE CONST		/* finite element type (CONST only now) */
/* Multipole Configuration */
#define DNTYPE GRENGD		/* type of downward/eval pass - see above */
#define MULTI ON		/* ON=> add in multipole contribution to P*q */
#define RADINTER ON	        /* ON=> Parent level multis in interlist. */
#define NNBRS 2			/* Distance to consider a nearest nbr. */
#define ADAPT ON		/* ON=> use adaptive algorithm */
#define OPCNT OFF		/* Counts the Matrix-Vector multiply ops. */
#define DEFORD 2		/* default expansion order */
#define MAXORDER 6		/* Maximum expansion order (sets ary sizes) */
#define MAXDEP 20		/* maximum partitioning depth */
#define NUMDPT 2		/* num pnts for ea dielec panel (2 or 3) */
#define SKIPQD OFF		/* ON => skip dielec panel chg in E eval */
/* Linear System Solution Configuration */
#define ITRTYP GMRES		/* type of iterative method */
#define PRECOND OL		/* NONE=> no preconditioner OL=> use prec. */
#define DIRSOL OFF		/* ON=> solve Pq=psi by Gaussian elim. */
#define EXPGCR OFF		/* ON=> do explicit full P*q products */
#define ABSTOL 0.01		/* iterations until ||res||inf < ABSTOL */
#define MAXITER size		/* max num iterations ('size' => # panels) */
#define EXRTSH 0.9		/* exact/ttl>EXRTSH for lev => make last lev */
/* (add any new configuration flags to dumpConfig() in mulDisplay.c) */

/* Output Format Configuration */
#define MKSDAT ON		/* ON=> dump symmetrized, MKS units cap mat */
#define CMDDAT ON		/* ON=> dump command line info to output */
#define RAWDAT OFF		/* ON=> dump unsymm, Gaussian units cap mat */
#define ITRDAT OFF		/* ON=> dump residuals for every iteration */
#define TIMDAT ON		/* ON=> dump time and memory usage numbers */
#define CFGDAT OFF		/* ON=> dump configuration flags to output */
#define MULDAT OFF		/* ON=> dump brief multipole setup info */
#define DISSYN OFF		/* ON=> display synopsis of cubes in lists */
#define DMTCNT OFF		/* ON=> display xform matrix counts by level */
#define DISSRF ON		/* ON=> display input surface information */
#define NAMDAT OFF		/* ON=> dump conductor names */
#define CAPVEW ON		/* ON=> enable ps file dumps of geometry */

/* display of transformation matrices */
#define DISQ2M OFF		/* ON=> display Q2M matrices when built */
#define DISM2M OFF		/* ON=> display M2M matrices when built */
#define DISM2P OFF		/* ON=> display M2P matrices when built */
#define DISL2P OFF		/* ON=> display L2P matrices when built */
#define DISQ2P OFF		/* ON=> display Q2P matrices when built */
#define DSQ2PD OFF		/* ON=> display Q2PDiag matrices > build */
#define DISQ2L OFF		/* ON=> display Q2L matrices when built */
#define DISM2L OFF		/* ON=> display M2L matrices when built */
#define DISL2L OFF		/* ON=> display L2L matrices when built */
#define DALQ2M OFF		/* ON=> display all Q2M matrix build steps */
#define DALM2P OFF		/* ON=> display all M2P matrix build steps */
#define DALL2P OFF		/* ON=> display all L2P matrix build steps */
#define DALQ2L OFF		/* ON=> display all Q2L matrix build steps */
/* display of other intermediate results */
#define DUPVEC OFF		/* ON=> display lev 1 upward pass vectors */
#define DISFAC OFF		/* ON=> display factorial fractions in M2L */
#define DPSYSD OFF		/* ON=> display system after direct build */
#define DILIST OFF		/* ON=> display interaction lists */
#define DMPELE OFF		/* ON=> display electric flux densities */
#define DMPCHG OFF		/* ON=> display all charge vector iterates
				   LAST=> display final charge vector */
/* misc debug */
#define CKDLST OFF		/* ON=> check direct list, prnt msg if bad */
#define DMPREC OFF		/* ON=> dump P and Ctil to matlab file */
#define CKCLST OFF		/* ON=> check charge list, prnt msg if bad */
#define DUMPPS OFF		/* ON=> dump ps file w/mulMatDirect calcp's
				   ALL=> dump adaptive alg calcp's as well */
#define DPCOMP OFF		/* ON=> dump prec pts before&aft compression */
#define DPDDIF OFF		/* ON=> dump divided difference components */
#define CHKDUM OFF		/* ON=> print msg if dummy list inconsistent */
#define JACDBG OFF		/* ON=> print random Jacob debug messages */
/* blkDirect.c related flags - used only when DIRSOL == ON || EXPGCR == ON */
#define MAXSIZ 0		/* any more tiles than this uses matrix on disk
				   for DIRSOL == ON or EXPGCR == ON */


// Enrico, file input routine prototypes
#define BOOL	int
#define TRUE	1
#define FALSE	0
extern BOOL isblankline(char *string);
