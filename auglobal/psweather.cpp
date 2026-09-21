//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psweather.cpp,v 1.22 2022/02/19 18:34:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psweather.h"
//
#include "augleffects.h"

static AuGLVAO s_SnowFlakeVAO;

PsWeather::PsWeather() : AuParticleSystem()
{
	m_vWind	= AuPoint(0.0f,0.0f,0.0f);
	m_sphereConstraintLength = 0.0f;
	m_renderMethod = RENDER_METHOD_Snow;
}

PsWeather::~PsWeather()
{

}

void PsWeather::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	if(!m_tFix.IsIdentity())
	{
		pParticle->m_vCurPos = m_tFix.Project(pParticle->m_vCurPos);
		pParticle->m_vCurVel = m_tFix.ProjectVector(pParticle->m_vCurVel);
	}
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += (pParticle->m_vCurVel + m_vWind) * GetElapsedTime();

	// If sphere constraint is present then determine outcome of particle
	double distToViewer = pParticle->m_vCurPos.Length2();
	if(distToViewer <= m_sphereConstraintLength || distToViewer > pParticle->m_InitDist)
		pParticle->m_fInitTime -= GetLifeCycle();	// Particle needs to die next pass
}

void PsWeather::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = m_gN * GetVelocity().z;
	if(GetVelocityVar() != 0)
		pParticle->m_vCurVel += GetRandomVector() * GetVelocityVar();
	pParticle->m_fInitTime = GetCurTime();
	pParticle->m_vCurPos = GetPosition();
	if(GetPositionVar() != 0)
	{
		// Offset the position by a random vector.
		AuPoint randv(GetRandomVector());
		randv.x *= GetPositionVar();
		randv.y *= GetPositionVar();
		randv.z = (randv.z * GetPositionVar()/2) + GetPositionVar();	// Move up, with variance, so first snow layer is not flat.
		pParticle->m_vCurPos += randv;
	}
	if(!m_tNew.IsIdentity())
		pParticle->m_vCurPos = m_tNew.Project(pParticle->m_vCurPos);
	double distToViewer = pParticle->m_vCurPos.Length2();
	if(distToViewer < m_sphereConstraintLength)
	{
		double l = sqrt(m_sphereConstraintLength) + 1;
		pParticle->m_vCurPos.Unit(l);
	}
	pParticle->m_InitDist = pParticle->m_vCurPos.Length2();	// Particle is killed when it moves farther away than it started.
	pParticle->m_vOldPos = pParticle->m_vCurPos;
}

