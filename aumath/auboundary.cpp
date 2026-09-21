//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auboundary.cpp,v 1.3 2022/02/19 18:38:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auboundary.h"
//
#include "auscface.h"

AuBoundary::const_iterator AuBoundary::Closest(const AuPoint& loc) const
{
	double closestDist2 = 0;		// Squared distance to closest boundary loc.
	const_iterator ci = cend();		// Closest boundary loc so far.
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		double dist2 = (*bi).Distance2_2D(loc);
		if(ci == cend() || dist2 < closestDist2)
		{
			if(dist2 == 0)
				return bi;			// We won't get any closer than that.
			closestDist2 = dist2;
			ci = bi;
		}
	}
	return ci;
}

AuBoundary::const_iterator AuBoundary::ClosestSegment(const AuPoint& p) const
{
	if(size() < 3)
		return cend();

	double closestDist2 = 0;		// Squared distance to closest boundary segment.
	const_iterator ci = cend();		// Starting end of the closest boundary segment so far.
	const_iterator bi = ci - 1;		// Start first boundary point at wraparound end.
	for(const_iterator ni = cbegin(); ni != cend(); ni++)
	{
		AuPoint pint(p.PointOnSegment_2D(*bi,*ni));
		double dist2 = p.Distance2_2D(pint);
		if(ci == cend() || dist2 < closestDist2)
		{
			if(dist2 == 0)
				return bi;			// We won't get any closer than that.
			closestDist2 = dist2;
			ci = bi;
		}
		bi = ni;					// Follow ni with bi.
	}
	return ci;						// Return the closest segment we found.
}

bool AuBoundary::PointInBoundary(const AuPoint& loc) const
{
	// Insure this array contains enough vertices to form a boundary.
	if(size() < 3)
		return false;

	// Find the closest boundary segment.
	const_iterator bi = ClosestSegment(loc);
	if(bi == cend())
		return false;	// wut?
	const_iterator ni = bi + 1;	// Next end of my boundary segment.
	if(ni == cend())
		ni = cbegin();

	// The relationship of loc to the closest end of the segment tells us if it is inside or not.
	double distb = loc.Distance2_2D(*bi);
	if(distb == 0)
		return false;	// False if ON the boundary.
	double distn = loc.Distance2_2D(*ni);
	if(distn == 0)
		return false;	// False if ON the boundary.
	if(distb < distn)
	{
		// Test loc to see if it is on the inside of the boundary line from bi to ni.
		AuPoint bvec(*ni - *bi);	// Vector out of closest segment end.
		AuPoint ov((*bi).OutsideVector());
		int outside = (*bi + ov).SideOfLine_2D(*bi, *ni);	// 1=left, 0=on, -1=right
		int locside = loc.SideOfLine_2D(*bi, *ni);
		if(locside == 0)
			return false;	// False if ON the boundary.
		return locside != outside;
	}
	else
	{
		// Test loc to see if it is on the inside of the boundary line from ni to bi.
		AuPoint bvec(*bi - *ni);	// Vector out of closest segment end.
		AuPoint ov((*ni).OutsideVector());
		int outside = (*ni + ov).SideOfLine_2D(*ni, *bi);	// 1=left, 0=on, -1=right
		int locside = loc.SideOfLine_2D(*ni, *bi);
		if(locside == 0)
			return false;	// False if ON the boundary.
		return locside != outside;
	}

	/*
	// Get the extents of this boundary.
	AuPoint minpt, maxpt;
	Extents(minpt, maxpt);
	if(loc.x <= minpt.x || loc.x >= maxpt.x || loc.y <= minpt.y || loc.y >= maxpt.y)
		return false;	// Point is outside the extents of the boundary.

	// Calculate the other end of a segment that can be tested against the boundary segments.
	AuPoint segVec(maxpt - minpt);
	segVec.z = 0;
	segVec = segVec.RotateZ(DEG2RAD);	// Rotate test segment by a degree so we're less likely to hit a vertex the first time.
	AuPoint segEnd(loc + segVec);

	// Count the number of boundary segments that intersect the test segment.
	bool hitVertex;
	int intersects;
	int tries = 0;
	do
	{
		hitVertex = false;
		intersects = 0;
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		AuPoint intersect, fInt2;
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it).AuPoint::operator!=(prev))
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				// This code was cobbled from SegmentIntersection_2D.
				// Perform 2D perpconn.
				if(!AuPoint(loc.x,loc.y,0).PerpConn(AuPoint(segEnd.x,segEnd.y,0),
							AuPoint((*it).x,(*it).y,0), AuPoint(prev.x,prev.y,0), intersect, fInt2))
				{
					// Segments are parallel. This is a problem if they overlap.
					if(::IsFuzzyZero((*it).DistToSegment_2D(loc,segEnd)))
						hitVertex = true;	// Close enough of an overlap that we should rotate the test vector.
				}
				else if(intersect.FuzzyEquals_2D(fInt2) && intersect.Between_2D(loc,segEnd) && intersect.Between_2D(*it,prev))
				{
					// The test segment intersected the boundary segment.
					if(intersect.FuzzyEquals_2D(loc))
						return false;	// Test location is on the boundary segment.
					if(intersect.FuzzyEquals_2D(*it) || intersect.FuzzyEquals_2D(prev))
					{
						// Intersect at a boundary vertex introduces several potential problems.
						// Let's not get stuck doing this forever.
						tries++;
						if(tries == 360)
							return false;	// We've tried every degree direction; that's enough.
						hitVertex = true;
					}

					// Count segment intersections.
					intersects++;
				}
				if(hitVertex)
				{
					// Rotate the test segment one degree and count boundary segment intersections again.
					segVec = segVec.RotateZ(DEG2RAD);
					segEnd = loc + segVec;
					break;
				}
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
	}
	while(hitVertex);

	// The point is in the boundary if an odd number of boundary segments must be crossed to leave the boundary.
	return (intersects&1)?true:false;
	*/
}

bool AuBoundary::PointOnBoundary(const AuPoint& loc) const
{
	if(size() < 3)
		return false;

	const_iterator pi = cend() - 1;	// Start at wrap end.
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		if(loc.IsOnSegment_2D(*pi, *bi))
			return true;
		pi = bi;			// Follow bi with pi.
	}
	return false;
}

