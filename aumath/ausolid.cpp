//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausolid.cpp,v 1.15 2022/02/19 18:38:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausolid.h"
//
#include "auboundary.h"
#include "aubox.h"
#include "auglobell.h"
#include "auscface.h"
#include "ausphere.h"

bool AuSolidShapes::Extents(AuPoint& min, AuPoint& max) const
{
	if(empty())
		return false;
	const_iterator si = cbegin();
	(*si)->Extents(min, max);
	AuPoint smin, smax;
	while(si != cend())
	{
		(*si)->Extents(smin, smax);
		min = min.Min(smin);
		max = max.Max(smax);
		si++;
	}
	return true;
}

void AuSolidShapes::TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		(*si)->TopologyAtZ(topo, zHeight, stepSize);
}

void AuSolidShapes::Add(const AuSolidShape& s)
{
	append(s.Clone());
}

void AuSolidShapes::Faces(AuScFaceList& faces, double stepSize) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		(*si)->Faces(faces, stepSize);
}

void AuSolidShapes::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		(*si)->SurfacePoints(points, stepSize);
}

bool AuSolidShapes::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	bool anyHit = false;
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si)->SightTarget(eyeLoc, sightLoc))
			anyHit = true;
	return anyHit;
}

void AuSolidShapes::Transform(const AuMatrix4x4& m)
{
	for(iterator si = begin(); si != end(); si++)
		(*si)->Transform(m);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidCylinder::Clone() const
{
	return new AuSolidCylinder(*this);
}

void AuSolidCylinder::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_Radius, -m_Radius, 0);
	max = AuPoint(m_Radius, m_Radius, m_Height);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidCylinder::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	if(mp.z <= 0 || mp.z >= m_Height)
		return false;
	return mp.Length2_2D() < m_Radius*m_Radius;
}

void AuSolidCylinder::TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const
{
	AuPoint base(M().Translation());		// My base center in the world space.
	AuPoint top(M().Project(0,0,m_Height));	// My top center in the world space.
	AuPoint up(top - base);
	if(up.IsFuzzyZero_2D())
	{
		// I am vertical enough to be sliced into circles.
		if(top.z < zHeight && base.z < zHeight)
			return;							// Z plane is below this cylinder.
		else if(top.z > zHeight && base.z > zHeight)
			return;							// Z plane is above this cylinder.
		AuPoint center(base.x, base.y, zHeight);

		// What is my XY radius at zHeight?
		double circ = PI * m_Radius * 2;	// Circumference at radz.
		int steps = ceil(circ / stepSize);
		if(steps < 5)
			steps = 5;

		// Add a circle to the topo complex.
		AuBoundary b;
		AuPoint radiusVec(m_Radius, 0, 0);
		for(int step=0; step<steps; step++)
		{
			double angle = PI*2 * step / steps;
			b.append(AuBoundaryPoint(center + radiusVec.RotateZ(angle), angle));
		}

		topo.Add(b);	// Add my topology, merging it with existing boundaries in topo.
		return;
	}
	else if(IsFuzzyZero(up.z))
	{
		// I am horizontal enough to be sliced with rectangles.
		double rz = zHeight - base.z;		// Relative Z vector to my center.
		if(fabs(rz) >= m_Radius)
			return;							// Z plane at zHeight does not intersect this horizontal cylinder.

		// What is my XY radius at zHeight?
		double sinz = rz / m_Radius;		// Sin of the angle to zHeight.
		double angz = asin(sinz);
		double cosz = cos(angz);

		AuPoint vcyl(up);
		vcyl.Unit();
		AuPoint vup(0,0,1);
		AuPoint vside(vcyl.Cross(vup));
		vside.Unit(cosz * m_Radius);
		vup.Unit(sinz * m_Radius);

		AuPoint c1(base - vside + vup);
		AuPoint c2(base + vside + vup);
		AuPoint c3(base + vside + vup + vcyl);
		AuPoint c4(base - vside + vup + vcyl);

		AuPointArray pa;
		pa.append(c1);
		pa.append(c2);
		pa.append(c3);
		pa.append(c4);

		AuBoundary b;
		b.FromPointArray(pa);
		topo.Add(b);
	}
	else
	{
		// I am at an angle to the Z plane.
		qDebug() << "AuSolidCylinder::TopologyAtZ at angle";
	}
}

void AuSolidCylinder::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_Radius * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	int levels = (int)ceil(m_Height / stepSize);
	if(levels < 1)
		levels = 1;

	if(m_FillBase)
	{
		AuScFaceList base;
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_Radius,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			AuPoint p = AuPoint(m_Radius,0,0).RotateZ(ang);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			base.AddFace(t);
			bp = bn;
		}
		base.Transform(M());
		faces.AddFaces(base);
	}

	if(m_FillTop)
	{
		AuScFaceList top;
		AuScPoint tc(0,0,m_Height, 0,0, AuPoint(0,0,1));
		AuScPoint tp(m_Radius,0,m_Height, 0,0, AuPoint(0,0,1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			AuPoint p = AuPoint(m_Radius,0,m_Height).RotateZ(ang);
			AuScPoint tn(p, 0,0, AuPoint(0,0,1));

			AuScFaceTri t(tc, tn, tp);
			top.AddFace(t);
			tp = tn;
		}
		top.Transform(M());
		faces.AddFaces(top);
	}

	// Spin sides in levels.
	AuScFaceList sides;
	double lht = (double)m_Height / levels;	// Height of each level.
	double bht = 0;							// Height at base of current level.
	for(int l=1; l<=levels; l++)
	{
		double tht;							// Height at top of current level.
		if(l == levels)
			tht = m_Height;					// For perfect seam at the top.
		else
			tht = bht + lht;				// Calculate height at top of the current level.

		// Add sides at this level.
		AuScPoint pb(m_Radius,0,bht, 0,0, AuPoint(1,0,0));
		AuScPoint pt(m_Radius,0,tht, 0,0, AuPoint(1,0,0));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			AuPoint n = AuPoint(1,0,0).RotateZ(ang);
			AuPoint b = AuPoint(m_Radius,0,bht).RotateZ(ang);
			AuPoint t = AuPoint(m_Radius,0,tht).RotateZ(ang);

			AuScPoint sb(b, 0,0, n);
			AuScPoint st(t, 0,0, n);

			AuScFaceQuad q(pb, pt, st, sb);
			sides.AddFace(q);
			pb = sb;
			pt = st;
		}
		bht = tht;
	}
	sides.Transform(M());
	faces.AddFaces(sides);
}

