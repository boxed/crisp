#ifndef __SYMMETRY_H__
#define __SYMMETRY_H__

typedef int MX2[2*3];

typedef struct
{
	LPSTR	name;		// Name like p1, p2,  pm, pg
	LPSTR	xname;		// Name like p1, p21, p12
	double	gamma;		// Gamma value: 90, 120, or 0
	BOOL	aeqb;		// a equals to b
	BOOL	Centric;	// Centric
	int		N;			// Number of symmetry matricies
	MX2		mx[12];		// Sym. matricies

} SYMMETRY, *LPSYMMETRY;

extern SYMMETRY Symmetry[NUMSYMM+1];

#endif // __SYMMETRY_H__