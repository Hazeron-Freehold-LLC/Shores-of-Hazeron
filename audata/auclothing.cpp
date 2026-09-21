//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclothing.cpp,v 1.54 2022/05/09 16:13:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclothing.h"
//
#include "audna.h"
#include "auglobell.h"
#include "aunoise.h"

#define d_FacetFace						0						// 1 results in faceted faces, not smoothed.

void AuClothingTextureArray::Create(AuGLEffects& re, const QString& name, const QString& file, const QColor& color)
{
	// Check to see if I already have the name/color combination.
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).Name() == name && (*ci).Color() == color)
			return;		// We already have the texture.
	if(size() >= MAX_CLOTHING_TEXTURES)
	{
		qWarning() << "warning: AuClothingTextureArray::Create MAX_CLOTHING_TEXTURES limit reached for a particular ensemble. "
			"Something won't look right.";
		return;
	}
	append(AuClothingTexture(name, re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, name, file, GL_LINEAR, GL_LINEAR, true), color));
}

int AuClothingTextureArray::IndexOf(const QString& name, const QColor& color) const
{
	// Search for the name/color combination.
	int idx = 0;
	for(const_iterator ci = cbegin(); ci != cend(); ci++, idx++)
		if((*ci).Name() == name && (*ci).Color() == color)
			return idx;
	qWarning() << "warning: AuClothingTextureArray::IndexOf" << name << color << "not found";
	return 0;	// The name/color combination was not found.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuClothingPoint::Wrinkle(float freqx, float freqy, float maxLift, AuClothingPointMap& done)
{
	AuClothingPointMap::const_iterator di = done.FindFuzzy(*this);
	if(di != done.cend())
	{
		AuPoint::operator=(*di);
		return;
	}

	// Perlin noise parameters.
	float persistence = 100;
	unsigned int octaves = 2;
	float noiseRange = AuNoiseF::PerlinNoiseRange(octaves, persistence);
	float noise = AuNoiseF::PerlinNoise(octaves, persistence, m_ClothingTx * freqx, m_ClothingTy * freqy);
	noise /= noiseRange;
	noise = noise * maxLift/2 + maxLift/2;
	AuPoint oloc(*this);
	operator+=(m_N * noise);

	done.insert(oloc, *this);	// Remember my old and new locations.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuClothingPointMap::const_iterator AuClothingPointMap::FindFuzzy(const AuClothingPoint& p) const
{
	const_iterator fi = find(p);
	if(fi != cend())
		return fi;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if(fi.key().FuzzyEquals(p))
			return fi;
	return cend();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

double AuClothingTri::Area() const
{
	const AuClothingPoint& v0 = m_V0;
	const AuClothingPoint& v1 = m_V1;
	const AuClothingPoint& v2 = m_V2;

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
AuPoint AuClothingTri::Center() const
{
	const AuClothingPoint& v0 = m_V0;
	const AuClothingPoint& v1 = m_V1;
	const AuClothingPoint& v2 = m_V2;
	return (v0 + v1 + v2) / 3.;
}

// Returns the 3D extents box of this triangle.
void AuClothingTri::Extents(AuPoint& min, AuPoint& max) const
{
	const AuClothingPoint& v0 = m_V0;
	const AuClothingPoint& v1 = m_V1;
	const AuClothingPoint& v2 = m_V2;

	min = v0;
	max = v0;

	min = min.Min(v1);
	max = max.Max(v1);

	min = min.Min(v2);
	max = max.Max(v2);
}

// Returns the unit vector that is perpendicular to this triangle.
const AuPoint& AuClothingTri::SurfaceNormalCalc() const
{
	if(m_N.IsZero())
	{
		const AuClothingPoint& v0 = m_V0;
		const AuClothingPoint& v1 = m_V1;
		const AuClothingPoint& v2 = m_V2;

		AuPoint vx = v2 - v0;
		AuPoint vy = v1 - v0;
		vx.Unit();
		vy.Unit();
		m_N = vx.Cross(vy);
		m_N.Unit();
	}
	return m_N;
}

void AuClothingTri::Wrinkle(float freqx, float freqy, float maxLift, AuClothingPointMap& done)
{
	m_V0.Wrinkle(freqx, freqy, maxLift, done);
	m_V1.Wrinkle(freqx, freqy, maxLift, done);
	m_V2.Wrinkle(freqx, freqy, maxLift, done);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuClothingMesh::ApplyDecal(CLOTH_DECAL decal, const AuMatrix4x4& m, float decalWidth, float decalHeight, float decalAlpha)
{
	float s, t, u, v;
	AuClothing::DecalTexture(decal, s, t, u, v);

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

void AuClothingMesh::ApplyDecal(const AuClothingDecal& decal)
{
	ApplyDecal(decal.Decal(), decal.M(), decal.Width(), decal.Height(), decal.Alpha()); 
}

void AuClothingMesh::ApplyDecals(const QList<AuClothingDecal>& decals)
{
	for(QList<AuClothingDecal>::const_iterator di = decals.cbegin(); di != decals.cend(); di++)
		ApplyDecal(*di);
}

void AuClothingMesh::Sag(float stiffness)
{
}

void AuClothingMesh::Wrinkle(float freqx, float freqy, float maxLift)
{
	SmoothNormals();
	AuClothingPointMap done;
	for(iterator mi = begin(); mi != end(); mi++)
		(*mi).Wrinkle(freqx, freqy, maxLift, done);
}

void AuClothingMesh::SmoothNormals()
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
			qWarning() << "error: AuClothingMesh::SmoothNormals triangle vertex 1 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V1);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V1))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V1.m_N = *si;
		else
			qWarning() << "error: AuClothingMesh::SmoothNormals triangle vertex 2 not in smooth vertices map";

		si = smoothVerts.find((*ti).m_V2);
		if(si == smoothVerts.cend())
			for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
				if(si.key().FuzzyEquals((*ti).m_V2))
					break;
		if(si != smoothVerts.cend())
			(*ti).m_V2.m_N = *si;
		else
			qWarning() << "error: AuClothingMesh::SmoothNormals triangle vertex 3 not in smooth vertices map";
	}
}

AuGLVAO AuClothingMesh::CreateAnimalClothingVAO(AuOpenGLFunctions& re) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = CountUnblended()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoordClothing = new GLfloat[vertices*3];
	GLfloat* texCoordDecalBlend = new GLfloat[vertices*3];
	GLfloat* dyeColorAlpha = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int dbi = 0;
	unsigned int ci = 0;
	unsigned int dci = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if((*ti).NeedsBlend())
			continue;	// Skip blended triangles.
		const AuClothingPoint& v0 = (*ti).m_V0;
		const AuClothingPoint& v1 = (*ti).m_V1;
		const AuClothingPoint& v2 = (*ti).m_V2;

		// Vertex 0
		positionShine[pidx++] = (float)v0.x;
		positionShine[pidx++] = (float)v0.y;
		positionShine[pidx++] = (float)v0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.alphaF();

		// Vertex 1
		positionShine[pidx++] = (float)v1.x;
		positionShine[pidx++] = (float)v1.y;
		positionShine[pidx++] = (float)v1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.alphaF();

		// Vertex 2
		positionShine[pidx++] = (float)v2.x;
		positionShine[pidx++] = (float)v2.y;
		positionShine[pidx++] = (float)v2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.alphaF();
	}

	// Generate vertex array object with a buffer for each array.
	vao.CreateGL(re, vertices, 7);

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
	re.glBufferData(GL_ARRAY_BUFFER, ci*sizeof(GLfloat), texCoordClothing, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dbi*sizeof(GLfloat), texCoordDecalBlend, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dci*sizeof(GLfloat), dyeColorAlpha, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoordClothing;
	delete [] texCoordDecalBlend;
	delete [] dyeColorAlpha;

	return vao;
}

AuGLVAO AuClothingMesh::CreateAnimalClothingBlendVAO(AuOpenGLFunctions& re) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = CountBlended()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoordClothing = new GLfloat[vertices*3];
	GLfloat* texCoordDecalBlend = new GLfloat[vertices*3];
	GLfloat* dyeColorAlpha = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int dbi = 0;
	unsigned int ci = 0;
	unsigned int dci = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if(!(*ti).NeedsBlend())
			continue;	// Skip unblended triangles.
		const AuClothingPoint& v0 = (*ti).m_V0;
		const AuClothingPoint& v1 = (*ti).m_V1;
		const AuClothingPoint& v2 = (*ti).m_V2;

		// Vertex 0
		positionShine[pidx++] = (float)v0.x;
		positionShine[pidx++] = (float)v0.y;
		positionShine[pidx++] = (float)v0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.alphaF();

		// Vertex 1
		positionShine[pidx++] = (float)v1.x;
		positionShine[pidx++] = (float)v1.y;
		positionShine[pidx++] = (float)v1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.alphaF();

		// Vertex 2
		positionShine[pidx++] = (float)v2.x;
		positionShine[pidx++] = (float)v2.y;
		positionShine[pidx++] = (float)v2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.alphaF();
	}

	// Generate vertex array object with a buffer for each array.
	vao.CreateGL(re, vertices, 7);

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
	re.glBufferData(GL_ARRAY_BUFFER, ci*sizeof(GLfloat), texCoordClothing, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dbi*sizeof(GLfloat), texCoordDecalBlend, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dci*sizeof(GLfloat), dyeColorAlpha, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoordClothing;
	delete [] texCoordDecalBlend;
	delete [] dyeColorAlpha;

	return vao;
}

AuGLVAO AuClothingMesh::CreateHeadClothingVAO(AuOpenGLFunctions& re) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = CountUnblendedHead()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoordClothing = new GLfloat[vertices*3];
	GLfloat* texCoordDecalBlend = new GLfloat[vertices*3];
	GLfloat* dyeColorAlpha = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int dbi = 0;
	unsigned int ci = 0;
	unsigned int dci = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if(!(*ti).IsHead())
			continue;
		if((*ti).NeedsBlend())
			continue;	// Skip blended triangles.
		const AuClothingPoint& v0 = (*ti).m_V0;
		const AuClothingPoint& v1 = (*ti).m_V1;
		const AuClothingPoint& v2 = (*ti).m_V2;

		// Vertex 0
		positionShine[pidx++] = (float)v0.x;
		positionShine[pidx++] = (float)v0.y;
		positionShine[pidx++] = (float)v0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.alphaF();

		// Vertex 1
		positionShine[pidx++] = (float)v1.x;
		positionShine[pidx++] = (float)v1.y;
		positionShine[pidx++] = (float)v1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.alphaF();

		// Vertex 2
		positionShine[pidx++] = (float)v2.x;
		positionShine[pidx++] = (float)v2.y;
		positionShine[pidx++] = (float)v2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.alphaF();
	}

	// Generate vertex array object with a buffer for each array.
	vao.CreateGL(re, vertices, 7);

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
	re.glBufferData(GL_ARRAY_BUFFER, ci*sizeof(GLfloat), texCoordClothing, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dbi*sizeof(GLfloat), texCoordDecalBlend, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dci*sizeof(GLfloat), dyeColorAlpha, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoordClothing;
	delete [] texCoordDecalBlend;
	delete [] dyeColorAlpha;

	return vao;
}

AuGLVAO AuClothingMesh::CreateHeadClothingBlendVAO(AuOpenGLFunctions& re) const
{
	AuGLVAO vao;
	if(isEmpty())
		return vao;

	// Store count of triangles in the vertex array object.
	int vertices = CountBlendedHead()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];
	GLfloat* texCoordClothing = new GLfloat[vertices*3];
	GLfloat* texCoordDecalBlend = new GLfloat[vertices*3];
	GLfloat* dyeColorAlpha = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	unsigned int dbi = 0;
	unsigned int ci = 0;
	unsigned int dci = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if(!(*ti).IsHead())
			continue;
		if(!(*ti).NeedsBlend())
			continue;	// Skip unblended triangles.
		const AuClothingPoint& v0 = (*ti).m_V0;
		const AuClothingPoint& v1 = (*ti).m_V1;
		const AuClothingPoint& v2 = (*ti).m_V2;

		// Vertex 0
		positionShine[pidx++] = (float)v0.x;
		positionShine[pidx++] = (float)v0.y;
		positionShine[pidx++] = (float)v0.z;
		positionShine[pidx++] = (float)(*ti).m_V0.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V0.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V0.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V0.m_DyeColor.alphaF();

		// Vertex 1
		positionShine[pidx++] = (float)v1.x;
		positionShine[pidx++] = (float)v1.y;
		positionShine[pidx++] = (float)v1.z;
		positionShine[pidx++] = (float)(*ti).m_V1.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V1.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V1.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V1.m_DyeColor.alphaF();

		// Vertex 2
		positionShine[pidx++] = (float)v2.x;
		positionShine[pidx++] = (float)v2.y;
		positionShine[pidx++] = (float)v2.z;
		positionShine[pidx++] = (float)(*ti).m_V2.m_Shine;

#if d_FacetFace
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).x;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).y;
		normalReflect[nidx++] = (float)(*ti).SurfaceNormal(*this).z;
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

		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTx;
		texCoordClothing[ci++] = (float)(*ti).m_V2.m_ClothingTy;
		texCoordClothing[ci++] = (float)(*ti).m_ClothingTexIndex;

		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTx;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalTy;
		texCoordDecalBlend[dbi++] = (float)(*ti).m_V2.m_DecalBlend;

		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.redF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.greenF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.blueF();
		dyeColorAlpha[dci++] = (float)(*ti).m_V2.m_DyeColor.alphaF();
	}

	// Generate vertex array object with a buffer for each array.
	vao.CreateGL(re, vertices, 7);

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
	re.glBufferData(GL_ARRAY_BUFFER, ci*sizeof(GLfloat), texCoordClothing, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dbi*sizeof(GLfloat), texCoordDecalBlend, usage);
	re.glVertexAttribPointer(bufi, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer and copy data into buffer.
	bufi++;
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(bufi));
	re.glBufferData(GL_ARRAY_BUFFER, dci*sizeof(GLfloat), dyeColorAlpha, usage);
	re.glVertexAttribPointer(bufi, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] boneIndex;
	delete [] boneWeight;
	delete [] texCoordClothing;
	delete [] texCoordDecalBlend;
	delete [] dyeColorAlpha;

	return vao;
}

