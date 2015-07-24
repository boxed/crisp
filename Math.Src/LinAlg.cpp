#include "StdAfx.h"

#include "LinAlg.h"

void dummy()
{
	CPlaned pd;
	CPlanef pf;
	std::vector<CVector3d> vInd;
	std::vector<CVector3f> vInf;

	pd.ClipByPlane(vInd, vInd);
//	pf.ClipByPlane(vInf, vInf);
}
/*
template <>
void CPlane<double>::ClipByPlane(const std::vector<CVector3<double> > &vIn, std::vector<CVector3<double> > &vOut) const;

template <>
void CPlane<float>::ClipByPlane(const std::vector<vec_type> &vIn, std::vector<vec_type> &vOut) const;
*/
template <typename _Tx>
void CPlane<_Tx>::ClipByPlane(const std::vector<vec_type> &vIn, std::vector<vec_type> &vOut) const
{
	vec_type s, e, cur;
	double dist, d1, d2;
	std::vector<vec_type>::const_iterator vit;
	std::vector<double> vDist(vIn.size());

	// clear the output array
	vOut.clear();
/*	// clip each edge
	int i = 0;
	for(vit = vIn.begin(); vit != vIn.end(); ++vit)
	{
		vDist[i++] = (m_vN & (*vit)) + m_Dist;
	}
	i = 0;
	for(vit = vIn.begin(); vit != vIn.end()-1; ++vit, i++)
	{
		dist = vDist[i];
	    // Need no clipping
		if( dist >= 0 )
		{
			vOut.push_back(*vit);
		}
		if( CSIGN(dist) != CSIGN(vDist[i+1]) )
		{
			s = *vit;
			e = *(vit+1);
			// clip
			CRay<_Tx> ray(s, Normalize(e-s));
			if( true == Intersect(ray, dist) )
			{
				if( FastAbs(dist) > 1e-4 )
					vOut.push_back(ray.Point(dist));
			}
		} // endof needs clipping
	}
	// check last point for clipping
	if( (vDist.back() >= 0) && (dist < 0) )
		vOut.push_back(vIn.back());
	// check last line for clipping (warp around!)
	if( CSIGN(vDist[0]) != CSIGN(vDist.back()) )
	{
		s = vIn.back();
		e = vIn.front();
		// clip
		CRay<_Tx> ray(s, Normalize(e-s));
		if( true == Intersect(ray, dist) )
		{
			if( FastAbs(dist) > 1e-4 )
				vOut.push_back(ray.Point(dist));
		}
	}
/*/
	// clip each edge
	for(vit = vIn.begin(); vit != vIn.end(); ++vit)
	{
		// start vertex
		s = *vit;
		// end vertex
		if( vit == vIn.end() - 1 )
			e = *vIn.begin();
		else
			e = *(vit+1);
		d1 = ClassifyPoint(s);
		d2 = ClassifyPoint(e);
		if( d1 >= -1e-3 )
		{
			vOut.push_back(s);
		}
		if( CSIGN(d1) != CSIGN(d2) )
		{
			// clip
			CRay<_Tx> ray(s, Normalize(e-s));
			if( true == Intersect(ray, dist) )
			{
				if( FastAbs(dist) > 1e-4 )
					vOut.push_back(ray.Point(dist));
			}
		}
	}
//*/
/*
	// clip each edge
	for(vit = vIn.begin(); vit != vIn.end(); ++vit)
	{
		// start vertex
		s = *vit;
		// end vertex
		if( vit == vIn.end() - 1 )
			e = *vIn.begin();
		else
			e = *(vit+1);
		d1 = ClassifyPoint(s);
		d2 = ClassifyPoint(e);
		if( (d1 < -1e-3) && (d2 >= -1e-3) )
		{
			// clip
			CRay<_Tx> ray(s, Normalize(e-s));
			if( true == Intersect(ray, dist) )
			{
				if( FastAbs(dist) > 1e-4 )
					vOut.push_back(ray.Point(dist));
//				vOut.push_back(e);
			}
		}
		else if( (d1 >= -1e-3) && (d2 < -1e-3) )
		{
			// clip
			CRay<_Tx> ray(s, Normalize(e-s));
			if( true == Intersect(ray, dist) )
			{
				vOut.push_back(s);
				if( FastAbs(dist) > 1e-4 )
					vOut.push_back(ray.Point(dist));
			}
		}
		else if( (d1 >= -1e-3) && (d2 >= -1e-4) )
		{
			vOut.push_back(s);
//			vOut.push_back(e);
		}
	}
*/
}

