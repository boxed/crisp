/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.PhIdo * by SK, ML *************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"

#include "eld.h"

#include "HelperFn.h"

#pragma optimize ("",off)

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define TINYVAL 0.0000000001

/****************************************************************************/
typedef struct {
	double x,y,z,l;
} C4;

typedef struct {
	int h, k, l;
} C3i;

// 'L-NiM         1  2   3.520   3.520   3.630  90.000  90.000  90.000'
#define	PNSIZE 14
typedef struct {
	C4    A, B, C;
	double   a, b, c;
	int	sys, lat;
	char name[16];
} Lattice3;

typedef struct {
	double	u,  v,  w;
	double	eu, ev, ew;
	C3i pu, pv;
} Solution;


#define	DEG   0.0174532925199432955
#define RAD  57.2957795130823229000


#define	SMAX	256		// Maximum number of solutions
#define	VMAX	1024	// Maximum number of vectors
// removed by Peter oon 30 June 2003
//#define	LMAX	256		// Maximum number of phases


typedef struct {
	Solution *	Cs;	// Solutions for the first lattice
	Solution *	Ds;	// Solutions for the second lattice
	Solution	C;	// First lattice definition
	Solution	D;	// Second lattice definition
	Lattice3 *	L;	// Known phase descriptions (direct space)
	Lattice3 *	Lr;	// Known phase descriptions (reciprocal space)
	int			lcnt;// Number of known pases
	double		w;	// Angle between first end second lattice
	double		ew;	// Error for above angle
	C4 *		P;	// Vectors array (VMAX)
	int *		pu;	// Indexes of vectors which are U compliant
	int *		pv;	// Indexes of vectors which are V compliant
} PHIDO;
typedef PHIDO far * LPPHIDO;

#define	AddProtocolString(pp)	SendDlgItemMessage(hDlg, IDC_PHI_LIST, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)pp)

