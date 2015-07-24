#pragma once

#include "../typedefs.h"
#include "MathDef.h"
#include "LinAlg.h"
#include "Vector.h"

namespace UnitCell
{

	enum
	{
		_A = 0,
		_B = 1,
		_C = 2,
		ALPHA = 3,
		BETA = 4,
		GAMMA = 5
	};

	enum
	{
		_H = 0,
		_K = 1,
		_L = 2
	};

	enum CELL_DIMENSION
	{
		CELL_2D = 1,
		CELL_3D = 2,
	};

	enum CELL_TYPE
	{
		REAL = 1,
		RECIPROCAL = 2,
	};

	enum ANGLES
	{
		DEGREES = 0,
		RADIANS = 1
	};

	enum SYMMETRY
	{
		NONE				= 0,
		TRICLINIC			= 1,
		MONOCLINIC			= 2,
		ORTHORHOMBIC		= 3,
		TETRAGONAL			= 4,
		HEXAGONAL			= 5,
		RHOMBOHEDRAL		= 6,
		CUBIC				= 7,
	};

	enum CENTERING
	{
		P_CENTRED			= 0,
		A_CENTRED			= 1,
		B_CENTRED			= 2,
		C_CENTRED			= 3,
		I_CENTRED			= 4,
		F_CENTRED			= 5,
		R_CENTRED			= 6,
	};

	typedef struct
	{
		HKLMiller	hkl;
		double		d;

	} REFLEX, *LPREFLEX;

	typedef vector<REFLEX> CReflVector;

	class CUnitCell
	{
	// constructor/destructor
	public:
//		CUnitCell2();
		virtual ~CUnitCell()	{ }

		static CUnitCell *CreateUnitCell(CELL_DIMENSION nDim);
		static bool DestroyUnitCell(CUnitCell *pUnitCell);
		
		virtual void CalculateVolume(CELL_TYPE nType) = 0;			// calculates the volume
		virtual void CalculateParameters(CELL_TYPE nType) = 0;		// calculates the cell parameters
		virtual double CalculateH(const HKLMiller &hkl,
			CELL_TYPE nType = REAL) = 0;	// calculate |H| = 1 / d for given HKL data

		virtual void SetParameters(const double *pParams,
			ANGLES nAngles = DEGREES, CELL_TYPE nType = REAL) = 0;	// set the parameters
		virtual void SetParameters(const float *pParams,
			ANGLES nAngles = DEGREES, CELL_TYPE nType = REAL) = 0;	// set the parameters

		virtual double GetParameter(UINT nParam, CELL_TYPE nType = REAL) const = 0;
		virtual const double *GetParameters(CELL_TYPE nType = REAL) = 0;
		virtual double GetCosine(UINT nParam, CELL_TYPE nType = REAL) = 0;
		virtual double GetSine(UINT nParam, CELL_TYPE nType = REAL) = 0;
		virtual double GetVolume(CELL_TYPE nType = REAL) = 0;

		virtual BOOL RefineParameters(SYMMETRY nSymmetry, CReflVector &vReflexes) = 0;

		// coordinate transformations
		virtual void CalcTransformMatrix() = 0;

		CELL_DIMENSION GetCellDim() { return m_nCellDimension; }

		void GetMatrix(CMatrix3x3 &out) { out = m_mCryst2Cartes; }
		void GetInvMatrix(CMatrix3x3 &out) { out = m_mInvCryst2Cartes; }

		void GetRecMatrix(CMatrix3x3 &out) { out = m_mRecCryst2Cartes; }
		void GetRecInvMatrix(CMatrix3x3 &out) { out = m_mInvRecCryst2Cartes; }

		void TransformPoint(const CVector3d &in, CVector3d &out)
		{
			out = m_mCryst2Cartes * in;
		}
		void TransformPoint(const CVector3f &in, CVector3f &out)
		{
			out = m_mCryst2Cartes * in;
		}
		void TransformRPoint(const CVector3d &in, CVector3d &out)
		{
			out = m_mInvCryst2Cartes * in;
		}
		void TransformRPoint(const CVector3f &in, CVector3f &out)
		{
			out = m_mInvCryst2Cartes * in;
		}
	protected:
		// matrix for coordinate transformations
		CMatrix3x3 m_mCryst2Cartes;		// first is row index (Y), next is column index  (X)
		CMatrix3x3 m_mInvCryst2Cartes;	// first is row index (Y), next is column index  (X)

		// matrix for coordinate transformations in reciprocal space
		CMatrix3x3 m_mRecCryst2Cartes;		// first is row index (Y), next is column index  (X)
		CMatrix3x3 m_mInvRecCryst2Cartes;	// first is row index (Y), next is column index  (X)

		CELL_DIMENSION m_nCellDimension;
	};

/*
	class CUnitCellSolver
	{
	public:
		CUnitCellSolver();
	};
*/
};	// namespace UnitCell
