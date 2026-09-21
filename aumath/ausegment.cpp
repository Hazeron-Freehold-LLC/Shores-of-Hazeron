//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausegment.cpp,v 1.10 2022/02/19 18:38:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausegment.h"
//


// 'ignoreDist' when positive skips this segment if view distance is beyond.
// Returns -1 if skipped.
double AuSegment::ViewDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint& edgeLoc, double ignoreDist) const
{
	// Project this segment into the view.
	AuSegment segpx(m_fLoc1, m_fLoc2);
	segpx.Transform(mvp);
	if(segpx.m_fLoc1.z < 0 && segpx.m_fLoc2.z < 0)
		return -1;											// Both ends are behind the eye plane.

	// Calculate distance to segment.
	AuPoint fpx(pixel);
	AuPoint pintpx(fpx.PointOnSegment_2D(segpx.m_fLoc1, segpx.m_fLoc2));
	double dseg = fpx.Distance_2D(pintpx);
	if(ignoreDist >= 0 && dseg >= ignoreDist)
		return -1;

	// Calculate intersection point along this segment.
	if(pintpx.FuzzyEquals_2D(segpx.m_fLoc1))
		edgeLoc = m_fLoc1;									// Unprojected corresponding loc.
	else if(pintpx.FuzzyEquals_2D(segpx.m_fLoc2))
		edgeLoc = m_fLoc1;									// Unprojected corresponding loc.
	else
	{
		// Interpolate between the points. This is inaccurate due to perspective projection.
		AuPoint pxvec = segpx.m_fLoc2 - segpx.m_fLoc1;
		double pxlen = pxvec.Length_2D();
		AuPoint pxintvec = pintpx - segpx.m_fLoc1;
		double pxintlen = pxintvec.Length_2D();
		double pxfrac = pxintlen / pxlen;

		AuPoint vec = m_fLoc2 - m_fLoc1;  					// Unprojected corresponding vector to pxvec.
		vec *= pxfrac;
		edgeLoc = m_fLoc1 + vec;
	}

	return dseg;
}

bool AuSegment::Overlaps(const AuSegment& s) const
{
	// Fuzzy comparison of end points, both ways.
	if(IsSegment(s))
		return true;										// We are the same segment.

	// Test for overlap that is not at an end.
	// If the segments are not parallel, they cannot overlap.
	AuPoint v(m_fLoc2 - m_fLoc1);
	v.Unit();
	AuPoint vs(s.m_fLoc2 - s.m_fLoc1);
	vs.Unit();
	if(!v.IsParallel(vs))
		return false;										// We are not parallel.

	// We overlap if either of my end points is between the end points of the other segment.
	if(!m_fLoc1.FuzzyEquals(s.m_fLoc1) && !m_fLoc1.FuzzyEquals(s.m_fLoc2) && m_fLoc1.Between(s.m_fLoc1, s.m_fLoc2))
		return true;
	if(!m_fLoc2.FuzzyEquals(s.m_fLoc1) && !m_fLoc2.FuzzyEquals(s.m_fLoc2) && m_fLoc2.Between(s.m_fLoc1, s.m_fLoc2))
		return true;

	// We overlap if either of the other segment's end points is between my end points.
	if(!s.m_fLoc1.FuzzyEquals(m_fLoc1) && !s.m_fLoc1.FuzzyEquals(m_fLoc2) && s.m_fLoc1.Between(m_fLoc1, m_fLoc2))
		return true;
	if(!s.m_fLoc2.FuzzyEquals(m_fLoc1) && !s.m_fLoc2.FuzzyEquals(m_fLoc2) && s.m_fLoc2.Between(m_fLoc1, m_fLoc2))
		return true;

	return false;
}