void AuClothingMesh::ExpandExtents(const AuClothingTri& t)
{
	m_Min = m_Min.Min(t.m_V0);
	m_Min = m_Min.Min(t.m_V1);
	m_Min = m_Min.Min(t.m_V2);

	m_Max = m_Max.Max(t.m_V0);
	m_Max = m_Max.Max(t.m_V1);
	m_Max = m_Max.Max(t.m_V2);
}

void AuClothingMesh::FromFaces(const AuScFaceList& faces, AuClothingTri::PART part, quint8 texIndex, const QColor& dyeColorAlpha)
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

			AuClothingPoint mp0(v1, v1.m_N, shine, reflect, boneIndex,boneWeight, v1.m_Tx,v1.m_Ty, dyeColorAlpha);
			AuClothingPoint mp1(v2, v2.m_N, shine, reflect, boneIndex,boneWeight, v2.m_Tx,v2.m_Ty, dyeColorAlpha);
			AuClothingPoint mp2(v3, v3.m_N, shine, reflect, boneIndex,boneWeight, v3.m_Tx,v3.m_Ty, dyeColorAlpha);

			AuClothingTri tri(part, texIndex, mp0, mp1, mp2);
			Append(tri);
		}
	}
}

void AuClothingMesh::SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2,
		float boneWeight2, quint8 boneIndex3, float boneWeight3)
{
	for(iterator pi = begin(); pi != end(); pi++)
	{
		(*pi).m_V0.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V1.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
		(*pi).m_V2.SetBones(boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuClothing::GetMesh(AuClothingMesh& clothing, const AuChromatid& dna, const AuMeatMesh& meat,
		const AuClothingTextureArray& ctex, quint32 unique) const
{
	if(m_Hat)
		GetItemMesh(*m_Hat, clothing, dna, meat, ctex, unique);
	if(m_Armor)
	{
		GetItemMesh(*m_Armor, clothing, dna, meat, ctex, unique);
		if(m_Armor->BareBodyCovered())
			return;	// No clothing can be seen under this armor.
	}
	if(m_Clothing)
		GetItemMesh(*m_Clothing, clothing, dna, meat, ctex, unique);
}

void AuClothing::GetItemMesh(const AuItem& item, AuClothingMesh& clothing, const AuChromatid& dna, const AuMeatMesh& meat,
		const AuClothingTextureArray& ctex, quint32 unique) const
{
	float neckAlt = dna.NeckAltitude();
	float shoulderAlt = neckAlt;
	float waistAlt = dna.ClothingWaistAltitude();
	float kneeAlt = dna.KneeAltitude();
	float ankleLen = dna.AnkleLength();
	float calfLen = dna.CalfLength();
	float ankleAlt = kneeAlt - calfLen;

	switch(item.CommodityId())
	{
	default:
		break;
	case CID_Invalid:	// Underwear
	case CID_Underwear:
		{
			// Select a color using the empire of origin value.
			QColor uwColors[] =
			{
				QColor(255, 255, 255),		// White
				QColor(247, 243, 232),		// Pressed Linen
				QColor(232, 228, 201),		// Dirty White
				QColor(221, 209, 185),		// Canvas Cloth
				QColor(194, 181, 165),		// Behr Burlap
				QColor(185, 163, 122),		// Burlap
				QColor(182, 175, 161),		// Camping Tent
				QColor(179, 172, 158),		// Homespun Linen
				QColor(168, 115,  81),		// Leather Goods
				QColor(137, 103,  87),		// Suede Leather
				QColor(134, 115,  84),		// Leather Loafers
				QColor(105, 105, 111),		// Chrysler Charcoal Gray
				QColor( 30,  30,  30),		// Ford Dark Charcoal
			};
			int uwColorCount = sizeof(uwColors) / sizeof(QColor);
			int uwColorIdx = unique % uwColorCount;
			QColor uwColor = uwColors[uwColorIdx];
			int uwTex = ctex.IndexOf("C_WhiteLeather", QColor(255,255,255));
			if(dna.HasBreasts())
			{
				// Halter
				float shirtDist = .25f/12;
				int mamms = dna.MammaryCount();
				float neckAlt = dna.ClothingBustAltitude(AuChromoWhich(0, mamms));
				float hemAlt;
				if(mamms > 2)
					hemAlt = dna.ClothingBustAltitude(AuChromoWhich(mamms - 2, mamms)) - 2.5f/12;	// Avoid the odd one at the end.
				else
					hemAlt = neckAlt - 2.5f/12;
				float cuff = -10000;
				QList<AuClothingDecal> decals;
				GetShirtMesh(clothing, uwTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, uwColor, decals, 3,3,.4f/12, 0);
			}

			// Underpants
			float groinAlt = dna.ClothingGroinAltitude();
			float thighLen = dna.ThighLength();
			float pantsDist = .1f/12;						// Distance between fabric and skin.
			float waistHemAlt = waistAlt;
			float cuffAlt = groinAlt - thighLen/3;
			GetPantsMesh(clothing, uwTex, dna, meat, pantsDist, waistHemAlt, cuffAlt, uwColor, 3,3,.2f/12);
		}
		break;
	case CID_LeatherClothes:
		{
			int leatherTex = ctex.IndexOf("C_BrownLeather", QColor(255,255,255));

			// Shirt
			float shirtDist = .4f/12;
			neckAlt += 10000;		// Tight to the neck.
			float cuff = .8f;
			float hemAlt = waistAlt - 4.f/12;
			QColor shirtColor(255, 255, 255);					// White
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, leatherTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 3,3,.3f/12, 0);

			// Pants
			float pantsDist = .1f/12;						// Distance between fabric and skin.
			float cuffAlt = ankleAlt + calfLen * .5f;
			QColor pantsColor = QColor(210, 200, 200);
			float waistHemAlt = hemAlt + 1.f/12;
			GetPantsMesh(clothing, leatherTex, dna, meat, pantsDist, waistHemAlt, cuffAlt, pantsColor, 2,2,.2f/12);

			// Boots
			float shoeDist = .5f/12;
			float shoeAlt = ankleAlt + calfLen * .6f;
			QColor shoeColor(202, 150, 78);
			GetShoesMesh(clothing, leatherTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		}
		break;
	case CID_JeansAndShirt:
		{
			// Shirt
			int shirtTex = ctex.IndexOf("C_Canvas", QColor(255,255,255));
			float shirtDist = .4f/12;
			neckAlt += 10000;		// Tight to the neck.
			float cuff = .5f;
			float hemAlt = waistAlt - 5.f/12;
			//QColor shirtColor(165, 125, 210);				// Passenger Icon Purple
			QColor shirtColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, shirtTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 4,4,.5f/12, 0);

			// Pants
			int pantsTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float pantsDist = .1f/12;						// Distance between fabric and skin.
			float cuffAlt = ankleAlt - ankleLen/2;
			float waistHemAlt = hemAlt + 1.f/12;
			QColor pantsColor = QColor(21, 96, 189);		// Blue denim
			GetPantsMesh(clothing, pantsTex, dna, meat, pantsDist, waistHemAlt, cuffAlt, pantsColor, 3,3,.2f/12);

			// Shoes
			int shoeTex = ctex.IndexOf("C_Canvas", QColor(255,255,255));
			float shoeDist = .4f/12;
			float shoeAlt = ankleAlt;
			QColor shoeColor(96, 96, 96);					// Blackish
			GetShoesMesh(clothing, shoeTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		}
		break;
	case CID_CapriHalterOutfit:
		{
			// Halter
			int shirtTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float shirtDist = .25f/12;
			int mamms = dna.MammaryCount();
			float neckAlt = dna.ClothingBustAltitude(AuChromoWhich(0, mamms));
			float hemAlt;
			if(mamms > 2)
				hemAlt = dna.ClothingBustAltitude(AuChromoWhich(mamms - 2, mamms)) - 4.f/12;	// Avoid the odd one at the end.
			else
				hemAlt = neckAlt - 4.f/12;

			float cuff = .2f;
			QColor shirtColor(166, 255, 220);
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, shirtTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 3,3,.4f/12, 0);

			// Capris pants
			int pantsTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float pantsDist = .15f/12;
			float cuffAlt = ankleAlt + calfLen / 2;
			QColor pantsColor(255, 166, 201);	// Carnation pink
			GetPantsMesh(clothing, pantsTex, dna, meat, pantsDist, waistAlt, cuffAlt, pantsColor, 0,0,0);

			// Shoes
			int shoeTex = ctex.IndexOf("C_Canvas", QColor(255,255,255));
			float shoeDist = .25f/12;
			float shoeAlt = ankleAlt - ankleLen * .4f;
			QColor shoeColor(255, 255, 255);
			GetShoesMesh(clothing, shoeTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		}
		break;
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
		{
			int camoTex = ctex.IndexOf("C_DigitalCamo", QColor(255,255,255));
			int leatherTex = ctex.IndexOf("C_WhiteLeather", QColor(255,255,255));
			QColor fabricColor;
			switch(item.CommodityId())
			{
			default:
			case CID_GreyBDU:
				fabricColor = QColor(255, 255, 255);
				break;
			case CID_KhakiBDU:
				fabricColor = QColor(195, 176, 145);			// Khaki
				break;
			case CID_OliveDrabBDU:
				fabricColor = QColor(107, 142, 35);
				break;
			}

			// Shirt
			float shirtDist = .35f/12;
			neckAlt += 10000;		// Tight to the neck.
			float cuff = 1.f;
			float hemAlt = waistAlt - 4.f/12;
			QList<AuClothingDecal> decals;
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 5.f/12;						// Extra size accounts for decal margin.
			const float decalAlpha = .45f;						// Subdued
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			GetShirtMesh(clothing, camoTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, fabricColor, decals, 4,4,.4f/12, 0);

			// Pants
			float pantsDist = .25f/12;							// Distance between fabric and skin.
			float cuffAlt = ankleAlt + calfLen * .4f;
			float waistHemAlt = hemAlt + 1.f/12;
			GetPantsMesh(clothing, camoTex, dna, meat, pantsDist, waistHemAlt, cuffAlt, fabricColor, 3,3,.5f/12);

			// Boots
			float shoeDist = .1f/12;
			float shoeAlt = ankleAlt + calfLen * .5f;
			QColor shoeColor(32, 32, 32);
			GetShoesMesh(clothing, leatherTex, dna, meat, shoeDist, shoeAlt, shoeColor);

			// Gloves
			float gloveDist = .25f/12;
			float gloveCuff = 1;
			GetGlovesMesh(clothing, leatherTex, dna, meat, gloveDist, gloveCuff, shoeColor);
		}
		break;
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
		{
			int clothTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			int leatherTex = ctex.IndexOf("C_WhiteLeather", QColor(255,255,255));

			QColor shirtColor;
			switch(item.CommodityId())
			{
			default:
				break;
			case CID_BlackUniform:
				shirtColor = QColor(32,32,32);
				break;
			case CID_BlueUniform:
				shirtColor = QColor(8,153,246);		// Star Trek
				break;
			case CID_BrownUniform:
				shirtColor = QColor(100, 65, 23);	// Pullman Brown
				break;
			case CID_GoldUniform:
				shirtColor = QColor(242,195,8);		// Star Trek
				break;
			case CID_GreenUniform:
				shirtColor = QColor(8,184,68);		// Star Trek
				break;
			case CID_GreyUniform:
				shirtColor = QColor(192,192,192);
				break;
			case CID_OrangeUniform:
				shirtColor = QColor(255,122,3);
				break;
			case CID_PinkUniform:
				shirtColor = QColor(240,16,240);
				break;
			case CID_RedUniform:
				shirtColor = QColor(223,8,8);		// Star Trek
				break;
			case CID_TealUniform:
				shirtColor = QColor(16,240,240);
				break;
			case CID_WhiteUniform:
				shirtColor = QColor(255,255,255);
				break;
			}

			// Shirt
			float shirtDist = .45f/12;
			neckAlt += 10000;		// Tight to the neck.
			float cuff = 1.f;
			float hemAlt = waistAlt - 5.f/12;
			QList<AuClothingDecal> decals;
			const float decalHt = 5.f/12;				// Extra size accounts for decal margin.
			AuMatrix4x4 decalM;
			decalM.Translate(-.25, 0, shoulderAlt + decalHt/4);
			decalM.RotateX(PI/2);
			decalM.RotateY(PI);
			const float decalWid = decalHt;
			const float decalAlpha = 1;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalWid, decalHt, decalAlpha));
			GetShirtMesh(clothing, clothTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 4,4,.25f/12, 0);

			// Pants
			float pantsDist = .15f/12;						// Distance between fabric and skin.
			float cuffAlt = ankleAlt - ankleLen * .5f;
			float waistHemAlt = hemAlt + 1.f/12;
			QColor pantsColor(20, 20, 20);
			GetPantsMesh(clothing, clothTex, dna, meat, pantsDist, waistHemAlt, cuffAlt, pantsColor, 3,3,.2f/12);

			// Shoes
			float shoeDist = .05f/12;
			float shoeAlt = ankleAlt;
			QColor shoeColor(10, 10, 10);
			GetShoesMesh(clothing, leatherTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		}
		break;
	case CID_ParcXUniform:
		{
			int clothTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			int leatherTex = ctex.IndexOf("C_WhiteLeather", QColor(255,255,255));

			// Shirt
			float shirtDist = .45f/12;
			neckAlt += 10000;		// Tight to the neck.
			float cuff = 1.f;
			float hemAlt = waistAlt - 6.f/12;
			QColor shirtColor(100, 65, 23);					// Pullman Brown
			QList<AuClothingDecal> decals;
			const float decalHt = 4.f/12;				// Extra size accounts for decal margin.
			AuMatrix4x4 decalM;
			decalM.Translate(-.25, 0, shoulderAlt + decalHt/4);
			decalM.RotateX(PI/2);
			decalM.RotateY(PI);
			const float decalWid = 8.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .8f;
			decals.append(AuClothingDecal(CD_ParcXLogo, decalM, decalWid, decalHt, decalAlpha));
			decalM.LoadIdentity();
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 5.f/12;				// Extra size accounts for decal margin.
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			GetShirtMesh(clothing, clothTex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 4,4,.5f/12, 0);

			// Pants
			float pantsDist = .15f/12;						// Distance between fabric and skin.
			float cuffAlt = ankleAlt - ankleLen * .5f;
			float waistHemAlt = hemAlt + 1.f/12;
			QColor pantsColor(100, 65, 23);					// Pullman Brown
			GetPantsMesh(clothing, clothTex, dna, meat, pantsDist, waistHemAlt, cuffAlt, pantsColor, 3,3,.2f/12);

			// Shoes
			float shoeDist = .05f/12;
			float shoeAlt = ankleAlt;
			QColor shoeColor(16, 16, 16);
			GetShoesMesh(clothing, leatherTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		}
		break;
	case CID_LeatherBodyArmor:
		{
			int tex = ctex.IndexOf("C_BrownLeather", QColor(255,255,255));
			float torsoThickness = dna.TorsoBottomThickness();

			// Vest
			float shirtDist = 1.1f/12;
			neckAlt += torsoThickness/2;
			float cuff = .25f;
			float hemAlt = waistAlt - torsoThickness;
			float thick = .4f/12;
			QColor shirtColor(128, 128, 128);
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, tex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 0,0,0, thick);
		}
		break;
	case CID_LeatherHelmet:
		{
			int helmTex = ctex.IndexOf("C_BrownLeather", QColor(255,255,255));
			QColor helmColor(128, 128, 128);
			QList<AuClothingDecal> decals;
			GetHelmetMesh(clothing, helmTex, dna, meat, helmColor, decals, item.CommodityId());
		}
		break;
	case CID_WovenBodyArmor:
		{
			int tex = ctex.IndexOf("C_DigitalCamo", QColor(255,255,255));
			float torsoThickness = dna.TorsoBottomThickness();

			// Vest
			float shirtDist = 1.5f/12;
			neckAlt += torsoThickness * .9;
			float cuff = .1f;
			float hemAlt = waistAlt - torsoThickness;
			float thick = 1.25f/12;
			//QColor shirtColor(195, 176, 145);				// Khaki
			QColor shirtColor(210, 180, 140);				// Tan
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, tex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 0,0,0, thick);
		}
		break;
	case CID_WovenHelmet:
		{
			int helmTex = ctex.IndexOf("C_DigitalCamo", QColor(255,255,255));
			QColor helmColor(210, 180, 140);				// Tan
			QList<AuClothingDecal> decals;
			GetHelmetMesh(clothing, helmTex, dna, meat, helmColor, decals, item.CommodityId());
		}
		break;
	case CID_PlasticBodyArmor:
		{
			int tex = ctex.IndexOf("C_PlasticBodyArmor", QColor(255,255,255));
			float torsoThickness = dna.TorsoBottomThickness();

			// Vest
			float shirtDist = 1.2f/12;
			neckAlt += torsoThickness * .8;
			float cuff = .9f;
			float hemAlt = waistAlt - torsoThickness * .9;
			float thick = .25f/12;
			QColor shirtColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, tex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 0,0,0, thick);
		}
		break;
	case CID_PlasticHelmet:
		{
			int helmTex = ctex.IndexOf("C_PlasticBodyArmor", QColor(255,255,255));
			QColor helmColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			GetHelmetMesh(clothing, helmTex, dna, meat, helmColor, decals, item.CommodityId());
		}
		break;
	case CID_MetalBodyArmor:
		{
			int tex = ctex.IndexOf("C_MetalBodyArmor", QColor(255,255,255));
			float torsoThickness = dna.TorsoBottomThickness();

			// Vest
			float shirtDist = 1.1f/12;
			neckAlt += torsoThickness/2;
			float cuff = .15f;
			float hemAlt = waistAlt - torsoThickness * .8;
			float thick = .3f/12;
			QColor shirtColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			GetShirtMesh(clothing, tex, dna, meat, shirtDist, neckAlt, hemAlt, cuff, shirtColor, decals, 0,0,0, thick);
		}
		break;
	case CID_MetalHelmet:
		{
			int helmTex = ctex.IndexOf("C_MetalPlateSquares", QColor(255,255,255));
			QColor helmColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			GetHelmetMesh(clothing, helmTex, dna, meat, helmColor, decals, item.CommodityId());
		}
		break;
	case CID_VacSuit:
		{
			// Suit
			int shirtTex = ctex.IndexOf("C_CrinklyWhiteFabric", QColor(255,255,255));
			float shirtDist = 1.f/12;
			float wrist = 1;
			float cuffAlt = ankleAlt + calfLen * .3f;
			QColor shirtColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .8f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			float maxLift = .8f/12;
			GetVacSuitMesh(clothing, shirtTex, shirtTex, shirtTex, dna, meat, shirtDist, wrist, cuffAlt, shirtColor, decals,
					maxLift);

			// Belt
			int beltTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float beltDist = 2.f/12;
			float beltTop = waistAlt + 1.f/12;
			float beltBottom = waistAlt - 1.f/12;
			float beltThick = 1.f/12;
			QColor beltColor(36, 36, 36);
			GetChestMesh(clothing, AuClothingTri::Suit, beltTex, dna, meat, beltDist, beltTop, beltBottom, beltColor, beltThick);

			// Collar
			int collarTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float collarDist = .5f/12;
			QColor collarColor(36, 36, 36);
			GetNeckMesh(clothing, AuClothingTri::Suit, collarTex, dna, meat, collarDist, 1, collarColor);

			// Gloves
			int gloveTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float gloveDist = .5f/12;
			float gloveCuff = .95f;
			QColor gloveColor(180, 151, 21);
			GetGlovesMesh(clothing, gloveTex, dna, meat, gloveDist, gloveCuff, gloveColor);

			// Boots
			int shoeTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float shoeDist = 1.f/12;
			float shoeAlt = ankleAlt + calfLen * .4f;
			QColor shoeColor(32, 32, 32);
			GetShoesMesh(clothing, shoeTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		};
		break;
	case CID_VacSuitHelmet:
		{
			// Suit
			int helmTex = ctex.IndexOf("C_CrinklyWhiteFabric", QColor(255,255,255));
			int phoneTex = ctex.IndexOf("C_MetalGrillHoles", QColor(255,255,255));
			int visorTex = ctex.IndexOf("C_Obsidian", QColor(255,255,255));
			QColor helmColor(255, 255, 255);
			QColor visorColor(128, 128, 128, 128);
			QList<AuClothingDecal> decals;
			/*
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .8f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			*/
			GetVacSuitHelmetMesh(clothing, helmTex, phoneTex, dna, meat, helmColor, visorTex, visorColor, decals);
		}
		break;
	case CID_MetalPowerArmor:
		{
			// Suit
			int shirtTex = ctex.IndexOf("C_MetalBodyArmor", QColor(255,255,255));
			int limbTex = ctex.IndexOf("C_MetalPlateSquares", QColor(255,255,255));
			float shirtDist = 1.f/12;
			float wrist = 1;
			float cuffAlt = ankleAlt + calfLen * .6f;
			QColor shirtColor(255, 255, 255);
			QList<AuClothingDecal> decals;
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .5f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			float maxLift = 0;
			GetVacSuitMesh(clothing, shirtTex, limbTex, limbTex, dna, meat, shirtDist, wrist, cuffAlt, shirtColor, decals,
					maxLift);

			// Belt
			int beltTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float beltDist = 1.5f/12;
			float beltTop = waistAlt + 1.f/12;
			float beltBottom = waistAlt - 1.f/12;
			float beltThick = 1.f/12;
			QColor beltColor(36, 36, 36);
			GetChestMesh(clothing, AuClothingTri::Suit, beltTex, dna, meat, beltDist, beltTop, beltBottom, beltColor, beltThick);

			// Collar
			int collarTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float collarDist = .5f/12;
			QColor collarColor(48, 48, 48);
			GetNeckMesh(clothing, AuClothingTri::Suit, collarTex, dna, meat, collarDist, 1, collarColor);

			// Gloves
			int gloveTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float gloveDist = .5f/12;
			float gloveCuff = .95f;
			QColor gloveColor(36, 36, 36);
			GetGlovesMesh(clothing, gloveTex, dna, meat, gloveDist, gloveCuff, gloveColor);

			// Boots
			int shoeTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float shoeDist = .85f/12;
			float shoeAlt = ankleAlt + calfLen * .7f;
			QColor shoeColor(32, 32, 32);
			GetShoesMesh(clothing, shoeTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		};
		break;
	case CID_MetalPowerArmorHelmet:
		{
			// Suit
			int helmTex = ctex.IndexOf("C_MetalPlateSquares", QColor(255,255,255));
			int visorTex = ctex.IndexOf("C_Obsidian", QColor(255,255,255));
			int phoneTex = ctex.IndexOf("C_VentBlack", QColor(255,255,255));
			QColor helmColor(255, 255, 255);
			QColor visorColor(128, 128, 128, 128);
			QList<AuClothingDecal> decals;
			/*
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .8f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			*/
			GetPowerArmorHelmetMesh(clothing, helmTex, phoneTex, dna, meat, helmColor, visorTex, visorColor, decals,
					item.CommodityId());
		}
		break;
	case CID_MagmiumPowerArmor:
		{
			// Suit
			int shirtTex = ctex.IndexOf("C_MagmiumPlateSquares", QColor(255,255,255));
			int limbTex = shirtTex;
			float shirtDist = .9f/12;
			float wrist = 1;
			float cuffAlt = ankleAlt + calfLen * .6f;
			QColor shirtColor(192, 192, 192);
			QList<AuClothingDecal> decals;
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .5f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			float maxLift = 0;
			GetVacSuitMesh(clothing, shirtTex, limbTex, limbTex, dna, meat, shirtDist, wrist, cuffAlt, shirtColor, decals,
					maxLift);

			// Belt
			int beltTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float beltDist = 1.4f/12;
			float beltTop = waistAlt + 1.f/12;
			float beltBottom = waistAlt - 1.f/12;
			float beltThick = 1.f/12;
			QColor beltColor(36, 36, 36);
			GetChestMesh(clothing, AuClothingTri::Suit, beltTex, dna, meat, beltDist, beltTop, beltBottom, beltColor, beltThick);

			// Collar
			int collarTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float collarDist = .4f/12;
			QColor collarColor(64, 48, 32);
			GetNeckMesh(clothing, AuClothingTri::Suit, collarTex, dna, meat, collarDist, 1, collarColor);

			// Gloves
			int gloveTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float gloveDist = .35f/12;
			float gloveCuff = .95f;
			QColor gloveColor(36, 36, 36);
			GetGlovesMesh(clothing, gloveTex, dna, meat, gloveDist, gloveCuff, gloveColor);

			// Boots
			int shoeTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float shoeDist = .8f/12;
			float shoeAlt = ankleAlt + calfLen * .7f;
			QColor shoeColor(32, 32, 32);
			GetShoesMesh(clothing, shoeTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		};
		break;
	case CID_MagmiumPowerArmorHelmet:
		{
			// Suit
			int helmTex = ctex.IndexOf("C_MagmiumPlateSquares", QColor(255,255,255));
			int visorTex = ctex.IndexOf("C_Obsidian", QColor(255,255,255));
			int phoneTex = ctex.IndexOf("C_VentBlack", QColor(255,255,255));
			QColor helmColor(192, 192, 192);
			QColor visorColor(128, 128, 128, 128);
			QList<AuClothingDecal> decals;
			/*
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .8f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			*/
			GetPowerArmorHelmetMesh(clothing, helmTex, phoneTex, dna, meat, helmColor, visorTex, visorColor, decals,
					item.CommodityId());
		}
		break;
	case CID_VulciumPowerArmor:
		{
			// Suit
			int shirtTex = ctex.IndexOf("C_VulciumPlateSquares", QColor(255,255,255));
			int limbTex = shirtTex;
			float shirtDist = .6f/12;
			float wrist = 1;
			float cuffAlt = ankleAlt + calfLen * .6f;
			QColor shirtColor(128, 128, 128);
			QList<AuClothingDecal> decals;
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .5f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			float maxLift = 0;
			GetVacSuitMesh(clothing, shirtTex, limbTex, limbTex, dna, meat, shirtDist, wrist, cuffAlt, shirtColor, decals,
					maxLift);

			// Belt
			int beltTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float beltDist = 1.1f/12;
			float beltTop = waistAlt + 1.f/12;
			float beltBottom = waistAlt - 1.f/12;
			float beltThick = 1.f/12;
			QColor beltColor(36, 36, 36);
			GetChestMesh(clothing, AuClothingTri::Suit, beltTex, dna, meat, beltDist, beltTop, beltBottom, beltColor, beltThick);

			// Collar
			int collarTex = ctex.IndexOf("C_Pleat", QColor(255,255,255));
			float collarDist = .3f/12;
			QColor collarColor(32, 64, 32);
			GetNeckMesh(clothing, AuClothingTri::Suit, collarTex, dna, meat, collarDist, 1, collarColor);

			// Gloves
			int gloveTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float gloveDist = .25f/12;
			float gloveCuff = .95f;
			QColor gloveColor(36, 36, 36);
			GetGlovesMesh(clothing, gloveTex, dna, meat, gloveDist, gloveCuff, gloveColor);

			// Boots
			int shoeTex = ctex.IndexOf("C_HomespunWhite", QColor(255,255,255));
			float shoeDist = .4f/12;
			float shoeAlt = ankleAlt + calfLen * .7f;
			QColor shoeColor(32, 32, 32);
			GetShoesMesh(clothing, shoeTex, dna, meat, shoeDist, shoeAlt, shoeColor);
		};
		break;
	case CID_VulciumPowerArmorHelmet:
		{
			// Suit
			int helmTex = ctex.IndexOf("C_VulciumPlateSquares", QColor(255,255,255));
			int visorTex = ctex.IndexOf("C_Obsidian", QColor(255,255,255));
			int phoneTex = ctex.IndexOf("C_VentBlack", QColor(255,255,255));
			QColor helmColor(128, 128, 128);
			QColor visorColor(128, 128, 128, 128);
			QList<AuClothingDecal> decals;
			/*
			AuMatrix4x4 decalM;
			decalM.Translate(0,0, shoulderAlt);
			decalM.RotateZ(PI/2);
			decalM.RotateX(PI/2);
			const float decalRad = 6.f/12;				// Extra size accounts for decal margin.
			const float decalAlpha = .8f;
			decals.append(AuClothingDecal(CD_SoHLogo, decalM, decalRad, decalRad, decalAlpha));
			*/
			GetPowerArmorHelmetMesh(clothing, helmTex, phoneTex, dna, meat, helmColor, visorTex, visorColor, decals,
					item.CommodityId());
		}
		break;
	}
}

void AuClothing::CreateTextures(AuGLEffects& re, AuClothingTextureArray& ctex) const
{
	if(m_Hat)
		CreateItemTextures(*m_Hat, re, ctex);
	if(m_Armor)
		CreateItemTextures(*m_Armor, re, ctex);
	if(m_Clothing)
		CreateItemTextures(*m_Clothing, re, ctex);
}

void AuClothing::CreateItemTextures(const AuItem& item, AuGLEffects& re, AuClothingTextureArray& ctex) const
{
	switch(item.CommodityId())
	{
	default:
		break;
	case CID_Invalid:	// Underwear
	case CID_Underwear:
		ctex.Create(re, "C_WhiteLeather", ":data/WhiteLeather.png", QColor(255,255,255));
		break;
	case CID_JeansAndShirt:
		ctex.Create(re, "C_Canvas", ":data/Canvas.png", QColor(255,255,255));
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		break;
	case CID_LeatherBodyArmor:
	case CID_LeatherClothes:
	case CID_LeatherHelmet:
		ctex.Create(re, "C_BrownLeather", ":data/BrownLeather.png", QColor(255,255,255));
		break;
	case CID_CapriHalterOutfit:
		ctex.Create(re, "C_Canvas", ":data/Canvas.png", QColor(255,255,255));
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		ctex.Create(re, "C_Pleat", ":data/Pleat.png", QColor(255,255,255));
		break;
	case CID_ParcXUniform:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		ctex.Create(re, "C_WhiteLeather", ":data/WhiteLeather.png", QColor(255,255,255));
		break;
	case CID_MetalBodyArmor:
		ctex.Create(re, "C_MetalBodyArmor", ":data/MetalBodyArmor.png", QColor(255,255,255));
		break;
	case CID_MetalHelmet:
		ctex.Create(re, "C_MetalPlateSquares", ":data/MetalPlateSquares.png", QColor(255,255,255));
		break;
	case CID_PlasticBodyArmor:
		ctex.Create(re, "C_PlasticBodyArmor", ":data/PlasticBodyArmor.png", QColor(255,255,255));
		break;
	case CID_PlasticHelmet:
		ctex.Create(re, "C_PlasticBodyArmor", ":data/PlasticBodyArmor.png", QColor(255,255,255));
		break;
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
		ctex.Create(re, "C_DigitalCamo", ":data/DigitalCamo.png", QColor(255,255,255));
		break;
	case CID_VacSuit:
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		ctex.Create(re, "C_CrinklyWhiteFabric", ":data/CrinklyWhiteFabric.png", QColor(255,255,255));
		ctex.Create(re, "C_Pleat", ":data/Pleat.png", QColor(255,255,255));
		break;
	case CID_VacSuitHelmet:
		ctex.Create(re, "C_CrinklyWhiteFabric", ":data/CrinklyWhiteFabric.png", QColor(255,255,255));
		ctex.Create(re, "C_MetalGrillHoles", ":data/MetalGrillHoles.png", QColor(255,255,255));
		ctex.Create(re, "C_Obsidian", ":data/Obsidian.png", QColor(255,255,255));
		break;
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
		ctex.Create(re, "C_DigitalCamo", ":data/DigitalCamo.png", QColor(255,255,255));
		ctex.Create(re, "C_WhiteLeather", ":data/WhiteLeather.png", QColor(255,255,255));
		break;
	case CID_MetalPowerArmor:
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		ctex.Create(re, "C_MetalBodyArmor", ":data/MetalBodyArmor.png", QColor(255,255,255));
		ctex.Create(re, "C_MetalPlateSquares", ":data/MetalPlateSquares.png", QColor(255,255,255));
		ctex.Create(re, "C_Pleat", ":data/Pleat.png", QColor(255,255,255));
		break;
	case CID_MetalPowerArmorHelmet:
		ctex.Create(re, "C_MetalPlateSquares", ":data/MetalPlateSquares.png", QColor(255,255,255));
		ctex.Create(re, "C_Obsidian", ":data/Obsidian.png", QColor(255,255,255));
		ctex.Create(re, "C_VentBlack", ":data/VentBlack.png", QColor(255,255,255));
		break;
	case CID_MagmiumPowerArmor:
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		ctex.Create(re, "C_MagmiumPlateSquares", ":data/MagmiumPlateSquares.png", QColor(255,255,255));
		ctex.Create(re, "C_Pleat", ":data/Pleat.png", QColor(255,255,255));
		break;
	case CID_MagmiumPowerArmorHelmet:
		ctex.Create(re, "C_MagmiumPlateSquares", ":data/MagmiumPlateSquares.png", QColor(255,255,255));
		ctex.Create(re, "C_Obsidian", ":data/Obsidian.png", QColor(255,255,255));
		ctex.Create(re, "C_VentBlack", ":data/VentBlack.png", QColor(255,255,255));
		break;
	case CID_VulciumPowerArmor:
		ctex.Create(re, "C_HomespunWhite", ":data/HomespunWhite.png", QColor(255,255,255));
		ctex.Create(re, "C_VulciumPlateSquares", ":data/VulciumPlateSquares.png", QColor(255,255,255));
		ctex.Create(re, "C_Pleat", ":data/Pleat.png", QColor(255,255,255));
		break;
	case CID_VulciumPowerArmorHelmet:
		ctex.Create(re, "C_VulciumPlateSquares", ":data/VulciumPlateSquares.png", QColor(255,255,255));
		ctex.Create(re, "C_Obsidian", ":data/Obsidian.png", QColor(255,255,255));
		ctex.Create(re, "C_VentBlack", ":data/VentBlack.png", QColor(255,255,255));
		break;
	}
}

void AuClothing::GetPantsMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna, const AuMeatMesh& meat,
		float skinDistance, float waistAlt, float cuffAlt, const QColor& dye, float noiseFreqX, float noiseFreqY,
		float noiseMaxLift) const
{
	AuClothingMesh pants;
	GetGroinMesh(pants, AuClothingTri::Pants, texIndex, dna, meat, skinDistance, waistAlt, dye);
	GetLegsMesh(pants, AuClothingTri::Pants, texIndex, dna, meat, skinDistance, waistAlt, cuffAlt, dye, 0);
	GetTailMesh(pants, AuClothingTri::Pants, texIndex, dna, meat, skinDistance, dye);

	pants.Wrinkle(noiseFreqX, noiseFreqY, noiseMaxLift);

	pants.SmoothNormals();

	clothing.Append(pants);
}

void AuClothing::GetShirtMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float neckAlt, float waistAlt, float cuff, const QColor& dye,
		const QList<AuClothingDecal>& decals, float noiseFreqX, float noiseFreqY, float noiseMaxLift, float thickness) const
{
	AuClothingMesh shirt;
	GetChestMesh(shirt, AuClothingTri::Shirt, texIndex, dna, meat, skinDistance, neckAlt, waistAlt, dye, thickness);
	if(cuff > -10000)
	{
		GetLegsMesh(shirt, AuClothingTri::Shirt, texIndex, dna, meat, skinDistance, waistAlt + 10000, waistAlt, dye, thickness);
		GetArmsMesh(shirt, AuClothingTri::Shirt, texIndex, dna, meat, skinDistance, cuff, dye, thickness);
	}

	shirt.ApplyDecals(decals);

	shirt.Wrinkle(noiseFreqX, noiseFreqY, noiseMaxLift);

	float stiffness = 1;
	shirt.Sag(stiffness);

	shirt.SmoothNormals();

	clothing.Append(shirt);
}

void AuClothing::GetVacSuitMesh(AuClothingMesh& clothing, int torsoTex, int armTex, int legTex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float wrist, float ankleAlt, const QColor& dye,
		const QList<AuClothingDecal>& decals, float maxLift) const
{
	float thickness = 2.f/12;
	AuClothingMesh suit;
	GetChestMesh(suit, AuClothingTri::Suit, torsoTex, dna, meat, skinDistance, +10000, -10000, dye, thickness);	// All torso
	GetLegsMesh(suit, AuClothingTri::Suit, legTex, dna, meat, skinDistance, +10000, ankleAlt, dye, 0);	// Legs to ankles.
	GetArmsMesh(suit, AuClothingTri::Suit, armTex, dna, meat, skinDistance, wrist, dye, 0);			// Arms to wrists
	GetTailMesh(suit, AuClothingTri::Suit, legTex, dna, meat, skinDistance, dye);

	suit.ApplyDecals(decals);

	float freqx = 6;
	float freqy = 6;
	suit.Wrinkle(freqx, freqy, maxLift);

	suit.SmoothNormals();

	clothing.Append(suit);
}

void AuClothing::GetVacSuitHelmetMesh(AuClothingMesh& clothing, int surfaceTex, int phonesTex, const AuChromatid& dna,
		const AuMeatMesh& meat, const QColor& dye, int visorTex, const QColor& visorColor,
		const QList<AuClothingDecal>& decals) const
{
	Q_UNUSED(meat);
	const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromoHead::ChromosomeType()));
	if(!head)
		return;	// wut?
	const AuChromoEar* ear = dynamic_cast<const AuChromoEar*>(dna.Chromosome(AuChromoEar::ChromosomeType()));	// optional

	// Design a helmet to fit the head.
	float headAlt = dna.HeadAltitude();
	double margin = .75/12;
	double radX = head->RadiusX() + margin;
	double radY = head->RadiusY() + margin;
	double radZ = head->RadiusZ() + margin;
	double radFaceY = dna.FaceRadiusY(*head) + margin;	// Allow space in front for facial features. Let's hope they fit.
	double faceScale = radFaceY / radY;
	double helmThick = .8/12;
	double earThick;
	double earRad;
	if(ear)
	{
		if(ear->Radius() > margin)
			earThick = helmThick + (ear->Radius() - margin);
		else
			earThick = helmThick + .5f/12;
		earRad = ear->Radius();
	}
	else
	{
		earThick = helmThick;
		earRad = 0;
	}

	// Primary helmet bulb, with face cut out by performing only a partial sweep.
	AuScFaceList fskull;
	AuScFaceList fphones;
	double helmSweep = 240*DEG2RAD;
	{
		// Start at left ear.
		AuPointArray profile;
		profile.append(AuPoint(-radX,0,0));
		if(earRad > 0)
		{
			profile.append(AuPoint(-(radX + earThick),0,0));
			profile.append(AuPoint(-(radX + earThick),-earRad,0));
			profile.append(AuPoint(-(radX + helmThick),-earRad * 1.25,0));
		}
		else
			profile.append(AuPoint(-(radX + helmThick),0,0));

		// Arc over to other ear.
		double angbeg = profile.last().ArcTangent_2D();
		double angend = -PI - angbeg;
		double angsweep = fabs(angend - angbeg);
		int steps = (int)(angsweep / (10 * DEG2RAD));
		double anginc = angsweep / steps;
		for(int s=1; s<steps; s++)
		{
			double ang = angbeg + anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
			profile.append(p);
		}

		// Right ear outside
		if(earRad > 0)
		{
			profile.append(AuPoint(radX + helmThick,-earRad * 1.25,0));
			profile.append(AuPoint(radX + earThick,-earRad,0));
			profile.append(AuPoint(radX + earThick,0,0));
		}
		else
			profile.append(AuPoint(radX + helmThick,0,0));

		// Arc back over to the left ear to line the helmet. This one does not bulge out for ears; it is just a smooth dome.
		steps = (int)PI / (10*DEG2RAD);
		anginc = PI / steps;
		for(int s=0; s<=steps; s++)
		{
			double ang = -anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			AuPoint p(ca * radX, sa * radX, 0);
			profile.append(p);
		}

		fskull.MakeSpindle(profile, steps, AuMatrix4x4(), helmSweep);

		AuPointArray tris;
		QVector<bool> edges;
		profile.Triangulate(tris, edges);
		AuScFacePolygon fcap(tris, edges);
		fskull.AddFace(fcap);

		AuMatrix4x4 mf;
		mf.RotateX(helmSweep);
		fcap.Transform(mf);
		fcap.ReverseVertexNormals();
		fskull.AddFace(fcap);

		AuScSelectInfo selInfo;
		fskull.FaceIds(selInfo.m_Faces);
		fskull.VertexIds(selInfo.m_Vertices);
		fskull.TextureSpherical(selInfo, 0, 1, AuMatrix4x4());

		// Earphones
		AuScFaceList fphone;
		double phoneX = radX - earThick * .25;
		double phoneThick = earThick * 1.4;
		profile.clear();
		profile.append(AuPoint(phoneX,0,0));
		profile.append(AuPoint(phoneX,earRad,0));
		profile.append(AuPoint(phoneX + phoneThick,earRad,0));
		profile.append(AuPoint(phoneX + phoneThick,0,0));
		fphone.MakeSpindle(profile, 12, AuMatrix4x4());
		fphone.TextureSpherical(selInfo, 0, 5, AuMatrix4x4());
		fphones.AddFaces(fphone);

		AuMatrix4x4 mp;
		mp.RotateZ(PI);
		fphone.Transform(mp);
		fphones.AddFaces(fphone);

		// Scale and transform to head.
		double yscl = radY / radX;
		double zscl = radZ / radX;
		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt));
		m.Scale(1, yscl, zscl);
		fphones.Transform(m);
		m.RotateX(-100*DEG2RAD);
		fskull.Transform(m);

		// We may need to stretch the front to accommodate facial features.
		if(faceScale > 1)
		{
			fskull.ScalePositiveY(faceScale);
			fphones.ScalePositiveY(faceScale);
		}
	}

	// Visor
	AuScFaceList fvisor;
	{
		double visRadOut = radX + helmThick * .75;

		// Start at left ear.
		AuPointArray profile;
		profile.append(AuPoint(-visRadOut,0,0));

		// Arc over to other ear.
		int steps = (int)(PI / (10 * DEG2RAD));
		double anginc = PI / steps;
		for(int s=1; s<steps; s++)
		{
			double ang = -PI + anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			AuPoint p(ca * visRadOut, sa * visRadOut, 0);
			profile.append(p);
		}

		// Right ear outside
		profile.append(AuPoint(visRadOut,0,0));

		double visorSweep = PI*2 - helmSweep;
		fvisor.MakeSpindle(profile, steps, AuMatrix4x4(), visorSweep);

		AuScSelectInfo selInfo;
		fvisor.FaceIds(selInfo.m_Faces);
		fvisor.VertexIds(selInfo.m_Vertices);
		fvisor.TextureSpherical(selInfo, 0, 1, AuMatrix4x4());
		fvisor.PolishFaces(selInfo, 100, 1);

		// Scale and transform to head.
		double yscl = radY / radX;
		double zscl = radZ / radX;
		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt));
		m.Scale(1, yscl, zscl);
		m.RotateX(-100*DEG2RAD - (PI*2 - helmSweep));
		fvisor.Transform(m);

		// We may need to stretch the front to accommodate facial features.
		if(faceScale > 1)
			fvisor.ScalePositiveY(faceScale);
	}

	// Collar is a spindle around the neck.
	AuScFaceList fcollar;
	{
		double collarHt = helmThick * 2;
		double collarThick = helmThick;
		double flangeThick = collarThick * 1.2;
		double collarRadX = radX * .7;
		AuPointArray profile;
		profile.append(AuPoint(-collarHt,0, 0));
		profile.append(AuPoint(0,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt * .5,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt * .5,collarRadX + flangeThick, 0));
		profile.append(AuPoint(collarHt * .75,collarRadX + flangeThick, 0));
		profile.append(AuPoint(collarHt * .75,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt,collarRadX, 0));
		profile.append(AuPoint(-collarHt,0, 0));
		
		int steps = 12;
		fcollar.MakeSpindle(profile, steps, AuMatrix4x4());

		AuScSelectInfo selInfo;
		fcollar.FaceIds(selInfo.m_Faces);
		fcollar.VertexIds(selInfo.m_Vertices);
		fcollar.TextureCylindrical(selInfo, 0, 1, collarHt * 5, AuMatrix4x4());

		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt - radZ + collarHt/2));
		m.RotateY(90*DEG2RAD);
		double yscl = radY / radX;
		m.Scale(1,yscl,1);
		fcollar.Transform(m);
		fcollar.FlattenVertexNormals();
	}

	AuClothingMesh helm;
	helm.FromFaces(fskull, AuClothingTri::Helmet, surfaceTex, dye);
	helm.FromFaces(fphones, AuClothingTri::Helmet, phonesTex, dye);
	helm.FromFaces(fcollar, AuClothingTri::Helmet, phonesTex, dye);
	helm.FromFaces(fvisor, AuClothingTri::Visor, visorTex, visorColor);

	// Configure helmet bones to match head bones.
	quint8 boneIndex[4];
	float boneWeight[4];
	if(meat.Bones(AuMeatTri::Head, boneIndex, boneWeight))
		helm.SetBones(boneIndex[0], boneWeight[0], boneIndex[1], boneWeight[1], boneIndex[2], boneWeight[2], boneIndex[3],
				boneWeight[3]);

	helm.ApplyDecals(decals);

	clothing.Append(helm);
}

