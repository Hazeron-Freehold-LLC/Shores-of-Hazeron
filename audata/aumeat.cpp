//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumeat.cpp,v 1.95 2022/02/19 18:20:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumeat.h"
//
#include "auglobell.h"
#include "aunoise.h"
#include "ausolid.h"

#define d_FacetFace						0						// 1 results in faceted faces, not smoothed.
#define d_MakeHair						1						// 1 results in faceted faces, not smoothed.
#define d_MaxTriangleSize				(50. * FeetPerMM())		// Rough size of largest triangles we create when making meshes.
#define d_TriangleAltTol				(.5 * FeetPerMM())		// Altitude above/below a triangle considered to be IN the triangle.
//#define d_TriangleAngTol				(.05234)// sin(3 deg)	// sin of the angle of segment to triangle N to be ok to clip it.
#define d_TriangleAngTol				(.01745)// sin(1 deg)	// sin of the angle of segment to triangle N to be ok to clip it.
#define d_TriangleTeeTol				(.25 * FeetPerMM())		// Distance to find intersections when cleaning up tees.
#define d_VertexMin						(0)						// This is the granularity of the vertex mesh while building it.
#define d_VertexRound					(FUZZ*100.)				// Final granularity of vertex mesh when finished joining.

void AuMeatPoint::AddBone(int bone, quint8 boneIndex, float boneWeight)
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

void AuMeatProfile::Load(const QString& fileName, float length, bool lenBetweenJoints, float triSize, float width, float height)
{
	if(triSize < d_MaxTriangleSize)
		m_TriangleSize = triSize;
	else
		m_TriangleSize = d_MaxTriangleSize;

	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: AuMeatProfile::Load" << fileName << "failed to open file";
		return;
	}

	AuPointArray meatProfileFront;
	AuPointArray meatJointsFront;
	AuPointArray meatProfileSide;
	AuPointArray meatJointsSide;

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_4_1);
	quint8 fileType, fileVer;
	ds >> fileType;
	ds >> fileVer;
	if(fileType == MEATPROFILE_FILE_TYPE && fileVer == MEATPROFILE_FILE_VER)
	{
		// The meat profile data.
		ds >> meatProfileFront;
		ds >> meatJointsFront;
		ds >> meatProfileSide;
		ds >> meatJointsSide;
		// Additional information.
		//jds >> m_MeatProfileImage;
		//jds >> m_Model;
	}

	file.close();

	if(meatProfileFront.isEmpty())
	{
		qWarning() << "error: AuMeatProfile::Load" << fileName << "no front profile";
		return;
	}
	else if(meatJointsFront.isEmpty())
	{
		qWarning() << "error: AuMeatProfile::Load" << fileName << "no front joints";
		return;
	}
	if(meatProfileSide.isEmpty())
	{
		qWarning() << "error: AuMeatProfile::Load" << fileName << "no side profile";
		return;
	}
	else if(meatJointsSide.isEmpty())
	{
		qWarning() << "error: AuMeatProfile::Load" << fileName << "no side joints";
		return;
	}
	else if(meatJointsSide.size() != meatJointsFront.size())
	{
		qWarning() << "error: AuMeatProfile::Load" << fileName << "unequal front/side joints. front joints"
			<< meatJointsFront.size() << "side joints" << meatJointsSide.size();
		return;
	}

	// Offset the meat profile boundaries to their origins. The origin is the first joint.
	meatProfileFront.Move(-meatJointsFront[0]);
	meatProfileSide.Move(-meatJointsSide[0]);

	// Determine the direction vector along which we will integrate each meat profile boundary.
	// They are expected to depict straight limbs but they could be entered at any angle. We determine the angle by using
	// the vector from the first joint to the second joint.
	AuPoint vecF;
	if(meatJointsFront.size() > 1)
	{
		vecF = meatJointsFront[1] - meatJointsFront[0];
		vecF.Unit();
	}
	else
		vecF = AuPoint(0,-1);
	AuPoint perpF = vecF.PerpendicularVector_2D();

	AuPoint vecS;
	if(meatJointsSide.size() > 1)
	{
		vecS = meatJointsSide[1] - meatJointsSide[0];
		vecS.Unit();
	}
	else
		vecS = AuPoint(0,-1);
	AuPoint perpS = vecS.PerpendicularVector_2D();

	// These min and max values tell us how far from negative to positive we have to integrate along their respective vectors.
	// We would expect the min and max from each boundary to be fairly close to each other.
	double minF,maxF;
	meatProfileFront.DotMinMax(vecF, minF, maxF);
	double minS,maxS;
	meatProfileSide.DotMinMax(vecS, minS, maxS);
	//qDebug() << "front dot vec" << vecF << "min" << minF << "max" << maxF;
	//qDebug() << "side dot vec" << vecS << "min" << minS << "max" << maxS;

	// Once we go off the end of either boundary, we're done.
	double startD;	// Start at the largest minimum.
	if(minF > minS)
		startD = minF;
	else
		startD = minS;

	double endD;	// End at the smallest maximum.
	if(maxF < maxS)
		endD = maxF;
	else
		endD = maxS;

	// Decide how big of steps to take along the way.
	double len = endD - startD;
	int steps = len / m_TriangleSize;
	if(steps > 100)
		steps = 100;
	double steplen = len / steps;

	//qDebug() << fileName << "startD" << startD << "endD" << endD << "len" << len << "steps" << steps << "stepLen" << steplen;

	// Construct meat shapes by integrating along the front and side profile boundaries to get the four radii we need at each step.
	clear();
	insert(startD, AuMeatShape(0, 0, 0, 0));
	for(int step=0; step<steps; step++)
	{
		double stepoff = startD + steplen * step;

		// Get intersections along the front boundary.
		AuPoint locF = vecF * stepoff;	// Location along the center line of the profile, from the origin.
		AuPoint locFp = locF + perpF;	// Perpendicular location so we can find intersections.

		AuPointArray intF;
		meatProfileFront.IntersectBoundaryLine_2D(locF, locFp, intF);
		if(intF.size() < 2)
		{
			//qDebug() << "step" << step << "hit front profile only" << intF.size() << "times";
			continue;
		}

		double front0 = (intF[0] - locF).Dot(perpF);
		double front1 = (intF[1] - locF).Dot(perpF);

		// Get intersections along the side boundary.
		AuPoint locS = vecS * stepoff;	// Location along the center line of the profile, from the origin.
		AuPoint locSp = locS + perpS;	// Perpendicular location so we can find intersections.

		AuPointArray intS;
		meatProfileSide.IntersectBoundaryLine_2D(locS, locSp, intS);
		if(intS.size() < 2)
		{
			//qDebug() << "step" << step << "hit side profile only" << intF.size() << "times";
			continue;
		}

		double side0 = (intS[0] - locS).Dot(perpS);
		double side1 = (intS[1] - locS).Dot(perpS);

		// Determine which is the bottom, top, left and right.
		float bottom, top;
		if(side0 > side1)
		{
			bottom = fabsf(side0);
			top = fabsf(side1);
		}
		else
		{
			bottom = fabsf(side1);
			top = fabsf(side0);
		}

		float left, right;
		if(front0 > front1)
		{
			left = fabsf(front0);
			right = fabsf(front1);
		}
		else
		{
			left = fabsf(front1);
			right = fabsf(front0);
		}

		//qDebug() << "off" << stepoff << "bt" << bottom << top << "lr" << left << right;
		insert(stepoff, AuMeatShape(bottom, top, left, right));
	}
	insert(endD, AuMeatShape(0, 0, 0, 0));

	// Scale the result to the desired length.
	double scaleY;
	if(lenBetweenJoints)
		scaleY = length / meatJointsFront.first().Distance_2D(meatJointsFront.last());
	else
		scaleY = length / endD;		// The caller wants the length to be from the attachment joint to the tip of the limb.

	double scaleX;
	if(width > 0)
		scaleX = width / MaxWidth();
	else
		scaleX = scaleY;

	double scaleZ;
	if(height > 0)
		scaleZ = height / MaxHeight();
	else
		scaleZ = scaleY;

	Scale(scaleX, scaleY, scaleZ, 1);

	// Joints.
	m_Joints.clear();
	AuPoint y0 = meatJointsFront.first();
	for(AuPointArray::const_iterator ji = meatJointsFront.cbegin(); ji != meatJointsFront.cend(); ji++)
	{
		double dotF = ((*ji) - y0).Dot(vecF) * scaleY;
		m_Joints.append(AuPoint(0, dotF));
	}

	//{
		//AuPoint min,max;
		//meatProfileFront.Extents(min, max);
		//qDebug() << "front profile extents" << min << max << "scaled to inches" << min*scale*12 << max*scale*12;
	//}
}

void AuMeatProfile::GetMesh(AuMeatMesh& mesh, AuMeatTri::PART part, int tex, const AuPoint& upVec, float blendAngle,
		float blendWid) const
{
	// We will step along the length of this meat profile creating bands of triangles.
	float len = Length();
	int bands = len / m_TriangleSize;
	if(bands < 2)
		bands = 2;						// Minimal tesselation.
	else if(bands > 100)
		bands = 100;
	float step = len / bands;

	// We are going to make rings of triangles at each step.
	float rod = RoundOffDistance();
	int ringVerts = rod*2*PI / step;	// Rough circumference divided by step gives number of band increments.
	if(ringVerts < 4)
		ringVerts = 4;					// Enough to make a tube.
	else
		ringVerts &= 0xfffffffc;		// Insure ringVerts is divisible by four, because we make our rings in quadrants.

	// Create the starting end cap.
	float startD = begin().key();
	float texYc = startD / len;
	AuPointArray pring;
	VertexRingAt(startD, ringVerts, pring);
	TriangleFan(mesh, part, pring, AuPoint(0,startD), false, tex, upVec, blendAngle, blendWid, texYc);
	float texYp = texYc;

	// Create the intermediate bands.
	for(int band=0; band<=bands; band++)
	{
		// Get the ring around the meat at the current offset.
		AuPointArray nring;
		if(band == 0)
		{
			// First band we'll take a half step and do an extra band, otherwise the fan at the end is huge or we get a pointy tip.
			float offset = startD + step/2;
			texYc = offset / len;
			VertexRingAt(offset, ringVerts, nring);
		}
		else if(band == bands)
		{
			// Last band we'll take a half step and do an extra band, otherwise the fan at the end is huge or we get a pointy tip.
			float offset = startD + step * band - step/2;
			texYc = offset / len;
			VertexRingAt(offset, ringVerts, nring);

			// Create the triangle band that connects the previous ring with the current ring.
			TriangleBand(mesh, part, pring, nring, tex, upVec, blendAngle, blendWid, texYp,texYc);

			// Step to the final ring to end the pass.
			pring = nring;
			nring.clear();
			offset = (end()-1).key();
			texYc = 1;
			VertexRingAt(offset, ringVerts, nring);
		}
		else
		{
			float offset = startD + step * band;
			texYc = offset / len;
			VertexRingAt(offset, ringVerts, nring);
		}

		// Create the triangle band that connects the previous ring with the current ring.
		TriangleBand(mesh, part, pring, nring, tex, upVec, blendAngle, blendWid, texYp,texYc);

		// Step to the new ring for the next pass.
		pring = nring;
		texYp = texYc;
	}

	// Create the closing end cap.
	TriangleFan(mesh, part, pring, AuPoint(0,(end()-1).key()), true, tex, upVec, blendAngle, blendWid, 1);
}

void AuMeatProfile::TriangleFan(AuMeatMesh& mesh, AuMeatTri::PART part, const AuPointArray& ring, const AuPoint& apex,
		bool reverse, int tex, const AuPoint& upVec, float blendAngle, float blendWid, float texY) const
{
	float shine = 20;
	float reflect = .1f;
	int boneIndex[4] = {0,0,0,0};
	float boneWeight[4] = {1,0,0,0};
	//float bottomTx = 0;
	//float bottomTy = 0;
	//float topBottomBlend = 0;
	//float topTx = 0;
	//float topTy = 0;

	AuPointArray::const_iterator pi = ring.cbegin();
	float angp = (*pi).YRotation();
	float texXp = angp / (PIf*2);
	for(AuPointArray::const_iterator ci = pi+1; ci < ring.cend(); ci++)
	{
		// Texture coordinate is based on angle around the Y axis.
		float angc = (*ci).YRotation();
		float texXc = angc / (PIf*2);
		if(ci == ring.cend()-1)
			texXc = -.5f;

		if(apex == *pi || apex == *ci || *pi == *ci)
			continue;

		AuPoint cN(*ci);
		cN.Unit();

		AuPoint pN(*pi);
		pN.Unit();

		// Calcualate top/bottom blend at each vertex.
		float tba, tbp, tbc;
		if(tex == 0)
		{
			tba = 0;
			tbp = 0;
			tbc = 0;
		}
		else if(tex == 1)
		{
			tba = 1;
			tbp = 1;
			tbc = 1;
		}
		else
		{
			// Blend zone is centered 10 degrees below the equator line and is 10 degrees wide.
			tba = AuMeatTri::TopBottomBlend(AuPoint(0,1), upVec, blendAngle, blendWid);
			tbp = AuMeatTri::TopBottomBlend(pN, upVec, blendAngle, blendWid);
			tbc = AuMeatTri::TopBottomBlend(cN, upVec, blendAngle, blendWid);
		}

		if(reverse)
		{
			AuMeatPoint mp0 = AuMeatPoint(apex, AuPoint(0,1), shine, reflect, boneIndex,boneWeight, texXc,1, tba, texXc,1);
			AuMeatPoint mp1 = AuMeatPoint(*pi, pN, shine, reflect, boneIndex,boneWeight, texXp,texY, tbp, texXp,texY);
			AuMeatPoint mp2 = AuMeatPoint(*ci, cN, shine, reflect, boneIndex,boneWeight, texXc,texY, tbc, texXc,texY);

			AuMeatTri tri(part, mp0, mp1, mp2);
			mesh.Append(tri);
		}
		else
		{
			AuMeatPoint mp0 = AuMeatPoint(apex, AuPoint(0,-1), shine, reflect, boneIndex,boneWeight, texXc,0, tba, texXc,0);
			AuMeatPoint mp1 = AuMeatPoint(*ci, cN, shine, reflect, boneIndex,boneWeight, texXc,texY, tbc, texXc,texY);
			AuMeatPoint mp2 = AuMeatPoint(*pi, pN, shine, reflect, boneIndex,boneWeight, texXp,texY, tbp, texXp,texY);

			AuMeatTri tri(part, mp0, mp1, mp2);
			mesh.Append(tri);
		}

		pi = ci;
		texXp = texXc;
	}
}

