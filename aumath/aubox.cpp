//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubox.cpp,v 1.2 2022/02/19 18:38:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubox.h"
//
#include "auscface.h"

void AuBox::GetFaces(AuScFaceList& faces) const
{
	// Front
	faces.AddFace(AuScFaceQuad(
				AuScPoint(m_fCorners[0], 0,0, AuPoint(0,-1,0)),
				AuScPoint(m_fCorners[4], 0,1, AuPoint(0,-1,0)),
				AuScPoint(m_fCorners[7], 1,1, AuPoint(0,-1,0)),
				AuScPoint(m_fCorners[3], 1,0, AuPoint(0,-1,0))));

	// Left
	faces.AddFace(AuScFaceQuad(
				AuScPoint(m_fCorners[1], 0,0, AuPoint(-1,0,0)),
				AuScPoint(m_fCorners[5], 0,1, AuPoint(-1,0,0)),
				AuScPoint(m_fCorners[4], 1,1, AuPoint(-1,0,0)),
				AuScPoint(m_fCorners[0], 1,0, AuPoint(-1,0,0))));

	// Back
	faces.AddFace(AuScFaceQuad(
				AuScPoint(m_fCorners[2], 0,0, AuPoint(0,1,0)),
				AuScPoint(m_fCorners[6], 0,1, AuPoint(0,1,0)),
				AuScPoint(m_fCorners[5], 1,1, AuPoint(0,1,0)),
				AuScPoint(m_fCorners[1], 1,0, AuPoint(0,1,0))));

	// Right
	faces.AddFace(AuScFaceQuad(
				AuScPoint(m_fCorners[3], 0,0, AuPoint(1,0,0)),
				AuScPoint(m_fCorners[7], 0,1, AuPoint(1,0,0)),
				AuScPoint(m_fCorners[6], 1,1, AuPoint(1,0,0)),
				AuScPoint(m_fCorners[2], 1,0, AuPoint(1,0,0))));

	// Top
	faces.AddFace(AuScFaceQuad(
				AuScPoint(m_fCorners[4], 0,0, AuPoint(0,0,1)),
				AuScPoint(m_fCorners[5], 0,1, AuPoint(0,0,1)),
				AuScPoint(m_fCorners[6], 1,1, AuPoint(0,0,1)),
				AuScPoint(m_fCorners[7], 1,0, AuPoint(0,0,1))));

	// Bottom
	faces.AddFace(AuScFaceQuad(
				AuScPoint(m_fCorners[1], 0,0, AuPoint(0,0,-1)),
				AuScPoint(m_fCorners[0], 0,1, AuPoint(0,0,-1)),
				AuScPoint(m_fCorners[3], 1,1, AuPoint(0,0,-1)),
				AuScPoint(m_fCorners[2], 1,0, AuPoint(0,0,-1))));
}