void AuClothing::GetPowerArmorHelmetMesh(AuClothingMesh& clothing, int helmTex, int phonesTex, const AuChromatid& dna,
		const AuMeatMesh& meat, const QColor& dye, int visorTex, const QColor& visorColor, const QList<AuClothingDecal>& decals,
		COMMODITYID helmCID) const
{
	const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromoHead::ChromosomeType()));
	if(!head)
		return;	// wut?
	const AuChromoEar* ear = dynamic_cast<const AuChromoEar*>(dna.Chromosome(AuChromoEar::ChromosomeType()));	// optional

	float headAlt = dna.HeadAltitude();
	double margin;
	double helmThick;
	double peakHt;
	double helmSweep;
	double helmRot;
	switch(helmCID)
	{
	case CID_MetalPowerArmorHelmet:
		margin = .7/12;
		helmThick = .7/12;
		peakHt = helmThick + .25/12;
		helmSweep = 290*DEG2RAD;
		helmRot = -150*DEG2RAD;
		break;
	case CID_MagmiumPowerArmorHelmet:
		margin = .6/12;
		helmThick = .6/12;
		peakHt = helmThick + .35/12;
		helmSweep = 300*DEG2RAD;
		helmRot = -160*DEG2RAD;
		break;
	case CID_VulciumPowerArmorHelmet:
		margin = .5/12;
		helmThick = .5/12;
		peakHt = helmThick + .4/12;
		helmSweep = 310*DEG2RAD;
		helmRot = -165*DEG2RAD;
		break;
	default:
		// This should get their attention. :D
		margin = 2./12;
		helmThick = 1./12;
		peakHt = helmThick + 1.;
		helmSweep = 270*DEG2RAD;
		helmRot = -130*DEG2RAD;
		break;
	}

	// Design a helmet to fit the head.
	double radX = head->RadiusX() + margin;
	double radY = head->RadiusY() + margin;
	double radZ = head->RadiusZ() + margin;
	double radFaceY = dna.FaceRadiusY(*head) + margin;	// Allow space in front for facial features. Let's hope they fit.
	double faceScale = radFaceY / radY;
	double earThick;
	double earRad;
	if(ear)
	{
		if(ear->Radius() > margin)
			earThick = helmThick + (ear->Radius() - margin);
		else
			earThick = helmThick + .5f/12;
		earRad = ear->Radius();
	}
	else
	{
		earThick = helmThick;
		earRad = 0;
	}

	// Primary helmet bulb, with face cut out by performing only a partial sweep.
	AuScFaceList fskull;
	AuScFaceList fphones;
	{
		// Start at left ear.
		AuPointArray profile;
		profile.append(AuPoint(-radX,0,0));
		if(earRad > 0)
		{
			profile.append(AuPoint(-(radX + earThick),0,0));
			profile.append(AuPoint(-(radX + earThick),-earRad,0));
			profile.append(AuPoint(-(radX + helmThick),-earRad * 1.25,0));
		}
		else
			profile.append(AuPoint(-(radX + helmThick),0,0));

		// Arc over to other ear.
		double angbeg = profile.last().ArcTangent_2D();
		double angend = -PI - angbeg;
		double angsweep = fabs(angend - angbeg);
		int steps = (int)(angsweep / (10 * DEG2RAD));
		steps |= 0x01;	// Now it's an odd number.
		int peakStep = steps/2;
		int mstep1 = peakStep - 1;
		int mstep2 = peakStep + 2;
		double anginc = angsweep / steps;
		for(int s=1; s<steps; s++)
		{
			double ang = angbeg + anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			switch(helmCID)
			{
			case CID_MetalPowerArmorHelmet:
				if(s >= mstep1 && s <= mstep2)
				{
					AuPoint p(ca * (radX + peakHt), sa * (radX + peakHt), 0);
					profile.append(p);
				}
				else
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			case CID_MagmiumPowerArmorHelmet:
				if(s == mstep1 || s == mstep2)
				{
					AuPoint p(ca * (radX + peakHt), sa * (radX + peakHt), 0);
					profile.append(p);
				}
				else
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			case CID_VulciumPowerArmorHelmet:
				if(s == peakStep)
				{
					AuPoint p(ca * (radX + peakHt), sa * (radX + peakHt), 0);
					profile.append(p);
				}
				else
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			default:
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			}
		}

		// Right ear outside
		if(earRad > 0)
		{
			profile.append(AuPoint(radX + helmThick,-earRad * 1.25,0));
			profile.append(AuPoint(radX + earThick,-earRad,0));
			profile.append(AuPoint(radX + earThick,0,0));
		}
		else
			profile.append(AuPoint(radX + helmThick,0,0));

		// Arc back over to the left ear to line the helmet. This one does not bulge out for ears; it is just a smooth dome.
		steps = (int)PI / (10*DEG2RAD);
		anginc = PI / steps;
		for(int s=0; s<=steps; s++)
		{
			double ang = -anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			AuPoint p(ca * radX, sa * radX, 0);
			profile.append(p);
		}

		fskull.MakeSpindle(profile, steps, AuMatrix4x4(), helmSweep);

		AuPointArray tris;
		QVector<bool> edges;
		profile.Triangulate(tris, edges);
		AuScFacePolygon fcap(tris, edges);
		fskull.AddFace(fcap);

		AuMatrix4x4 mf;
		mf.RotateX(helmSweep);
		fcap.Transform(mf);
		fcap.ReverseVertexNormals();
		fskull.AddFace(fcap);

		AuScSelectInfo selInfo;
		fskull.FaceIds(selInfo.m_Faces);
		fskull.VertexIds(selInfo.m_Vertices);
		fskull.TextureSpherical(selInfo, 0, 1, AuMatrix4x4());

		// Earphones
		AuScFaceList fphone;
		double phoneX = radX - earThick * .25;
		double phoneThick = earThick * 1.3;
		profile.clear();
		profile.append(AuPoint(phoneX,0,0));
		profile.append(AuPoint(phoneX,earRad,0));
		profile.append(AuPoint(phoneX + phoneThick,earRad,0));
		profile.append(AuPoint(phoneX + phoneThick,0,0));
		fphone.MakeSpindle(profile, 12, AuMatrix4x4());
		fphone.TextureSpherical(selInfo, 0, 5, AuMatrix4x4());
		fphones.AddFaces(fphone);

		AuMatrix4x4 mp;
		mp.RotateZ(PI);
		fphone.Transform(mp);
		fphones.AddFaces(fphone);

		// Scale and transform to head.
		double yscl = radY / radX;
		double zscl = radZ / radX;
		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt));
		m.Scale(1, yscl, zscl);
		fphones.Transform(m);
		m.RotateX(helmRot);
		fskull.Transform(m);

		// We may need to stretch the front to accommodate facial features.
		if(faceScale > 1)
		{
			fskull.ScalePositiveY(faceScale);
			fphones.ScalePositiveY(faceScale);
		}
	}

	// Visor
	AuScFaceList fvisor;
	{
		double visRadOut = radX + helmThick * .75;

		// Start at left ear.
		AuPointArray profile;
		profile.append(AuPoint(-visRadOut,0,0));

		// Arc over to other ear.
		int steps = (int)(PI / (10 * DEG2RAD));
		double anginc = PI / steps;
		for(int s=1; s<steps; s++)
		{
			double ang = -PI + anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			AuPoint p(ca * visRadOut, sa * visRadOut, 0);
			profile.append(p);
		}

		// Right ear outside
		profile.append(AuPoint(visRadOut,0,0));

		double visorSweep = PI*2 - helmSweep;
		fvisor.MakeSpindle(profile, steps, AuMatrix4x4(), visorSweep);

		AuScSelectInfo selInfo;
		fvisor.FaceIds(selInfo.m_Faces);
		fvisor.VertexIds(selInfo.m_Vertices);
		fvisor.TextureSpherical(selInfo, 0, 1, AuMatrix4x4());
		fvisor.PolishFaces(selInfo, 100, 1);

		// Scale and transform to head.
		double yscl = radY / radX;
		double zscl = radZ / radX;
		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt));
		m.Scale(1, yscl, zscl);
		m.RotateX(helmRot - (PI*2 - helmSweep));
		fvisor.Transform(m);

		// We may need to stretch the front to accommodate facial features.
		if(faceScale > 1)
			fvisor.ScalePositiveY(faceScale);
	}

	// Collar is a spindle around the neck.
	AuScFaceList fcollar;
	{
		double collarHt = peakHt * 2;
		double collarThick = helmThick;
		double flangeThick = collarThick * 1.2;
		double collarRadX = radX * .7;
		AuPointArray profile;
		profile.append(AuPoint(-collarHt,0, 0));
		profile.append(AuPoint(0,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt * .5,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt * .5,collarRadX + flangeThick, 0));
		profile.append(AuPoint(collarHt * .75,collarRadX + flangeThick, 0));
		profile.append(AuPoint(collarHt * .75,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt,collarRadX + collarThick, 0));
		profile.append(AuPoint(collarHt,collarRadX, 0));
		profile.append(AuPoint(-collarHt,0, 0));
		
		int steps = 12;
		fcollar.MakeSpindle(profile, steps, AuMatrix4x4());

		AuScSelectInfo selInfo;
		fcollar.FaceIds(selInfo.m_Faces);
		fcollar.VertexIds(selInfo.m_Vertices);
		fcollar.TextureCylindrical(selInfo, 0, 1, collarHt * 5, AuMatrix4x4());

		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt - radZ + collarHt/2));
		m.RotateY(90*DEG2RAD);
		double yscl = radY / radX;
		m.Scale(1,yscl,1);
		fcollar.Transform(m);
		fcollar.FlattenVertexNormals();
	}

	AuClothingMesh helm;
	helm.FromFaces(fskull, AuClothingTri::Helmet, helmTex, dye);
	helm.FromFaces(fphones, AuClothingTri::Helmet, phonesTex, dye);
	helm.FromFaces(fcollar, AuClothingTri::Helmet, phonesTex, dye);
	helm.FromFaces(fvisor, AuClothingTri::Visor, visorTex, visorColor);

	// Configure helmet bones to match head bones.
	quint8 boneIndex[4];
	float boneWeight[4];
	if(meat.Bones(AuMeatTri::Head, boneIndex, boneWeight))
		helm.SetBones(boneIndex[0], boneWeight[0], boneIndex[1], boneWeight[1], boneIndex[2], boneWeight[2], boneIndex[3],
				boneWeight[3]);

	helm.ApplyDecals(decals);

	clothing.Append(helm);
}

