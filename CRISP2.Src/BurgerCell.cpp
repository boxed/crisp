//////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AnaliteX
// BuergerCell.cpp : Defines the 3D cell reduction.
//

#include "StdAfx.h"

#include "commondef.h"
#include "objects.h"
#include "eld.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////

static double a_, b_, c_, d_, e_, f_;
static double multiplier_significant_change_test_;
static size_t n_iterations_;
static size_t iteration_limit_;
static size_t min_n_no_significant_change_;
static size_t n_no_significant_change_;
static CMatrix3x3 r_inv_;
static CVector3d last_abc_significant_change_test_;
static bool termination_due_to_significant_change_test_;

struct error_iteration_limit_exceeded {};
struct error_degenerate_unit_cell_parameters {};
struct error_z_became_negative {};

struct duo
{
	duo(int _1, int _2) { a0 = _1, a1 = _2; }
	int a0;
	int a1;
};

duo def_test()
{
	int n_zero = 0;
	int n_positive = 0;
	if( 0 < d_ ) n_positive += 1;
	else if (!(d_ < 0)) n_zero += 1;
	if( 0 < e_ ) n_positive += 1;
	else if (!(e_ < 0)) n_zero += 1;
	if( 0 < f_ ) n_positive += 1;
	else if (!(f_ < 0)) n_zero += 1;
	return duo(n_zero, n_positive);
}

bool def_gt_0()
{
	duo nz_np = def_test();
	return (3 == nz_np.a1) || ((0 == nz_np.a0) && (1 == nz_np.a1));
}

//float spoil_optimization(float value) { return value; }
//double spoil_optimization(double value) { return value; }

double significant_change_test(double const& new_value, size_t i)
{
	double m_new = multiplier_significant_change_test_ * new_value;
	double diff = new_value - last_abc_significant_change_test_[(int)i];
	double m_new_plus_diff = m_new + diff;
	double m_new_plus_diff_minus_m_new = m_new_plus_diff - m_new;
	return (0 != m_new_plus_diff_minus_m_new);
}

bool significant_change_test()
{
	if( significant_change_test(a_, 0) ||
		significant_change_test(b_, 1) ||
		significant_change_test(c_, 2) )
	{
		n_no_significant_change_ = 0;
	}
	else
	{
		n_no_significant_change_++;
		if( n_no_significant_change_ == min_n_no_significant_change_ )
		{
            return false;
		}
	}
	last_abc_significant_change_test_ = CVector3d(a_, b_, c_);
	return true;
}

inline int entier(double const& x)
{
	int result = static_cast<int>(x);
	if( x - result < 0 ) result--;
	if( !(x - result < 1) ) result++; // work around rounding errors
	return result;
}

void cb_update(CMatrix3x3 const& m)
{
	if( n_iterations_ == iteration_limit_ )
	{
		throw error_iteration_limit_exceeded();
	}
	r_inv_ = r_inv_ * m;
	n_iterations_ += 1;
}

void n1_action()
{
	static const CMatrix3x3 m(0,-1,0, -1,0,0, 0,0,-1);
	cb_update(m);
	std::swap(a_, b_);
	std::swap(d_, e_);
}

void n2_action()
{
	static const CMatrix3x3 m(-1,0,0, 0,0,-1, 0,-1,0);
	cb_update(m);
	std::swap(b_, c_);
	std::swap(e_, f_);
}

void n3_true_action()
{
	CMatrix3x3 m;
	m.LoadIdentity();
	if( d_ < 0 ) m(0, 0) = -1;
	if( e_ < 0 ) m(1, 1) = -1;
	if( f_ < 0 ) m(2, 2) = -1;
	cb_update(m);
	d_ = abs(d_);
	e_ = abs(e_);
	f_ = abs(f_);
}

void n3_false_action()
{
	CMatrix3x3 m;
	m.LoadIdentity();
	int z = -1;
	if (0 < d_) m(0, 0) = -1;
	else if (!(d_ < 0)) z = 0;
	if (0 < e_) m(1, 1) = -1;
	else if (!(e_ < 0)) z = 1;
	if (0 < f_) m(2, 2) = -1;
	else if (!(f_ < 0)) z = 2;
	if( m(0, 0) * m(1, 1) * m(2, 2) < 0)
	{
		if(z == -1)
			throw error_z_became_negative();
		m(z, z) = -1;
	}
	cb_update(m);
	d_ = -abs(d_);
	e_ = -abs(e_);
	f_ = -abs(f_);
}

bool b2_action()
{
	if (!(b_ < abs(d_))) return false;
	int j = entier((d_+b_)/(2*b_));
	if (j == 0) return false;
	cb_update(CMatrix3x3(1,0,0,0,1,-j,0,0,1));
	c_ += j*j*b_ - j*d_;
	d_ -= 2*j*b_;
	e_ -= j*f_;
	if (!(0 < c_)) throw error_degenerate_unit_cell_parameters();
	return true;
}

