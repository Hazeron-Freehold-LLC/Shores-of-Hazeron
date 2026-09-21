//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumatrix4x4.h,v 1.20 2022/02/19 18:38:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMatrix4x4_H
#define AuMatrix4x4_H

#include "aupoint.h"
#include <QMatrix4x4>
#include <string.h>

class QDebug;

//  Summary:
//		Transformation matrix as defined in OpenGL.
//
//  Description:
//		- Default constructor initializes to identity matrix.
//
//		- All angle values are in radians.
//
//		- Matrix order follows that defined by OpenGL.
//			Note that this is not the same result as C[4][4].
//			| n0  n4  n8  n12 |
//			| n1  n5  n9  n13 |
//			| n2  n6  n10 n14 |
//			| n3  n7  n11 n15 |
//
//		- Identity Matrix
//			| 1  0  0  0 |
//			| 0  1  0  0 |
//			| 0  0  1  0 |
//			| 0  0  0  1 |
//
//		- Zero Matrix
//			| 0  0  0  0 |
//			| 0  0  0  0 |
//			| 0  0  0  0 |
//			| 0  0  0  0 |
//
// References:
//	- "OpenGL Programming Guide" Second Edition, Woo, Neider, Davis, Pub. Addison Wesley
//
//	- "OpenGL Reference Manual" Second Edition, Kempf, Frazier, Pub. Addison Wesley

class AUMATH_DLL AuMatrix4x4
{
public:
	enum ISOMETRIC_QUADRANT
	{
		ISOQUAD_NE,
		ISOQUAD_NW,
		ISOQUAD_SW,
		ISOQUAD_SE
	};

private:
	double			C[16];		// Transformation matrix (in column order, see above).
	mutable double	I[16];		// Inverse transformation matrix. Calculated when needed.
	mutable bool	Iok;		// true when the inverse matrix has been calculated.

public:	// Constructors
	//  Summary:
	//      Default contructor.
	//  Description:
	//		Default constructor loads identity.
	//  See Also:
	//      AuMatrix4x4::LoadIdentity
	inline AuMatrix4x4()
	{
		LoadIdentity();
	}

	//  Summary:
	//		Construct from array.
	//  Parameters:
	//		M -	Array of values to assign to this matrix.
	//  See Also:
	//      AuMatrix4x4::LoadArray
	inline AuMatrix4x4(const double M[16])
	{
		LoadArray(M);
	}
	
	//  Summary:
	//		Construct from 4x4 float matrix.
	//  Parameters:
	//		Mf -	4x4 matrix of float values.
	inline AuMatrix4x4(const float Mf[4][4])
	{
		for(int ii=0; ii<4; ii++)
			for(int ij=0; ij<4; ij++)
				C[ii*4 + ij] = Mf[ii][ij];
		Iok = false;
	}
	
	//  Summary:
	//		Copy constructor.
	//  Parameters:
	//		M -	The source transform forthe constructor.
	inline AuMatrix4x4(const AuMatrix4x4& M)
	{
		memcpy(C, M.C, sizeof(C));
		Iok = M.Iok;
		if(Iok)
			memcpy(I, M.I, sizeof(I));
	}

	inline AuMatrix4x4(const AuPoint& origin, const AuPoint& xvec, const AuPoint& yvec)
	{
		LoadIdentity();
		Translate(origin);
		RotateToPlane(xvec, yvec);
	}

public:	// Operators
	//  Summary:
	//      Equality test.
	//  Parameters:
	//		M -	The transform to compare against this transform.
	//  Return Value:
	//      Returns true if the transforms are exactly equal.
	inline bool operator==(const AuMatrix4x4& M) const
	{
		return !memcmp(C, M.C, sizeof(C));
	}
	
	//  Summary:
	//      Inequality test.
	//  Parameters:
	//		M -	The transform to compare against this transform.
	//  Return Value:
	//      Returns true ifthe transforms are not exactly equal.
	inline bool operator!=(const AuMatrix4x4 &M) const
	{
		return !operator==(M);
	}
	