void AuClothing::GetHelmetMesh(AuClothingMesh& clothing, int helmTex, const AuChromatid& dna, const AuMeatMesh& meat,
		const QColor& helmColor, const QList<AuClothingDecal>& decals, COMMODITYID helmCID) const
{
	const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromoHead::ChromosomeType()));
	if(!head)
		return;	// wut?
	const AuChromoEar* ear = dynamic_cast<const AuChromoEar*>(dna.Chromosome(AuChromoEar::ChromosomeType()));	// optional

	float headAlt = dna.HeadAltitude();
	double margin;
	double helmThick;
	double peakHt;
	double helmSweep;
	double helmRot;
	bool earBulge;
	bool faceBulge;
	switch(helmCID)
	{
	case CID_LeatherHelmet:
		margin = .3/12;
		helmThick = .4/12;
		peakHt = helmThick;
		helmSweep = 180*DEG2RAD;
		helmRot = -150*DEG2RAD;
		earBulge = false;
		faceBulge = false;
		break;
	case CID_WovenHelmet:
		margin = .5/12;
		helmThick = .5/12;
		peakHt = helmThick;
		helmSweep = 220*DEG2RAD;
		helmRot = -165*DEG2RAD;
		earBulge = true;
		faceBulge = false;
		break;
	case CID_PlasticHelmet:
		margin = .2/12;
		helmThick = .75/12;
		peakHt = helmThick;
		helmSweep = 325*DEG2RAD;
		helmRot = -165*DEG2RAD;
		earBulge = true;
		faceBulge = true;
		break;
	case CID_MetalHelmet:
		margin = .7/12;
		helmThick = .3/12;
		peakHt = helmThick;
		helmSweep = 180*DEG2RAD;
		helmRot = -160*DEG2RAD;
		earBulge = true;
		faceBulge = false;
		break;
	default:
		// This should get their attention. :D
		margin = 2./12;
		helmThick = 1./12;
		peakHt = helmThick + 1.;
		helmSweep = 270*DEG2RAD;
		helmRot = -130*DEG2RAD;
		earBulge = true;
		faceBulge = false;
		break;
	}

	// Design a helmet to fit the head.
	double radX = head->RadiusX() + margin;
	double radY = head->RadiusY() + margin;
	double radZ = head->RadiusZ() + margin;
	double radFaceY;
	if(faceBulge)
		radFaceY = dna.FaceRadiusY(*head) + margin;	// Allow space in front for facial features. Let's hope they fit.
	else
		radFaceY = radY;
	double faceScale = radFaceY / radY;
	double earThick;
	double earRad;
	if(ear && earBulge)
	{
		if(ear->Radius() > margin)
			earThick = helmThick + (ear->Radius() - margin);
		else
			earThick = helmThick + .5f/12;
		earRad = ear->Radius();
	}
	else
	{
		earThick = 0;
		earRad = 0;
	}

	// Primary helmet bulb, with face cut out by performing only a partial sweep.
	AuScFaceList fskull;
	AuScFaceList fphones;
	{
		// Start at left ear.
		AuPointArray profile;
		profile.append(AuPoint(-radX,0,0));
		if(earRad > 0)
		{
			profile.append(AuPoint(-(radX + earThick),0,0));
			profile.append(AuPoint(-(radX + earThick),-earRad,0));
			profile.append(AuPoint(-(radX + helmThick),-earRad * 1.25,0));
		}
		else
			profile.append(AuPoint(-(radX + helmThick),0,0));

		// Arc over to other ear.
		double angbeg = profile.last().ArcTangent_2D();
		if(angbeg == PI)
			angbeg = -PI;
		double angend = -PI - angbeg;
		double angsweep = fabs(angend - angbeg);
		int steps = (int)(angsweep / (10 * DEG2RAD));
		steps |= 0x01;	// Now it's an odd number.
		int peakStep = steps/2;
		int mstep1 = peakStep - 1;
		int mstep2 = peakStep + 2;
		double anginc = angsweep / steps;
		for(int s=1; s<steps; s++)
		{
			double ang = angbeg + anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			switch(helmCID)
			{
			case CID_MetalPowerArmorHelmet:
				if(s >= mstep1 && s <= mstep2)
				{
					AuPoint p(ca * (radX + peakHt), sa * (radX + peakHt), 0);
					profile.append(p);
				}
				else
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			case CID_MagmiumPowerArmorHelmet:
				if(s == mstep1 || s == mstep2)
				{
					AuPoint p(ca * (radX + peakHt), sa * (radX + peakHt), 0);
					profile.append(p);
				}
				else
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			case CID_VulciumPowerArmorHelmet:
				if(s == peakStep)
				{
					AuPoint p(ca * (radX + peakHt), sa * (radX + peakHt), 0);
					profile.append(p);
				}
				else
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			default:
				{
					AuPoint p(ca * (radX + helmThick), sa * (radX + helmThick), 0);
					profile.append(p);
				}
				break;
			}
		}

		// Right ear outside
		if(earRad > 0)
		{
			profile.append(AuPoint(radX + helmThick,-earRad * 1.25,0));
			profile.append(AuPoint(radX + earThick,-earRad,0));
			profile.append(AuPoint(radX + earThick,0,0));
		}
		else
			profile.append(AuPoint(radX + helmThick,0,0));

		// Arc back over to the left ear to line the helmet. This one does not bulge out for ears; it is just a smooth dome.
		steps = (int)PI / (10*DEG2RAD);
		anginc = PI / steps;
		for(int s=0; s<=steps; s++)
		{
			double ang = -anginc * s;
			double ca = cos(ang);
			double sa = sin(ang);
			AuPoint p(ca * radX, sa * radX, 0);
			profile.append(p);
		}

		fskull.MakeSpindle(profile, steps, AuMatrix4x4(), helmSweep);

		AuPointArray tris;
		QVector<bool> edges;
		profile.Triangulate(tris, edges);
		AuScFacePolygon fcap(tris, edges);
		fskull.AddFace(fcap);

		AuMatrix4x4 mf;
		mf.RotateX(helmSweep);
		fcap.Transform(mf);
		fcap.ReverseVertexNormals();
		fskull.AddFace(fcap);

		AuScSelectInfo selInfo;
		fskull.FaceIds(selInfo.m_Faces);
		fskull.VertexIds(selInfo.m_Vertices);
		fskull.TextureSpherical(selInfo, 0, 1, AuMatrix4x4());

		// Earphones
		AuScFaceList fphone;
		double phoneX = radX - earThick * .25;
		double phoneThick = earThick * 1.3;
		profile.clear();
		profile.append(AuPoint(phoneX,0,0));
		profile.append(AuPoint(phoneX,earRad,0));
		profile.append(AuPoint(phoneX + phoneThick,earRad,0));
		profile.append(AuPoint(phoneX + phoneThick,0,0));
		fphone.MakeSpindle(profile, 12, AuMatrix4x4());
		fphone.TextureSpherical(selInfo, 0, 5, AuMatrix4x4());
		fphones.AddFaces(fphone);

		AuMatrix4x4 mp;
		mp.RotateZ(PI);
		fphone.Transform(mp);
		fphones.AddFaces(fphone);

		// Scale and transform to head.
		double yscl = radY / radX;
		double zscl = radZ / radX;
		AuMatrix4x4 m;
		m.Translate(head->CenterOffset() + AuPoint(0,0,headAlt));
		m.Scale(1, yscl, zscl);
		fphones.Transform(m);
		m.RotateX(helmRot);
		fskull.Transform(m);

		// We may need to stretch the front to accommodate facial features.
		if(faceScale > 1)
		{
			fskull.ScalePositiveY(faceScale);
			fphones.ScalePositiveY(faceScale);
		}
	}

	AuClothingMesh helm;
	helm.FromFaces(fskull, AuClothingTri::Helmet, helmTex, helmColor);

	// Configure helmet bones to match head bones.
	quint8 boneIndex[4];
	float boneWeight[4];
	if(meat.Bones(AuMeatTri::Head, boneIndex, boneWeight))
		helm.SetBones(boneIndex[0], boneWeight[0], boneIndex[1], boneWeight[1], boneIndex[2], boneWeight[2], boneIndex[3],
				boneWeight[3]);

	helm.ApplyDecals(decals);

	clothing.Append(helm);
}