void AuSegment::SplitEvenly(double maxSegLen, AuSegmentArray& segs) const
{
	// Calculate my length in preparation to split this segment.
	AuPoint v(m_fLoc2 - m_fLoc1);				// My vector from beginning to end.
	double len = v.Length();					// Length of this segment.
	if(len <= maxSegLen)
	{
		// This is an easy one. I can return myself without splitting.
		segs.append(*this);
		return;
	}

	// Split this segment into pieces of equal length, not to exceed maxSegLen.
	int pieces = (int)ceil(len / maxSegLen);	// Number of resulting segment pieces.
	double pieceLen = len / pieces;				// Length of each resulting segment piece.
	v /= len;									// My unit direction vector.
	AuPoint s1(m_fLoc1);						// Segment piece end 1 starts at my beginning, for our walk along the segment.
	for(int p = 1; p < pieces; p++)				// Do all but the last segment.
	{
		double pe = pieceLen * p;				// Distance at the end of the piece.
		AuPoint s2(m_fLoc1 + v * pe);			// Location at the end of the piece.
		segs.append(AuSegment(s1, s2));			// Add the piece to the return result.
		s1 = s2;								// Step to the end for the next, or final, segment.
	}

	// We walked almost to the end of the segment. Doing this causes the final segment to end precisely where this segment ends,
	// with no variance due to calculations.
	segs.append(AuSegment(s1, m_fLoc2));		// Add the final piece to the return result.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// IsSegment is used to test for uniqueness.
void AuSegmentArray::AddIfUnique(const AuSegment& seg)
{
	if(!Contains(seg))
		append(seg);
}

// IsSegment is used to test for uniqueness.
void AuSegmentArray::AddIfUnique(const AuSegmentArray& segs)
{
	for(const_iterator si = segs.cbegin(); si != segs.cend(); si++)
		AddIfUnique(*si);
}

bool AuSegmentArray::Contains(const AuSegment& seg) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).IsSegment(seg))
			return true;
	return false;
}

void AuSegmentArray::Intersect(const AuSegmentArray& segs)
{
	for(const_iterator si = segs.cbegin(); si != segs.cend(); si++)
	{
		// Check for overlap.
		bool overlap = false;
		for(iterator msi = begin(); msi != end(); msi++)
		{
			if((*msi).IsSegment(*si))
			{
				// Remove my overlapped segment.
				msi = erase(msi);
				overlap = true;
				break;
			}
		}

		// Do not add overlapped segments.
		if(overlap)
			continue;

		append(*si);
	}
}

