//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglsl.cpp,v 1.183 2022/02/19 18:29:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auglsl.h"
//
#include "augleffects.h"
#include "auglslerr.h"
#include "auscface.h"
#include "hashpack.h"
#include <QFile>

const bool s_DebugGLSL = false;									// Set true to see debug messages generated in this file.

// Compressed source loaded from glsl.har. Only contains data during Initialize.
QHash<QString, QByteArray>	s_GLSLProgramSrc;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLVAO::CreateForLines(AuOpenGLFunctions& GLf, const AuPointArray& verts, bool closeBoundary, GLenum usage)
{
	if(verts.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of vertices in the vertex array object.
	int vertices = verts.count();
	if(closeBoundary)
	{
		// Caller wants the boundary closed.
		if(verts.first() != verts.last())
			vertices++;				// An additional vertex is needed to close the boundary.
		else
			closeBoundary = false;	// Boundary is already closed.
	}

	// Copy vertices into gl array buffer.
	// This array buffer corresponds literally to the 'in vec4 position' variable in the vertex shader.
	GLfloat* position = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	for(AuPointArray::const_iterator ti = verts.constBegin(); ti != verts.constEnd(); ti++)
	{
		position[pidx++] = (float)(*ti).x;
		position[pidx++] = (float)(*ti).y;
		position[pidx++] = (float)(*ti).z;
		position[pidx++] = (float)1;
	}

	if(closeBoundary)
	{
		position[pidx++] = (float)verts.first().x;
		position[pidx++] = (float)verts.first().y;
		position[pidx++] = (float)verts.first().z;
		position[pidx++] = (float)1;
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 1);

	SelectGL();

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] position;
}

void AuGLVAO::CreateForSprites(AuOpenGLFunctions& GLf, const QList<AuPointAlpha>& verts, GLenum usage)
{
	if(verts.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of vertices in the vertex array object.
	int vertices = verts.count();

	// Copy vertices into gl array buffer.
	// This array buffer corresponds literally to the 'in vec4 position' variable in the vertex shader.
	GLfloat* position = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	for(QList<AuPointAlpha>::const_iterator ti = verts.constBegin(); ti != verts.constEnd(); ti++)
	{
		position[pidx++] = (float)(*ti).x;
		position[pidx++] = (float)(*ti).y;
		position[pidx++] = (float)(*ti).z;
		position[pidx++] = (float)(*ti).m_Alpha;
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 1);

	SelectGL();

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] position;
}

void AuGLVAO::CreateForSpriteSizeColor(AuOpenGLFunctions& GLf, const QList<AuScPoint>& verts, GLenum usage)
{
	if(verts.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of vertices in the vertex array object.
	int vertices = verts.count();

	// Copy vertices into gl array buffer.
	// This array buffer corresponds literally to the 'in vec4 position' variable in the vertex shader.
	GLfloat* position = new GLfloat[vertices*4];
	GLfloat* color = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int cidx = 0;
	for(QList<AuScPoint>::const_iterator ti = verts.constBegin(); ti != verts.constEnd(); ti++)
	{
		position[pidx++] = (float)(*ti).x;
		position[pidx++] = (float)(*ti).y;
		position[pidx++] = (float)(*ti).z;
		position[pidx++] = (*ti).EffectIntensity();		// This is the sprite size.

		// Sprite color comes from the glow component of the point, including alpha.
		color[cidx++] = (float)(*ti).Glow().redF();
		color[cidx++] = (float)(*ti).Glow().greenF();
		color[cidx++] = (float)(*ti).Glow().blueF();
		color[cidx++] = (float)(*ti).Glow().alphaF();
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 2);

	SelectGL();

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, cidx*sizeof(GLfloat), color, usage);
	GLf.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] position;
	delete [] color;
}

void AuGLVAO::CreateForSc(AuOpenGLFunctions& GLf, const AuScTriRenderList& tris, GLenum usage, quint8 effect)
{
	//if(s_DebugGLSL)
		//qDebug() << "AuGLVAO::CreateForSc tris" << tris.count();
	if(tris.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Count triangles that will be used.
	int triCount = 0;
	for(AuScTriRenderList::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
		if(ti.key() == effect)
			triCount++;
	if(!triCount)
		return;	// None of the triangles have the surface effect.

	// Store count of triangles in the vertex array object.
	int vertices = triCount*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* positionShine = new GLfloat[vertices*4];
	GLfloat* normalReflect = new GLfloat[vertices*4];
	GLfloat* texCoordIndex = new GLfloat[vertices*4];
	GLfloat* texEffectAppl = new GLfloat[vertices*4];
	GLfloat* paintBuf = new GLfloat[vertices*4];
	GLfloat* glowBuf = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int tidx = 0;
	unsigned int eidx = 0;
	unsigned int aidx = 0;
	unsigned int gidx = 0;
	for(AuScTriRenderList::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
	{
		if(ti.key() != effect)
			continue;

		// Get paint color, which could have come from the theme. That substitution was done when the render tris were collected.
		QColor paintColor((*ti).Paint());

		// Vertex 0
		positionShine[pidx++] = (float)(*ti).V0().x;
		positionShine[pidx++] = (float)(*ti).V0().y;
		positionShine[pidx++] = (float)(*ti).V0().z;
		positionShine[pidx++] = (*ti).Shine();

		AuPoint n0((*ti).V0().m_N);	// Normalize normals.
		n0.Unit();
		normalReflect[nidx++] = (float)n0.x;
		normalReflect[nidx++] = (float)n0.y;
		normalReflect[nidx++] = (float)n0.z;
		normalReflect[nidx++] = (*ti).Reflectivity();

		texCoordIndex[tidx++] = (*ti).V0().m_Tx;
		texCoordIndex[tidx++] = (*ti).V0().m_Ty;
		texCoordIndex[tidx++] = (*ti).TextureIndex();
		texCoordIndex[tidx++] = (*ti).Opacity();

		texEffectAppl[eidx++] = (*ti).V0().EffectTx();
		texEffectAppl[eidx++] = (*ti).V0().EffectTy();
		texEffectAppl[eidx++] = (*ti).V0().EffectIntensity();
		texEffectAppl[eidx++] = (*ti).EffectAppl() ? 1 : 0;	// This has become an on/off switch.

		paintBuf[aidx++] = paintColor.redF();
		paintBuf[aidx++] = paintColor.greenF();
		paintBuf[aidx++] = paintColor.blueF();
		paintBuf[aidx++] = paintColor.alphaF();

		//if((*ti).V0().Glow().redF() > 0)
			//qDebug() << "red glow" << (*ti).V0().Glow().redF() << "at" << (*ti).V0().x << (*ti).V0().y << (*ti).V0().z;
		glowBuf[gidx++] = (*ti).V0().Glow().redF();
		glowBuf[gidx++] = (*ti).V0().Glow().greenF();
		glowBuf[gidx++] = (*ti).V0().Glow().blueF();
		glowBuf[gidx++] = (*ti).SelfIllumination();

		// Vertex 1
		positionShine[pidx++] = (float)(*ti).V1().x;
		positionShine[pidx++] = (float)(*ti).V1().y;
		positionShine[pidx++] = (float)(*ti).V1().z;
		positionShine[pidx++] = (*ti).Shine();

		AuPoint n1((*ti).V1().m_N);	// Normalize normals.
		n1.Unit();
		normalReflect[nidx++] = (float)n1.x;
		normalReflect[nidx++] = (float)n1.y;
		normalReflect[nidx++] = (float)n1.z;
		normalReflect[nidx++] = (*ti).Reflectivity();

		texCoordIndex[tidx++] = (*ti).V1().m_Tx;
		texCoordIndex[tidx++] = (*ti).V1().m_Ty;
		texCoordIndex[tidx++] = (*ti).TextureIndex();
		texCoordIndex[tidx++] = (*ti).Opacity();

		texEffectAppl[eidx++] = (*ti).V1().EffectTx();
		texEffectAppl[eidx++] = (*ti).V1().EffectTy();
		texEffectAppl[eidx++] = (*ti).V1().EffectIntensity();
		texEffectAppl[eidx++] = (*ti).EffectAppl() ? 1 : 0;	// This has become an on/off switch.

		paintBuf[aidx++] = paintColor.redF();
		paintBuf[aidx++] = paintColor.greenF();
		paintBuf[aidx++] = paintColor.blueF();
		paintBuf[aidx++] = paintColor.alphaF();

		//if((*ti).V1().Glow().redF() > 0)
			//qDebug() << "red glow" << (*ti).V1().Glow().redF() << "at" << (*ti).V1().x << (*ti).V1().y << (*ti).V1().z;
		glowBuf[gidx++] = (*ti).V1().Glow().redF();
		glowBuf[gidx++] = (*ti).V1().Glow().greenF();
		glowBuf[gidx++] = (*ti).V1().Glow().blueF();
		glowBuf[gidx++] = (*ti).SelfIllumination();

		// Vertex 2
		positionShine[pidx++] = (float)(*ti).V2().x;
		positionShine[pidx++] = (float)(*ti).V2().y;
		positionShine[pidx++] = (float)(*ti).V2().z;
		positionShine[pidx++] = (*ti).Shine();

		AuPoint n2((*ti).V2().m_N);	// Normalize normals.
		n2.Unit();
		normalReflect[nidx++] = (float)n2.x;
		normalReflect[nidx++] = (float)n2.y;
		normalReflect[nidx++] = (float)n2.z;
		normalReflect[nidx++] = (*ti).Reflectivity();

		texCoordIndex[tidx++] = (*ti).V2().m_Tx;
		texCoordIndex[tidx++] = (*ti).V2().m_Ty;
		texCoordIndex[tidx++] = (*ti).TextureIndex();
		texCoordIndex[tidx++] = (*ti).Opacity();

		texEffectAppl[eidx++] = (*ti).V2().EffectTx();
		texEffectAppl[eidx++] = (*ti).V2().EffectTy();
		texEffectAppl[eidx++] = (*ti).V2().EffectIntensity();
		texEffectAppl[eidx++] = (*ti).EffectAppl() ? 1 : 0;	// This has become an on/off switch.

		paintBuf[aidx++] = paintColor.redF();
		paintBuf[aidx++] = paintColor.greenF();
		paintBuf[aidx++] = paintColor.blueF();
		paintBuf[aidx++] = paintColor.alphaF();

		//if((*ti).V2().Glow().redF() > 0)
			//qDebug() << "red glow" << (*ti).V2().Glow().redF() << "at" << (*ti).V2().x << (*ti).V2().y << (*ti).V2().z;
		glowBuf[gidx++] = (*ti).V2().Glow().redF();
		glowBuf[gidx++] = (*ti).V2().Glow().greenF();
		glowBuf[gidx++] = (*ti).V2().Glow().blueF();
		glowBuf[gidx++] = (*ti).SelfIllumination();
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 6);

	SelectGL();

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), positionShine, usage);
	GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, nidx*sizeof(GLfloat), normalReflect, usage);
	GLf.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for texture coords, shininess and topFace and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, tidx*sizeof(GLfloat), texCoordIndex, usage);
	GLf.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for effect texture coords and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(3));
	GLf.glBufferData(GL_ARRAY_BUFFER, eidx*sizeof(GLfloat), texEffectAppl, usage);
	GLf.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for paint and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(4));
	GLf.glBufferData(GL_ARRAY_BUFFER, aidx*sizeof(GLfloat), paintBuf, usage);
	GLf.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for glow and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(5));
	GLf.glBufferData(GL_ARRAY_BUFFER, gidx*sizeof(GLfloat), glowBuf, usage);
	GLf.glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] positionShine;
	delete [] normalReflect;
	delete [] texCoordIndex;
	delete [] texEffectAppl;
	delete [] paintBuf;
	delete [] glowBuf;
}