/////////////////////////////////////////////////////////////////
CMatrix3x3::CMatrix3x3()
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			d[i][j] = 0.0;
		}
	}
}

CMatrix3x3::CMatrix3x3(double fVal)
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			d[i][j] = fVal;
		}
	}
}

CMatrix3x3::CMatrix3x3(const double *pMatrix)
{
	int i, off;
	for(off = i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++, off++)
		{
			d[i][j] = pMatrix[off];
		}
	}
}

CMatrix3x3::CMatrix3x3(const CMatrix3x3 &m)
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			d[i][j] = m.d[i][j];
		}
	}
}

CMatrix3x3::~CMatrix3x3()
{
}

CVector3d CMatrix3x3::Row(int iRow) const
{
	return CVector3d(d[iRow][0], d[iRow][1], d[iRow][2]);
}

CVector3d CMatrix3x3::Col(int iCol) const
{
	return CVector3d(d[0][iCol], d[1][iCol], d[2][iCol]);
}

void CMatrix3x3::Row(int iRow, const CVector3d &row)
{
	d[iRow][0] = row.x;
	d[iRow][1] = row.y;
	d[iRow][2] = row.z;
}

void CMatrix3x3::Col(int iCol, const CVector3d &col)
{
	d[0][iCol] = col.x;
	d[1][iCol] = col.y;
	d[2][iCol] = col.z;
}

void CMatrix3x3::LoadIdentity()
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			d[i][j] = (i == j) ? 1.0 : 0.0;
		}
	}
}

double CMatrix3x3::Determinant()
{
	return (d[0][0] * d[1][1] * d[2][2] +
		    d[1][0] * d[2][1] * d[0][2] +
		    d[2][0] * d[0][1] * d[1][2])
		 -( d[0][0] * d[2][1] * d[1][2] +
		    d[1][0] * d[0][1] * d[2][2] +
		    d[2][0] * d[1][1] * d[0][2]);
}

bool CMatrix3x3::Invert(CMatrix3x3 &out)
{
	double det;		// det
	double invDet;
	double D[3][3];  // alg reminder
	
	if( 0.0 == (det = Determinant()) )
	{
		::OutputDebugString("CMatrix3x3::Invert() -> determinant is ZERO.\n");
		return false;
	}

	D[0][0] =  (d[1][1] * d[2][2] - d[2][1] * d[1][2]);
	D[0][1] = -(d[0][1] * d[2][2] - d[2][1] * d[0][2]);
	D[0][2] =  (d[0][1] * d[1][2] - d[1][1] * d[0][2]);

	D[1][0] = -(d[1][0] * d[2][2] - d[2][0] * d[1][2]);
	D[1][1] =  (d[0][0] * d[2][2] - d[2][0] * d[0][2]);
	D[1][2] = -(d[0][0] * d[1][2] - d[1][0] * d[0][2]);

	D[2][0] =  (d[1][0] * d[2][1] - d[2][0] * d[1][1]);
	D[2][1] = -(d[0][0] * d[2][1] - d[2][0] * d[0][1]);
	D[2][2] =  (d[0][0] * d[1][1] - d[1][0] * d[0][1]);

	invDet = 1.0 / det;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			out.d[i][j] = invDet * D[i][j];
		}
	}
	return true;
}

