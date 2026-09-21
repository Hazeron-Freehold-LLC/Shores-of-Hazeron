//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhair.cpp,v 1.65 2022/02/19 18:20:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auhair.h"
//
#include "aunoise.h"
#include "ausolid.h"

void AuHairPoint::AddBone(int bone, quint8 boneIndex, float boneWeight)
{
	m_BoneIndex[bone] = boneIndex;
	m_BoneWeight[bone] = boneWeight;

	// Insure all bone weights equal 1.
	double total = m_BoneWeight[0] + m_BoneWeight[1] + m_BoneWeight[2] + m_BoneWeight[3];
	if(total == 1 || total <= 0)
		return;
	m_BoneWeight[0] /= total;
	m_BoneWeight[1] /= total;
	m_BoneWeight[2] /= total;
	m_BoneWeight[3] /= total;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Returns the 3D extents box of this triangle.
void AuHairTri::Extents(AuPoint& min, AuPoint& max) const
{
	min = m_V0;
	max = m_V0;

	min = min.Min(m_V1);
	max = max.Max(m_V1);

	min = min.Min(m_V2);
	max = max.Max(m_V2);
}

const AuPoint& AuHairTri::SurfaceNormalCalc() const
{
	if(m_N.IsZero())
	{
		AuPoint vx = m_V2 - m_V0;
		AuPoint vy = m_V1 - m_V0;
		vx.Unit();
		vy.Unit();
		m_N = vx.Cross(vy);
		m_N.Unit();
	}
	return m_N;
}

bool AuHairTri::Above(double z) const
{
	if(m_V0.z > z)
		return true;
	if(m_V1.z > z)
		return true;
	if(m_V2.z > z)
		return true;
	return false;
}

bool AuHairTri::Below(double z) const
{
	if(m_V0.z < z)
		return true;
	if(m_V1.z < z)
		return true;
	if(m_V2.z < z)
		return true;
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuHairMesh::MakeLock(const AuFolliclePoint& follicle, AuHairStyleLock::STYLE style, float headSpace, float lockWidth,
		float lockLength, float hangLength, const AuPoint& knotLoc, float knotLength, float widAng, const AuPoint& combDir,
		bool basePatch, const AuSolidEllipsoid* skull)
{
	if(style == AuHairStyleLock::Shaved && !basePatch)
		return;
	if(style == AuHairStyleLock::Mohawk || style == AuHairStyleLock::MohawkHang)
	{
		float fang = follicle.m_N.YRotation();	// 0 is to the right, 90 is straight up, 180 is to the left
		fang -= PIf/2;					// -90 to the right, 0 straight up, 90 to the left
		if(fang > widAng || fang < -widAng)
			return;
	}

	if(lockLength <= 0 || lockWidth <= 0)
		return;
	int steps = lockLength / (lockWidth * 2.f);
	if(steps < 1)
		steps = 1;
	double stepLen = lockLength / steps;
	if(headSpace > lockLength)
		headSpace = lockLength;

	// Perlin noise parameters.
	float noiseoff = follicle.x + follicle.y + follicle.z;
	float nfreq = AuHairStyleLock::NoiseFrequency(style);	// Number of times the pattern repeats per foot?
	float persSideToSide;	// Persistence. Bigger is wavier
	float persUpAndDown;	// Persistence. Bigger is wavier
	if(style == AuHairStyleLock::Rough)
	{
		persSideToSide = lockLength;
		persUpAndDown = headSpace*10;
	}
	else
	{
		persSideToSide = headSpace*2;
		persUpAndDown = headSpace;
	}

	int fall = AuHairStyleLock::FallWithGravity(style);
	bool ignoreHead = AuHairStyleLock::IgnoreHead(style);

	double combLimit = AuHairStyleLock::IsKeptClose(style) ? headSpace + headSpace/2: 0;	// Top of combed hair. n/a if no skull.
	AuPoint centerRef;
	if(skull)
		centerRef = skull->M().Translation();

	AuPoint vcomb;
	double knotAng = 0;
	double knotPol = 0;
	bool knotStyle = style == AuHairStyleLock::HeadKnot;
	if(knotStyle)
	{
		knotStyle = true;
		vcomb = knotLoc - follicle;
		AuPoint vknot(knotLoc - centerRef);
		knotAng = vknot.ZRotation();
		knotPol = vknot.PolarIncline();
	}
	else
		vcomb = combDir;

	AuPoint lockDir(follicle.m_N + vcomb);
	lockDir.Unit();

	AuHairMesh lockTop;		// Up facing polygons.
	AuHairMesh lockBottom;	// Down facing polygons.

	AuHairPoint sllt, slct, slrt, sllb, slcb, slrb;
	AuHairPoint sllt2, slct2, slrt2, sllb2, slcb2, slrb2;

	// Texture coords.
	float lltx = 0;
	float lctx = .5f;
	float lrtx = 1;

	// Render parameters.
	float shine = 100;
	float reflect = 1;
	int boneIndex[4] = {0,0,0,0};
	float boneWeight[4] = {1,0,0,0};
	const QColor& hairColor = follicle.HairColor();

	AuPoint prevCen(follicle);	// Previous center line point.
	AuPoint cen, vup, vx, vfolx, vfoly;
	double acclen = 0;
	bool crested = false;
	bool knotted = false;
	double hangLenSoFar = 0;
	double fuzzFactor = 1*FeetPerMM();
	for(int s=0; FuzzyLessThanFactor(acclen, lockLength, fuzzFactor) && s < steps*2; s++)
	{
		// Walk center line of lock back across head.
		if(s == 0)
		{
			cen = follicle;
			vup = follicle.m_N;

			vfolx = vup.Cross(AuPoint(0,0,1));
			vfolx.Unit();
			vfoly = vup.Cross(vfolx);
			vfoly.Unit();

			vfolx *= lockWidth/2;
			vfoly *= lockWidth;

			// Calculate edges of lock.
			if(vcomb.IsZero())
				vx = vfolx;
			else
				vx = vcomb.Cross(vup);
			if(lockWidth > lockLength)
				vx.Unit(lockLength/2);		// Let the actual hair poly be narrower than the follicle patch.
			else
				vx.Unit(lockWidth/2);
		}
		else
		{
			switch(fall)
			{
			case 0:
				break;
			case 1:
				lockDir.x *= .9;			// Apply drag to vector away from skull.
				lockDir.y *= .9;			// Apply drag to vector away from skull.
				lockDir.z -= stepLen*s*10;	// Hair bends down with its own weight.
				lockDir.Unit();
				break;
			case 2:
				if(!knotted)
					break;
				lockDir = AuPoint(0,0,-1);	// Head down at the knot
				break;
			case 3:
				lockDir.x *= .5;			// Apply aggressive drag to vector away from skull.
				lockDir.y *= .5;			// Apply aggressive drag to vector away from skull.
				lockDir.z -= stepLen*s*10;	// Hair bends down with its own weight.
				lockDir.Unit();
				break;
			}

			AuPoint hvec(lockDir * stepLen);
			if(!vcomb.IsZero())
			{
				double cstr;
				if(!knotted && !crested)
					cstr = 1;
				else
				{
					cstr = vcomb.Dot(vup);					// -1 in opposite direction, 0 lateral, 1 in direction of comb
					cstr = abs(cstr - 1) / 2;				// 1 in opposite direction, .5 lateral, 0 in direction of comb
					cstr = sin(cstr * PI/2);
				}
				hvec += AuPoint(vcomb * stepLen * cstr);
				hvec.Unit(stepLen);
			}

			double lockRemains = lockLength - acclen;
			if(lockRemains <= 0)
				break;
			double h = stepLen;
			if(h > lockRemains)
			{
				h = lockRemains;
				hvec.Unit(h);
			}

			if(!crested && !knotted)
			{
				// Lock has not crested the head yet, or reached a knot.
				cen = prevCen + hvec;

				// Elevate cen to the skull surface if it is lower than that.
				if(skull)
				{
					if(!ignoreHead)
					{
						if(s == 1)
							cen = skull->RaiseToSurface(cen, headSpace/2);
						else
							cen = skull->RaiseToSurface(cen, headSpace);
						if(combLimit > 0)
							cen = skull->LowerToSurface(cen, combLimit);
					}
					vup = cen - centerRef;
					vup.Unit();

					// Determine if we have crested the head and it is time to fall.
					if(!knotStyle)
					{
						// Test current cen location to see if it is inside the top-down 2D skull, with a rough margin.
						AuPoint testInside(cen - centerRef);
						double margin = headSpace/2;
						if(testInside.x < 0)
							testInside.x += margin;	// margin
						else
							testInside.x -= margin;	// margin
						if(testInside.y < 0)
							testInside.y += margin;	// margin
						else
							testInside.y -= margin;	// margin
						testInside.z = 0;			// For 2D test over skull footprint
						testInside += centerRef;
						crested = !skull->IsInside(testInside);	// Quickie test for clear of skull
					}
				}
				else
				{
					if(!knotStyle)
						crested = lockDir.z < 0;
					vup = cen - centerRef;
					vup.Unit();
				}

				if(knotStyle)
				{
					double vang = vup.ZRotation();
					double vpol = vup.PolarIncline();
					if(fabs(vang - knotAng) < widAng &&
							fabs(vpol - knotPol) < widAng)
					{
						knotted = true;
						vcomb = combDir;
					}
					else
					{
						vcomb = knotLoc - cen;
						vcomb.Unit();
					}
				}

				if(crested || knotted)
				{
					AuPoint svec(cen - prevCen);
					double dist = svec.Length();
					if(dist > hangLength)
					{
						svec = svec / dist * hangLength;
						cen = prevCen + svec;
						hangLenSoFar = hangLength;
					}
					else
						hangLenSoFar = dist;
				}
			}
			else
			{
				double hangRemaining = hangLength - hangLenSoFar;
				if(hangRemaining > 0)
				{
					// Build a knot or not.
					if(knotLength > 0 && hangLenSoFar < knotLength)
					{
						// Knot.
						if(hangRemaining > knotLength)
						{
							cen = prevCen + vup * knotLength;	// Knot goes straight out of the head.
							hangLenSoFar += knotLength;
						}
						else
						{
							cen = prevCen + vup * hangRemaining;	// Knot goes straight out of the head.
							hangLenSoFar += hangRemaining;
						}
					}
					else
					{
						// No knot. Could be done with the knot and falling out now.
						if(h >= hangRemaining)
						{
							h = hangRemaining;
							hvec.Unit(h);
						}
						hangLenSoFar += h;
						cen = prevCen + hvec;

						// Elevate cen to the skull surface if it is lower than that.
						if(skull && !ignoreHead)
						{
							if(s == 1)
								cen = skull->RaiseToSurface(cen, headSpace/2);
							else
								cen = skull->RaiseToSurface(cen, headSpace);
						}
					}
				}
				else
					break;
			}

			if(s > 1 && (fall || !vcomb.IsZero()) && !cen.FuzzyEquals(prevCen))
			{
				// Calculate edges of lock.
				AuPoint vy(cen - prevCen);
				vy.Unit();
				if(!vy.FuzzyEquals(vup))
				{
					vx = vy.Cross(vup);
					if(lockWidth > lockLength)
						vx.Unit(lockLength/2);		// Let the actual hair poly be narrower than the follicle patch.
					else
						vx.Unit(lockWidth/2);
				}
			}
		}

		AuPoint ul(cen - vx);
		AuPoint ur(cen + vx);

		// Apply noise to locs.
		AuPoint sul(ul);
		AuPoint suc(cen);
		AuPoint sur(ur);
		AuPoint sul2(ul);
		AuPoint suc2(cen);
		AuPoint sur2(ur);
		if(s > 2)
		{
			int sn = s - 2;
			float noiseSideToSide = AuNoiseF::PerlinNoise(2, persSideToSide, noiseoff + sn * stepLen * nfreq);
			AuPoint vxn(vx * noiseSideToSide);
			sul += vxn;
			suc += vxn;
			sur += vxn;
			noiseSideToSide = AuNoiseF::PerlinNoise(2, persSideToSide, (noiseoff+5) + sn * stepLen * nfreq);
			vxn = vx * noiseSideToSide;
			sul2 += vxn;
			suc2 += vxn;
			sur2 += vxn;

			float noiseUpAndDown = AuNoiseF::PerlinNoise(2, persUpAndDown, (noiseoff+10) + sn * stepLen * nfreq);
			sul += vup * (headSpace * noiseUpAndDown);
			suc += vup * (headSpace * noiseUpAndDown + headSpace/2);
			sur += vup * (headSpace * noiseUpAndDown);
			noiseUpAndDown = AuNoiseF::PerlinNoise(2, persUpAndDown, (noiseoff+20) + sn * stepLen * nfreq);
			sul2 += vup * (headSpace * noiseUpAndDown);
			suc2 += vup * (headSpace * noiseUpAndDown + headSpace/2);
			sur2 += vup * (headSpace * noiseUpAndDown);
		}

		acclen += prevCen.Distance(cen);
		float sty = acclen / lockLength;
		if(sty > 1)
			sty = 1;

		// Strip 1
		AuHairPoint sult(sul,
				vup,
				shine, reflect, boneIndex, boneWeight,
				lltx, sty,
				hairColor);
		AuHairPoint suct(suc,
				vup,
				shine, reflect, boneIndex, boneWeight,
				lctx, sty,
				hairColor);
		AuHairPoint surt(sur,
				vup,
				shine, reflect, boneIndex, boneWeight,
				lrtx, sty,
				hairColor);

		AuHairPoint sulb(sur,
				-vup,
				shine, reflect, boneIndex, boneWeight,
				lrtx, sty,
				hairColor);
		AuHairPoint sucb(suc,
				-vup,
				shine, reflect, boneIndex, boneWeight,
				lctx, sty,
				hairColor);
		AuHairPoint surb(sul,
				-vup,
				shine, reflect, boneIndex, boneWeight,
				lltx, sty,
				hairColor);

		// Strip 2 - Texture is reversed left/right on this one.
		AuHairPoint sult2(sul2,
				vup,
				shine, reflect, boneIndex, boneWeight,
				lrtx, sty,
				hairColor);
		AuHairPoint suct2(suc2,
				vup,
				shine, reflect, boneIndex, boneWeight,
				lctx, sty,
				hairColor);
		AuHairPoint surt2(sur2,
				vup,
				shine, reflect, boneIndex, boneWeight,
				lltx, sty,
				hairColor);

		AuHairPoint sulb2(sur2,
				-vup,
				shine, reflect, boneIndex, boneWeight,
				lltx, sty,
				hairColor);
		AuHairPoint sucb2(suc2,
				-vup,
				shine, reflect, boneIndex, boneWeight,
				lctx, sty,
				hairColor);
		AuHairPoint surb2(sul2,
				-vup,
				shine, reflect, boneIndex, boneWeight,
				lrtx, sty,
				hairColor);

		if(s == 0)
		{
			if(basePatch)
			{
				// Follicle base patch to cover the skin, to give the appearance of hair density.
				// Elevate cen slightly above the skull surface.
				cen += follicle.m_N * FeetPerMM();
				AuPoint ful(cen - vfolx);
				AuPoint fur(cen + vfolx);
				AuPoint bl = ful - vfoly/2;
				AuPoint br = fur - vfoly/2;
				AuPoint nl = bl + vfoly;
				AuPoint nr = br + vfoly;

				AuHairPoint fp(cen,
						follicle.m_N,
						shine, reflect, boneIndex, boneWeight,
						lctx, .5f,
						hairColor);
				AuHairPoint blt(bl,
						follicle.m_N,
						shine, reflect, boneIndex, boneWeight,
						lltx, 0,
						hairColor);
				AuHairPoint brt(br,
						follicle.m_N,
						shine, reflect, boneIndex, boneWeight,
						lrtx, 0,
						hairColor);
				AuHairPoint nlt(nl,
						follicle.m_N,
						shine, reflect, boneIndex, boneWeight,
						lltx, 1,
						hairColor);
				AuHairPoint nrt(nr,
						follicle.m_N,
						shine, reflect, boneIndex, boneWeight,
						lrtx, 1,
						hairColor);

				AuHairTri t1(follicle, fp, brt, blt, true);
				lockTop.Append(t1);

				AuHairTri t2(follicle, fp, blt, nlt, true);
				lockTop.Append(t2);

				AuHairTri t3(follicle, fp, nlt, nrt, true);
				lockTop.Append(t3);

				AuHairTri t4(follicle, fp, nrt, brt, true);
				lockTop.Append(t4);

				// When making the base patch for a shaved lock, we're done.
				if(style == AuHairStyleLock::Shaved)
					break;
			}
		}
		else
		{
			// Strip 1
			AuHairTri t1t(follicle, sllt, sult, suct, true);
			lockTop.Append(t1t);

			AuHairTri t2t(follicle, suct, slct, sllt, true);
			lockTop.Append(t2t);

			AuHairTri t3t(follicle, slct, suct, surt, true);
			lockTop.Append(t3t);

			AuHairTri t4t(follicle, surt, slrt, slct, true);
			lockTop.Append(t4t);

			AuHairTri t1b(follicle, sllb, sulb, sucb, false);
			lockBottom.Append(t1b);

			AuHairTri t2b(follicle, sucb, slcb, sllb, false);
			lockBottom.Append(t2b);

			AuHairTri t3b(follicle, slcb, sucb, surb, false);
			lockBottom.Append(t3b);

			AuHairTri t4b(follicle, surb, slrb, slcb, false);
			lockBottom.Append(t4b);

			// Strip 2
			AuHairTri t1t2(follicle, sllt2, sult2, suct2, true);
			lockTop.Append(t1t2);

			AuHairTri t2t2(follicle, suct2, slct2, sllt2, true);
			lockTop.Append(t2t2);

			AuHairTri t3t2(follicle, slct2, suct2, surt2, true);
			lockTop.Append(t3t2);

			AuHairTri t4t2(follicle, surt2, slrt2, slct2, true);
			lockTop.Append(t4t2);

			AuHairTri t1b2(follicle, sllb2, sulb2, sucb2, false);
			lockBottom.Append(t1b2);

			AuHairTri t2b2(follicle, sucb2, slcb2, sllb2, false);
			lockBottom.Append(t2b2);

			AuHairTri t3b2(follicle, slcb2, sucb2, surb2, false);
			lockBottom.Append(t3b2);

			AuHairTri t4b2(follicle, surb2, slrb2, slcb2, false);
			lockBottom.Append(t4b2);
		}

		prevCen = cen;
		sllt = sult;
		slct = suct;
		slrt = surt;
		sllb = sulb;
		slcb = sucb;
		slrb = surb;

		sllt2 = sult2;
		slct2 = suct2;
		slrt2 = surt2;
		sllb2 = sulb2;
		slcb2 = sucb2;
		slrb2 = surb2;

		if((s+2) < steps)
		{
			// At least two steps remain.
			// Increase the step size to create less triangles.
			if(crested || knotted)
			{
				if(steps >= 2)
				{
					stepLen *= 2;
					steps /= 2;
					if((s+1) >= steps)
						steps = (s+1) + 1;	// Insure one more pass through the loop.
				}
			}
			else
			{
				if(steps >= 8)
				{
					stepLen *= 2;
					steps /= 2;
					if((s+1) >= steps)
						steps = (s+1) + 1;	// Insure one more pass through the loop.
				}
			}
		}
	}
	lockBottom.SmoothNormals();
	lockTop.SmoothNormals();

	Append(lockBottom);
	Append(lockTop);
}

void AuHairMesh::SmoothNormals()
{
	QMultiMap<AuPoint, AuPoint> verts;	// Key is vertex, value is normal perpendicular to its triangle.
	for(AuHairMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		AuPoint N = (*ti).SurfaceNormalCalc();
		verts.insert((*ti).m_V0, N);
		verts.insert((*ti).m_V1, N);
		verts.insert((*ti).m_V2, N);
	}

	AuPoint curVert;
	AuPoint curN;
	int curCount = 0;
	QMap<AuPoint, AuPoint> smoothVerts;	// Key is vertex, value is smooth normal.
	for(QMultiMap<AuPoint, AuPoint>::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
	{
		if(curCount == 0)
		{
			// First vertex in the mulitimap.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}
		else if(vi.key().FuzzyEquals(curVert))
		{
			// Subsequent matching vertex. Accumulate normals so we can average them.
			curN += vi.value();
			curCount++;
		}
		else
		{
			// Subsequent vertex did not match. Calculate accumulated average of current normal.
			if(curCount > 1)
			{
				curN /= curCount;
				curN.Unit();
			}
			smoothVerts.insert(curVert, curN);

			// Start accumulating next vertex.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}

		if(vi == verts.cend()-1)
		{
			// We just finished the last vertex. Close out our last accumulated vertex normal.
			if(curCount > 1)
			{
				curN /= curCount;
				curN.Unit();
			}
			smoothVerts.insert(curVert, curN);
		}
	}

	// Update all my triangles with new smooth normals.
	for(AuHairMesh::iterator ti = begin(); ti != end(); ti++)
	{
		QMap<AuPoint, AuPoint>::const_iterator si = smoothVerts.find((*ti).m_V0);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V0))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V0.m_N = *si;
		else
			qWarning() << "error: AuHairMesh::SmoothNormals triangle vertex 1 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V1);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V1))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V1.m_N = *si;
		else
			qWarning() << "error: AuHairMesh::SmoothNormals triangle vertex 2 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V2);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V2))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V2.m_N = *si;
		else
			qWarning() << "error: AuHairMesh::SmoothNormals triangle vertex 3 not in smooth vertices map";
	}
}

AuGLVAO AuHairMesh::CreateAnimalHairVAO(AuOpenGLFunctions& re, bool isTop) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	int hcount = 0;
	for(QList<AuHairTri>::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Skip top facing tris in the first half.
		if((*ti).IsTop() != isTop)
			continue;
		hcount++;
	}
	if(!hcount)
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = hcount*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoord = new GLfloat[vertices*2];
	GLfloat* hairColor = new GLfloat[vertices*3];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int bti = 0;
	unsigned int hci = 0;
	for(QList<AuHairTri>::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Skip top facing tris in the first half.
		if((*ti).IsTop() != isTop)
			continue;

		// Vertex 0
		positionShine[pidx++] = (float)(*ti).m_V0.x;
		positionShine[pidx++] = (float)(*ti).m_V0.y;
		positionShine[pidx++] = (float)(*ti).m_V0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V0.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V0.m_HairTy;

		hairColor[hci++] = (float)(*ti).m_V0.m_Color.redF();
		hairColor[hci++] = (float)(*ti).m_V0.m_Color.greenF();
		hairColor[hci++] = (float)(*ti).m_V0.m_Color.blueF();

		// Vertex 1
		positionShine[pidx++] = (float)(*ti).m_V1.x;
		positionShine[pidx++] = (float)(*ti).m_V1.y;
		positionShine[pidx++] = (float)(*ti).m_V1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V1.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V1.m_HairTy;

		hairColor[hci++] = (float)(*ti).m_V1.m_Color.redF();
		hairColor[hci++] = (float)(*ti).m_V1.m_Color.greenF();
		hairColor[hci++] = (float)(*ti).m_V1.m_Color.blueF();

		// Vertex 2
		positionShine[pidx++] = (float)(*ti).m_V2.x;
		positionShine[pidx++] = (float)(*ti).m_V2.y;
		positionShine[pidx++] = (float)(*ti).m_V2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V2.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V2.m_HairTy;

		hairColor[hci++] = (float)(*ti).m_V2.m_Color.redF();
		hairColor[hci++] = (float)(*ti).m_V2.m_Color.greenF();
		hairColor[hci++] = (float)(*ti).m_V2.m_Color.blueF();
	}

	/*
	for(QList<AuHairTri>::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Skip bottom facing tris in the second half.
		if(!(*ti).IsTop())
			continue;

		// Vertex 0
		positionShine[pidx++] = (float)m_V0.x;
		positionShine[pidx++] = (float)m_V0.y;
		positionShine[pidx++] = (float)m_V0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V0.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V0.m_HairTy;

		// Vertex 1
		positionShine[pidx++] = (float)m_V1.x;
		positionShine[pidx++] = (float)m_V1.y;
		positionShine[pidx++] = (float)m_V1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V1.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V1.m_HairTy;

		// Vertex 2
		positionShine[pidx++] = (float)m_V2.x;
		positionShine[pidx++] = (float)m_V2.y;
		positionShine[pidx++] = (float)m_V2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V2.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V2.m_HairTy;
	}
*/

	// Generate vertex array object with a buffer for each array.
	vao.CreateGL(re, vertices, 6);

	vao.SelectGL();

	GLuint usage = GL_STATIC_DRAW;

	// bind buffer and copy data into buffer.
	int bufi = 0;	// Buffer index.
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), positionShine, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, nidx*sizeof(GLfloat), normalReflect, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, bi*sizeof(GLfloat), boneIndex, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, bw*sizeof(GLfloat), boneWeight, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, bti*sizeof(GLfloat), texCoord, usage);
	re.glVertexAttribPointer(bufi, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, hci*sizeof(GLfloat), hairColor, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoord;
	delete [] hairColor;
	return vao;
}

AuGLVAO AuHairMesh::CreateHeadHairVAO(AuOpenGLFunctions& re, bool isTop) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	int hcount = 0;
	for(QList<AuHairTri>::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Skip top facing tris in the first half.
		if((*ti).IsTop() != isTop)
			continue;
		if(!(*ti).IsHead())
			continue;
		hcount++;
	}
	if(!hcount)
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = hcount*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoord = new GLfloat[vertices*2];
	GLfloat* hairColor = new GLfloat[vertices*3];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int bti = 0;
	unsigned int hci = 0;
	for(QList<AuHairTri>::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Skip top facing tris in the first half.
		if((*ti).IsTop() != isTop)
			continue;
		if(!(*ti).IsHead())
			continue;

		// Vertex 0
		positionShine[pidx++] = (float)(*ti).m_V0.x;
		positionShine[pidx++] = (float)(*ti).m_V0.y;
		positionShine[pidx++] = (float)(*ti).m_V0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V0.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V0.m_HairTy;

		hairColor[hci++] = (float)(*ti).m_V0.m_Color.redF();
		hairColor[hci++] = (float)(*ti).m_V0.m_Color.greenF();
		hairColor[hci++] = (float)(*ti).m_V0.m_Color.blueF();

		// Vertex 1
		positionShine[pidx++] = (float)(*ti).m_V1.x;
		positionShine[pidx++] = (float)(*ti).m_V1.y;
		positionShine[pidx++] = (float)(*ti).m_V1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V1.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V1.m_HairTy;

		hairColor[hci++] = (float)(*ti).m_V1.m_Color.redF();
		hairColor[hci++] = (float)(*ti).m_V1.m_Color.greenF();
		hairColor[hci++] = (float)(*ti).m_V1.m_Color.blueF();

		// Vertex 2
		positionShine[pidx++] = (float)(*ti).m_V2.x;
		positionShine[pidx++] = (float)(*ti).m_V2.y;
		positionShine[pidx++] = (float)(*ti).m_V2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V2.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V2.m_HairTy;

		hairColor[hci++] = (float)(*ti).m_V2.m_Color.redF();
		hairColor[hci++] = (float)(*ti).m_V2.m_Color.greenF();
		hairColor[hci++] = (float)(*ti).m_V2.m_Color.blueF();
	}

	/*
	for(QList<AuHairTri>::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Skip bottom facing tris in the second half.
		if(!(*ti).IsTop())
			continue;

		// Vertex 0
		positionShine[pidx++] = (float)m_V0.x;
		positionShine[pidx++] = (float)m_V0.y;
		positionShine[pidx++] = (float)m_V0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V0.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V0.m_HairTy;

		// Vertex 1
		positionShine[pidx++] = (float)m_V1.x;
		positionShine[pidx++] = (float)m_V1.y;
		positionShine[pidx++] = (float)m_V1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V1.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V1.m_HairTy;

		// Vertex 2
		positionShine[pidx++] = (float)m_V2.x;
		positionShine[pidx++] = (float)m_V2.y;
		positionShine[pidx++] = (float)m_V2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.z;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];

		texCoord[bti++] = (float)(*ti).m_V2.m_HairTx;
		texCoord[bti++] = (float)(*ti).m_V2.m_HairTy;
	}
*/

	// Generate vertex array object with a buffer for each array.
	vao.CreateGL(re, vertices, 6);

	vao.SelectGL();

	GLuint usage = GL_STATIC_DRAW;

	// bind buffer and copy data into buffer.
	int bufi = 0;	// Buffer index.
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), positionShine, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, nidx*sizeof(GLfloat), normalReflect, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, bi*sizeof(GLfloat), boneIndex, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, bw*sizeof(GLfloat), boneWeight, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, bti*sizeof(GLfloat), texCoord, usage);
	re.glVertexAttribPointer(bufi, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, hci*sizeof(GLfloat), hairColor, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoord;
	delete [] hairColor;
	return vao;
}

