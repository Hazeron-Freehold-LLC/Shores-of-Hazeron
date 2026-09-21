//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumatrix4x4.cpp,v 1.7 2022/02/19 18:38:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumatrix4x4.h"
//
#include <QDebug>

AuMatrix4x4 s_Identity;					// Default identity matrix.

QDebug operator<<(QDebug dbg, const AuMatrix4x4& t)
{
	dbg.nospace() << "\n"
		<< QString("/ %1 %2 %3 %4 |")
		.arg(t[0], 16,'f',4)
		.arg(t[4], 16,'f',4)
		.arg(t[8], 16,'f',4)
		.arg(t[12], 16,'f',4);

	dbg.nospace() << "\n"
		<< QString("| %1 %2 %3 %4 |")
		.arg(t[1], 16,'f',4)
		.arg(t[5], 16,'f',4)
		.arg(t[9], 16,'f',4)
		.arg(t[13], 16,'f',4);

	dbg.nospace() << "\n"
		<< QString("| %1 %2 %3 %4 |")
		.arg(t[2], 16,'f',4)
		.arg(t[6], 16,'f',4)
		.arg(t[10], 16,'f',4)
		.arg(t[14], 16,'f',4);

	dbg.nospace() << "\n |"
		<< QString("%1 %2 %3 %4 /")
		.arg(t[3], 16,'f',4)
		.arg(t[7], 16,'f',4)
		.arg(t[11], 16,'f',4)
		.arg(t[15], 16,'f',4);

	return dbg.space();
}

//static
const AuMatrix4x4& AuMatrix4x4::IdentityMatrix()
{
	return s_Identity;
}

// Applies a 3D rotation to this matrix to align with the plane defined by 'xvec' and 'yvec', which are expected to be unit
// vectors though 'yvec' need not be perpendicular to 'xvec'.
void AuMatrix4x4::RotateToPlane(const AuPoint& xvec, const AuPoint& yvec)
{
	// Determine plane normal.
	AuPoint n = xvec.Cross(yvec);
	n.Unit();

	// Construct matrix that will align the normal with the one desired.
	AuMatrix4x4 m(AuPoint(0,0,1).GetAlignmentTransform(n));

	// Rotate matrix on it's Z axis to align with xvec.
	AuPoint xv = m.UnProject(xvec);		// Calculate xv relative to the plane m.
	m.RotateZ(xv.ArcTangent_2D());		// Rotate around Z axis to align xvec.

	// Multiply by current matrix to combine result.
	*this *= m;
}

void AuMatrix4x4::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(IsIdentity())
		out << hasWhat;					// 0 indicates identity matrix.
	else
	{
		// Some values are often zeros.
		//		| n0  n4  n8  n12 |
		//		| n1  n5  n9  n13 |
		//		| n2  n6  n10 n14 |
		//		| n3  n7  n11 n15 |
		if(C[3])
			hasWhat |= 0x01;
		if(C[7])
			hasWhat |= 0x02;
		if(C[11])
			hasWhat |= 0x04;
		if(C[12])
			hasWhat |= 0x08;
		if(C[13])
			hasWhat |= 0x10;
		if(C[14])
			hasWhat |= 0x20;
		if(C[15])
			hasWhat |= 0x40;

		out << hasWhat;

		out << C[0];
		out << C[1];
		out << C[2];
		if(hasWhat & 0x01)
			out << C[3];
		out << C[4];
		out << C[5];
		out << C[6];
		if(hasWhat & 0x02)
			out << C[7];
		out << C[8];
		out << C[9];
		out << C[10];
		if(hasWhat & 0x04)
			out << C[11];
		if(hasWhat & 0x08)
			out << C[12];
		if(hasWhat & 0x10)
			out << C[13];
		if(hasWhat & 0x20)
			out << C[14];
		if(hasWhat & 0x40)
			out << C[15];
	}
}

void AuMatrix4x4::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;
	if(!hasWhat)
		LoadIdentity();					// received identity matrix.
	else
	{
		in >> C[0];
		in >> C[1];
		in >> C[2];
		if(hasWhat & 0x01)
			in >> C[3];
		else
			C[3] = 0;
		in >> C[4];
		in >> C[5];
		in >> C[6];
		if(hasWhat & 0x02)
			in >> C[7];
		else
			C[7] = 0;
		in >> C[8];
		in >> C[9];
		in >> C[10];
		if(hasWhat & 0x04)
			in >> C[11];
		else
			C[11] = 0;
		if(hasWhat & 0x08)
			in >> C[12];
		else
			C[12] = 0;
		if(hasWhat & 0x10)
			in >> C[13];
		else
			C[13] = 0;
		if(hasWhat & 0x20)
			in >> C[14];
		else
			C[14] = 0;
		if(hasWhat & 0x40)
			in >> C[15];
		else
			C[15] = 0;
	}
}