int AuBoundary::Add(const AuBoundary& ob)
{
	// Do I have enough vertices to form a boundary?
	if(size() < 3 || ob.size() < 3)
		return 0;

	// Check rectangular extents for overlap.
	AuPoint myMin, myMax, obMin, obMax;
	Extents(myMin, myMax);
	ob.Extents(obMin, obMax);
	if(myMin.x > obMax.x ||
			myMax.x < obMin.x ||
			myMin.y > obMax.y ||
			myMax.y < obMin.y)
		return 0;
	//qDebug() << "AuBoundary::Add" << size() << "adding" << ob.size();

	// Insert points in this boundary and in the join boundary where they intersect each other.
	AuBoundary obi(ob);			// Other boundary with intersections.
	AuPointArray intersections;
	int ics2, ics=0;
	for(ics=0; ics<obi.size(); ics++)
	{
		bool bJoinGrew = false;
		for(int ibnd=0; ibnd<size(); ibnd++)
		{
			// Get the join boundary segment vertices.
			ics2 = (ics != obi.size()-1) ? ics+1: 0;	// Index of next boundary vertex. Wrap at end.
			AuBoundaryPoint fc1 = obi.at(ics);
			AuBoundaryPoint fc2 = obi.at(ics2);
			if(fc1.FuzzyEquals_2D(fc2))
				continue;
			double rcl = fc1.Distance_2D(fc2);

			// Get this boundary segment vertices.
			int ibnd2 = (ibnd != size()-1) ? ibnd+1: 0;	// Index of next boundary vertex. Wrap at end.
			AuBoundaryPoint fb1 = at(ibnd);
			AuBoundaryPoint fb2 = at(ibnd2);
			if(fb1.FuzzyEquals_2D(fb2))
				continue;

			// Test the segments for intersection.
			AuPoint fint;
			if(!fint.SegmentIntersection_2D(fc1, fc2, fb1, fb2))
				continue;	// No intersection. They could be parallel.
			if(intersections.AddIfUnique(fint))
			{
				//qDebug() << "intersect" << fint << "ics" << ics << ics2 << "ibnd" << ibnd << ibnd2;
			}

			// Insert the intersection point into the join poly.
			if(!fc1.FuzzyEquals_2D(fint) && !fc2.FuzzyEquals_2D(fint))
			{
				// The intersection is not at an end point.
				double r = fint.Distance_2D(fc1)/rcl;
				Q_ASSERT(r > 0 && r < 1);
				AuBoundaryPoint bint = fc1.LinearInterpolate(fc2, r);
				obi.insert(obi.begin()+ics2, bint);
				//qDebug() << "join grew" << fint << bint << "rots" << (fc1.Rotation()*RAD2DEG) << (bint.Rotation()*RAD2DEG) <<
					//(fc2.Rotation()*RAD2DEG);
				bJoinGrew = true;
			}

			// Insert the intersection point into this boundary.
			if(!fb1.FuzzyEquals_2D(fint) && !fb2.FuzzyEquals_2D(fint))
			{
				// The intersection is not at an end point.
				double rbl = fb1.Distance_2D(fb2);
				double r = fint.Distance_2D(fb1)/rbl;
				Q_ASSERT(r > 0 && r < 1);
				AuBoundaryPoint bint = fb1.LinearInterpolate(fb2, r);
				insert(begin()+ibnd2, bint);
				//qDebug() << "this grew" << fint << bint << "rots" << (fb1.Rotation()*RAD2DEG) << (bint.Rotation()*RAD2DEG) <<
					//(fb2.Rotation()*RAD2DEG);
				
				// Decrement the loop controller so we'll redo the segment that now goes from
				// 'ibnd' to the intersect point we just added.
				ibnd--;
				//qDebug() << "backed up ibnd to" << ibnd;
			}
		}
		
		if(bJoinGrew)
		{
			// Decrement the loop controller so we'll redo the segment that now goes from
			// 'ics' to the intersect point we just added.
			ics--;
			//qDebug() << "backed up ics to" << ics;
		}
	}

	if(intersections.size() < 2)
	{
		// Not enough intersections were found. One boundary could be inside the other, possibly just touching at one vertex.
		//qDebug() << "not enough intersections" << intersections.size();
		if(PointInBoundary(ob.first()) || PointInBoundary(ob.last()))
		{
			RemoveBoundaryDuplicates();
			return 2;	// 'ob' is completely absorbed by this boundary.
		}
		else if(ob.PointInBoundary(first()) || ob.PointInBoundary(last()))
		{
			// This boundary is completely absorbed by 'ob'.
			*this = ob;
			RemoveBoundaryDuplicates();
			return 1;
		}
		return 0;
	}
	//qDebug() << "intersections" << intersections.size() << "obi size" << obi.size() << "my size" << size();

	//RemoveBoundaryDuplicates();
	//obi.RemoveBoundaryDuplicates();

	// Choose where to start walking, on this boundary or the other boundary.
	// We are going to create a super boundary around the combined boundaries.
	// We will start at the ouside and trace around both of them until we return to our starting point.
	const_iterator smi = cend();
	const_iterator soi = obi.cend();
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		if(myMin.x < obMin.x && (*mi).x == myMin.x && !intersections.contains(*mi))
		{
			// My location is at the -x extent and it is not an intersection.
			smi = mi;
			break;
		}
		if(myMax.x > obMax.x && (*mi).x == myMax.x && !intersections.contains(*mi))
		{
			// My location is at the +x extent and it is not an intersection.
			smi = mi;
			break;
		}

		if(myMin.y < obMin.y && (*mi).y == myMin.y && !intersections.contains(*mi))
		{
			// My location is at the -y extent and it is not an intersection.
			smi = mi;
			break;
		}
		if(myMax.y > obMax.y && (*mi).y == myMax.y && !intersections.contains(*mi))
		{
			// My location is at the +y extent and it is not an intersection.
			smi = mi;
			break;
		}
	}

	if(smi == cend())
	{
		// None of my boundary locations is at an extent. We should start on the other boundary.
		//qDebug() << "no place to start on my boundary";
		for(const_iterator oi = obi.cbegin(); oi != obi.cend(); oi++)
		{
			if(obMin.x < myMin.x && (*oi).x == obMin.x && !intersections.contains(*oi))
			{
				// Other boundary location is at the -x extent and it is not an intersection.
				soi = oi;
				break;
			}
			if(obMax.x > myMax.x && (*oi).x == obMax.x && !intersections.contains(*oi))
			{
				// Other boundary location is at the +x extent and it is not an intersection.
				soi = oi;
				break;
			}

			if(obMin.y < myMin.y && (*oi).y == obMin.y && !intersections.contains(*oi))
			{
				// Other boundary location is at the -y extent and it is not an intersection.
				soi = oi;
				break;
			}
			if(obMax.y > myMax.y && (*oi).y == obMax.y && !intersections.contains(*oi))
			{
				// Other boundary location is at the +y extent and it is not an intersection.
				soi = oi;
				break;
			}
		}
	}

	int mdir, odir;				// 1=forward, 0=stop, -1=backward
	if(smi != cend())
	{
		//qDebug() << "starting on my boundary at" << *smi;
		mdir = 1;
		odir = 0;
	}
	else if(soi != obi.cend())
	{
		//qDebug() << "starting on other boundary at" << *soi;
		mdir = 0;
		odir = 1;
	}
	else
	{
		//qDebug() << "no place to start on either boundary";
		return 0;
	}

	// Build the combined boundary by walking around the outside.
	AuBoundary aNewBnd;
	const_iterator mi(smi);
	const_iterator oi(soi);
	do
	{
		if(mdir)
		{
			// Walk my boundary to the next intersection or until I return to the starting point.
			odir = WalkAdd(aNewBnd, mi, mdir, obi, oi, intersections);
			if(odir)
			{
				mdir = 0;		// Stop following my boundary.
				if(odir > 1)
					odir = 0;	// Stop following the other boundary too.
			}
		}
		else
		{
			// Walk the other boundary to the next intersection or until I return to the starting point.
			mdir = obi.WalkAdd(aNewBnd, oi, odir, *this, mi, intersections);
			if(mdir)
			{
				odir = 0;		// Stop following other boundary.
				if(mdir > 1)
					mdir = 0;	// Stop following my boundary too.
			}
		}
	}
	while(mdir || odir);

	// Replace this boundary with the new one.
	clear();
	for(const_iterator bi = aNewBnd.cbegin(); bi != aNewBnd.cend(); bi++)
		append(*bi);
	Smooth();
	return 1;
}