void AuClothing::GetShoesMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float bootStrapAlt, const QColor& dye) const
{
	AuClothingMesh shoes;
	GetLegsMesh(shoes, AuClothingTri::Shoes, texIndex, dna, meat, skinDistance, bootStrapAlt, -10000, dye, 0);
	GetShoesMesh(shoes, AuClothingTri::Shoes, texIndex, dna, meat, skinDistance, dye);

	shoes.SmoothNormals();

	clothing.Append(shoes);
}

void AuClothing::GetGlovesMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float cuff, const QColor& dye) const
{
	AuClothingMesh gloves;
	GetHandsMesh(gloves, AuClothingTri::Gloves, texIndex, dna, meat, skinDistance, cuff, dye);

	gloves.SmoothNormals();

	clothing.Append(gloves);
}

void AuClothing::GetChestMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float neckAlt, float waistAlt, const QColor& dye, float thickness) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::TorsoUpper &&
				(*mi).Part() != AuMeatTri::TorsoLower &&
				(*mi).Part() != AuMeatTri::MammaryUpper)
			continue;

		// Make a clothing triangle from the meat triangle.
		const AuPointN& v0 = (*mi).m_V0;
		const AuPointN& v1 = (*mi).m_V1;
		const AuPointN& v2 = (*mi).m_V2;
		if(v0.z > neckAlt && v1.z > neckAlt && v2.z > neckAlt)
			continue;
		if(v0.z < waistAlt && v1.z < waistAlt && v2.z < waistAlt)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	AuClothingSegmentArray edges;
	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::TorsoUpper &&
				(*mi).Part() != AuMeatTri::TorsoLower &&
				(*mi).Part() != AuMeatTri::MammaryUpper)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;
		if(v0.z > neckAlt && v1.z > neckAlt && v2.z > neckAlt)
			continue;
		if(v0.z < waistAlt && v1.z < waistAlt && v2.z < waistAlt)
			continue;

		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		if(v0.z < waistAlt)
			v0.z = waistAlt;
		if(v1.z < waistAlt)
			v1.z = waistAlt;
		if(v2.z < waistAlt)
			v2.z = waistAlt;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);

		if(thickness > 0 && (*mi).Part() != AuMeatTri::MammaryUpper)
		{
			// When creating walls at the edges for thickness, we only want unique triangle edge lines. Since we don't really ever
			// get more than two triangles sharing the same edge, we'll add an edge where we haven't found one and throw it away
			// if we find another edge there.
			AuClothingSegment seg1(tri.m_V0, tri.m_V1);
			AuClothingSegmentArray::iterator ei = edges.FindSegmentFuzzy(seg1);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg1);

			AuClothingSegment seg2(tri.m_V1, tri.m_V2);
			ei = edges.FindSegmentFuzzy(seg2);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg2);

			AuClothingSegment seg3(tri.m_V2, tri.m_V0);
			ei = edges.FindSegmentFuzzy(seg3);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg3);
		}
	}

	// Add side walls at edges.
	for(AuClothingSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
	{
		AuClothingPoint e1((*ei).m_Loc1);
		AuClothingPoint b1(e1);
		b1.AuPoint::operator-=(e1.m_N * thickness);
		AuClothingPoint e2((*ei).m_Loc2);
		AuClothingPoint b2(e2);
		b2.AuPoint::operator-=(e2.m_N * thickness);

		AuClothingTri tri1(part, texIndex, b1, b2, e2);
		clothing.append(tri1);
		AuClothingTri tri2(part, texIndex, e2, e1, b1);
		clothing.append(tri2);
	}

	clothing.Append(cloth);
}