void AuSegmentArray::Fuse()
{
	// Combine parallel edge segments that meet end-to-end or overlap.
	AuSegmentArray minedges;
	while(!empty())
	{
		// Grab the first edge segment.
		AuSegment seg = takeFirst();
		//if(seg.m_fLoc1 == seg.m_fLoc2)
		if(seg.m_fLoc1.FuzzyEquals(seg.m_fLoc2))
			continue;	// Edge line is collapsed.
		AuPoint sv(seg.m_fLoc2 - seg.m_fLoc1);
		sv.Unit();

		// Join all edge segments to it that are parallel and share a vertex.
		bool segChanged;
		do
		{
			segChanged = false;
			iterator ei = begin();
			while(ei != end())
			{
				//if((*ei).m_fLoc1 == (*ei).m_fLoc2)
				if((*ei).m_fLoc1.FuzzyEquals((*ei).m_fLoc2))
				{
					// Edge line is collapsed.
					ei = erase(ei);
					continue;
				}

				AuPoint ev((*ei).m_fLoc2 - (*ei).m_fLoc1);
				ev.Unit();
				if(!sv.IsParallel(ev))
				{
					ei++;
					continue;
				}

				if(seg.m_fLoc1.FuzzyEquals((*ei).m_fLoc1))
				//if(seg.m_fLoc1 == ((*ei).m_fLoc1))
				{
					if(!(*ei).m_fLoc2.Between(seg.m_fLoc1, seg.m_fLoc2))
					{
						seg.m_fLoc1 = (*ei).m_fLoc2;
						segChanged = true;
					}
					ei = erase(ei);
				}
				else if(seg.m_fLoc2.FuzzyEquals((*ei).m_fLoc1))
				//else if(seg.m_fLoc2 == ((*ei).m_fLoc1))
				{
					if(!(*ei).m_fLoc2.Between(seg.m_fLoc1, seg.m_fLoc2))
					{
						seg.m_fLoc2 = (*ei).m_fLoc2;
						segChanged = true;
					}
					ei = erase(ei);
				}
				else if(seg.m_fLoc1.FuzzyEquals((*ei).m_fLoc2))
				//else if(seg.m_fLoc1 == ((*ei).m_fLoc2))
				{
					if(!(*ei).m_fLoc1.Between(seg.m_fLoc1, seg.m_fLoc2))
					{
						seg.m_fLoc1 = (*ei).m_fLoc1;
						segChanged = true;
					}
					ei = erase(ei);
				}
				else if(seg.m_fLoc2.FuzzyEquals((*ei).m_fLoc2))
				//else if(seg.m_fLoc2 == ((*ei).m_fLoc2))
				{
					if(!(*ei).m_fLoc1.Between(seg.m_fLoc1, seg.m_fLoc2))
					{
						seg.m_fLoc2 = (*ei).m_fLoc1;
						segChanged = true;
					}
					ei = erase(ei);
				}
				else if(seg.m_fLoc1.DistToLine((*ei).m_fLoc1, (*ei).m_fLoc2) < FUZZ)
				{
					// Lines are parallel and colinear.
					bool sb1 = seg.m_fLoc1.Between((*ei).m_fLoc1, (*ei).m_fLoc2);
					bool sb2 = seg.m_fLoc2.Between((*ei).m_fLoc1, (*ei).m_fLoc2);
					if(sb1 && sb2)
					{
						// seg is absorbed by ei.
						seg = *ei;
						segChanged = true;
						ei = erase(ei);
					}
					else
					{
						bool eb1 = (*ei).m_fLoc1.Between(seg.m_fLoc1, seg.m_fLoc2);
						bool eb2 = (*ei).m_fLoc2.Between(seg.m_fLoc1, seg.m_fLoc2);
						if(eb1 && eb2)
						{
							// ei is absorbed by seg.
							ei = erase(ei);
						}
						else if(sb1)
						{
							// seg loc 1 is between the ends of ei. It will move to the end of ei that is not between seg's ends.
							if(eb1)
								seg.m_fLoc1 = (*ei).m_fLoc2;	// ei loc 2 is not between the ends of seg.
							else
								seg.m_fLoc1 = (*ei).m_fLoc1;	// ei loc 1 is not between the ends of seg.
							segChanged = true;
							ei = erase(ei);
						}
						else if(sb2)
						{
							// seg loc 2 is between the ends of ei. It will move to the end of ei that is not between seg's ends.
							if(eb1)
								seg.m_fLoc2 = (*ei).m_fLoc2;	// ei loc 2 is not between the ends of seg.
							else
								seg.m_fLoc2 = (*ei).m_fLoc1;	// ei loc 1 is not between the ends of seg.
							segChanged = true;
							ei = erase(ei);
						}
						else
							ei++;
					}
				}
				else
					ei++;
			}
		}
		while(segChanged);
		minedges.append(seg);
	}
	*this = minedges;
}

void AuSegmentArray::BreakIntersections()
{
	AuSegmentArray osegs(*this);	// Original segments. I will store the result into myself.
	clear();
	for(const_iterator oi = osegs.cbegin(); oi != osegs.cend(); oi++)
	{
		// Process the segment in oi. If it is not broken, it will be added intact to this array.
		QMap<double, AuPoint> intersections;	// Key is distance from first end.
		double ol = (*oi).Length();
		if(IsFuzzyZero(ol))
		{
			//qWarning() << "error: AuSegmentArray::BreakIntersections null segment" << ol;
			continue;
		}

		for(const_iterator bi = oi + 1; bi != osegs.cend(); bi++)
		{
			AuPoint i;
			if(!i.SegmentIntersection((*oi).m_fLoc1, (*oi).m_fLoc2, (*bi).m_fLoc1, (*bi).m_fLoc2))
				continue;
			double d = (*oi).m_fLoc1.Distance(i);	// Distance along oi.
			if(IsFuzzyZero(d))
				continue;							// Too fuzzy close to the beginning.
			else if(FuzzyEquals(d, ol))
				continue;							// Too fuzzy close to the end.
			intersections.insert(FROUND(d, FUZZ), i);
		}

		if(!intersections.empty())
		{
			// oi is broken into multiple segments.
			AuPoint p((*oi).m_fLoc1);
			for(QMap<double, AuPoint>::const_iterator ii = intersections.cbegin(); ii != intersections.cend(); ii++)
			{
				append(AuSegment(p, *ii));
				p = *ii;
			}
			append(AuSegment(p, (*oi).m_fLoc2));
		}
		else
			append(*oi);
	}
}