bool b3_action()
{
	if (!(a_ < abs(e_))) return false;
	int j = entier((e_+a_)/(2*a_));
	if (j == 0) return false;
	cb_update(CMatrix3x3(1,0,-j,0,1,0,0,0,1));
	c_ += j*j*a_ - j*e_;
	d_ -= j*f_;
	e_ -= 2*j*a_;
	if (!(0 < c_)) throw error_degenerate_unit_cell_parameters();
	return true;
}

bool b4_action()
{
	if (!(a_ < abs(f_))) return false;
	int j = entier((f_+a_)/(2*a_));
	if (j == 0) return false;
	cb_update(CMatrix3x3(1,-j,0,0,1,0,0,0,1));
	b_ += j*j*a_ - j*f_;
	d_ -= j*e_;
	f_ -= 2*j*a_;
	if (!(0 < b_)) throw error_degenerate_unit_cell_parameters();
	return true;
}

bool b5_action()
{
	double de = d_ + e_;
	double fab = f_ + a_ + b_;
	if (!(de+fab < 0)) return false;
	int j = entier((de+fab)/(2*fab));
	if (j == 0) return false;
	cb_update(CMatrix3x3(1,0,-j,0,1,-j,0,0,1));
	c_ += j*j*fab-j*de;
	d_ -= j*(2*b_+f_);
	e_ -= j*(2*a_+f_);
	if (!(0 < c_)) throw error_degenerate_unit_cell_parameters();
	return true;
}

bool step()
{
	// N1
	if( b_ < a_ )
	{
		n1_action();
	}
	// N2
	if( c_ < b_ )
	{
		n2_action();
		return true;
	}
	// N3
	if( def_gt_0() )
	{
		n3_true_action();
	}
	else
	{
		n3_false_action();
		if( !significant_change_test() )
		{
            return false;
		}
	}
	if( b2_action() ) return true;
	if( b3_action() ) return true;
	if( b4_action() ) return true;
	if( b5_action() ) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only application object
void Buerger_ReduceCell(double *pdCell, double *pdRedCell, CMatrix3x3 &cb)
{
	double a, b, c;

	// initialize
	iteration_limit_ = 100;
	multiplier_significant_change_test_ = 10;
	min_n_no_significant_change_ = 2;
	r_inv_.LoadIdentity();
	cb.LoadIdentity();
	n_iterations_ = 0;
	n_no_significant_change_ = 0;
	termination_due_to_significant_change_test_ = false;

	if( (NULL == pdCell) || (NULL == pdRedCell) )
	{
		return;
	}
	// Executes the reduction algorithm.
	a = pdCell[0];
	b = pdCell[1];
	c = pdCell[2];
	a_ = SQR(a);
	b_ = SQR(b);
	c_ = SQR(c);
	d_ = 2*b*c*FastCos(DEG2RAD(pdCell[3]));
	e_ = 2*a*c*FastCos(DEG2RAD(pdCell[4]));
	f_ = 2*a*b*FastCos(DEG2RAD(pdCell[5]));
    last_abc_significant_change_test_ = CVector3d(-a_, -b_, -c_);
	try
	{
		// iterate
		while( true == step() )
			;
	}
	catch( error_iteration_limit_exceeded & )
	{
		Trace("Buerger_ReduceCell() -> iteration limit of 100 exceeded.");
	}
	catch( error_degenerate_unit_cell_parameters & )
	{
		Trace("Buerger_ReduceCell() -> found degenerate unit cell parameters.");
	}
	catch( error_z_became_negative &)
	{
		Trace("Buerger_ReduceCell() -> z is negative during iterations.");
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("Buerger_ReduceCell() -> %s"), e.what() ));
	}

	a_ = FastSqrt(a_), b_ = FastSqrt(b_), c_ = FastSqrt(c_);
	pdRedCell[0] = a_;
	pdRedCell[1] = b_;
	pdRedCell[2] = c_;
	pdRedCell[3] = FastAcos(d_*0.5/(b_*c_));
	pdRedCell[4] = FastAcos(e_*0.5/(a_*c_));
	pdRedCell[5] = FastAcos(f_*0.5/(a_*b_));

	cb = r_inv_;
	cb.Transpose();
}

void Buerger_ReduceCell(const LATTICE_CELL &BaseCell, LATTICE_CELL &ReducedCell)
{
	double cell[6], redcell[6];
	int i;

	for(i = 0; i < 6; i++)
	{
		cell[i] = BaseCell.cell[i];
	}
	Buerger_ReduceCell(cell, redcell, ReducedCell.cb);
	for(i = 0; i < 6; i++)
	{
		ReducedCell.cell[i] = redcell[i];
	}
}