void AuMeatProfile::TriangleBand(AuMeatMesh& mesh, AuMeatTri::PART part, const AuPointArray& pring, const AuPointArray& nring,
		int tex, const AuPoint& upVec, float blendAngle, float blendWid, float texYp, float texYc) const
{
	float shine = 20;
	float reflect = .1f;
	int boneIndex[4] = {0,0,0,0};
	float boneWeight[4] = {1,0,0,0};

	AuPointArray::const_iterator pi = pring.cbegin();	// Previous ring of vertices around the body.
	AuPointArray::const_iterator npi = nring.cbegin();	// Next ring of vertices around the body.
	float angp = (*pi).YRotation();
	float texXp = angp / (PIf*2);
	float angnp = (*npi).YRotation();
	float texXnp = angnp / (PIf*2);
	for(AuPointArray::const_iterator ci = pi+1, nci = npi+1; ci < pring.cend() && nci < nring.cend(); ci++, nci++)
	{
		// Texture coordinate is based on angle around the Y axis.
		float angc = (*ci).YRotation();
		float texXc = angc / (PIf*2);
		float angnc = (*nci).YRotation();
		float texXnc = angnc / (PIf*2);
		if(ci == pring.cend()-1)
		{
			texXc = -.5f;
			texXnc = -.5f;
		}

		AuPoint cN((*ci).x, 0, (*ci).z);
		cN.Unit();

		AuPoint pN((*pi).x, 0, (*pi).z);
		pN.Unit();

		AuPoint ncN((*nci).x, 0, (*nci).z);
		ncN.Unit();

		AuPoint npN((*npi).x, 0, (*npi).z);
		npN.Unit();

		// Calcualate top/bottom blend at each vertex.
		float tbp, tbc, tbnp, tbnc;
		if(tex == 0)
		{
			tbp = 0;
			tbc = 0;
			tbnp = 0;
			tbnc = 0;
		}
		else if(tex == 1)
		{
			tbp = 1;
			tbc = 1;
			tbnp = 1;
			tbnc = 1;
		}
		else
		{
			// Blend zone is centered 10 degrees below the equator line and is 10 degrees wide.
			tbp = AuMeatTri::TopBottomBlend(pN, upVec, blendAngle, blendWid);
			tbc = AuMeatTri::TopBottomBlend(cN, upVec, blendAngle, blendWid);
			tbnp = AuMeatTri::TopBottomBlend(npN, upVec, blendAngle, blendWid);
			tbnc = AuMeatTri::TopBottomBlend(ncN, upVec, blendAngle, blendWid);
		}

		// Left half of body quads are triangulated with the split going opposite the right half of the body. This gives us better
		// symmetry when deforming the meshes, especially around the eyes.
		if((*ci).x < 0)
		{
			// Left side
			if(*npi != *pi && *npi != *ci && *pi != *ci)
			{
				AuMeatPoint mp0 = AuMeatPoint(*npi, npN, shine, reflect, boneIndex,boneWeight, texXnp,texYc, tbnp, texXnp,texYc);
				AuMeatPoint mp1 = AuMeatPoint(*pi, pN, shine, reflect, boneIndex,boneWeight, texXp,texYp, tbp, texXp,texYp);
				AuMeatPoint mp2 = AuMeatPoint(*ci, cN, shine, reflect, boneIndex,boneWeight, texXc,texYp, tbc, texXc,texYp);

				AuMeatTri tri(part, mp0, mp1, mp2);
				mesh.Append(tri);
			}

			if(*ci != *nci && *ci != *npi && *nci != *npi)
			{
				AuMeatPoint mp0 = AuMeatPoint(*ci, cN, shine, reflect, boneIndex,boneWeight, texXc,texYp, tbc, texXc,texYp);
				AuMeatPoint mp1 = AuMeatPoint(*nci, ncN, shine, reflect, boneIndex,boneWeight, texXnc,texYc, tbnc, texXnc,texYc);
				AuMeatPoint mp2 = AuMeatPoint(*npi, npN, shine, reflect, boneIndex,boneWeight, texXnp,texYc, tbnp, texXnp,texYc);

				AuMeatTri tri(part, mp0, mp1, mp2);
				mesh.Append(tri);
			}
		}
		else
		{
			// Right side
			if(*pi != *ci && *pi != *nci && *ci != *nci)
			{
				AuMeatPoint mp0 = AuMeatPoint(*pi, pN, shine, reflect, boneIndex,boneWeight, texXp,texYp, tbp, texXp,texYp);
				AuMeatPoint mp1 = AuMeatPoint(*ci, cN, shine, reflect, boneIndex,boneWeight, texXc,texYp, tbc, texXc,texYp);
				AuMeatPoint mp2 = AuMeatPoint(*nci, ncN, shine, reflect, boneIndex,boneWeight, texXnc,texYc, tbnc, texXnc,texYc);

				AuMeatTri tri(part, mp0, mp1, mp2);
				mesh.Append(tri);
			}

			if(*nci != *npi && *nci != *pi && *npi != *pi)
			{
				AuMeatPoint mp0 = AuMeatPoint(*nci, ncN, shine, reflect, boneIndex,boneWeight, texXnc,texYc, tbnc, texXnc,texYc);
				AuMeatPoint mp1 = AuMeatPoint(*npi, npN, shine, reflect, boneIndex,boneWeight, texXnp,texYc, tbnp, texXnp,texYc);
				AuMeatPoint mp2 = AuMeatPoint(*pi, pN, shine, reflect, boneIndex,boneWeight, texXp,texYp, tbp, texXp,texYp);

				AuMeatTri tri(part, mp0, mp1, mp2);
				mesh.Append(tri);
			}
		}

		pi = ci;
		npi = nci;

		texXp = texXc;
		texXnp = texXnc;
	}
}

void AuMeatProfile::VertexRingAt(float offset, int ringVerts, AuPointArray& ring) const
{
	Q_ASSERT((ringVerts&0xfffffffc) == (unsigned int)ringVerts);	// ringVerts must be evenly divisible by four.

	// Beginning triangle fan end cap.
	float bottom, top, left, right;
	RadiiAt(offset, bottom, top, left, right);

	int qv = ringVerts/4;				// Number of vertices per quarter.
	int q0 = qv;						// Last vertex of first quarter.
	int q1 = ringVerts/2;				// Last vertex of second quarter.
	int q2 = q0 + q1;					// Last vertex of third quarter.

	// Create the ring of vertices. It will be used in the next band.
	for(int v=0; v<ringVerts; v++)
	{
		float ang = PIf*2 * v / ringVerts;
		float ca = cosf(ang);
		float sa = sinf(ang);

		if(v <= q0)
		{
			// First quadrant.
			ring.append(AuPoint(right*ca, offset, top*sa));
		}
		else if(v <= q1)
		{
			// Second quadrant.
			ring.append(AuPoint(left*ca, offset, top*sa));
		}
		else if(v <= q2)
		{
			// Third quadrant.
			ring.append(AuPoint(left*ca, offset, bottom*sa));
		}
		else
		{
			// Fourth quadrant.
			ring.append(AuPoint(right*ca, offset, bottom*sa));
		}
	}

	// Close perfectly.
	ring.append(ring.first());
}

void AuMeatProfile::RadiiAt(float offset, float& bottom, float& top, float& left, float& right) const
{
	Q_ASSERT(count() > 1);
	if(offset <= begin().key())
	{
		// Offset is at or before the beginning of the profile.
		const AuMeatShape& meat = first();
		bottom = meat.BottomRadius();
		top = meat.TopRadius();
		left = meat.LeftRadius();
		right = meat.RightRadius();
		return;
	}

	if(offset >= (end()-1).key())
	{
		// Offset is at or after the end of the profile.
		const AuMeatShape& meat = last();
		bottom = meat.BottomRadius();
		top = meat.TopRadius();
		left = meat.LeftRadius();
		right = meat.RightRadius();
		return;
	}

	// Offset is between the beginning and ending of the profile.
	// Interpolate between meat shapes.
	const_iterator previ = cbegin();
	for(const_iterator mi = previ+1; mi != cend(); mi++)
	{
		if(offset <= mi.key())
		{
			// Interpolate between previ and mi.
			float frac = (offset - previ.key())/(mi.key() - previ.key());	// Fraction of the distance from previ to mi.
			if(previ == cbegin())
				frac = frac / 2 + .5f;
			bottom = AuNoiseF::Cosine_Interpolate((*previ).BottomRadius(), (*mi).BottomRadius(), frac);
			top = AuNoiseF::Cosine_Interpolate((*previ).TopRadius(), (*mi).TopRadius(), frac);
			left = AuNoiseF::Cosine_Interpolate((*previ).LeftRadius(), (*mi).LeftRadius(), frac);
			right = AuNoiseF::Cosine_Interpolate((*previ).RightRadius(), (*mi).RightRadius(), frac);
			return;
		}
		previ = mi;
	}

	// We should never get here.
	bottom = 0;
	top = 0;
	left = 0;
	right = 0;
	qWarning() << "error: AuMeatProfile::RadiiAt offset" << offset << "len" << Length() << "count" << count();
}

float AuMeatProfile::RoundOffDistance() const
{
	float maxW = MaxWidth();
	float maxH = MaxHeight();
	if(maxW > maxH)
		return maxW/2;
	else
		return maxH/2;
}

void AuMeatProfile::GetSticks(AuSegmentArray& sticks) const
{
	AuPoint prevC, prevB, prevT, prevL, prevR;
	AuPoint curC, curB, curT, curL, curR;
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		float len = mi.key();
		curC = AuPoint(0, len);
		curB = AuPoint(0, len, -(*mi).BottomRadius());
		curT = AuPoint(0, len, (*mi).TopRadius());
		curL = AuPoint(-(*mi).LeftRadius(), len);
		curR = AuPoint((*mi).RightRadius(), len);

		if(mi != cbegin())
		{
			sticks.append(AuSegment(prevC, curC));
			sticks.append(AuSegment(prevB, curB));
			sticks.append(AuSegment(prevT, curT));
			sticks.append(AuSegment(prevL, curL));
			sticks.append(AuSegment(prevR, curR));
		}

		sticks.append(AuSegment(curB, curL));
		sticks.append(AuSegment(curL, curT));
		sticks.append(AuSegment(curT, curR));
		sticks.append(AuSegment(curR, curB));

		prevC = curC;
		prevB = curB;
		prevT = curT;
		prevL = curL;
		prevR = curR;
	}
}