void CMatrix3x3::Transpose()
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = i + 1; j < 3; j++)
		{
			std::swap(d[i][j], d[j][i]);
		}
	}
}

void CMatrix3x3::Translate(const CVector3d &v)
{
	LoadIdentity();
	d[0][2] = v.x;
	d[1][2] = v.y;
}

void CMatrix3x3::Scale(const CVector3d &v)
{
	LoadIdentity();
	d[0][0] = v.x;
	d[1][1] = v.y;
	d[2][2] = v.z;
}

void CMatrix3x3::RotateZ(double dAng)
{
	double dSin, dCos;

	FastSinCos(dAng, dSin, dCos);
	LoadIdentity();
	d[0][0] =  dCos;
	d[1][1] =  dCos;
	d[0][1] =  dSin;
	d[1][0] = -dSin;
}

CMatrix3x3 operator * (const CMatrix3x3& A, const CMatrix3x3 &B)
{
	double sum;
	CMatrix3x3 res;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			sum = 0;
			for(int k = 0; k < 3; k++)
				sum += A.d[i][k] * B.d[k][j];
			res.d[i][j] = sum;
		}
	}
	return res;
}

//---------------------------------------------------------------------------
//	CMatrix4x4 math
//---------------------------------------------------------------------------
CMatrix4x4::CMatrix4x4()
{
	ZeroMatrix();
}

CMatrix4x4::CMatrix4x4(double fVal)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			d[i][j] = (i == j) ? fVal : 0.0;
		}
	}
	d[3][3] = 1.0;
}

void CMatrix4x4::LoadIdentity()
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			d[i][j] = (i == j) ? 1.0 : 0.0;
		}
	}
	d[3][3] = 1.0;
}

CMatrix4x4::~CMatrix4x4()
{
}

void CMatrix4x4::LoadMatrix(double* pfData)
{
	memcpy(d, pfData, sizeof(double) << 4);
}

void CMatrix4x4::ZeroMatrix()
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			d[i][j] = 0;
		}
	}
}

bool CMatrix4x4::IsOrthonormal()
{
	CVector3d Col1,Col2,Col3;
	CVector3d Col1CrossCol2;
	bool IsOrthonormal;

	Col1 = CVector3d(d[0][0], d[1][0], d[2][0]);
	Col2 = CVector3d(d[0][1], d[1][1], d[2][1]);
	Col3 = CVector3d(d[0][2], d[1][2], d[2][2]);

	Col1CrossCol2 = Col1 ^ Col2;

	IsOrthonormal = Col1CrossCol2.Equals(Col3, 0.001f);
	if (IsOrthonormal == false)
	{
		Col3 = - Col3;
		IsOrthonormal = Col1CrossCol2.Equals(Col3, 0.001f);
	}

	return IsOrthonormal;
}

CMatrix4x4& CMatrix4x4::operator += (const CMatrix4x4& A)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			d[i][j] += A.d[i][j];
		}
	}
	return *this;
}

CMatrix4x4& CMatrix4x4::operator -= (const CMatrix4x4& A)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			d[i][j] -= A.d[i][j];
		}
	}
	return *this;
}

CMatrix4x4& CMatrix4x4::operator *= (double v)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			d[i][j] *= v;
		}
	}
	return *this;
}

CMatrix4x4& CMatrix4x4::operator *= (const CMatrix4x4& A)
{
	double sum;
	CMatrix4x4 res = *this;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			sum = 0;
			for(int k = 0; k < 4; k++)
				sum += res.d[i][k] * A.d[k][j];
			d[i][j] = sum;
		}
	}
	return *this;
}