/*
void AuHairMesh::Insert(const AuHairTri& t, const AuHairMesh& sourceMesh, bool combineVerts)
{
	append(t);
	iterator nt = end()-1;

	if(&sourceMesh != this)
	{
		// Convert indices in the new triangle to reference my vertices.
		if(combineVerts)
		{
			(*nt).m_V0.m_VertexIndex = m_Vertices.IndexOf(sourceMesh.Vertex(t.m_V0.Index()));
			if((*nt).m_V0.m_VertexIndex < 0)
				(*nt).m_V0.m_VertexIndex = VertexIndexAdd(sourceMesh.Vertex(t.m_V0.Index()));

			(*nt).m_V1.m_VertexIndex = m_Vertices.IndexOf(sourceMesh.Vertex(t.m_V1.Index()));
			if((*nt).m_V1.m_VertexIndex < 0)
				(*nt).m_V1.m_VertexIndex = VertexIndexAdd(sourceMesh.Vertex(t.m_V1.Index()));

			(*nt).m_V2.m_VertexIndex = m_Vertices.IndexOf(sourceMesh.Vertex(t.m_V2.Index()));
			if((*nt).m_V2.m_VertexIndex < 0)
				(*nt).m_V2.m_VertexIndex = VertexIndexAdd(sourceMesh.Vertex(t.m_V2.Index()));
		}
		else
		{
			(*nt).m_V0.m_VertexIndex = VertexIndexAdd(sourceMesh.Vertex(t.m_V0.Index()));
			(*nt).m_V1.m_VertexIndex = VertexIndexAdd(sourceMesh.Vertex(t.m_V1.Index()));
			(*nt).m_V2.m_VertexIndex = VertexIndexAdd(sourceMesh.Vertex(t.m_V2.Index()));
		}
	}

	if(size() == 1)
		(*nt).Extents(m_Min, m_Max, *this);	// Initialize mesh extents.
	else
	{
		// Expand mesh extents.
		AuPoint min, max;
		(*nt).Extents(min, max, *this);

		m_Min = m_Min.Min(min);
		m_Max = m_Max.Max(max);
	}
}
*/