void AuMeatProfile::Add(float distAlongBody, const AuMeatShape& shape)
{
	iterator pi = find(distAlongBody);
	if(pi == end())
	{
		// Search using fuzzy check.
		for(pi = begin(); pi != end(); pi++)
			if(FuzzyEquals(pi.key(), distAlongBody))
				break;
	}
	if(pi == end())
		insert(distAlongBody, shape);
	else
		(*pi).Expand(shape);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMeatTri::AuMeatTri(PART part, const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2, double radius,
		float s, float t, float u, float v, quint8 bone)
{
	m_Part = part;

	AuPoint n0 = ll0.GlobeLocation(1);
	AuPoint n1 = ll1.GlobeLocation(1);
	AuPoint n2 = ll2.GlobeLocation(1);

	AuPoint v0 = n0 * radius;
	AuPoint v1 = n1 * radius;
	AuPoint v2 = n2 * radius;

	double texX0 = ll0.Longitude() / (PI*2) + .5;
	double texY0 = ll0.Latitude() / PI + .5;

	double texX1 = ll1.Longitude() / (PI*2) + .5;
	double texY1 = ll1.Latitude() / PI + .5;

	double texX2 = ll2.Longitude() / (PI*2) + .5;
	double texY2 = ll2.Latitude() / PI + .5;

	texX0 = s + (u - s) * texX0;
	texY0 = t + (v - t) * texY0;
	texX1 = s + (u - s) * texX1;
	texY1 = t + (v - t) * texY1;
	texX2 = s + (u - s) * texX2;
	texY2 = t + (v - t) * texY2;

	float shine = 500;
	float reflect = 1;
	int boneIndex[4] = {bone,0,0,0};
	float boneWeight[4] = {1,0,0,0};

	m_V0 = AuMeatPoint(v0, n0, shine, reflect, boneIndex,boneWeight, texX0,texY0);
	m_V1 = AuMeatPoint(v1, n1, shine, reflect, boneIndex,boneWeight, texX1,texY1);
	m_V2 = AuMeatPoint(v2, n2, shine, reflect, boneIndex,boneWeight, texX2,texY2);
}

double AuMeatTri::Area() const
{
	const AuMeatPoint& v0 = m_V0;
	const AuMeatPoint& v1 = m_V1;
	const AuMeatPoint& v2 = m_V2;

	AuPoint base(v2 - v0);
	AuPoint side(v1 - v0);

	AuPoint vx(base);
	vx.Unit();

	AuPoint vy(SurfaceNormalCalc().Cross(vx));
	vy.Unit();

	double width = fabs(base.Dot(vx));
	double height = fabs(side.Dot(vy));

	// Area = 1/2 base * height
	return width * height / 2.;
}
AuPoint AuMeatTri::Center() const
{
	const AuMeatPoint& v0 = m_V0;
	const AuMeatPoint& v1 = m_V1;
	const AuMeatPoint& v2 = m_V2;
	return (v0 + v1 + v2) / 3.;
}

int AuMeatTri::Contains(const AuPoint& loc) const
{
	return loc.InTriangle_3D(m_V0, m_V1, m_V2, SurfaceNormalCalc());
}

int AuMeatTri::ContainsFuzzy(const AuPoint& loc) const
{
	return loc.InTriangleFuzzy_3D(m_V0, m_V1, m_V2, SurfaceNormalCalc(), d_TriangleAltTol);
}

// Returns the 3D extents box of this triangle.
void AuMeatTri::Extents(AuPoint& min, AuPoint& max) const
{
	min = m_V0;
	max = m_V0;

	min = min.Min(m_V1);
	max = max.Max(m_V1);

	min = min.Min(m_V2);
	max = max.Max(m_V2);
}

const AuPoint& AuMeatTri::SurfaceNormalCalc() const
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

/*
bool AuMeatTri::TriangleIntersection(const AuMeatTri& tri, const AuMeatMesh& trimesh, AuPoint& seg1, AuPoint& seg2,
		const AuMeatMesh& mesh) const
{
	// Skip triangles with parallel planes.
	if(SurfaceNormal(mesh).IsParallel(tri.SurfaceNormal(trimesh)))
		return false;	// The triangles have parallel planes.

	// If the incoming tri is completely above or below my plane, then we cannot intersect.
	double a0 = (tri.m_V0 - m_V0).Dot(SurfaceNormal(mesh));
	double a1 = (tri.m_V1 - m_V0).Dot(SurfaceNormal(mesh));
	double a2 = (tri.m_V2 - m_V0).Dot(SurfaceNormal(mesh));
	if((FuzzyLessThanFactor(a0, 0, d_TriangleAltTol) &&
				FuzzyLessThanFactor(a1, 0, d_TriangleAltTol) &&
				FuzzyLessThanFactor(a2, 0, d_TriangleAltTol)) ||
			(FuzzyGreaterThanFactor(a0, 0, d_TriangleAltTol) &&
			 FuzzyGreaterThanFactor(a1, 0, d_TriangleAltTol) &&
			 FuzzyGreaterThanFactor(a2, 0, d_TriangleAltTol)))
		return false;

	// If I am completely above or below the plane of the incoming triangle, then we cannot intersect.
	a0 = (m_V0 - tri.m_V0).Dot(tri.SurfaceNormal(trimesh));
	a1 = (m_V1 - tri.m_V0).Dot(tri.SurfaceNormal(trimesh));
	a2 = (m_V2 - tri.m_V0).Dot(tri.SurfaceNormal(trimesh));
	if((FuzzyLessThanFactor(a0, 0, d_TriangleAltTol) &&
				FuzzyLessThanFactor(a1, 0, d_TriangleAltTol) &&
				FuzzyLessThanFactor(a2, 0, d_TriangleAltTol)) ||
			(FuzzyGreaterThanFactor(a0, 0, d_TriangleAltTol) &&
			 FuzzyGreaterThanFactor(a1, 0, d_TriangleAltTol) &&
			 FuzzyGreaterThanFactor(a2, 0, d_TriangleAltTol)))
		return false;

	// Test the edge segments of 'tri' for intersections with this triangle.
	AuPoint o0, o0p;
	int oi0 = SegmentIntersection(o0, o0p, tri.m_V0, tri.m_V1, mesh);
	if(oi0 == 2)
	{
		// The first segment hit me in two places.
		seg1 = o0;
		seg2 = o0p;
		return true;
	}

	AuPoint o1, o1p;
	int oi1 = SegmentIntersection(o1, o1p, tri.m_V1, tri.m_V2, mesh);
	if(oi1 == 2)
	{
		// The second segment hit me in two places.
		seg1 = o1;
		seg2 = o1p;
		return true;
	}

	if(oi0 && oi1 && !o0.FuzzyEquals(o1))
	{
		// Each segment produced a different intersection.
		seg1 = o0;
		seg2 = o1;
		return true;
	}

	AuPoint o2, o2p;
	int oi2 = SegmentIntersection(o2, o2p, tri.m_V2, tri.m_V0, mesh);
	if(oi2 == 2)
	{
		// The third segment hit me in two places.
		seg1 = o2;
		seg2 = o2p;
		return true;
	}

	if(oi2)
	{
		// The third segment hit me in one place.
		if(oi0 && !o0.FuzzyEquals(o2))
		{
			// First segment hit at a different place than the third segment.
			seg1 = o0;
			seg2 = o2;
			return true;
		}
		else if(oi1 && !o1.FuzzyEquals(o2))
		{
			// Second segment hit at a different place than the third segment.
			seg1 = o1;
			seg2 = o2;
			return true;
		}
	}

	// Test the edge segments of this triangle for intersections with 'tri'.
	AuPoint m0, m0p;
	int mi0 = tri.SegmentIntersection(m0, m0p, m_V0, m_V1, trimesh);
	if(mi0 == 2)
	{
		// The first segment hit tri in two places.
		seg1 = m0;
		seg2 = m0p;
		return true;
	}

	AuPoint m1, m1p;
	int mi1 = tri.SegmentIntersection(m1, m1p, m_V1, m_V2, trimesh);
	if(mi1 == 2)
	{
		// The second segment hit tri in two places.
		seg1 = m1;
		seg2 = m1p;
		return true;
	}

	if(mi0 && mi1 && !m0.FuzzyEquals(m1))
	{
		// Each segment produced a different intersection.
		seg1 = m0;
		seg2 = m1;
		return true;
	}

	AuPoint m2, m2p;
	int mi2 = tri.SegmentIntersection(m2, m2p, m_V2, m_V0, trimesh);
	if(mi2 == 2)
	{
		// The third segment hit tri in two places.
		seg1 = m2;
		seg2 = m2p;
		return true;
	}

	if(mi2)
	{
		// The third segment hit tri in one place.
		if(mi0 && !m0.FuzzyEquals(m2))
		{
			// First segment hit tri at a different place than the third segment.
			seg1 = m0;
			seg2 = m2;
			return true;
		}
		else if(mi1 && !m1.FuzzyEquals(m2))
		{
			// Second segment hit tri at a different place than the third segment.
			seg1 = m1;
			seg2 = m2;
			return true;
		}
	}

	if((oi0 || oi1 || oi2) && (mi0 || mi1 || mi2))
	{
		// One edge segment of each triangle intersected the other triangle. The intersection segment goes from one to the other.
		if(oi0)
			seg1 = o0;
		else if(oi1)
			seg1 = o1;
		else
			seg1 = o2;

		if(mi0)
			seg2 = m0;
		else if(mi1)
			seg2 = m1;
		else
			seg2 = m2;
		return true;
	}

	// If we hit a single edge, create an intersection segment at that point. This causes the triangle to break there.
	if(oi0)
	{
		seg1 = o0;
		seg2 = o0;
		return true;
	}
	else if(oi1)
	{
		seg1 = o1;
		seg2 = o1;
		return true;
	}
	else if(oi2)
	{
		seg1 = o2;
		seg2 = o2;
		return true;
	}
	else if(mi0)
	{
		seg1 = m0;
		seg2 = m0;
		return true;
	}
	else if(mi1)
	{
		seg1 = m1;
		seg2 = m1;
		return true;
	}
	else if(mi2)
	{
		seg1 = m2;
		seg2 = m2;
		return true;
	}

	return false;
}
*/

bool AuMeatTri::SegmentIntersection(AuPoint& intersect, const AuPoint& seg1, const AuPoint& seg2) const
{
	const AuPoint& N = SurfaceNormalCalc();
	AuLine fl(seg1, seg2);										// Infinite line through seg1 and seg2.
	double rDot = fl.V.Dot(N);
	if(fabs(rDot) < d_TriangleAngTol)
		return false;

	// The line is not parallel with the plane of this triangle. Find the intersection.
	double d = m_V0.Dot(N);
	double t = (d - fl.U.Dot(N))/rDot;
	intersect = fl.U + fl.V*t;

	// Test the intersection point to see if it is within the line segment.
	if(!intersect.Between(seg1, seg2))
		return false;

	// Test the intersection point to see if it is within the triangle.
	return intersect.InTriangleFuzzy_3D(m_V0, m_V1, m_V2, N, FUZZ);
}

/*
int AuMeatTri::SegmentIntersection(AuPoint& intersect, AuPoint& intersect2, const AuPoint& seg1, const AuPoint& seg2,
		const AuMeatMesh& mesh) const
{
	const AuPoint& N = SurfaceNormal(mesh);
	AuLine fl(seg1, seg2);										// Infinite line through seg1 and seg2.
	bool haveIntersect = false;
	double rDot = fl.V.Dot(N);
	if(fabs(rDot) >= d_TriangleAngTol)
	{
		// The line is not parallel with the plane of this triangle. Find the intersection.
		double d = m_V0.Dot(N);
		double t = (d - fl.U.Dot(N))/rDot;
		intersect = fl.U + fl.V*t;
		haveIntersect = true;
	}

	if(!haveIntersect)
	{
		// The line does not intersect the plane or they are close to parallel. It could touch on edge.
		double alt = (seg1 - m_V0).Dot(N);
		if(fabs(alt) > d_TriangleAltTol)
			return 0;											// Segment is too far away from my plane.

		// Does the segment intersect this triangle in the plane of the triangle?
		// What are the odds that the segment matches one of my edge lines?
		if(seg1.FuzzyEquals(m_V0))
		{
			// One end of the segment is on one of my vertices.
			if(seg2.FuzzyEquals(m_V1))
			{
				// The other end of the segment is on another of my vertices.
				intersect = m_V0;
				intersect2 = m_V1;
				return 2;
			}
			else if(seg2.FuzzyEquals(m_V2))
			{
				// The other end of the segment is on another of my vertices.
				intersect = m_V0;
				intersect2 = m_V2;
				return 2;
			}
		}
		else if(seg1.FuzzyEquals(m_V1))
		{
			// One end of the segment is on one of my vertices.
			if(seg2.FuzzyEquals(m_V0))
			{
				// The other end of the segment is on another of my vertices.
				intersect = m_V1;
				intersect2 = m_V0;
				return 2;
			}
			else if(seg2.FuzzyEquals(m_V2))
			{
				// The other end of the segment is on another of my vertices.
				intersect = m_V1;
				intersect2 = m_V2;
				return 2;
			}
		}
		else if(seg1.FuzzyEquals(m_V2))
		{
			// One end of the segment is on one of my vertices.
			if(seg2.FuzzyEquals(m_V1))
			{
				// The other end of the segment is on another of my vertices.
				intersect = m_V2;
				intersect2 = m_V1;
				return 2;
			}
			else if(seg2.FuzzyEquals(m_V0))
			{
				// The other end of the segment is on another of my vertices.
				intersect = m_V2;
				intersect2 = m_V0;
				return 2;
			}
		}

		// Segment did not happen to fall on an edge segment.
		// Does it cross any edge segment?
		AuPoint e0, e1, e2;
		bool ei0 = e0.SegmentIntersection(m_V0, m_V1, seg1, seg2);
		bool ei1 = e1.SegmentIntersection(m_V1, m_V2, seg1, seg2);
		bool ei2 = e2.SegmentIntersection(m_V2, m_V0, seg1, seg2);
		if(ei0 && ei1 && ei2)
		{
			// Hit all three edges. Must have crossed a vertex and intersected the opposite side.
			if(e0.FuzzyEquals(e1))
			{
				intersect = e0;
				intersect2 = e2;
				return 2;
			}
			else
			{
				intersect = e0;
				intersect2 = e1;
				return 2;
			}
		}

		// Remaining cases require us to know if either end of the segment is inside the triangle.
		int st1 = ContainsFuzzy(seg1, mesh);
		int st2 = ContainsFuzzy(seg2, mesh);
		if(st1 && st2)
		{
			// Segment is totally contained within this triangle, could be touching one or two edge lines.
			intersect = seg1;
			intersect2 = seg2;
			return 2;
		}
		else if(!st1 && !st2)
		{
			// Neither end of the segment is inside this triangle or touching. Did it graze a vertex?
			if(ei0)
			{
				intersect = e0;
				return 1;
			}
			else if(ei1)
			{
				intersect = e1;
				return 1;
			}
			else if(ei2)
			{
				intersect = e2;
				return 1;
			}

			// Segment did not touch any of my vertices.
			return 0;
		}
		else if(ei0 && ei1)
		{
			// Segment intersected two of my sides.
			intersect = e0;
			intersect2 = e1;
			return 2;
		}
		else if(ei1 && ei2)
		{
			// Segment intersected two of my sides.
			intersect = e1;
			intersect2 = e2;
			return 2;
		}
		else if(ei2 && ei0)
		{
			// Segment intersected two of my sides.
			intersect = e2;
			intersect2 = e0;
			return 2;
		}
		else if(ei0 || ei1 || ei2)
		{
			// The segment intersected one of my sides.
			if(st1)
				intersect = seg1;	// The first end of the segment is in this triangle, could be on an edge line.
			else
				intersect = seg2;	// The second end of the segment is in this triangle, could be on an edge line.

			// Other end intersected an edge line.
			if(ei0)
				intersect2 = e0;
			else if(ei1)
				intersect2 = e1;
			else
				intersect2 = e2;
			return 2;
		}

		// Segment did not intersect in the plane of this triangle.
		return 0;
	}

	// Test the intersection point to see if it is within the line segment.
	if(!intersect.Between(seg1, seg2))
		return 0;

	// Test the intersection point to see if it is within the triangle.
	if(!intersect.InTriangleFuzzy_3D(m_V0, m_V1, m_V2, SurfaceNormal(mesh), d_TriangleAltTol))
		return 0;
	
	return 1;
}
*/

/*
bool AuMeatTri::Clip(const AuSegment& clipSeg, AuMeatMesh& result, const AuMeatMesh& mesh) const
{
	// Test the segment for intersection with this triangle.
	AuPoint intersect, intersect2;
	int segint = SegmentIntersection(intersect, intersect2, clipSeg.m_fLoc1, clipSeg.m_fLoc2, mesh);
	if(!segint)
		return false;	// No intersection at all.

	// Do not clip when hit at a single vertex.
	if(segint == 1 && (m_V0.FuzzyEquals(intersect) || m_V1.FuzzyEquals(intersect) || m_V2.FuzzyEquals(intersect)))
		return false;	// Segment touched a single vertex of mine.

	if(segint == 2)
	{
		// The segment hit me in two places.
		// If one of them is a vertex, I can treat it like a single hit at the other end.
		bool hitvert = m_V0.FuzzyEquals(intersect) || m_V1.FuzzyEquals(intersect) || m_V2.FuzzyEquals(intersect);
		bool hitvert2 = m_V0.FuzzyEquals(intersect2) || m_V1.FuzzyEquals(intersect2) || m_V2.FuzzyEquals(intersect2);
		if(hitvert && hitvert2)
			return false;	// Segment exactly fell on one of my edge lines.

		// If the first intersection is on a vertex, break at the other intersection.
		if(hitvert)
			return Clip(intersect2, result, mesh);

		// If the second intersection is on a vertex, break at the first intersection.
		if(hitvert2)
			return Clip(intersect, result, mesh);

		// Neither intersection is at a vertex.
		// Break this triangle into three.
		bool ons0 = intersect.IsOnSegment(m_V0, m_V1);
		bool ons1 = intersect.IsOnSegment(m_V1, m_V2);
		bool ons2 = intersect.IsOnSegment(m_V2, m_V0);

		bool ons0p = intersect2.IsOnSegment(m_V0, m_V1);
		bool ons1p = intersect2.IsOnSegment(m_V1, m_V2);
		bool ons2p = intersect2.IsOnSegment(m_V2, m_V0);

		if((ons0 && ons0p) ||
				(ons1 && ons1p) ||
				(ons2 && ons2p))
			return false;	// intersect and intersect2 hit the same edge line.

		qint32 nui1 = result.VertexIndexAdd(intersect, 1, d_VertexMin);		// Intersections are implicitly on the skin surface.
		qint32 nui2 = result.VertexIndexAdd(intersect2, 1, d_VertexMin);	// Intersections are implicitly on the skin surface.

		if(ons0 && ons1p)
		{
			// The m_V2 to m_V0 edge is unbroken.
			AuMeatTri nta(*this);
			nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			nta.m_V1.m_VertexIndex = nui1;
			nta.m_V2.m_VertexIndex = nui2;
			result.Append(nta, result, true);

			AuMeatTri ntb(*this);
			ntb.m_V0.m_VertexIndex = nui1;
			ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			ntb.m_V2.m_VertexIndex = nui2;
			result.Append(ntb, result, true);

			AuMeatTri ntc(*this);
			ntc.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			ntc.m_V1.m_VertexIndex = nui2;
			ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntc, result, true);
			return true;
		}

		if(ons1 && ons0p)
		{
			// The m_V2 to m_V0 edge is unbroken.
			AuMeatTri nta(*this);
			nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			nta.m_V1.m_VertexIndex = nui2;
			nta.m_V2.m_VertexIndex = nui1;
			result.Append(nta, result, true);

			AuMeatTri ntb(*this);
			ntb.m_V0.m_VertexIndex = nui2;
			ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			ntb.m_V2.m_VertexIndex = nui1;
			result.Append(ntb, result, true);

			AuMeatTri ntc(*this);
			ntc.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			ntc.m_V1.m_VertexIndex = nui1;
			ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntc, result, true);
			return true;
		}

		if(ons1 && ons2p)
		{
			// The m_V0 to m_V1 edge is unbroken.
			AuMeatTri nta(*this);
			nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			nta.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			nta.m_V2.m_VertexIndex = nui2;
			result.Append(nta, result, true);

			AuMeatTri ntb(*this);
			ntb.m_V0.m_VertexIndex = nui2;
			ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			ntb.m_V2.m_VertexIndex = nui1;
			result.Append(ntb, result, true);

			AuMeatTri ntc(*this);
			ntc.m_V0.m_VertexIndex = nui2;
			ntc.m_V1.m_VertexIndex = nui1;
			ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntc, result, true);
			return true;
		}

		if(ons2 && ons1p)
		{
			// The m_V0 to m_V1 edge is unbroken.
			AuMeatTri nta(*this);
			nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			nta.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			nta.m_V2.m_VertexIndex = nui1;
			result.Append(nta, result, true);

			AuMeatTri ntb(*this);
			ntb.m_V0.m_VertexIndex = nui1;
			ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			ntb.m_V2.m_VertexIndex = nui2;
			result.Append(ntb, result, true);

			AuMeatTri ntc(*this);
			ntc.m_V0.m_VertexIndex = nui1;
			ntc.m_V1.m_VertexIndex = nui2;
			ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntc, result, true);
			return true;
		}

		if(ons0 && ons2p)
		{
			// The m_V1 to m_V2 edge is unbroken.
			AuMeatTri nta(*this);
			nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			nta.m_V1.m_VertexIndex = nui1;
			nta.m_V2.m_VertexIndex = nui2;
			result.Append(nta, result, true);

			AuMeatTri ntb(*this);
			ntb.m_V0.m_VertexIndex = nui1;
			ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			ntb.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntb, result, true);

			AuMeatTri ntc(*this);
			ntc.m_V0.m_VertexIndex = nui2;
			ntc.m_V1.m_VertexIndex = nui1;
			ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntc, result, true);
			return true;
		}

		if(ons2 && ons0p)
		{
			// The m_V1 to m_V2 edge is unbroken.
			AuMeatTri nta(*this);
			nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
			nta.m_V1.m_VertexIndex = nui2;
			nta.m_V2.m_VertexIndex = nui1;
			result.Append(nta, result, true);

			AuMeatTri ntb(*this);
			ntb.m_V0.m_VertexIndex = nui2;
			ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
			ntb.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntb, result, true);

			AuMeatTri ntc(*this);
			ntc.m_V0.m_VertexIndex = nui1;
			ntc.m_V1.m_VertexIndex = nui2;
			ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
			result.Append(ntc, result, true);
			return true;
		}

		// At least one end of the segment hit me in the middle of my area or on an edge line.
		// Neither end is at one of my vertices.
		// If we hit on a segment with the second intersection, break it there. That will create only two triangles vs a hit in the
		// middle.
		if(ons0p || ons1p || ons2p)
			return Clip(intersect2, result, mesh);

		// Break the triangle at intersect.
	}

	// I was hit in one place but not at a vertex.
	return Clip(intersect, result, mesh);
}

bool AuMeatTri::Clip(const AuPoint& clipLoc, AuMeatMesh& result, const AuMeatMesh& mesh, double tol) const
{
	// Do not clip when hit at a vertex.
	if(m_V0.FuzzyEquals(clipLoc, tol) ||
			m_V1.FuzzyEquals(clipLoc, tol) ||
			m_V2.FuzzyEquals(clipLoc, tol))
		return false;

	// Triangles that are hit on an edge get split in two.
	// Triangles that are hit in the middle are split in three.
	if(clipLoc.IsOnSegment(m_V0, m_V1, tol))
	{
		// Split triangle into two triangles.
		qint32 nui = result.VertexIndexAdd(clipLoc, 1, d_VertexMin);	// Intersections are implicitly on the skin surface.

		AuMeatTri nta(*this);
		nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
		nta.m_V1.m_VertexIndex = nui;
		nta.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
		result.Append(nta, result, true);

		AuMeatTri ntb(*this);
		ntb.m_V0.m_VertexIndex = nui;
		ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
		ntb.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
		result.Append(ntb, result, true);

		return true;
	}

	if(clipLoc.IsOnSegment(m_V1, m_V2, tol))
	{
		// Split triangle into two triangles.
		qint32 nui = result.VertexIndexAdd(clipLoc, 1, d_VertexMin);	// Intersections are implicitly on the skin surface.

		AuMeatTri nta(*this);
		nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
		nta.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
		nta.m_V2.m_VertexIndex = nui;
		result.Append(nta, result, true);

		AuMeatTri ntb(*this);
		ntb.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
		ntb.m_V1.m_VertexIndex = nui;
		ntb.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
		result.Append(ntb, result, true);

		return true;
	}

	if(clipLoc.IsOnSegment(m_V2, m_V0, tol))
	{
		// Split triangle into two triangles.
		qint32 nui = result.VertexIndexAdd(clipLoc, 1, d_VertexMin);	// Intersections are implicitly on the skin surface.

		AuMeatTri nta(*this);
		nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
		nta.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
		nta.m_V2.m_VertexIndex = nui;
		result.Append(nta, result, true);

		AuMeatTri ntb(*this);
		ntb.m_V0.m_VertexIndex = nui;
		ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
		ntb.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
		result.Append(ntb, result, true);

		return true;
	}

	if(ContainsFuzzy(clipLoc, mesh))
	{
		// Split triangle into three triangles.
		qint32 nui = result.VertexIndexAdd(clipLoc, 1, d_VertexMin);	// Intersections are implicitly on the skin surface.

		AuMeatTri nta(*this);
		nta.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
		nta.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
		nta.m_V2.m_VertexIndex = nui;
		result.Append(nta, result, true);

		AuMeatTri ntb(*this);
		ntb.m_V0.m_VertexIndex = nui;
		ntb.m_V1.m_VertexIndex = result.VertexIndexAdd(m_V1, d_VertexMin);
		ntb.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
		result.Append(ntb, result, true);

		AuMeatTri ntc(*this);
		ntc.m_V0.m_VertexIndex = result.VertexIndexAdd(m_V0, d_VertexMin);
		ntc.m_V1.m_VertexIndex = nui;
		ntc.m_V2.m_VertexIndex = result.VertexIndexAdd(m_V2, d_VertexMin);
		result.Append(ntc, result, true);

		return true;
	}
	return false;
}
*/

void AuMeatTri::ApplySkinTexture(const AuMatrix4x4& mTex, float length, int topBottomBlend, float blendAngle, float blendWid)
{
	// Calculate vertex location and normal relative to the texture.
	AuPoint p0, p1, p2, n0, n1, n2;
	p0 = mTex.UnProject(m_V0);
	n0 = mTex.UnProjectVector(m_V0.m_N);
	p1 = mTex.UnProject(m_V1);
	n1 = mTex.UnProjectVector(m_V1.m_N);
	p2 = mTex.UnProject(m_V2);
	n2 = mTex.UnProjectVector(m_V2.m_N);

	// Scale vertex locations to the texture size. This gives us the 2D texture coordinates for a simple projection.
	p0 /= length;
	p1 /= length;
	p2 /= length;

	// Calculate top/bottom blend at each vertex.
	if(topBottomBlend == 0)
	{
		m_V0.m_SkinTopBottomBlend = 0;
		m_V1.m_SkinTopBottomBlend = 0;
		m_V2.m_SkinTopBottomBlend = 0;
	}
	else if(topBottomBlend == 1)
	{
		m_V0.m_SkinTopBottomBlend = 1;
		m_V1.m_SkinTopBottomBlend = 1;
		m_V2.m_SkinTopBottomBlend = 1;
	}
	else
	{
		m_V0.m_SkinTopBottomBlend = TopBottomBlend(n0, AuPoint(0,0,1), blendAngle, blendWid);
		m_V1.m_SkinTopBottomBlend = TopBottomBlend(n1, AuPoint(0,0,1), blendAngle, blendWid);
		m_V2.m_SkinTopBottomBlend = TopBottomBlend(n2, AuPoint(0,0,1), blendAngle, blendWid);
	}

	// Configure texture coords at each vertex.
	m_V0.m_SkinBottomTx = (float)p0.x;
	m_V0.m_SkinBottomTy = (float)p0.y;
	m_V0.m_SkinTopTx = m_V0.m_SkinBottomTx;
	m_V0.m_SkinTopTy = m_V0.m_SkinBottomTy;

	m_V1.m_SkinBottomTx = (float)p1.x;
	m_V1.m_SkinBottomTy = (float)p1.y;
	m_V1.m_SkinTopTx = m_V1.m_SkinBottomTx;
	m_V1.m_SkinTopTy = m_V1.m_SkinBottomTy;

	m_V2.m_SkinBottomTx = (float)p2.x;
	m_V2.m_SkinBottomTy = (float)p2.y;
	m_V2.m_SkinTopTx = m_V2.m_SkinBottomTx;
	m_V2.m_SkinTopTy = m_V2.m_SkinBottomTy;
}

void AuMeatTri::RemoveDecal()
{
	m_V0.RemoveDecal();
	m_V1.RemoveDecal();
	m_V2.RemoveDecal();
}

bool AuMeatTri::Above(double z) const
{
	if(m_V0.z > z)
		return true;
	if(m_V1.z > z)
		return true;
	if(m_V2.z > z)
		return true;
	return false;
}

bool AuMeatTri::Below(double z) const
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

AuMeatMesh::AuMeatMesh(const AuScFaceList& faces, AuMeatTri::PART part)
{
	FromFaces(faces, part);
}

AuMeatMesh::AuMeatMesh(const AuScFaceList& faces, AuMeatTri::PART part, int topBottomBlend, const AuPoint& upVec, float blendAngle,
		float blendWid)
{
	FromFaces(faces, part, topBottomBlend, upVec, blendAngle, blendWid);
}

void AuMeatMesh::RemoveDecals()
{
	for(iterator mi = begin(); mi != end(); mi++)
		(*mi).RemoveDecal();
}

void AuMeatMesh::ApplyDecal(MEAT_DECAL decal, const AuMatrix4x4& m, float decalWidth, float decalHeight, float decalAlpha)
{
	float s, t, u, v;
	AuMeat::DecalTexture(decal, s, t, u, v);

	double wd2 = decalWidth/2;
	double ht2 = decalHeight/2;
	AuPoint vDec[3];
	for(iterator mi = begin(); mi != end(); mi++)
	{
		vDec[0] = (*mi).m_V0;
		vDec[1] = (*mi).m_V1;
		vDec[2] = (*mi).m_V2;

		for(int vi=0; vi<3; vi++)
		{
			// Project my vertices into the space of the decal rectangle.
			vDec[vi] = m.UnProject(vDec[vi]);

			// Skip vertices pointing away from the decal. This is not perfect but will work to get going.
			if(vDec[vi].z <= 0)
				continue;

			// Skip vertices that are not inside the decal rectangle.
			if(vDec[vi].x < -wd2 || vDec[vi].x > wd2)
				continue;
			if(vDec[vi].y < -ht2 || vDec[vi].y > ht2)
				continue;

			// Calculate texture coordinates for the vertex.
			float tx = vDec[vi].x / decalWidth + .5f;
			if(tx < 0)
				tx = 0;
			else if(tx > 1)
				tx = 1;
			float ty = vDec[vi].y / decalHeight + .5f;
			if(ty < 0)
				ty = 0;
			else if(ty > 1)
				ty = 1;

			// Move the texture coords to the decal.
			tx = s + (u - s) * tx;
			ty = t + (v - t) * ty;

			if(vi == 0)
			{
				(*mi).m_V0.m_DecalBlend = decalAlpha;
				(*mi).m_V0.m_DecalTx = tx;
				(*mi).m_V0.m_DecalTy = ty;
			}
			else if(vi == 1)
			{
				(*mi).m_V1.m_DecalBlend = decalAlpha;
				(*mi).m_V1.m_DecalTx = tx;
				(*mi).m_V1.m_DecalTy = ty;
			}
			else if(vi == 2)
			{
				(*mi).m_V2.m_DecalBlend = decalAlpha;
				(*mi).m_V2.m_DecalTx = tx;
				(*mi).m_V2.m_DecalTy = ty;
			}
		}
	}

	// Any triangles that have only one or two vertices in the decal must be disabled or the decal bleeds into neighboring decals.
	for(iterator mi = begin(); mi != end(); mi++)
	{
		if((*mi).m_V0.m_DecalBlend && (*mi).m_V1.m_DecalBlend && (*mi).m_V2.m_DecalBlend)
			continue;	// Skip fully decaled triangle.

		if((*mi).m_V0.m_DecalBlend || (*mi).m_V1.m_DecalBlend || (*mi).m_V2.m_DecalBlend)
		{
			// Remove decal from partially decaled triangle.
			(*mi).m_V0.m_DecalBlend = 0;
			(*mi).m_V1.m_DecalBlend = 0;
			(*mi).m_V2.m_DecalBlend = 0;
		}
	}
}

void AuMeatMesh::Sphere(AuMeatTri::PART part, float triangleSize, float radius, float s, float t, float u, float v, quint8 bone)
{
	float c = radius * 2.f * PIf;
	int eqdivs = c / triangleSize;
	if(eqdivs < 8)
		eqdivs = 8;

	// Poles
	AuGlobeLL poleN(0,PI/2);									// North pole
	AuGlobeLL poleS(0,-PI/2);									// South pole

	// North and south hemisphere except poles and equator.
	int latdivshemi = eqdivs/4;									// Latitude divisions per hemisphere.
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	QList<AuGlobeLL> lastRowN, lastRowS;						// Previous rows of north and south globe locations.
	for(int latinc=1; latinc<latdivshemi; latinc++)
	{
		// We can do both hemispheres at once.
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		Q_ASSERT(latitude < PI/2);
		Q_ASSERT(latitude > 0);
		int londivs = (int)ceilf(cosf(latitude) * eqdivs);
		if(londivs < 3)
			londivs = 3;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		QList<AuGlobeLL> rowN, rowS;							// Current rows of north and south globe locations.
		AuGlobeLL lastN, lastS;
		int lastRowIdx=0;
		for(int loninc=0; loninc<=londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			Q_ASSERT(longitude >= -PI);
			Q_ASSERT(longitude <= PI);
			AuGlobeLL llN(longitude, latitude);					// North hemisphere
			AuGlobeLL llS(longitude, -latitude);				// South hemisphere

			if(loninc)
			{
				// Not the first location at this latitude so we can draw a triangle.
				if(latinc == 1)
				{
					// Tessellate the first row to the poles.
					Append(AuMeatTri(part, poleN, llN, lastN, radius, s,t,u,v, bone));
					Append(AuMeatTri(part, poleS, lastS, llS, radius, s,t,u,v, bone));
				}
				else
				{
					// Tessellate subsequent row to the previous row.
					Append(AuMeatTri(part, lastRowN[lastRowIdx], llN, lastN, radius, s,t,u,v, bone));
					Append(AuMeatTri(part, lastRowS[lastRowIdx], lastS, llS, radius, s,t,u,v, bone));

					if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
					{
						// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
						// there and bump the previous row index.
						int nextLastRowIdx = lastRowIdx+1;
						if(nextLastRowIdx >= lastRowN.count())
							nextLastRowIdx = 0;					// Wrap to beginning of last row.
						Append(AuMeatTri(part, lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llN, radius, s,t,u,v, bone));
						Append(AuMeatTri(part, lastRowS[lastRowIdx], llS, lastRowS[nextLastRowIdx], radius, s,t,u,v, bone));
						lastRowIdx = nextLastRowIdx;
					}
				}
			}
			lastN = llN;
			lastS = llS;
			rowN.append(llN);
			rowS.append(llS);
		}
		lastRowN = rowN;
		lastRowS = rowS;
	}

	// Equator
	int londivs = eqdivs;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	AuGlobeLL lastE;
	int lastRowIdx=0;
	for(int loninc=0; loninc<=londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		Q_ASSERT(longitude >= -PI);
		Q_ASSERT(longitude <= PI);
		AuGlobeLL llE(longitude, 0);							// Equator

		if(loninc)
		{
			// Not the first location at the equator so we can draw a triangle.
			// Tessellate equator row to the previous row.
			Append(AuMeatTri(part, lastRowN[lastRowIdx], llE, lastE, radius, s,t,u,v, bone));
			Append(AuMeatTri(part, lastRowS[lastRowIdx], lastE, llE, radius, s,t,u,v, bone));

			if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
			{
				// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
				// there and bump the previous row index.
				int nextLastRowIdx = lastRowIdx+1;
				if(nextLastRowIdx >= lastRowN.count())
					nextLastRowIdx = 0;					// Wrap to beginning of last row.
				Append(AuMeatTri(part, lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llE, radius, s,t,u,v, bone));
				Append(AuMeatTri(part, lastRowS[lastRowIdx], llE, lastRowS[nextLastRowIdx], radius, s,t,u,v, bone));
				lastRowIdx = nextLastRowIdx;
			}
		}
		lastE = llE;
	}
}

/*
void AuMeatMesh::Bulge(const AuPointArray& poly, const AuPoint& vector, double tensionRadius, bool isOut, bool fill)
{
	if(isEmpty() || poly.isEmpty())
		return;

	if(tensionRadius <= 0)
		return;

	AuPoint vx;
	vector.ArbitraryAxis(vx);
	AuPoint vy = vector.Cross(vx);
	vy.Unit();

	// Move all vertices that are within range of the polyline.
	const double tensionRad2 = tensionRadius * tensionRadius;
	for(AuMeatVertexArray::iterator vi = m_Vertices.begin(); vi != m_Vertices.end(); vi++)
	{
		// Skip vertices on the other side of the mesh.
		double tz = (*vi).Dot(vector);
		if(tz < 0)
			continue;						// Do not bulge in.

		// Find the closest point along the polyline.
		AuPoint c = poly.ClosestSegmentLoc(*vi);
		AuPoint cv = c - *vi;				// Vector from vertex to closest segment location.
		double dz = cv.Dot(vector);			// Distance to move in the direction of the vector to reach the height of the segment.
		if(isOut)
		{
			if(dz <= 0)
				continue;						// Do not move inward, only outward.
		}
		else
		{
			if(dz >= 0)
				continue;						// Do not move outward, only inward.
		}

		if(fill && poly.PointInBoundary(*vi))
		{
			*vi += vector * dz;
			continue;
		}

		double dx = cv.Dot(vx);
		double dy = cv.Dot(vy);
		double dist2 = dx * dx + dy * dy;	// Squared distance to the intersection point in the plane of the polyline.

		// Is the closest point within the tension radius?
		if(dist2 < tensionRad2)
		{
			// This vertex must move.
			double d = sqrt(dist2);
			double fact = d / tensionRadius;	// 0 if vi is right on the segment, 1 if vi is at the tensionRadius distance away.
			double a = AuNoise::Cosine_Interpolate(dz, 0, fact);
			//double a = AuNoise::Linear_Interpolate(dz, 0, fact);
			*vi += vector * a;
		}
	}

	if(vector.z != 0)
	{
		// Bulge vector could have changed the sort order of my triangles.
		AuMeatMesh result(*this);
		Clear();
		Append(result, false);
	}

	//SmoothNormals();
}
*/

/*
void AuMeatMesh::BreakBigTriangles(double maxEdgeLength)
{
	if(isEmpty())
		return;

	double max2 = maxEdgeLength * maxEdgeLength;

	bool anySplit;
	do
	{
		anySplit = false;
		AuMeatMesh numesh;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
		{
			const AuMeatVertex& v0 = Vertex((*mi).m_V0.Index());
			const AuMeatVertex& v1 = Vertex((*mi).m_V1.Index());
			const AuMeatVertex& v2 = Vertex((*mi).m_V2.Index());

			double l01 = v0.Distance2(v1);
			double l02 = v0.Distance2(v2);
			double l12 = v1.Distance2(v2);

			if(l01 <= max2 && l02 <= max2 && l12 <= max2)
			{
				// Triangle is not too big.
				numesh.Append(*mi, *this, true);
			}
			else
			{
				// Split the triangle on it's longest side.
				AuMeatTri tri0(*mi);
				AuMeatTri tri1(*mi);

				if(l01 > l02 && l01 > l12)
				{
					// Break on the side from v0 to v1.
					AuPoint c = (v0 + v1)/2;
					int ic = m_Vertices.IndexOf(c);
					if(ic < 0)
						ic = VertexIndexAdd(c, 1);

					tri0.m_V1.m_VertexIndex = ic;
					tri1.m_V0.m_VertexIndex = ic;
				}
				else if(l12 > l02)
				{
					// Break on the side from v1 to v2.
					AuPoint c = (v1 + v2)/2;
					int ic = m_Vertices.IndexOf(c);
					if(ic < 0)
						ic = VertexIndexAdd(c, 1);

					tri0.m_V2.m_VertexIndex = ic;
					tri1.m_V1.m_VertexIndex = ic;
				}
				else
				{
					// Break on the side from v0 to v2.
					AuPoint c = (v0 + v2)/2;
					int ic = m_Vertices.IndexOf(c);
					if(ic < 0)
						ic = VertexIndexAdd(c, 1);

					tri0.m_V2.m_VertexIndex = ic;
					tri1.m_V0.m_VertexIndex = ic;
				}

				numesh.Append(tri0, *this, true);
				numesh.Append(tri1, *this, true);

				anySplit = true;
			}
		}

		qDebug() << "AuMeatMesh::BreakBigTriangles numesh" << numesh.size() << "oldmesh" << size();

		clear();
		Append(numesh, true);
	}
	while(anySplit);
}
*/

/*
void AuMeatMesh::Bulge(const AuMeatMesh& mesh, const AuPoint& vector, double tensionRadius, bool isOut)
{
	if(isEmpty() || mesh.isEmpty())
		return;

	// Find every one of my vertexes that is inside the mesh.
	QHash<int, double> moved;			// Key is vertex index that will move, value is the distance that it will moved.
	AuPoint skinN;
	int ii = 0;
	for(AuMeatVertexArray::iterator vi = m_Vertices.begin(); vi != m_Vertices.end(); vi++, ii++)
	{
		AuPoint i = mesh.InsideSkin(*vi, vector, skinN);
		if(i.IsZero())
			continue;

		// My vector is inside the mesh. It will be moved.
		double moveDist = i.Distance(*vi);	// The distance the vertex will move.
		moved.insert(ii, moveDist);
	}

	if(!moved.isEmpty())
	{
		// Some part of my skin will move.
		if(tensionRadius > 0)
		{
			// Move all vertices that did not move
			AuPoint vx;
			vector.ArbitraryAxis(vx);
			AuPoint vy = vector.Cross(vx);
			vy.Unit();

			double tensionRad2 = tensionRadius * tensionRadius;
			int ii=0;
			for(AuMeatVertexArray::iterator vi = m_Vertices.begin(); vi != m_Vertices.end(); vi++, ii++)
			{
				// Skip vertices that we already moved.
				if(moved.contains(ii))
					continue;

				if(isOut)
				{
					if((*vi).Dot(vector) <= 0)
						continue;						// Do not move inward, only outward.
				}
				else
				{
					if((*vi).Dot(vector) >= 0)
						continue;						// Do not move outward, only inward.
				}

				// Find the closest vertex that did move.
				QHash<int, double>::const_iterator mi = moved.cbegin();
				QHash<int, double>::const_iterator closesti = mi;				// Closest vertex that is slated to move.
				double closestdist2 = (*vi).Distance2(m_Vertices[mi.key()]);
				for(mi++; mi != moved.cend(); mi++)
				{
					const AuMeatVertex& v = m_Vertices[mi.key()];
					double dist2 = (*vi).Distance2(v);
					if(dist2 < closestdist2)
					{
						closestdist2 = dist2;
						closesti = mi;
					}
				}

				// Calculate 2D distance in the plane of movement.
				AuPoint cv = m_Vertices[closesti.key()] - *vi;	// Vector from vertex to closest vertex that moved the full way.
				double dx = cv.Dot(vx);
				double dy = cv.Dot(vy);
				double dist2 = dx * dx + dy * dy;	// Squared distance to the intersection point in the plane of the polyline.

				// Is the closest vertex within the tension radius?
				if(dist2 < tensionRad2)
				{
					// This vertex must move.
					double d = sqrt(dist2);
					double fact = d / tensionRadius;
					double a = AuNoise::Cosine_Interpolate(*mi, 0, fact);
					*vi += vector * a;
				}
			}
		}
		/+
		{
			// Stretch move all vertices that did not move
			AuPoint vx;
			vector.ArbitraryAxis(vx);
			AuPoint vy = vector.Cross(vx);
			vy.Unit();

			int passes = 0;
			bool anyMoved;
			QHash<int, double> stretchmoved;		// Key is vertex index that will move, value is the distance that it will moved.
			do
			{
				anyMoved = false;
				for(QHash<int, double>::const_iterator mi = moved.cbegin(); mi != moved.cend(); mi++)
				{
					// Move vertices of all triangles that share vertex mi.
					const AuMeatVertex& v = m_Vertices[mi.key()];
					for(const_iterator ti = cbegin(); ti != cend(); ti++)
					{
						// Skip triangles that do not share mi.
						if((*ti).m_V0.Index() != mi.key() && (*ti).m_V1.Index() != mi.key() && (*ti).m_V2.Index() != mi.key())
							continue;

						// Found a triangle that shares vertex mi.
						// Move each vertex that has not moved yet.
						if((*ti).m_V0.Index() != mi.key() &&
								!moved.contains((*ti).m_V0.Index()) &&
								!stretchmoved.contains((*ti).m_V0.Index()))
						{
							// Triangle vertex has not moved yet.
							// Calculate movement based on the stretch factor if the vertex is not pointing away from the extrusion.
							const AuMeatVertex& v0 = m_Vertices[(*ti).m_V0.Index()];
							if((isOut && v0.Dot(vector) > 0) ||
								(!isOut && v0.Dot(vector) < 0))
							{
								// Calculate 2D distance in the plane of extrusion.
								AuPoint cv = v - v0;					// Vector from tri vertex to the vertex that already moved.
								double dx = cv.Dot(vx);
								double dy = cv.Dot(vy);
								double dist = sqrt(dx * dx + dy * dy);	// distance to the vertex in the plane of the extrusion.
								double lift = pow(tension, dist) * *mi;
								if(lift > d_TriangleAltTol)
								{
									qDebug() << " v0 dist" << dist << "pow" << pow(tension, dist) << "lift" << lift;
									stretchmoved.insert((*ti).m_V0.Index(), lift);
									anyMoved = true;
								}
							}
						}

						if((*ti).m_V1.Index() != mi.key() &&
								!moved.contains((*ti).m_V1.Index()) &&
								!stretchmoved.contains((*ti).m_V1.Index()))
						{
							// Triangle vertex has not moved yet.
							// Calculate movement based on the stretch factor if the vertex is not pointing away from the extrusion.
							const AuMeatVertex& v1 = m_Vertices[(*ti).m_V1.Index()];
							if((isOut && v1.Dot(vector) > 0) ||
								(!isOut && v1.Dot(vector) < 0))
							{
								// Calculate 2D distance in the plane of extrusion.
								AuPoint cv = v - v1;					// Vector from tri vertex to the vertex that already moved.
								double dx = cv.Dot(vx);
								double dy = cv.Dot(vy);
								double dist = sqrt(dx * dx + dy * dy);	// distance to the vertex in the plane of the extrusion.
								double lift = pow(tension, dist) * *mi;
								if(lift > d_TriangleAltTol)
								{
									qDebug() << " v1 dist" << dist << "pow" << pow(tension, dist) << "lift" << lift;
									stretchmoved.insert((*ti).m_V1.Index(), lift);
									anyMoved = true;
								}
							}
						}

						if((*ti).m_V2.Index() != mi.key() &&
								!moved.contains((*ti).m_V2.Index()) &&
								!stretchmoved.contains((*ti).m_V2.Index()))
						{
							// Triangle vertex has not moved yet.
							// Calculate movement based on the stretch factor if the vertex is not pointing away from the extrusion.
							const AuMeatVertex& v2 = m_Vertices[(*ti).m_V2.Index()];
							if((isOut && v2.Dot(vector) > 0) ||
								(!isOut && v2.Dot(vector) < 0))
							{
								// Calculate 2D distance in the plane of extrusion.
								AuPoint cv = v - v2;					// Vector from tri vertex to the vertex that already moved.
								double dx = cv.Dot(vx);
								double dy = cv.Dot(vy);
								double dist = sqrt(dx * dx + dy * dy);	// distance to the vertex in the plane of the extrusion.
								double lift = pow(tension, dist) * *mi;
								if(lift > d_TriangleAltTol)
								{
									qDebug() << " v2 dist" << dist << "pow" << pow(tension, dist) << "lift" << lift;
									stretchmoved.insert((*ti).m_V2.Index(), lift);
									anyMoved = true;
								}
							}
						}
					}
				}

				// Append stretch moved info to the moved map for the next pass.
				qDebug() << "stretch moved" << stretchmoved.size() << "moved" << moved.size() << "total" << m_Vertices.size();
				for(QHash<int, double>::const_iterator mi = stretchmoved.cbegin(); mi != stretchmoved.cend(); mi++)
					moved.insert(mi.key(), mi.value());
				stretchmoved.clear();
				passes++;
			}
			while(anyMoved && passes < 2);
		}
	+/

		// Move my vertices.
		for(QHash<int, double>::const_iterator mi = moved.cbegin(); mi != moved.cend(); mi++)
		{
			AuMeatVertexArray::iterator vi = m_Vertices.begin() + mi.key();
			*vi += vector * *mi;
		}

		//if(vector.z != 0)
		{
			// Bulge vector could have changed the sort order of my triangles.
			AuMeatMesh result(*this);
			Clear();
			Append(result, false);
		}

		//SmoothNormals();
	}
}
*/

/*
void AuMeatMesh::Join(AuMeatMesh& mesh, AuSegmentArray& intersection)
{
	//mesh.SmoothNormals();
	Append(mesh, false);
	return;

	qDebug() << "AuMeatMesh::Join mymesh" << size() << "tris joining mesh" << mesh.size() << "tris" << AuTime::CurrentUTC();
	if(isEmpty())
	{
		intersection.clear();
		*this = mesh;
		qDebug() << "AuMeatMesh::Join done" << AuTime::CurrentUTC();
		return;
	}

	// We will construct a new mesh from my current mesh and the incoming mesh.
	AuMeatMesh result;

	// Check the incoming mesh against this mesh.
	TestInside(mesh);
	//qDebug() << "AuMeatMesh::Join mymesh TestInside mesh" << AuTime::CurrentUTC();

	// Triangles completely contained inside my current mesh are discarded.
	// Triangles completely outside my current mesh are kept.
	// Triangles that intersect my current mesh are split; the inside piece(s) are discarded and the outside piece(s) are kept.
	AuMeatMesh meshclip;
	for(const_iterator mi = mesh.cbegin(); mi != mesh.cend(); mi++)
	{
		int t0 = mesh.Vertex((*mi).m_V0.Index()).InsideMesh();
		int t1 = mesh.Vertex((*mi).m_V1.Index()).InsideMesh();
		int t2 = mesh.Vertex((*mi).m_V2.Index()).InsideMesh();
		if(t0 >= 1 && t1 >= 1 && t2 >= 1)
		{
			// Triangle is inside this mesh.
			continue;
		}
		else if(t0 <= 1 && t1 <= 1 && t2 <= 1)
		{
			// Triangle is outside this mesh.
			result.Insert(mi.key(), *mi, mesh, true);
		}
		else
		{
			// Triangle intersects this mesh.
			meshclip.Insert(mi.key(), *mi, mesh, true);
		}
	}
	//qDebug() << "AuMeatMesh::Join meshclip" << meshclip.size() << AuTime::CurrentUTC();

	// Check this mesh against the incoming mesh.
	mesh.TestInside(*this);
	//qDebug() << "AuMeatMesh::Join mesh TestInside mymesh" << AuTime::CurrentUTC();

	// Triangles completely contained inside the incoming mesh are discarded.
	// Triangles completely outside the incoming mesh are kept.
	// Triangles that intersect the incoming mesh are split; the inside piece(s) are discarded and the outside piece(s) are kept.
	AuMeatMesh myclip;
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		int t0 = Vertex((*mi).m_V0.Index()).InsideMesh();
		int t1 = Vertex((*mi).m_V1.Index()).InsideMesh();
		int t2 = Vertex((*mi).m_V2.Index()).InsideMesh();
		if(t0 >= 1 && t1 >= 1 && t2 >= 1)
		{
			// Triangle is inside the mesh.
			continue;
		}
		else if(t0 <= 1 && t1 <= 1 && t2 <= 1)
		{
			// Triangle is outside the mesh.
			result.Insert(mi.key(), *mi, *this, true);
		}
		else
		{
			// Triangle intersects the mesh.
			myclip.Insert(mi.key(), *mi, *this, true);
		}
	}
	//qDebug() << "AuMeatMesh::Join myclip" << myclip.size() << AuTime::CurrentUTC();

	// Create a list of the segments that form the clip intersection line between the meshes.
	IntersectionSegments(intersection, mesh);
	//qDebug() << "AuMeatMesh::Join intersection segments" << intersection.size() << AuTime::CurrentUTC();
	if(!intersection.isEmpty())
	{
		// Clip incoming triangles against the intersection boundary on the incoming triangles.
		meshclip.ClipTriangles(*this, intersection);
		//qDebug() << "AuMeatMesh::Join meshclip Clipped" << meshclip.size() << AuTime::CurrentUTC();

		// Clip my triangles against the intersection boundary on my triangles.
		myclip.ClipTriangles(mesh, intersection);
		//qDebug() << "AuMeatMesh::Join myclip Clipped" << myclip.size() << AuTime::CurrentUTC();
	}

	// Add the clipped triangles to the result even if we didn't find any intersection segs, so we get something even if it's wrong.
	//result.Clear();
	result.Append(meshclip, true);
	result.Append(myclip, true);
	//qDebug() << "AuMeatMesh::Join result clipped" << result.size() << AuTime::CurrentUTC();

	// We must avoid undesirable T intersections. They don't smooth well and they really don't round well, often leaving gaps.
	// This is really slow.
	//result.SplitTees(d_TriangleTeeTol);
	//qDebug() << "AuMeatMesh::Join tees split" << result.size() << AuTime::CurrentUTC();

	if(true)
	{
		// Rebuild this entire mesh, joining vertices together where they are close.
		// Triangles that get smashed are removed.
		Clear();
		for(const_iterator mi = result.cbegin(); mi != result.cend(); mi++)
		{
			const AuMeatVertex& v0 = result.Vertex((*mi).m_V0.Index());
			const AuMeatVertex& v1 = result.Vertex((*mi).m_V1.Index());
			const AuMeatVertex& v2 = result.Vertex((*mi).m_V2.Index());

			int i0 = VertexIndexAdd(v0, d_VertexRound);
			int i1 = VertexIndexAdd(v1, d_VertexRound);
			int i2 = VertexIndexAdd(v2, d_VertexRound);

			// Skip triangles that collapse.
			if(i0 == i1 || i0 == i2 || i1 == i2)
				continue;

			// Stronger test for collapsed triangles.
			AuPoint vx = v2 - v0;
			vx.Unit();
			AuPoint vy = v1 - v0;
			vy.Unit();
			if(vx.IsParallel(vy))
				continue;

			AuMeatTri t(*mi);
			t.m_V0.m_VertexIndex = i0;
			t.m_V1.m_VertexIndex = i1;
			t.m_V2.m_VertexIndex = i2;

			Insert(mi.key(), t, *this, true);
		}
		//qDebug() << "AuMeatMesh::Join vertices joined" << size() << AuTime::CurrentUTC();
	}
	else
		*this = result;	// Sometimes during testing it is handy to see the raw unrounded triangles.

	// Smooth the normals of this resulting mesh.
	//SmoothNormalsArea();	// No noticeable difference over SmoothNormals.
	//SmoothNormals();

	qDebug() << "AuMeatMesh::Join result" << size() << "tris" << m_Vertices.size() << "vertices" << AuTime::CurrentUTC();
}
*/

/*
void AuMeatMesh::ClipTriangles(const AuMeatMesh& mesh, const AuSegmentArray& intersection)
{
	Q_UNUSED(mesh);
	// Clip my triangles against the intersection boundary.
	bool triclipped;
	do
	{
		triclipped = false;
		AuMeatMesh newtris;
		for(AuMeatMesh::iterator mi = begin(); mi != end(); mi++)
		{
			// Clip the triangle using intersection line. It could produce many triangles that are added to newtris.
			bool clippedit = false;
			for(AuSegmentArray::const_iterator bi = intersection.cbegin(); bi != intersection.cend(); bi++)
				if((*bi).m_fLoc1 == (*bi).m_fLoc2)
				{
					// Zero length segment, probably an edge hit. Just break the triangle at that one point.
					if((*mi).Clip((*bi).m_fLoc1, newtris, *this))
					{
						clippedit = true;
						break;
					}
				}
				else if((*mi).Clip(*bi, newtris, *this))
				{
					clippedit = true;
					break;
				}

			// What happened?
			if(!clippedit)
				newtris.Insert(mi.key(), *mi, *this, true);	// Triangle was not clipped so keep it.
			else
				triclipped = true;				// Triangle was clipped. Result was already added to newtris.
		}

		// Each time we make a pass and clip at least one triangle, we have to update ourself.
		if(triclipped)
			*this = newtris;
	}
	while(triclipped);

	// Discard resulting triangles that have a vertex inside the mesh.
	AuMeatMesh::iterator mi = begin();
	while(mi != end())
	{
		const AuMeatVertex& v0 = Vertex((*mi).m_V0.Index());
		const AuMeatVertex& v1 = Vertex((*mi).m_V1.Index());
		const AuMeatVertex& v2 = Vertex((*mi).m_V2.Index());
		if(v0.InsideMesh() == 2 || v1.InsideMesh() == 2 || v2.InsideMesh() == 2)
		{
			// One vertex is inside the mesh. Check the center point just to be sure.
			if(mesh.TestInside(mi.key()) == 2)
				mi = erase(mi);		// Discard the triangle.
			else
				mi++;
		}
		else
			mi++;
	}
	//qDebug() << "AuMeatMesh::ClipTriangles done tris" << size();
}
*/

/*
// Finds the intersection between each triangle in mesh with my triangles. Adds the segments to 'segs'.
void AuMeatMesh::IntersectionSegments(AuSegmentArray& segs, const AuMeatMesh& mesh) const
{
	// Check all my triangles against the incoming triangles.
	AuPoint seg1, seg2;
	for(AuMeatMesh::const_iterator mi = mesh.cbegin(); mi != mesh.cend(); mi++)
	{
		// Our inside mesh flags can save a lot of work at this point.
		// If all three vertices are completely inside or outside the mesh, there is no intersection.
		const AuMeatVertex& v0 = mesh.Vertex((*mi).m_V0.Index());
		const AuMeatVertex& v1 = mesh.Vertex((*mi).m_V1.Index());
		const AuMeatVertex& v2 = mesh.Vertex((*mi).m_V2.Index());
		if(v0.InsideMesh() == 0 && v1.InsideMesh() == 0 && v2.InsideMesh() == 0)
			continue;					// All three mesh vertices are outside this mesh and not touching my skin.
		else if(v0.InsideMesh() == 2 && v1.InsideMesh() == 2 && v2.InsideMesh() == 2)
			continue;					// All three mesh vertices are inside this mesh and not touching my skin.

		for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
		{
			// If all three vertices are completely inside or outside the mesh, there is no intersection.
			const AuMeatVertex& tv0 = Vertex((*ti).m_V0.Index());
			const AuMeatVertex& tv1 = Vertex((*ti).m_V1.Index());
			const AuMeatVertex& tv2 = Vertex((*ti).m_V2.Index());
			if(tv0.InsideMesh() == 0 && tv1.InsideMesh() == 0 && tv2.InsideMesh() == 0)
				continue;				// All three of my vertices are outside the mesh and not touching its skin.
			else if(tv0.InsideMesh() == 2 && tv1.InsideMesh() == 2 && tv2.InsideMesh() == 2)
				continue;				// All three of my vertices are inside the mesh and not touching its skin.

			if((*ti).TriangleIntersection(*mi, mesh, seg1, seg2, *this))
				segs.append(AuSegment(seg1, seg2));				// Add all segments, even zero-length ones; they are special.
		}
	}

	//qDebug() << "AuMeatMesh::IntersectionSegments" << segs.size();
}
*/

/*
int AuMeatMesh::TestInside(AuMeatMesh& mesh) const
{
	int r = 0;
	for(AuMeatVertexArray::iterator vi = mesh.m_Vertices.begin(); vi != mesh.m_Vertices.end(); vi++)
	{
		int inMesh = TestInside(*vi);
		(*vi).SetInsideMesh(inMesh);
		r |= 1<<inMesh;
	}
	return r;
}
*/

/*
// Checks to see if 'p' is contained within this meat mesh.
// Returns 0 if 'p' is outside this mesh.
// Returns 1 if 'p' is on the surface of this mesh.
// Returns 2 if 'p' is inside this mesh.
int AuMeatMesh::TestInside(const AuPoint& p) const
{
	// Test my extents box before doing any hard math.
	if(p.x < m_Min.x || p.y < m_Min.y || p.z < m_Min.z ||
			p.x > m_Max.x || p.y > m_Max.y || p.z > m_Max.z)
		return false;

	// Find closest triangle to p along a ray exiting p.
	// Note the direction of the ray is parallel to the Y axis due to the order of sorting of my triangles.
	AuPoint pp(p.x, p.y + 10000000, p.z);		// This should be a long enough "ray".
	double searchRad = d_MaxTriangleSize;	// Z distance above and below the center of each triangle we will test.
	double searchMinZ = p.z - searchRad;
	double searchMaxZ = p.z + searchRad;
	AuPoint intersect;
	AuMeatMesh::const_iterator foundi = cend();
	for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if(ti.key().z < searchMinZ || ti.key().z > searchMaxZ)
			continue;

		// Test for intersection between the triangle and the segment.
		if(!(*ti).SegmentIntersection(intersect, p, pp, *this))
			continue;					// Segment does not hit the triangle.

		// Our segment hit a triangle.
		if(p.FuzzyEquals(intersect))
			return 1;					// p is on the surface of this mesh.

		// Clip the ray at the triangle and keep on going.
		// When we're done, the end of the ray will be on the closest triangle.
		pp = intersect;
		foundi = ti;
	}

	// If we did not find any intersection at all then the ray was outside the mesh.
	if(foundi == cend())
		return 0;

	// Which side of the closest triangle is p on?
	if((p - Vertex((*foundi).m_V0.Index())).Dot((*foundi).SurfaceNormal(*this)) < 0)
		return 2;						// p is inside this mesh.
	return 0;							// p is outside this mesh.
}
*/

/*
AuPoint AuMeatMesh::InsideSkin(const AuPoint& p, const AuPoint& vec, AuPoint& skinN) const
{
	// Test my extents box before doing any hard math.
	if(p.x < m_Min.x || p.y < m_Min.y || p.z < m_Min.z ||
			p.x > m_Max.x || p.y > m_Max.y || p.z > m_Max.z)
		return AuPoint(0,0);

	// Find closest triangle to p along a ray exiting p.
	// Note the direction of the ray is parallel to the Y axis due to the order of sorting of my triangles.
	AuPoint pp(p + vec*10000000);			// This should be a long enough "ray".
	AuPoint intersect;
	AuMeatMesh::const_iterator foundi = cend();
	for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Test for intersection between the triangle and the segment.
		if(!(*ti).SegmentIntersection(intersect, p, pp, *this))
			continue;					// Segment does not hit the triangle.

		// Our segment hit a triangle.
		if(p.FuzzyEquals(intersect))
		{
			skinN = (*ti).SurfaceNormal(*this);
			return intersect;			// p is on the surface of this mesh.
		}

		// Clip the ray at the triangle and keep on going.
		// When we're done, the end of the ray will be on the closest triangle.
		pp = intersect;
		foundi = ti;
	}

	// If we did not find any intersection at all then the ray was outside the mesh.
	if(foundi == cend())
		return AuPoint(0,0);

	// Return the surface normal at the hit location.
	skinN = (*foundi).SurfaceNormal(*this);

	// Which side of the closest triangle is p on?
	if((p - Vertex((*foundi).m_V0.Index())).Dot(skinN) < 0)
		return pp;						// p is inside this mesh.
	return AuPoint(0,0);				// p is outside this mesh.
}
*/

AuPoint AuMeatMesh::SkinSurface(const AuPoint& p, const AuPoint& vec, AuPoint& skinN) const
{
	// Test my extents box before doing any hard math.
	if(p.x < m_Min.x || p.y < m_Min.y || p.z < m_Min.z ||
			p.x > m_Max.x || p.y > m_Max.y || p.z > m_Max.z)
		return AuPoint(0,0);

	// Find closest triangle to p along a ray exiting p.
	// Note the direction of the ray is parallel to the Y axis due to the order of sorting of my triangles.
	AuPoint pp(p + vec*10000000);			// This should be a long enough "ray".
	AuPoint intersect;
	AuMeatMesh::const_iterator foundi = cend();
	for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		// Test for intersection between the triangle and the segment.
		if(!(*ti).SegmentIntersection(intersect, p, pp))
			continue;					// Segment does not hit the triangle.

		// Our segment hit a triangle.
		if(p.FuzzyEquals(intersect))
		{
			skinN = (*ti).SurfaceNormalCalc();
			return intersect;			// p is on the surface of this mesh.
		}

		// Clip the ray at the triangle and keep on going.
		// When we're done, the end of the ray will be on the closest triangle.
		pp = intersect;
		foundi = ti;
	}

	// If we did not find any intersection at all then the ray was outside the mesh.
	if(foundi == cend())
		return AuPoint(0,0);

	// Return the surface normal at the hit location.
	skinN = (*foundi).SurfaceNormalCalc();

	// Return the closest intersection on the skin that was found.
	return pp;
}