void AuClothing::GetGroinMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float waistAlt, const QColor& dye) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::TorsoUpper &&
				(*mi).Part() != AuMeatTri::TorsoLower &&
				(*mi).Part() != AuMeatTri::MammaryUpper)
			continue;

		// Make a clothing triangle from the meat triangle.
		const AuPointN& v0 = (*mi).m_V0;
		const AuPointN& v1 = (*mi).m_V1;
		const AuPointN& v2 = (*mi).m_V2;
		if(v0.z > waistAlt && v1.z > waistAlt && v2.z > waistAlt)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::TorsoUpper &&
				(*mi).Part() != AuMeatTri::TorsoLower &&
				(*mi).Part() != AuMeatTri::MammaryUpper)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;
		if(v0.z > waistAlt && v1.z > waistAlt && v2.z > waistAlt)
			continue;

		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		if(v0.z > waistAlt)
			v0.z = waistAlt;
		if(v1.z > waistAlt)
			v1.z = waistAlt;
		if(v2.z > waistAlt)
			v2.z = waistAlt;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);
	}

	clothing.Append(cloth);
}

void AuClothing::GetTailMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, const QColor& dye) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Tail)
			continue;

		// Make a clothing triangle from the meat triangle.
		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Tail)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;

		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);
	}

	clothing.Append(cloth);
}