void PsWeather::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Q_UNUSED(projection);
	Q_UNUSED(scale);
	if(m_renderMethod == RENDER_METHOD_Snow)
	{
		if(!s_SnowFlakeVAO.VAO())
			CreateSnowFlakeVAO(re, s_SnowFlakeVAO);

		s_SnowFlakeVAO.SelectGL();
		for(auParticle* pParticle = GetActiveList(); pParticle; pParticle=pParticle->m_pNext)
		{
			// Skip the particles behind the viewer.
			AuPoint viewPt = model.Project(pParticle->m_vCurPos);
			if(viewPt.z >= 0)
				continue;
			else
			{
				viewPt.Unit();
				if(viewPt.Dot(AuPoint(0,0,-1)) < .7071)
					continue;
			}

			re.GLSL().SnowShader().SetFlakePosition(pParticle->m_vCurPos.x, pParticle->m_vCurPos.y, pParticle->m_vCurPos.z);
			s_SnowFlakeVAO.DrawGL();
		}
		s_SnowFlakeVAO.DeselectGL();
	}
	else if(m_renderMethod == RENDER_METHOD_Snow_Ringworld)
	{
		if(!s_SnowFlakeVAO.VAO())
			CreateSnowFlakeVAO(re, s_SnowFlakeVAO);

		s_SnowFlakeVAO.SelectGL();

		for(auParticle* pParticle = GetActiveList(); pParticle; pParticle=pParticle->m_pNext)
		{
			// Skip the particles behind the viewer.
			AuPoint viewPt = model.Project(pParticle->m_vCurPos);
			if(viewPt.z >= 0)
				continue;
			else
			{
				viewPt.Unit();
				if(viewPt.Dot(AuPoint(0,0,-1)) < .7071)
					continue;
			}

			re.GLSL().SnowRingworldShader().SetFlakePosition(pParticle->m_vCurPos.x,pParticle->m_vCurPos.y,pParticle->m_vCurPos.z);
			s_SnowFlakeVAO.DrawGL();
		}

		s_SnowFlakeVAO.DeselectGL();
	}
	else if(m_renderMethod == RENDER_METHOD_Rain)
	{
		// Render each particle as a line segment.
		// Compile a list of vertex locations.
		AuPointArray verts;
		for(auParticle* pParticle = GetActiveList(); pParticle; pParticle=pParticle->m_pNext)
		{
			// Skip the particles behind the viewer.
			AuPoint viewPt = model.Project(pParticle->m_vCurPos);
			if(viewPt.z >= 0)
				continue;
			else
			{
				viewPt.Unit();
				if(viewPt.Dot(AuPoint(0,0,-1)) < .7071)
					continue;
			}

			AuPoint vDir(pParticle->m_vOldPos - pParticle->m_vCurPos);
			vDir.Unit(.01);
			AuPoint vOldPos(pParticle->m_vOldPos - vDir);

			verts.append(vOldPos);
			verts.append(pParticle->m_vCurPos);
		}

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}
}

void PsWeather::CreateSnowFlakeVAO(AuGLEffects& GLf, AuGLVAO& vao)
{
	int vertices = 2*3;										// We will create 2 triangles.
	GLfloat* vaoPos = new GLfloat[vertices*2];				// vertices(xy) for triangles that make up the model.
	GLfloat* vaoTex = new GLfloat[vertices*2];				// texture coords(xy)
	unsigned int vaoPosIdx = 0;
	unsigned int vaoTexIdx = 0;
	{
		// Triangle 1
		// Vertex 0
		vaoPos[vaoPosIdx++] = -.5f;
		vaoPos[vaoPosIdx++] = -.5f;

		vaoTex[vaoTexIdx++] = 0;
		vaoTex[vaoTexIdx++] = 0;

		// Vertex 1
		vaoPos[vaoPosIdx++] = -.5f;
		vaoPos[vaoPosIdx++] = .5f;

		vaoTex[vaoTexIdx++] = 0;
		vaoTex[vaoTexIdx++] = 1;

		// Vertex 2
		vaoPos[vaoPosIdx++] = .5f;
		vaoPos[vaoPosIdx++] = .5f;

		vaoTex[vaoTexIdx++] = 1;
		vaoTex[vaoTexIdx++] = 1;

		// Triangle 2
		// Vertex 0
		vaoPos[vaoPosIdx++] = .5f;
		vaoPos[vaoPosIdx++] = .5f;

		vaoTex[vaoTexIdx++] = 1;
		vaoTex[vaoTexIdx++] = 1;

		// Vertex 1
		vaoPos[vaoPosIdx++] = .5f;
		vaoPos[vaoPosIdx++] = -.5f;

		vaoTex[vaoTexIdx++] = 1;
		vaoTex[vaoTexIdx++] = 0;

		// Vertex 2
		vaoPos[vaoPosIdx++] = -.5f;
		vaoPos[vaoPosIdx++] = -.5f;

		vaoTex[vaoTexIdx++] = 0;
		vaoTex[vaoTexIdx++] = 0;
	}

	// Generate vertex array(s) for this object. We only need one.
	vao.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	vao.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, vaoPosIdx*sizeof(GLfloat), vaoPos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, vaoTexIdx*sizeof(GLfloat), vaoTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	vao.DeselectGL();

	delete [] vaoPos;
	delete [] vaoTex;
}