AuPoint AuMeatMesh::SkinSurface(const AuPoint& p, const AuPoint& vec, AuPoint& skinN, AuMeatTri::PART partType) const
{
	// Test my extents box before doing any hard math.
	if(p.x < m_Min.x || p.y < m_Min.y || p.z < m_Min.z ||
			p.x > m_Max.x || p.y > m_Max.y || p.z > m_Max.z)
		return AuPoint(0,0);

	// Find closest triangle to p along a ray exiting p.
	// Note the direction of the ray is parallel to the Y axis due to the order of sorting of my triangles.
	AuPoint pp(p + vec*10000000);			// This should be a long enough "ray".
	AuPoint intersect;
	AuMeatMesh::const_iterator foundi = cend();
	for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if((*ti).Part() != partType)
			continue;
		// Test for intersection between the triangle and the segment.
		if(!(*ti).SegmentIntersection(intersect, p, pp))
			continue;					// Segment does not hit the triangle.

		// Our segment hit a triangle.
		if(p.FuzzyEquals(intersect))
		{
			skinN = (*ti).SurfaceNormalCalc();
			return intersect;			// p is on the surface of this mesh.
		}

		// Clip the ray at the triangle and keep on going.
		// When we're done, the end of the ray will be on the closest triangle.
		pp = intersect;
		foundi = ti;
	}

	// If we did not find any intersection at all then the ray was outside the mesh.
	if(foundi == cend())
		return AuPoint(0,0);

	// Return the surface normal at the hit location.
	skinN = (*foundi).SurfaceNormalCalc();

	// Return the closest intersection on the skin that was found.
	return pp;
}