static	LPSTR lpSys[7] = {"","Cubic", "Tetragonal", "Hexagonal", "Orthorhombic", "Monoclinic", "Triclinic"};
static	LPSTR lpLat[6] = {"","Simple (P)", "Body-centered (I)", "Face-centered (F)", "Base-centered (C)", "Rhombohedral (R)" };
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// TODO: check this, it is same as defined in ELD.cpp
static double	AB_Angle(double ax, double ay, double bx, double by)
{
	return fabs(P2R(R2P( ATAN2(by, bx) - ATAN2(ay, ax) )));
}
/*--------------------------------------------------------------------------*/
double	AbsC(double re, double im)
{
	return sqrt(re*re+im*im);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#define frcomp(a,b,err) ( fabs(a-b) < a*err )
#define facomp(a,b,err) ( fabs(a-b) <   err )
/*--------------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
C4 NormAB(C4 * A, C4 * B)
{
C4 R;
	R.x = B->y*A->z - A->y*B->z;
	R.y = A->x*B->z - B->x*A->z;
	R.z = B->x*A->y - A->x*B->y;
	R.l = sqrt(R.x*R.x+R.y*R.y+R.z*R.z);
	R.x /= R.l;	R.y /= R.l;	R.z /= R.l; R.l = 1.;
	return R;
}
/*--------------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
BOOL __inline IsFriedel( C3i * a ) {
	if(a->h<0) return TRUE;
	if(a->h==0 && a->k<0) return TRUE;
	if(a->h==0 && a->k==0 && a->l<0) return TRUE;
	return FALSE;
}
/*----------------------------------------------------------------------*/
C3i NormHKL(C3i * A, C3i * B)
{
C3i R;
	R.h = B->k*A->l - A->k*B->l;
	R.k = A->h*B->l - B->h*A->l;
	R.l = B->h*A->k - A->h*B->k;
	return R;
}
/*----------------------------------------------------------------------*/
double	SMul(C4* U, C4* V)
{
	return( U->x*V->x + U->y*V->y + U->z*V->z);
}
/*--------------------------------------------------------------------------*/
double	AB_CAngle(C4* a, C4* b)
{
double x;
	x = SMul(a,b) / a->l / b->l;
	if(x >  1.) x = 1.;
	if(x < -1.) x =-1.;
	return RAD * acos(x);
}
/*--------------------------------------------------------------------------*/
double	cosAB_CAngle(C4* a, C4* b)
{
	return SMul(a,b) / a->l / b->l;
}
/*--------------------------------------------------------------------------*/
double	Calc_Point(C3i * p, Lattice3 * L, C4* P)
{
double h = p->h, k = p->k, l = p->l;
	P->x = h*L->A.x + k*L->B.x + l*L->C.x;
	P->y =			  k*L->B.y + l*L->C.y;
	P->z =						 l*L->C.z;
	P->l = sqrt(P->x*P->x + P->y*P->y + P->z*P->z);
	return P->l;
}
/*--------------------------------------------------------------------------*/
void	calc_hkl(Lattice3 * L, C4 * P, C3i* hkl)
{
double h, k, l;
	l = P->z / L->C.z;
	k = (P->y - l*L->C.y) / L->B.y;
	h = (P->x - k*L->B.x - l*L->C.x) / L->A.x;
	hkl->h = round(h);
	hkl->k = round(k);
	hkl->l = round(l);
}
/*--------------------------------------------------------------------------*/
BOOL	check_Pt2(C4 * A, C4 * B, C4 * P)
{
double h, k;
	k = P->y / B->y;
	if( fabs(k - round(k)) > TINYVAL ) return FALSE;
	h = (P->x - k*B->x) / A->x;
	if( fabs(h - round(h)) > TINYVAL ) return FALSE;
	return TRUE;
}
/*--------------------------------------------------------------------------*/
BOOL scanf_Lat(char * str, Lattice3 * l)
{
/* L-NiM         1  2   3.520   3.520   3.630  90.000  90.000  90.000 */
int n;
	memset(l->name,0,sizeof(l->name));
	strncpy(l->name,str,PNSIZE);
	if((n=sizeof(double))==4)
		n = sscanf(str+PNSIZE,"%d%d%f%f%f%f%f%f", &l->lat, &l->sys,
						 &l->A.l,&l->B.l,&l->C.l, &l->a,&l->b,&l->c);
	else
		n = sscanf(str+PNSIZE,"%d%d%lf%lf%lf%lf%lf%lf", &l->lat, &l->sys,
						 &l->A.l,&l->B.l,&l->C.l, &l->a,&l->b,&l->c);
	l->a *= DEG; l->b *= DEG; l->c *= DEG;
	return (n==8);
}
/*--------------------------------------------------------------------------*/
void	Calc_R_Lattice( Lattice3 * I, Lattice3 * R)
{
double ca = cos(I->a), cb = cos(I->b), cc = cos(I->c);
double sa = sin(I->a), sb = sin(I->b), sc = sin(I->c);
double v,wa,wb,wc;
	v = I->A.l * I->B.l * I->C.l * sqrt(1. + 2.*ca*cb*cc - ca*ca - cb*cb - cc*cc);
	memcpy(R,I,sizeof(Lattice3));
	R->A.l = I->B.l * I->C.l * sa / v;
	R->B.l = I->C.l * I->A.l * sb / v;
	R->C.l = I->A.l * I->B.l * sc / v;
	wa = (cb*cc-ca)/sb/sc; R->a = acos(wa);
	wb = (cc*ca-cb)/sc/sa; R->b = acos(wb);
	wc = (ca*cb-cc)/sa/sb; R->c = acos(wc);
	R->A.x = R->A.l; R->A.y = R->A.z = R->B.z = 0.;
	R->B.x = R->B.l*wc; R->B.y = R->B.l*sqrt(1.-wc*wc);
	R->C.x = R->C.l*wb;
	R->C.y = (wa*R->B.l*R->C.l - R->B.x*R->C.x) / R->B.y;
	R->C.z = sqrt(R->C.l*R->C.l - R->C.x*R->C.x - R->C.y*R->C.y);
}
/*--------------------------------------------------------------------------*/
BOOL	check_lat(Lattice3* L, C3i * p)
{
int ret = 0;

		switch(L->lat) {
		case 2: ret =  (p->h + p->k + p->l)&1; break;
		case 3: ret = ((p->l + p->h) | (p->k + p->l)) & 1;
		case 4: ret |= (p->k + p->h) & 1;  break;
		case 5: ret =  (p->k + p->l - p->h)%3;
	}
	return (ret==0);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int	Check_Solution(Lattice3 * L, Solution * S)
{
	int hm,km,lm,i,j;
	double s,cw,sw;
	C3i p;
	C4	P2,P,N,A1,A2,LV1,LV2;

	Calc_Point(&S->pu, L, &A1); LV1 = A1;
	Calc_Point(&S->pv, L, &A2); LV2 = A2;
	N = NormAB(&A1,&A2);

	LV1.x = LV1.l = S->u; LV2.l = S->v; LV1.y = LV1.z = LV2.z = 0.;
	LV2.x = S->v * cos(DEG*S->w);	LV2.y = S->v * sin(DEG*S->w);

	s = __max( S->u + S->eu, S->v + S->ev );
	hm = (int)(s / L->A.l +0.999);
	km = (int)(s / L->B.l +0.999);
	lm = (int)(s / L->C.l +0.999);
	for(p.h = 0; p.h<=hm; p.h++)
	 for(j=-lm; j<=lm; j++)
	  for(i=-km; i<=km; i++) {
		if(i<=0) p.k = i + km; else p.k = -i;
		if(j<=0) p.l = j + lm; else p.l = -j;
//		if(IsFriedel(&p)) continue;
		if((p.h | p.k | p.l)==0) continue;
		if(!check_lat(L, &p)) continue;
		Calc_Point(&p, L, &P);
		if( fabs(SMul(&N,&P)) < TINYVAL ) {
			cw = cosAB_CAngle(&P, &A1);
			sw = cosAB_CAngle(&P, &A2);
			sw = (sw - cos(DEG*S->w)*cw)/sin(DEG*S->w);
			P2.x = P.l * cw;
			P2.y = P.l * sw;
			if(!check_Pt2(&LV1,&LV2,&P2))
			 return FALSE;
		}
	}
	return TRUE;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int	Find_Solution(LPPHIDO S, Lattice3 * L, Solution * I, Solution * O)
{
	static int hm,km,lm,n,f,t,u,v,i,j;
	double s;
	Solution X;
	C3i p;
	C4 * P = S->P;
	int * pu = S->pu;
	int * pv = S->pv;
	int	  mhkl=16;
	s = __max( I->u + I->eu, I->v + I->ev );
	hm = (int)(s / L->A.l +0.999);
	km = (int)(s / L->B.l +0.999);
	lm = (int)(s / L->C.l +0.999);
	if (hm>mhkl) hm=mhkl; if (km>mhkl) km=mhkl; if (lm>mhkl) lm=mhkl;
	s = (hm+1.)*(2.*km+1.)*(2.*lm+1.);
	t = u = v = 0;
	for(p.h = 0; p.h<=hm; p.h++)
	 for(p.k=-km; p.k<=km; p.k++)
	  for(p.l=-lm; p.l<=lm; p.l++){
	  	if (p.h==0 && p.k==0 && p.l==0) continue;
//	  	if (IsFriedel(&p)) continue;
		if(!check_lat(L, &p)) continue;
		s = Calc_Point(&p, L, P+t); f = 0;
		if(facomp(I->u, s, I->eu)) { pu[u++] = t; f=1;}
		if(facomp(I->v, s, I->ev)) { pv[v++] = t; f=1;}
		if((t += f) >= VMAX) {
//			printf("!!! VMAX\n");
			goto xx;
		}
	}
xx:
	for(n=i=0; i<u; i++) for(j=0; j<v; j++) {
		X.w = AB_CAngle(P+pu[i], P+pv[j]);
		if(facomp(I->w, X.w, I->ew)) {
			X.u = P[pu[i]].l; X.v = P[pv[j]].l;
			X.eu = fabs(X.u - I->u);
			X.ev = fabs(X.v - I->v);
			X.ew = fabs(X.w - I->w);
			calc_hkl(L, P+pu[i], &X.pu);
			calc_hkl(L, P+pv[j], &X.pv);
			if(Check_Solution(L,&X)) O[n++] = X;
			if(n >= SMAX || GetAsyncKeyState(VK_ESCAPE)) {
//				printf("!!! SMAX\n");
				goto x1;
			}
		}
	}
x1:
	return n;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void printV( int i, Solution * S)
{
//C3i N = NormHKL(&S->pu,&S->pv);
//gotoxy(1,2);
//		cprintf("%3d %8.4lf %8.5lf %8.5lf %5.3lf %5.3lf %5.3lf%3d%3d%3d %3d%3d%3d :%3d%3d%3d\n",
//			i,S->w,S->u,S->v,S->ew,S->eu,S->ev,
//			S->pu.h,S->pu.k,S->pu.l,S->pv.h,S->pv.k,S->pv.l,N.h,N.k,N.l);
}
/*--------------------------------------------------------------------------*/
void printS(FILE * ouf, int i, Solution * S)
{
//C3i N = NormHKL(&S->pu,&S->pv);
//		fprintf(ouf,"%3d %8.4lf %8.5lf %8.5lf %5.3lf %5.3lf %5.3lf%3d%3d%3d %3d%3d%3d :%3d%3d%3d\n",
//			i,S->w,S->u,S->v,S->ew,S->eu,S->ev,
//			S->pu.h,S->pu.k,S->pu.l,S->pv.h,S->pv.k,S->pv.l,N.h,N.k,N.l);
}
/*--------------------------------------------------------------------------*/
int scmp(Solution * S1, Solution * S2)
{
	if(S1->w > S2->w) return 1;
	if(S1->w < S2->w) return -1;
	return 0;
}
/*--------------------------------------------------------------------------*/
static void ScanFields( HWND hDlg, Solution * s1, Solution * s2, BOOL *bTwo, double * w, double * ew )
{
	GetDlgItemDouble( hDlg, IDC_PHI_U1, &s1->u, 6 );
	GetDlgItemDouble( hDlg, IDC_PHI_V1, &s1->v, 6 );
	GetDlgItemDouble( hDlg, IDC_PHI_W1, &s1->w, 6 );
	GetDlgItemDouble( hDlg, IDC_PHI_EU1, &s1->eu, 3 );
	GetDlgItemDouble( hDlg, IDC_PHI_EV1, &s1->ev, 3 );
	GetDlgItemDouble( hDlg, IDC_PHI_EW1, &s1->ew, 3 );

	if ((*bTwo = IsDlgButtonChecked( hDlg, IDC_PHI_LAT2 )) != FALSE ) {
		GetDlgItemDouble( hDlg, IDC_PHI_U2, &s2->u, 6 );
		GetDlgItemDouble( hDlg, IDC_PHI_V2, &s2->v, 6 );
		GetDlgItemDouble( hDlg, IDC_PHI_W2, &s2->w, 6 );
		GetDlgItemDouble( hDlg, IDC_PHI_EU2, &s2->eu, 3 );
		GetDlgItemDouble( hDlg, IDC_PHI_EV2, &s2->ev, 3 );
		GetDlgItemDouble( hDlg, IDC_PHI_EW2, &s2->ew, 3 );
		GetDlgItemDouble( hDlg, IDC_PHI_12W, w, 6 );
		GetDlgItemDouble( hDlg, IDC_PHI_12EW, ew, 3 );
	}
}
/*--------------------------------------------------------------------------*/
static void doit( HWND hDlg, LPPHIDO S )
{
int l,n,m,i,j,f, tm=0, ts=0;
C4 U,V,N,M;
Lattice3 Lr;
BOOL	fTwo = FALSE;
C3i ZA, ZB;
OBJ*  O = OBJ::GetOBJ(hDlg);
Solution C,D;
HCURSOR		hOldCur;

	hOldCur = SetCursor(hcWait);

	AddProtocolString(";");
	sprintf(TMP,"; File : %s, Phase Identification", O->fname);
	AddProtocolString(TMP);
	AddProtocolString(";");

	ScanFields( hDlg, &S->C, &S->D, &fTwo, &S->w, &S->ew );	// Scan parameters of the lattice 1 and 2, and angle between them
	C = S->C;
	C.eu *= C.u/100.;
	C.ev *= C.v/100.;
	if (fTwo) {
		D = S->D;
		D.eu *= D.u/100.;
		D.ev *= D.v/100.;
		AddProtocolString(";  U1        V1     [Zone ax.1]    U2        V2     [Zone ax.2]");
	} else {
		AddProtocolString(";  U1        V1     [Zone axes] U^Verr Uerr  Verr");
	}
	GetAsyncKeyState(VK_ESCAPE);
	for(l=0;l<S->lcnt;l++) {
		Calc_R_Lattice(&S->L[l], &Lr);						// Calculate reciprocal lattice parameters for given phase
		n = Find_Solution(S, &Lr, &C, S->Cs);
		if (fTwo) {
			m = Find_Solution(S, &Lr, &D, S->Ds);
			if(n&&m) {
				for(f=i=0; i<n; i++) for(j=0; j<m; j++) {
					Calc_Point(&S->Cs[i].pu, &Lr, &U);	Calc_Point(&S->Cs[i].pv, &Lr, &V);
					N = NormAB(&U,&V);
					Calc_Point(&S->Ds[j].pu, &Lr, &U);	Calc_Point(&S->Ds[j].pv, &Lr, &V);
					M = NormAB(&U,&V);
					if(fabs(AB_CAngle(&N,&M) - S->w)<S->ew) {
						if (f==0) {
							AddProtocolString(";------------------------------------------------");
							sprintf(TMP,"%s : %s , %s", S->L[l].name, lpSys[S->L[l].sys], lpLat[S->L[l].lat] );
							AddProtocolString(TMP);
						}
						ZA = NormHKL(&S->Cs[i].pu,&S->Cs[i].pv);
						ZB = NormHKL(&S->Ds[j].pu,&S->Ds[j].pv);
						sprintf(TMP,"%3d%3d%3d %3d%3d%3d [%3d%3d%3d] %3d%3d%3d %3d%3d%3d [%3d%3d%3d]",
								S->Cs[i].pu.h,S->Cs[i].pu.k,S->Cs[i].pu.l,S->Cs[i].pv.h,S->Cs[i].pv.k,S->Cs[i].pv.l,ZA.h,ZA.k,ZA.l,
								S->Ds[j].pu.h,S->Ds[j].pu.k,S->Ds[j].pu.l,S->Ds[j].pv.h,S->Ds[j].pv.k,S->Ds[j].pv.l,ZB.h,ZB.k,ZB.l);
						AddProtocolString(TMP);
						tm++; f++;
					}
				}
				if (f) ts++;
			}
		} else {
			if (n) {
				AddProtocolString(";------------------------------------------------");
				sprintf(TMP,"%s : Lattice=%s , %s", S->L[l].name, lpSys[S->L[l].sys], lpLat[S->L[l].lat] );
				AddProtocolString(TMP);
				for (i=0;i<n;i++) {
					ZA = NormHKL(&S->Cs[i].pu,&S->Cs[i].pv);
					sprintf(TMP,"%3d%3d%3d %3d%3d%3d [%3d%3d%3d] %5.3f %5.3f %5.3f",
						S->Cs[i].pu.h,S->Cs[i].pu.k,S->Cs[i].pu.l,S->Cs[i].pv.h,S->Cs[i].pv.k,S->Cs[i].pv.l,ZA.h,ZA.k,ZA.l,
						S->Cs[i].ew,S->Cs[i].eu,S->Cs[i].ev);
					AddProtocolString(TMP);
				}
				tm+=n; ts++;
			}
		}
		SendDlgItemMessage( hDlg, IDC_PHI_LIST, CB_SETCURSEL,
							(int)SendDlgItemMessage(hDlg,IDC_PHI_LIST,CB_GETCOUNT,0,0)-1, 0 );
		if(GetAsyncKeyState(VK_ESCAPE)) break;
	}
	sprintf(TMP," %3d matches (%d phases)", tm, ts );
	SetDlgItemText( hDlg, IDC_PHI_INFO, TMP );
	SetCursor(hOldCur);
}

/****************************************************************************/
static void EnableFields( HWND hDlg )
{
BOOL fTwo = IsDlgButtonChecked( hDlg, IDC_PHI_LAT2 );
#define xEn(id,b) SendDlgItemMessage(hDlg,id,WM_ENABLE,b,0); EnableWindow(GetDlgItem(hDlg,id),b);

	xEn( IDC_PHI_U2,   fTwo);
	xEn( IDC_PHI_V2,   fTwo);
	xEn( IDC_PHI_W2,   fTwo);
	xEn( IDC_PHI_EU2,  fTwo);
	xEn( IDC_PHI_EV2,  fTwo);
	xEn( IDC_PHI_EW2,  fTwo);
	xEn( IDC_PHI_12W,  fTwo);
	xEn( IDC_PHI_12EW, fTwo);
#undef xEn
}
/*-------------------------------------------------------------------------*/
static void SavePHIDOList( HWND hDlg )
{
FILE	* out;
HWND	hw = GetDlgItem(hDlg, IDC_PHI_LIST);
int		i=0,cnt;

	if (!fioGetFileNameDialog( MainhWnd, "Save PhIdo list", &ofNames[OFN_LIST], "", TRUE))
		return;

	if ((out = fopen( fioFileName(), "w")) == NULL) return;
	cnt = SendMessage(hw, CB_GETCOUNT, 0, 0);
	for(i=0;i<cnt;i++) {
		if (SendMessage(hw,CB_GETLBTEXT, i, (LPARAM)(LPSTR)TMP) != CB_ERR)
			fprintf(out, "%s\n", TMP);
	}
	fclose( out );
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static	BOOL	InitPHIDOObject( HWND hDlg, OBJ* O, LPPHIDO S )
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
	LPELD	eld = (LPELD) &pr->dummy;
	C3i		c0 = {0,0,0};
	FILE *	inf;
	double	al,bl;
	char	fname[_MAX_PATH];
	Lattice3 latt3;
	std::vector<Lattice3> LatticeVec;
//	std::vector<Lattice3>::iterator it;
//	int		i;
	LaueZone *pLZ;

	memset(S, 0, sizeof(PHIDO));

	// Added by Peter on [10/8/2005]
	if( true == eld->m_vLaueZones.empty() )
	{
		MessageBox(NULL, "The pattern must be correctly indexed",
			"PHIDO ERROR", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if( (0 == (eld->m_vLaueZones[0]->m_nFlags & LaueZone::ZONE_INDEXED)) ||
		((TRUE == eld->m_bUseLaueCircles) && (false == eld->m_vbLaueCircles[0])) )
//	if( (false == eld->m_vbLaueCircles[0]) ||
//		(0 == (eld->m_vLaueZones[0]->m_nFlags & LaueZone::ZONE_INDEXED)) )
	{
		MessageBox(NULL, "The Zero Order Laue Zone must be correctly indexed",
			"PHIDO ERROR", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	pLZ = eld->m_vLaueZones[0];
	// End of addition [10/8/2005]

	al = AbsC(pLZ->h_dir.x, pLZ->h_dir.y*O->NI.AspectXY);
	bl = AbsC(pLZ->k_dir.x, pLZ->k_dir.y*O->NI.AspectXY);
//	al = AbsC(eld->m_hx, eld->m_hy*O->NI.AspectXY);
//	bl = AbsC(eld->m_kx, eld->m_ky*O->NI.AspectXY);
	if (al<1. || bl<1.) return FALSE;
//----------------------------------------------------------------------
	S->Cs = (Solution*)malloc( sizeof(Solution)*SMAX );
	S->Ds = (Solution*)malloc( sizeof(Solution)*SMAX );
	S->P  = (C4 *) malloc( sizeof(C4)*VMAX );
	S->pu = (int*) malloc( sizeof(int)*VMAX );
	S->pv = (int*) malloc( sizeof(int)*VMAX );
	// removed by Peter on 30 June 2003
//	S->L  = (Lattice3*)malloc( sizeof(Lattice3)*LMAX );

	if (S->Cs==NULL || S->Ds==NULL || S->P==NULL || S->pu==NULL ||
		S->pv==NULL/*|| S->L==NULL*/) return FALSE;

	GetModuleFileName( hInst, fname, sizeof(fname));
	strcpy(strrchr(fname,'\\'), "\\phido.tbl");
	if( NULL == (inf = fopen(fname,"r")) )
	{
		// Added by Peter on 30 June 2003
		MessageBox(hDlg, _T("PHIDO cannot open file 'phide.tbl'"), _T("PHIDO Error"), MB_OK);
		return FALSE;
	}
//	i = 0;
	while( !feof(inf) )
	{
		fgets(TMP, 80, inf);
		if (TMP[0] == ';')
			continue;						// Skip comment lines
		// changed by Peter on 30 June 2003 in order
		// to support unlimited load of entries
		if( !scanf_Lat(TMP, &latt3))
			continue;						// Skip invalid lines
		else
		{
			// add the lattice to the vector
			LatticeVec.push_back(latt3);
		}
//		if( !scanf_Lat(TMP, &S->L[i]))
//			continue;						// Skip invalid lines
//		i++;
	}
	S->lcnt = LatticeVec.size();
//	S->lcnt = i;
	// copy the data
	if( false == LatticeVec.empty() )
	{
		S->L = (Lattice3*)malloc( sizeof(Lattice3) * S->lcnt );
		std::copy(LatticeVec.begin(), LatticeVec.end(), S->L);
	}

	fclose(inf);

	S->C.pu = c0;
	S->C.pv = c0;
//	S->C.u = eld->m_??; S->C.v = eld->m_??; S->C.w = eld->m_??;
//	S->C.u = 0.28493; S->C.v = 0.21803; S->C.w = 40.0748;

	S->C.u = al/(O->NI.Scale * 1e10);
	S->C.v = bl/(O->NI.Scale * 1e10);
	S->C.w = R2D(AB_Angle( pLZ->h_dir.x, pLZ->h_dir.y*O->NI.AspectXY,
		pLZ->k_dir.x, pLZ->k_dir.y*O->NI.AspectXY));
//	S->C.w = R2D(AB_Angle( eld->m_hx, eld->m_hy*O->NI.AspectXY, eld->m_kx, eld->m_ky*O->NI.AspectXY));

	S->C.eu = 2; S->C.ev = 2; S->C.ew = 2;

	S->D.pu = c0; S->D.pv = c0;
//	S->D.u = 0.23234; S->D.v = 0.43426; S->D.w = 91.9942;
	S->D.u = S->C.u;
	S->D.v = S->C.v;
	S->D.w = S->C.w;
	S->D.eu = 2; S->D.ev = 2; S->D.ew = 2;

//	S->w = 108; S->ew = 2;
	S->w = 90;  S->ew = 2;

	EnableFields( hDlg );

#ifdef USE_FONT_MANAGER
	SendDlgItemMessage(hDlg, IDC_PHI_LIST, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_MEDIUM_FONT), 0);
#else
	SendDlgItemMessage(hDlg, IDC_PHI_LIST, WM_SETFONT, (WPARAM)hfMedium, 0);
#endif
//	SendDlgItemMessage(hDlg, IDC_PHI_INFO, WM_SETFONT, HF_Dial, 0);

	SetDlgItemDouble(hDlg, IDC_PHI_U1, S->C.u,6);
	SetDlgItemDouble(hDlg, IDC_PHI_V1, S->C.v,6);
	SetDlgItemDouble(hDlg, IDC_PHI_W1, S->C.w,6);
	SetDlgItemDouble(hDlg, IDC_PHI_EU1, S->C.eu,3);
	SetDlgItemDouble(hDlg, IDC_PHI_EV1, S->C.ev,3);
	SetDlgItemDouble(hDlg, IDC_PHI_EW1, S->C.ew,3);

	SetDlgItemDouble(hDlg, IDC_PHI_U2, S->D.u,6);
	SetDlgItemDouble(hDlg, IDC_PHI_V2, S->D.v,6);
	SetDlgItemDouble(hDlg, IDC_PHI_W2, S->D.w,6);
	SetDlgItemDouble(hDlg, IDC_PHI_EU2, S->D.eu,3);
	SetDlgItemDouble(hDlg, IDC_PHI_EV2, S->D.ev,3);
	SetDlgItemDouble(hDlg, IDC_PHI_EW2, S->D.ew,3);

	SetDlgItemDouble(hDlg, IDC_PHI_12W, S->w,6);
	SetDlgItemDouble(hDlg, IDC_PHI_12EW, S->ew,3);

	return	TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LRESULT CALLBACK PhidoWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*		O = OBJ::GetOBJ( hDlg );
	LPPHIDO		S = (LPPHIDO) &O->dummy;

	switch (message)
	{
		case WM_INITDIALOG:
			{
				// ADDED BY PETER ON 29 JUNE 2004
				// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
				// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
				O = (OBJ*)lParam;
				S = (LPPHIDO) &O->dummy;
				InitializeObjectDialog(hDlg, O);
				
				if ( ! InitPHIDOObject( hDlg, O, S ))
					DestroyWindow( hDlg );
				
#ifdef USE_FONT_MANAGER
				HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
#else
				HFONT hFont = hfNormal;
#endif
				SendDlgItemMessage(hDlg, IDC_PHIDO_ANGSTR1, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_PHIDO_ANGSTR2, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_PHIDO_ANGSTR4, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_PHIDO_ANGSTR5, WM_SETFONT, (WPARAM)hFont, 0);
				
				TextToDlgItemW( hDlg, IDC_PHIDO_ANGSTR1, "1/%c ±", ANGSTR_CHAR );
				TextToDlgItemW( hDlg, IDC_PHIDO_ANGSTR2, "1/%c ±", ANGSTR_CHAR );
				TextToDlgItemW( hDlg, IDC_PHIDO_ANGSTR4, "1/%c ±", ANGSTR_CHAR );
				TextToDlgItemW( hDlg, IDC_PHIDO_ANGSTR5, "1/%c ±", ANGSTR_CHAR );
				//			SetDlgItemText( hDlg, IDC_PHIDO_ANGSTR1, "1/\302 ±" );
				//			SetDlgItemText( hDlg, IDC_PHIDO_ANGSTR2, "1/\302 ±" );
				//			SetDlgItemText( hDlg, IDC_PHIDO_ANGSTR4, "1/\302 ±" );
				//			SetDlgItemText( hDlg, IDC_PHIDO_ANGSTR5, "1/\302 ±" );
			}
			return TRUE;

		case WM_NCDESTROY:
			if (S->Cs) free(S->Cs); S->Cs = NULL;
			if (S->Ds) free(S->Ds); S->Ds = NULL;
			if (S->P)  free(S->P);  S->P = NULL;
			if (S->pu) free(S->pu); S->pu = NULL;
			if (S->pv) free(S->pv); S->pv = NULL;
			if (S->L)  free(S->L);  S->L = NULL;
			objFreeObj( hDlg );
			break;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
		case WM_QUERYOPEN:
			ActivateObj(hDlg);
			break;

		case FM_UpdateNI:
			O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    		objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    		break;

		case WM_CLOSE:
			DestroyWindow(hDlg); return TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_PHI_FIND:		doit(hDlg, S);	break;
			case IDC_PHI_LAT2:		EnableFields( hDlg ); break;
			case IDC_PHI_CLRLIST:	SendDlgItemMessage( hDlg, IDC_PHI_LIST, CB_RESETCONTENT, 0, 0 ); break;
			case IDC_PHI_SAVE:		SavePHIDOList( hDlg ); break;
			case IDC_CANCEL:		DestroyWindow( hDlg ); return TRUE;
		}
		break;
	}
	return FALSE;
}
//	sprintf(TMP,  "Phase Identification from d-spacing (%s)", GetParentImageName(hDlg));