bool AuSolidCylinder::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	return false;	//DHH
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidSphere::Clone() const
{
	return new AuSolidSphere(*this);
}

void AuSolidSphere::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_Radius, -m_Radius, -m_Radius);
	max = AuPoint(m_Radius, m_Radius, m_Radius);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidSphere::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	return mp.Length2() < m_Radius*m_Radius;
}

void AuSolidSphere::TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const
{
	AuPoint center(M().Translation());	// My center in the world space.
	double rz = zHeight - center.z;		// Relative Z vector to my center.
	//qDebug() << "AuSolidSphere::TopologyAtZ c" << center << "radius" << m_Radius << "zHeight" << zHeight << "rz" << rz;
	if(fabs(rz) >= m_Radius)
		return;							// Z plane at zHeight does not intersect this sphere.
	center.z = zHeight;

	// What is my XY radius at zHeight?
	double sinz = rz / m_Radius;		// Sin of the angle to zHeight.
	double angz = asin(sinz);
	double cosz = cos(angz);
	double radz = cosz * m_Radius;
	double circ = PI * radz * 2;		// Circumference at radz.
	int steps = ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;

	// Add a circle to the topo complex.
	AuBoundary b;
	AuPoint radiusVec(radz, 0, 0);
	for(int step=0; step<steps; step++)
	{
		double angle = PI*2 * step / steps;
		b.append(AuBoundaryPoint(center + radiusVec.RotateZ(angle), angle));
	}

	topo.Add(b);	// Add my topology, merging it with existing boundaries in topo.
}

void AuSolidSphere::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_Radius * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	AuScFaceList sphere;
	sphere.MakeSphereUniform(AuPoint(0,0,0), m_Radius, steps, 1, M());
	faces.AddFaces(sphere);
}

void AuSolidSphere::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	double circ = PI * m_Radius * 2;		// Circumference
	int equatorSteps = (int)ceil(circ / stepSize);
	if(equatorSteps < 5)
		equatorSteps = 5;

	// Each pole will have a single vertex.
	// Equator has 'equatorSteps' vertexes.

	// Poles
	AuGlobeLL poleN(0,PI/2);									// North pole
	points.Append(poleN, m_Radius, M());
	AuGlobeLL poleS(0,-PI/2);									// South pole
	points.Append(poleS, m_Radius, M());

	// North and south hemisphere except poles and equator.
	int latdivshemi = equatorSteps/4;							// Latitude divisions per hemisphere.
	if(latdivshemi < 2)
		latdivshemi = 2;
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	for(int latinc=1; latinc<latdivshemi; latinc++)
	{
		// We can do both hemispheres at once.
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		int londivs = (int)ceilf(cosf(latitude) * equatorSteps);
		if(londivs < 3)
			londivs = 3;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		for(int loninc=0; loninc<londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			AuGlobeLL llN(longitude, latitude);					// North hemisphere
			AuGlobeLL llS(longitude, -latitude);				// South hemisphere
			points.Append(llN, m_Radius, M());
			points.Append(llS, m_Radius, M());
		}
	}

	// Equator
	int londivs = equatorSteps;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	for(int loninc=0; loninc<londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		AuGlobeLL llE(longitude, 0);							// Equator
		points.Append(llE, m_Radius, M());
	}
}

bool AuSolidSphere::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	// Test for intersection.
	AuPoint hit1, hit2;
	int intResult = Intersect(eyeLoc, sightLoc, hit1, hit2);
	if(!intResult)
		return false;											// Segment is entirely outside sphere.

	// Return the first intersection closest to eyeLoc.
	sightLoc = hit1;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidEllipsoid::Clone() const
{
	return new AuSolidEllipsoid(*this);
}

void AuSolidEllipsoid::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_RadiusX, -m_RadiusY, -m_RadiusZ);
	max = AuPoint(m_RadiusX, m_RadiusY, m_RadiusZ);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidEllipsoid::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	mp.y /= m_RadiusY/m_RadiusX;
	mp.z /= m_RadiusZ/m_RadiusX;
	return mp.Length2() < m_RadiusX*m_RadiusX;
}

void AuSolidEllipsoid::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_RadiusX * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	AuScFaceList sphere;
	sphere.MakeSphereUniform(AuPoint(0,0,0), m_RadiusX, steps, 1, AuMatrix4x4());
	AuMatrix4x4 ts;
	ts.Scale(1, m_RadiusY/m_RadiusX, m_RadiusZ/m_RadiusX);
	sphere.Transform(M() * ts);
	sphere.SmoothVertices();
	faces.AddFaces(sphere);
}

void AuSolidEllipsoid::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	double circ = PI * (m_RadiusX + m_RadiusY + m_RadiusZ)/3 * 2;		// Circumference, more or less
	int equatorSteps = (int)ceil(circ / stepSize);
	if(equatorSteps < 5)
		equatorSteps = 5;

	// Each pole will have a single vertex.
	// Equator has 'equatorSteps' vertexes.

	// Poles
	AuPointN np(AuPoint(0,0,m_RadiusZ), AuPoint(0,0,1));
	np.Transform(M());
	AuPointN sp(AuPoint(0,0,-m_RadiusZ), AuPoint(0,0,-1));
	sp.Transform(M());
	points.append(np);
	points.append(sp);

	// North and south hemisphere except poles and equator.
	int latdivshemi = equatorSteps/4;							// Latitude divisions per hemisphere.
	if(latdivshemi < 2)
		latdivshemi = 2;
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	for(int latinc=1; latinc<latdivshemi; latinc++)
	{
		// We can do both hemispheres at once.
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		double cz = cos(latitude);
		double sz = sin(latitude);
		int londivs = (int)ceilf(cosf(latitude) * equatorSteps);
		if(londivs < 3)
			londivs = 3;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		for(int loninc=0; loninc<londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			double cx = cos(longitude);
			double sx = sin(longitude);
			AuPoint pN(cx * m_RadiusX * cz, sx * m_RadiusY * cz, sz * m_RadiusZ);
			AuPoint pS(cx * m_RadiusX * cz, sx * m_RadiusY * cz, -sz * m_RadiusZ);
			AuPoint pnN(pN);
			pnN.Unit();
			AuPoint pnS(pS);
			pnS.Unit();
			AuPointN ppN(pN, pnN);
			ppN.Transform(M());
			points.append(ppN);
			AuPointN ppS(pS, pnS);
			ppS.Transform(M());
			points.append(ppS);
		}
	}

	// Equator
	int londivs = equatorSteps;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	for(int loninc=0; loninc<londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		double cx = cos(longitude);
		double sx = sin(longitude);
		AuPoint p(cx * m_RadiusX, sx * m_RadiusY, 0);
		AuPoint pn(cx, sx);
		AuPointN pp(p, pn);
		pp.Transform(M());
		points.append(pp);
	}
}