/*
void AuMeatMesh::SplitTees(double tol)
{
	// Split all triangles that are hit by a vertex somewhere besides on a vertex.
	AuMeatMesh newtris;
	AuMeatMesh::iterator ti = begin();
	while(ti != end())
	{
		// Check all vertices to see if any of them land on an edge segment of the triangle.
		bool clippedTri = false;
		int vindex = 0;
		for(AuMeatVertexArray::const_iterator vi = m_Vertices.cbegin(); vi != m_Vertices.cend(); vi++, vindex++)
		{
			// Skip vertices that are on the triangle.
			if((*ti).m_V0.Index() == vindex ||
					(*ti).m_V1.Index() == vindex ||
					(*ti).m_V2.Index() == vindex)
				continue;

			if((*ti).Clip(*vi, newtris, *this, tol))
			{
				clippedTri = true;
				break;
			}
		}

		if(clippedTri)
			ti = erase(ti);
		else
			ti++;
	}

	// Append any new triangles to the end of this mesh.
	Append(newtris, true);
}
*/

void AuMeatMesh::SmoothNormals()
{
	QMultiMap<AuPoint, AuPoint> verts;	// Key is vertex, value is normal perpendicular to its triangle.
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
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
	for(iterator ti = begin(); ti != end(); ti++)
	{
		QMap<AuPoint, AuPoint>::const_iterator si = smoothVerts.find((*ti).m_V0);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V0))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V0.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormals triangle vertex 1 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V1);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V1))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V1.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormals triangle vertex 2 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V2);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V2))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V2.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormals triangle vertex 3 not in smooth vertices map";
	}
}