void CMatrix4x4::Invert()
{
	CMatrix4x4 Out;
	double x;

	Out.LoadIdentity();
	for(int i = 0; i < 4; i++)
	{
		x = d[i][i];
		if(x != 1)
		{
			for(int j = 0; j < 4; j++)
			{
				Out.d[i][j] /= x;
				d[i][j]	 /= x;
			}
		}
		for(int j = 0; j < 4; j++)
		{
			if(j != i)
			{
				if(d[j][i] != 0)
				{
					double mulby = d[j][i];
					for(int k = 0; k < 4; k++)
					{
						d[j][k]	 -= mulby*d[i][k];
						Out.d[j][k] -= mulby*Out.d[i][k];
					}
				}
			}
		}
	}
	*this = Out;
}

void CMatrix4x4::Transpose()
{
	double t;

	for(int i = 0; i < 4; i++)
	{
		for(int j = i; j < 4; j++)
		{
			if(i != j)
			{
				t = d[i][j];
				d[i][j] = d[j][i];
				d[j][i] = t;
			}
		}
	}
}

CMatrix4x4 operator * (const CMatrix4x4 &A, double v)
{
	CMatrix4x4 Res;
	for(int i = 0; i < 4; i++)
		for(int j = i; j < 4; j++)
			Res.d[i][j] = A.d[i][j] * v;
	return Res;
}

CMatrix4x4 operator * (const CMatrix4x4 &A, const CMatrix4x4 &B)
{
	CMatrix4x4 Res;
	double sum;

	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			sum = 0;
			for(int k = 0; k < 4; k++)
				sum += A.d[i][k] * B.d[k][j];
			Res.d[i][j] = sum;
		}
	}
	return Res;
}

CVector3d operator * (const CMatrix4x4 &M, const CVector3d &v)
{
	CVector3d out;
	double denom;

	out.x = v.x*M.d[0][0] + v.y*M.d[0][1] + v.z*M.d[0][2] + M.d[0][3];
	out.y = v.x*M.d[1][0] + v.y*M.d[1][1] + v.z*M.d[1][2] + M.d[1][3];
	out.z = v.x*M.d[2][0] + v.y*M.d[2][1] + v.z*M.d[2][2] + M.d[2][3];
	denom = v.x*M.d[3][0] + v.y*M.d[3][1] + v.z*M.d[3][2] + M.d[3][3];
	if(denom != 1.0)
		out /= denom;
	return out;
}

CVector3d operator * (const CVector3d &v, const CMatrix4x4 &M)
{
	CVector3d out;
	double denom;

	out.x = v.x*M.d[0][0] + v.y*M.d[1][0] + v.z*M.d[2][0] + M.d[3][0];
	out.y = v.x*M.d[0][1] + v.y*M.d[1][1] + v.z*M.d[2][1] + M.d[3][1];
	out.z = v.x*M.d[0][2] + v.y*M.d[1][2] + v.z*M.d[2][2] + M.d[3][2];
	denom = v.x*M.d[0][3] + v.y*M.d[1][3] + v.z*M.d[2][3] + M.d[3][3];
	if(denom != 1.0)
		out /= denom;
	return out;
}

CVector3d CMatrix4x4::GetTranslation()
{
	return CVector3d(d[0][3], d[1][3], d[2][3]);
}

CMatrix3x3 CMatrix4x4::GetRotation()
{
	CMatrix3x3 r;
	r.d[0][0] = d[0][0]; r.d[0][1] = d[0][1]; r.d[0][2] = d[0][2];
	r.d[1][0] = d[1][0]; r.d[1][1] = d[1][1]; r.d[1][2] = d[0][2];
	r.d[2][0] = d[2][0]; r.d[2][1] = d[2][1]; r.d[2][2] = d[0][2];
	return r;
}

void CMatrix4x4::SetTranslation(const CVector3d &cLoc)
{
	LoadIdentity();
	d[0][3] = cLoc.x;
	d[1][3] = cLoc.y;
	d[2][3] = cLoc.z;
}