AuPoint AuSolidEllipsoid::RaiseToSurface(const AuPoint& p, double margin) const
{
	// Get Y and Z scaling factors.
	double sy = m_RadiusY/m_RadiusX;
	double sz = m_RadiusZ/m_RadiusX;

	AuPoint mp = M().UnProject(p);		// mp is in my space
	mp.y /= sy;
	mp.z /= sz;
	double mplen = mp.Length();
	if(mplen > m_RadiusX + margin)
		return p;						// p is not inside this ellipsoid, plus the margin

	// Elevate mp to my surface, plus the margin.
	mp.Unit(m_RadiusX + margin);
	mp.y *= sy;
	mp.z *= sz;
	return M().Project(mp);
}

AuPoint AuSolidEllipsoid::LowerToSurface(const AuPoint& p, double margin) const
{
	// Get Y and Z scaling factors.
	double sy = m_RadiusY/m_RadiusX;
	double sz = m_RadiusZ/m_RadiusX;

	AuPoint mp = M().UnProject(p);		// mp is in my space
	mp.y /= sy;
	mp.z /= sz;
	double mplen = mp.Length();
	if(mplen < m_RadiusX + margin)
		return p;						// p is not outside this ellipsoid, plus the margin

	// Lower mp to my surface, plus the margin.
	mp.Unit(m_RadiusX + margin);
	mp.y *= sy;
	mp.z *= sz;
	return M().Project(mp);
}

bool AuSolidEllipsoid::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	AuPoint myEye = M().UnProject(eyeLoc);		// myEye is relative to me
	AuPoint mySight = M().UnProject(sightLoc);	// mySight is relative to me

	if(myEye.x < -m_RadiusX && mySight.x < -m_RadiusX)
		return false;
	if(myEye.x > m_RadiusX && mySight.x > m_RadiusX)
		return false;
	if(myEye.y < -m_RadiusY && mySight.y < -m_RadiusY)
		return false;
	if(myEye.y > m_RadiusY && mySight.y > m_RadiusY)
		return false;
	if(myEye.z < -m_RadiusZ && mySight.z < -m_RadiusZ)
		return false;
	if(myEye.z > m_RadiusZ && mySight.z > m_RadiusZ)
		return false;

	myEye.y /= m_RadiusY/m_RadiusX;
	myEye.z /= m_RadiusZ/m_RadiusX;
	if(myEye.Length2() < m_RadiusX*m_RadiusX)
	{
		// Eye location is inside me.
		sightLoc = eyeLoc;
		return true;
	}

	mySight.y /= m_RadiusY/m_RadiusX;
	mySight.z /= m_RadiusZ/m_RadiusX;

	// Test against a sphere with my radius.
	//  -1 = no intersect, segment is inside sphere
	//   0 = no intersect, segment is outside sphere
	//   1 = 1 intersection, result in 'hit1'
	//   2 = 2 intersections, results in 'hit1' and 'hit2' where 'hit1' is the closest intersection to 'eyeLoc'.
	AuSolidSphere sphere(AuMatrix4x4(), m_RadiusX);
	AuPoint hit1, hit2;
	int intResult = sphere.Intersect(myEye, mySight, hit1, hit2);
	if(!intResult)
		return false;											// Segment is entirely outside sphere.

	// Return the first intersection closest to eyeLoc.
	hit1.y *= m_RadiusY/m_RadiusX;
	hit1.z *= m_RadiusZ/m_RadiusX;
	sightLoc = M().Project(hit1);
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidDome::Clone() const
{
	return new AuSolidDome(*this);
}

void AuSolidDome::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_Radius, -m_Radius, 0);
	max = AuPoint(m_Radius, m_Radius, m_Radius);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidDome::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	if(mp.z <= 0)
		return false;
	return mp.Length2() < m_Radius*m_Radius;
}

void AuSolidDome::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_Radius * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;

	AuMatrix4x4 tdome(M());
	tdome.RotateY(-PI/2);
	tdome.RotateX(PI/2);

	if(m_FillBase)
	{
		AuPointArray bprofile;
		bprofile.append(AuPoint(0,0));
		bprofile.append(AuPoint(0,-m_Radius));
		AuScFaceList base;
		base.MakeSpindle(bprofile, steps, AuMatrix4x4());
		base.Transform(tdome);
		base.FlattenVertexNormals();
		faces.AddFaces(base);
		//qDebug() << "AuSolidDome::Faces base" << base.size() << "steps" << steps;
	}

	AuPointArray dprofile;
	int dsteps = steps/4;
	if(dsteps < 2)
		dsteps = 2;
	dprofile.append(AuPoint(0,-m_Radius));
	double angle = PI/2 * .1 / dsteps;		// 1/10 the first angle
	double r = m_Radius * sin(angle);
	dprofile.append(AuPoint(r,-m_Radius));	// First point is exactly cylindrical for smoothing at rim to match up with cylinders.
	for(int d=1; d<=dsteps; d++)
	{
		double angle = PI/2 * d / dsteps;
		AuPoint p = AuPoint(0,-m_Radius).RotateZ(angle);
		dprofile.append(p);
	}

	AuScFaceList dome;
	dome.MakeSpindle(dprofile, steps, AuMatrix4x4());
	dome.Transform(tdome);
	faces.AddFaces(dome);
}