int AuBoundary::WalkAdd(AuBoundary& bnd, const_iterator mi, int mdir, const AuBoundary& ob, const_iterator& oi,
		const AuPointArray& intersections) const
{
	bool firstAdd = true;	// True until we have added a point, for rotation blending.
	AuBoundary check;		// We will stop if we try doing the same boundary point twice.
	while(!check.contains(*mi))
	{
		// Check for boundary closure.
		if(bnd.size() > 1 && bnd.first().FuzzyEquals_2D(*mi))
			return 2;	// The boundary is closed.

		// Add the point to the check array so we don't run in circles.
		check.append(*mi);

		// Increment my boundary iterator.
		const_iterator pmi(mi);	// Keep iterator to my previous boundary point.
		if(mdir > 0)
		{
			// Forward.
			mi++;
			if(mi == cend())
				mi = cbegin();	// Wrap to beginning.
		}
		else
		{
			// Backward.
			if(mi == cbegin())
				mi = cend();	// Wrap to end.
			mi--;
		}
		if((*pmi).FuzzyEquals_2D(*mi))
			continue;

		// Add the point to the accumulating boundary, if it isn't there already.
		if(firstAdd)
		{
			// When adding the first one to an existing boundary, we must blend the rotation angle at the turn.
			if(!bnd.empty())
			{
				double orot = bnd.last().Rotation();
				double nrot = (*pmi).Rotation();
				double rot = AuNoise::Linear_Interpolate(orot, nrot, .5);
				if(fabs(nrot - orot) > PI)
					rot -= PI;		// We calculated the long way around the circle. Go the other way.
				AuBoundaryPoint bp(*pmi, rot);
				bnd.AddIfDifferentFromLast(bp);
			}
			firstAdd = false;
		}
		else
			bnd.AddIfDifferentFromLast(*pmi);

		// Check for intersection at next step.
		if(!intersections.DoesContain_2D(*mi))
			continue;

		// Found an intersection with this boundary. Which way do we go?

		// Get iterator to my next boundary point.
		const_iterator nmi(mi);
		if(mdir > 0)
		{
			// Forward.
			nmi++;
			if(nmi == cend())
				nmi = cbegin();	// Wrap to beginning.
		}
		else
		{
			// Backward.
			if(nmi == cbegin())
				nmi = cend();	// Wrap to end.
			nmi--;
		}
		Q_ASSERT(mi != nmi);
		Q_ASSERT(*mi != *nmi);

		// Calculate vector to next point in this boundary.
		AuPoint mvn(*nmi - *mi);			// Unit vector in direction of next boundary segment.
		mvn.Unit_2D();

		// Calculate vector from previous point in this boundary, pointing into the intersection.
		AuPoint mvp(*mi - *pmi);			// Unit vector in direction of previous boundary segment, entering mi.
		mvp.Unit_2D();

		// Determine the outside of the segment entering mi from the previous point. We must depart on a vector with the outside on
		// the same side as we arrived.
		AuPoint mov((*mi).OutsideVector());	// 2D unit vector pointing out of vertex at mi.
		int outside = (*mi + mov).SideOfLine_2D(*pmi, *mi);			// 1=left, 0=on, -1=right
		double ca = mvn.Dot_2D(mvp);

		// Decide whether to follow the other boundary or continue with this boundary.
		// Find the corresponding intersection in ob.
		for(oi = ob.cbegin(); oi != ob.cend(); oi++)
		{
			if(!(*oi).FuzzyEquals_2D(*mi))
				continue;

			// Found the corresponding intersection point in ob. Only one of the other boundary segments that departs the
			// intersection can do so with the outside on the same side as my incoming segment.

			// Calculate vector to next point in ob boundary.
			const_iterator oin = oi + 1;
			if(oin == ob.cend())
				oin = ob.cbegin();		// Wrap to beginning.
			AuPoint ovn(*oin - *oi);	// Vector to next point in ob boundary out of the vertex.
			ovn.Unit_2D();

			// Determine the outside of the next segment departing oi.
			AuPoint oov((*oi).OutsideVector());	// 2D unit vector pointing out of vertex at oi.
			int onext = (*oi + oov).SideOfLine_2D(*oi, *oin);	// 1=left, 0=on, -1=right
			if(onext == outside)
			{
				// Next other segment departs the intersection with the outside on the same side as the incoming vector.
				double cn = ovn.Dot_2D(mvp);
				// We are looking for the departing vector with the tightest angle on the appropriate departure side.
				if(cn < ca)
					return 1;				// Walk forward along ob, at oi.
			}
			else
			{
				// Previous other segment departs the intersection with the outside on the same side as the incoming vector.
				// Calculate vector to previous point in ob boundary.
				const_iterator oip;
				if(oi == ob.cbegin())
					oip = ob.cend();		// Wrap to end.
				else
					oip = oi;
				oip--;
				AuPoint ovp(*oip - *oi);	// Vector to previous point in ob boundary out of the vertex.
				ovp.Unit_2D();

				// We are looking for the departing vector with the tightest angle on the appropriate departure side.
				double cp = ovp.Dot_2D(mvp);
				if(cp < ca)
					return -1;			// Walk backward along ob, at oi.
			}
			break;	// Stop looking for intersections with the other boundary and continue walking this boundary.
		}
	}
	// Loops back on itself without completing the boundary.
	return 3;
}

int AuBoundary::WalkSubtract(AuBoundary& bnd, bool reverseEm, const_iterator mi, int mdir, const AuBoundary& ob, const_iterator& oi,
		const AuPointArray& intersections, const AuBoundaryComplex& pieces) const
{
	AuBoundary check;	// We will stop if we try doing the same boundary point twice.
	while(!check.contains(*mi))
	{
		// Check for boundary closure.
		if(bnd.size() > 1 && bnd.first().FuzzyEquals_2D(*mi))
			return 2;	// The boundary is closed.

		// Add the point to the check array so we don't run in circles.
		check.append(*mi);

		// Increment my boundary iterator.
		const_iterator pmi(mi);	// Keep iterator to my previous boundary point.
		if(mdir > 0)
		{
			// Forward.
			mi++;
			if(mi == cend())
				mi = cbegin();	// Wrap to beginning.
		}
		else
		{
			// Backward.
			if(mi == cbegin())
				mi = cend();	// Wrap to end.
			mi--;
		}

		// Now that mi is incremented, I can skip over my own boundary points that have already been turned into boundaries.
		if(pieces.Contains(*pmi))
			continue;

		// Add the point to the accumulating boundary, if it isn't there already.
		if(reverseEm)
		{
			AuBoundaryPoint p(*pmi);
			p.ReverseRotation();
			bnd.AddIfDifferentFromLast(*pmi);
		}
		else
			bnd.AddIfDifferentFromLast(*pmi);

		// Check for intersection at next step.
		if(!intersections.DoesContain_2D(*mi))
			continue;

		// Found an intersection with this boundary. Which way do we go?

		// Get iterator to my next boundary point.
		const_iterator nmi(mi);
		if(mdir > 0)
		{
			// Forward.
			nmi++;
			if(nmi == cend())
				nmi = cbegin();	// Wrap to beginning.
		}
		else
		{
			// Backward.
			if(nmi == cbegin())
				nmi = cend();	// Wrap to end.
			nmi--;
		}
		Q_ASSERT(mi != nmi);
		Q_ASSERT(*mi != *nmi);

		// Calculate vector to next point in this boundary.
		AuPoint mvn(*nmi - *mi);			// Unit vector in direction of next boundary segment.
		mvn.Unit_2D();

		// Calculate vector from previous point in this boundary, pointing into the intersection.
		AuPoint mvp(*mi - *pmi);			// Unit vector in direction of previous boundary segment, entering mi.
		mvp.Unit_2D();

		// Determine the outside of the segment entering mi from the previous point. We must depart on a vector with the outside on
		// the same side as we arrived.
		AuPoint mov((*mi).OutsideVector());	// 2D unit vector pointing out of vertex at mi.
		int outside = (*mi + mov).SideOfLine_2D(*pmi, *mi);			// 1=left, 0=on, -1=right
		double ca = mvn.Dot_2D(mvp);

		// Decide whether to follow the other boundary or continue with this boundary.
		// Find the corresponding intersection in ob.
		for(oi = ob.cbegin(); oi != ob.cend(); oi++)
		{
			if(!(*oi).FuzzyEquals_2D(*mi))
				continue;

			// Found the corresponding intersection point in ob. Only one of the other boundary segments that departs the
			// intersection can do so with the outside on the opposite side as my incoming segment. That is the direction we want.

			// Calculate vector to next point in ob boundary.
			const_iterator oin = oi + 1;
			if(oin == ob.cend())
				oin = ob.cbegin();		// Wrap to beginning.
			AuPoint ovn(*oin - *oi);	// Vector to next point in ob boundary out of the vertex.
			ovn.Unit_2D();

			// Determine the outside of the next segment departing oi.
			AuPoint oov((*oi).OutsideVector());	// 2D unit vector pointing out of vertex at oi.
			int onext = (*oi + oov).SideOfLine_2D(*oi, *oin);	// 1=left, 0=on, -1=right
			if(onext != outside)
			{
				// Next other segment departs the intersection with the outside on the opposite side as the incoming vector.
				double cn = ovn.Dot_2D(mvp);
				// We are looking for the departing vector with the tightest angle on the appropriate departure side.
				if(cn < ca)
					return 1;				// Walk forward along ob, at oi.
			}
			else
			{
				// Previous other segment departs the intersection with the outside on the opposite side as the incoming vector.
				// Calculate vector to previous point in ob boundary.
				const_iterator oip;
				if(oi == ob.cbegin())
					oip = ob.cend();		// Wrap to end.
				else
					oip = oi;
				oip--;
				AuPoint ovp(*oip - *oi);	// Vector to previous point in ob boundary out of the vertex.
				ovp.Unit_2D();

				// We are looking for the departing vector with the tightest angle on the appropriate departure side.
				double cp = ovp.Dot_2D(mvp);
				if(cp < ca)
					return -1;			// Walk backward along ob, at oi.
			}
			break;	// Stop looking for intersections with the other boundary and continue walking this boundary.
		}
	}
	// Loops back on itself without completing the boundary.
	return 3;
}