void AuHairMesh::SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2,
		float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}
}

void AuHairMesh::SetBones(quint8 id, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2,
		float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		if((*pi).Id() != id)
			continue;
		(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}
}

void AuHairMesh::SetBonesRangeZ(double minZ, double maxZ, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1,
		float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		double z = (*pi).m_Follicle.z;
		if(z >= minZ && z <= maxZ)
		{
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}
	}
}

void AuHairMesh::SetBonesRangeYZ(double minY, double maxY, double minZ, double maxZ, quint8 boneIndex0, float boneWeight0,
		quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		double y = (*pi).m_Follicle.y;
		double z = (*pi).m_Follicle.z;
		if(y >= minY && y <= maxY &&
				z >= minZ && z <= maxZ)
		{
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}
	}
}

void AuHairMesh::SetBonesRangeBlendY(double minY, double maxY, quint8 boneMin, quint8 boneMax)
{
	quint8 boneIndex0 = boneMin;
	float boneWeight0 = 0;
	quint8 boneIndex1 = boneMax;
	float boneWeight1 = 0;
	quint8 boneIndex2 = 0;
	float boneWeight2 = 0;
	quint8 boneIndex3 = 0;
	float boneWeight3 = 0;

	double range = maxY - minY;
	double y;
	for(iterator pi = begin(); pi != end(); pi++)
	{
		y = (*pi).m_V0.y;
		if(y >= minY && y <= maxY)
		{
			boneWeight1 = (y - minY)/range;
			boneWeight0 = 1 - boneWeight1;
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}

		y = (*pi).m_V1.y;
		if(y >= minY && y <= maxY)
		{
			boneWeight1 = (y - minY)/range;
			boneWeight0 = 1 - boneWeight1;
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}

		y = (*pi).m_V2.y;
		if(y >= minY && y <= maxY)
		{
			boneWeight1 = (y - minY)/range;
			boneWeight0 = 1 - boneWeight1;
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}
	}
}