void AuSolidDome::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	AuSolidDomoid d(M(), m_Radius, m_Radius, m_Radius, m_FillBase);
	d.SurfacePoints(points, stepSize);
}

bool AuSolidDome::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	return false;	//DHH
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidDomoid::Clone() const
{
	return new AuSolidDomoid(*this);
}

void AuSolidDomoid::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_RadiusX, -m_RadiusY, 0);
	max = AuPoint(m_RadiusX, m_RadiusY, m_RadiusZ);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidDomoid::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	if(mp.z <= 0)
		return false;
	mp.y /= m_RadiusY/m_RadiusX;
	mp.z /= m_RadiusZ/m_RadiusX;
	return mp.Length2() < m_RadiusX*m_RadiusX;
}

void AuSolidDomoid::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_RadiusX * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;

	if(m_FillBase && m_RadiusX > 0 && m_RadiusY > 0)
	{
		AuScFaceList base;
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_RadiusX,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusX * ca;
			double y = m_RadiusY * sa;
			AuPoint p(x,y);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			base.AddFace(t);
			bp = bn;
		}
		base.Transform(M());
		faces.AddFaces(base);
	}

	AuPointArray dprofile;
	int dsteps = steps/4;
	if(dsteps < 2)
		dsteps = 2;
	dprofile.append(AuPoint(0,-m_RadiusX));
	double angle = PI/2 * .1 / dsteps;		// 1/10 the first angle
	double r = m_RadiusX * sin(angle);
	dprofile.append(AuPoint(r,-m_RadiusX));	// First point is exactly cylindrical for smoothing at rim to match up with cylinders.
	for(int d=1; d<=dsteps; d++)
	{
		double angle = PI/2 * d / dsteps;
		AuPoint p = AuPoint(0,-m_RadiusX).RotateZ(angle);
		dprofile.append(p);
	}

	AuMatrix4x4 tdome;
	tdome.Scale(1,m_RadiusY/m_RadiusX,m_RadiusZ/m_RadiusX);
	tdome.RotateY(-PI/2);
	tdome.RotateX(PI/2);

	AuScFaceList dome;
	dome.MakeSpindle(dprofile, steps, AuMatrix4x4());
	dome.Transform(tdome);
	dome.Transform(M());
	dome.SmoothVertices();
	faces.AddFaces(dome);
}

void AuSolidDomoid::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	double circ = PI * (m_RadiusX + m_RadiusY + m_RadiusZ)/3 * 2;		// Circumference, more or less
	int equatorSteps = (int)ceil(circ / stepSize);
	if(equatorSteps < 5)
		equatorSteps = 5;

	// Each pole will have a single vertex.
	// Equator has 'equatorSteps' vertexes.

	// Poles
	AuGlobeLL poleN(0,PI/2);									// North pole
	points.Append(poleN, m_RadiusZ, M());

	// North hemisphere except poles and equator.
	int latdivshemi = equatorSteps/4;							// Latitude divisions per hemisphere.
	if(latdivshemi < 2)
		latdivshemi = 2;
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	for(int latinc=1; latinc<latdivshemi; latinc++)
	{
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		double cz = cos(latitude);
		double sz = sin(latitude) * m_RadiusZ;
		int londivs = (int)ceilf(cosf(latitude) * equatorSteps);
		if(londivs < 3)
			londivs = 3;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		for(int loninc=0; loninc<londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			double cx = cos(longitude) * m_RadiusX * cz;
			double sx = sin(longitude) * m_RadiusY * cz;
			AuPoint p(cx, sx, sz);
			double radius = p.Length();
			AuGlobeLL llN(longitude, latitude);					// North hemisphere
			points.Append(llN, radius, M());
		}
	}

	// Equator
	int londivs = equatorSteps;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	for(int loninc=0; loninc<londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		double cx = cos(longitude) * m_RadiusX;
		double sx = sin(longitude) * m_RadiusY;
		AuPoint p(cx, sx, 0);
		double radius = p.Length_2D();
		AuGlobeLL llE(longitude, 0);							// Equator
		points.Append(llE, radius, M());
	}

	if(m_FillBase && m_RadiusX > 0 && m_RadiusY > 0)
	{
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_RadiusX,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=equatorSteps; i++)
		{
			double ang = PI*2 * i / equatorSteps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusX * ca;
			double y = m_RadiusY * sa;
			AuPoint p(x,y);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			AuPointN ptn(t.FaceLocation(), t.FaceNormal());
			ptn.Transform(M());
			points.append(ptn);
			bp = bn;
		}
	}
}

bool AuSolidDomoid::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	AuPoint myEye = M().UnProject(eyeLoc);		// myEye is relative to me
	AuPoint mySight = M().UnProject(sightLoc);	// mySight is relative to me

	if(myEye.x < -m_RadiusX && mySight.x < -m_RadiusX)
		return false;
	if(myEye.x > m_RadiusX && mySight.x > m_RadiusX)
		return false;
	if(myEye.y < -m_RadiusY && mySight.y < -m_RadiusY)
		return false;
	if(myEye.y > m_RadiusY && mySight.y > m_RadiusY)
		return false;
	if(myEye.z < 0 && mySight.z < 0)
		return false;
	if(myEye.z > m_RadiusZ && mySight.z > m_RadiusZ)
		return false;

	myEye.y /= m_RadiusY/m_RadiusX;
	myEye.z /= m_RadiusZ/m_RadiusX;
	if(myEye.Length2() < m_RadiusX*m_RadiusX)
	{
		// Eye location is inside me.
		sightLoc = eyeLoc;
		return true;
	}

	mySight.y /= m_RadiusY/m_RadiusX;
	mySight.z /= m_RadiusZ/m_RadiusX;

	// Test against a sphere with my radius.
	//  -1 = no intersect, segment is inside sphere
	//   0 = no intersect, segment is outside sphere
	//   1 = 1 intersection, result in 'hit1'
	//   2 = 2 intersections, results in 'hit1' and 'hit2' where 'hit1' is the closest intersection to 'eyeLoc'.
	AuSolidSphere sphere(AuMatrix4x4(), m_RadiusX);
	AuPoint hit1, hit2;
	int intResult = sphere.Intersect(myEye, mySight, hit1, hit2);
	if(!intResult)
		return false;											// Segment is entirely outside sphere.

	//DHH This is incorrect for a dome but it works for my purposes.
	// Return the first intersection closest to eyeLoc.
	hit1.y *= m_RadiusY/m_RadiusX;
	hit1.z *= m_RadiusZ/m_RadiusX;
	sightLoc = M().Project(hit1);
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidCylindomoid::Clone() const
{
	return new AuSolidCylindomoid(*this);
}