void AuBoundary::RemoveBoundaryDuplicates()
{
	if(size() <= 1)
		return;

	for(int ip=1; ip<size();)
	{
		if(at(ip).FuzzyEquals_2D(at(ip-1)))
			erase(begin()+ip);
		else
			ip++;
	}

	while(size() > 1 && last().FuzzyEquals_2D(first()))
		erase(end());
}

void AuBoundary::Subtract(const AuBoundary& ob, AuBoundaryComplex& pieces)
{
	// Do I have enough vertices to form a boundary?
	if(size() < 3 || ob.size() < 3)
		return;

	// Check rectangular extents for overlap.
	AuPoint myMin, myMax, obMin, obMax;
	Extents(myMin, myMax);
	ob.Extents(obMin, obMax);
	if(myMin.x > obMax.x ||
			myMax.x < obMin.x ||
			myMin.y > obMax.y ||
			myMax.y < obMin.y)
		return;

	// Insert points in this boundary and in the join boundary where they intersect each other.
	AuBoundary obi(ob);			// Other boundary with intersections.
	AuPointArray intersections;
	int ics2, ics=0;
	for(ics=0; ics<obi.size(); ics++)
	{
		bool bJoinGrew = false;
		for(int ibnd=0; ibnd<size(); ibnd++)
		{
			// Get the join boundary segment vertices.
			ics2 = (ics != obi.size()-1) ? ics+1: 0;	// Index of next boundary vertex. Wrap at end.
			AuBoundaryPoint fc1 = obi.at(ics);
			AuBoundaryPoint fc2 = obi.at(ics2);
			if(fc1.FuzzyEquals_2D(fc2))
				continue;
			double rcl = fc1.Distance_2D(fc2);

			// Get this boundary segment vertices.
			int ibnd2 = (ibnd != size()-1) ? ibnd+1: 0;	// Index of next boundary vertex. Wrap at end.
			AuBoundaryPoint fb1 = at(ibnd);
			AuBoundaryPoint fb2 = at(ibnd2);
			if(fb1.FuzzyEquals_2D(fb2))
				continue;

			// Test the segments for intersection.
			AuPoint fint;
			if(!fint.SegmentIntersection_2D(fc1, fc2, fb1, fb2))
				continue;	// No intersection. They could be parallel.
			intersections.AddIfUnique(fint);

			// Insert the intersection point into the join poly.
			if(!fc1.FuzzyEquals_2D(fint) && !fc2.FuzzyEquals_2D(fint))
			{
				// The intersection is not at an end point.
				double r = fint.Distance_2D(fc1)/rcl;
				AuBoundaryPoint bint = fc1.LinearInterpolate(fc2, r);
				if(ics2 == 0)
					obi.append(bint);
				else
					obi.insert(obi.begin()+ics2, bint);
				bJoinGrew = true;
			}

			// Insert the intersection point into this boundary.
			if(!fb1.FuzzyEquals_2D(fint) && !fb2.FuzzyEquals_2D(fint))
			{
				// The intersection is not at an end point.
				double rbl = fb1.Distance_2D(fb2);
				double r = fint.Distance_2D(fb1)/rbl;
				AuBoundaryPoint bint = fb1.LinearInterpolate(fb2, r);
				if(ibnd2 == 0)
					append(bint);
				else
					insert(begin()+ibnd2, bint);
				
				// Decrement the loop controller so we'll redo the segment that now goes from
				// 'ibnd' to the intersect point we just added.
				ibnd--;
			}
		}
		
		if(bJoinGrew)
		{
			// Decrement the loop controller so we'll redo the segment that now goes from
			// 'ics' to the intersect point we just added.
			ics--;
		}
	}

	if(intersections.size() < 2)
	{
		// Not enough intersections were found. One boundary could be inside the other, possibly just touching at one vertex.
		if(PointInBoundary(ob.first()) || PointInBoundary(ob.last()))
		{
			// 'ob' is completely inside this boundary. We do not create internal voids so there is nothing to be done.
			RemoveBoundaryDuplicates();
		}
		else if(ob.PointInBoundary(first()) || ob.PointInBoundary(last()))
		{
			// This boundary is completely inside 'ob'. I got subtracted away.
			clear();
		}
		return;
	}

	RemoveBoundaryDuplicates();
	obi.RemoveBoundaryDuplicates();

	// Choose where to start walking on this boundary.
	const_iterator smi = FirstNotInBoundary(obi, intersections);
	if(smi == cend())
	{
		// None of my vertices is not in obi so all of my vertices are in obi. I got subtracted away.
		clear();
		return;
	}

	int mdir, odir;				// 1=forward, 0=stop, -1=backward
	mdir = 1;
	odir = 0;

	// Build boundary pieces by walking around the outside of the boundary and the inside of the subtraction boundary.
	AuBoundaryComplex result;
	{
		AuBoundary aNewBnd;
		const_iterator mi = smi;
		const_iterator oi = obi.cend();
		do
		{
			if(mdir)
			{
				// Walk my boundary to the next intersection or until I return to the starting point.
				odir = WalkSubtract(aNewBnd, false, mi, mdir, obi, oi, intersections, result);
				if(odir)
				{
					mdir = 0;		// Stop following my boundary.
					if(odir > 1)
						odir = 0;	// Stop following the other boundary too.
				}
			}
			else
			{
				// Walk the other boundary to the next intersection or until I return to the starting point.
				// The rotation of its points must be reversed to orient properly to the outside.
				mdir = obi.WalkSubtract(aNewBnd, true, oi, odir, *this, mi, intersections, result);
				if(mdir)
				{
					odir = 0;		// Stop following other boundary.
					if(mdir > 1)
						mdir = 0;	// Stop following my boundary too.
				}
			}
		}
		while(mdir || odir);
		if(!aNewBnd.empty())
			result.append(aNewBnd);
	}

	// Replace this boundary with the first result.
	clear();
	if(result.empty())
		return;
	// Copy the first piece into this boundary.
	for(const_iterator bi = result.first().cbegin(); bi != result.first().cend(); bi++)
		append(*bi);
	Smooth();
	// Remaining pieces are returned as pieces.
	for(AuBoundaryComplex::const_iterator ri = result.cbegin(); ri != result.cend(); ri++)
		pieces.append(*ri);
}

void AuBoundary::FacesBottom(AuScFaceList& faces) const
{
	// Triangulate this boundary.
	AuPointArray tris;
	QVector<bool> edges;
	TriangulateBoundary(tris, edges);

	// Create a face from the triangles and edges.
	AuScFacePolygon p(tris, edges);

	// Insure the face is pointing upward.
	if(p.FaceNormal().Dot(AuPoint(0,0,-1)) < 0)
		p.ReverseTriangleFacing();				// Reverse triangle facing to be visible from above.
	p.FlattenVertexNormals();					// Set all vertex normals to match face normal.

	faces.AddFace(p);
}

void AuBoundary::FacesTop(AuScFaceList& faces) const
{
	// Triangulate this boundary.
	AuPointArray tris;
	QVector<bool> edges;
	TriangulateBoundary(tris, edges);

	// Create a face from the triangles and edges.
	AuScFacePolygon p(tris, edges);

	// Insure the face is pointing upward.
	if(p.FaceNormal().Dot(AuPoint(0,0,1)) < 0)
		p.ReverseTriangleFacing();				// Reverse triangle facing to be visible from above.
	p.FlattenVertexNormals();					// Set all vertex normals to match face normal.

	faces.AddFace(p);
}