void AuHairMesh::AddBoneRangeBlendX(double fromX, double toX, int bone, quint8 boneIndex)
{
	float boneWeight;
	double range = toX - fromX;	// This will be negative when toX is less than fromX.
	double x;
	if(toX > fromX)
	{
		for(iterator pi = begin(); pi != end(); pi++)
		{
			x = (*pi).m_V0.x;
			if(x > fromX && x <= toX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V0.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V1.x;
			if(x > fromX && x <= toX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V1.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V2.x;
			if(x > fromX && x <= toX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V2.AddBone(bone, boneIndex, boneWeight);
			}
		}
	}
	else
	{
		for(iterator pi = begin(); pi != end(); pi++)
		{
			x = (*pi).m_V0.x;
			if(x >= toX && x < fromX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V0.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V1.x;
			if(x >= toX && x < fromX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V1.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V2.x;
			if(x >= toX && x < fromX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V2.AddBone(bone, boneIndex, boneWeight);
			}
		}
	}
}

void AuHairMesh::AddBoneRangeBlendXMouth(double fromX, double toX, int bone, quint8 boneIndex)
{
	float boneWeight;
	double range = toX - fromX;	// This will be negative when toX is less than fromX.
	double x;
	if(toX > fromX)
	{
		for(iterator pi = begin(); pi != end(); pi++)
		{
			if(!AuFolliclePoint::IsMouth((*pi).Role()))
				continue;
			x = (*pi).m_V0.x;
			if(x > fromX && x <= toX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V0.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V1.x;
			if(x > fromX && x <= toX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V1.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V2.x;
			if(x > fromX && x <= toX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V2.AddBone(bone, boneIndex, boneWeight);
			}
		}
	}
	else
	{
		for(iterator pi = begin(); pi != end(); pi++)
		{
			if(!AuFolliclePoint::IsMouth((*pi).Role()))
				continue;
			x = (*pi).m_V0.x;
			if(x >= toX && x < fromX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V0.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V1.x;
			if(x >= toX && x < fromX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V1.AddBone(bone, boneIndex, boneWeight);
			}

			x = (*pi).m_V2.x;
			if(x >= toX && x < fromX)
			{
				boneWeight = (x - fromX)/range;
				(*pi).m_V2.AddBone(bone, boneIndex, boneWeight);
			}
		}
	}
}

void AuHairMesh::SetBonesRangeBlendZ(double minZ, double maxZ, quint8 boneMin, quint8 boneMax)
{
	quint8 boneIndex0 = boneMin;
	float boneWeight0 = 0;
	quint8 boneIndex1 = boneMax;
	float boneWeight1 = 0;
	quint8 boneIndex2 = 0;
	float boneWeight2 = 0;
	quint8 boneIndex3 = 0;
	float boneWeight3 = 0;

	double range = maxZ - minZ;
	double z;
	for(iterator pi = begin(); pi != end(); pi++)
	{
		z = (*pi).m_V0.z;
		if(z >= minZ && z <= maxZ)
		{
			boneWeight1 = (z - minZ)/range;
			boneWeight0 = 1 - boneWeight1;
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}

		z = (*pi).m_V1.z;
		if(z >= minZ && z <= maxZ)
		{
			boneWeight1 = (z - minZ)/range;
			boneWeight0 = 1 - boneWeight1;
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}

		z = (*pi).m_V2.z;
		if(z >= minZ && z <= maxZ)
		{
			boneWeight1 = (z - minZ)/range;
			boneWeight0 = 1 - boneWeight1;
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		}
	}
}

void AuHairMesh::ExpandExtents(const AuHairTri& t)
{
	m_Min = m_Min.Min(t.m_V0);
	m_Min = m_Min.Min(t.m_V1);
	m_Min = m_Min.Min(t.m_V2);

	m_Max = m_Max.Max(t.m_V0);
	m_Max = m_Max.Max(t.m_V1);
	m_Max = m_Max.Max(t.m_V2);
}

void AuHairMesh::RemoveAbove(double z)
{
	iterator ti = begin();
	while(ti != end())
	{
		if((*ti).Above(z))
			ti = erase(ti);
		else
			ti++;
	}
}

void AuHairMesh::RemoveBelow(double z)
{
	iterator ti = begin();
	while(ti != end())
	{
		if((*ti).Below(z))
			ti = erase(ti);
		else
			ti++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

/*
void AuFolliclePointArray::Append(const AuGlobeLL& ll, double altitude, const AuMatrix4x4& m, AuFolliclePoint::ROLE role)
{
	AuPoint n(ll.GlobeLocation(1));
	AuPoint l = m.Project(AuPoint(n * altitude));
	append(AuFolliclePoint(l, n, role));
}
*/

AuHairStyleLock::STYLE AuHairStyleLock::Lock(const AuFolliclePoint& f, float& headSpace, float& lockLen, float& hangLen,
		float& knotLen, float& widAng, AuPoint& combVec, bool& basePatch) const
{
	if(m_StyleLeft == MoustacheComb || m_StyleLeft == MoustacheCombHang)
	{
		double xyang = f.m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
		if(fabs(xyang) > m_PartAngle)
		{
			// Outside the edges of the moustache.
			headSpace = m_HeadSpaceLeft;
			lockLen = 0;
			hangLen = 0;
			knotLen = 0;
			widAng = 0;
			combVec = AuPoint(0,0);
			basePatch = false;
			return Shaved;
		}

		basePatch = m_Base;
		widAng = 0;
		if(xyang < m_WidAngLeft && xyang > -m_WidAngRight)
		{
			combVec = m_CombLeft;
			headSpace = m_HeadSpaceLeft;
			knotLen = m_KnotLeft;
			lockLen = m_LengthLeft;
			if(lockLen < 0 || lockLen > f.MaxLength())
				lockLen = f.MaxLength();
			if(m_HangLeft >= 0)
				hangLen = m_HangLeft;
			else
				hangLen = lockLen;		// Allow the hair to fall to its natural length.
		}
		else
		{
			combVec = m_CombRight;
			headSpace = m_HeadSpaceRight;
			knotLen = m_KnotRight;
			lockLen = m_LengthRight;
			if(lockLen < 0 || lockLen > f.MaxLength())
				lockLen = f.MaxLength();
			if(m_HangRight >= 0)
				hangLen = m_HangRight;
			else
				hangLen = lockLen;		// Allow the hair to fall to its natural length.
		}
		return m_StyleLeft;
	}
	else if(m_StyleLeft == HandlebarCombHang)
	{
		double xyang = f.m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
		basePatch = m_Base;
		widAng = 0;
		if(xyang < m_PartAngle)
		{
			combVec = m_CombLeft;
			combVec = combVec.RotateX(-fabs(xyang));	// Rotate comb vec as the normal rotates around the face.
			headSpace = m_HeadSpaceLeft;
			knotLen = m_KnotLeft;
			lockLen = m_LengthLeft;
			if(lockLen < 0 || lockLen > f.MaxLength())
				lockLen = f.MaxLength();
			if(m_HangLeft >= 0)
				hangLen = m_HangLeft;
			else
				hangLen = lockLen;		// Allow the hair to fall to its natural length.
		}
		else
		{
			combVec = m_CombRight;
			combVec = combVec.RotateX(-fabs(xyang));	// Rotate comb vec as the normal rotates around the face.
			headSpace = m_HeadSpaceRight;
			knotLen = m_KnotRight;
			lockLen = m_LengthRight;
			if(lockLen < 0 || lockLen > f.MaxLength())
				lockLen = f.MaxLength();
			if(m_HangRight >= 0)
				hangLen = m_HangRight;
			else
				hangLen = lockLen;		// Allow the hair to fall to its natural length.
		}
		return m_StyleLeft;
	}
	else if(m_StyleLeft == SoulPatch)
	{
		basePatch = m_Base;
		widAng = 0;
		double zang = f.m_N.PolarIncline();
		if(zang > m_PartAngle)
		{
			combVec = m_CombLeft;
			headSpace = m_HeadSpaceLeft;
			knotLen = m_KnotLeft;
			lockLen = m_LengthLeft;
			if(lockLen < 0 || lockLen > f.MaxLength())
				lockLen = f.MaxLength();
			if(m_HangLeft >= 0)
				hangLen = m_HangLeft;
			else
				hangLen = lockLen;		// Allow the hair to fall to its natural length.
		}
		else
		{
			combVec = m_CombRight;
			headSpace = m_HeadSpaceRight;
			knotLen = m_KnotRight;
			lockLen = m_LengthRight;
			if(lockLen < 0 || lockLen > f.MaxLength())
				lockLen = f.MaxLength();
			if(m_HangRight >= 0)
				hangLen = m_HangRight;
			else
				hangLen = lockLen;		// Allow the hair to fall to its natural length.
		}
		return m_StyleLeft;
	}

	// Decide whether to use left or right parameters.
	bool isLeft;
	if(m_PartHair)
	{
		// Follicles are created on a head that is facing forward down the +Y axis, -Y is behind, +Z is up, +X is right.
		float fang = f.m_N.YRotation();	// 0 is to the right, 90 is straight up, 180 is to the left
		fang -= PIf/2;				// -90 to the right, 0 straight up, 90 to the left
		if(fang < -PIf)
			fang += PIf*2;
		isLeft = fang < m_PartAngle;
	}
	else
		isLeft = true;				// When not parting we use the left parameters.

	basePatch = m_Base;
	if(isLeft)
	{
		combVec = m_CombLeft;
		headSpace = m_HeadSpaceLeft;
		knotLen = m_KnotLeft;
		lockLen = m_LengthLeft;
		if(lockLen < 0 || lockLen > f.MaxLength())
			lockLen = f.MaxLength();
		if(m_HangLeft >= 0)
			hangLen = m_HangLeft;
		else
			hangLen = lockLen;		// Allow the hair to fall to its natural length.
		widAng = m_WidAngLeft;
		return m_StyleLeft;
	}
	else
	{
		combVec = m_CombRight;
		headSpace = m_HeadSpaceRight;
		knotLen = m_KnotRight;
		lockLen = m_LengthRight;
		if(lockLen < 0 || lockLen > f.MaxLength())
			lockLen = f.MaxLength();
		if(m_HangRight >= 0)
			hangLen = m_HangRight;
		else
			hangLen = lockLen;		// Allow the hair to fall to its natural length.
		widAng = m_WidAngRight;
		return m_StyleRight;
	}
}

//static
float AuHairStyleLock::NoiseFrequency(STYLE style)
{
	if(style == Rough)
		return 4;
	return 3;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuHairStyleLock::STYLE AuHairStyle::Lock(const AuFolliclePoint& f, float& headSpace, float& lockLen, float& hangLen, float& knotLen,
		float& widAng, AuPoint& combVec, bool& basePatch) const
{
	//qDebug() << "AuHairStyle::Lock" << f.Role();
	return m_Locks[f.Role()].Lock(f, headSpace, lockLen, hangLen, knotLen, widAng, combVec, basePatch);
}

void AuHairStyle::Init(STYLE style, SHAVE shave, float headSpace, bool isRightHanded)
{
	AuPoint combBack(0,-1,0);
	AuPoint combDown(0,0,-1);
	AuPoint combUp(0,0,1);
	AuPoint combNA;

	// Initialize all locks on the body to unkempt at full natural length.
	AuPoint combRough = AuPoint(combBack * .25).RotateX(45*DEG2RAD);
	for(int role=0; role<AuFolliclePoint::ROLE_Count; role++)
		m_Locks[role].Setup(AuHairStyleLock::Rough, -1, headSpace, -1, combRough, true);

	// Pubic hair
	m_Locks[AuFolliclePoint::Butt].Setup(AuHairStyleLock::Rough, -1, headSpace, -1, combNA, true);
	m_Locks[AuFolliclePoint::Groin].Setup(AuHairStyleLock::Rough, -1, headSpace, -1, combNA, true);

	// Head hair
	AuFolliclePoint::ROLE hairRoles[] =
	{
		AuFolliclePoint::BackOfHead,
		AuFolliclePoint::Bangs,
		AuFolliclePoint::Crown,
		AuFolliclePoint::HeadNeckBack,
		AuFolliclePoint::Scalp,
		AuFolliclePoint::SideLeft,
		AuFolliclePoint::SideRight,
		AuFolliclePoint::SideburnLeft,
		AuFolliclePoint::SideburnRight,
	};
	int hairRoleCount = sizeof(hairRoles)/sizeof(AuFolliclePoint::ROLE);
	switch(style)
	{
	case Bald:
		for(int r=0; r<hairRoleCount; r++)
		{
			AuFolliclePoint::ROLE role = hairRoles[r];
			m_Locks[role].SetupShaved(headSpace, false);
		}
		break;
	case Buzz:
		for(int r=0; r<hairRoleCount; r++)
		{
			AuFolliclePoint::ROLE role = hairRoles[r];
			m_Locks[role].Setup(AuHairStyleLock::Comb, headSpace/2, headSpace, -1, combNA, false);
		}
		break;
	case Close:
		for(int r=0; r<hairRoleCount; r++)
		{
			AuFolliclePoint::ROLE role = hairRoles[r];
			m_Locks[role].SetupShaved(headSpace, true);
		}
		break;
	case Straight:
		for(int r=0; r<hairRoleCount; r++)
		{
			AuFolliclePoint::ROLE role = hairRoles[r];
			m_Locks[role].Setup(AuHairStyleLock::Comb, -1, headSpace, -1, combNA, true);
		}
		break;
	case Unkempt:
		for(int r=0; r<hairRoleCount; r++)
		{
			AuFolliclePoint::ROLE role = hairRoles[r];
			m_Locks[role].Setup(AuHairStyleLock::Rough, -1, headSpace, -1, combBack * .85, true);
		}
		break;

	case CousinIt:
		for(int r=0; r<hairRoleCount; r++)
		{
			AuFolliclePoint::ROLE role = hairRoles[r];
			m_Locks[role].Setup(AuHairStyleLock::CombHang, -1, headSpace, -1, combNA, true);
		}
		break;
	case Short:
		{
			float lenTop = 1.f/12;
			float lenBangs = 1.f/12;
			float hangBangs = 1.f/12;
			float lenBS = .7f/12;
			float lenBurns = .8f/12;
			float hangLen = 1.f/12;
			AuPoint combS = combBack * .6;
			combS.RotateX(-30*DEG2RAD);
			m_Locks[AuFolliclePoint::BackOfHead].Setup(AuHairStyleLock::CombHang, lenBS, headSpace, hangLen, combS, true);
			m_Locks[AuFolliclePoint::Bangs].Setup(AuHairStyleLock::CombHang, lenBangs, headSpace*2, hangBangs, combS, true);
			m_Locks[AuFolliclePoint::Crown].Setup(AuHairStyleLock::CombHang, lenTop, headSpace*2, hangLen, combS, true);
			m_Locks[AuFolliclePoint::HeadNeckBack].SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Scalp].Setup(AuHairStyleLock::CombHang, lenTop, headSpace*2, hangLen, combS, true);
			m_Locks[AuFolliclePoint::SideLeft].Setup(AuHairStyleLock::CombHang, lenBS, headSpace, hangLen, combS, true);
			m_Locks[AuFolliclePoint::SideRight].Setup(AuHairStyleLock::CombHang, lenBS, headSpace, hangLen, combS, true);
			m_Locks[AuFolliclePoint::SideburnLeft].Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combS, true);
			m_Locks[AuFolliclePoint::SideburnRight].Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combS, true);
		}
		break;
	case Regulation:
		{
			float scalpLenL = .65f/12;
			float scalpLenR = .65f/12;
			float crownLenL = .65f/12;
			float crownLenR = .65f/12;
			float bangLenL = .65f/12;
			float bangLenR = .65f/12;
			float backLenL = .6f/12;
			float backLenR = .6f/12;
			float lenSides = .55f/12;
			float lenBurns = .4f/12;
			float hangLen = -1;
			float partAng;
			if(isRightHanded)
				partAng = 40*DEG2RADf;	// scalpangz from AuDNAHead::HairFollicles
			else
				partAng = -40*DEG2RADf;	// scalpangz from AuDNAHead::HairFollicles
			AuPoint combL(AuPoint(-1,0,0).RotateZ(70*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(-20*DEG2RAD);
			AuPoint combR(AuPoint(1,0,0).RotateZ(-70*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(-20*DEG2RAD);
			m_Locks[AuFolliclePoint::Scalp].SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, scalpLenL,
					scalpLenR, headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Crown].SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, crownLenL,
					crownLenR, headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Bangs].SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, bangLenL,
					bangLenR, headSpace, headSpace, hangLen, hangLen, combL, combR, true);

			m_Locks[AuFolliclePoint::BackOfHead].SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb,
					backLenL, backLenR, headSpace, headSpace, hangLen, hangLen, combL, combR, true);

			m_Locks[AuFolliclePoint::HeadNeckBack].SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft].Setup(AuHairStyleLock::HeadComb, lenSides, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideRight].Setup(AuHairStyleLock::HeadComb, lenSides, headSpace, hangLen, combR, true);
			m_Locks[AuFolliclePoint::SideburnLeft].Setup(AuHairStyleLock::HeadComb, lenBurns, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideburnRight].Setup(AuHairStyleLock::HeadComb, lenBurns, headSpace, hangLen, combR, true);
		}
		break;
	case Baltar:
		{
			float scalpLenL = 3.0f/12;
			float scalpLenR = 3.0f/12;
			float crownLenL = 2.0f/12;
			float crownLenR = 2.0f/12;
			float bangLenL = 2.f/12;
			float bangLenR = 2.f/12;
			float backLenL = 6.0f/12;
			float backLenR = 6.0f/12;
			float lenSides = 4.5f/12;
			float lenBurns = .5f/12;
			float hangLen = 2.f/12;
			float partAng = 0;
			AuPoint combL(AuPoint(-1,0,0).RotateZ(70*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(-30*DEG2RAD);
			combL *= .7;
			AuPoint combR(AuPoint(1,0,0).RotateZ(-70*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(-30*DEG2RAD);
			combR *= .7;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, scalpLenL, scalpLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, crownLenL, crownLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, bangLenL, bangLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);

			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, backLenL, backLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);

			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, lenSides, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, lenSides, headSpace, hangLen, combR, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combR, true);
		}
		break;
	case Eowyn:
		{
			float scalpLenL = -1;
			float scalpLenR = -1;
			float crownLenL = -1;
			float crownLenR = -1;
			float bangLenL = 2.f/12.f;
			float bangLenR = 2.f/12.f;
			float backLenL = -1;
			float backLenR = -1;
			float lenSides = -1;
			float lenBurns = -1;
			float hangLen = -1;
			float partAng = 0;
			AuPoint combL(AuPoint(-1,0,0).RotateZ(80*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(10*DEG2RAD);
			AuPoint combR(AuPoint(1,0,0).RotateZ(-80*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(10*DEG2RAD);
			AuPoint bangL(AuPoint(-1,0,0).RotateZ(80*DEG2RAD));	// Comb to the left and back.
			bangL = bangL.RotateX(-15*DEG2RAD);
			AuPoint bangR(AuPoint(1,0,0).RotateZ(-80*DEG2RAD));	// Comb to the right and back.
			bangR = bangR.RotateX(-15*DEG2RAD);
			double scalpC = .85;
			double crownC = .8;
			double bangC = 1.5;
			double backC = .5;
			double sideC = .5;
			double burnC = .5;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, scalpLenL, scalpLenR,
						headSpace, headSpace, hangLen, hangLen, combL*scalpC, combR*scalpC, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, crownLenL, crownLenR,
						headSpace, headSpace, hangLen, hangLen, combL*crownC, combR*crownC, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, bangLenL, bangLenR,
						headSpace, headSpace, hangLen, hangLen, bangL*bangC, bangR*bangC, true);

			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, backLenL, backLenR,
						headSpace, headSpace, hangLen, hangLen, combL*backC, combR*backC, true);

			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, lenSides, headSpace, hangLen, combL*sideC, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, lenSides, headSpace, hangLen, combR*sideC, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combL*burnC, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combR*burnC, true);
		}
		break;
	case Arwen:
		{
			float scalpLenL = -1;
			float scalpLenR = -1;
			float crownLenL = -1;
			float crownLenR = -1;
			float bangLenL = -1;
			float bangLenR = -1;
			float backLenL = -1;
			float backLenR = -1;
			float lenSides = -1;
			float lenBurns = -1;
			float hangLen = -1;
			float partAng = 0;
			float knotLen = 1.5f/12;
			float widAng = 10*DEG2RADf;
			AuPoint combL(AuPoint(-1,0,0).RotateZ(85*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(-40*DEG2RAD);
			AuPoint combR(AuPoint(1,0,0).RotateZ(-85*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(-40*DEG2RAD);
			double scalpC = .8;
			double crownC = .8;
			double bangC = .9;
			double backC = .75;
			double sideC = .8;
			double burnC = .8;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnottedParted(partAng, AuHairStyleLock::HeadKnot, AuHairStyleLock::HeadKnot, scalpLenL, scalpLenR,
						headSpace, headSpace, knotLen, knotLen, hangLen, hangLen, widAng, widAng, combL*scalpC, combR*scalpC, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnottedParted(partAng, AuHairStyleLock::HeadKnot, AuHairStyleLock::HeadKnot, crownLenL, crownLenR,
						headSpace, headSpace, knotLen, knotLen, hangLen, hangLen, widAng, widAng, combL*crownC, combR*crownC, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnottedParted(partAng, AuHairStyleLock::HeadKnot, AuHairStyleLock::HeadKnot, bangLenL, bangLenR,
						headSpace, headSpace, knotLen, knotLen, hangLen, hangLen, widAng, widAng, combL*bangC, combR*bangC, true);

			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, backLenL, backLenR,
						headSpace, headSpace, hangLen, hangLen, combL*backC, combR*backC, true);

			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, lenSides, headSpace, hangLen, combL*sideC, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, lenSides, headSpace, hangLen, combR*sideC, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combL*burnC, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combR*burnC, true);
		}
		break;
	case Elrond:
		{
			float hangLen = -1;			// Hair goes to full natural length.
			const float knotLen = 2.f/12;
			float lockLen = -1;
			float lenBurns = -1;
			float widAng = 5*DEG2RADf;
			AuPoint combE(combBack * .3);
			AuPoint combES(combDown * .8);
			combES += combE;
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combE, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combE, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::HeadComb, lockLen, headSpace, hangLen, combE, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combE, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combE, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combES, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combES, true);
		}
		break;
	case Mandinka:
		{
			float scalpLen = 1.f/12;
			float crownLen = 1.f/12;
			float bangLen = 1.f/12;
			float backLen = .4f/12;
			float sideLen = .35f/12;
			float lenBurns = .3f/12;
			float hangLen = -1;
			float widAng = 10*DEG2RADf;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::Mohawk, scalpLen, headSpace, 0, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::Mohawk, crownLen, headSpace, 0, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::Mohawk, bangLen, headSpace, 0, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::CombHang, backLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combNA, true);
		}
		break;
	case Mohawk:
		{
			float scalpLen = 2.f/12;
			float crownLen = 2.f/12;
			float bangLen = 1.5f/12;
			float lenBack = 2.f/12;
			float hangLen = -1;
			float widAng = 10*DEG2RADf;
			AuPoint combRoo(combBack.RotateX(-60*DEG2RAD) * .5);
			AuPoint combScalp(combBack.RotateX(-70*DEG2RAD) * .6);
			AuPoint combBang(combBack.RotateX(-80*DEG2RAD) * .7);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::MohawkHang, scalpLen, headSpace, 0, hangLen, widAng, combScalp, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::MohawkHang, crownLen, headSpace, 0, hangLen, widAng, combRoo, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::MohawkHang, bangLen, headSpace, 0, hangLen, widAng, combBang, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::MohawkHang, lenBack, headSpace, 0, hangLen, widAng, combRoo, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, false);
		}
		break;
	case MohawkPony:
		{
			float scalpLen = 2.f/12;
			float crownLen = 2.f/12;
			float bangLen = 1.5f/12;
			float lenBack = -1;
			float hangLen = -1;
			float widAng = 10*DEG2RADf;
			AuPoint combRoo(combBack.RotateX(-60*DEG2RAD) * .5);
			AuPoint combScalp(combBack.RotateX(-70*DEG2RAD) * .6);
			AuPoint combBang(combBack.RotateX(-80*DEG2RAD) * .7);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::MohawkHang, scalpLen, headSpace, 0, hangLen, widAng, combScalp, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::MohawkHang, crownLen, headSpace, 0, hangLen, widAng, combRoo, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::MohawkHang, bangLen, headSpace, 0, hangLen, widAng, combBang, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::MohawkHang, lenBack, headSpace, 0, hangLen, widAng, combRoo, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, false);
		}
		break;
	case PonyBangs:
		{
			float hangLen = -1;			// Hair goes to full natural length.
			float lockLen = -1;
			float lenBurns = 1.f/12;
			float lenBangs = 4.f/12;
			float hangBang = 1.5f/12;
			float knotLen = 1.f/12;
			float widAng = 5*DEG2RADf;
			m_Locks[AuFolliclePoint::Bangs]
				.Setup(AuHairStyleLock::CombHang, lenBangs, headSpace, hangBang, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
		}
		break;
	case PonyTail:
		{
			float hangLen = -1;			// Hair goes to full natural length.
			float lockLen = -1;
			float lenBurns = 1.f/12.f;
			float knotLen = 1.25f/12.f;
			float widAng = 5*DEG2RADf;
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
		}
		break;
	case Egghead:
		{
			float crownLen = .45f/12;
			float backLen = .5f/12;
			float sideLen = .5f/12;
			float lenBurns = .4f/12;
			float hangLen = -1;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.Setup(AuHairStyleLock::CombHang, crownLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::CombHang, backLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combNA, true);
		}
		break;
	case Spacely:
		{
			float backLen = .4f/12;
			float sideLen = .35f/12;
			float lenBurns = .3f/12;
			float hangLen = -1;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::CombHang, backLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combNA, true);
		}
		break;
	case Waterfall:
		{
			float scalpLen = -1;
			float crownLen = -1;
			float bangLen = -1;
			float backLen = -1;
			float sideLen = -1;
			float lenBurns = 1.f/12;
			float knotLen = 0;
			float hangLen = -1;
			float widAng = 25*DEG2RADf;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, scalpLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, crownLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::HeadKnot, bangLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::HeadKnot, backLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, sideLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, sideLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combBack, true);
		}
		break;
	case HighAndTight:
		{
			float scalpLenL = .3f/12;
			float scalpLenR = .3f/12;
			float backLen = .1f/12;
			float sideLen = .1f/12;
			float lenBurns = .1f/12;
			float hangLen = .5f/12;
			float partAng;
			if(isRightHanded)
				partAng = 40*DEG2RADf;	// scalpangz from AuDNAHead::HairFollicles
			else
				partAng = -40*DEG2RADf;	// scalpangz from AuDNAHead::HairFollicles
			AuPoint combL(AuPoint(-1,0,0).RotateZ(70*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(-20*DEG2RAD);
			AuPoint combR(AuPoint(1,0,0).RotateZ(-70*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(-20*DEG2RAD);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, scalpLenL, scalpLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::Comb, backLen, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::Comb, sideLen, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::Comb, sideLen, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::Comb, lenBurns, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::Comb, lenBurns, headSpace, hangLen, combNA, false);
		}
		break;
	case Crop:
		{
			float scalpLen = 1.f/12;
			float crownLen = 1.f/12;
			float bangLen = 1.f/12;
			float hangLen = -1;
			float partAng;
			if(isRightHanded)
				partAng = 30*DEG2RADf;
			else
				partAng = -30*DEG2RADf;
			AuPoint combL(AuPoint(-1,0,0).RotateZ(80*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(-50*DEG2RAD);
			AuPoint combR(AuPoint(1,0,0).RotateZ(-80*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(-50*DEG2RAD);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, scalpLen, scalpLen,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, crownLen, crownLen,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, bangLen, bangLen,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, true);
		}
		break;
	case Manager:
		{
			float scalpLenL = .6f/12;
			float scalpLenR = .6f/12;
			float crownLenL = .6f/12;
			float crownLenR = .6f/12;
			float backLenL = .55f/12;
			float backLenR = .55f/12;
			float lenSides = .5f/12;
			float lenBurns = .4f/12;
			float hangLen = -1;
			float partAng;
			if(isRightHanded)
				partAng = 40*DEG2RADf;	// scalpangz from AuDNAHead::HairFollicles
			else
				partAng = -40*DEG2RADf;	// scalpangz from AuDNAHead::HairFollicles
			AuPoint combL(AuPoint(-1,0,0).RotateZ(70*DEG2RAD));	// Comb to the left and back.
			combL = combL.RotateX(-20*DEG2RAD);
			AuPoint combR(AuPoint(1,0,0).RotateZ(-70*DEG2RAD));	// Comb to the right and back.
			combR = combR.RotateX(-20*DEG2RAD);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, scalpLenL, scalpLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, crownLenL, crownLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);

			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupParted(partAng, AuHairStyleLock::HeadComb, AuHairStyleLock::HeadComb, backLenL, backLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);

			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::HeadComb, lenSides, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::HeadComb, lenSides, headSpace, hangLen, combR, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::HeadComb, lenBurns, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::HeadComb, lenBurns, headSpace, hangLen, combR, true);
		}
		break;
	case FrTuck:
		{
			float bangLen = 1.f/12;
			float crownLen = 1.5f/12;
			float burnLen = .8f/12;
			float hangLen = -1;
			m_Locks[AuFolliclePoint::Bangs]
				.Setup(AuHairStyleLock::CombHang, bangLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.Setup(AuHairStyleLock::CombHang, crownLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, burnLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, burnLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
		}
		break;
	case Carrot:
		{
			float scalpLen = 1.5f/12;
			float hangLen = 0;
			AuPoint combCar(combUp * 1.5);
			m_Locks[AuFolliclePoint::Scalp]
				.Setup(AuHairStyleLock::CombHang, scalpLen, headSpace, hangLen, combCar, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, false);
		}
		break;
	case Crown:
		{
			float crownLen = 1.f/12;
			float hangLen = -1;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.Setup(AuHairStyleLock::CombHang, crownLen, headSpace, hangLen, combNA, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, false);
		}
		break;
	case Mentat:
		{
			float crownLenL = 3.f/12;
			float crownLenR = 3.f/12;
			float lenBurns = 1.5f/12;
			float hangLen = -1;
			float partAng = 0;
			AuPoint combL(combUp.RotateY(-20*DEG2RAD));
			AuPoint combR(combUp.RotateY(20*DEG2RAD));
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang, crownLenL, crownLenR,
						headSpace, headSpace, hangLen, hangLen, combL, combR, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combL, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::CombHang, lenBurns, headSpace, hangLen, combR, true);
		}
		break;
	case Flattop:
		{
			float htAboveSkull = .1f/12;
			float backLen = .1f/12;
			float sideLen = .1f/12;
			float lenBurns = .1f/12;
			float hangLen = .5f/12;
			m_Locks[AuFolliclePoint::Scalp]
				.Setup(AuHairStyleLock::HeadFlattop, htAboveSkull, headSpace, hangLen, combUp, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, true);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::Comb, backLen, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::Comb, sideLen, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::Comb, sideLen, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.Setup(AuHairStyleLock::Comb, lenBurns, headSpace, hangLen, combNA, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.Setup(AuHairStyleLock::Comb, lenBurns, headSpace, hangLen, combNA, false);
		}
		break;
	case Bun:
		{
			float hangLen = 1.5f/12;		// Hang length matches knot length so there is no hang from the knot.
			float lockLen = -1;
			float lenBurns = 1.f/12;
			float knotLen = 1.5f/12;
			float widAng = 10*DEG2RADf;
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
		}
		break;
	case BunBangs:
		{
			float hangLen = 1.5f/12;		// Hang length matches knot length so there is no hang from the knot.
			float lockLen = -1;
			float lenBurns = 1.f/12;
			float lenBangs = 4.f/12;
			float hangBang = 1.5f/12;
			float knotLen = 1.5f/12;
			float widAng = 10*DEG2RADf;
			m_Locks[AuFolliclePoint::Bangs]
				.Setup(AuHairStyleLock::CombHang, lenBangs, headSpace, hangBang, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
		}
		break;
	case Bullet:
		{
			float hangLen = 0;
			float lockLen = -1;
			float lenBurns = 1.f/12;
			float knotLen = 0;
			float widAng = 2.f*DEG2RADf;
			m_Locks[AuFolliclePoint::Bangs]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lockLen, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupKnotted(AuHairStyleLock::HeadKnot, lenBurns, headSpace, knotLen, hangLen, widAng, combNA, true);
		}
		break;
	case Fallout:
		{
			float bangLen = 5.f/12;
			float hangLen = -1;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			AuPoint combFS(AuPoint(combDown * .8).RotateX(-40*DEG2RAD));
			AuPoint combFB(combBack * .4);
			AuPoint combBangs;
			if(isRightHanded)
			{
				float sideLenL = 2.f/12;
				float sideLenR = 4.f/12;
				float lenBurnR = 5.5f/12;
				combBangs = AuPoint(1,0,0).RotateZ(-60*DEG2RAD);
				combBangs *= .9;
				AuPoint combOver(AuPoint(1,0,0).RotateZ(-30*DEG2RAD).RotateY(-60*DEG2RAD));
				m_Locks[AuFolliclePoint::SideLeft]
					.Setup(AuHairStyleLock::HeadComb, sideLenL, headSpace, hangLen, combFB, false);
				m_Locks[AuFolliclePoint::SideRight]
					.Setup(AuHairStyleLock::CombHang, sideLenR, headSpace, hangLen, combFS, true);
				m_Locks[AuFolliclePoint::SideburnLeft]
					.SetupShaved(headSpace, false);
				m_Locks[AuFolliclePoint::SideburnRight]
					.Setup(AuHairStyleLock::CombHang, lenBurnR, headSpace, hangLen, combFS, true);
			}
			else
			{
				float sideLenL = 4.f/12;
				float sideLenR = 2.f/12;
				float lenBurnL = 5.5f/12;
				combBangs = AuPoint(-1,0,0).RotateZ(60*DEG2RAD);
				combBangs *= .9;
				AuPoint combOver(AuPoint(-1,0,0).RotateZ(30*DEG2RAD).RotateY(60*DEG2RAD));
				m_Locks[AuFolliclePoint::SideLeft]
					.Setup(AuHairStyleLock::CombHang, sideLenL, headSpace, hangLen, combFS, false);
				m_Locks[AuFolliclePoint::SideRight]
					.Setup(AuHairStyleLock::HeadComb, sideLenR, headSpace, hangLen, combFB, true);
				m_Locks[AuFolliclePoint::SideburnLeft]
					.Setup(AuHairStyleLock::CombHang, lenBurnL, headSpace, hangLen, combFS, true);
				m_Locks[AuFolliclePoint::SideburnRight]
					.SetupShaved(headSpace, false);
			}
			m_Locks[AuFolliclePoint::Bangs]
				.Setup(AuHairStyleLock::HeadComb, bangLen, headSpace, hangLen, combBangs, false);
		}
		break;
	case Curtain:
		{
			float backLen = -1;
			float sideLen = -1;
			float hangLen = -1;
			AuPoint combCurt(combBack * .25);
			m_Locks[AuFolliclePoint::Scalp]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.Setup(AuHairStyleLock::CombHang, backLen, headSpace, hangLen, combCurt, true);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combCurt, true);
			m_Locks[AuFolliclePoint::SideRight]
				.Setup(AuHairStyleLock::CombHang, sideLen, headSpace, hangLen, combCurt, true);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, false);
		}
		break;
	case Manchu:
		{
			float scalpLen = -1;
			float knotLen = 0;
			float hangLen = -1;
			float widAng = 5*DEG2RADf;
			m_Locks[AuFolliclePoint::Scalp]
				.SetupKnotted(AuHairStyleLock::HeadKnot, scalpLen, headSpace, knotLen, hangLen, widAng, combBack, true);
			m_Locks[AuFolliclePoint::Crown]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::Bangs]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::BackOfHead]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::HeadNeckBack]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideRight]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnLeft]
				.SetupShaved(headSpace, false);
			m_Locks[AuFolliclePoint::SideburnRight]
				.SetupShaved(headSpace, false);
		}
		break;
	case STYLE_Count:
		break;
	}

	// Facial hair
	AuFolliclePoint::ROLE whiskerRoles[] =
	{
		AuFolliclePoint::CheekLeft,
		AuFolliclePoint::CheekRight,
		AuFolliclePoint::ChinFront,
		AuFolliclePoint::ChinLeft,
		AuFolliclePoint::ChinRight,
		AuFolliclePoint::ChopLeft,
		AuFolliclePoint::ChopRight,
		AuFolliclePoint::EarLeft,
		AuFolliclePoint::EarMiddle,
		AuFolliclePoint::EarRight,
		AuFolliclePoint::EyeLeft,
		AuFolliclePoint::EyeMiddle,
		AuFolliclePoint::EyeRight,
		AuFolliclePoint::Face,
		AuFolliclePoint::JowlLeft,
		AuFolliclePoint::JowlRight,
		AuFolliclePoint::LipTop,
		AuFolliclePoint::HeadNeckFront,
		AuFolliclePoint::Nose,
	};
	int whiskerRoleCount = sizeof(whiskerRoles)/sizeof(AuFolliclePoint::ROLE);
	AuPoint combVec = AuPoint(0,0,-1).RotateX(-15*DEG2RAD) * .5;	// Common whisker comb vector, mostly down, a little back.
	switch(shave)
	{
	case Clean:
		for(int w=0; w<whiskerRoleCount; w++)
		{
			AuFolliclePoint::ROLE role = whiskerRoles[w];
			m_Locks[role].SetupShaved(headSpace, false);
		}
		break;
	case Stubble:
		{
			float len = headSpace / 3;
			for(int w=0; w<whiskerRoleCount; w++)
			{
				AuFolliclePoint::ROLE role = whiskerRoles[w];
				m_Locks[role].Setup(AuHairStyleLock::Comb, len, 0, len, AuPoint(), false);
			}
		}
		break;
	case CloseWhiskers:
		{
			for(int w=0; w<whiskerRoleCount; w++)
			{
				AuFolliclePoint::ROLE role = whiskerRoles[w];
				m_Locks[role].SetupShaved(headSpace, true);
			}
		}
		break;
	case StraightWhiskers:
		{
			float hangLen = -1;			// Hair goes to full natural length.
			for(int w=0; w<whiskerRoleCount; w++)
			{
				AuFolliclePoint::ROLE role = whiskerRoles[w];
				m_Locks[role].Setup(AuHairStyleLock::Comb, hangLen, headSpace, hangLen, combNA, true);
			}
		}
		break;
	case Unshaven:
		{
			float hangLen = -1;			// Hair goes to full natural length.
			float partAng = 0;
			AuPoint combWhiskL(AuPoint(combBack * .5).RotateZ(-30*DEG2RAD));
			AuPoint combWhiskR(AuPoint(combBack * .5).RotateZ(30*DEG2RAD));
			for(int w=0; w<whiskerRoleCount; w++)
			{
				AuFolliclePoint::ROLE role = whiskerRoles[w];
				m_Locks[role].SetupParted(partAng, AuHairStyleLock::CombHang, AuHairStyleLock::CombHang,
						hangLen, hangLen, headSpace, headSpace, hangLen, hangLen, combWhiskL, combWhiskR, true);
			}
		}
		break;

	case ShapedStubble:
		{
			float len = headSpace / 3;
			for(int w=0; w<whiskerRoleCount; w++)
			{
				AuFolliclePoint::ROLE role = whiskerRoles[w];
				if(role == AuFolliclePoint::CheekLeft ||
						role == AuFolliclePoint::CheekRight ||
						role == AuFolliclePoint::HeadNeckFront ||
						role == AuFolliclePoint::Face ||
						role == AuFolliclePoint::Nose) 
					m_Locks[role].SetupShaved(headSpace, false);
				else
					m_Locks[role].Setup(AuHairStyleLock::Comb, len, 0, len, AuPoint(), false);
			}
		}
		break;
	case RapStandard:
		{
			float len = headSpace / 3;
			for(int w=0; w<whiskerRoleCount; w++)
			{
				AuFolliclePoint::ROLE role = whiskerRoles[w];
				if(role == AuFolliclePoint::CheekLeft ||
						role == AuFolliclePoint::CheekRight ||
						role == AuFolliclePoint::ChopLeft ||
						role == AuFolliclePoint::ChopRight ||
						role == AuFolliclePoint::JowlLeft ||
						role == AuFolliclePoint::JowlRight ||
						role == AuFolliclePoint::HeadNeckFront ||
						role == AuFolliclePoint::Face ||
						role == AuFolliclePoint::Nose) 
					m_Locks[role].SetupShaved(headSpace, false);
				else
					m_Locks[role].Setup(AuHairStyleLock::Comb, len, 0, len, AuPoint(), false);
			}
		}
		break;
	case Hollywoodian:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = 1.f/12;
			float chopLen = .4f/12;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = .5f/12;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 85*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case MuttonChops:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 1.5f/12;
			float jowlLen = 1.25f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);
		}
		break;
	case FrenchFork:
		{
			float moustacheLen = 0;
			float chinLen = .5f/12;
			float chinSidesLen = 3.f/12;
			float chopLen = .4f/12;
			float jowlLen = .3f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combDown);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combDown, combDown, true);
		}
		break;
	case FuManchu:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheCombHang;
			float angleF = 85*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 3.f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case OldDutch:
		{
			float moustacheLen = 0;
			float chinLen = 2.f/12;
			float chinSidesLen = 1.75f/12;
			float chopLen = 2.f/12;
			float jowlLen = 1.5f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);
		}
		break;
	case VanDyke:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = .75f/12;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case ExtendedVanDyke:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = .75f/12;
			float chopLen = .4f/12;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case FriendlyMuttonChops:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 1.5f/12;
			float jowlLen = 1.25f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combDown, combDown, true);
		}
		break;
	case Balbo:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = .75f/12;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 75*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 0;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Klingon:
		{
			float moustacheLen = 0;
			float chinLen = 2.f/12;
			float chinSidesLen = 1.75f/12;
			float chopLen = 0;
			float jowlLen = .75f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheCombHang;
			float angleF = 75*DEG2RADf;
			float angleS = 85*DEG2RADf;
			float lenF = 0;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case ShortBoxedBeard:
		{
			float moustacheLen = 0;
			float chinLen = .5f/12;
			float chinSidesLen = .5f/12;
			float chopLen = .5f/12;
			float jowlLen = .25f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case FullBeard:
		{
			float moustacheLen = 0;
			float chinLen = 1.5f/12;
			float chinSidesLen = 1.5f/12;
			float chopLen = 1.5f/12;
			float jowlLen = .5f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Goatee:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = .75f/12;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);
		}
		break;
	case ExtendedGoatee:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = .75f/12;
			float chopLen = .4f/12;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);
		}
		break;
	case ChinStrap:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = .4f/12;
			float jowlLen = .4f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);
		}
		break;
	case ChinCurtain:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 1.25f/12;
			float jowlLen = 1.25f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);
		}
		break;
	case Zappa:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Soul patch
			float angleL = -30*DEG2RADf;
			float lenU = .4f/12;
			float lenL = 0;
			m_Locks[AuFolliclePoint::ChinFront]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);

			// Moustache
			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 55*DEG2RADf;
			float angleS = 85*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.25f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case SoulPatch:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Soul patch
			float angleL = -30*DEG2RADf;
			float lenU = .4f/12;
			float lenL = 0;
			m_Locks[AuFolliclePoint::ChinFront]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Winnfield:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = .5f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 85*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 3.f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case PetitGoatee:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			float angleL = -30*DEG2RADf;
			float lenU = 1.5f/12;
			float lenL = 0;
			m_Locks[AuFolliclePoint::ChinFront]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case FranzJosef:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = .4f/12;
			float jowlLen = .4f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 1.f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Anchor:
		{
			float moustacheLen = 0;
			float chinLen = .5f/12;
			float chinSidesLen = 0;
			float chopLen = .5f/12;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 45*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 0;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Napoleon:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			float angleL = -30*DEG2RADf;
			float lenU = 3.f/12;
			float lenL = 0;
			m_Locks[AuFolliclePoint::ChinFront]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);

			// Handlebar moustache
			float len = .5f/12;
			m_Locks[AuFolliclePoint::LipTop].SetupHandlebar(len, headSpace);
		}
		break;
	case Copstash:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheCombHang;
			float angleF = 85*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .6f/12;
			float lenS = 0;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Gandalf:
		{
			float moustacheLen = 0;
			float chinLen = 4.5f/12;
			float chinSidesLen = 4.5f/12;
			float chopLen = 4.0f/12;
			float jowlLen = 4.0f/12;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheCombHang;
			float angleF = 80*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 2.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Confucius:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Soul patch
			float angleL = -30*DEG2RADf;
			float lenU = 4.5f/12;
			float lenL = 0;
			m_Locks[AuFolliclePoint::ChinFront]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);

			// Moustache
			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheCombHang;
			float angleF = 70*DEG2RADf;
			float angleS = 80*DEG2RADf;
			float lenF = 0;
			float lenS = 4.f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Boris:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Soul patch
			float angleL = -30*DEG2RADf;
			float lenU = 1.5f/12;
			float lenL = 0;
			m_Locks[AuFolliclePoint::ChinFront]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);

			// Moustache
			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 70*DEG2RADf;
			float angleS = 80*DEG2RADf;
			float lenF = 0;
			float lenS = 0.5f/12;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Toothbrush:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combDown);

			// Moustache
			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 20*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 0;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combDown, combDown, true);
		}
		break;
	case ChinPuff:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);
		}
		break;
	case Stripe:
		{
			float moustacheLen = 0;
			float chinLen = .5f/12;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Moustache
			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::MoustacheComb;
			float angleF = 20*DEG2RADf;
			float angleS = 90*DEG2RADf;
			float lenF = .4f/12;
			float lenS = 0;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupMoustache(mstyle, mstyle, angleF, angleS, lenF, lenS, headSpace, headSpace, -1, -1, combVec, combVec, true);
		}
		break;
	case Pencil:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			float angleL = 15*DEG2RADf;
			float lenU = 0;
			float lenL = headSpace;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, false);
		}
		break;
	case PencilPatch:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			// Soul patch
			{
				float angleL = -30*DEG2RADf;
				float lenU = .4f/12;
				float lenL = 0;
				m_Locks[AuFolliclePoint::ChinFront]
					.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, true);
			}

			// Pencil moustache.
			{
				float angleL = 15*DEG2RADf;
				float lenU = 0;
				float lenL = headSpace;
				m_Locks[AuFolliclePoint::LipTop]
					.SetupSoulPatch(angleL, lenU, lenL, headSpace, headSpace, -1, -1, combVec, combVec, false);
			}
		}
		break;
	case Handlebar:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			// Handlebar moustache
			float len = .5f/12;
			m_Locks[AuFolliclePoint::LipTop].SetupHandlebar(len, headSpace);
		}
		break;
	case HandlebarAndChinPuff:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Handlebar moustache
			float len = .5f/12;
			m_Locks[AuFolliclePoint::LipTop].SetupHandlebar(len, headSpace);
		}
		break;
	case HandlebarAndGoatee:
		{
			float moustacheLen = 0;
			float chinLen = 1.f/12;
			float chinSidesLen = .75f/12;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::CombHang, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combVec);

			// Handlebar moustache
			float len = .5f/12;
			m_Locks[AuFolliclePoint::LipTop].SetupHandlebar(len, headSpace);
		}
		break;
	case Wally:
		{
			float moustacheLen = 0;
			float chinLen = 0;
			float chinSidesLen = 0;
			float chopLen = 0;
			float jowlLen = 0;
			float cheekLen = 0;
			float neckLen = 0;
			StyleFacialHair(AuHairStyleLock::Shaved, moustacheLen, chinLen, chinSidesLen, chopLen, jowlLen, cheekLen, neckLen,
					headSpace, combNA);

			// Long moustache
			AuHairStyleLock::STYLE mstyle = AuHairStyleLock::CombHang;
			float len = 3.5f/12;
			AuPoint combL = AuPoint(-1,0,0).RotateZ(10*RAD2DEG) * .75;
			AuPoint combR = AuPoint(1,0,0).RotateZ(-10*RAD2DEG) * .75;
			m_Locks[AuFolliclePoint::LipTop]
				.SetupParted(0, mstyle, mstyle, len, len, headSpace, headSpace, -1, -1, combL, combR, true);
		}
		break;
	case SHAVE_Count:
		break;
	}
}