void AuSolidCylindomoid::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_RadiusX, -m_RadiusY, 0);
	max = AuPoint(m_RadiusX, m_RadiusY, m_CylHt + m_RadiusZ);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidCylindomoid::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	if(mp.z <= 0)
		return false;
	mp.y /= m_RadiusY/m_RadiusX;
	mp.z /= m_RadiusZ/m_RadiusX;
	return mp.Length2() < m_RadiusX*m_RadiusX;
}

void AuSolidCylindomoid::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_RadiusX * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;

	if(m_FillBase && m_RadiusX > 0 && m_RadiusY > 0)
	{
		AuScFaceList base;
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_RadiusX,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusX * ca;
			double y = m_RadiusY * sa;
			AuPoint p(x,y);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			base.AddFace(t);
			bp = bn;
		}
		base.Transform(M());
		faces.AddFaces(base);
	}

	AuPointArray dprofile;
	int dsteps = steps/4;
	if(dsteps < 2)
		dsteps = 2;
	dprofile.append(AuPoint(0,-m_RadiusX));
	dprofile.append(AuPoint(m_CylHt,-m_RadiusX));
	for(int d=1; d<=dsteps; d++)
	{
		double angle = PI/2 * d / dsteps;
		AuPoint p = AuPoint(0,-m_RadiusX).RotateZ(angle);
		dprofile.append(AuPoint(p.x + m_CylHt, p.y, p.z));
	}

	AuMatrix4x4 tdome;
	tdome.Scale(1,m_RadiusY/m_RadiusX,m_RadiusZ/m_RadiusX);
	tdome.RotateY(-PI/2);
	tdome.RotateX(PI/2);

	AuScFaceList dome;
	dome.MakeSpindle(dprofile, steps, AuMatrix4x4());
	dome.Transform(tdome);
	dome.Transform(M());
	dome.SmoothVertices();
	faces.AddFaces(dome);
}

void AuSolidCylindomoid::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	AuSolid s;
	AuSolidConoid sides(M(), m_CylHt, m_RadiusX, m_RadiusY, m_RadiusX, m_RadiusY, m_FillBase, false);
	s.Add(sides);
	AuMatrix4x4 mtop(M());
	mtop.Translate(0,0,m_CylHt);
	AuSolidDomoid top(mtop, m_RadiusX, m_RadiusY, m_RadiusZ, false);
	s.Add(top);
	s.SurfacePoints(points, stepSize);
}

bool AuSolidCylindomoid::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	AuSolid s;
	AuSolidConoid sides(M(), m_CylHt, m_RadiusX, m_RadiusY, m_RadiusX, m_RadiusY, m_FillBase, false);
	s.Add(sides);
	AuMatrix4x4 mtop(M());
	mtop.Translate(0,0,m_CylHt);
	AuSolidDomoid top(mtop, m_RadiusX, m_RadiusY, m_RadiusZ, false);
	s.Add(top);
	return s.SightTarget(eyeLoc, sightLoc);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidCone::Clone() const
{
	return new AuSolidCone(*this);
}

void AuSolidCone::Extents(AuPoint& min, AuPoint& max) const
{
	double maxrad;
	if(m_RadiusBase > m_RadiusTop)
		maxrad = m_RadiusBase;
	else
		maxrad = m_RadiusTop;
	min = AuPoint(-maxrad, -maxrad, 0);
	max = AuPoint(maxrad, maxrad, m_Height);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidCone::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	double radAtZ = RadiusAtZ(mp.z);
	if(radAtZ == 0)
		return false;
	return mp.Length2_2D() < radAtZ*radAtZ;
}

double AuSolidCone::RadiusAtZ(double z) const
{
	if(z <= 0)
		return m_RadiusBase;
	if(z >= m_Height)
		return m_RadiusTop;
	return AuNoise::Linear_Interpolate(m_RadiusBase, m_RadiusTop, z / m_Height);
}

void AuSolidCone::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_RadiusBase * 2;		// Circumference at the base
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	int levels = (int)ceil(m_Height / stepSize);
	if(levels < 1)
		levels = 1;

	if(m_FillBase && m_RadiusBase > 0)
	{
		AuScFaceList base;
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_RadiusBase,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			AuPoint p = AuPoint(m_RadiusBase,0,0).RotateZ(ang);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			base.AddFace(t);
			bp = bn;
		}
		base.Transform(M());
		faces.AddFaces(base);
	}

	if(m_FillTop && m_RadiusTop > 0)
	{
		AuScFaceList top;
		AuScPoint tc(0,0,m_Height, 0,0, AuPoint(0,0,1));
		AuScPoint tp(m_RadiusTop,0,m_Height, 0,0, AuPoint(0,0,1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			AuPoint p = AuPoint(m_RadiusTop,0,m_Height).RotateZ(ang);
			AuScPoint tn(p, 0,0, AuPoint(0,0,1));

			AuScFaceTri t(tc, tn, tp);
			top.AddFace(t);
			tp = tn;
		}
		top.Transform(M());
		faces.AddFaces(top);
	}

	// Spin sides in levels.
	AuScFaceList sides;
	double lht = (double)m_Height / levels;	// Height of each level.
	double bht = 0;							// Height at base of current level.
	for(int l=1; l<=levels; l++)
	{
		double tht;							// Height at top of current level.
		if(l == levels)
			tht = m_Height;					// For perfect seam at the top.
		else
			tht = bht + lht;				// Calculate height at top of the current level.

		double brad = RadiusAtZ(bht);
		double trad = RadiusAtZ(tht);
		if(trad == 0)
		{
			// Add triangle cone at this level.
			AuScPoint pb(brad,0,bht, 0,0, AuPoint(1,0,0));
			AuScPoint pt(0,0,tht, 0,0, AuPoint(0,0,1));
			for(int i=1; i<=steps; i++)
			{
				double ang = PI*2 * i / steps;
				AuPoint n = AuPoint(1,0,0).RotateZ(ang);
				AuPoint b = AuPoint(brad,0,bht).RotateZ(ang);

				AuScPoint sb(b, 0,0, n);

				AuScFaceTri t(pb, pt, sb);
				sides.AddFace(t);
				pb = sb;
			}
		}
		else
		{
			// Add quad band at this level.
			AuScPoint pb(brad,0,bht, 0,0, AuPoint(1,0,0));
			AuScPoint pt(trad,0,tht, 0,0, AuPoint(1,0,0));
			for(int i=1; i<=steps; i++)
			{
				double ang = PI*2 * i / steps;
				AuPoint n = AuPoint(1,0,0).RotateZ(ang);
				AuPoint b = AuPoint(brad,0,bht).RotateZ(ang);
				AuPoint t = AuPoint(trad,0,tht).RotateZ(ang);

				AuScPoint sb(b, 0,0, n);
				AuScPoint st(t, 0,0, n);

				AuScFaceQuad q(pb, pt, st, sb);
				sides.AddFace(q);
				pb = sb;
				pt = st;
			}
		}
		bht = tht;
	}
	sides.Transform(M());
	faces.AddFaces(sides);
}