/*
void AuMeatMesh::SmoothNormals()
{
	QMultiMap<quint32, AuPoint> verts;	// Key is vertex index, value is normal perpendicular to its triangle.
	for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		AuPoint N = (*ti).SurfaceNormal(*this);
		verts.insert((*ti).m_V0.Index(), N);
		verts.insert((*ti).m_V1.Index(), N);
		verts.insert((*ti).m_V2.Index(), N);
	}

	quint32 curVert = 0;
	AuPoint curN;
	int curCount = 0;
	QMap<quint32, AuPoint> smoothVerts;	// Key is vertex index, value is smooth normal.
	for(QMultiMap<quint32, AuPoint>::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
	{
		if(curCount == 0)
		{
			// First vertex in the mulitimap.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}
		else if(vi.key() == curVert)
		{
			// Subsequent matching vertex. Accumulate normals so we can average them.
			curN += vi.value();
			curCount++;
		}
		else
		{
			// Subsequent vertex did not match. Calculate accumulated average of current normal.
			if(curCount > 1)
				curN.Unit();
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
				curN.Unit();
			smoothVerts.insert(curVert, curN);
		}
	}

	// Update all my triangles with new smooth normals.
	for(AuMeatMesh::iterator ti = begin(); ti != end(); ti++)
	{
		QMap<quint32, AuPoint>::const_iterator si = smoothVerts.find((*ti).m_V0.Index());
		if(si != smoothVerts.cend())
			(*ti).m_V0.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormals triangle vertex 1 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V1.Index());
		if(si != smoothVerts.cend())
			(*ti).m_V1.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormals triangle vertex 2 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V2.Index());
		if(si != smoothVerts.cend())
			(*ti).m_V2.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormals triangle vertex 3 not in smooth vertices map";
	}
}
*/