void AuGLVAO::CreateForBody(AuOpenGLFunctions& GLf, const QList<AuTriBody>& tris, GLenum usage)
{
	//qDebug() << "AuGLVAO::CreateForBody tris" << tris.count();
	if(tris.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of triangles in the vertex array object.
	int vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	// These array buffers correspond literally to the three 'in vec3' variables 'normal', 'position' and 'texCoordShine' in
	// the vertex shader. You can cram up to 4 floats in each array by using vec4's as the in variables.
	GLfloat* position = new GLfloat[vertices*3];
	GLfloat* normal = new GLfloat[vertices*3];
	GLfloat* texCoordShineTopface = new GLfloat[vertices*4];
	GLfloat* boneIndex = new GLfloat[vertices*4];
	GLfloat* boneWeight = new GLfloat[vertices*4];

	unsigned int pidx = 0;
	unsigned int nidx = 0;
	unsigned int tidx = 0;
	unsigned int bi = 0;
	unsigned int bw = 0;
	for(QList<AuTriBody>::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
	{
		// Vertex 0
		position[pidx++] = (float)(*ti).m_V0.x;
		position[pidx++] = (float)(*ti).m_V0.y;
		position[pidx++] = (float)(*ti).m_V0.z;

		normal[nidx++] = (float)(*ti).m_V0.m_N.x;
		normal[nidx++] = (float)(*ti).m_V0.m_N.y;
		normal[nidx++] = (float)(*ti).m_V0.m_N.z;

		texCoordShineTopface[tidx++] = (*ti).m_V0.m_Tx;
		texCoordShineTopface[tidx++] = (*ti).m_V0.m_Ty;
		texCoordShineTopface[tidx++] = (*ti).m_V0.m_Shine;
		texCoordShineTopface[tidx++] = (*ti).m_V0.m_TopFace ? 1.f : 0.f;

		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V0.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V0.m_BoneWeight[3];

		// Vertex 1
		position[pidx++] = (float)(*ti).m_V1.x;
		position[pidx++] = (float)(*ti).m_V1.y;
		position[pidx++] = (float)(*ti).m_V1.z;

		normal[nidx++] = (float)(*ti).m_V1.m_N.x;
		normal[nidx++] = (float)(*ti).m_V1.m_N.y;
		normal[nidx++] = (float)(*ti).m_V1.m_N.z;

		texCoordShineTopface[tidx++] = (*ti).m_V1.m_Tx;
		texCoordShineTopface[tidx++] = (*ti).m_V1.m_Ty;
		texCoordShineTopface[tidx++] = (*ti).m_V1.m_Shine;
		texCoordShineTopface[tidx++] = (*ti).m_V1.m_TopFace ? 1.f : 0.f;

		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V1.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V1.m_BoneWeight[3];

		// Vertex 2
		position[pidx++] = (float)(*ti).m_V2.x;
		position[pidx++] = (float)(*ti).m_V2.y;
		position[pidx++] = (float)(*ti).m_V2.z;

		normal[nidx++] = (float)(*ti).m_V2.m_N.x;
		normal[nidx++] = (float)(*ti).m_V2.m_N.y;
		normal[nidx++] = (float)(*ti).m_V2.m_N.z;

		texCoordShineTopface[tidx++] = (*ti).m_V2.m_Tx;
		texCoordShineTopface[tidx++] = (*ti).m_V2.m_Ty;
		texCoordShineTopface[tidx++] = (*ti).m_V2.m_Shine;
		texCoordShineTopface[tidx++] = (*ti).m_V2.m_TopFace ? 1.f : 0.f;

		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[0];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[1];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[2];
		boneIndex[bi++] = (float)(*ti).m_V2.m_BoneIndex[3];

		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[0];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[1];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[2];
		boneWeight[bw++] = (*ti).m_V2.m_BoneWeight[3];
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 5);

	SelectGL();

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, nidx*sizeof(GLfloat), normal, usage);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for texture coords, shininess and topFace and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, tidx*sizeof(GLfloat), texCoordShineTopface, usage);
	GLf.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for bone indexes and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(3));
	GLf.glBufferData(GL_ARRAY_BUFFER, bi*sizeof(GLfloat), boneIndex, usage);
	GLf.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for bone weights and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(4));
	GLf.glBufferData(GL_ARRAY_BUFFER, bw*sizeof(GLfloat), boneWeight, usage);
	GLf.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] position;
	delete [] normal;
	delete [] texCoordShineTopface;
	delete [] boneIndex;
	delete [] boneWeight;
}

void AuGLVAO::CreateForModelTexFog(AuOpenGLFunctions& GLf, const QList<AuTriTexNS>& tris, GLenum usage)
{
	if(tris.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of triangles in the vertex array object.
	int vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	// These array buffers correspond literally to the three 'in vec3' variables 'normal', 'position' and 'texCoordShine' in
	// the vertex shader. You can cram up to 4 floats in each array by using vec4's as the in variables.
	GLfloat* normal = new GLfloat[vertices*3];
	GLfloat* position = new GLfloat[vertices*3];
	GLfloat* texCoordShine = new GLfloat[vertices*3];

	unsigned int nidx = 0;
	unsigned int pidx = 0;
	unsigned int tidx = 0;
	for(QList<AuTriTexNS>::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
	{
		// Vertex 0
		normal[nidx++] = (float)(*ti).m_V0.m_N.x;
		normal[nidx++] = (float)(*ti).m_V0.m_N.y;
		normal[nidx++] = (float)(*ti).m_V0.m_N.z;

		position[pidx++] = (float)(*ti).m_V0.x;
		position[pidx++] = (float)(*ti).m_V0.y;
		position[pidx++] = (float)(*ti).m_V0.z;

		texCoordShine[tidx++] = (float)(*ti).m_V0.m_Tx;
		texCoordShine[tidx++] = (float)(*ti).m_V0.m_Ty;
		texCoordShine[tidx++] = (float)(*ti).m_V0.m_Shine;
		//texCoordShine[tidx++] = (float)(*ti).m_Texture;

		// Vertex 1
		normal[nidx++] = (float)(*ti).m_V1.m_N.x;
		normal[nidx++] = (float)(*ti).m_V1.m_N.y;
		normal[nidx++] = (float)(*ti).m_V1.m_N.z;

		position[pidx++] = (float)(*ti).m_V1.x;
		position[pidx++] = (float)(*ti).m_V1.y;
		position[pidx++] = (float)(*ti).m_V1.z;

		texCoordShine[tidx++] = (float)(*ti).m_V1.m_Tx;
		texCoordShine[tidx++] = (float)(*ti).m_V1.m_Ty;
		texCoordShine[tidx++] = (float)(*ti).m_V1.m_Shine;
		//texCoordShine[tidx++] = (float)(*ti).m_Texture;

		// Vertex 2
		normal[nidx++] = (float)(*ti).m_V2.m_N.x;
		normal[nidx++] = (float)(*ti).m_V2.m_N.y;
		normal[nidx++] = (float)(*ti).m_V2.m_N.z;

		position[pidx++] = (float)(*ti).m_V2.x;
		position[pidx++] = (float)(*ti).m_V2.y;
		position[pidx++] = (float)(*ti).m_V2.z;

		texCoordShine[tidx++] = (float)(*ti).m_V2.m_Tx;
		texCoordShine[tidx++] = (float)(*ti).m_V2.m_Ty;
		texCoordShine[tidx++] = (float)(*ti).m_V2.m_Shine;
		//texCoordShine[tidx++] = (float)(*ti).m_Texture;
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 3);

	SelectGL();

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, nidx*sizeof(GLfloat), normal, usage);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for texture coords and shininess and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, tidx*sizeof(GLfloat), texCoordShine, usage);
	GLf.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] normal;
	delete [] position;
	delete [] texCoordShine;
}

void AuGLVAO::CreateForModel(AuOpenGLFunctions& GLf, const QList<AuTriTexNS>& tris, GLenum usage)
{
	if(tris.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of triangles in the vertex array object.
	int vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	// These array buffers correspond literally to the 'in vec4 normalShine' and 'in vec3 position' variables in
	// the vertex shader. You can cram up to 4 floats in each array by using vec4's as the in variables.
	GLfloat* normalShine = new GLfloat[vertices*4];
	GLfloat* position = new GLfloat[vertices*3];

	unsigned int nidx = 0;
	unsigned int pidx = 0;
	for(QList<AuTriTexNS>::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
	{
		// Vertex 0
		normalShine[nidx++] = (float)(*ti).m_V0.m_N.x;
		normalShine[nidx++] = (float)(*ti).m_V0.m_N.y;
		normalShine[nidx++] = (float)(*ti).m_V0.m_N.z;
		normalShine[nidx++] = (float)(*ti).m_V0.m_Shine;

		position[pidx++] = (float)(*ti).m_V0.x;
		position[pidx++] = (float)(*ti).m_V0.y;
		position[pidx++] = (float)(*ti).m_V0.z;

		// Vertex 1
		normalShine[nidx++] = (float)(*ti).m_V1.m_N.x;
		normalShine[nidx++] = (float)(*ti).m_V1.m_N.y;
		normalShine[nidx++] = (float)(*ti).m_V1.m_N.z;
		normalShine[nidx++] = (float)(*ti).m_V1.m_Shine;

		position[pidx++] = (float)(*ti).m_V1.x;
		position[pidx++] = (float)(*ti).m_V1.y;
		position[pidx++] = (float)(*ti).m_V1.z;

		// Vertex 2
		normalShine[nidx++] = (float)(*ti).m_V2.m_N.x;
		normalShine[nidx++] = (float)(*ti).m_V2.m_N.y;
		normalShine[nidx++] = (float)(*ti).m_V2.m_N.z;
		normalShine[nidx++] = (float)(*ti).m_V2.m_Shine;

		position[pidx++] = (float)(*ti).m_V2.x;
		position[pidx++] = (float)(*ti).m_V2.y;
		position[pidx++] = (float)(*ti).m_V2.z;
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 2);

	SelectGL();

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, nidx*sizeof(GLfloat), normalShine, usage);
	GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] normalShine;
	delete [] position;
}