void AuSolidCone::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	AuSolidConoid cone(M(), m_Height, m_RadiusBase, m_RadiusBase, m_RadiusTop, m_RadiusTop, m_FillBase, m_FillTop);
	cone.SurfacePoints(points, stepSize);
}

bool AuSolidCone::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	return false;	//DHH
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidConoid::Clone() const
{
	return new AuSolidConoid(*this);
}

void AuSolidConoid::Extents(AuPoint& min, AuPoint& max) const
{
	double maxradx;
	if(m_RadiusBaseX > m_RadiusTopX)
		maxradx = m_RadiusBaseX;
	else
		maxradx = m_RadiusTopX;

	double maxrady;
	if(m_RadiusBaseY > m_RadiusTopY)
		maxrady = m_RadiusBaseY;
	else
		maxrady = m_RadiusTopY;

	min = AuPoint(-maxradx, -maxrady, 0);
	max = AuPoint(maxradx, maxrady, m_Height);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidConoid::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	double radAtZ = RadiusXAtZ(mp.z);
	if(radAtZ == 0)
		return false;
	double topy = m_RadiusTopY/m_RadiusTopX;
	double bottomy = m_RadiusBaseY/m_RadiusBaseX;
	double zy = AuNoise::Linear_Interpolate(bottomy, topy, mp.z / m_Height);
	mp.y /= zy;
	return mp.Length2_2D() < radAtZ*radAtZ;
}

double AuSolidConoid::RadiusXAtZ(double z) const
{
	if(z <= 0)
		return m_RadiusBaseX;
	if(z >= m_Height)
		return m_RadiusTopX;
	return AuNoise::Linear_Interpolate(m_RadiusBaseX, m_RadiusTopX, z / m_Height);
}

double AuSolidConoid::RadiusYAtZ(double z) const
{
	if(z <= 0)
		return m_RadiusBaseY;
	if(z >= m_Height)
		return m_RadiusTopY;
	return AuNoise::Linear_Interpolate(m_RadiusBaseY, m_RadiusTopY, z / m_Height);
}

void AuSolidConoid::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_RadiusBaseX * 2;		// Circumference at the base
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	int levels = (int)ceil(m_Height / stepSize);
	if(levels < 1)
		levels = 1;

	if(m_FillBase && m_RadiusBaseX > 0 && m_RadiusBaseY > 0)
	{
		AuScFaceList base;
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_RadiusBaseX,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusBaseX * ca;
			double y = m_RadiusBaseY * sa;
			AuPoint p(x,y);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			base.AddFace(t);
			bp = bn;
		}
		base.Transform(M());
		faces.AddFaces(base);
	}

	if(m_FillTop && m_RadiusTopX > 0 && m_RadiusTopY > 0)
	{
		AuScFaceList top;
		AuScPoint tc(0,0,m_Height, 0,0, AuPoint(0,0,1));
		AuScPoint tp(m_RadiusTopX,0,m_Height, 0,0, AuPoint(0,0,1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusTopX * ca;
			double y = m_RadiusTopY * sa;
			AuPoint p(x,y,m_Height);
			AuScPoint tn(p, 0,0, AuPoint(0,0,1));

			AuScFaceTri t(tc, tn, tp);
			top.AddFace(t);
			tp = tn;
		}
		top.Transform(M());
		faces.AddFaces(top);
	}

	// Spin sides in levels.
	AuScFaceList sides;
	double lht = (double)m_Height / levels;	// Height of each level.
	double bht = 0;							// Height at base of current level.
	for(int l=1; l<=levels; l++)
	{
		double tht;							// Height at top of current level.
		if(l == levels)
			tht = m_Height;					// For perfect seam at the top.
		else
			tht = bht + lht;				// Calculate height at top of the current level.

		double bradx = RadiusXAtZ(bht);
		double brady = RadiusYAtZ(bht);
		double tradx = RadiusXAtZ(tht);
		double trady = RadiusYAtZ(tht);
		if(tradx == 0 || trady == 0)
		{
			// Add triangle cone at this level.
			AuScPoint pb(bradx,0,bht, 0,0, AuPoint(1,0,0));
			AuScPoint pt(0,0,tht, 0,0, AuPoint(0,0,1));
			for(int i=1; i<=steps; i++)
			{
				double ang = PI*2 * i / steps;
				double ca = cos(ang);
				double sa = sin(ang);
				double x = bradx * ca;
				double y = brady * sa;
				AuPoint b(x,y,bht);
				AuPoint n(b);
				n.Unit();

				AuScPoint sb(b, 0,0, n);

				AuScFaceTri t(pb, pt, sb);
				sides.AddFace(t);
				pb = sb;
			}
		}
		else
		{
			// Add quad band at this level.
			AuScPoint pb(bradx,0,bht, 0,0, AuPoint(1,0,0));
			AuScPoint pt(tradx,0,tht, 0,0, AuPoint(1,0,0));
			for(int i=1; i<=steps; i++)
			{
				double ang = PI*2 * i / steps;
				double ca = cos(ang);
				double sa = sin(ang);
				double x = bradx * ca;
				double y = brady * sa;
				AuPoint b(x,y,bht);
				x = tradx * ca;
				y = trady * sa;
				AuPoint t(x,y,tht);
				AuPoint nb(b);
				nb.Unit();
				AuPoint nt(t);
				nt.Unit();

				AuScPoint sb(b, 0,0, nb);
				AuScPoint st(t, 0,0, nt);

				AuScFaceQuad q(pb, pt, st, sb);
				sides.AddFace(q);
				pb = sb;
				pt = st;
			}
		}
		bht = tht;
		sides.SmoothVertices();
	}
	sides.Transform(M());
	faces.AddFaces(sides);
}