void AuBoundary::TriangulateBoundary(AuPointArray &pTri, QVector<bool> &bTedge) const
{
	// Copy my points into a boundary list that we can modify.
	AuPointArray lBoundary;
	for(AuBoundary::const_iterator bi = cbegin(); bi != cend(); bi++)
		lBoundary.AddIfDifferentFromLast(*bi);	// Only add unique points. Don't add repeats of the same point.

	// I'm pretty sure the boundary has to be clockwise for this algorithm.
	if(!lBoundary.IsClockwise())
		lBoundary.ReverseOrder();

	// Remove points at the end of the boundary that match the beginning of the boundary.
	if(lBoundary.size() > 1)
		while(lBoundary.last().FuzzyEquals(lBoundary.first()))
			lBoundary.removeLast();

	// Create triangles within the boundary.
	while(lBoundary.size() >= 3)
	{
		// Search for a place to add a triangle.
		bool bAddedTri = false;
		AuPointArray::iterator pos = lBoundary.begin();
		while(pos != lBoundary.end())
		{
			// Get the next three points in the list.
			const AuPoint &fLoc = *pos;
			pos++;

			AuPointArray::iterator npos = pos;	// Pos of next point in boundary.
			if(npos == lBoundary.end())
				npos = lBoundary.begin();
			AuPointArray::iterator rpos = npos;	// Pos to remove a pt if we add a tri.

			// Get next two boundary points.
			const AuPoint &fNextLoc1 = *npos;
			npos++;
			if(npos == lBoundary.end())
				npos = lBoundary.begin();
			const AuPoint &fNextLoc2 = *npos;

			// Test for clockwise or counterclockwise triangle.
			if(fLoc.SideOfLine_2D(fNextLoc1, fNextLoc2) >= 0)
				continue;	// Skip triangle that goes outside the boundary.

			// Test for overlap of edge lines with any existing edge lines.
			bool bOverlap = false;
			AuPointArray::iterator ipos = lBoundary.begin();
			while(ipos != lBoundary.end())
			{
				// Get the next two points in the list.
				const AuPoint &fILoc = *ipos;
				ipos++;
				AuPointArray::iterator nipos = ipos;
				if(nipos == lBoundary.end())
					nipos = lBoundary.begin();
				const AuPoint &fNextILoc = *nipos;

				// Skip segments that abut my triangle segments.
				if(fILoc.FuzzyEquals(fLoc) || fILoc.FuzzyEquals(fNextLoc1) || fILoc.FuzzyEquals(fNextLoc2))
					continue;

				// Test for boundary points inside of this triangle.
				if(fILoc.InTriangle_3D(fLoc, fNextLoc1, fNextLoc2))
				{
					bOverlap = true;
					break;
				}

				if(fNextILoc.FuzzyEquals(fLoc) || fNextILoc.FuzzyEquals(fNextLoc1) || fNextILoc.FuzzyEquals(fNextLoc2))
					continue;

				// Test for overlap with each potential triangle segment.
				AuPoint fInt;
				if(fInt.SegmentIntersection(fILoc, fNextILoc, fLoc, fNextLoc1) ||
						fInt.SegmentIntersection(fILoc, fNextILoc, fNextLoc1, fNextLoc2) ||
						fInt.SegmentIntersection(fILoc, fNextILoc, fNextLoc2, fLoc))
				{
					bOverlap = true;
					break;
				}
			}
			if(bOverlap)
				continue;	// Skip triangle that would overlap any existing boundary segment.

			// Determine edge visibility.
			bool bEdge0 = false;
			bool bEdge1 = false;
			bool bEdge2 = false;
			for(const_iterator bi = cbegin(); bi != cend(); bi++)
			{
				const_iterator nbi = bi + 1;
				if(nbi == cend())
					nbi = cbegin();	// Wrap to beginning.

				if((*bi).FuzzyEquals(fLoc) && (*nbi).FuzzyEquals(fNextLoc1))
					bEdge0 = true;
				else if((*bi).FuzzyEquals(fNextLoc1) && (*nbi).FuzzyEquals(fNextLoc2))
					bEdge1 = true;
				else if((*bi).FuzzyEquals(fNextLoc2) && (*nbi).FuzzyEquals(fLoc))
					bEdge2 = true;
			}

			// Add a triangle.
			pTri.append(fLoc);
			bTedge.append(bEdge0);
			pTri.append(fNextLoc1);
			bTedge.append(bEdge1);
			pTri.append(fNextLoc2);
			bTedge.append(bEdge2);
			lBoundary.erase(rpos);
			bAddedTri = true;
			break;
		}
		if(!bAddedTri)
			break;
	}
}

AuBoundary::OverlapStatus AuBoundary::Overlap(const AuBoundary& ob) const
{
	// Do I have enough vertices to form a boundary?
	if(size() < 3 || ob.size() < 3)
		return NoOverlap;

	// Check rectangular extents for overlap.
	AuPoint myMin, myMax, obMin, obMax;
	Extents(myMin, myMax);
	ob.Extents(obMin, obMax);
	if(myMin.x > obMax.x ||
			myMax.x < obMin.x ||
			myMin.y > obMax.y ||
			myMax.y < obMin.y)
		return NoOverlap;

	// At this point it is possible that the boundaries overlap.
	// to test, need to try and find any intersections between
	// the boundaries.
	// If there are at least 2 unique intersections then there is overlap.
	int ip2, ic2;
	AuPointArray aIntersections;
	for(int iparent = 0; iparent < size(); iparent++)
	{
		// get indices for parent segment
		ip2 = (iparent != size() - 1) ? iparent+1 : 0;
		for(int ichild=0; ichild < ob.size(); ichild++)
		{
			// get indices for child segment
			ic2 = (ichild != ob.size() - 1) ? ichild+1 : 0;

			// Get the segment vertices that we care about.
			AuPoint fc1(ob.at(ichild));
			AuPoint fc2(ob.at(ic2));
			double rcl = fc1.Distance_2D(fc2);
			if(::IsFuzzyZero(rcl))
				continue;

			AuPoint fb1(at(iparent));
			AuPoint fb2(at(ip2));
			double rbl = fb1.Distance_2D(fb2);
			if(::IsFuzzyZero(rbl))
				continue;

			// Test the segments for intersection.
			AuPoint fint;
			if(fint.SegmentIntersection_2D(fc1, fc2, fb1, fb2))
			{
				aIntersections.AddIfUnique(fint);
				if(aIntersections.size() > 1)
					return Overlapped;
			}
		}
	}

	// No overlap has been found; check for containment.
	// If there was one intersection point then these boundaries
	// are tangent. We do not consider that an overlap case.
	if(aIntersections.empty())
	{
		// test if parent contains the child
		if(PointInBoundary(ob.first()) || PointInBoundary(ob.last()))
			return ContainsChild;
		else if(ob.PointInBoundary(first()) || ob.PointInBoundary(last()))
			return ContainedByChild;
	}
	return NoOverlap;
}

double AuBoundary::Distance2_2D(const AuBoundaryPoint& p) const
{
	double closest = 0;
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		double dist = (*bi).Distance2_2D(p);
		if(bi == cbegin() || dist < closest)
		{
			if(dist == 0)
				return dist;
			closest = dist;
		}
	}
	return closest;
}

double AuBoundary::Distance2_2D(const AuBoundary& b) const
{
	if(b.empty())
		return 0;
	double closest = 0;
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		double dist = b.Distance2_2D(*bi);
		if(bi == cbegin() || dist < closest)
		{
			if(dist == 0)
				return dist;
			closest = dist;
		}
	}
	return closest;
}

AuBoundary::const_iterator AuBoundary::Closest(const AuBoundary& b, const_iterator& bi) const
{
	if(b.empty())
	{
		bi = b.cend();
		return cend();
	}

	const_iterator ci = cend();
	double closest = 0;
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		const_iterator bci = b.Closest(*mi);
		double dist = (*bci).Distance2_2D(*mi);
		if(mi == cbegin() || dist < closest)
		{
			// This is my closest approach to 'b' so far.
			ci = mi;
			bi = bci;
			if(dist == 0)
				return ci;
			closest = dist;
		}
	}
	return ci;
}