/*
void AuMeatMesh::SmoothNormalsArea()
{
	QMultiMap<quint32, const_iterator> verts;	// Key is vertex index, value is normal perpendicular to its triangle.
	for(AuMeatMesh::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		verts.insert((*ti).m_V0.Index(), ti);
		verts.insert((*ti).m_V1.Index(), ti);
		verts.insert((*ti).m_V2.Index(), ti);
	}

	quint32 curVert = 0;
	AuPoint curN;
	int curCount = 0;
	QMap<quint32, AuPoint> smoothVerts;	// Key is vertex index, value is smooth normal.
	for(QMultiMap<quint32, const_iterator>::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
	{
		if(curCount == 0)
		{
			// First vertex in the mulitimap.
			curVert = vi.key();
			curN = (*vi.value()).SurfaceNormal(*this) * (*vi.value()).Area(*this);
			curCount = 1;
		}
		else if(vi.key() == curVert)
		{
			// Subsequent matching vertex. Accumulate normals so we can average them.
			curN += ((*vi.value()).SurfaceNormal(*this) * (*vi.value()).Area(*this));
			curCount++;
		}
		else
		{
			// Subsequent vertex did not match. Calculate accumulated average of current normal.
			if(curCount > 0)
				curN.Unit();
			smoothVerts.insert(curVert, curN);

			// Start accumulating next vertex.
			curVert = vi.key();
			curN = (*vi.value()).SurfaceNormal(*this) * (*vi.value()).Area(*this);
			curCount = 1;
		}

		if(vi == verts.cend()-1)
		{
			// We just finished the last vertex. Close out our last accumulated vertex normal.
			if(curCount > 0)
				curN.Unit();
			smoothVerts.insert(curVert, curN);
		}
	}

	// Update all my triangles with new smooth normals.
	for(AuMeatMesh::iterator ti = begin(); ti != end(); ti++)
	{
		QMap<quint32, AuPoint>::const_iterator si = smoothVerts.find((*ti).m_V0.Index());
		if(si != smoothVerts.cend())
			(*ti).m_V0.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormalsArea triangle vertex 1 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V1.Index());
		if(si != smoothVerts.cend())
			(*ti).m_V1.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormalsArea triangle vertex 2 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V2.Index());
		if(si != smoothVerts.cend())
			(*ti).m_V2.m_N = *si;
		else
			qWarning() << "error: AuMeatMesh::SmoothNormalsArea triangle vertex 3 not in smooth vertices map";
	}
}
*/

AuGLVAO AuMeatMesh::CreateLinesVAO(AuOpenGLFunctions& re) const
{
	// Store count of vertices in the vertex array object.
	int vertices = count() * 6;

	// Copy vertices into gl array buffer.
	// This array buffer corresponds literally to the 'in vec4 position' variable in the vertex shader.
	GLfloat* position = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	for(AuMeatMesh::const_iterator ti = constBegin(); ti != constEnd(); ti++)
	{
		const AuPointN& v0 = (*ti).m_V0;
		const AuPointN& v1 = (*ti).m_V1;
		const AuPointN& v2 = (*ti).m_V2;

		// Edge 0
		position[pidx++] = (float)v0.x;
		position[pidx++] = (float)v0.y;
		position[pidx++] = (float)v0.z;
		position[pidx++] = 1.f;				// Alpha

		position[pidx++] = (float)v1.x;
		position[pidx++] = (float)v1.y;
		position[pidx++] = (float)v1.z;
		position[pidx++] = 1.f;				// Alpha

		// Edge 1
		position[pidx++] = (float)v1.x;
		position[pidx++] = (float)v1.y;
		position[pidx++] = (float)v1.z;
		position[pidx++] = 1.f;				// Alpha

		position[pidx++] = (float)v2.x;
		position[pidx++] = (float)v2.y;
		position[pidx++] = (float)v2.z;
		position[pidx++] = 1.f;				// Alpha

		// Edge 2
		position[pidx++] = (float)v2.x;
		position[pidx++] = (float)v2.y;
		position[pidx++] = (float)v2.z;
		position[pidx++] = 1.f;				// Alpha

		position[pidx++] = (float)v0.x;
		position[pidx++] = (float)v0.y;
		position[pidx++] = (float)v0.z;
		position[pidx++] = 1.f;				// Alpha
	}

	// Generate vertex array object with a buffer for each array.
	AuGLVAO vao;
	vao.CreateGL(re, vertices, 1);

	vao.SelectGL();

	// bind buffer for positions and copy data into buffer.
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
	re.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, GL_STATIC_DRAW);
	re.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] position;

	return vao;
}

AuGLVAO AuMeatMesh::CreateAnimalVAO(AuOpenGLFunctions& re) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoordTopBottom = new GLfloat[vertices*4];
	GLfloat* texCoordDetailBlend = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int bti = 0;
	unsigned int dbi = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuPointN& v0 = (*ti).m_V0;
		const AuPointN& v1 = (*ti).m_V1;
		const AuPointN& v2 = (*ti).m_V2;

		// Vertex 0
		positionShine[pidx++] = (float)v0.x;
		positionShine[pidx++] = (float)v0.y;
		positionShine[pidx++] = (float)v0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).z;
#else
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.z;
#endif
		normalReflect[nidx++] = (float)(*ti).m_V0.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinTopTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinTopTy;
		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinBottomTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinBottomTy;

		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_DecalTx;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_DecalTy;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_SkinTopBottomBlend;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_DecalBlend;

		// Vertex 1
		positionShine[pidx++] = (float)v1.x;
		positionShine[pidx++] = (float)v1.y;
		positionShine[pidx++] = (float)v1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).z;
#else
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.z;
#endif
		normalReflect[nidx++] = (float)(*ti).m_V1.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinTopTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinTopTy;
		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinBottomTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinBottomTy;

		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_DecalTx;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_DecalTy;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_SkinTopBottomBlend;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_DecalBlend;

		// Vertex 2
		positionShine[pidx++] = (float)v2.x;
		positionShine[pidx++] = (float)v2.y;
		positionShine[pidx++] = (float)v2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).z;
#else
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.z;
#endif
		normalReflect[nidx++] = (float)(*ti).m_V2.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];

		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinTopTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinTopTy;
		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinBottomTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinBottomTy;

		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_DecalTx;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_DecalTy;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_SkinTopBottomBlend;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_DecalBlend;
	}

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
	re.glBufferData(GL_ARRAY_BUFFER, bti*sizeof(GLfloat), texCoordTopBottom, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dbi*sizeof(GLfloat), texCoordDetailBlend, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoordTopBottom;
	delete [] texCoordDetailBlend;
	return vao;
}

AuGLVAO AuMeatMesh::CreateHeadVAO(AuOpenGLFunctions& re) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	int hcount = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if((*ti).IsHead())
			hcount++;
	if(!hcount)
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = hcount*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoordTopBottom = new GLfloat[vertices*4];
	GLfloat* texCoordDetailBlend = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int bti = 0;
	unsigned int dbi = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if(!(*ti).IsHead())
			continue;

		const AuPointN& v0 = (*ti).m_V0;
		const AuPointN& v1 = (*ti).m_V1;
		const AuPointN& v2 = (*ti).m_V2;

		// Vertex 0
		positionShine[pidx++] = (float)v0.x;
		positionShine[pidx++] = (float)v0.y;
		positionShine[pidx++] = (float)v0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).z;
#else
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V0.m_N.z;
#endif
		normalReflect[nidx++] = (float)(*ti).m_V0.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinTopTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinTopTy;
		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinBottomTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V0.m_SkinBottomTy;

		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_DecalTx;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_DecalTy;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_SkinTopBottomBlend;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V0.m_DecalBlend;

		// Vertex 1
		positionShine[pidx++] = (float)v1.x;
		positionShine[pidx++] = (float)v1.y;
		positionShine[pidx++] = (float)v1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).z;
#else
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V1.m_N.z;
#endif
		normalReflect[nidx++] = (float)(*ti).m_V1.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinTopTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinTopTy;
		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinBottomTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V1.m_SkinBottomTy;

		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_DecalTx;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_DecalTy;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_SkinTopBottomBlend;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V1.m_DecalBlend;

		// Vertex 2
		positionShine[pidx++] = (float)v2.x;
		positionShine[pidx++] = (float)v2.y;
		positionShine[pidx++] = (float)v2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormalCalc(*this).z;
#else
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalReflect[nidx++] = (float)(*ti).m_V2.m_N.z;
#endif
		normalReflect[nidx++] = (float)(*ti).m_V2.m_Reflect;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];

		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinTopTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinTopTy;
		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinBottomTx;
		texCoordTopBottom[bti++] = (float)(*ti).m_V2.m_SkinBottomTy;

		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_DecalTx;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_DecalTy;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_SkinTopBottomBlend;
		texCoordDetailBlend[dbi++] = (float)(*ti).m_V2.m_DecalBlend;
	}

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
	re.glBufferData(GL_ARRAY_BUFFER, bti*sizeof(GLfloat), texCoordTopBottom, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dbi*sizeof(GLfloat), texCoordDetailBlend, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoordTopBottom;
	delete [] texCoordDetailBlend;
	return vao;
}