bool AuSegmentArray::Align()
{
	int boundaries = 0;
	AuSegmentArray nusegs;
	do
	{
		// Grab the first segment to build onto.
		AuSegment s = takeFirst();
		nusegs.append(s);
		boundaries++;

		// Look for other segments that abut nusegs.
		bool foundSeg;
		do
		{
			// Find a segment that abuts s.
			foundSeg = false;
			for(iterator si = begin(); si != end(); si++)
			{
				if((*si).m_fLoc1.FuzzyEquals(nusegs.first().m_fLoc1))
				{
					// Prepend segment with ends reversed.
					nusegs.prepend(AuSegment((*si).m_fLoc2, (*si).m_fLoc1));
					si = erase(si);
					foundSeg = true;
					break;
				}
				else if((*si).m_fLoc2.FuzzyEquals(nusegs.first().m_fLoc1))
				{
					// Prepend segment as is.
					nusegs.prepend(*si);
					si = erase(si);
					foundSeg = true;
					break;
				}
				else if((*si).m_fLoc1.FuzzyEquals(nusegs.last().m_fLoc2))
				{
					// Append segment as is.
					nusegs.append(*si);
					si = erase(si);
					foundSeg = true;
					break;
				}
				else if((*si).m_fLoc2.FuzzyEquals(nusegs.last().m_fLoc2))
				{
					// Append segment with ends reversed.
					nusegs.append(AuSegment((*si).m_fLoc2, (*si).m_fLoc1));
					si = erase(si);
					foundSeg = true;
					break;
				}
			}
		}
		while(foundSeg);
	}
	while(!empty());

	*this = nusegs;
	return boundaries == 1 && nusegs.size() > 2 && nusegs.first().m_fLoc1.FuzzyEquals(nusegs.last().m_fLoc2);
}

void AuSegmentArray::MakeBoundary(AuPoint2dArray& b) const
{
	b.clear();
	if(size() < 3)
		return;

	// We must start at an outside vertex, so find the one with the least X.
	AuPoint preV(first().m_fLoc1);
	for(const_iterator si = cbegin(); si != cend(); si++)
	{
		if((*si).m_fLoc1.x < preV.x)
			preV = (*si).m_fLoc1;
		if((*si).m_fLoc2.x < preV.x)
			preV = (*si).m_fLoc2;
	}

	// Starting at the farthest left vertex at preV.
	b.append(AuPoint2d(preV.x, preV.y));
	double preSegAng = 0;				// Largest angle leaving this vertex could not be more than PI/2 due to preV being at min X.

	// Follow from one segment to the next, making left turns. Here we depend on the caller to have already called Fuse so we know
	// we don't have overlapping segments, or parallel segments end-to-end, which is a waste of time.
	//qDebug() << "segs" << size();
	QSet<const_iterator> usedSegs;		// We will only traverse each segment once.
	do
	{
		AuPoint nextV;
		double bestIncAng = 0;
		double bestSegAng = 0;
		const_iterator besti = cend();
		for(const_iterator si = cbegin(); si != cend(); si++)
		{
			// Skip segments we have already traced.
			if(usedSegs.contains(si))
				continue;
			//if((*si).m_fLoc1.FuzzyEquals((*si).m_fLoc2))
				//continue;

			if((*si).m_fLoc1.FuzzyEquals_2D(preV))
			{
				// Calculate angle of segment departing preV.
				double segAng = ((*si).m_fLoc2 - (*si).m_fLoc1).ZRotation();
				double incAng = segAng - preSegAng;
				if(incAng > PI)
					incAng -= PI*2;
				else if(incAng < -PI)
					incAng += PI*2;

				//qDebug() << "loc1 segAng" << (segAng*RAD2DEGf) << "incAng" << (incAng*RAD2DEGf)
					//<< "seg" << (*si).m_fLoc1 << (*si).m_fLoc2;
				// Largest angle wins.
				if(besti == cend() || incAng > bestIncAng)
				{
					nextV = (*si).m_fLoc2;
					bestIncAng = incAng;
					bestSegAng = segAng;
					besti = si;
				}
			}
			else if((*si).m_fLoc2.FuzzyEquals_2D(preV))
			{
				// Calculate angle of segment departing preV.
				double segAng = ((*si).m_fLoc1 - (*si).m_fLoc2).ZRotation();
				double incAng = segAng - preSegAng;
				if(incAng > PI)
					incAng -= PI*2;
				else if(incAng < -PI)
					incAng += PI*2;

				//qDebug() << "loc2 segAng" << (segAng*RAD2DEGf) << "incAng" << (incAng*RAD2DEGf)
					//<< "seg" << (*si).m_fLoc1 << (*si).m_fLoc2;
				// Largest angle wins.
				if(besti == cend() || incAng > bestIncAng)
				{
					nextV = (*si).m_fLoc1;
					bestIncAng = incAng;
					bestSegAng = segAng;
					besti = si;
				}
			}
			//else
				//qDebug() << "preV" << preV << "seg" << (*si).m_fLoc1 << (*si).m_fLoc2;
		}
		if(besti == cend())
		{
			b.clear();
			qWarning() << "error: AuSegmentArray::MakeBoundary lost the trail" << size();
			//for(const_iterator si = cbegin(); si != cend(); si++)
				//qDebug() << (*si).Vertex(0) << (*si).Vertex(1);
			return;	// wtf?
		}

		// We found the next outer segment from preV to nextV.
		preV = nextV;
		b.append(AuPoint2d(preV.x, preV.y));
		//qDebug() << "bestSegAng" << (bestSegAng*RAD2DEGf);
		//return;
		preSegAng = bestSegAng;

		// We only get to traverse each segment once.
		usedSegs.insert(besti);
	}
	while(!b.first().FuzzyEquals(b.last(), FUZZ*100));
	if(!b.empty())
		b.removeLast();	// The closing boundary point is not needed.
}