void CMatrix4x4::SetRotation(const CMatrix3x3 &mRot)
{
	LoadIdentity();
	d[0][0] = mRot.d[0][0]; d[0][1] = mRot.d[0][1]; d[0][2] = mRot.d[0][2];
	d[1][0] = mRot.d[1][0]; d[1][1] = mRot.d[1][1]; d[1][2] = mRot.d[1][2];
	d[2][0] = mRot.d[2][0]; d[2][1] = mRot.d[2][1]; d[2][2] = mRot.d[2][2];
}

void CMatrix4x4::SetScale(const CVector3d &cScale)
{
	LoadIdentity();
	d[0][0] = cScale.x;
	d[1][1] = cScale.y;
	d[2][2] = cScale.z;
}

void CMatrix4x4::RotateX(double fAngle)
{
	double Sin, Cos;
	LoadIdentity();
	fAngle = DEG2RAD(fAngle);
	FastSinCos(fAngle, Sin, Cos);
	d[1][1] =  Cos;
	d[1][2] =  Sin;
	d[2][1] = -Sin;
	d[2][2] =  Cos;
}

void CMatrix4x4::RotateY  (double fAngle)
{
	double Sin, Cos;
	LoadIdentity();
	fAngle = DEG2RAD(fAngle);
	FastSinCos(fAngle, Sin, Cos);
	d[0][0] =  Cos;
	d[0][2] =  Sin;
	d[2][0] = -Sin;
	d[2][2] =  Cos;
}

void CMatrix4x4::RotateZ(double fAngle)
{
	double Sin, Cos;
	LoadIdentity();
	fAngle = DEG2RAD(fAngle);
	FastSinCos(fAngle, Sin, Cos);
	d[0][0] =  Cos;
	d[1][0] = -Sin;
	d[0][1] =  Sin;
	d[1][1] =  Cos;
}

void CMatrix4x4::MirrorX()
{
	LoadIdentity();
	d[0][0] = -1.0f;
}

void CMatrix4x4::MirrorY()
{
	LoadIdentity();
	d[1][1] = -1.0f;
}

void CMatrix4x4::MirrorZ()
{
	LoadIdentity();
	d[2][2] = -1.0f;
}

void CMatrix4x4::SetView( const CVector3d &dvFrom, const CVector3d &dvTo, CVector3d &dvUp )
{
	// Create the View vector
	CVector3d dvView = dvTo - dvFrom;

	double fLength = !dvView;
	if( fLength < 1e-6f )
		return;

	// Normalize the view vector
	dvView /= fLength;

	// Get the dot product, and calculate the projection of the view
	// vector onto the up vector. The projection is the y basis vector.
	double fDotProduct = dvUp & dvView;

	dvUp = dvUp - fDotProduct * dvView;

	// If this vector has near-zero length because the input specified a
	// bogus up vector, let's try a default up vector
	if( 1e-6f > ( fLength = !dvUp ) )
	{
		dvUp = CVector3d( 0.0f, 1.0f, 0.0f ) - dvView.y * dvView;

		// If we still have near-zero length, resort to a different axis.
		if( 1e-6f > ( fLength = !dvUp ) )
		{
			dvUp = CVector3d( 0.0f, 0.0f, 1.0f ) - dvView.z * dvView;

			if( 1e-6f > ( fLength = !dvUp ) )
				return;
		}
	}

	// Normalize the y basis vector
	dvUp /= fLength;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors
	CVector3d dvRight = dvUp ^ dvView;

	LoadIdentity();
	d[0][0] = dvRight.x;	d[0][1] = dvUp.x;	d[0][2] = dvView.x;
	d[1][0] = dvRight.y;	d[1][1] = dvUp.y;	d[1][2] = dvView.y;
	d[2][0] = dvRight.z;	d[2][1] = dvUp.z;	d[2][2] = dvView.z;

	// Do the translation values (rotations are still about the eyepoint)
	d[3][0] = - ( dvFrom & dvRight );
	d[3][1] = - ( dvFrom & dvUp );
	d[3][2] = - ( dvFrom & dvView );
}