/*
void AuMeatMesh::Insert(const AuPoint& center, const AuMeatTri& t, const AuMeatMesh& sourceMesh, bool combineVerts)
{
	iterator nt = insert(center, t);

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

void AuMeatMesh::SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2,
		float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}
	m_HairMesh.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
}

/*
void AuMeatMesh::SetBonesRangeY(double minY, double maxY, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1,
		float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	double y;
	for(iterator pi = begin(); pi != end(); pi++)
	{
		y = (*pi).m_V0.y;
		if(y >= minY && y <= maxY)
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);

		y = (*pi).m_V1.y;
		if(y >= minY && y <= maxY)
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);

		y = (*pi).m_V2.y;
		if(y >= minY && y <= maxY)
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}
}
*/

void AuMeatMesh::SetBonesRangeZ(double minZ, double maxZ, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1,
		float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	double z;
	for(iterator pi = begin(); pi != end(); pi++)
	{
		z = (*pi).m_V0.z;
		if(z >= minZ && z <= maxZ)
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);

		z = (*pi).m_V1.z;
		if(z >= minZ && z <= maxZ)
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);

		z = (*pi).m_V2.z;
		if(z >= minZ && z <= maxZ)
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}

	m_HairMesh.SetBonesRangeZ(minZ, maxZ, boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3,
			boneWeight3);
}

void AuMeatMesh::SetBonesRangeYZ(double minY, double maxY, double minZ, double maxZ, quint8 boneIndex0, float boneWeight0,
		quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	double y;
	double z;
	for(iterator pi = begin(); pi != end(); pi++)
	{
		y = (*pi).m_V0.y;
		z = (*pi).m_V0.z;
		if(y >= minY && y <= maxY &&
				z >= minZ && z <= maxZ)
			(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);

		y = (*pi).m_V1.y;
		z = (*pi).m_V1.z;
		if(y >= minY && y <= maxY &&
				z >= minZ && z <= maxZ)
			(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);

		y = (*pi).m_V2.y;
		z = (*pi).m_V2.z;
		if(y >= minY && y <= maxY &&
				z >= minZ && z <= maxZ)
			(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}

	m_HairMesh.SetBonesRangeYZ(minY, maxY, minZ, maxZ, boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2,
			boneIndex3, boneWeight3);
}

void AuMeatMesh::AddBoneRangeBlendX(double fromX, double toX, int bone, quint8 boneIndex)
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

	m_HairMesh.AddBoneRangeBlendX(fromX, toX, bone, boneIndex);
}

void AuMeatMesh::AddBoneRangeBlendXMouth(double fromX, double toX, int bone, quint8 boneIndex)
{
	float boneWeight;
	double range = toX - fromX;	// This will be negative when toX is less than fromX.
	double x;
	if(toX > fromX)
	{
		for(iterator pi = begin(); pi != end(); pi++)
		{
			if(!(*pi).IsMouth())
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
			if(!(*pi).IsMouth())
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

	m_HairMesh.AddBoneRangeBlendXMouth(fromX, toX, bone, boneIndex);
}

void AuMeatMesh::SetBonesRangeBlendY(double minY, double maxY, quint8 boneMin, quint8 boneMax)
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

	m_HairMesh.SetBonesRangeBlendY(minY, maxY, boneMin, boneMax);
}

void AuMeatMesh::SetBonesRangeBlendZ(double minZ, double maxZ, quint8 boneMin, quint8 boneMax)
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

	m_HairMesh.SetBonesRangeBlendZ(minZ, maxZ, boneMin, boneMax);
}

void AuMeatMesh::SetPartRangeZ(AuMeatTri::PART part, double minZ, double maxZ)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		double z0 = (*pi).m_V0.z;
		double z1 = (*pi).m_V1.z;
		double z2 = (*pi).m_V2.z;
		if(z0 >= minZ && z0 <= maxZ &&
				z1 >= minZ && z1 <= maxZ &&
				z2 >= minZ && z2 <= maxZ)
			(*pi).SetPart(part);
	}
}

void AuMeatMesh::FromFaces(const AuScFaceList& faces, AuMeatTri::PART part)
{
	int boneIndex[4] = {0,0,0,0};
	float boneWeight[4] = {1,0,0,0};

	QColor themeColors[AuScFace::THEME_Count];
	for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend(); fi++)
	{
		AuScTriRenderList opaqueTris, transparentTris;
		(*fi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
		Q_ASSERT(transparentTris.empty());

		for(AuScTriRenderList::const_iterator ti = opaqueTris.cbegin(); ti != opaqueTris.cend(); ti++)
		{
			float shine = (*ti).Shine();
			float reflect = (*ti).Reflectivity();

			const AuScPoint& v1 = (*ti).Vertex(0);
			const AuScPoint& v2 = (*ti).Vertex(1);
			const AuScPoint& v3 = (*ti).Vertex(2);

			// Calcualate top/bottom blend at each vertex.
			float tb1 = 0;	// Top/bottom blend at v1. 0=bottom, 1=top.
			float tb2 = 0;	// Top/bottom blend at v2. 0=bottom, 1=top.
			float tb3 = 0;	// Top/bottom blend at v3. 0=bottom, 1=top.

			AuMeatPoint mp0(v1, v1.m_N, shine, reflect, boneIndex,boneWeight, v1.m_Tx,v1.m_Ty, tb1, v1.m_Tx,v1.m_Ty);
			AuMeatPoint mp1(v2, v2.m_N, shine, reflect, boneIndex,boneWeight, v2.m_Tx,v2.m_Ty, tb2, v2.m_Tx,v2.m_Ty);
			AuMeatPoint mp2(v3, v3.m_N, shine, reflect, boneIndex,boneWeight, v3.m_Tx,v3.m_Ty, tb3, v3.m_Tx,v3.m_Ty);

			AuMeatTri tri(part, mp0, mp1, mp2);
			Append(tri);
		}
	}
}

void AuMeatMesh::FromFaces(const AuScFaceList& faces, AuMeatTri::PART part, int tex, const AuPoint& upVec, float blendAngle,
		float blendWid)
{
	int boneIndex[4] = {0,0,0,0};
	float boneWeight[4] = {1,0,0,0};

	QColor themeColors[AuScFace::THEME_Count];
	for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend(); fi++)
	{
		AuScTriRenderList opaqueTris, transparentTris;
		(*fi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
		Q_ASSERT(transparentTris.empty());

		for(AuScTriRenderList::const_iterator ti = opaqueTris.cbegin(); ti != opaqueTris.cend(); ti++)
		{
			float shine = (*ti).Shine();
			float reflect = (*ti).Reflectivity();

			const AuScPoint& v1 = (*ti).Vertex(0);
			const AuScPoint& v2 = (*ti).Vertex(1);
			const AuScPoint& v3 = (*ti).Vertex(2);

			// Calcualate top/bottom blend at each vertex.
			float tb1;	// Top/bottom blend at v1. 0=bottom, 1=top.
			float tb2;	// Top/bottom blend at v2. 0=bottom, 1=top.
			float tb3;	// Top/bottom blend at v3. 0=bottom, 1=top.
			if(tex == 0)
			{
				tb1 = 0;
				tb2 = 0;
				tb3 = 0;
			}
			else if(tex == 1)
			{
				tb1 = 1;
				tb2 = 1;
				tb3 = 1;
			}
			else
			{
				// Blend zone is centered 10 degrees below the equator line and is 10 degrees wide.
				tb1 = AuMeatTri::TopBottomBlend(v1.m_N, upVec, blendAngle, blendWid);
				tb2 = AuMeatTri::TopBottomBlend(v2.m_N, upVec, blendAngle, blendWid);
				tb3 = AuMeatTri::TopBottomBlend(v3.m_N, upVec, blendAngle, blendWid);
			}

			AuMeatPoint mp0(v1, v1.m_N, shine, reflect, boneIndex,boneWeight, v1.m_Tx,v1.m_Ty, tb1, v1.m_Tx,v1.m_Ty);
			AuMeatPoint mp1(v2, v2.m_N, shine, reflect, boneIndex,boneWeight, v2.m_Tx,v2.m_Ty, tb2, v2.m_Tx,v2.m_Ty);
			AuMeatPoint mp2(v3, v3.m_N, shine, reflect, boneIndex,boneWeight, v3.m_Tx,v3.m_Ty, tb3, v3.m_Tx,v3.m_Ty);

			AuMeatTri tri(part, mp0, mp1, mp2);
			Append(tri);
		}
	}
}

void AuMeatMesh::ApplySkinTexture(const AuPoint& origin, const AuPoint& yAxis, const AuPoint& upVec, float length,
		int topBottomBlend, float blendAngle, float blendWid)
{
	// Create a matrix to transform my vertices into the space of the texture, so I can measure the extents in that way.
	AuPoint mx(yAxis.Cross(upVec));
	mx.Unit();
	AuPoint my(upVec.Cross(mx));
	my.Unit();
	AuMatrix4x4 mTex(origin, mx, my);

	for(iterator ti = begin(); ti != end(); ti++)
		(*ti).ApplySkinTexture(mTex, length, topBottomBlend, blendAngle, blendWid);
}

void AuMeatMesh::ExpandExtents(const AuMeatTri& t)
{
	m_Min = m_Min.Min(t.m_V0);
	m_Min = m_Min.Min(t.m_V1);
	m_Min = m_Min.Min(t.m_V2);

	m_Max = m_Max.Max(t.m_V0);
	m_Max = m_Max.Max(t.m_V1);
	m_Max = m_Max.Max(t.m_V2);
}

void AuMeatMesh::SurfacePoints(AuPointNArray& points) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		points.append(AuPointN((*ti).Center(), (*ti).SurfaceNormalCalc()));
}

void AuMeatMesh::RemoveAbove(double z)
{
	iterator ti = begin();
	while(ti != end())
	{
		if((*ti).Above(z))
			ti = erase(ti);
		else
			ti++;
	}
	m_HairMesh.RemoveAbove(z);
}

void AuMeatMesh::RemoveBelow(double z)
{
	iterator ti = begin();
	while(ti != end())
	{
		if((*ti).Below(z))
			ti = erase(ti);
		else
			ti++;
	}
	m_HairMesh.RemoveBelow(z);
}

bool AuMeatMesh::Bones(AuMeatTri::PART part, quint8 boneIndex[4], float boneWeight[4]) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if((*ti).Part() == part)
		{
			for(int b=0; b<4; b++)
			{
				boneIndex[b] = (*ti).m_V0.m_BoneIndex[b];
				boneWeight[b] = (*ti).m_V0.m_BoneWeight[b];
			}
			return true;
		}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
void AuMeat::LoadPoly(const QString& fileName, AuPointArray& poly, float wid, float ht)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: AuMeat::LoadPoly" << fileName << "failed to open file";
		return;
	}

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_4_1);
	quint8 fileType, fileVer;
	ds >> fileType;
	ds >> fileVer;

	AuPointArray joints;
	if(fileType == MEATLINE_FILE_TYPE && fileVer == MEATLINE_FILE_VER)
	{
		ds >> poly;
		ds >> joints;
	}

	file.close();

	if(poly.isEmpty())
	{
		qWarning() << "error: AuMeat::LoadPoly" << fileName << "no poly";
		return;
	}
	else if(joints.isEmpty())
	{
		qWarning() << "error: AuMeat::LoadPoly" << fileName << "no joints";
		return;
	}

	// Scale the result.
	AuPoint min, max;
	poly.Extents(min, max);
	//qDebug() << "Loaded poly" << poly.size() << "vertexes extents" << min << max << "joint" << joints.first();

	double sx;
	if(max.x != min.x)
		sx = wid / (max.x - min.x);
	else
		sx = 1;

	double sy;
	if(max.y != min.y)
		sy = ht / (max.y - min.y);
	else
		sy = 1;

	AuMatrix4x4 m;
	m.Scale(sx, sy, 1);
	m.Translate(-joints.first());
	poly.Transform(m);

	poly.Extents(min, max);
	//qDebug() << "Loaded poly" << poly.size() << "vertexes final extents" << min << max << "joint" << joints.first();
}

//static
void AuMeat::DecalTexture(MEAT_DECAL tx, float& s, float& t, float& u, float& v)
{
	class TexPos
	{
	public:
		MEAT_DECAL texId;
		int ox, oy;			// pixel coordinate of lower left corner of texture (pos y goes up)
		int sx, sy;			// size of texture in pixels

		TexPos(MEAT_DECAL detailTexEnum, int offsetX, int offsetY, int sizeX, int sizeY)
		{
			texId = detailTexEnum;
			ox = offsetX;
			oy = offsetY;
			sx = sizeX;
			sy = sizeY;
		}
	};

	const TexPos texPos[] =
	{
		TexPos(MD_HumanEye1, 0, 64*7, 64, 64),
		TexPos(MD_HumanEye2, 3*16, 64*7, 64, 64),
		TexPos(MD_HumanEye3, 6*16, 64*7, 64, 64),
		TexPos(MD_HumanEye4, 9*16, 64*7, 64, 64),
		TexPos(MD_HumanEye5, 12*16, 64*7, 64, 64),
		TexPos(MD_HumanEye6, 15*16, 64*7, 64, 64),
		TexPos(MD_HumanEye7, 18*16, 64*7, 64, 64),
		TexPos(MD_HumanEye8, 21*16, 64*7, 64, 64),
		TexPos(MD_HumanEye9, 24*16, 64*7, 64, 64),
		TexPos(MD_HumanEye10, 27*16, 64*7, 64, 64),
		TexPos(MD_HumanMouth1, 0, 16*24, 16*6, 16*4),
		TexPos(MD_HumanMouth2, 6*16, 16*24, 16*6, 16*4),
		TexPos(MD_HumanMouth3, 12*16, 16*24, 16*6, 16*4),
		TexPos(MD_HumanMouth4, 18*16, 16*24, 16*6, 16*4),
		TexPos(MD_HumanMouth5, 24*16, 16*24, 16*6, 16*4),
		TexPos(MD_HumanMouth6, 0, 16*20, 16*6, 16*4),
		TexPos(MD_HumanMouth7, 6*16, 16*20, 16*6, 16*4),
		TexPos(MD_HumanMouth8, 12*16, 16*20, 16*6, 16*4),
		TexPos(MD_HumanMouth9, 18*16, 16*20, 16*6, 16*4),
		TexPos(MD_HumanMouth10, 24*16, 16*20, 16*6, 16*4),
		TexPos(MD_Teeth, 0, 64*4, 64, 64),
		TexPos(MD_Nipple, 64*1, 64*4, 64, 64),
	};
	int texPosCount = sizeof(texPos)/sizeof(TexPos);
	if(tx > texPosCount)
		tx = MD_HumanEye1;
	Q_ASSERT(texPos[tx].texId == tx);	// Verify data validity.

	// Calculate texture coordinates.
	const double maxx = 512;
	const double maxy = 512;
	s = (texPos[tx].ox + .5) / maxx;
	t = (texPos[tx].oy + .5) / maxy;
	u = (texPos[tx].ox + texPos[tx].sx - .5) / maxx;
	v = (texPos[tx].oy + texPos[tx].sy - .5) / maxy;
}

