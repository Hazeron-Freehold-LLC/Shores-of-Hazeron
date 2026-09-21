//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autriangle.cpp,v 1.2 2022/02/19 18:38:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autriangle.h"
//
#include "auplane.h"

// Adds two or three triangles to cResult after slicing this triangle against the segment from fSeg0 to fSeg1.
// Nothing is added to cResult if this triangle is not cut by the segment.
void AuTriangle::Slice_2D(const AuPoint &fSeg0, const AuPoint &fSeg1, AuTriangleList &cResult) const
{
	AuPoint fSeg2d0(fSeg0.x, fSeg0.y, 0.0);
	AuPoint fSeg2d1(fSeg1.x, fSeg1.y, 0.0);
	if (fSeg2d0.FuzzyEquals(fSeg2d1))
		return;

	// Make 2D triangle locations for segment intersection tests.
	AuPoint fT0(m_fV[0].x, m_fV[0].y, 0.0);
	AuPoint fT1(m_fV[1].x, m_fV[1].y, 0.0);
	AuPoint fT2(m_fV[2].x, m_fV[2].y, 0.0);

	// Segment intersections.
	AuPoint fSegInt0;
	bool bInt0 = !fT0.FuzzyEquals(fT1) && fSegInt0.SegmentIntersection(fSeg2d0, fSeg2d1, fT0, fT1, false, false);
	if (bInt0)
	{
		if (fSegInt0.FuzzyEquals(fSeg2d0))
		{
			// Segment abuts triangle edge.
			if (fSeg2d1.SideOfLine_2D(fT0, fT1) != fT2.SideOfLine_2D(fT0, fT1))
				bInt0 = false;	// Segment abuts but does not overlap triangle.
		}
		else if (fSegInt0.FuzzyEquals(fSeg2d1))
		{
			// Segment abuts triangle edge.
			if (fSeg2d0.SideOfLine_2D(fT0, fT1) != fT2.SideOfLine_2D(fT0, fT1))
				bInt0 = false;	// Segment abuts but does not overlap triangle.
		}
	}
	AuPoint fSegInt1;
	bool bInt1 = !fT1.FuzzyEquals(fT2) && fSegInt1.SegmentIntersection(fSeg2d0, fSeg2d1, fT1, fT2, false, false);
	if (bInt1)
	{
		if (fSegInt1.FuzzyEquals(fSeg2d0))
		{
			// Segment abuts triangle edge.
			if (fSeg2d1.SideOfLine_2D(fT1, fT2) != fT0.SideOfLine_2D(fT1, fT2))
				bInt1 = false;	// Segment abuts but does not overlap triangle.
		}
		else if (fSegInt1.FuzzyEquals(fSeg2d1))
		{
			// Segment abuts triangle edge.
			if (fSeg2d0.SideOfLine_2D(fT1, fT2) != fT0.SideOfLine_2D(fT1, fT2))
				bInt1 = false;	// Segment abuts but does not overlap triangle.
		}
	}
	AuPoint fSegInt2;
	bool bInt2 = !fT2.FuzzyEquals(fT0) && fSegInt2.SegmentIntersection(fSeg2d0, fSeg2d1, fT2, fT0, false, false);
	if (bInt2)
	{
		if (fSegInt2.FuzzyEquals(fSeg2d0))
		{
			// Segment abuts triangle edge.
			if (fSeg2d1.SideOfLine_2D(fT2, fT0) != fT1.SideOfLine_2D(fT2, fT0))
				bInt2 = false;	// Segment abuts but does not overlap triangle.
		}
		else if (fSegInt2.FuzzyEquals(fSeg2d1))
		{
			// Segment abuts triangle edge.
			if (fSeg2d0.SideOfLine_2D(fT2, fT0) != fT1.SideOfLine_2D(fT2, fT0))
				bInt2 = false;	// Segment abuts but does not overlap triangle.
		}
	}

	// In triangle queries test for inside but not ON the edge lines.
	bool bIn0 = false;
	int lSide00 = fSeg2d0.SideOfLine_2D(fT0,fT1);
	int lSide01 = fSeg2d0.SideOfLine_2D(fT1,fT2);
	int lSide02 = fSeg2d0.SideOfLine_2D(fT2,fT0);
	if (lSide00 != 0 && lSide00 == lSide01 && lSide01 == lSide02)
		bIn0 = true;
	bool bIn1 = false;
	int lSide10 = fSeg2d1.SideOfLine_2D(fT0,fT1);
	int lSide11 = fSeg2d1.SideOfLine_2D(fT1,fT2);
	int lSide12 = fSeg2d1.SideOfLine_2D(fT2,fT0);
	if (lSide10 != 0 && lSide10 == lSide11 && lSide11 == lSide12)
		bIn1 = true;

	if (bInt0 || bInt1 || bInt2 || bIn0 || bIn1)
	{
		// This triangle is cut by the segment.
		// Extend the segment to find the edge intersections.
		if (!bInt0)
			bInt0 = !fT0.FuzzyEquals(fT1) && fSegInt0.SegmentIntersection(fSeg2d0, fSeg2d1, fT0, fT1, false, true);
		if (!bInt1)
			bInt1 = !fT1.FuzzyEquals(fT2) && fSegInt1.SegmentIntersection(fSeg2d0, fSeg2d1, fT1, fT2, false, true);
		if (!bInt2)
			bInt2 = !fT2.FuzzyEquals(fT0) && fSegInt2.SegmentIntersection(fSeg2d0, fSeg2d1, fT2, fT0, false, true);

		// We want intersections that are not at the ends of edge segments.
		if (fSegInt0.FuzzyEquals(fT0) || fSegInt0.FuzzyEquals(fT1))
			bInt0 = false;	// Triangle edge terminates at intersection point.
		if (fSegInt1.FuzzyEquals(fT1) || fSegInt1.FuzzyEquals(fT2))
			bInt1 = false;	// Triangle edge terminates at intersection point.
		if (fSegInt2.FuzzyEquals(fT2) || fSegInt2.FuzzyEquals(fT0))
			bInt2 = false;	// Triangle edge terminates at intersection point.

		// Look for two edge segments that are intersected by the extended segment line.
		if (bInt0 && bInt1)
		{
			// Calculate the intersect points and add three triangles to the result.
			double rSegRatio0 = fT0.Distance_2D(fSegInt0) / fT0.Distance_2D(fT1);
			AuPoint fInt0 = m_fV[0] + (m_fV[1] - m_fV[0]) * rSegRatio0;
			double rSegRatio1 = fT1.Distance_2D(fSegInt1) / fT1.Distance_2D(fT2);
			AuPoint fInt1 = m_fV[1] + (m_fV[2] - m_fV[1]) * rSegRatio1;

			AuTriangle cT1(fInt0, m_fV[1], fInt1, m_bEdge[0], m_bEdge[1], false);
			AuTriangle cT2(fInt1, m_fV[2], m_fV[0], m_bEdge[1], m_bEdge[2], false);
			AuTriangle cT3(m_fV[0], fInt0, fInt1, m_bEdge[0], false, false);

			cResult.append(cT1);
			cResult.append(cT2);
			cResult.append(cT3);
		}
		else if (bInt1 && bInt2)
		{
			// Calculate the intersect points and add three triangles to the result.
			double rSegRatio1 = fT1.Distance_2D(fSegInt1) / fT1.Distance_2D(fT2);
			AuPoint fInt1 = m_fV[1] + (m_fV[2] - m_fV[1]) * rSegRatio1;
			double rSegRatio2 = fT2.Distance_2D(fSegInt2) / fT2.Distance_2D(fT0);
			AuPoint fInt2 = m_fV[2] + (m_fV[0] - m_fV[2]) * rSegRatio2;

			AuTriangle cT1(fInt1, m_fV[2], fInt2, m_bEdge[1], m_bEdge[2], false);
			AuTriangle cT2(fInt2, m_fV[0], m_fV[1], m_bEdge[2], m_bEdge[0], false);
			AuTriangle cT3(m_fV[1], fInt1, fInt2, m_bEdge[1], false, false);

			cResult.append(cT1);
			cResult.append(cT2);
			cResult.append(cT3);
		}
		else if (bInt2 && bInt0)
		{
			// Calculate the intersect points and add three triangles to the result.
			double rSegRatio2 = fT2.Distance_2D(fSegInt2) / fT2.Distance_2D(fT0);
			AuPoint fInt2 = m_fV[2] + (m_fV[0] - m_fV[2]) * rSegRatio2;
			double rSegRatio0 = fT0.Distance_2D(fSegInt0) / fT0.Distance_2D(fT1);
			AuPoint fInt0 = m_fV[0] + (m_fV[1] - m_fV[0]) * rSegRatio0;

			AuTriangle cT1(fInt2, m_fV[0], fInt0, m_bEdge[2], m_bEdge[0], false);
			AuTriangle cT2(fInt0, m_fV[1], m_fV[2], m_bEdge[0], m_bEdge[1], false);
			AuTriangle cT3(m_fV[2], fInt2, fInt0, m_bEdge[2], false, false);

			cResult.append(cT1);
			cResult.append(cT2);
			cResult.append(cT3);
		}
		else
		{
			// The extended segment line does not intersect two edge segments. 
			// It might intersect one edge segment and the opposite vertex.
			if (bInt0)
			{
				// Calculate the intersect point and add two triangles to the result.
				double rSegRatio0 = fT0.Distance_2D(fSegInt0) / fT0.Distance_2D(fT1);
				AuPoint fInt0 = m_fV[0] + (m_fV[1] - m_fV[0]) * rSegRatio0;

				AuTriangle cT1(m_fV[2], m_fV[0], fInt0, m_bEdge[2], m_bEdge[0], false);
				AuTriangle cT2(fInt0, m_fV[1], m_fV[2], m_bEdge[0], m_bEdge[1], false);

				cResult.append(cT1);
				cResult.append(cT2);
			}
			else if (bInt1)
			{
				// Calculate the intersect point and add two triangles to the result.
				double rSegRatio1 = fT1.Distance_2D(fSegInt1) / fT1.Distance_2D(fT2);
				AuPoint fInt1 = m_fV[1] + (m_fV[2] - m_fV[1]) * rSegRatio1;

				AuTriangle cT1(m_fV[0], m_fV[1], fInt1, m_bEdge[0], m_bEdge[1], false);
				AuTriangle cT2(fInt1, m_fV[2], m_fV[0], m_bEdge[1], m_bEdge[2], false);

				cResult.append(cT1);
				cResult.append(cT2);
			}
			else if (bInt2)
			{
				// Calculate the intersect point and add two triangles to the result.
				double rSegRatio2 = fT2.Distance_2D(fSegInt2) / fT2.Distance_2D(fT0);
				AuPoint fInt2 = m_fV[2] + (m_fV[0] - m_fV[2]) * rSegRatio2;

				AuTriangle cT1(m_fV[1], m_fV[2], fInt2, m_bEdge[1], m_bEdge[2], false);
				AuTriangle cT2(fInt2, m_fV[0], m_fV[1], m_bEdge[2], m_bEdge[0], false);

				cResult.append(cT1);
				cResult.append(cT2);
			}
		}
	}
}