void AuSegmentArray::RemoveDeadEnds(double tol)
{
	// Do this repeatedly until we make a pass without removing a segment.
	bool removedOne;
	do
	{
		// Look for a segment to remove that has no other segment abutting one or both of its ends.
		removedOne = false;
		iterator ri = begin();
		while(ri != end())
		{
			// Find a segment abutting each end of the ri segment.
			bool end1 = false;
			bool end2 = false;
			for(iterator ai = begin(); ai != end(); ai++)
			{
				// Skip the ri segment.
				if(ai == ri)
					continue;

				if(!end1)
				{
					// Looking for a segment to abut end 1.
					if((*ri).m_fLoc1.FuzzyEquals((*ai).m_fLoc1, tol) || (*ri).m_fLoc1.FuzzyEquals((*ai).m_fLoc2, tol))
					{
						end1 = true;	// Segment ai abuts end 1 of segment ri.
						if(end2)
							break;		// We're done looking.
					}
				}

				if(!end2)
				{
					// Looking for a segment to abut end 2.
					if((*ri).m_fLoc2.FuzzyEquals((*ai).m_fLoc1, tol) || (*ri).m_fLoc2.FuzzyEquals((*ai).m_fLoc2, tol))
					{
						end2 = true;	// Segment ai abuts end 1 of segment ri.
						if(end1)
							break;		// We're done looking.
					}
				}
			}

			// What did we find?
			if(!end1 || !end2)
			{
				// One end of segment ri is open. It can be removed.
				ri = erase(ri);
				removedOne = true;
			}
			else
				ri++;	// We found connections to both ends of segment ri.
		}
	}
	while(removedOne);
}

void AuSegmentArray::Extents(AuPoint& min, AuPoint& max) const
{
	if(empty())
		return;
	const_iterator si = cbegin();
	min = (*si).m_fLoc1.Min((*si).m_fLoc2);
	max = (*si).m_fLoc1.Max((*si).m_fLoc2);
	si++;
	for(; si != cend(); si++)
	{
		min = min.Min((*si).m_fLoc1);
		max = max.Max((*si).m_fLoc1);
		min = min.Min((*si).m_fLoc2);
		max = max.Max((*si).m_fLoc2);
	}
}