void AuBoundary::FacesInterconnect(AuScFaceList& faces, const AuBoundary& above) const
{
	// Find my closest vertex to 'above'.
	const_iterator ai;
	const_iterator mi = Closest(above, ai);
	if(mi == cend() || ai == above.cend())
		return;
	int mverts = size();
	int averts = above.size();
	if(mverts == averts)
	{
		// 1:1 boundaries.
		for(int mv=0; mv<mverts; mv++)
		{
			const_iterator nmi(mi + 1);
			if(nmi == cend())
				nmi = cbegin();

			const_iterator nai(ai + 1);
			if(nai == above.cend())
				nai = above.cbegin();

			// We can build a quad now.
			AuScPoint lr(*nmi, 0,0, (*nmi).OutsideVector());
			AuScPoint ur(*nai, 0,0, (*nai).OutsideVector());
			AuScPoint ul(*ai, 0,0, (*ai).OutsideVector());
			AuScPoint ll(*mi, 0,0, (*mi).OutsideVector());
			/*
			AuScPoint ll(*nmi, 0,0, (*nmi).OutsideVector());
			AuScPoint ul(*nai, 0,0, (*nai).OutsideVector());
			AuScPoint ur(*ai, 0,0, (*ai).OutsideVector());
			AuScPoint lr(*mi, 0,0, (*mi).OutsideVector());
			*/
			AuScFaceQuad q(ll, ul, ur, lr);
			faces.AddFace(q);

			// Step to the next vertex in each boundary.
			mi = nmi;
			ai = nai;
		}
	}
	else if(mverts < averts)
	{
		// My boundary has fewer vertices than the upper boundary.
		double mpera = (double)mverts / (double)averts;		// Step at each vertex in the above boundary.
		double step = 0;
		for(int mv=1; mv<=mverts; mv++)
		{
			// Get iterator to next of my verts, with wrap.
			const_iterator nmi(mi + 1);
			if(nmi == cend())
				nmi = cbegin();

			// Build first triangle from mi to nmi to ai.
			AuScPoint ul(*ai, 0,0, (*ai).OutsideVector());
			AuScPoint ll(*mi, 0,0, (*mi).OutsideVector());
			AuScPoint lr(*nmi, 0,0, (*nmi).OutsideVector());
			AuScFaceTri t(ll, ul, lr);
			faces.AddFace(t);

			do
			{
				// Get iterator to next of the above verts, with wrap.
				const_iterator nai(ai + 1);
				if(nai == above.cend())
					nai = above.cbegin();

				AuScPoint ur(*nai, 0,0, (*nai).OutsideVector());
				AuScFaceTri t(ul, ur, lr);
				faces.AddFace(t);

				// Step to the next vertex in the longer above boundary.
				ai = nai;
				ul = ur;
				step += mpera;
			}
			while(FuzzyLessThan(step, mv));

			// Step to the next vertex in the shorter below boundary.
			mi = nmi;
		}
	}
	else
	{
		// My boundary has more vertices than the upper boundary.
		double aperm = (double)averts / (double)mverts;		// Step at each vertex in this boundary.
		double step = 0;
		for(int av=1; av<=averts; av++)
		{
			// Get iterator to next of the above verts, with wrap.
			const_iterator nai(ai + 1);
			if(nai == above.cend())
				nai = above.cbegin();

			// Build first triangle from ai to nai to mi.
			AuScPoint ul(*ai, 0,0, (*ai).OutsideVector());
			AuScPoint ll(*mi, 0,0, (*mi).OutsideVector());
			AuScPoint ur(*nai, 0,0, (*nai).OutsideVector());
			AuScFaceTri t(ll, ul, ur);
			faces.AddFace(t);

			do
			{
				// Get iterator to next of the below verts, with wrap.
				const_iterator nmi(mi + 1);
				if(nmi == cend())
					nmi = cbegin();

				AuScPoint lr(*nmi, 0,0, (*nmi).OutsideVector());
				AuScFaceTri t(ur, lr, ll);
				faces.AddFace(t);

				// Step to the next vertex in the longer below boundary.
				mi = nmi;
				ll = lr;
				step += aperm;
			}
			while(FuzzyLessThan(step, av));

			// Step to the next vertex in the shorter above boundary.
			ai = nai;
		}
	}
}

void AuBoundary::FromPointArray(const AuPointArray& pa)
{
	clear();
	if(pa.size() < 3)
		return;

	int outside = pa.IsClockwise() ? 1 : -1;			// 1=left, 0=on, -1=right
	AuPointArray::const_iterator pi = pa.cend() - 1;	// Start with 'previous' point in point array here, for closure.
	for(AuPointArray::const_iterator ai = pa.cbegin(); ai != pa.cend(); ai++)
	{
		// Calculate iterator to 'next' point in point array, with wrap.
		AuPointArray::const_iterator ni(ai + 1);
		if(ni == pa.cend())
			ni = pa.cbegin();

		// Knowing previous, current, and next points we can calculate the rotation angle at the vertex.
		AuPoint pvec(*pi - *ai);	// Vector pointing back to the previous point, from the current point.
		pvec.Unit();
		AuPoint nvec(*ni - *ai);	// Vector pointing forward to the next point, from the current point.
		nvec.Unit();

		// The average of the two departing vectors is our smoothing vector, though it could be backward.
		AuPoint avec((pvec + nvec) / 2);
		avec.Unit();

		// The outside vector must point to the appopriate side of the departing vector.
		int aside = (*ai + avec).SideOfLine_2D(*ai, *ni);	// 1=left, 0=on, -1=right
		if(outside != aside)
			avec = -avec;

		// Add a boundary point.
		append(AuBoundaryPoint(*ai, avec));
		pi = ai;
	}

	// Make sure I don't have any zero-length segments.
	RemoveBoundaryDuplicates();
}

void AuBoundary::Intersect(const AuLine& line, QList<const_iterator>& intersections) const
{
	if(size() < 2)
		return;
	const_iterator pi = cend() - 1;		// Wrap for closure.
	AuPoint i;
	AuPoint p(*pi);
	p.z = 0;
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		AuPoint b(*bi);
		b.z = 0;
		AuLine edge(p, b);
		if(line.Intersect(i, edge) && i.Between_2D(p, b))
		{
			// Lines intersect and the intersection is contained within the boundary segment.
			if(i.Distance_2D(p) < i.Distance_2D(b) && !intersections.contains(pi))
				intersections.append(pi);	// Intersect is closer to the previous point.
			else if(!intersections.contains(bi))
				intersections.append(bi);	// Intersect is closer to the current point.
		}
		pi = bi;
		p = b;
	}
}