void AuSolidConoid::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	double circ = PI * m_RadiusBaseX * 2;		// Circumference at the base
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	int levels = (int)ceil(m_Height / stepSize);
	if(levels < 1)
		levels = 1;

	if(m_FillBase && m_RadiusBaseX > 0 && m_RadiusBaseY > 0)
	{
		AuScPoint bc(0,0,0, 0,0, AuPoint(0,0,-1));
		AuScPoint bp(m_RadiusBaseX,0,0, 0,0, AuPoint(0,0,-1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusBaseX * ca;
			double y = m_RadiusBaseY * sa;
			AuPoint p(x,y);
			AuScPoint bn(p, 0,0, AuPoint(0,0,-1));

			AuScFaceTri t(bc, bp, bn);
			AuPointN ptn(t.FaceLocation(), t.FaceNormal());
			ptn.Transform(M());
			points.append(ptn);
			bp = bn;
		}
	}

	if(m_FillTop && m_RadiusTopX > 0 && m_RadiusTopY > 0)
	{
		AuScPoint tc(0,0,m_Height, 0,0, AuPoint(0,0,1));
		AuScPoint tp(m_RadiusTopX,0,m_Height, 0,0, AuPoint(0,0,1));
		for(int i=1; i<=steps; i++)
		{
			double ang = PI*2 * i / steps;
			double ca = cos(ang);
			double sa = sin(ang);
			double x = m_RadiusTopX * ca;
			double y = m_RadiusTopY * sa;
			AuPoint p(x,y,m_Height);
			AuScPoint tn(p, 0,0, AuPoint(0,0,1));

			AuScFaceTri t(tc, tn, tp);
			AuPointN ptn(t.FaceLocation(), t.FaceNormal());
			ptn.Transform(M());
			points.append(ptn);
			tp = tn;
		}
	}

	// Spin sides in levels.
	double lht = (double)m_Height / levels;	// Height of each level.
	double bht = 0;							// Height at base of current level.
	for(int l=1; l<=levels; l++)
	{
		double tht;							// Height at top of current level.
		if(l == levels)
			tht = m_Height;					// For perfect seam at the top.
		else
			tht = bht + lht;				// Calculate height at top of the current level.

		double bradx = RadiusXAtZ(bht);
		double brady = RadiusYAtZ(bht);
		double tradx = RadiusXAtZ(tht);
		double trady = RadiusYAtZ(tht);
		if(tradx == 0 || trady == 0)
		{
			// Add triangle cone at this level.
			AuScPoint pb(bradx,0,bht, 0,0, AuPoint(1,0,0));
			AuScPoint pt(0,0,tht, 0,0, AuPoint(0,0,1));
			for(int i=1; i<=steps; i++)
			{
				double ang = PI*2 * i / steps;
				double ca = cos(ang);
				double sa = sin(ang);
				double x = bradx * ca;
				double y = brady * sa;
				AuPoint b(x,y,bht);
				AuPoint n(b);
				n.Unit();

				AuScPoint sb(b, 0,0, n);

				AuScFaceTri t(pb, pt, sb);
				AuPointN ptn(t.FaceLocation(), t.FaceNormal());
				ptn.Transform(M());
				points.append(ptn);
				pb = sb;
			}
		}
		else
		{
			// Add quad band at this level.
			AuScPoint pb(bradx,0,bht, 0,0, AuPoint(1,0,0));
			AuScPoint pt(tradx,0,tht, 0,0, AuPoint(1,0,0));
			for(int i=1; i<=steps; i++)
			{
				double ang = PI*2 * i / steps;
				double ca = cos(ang);
				double sa = sin(ang);
				double x = bradx * ca;
				double y = brady * sa;
				AuPoint b(x,y,bht);
				x = tradx * ca;
				y = trady * sa;
				AuPoint t(x,y,tht);
				AuPoint nb(b);
				nb.Unit();
				AuPoint nt(t);
				nt.Unit();

				AuScPoint sb(b, 0,0, nb);
				AuScPoint st(t, 0,0, nt);

				AuScFaceQuad q(pb, pt, st, sb);
				AuPointN ptn(q.FaceLocation(), q.FaceNormal());
				ptn.Transform(M());
				points.append(ptn);
				pb = sb;
				pt = st;
			}
		}
		bht = tht;
	}
}

bool AuSolidConoid::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	AuPoint myEye(M().UnProject(eyeLoc));		// in my space
	AuPoint mySight(M().UnProject(sightLoc));	// in my space
	if(myEye.z < 0 && mySight.z < 0)
		return false;
	if(myEye.z > m_Height && mySight.z > m_Height)
		return false;

	double maxRadX;
	if(m_RadiusTopX > m_RadiusBaseX)
		maxRadX = m_RadiusTopX;
	else
		maxRadX = m_RadiusBaseX;
	double maxRadY;
	if(m_RadiusTopY > m_RadiusBaseY)
		maxRadY = m_RadiusTopY;
	else
		maxRadY = m_RadiusBaseY;
	if(myEye.x < -maxRadX && mySight.x < -maxRadX)
		return false;
	if(myEye.x > maxRadX && mySight.x > maxRadX)
		return false;
	if(myEye.y < -maxRadY && mySight.y < -maxRadY)
		return false;
	if(myEye.y > maxRadY && mySight.y > maxRadY)
		return false;

	AuScFaceList faces;
	Faces(faces, m_Height);	// Using my height insures only one band of faces, enough for sighting.
	return faces.Sight(eyeLoc, sightLoc);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolidShape* AuSolidBox::Clone() const
{
	return new AuSolidBox(*this);
}

