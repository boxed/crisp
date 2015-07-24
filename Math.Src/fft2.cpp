/****************************************************************************/
/* Copyright (c) 2006 Ananlitex *********************************************/
/****************************************************************************/
/* CRISP2.EDRD_centre * by Peter Oleynikov **********************************/
/****************************************************************************/

#include "StdAfx.h"

#include "MathKrnl.h"

#define SWAP(x,y)		tmp=(x);(x)=(y);(y)=tmp;

bool Fft1D_Calculate(double *data, size_t nSize, int iSign)
{
	int n, nn, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi, tmp;
	
	nn = nSize;// >> 1;
	n = nn << 1;
	j = 1;
	for(i = 1; i < n; i += 2)
	{
		if( j > i )
		{
			SWAP(data[j-1], data[i-1]);
			SWAP(data[j], data[i]);
		}
		m = nn;
		while (m >= 2 && j > m)
		{
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax = 2;
	theta = iSign * M_PI;
	while(n > mmax)
	{
		istep = mmax << 1;
		wtemp = FastSin(0.5 * theta);
		wpr = -2.0 * wtemp * wtemp;
		wpi = FastSin(theta);
		theta *= 0.5;
		wr = 1.0;
		wi = 0.0;
		for(m = 0; m < mmax-1; m += 2)
		{
			for( i = m; i < n; i += istep)
			{
				j = i + mmax;
				tempr = wr * data[j] - wi * data[j + 1];
				tempi = wr * data[j + 1] + wi * data[j];
				data[j] = data[i] - tempr;
				data[j + 1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i + 1] += tempi;
			}
			wtemp = wr;
			wr += wr * wpr - wi * wpi;
			wi += wi * wpr + wtemp * wpi;
		}
		mmax <<= 1;
	}
	return true;
}

bool Fft2D_Calculate(double *pData, size_t nn, size_t mm, int iSign)
{
	double *data = pData;
	size_t i, j, index1, index2;

	// Error checking section
	i = 1;
	j = nn;
	while (j > 1)
	{
		i = i << 1;
		j = j >> 1;
	}
	if( nn != i )
	{
		return false;
	}
	i = 1;
	j = mm;
	while( j > 1 )
	{
		i = i << 1;
		j = j >> 1;
	}
	if( mm != i )
	{
		return false;
	}
	// Transform by ROWS for forward transform
	try
	{
		if(iSign == 1)
		{
			index1 = 0;
			for (i = 0; i < mm; i++)
			{
				Fft1D_Calculate( data+index1, nn, iSign );
				index1 += (nn << 1);
			}
		}
	}
	catch( ... )
	{
		OutputDebugString("Fft2D_Calculate() -> unhandled exception.\n");
	}

	// Allocate space for temporary array
	std::vector<double> vCopy(mm*2);
	double *copy = &*vCopy.begin();
	// Transform by COLUMNS
	for(j = 0; j < nn; j++)
	{
		// Copy pixels into temp array
		index1 = (j << 1);
		index2 = 0;
		for (i = 0; i < mm; i++)
		{
			copy[index2++] = data[index1];
			copy[index2++] = data[index1 + 1];
			index1 += (nn << 1);
		}

		// Perform transform
		Fft1D_Calculate(copy, mm, iSign);

		// Copy pixels back into data array
		index1 = (j << 1);
		index2 = 0;
		for (i = 0; i < mm; i++)
		{
			data[index1] = copy[index2++];
			data[index1 + 1] = copy[index2++];
			index1 += (nn << 1);
		}
	}

	// Transform by ROWS for inverse transform
	try
	{
		if( iSign == -1 )
		{
			index1 = 0;
			for (i = 0; i < mm; i++)
			{
				Fft1D_Calculate(data+index1, nn, iSign);
				index1 += (nn << 1);
			}
		}
	}
	catch( ... )
	{
		OutputDebugString("Fft2D_Calculate() -> unhandled exception.\n");
	}
	
	if( iSign == 1 )
	{
		double dScale = 1.0 / double(nn*mm);
		for(i = 0; i < nn*mm<<1; i += 2)
		{
			data[i] *= dScale;
			data[i+1] *= dScale;
		}
	}

	return true;
}