	//  Summary:
	//      Assignment.
	//  Parameters:
	//		M -	The transform values to assign to this transform.
	//  Return Value:
	//      A reference to the result of the assignment.
	//  Description:
	//      Copies the contents of transform 'M' to this transform.
	inline AuMatrix4x4& operator=(const AuMatrix4x4 &M)
	{
		memcpy(C, M.C, sizeof(C));
		Iok = M.Iok;
		if(Iok)
			memcpy(I, M.I, sizeof(I));
		return(*this);
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	//  Summary:
	//      Multiplication.
	//  Parameters:
	//		M - The transform to multiply by this transform.
	//  Return Value:
	//      Returns the transform that is the result of the multiplication.
	//  Description:
	//      Multiplies this transform by transform M, in the order
	//		Result = this * M.
	inline AuMatrix4x4 operator*(const AuMatrix4x4& M) const
	{
		// Construct matrix for return value.
		// No need to init because we'll assign every value.
		AuMatrix4x4 mT;

		// Multiply rows by columns.
		// Matrix order follows that defined by OpenGL.
		// Note that this is not the same result as C[4][4].
		// | n0  n4  n8  n12 |
		// | n1  n5  n9  n13 |
		// | n2  n6  n10 n14 |
		// | n3  n7  n11 n15 |
		/*
		// First row of result.
		mT[0]  = C[0]*M[0]  + C[4]*M[1]  + C[8] *M[2]  + C[12]*M[3];
		mT[4]  = C[0]*M[4]  + C[4]*M[5]  + C[8] *M[6]  + C[12]*M[7];
		mT[8]  = C[0]*M[8]  + C[4]*M[9]  + C[8] *M[10] + C[12]*M[11];
		mT[12] = C[0]*M[12] + C[4]*M[13] + C[8] *M[14] + C[12]*M[15];

		// Second row of result.
		mT[1]  = C[1]*M[0]  + C[5]*M[1]  + C[9] *M[2]  + C[13]*M[3];
		mT[5]  = C[1]*M[4]  + C[5]*M[5]  + C[9] *M[6]  + C[13]*M[7];
		mT[9]  = C[1]*M[8]  + C[5]*M[9]  + C[9] *M[10] + C[13]*M[11];
		mT[13] = C[1]*M[12] + C[5]*M[13] + C[9] *M[14] + C[13]*M[15];

		// Third row of result.
		mT[2]  = C[2]*M[0]  + C[6]*M[1]  + C[10]*M[2]  + C[14]*M[3];
		mT[6]  = C[2]*M[4]  + C[6]*M[5]  + C[10]*M[6]  + C[14]*M[7];
		mT[10] = C[2]*M[8]  + C[6]*M[9]  + C[10]*M[10] + C[14]*M[11];
		mT[14] = C[2]*M[12] + C[6]*M[13] + C[10]*M[14] + C[14]*M[15];

		// Fourth row of result.
		mT[3]  = C[3]*M[0]  + C[7]*M[1]  + C[11]*M[2]  + C[15]*M[3];
		mT[7]  = C[3]*M[4]  + C[7]*M[5]  + C[11]*M[6]  + C[15]*M[7];
		mT[11] = C[3]*M[8]  + C[7]*M[9]  + C[11]*M[10] + C[15]*M[11];
		mT[15] = C[3]*M[12] + C[7]*M[13] + C[11]*M[14] + C[15]*M[15];
		*/

		// Precalculate pointers for matrix entries.
		const double* C0 = C;
		const double* C1 = C+1;
		const double* C2 = C+2;
		const double* C3 = C+3;
		const double* C4 = C+4;
		const double* C5 = C+5;
		const double* C6 = C+6;
		const double* C7 = C+7;
		const double* C8 = C+8;
		const double* C9 = C+9;
		const double* C10 = C+10;
		const double* C11 = C+11;
		const double* C12 = C+12;
		const double* C13 = C+13;
		const double* C14 = C+14;
		const double* C15 = C+15;
		const double* M0 = M.C;
		const double* M1 = M.C+1;
		const double* M2 = M.C+2;
		const double* M3 = M.C+3;
		const double* M4 = M.C+4;
		const double* M5 = M.C+5;
		const double* M6 = M.C+6;
		const double* M7 = M.C+7;
		const double* M8 = M.C+8;
		const double* M9 = M.C+9;
		const double* M10 = M.C+10;
		const double* M11 = M.C+11;
		const double* M12 = M.C+12;
		const double* M13 = M.C+13;
		const double* M14 = M.C+14;
		const double* M15 = M.C+15;

		double* pT = mT.C;

		// First row of result.
		*pT  = *C0**M0  + *C4**M1  + *C8 **M2  + *C12**M3;
		*(pT+4)  = *C0**M4  + *C4**M5  + *C8 **M6  + *C12**M7;
		*(pT+8)  = *C0**M8  + *C4**M9  + *C8 **M10 + *C12**M11;
		*(pT+12) = *C0**M12 + *C4**M13 + *C8 **M14 + *C12**M15;

		// Second row of result.
		*(pT+1)  = *C1**M0  + *C5**M1  + *C9 **M2  + *C13**M3;
		*(pT+5)  = *C1**M4  + *C5**M5  + *C9 **M6  + *C13**M7;
		*(pT+9)  = *C1**M8  + *C5**M9  + *C9 **M10 + *C13**M11;
		*(pT+13) = *C1**M12 + *C5**M13 + *C9 **M14 + *C13**M15;

		// Third row of result.
		*(pT+2)  = *C2**M0  + *C6**M1  + *C10**M2  + *C14**M3;
		*(pT+6)  = *C2**M4  + *C6**M5  + *C10**M6  + *C14**M7;
		*(pT+10) = *C2**M8  + *C6**M9  + *C10**M10 + *C14**M11;
		*(pT+14) = *C2**M12 + *C6**M13 + *C10**M14 + *C14**M15;

		// Fourth row of result.
		*(pT+3)  = *C3**M0  + *C7**M1  + *C11**M2  + *C15**M3;
		*(pT+7)  = *C3**M4  + *C7**M5  + *C11**M6  + *C15**M7;
		*(pT+11) = *C3**M8  + *C7**M9  + *C11**M10 + *C15**M11;
		*(pT+15) = *C3**M12 + *C7**M13 + *C11**M14 + *C15**M15;

		/*
		// Calculated in column order to optimize destination pointer usage.
		double* pT = mT.C;
		*pT++ = *C0**M0  + *C4**M1  + *C8 **M2  + *C12**M3;
		*pT++ = *C1**M0  + *C5**M1  + *C9 **M2  + *C13**M3;
		*pT++ = *C2**M0  + *C6**M1  + *C10**M2  + *C14**M3;
		*pT++ = *C3**M0  + *C7**M1  + *C11**M2  + *C15**M3;
		*pT++ = *C0**M4  + *C4**M5  + *C8 **M6  + *C12**M7;
		*pT++ = *C1**M4  + *C5**M5  + *C9 **M6  + *C13**M7;
		*pT++ = *C2**M4  + *C6**M5  + *C10**M6  + *C14**M7;
		*pT++ = *C3**M4  + *C7**M5  + *C11**M6  + *C15**M7;
		*pT++ = *C0**M8  + *C4**M9  + *C8 **M10 + *C12**M11;
		*pT++ = *C1**M8  + *C5**M9  + *C9 **M10 + *C13**M11;
		*pT++ = *C2**M8  + *C6**M9  + *C10**M10 + *C14**M11;
		*pT++ = *C3**M8  + *C7**M9  + *C11**M10 + *C15**M11;
		*pT++ = *C0**M12 + *C4**M13 + *C8 **M14 + *C12**M15;
		*pT++ = *C1**M12 + *C5**M13 + *C9 **M14 + *C13**M15;
		*pT++ = *C2**M12 + *C6**M13 + *C10**M14 + *C14**M15;
		*pT++ = *C3**M12 + *C7**M13 + *C11**M14 + *C15**M15;
		*/
		return mT;
	}

	inline AuMatrix4x4 operator/(const AuMatrix4x4& M) const
	{
		return *this * M.Inverse();
	}

	inline AuPoint operator*(const AuPoint& P) const
	{
		return Project(P);
	}

	inline AuPoint operator/(const AuPoint& P) const
	{
		return UnProject(P);
	}

	//  Summary:
	//      Multiplication/assignment.
	//  Parameters:
	//		M - The transform to multiply by this transform.
	//  Return Value:
	//      Returns the a reference to this transform.
	//  Description:
	//      Multiplies this transform by transform M, in the order this = this   M.
	inline AuMatrix4x4& operator*=(const AuMatrix4x4& M)
	{
		 *this = *this * M;
		 return *this;
	}
	
	inline AuMatrix4x4& operator/=(const AuMatrix4x4& M)
	{
		 *this = *this * M.Inverse();
		 return *this;
	}
	
	//  Summary:
	//		One-dimensional matrix access (read only).
	//  Parameters:
	//		iEntry -	The index of the matrix value to return. Must be in the range 0..15.
	//  Return Value:
	//      Returns the matrix value at the requested index.
	inline double operator[](int iEntry) const
	{
		return *(C + iEntry);
	}
	
	//  Summary:
	//		One-dimensional matrix access.
	//  Parameters:
	//		iEntry -	The index of the matrix value to access. Must be in the range 0..15.
	//  Return Value:
	//      Returns the matrix value at the requested index.
	inline double& operator[](int iEntry)
	{
		return *(C + iEntry);
	}

public:	// Methods
	inline const double* GLMatrix() const
	{
		return C;
	}

	// Returns a reference to a static identity matrix.
	static const AuMatrix4x4& IdentityMatrix();

	//  Summary:
	//		Loads this matrix with the identity matrix.
	inline void LoadIdentity()
	{
		LoadZero();
		*C = 1.;
		*(C+5) = 1.;
		*(C+10) = 1.;
		*(C+15) = 1.;
	}
	
	//  Summary:
	//		Loads this matrix with the zero matrix.
	//  Description:
	//      Sets all values in this transform matrix to zero.
	inline void LoadZero()
	{
		memset(C, 0, sizeof(C));
		Iok = false;
	}
	
	//  Summary:
	//      Loads this matrix from an array.
	//  Parameters:
	//		M -	Loads an array of double values into this transform.
	//  Description:
	//      Copies the array into this transformation matrix.
	//
	//		| n0  n4  n8  n12 |
	//		| n1  n5  n9  n13 |
	//		| n2  n6  n10 n14 |
	//		| n3  n7  n11 n15 |
	inline void LoadArray(const double M[16])
	{
		memcpy(C, M, sizeof(C));
		Iok = false;
	}
	
	//  Summary:
	//      Gets the inverse of this transformation matrix.
	//  Return Value:
	//      Returns the inverse of this transform.
	inline AuMatrix4x4 Inverse() const
	{
		if(!Iok)
		{
			/*
			//qDebug() << "calc Qt inverse";
			// This used to work great but as of Qt5 the result started getting calculated using floats, which is inadequate.
			// Precalculate pointers for matrix entries.
			const double* C0 = C;
			const double* C1 = C+1;
			const double* C2 = C+2;
			const double* C3 = C+3;
			const double* C4 = C+4;
			const double* C5 = C+5;
			const double* C6 = C+6;
			const double* C7 = C+7;
			const double* C8 = C+8;
			const double* C9 = C+9;
			const double* C10 = C+10;
			const double* C11 = C+11;
			const double* C12 = C+12;
			const double* C13 = C+13;
			const double* C14 = C+14;
			const double* C15 = C+15;
			QMatrix4x4 tI = QMatrix4x4(	*C0, *C4, *C8, *C12,
										*C1, *C5, *C9, *C13,
										*C2, *C6, *C10, *C14,
										*C3, *C7, *C11, *C15).inverted();

			// Move float results to doubles.
			// QMatrix4x4 data is stored in column major order, just like mine. Too bad they use floats instead of doubles. I would
			// switch.
			double* pI = I;
			const float* tIdata = tI.data();
			for(int i=0; i<16; i++, pI++, tIdata++)
				*pI = *tIdata;
			*/

			memcpy(I, C, sizeof(I));
			inv(I);

			Iok = true;
		}
		return AuMatrix4x4(I);
	}

	inline AuPoint Translation() const
	{
		// Grab the translation values from the last column.
		AuPoint fRes;
		fRes.x = *(C+12);
		fRes.y = *(C+13);
		fRes.z = *(C+14);
		double w = *(C+3) + *(C+7) + *(C+11) + *(C+15);

		// Divide out 'w' ordinate of homogeneous vector.
		if(w != 0 && w != 1)
			fRes /= w;
		return fRes;
	}
	
	//  Summary:
	//      Queries the 3D rotation needed to achieve an isometric view
	//		rotation.
	//  Parameters:
	//		eIsoDir -	The isometric viewing quadrant. Can be ISOQUAD_NE,
	//					ISOQUAD_SE, ISOQUAD_NW, or ISOQUAD_SW.
	//  Return Value:
	//      Returns a 3D rotation value that can be used to configure an
	//		isometric view rotation.
	//  Description:
	//      In an orthogonal isometric view, a distance has the same apparent
	//		offset along each of the
	//		X, Y, and Z axes. In other words, a given distance along the X
	//		axis will match the same distance along the Y and Z axes.
	inline static const AuPoint& GetIsometricRotation(const ISOMETRIC_QUADRANT eIsoDir)
	{
		static const AuPoint fIsoSW(-54.72*DEG2RAD, 0.0, 45.0*DEG2RAD);
		static const AuPoint fIsoSE(-54.72*DEG2RAD, 0.0, -45.0*DEG2RAD);
		static const AuPoint fIsoNE(-54.72*DEG2RAD, 0.0, -135.0*DEG2RAD);
		static const AuPoint fIsoNW(-54.72*DEG2RAD, 0.0, 135.0*DEG2RAD);
		static const AuPoint fIsoNone(-54.72*DEG2RAD, 0.0, 135.0*DEG2RAD);

		switch(eIsoDir)
		{
		case ISOQUAD_NE:
			return fIsoNE;
		case ISOQUAD_NW:
			return fIsoNW;
		case ISOQUAD_SW:
			return fIsoSW;
		case ISOQUAD_SE:
			return fIsoSE;
		}
		return fIsoNone;
	}
	
	//  Summary:
	//      Query this transform forthe rotation it applies to vectors projected through it.
	//  Return Value:
	//      Returns the 3D rotation of this transform.
	//  Description:
	//		Returns the rotation that is applied to vectors that are projected through this transform.
	inline AuPoint GetRotation() const
	{
		AuPoint forigin(Translation());
		AuPoint vx(Project(1, 0, 0) - forigin);
		AuPoint vy(Project(0, 1, 0) - forigin);
		return AuPoint::CalculateRotation(vx, vy);
	}
	
	//  Summary:
	//      Query this transform forits scale.
	//  Return Value:
	//      Returns the 3D scale of this transform.
	//  Description:
	//		The return result is only correct ifthere's no rotation. Test with IsTranslateAndScale first.
	//  See Also:
	//		IsScale
	inline AuPoint GetScale() const
	{
		return AuPoint(*C, *(C+5), *(C+10));
	}
	
	//  Summary:
	//      Query to determine if this transform is only a scaling matrix.
	//  Return Value:
	//      true if this transform is only a scaling matrix.
	//  See Also:
	//      GetScale
	inline bool IsScale() const
	{
		return((*C !=0.0 || *(C+5) !=0.0 || *(C+10) !=0.0) && *(C+1) ==0.0 && *(C+2) ==0.0 && *(C+3) ==0.0 &&
				*(C+4) ==0.0 && *(C+6) ==0.0 && *(C+7) ==0.0 &&
				*(C+8) ==0.0 && *(C+9) ==0.0 && *(C+11) ==0.0 &&
				*(C+12) ==0.0 && *(C+13) ==0.0 && *(C+14) ==0.0 && *(C+15) ==1.0);
	}
	
	//  Summary:
	//      Query to determine if this transform represents a "flat" 2D projection.
	//  Return Value:
	//		Returns true ifa view projection represented by this transform is "flat". That is, the world normal and the view normal are parallel.
	//  Description:
	//		A top or bottom view is flat. Any view that is only rotated about the Z axis is also flat.
	inline bool IsFlat() const
	{
		AuPoint fNormal = ProjectVector(0, 0, 1);
		return(fNormal.IsZero_2D());
	}
	
	//  Summary:
	//      Query to determine if this transform is an identity matrix.
	//  Return Value:
	//      Returns true if this transform is an identity matrix.
	inline bool IsIdentity() const
	{
		return(*C==1.0 && *(C+1)==0.0 && *(C+2)==0.0 && *(C+3)==0.0 &&
				*(C+4)==0.0 && *(C+5)==1.0 && *(C+6)==0.0 && *(C+7)==0.0 &&
				*(C+8)==0.0 && *(C+9)==0.0 && *(C+10)==1.0 && *(C+11)==0.0 &&
				*(C+12)==0.0 && *(C+13)==0.0 && *(C+14)==0.0 && *(C+15)==1.0);
	}
	
	//  Summary:
	//      Query to determine if this transform is ONLY a translation matrix.
	//  Return Value:
	//		Returns true if this "looks" like a translation matrix only.
	//		Returns false if this transform appears to have rotation or scale.
	inline bool IsTranslation() const
	{
		return(FuzzyEquals(*C,1.0) &&
				IsFuzzyZero(*(C+1)) &&
				IsFuzzyZero(*(C+2)) &&
				IsFuzzyZero(*(C+3)) &&
				IsFuzzyZero(*(C+4)) &&
				FuzzyEquals(*(C+5),1.0) &&
				IsFuzzyZero(*(C+6)) &&
				IsFuzzyZero(*(C+7)) &&
				IsFuzzyZero(*(C+8)) &&
				IsFuzzyZero(*(C+9)) &&
				FuzzyEquals(*(C+10),1.0) &&
				IsFuzzyZero(*(C+11)) &&
				(*(C+12) != 0.0 || *(C+13) != 0.0 || *(C+14) != 0.0) &&
				FuzzyEquals(*(C+15),1.0));
	}
	
	//  Summary:
	//      Query to determine if this transform is ONLY a translation and
	//		scale matrix.
	//  Parameters:
	//		cons -
	//  Return Value:
	//		Returns true if this "looks" like a translation and scaling matrix
	//		only.
	//      Returns false if this transform appears to have rotation.
	inline bool IsTranslateAndScale() const
	{
		if (IsIdentity())
			return(false);

		return(!IsFuzzyZero(*C) &&
				IsFuzzyZero(*(C+1)) &&
				IsFuzzyZero(*(C+2)) &&
				IsFuzzyZero(*(C+3)) &&
				IsFuzzyZero(*(C+4)) &&
				FuzzyEquals(*(C+5),*C) &&
				IsFuzzyZero(*(C+6)) &&
				IsFuzzyZero(*(C+7)) &&
				IsFuzzyZero(*(C+8)) &&
				IsFuzzyZero(*(C+9)) &&
				FuzzyEquals(*(C+10),*C) &&
				IsFuzzyZero(*(C+11)) &&
				/*(*(C+12) != 0.0 || *(C+13) != 0.0 || *(C+14) != 0.0) && */
				FuzzyEquals(*(C+15),1.0));
	}

	inline bool IsZero() const
	{
		return(*C == 0 &&
				*(C+1) == 0 &&
				*(C+2) == 0 &&
				*(C+3) == 0 &&
				*(C+4) == 0 &&
				*(C+5) == 0 &&
				*(C+6) == 0 &&
				*(C+7) == 0 &&
				*(C+8) == 0 &&
				*(C+9) == 0 &&
				*(C+10) == 0 &&
				*(C+11) == 0 &&
				*(C+12) == 0 &&
				*(C+13) == 0 &&
				*(C+14) == 0 &&
				*(C+15) == 0);
	}

	//  Summary:
	//      Configures this matrix to be a perspective projection.
	//  Parameters:
	//		l -	Left
	//		r -	Right
	//		b -	Bottom
	//		t -	Top
	//		n -	Near Clip
	//		f -	Far Clip
	//  See Also:
	//      AuMatrix4x4::Perspective
	inline void Frustum(double l, double r, double b, double t, double n, double f)
	{
		// Some combinations of values are invalid and would result in division by zero.
		if(l == r)
			return;
		if(t == b)
			return;
		if(n == f)
			return;

		// Pre-calc a few values that get used more than once.
		double rl = r - l;
		double tb = t - b;
		double fn = f - n;
		double n2 = n * 2.0;

		// Start with a zero matrix.
		AuMatrix4x4 mR;
		mR.LoadZero();

		// Set a few values to create perspective matrix.
		*mR.C = n2/rl;
		*(mR.C+5) = n2/tb;
		*(mR.C+8) = (r+l)/rl;
		*(mR.C+9) = (t+b)/tb;
		*(mR.C+10) = -(f+n)/fn;
		*(mR.C+11) = -1.0;
		*(mR.C+14) = -f*n2/fn;

		// Multiply by current matrix to combine result.
		*this *= mR;
	}
	
	//  Summary:
	//      Configures this matrix to be a perspective projection.
	//  Parameters:
	//		eyex -	X location of the eye.
	//		eyey -	Y location of the eye.
	//		eyez -	Z location of the eye.
	//		centerx -	X location at the center of the view.
	//		centery -	Y location at the center of the view.
	//		centerz -	Z location at the center of the view.
	//		upx -	Up vector X value.
	//		upy -	Up vector Y value.
	//		upz -	Up vector Z value.
	//	Description:
	//		This function is not currently implemented.
	//  See Also:
	//      AuMatrix4x4::Perspective
	//void LookAt(double eyex, double eyey, double eyez,
			//double centerx, double centery, double centerz,
			//double upx, double upy, double upz)
	//{
		// See OpenGL Reference Manual p. 405.
	//}
	
	//  Summary:
	//      Configures this matrix to be a perspective projection.
	//  Parameters:
	//		fEye -	Eye point location.
	//		fCenter -	Center of view point location.
	//		fUp -	Up vector.
	//	Description:
	//		This function is not currently implemented.
	//  See Also:
	//      AuMatrix4x4::Perspective
	//void LookAt(const AuPoint &fEye, const AuPoint &fCenter, const AuPoint &fUp)
	//{
		//LookAt(fEye.x, fEye.y, fEye.z, fCenter.x, fCenter.y, fCenter.z, fUp.x, fUp.y, fUp.z);
	//}
	
	//  Summary:
	//      Configures this matrix to be an orthographic projection.
	//  Parameters:
	//		l -	Left
	//		r -	Right
	//		b -	Bottom
	//		t -	Top
	//		n -	Near Clip
	//		f -	Far Clip
	//  Description:
	//      In an orthographic projection, lines parallel in the world space
	//		are parallel in the view.
	//  See Also:
	//      AuMatrix4x4::Perspective
	inline void Ortho(double l, double r, double b, double t, double n, double f)
	{
		// Some combinations of values are invalid and would result in division by zero.
		if(l == r)
			return;
		if(t == b)
			return;
		if(n == f)
			return;

		// Pre-calc a few values that get used more than once.
		double rl = r - l;
		double tb = t - b;
		double fn = f - n;

		// Construct identity matrix.
		AuMatrix4x4 mR;

		// Set a few values to create orthographic matrix.
		*mR.C = 2.0/rl;
		*(mR.C+5) = 2.0/tb;
		*(mR.C+10) = -2.0/fn;
		*(mR.C+12) = (r+l)/rl;
		*(mR.C+13) = (t+b)/tb;
		*(mR.C+14) = (f+n)/fn;

		// Multiply by current matrix to combine result.
		*this *= mR;
	}
	
	//  Summary:
	//      Configures this matrix to be a perspective projection.
	//  Parameters:
	//		fovy - The field of view angle, in radians, in the y-direction. 
	//		aspect - The aspect ratio that determines the field of view in the x-direction. The aspect ratio is the ratio of x
	//		      (width) to y (height). 
	//		zNear - The distance from the viewer to the near clipping plane (always positive). 
	//		zFar - The distance from the viewer to the far clipping plane (always positive). 
	//  Description:
	//      In a perspective projection, parallel lines converge as the distance from the eye increases.
	//  See Also:
	//      AuMatrix4x4::Ortho
	inline void Perspective(double fovy, double aspect, double zNear, double zFar)
	{
		// Some combinations of values are invalid and would result in division by zero.
		if (FuzzyEquals(zFar, zNear))
			return;
		double s2 = sin(fovy/2.0);
		if (IsFuzzyZero(fabs(s2)))
			return;
		double c2 = cos(fovy/2.0);
		double cs2 = c2/s2;

		double w = aspect * cs2;
		double h = cs2;
		double Q = zFar / (zNear - zFar);

		// Construct zero matrix.
		AuMatrix4x4 mR;
		mR.LoadZero();

		// Set a few values to create perspective matrix.
		*mR.C = w;
		*(mR.C+5) = h;
		*(mR.C+10) = Q;
		*(mR.C+11) = -1.0f;
		*(mR.C+14) = Q*zNear;

		// Multiply by current matrix to combine result.
		*this *= mR;
	}

	// Returns zNear from the Perspective call.
	inline double PerspectiveZNear() const
	{
		if(C[10] == 0)
			return 0;
		return C[14] / C[10];
	}
 
	// Thanks to Vrej Melkonian at http://www.geocities.com/vmelkon/zprecision.html for this solution to the
	// perspective Z-precision problem. His solution is to replace the third row of the perspective matrix with the third row
	// of the orthographic projection matrix.
	inline void PerspectiveOrthoZ(double fovy, double aspect, double zNear, double zFar)
	{
		// Some combinations of values are invalid and would result in division by zero.
		if (FuzzyEquals(zFar, zNear))
			return;
		double s2 = sin(fovy/2.0);
		if (IsFuzzyZero(fabs(s2)))
			return;
		double c2 = cos(fovy/2.0);
		double cs2 = c2/s2;

		double w = aspect * cs2;
		double h = cs2;
		double Q0 = -2. / (zFar - zNear);
		double Q1 = (-zFar - zNear) / (zFar - zNear);

		// Construct zero matrix.
		AuMatrix4x4 mR;
		mR.LoadZero();

		// Set a few values to create perspective matrix.
		*mR.C = w;
		*(mR.C+5) = h;
		*(mR.C+10) = Q0;
		*(mR.C+11) = -1.0f;
		*(mR.C+14) = Q1;

		// Multiply by current matrix to combine result.
		*this *= mR;
	}
 
	//  Summary:
	//		Project a point through this matrix.
	//  Parameters:
	//		x -	X offset of the point to project.
	//		y -	Y offset of the point to project.
	//		z -	Z offset of the point to project.
	//  Return Value:
	//      The projected point.
	//  Description:
	//		The rows of this transformation matrix are multiplied by the single
	//		column vector matrix.
	inline AuPoint Project(double x, double y, double z) const
	{
		AuPoint fRes;

		// Multiply rows of transformation matrix by single column of vector matrix.
		//fRes.x = *C*x + *(C+4)*y + *(C+8) *z + *(C+12);
		//fRes.y = *(C+1)*x + *(C+5)*y + *(C+9) *z + *(C+13);
		//fRes.z = *(C+2)*x + *(C+6)*y + *(C+10)*z + *(C+14);
		//double w = *(C+3)*x + *(C+7)*y + *(C+11)*z + *(C+15);
		double w;
		if(x == 0 && y == 0 && z == 0)
		{
			// This is the simplest case and one of the most common cases.
			fRes.x = *(C+12);
			fRes.y = *(C+13);
			fRes.z = *(C+14);
			w = *(C+3) + *(C+7) + *(C+11) + *(C+15);
		}
		else
		{
			// We're not projecting (0,0,0). Perhaps we can still save some math.
			double i0,i1,i2,i3;		// Intermediate results that result from multiply by x.
			if(x == 0)
			{
				i0 = 0;
				i1 = 0;
				i2 = 0;
				i3 = 0;
			}
			else
			{
				i0 = *C*x;
				i1 = *(C+1)*x;
				i2 = *(C+2)*x;
				i3 = *(C+3)*x;
			}

			double j0,j1,j2,j3;		// Intermediate results that result from multiply by y.
			if(y == 0)
			{
				j0 = 0;
				j1 = 0;
				j2 = 0;
				j3 = 0;
			}
			else
			{
				j0 = *(C+4)*y;
				j1 = *(C+5)*y;
				j2 = *(C+6)*y;
				j3 = *(C+7)*y;
			}

			double k0,k1,k2,k3;		// Intermediate results that result from multiply by z.
			if(z == 0)
			{
				k0 = 0;
				k1 = 0;
				k2 = 0;
				k3 = 0;
			}
			else
			{
				k0 = *(C+8)*z;
				k1 = *(C+9)*z;
				k2 = *(C+10)*z;
				k3 = *(C+11)*z;
			}

			fRes.x = i0 + j0 + k0 + *(C+12);
			fRes.y = i1 + j1 + k1 + *(C+13);
			fRes.z = i2 + j2 + k2 + *(C+14);
			w = i3 + j3 + k3 + *(C+15);
		}

		// Divide out 'w' ordinate of homogeneous vector.
		// If it's zero, the point is at infinity along the vector...not really
		// sure what to do about that.
		if(w != 0 && w != 1)
			fRes /= w;

		return fRes;
	}
	
	//  Summary:
	//		Project a point through this matrix.
	//  Parameters:
	//		fLoc -	The 3D point to project.
	//  Return Value:
	//      The projected point.
	//  Description:
	//		The rows of this transformation matrix are multiplied by the
	//		single column vector matrix.
	inline AuPoint Project(const AuPoint &fLoc) const
	{
		return Project(fLoc.x, fLoc.y, fLoc.z);
	}
	
	//  Summary:
	//      Projects a point through the inverse of this matrix.
	//  Parameters:
	//		x -	X offset of the point to project.
	//		y -	Y offset of the point to project.
	//		z -	Z offset of the point to project.
	//  Return Value:
	//      The projected point.
	//  Description:
	//		The rows of the inverse of this transformation matrix are
	//		multiplied by the single column vector matrix.
	inline AuPoint UnProject(double x, double y, double z) const
	{
		return Inverse().Project(x,y,z);
	}
	
	//  Summary:
	//		Project a point through the inverse of this matrix.
	//  Parameters:
	//		fLoc -	The 3D point to project.
	//  Return Value:
	//      The projected point.
	//  Description:
	//		The rows of the inverse of this transformation matrix are
	//		multiplied by the single column vector matrix.
	inline AuPoint UnProject(const AuPoint &fLoc) const
	{
		return UnProject(fLoc.x, fLoc.y, fLoc.z);
	}
	
	//  Summary:
	//      Projects a 3D pixel point through the inverse of this
	//		transformation matrix.
	//  Parameters:
	//		fResult -	The resulting point in the plane.
	//		p2dPoint -	The 2D point to project.
	//		tPlane -	The plane into which the point is projected.
	//  Return Value:
	//		Returns true ifthe point is converted successfully.
	//      Returns false ifthe 2D point does not intersect the plane.
	//  Description:
	//		Unprojects a 2D pixel point through 'this' view transformation.
	//		The result point is calculated such that it intersects a given
	//		plane transform.
	//		Sets the world space location of fResult where it intersects the XY
	//		plane of the tPlane transform.
	inline bool UnProjectIntoPlane(AuPoint &fResult, const AuPoint &p2dPoint, const AuMatrix4x4 &tPlane) const
	{
		const AuMatrix4x4 J = Inverse();			// The inverse of 'this' camera/device transformation.
		const AuMatrix4x4 W = tPlane.Inverse();	// The inverse of the work plane transformation.

		// The denominator so we can prevent division by zero.
		double d = (W[2]*J[8] + W[6]*J[9] + W[10]*J[10] + W[14]*J[11]);
		if(IsFuzzyZero(d))
			return(false);

		AuPoint p(p2dPoint);
		p.z = (-W[14]*(J[3]*p.x + J[7]*p.y + J[15]) -
				W[2]*(J[0]*p.x + J[4]*p.y + J[12]) -
				W[6]*(J[1]*p.x + J[5]*p.y + J[13]) -
				W[10]*(J[2]*p.x + J[6]*p.y + J[14]))/d;	
		fResult = UnProject(p);

		return true;
	}

	//  Summary:
	//		Project a vector through this matrix.
	//  Parameters:
	//		fVec -	The vector to project.
	//  Return Value:
	//      Returns the projected vector.
	//  Description:
	//      The result is calculated by subtracting the projected vector from the projected origin,
	//		thus eliminating the effect of translations.
	//  See Also:
	//      AuMatrix4x4::UnProjectVector
	inline AuPoint ProjectVector(const AuPoint &fVec) const
	{
		return ProjectVector(fVec.x, fVec.y, fVec.z);
	}

	inline AuPoint ProjectVector(double x, double y, double z) const
	{
		//return Project(x, y, z) - Translation();
		AuPoint fRes;

		// Multiply rows of transformation matrix by single column of vector matrix.
		//fRes.x = *C*x + *(C+4)*y + *(C+8) *z;
		//fRes.y = *(C+1)*x + *(C+5)*y + *(C+9) *z;
		//fRes.z = *(C+2)*x + *(C+6)*y + *(C+10)*z;
		//double w = *(C+3)*x + *(C+7)*y + *(C+11)*z + *(C+15);
		double w;
		if(x == 0 && y == 0 && z == 0)
		{
			// This is the simplest case and one of the most common cases.
			fRes.x = 0;
			fRes.y = 0;
			fRes.z = 0;
			w = *(C+3) + *(C+7) + *(C+11) + *(C+15);
		}
		else
		{
			// We're not projecting (0,0,0). Perhaps we can still save some math.
			double i0,i1,i2,i3;		// Intermediate results that result from multiply by x.
			if(x == 0)
			{
				i0 = 0;
				i1 = 0;
				i2 = 0;
				i3 = 0;
			}
			else
			{
				i0 = *C*x;
				i1 = *(C+1)*x;
				i2 = *(C+2)*x;
				i3 = *(C+3)*x;
			}

			double j0,j1,j2,j3;		// Intermediate results that result from multiply by y.
			if(y == 0)
			{
				j0 = 0;
				j1 = 0;
				j2 = 0;
				j3 = 0;
			}
			else
			{
				j0 = *(C+4)*y;
				j1 = *(C+5)*y;
				j2 = *(C+6)*y;
				j3 = *(C+7)*y;
			}

			double k0,k1,k2,k3;		// Intermediate results that result from multiply by z.
			if(z == 0)
			{
				k0 = 0;
				k1 = 0;
				k2 = 0;
				k3 = 0;
			}
			else
			{
				k0 = *(C+8)*z;
				k1 = *(C+9)*z;
				k2 = *(C+10)*z;
				k3 = *(C+11)*z;
			}

			fRes.x = i0 + j0 + k0;
			fRes.y = i1 + j1 + k1;
			fRes.z = i2 + j2 + k2;
			w = i3 + j3 + k3 + *(C+15);
		}

		// Divide out 'w' ordinate of homogeneous vector.
		// If it's zero, the point is at infinity along the vector...not really
		// sure what to do about that.
		if(w != 0 && w != 1)
			fRes /= w;

		return fRes;
	}

	//  Summary:
	//		Project a vector through the inverse of this matrix.
	//  Parameters:
	//		fVec -	The vector to project.
	//  Return Value:
	//      Returns the projected vector.
	//  Description:
	//      The result is calculated by subtracting the projected vector from the projected origin,
	//		thus eliminating the effect of translations.
	//  See Also:
	//      AuMatrix4x4::UnProjectVector
	inline AuPoint UnProjectVector(const AuPoint &fVec) const
	{
		return UnProject(fVec) - UnProject(0, 0, 0);
	}

	//  Summary:
	//      Rotate this matrix about an arbitrary axis.
	//  Parameters:
	//		a -	The angle, in radians.
	//		x -	The rotation axis X value.
	//		y -	The rotation axis Y value.
	//		z -	The rotation axis Z value.
	//  Description:
	//      Rotates this transform about an arbitrary axis.
	inline void RotateAxis(double a, double x, double y, double z)
	{
		// Get trig sin and cos of a.
		//DHH Compared to results of glRotated, this is backward unless I change the sign of 'a'.
		double c = cos(-a);
		double s = sin(-a);

		// Construct matrix with only the values we will be using.
		double mT[16];

		// Precalculate pointers for matrix entries.
		const double* C0 = C;
		const double* C1 = C+1;
		const double* C2 = C+2;
		const double* C3 = C+3;
		const double* C4 = C+4;
		const double* C5 = C+5;
		const double* C6 = C+6;
		const double* C7 = C+7;
		const double* C8 = C+8;
		const double* C9 = C+9;
		const double* C10 = C+10;
		const double* C11 = C+11;
		const double* C12 = C+12;
		const double* C13 = C+13;
		const double* C14 = C+14;
		const double* C15 = C+15;
		double* M0 = mT;
		double* M1 = mT+1;
		double* M2 = mT+2;
		//double* M3 = mT+3;
		double* M4 = mT+4;
		double* M5 = mT+5;
		double* M6 = mT+6;
		//double* M7 = mT+7;
		double* M8 = mT+8;
		double* M9 = mT+9;
		double* M10 = mT+10;
		//double* M11 = mT+11;
		//double* M12 = mT+12;
		//double* M13 = mT+13;
		//double* M14 = mT+14;
		//double* M15 = mT+15;

		// Set a few values to create rotation matrix.
		*M0 = ( x * x ) * ( 1. - c ) + c;
		*M1 = ( x * y ) * ( 1. - c ) - (z * s);
		*M2 = ( x * z ) * ( 1. - c ) + (y * s);

		*M4 = ( y * x ) * ( 1. - c ) + (z * s);
		*M5 = ( y * y ) * ( 1. - c ) + c ;
		*M6 = ( y * z ) * ( 1. - c ) - (x * s);

		*M8 = ( z * x ) * ( 1. - c ) - (y * s);
		*M9 = ( z * y ) * ( 1. - c ) + (x * s);
		*M10 = ( z * z ) * ( 1. - c ) + c;

		// Multiply by current matrix to combine result.
		// This multiply has been trimmed down to the remaining elements, after removing the divide by zeros.
		double res[16];

		// First row of result.
		*res  = *C0**M0  + *C4**M1  + *C8 **M2;
		*(res+4)  = *C0**M4  + *C4**M5  + *C8 **M6;
		*(res+8)  = *C0**M8  + *C4**M9  + *C8 **M10;
		*(res+12) = *C12;

		// Second row of result.
		*(res+1)  = *C1**M0  + *C5**M1  + *C9 **M2;
		*(res+5)  = *C1**M4  + *C5**M5  + *C9 **M6;
		*(res+9)  = *C1**M8  + *C5**M9  + *C9 **M10;
		*(res+13) = *C13;

		// Third row of result.
		*(res+2)  = *C2**M0  + *C6**M1  + *C10**M2;
		*(res+6)  = *C2**M4  + *C6**M5  + *C10**M6;
		*(res+10) = *C2**M8  + *C6**M9  + *C10**M10;
		*(res+14) = *C14;

		// Fourth row of result.
		*(res+3)  = *C3**M0  + *C7**M1  + *C11**M2;
		*(res+7)  = *C3**M4  + *C7**M5  + *C11**M6;
		*(res+11) = *C3**M8  + *C7**M9  + *C11**M10;
		*(res+15) = *C15;

		LoadArray(res);
	}

	//  Summary:
	//      Rotate this matrix about an arbitrary axis.
	//  Parameters:
	//		a -	The angle, in radians.
	//		fRot -	The rotation axis.
	//  Description:
	//      Rotates this transform about an arbitrary axis.
	inline void RotateAxis(double a, const AuPoint &fRot)
	{
		RotateAxis(a, fRot.x, fRot.y, fRot.z);
	}
	
	//  Summary:
	//      Applies a 3D rotation to this matrix.
	//  Parameters:
	//		fRotation -	
	//  Description:
	//      Rotates this matrix about X, Y, then Z axis.
	inline void Rotate(const AuPoint &fRotation)
	{
		RotateX(fRotation.x);
		RotateY(fRotation.y);
		RotateZ(fRotation.z);
	}

	// Applies a 3D rotation to this matrix to align with the plane defined by 'xvec' and 'yvec', which are expected to be unit
	// vectors though 'yvec' need not be perpendicular to 'xvec'.
	void RotateToPlane(const AuPoint& xvec, const AuPoint& yvec);
	
	//  Summary:
	//      Applies an isometric rotation to this transform.
	//  Parameters:
	//		eIsoDir -	The isometric viewing quadrant. Can be ISOQUAD_NE,
	//					ISOQUAD_SE, ISOQUAD_NW, or ISOQUAD_SW.
	inline void RotateIsometric(ISOMETRIC_QUADRANT eIsoDir = ISOQUAD_SW)
	{
		Rotate(GetIsometricRotation(eIsoDir));
	}
	
	//  Summary:
	//      Rotate about the X axis.
	//  Parameters:
	//		a -	The angle, in radians, to rotate about the X axis.
	inline void RotateX(double a)
	{
		// Get trig sin and cos of a.
		double c = cos(a);
		double s = sin(a);

		/*
		// Construct identity matrix.
		AuMatrix4x4 mT;

		// Set a few values to create standard X rotation matrix.
		mT[5] = c;
		mT[6] = s;
		mT[9] = -s;
		mT[10] = c;

		// Multiply by current matrix to combine result.
		*this *= mT;
		*/

		// Precalculate pointers for matrix entries.
		const double* C0 = C;
		const double* C1 = C+1;
		const double* C2 = C+2;
		const double* C3 = C+3;
		const double* C4 = C+4;
		const double* C5 = C+5;
		const double* C6 = C+6;
		const double* C7 = C+7;
		const double* C8 = C+8;
		const double* C9 = C+9;
		const double* C10 = C+10;
		const double* C11 = C+11;
		const double* C12 = C+12;
		const double* C13 = C+13;
		const double* C14 = C+14;
		const double* C15 = C+15;

		// Multiply by current matrix to combine result.
		// This multiply has been trimmed down to the remaining elements, after removing the divide by zeros and simplifying x1's.
		double res[16];

		// First row of result.
		*res  = *C0;
		*(res+4)  = *C4*c  + *C8 *s;
		*(res+8)  = *C4*-s  + *C8 *c;
		*(res+12) = *C12;

		// Second row of result.
		*(res+1)  = *C1;
		*(res+5)  = *C5*c  + *C9 *s;
		*(res+9)  = *C5*-s  + *C9 *c;
		*(res+13) = *C13;

		// Third row of result.
		*(res+2)  = *C2;
		*(res+6)  = *C6*c  + *C10*s;
		*(res+10) = *C6*-s  + *C10*c;
		*(res+14) = *C14;

		// Fourth row of result.
		*(res+3)  = *C3;
		*(res+7)  = *C7*c  + *C11*s;
		*(res+11) = *C7*-s  + *C11*c;
		*(res+15) = *C15;

		LoadArray(res);
	}
	
	//  Summary:
	//      Rotate about the Y axis.
	//  Parameters:
	//		a -	The angle, in radians, to rotate about the Y axis.
	inline void RotateY(double a)
	{
		// Get trig sin and cos of a.
		double c = cos(a);
		double s = sin(a);

		/*
		// Construct identity matrix.
		AuMatrix4x4 mT;

		// Set a few values to create standard Y rotation matrix.
		mT[0] = c;
		mT[2] = -s;
		mT[8] = s;
		mT[10] = c;

		// Multiply by current matrix to combine result.
		*this *= mT;
		*/

		// Precalculate pointers for matrix entries.
		const double* C0 = C;
		const double* C1 = C+1;
		const double* C2 = C+2;
		const double* C3 = C+3;
		const double* C4 = C+4;
		const double* C5 = C+5;
		const double* C6 = C+6;
		const double* C7 = C+7;
		const double* C8 = C+8;
		const double* C9 = C+9;
		const double* C10 = C+10;
		const double* C11 = C+11;
		const double* C12 = C+12;
		const double* C13 = C+13;
		const double* C14 = C+14;
		const double* C15 = C+15;

		// Multiply by current matrix to combine result.
		// This multiply has been trimmed down to the remaining elements, after removing the divide by zeros and simplifying x1's.
		double res[16];

		// First row of result.
		*res  = *C0*c  + *C8 *-s;
		*(res+4)  = *C4;
		*(res+8)  = *C0*s  + *C8 *c;
		*(res+12) = *C12;

		// Second row of result.
		*(res+1)  = *C1*c  + *C9 *-s;
		*(res+5)  = *C5;
		*(res+9)  = *C1*s  + *C9 *c;
		*(res+13) = *C13;

		// Third row of result.
		*(res+2)  = *C2*c  + *C10*-s;
		*(res+6)  = *C6;
		*(res+10) = *C2*s  + *C10*c;
		*(res+14) = *C14;

		// Fourth row of result.
		*(res+3)  = *C3*c  + *C11*-s;
		*(res+7)  = *C7;
		*(res+11) = *C3*s  + *C11*c;
		*(res+15) = *C15;

		LoadArray(res);
	}
	
	//  Summary:
	//      Rotate about the Z axis.
	//  Parameters:
	//		a -	The angle, in radians, to rotate about the Z axis.
	inline void RotateZ(double a)
	{
		// Get trig sin and cos of a.
		double c = cos(a);
		double s = sin(a);

		/*
		// Construct identity matrix.
		AuMatrix4x4 mT;

		// Set a few values to create standard Z rotation matrix.
		mT[0] = c;
		mT[1] = s;
		mT[4] = -s;
		mT[5] = c;

		// Multiply by current matrix to combine result.
		*this *= mT;
		*/

		// Precalculate pointers for matrix entries.
		const double* C0 = C;
		const double* C1 = C+1;
		const double* C2 = C+2;
		const double* C3 = C+3;
		const double* C4 = C+4;
		const double* C5 = C+5;
		const double* C6 = C+6;
		const double* C7 = C+7;
		const double* C8 = C+8;
		const double* C9 = C+9;
		const double* C10 = C+10;
		const double* C11 = C+11;
		const double* C12 = C+12;
		const double* C13 = C+13;
		const double* C14 = C+14;
		const double* C15 = C+15;

		// Multiply by current matrix to combine result.
		// This multiply has been trimmed down to the remaining elements, after removing the divide by zeros and simplifying x1's.
		double res[16];

		// First row of result.
		*res  = *C0*c  + *C4*s;
		*(res+4)  = *C0*-s  + *C4*c;
		*(res+8)  = *C8;
		*(res+12) = *C12;

		// Second row of result.
		*(res+1)  = *C1*c  + *C5*s;
		*(res+5)  = *C1*-s  + *C5*c;
		*(res+9)  = *C9;
		*(res+13) = *C13;

		// Third row of result.
		*(res+2)  = *C2*c  + *C6*s;
		*(res+6)  = *C2*-s  + *C6*c;
		*(res+10) = *C10;
		*(res+14) = *C14;

		// Fourth row of result.
		*(res+3)  = *C3*c  + *C7*s;
		*(res+7)  = *C3*-s  + *C7*c;
		*(res+11) = *C11;
		*(res+15) = *C15;

		LoadArray(res);
	}

	//  Summary:
	//      Rotate about Z, Y, then X axes using the negative of the provided
	//		rotation angles.
	//  Parameters:
	//		fRotation -	3D rotation angles, in radians.
	//  Description:
	//      Rotates about Z, Y, then X axes using the negative of the
	//		provided rotation angles.
	inline void UnRotate(const AuPoint &fRotation)
	{
		RotateZ(-fRotation.z);
		RotateY(-fRotation.y);
		RotateX(-fRotation.x);
	}

	//  Summary:
	//      Scale this matrix.
	//  Parameters:
	//		x -	X scale value.
	//		y -	Y scale value.
	//		z -	Z scale value.
	//  Description:
	//      Applies a 3D scale factor to this transform. Zero values are not
	//		recommended and should be avoided.
	inline void Scale(double x, double y, double z)
	{
		/*
		// Construct identity matrix.
		AuMatrix4x4 mT;

		// Set a few values to create scale matrix.
		mT[0] = x;
		mT[5] = y;
		mT[10] = z;

		// Multiply by current matrix to combine result.
		*this *= mT;
		*/

		// Multiply by current matrix to combine result.
		// This multiply has been trimmed down to the remaining elements, after removing the divide by zeros and simplifying x1's.
		*C *= x;
		*(C+4) *= y;
		*(C+8) *= z;

		*(C+1) *= x;
		*(C+5) *= y;
		*(C+9) *= z;

		*(C+2) *= x;
		*(C+6) *= y;
		*(C+10) *= z;

		*(C+3) *= x;
		*(C+7) *= y;
		*(C+11) *= z;

		Iok = false;
	}
	
	//  Summary:
	//      Scale this matrix.
	//  Parameters:
	//		fScale -	3D scale value.
	//  Description:
	//      Applies a 3D scale factor to this transform. Zero values are not recommended and should be avoided.
	inline void Scale(const AuPoint &fScale)
	{
		Scale(fScale.x, fScale.y, fScale.z);
	}

	//  Summary:
	//      Translate this matrix.
	//  Parameters:
	//		x -	X offset of the translation.
	//		y -	Y offset of the translation.
	//		z -	Z offset of the translation.
	//  Description:
	//      Applies a translation vector to this transform.
	inline void Translate(double x, double y, double z)
	{
		*(C+12) += *C*x + *(C+4)*y + *(C+8) *z;
		*(C+13) += *(C+1)*x + *(C+5)*y + *(C+9) *z;
		*(C+14) += *(C+2)*x + *(C+6)*y + *(C+10)*z;
		*(C+15) += *(C+3)*x + *(C+7)*y + *(C+11)*z;
		Iok = false;
	}

	//  Summary:
	//      Translate this matrix.
	//  Parameters:
	//		fVec -	The translation vector.
	//  Description:
	//      Applies a translation vector to this transform.
	inline void Translate(const AuPoint &fVec)
	{
		Translate(fVec.x, fVec.y, fVec.z);
	}

#if!defined(QT_NO_DEBUG)
	inline void Dump(const char* msg) const
	{
		if(msg)
			qDebug("AuMatrix4x4::Dump - %s", msg);
		else
			qDebug("AuMatrix4x4::Dump");
		qDebug(" / %10.4f %10.4f %10.4f %10.4f \\", C[0], C[4], C[8], C[12]);
		qDebug(" | %10.4f %10.4f %10.4f %10.4f |", C[1], C[5], C[9], C[13]);
		qDebug(" | %10.4f %10.4f %10.4f %10.4f |", C[2], C[6], C[10], C[14]);
		qDebug(" \\ %10.4f %10.4f %10.4f %10.4f /", C[3], C[7], C[11], C[15]);
	}
#endif

private:
	inline double inv(double* a) const	// Invert matrix in place
	{
		const int n = 4;
		const int m = 4;

		int i,j,k,l;
		double p,q,d,*pp,*pr,*pa,*pb;

		int* swap = (int*)calloc(4,sizeof(int));
		for(d=1,pr=pp=a,l=k=0; l<n; pr+=m,pp+=m+1,l++)
		{
			for(j=0,p=fabs(*pp),pa=pp+m,i=l+1; i<n; pa+=m,i++)
			{
				q = fabs(*pa);
				if(q > p)
				{
					p=q;
					j=i;
				}
			}
			if(p == 0.0)
			{
				free(swap);
				return 0.;
			}
			if(j)
			{
				swap[l]=j;
				dswapn(pr,a+m*j,n);
			}
			for(d*=*pp,p=1/(*pp),pa=pr,j=0; j<n; pa++,j++)
			{
				if(j==l)
					*pa = p;
				else
					*pa *= p;
			}
			for(pa=a,i=0; i<n; i++)
			{
				q = pa[l];
				if(i==l || q == 0.0)
					pa+=m;
				else
				{
					for(pb=pr,j=0; j<n; pa++,pb++,j++)
					{
						if(j==l)
							*pa=-q*p;
						else
							if(*pb != 0.0)
								*pa-=*pb*q;
					}
				}
			}
		}
		for(l--; l--; )
		{
			j = swap[l];
			if(j != 0)
				for(k++,pa=a+l,pb=a+j,i=0; i<n; pa+=m,pb+=m,i++)
					dswapn(pa,pb,1);
		}
		free(swap);
		return k&1 ? -d : d;
	}

	inline void dswapn(double *a,double *b,int n) const
	{
		double dswap_temp;
		while(--n>=0)
		{
			dswap_temp = *a;
			*a = *b;
			*b = dswap_temp;
			a++;
			b++;
		}
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuMatrix4x4& m)
{
	m >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuMatrix4x4& m)
{
	m << in;
	return in;
}

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuMatrix4x4& t);

#endif// AuMatrix4x4_H