void AuClothing::GetArmsMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float cuff, const QColor& dye, float thickness) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Arm)
			continue;

		if((*mi).m_V0.m_SkinBottomTy > cuff && (*mi).m_V1.m_SkinBottomTy > cuff && (*mi).m_V2.m_SkinBottomTy > cuff)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	AuClothingSegmentArray edges;
	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Arm)
			continue;

		if((*mi).m_V0.m_SkinBottomTy > cuff && (*mi).m_V1.m_SkinBottomTy > cuff && (*mi).m_V2.m_SkinBottomTy > cuff)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;
		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);

		if(thickness > 0)
		{
			// When creating walls at the edges for thickness, we only want unique triangle edge lines. Since we don't really ever
			// get more than two triangles sharing the same edge, we'll add an edge where we haven't found one and throw it away
			// if we find another edge there.
			AuClothingSegment seg1(tri.m_V0, tri.m_V1);
			AuClothingSegmentArray::iterator ei = edges.FindSegmentFuzzy(seg1);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg1);

			AuClothingSegment seg2(tri.m_V1, tri.m_V2);
			ei = edges.FindSegmentFuzzy(seg2);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg2);

			AuClothingSegment seg3(tri.m_V2, tri.m_V0);
			ei = edges.FindSegmentFuzzy(seg3);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg3);
		}
	}

	// Add side walls at edges.
	for(AuClothingSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
	{
		AuClothingPoint e1((*ei).m_Loc1);
		AuClothingPoint b1(e1);
		b1.AuPoint::operator-=(e1.m_N * thickness);
		AuClothingPoint e2((*ei).m_Loc2);
		AuClothingPoint b2(e2);
		b2.AuPoint::operator-=(e2.m_N * thickness);

		AuClothingTri tri1(part, texIndex, b1, b2, e2);
		clothing.append(tri1);
		AuClothingTri tri2(part, texIndex, e2, e1, b1);
		clothing.append(tri2);
	}

	clothing.Append(cloth);
}

void AuClothing::GetNeckMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float collar, const QColor& dye) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Neck)
			continue;

		if((*mi).m_V0.m_SkinBottomTy > collar && (*mi).m_V1.m_SkinBottomTy > collar && (*mi).m_V2.m_SkinBottomTy > collar)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Neck)
			continue;

		if((*mi).m_V0.m_SkinBottomTy > collar && (*mi).m_V1.m_SkinBottomTy > collar && (*mi).m_V2.m_SkinBottomTy > collar)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;
		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);
	}

	clothing.Append(cloth);
}

void AuClothing::GetHandsMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float cuff, const QColor& dye) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() == AuMeatTri::Arm)
		{
			if((*mi).m_V0.m_SkinBottomTy < cuff && (*mi).m_V1.m_SkinBottomTy < cuff && (*mi).m_V2.m_SkinBottomTy < cuff)
				continue;
		}
		else if((*mi).Part() != AuMeatTri::Finger && (*mi).Part() != AuMeatTri::Hand && (*mi).Part() != AuMeatTri::Thumb)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() == AuMeatTri::Arm)
		{
			if((*mi).m_V0.m_SkinBottomTy < cuff && (*mi).m_V1.m_SkinBottomTy < cuff && (*mi).m_V2.m_SkinBottomTy < cuff)
				continue;
		}
		else if((*mi).Part() != AuMeatTri::Finger && (*mi).Part() != AuMeatTri::Hand && (*mi).Part() != AuMeatTri::Thumb)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;
		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);
	}

	clothing.Append(cloth);
}