void AuGLVAO::CreateForTexture(AuOpenGLFunctions& GLf, const QList<AuTriTex>& tris, GLenum usage)
{
	if(tris.isEmpty())
		return;

	if(VAO())
		return;

	m_GLf = &GLf;

	// Store count of triangles in the vertex array object.
	int vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	// These array buffers correspond literally to the 'in vec2 texCoord' and 'in vec3 position' variables in
	// the vertex shader. You can cram up to 4 floats in each array by using vec4's as the in variables.
	GLfloat* texCoord = new GLfloat[vertices*2];
	GLfloat* position = new GLfloat[vertices*3];

	unsigned int tidx = 0;
	unsigned int pidx = 0;
	for(QList<AuTriTex>::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
	{
		// Vertex 0
		texCoord[tidx++] = (float)(*ti).m_V0.m_Tx;
		texCoord[tidx++] = (float)(*ti).m_V0.m_Ty;

		position[pidx++] = (float)(*ti).m_V0.x;
		position[pidx++] = (float)(*ti).m_V0.y;
		position[pidx++] = (float)(*ti).m_V0.z;

		// Vertex 1
		texCoord[tidx++] = (float)(*ti).m_V1.m_Tx;
		texCoord[tidx++] = (float)(*ti).m_V1.m_Ty;

		position[pidx++] = (float)(*ti).m_V1.x;
		position[pidx++] = (float)(*ti).m_V1.y;
		position[pidx++] = (float)(*ti).m_V1.z;

		// Vertex 2
		texCoord[tidx++] = (float)(*ti).m_V2.m_Tx;
		texCoord[tidx++] = (float)(*ti).m_V2.m_Ty;

		position[pidx++] = (float)(*ti).m_V2.x;
		position[pidx++] = (float)(*ti).m_V2.y;
		position[pidx++] = (float)(*ti).m_V2.z;
	}

	// Generate vertex array object with a buffer for each array.
	CreateGL(GLf, vertices, 2);

	SelectGL();

	// bind buffer for positions and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, usage);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for tex coords and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, tidx*sizeof(GLfloat), texCoord, usage);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	DeselectGL();

	// OpenGL has the data now. I can discard my local buffers.
	delete [] texCoord;
	delete [] position;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScVAO::Create(AuOpenGLFunctions& GLf, const AuScTriRenderList& tris, GLenum usage)
{
	if(tris.isEmpty())
		return;
	if(!m_VAOs.empty())
		return;
	for(quint8 effect=EFF_None; effect < EFF_Count; effect++)
	{
		AuGLVAO vao;
		vao.CreateForSc(GLf, tris, usage, effect);
		if(vao.VAO())
			m_VAOs.insert(effect, vao);
	}
}

void AuScVAO::DeleteGL()
{
	for(QHash<quint8, AuGLVAO>::iterator vi = m_VAOs.begin(); vi != m_VAOs.end(); vi++)
		(*vi).DeleteGL();
	m_VAOs.clear();
}

void AuScVAO::DrawSc(const AuGLSLScBase& shader, const GLuint effectTex[EFF_Count], int hullEffAppl)
{
	for(QHash<quint8, AuGLVAO>::iterator vi = m_VAOs.begin(); vi != m_VAOs.end(); vi++)
	{
		// When the effect is none, no effect texture is needed by the shader.
		if(effectTex[vi.key()])
		{
			if(vi.key() == EFF_Hull)
				shader.SetEffectTexture(effectTex[vi.key()], hullEffAppl);	// Hull effect varies between lit/glow.
			else
				shader.SetEffectTexture(effectTex[vi.key()], 2);			// All other surface effects glow.
		}
		(*vi).SelectGL();
		(*vi).DrawGL();
		(*vi).DeselectGL();
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGLSL::AuGLSL()
{
	m_Animal = new AuGLSLAnimal;
	m_AnimalFog = new AuGLSLAnimalFog;
	m_AnimalGlobe = new AuGLSLAnimalGlobe;
	m_AnimalGlobeFog = new AuGLSLAnimalGlobeFog;
	m_AnimalRingworld = new AuGLSLAnimalRingworld;
	m_AnimalRingworldFog = new AuGLSLAnimalRingworldFog;
	m_AnimalClothing = new AuGLSLAnimalClothing;
	m_AnimalClothingFog = new AuGLSLAnimalClothingFog;
	m_AnimalClothingGlobe = new AuGLSLAnimalClothingGlobe;
	m_AnimalClothingGlobeFog = new AuGLSLAnimalClothingGlobeFog;
	m_AnimalClothingRingworld = new AuGLSLAnimalClothingRingworld;
	m_AnimalClothingRingworldFog = new AuGLSLAnimalClothingRingworldFog;
	m_AnimalHair = new AuGLSLAnimalHair;
	m_AnimalHairFog = new AuGLSLAnimalHairFog;
	m_AnimalHairGlobe = new AuGLSLAnimalHairGlobe;
	m_AnimalHairGlobeFog = new AuGLSLAnimalHairGlobeFog;
	m_AnimalHairRingworld = new AuGLSLAnimalHairRingworld;
	m_AnimalHairRingworldFog = new AuGLSLAnimalHairRingworldFog;
	m_AquaticFlora = new AuGLSLAquaticFlora;
	m_AquaticFloraRingworld = new AuGLSLAquaticFloraRingworld;
	m_Atmosphere = new AuGLSLAtmosphere;
	m_AtmosphereRingworld = new AuGLSLAtmosphereRingworld;
	m_BlackHole = new AuGLSLBlackHole;
	m_BlackHoleSphere = new AuGLSLBlackHoleSphere;
	m_Body = new AuGLSLBody;
	m_BodyFog = new AuGLSLBodyFog;
	m_BodyFogGlobe = new AuGLSLBodyFogGlobe;
	m_BodyGlobe = new AuGLSLBodyGlobe;
	m_BodyFogRingworld = new AuGLSLBodyFogRingworld;
	m_BodyRingworld = new AuGLSLBodyRingworld;
	m_Cloud = new AuGLSLCloud;
	m_CloudRingworld = new AuGLSLCloudRingworld;
	m_Color = new AuGLSLColor;
	m_FarGlobe = new AuGLSLFarGlobe;
	m_FarGlobeLit = new AuGLSLFarGlobeLit;
	m_FarGlobeLitClouds = new AuGLSLFarGlobeLitClouds;
	m_FarGlobeLitLava = new AuGLSLFarGlobeLitLava;
	m_FarGlobeLitLavaClouds = new AuGLSLFarGlobeLitLavaClouds;
	m_FarRingworldLit = new AuGLSLFarRingworldLit;
	m_FarRingworldLitClouds = new AuGLSLFarRingworldLitClouds;
	m_FarStar = new AuGLSLFarStar;
	m_Flag = new AuGLSLFlag;
	m_Flora = new AuGLSLFlora;
	m_FloraRingworld = new AuGLSLFloraRingworld;
	m_GalaxyCloud = new AuGLSLGalaxyCloud;
	m_GalaxyStar = new AuGLSLGalaxyStar;
	m_GasGiant = new AuGLSLGasGiant;
	m_GasGiantLayer = new AuGLSLGasGiantLayer;
	m_Globe = new AuGLSLGlobe;
	m_GlobeFlora = new AuGLSLGlobeFlora;
	m_GlobeLit = new AuGLSLGlobeLit;
	m_GlobeLitFlora = new AuGLSLGlobeLitFlora;
	m_Lava = new AuGLSLLava;
	m_LavaNoBump = new AuGLSLLavaNoBump;
	m_Line = new AuGLSLLine;
	m_LineFog = new AuGLSLLineFog;
	m_Model = new AuGLSLModel;
	m_ModelTex = new AuGLSLModelTex;
	m_ModelTexColor = new AuGLSLModelTexColor;
	m_ModelTexFog = new AuGLSLModelTexFog;
	m_ModelTexFogGlobe = new AuGLSLModelTexFogGlobe;
	m_ModelTexFogRingworld = new AuGLSLModelTexFogRingworld;
	m_NearGlobe = new AuGLSLNearGlobe;
	m_NearGlobeFog = new AuGLSLNearGlobeFog;
	m_NearGlobeLava = new AuGLSLNearGlobeLava;
	m_NearGlobeLavaFog = new AuGLSLNearGlobeLavaFog;
	m_NearRingworld = new AuGLSLNearRingworld;
	m_NearRingworldFog = new AuGLSLNearRingworldFog;
	m_NearRingworldGlow = new AuGLSLNearRingworldGlow;
	m_NearRingworldGlowFog = new AuGLSLNearRingworldGlowFog;
	m_NearStar = new AuGLSLNearStar;
	m_Nebula = new AuGLSLNebula;
	m_PhotonSphere = new AuGLSLPhotonSphere;
	m_Rain = new AuGLSLRain;
	m_RainRingworld = new AuGLSLRainRingworld;
	m_RingworldFlag = new AuGLSLRingworldFlag;
	m_RingworldFlora = new AuGLSLRingworldFlora;
	m_RingworldHull = new AuGLSLRingworldHull;
	m_RingworldHullFog = new AuGLSLRingworldHullFog;
	m_RingworldLitFlora = new AuGLSLRingworldLitFlora;
	m_RingworldStatue = new AuGLSLRingworldStatue;
	m_RingworldTerrainLitFlora = new AuGLSLRingworldTerrainLitFlora;
	m_RingworldTerrainStatue = new AuGLSLRingworldTerrainStatue;
	m_Rock = new AuGLSLRock;
	m_RockRingworld = new AuGLSLRockRingworld;
	m_Sc = new AuGLSLSc;
	m_ScFog = new AuGLSLScFog;
	m_ScGlobe = new AuGLSLScGlobe;
	m_ScGlobeFog = new AuGLSLScGlobeFog;
	m_ScRingworld = new AuGLSLScRingworld;
	m_ScRingworldFog = new AuGLSLScRingworldFog;
	m_ShadowSquare = new AuGLSLShadowSquare;
	m_Shield = new AuGLSLShield;
	m_ShieldHits = new AuGLSLShieldHits;
	m_SkyBox = new AuGLSLSkyBox;
	m_Snow = new AuGLSLSnow;
	m_SnowRingworld = new AuGLSLSnowRingworld;
	m_Splat = new AuGLSLSplat;
	m_SpriteActualSize = new AuGLSLSpriteActualSize;
	m_SpriteSize = new AuGLSLSpriteSize;
	m_SpriteSizeColor = new AuGLSLSpriteSizeColor;
	m_TerrainLit = new AuGLSLTerrainLit;
	m_TerrainLitFlora = new AuGLSLTerrainLitFlora;
	m_TexBar = new AuGLSLTexBar;
	m_TexOffset = new AuGLSLTexOffset;
	m_Texture = new AuGLSLTexture;
	m_WarpTunnel = new AuGLSLWarpTunnel;
	m_Water = new AuGLSLWater;
	m_WaterNoBump = new AuGLSLWaterNoBump;
	m_WaterNoBumpNoHole = new AuGLSLWaterNoBumpNoHole;
	m_WaterNoHole = new AuGLSLWaterNoHole;
	m_WaterRingworld = new AuGLSLWaterRingworld;
	m_WaterRingworldNoBump = new AuGLSLWaterRingworldNoBump;
	m_WaterRingworldNoBumpNoHole = new AuGLSLWaterRingworldNoBumpNoHole;
	m_WaterRingworldNoHole = new AuGLSLWaterRingworldNoHole;
}

AuGLSL::~AuGLSL()
{
	delete m_Animal;
	delete m_AnimalFog;
	delete m_AnimalGlobe;
	delete m_AnimalGlobeFog;
	delete m_AnimalRingworld;
	delete m_AnimalRingworldFog;
	delete m_AnimalClothing;
	delete m_AnimalClothingFog;
	delete m_AnimalClothingGlobe;
	delete m_AnimalClothingGlobeFog;
	delete m_AnimalClothingRingworld;
	delete m_AnimalClothingRingworldFog;
	delete m_AnimalHair;
	delete m_AnimalHairFog;
	delete m_AnimalHairGlobe;
	delete m_AnimalHairGlobeFog;
	delete m_AnimalHairRingworld;
	delete m_AnimalHairRingworldFog;
	delete m_AquaticFlora;
	delete m_AquaticFloraRingworld;
	delete m_Atmosphere;
	delete m_AtmosphereRingworld;
	delete m_BlackHole;
	delete m_BlackHoleSphere;
	delete m_Body;
	delete m_BodyFog;
	delete m_BodyFogGlobe;
	delete m_BodyGlobe;
	delete m_BodyFogRingworld;
	delete m_BodyRingworld;
	delete m_Cloud;
	delete m_CloudRingworld;
	delete m_Color;
	delete m_FarGlobe;
	delete m_FarGlobeLit;
	delete m_FarGlobeLitClouds;
	delete m_FarGlobeLitLava;
	delete m_FarGlobeLitLavaClouds;
	delete m_FarRingworldLit;
	delete m_FarRingworldLitClouds;
	delete m_FarStar;
	delete m_Flag;
	delete m_Flora;
	delete m_FloraRingworld;
	delete m_GalaxyCloud;
	delete m_GalaxyStar;
	delete m_GasGiant;
	delete m_GasGiantLayer;
	delete m_Globe;
	delete m_GlobeFlora;
	delete m_GlobeLit;
	delete m_GlobeLitFlora;
	delete m_Lava;
	delete m_LavaNoBump;
	delete m_Line;
	delete m_LineFog;
	delete m_Model;
	delete m_ModelTex;
	delete m_ModelTexColor;
	delete m_ModelTexFog;
	delete m_ModelTexFogGlobe;
	delete m_ModelTexFogRingworld;
	delete m_NearGlobe;
	delete m_NearGlobeFog;
	delete m_NearGlobeLava;
	delete m_NearGlobeLavaFog;
	delete m_NearRingworld;
	delete m_NearRingworldFog;
	delete m_NearRingworldGlow;
	delete m_NearRingworldGlowFog;
	delete m_NearStar;
	delete m_Nebula;
	delete m_PhotonSphere;
	delete m_Rain;
	delete m_RainRingworld;
	delete m_RingworldFlag;
	delete m_RingworldFlora;
	delete m_RingworldHull;
	delete m_RingworldHullFog;
	delete m_RingworldLitFlora;
	delete m_RingworldStatue;
	delete m_RingworldTerrainLitFlora;
	delete m_RingworldTerrainStatue;
	delete m_Rock;
	delete m_RockRingworld;
	delete m_Sc;
	delete m_ScFog;
	delete m_ScGlobe;
	delete m_ScGlobeFog;
	delete m_ScRingworld;
	delete m_ScRingworldFog;
	delete m_ShadowSquare;
	delete m_Shield;
	delete m_ShieldHits;
	delete m_SkyBox;
	delete m_Snow;
	delete m_SnowRingworld;
	delete m_Splat;
	delete m_SpriteActualSize;
	delete m_SpriteSize;
	delete m_SpriteSizeColor;
	delete m_TerrainLit;
	delete m_TerrainLitFlora;
	delete m_TexBar;
	delete m_TexOffset;
	delete m_Texture;
	delete m_WarpTunnel;
	delete m_Water;
	delete m_WaterNoBump;
	delete m_WaterNoBumpNoHole;
	delete m_WaterNoHole;
	delete m_WaterRingworld;
	delete m_WaterRingworldNoBump;
	delete m_WaterRingworldNoBumpNoHole;
	delete m_WaterRingworldNoHole;
}

void AuGLSL::Initialize(AuOpenGLFunctions& GLf, unsigned int maxLights)
{
	m_GLf = &GLf;
	m_MaxLights = maxLights;

	// Load program source from our compressed hash archive. This is not due to the size of the source code, which is not large. 
	// The source code is compressed so it is not easily visible in the final executable to the idly curious.
	Hashpack::ReadFiles(":glsl.har", s_GLSLProgramSrc);
	if(s_DebugGLSL)
		qDebug() << "GLSL shader source loaded from hash archive" << s_GLSLProgramSrc.size() << "files";
	if(s_GLSLProgramSrc.empty())
		return;

	AuGLSLProgramOpt opt(maxLights);
	m_Animal->Build(GLf, opt);
	m_AnimalFog->Build(GLf, opt);
	m_AnimalGlobe->Build(GLf, opt);
	m_AnimalGlobeFog->Build(GLf, opt);
	m_AnimalRingworld->Build(GLf, opt);
	m_AnimalRingworldFog->Build(GLf, opt);
	m_AnimalClothing->Build(GLf, opt);
	m_AnimalClothingFog->Build(GLf, opt);
	m_AnimalClothingGlobe->Build(GLf, opt);
	m_AnimalClothingGlobeFog->Build(GLf, opt);
	m_AnimalClothingRingworld->Build(GLf, opt);
	m_AnimalClothingRingworldFog->Build(GLf, opt);
	m_AnimalHair->Build(GLf, opt);
	m_AnimalHairFog->Build(GLf, opt);
	m_AnimalHairGlobe->Build(GLf, opt);
	m_AnimalHairGlobeFog->Build(GLf, opt);
	m_AnimalHairRingworld->Build(GLf, opt);
	m_AnimalHairRingworldFog->Build(GLf, opt);
	m_AquaticFlora->Build(GLf, opt);
	m_AquaticFloraRingworld->Build(GLf, opt);
	m_Atmosphere->Build(GLf, opt);
	m_AtmosphereRingworld->Build(GLf, opt);
	m_BlackHole->Build(GLf, opt);
	m_BlackHoleSphere->Build(GLf, opt);
	m_Body->Build(GLf, opt);
	m_BodyFog->Build(GLf, opt);
	m_BodyFogGlobe->Build(GLf, opt);
	m_BodyGlobe->Build(GLf, opt);
	m_BodyFogRingworld->Build(GLf, opt);
	m_BodyRingworld->Build(GLf, opt);
	m_Cloud->Build(GLf, opt);
	m_CloudRingworld->Build(GLf, opt);
	m_Color->Build(GLf, opt);
	m_FarGlobe->Build(GLf, opt);
	m_FarGlobeLit->Build(GLf, opt);
	m_FarGlobeLitClouds->Build(GLf, opt);
	m_FarGlobeLitLava->Build(GLf, opt);
	m_FarGlobeLitLavaClouds->Build(GLf, opt);
	m_FarRingworldLit->Build(GLf, opt);
	m_FarRingworldLitClouds->Build(GLf, opt);
	m_FarStar->Build(GLf, opt);
	m_Flag->Build(GLf, opt);
	m_Flora->Build(GLf, opt);
	m_FloraRingworld->Build(GLf, opt);
	m_GalaxyCloud->Build(GLf, opt);
	m_GalaxyStar->Build(GLf, opt);
	m_GasGiant->Build(GLf, opt);
	m_GasGiantLayer->Build(GLf, opt);
	m_Globe->Build(GLf, opt);
	m_GlobeFlora->Build(GLf, opt);
	m_GlobeLit->Build(GLf, opt);
	m_GlobeLitFlora->Build(GLf, opt);
	m_Lava->Build(GLf, opt);
	m_LavaNoBump->Build(GLf, opt);
	m_Line->Build(GLf, opt);
	m_LineFog->Build(GLf, opt);
	m_Model->Build(GLf, opt);
	m_ModelTex->Build(GLf, opt);
	m_ModelTexColor->Build(GLf, opt);
	m_ModelTexFog->Build(GLf, opt);
	m_ModelTexFogGlobe->Build(GLf, opt);
	m_ModelTexFogRingworld->Build(GLf, opt);
	m_NearGlobe->Build(GLf, opt);
	m_NearGlobeFog->Build(GLf, opt);
	m_NearGlobeLava->Build(GLf, opt);
	m_NearGlobeLavaFog->Build(GLf, opt);
	m_NearRingworld->Build(GLf, opt);
	m_NearRingworldFog->Build(GLf, opt);
	m_NearRingworldGlow->Build(GLf, opt);
	m_NearRingworldGlowFog->Build(GLf, opt);
	m_NearStar->Build(GLf, opt);
	m_Nebula->Build(GLf, opt);
	m_PhotonSphere->Build(GLf, opt);
	m_Rain->Build(GLf, opt);
	m_RainRingworld->Build(GLf, opt);
	m_RingworldFlag->Build(GLf, opt);
	m_RingworldFlora->Build(GLf, opt);
	m_RingworldHull->Build(GLf, opt);
	m_RingworldHullFog->Build(GLf, opt);
	m_RingworldLitFlora->Build(GLf, opt);
	m_RingworldStatue->Build(GLf, opt);
	m_RingworldTerrainLitFlora->Build(GLf, opt);
	m_RingworldTerrainStatue->Build(GLf, opt);
	m_Rock->Build(GLf, opt);
	m_RockRingworld->Build(GLf, opt);
	m_Sc->Build(GLf, opt);
	m_ScFog->Build(GLf, opt);
	m_ScGlobe->Build(GLf, opt);
	m_ScGlobeFog->Build(GLf, opt);
	m_ScRingworld->Build(GLf, opt);
	m_ScRingworldFog->Build(GLf, opt);
	m_ShadowSquare->Build(GLf, opt);
	m_Shield->Build(GLf, opt);
	m_ShieldHits->Build(GLf, opt);
	m_SkyBox->Build(GLf, opt);
	m_Snow->Build(GLf, opt);
	m_SnowRingworld->Build(GLf, opt);
	m_Splat->Build(GLf, opt);
	m_SpriteActualSize->Build(GLf, opt);
	m_SpriteSize->Build(GLf, opt);
	m_SpriteSizeColor->Build(GLf, opt);
	m_TerrainLit->Build(GLf, opt);
	m_TerrainLitFlora->Build(GLf, opt);
	m_TexBar->Build(GLf, opt);
	m_TexOffset->Build(GLf, opt);
	m_Texture->Build(GLf, opt);
	m_WarpTunnel->Build(GLf, opt);
	m_Water->Build(GLf, opt);
	m_WaterNoBump->Build(GLf, opt);
	m_WaterNoBumpNoHole->Build(GLf, opt);
	m_WaterNoHole->Build(GLf, opt);
	m_WaterRingworld->Build(GLf, opt);
	m_WaterRingworldNoBump->Build(GLf, opt);
	m_WaterRingworldNoBumpNoHole->Build(GLf, opt);
	m_WaterRingworldNoHole->Build(GLf, opt);

	// Deselect shaders from the GL context to restore default OpenGL behavior.
	DeselectProgram();

	// The .glsl program source code hash table is no longer needed.
	s_GLSLProgramSrc.clear();
}

void AuGLSL::InvalidateLights()
{
	m_Animal->InvalidateLights();
	m_AnimalFog->InvalidateLights();
	m_AnimalGlobe->InvalidateLights();
	m_AnimalGlobeFog->InvalidateLights();
	m_AnimalRingworld->InvalidateLights();
	m_AnimalRingworldFog->InvalidateLights();
	m_AnimalClothing->InvalidateLights();
	m_AnimalClothingFog->InvalidateLights();
	m_AnimalClothingGlobe->InvalidateLights();
	m_AnimalClothingGlobeFog->InvalidateLights();
	m_AnimalClothingRingworld->InvalidateLights();
	m_AnimalClothingRingworldFog->InvalidateLights();
	m_AnimalHair->InvalidateLights();
	m_AnimalHairFog->InvalidateLights();
	m_AnimalHairGlobe->InvalidateLights();
	m_AnimalHairGlobeFog->InvalidateLights();
	m_AnimalHairRingworld->InvalidateLights();
	m_AnimalHairRingworldFog->InvalidateLights();
	m_AquaticFlora->InvalidateLights();
	m_AquaticFloraRingworld->InvalidateLights();
	//m_Atmosphere->InvalidateLights();
	//m_AtmosphereRingworld->InvalidateLights();
	//m_BlackHole->InvalidateLights();
	//m_BlackHoleSphere->InvalidateLights();
	m_Body->InvalidateLights();
	m_BodyFog->InvalidateLights();
	m_BodyFogGlobe->InvalidateLights();
	m_BodyGlobe->InvalidateLights();
	m_BodyFogRingworld->InvalidateLights();
	m_BodyRingworld->InvalidateLights();
	//m_Cloud->InvalidateLights();
	//m_CloudRingworld->InvalidateLights();
	//m_Color->InvalidateLights();
	//m_FarGlobe->InvalidateLights();
	//m_FarGlobeLit->InvalidateLights();
	//m_FarGlobeLitClouds->InvalidateLights();
	//m_FarGlobeLitLava->InvalidateLights();
	//m_FarGlobeLitLavaClouds->InvalidateLights();
	//m_FarRingworldLit->InvalidateLights();
	//m_FarRingworldLitClouds->InvalidateLights();
	//m_FarStar->InvalidateLights();
	m_Flag->InvalidateLights();
	m_Flora->InvalidateLights();
	m_FloraRingworld->InvalidateLights();
	//m_GalaxyCloud->InvalidateLights();
	//m_GalaxyStar->InvalidateLights();
	//m_GasGiant->InvalidateLights();
	//m_GasGiantLayer->InvalidateLights();
	//m_Globe->InvalidateLights();
	//m_GlobeFlora->InvalidateLights();
	//m_GlobeLit->InvalidateLights();
	//m_GlobeLitFlora->InvalidateLights();
	//m_Lava->InvalidateLights();
	//m_LavaNoBump->InvalidateLights();
	//m_Line->InvalidateLights();
	//m_LineFog->InvalidateLights();
	m_Model->InvalidateLights();
	m_ModelTex->InvalidateLights();
	m_ModelTexColor->InvalidateLights();
	m_ModelTexFog->InvalidateLights();
	m_ModelTexFogGlobe->InvalidateLights();
	m_ModelTexFogRingworld->InvalidateLights();
	//m_NearGlobe->InvalidateLights();
	//m_NearGlobeFog->InvalidateLights();
	//m_NearGlobeLava->InvalidateLights();
	//m_NearGlobeLavaFog->InvalidateLights();
	//m_NearRingworld->InvalidateLights();
	//m_NearRingworldFog->InvalidateLights();
	//m_NearRingworldGlow->InvalidateLights();
	//m_NearRingworldGlowFog->InvalidateLights();
	//m_NearStar->InvalidateLights();
	//m_Nebula->InvalidateLights();
	//m_PhotonSphere->InvalidateLights();
	//m_Rain->InvalidateLights();
	//m_RainRingworld->InvalidateLights();
	m_RingworldFlag->InvalidateLights();
	//m_RingworldFlora->InvalidateLights();
	//m_RingworldHull->InvalidateLights();
	//m_RingworldHullFog->InvalidateLights();
	//m_RingworldLitFlora->InvalidateLights();
	//m_RingworldStatue->InvalidateLights();
	//m_RingworldTerrainLitFlora->InvalidateLights();
	//m_RingworldTerrainStatue->InvalidateLights();
	m_Rock->InvalidateLights();
	m_RockRingworld->InvalidateLights();
	m_Sc->InvalidateLights();
	m_ScFog->InvalidateLights();
	m_ScGlobe->InvalidateLights();
	m_ScGlobeFog->InvalidateLights();
	m_ScRingworld->InvalidateLights();
	m_ScRingworldFog->InvalidateLights();
	//m_ShadowSquare->InvalidateLights();
	//m_Shield->InvalidateLights();
	//m_ShieldHits->InvalidateLights();
	//m_SkyBox->InvalidateLights();
	//m_Snow->InvalidateLights();
	//m_SnowRingworld->InvalidateLights();
	//m_Splat->InvalidateLights();
	//m_SpriteActualSize->InvalidateLights();
	//m_SpriteSize->InvalidateLights();
	//m_SpriteSizeColor->InvalidateLights();
	//m_TerrainLit->InvalidateLights();
	//m_TerrainLitFlora->InvalidateLights();
	//m_TexBar->InvalidateLights();
	//m_TexOffset->InvalidateLights();
	//m_Texture->InvalidateLights();
	//m_WarpTunnel->InvalidateLights();
	//m_Water->InvalidateLights();
	//m_WaterNoBump->InvalidateLights();
	//m_WaterNoBumpNoHole->InvalidateLights();
	//m_WaterNoHole->InvalidateLights();
	//m_WaterRingworld->InvalidateLights();
	//m_WaterRingworldNoBump->InvalidateLights();
	//m_WaterRingworldNoBumpNoHole->InvalidateLights();
	//m_WaterRingworldNoHole->InvalidateLights();
}

void AuGLSL::InvalidateSuns()
{
	m_Animal->InvalidateSuns();
	m_AnimalFog->InvalidateSuns();
	m_AnimalGlobe->InvalidateSuns();
	m_AnimalGlobeFog->InvalidateSuns();
	m_AnimalRingworld->InvalidateSuns();
	m_AnimalRingworldFog->InvalidateSuns();
	m_AnimalClothing->InvalidateSuns();
	m_AnimalClothingFog->InvalidateSuns();
	m_AnimalClothingGlobe->InvalidateSuns();
	m_AnimalClothingGlobeFog->InvalidateSuns();
	m_AnimalClothingRingworld->InvalidateSuns();
	m_AnimalClothingRingworldFog->InvalidateSuns();
	m_AnimalHair->InvalidateSuns();
	m_AnimalHairFog->InvalidateSuns();
	m_AnimalHairGlobe->InvalidateSuns();
	m_AnimalHairGlobeFog->InvalidateSuns();
	m_AnimalHairRingworld->InvalidateSuns();
	m_AnimalHairRingworldFog->InvalidateSuns();
	m_AquaticFlora->InvalidateSuns();
	m_AquaticFloraRingworld->InvalidateSuns();
	//m_Atmosphere->InvalidateSuns();
	//m_AtmosphereRingworld->InvalidateSuns();
	//m_BlackHole->InvalidateSuns();
	//m_BlackHoleSphere->InvalidateSuns();
	m_Body->InvalidateSuns();
	m_BodyFog->InvalidateSuns();
	m_BodyFogGlobe->InvalidateSuns();
	m_BodyGlobe->InvalidateSuns();
	m_BodyFogRingworld->InvalidateSuns();
	m_BodyRingworld->InvalidateSuns();
	//m_Cloud->InvalidateSuns();
	//m_CloudRingworld->InvalidateSuns();
	//m_Color->InvalidateSuns();
	//m_FarGlobe->InvalidateSuns();
	//m_FarGlobeLit->InvalidateSuns();
	//m_FarGlobeLitClouds->InvalidateSuns();
	//m_FarGlobeLitLava->InvalidateSuns();
	//m_FarGlobeLitLavaClouds->InvalidateSuns();
	//m_FarRingworldLit->InvalidateSuns();
	//m_FarRingworldLitClouds->InvalidateSuns();
	//m_FarStar->InvalidateSuns();
	m_Flag->InvalidateSuns();
	m_Flora->InvalidateSuns();
	m_FloraRingworld->InvalidateSuns();
	//m_GalaxyCloud->InvalidateSuns();
	//m_GalaxyStar->InvalidateSuns();
	//m_GasGiant->InvalidateSuns();
	//m_GasGiantLayer->InvalidateSuns();
	//m_Globe->InvalidateSuns();
	//m_GlobeFlora->InvalidateSuns();
	//m_GlobeLit->InvalidateSuns();
	//m_GlobeLitFlora->InvalidateSuns();
	//m_Lava->InvalidateSuns();
	//m_LavaNoBump->InvalidateSuns();
	//m_Line->InvalidateSuns();
	//m_LineFog->InvalidateSuns();
	m_Model->InvalidateSuns();
	m_ModelTex->InvalidateSuns();
	m_ModelTexColor->InvalidateSuns();
	m_ModelTexFog->InvalidateSuns();
	m_ModelTexFogGlobe->InvalidateSuns();
	m_ModelTexFogRingworld->InvalidateSuns();
	//m_NearGlobe->InvalidateSuns();
	//m_NearGlobeFog->InvalidateSuns();
	//m_NearGlobeLava->InvalidateSuns();
	//m_NearGlobeLavaFog->InvalidateSuns();
	//m_NearRingworld->InvalidateSuns();
	//m_NearRingworldFog->InvalidateSuns();
	//m_NearRingworldGlow->InvalidateSuns();
	//m_NearRingworldGlowFog->InvalidateSuns();
	//m_NearStar->InvalidateSuns();
	//m_Nebula->InvalidateSuns();
	//m_PhotonSphere->InvalidateSuns();
	//m_Rain->InvalidateSuns();
	//m_RainRingworld->InvalidateSuns();
	m_RingworldFlag->InvalidateSuns();
	//m_RingworldFlora->InvalidateSuns();
	//m_RingworldHull->InvalidateSuns();
	//m_RingworldHullFog->InvalidateSuns();
	//m_RingworldLitFlora->InvalidateSuns();
	//m_RingworldStatue->InvalidateSuns();
	//m_RingworldTerrainLitFlora->InvalidateSuns();
	//m_RingworldTerrainStatue->InvalidateSuns();
	m_Rock->InvalidateSuns();
	m_RockRingworld->InvalidateSuns();
	m_Sc->InvalidateSuns();
	m_ScFog->InvalidateSuns();
	m_ScGlobe->InvalidateSuns();
	m_ScGlobeFog->InvalidateSuns();
	m_ScRingworld->InvalidateSuns();
	m_ScRingworldFog->InvalidateSuns();
	//m_ShadowSquare->InvalidateSuns();
	//m_Shield->InvalidateSuns();
	//m_ShieldHits->InvalidateSuns();
	//m_SkyBox->InvalidateSuns();
	//m_Snow->InvalidateSuns();
	//m_SnowRingworld->InvalidateSuns();
	//m_Splat->InvalidateSuns();
	//m_SpriteActualSize->InvalidateSuns();
	//m_SpriteSize->InvalidateSuns();
	//m_SpriteSizeColor->InvalidateSuns();
	//m_TerrainLit->InvalidateSuns();
	//m_TerrainLitFlora->InvalidateSuns();
	//m_TexBar->InvalidateSuns();
	//m_TexOffset->InvalidateSuns();
	//m_Texture->InvalidateSuns();
	//m_WarpTunnel->InvalidateSuns();
	//m_Water->InvalidateSuns();
	//m_WaterNoBump->InvalidateSuns();
	//m_WaterNoBumpNoHole->InvalidateSuns();
	//m_WaterNoHole->InvalidateSuns();
	//m_WaterRingworld->InvalidateSuns();
	//m_WaterRingworldNoBump->InvalidateSuns();
	//m_WaterRingworldNoBumpNoHole->InvalidateSuns();
	//m_WaterRingworldNoHole->InvalidateSuns();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGLSLShader::AuGLSLShader(AuOpenGLFunctions& GLf, GLenum shaderType, const QString& shaderName, unsigned int maxLights)
{
	if(s_DebugGLSL)
		qDebug() << " AuGLSLShader::AuGLSLShader" << shaderName;
	Q_ASSERT(shaderType != GL_VERTEX_SHADER || maxLights != 0);	// Vertex shader requires a non-zero maxLights.

	// Locate the source code in the hash table loaded from the glsl.har file.
	QHash<QString, QByteArray>::const_iterator psi = s_GLSLProgramSrc.find(QString("%1.glsl").arg(shaderName));
	if(psi == s_GLSLProgramSrc.end() || (*psi).isEmpty())
	{
		// Abort if a program source fails to load.
		QString title = QObject::tr("%1 Load Error").arg(shaderType == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
		AuGLSLErr mb(title,
				QObject::tr("<html>Failed to load shader %1.<br>"
					"This is a critical error. The program must abort.</html>").arg(shaderName));
		mb.exec();
		//exit(-1);
		m_Shader = 0;
		return;
	}

	// Create an OpenGL shader object.
	m_Shader = GLf.glCreateShader(shaderType);

	// Read the source code from the hash table of compressed source code.
	QByteArray src = AuUncompress(*psi, __FILE__, __LINE__,shaderName);

	// Change the version in the shader source to match our current OpenGL version.
	// All shader source is marked as '#version 150'.
	if(GLMajorVersion == 3 && GLMinorVersion == 0)
		src.replace(QByteArray("#version 150"), QByteArray("#version 130"));
	else if(GLMajorVersion == 3 && GLMinorVersion == 1)
		src.replace(QByteArray("#version 150"), QByteArray("#version 140"));
	//else if(GLMajorVersion == 3 && GLMinorVersion == 2)
		//src.replace(QByteArray("#version 150"), QByteArray("#version 150"));
	else if(GLMajorVersion == 3 && GLMinorVersion == 3)
		src.replace(QByteArray("#version 150"), QByteArray("#version 330"));
	else if(GLMajorVersion == 4 && GLMinorVersion == 0)
		src.replace(QByteArray("#version 150"), QByteArray("#version 400"));
	else if(GLMajorVersion == 4 && GLMinorVersion == 1)
		src.replace(QByteArray("#version 150"), QByteArray("#version 410"));
	else if(GLMajorVersion == 4 && GLMinorVersion == 2)
		src.replace(QByteArray("#version 150"), QByteArray("#version 420"));
	else if(GLMajorVersion == 4 && GLMinorVersion == 3)
		src.replace(QByteArray("#version 150"), QByteArray("#version 430"));
	else if(GLMajorVersion == 4 && GLMinorVersion == 4)
		src.replace(QByteArray("#version 150"), QByteArray("#version 440"));
	else if(GLMajorVersion == 4 && GLMinorVersion == 5)
		src.replace(QByteArray("#version 150"), QByteArray("#version 450"));

	// Resolve our special includes.
	if(src.contains("#SunLightGlobe"))
	{
		QHash<QString, QByteArray>::const_iterator lsi = s_GLSLProgramSrc.find(QString("H_SunLightGlobe.glsl"));
		if(lsi == s_GLSLProgramSrc.end() || (*lsi).isEmpty())
		{
			// Abort if we fail to load the light code.
			QString title = QObject::tr("%1 Load Error").arg(shaderType == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
			AuGLSLErr mb(title,
					QObject::tr("<html>Failed to include H_SunLightGlobe.glsl in shader %1.<br>"
						"This is a critical error. The program must abort.</html>").arg(shaderName));
			mb.exec();
			//exit(-1);
			m_Shader = 0;
			return;
		}
		QByteArray light = AuUncompress(*lsi, __FILE__, __LINE__,shaderName);
		src.replace(QByteArray("#SunLightGlobe"), light);
	}
	else if(src.contains("#SunLightRingworld"))
	{
		QHash<QString, QByteArray>::const_iterator lsi = s_GLSLProgramSrc.find(QString("H_SunLightRingworld.glsl"));
		if(lsi == s_GLSLProgramSrc.end() || (*lsi).isEmpty())
		{
			// Abort if we fail to load the light code.
			QString title = QObject::tr("%1 Load Error").arg(shaderType == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
			AuGLSLErr mb(title,
					QObject::tr("<html>Failed to include LightRingworld.glsl in shader %1.<br>"
						"This is a critical error. The program must abort.</html>").arg(shaderName));
			mb.exec();
			//exit(-1);
			m_Shader = 0;
			return;
		}
		QByteArray light = AuUncompress(*lsi, __FILE__, __LINE__,shaderName);
		src.replace(QByteArray("#SunLightRingworld"), light);
	}
	else if(src.contains("#SunLight"))
	{
		QHash<QString, QByteArray>::const_iterator lsi = s_GLSLProgramSrc.find(QString("H_SunLight.glsl"));
		if(lsi == s_GLSLProgramSrc.end() || (*lsi).isEmpty())
		{
			// Abort if we fail to load the light code.
			QString title = QObject::tr("%1 Load Error").arg(shaderType == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
			AuGLSLErr mb(title,
					QObject::tr("<html>Failed to include H_SunLight.glsl in shader %1.<br>"
						"This is a critical error. The program must abort.</html>").arg(shaderName));
			mb.exec();
			//exit(-1);
			m_Shader = 0;
			return;
		}
		QByteArray light = AuUncompress(*lsi, __FILE__, __LINE__,shaderName);
		src.replace(QByteArray("#SunLight"), light);
	}

	QString maxl = QString("%1").arg(maxLights);
	src.replace(QByteArray("MAX_LIGHTS"), maxl.toUtf8());

	QString maxab = QString("%1").arg(MAX_ANIMAL_BONES);
	src.replace(QByteArray("MAX_ANIMAL_BONES"), maxab.toUtf8());

	// Load the source code into the shader object.
	const char* srcData = src.constData();
	GLf.glShaderSource(m_Shader, 1, &srcData, NULL);

	// Compile the shader source code.
	GLf.glCompileShader(m_Shader);
	GLint compiled;
	GLf.glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &compiled);
#if !defined(QT_NO_DEBUG)
	DumpInfoLog(GLf);
#endif
	if(!compiled)
	{
		// Abort if a program fails to compile.
		QString title = QObject::tr("%1 Compile Error").arg(shaderType == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
		AuGLSLErr mb(title,
				QObject::tr("<html>Failed to compile shader %1.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%2</html>").arg(shaderName).arg(GetInfoLog(GLf)));
		mb.exec();
		//exit(-1);
	}
}

QString AuGLSLShader::GetInfoLog(AuOpenGLFunctions& GLf)
{
	QString info;
	GLint infologLen = 0;
	GLf.glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &infologLen);
	if(infologLen > 2)
	{
		GLchar* infoLog = new GLchar[infologLen];
		if(!infoLog)
		{
			qWarning() << "error: AuGLSLShader::GetInfoLog Unable to allocate info log buffer" << infologLen << "chars";
		}
		else
		{
			int charsWritten = 0;
			GLf.glGetShaderInfoLog(m_Shader, infologLen, &charsWritten, infoLog);
			info = QString(infoLog);
			info.replace("\n", "<br>");
			delete [] infoLog;
		}
	}
	return info;
}

void AuGLSLShader::DumpInfoLog(AuOpenGLFunctions& GLf)
{
	if(!s_DebugGLSL)
		return;

	GLint infologLen = 0;
	GLf.glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &infologLen);
	if(infologLen > 2)
	{
		GLchar* infoLog = new GLchar[infologLen];
		if(!infoLog)
		{
			qWarning() << "error: AuGLSLShader::DumpInfoLog Unable to allocate info log buffer" << infologLen << "chars";
		}
		else
		{
			int charsWritten = 0;
			GLf.glGetShaderInfoLog(m_Shader, infologLen, &charsWritten, infoLog);
			qDebug() << "Shader Info Log, Length " << infologLen << '\n' << infoLog;
			delete [] infoLog;
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGLSLProgram::AuGLSLProgram()
{
	m_Program = 0;
	m_GLf = 0;
}

AuGLSLProgram::~AuGLSLProgram()
{
}

void AuGLSLProgram::Select()
{
	m_GLf->glUseProgram(m_Program);	// Select this shader program.
}

void AuGLSLProgram::Deselect()
{
	m_GLf->glUseProgram(0);
	OutputGLError("AuGLSLProgram::Deselect");
}

GLint AuGLSLProgram::GetAttribLoc(const QString& attrib) const
{
	GLint loc = m_GLf->glGetAttribLocation(m_Program, attrib.toLocal8Bit());
#if !defined(QT_NO_DEBUG)
	if(loc == -1)
		qWarning() << "error: AuGLSLProgram::GetAttribLoc No such attrib" << attrib;
#endif
	return loc;
}

GLint AuGLSLProgram::GetUniformLoc(const QString& uniform) const
{
	GLint loc = m_GLf->glGetUniformLocation(m_Program, uniform.toLocal8Bit());
#if !defined(QT_NO_DEBUG)
	if(loc == -1)
		qWarning() << "error: AuGLSLProgram::GetUniformLoc No such uniform" << uniform;
#endif
	return loc;
}

QString AuGLSLProgram::GetProgramInfoLog() const
{
	QString info;
	GLint infologLen = 0;
	m_GLf->glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &infologLen);
	if(infologLen > 2)
	{
		GLchar* infoLog = new GLchar[infologLen];
		if(!infoLog)
		{
			qWarning() << "error: AuGLSLProgram::GetProgramInfoLog Unable to allocate info log buffer" << infologLen << "chars";
		}
		else
		{
			int charsWritten = 0;
			m_GLf->glGetProgramInfoLog(m_Program, infologLen, &charsWritten, infoLog);
			info = QString(infoLog);
			info.replace("\n", "<br>");
			delete [] infoLog;
		}
	}
	return info;
}

void AuGLSLProgram::DumpProgramInfoLog() const
{
	if(!s_DebugGLSL)
		return;

	GLint infologLen = 0;
	m_GLf->glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &infologLen);
	if(infologLen > 2)
	{
		GLchar* infoLog = new GLchar[infologLen];
		if(!infoLog)
		{
			qWarning() << "error: AuGLSLProgram::DumpProgramInfoLog Unable to allocate info log buffer" << infologLen << "chars";
		}
		else
		{
			int charsWritten = 0;
			m_GLf->glGetProgramInfoLog(m_Program, infologLen, &charsWritten, infoLog);
			qDebug() << "Program Info Log, Length" << infologLen << '\n' << infoLog;
			delete [] infoLog;
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLColor::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLColor::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Color.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Color.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "color");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Color"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLLine::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLLine::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Line.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Line.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Line"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_pmv = GetUniformLoc("pmv");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLLineFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLLineFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "LineFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "LineFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("LineFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogParm = GetUniformLoc("fogParm");
	m_pmv = GetUniformLoc("pmv");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearStar::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearStar::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearStar.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearStar.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearStar"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_opacity = GetUniformLoc("opacity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarStar::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarStar::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarStar.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarStar.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarStar"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_size = GetUniformLoc("size");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGalaxyStar::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGalaxyStar::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GalaxyStar.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "GalaxyStar.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GalaxyStar"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_intenDist = GetUniformLoc("intenDist");
	m_intenRanges = GetUniformLoc("intenRanges");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_size = GetUniformLoc("size");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGalaxyCloud::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGalaxyCloud::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GalaxyCloud.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "GalaxyCloud.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GalaxyCloud"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNebula::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNebula::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Nebula.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Nebula.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");
	m_GLf->glBindAttribLocation(m_Program, 2, "color");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Nebula"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSkyBox::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSkyBox::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "SkyBox.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "SkyBox.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("SkyBox"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
}

void AuGLSLSkyBox::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearGlobe.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearGlobeFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearGlobeFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearGlobeFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearGlobeFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearGlobeFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearRingworldFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearRingworldFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearRingworldFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearRingworldFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearRingworldFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearGlobeLava::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearGlobeLava::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearGlobeLava.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearGlobeLava.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearGlobeLava"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearGlobeLavaFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearGlobeLavaFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearGlobeLavaFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearGlobeLavaFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearGlobeLavaFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearRingworldGlow::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearRingworldGlow::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearRingworldGlow.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearRingworldGlow.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearRingworldGlow"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLNearRingworldGlowFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLNearRingworldGlowFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "NearRingworldGlowFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "NearRingworldGlowFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("NearRingworldGlowFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Globe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("Globe.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoordAlt");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Globe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	//m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_pmv = GetUniformLoc("pmv");
	m_texLookup = GetUniformLoc("texLookup");
	//m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGlobeFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGlobeFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GlobeFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("GlobeFlora.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoordAltVeg");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GlobeFlora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_texLookup = GetUniformLoc("texLookup");
	m_texSnow = GetUniformLoc("texSnow");
	m_texGrass = GetUniformLoc("texGrass");

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("RingworldFlora.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoordAltVeg");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldFlora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_texLookup = GetUniformLoc("texLookup");
	m_texSnow = GetUniformLoc("texSnow");
	m_texGrass = GetUniformLoc("texGrass");

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGlobeLit::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGlobeLit::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GlobeLit.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("GlobeLit.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAlt");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GlobeLit"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_clipCos = GetUniformLoc("clipCos");
	m_clipUp = GetUniformLoc("clipUp");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_texLookup = GetUniformLoc("texLookup");
	m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGlobeLitFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGlobeLitFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GlobeLitFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("GlobeLitFlora.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAltVeg");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GlobeLitFlora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_clipCos = GetUniformLoc("clipCos");
	m_clipUp = GetUniformLoc("clipUp");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_texLookup = GetUniformLoc("texLookup");
	m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");
	m_texGrass = GetUniformLoc("texGrass");

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldLitFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldLitFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldLitFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("RingworldLitFlora.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAltVeg");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldLitFlora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_clipCenter = GetUniformLoc("clipCenter");
	m_clipDist = GetUniformLoc("clipDist");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_texLookup = GetUniformLoc("texLookup");
	m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");
	m_texGrass = GetUniformLoc("texGrass");

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLTerrainLit::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLTerrainLit::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "TerrainLit.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("TerrainLit.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAltVeg");
	m_GLf->glBindAttribLocation(m_Program, 3, "paveCity");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("TerrainLit"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texLookup = GetUniformLoc("texLookup");
	m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");

	for(int pave=0; pave<7; pave++)
		m_texPavement[pave] = GetUniformLoc(QString("texPavement[%1]").arg(pave));

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));

	m_ambient = GetUniformLoc("light.ambient");
	m_cityAmbient = GetUniformLoc("light.cityAmbient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLTerrainLit::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldNoise);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
	texUnit++;

	for(int ter=0; ter<5; ter++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
		texUnit++;
	}

	for(int pave=0; pave<7; pave++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldPavement[pave]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLTerrainLitFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLTerrainLitFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "TerrainLitFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("TerrainLitFlora.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAltVeg");
	m_GLf->glBindAttribLocation(m_Program, 3, "paveCity");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("TerrainLitFlora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texLookup = GetUniformLoc("texLookup");
	m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");
	m_texGrass = GetUniformLoc("texGrass");

	for(int pave=0; pave<7; pave++)
		m_texPavement[pave] = GetUniformLoc(QString("texPavement[%1]").arg(pave));

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));

	m_ambient = GetUniformLoc("light.ambient");
	m_cityAmbient = GetUniformLoc("light.cityAmbient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLTerrainLitFlora::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldNoise);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldGrass);
	texUnit++;

	for(int ter=0; ter<5; ter++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
		texUnit++;
	}

	for(int pave=0; pave<7; pave++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldPavement[pave]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldTerrainLitFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldTerrainLitFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldTerrainLitFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("RingworldTerrainLitFlora.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAltVeg");
	m_GLf->glBindAttribLocation(m_Program, 3, "paveCity");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldTerrainLitFlora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_altNoiseFactor = GetUniformLoc("altNoiseFactor");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texLookup = GetUniformLoc("texLookup");
	m_texNoise = GetUniformLoc("texNoise");
	m_texSnow = GetUniformLoc("texSnow");
	m_texGrass = GetUniformLoc("texGrass");

	for(int pave=0; pave<7; pave++)
		m_texPavement[pave] = GetUniformLoc(QString("texPavement[%1]").arg(pave));

	for(int ter=0; ter<5; ter++)
		m_texTerrain[ter] = GetUniformLoc(QString("texTerrain[%1]").arg(ter));

	m_ambient = GetUniformLoc("light.ambient");
	m_cityAmbient = GetUniformLoc("light.cityAmbient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLRingworldTerrainLitFlora::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldNoise);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldGrass);
	texUnit++;

	for(int ter=0; ter<5; ter++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
		texUnit++;
	}

	for(int pave=0; pave<7; pave++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldPavement[pave]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWater::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWater::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Water.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Water.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");
	m_GLf->glBindAttribLocation(m_Program, 2, "tangent");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Water"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_clipCos = GetUniformLoc("clipCos");
	m_clipUp = GetUniformLoc("clipUp");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texBump = GetUniformLoc("texBump");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWater::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");
	m_GLf->glBindAttribLocation(m_Program, 3, "tangent");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_clipCenter = GetUniformLoc("clipCenter");
	m_clipDist = GetUniformLoc("clipDist");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texBump = GetUniformLoc("texBump");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterRingworld::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterNoBump::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterNoBump::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterNoBump.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterNoBump.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterNoBump"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_clipCos = GetUniformLoc("clipCos");
	m_clipUp = GetUniformLoc("clipUp");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterNoBump::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterRingworldNoBump::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterRingworldNoBump::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterRingworldNoBump.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterRingworldNoBump.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterRingworldNoBump"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_clipCenter = GetUniformLoc("clipCenter");
	m_clipDist = GetUniformLoc("clipDist");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterRingworldNoBump::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterNoHole::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterNoHole::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterNoHole.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterNoHole.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");
	m_GLf->glBindAttribLocation(m_Program, 2, "tangent");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterNoHole"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texBump = GetUniformLoc("texBump");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterNoHole::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterRingworldNoHole::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterRingworldNoHole::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterRingworldNoHole.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterRingworldNoHole.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");
	m_GLf->glBindAttribLocation(m_Program, 3, "tangent");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterRingworldNoHole"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texBump = GetUniformLoc("texBump");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterRingworldNoHole::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterNoBumpNoHole::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterNoBumpNoHole::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterNoBumpNoHole.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterNoBumpNoHole.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterNoBumpNoHole"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterNoBumpNoHole::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLWaterRingworldNoBumpNoHole::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWaterRingworldNoBumpNoHole::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WaterRingworldNoBumpNoHole.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WaterRingworldNoBumpNoHole.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WaterRingworldNoBumpNoHole"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_opacity = GetUniformLoc("opacity");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLWaterRingworldNoBumpNoHole::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLLava::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLLava::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Lava.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Lava.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");
	m_GLf->glBindAttribLocation(m_Program, 2, "tangent");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Lava"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texBump = GetUniformLoc("texBump");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLLava::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAtmosphere::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAtmosphere::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Atmosphere.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Atmosphere.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Atmosphere"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmAlpha = GetUniformLoc("atmAlpha");
	m_atmColor = GetUniformLoc("atmColor");
	m_atmRadius = GetUniformLoc("atmRadius");
	m_pmv = GetUniformLoc("pmv");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAtmosphereRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAtmosphereRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AtmosphereRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AtmosphereRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AtmosphereRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmAlpha = GetUniformLoc("atmAlpha");
	m_atmAltMSL = GetUniformLoc("atmAltMSL");
	m_atmColor = GetUniformLoc("atmColor");
	m_pmv = GetUniformLoc("pmv");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLCloud::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLCloud::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Cloud.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Cloud.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Cloud"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_cloudColor = GetUniformLoc("cloudColor");
	m_cloudLayerRadius = GetUniformLoc("cloudLayerRadius");
	m_cloudLiftMax = GetUniformLoc("cloudLiftMax");
	m_cloudSunset = GetUniformLoc("cloudSunset");
	m_maxWx = GetUniformLoc("maxWx");
	m_minWx = GetUniformLoc("minWx");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_texWx = GetUniformLoc("texWx");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunSunsetColor[sun] = GetUniformLoc(QString("light.sun%1.sunsetColor").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLCloudRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLCloudRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "CloudRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "CloudRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("CloudRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_cloudColor = GetUniformLoc("cloudColor");
	m_cloudLayerRadius = GetUniformLoc("cloudLayerRadius");
	m_cloudLiftMax = GetUniformLoc("cloudLiftMax");
	//m_cloudSunset = GetUniformLoc("cloudSunset");
	m_maxWx = GetUniformLoc("maxWx");
	m_minWx = GetUniformLoc("minWx");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_texWx = GetUniformLoc("texWx");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarGlobe.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_atmosphereRadius = GetUniformLoc("atmosphereRadius");
	m_lowMaxWx = GetUniformLoc("lowMaxWx");
	m_lowMinWx = GetUniformLoc("lowMinWx");
	m_lowTexWx = GetUniformLoc("lowTexWx");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarGlobeLit::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarGlobeLit::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarGlobeLit.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarGlobeLit.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarGlobeLit"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_atmosphereRadius = GetUniformLoc("atmosphereRadius");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarGlobeLitClouds::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarGlobeLitClouds::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarGlobeLit.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarGlobeLitClouds.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarGlobeLitClouds"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_atmosphereRadius = GetUniformLoc("atmosphereRadius");
	m_lowMaxWx = GetUniformLoc("lowMaxWx");
	m_lowMinWx = GetUniformLoc("lowMinWx");
	m_lowTexWx = GetUniformLoc("lowTexWx");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarGlobeLitLava::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarGlobeLitLava::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarGlobeLitLava.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarGlobeLitLava.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarGlobeLitLava"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_atmosphereRadius = GetUniformLoc("atmosphereRadius");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarGlobeLitLavaClouds::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarGlobeLitLavaClouds::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarGlobeLitLava.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarGlobeLitLavaClouds.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarGlobeLitLavaClouds"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_atmosphereRadius = GetUniformLoc("atmosphereRadius");
	m_lowMaxWx = GetUniformLoc("lowMaxWx");
	m_lowMinWx = GetUniformLoc("lowMinWx");
	m_lowTexWx = GetUniformLoc("lowTexWx");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarRingworldLit::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarRingworldLit::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarRingworldLit.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarRingworldLit.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarRingworldLit"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFarRingworldLitClouds::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFarRingworldLitClouds::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FarRingworldLit.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FarRingworldLitClouds.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FarRingworldLitClouds"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmosphereColor = GetUniformLoc("atmosphereColor");
	m_lowMaxWx = GetUniformLoc("lowMaxWx");
	m_lowMinWx = GetUniformLoc("lowMinWx");
	m_lowTexWx = GetUniformLoc("lowTexWx");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_texPlanet = GetUniformLoc("texPlanet");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Flora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Flora.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordMovement");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Flora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_plantM = GetUniformLoc("plantM");
	m_tex = GetUniformLoc("tex");
	m_vForce = GetUniformLoc("vForce");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLFlora::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFloraRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFloraRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "FloraRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "FloraRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordMovement");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FloraRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_plantM = GetUniformLoc("plantM");
	m_tex = GetUniformLoc("tex");
	m_vForce = GetUniformLoc("vForce");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLFloraRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAquaticFlora::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAquaticFlora::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AquaticFlora.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AquaticFlora.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordMovement");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Flora"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_plantM = GetUniformLoc("plantM");
	m_tex = GetUniformLoc("tex");
	m_vForce = GetUniformLoc("vForce");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAquaticFlora::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAquaticFloraRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAquaticFloraRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AquaticFloraRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AquaticFloraRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordMovement");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("FloraRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_plantM = GetUniformLoc("plantM");
	m_tex = GetUniformLoc("tex");
	m_vForce = GetUniformLoc("vForce");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAquaticFloraRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRock::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRock::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Rock.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Rock.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Rock"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_rockM = GetUniformLoc("rockM");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLRock::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRockRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRockRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RockRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RockRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RockRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_rockM = GetUniformLoc("rockM");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLRockRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSpriteActualSize::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSpriteActualSize::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "SpriteActualSize.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "SpriteActualSize.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("SpriteActualSize"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSpriteSize::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSpriteSize::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "SpriteSize.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "SpriteSize.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("SpriteSize"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_pmv = GetUniformLoc("pmv");
	m_size = GetUniformLoc("size");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSpriteSizeColor::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSpriteSizeColor::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "SpriteSizeColor.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "SpriteSizeColor.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "color");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("SpriteSizeColor"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLFlag::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLFlag::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Flag.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Flag.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordWave");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Flag"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texFlag = GetUniformLoc("texFlag");
	m_wind = GetUniformLoc("wind");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLFlag::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldFlag::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldFlag::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldFlag.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RingworldFlag.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordWave");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldFlag"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texFlag = GetUniformLoc("texFlag");
	m_wind = GetUniformLoc("wind");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLRingworldFlag::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLShadowSquare::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLShadowSquare::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ShadowSquare.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ShadowSquare.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ShadowSquare"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldHull::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldHull::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldHull.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RingworldHull.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldHull"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldHullFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldHullFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldHullFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RingworldHullFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldHullFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldStatue::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldStatue::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldStatue.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RingworldStatue.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldStatue"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRingworldTerrainStatue::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRingworldTerrainStatue::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RingworldTerrainStatue.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RingworldTerrainStatue.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RingworldTerrainStatue"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRain::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRain::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Rain.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Rain.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Rain"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");

	m_ambient = GetUniformLoc("light.ambient");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLRainRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLRainRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "RainRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "RainRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("RainRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_pmv = GetUniformLoc("pmv");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSnow::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSnow::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Snow.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Snow.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Snow"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_offset = GetUniformLoc("offset");
	m_p = GetUniformLoc("p");
	m_size = GetUniformLoc("size");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLSnow::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSnowRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSnowRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "SnowRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "SnowRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("SnowRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_offset = GetUniformLoc("offset");
	m_p = GetUniformLoc("p");
	m_size = GetUniformLoc("size");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLSnowRingworld::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBlackHole::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBlackHole::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BlackHole.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "BlackHole.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BlackHole"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_rotSky = GetUniformLoc("rotSky");
	m_tex = GetUniformLoc("tex");
	m_texSky = GetUniformLoc("texSky");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBlackHoleSphere::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBlackHoleSphere::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BlackHoleSphere.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "BlackHoleSphere.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BlackHoleSphere"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLPhotonSphere::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLPhotonSphere::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "PhotonSphere.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "PhotonSphere.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("PhotonSphere"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLModelTexColor::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLModelTexColor::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ModelTexColor.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ModelTexColor.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ModelTexColor"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_glow = GetUniformLoc("glow");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texPaletteHigh = GetUniformLoc("texPaletteHigh");
	m_texPaletteLow = GetUniformLoc("texPaletteLow");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLModelTexColor::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLModelTexFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLModelTexFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ModelTexFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ModelTexFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ModelTexFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLModelTexFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLModelTexFogGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLModelTexFogGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ModelTexFogGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ModelTexFogGlobe.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ModelTexFogGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLModelTexFogGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLModelTexFogRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLModelTexFogRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ModelTexFogRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ModelTexFogRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ModelTexFogRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLModelTexFogRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLModelTex::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLModelTex::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ModelTex.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ModelTex.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ModelTex"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLModelTex::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLModel::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLModel::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Model.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Model.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normalShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Model"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_glow = GetUniformLoc("glow");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLModel::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLTexture::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLTexture::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Texture.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Texture.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Texture"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
}

void AuGLSLTexture::Select()
{
	AuGLSLProgram::Select();

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLTexBar::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLTexBar::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "TexBar.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "TexBar.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("TexBar"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_colorLeft = GetUniformLoc("colorLeft");
	m_colorRight = GetUniformLoc("colorRight");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");
	m_threshold = GetUniformLoc("threshold");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLTexOffset::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLTexOffset::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "TexOffset.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "TexOffset.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("TexOffset"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_discardAlpha = GetUniformLoc("discardAlpha");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
void AuGLSLWarpTunnel::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLWarpTunnel::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "WarpTunnel.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "WarpTunnel.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("WarpTunnel"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_pmv = GetUniformLoc("pmv");
	m_rotationAngle = GetUniformLoc("rotationAngle");
	m_texSky = GetUniformLoc("texSky");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLShield::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLShield::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Shield.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Shield.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Shield"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_btr = GetUniformLoc("btr");
	m_btrScram = GetUniformLoc("btrScram");
	m_color = GetUniformLoc("color");
	m_fbl = GetUniformLoc("fbl");
	m_fblScram = GetUniformLoc("fblScram");
	m_pmv = GetUniformLoc("pmv");
	m_scale = GetUniformLoc("scale");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");
	m_texScram = GetUniformLoc("texScram");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLShieldHits::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLShieldHits::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ShieldHits.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "ShieldHits.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ShieldHits"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_btr = GetUniformLoc("btr");
	m_btrScram = GetUniformLoc("btrScram");
	m_color = GetUniformLoc("color");
	m_fbl = GetUniformLoc("fbl");
	m_fblScram = GetUniformLoc("fblScram");
	for(int h=0; h<MAX_SHIELD_HIT_EFFECTS; h++)
	{
		m_hitDur[h] = GetUniformLoc(QString("hitDur[%1]").arg(h));
		m_hitPos[h] = GetUniformLoc(QString("hitPos[%1]").arg(h));
	}
	m_pmv = GetUniformLoc("pmv");
	m_scale = GetUniformLoc("scale");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");
	m_texScram = GetUniformLoc("texScram");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGasGiant::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGasGiant::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GasGiant.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "GasGiant.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShine");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GasGiant"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmColor = GetUniformLoc("atmColor");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLGasGiantLayer::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLGasGiantLayer::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "GasGiantLayer.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "GasGiantLayer.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "normal");
	m_GLf->glBindAttribLocation(m_Program, 1, "position");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("GasGiantLayer"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_atmColor = GetUniformLoc("atmColor");
	m_atmRadius = GetUniformLoc("atmRadius");
	m_offWx = GetUniformLoc("offWx");
	m_pmv = GetUniformLoc("pmv");
	m_tex = GetUniformLoc("tex");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSplat::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSplat::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Splat.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Splat.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Splat"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_color = GetUniformLoc("color");
	m_pmv = GetUniformLoc("pmv");
	m_splatScale = GetUniformLoc("splatScale");
	m_tex = GetUniformLoc("tex");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimal::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimal::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Animal.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("Animal.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordTopBottomIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDetailBlendIn");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Animal"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_bottomSkinColor = GetUniformLoc("bottomSkinColor");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texSkinTop = GetUniformLoc("texSkinTop");
	m_texSkinBottom = GetUniformLoc("texSkinBottom");
	m_texSkinDetail = GetUniformLoc("texSkinDetail");
	m_topSkinColor = GetUniformLoc("topSkinColor");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimal::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinBottom);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinDetail);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinTop);
	texUnit++;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordTopBottomIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDetailBlendIn");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_bottomSkinColor = GetUniformLoc("bottomSkinColor");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texSkinTop = GetUniformLoc("texSkinTop");
	m_texSkinBottom = GetUniformLoc("texSkinBottom");
	m_texSkinDetail = GetUniformLoc("texSkinDetail");
	m_topSkinColor = GetUniformLoc("topSkinColor");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Fog
	float fogR, fogG, fogB, fogDensity;
	re.CurrentFog(fogR, fogG, fogB, fogDensity);
	SetFog(fogDensity, fogR, fogG, fogB);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinBottom);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinDetail);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinTop);
	texUnit++;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalGlobe.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordTopBottomIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDetailBlendIn");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_bottomSkinColor = GetUniformLoc("bottomSkinColor");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texSkinTop = GetUniformLoc("texSkinTop");
	m_texSkinBottom = GetUniformLoc("texSkinBottom");
	m_texSkinDetail = GetUniformLoc("texSkinDetail");
	m_topSkinColor = GetUniformLoc("topSkinColor");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinBottom);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinDetail);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinTop);
	texUnit++;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalGlobeFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalGlobeFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalGlobeFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalGlobeFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordTopBottomIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDetailBlendIn");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalGlobeFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_bottomSkinColor = GetUniformLoc("bottomSkinColor");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texSkinTop = GetUniformLoc("texSkinTop");
	m_texSkinBottom = GetUniformLoc("texSkinBottom");
	m_texSkinDetail = GetUniformLoc("texSkinDetail");
	m_topSkinColor = GetUniformLoc("topSkinColor");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalGlobeFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Fog
	float fogR, fogG, fogB, fogDensity;
	re.CurrentFog(fogR, fogG, fogB, fogDensity);
	SetFog(fogDensity, fogR, fogG, fogB);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinBottom);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinDetail);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinTop);
	texUnit++;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalRingworld.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordTopBottomIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDetailBlendIn");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_bottomSkinColor = GetUniformLoc("bottomSkinColor");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texSkinTop = GetUniformLoc("texSkinTop");
	m_texSkinBottom = GetUniformLoc("texSkinBottom");
	m_texSkinDetail = GetUniformLoc("texSkinDetail");
	m_topSkinColor = GetUniformLoc("topSkinColor");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinBottom);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinDetail);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinTop);
	texUnit++;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalRingworldFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalRingworldFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalRingworldFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalRingworldFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordTopBottomIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDetailBlendIn");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalRingworldFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_bottomSkinColor = GetUniformLoc("bottomSkinColor");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texSkinTop = GetUniformLoc("texSkinTop");
	m_texSkinBottom = GetUniformLoc("texSkinBottom");
	m_texSkinDetail = GetUniformLoc("texSkinDetail");
	m_topSkinColor = GetUniformLoc("topSkinColor");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalRingworldFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Fog
	float fogR, fogG, fogB, fogDensity;
	re.CurrentFog(fogR, fogG, fogB, fogDensity);
	SetFog(fogDensity, fogR, fogG, fogB);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinBottom);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinDetail);
	texUnit++;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSkinTop);
	texUnit++;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalClothing::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalClothing::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalClothing.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalClothing.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordClothing");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDecalBlendIn");
	m_GLf->glBindAttribLocation(m_Program, 6, "dyeColorAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalClothing"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothingDecal = GetUniformLoc("texClothingDecal");
	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_clothColor[c] = GetUniformLoc(QString("clothColor[%1]").arg(c));
		m_texClothing[c] = GetUniformLoc(QString("texClothing[%1]").arg(c));
	}

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalClothing::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothingDecal);
	texUnit++;

	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothing[c]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalClothingFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalClothingFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalClothingFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalClothingFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordClothing");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDecalBlendIn");
	m_GLf->glBindAttribLocation(m_Program, 6, "dyeColorAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalClothingFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothingDecal = GetUniformLoc("texClothingDecal");
	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_clothColor[c] = GetUniformLoc(QString("clothColor[%1]").arg(c));
		m_texClothing[c] = GetUniformLoc(QString("texClothing[%1]").arg(c));
	}

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalClothingFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothingDecal);
	texUnit++;

	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothing[c]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalClothingGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalClothingGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalClothingGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalClothingGlobe.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordClothing");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDecalBlendIn");
	m_GLf->glBindAttribLocation(m_Program, 6, "dyeColorAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalClothingGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothingDecal = GetUniformLoc("texClothingDecal");
	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_clothColor[c] = GetUniformLoc(QString("clothColor[%1]").arg(c));
		m_texClothing[c] = GetUniformLoc(QString("texClothing[%1]").arg(c));
	}

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalClothingGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothingDecal);
	texUnit++;

	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothing[c]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalClothingGlobeFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalClothingGlobeFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalClothingGlobeFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalClothingGlobeFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordClothing");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDecalBlendIn");
	m_GLf->glBindAttribLocation(m_Program, 6, "dyeColorAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalClothingGlobeFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothingDecal = GetUniformLoc("texClothingDecal");
	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_clothColor[c] = GetUniformLoc(QString("clothColor[%1]").arg(c));
		m_texClothing[c] = GetUniformLoc(QString("texClothing[%1]").arg(c));
	}

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalClothingGlobeFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothingDecal);
	texUnit++;

	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothing[c]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalClothingRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalClothingRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalClothingRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalClothingRingworld.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordClothing");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDecalBlendIn");
	m_GLf->glBindAttribLocation(m_Program, 6, "dyeColorAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalClothingRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothingDecal = GetUniformLoc("texClothingDecal");
	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_clothColor[c] = GetUniformLoc(QString("clothColor[%1]").arg(c));
		m_texClothing[c] = GetUniformLoc(QString("texClothing[%1]").arg(c));
	}

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalClothingRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothingDecal);
	texUnit++;

	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothing[c]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalClothingRingworldFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalClothingRingworldFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalClothingRingworldFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("AnimalClothingRingworldFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordClothing");
	m_GLf->glBindAttribLocation(m_Program, 5, "texCoordDecalBlendIn");
	m_GLf->glBindAttribLocation(m_Program, 6, "dyeColorAlpha");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalClothingRingworldFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothingDecal = GetUniformLoc("texClothingDecal");
	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_clothColor[c] = GetUniformLoc(QString("clothColor[%1]").arg(c));
		m_texClothing[c] = GetUniformLoc(QString("texClothing[%1]").arg(c));
	}

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalClothingRingworldFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	GLuint texUnit = GL_TEXTURE0;

	m_GLf->glActiveTexture(texUnit);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothingDecal);
	texUnit++;

	for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
	{
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldClothing[c]);
		texUnit++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalHair::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalHair::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalHair.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AnimalHair.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "hairColor");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalHair"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texHair = GetUniformLoc("texHair");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalHair::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldHair);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalHairFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalHairFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalHairFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AnimalHairFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "hairColor");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalHairFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texHair = GetUniformLoc("texHair");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalHairFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldHair);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalHairGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalHairGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalHairGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AnimalHairGlobe.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "hairColor");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalHairGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texHair = GetUniformLoc("texHair");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalHairGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldHair);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalHairGlobeFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalHairGlobeFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalHairGlobeFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AnimalHairGlobeFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "hairColor");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalHairGlobeFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texHair = GetUniformLoc("texHair");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalHairGlobeFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldHair);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalHairRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalHairRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalHairRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AnimalHairRingworld.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "hairColor");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalHairRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texHair = GetUniformLoc("texHair");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalHairRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldHair);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLAnimalHairRingworldFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLAnimalHairRingworldFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "AnimalHairRingworldFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "AnimalHairRingworldFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneWeight");
	m_GLf->glBindAttribLocation(m_Program, 4, "texCoordIn");
	m_GLf->glBindAttribLocation(m_Program, 5, "hairColor");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("AnimalHairRingworldFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_ANIMAL_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texHair = GetUniformLoc("texHair");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLAnimalHairRingworldFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Ambient light
	float r,g,b,a;
	re.CurrentAmbientLight(r, g, b, a);
	SetAmbientLight(r, g, b);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldHair);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBody::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBody::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Body.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Body.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShineTopface");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 4, "boneWeight");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Body"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothes = GetUniformLoc("texClothes");
	m_texClothesOffsetWear = GetUniformLoc("texClothesOffsetWear");
	m_texSkin = GetUniformLoc("texSkin");
	m_texSkinOffset = GetUniformLoc("texSkinOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLBody::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBodyFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBodyFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BodyFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "BodyFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShineTopface");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 4, "boneWeight");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BodyFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothes = GetUniformLoc("texClothes");
	m_texClothesOffsetWear = GetUniformLoc("texClothesOffsetWear");
	m_texSkin = GetUniformLoc("texSkin");
	m_texSkinOffset = GetUniformLoc("texSkinOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLBodyFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBodyGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBodyGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BodyGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Body.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShineTopface");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 4, "boneWeight");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BodyGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothes = GetUniformLoc("texClothes");
	m_texClothesOffsetWear = GetUniformLoc("texClothesOffsetWear");
	m_texSkin = GetUniformLoc("texSkin");
	m_texSkinOffset = GetUniformLoc("texSkinOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLBodyGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBodyFogGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBodyFogGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BodyFogGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "BodyFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShineTopface");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 4, "boneWeight");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BodyFogGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothes = GetUniformLoc("texClothes");
	m_texClothesOffsetWear = GetUniformLoc("texClothesOffsetWear");
	m_texSkin = GetUniformLoc("texSkin");
	m_texSkinOffset = GetUniformLoc("texSkinOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLBodyFogGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBodyRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBodyRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BodyRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "Body.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShineTopface");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 4, "boneWeight");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BodyRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothes = GetUniformLoc("texClothes");
	m_texClothesOffsetWear = GetUniformLoc("texClothesOffsetWear");
	m_texSkin = GetUniformLoc("texSkin");
	m_texSkinOffset = GetUniformLoc("texSkinOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLBodyRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLBodyFogRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLBodyFogRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "BodyFogRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "BodyFog.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "normal");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordShineTopface");
	m_GLf->glBindAttribLocation(m_Program, 3, "boneIndex");
	m_GLf->glBindAttribLocation(m_Program, 4, "boneWeight");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("BodyFogRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	for(int bm=0; bm<MAX_BONES; bm++)
		m_bM[bm] = GetUniformLoc(QString("bM[%1]").arg(bm));
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowAlpha = GetUniformLoc("glowAlpha");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texClothes = GetUniformLoc("texClothes");
	m_texClothesOffsetWear = GetUniformLoc("texClothesOffsetWear");
	m_texSkin = GetUniformLoc("texSkin");
	m_texSkinOffset = GetUniformLoc("texSkinOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLBodyFogRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLProgram::Select();
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec.x, sun.m_XVec.y, sun.m_XVec.z, sun.m_YVec.x, sun.m_YVec.y, sun.m_YVec.z);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	m_GLf->glActiveTexture(GL_TEXTURE1);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLSc::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLScBase::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLSc::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "Sc.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("Sc.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "texEffectAppl");
	m_GLf->glBindAttribLocation(m_Program, 4, "paint");
	m_GLf->glBindAttribLocation(m_Program, 5, "glow");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("Sc"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_effectApply = GetUniformLoc("effectApply");
	m_glowDiscard = GetUniformLoc("glowDiscard");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texEffect = GetUniformLoc("texEffect");
	for(int t=0; t<SCTEXI_Count; t++)
		m_texSurface[t] = GetUniformLoc(QString("texSurface[%1]").arg(t));
	m_vertAlpha = GetUniformLoc("vertAlpha");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLSc::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLScBase::Select(projection, re);
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	for(int t=0; t<SCTEXI_Count; t++)
	{
		m_GLf->glActiveTexture(GL_TEXTURE1 + t);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture[t]);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLScFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLScBase::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLScFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ScFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("ScFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "texEffectAppl");
	m_GLf->glBindAttribLocation(m_Program, 4, "paint");
	m_GLf->glBindAttribLocation(m_Program, 5, "glow");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ScFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_effectApply = GetUniformLoc("effectApply");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowDiscard = GetUniformLoc("glowDiscard");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texEffect = GetUniformLoc("texEffect");
	for(int t=0; t<SCTEXI_Count; t++)
		m_texSurface[t] = GetUniformLoc(QString("texSurface[%1]").arg(t));
	m_vertAlpha = GetUniformLoc("vertAlpha");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLScFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLScBase::Select(projection, re);
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	for(int t=0; t<SCTEXI_Count; t++)
	{
		m_GLf->glActiveTexture(GL_TEXTURE1 + t);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture[t]);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLScGlobe::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLScBase::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLScGlobe::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ScGlobe.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("ScGlobe.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "texEffectAppl");
	m_GLf->glBindAttribLocation(m_Program, 4, "paint");
	m_GLf->glBindAttribLocation(m_Program, 5, "glow");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ScGlobe"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_effectApply = GetUniformLoc("effectApply");
	m_glowDiscard = GetUniformLoc("glowDiscard");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texEffect = GetUniformLoc("texEffect");
	for(int t=0; t<SCTEXI_Count; t++)
		m_texSurface[t] = GetUniformLoc(QString("texSurface[%1]").arg(t));
	m_vertAlpha = GetUniformLoc("vertAlpha");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLScGlobe::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLScBase::Select(projection, re);
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	for(int t=0; t<SCTEXI_Count; t++)
	{
		m_GLf->glActiveTexture(GL_TEXTURE1 + t);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture[t]);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLScGlobeFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLScBase::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLScGlobeFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ScGlobeFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("ScGlobeFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "texEffectAppl");
	m_GLf->glBindAttribLocation(m_Program, 4, "paint");
	m_GLf->glBindAttribLocation(m_Program, 5, "glow");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ScGlobeFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_effectApply = GetUniformLoc("effectApply");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowDiscard = GetUniformLoc("glowDiscard");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texEffect = GetUniformLoc("texEffect");
	for(int t=0; t<SCTEXI_Count; t++)
		m_texSurface[t] = GetUniformLoc(QString("texSurface[%1]").arg(t));
	m_vertAlpha = GetUniformLoc("vertAlpha");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");
	m_globeP = GetUniformLoc("light.globeP");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLScGlobeFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLScBase::Select(projection, re);
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	for(int t=0; t<SCTEXI_Count; t++)
	{
		m_GLf->glActiveTexture(GL_TEXTURE1 + t);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture[t]);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLScRingworld::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLScBase::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLScRingworld::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ScRingworld.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("ScRingworld.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "texEffectAppl");
	m_GLf->glBindAttribLocation(m_Program, 4, "paint");
	m_GLf->glBindAttribLocation(m_Program, 5, "glow");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ScRingworld"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_effectApply = GetUniformLoc("effectApply");
	m_glowDiscard = GetUniformLoc("glowDiscard");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texEffect = GetUniformLoc("texEffect");
	for(int t=0; t<SCTEXI_Count; t++)
		m_texSurface[t] = GetUniformLoc(QString("texSurface[%1]").arg(t));
	m_vertAlpha = GetUniformLoc("vertAlpha");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLScRingworld::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLScBase::Select(projection, re);
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec, sun.m_YVec);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	for(int t=0; t<SCTEXI_Count; t++)
	{
		m_GLf->glActiveTexture(GL_TEXTURE1 + t);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture[t]);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLScRingworldFog::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLScBase::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLScRingworldFog::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "ScRingworldFog.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, QString("ScRingworldFog.f"));
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "positionShine");
	m_GLf->glBindAttribLocation(m_Program, 1, "normalReflect");
	m_GLf->glBindAttribLocation(m_Program, 2, "texCoordIndex");
	m_GLf->glBindAttribLocation(m_Program, 3, "texEffectAppl");
	m_GLf->glBindAttribLocation(m_Program, 4, "paint");
	m_GLf->glBindAttribLocation(m_Program, 5, "glow");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("ScRingworldFog"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_effectApply = GetUniformLoc("effectApply");
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_glowDiscard = GetUniformLoc("glowDiscard");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_texEffect = GetUniformLoc("texEffect");
	for(int t=0; t<SCTEXI_Count; t++)
		m_texSurface[t] = GetUniformLoc(QString("texSurface[%1]").arg(t));
	m_vertAlpha = GetUniformLoc("vertAlpha");

	m_ambient = GetUniformLoc("light.ambient");
	m_ambientUp = GetUniformLoc("light.ambientUp");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
		m_sunShadowAngle[sun] = GetUniformLoc(QString("light.sun%1.shadowAngle").arg(sun));
		m_sunShadowCount[sun] = GetUniformLoc(QString("light.sun%1.shadowCount").arg(sun));
		m_sunShadowXVec[sun] = GetUniformLoc(QString("light.sun%1.shadowXVec").arg(sun));
		m_sunShadowYVec[sun] = GetUniformLoc(QString("light.sun%1.shadowYVec").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLScRingworldFog::Select(const AuMatrix4x4& projection, const AuGLEffects& re)
{
	AuGLSLScBase::Select(projection, re);
	SetProjection(projection);
	if(!m_SunsValid)
	{
		// Suns 
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG, sun.m_ColorB,
					sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius, 0, sun.m_ShadowCount,
					sun.m_XVec, sun.m_YVec);
		}
		m_SunsValid = true;
	}

	if(!m_LightsValid)
	{
		// Point lights
		for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
		{
			const AuGLLight& light = re.Light(lightIndex);
			SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR, light.m_ColorG, light.m_ColorB,
					light.m_Position.x, light.m_Position.y, light.m_Position.z, light.m_cosOuter, light.m_cosInner,
					light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
		}
		m_LightsValid = true;
	}

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	for(int t=0; t<SCTEXI_Count; t++)
	{
		m_GLf->glActiveTexture(GL_TEXTURE1 + t);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture[t]);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGLSLLavaNoBump::Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
{
	AuGLSLProgram::Build(GLf, opt);

	// Create a shader program.
	if(s_DebugGLSL)
		qDebug() << "AuGLSLLavaNoBump::Build";
	Q_ASSERT(!m_Program);
	m_Program = m_GLf->glCreateProgram();

	// Load and compile source code for individual shaders that comprise the shader program.
	AuGLSLShader vs(*m_GLf, GL_VERTEX_SHADER, "LavaNoBump.v", opt.MaxLights());
	m_GLf->glAttachShader(m_Program, vs.Id());

	AuGLSLShader fs(*m_GLf, GL_FRAGMENT_SHADER, "LavaNoBump.f");
	m_GLf->glAttachShader(m_Program, fs.Id());

	// Bind user defined 'in' variables to vertex attribute indices.
	m_GLf->glBindAttribLocation(m_Program, 0, "position");
	m_GLf->glBindAttribLocation(m_Program, 1, "texCoord");

	// Bind user defined 'out' variable to data buffer.
	m_GLf->glBindFragDataLocation(m_Program, 0, "fragColor");

	// Link the shader program.
	m_GLf->glLinkProgram(m_Program);
	GLint linked;
	m_GLf->glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
#if !defined(QT_NO_DEBUG)
	DumpProgramInfoLog();
#endif
	if(!linked)
	{
		AuGLSLErr mb(QObject::tr("Shader Linking Error"),
				QObject::tr("<html>Failed to link <b>%2</b> shader.<br>"
					"This is a critical error. "
					"The program must abort.<br><br>%1</html>")
				.arg(GetProgramInfoLog())
				.arg("LavaNoBump"));
		mb.exec();
		//exit(-1);
	}

	// Clean up.
	m_GLf->glDetachShader(m_Program, vs.Id());
	m_GLf->glDeleteShader(vs.Id());

	m_GLf->glDetachShader(m_Program, fs.Id());
	m_GLf->glDeleteShader(fs.Id());

	// Install program object as part of current GL state so we can query locations of uniform variables.
	m_GLf->glUseProgram(m_Program);

	// Locate uniform variables. Their locations will not change unless/until the program is relinked.
	m_fogColor = GetUniformLoc("fogColor");
	m_fogDensity = GetUniformLoc("fogDensity");
	m_mv = GetUniformLoc("mv");
	m_p = GetUniformLoc("p");
	m_tex = GetUniformLoc("tex");
	m_texOffset = GetUniformLoc("texOffset");

	m_ambient = GetUniformLoc("light.ambient");
	m_eyeLocation = GetUniformLoc("light.eyeLocation");

	for(int sun=0; sun<MAX_SUNS; sun++)
	{
		m_sunEnabled[sun] = GetUniformLoc(QString("light.sun%1.enabled").arg(sun));
		m_sunAttenuation[sun] = GetUniformLoc(QString("light.sun%1.attenuation").arg(sun));
		m_sunColor[sun] = GetUniformLoc(QString("light.sun%1.color").arg(sun));
		m_sunPosition[sun] = GetUniformLoc(QString("light.sun%1.position").arg(sun));
		m_sunRadius[sun] = GetUniformLoc(QString("light.sun%1.radius").arg(sun));
	}

	for(unsigned int light=0; light<opt.MaxLights(); light++)
	{
		m_lightEnabled[light] = GetUniformLoc(QString("light.light[%1].enabled").arg(light));
		m_lightKL[light] = GetUniformLoc(QString("light.light[%1].kL").arg(light));
		m_lightKQ[light] = GetUniformLoc(QString("light.light[%1].kQ").arg(light));
		m_lightColor[light] = GetUniformLoc(QString("light.light[%1].color").arg(light));
		m_lightPosition[light] = GetUniformLoc(QString("light.light[%1].position").arg(light));
		m_lightCosOuterCone[light] = GetUniformLoc(QString("light.light[%1].cosOuterCone").arg(light));
		m_lightCosInnerCone[light] = GetUniformLoc(QString("light.light[%1].cosInnerCone").arg(light));
		m_lightDirection[light] = GetUniformLoc(QString("light.light[%1].direction").arg(light));
	}
}

void AuGLSLLavaNoBump::Select(const AuMatrix4x4& projection)
{
	AuGLSLProgram::Select();
	SetProjection(projection);

	// Remember previous texture bindings.
	m_GLf->glActiveTexture(GL_TEXTURE0);
	m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