void AuBoundary::Smooth()
{
	if(size() < 3)
		return;
	// Determine if current outside is left or right of the boundary direction, based on vertex index 1.
	iterator pi = begin();
	iterator bi = pi + 1;
	AuPoint ov((*bi).OutsideVector());
	int outside = (*bi + ov).SideOfLine_2D(*pi, *bi);		// 1=left, 0=on, -1=right
	if(outside == 0)
		return;

	// Smooth all vertices of this array.
	pi = end() - 1;				// Previous vertex
	for(bi = begin(); bi != end(); bi++)
	{
		// Calculate iterator to 'next' point in point array, with wrap.
		iterator ni = bi + 1;	// Next vertex
		if(ni == end())
			ni = begin();

		// Knowing previous, current, and next points we can calculate the rotation angle at the vertex.
		AuPoint pvec(*pi - *bi);	// Vector pointing back to the previous point, from the current point.
		pvec.Unit_2D();
		AuPoint nvec(*ni - *bi);	// Vector pointing forward to the next point, from the current point.
		nvec.Unit_2D();

		// The average of the two departing vectors is our smoothing vector, though it could be backward.
		AuPoint avec((pvec + nvec) / 2);
		avec.Unit_2D();

		// The outside vector must point to the appopriate side of the departing vector.
		int aside = (*bi + avec).SideOfLine_2D(*bi, *ni);	// 1=left, 0=on, -1=right
		if(outside != aside)
			avec = -avec;

		// Update the outside vector on my boundary point.
		(*bi).SetRotation(avec.ZRotation());
		pi = bi;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuBoundaryComplex::Add(const AuBoundary& b)
{
	if(b.size() < 3)
		return;

	//qDebug() << "AuBoundaryComplex::Add" << size() << "b" << b.size();
	int addres = 0;
	iterator bi = begin();
	while(bi != end())
	{
		addres = (*bi).Add(b);
		if(addres)
			break;
		bi++;
	}

	switch(addres)
	{
	case 0:
		// 'b' did not overlap any of my boundaries. Add it as a new boundary.
		//qDebug() << "AuBoundaryComplex::Add 'b' does not overlap";
		append(b);
		break;
	case 1:
		// 'b' modified 'bi' as a result of the intersection. 'bi' could now be overlapping other boundaries.
		{
			// Remove the enlarged boundary and add it back to myself, so it gets joined with any other overlaps.
			//qDebug() << "AuBoundaryComplex::Add 'b' modified me";
			AuBoundary bnd(*bi);
			erase(bi);
			Add(bnd);
		}
		break;
	case 2:
		// 'b' was completely absorbed by an existing boundary without changing anything.
		//qDebug() << "AuBoundaryComplex::Add 'b' absorbed";
		break;
	}
}

void AuBoundaryComplex::Add(const AuBoundaryComplex& bc)
{
	for(const_iterator bi = bc.cbegin(); bi != bc.cend(); bi++)
		Add(*bi);
}

void AuBoundaryComplex::Subtract(const AuBoundary& b)
{
	// Subtract 'b' from each of my boundaries.
	AuBoundaryComplex pieces;
	iterator bi = begin();
	while(bi != end())
	{
		(*bi).Subtract(b, pieces);
		if((*bi).empty())
			bi = erase(bi);	// bi completely subtracted
		else
			bi++;
	}

	// Add the pieces.
	//DHH Add(pieces);
}

void AuBoundaryComplex::Subtract(const AuBoundaryComplex& bc)
{
	for(const_iterator bi = bc.cbegin(); bi != bc.cend(); bi++)
		Subtract(*bi);
}

bool AuBoundaryComplex::Contains(const AuBoundaryPoint& p) const
{
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
		if((*bi).Contains(p))
			return true;
	return false;
}

int AuBoundaryComplex::OverlapCount(const AuBoundary& b) const
{
	int count = 0;
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
		if(b.Overlap(*bi) != AuBoundary::NoOverlap)
			count++;
	return count;
}

void AuBoundaryComplex::Overlaps(const AuBoundary& b, AuBoundaryComplex& overlaps) const
{
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
		if(b.Overlap(*bi) != AuBoundary::NoOverlap)
			overlaps.append(*bi);
}

AuBoundaryComplex::iterator AuBoundaryComplex::FindOverlap(const AuBoundary& b)
{
	for(iterator bi = begin(); bi != end(); bi++)
		if(b.Overlap(*bi) != AuBoundary::NoOverlap)
			return bi;
	return end();
}

AuBoundaryComplex::const_iterator AuBoundaryComplex::FindOverlap(const AuBoundary& b) const
{
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
		if(b.Overlap(*bi) != AuBoundary::NoOverlap)
			return bi;
	return cend();
}

void AuBoundaryComplex::FacesBottom(AuScFaceList& faces) const
{
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
		(*bi).FacesBottom(faces);
	//qDebug() << "AuBoundaryComplex::FacesBottom" << size() << "faces" << faces.size();
}

void AuBoundaryComplex::FacesTop(AuScFaceList& faces) const
{
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
		(*bi).FacesTop(faces);
	//qDebug() << "AuBoundaryComplex::FacesTop" << size() << "faces" << faces.size();
}

void AuBoundaryComplex::FacesInterconnect(AuScFaceList& faces, const AuBoundaryComplex& above, double stepSize) const
{
	if(true)	//DHH
	{
		AuBoundaryComplex tsplit;	// Top boundaries, split due to overlaps.
		for(const_iterator mi = above.cbegin(); mi != above.cend(); mi++)
		{
			// Get a list of the upper boundaries that overlap my boundary.
			AuBoundaryComplex overlaps;
			for(const_iterator ai = cbegin(); ai != cend(); ai++)
			{
				AuBoundary::OverlapStatus overlap = (*mi).Overlap(*ai);
				if(overlap != AuBoundary::NoOverlap)
					overlaps.append(*ai);
			}

			if(overlaps.empty())
			{
				// Upper boundary does not overlap the lower complex. A bottom is formed.
				(*mi).FacesBottom(faces);
			}
			else if(overlaps.size() == 1)
			{
				// My boundary underlaps one lower boundary. It does not need to be split.
				tsplit.append(*mi);
			}
			else if(overlaps.size() == 2)
			{
				// My boundary overlaps two lower boundaries. Divide this boundary in half.
				AuBoundary::const_iterator o2;
				AuBoundary::const_iterator o1 = overlaps.first().Closest(overlaps.last(), o2);
				if(o1 != overlaps.first().cend())
				{
					AuPoint midPt = (*o1 + *o2)/2;
					AuPoint dir1 = (*o1).OutsideVector().RotateZ(PI/2);	// tangent to the outside vector at o1
					AuPoint dir2 = (*o1).OutsideVector().RotateZ(PI/2);	// tangent to the outside vector at o2
					AuPoint dir;
					if(dir1.Dot(dir2) < 0)
					{
						// dir1 and dir2 point in opposite directions, as expected.
						dir = (dir1 - dir2) / 2;	// Average direction.
					}
					else
					{
						// dir1 and dir2 point in the same direction, which is unexpected. They could be perpendicular, which is also
						// unexpected.
						dir = (dir1 + dir2) / 2;	// Average direction.
					}
					dir.Unit_2D();
					AuLine clipLine(midPt.x, midPt.y, 0, dir.x, dir.y, 0);
					QList<AuBoundary::const_iterator> intersections;
					(*mi).Intersect(clipLine, intersections);
					if(intersections.size() == 2)
					{
						// My boundary can be split into two pieces, at the intersections.
						AuBoundary b1;
						AuBoundary::const_iterator b1i = intersections.first();
						while(b1i != intersections.last())
						{
							b1.append(*b1i);
							b1i++;
							if(b1i == (*mi).cend())
								b1i = (*mi).cbegin();
						}
						b1.append(*b1i);	// Terminate the last segment.

						// The closing segment may need to be broken into stepSize pieces.
						AuPoint sv1(b1.first() - b1.last());		// Vector from last boundary point to first boundary point.
						double l1 = sv1.Length();
						int segs1 = (int)ceil(l1 / stepSize);
						if(segs1 > 1)
						{
							sv1 /= segs1;
							segs1--;					// Fill in all but the last segment.
							for(int s=0; s<segs1; s++)
								b1.append(AuBoundaryPoint(b1.last() + sv1, 0));
						}
						// Fix rotation angles on the boundary, due to the points added.
						b1.Smooth();
						tsplit.append(b1);

						AuBoundary b2;
						AuBoundary::const_iterator b2i = intersections.last();
						while(b2i != intersections.first())
						{
							b2.append(*b2i);
							b2i++;
							if(b2i == (*mi).cend())
								b2i = (*mi).cbegin();
						}
						b2.append(*b2i);	// Terminate the last segment.

						// The closing segment may need to be broken into stepSize pieces.
						AuPoint sv2(b2.first() - b2.last());		// Vector from last boundary point to first boundary point.
						double l2 = sv2.Length();
						int segs2 = (int)ceil(l2 / stepSize);
						if(segs2 > 2)
						{
							sv2 /= segs2;
							segs2--;					// Fill in all but the last segment.
							for(int s=0; s<segs2; s++)
								b2.append(AuBoundaryPoint(b2.last() + sv2, 0));
						}
						// Fix rotation angles on the boundary, due to the points added.
						b2.Smooth();
						tsplit.append(b2);
					}
					else
						tsplit.append(*mi);
				}
				else
					tsplit.append(*mi);
			}
			else
			{
				// My boundary overlaps more than two lower boundaries.
				// Do this the crude way.
				//DHH for(int o=0; o<overlaps.size(); o++)
					//DHH tsplit.append(*mi);
			}
		}

		AuBoundaryComplex bsplit;	// Bottom boundaries, split due to overlaps.
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
		{
			// Get a list of the upper boundaries that overlap my boundary.
			AuBoundaryComplex overlaps;
			for(const_iterator ai = above.cbegin(); ai != above.cend(); ai++)
			{
				AuBoundary::OverlapStatus overlap = (*mi).Overlap(*ai);
				if(overlap != AuBoundary::NoOverlap)
					overlaps.append(*ai);
			}

			if(overlaps.empty())
			{
				// My boundary does not underlap the upper complex. A top is formed.
				(*mi).FacesTop(faces);
			}
			else if(overlaps.size() == 1)
			{
				// My boundary underlaps one upper boundary. It does not need to be split.
				bsplit.append(*mi);
			}
			else if(overlaps.size() == 2)
			{
				// My boundary underlaps two upper boundaries. Divide this boundary in half.
				AuBoundary::const_iterator o2;
				AuBoundary::const_iterator o1 = overlaps.first().Closest(overlaps.last(), o2);
				if(o1 != overlaps.first().cend())
				{
					AuPoint midPt = (*o1 + *o2)/2;
					AuPoint dir1 = (*o1).OutsideVector().RotateZ(PI/2);	// tangent to the outside vector at o1
					AuPoint dir2 = (*o1).OutsideVector().RotateZ(PI/2);	// tangent to the outside vector at o2
					AuPoint dir;
					if(dir1.Dot(dir2) < 0)
					{
						// dir1 and dir2 point in opposite directions, as expected.
						dir = (dir1 - dir2) / 2;	// Average direction.
					}
					else
					{
						// dir1 and dir2 point in the same direction, which is unexpected. They could be perpendicular, which is also
						// unexpected.
						dir = (dir1 + dir2) / 2;	// Average direction.
					}
					dir.Unit_2D();
					AuLine clipLine(midPt.x, midPt.y, 0, dir.x, dir.y, 0);
					QList<AuBoundary::const_iterator> intersections;
					(*mi).Intersect(clipLine, intersections);
					if(intersections.size() == 2)
					{
						// My boundary can be split into two pieces, at the intersections.
						AuBoundary b1;
						AuBoundary::const_iterator b1i = intersections.first();
						while(b1i != intersections.last())
						{
							b1.append(*b1i);
							b1i++;
							if(b1i == (*mi).cend())
								b1i = (*mi).cbegin();
						}
						b1.append(*b1i);	// Terminate the last segment.

						// The closing segment may need to be broken into stepSize pieces.
						AuPoint sv1(b1.first() - b1.last());		// Vector from last boundary point to first boundary point.
						double l1 = sv1.Length();
						int segs1 = (int)ceil(l1 / stepSize);
						if(segs1 > 1)
						{
							sv1 /= segs1;
							segs1--;					// Fill in all but the last segment.
							for(int s=0; s<segs1; s++)
								b1.append(AuBoundaryPoint(b1.last() + sv1, 0));
						}
						// Fix rotation angles on the boundary, due to the points added.
						b1.Smooth();
						bsplit.append(b1);

						AuBoundary b2;
						AuBoundary::const_iterator b2i = intersections.last();
						while(b2i != intersections.first())
						{
							b2.append(*b2i);
							b2i++;
							if(b2i == (*mi).cend())
								b2i = (*mi).cbegin();
						}
						b2.append(*b2i);	// Terminate the last segment.

						// The closing segment may need to be broken into stepSize pieces.
						AuPoint sv2(b2.first() - b2.last());		// Vector from last boundary point to first boundary point.
						double l2 = sv2.Length();
						int segs2 = (int)ceil(l2 / stepSize);
						if(segs2 > 2)
						{
							sv2 /= segs2;
							segs2--;					// Fill in all but the last segment.
							for(int s=0; s<segs2; s++)
								b2.append(AuBoundaryPoint(b2.last() + sv2, 0));
						}
						// Fix rotation angles on the boundary, due to the points added.
						b2.Smooth();
						bsplit.append(b2);
					}
					else
						bsplit.append(*mi);
				}
				else
					bsplit.append(*mi);
			}
			else
			{
				// My boundary underlaps more than two upper boundaries.
				// Do this the crude way.
				//DHH for(int o=0; o<overlaps.size(); o++)
					//DHH bsplit.append(*mi);
			}
		}

		// We could be done.
		if(bsplit.empty() || tsplit.empty())
			return;

		if(bsplit.size() != tsplit.size())
		{
			qWarning() << "error: AuBoundaryComplex::FacesInterconnect bsplit" << bsplit.size() << "tsplit" << tsplit.size();
			return;
		}

		// Let's tessellate.
		// Interconnect each bottom boundary with one overlapping upper boundary.
		for(const_iterator bi = bsplit.cbegin(); bi != bsplit.cend(); bi++)
		{
			// Find one overlapping upper split boundary.
			for(iterator ti = tsplit.begin(); ti != tsplit.end(); ti++)
			{
				AuBoundary::OverlapStatus overlap = (*bi).Overlap(*ti);
				if(overlap == AuBoundary::NoOverlap)
					continue;
				(*bi).FacesInterconnect(faces, *ti);
				ti = tsplit.erase(ti);
				break;
			}
		}
	}
	else
	{
		// Build 1:1 bottom and top boundaries.
		AuBoundaryComplex bottom;	// Lower boundaries, duplicated by overlaps with the upper complex.
		AuBoundaryComplex top;		// Upper boundaries, duplicated by overlaps with this complex.
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
		{
			// Add a pair of boundaries for every overlap.
			// This will create excess triangles at the split points but it is faster than breaking them apart.
			bool anyOverlap = false;
			for(const_iterator ai = above.cbegin(); ai != above.cend(); ai++)
			{
				AuBoundary::OverlapStatus overlap = (*mi).Overlap(*ai);
				if(overlap != AuBoundary::NoOverlap)// && overlap != AuBoundary::Overlapped)
				{
					bottom.append(*mi);
					top.append(*ai);
					anyOverlap = true;
				}
			}

			// When my boundary does not overlap the upper complex, it forms a top.
			if(!anyOverlap)
				(*mi).FacesTop(faces);
		}

		// Check for upper boundaries that do not overlap any of mine, to form bottoms.
		for(const_iterator ai = above.cbegin(); ai != above.cend(); ai++)
			if(FindOverlap(*ai) == cend())
				(*ai).FacesBottom(faces);	// No overlap so form a bottom.

		// We could be done.
		if(bottom.empty() || top.empty())
			return;

		// Let's tessellate.
		// Interconnect each bottom boundary with the overlapping upper boundary.
		const_iterator ti = top.cbegin();
		const_iterator bi = bottom.cbegin();
		while(ti != top.cend() && bi != bottom.cend())
		{
			(*bi).FacesInterconnect(faces, *ti);
			bi++;
			ti++;
		}
	}
}

void AuBoundaryComplex::FacesAsLines(AuScFaceList& faces) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		AuBoundary::const_iterator pbi = (*mi).cend() - 1;	// for wrap/closure at end
		for(AuBoundary::const_iterator bi = (*mi).cbegin(); bi != (*mi).cend(); bi++)
		{
			AuScPoint v1(*pbi, 0,0, AuPoint(0,0,1));
			AuScPoint v2(*bi, 0,0, AuPoint(0,0,1));
			AuScPoint v1o = AuScPoint(v1 + (*pbi).OutsideVector() * .02, 0,0, AuPoint(0,0,1));
			AuScPoint v2o = AuScPoint(v2 + (*bi).OutsideVector() * .02, 0,0, AuPoint(0,0,1));
			AuScFaceQuad q(v1, v2, v2o, v1o);
			q.FlattenVertexNormals();
			faces.AddFace(q);
			pbi = bi;
		}
	}
}

AuBoundaryComplex::iterator AuBoundaryComplex::Closest(const AuBoundary& b)
{
	double closest = 0;
	iterator ci = end();
	for(iterator bi = begin(); bi != end(); bi++)
	{
		double dist = b.Distance2_2D(*bi);
		if(bi == cbegin() || dist < closest)
		{
			ci = bi;
			closest = dist;
		}
	}
	return ci;
}

AuBoundaryComplex::const_iterator AuBoundaryComplex::Closest(const AuBoundary& b) const
{
	double closest = 0;
	const_iterator ci = cend();
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		double dist = b.Distance2_2D(*bi);
		if(bi == cbegin() || dist < closest)
		{
			ci = bi;
			closest = dist;
		}
	}
	return ci;
}