void AuSolidBox::Extents(AuPoint& min, AuPoint& max) const
{
	min = AuPoint(-m_RadiusX, -m_RadiusY, -m_RadiusZ);
	max = AuPoint(m_RadiusX, m_RadiusY, m_RadiusZ);
	AuBox b(min, max);
	b.Transform(M());
	b.GetMinMax(min, max);
}

bool AuSolidBox::IsInside(const AuPoint& p) const
{
	AuPoint mp = M().UnProject(p);	// mp is in my space
	return fabs(mp.x) < m_RadiusX &&
		fabs(mp.y) < m_RadiusY &&
		fabs(mp.z) < m_RadiusZ;
}

void AuSolidBox::Faces(AuScFaceList& faces, double stepSize) const
{
	double circ = PI * m_RadiusX * 2;		// Circumference
	int steps = (int)ceil(circ / stepSize);
	if(steps < 5)
		steps = 5;
	AuBox box(AuPoint(-m_RadiusX, -m_RadiusY, -m_RadiusZ), AuPoint(m_RadiusX, m_RadiusY, m_RadiusZ));
	AuQuadArray quads;
	box.Transform(M());
	box.GetFaces(faces);
}

bool AuSolidBox::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	AuPoint myEye(M().UnProject(eyeLoc));		// in my space
	AuPoint mySight(M().UnProject(sightLoc));	// in my space
	if(myEye.x < -m_RadiusX && mySight.x < -m_RadiusX)
		return false;
	if(myEye.x > m_RadiusX && mySight.x > m_RadiusX)
		return false;
	if(myEye.y < -m_RadiusY && mySight.y < -m_RadiusY)
		return false;
	if(myEye.y > m_RadiusY && mySight.y > m_RadiusY)
		return false;
	if(myEye.z < -m_RadiusZ && mySight.z < -m_RadiusZ)
		return false;
	if(myEye.z > m_RadiusZ && mySight.z > m_RadiusZ)
		return false;

	AuBox box(AuPoint(-m_RadiusX, -m_RadiusY, -m_RadiusZ), AuPoint(m_RadiusX, m_RadiusY, m_RadiusZ));
	AuPoint int0, int1;
	int isect = box.SegmentIntersection(myEye, mySight, int0, int1);
	if(!isect)
		return false;
	if(isect == 1)
		sightLoc = M().Project(int0);
	else
	{
		if(eyeLoc.Distance2(int0) < eyeLoc.Distance2(int1))
			sightLoc = M().Project(int0);
		else
			sightLoc = M().Project(int1);
	}
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSolid::AuSolid()
{
}

AuSolid::~AuSolid()
{
}

AuSolid::AuSolid(const AuSolid& c) : m_ShapesAdd(c.m_ShapesAdd), m_ShapesSubtract(c.m_ShapesSubtract)
{
}

void AuSolid::TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const
{
	// Clear the incoming topo.
	topo.clear();
	if(m_ShapesAdd.empty())
		return;

	// Get the boundaries of the adds.
	AuBoundaryComplex adds;
	m_ShapesAdd.TopologyAtZ(adds, zHeight, stepSize);
	if(adds.empty())
		return;

	// Get the boundaries of the subtracts.
	AuBoundaryComplex subs;
	m_ShapesSubtract.TopologyAtZ(subs, zHeight, stepSize);

	// Construct the topology.
	topo.Add(adds);
	topo.Subtract(subs);
}

void AuSolid::Faces(AuScFaceList& faces, double stepSize) const
{
	m_ShapesAdd.Faces(faces, stepSize);
	/*
	AuPoint min, max;
	if(!m_ShapesAdd.Extents(min, max))
		return;
	double stepSize = .083/2;	// feet between topo bands
	AuBoundaryComplex topo, prevtopo;
	//int tiers=3;	//DHH
	for(double zt=max.z - stepSize/2; zt >= min.z || !prevtopo.empty(); zt -= stepSize)
	{
		//if(!tiers--)	//DHH
			//break;	//DHH
		TopologyAtZ(topo, zt, stepSize);
		if(topo.empty())
		{
			// No topology at z height. Close off anything we have going.
			//qDebug() << "AuSolid::Faces no topology at z" << zt << "close bottom?" << !prevtopo.empty();
			if(!prevtopo.empty())
				prevtopo.FacesBottom(faces);	// Tessellate previous topo boundaries as downward facing faces.
		}
		else
		{
			// Topo contains data.
			if(prevtopo.empty())
			{
				// No topo above z height. Tessellate topo boundaries as flat surfaces facing upward.
				//qDebug() << "AuSolid::Faces no topology above z" << zt << "close top";
				topo.FacesTop(faces);			// Tessellate current topo boundaries as upward facing faces.
			}
			else
			{
				// Interconnect the topo lines of the previous layer and this layer with triangles.
				//qDebug() << "AuSolid::Faces interconnect faces at z" << zt << "top topo" << prevtopo.size() << "bottom topo"
					//<< topo.size();
				topo.FacesInterconnect(faces, prevtopo, stepSize);
				//topo.FacesAsLines(faces);	//DHH
			}
		}
		prevtopo = topo;
	}

	//faces.FlattenVertexNormals();	//DHH Is this necessary?
	faces.SmoothVertices();
	*/
}

void AuSolid::Add(const AuSolidShape& shape)
{
	m_ShapesAdd.Add(shape);
}

void AuSolid::Subtract(const AuSolidShape& shape)
{
	m_ShapesSubtract.Add(shape);
}

void AuSolid::SurfacePoints(AuPointNArray& points, double stepSize) const
{
	m_ShapesAdd.SurfacePoints(points, stepSize);
}

bool AuSolid::SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	return m_ShapesAdd.SightTarget(eyeLoc, sightLoc);
}

void AuSolid::Transform(const AuMatrix4x4& m)
{
	m_ShapesAdd.Transform(m);
	m_ShapesSubtract.Transform(m);
}

bool AuSolid::Extents(AuPoint& min, AuPoint& max) const
{
	return m_ShapesAdd.Extents(min, max);
}

