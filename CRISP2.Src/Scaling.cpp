/****************************************************************************/
/* Copyright (c) 1994,98 SoftHard Technology, Ltd. **************************/
/****************************************************************************/
/* Image scaling functions * by ML ******************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "scaling.h"

/****************************************************************************/
// Export from sc3_subr.asm
extern "C" {
void	sc3_exp_init	( int i );
LPBYTE	sc3_exp_str_X	( LPBYTE bi, LPBYTE bo, int xi, int xo );
void	sc3_str_ADD		( LPBYTE bi, LPBYTE bo, int len, char f );
void	sc3_str_DIV		( LPBYTE bio, int len, int div );
int		sc3_str_CNT		( int yi, int yo );
int		sc3_exp_Y		( LPBYTE b1, LPBYTE b2, LPBYTE bo, int xo, int yi, int yo );

// Export from sc1_subr.asm
void	sc1_exp_init	( int i );
LPBYTE	sc1_exp_str_X	( LPBYTE bi, LPBYTE bo, int xi, int xo );
void	sc1_str_ADD		( LPBYTE bi, LPBYTE bo, int len, char f );
void	sc1_str_DIV		( LPBYTE bio, int len, int div );
int		sc1_str_CNT		( int yi, int yo );
int		sc1_exp_Y		( LPBYTE b1, LPBYTE b2, LPBYTE bo, int xo, int yi, int yo );
}
/*--------------------------------------------------------------------------*/
static	LPBYTE	pb1, pb2, pb;
static	int		cnt, cnts;
static	int		ff, fy;
static  int 	xi, xo, yi, yo;

/****************************************************************************/
BOOL	sc3Init( int dxout, int dyout, int dxinp, int dyinp )
{
	int size = __max(dxinp,dxout) * 3;	// for RGB

	sc3Free();
	pb1 = (LPBYTE)calloc(size, 1);
	if (pb1 == NULL) return FALSE;
	pb2 = (LPBYTE)calloc(size, 1);
	if (pb2 == NULL) {
		free(pb1); pb1 = NULL;
		return FALSE;
	}
	fy = ff = 0; xi = dxinp; xo = dxout; yi = dyinp; yo = dyout;
	if(yi < yo) fy = 1; if(yi > yo) fy = 2;
	sc3_exp_init(0);                                          
	return TRUE;
}                                
/****************************************************************************/
VOID	sc3Free( VOID )
{
	if (pb1) { free( pb1 ); pb1 = NULL; }
	if (pb2) { free( pb2 ); pb2 = NULL; }
}
/****************************************************************************/
BOOL	sc3Line( LPBYTE eout_buf, LPBYTE bi )
{
    switch( fy )
    {
    case 0 :	// Y not changing, work on X
	  if(ff) { ff = 0; return FALSE; }
	  ff = 1; sc3_exp_str_X(bi, eout_buf, xi, xo); break;
/*----------*/
    case 1 :	// Expand Y
	  if(ff == 0) { sc3_exp_str_X(bi, pb1, xi, xo);      ff = 1; return FALSE; }
	  if(ff == 3) { pb = pb1; pb1 = pb2; pb2 = pb;       ff = 1; return FALSE; }
	  if(ff == 1) { sc3_exp_str_X(bi, pb2, xi, xo);      ff = 2;            }
	  if(sc3_exp_Y(pb1, pb2, eout_buf, xo, yi, yo) == 0) ff = 3;
      break;
/*----------*/
    case 2 :	// Squeeze Y
      if(ff == 0) {
		ff = 1; cnts = cnt = sc3_str_CNT(yi, yo);
		if(--cnt) {
		   sc3_str_ADD( sc3_exp_str_X(bi, pb1, xi, xo), eout_buf, xo, 0 );
		   return FALSE;
		}
		sc3_exp_str_X(bi, eout_buf, xi, xo); break;
      }
	  if(cnt == 0) { ff = 0; return FALSE; }
	  sc3_str_ADD( sc3_exp_str_X(bi, pb1, xi, xo), eout_buf, xo, 1 );
	  if(--cnt) return FALSE;
	  sc3_str_DIV( eout_buf, xo, cnts ); break;
    }
/*-------------------------------*/
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL	sc1Init( int dxout, int dyout, int dxinp, int dyinp )
{
	int size = __max(dxinp,dxout);		// for Gray scale

	sc1Free();
	pb1 = (LPBYTE)calloc(size, 1);
	if (pb1 == NULL) return FALSE;
	pb2 = (LPBYTE)calloc(size, 1);
	if (pb2 == NULL) {
		free(pb1); pb1 = NULL;
		return FALSE;
	}
	fy = ff = 0; xi = dxinp; xo = dxout; yi = dyinp; yo = dyout;
	if(yi < yo) fy = 1; if(yi > yo) fy = 2;
	sc1_exp_init(0);                                          
	return TRUE;
}                                
/****************************************************************************/
VOID	sc1Free( VOID )
{
	if (pb1) { free( pb1 ); pb1 = NULL; }
	if (pb2) { free( pb2 ); pb2 = NULL; }
}
/****************************************************************************/
BOOL	sc1Line( LPBYTE eout_buf, LPBYTE bi )
{
    switch( fy )
	{
    case 0 :	// Y not changing, work on X
		if(ff) { ff = 0; return FALSE; }
		ff = 1; sc1_exp_str_X(bi, eout_buf, xi, xo); break;
/*----------*/
    case 1 :	// Expand Y
		if(ff == 0) { sc1_exp_str_X(bi, pb1, xi, xo);      ff = 1; return FALSE; }
		if(ff == 3) { pb = pb1; pb1 = pb2; pb2 = pb;       ff = 1; return FALSE; }
		if(ff == 1) { sc1_exp_str_X(bi, pb2, xi, xo);      ff = 2;            }
		if(sc1_exp_Y(pb1, pb2, eout_buf, xo, yi, yo) == 0) ff = 3;
		break;
/*----------*/
    case 2 :	// Squeeze Y
		if(ff == 0)
		{
			ff = 1; cnts = cnt = sc1_str_CNT(yi, yo);
			if(--cnt)
			{
				sc1_str_ADD( sc1_exp_str_X(bi, pb1, xi, xo), eout_buf, xo, 0 );
				return FALSE;
			}
			sc1_exp_str_X(bi, eout_buf, xi, xo); break;
		}
		if(cnt == 0) { ff = 0; return FALSE; }
		sc1_str_ADD( sc1_exp_str_X(bi, pb1, xi, xo), eout_buf, xo, 1 );
		if(--cnt) return FALSE;
		sc1_str_DIV( eout_buf, xo, cnts ); break;
	}
/*-------------------------------*/
	return TRUE;
}