void AuClothing::GetLegsMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, float waistAlt, float cuffAlt, const QColor& dye, float thickness) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Leg)
			continue;

		// Make a clothing triangle from the meat triangle.
		const AuPointN& v0 = (*mi).m_V0;
		const AuPointN& v1 = (*mi).m_V1;
		const AuPointN& v2 = (*mi).m_V2;
		if(v0.z < cuffAlt && v1.z < cuffAlt && v2.z < cuffAlt)
			continue;
		if(v0.z > waistAlt && v1.z > waistAlt && v2.z > waistAlt)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	AuClothingSegmentArray edges;
	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Leg)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;
		if(v0.z < cuffAlt && v1.z < cuffAlt && v2.z < cuffAlt)
			continue;
		if(v0.z > waistAlt && v1.z > waistAlt && v2.z > waistAlt)
			continue;

		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		if(v0.z < cuffAlt)
			v0.z = cuffAlt;
		if(v1.z < cuffAlt)
			v1.z = cuffAlt;
		if(v2.z < cuffAlt)
			v2.z = cuffAlt;

		if(v0.z > waistAlt)
			v0.z = waistAlt;
		if(v1.z > waistAlt)
			v1.z = waistAlt;
		if(v2.z > waistAlt)
			v2.z = waistAlt;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);

		if(thickness > 0)
		{
			// When creating walls at the edges for thickness, we only want unique triangle edge lines. Since we don't really ever
			// get more than two triangles sharing the same edge, we'll add an edge where we haven't found one and throw it away
			// if we find another edge there.
			AuClothingSegment seg1(tri.m_V0, tri.m_V1);
			AuClothingSegmentArray::iterator ei = edges.FindSegmentFuzzy(seg1);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg1);

			AuClothingSegment seg2(tri.m_V1, tri.m_V2);
			ei = edges.FindSegmentFuzzy(seg2);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg2);

			AuClothingSegment seg3(tri.m_V2, tri.m_V0);
			ei = edges.FindSegmentFuzzy(seg3);
			if(ei != edges.end())
				edges.erase(ei);
			else
				edges.append(seg3);
		}
	}

	// Add side walls at edges.
	for(AuClothingSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
	{
		AuClothingPoint e1((*ei).m_Loc1);
		AuClothingPoint b1(e1);
		b1.AuPoint::operator-=(e1.m_N * thickness);
		AuClothingPoint e2((*ei).m_Loc2);
		AuClothingPoint b2(e2);
		b2.AuPoint::operator-=(e2.m_N * thickness);

		AuClothingTri tri1(part, texIndex, b1, b2, e2);
		clothing.append(tri1);
		AuClothingTri tri2(part, texIndex, e2, e1, b1);
		clothing.append(tri2);
	}

	clothing.Append(cloth);
}

void AuClothing::GetShoesMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
		const AuMeatMesh& meat, float skinDistance, const QColor& dye) const
{
	// Determine top texture Y coord so we can scale the texture coords to fit vertically into what we end up with.
	float topTy = 0;
	float bottomTy = 0;
	bool haveTy = false;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Foot)
			continue;

		if(!haveTy)
		{
			topTy = (*mi).m_V0.m_SkinBottomTy;
			bottomTy = topTy;
			haveTy = true;
		}

		if((*mi).m_V0.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V0.m_SkinBottomTy;
		if((*mi).m_V0.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V0.m_SkinBottomTy;

		if((*mi).m_V1.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V1.m_SkinBottomTy;
		if((*mi).m_V1.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V1.m_SkinBottomTy;

		if((*mi).m_V2.m_SkinBottomTy > topTy)
			topTy = (*mi).m_V2.m_SkinBottomTy;
		if((*mi).m_V2.m_SkinBottomTy < bottomTy)
			bottomTy = (*mi).m_V2.m_SkinBottomTy;
	}

	if(!haveTy)
		return;
	float rangeTy = topTy - bottomTy;

	// Collect the cloth surface triangles.
	AuClothingMesh cloth;
	for(AuMeatMesh::const_iterator mi = meat.cbegin(); mi != meat.cend(); mi++)
	{
		if((*mi).Part() != AuMeatTri::Foot)
			continue;

		// Make a clothing triangle from the meat triangle.
		AuPointN v0 = (*mi).m_V0;
		AuPointN v1 = (*mi).m_V1;
		AuPointN v2 = (*mi).m_V2;

		v0 += (*mi).m_V0.m_N * skinDistance;
		v1 += (*mi).m_V1.m_N * skinDistance;
		v2 += (*mi).m_V2.m_N * skinDistance;

		QColor dye0(dye);
		QColor dye1(dye);
		QColor dye2(dye);

		AuClothingTri tri(part, texIndex, *mi, v0, dye0, v1, dye2, v2, dye2);
		if(topTy != 0)
		{
			tri.m_V0.m_ClothingTy = (tri.m_V0.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V1.m_ClothingTy = (tri.m_V1.m_ClothingTy - bottomTy) / rangeTy;
			tri.m_V2.m_ClothingTy = (tri.m_V2.m_ClothingTy - bottomTy) / rangeTy;
		}
		cloth.append(tri);
	}

	clothing.Append(cloth);
}

//static
void AuClothing::DecalTexture(CLOTH_DECAL tx, float& s, float& t, float& u, float& v)
{
	class TexPos
	{
	public:
		CLOTH_DECAL texId;
		int ox, oy;			// pixel coordinate of lower left corner of texture (pos y goes up)
		int sx, sy;			// size of texture in pixels

		TexPos(CLOTH_DECAL detailTexEnum, int offsetX, int offsetY, int sizeX, int sizeY)
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
		TexPos(CD_SoHLogo, 0, 288, 224, 224),
		TexPos(CD_ParcXLogo, 0, 176, 224, 160),
	};
	int texPosCount = sizeof(texPos)/sizeof(TexPos);
	if(tx > texPosCount)
		tx = CD_SoHLogo;
	Q_ASSERT(texPos[tx].texId == tx);	// Verify data validity.

	// Calculate texture coordinates.
	const double maxx = 512;
	const double maxy = 512;
	s = (texPos[tx].ox + .5) / maxx;
	t = (texPos[tx].oy + .5) / maxy;
	u = (texPos[tx].ox + texPos[tx].sx - .5) / maxx;
	v = (texPos[tx].oy + texPos[tx].sy - .5) / maxy;
}

bool AuClothing::CoverHair(const AuFolliclePoint& fp, bool basePatch, float& lockLen, bool wearsBra) const
{
	bool partial = false;
	if(m_Armor)
	{
		switch(m_Armor->CoverHair(fp, wearsBra))
		{
		case 0:				// Hair is not covered.
			break;
		case 1:
			partial = true;	// Hair is partially covered.
			break;
		case 2:
			return true;	// Hair is completely covered.
		}
	}

	if(m_Clothing)
	{
		switch(m_Clothing->CoverHair(fp, wearsBra))
		{
		case 0:				// Hair is not covered.
			break;
		case 1:
			partial = true;	// Hair is partially covered.
			break;
		case 2:
			return true;	// Hair is completely covered.
		}
	}

	if(m_Hat)
	{
		switch(m_Hat->CoverHair(fp, wearsBra))
		{
		case 0:				// Hair is not covered.
			break;
		case 1:
			partial = true;	// Hair is partially covered.
			break;
		case 2:
			return true;	// Hair is completely covered.
		}
	}

	if(partial)
	{
		// Reduce the hair to only the base patch, if any.
		lockLen = 0;
		return !basePatch;
	}
	return false;
}

bool AuClothing::BareUpperArmCovered() const
{
	if(m_Armor && m_Armor->BareUpperArmCovered())
		return true;
	if(m_Clothing && m_Clothing->BareUpperArmCovered())
		return true;
	return false;
}

bool AuClothing::BareForeArmCovered() const
{
	if(m_Armor && m_Armor->BareForeArmCovered())
		return true;
	if(m_Clothing && m_Clothing->BareForeArmCovered())
		return true;
	return false;
}

bool AuClothing::BareBodyCovered() const
{
	return 
		BareNeckCovered() &&
		BareUpperTorsoCovered() &&
		BareLowerTorsoCovered() &&

		BareUpperArmCovered() &&
		BareForeArmCovered() &&
		BareHandCovered() &&

		BareThighCovered() &&
		BareCalfCovered() &&
		BareFootCovered();
}

bool AuClothing::BareFootCovered() const
{
	if(m_Armor && m_Armor->BareFootCovered())
		return true;
	if(m_Clothing && m_Clothing->BareFootCovered())
		return true;
	return false;
}

bool AuClothing::BareHandCovered() const
{
	if(m_Armor && m_Armor->BareHandCovered())
		return true;
	if(m_Clothing && m_Clothing->BareHandCovered())
		return true;
	return false;
}

bool AuClothing::BareThighCovered() const
{
	if(m_Armor && m_Armor->BareThighCovered())
		return true;
	if(m_Clothing && m_Clothing->BareThighCovered())
		return true;
	return false;
}

bool AuClothing::BareCalfCovered() const
{
	if(m_Armor && m_Armor->BareCalfCovered())
		return true;
	if(m_Clothing && m_Clothing->BareCalfCovered())
		return true;
	return false;
}

bool AuClothing::BareNeckCovered() const
{
	if(m_Armor && m_Armor->BareNeckCovered())
		return true;
	if(m_Clothing && m_Clothing->BareNeckCovered())
		return true;
	return false;
}

bool AuClothing::BareUpperTorsoCovered() const
{
	if(m_Armor && m_Armor->BareUpperTorsoCovered())
		return true;
	if(m_Clothing && m_Clothing->BareUpperTorsoCovered())
		return true;
	return false;
}

bool AuClothing::BareLowerTorsoCovered() const
{
	if(m_Armor && m_Armor->BareLowerTorsoCovered())
		return true;
	if(m_Clothing && m_Clothing->BareLowerTorsoCovered())
		return true;
	return false;
}

bool AuClothing::SetArmor(const AuItem* item)
{
	if(m_Armor)
		delete m_Armor;

	if(item && item->CommodityId() != CID_Invalid)
		m_Armor = item->Clone();
	else
		m_Armor = 0;

	return true;
}

bool AuClothing::SetArmor(COMMODITYID cid)
{
	if(cid == CID_Invalid)
	{
		if(m_Armor)
		{
			delete m_Armor;
			m_Armor = 0;
			return true;
		}
		return false;
	}
	else
	{
		if(m_Armor)
		{
			if(m_Armor->CommodityId() == cid)
				return false;
			delete m_Armor;
			m_Armor = 0;
		}
		m_Armor = AuItem::Create(cid, 1, 1);
		return true;
	}
}

bool AuClothing::SetClothing(const AuItem* item)
{
	if(m_Clothing)
		delete m_Clothing;

	if(item && item->CommodityId() != CID_Invalid)
		m_Clothing = item->Clone();
	else
		m_Clothing = 0;

	return true;
}

bool AuClothing::SetClothing(COMMODITYID cid)
{
	if(cid == CID_Invalid)
	{
		if(m_Clothing)
		{
			delete m_Clothing;
			m_Clothing = 0;
			return true;
		}
		return false;
	}
	else
	{
		if(m_Clothing)
		{
			if(m_Clothing->CommodityId() == cid)
				return false;
			delete m_Clothing;
			m_Clothing = 0;
		}
		m_Clothing = AuItem::Create(cid, 1, 1);
		return true;
	}
}

bool AuClothing::SetHat(const AuItem* item)
{
	if(m_Hat)
		delete m_Hat;

	if(item && item->CommodityId() != CID_Invalid)
		m_Hat = item->Clone();
	else
		m_Hat = 0;

	return true;
}

bool AuClothing::SetHat(COMMODITYID cid)
{
	if(cid == CID_Invalid)
	{
		if(m_Hat)
		{
			delete m_Hat;
			m_Hat = 0;
			return true;
		}
		return false;
	}
	else
	{
		if(m_Hat)
		{
			if(m_Hat->CommodityId() == cid)
				return false;
			delete m_Hat;
			m_Hat = 0;
		}
		m_Hat = AuItem::Create(cid, 1, 1);
		return true;
	}
}