void AuHairStyle::StyleFacialHair(AuHairStyleLock::STYLE style, float moustacheLen, float chinLen, float chinSidesLen,
		float chopLen, float jowlLen, float cheekLen, float neckLen, float headSpace, const AuPoint& combVec)
{
	// Moustache
	m_Locks[AuFolliclePoint::LipTop]
		.Setup(style, moustacheLen, headSpace, moustacheLen, combVec, true);

	// Beard
	m_Locks[AuFolliclePoint::ChinFront]
		.Setup(style, chinLen, headSpace, chinLen, combVec, true);
	m_Locks[AuFolliclePoint::ChinLeft]
		.Setup(style, chinSidesLen, headSpace, chinSidesLen, combVec, true);
	m_Locks[AuFolliclePoint::ChinRight]
		.Setup(style, chinSidesLen, headSpace, chinSidesLen, combVec, true);
	m_Locks[AuFolliclePoint::ChopLeft]
		.Setup(style, chopLen, headSpace, chopLen, combVec, true);
	m_Locks[AuFolliclePoint::ChopRight]
		.Setup(style, chopLen, headSpace, chopLen, combVec, true);
	m_Locks[AuFolliclePoint::JowlLeft]
		.Setup(style, jowlLen, headSpace, jowlLen, combVec, true);
	m_Locks[AuFolliclePoint::JowlRight]
		.Setup(style, jowlLen, headSpace, jowlLen, combVec, true);

	// Cheeks
	m_Locks[AuFolliclePoint::CheekLeft]
		.Setup(style, cheekLen, headSpace, cheekLen, combVec, true);
	m_Locks[AuFolliclePoint::CheekRight]
		.Setup(style, cheekLen, headSpace, cheekLen, combVec, true);

	// Neck
	m_Locks[AuFolliclePoint::HeadNeckFront]
		.Setup(style, neckLen, headSpace, neckLen, combVec, true);

	// Face and nose
	m_Locks[AuFolliclePoint::Face]
		.Setup(style, 0, headSpace, 0, combVec, true);
	m_Locks[AuFolliclePoint::Nose]
		.Setup(style, 0, headSpace, 0, combVec, true);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
void AuFolliclePoint::Roles(QMap<QString, ROLE>& roleMap)
{
	roleMap.insert(QObject::tr("Back of Head"), BackOfHead);
	roleMap.insert(QObject::tr("Bangs"), Bangs);
	roleMap.insert(QObject::tr("Crown"), Crown);
	roleMap.insert(QObject::tr("Scalp"), Scalp);
	roleMap.insert(QObject::tr("Side Left"), SideLeft);
	roleMap.insert(QObject::tr("Side Right"), SideRight);
	roleMap.insert(QObject::tr("Sideburn Left"), SideburnLeft);
	roleMap.insert(QObject::tr("Sideburn Right"), SideburnRight);
	roleMap.insert(QObject::tr("Neck Back"), HeadNeckBack);

	roleMap.insert(QObject::tr("Neck Front"), HeadNeckFront);
	roleMap.insert(QObject::tr("Cheek Left"), CheekLeft);
	roleMap.insert(QObject::tr("Cheek Right"), CheekRight);
	roleMap.insert(QObject::tr("Chin Front"), ChinFront);
	roleMap.insert(QObject::tr("Chin Left"), ChinLeft);
	roleMap.insert(QObject::tr("Chin Right"), ChinRight);
	roleMap.insert(QObject::tr("Ear Left"), EarLeft);
	roleMap.insert(QObject::tr("Ear Middle"), EarMiddle);
	roleMap.insert(QObject::tr("Ear Right"), EarRight);
	roleMap.insert(QObject::tr("Eye Left"), EyeLeft);
	roleMap.insert(QObject::tr("Eye Middle"), EyeMiddle);
	roleMap.insert(QObject::tr("Eye Right"), EyeRight);
	roleMap.insert(QObject::tr("Face"), Face);
	roleMap.insert(QObject::tr("Lip Top"), LipTop);
	roleMap.insert(QObject::tr("Chop Left"), ChopLeft);
	roleMap.insert(QObject::tr("Chop Right"), ChopRight);
	roleMap.insert(QObject::tr("Jowl Left"), JowlLeft);
	roleMap.insert(QObject::tr("Jowl Right"), JowlRight);
	roleMap.insert(QObject::tr("Nose"), Nose);

	roleMap.insert(QObject::tr("Neck Backside"), NeckBack);
	roleMap.insert(QObject::tr("Neck Underside"), NeckUnder);
	roleMap.insert(QObject::tr("Neck Sides"), NeckSides);

	roleMap.insert(QObject::tr("Shoulders"), Shoulders);
	roleMap.insert(QObject::tr("Back"), Back);
	roleMap.insert(QObject::tr("Chest"), Chest);
	roleMap.insert(QObject::tr("Flanks Back"), FlanksBack);
	roleMap.insert(QObject::tr("Flanks Front"), FlanksUnder);
	roleMap.insert(QObject::tr("Butt"), Butt);
	roleMap.insert(QObject::tr("Groin"), Groin);

	roleMap.insert(QObject::tr("Arm Backside"), ArmBack);
	roleMap.insert(QObject::tr("Arm Underside"), ArmUnder);
	roleMap.insert(QObject::tr("Wrist Backside"), WristBack);
	roleMap.insert(QObject::tr("Wrist Underside"), WristUnder);
	roleMap.insert(QObject::tr("Hands"), Hands);
	roleMap.insert(QObject::tr("Palms"), Palms);

	roleMap.insert(QObject::tr("Leg Backside"), LegBack);
	roleMap.insert(QObject::tr("Leg Underside"), LegUnder);
	roleMap.insert(QObject::tr("Ankle Backside"), AnkleBack);
	roleMap.insert(QObject::tr("Ankle Underside"), AnkleUnder);
	roleMap.insert(QObject::tr("Feet"), Feet);
	roleMap.insert(QObject::tr("Soles"), Soles);

	roleMap.insert(QObject::tr("Tail Backside"), TailBack);
	roleMap.insert(QObject::tr("Tail Underside"), TailUnder);
	roleMap.insert(QObject::tr("Tail Tip Backside"), TailTipBack);
	roleMap.insert(QObject::tr("Tail Tip Underside"), TailTipUnder);

	Q_ASSERT(roleMap.size() == ROLE_Count);
}

//static
QStringList AuFolliclePoint::RoleNames(quint64 roleBits)
{
	QStringList names;
	if(roleBits & RoleBit(BackOfHead)) names << QObject::tr("Back of Head");
	if(roleBits & RoleBit(Bangs)) names << QObject::tr("Bangs");
	if(roleBits & RoleBit(Scalp)) names << QObject::tr("Crown");
	if(roleBits & RoleBit(Scalp)) names << QObject::tr("Scalp");
	if(roleBits & RoleBit(SideLeft)) names << QObject::tr("Side Left");
	if(roleBits & RoleBit(SideRight)) names << QObject::tr("Side Right");
	if(roleBits & RoleBit(SideburnLeft)) names << QObject::tr("Sideburn Left");
	if(roleBits & RoleBit(SideburnRight)) names << QObject::tr("Sideburn Right");
	if(roleBits & RoleBit(HeadNeckBack)) names << QObject::tr("Neck Back");

	if(roleBits & RoleBit(HeadNeckFront)) names << QObject::tr("Neck Front");
	if(roleBits & RoleBit(CheekLeft)) names << QObject::tr("Cheek Left");
	if(roleBits & RoleBit(CheekRight)) names << QObject::tr("Cheek Right");
	if(roleBits & RoleBit(ChinFront)) names << QObject::tr("Chin Front");
	if(roleBits & RoleBit(ChinLeft)) names << QObject::tr("Chin Left");
	if(roleBits & RoleBit(ChinRight)) names << QObject::tr("Chin Right");
	if(roleBits & RoleBit(EarLeft)) names << QObject::tr("Ear Left");
	if(roleBits & RoleBit(EarMiddle)) names << QObject::tr("Ear Middle");
	if(roleBits & RoleBit(EarRight)) names << QObject::tr("Ear Right");
	if(roleBits & RoleBit(EyeLeft)) names << QObject::tr("Eye Left");
	if(roleBits & RoleBit(EyeMiddle)) names << QObject::tr("Eye Middle");
	if(roleBits & RoleBit(EyeRight)) names << QObject::tr("Eye Right");
	if(roleBits & RoleBit(Face)) names << QObject::tr("Face");
	if(roleBits & RoleBit(LipTop)) names << QObject::tr("Lip Top");
	if(roleBits & RoleBit(ChopLeft)) names << QObject::tr("Chop Left");
	if(roleBits & RoleBit(ChopRight)) names << QObject::tr("Chop Right");
	if(roleBits & RoleBit(JowlLeft)) names << QObject::tr("Jowl Left");
	if(roleBits & RoleBit(JowlRight)) names << QObject::tr("Jowl Right");
	if(roleBits & RoleBit(Nose)) names << QObject::tr("Nose");

	if(roleBits & RoleBit(NeckBack)) names << QObject::tr("Neck Backside");
	if(roleBits & RoleBit(NeckUnder)) names << QObject::tr("Neck Underside");
	if(roleBits & RoleBit(NeckSides)) names << QObject::tr("Neck Sides");

	if(roleBits & RoleBit(Shoulders)) names << QObject::tr("Shoulders");
	if(roleBits & RoleBit(Back)) names << QObject::tr("Back");
	if(roleBits & RoleBit(Chest)) names << QObject::tr("Chest");
	if(roleBits & RoleBit(FlanksBack)) names << QObject::tr("Flanks Back");
	if(roleBits & RoleBit(FlanksUnder)) names << QObject::tr("Flanks Front");
	if(roleBits & RoleBit(Butt)) names << QObject::tr("Butt");
	if(roleBits & RoleBit(Groin)) names << QObject::tr("Groin");

	if(roleBits & RoleBit(ArmBack)) names << QObject::tr("Arm Backside");
	if(roleBits & RoleBit(ArmUnder)) names << QObject::tr("Arm Underside");
	if(roleBits & RoleBit(WristBack)) names << QObject::tr("Wrist Backside");
	if(roleBits & RoleBit(WristUnder)) names << QObject::tr("Wrist Underside");
	if(roleBits & RoleBit(Hands)) names << QObject::tr("Hands");
	if(roleBits & RoleBit(Palms)) names << QObject::tr("Palms");

	if(roleBits & RoleBit(LegBack)) names << QObject::tr("Leg Backside");
	if(roleBits & RoleBit(LegUnder)) names << QObject::tr("Leg Underside");
	if(roleBits & RoleBit(AnkleBack)) names << QObject::tr("Ankle Backside");
	if(roleBits & RoleBit(AnkleUnder)) names << QObject::tr("Ankle Underside");
	if(roleBits & RoleBit(Feet)) names << QObject::tr("Feet");
	if(roleBits & RoleBit(Soles)) names << QObject::tr("Soles");

	if(roleBits & RoleBit(TailBack)) names << QObject::tr("Tail Backside");
	if(roleBits & RoleBit(TailUnder)) names << QObject::tr("Tail Underside");
	if(roleBits & RoleBit(TailTipBack)) names << QObject::tr("Tail Tip Backside");
	if(roleBits & RoleBit(TailTipUnder)) names << QObject::tr("Tail Tip Underside");

	names.sort();
	return names;
}
