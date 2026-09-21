//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auNebula.cpp,v 1.74 2022/03/14 15:35:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auNebula.h"
//
#include "auatomrenderinfo.h"
#include "auglobal.h"
#include "aunoise.h"
#include "dagalaxy.h"
#define FARPOINT 2.5980762113533159402911695122588	//.3*sqrt(75);	// Farthest point possible away from nebula to be visible

auNebula::auNebula()
{
	m_CloudDensity = 0;
	m_CloudTexture = 0;
	m_CoreRadPc = DaGalaxy::CoreRadius(ShoresOfHazeron);	// Radius of the galaxy core in pc.
	m_GalaxyRadPc = DaGalaxy::Radius(ShoresOfHazeron);	// Radius of the galaxy in pc.
	m_iParticles = 1048576;
	m_iStars = m_iParticles/40;			// 1/40 the number of particles for the nebula
	m_fA = -2.9924924f;
	m_fB = 0.38151205f;
	m_fC = -1.8401746f;
	m_fD = -1.8524430f;
	m_fParticleSize = 25;
	m_iColor = 0;
	m_bReverseColor = false;
	m_Center = AuPoint(0,0,0);			// This is sector coordinates
	m_Seed = 0;
	m_Type = 0;
	m_fTotalMax = (float)0xffffffff;
	m_img = 0;
}

auNebula::auNebula(int iParticles, int iStars, float fA, float fB, float fC, float fD,
				   float fParticleSize, int iColor, bool bReverse, AuPoint center, int seed, int type, const QColor& galaxyColor)
{
	m_CloudDensity = 0;
	m_CloudTexture = 0;
	m_CoreRadPc = DaGalaxy::CoreRadius(ShoresOfHazeron);	// Radius of the galaxy core in pc.
	m_GalaxyRadPc = DaGalaxy::Radius(ShoresOfHazeron);	// Radius of the galaxy in pc.
	m_iParticles = iParticles;
	m_iStars = iStars;
	m_fA = fA;
	m_fB = fB;
	m_fC = fC;
	m_fD = fD;
	m_fParticleSize = fParticleSize;
	m_GalaxyColor = galaxyColor;
	m_iColor = iColor;
	m_bReverseColor = bReverse;
	m_Center = center;
	m_Seed = seed;
	m_Type = type;
	m_fTotalMax = (float)0xffffffff;
	m_img = 0;
}

auNebula::auNebula(const auNebula &rhs) : m_vNebula(rhs.m_vNebula), m_vStars(rhs.m_vStars)
{
	m_CloudColor = rhs.m_CloudColor;
	m_CloudDensity = rhs.m_CloudDensity;
	m_CloudTexture = rhs.m_CloudTexture;
	m_CoreRadPc = rhs.m_CoreRadPc;
	m_GalaxyRadPc = rhs.m_GalaxyRadPc;
	m_iParticles = rhs.m_iParticles;
	m_iStars = rhs.m_iStars;
	m_fA = rhs.m_fA;
	m_fB = rhs.m_fB;
	m_fC = rhs.m_fC;
	m_fD = rhs.m_fD;
	m_fParticleSize = rhs.m_fParticleSize;
	m_GalaxyColor = rhs.m_GalaxyColor;
	m_iColor = rhs.m_iColor;
	m_bReverseColor = rhs.m_bReverseColor;
	m_Center = rhs.m_Center;
	m_Seed = rhs.m_Seed;
	m_img = 0;

	m_fMaxAngle = rhs.m_fMaxAngle;
	m_fMaxSpeed = rhs.m_fMaxSpeed;
	m_Dimensions = rhs.m_Dimensions;
	m_Dice = rhs.m_Dice;
	m_fTotalMax = rhs.m_fTotalMax;
	m_Type = rhs.m_Type;
}

auNebula::~auNebula()
{
}

void auNebula::DeleteDeviceObjects(AuOpenGLFunctions& GLf)
{
	if(m_img)
	{
		GLf.glDeleteTextures(1, &m_img);
		m_img = 0;
	}
	if(m_SkyBoxVAO.VAO())
		m_SkyBoxVAO.DeleteGL();
}

void auNebula::CreateSkyBox(AuAtomRenderInfo& ari, const AuPoint& pos, unsigned int paneSize, bool mipMap)
{
	//qDebug() << "auNebula::CreateSkyBox pos" << pos << "size" << paneSize << "mip" << mipMap;
	CreateNebula();
	//qDebug() << "auNebula::CreateSkyBox nebula created";

	GLuint glowTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "StarGlow", ":data/smoketransparent.png",
			GL_LINEAR, GL_LINEAR, false, 2);
	GLuint sunGlowTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "StarSpriteDot", ":data/StarSpriteDot.png",
			GL_LINEAR, GL_LINEAR, false, 2);

	// Get current fbo binding. If it is not restored properly, things in the scene blink.
	GLint oldfbo;
	ari.re->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

	// Setup our FBO
	GLuint fbo;
	ari.re->glGenFramebuffers(1, &fbo);
	ari.re->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	if(!fbo)
		return;

	// Now setup the environment map texture
	ari.re->glGenTextures(1, &m_img);
	ari.re->glBindTexture(GL_TEXTURE_CUBE_MAP, m_img);
	ari.re->glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if(mipMap)
		ari.re->glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		ari.re->glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	ari.re->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, paneSize, paneSize, 0, GL_RGBA, GL_FLOAT, NULL);
	ari.re->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, paneSize, paneSize, 0, GL_RGBA, GL_FLOAT, NULL);
	ari.re->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, paneSize, paneSize, 0, GL_RGBA, GL_FLOAT, NULL);
	ari.re->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, paneSize, paneSize, 0, GL_RGBA, GL_FLOAT, NULL);
	ari.re->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, paneSize, paneSize, 0, GL_RGBA, GL_FLOAT, NULL);
	ari.re->glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, paneSize, paneSize, 0, GL_RGBA, GL_FLOAT, NULL);

	// Attach it to the FBO so we can render to it
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+0, m_img, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X+1, m_img, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X+2, m_img, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_CUBE_MAP_POSITIVE_X+3, m_img, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_CUBE_MAP_POSITIVE_X+4, m_img, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_CUBE_MAP_POSITIVE_X+5, m_img, 0);

	GLenum status = ari.re->glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		return;

	// Determine max total returned fudge factor.  This creates different types of nebula.
	// Ones that are more swirly to ones that are more dispersed making them look more cloudy.
	switch(m_Type)
	{
	case 0:
		m_fTotalMax = (float)0xcfffffff;	// More swirly
		break;
	case 1:
		m_fTotalMax = (float)0xafffffff;
		break;
	case 2:
		m_fTotalMax = (float)0x8fffffff;
		break;
	case 4:
		m_fTotalMax = (float)0x6fffffff;
		break;
	case 5:
		m_fTotalMax = (float)0x4fffffff;
		break;
	case 6:
		m_fTotalMax = (float)0x2fffffff;	// More cloudy
		break;
	default:
		break;
	}

	// Enable some blending and other attributes before drawing
	ari.re->glDepthMask(GL_FALSE);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Calculate position to translate nebula based on its center and player's position
	// Nebula is in Sector coordinates and player's position is Sector Coordinates
	AuPoint trans = m_Center-pos;
	double distance = pos.Distance(m_Center);

	GLuint colorAttachments[6] = 
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
	};

	// Set up perspective projection.
	AuMatrix4x4 projection;
	projection.Perspective(90*DEG2RAD, 1, .1, 100000);

	ari.re->glViewport(0,0, paneSize,paneSize);
	for(int i=0; i<6; i++)
	{
		// Then render as normal
		ari.re->glDrawBuffer(colorAttachments[i]);
		ari.re->glClearColor(0,0,0,1);
		ari.re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

		// Set up the mapping of 3-space to screen space.
		AuMatrix4x4 model;
		// Order is POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z
		if(i == 0)			// POS_X
		{
			model.RotateAxis( -180*DEG2RAD, 0, 0, 1);
			model.RotateAxis( 90*DEG2RAD, 0, 1, 0);
		}
		else if(i == 1)		// NEG_X
		{
			model.RotateAxis( -180*DEG2RAD, 0, 0, 1);
			model.RotateAxis(  -90*DEG2RAD, 0, 1, 0);
		}
		else if(i == 2)		// POS_Y
		{
			model.RotateAxis(  -90*DEG2RAD, 1, 0, 0);
		}
		else if(i == 3)		// NEG_Y
		{
			model.RotateAxis( 90*DEG2RAD, 1, 0, 0);
		}
		else if(i == 4)		// POS_Z
		{
			model.RotateAxis( -180*DEG2RAD, 1, 0, 0);
		}
		else if(i == 5)		// NEG_Z
		{
			model.RotateAxis( -180*DEG2RAD, 0, 0, 1);
		}

		// Always draw stars. Since the stars are tiny they can be rendered as point sprites.
		// Unfortunately when the location of point sprites is off the screen
		// they get clipped on ATI cards.  On NVIDIA cards the point size is
		// taken into account before clipping.  So that means our larger point sprites
		// must be drawn as quads to support ATI cards.
		float starDotSize = paneSize / 2048.f;	// This is used more as a scaling factor than a size.
		RenderStars(starDotSize, *ari.re, projection, model, sunGlowTexture, pos);

		// Now draw galaxy cloud as quads
		if(!m_vStars.isEmpty())
		{
			ari.re->GLSL().GalaxyCloudShader().Select();
			ari.re->GLSL().GalaxyCloudShader().SetProjection(projection);
			ari.re->GLSL().GalaxyCloudShader().SetModelView(model);
			ari.re->GLSL().GalaxyCloudShader().SetTexture(glowTexture);
			ari.re->GLSL().GalaxyCloudShader().SetColor(m_GalaxyColor);

			AuPoint vRight(model[0], model[4], model[8]);
			AuPoint vUp(model[1], model[5], model[9]);
			AuPoint vPoint0;
			AuPoint vPoint1;
			AuPoint vPoint2;
			AuPoint vPoint3;
			const double fAdjustedSize = 1;

			// Create vertex locations, 6 vertices per star position because the quad is made with two triangles.
			int vertices = m_vStars.count()*6;
			GLfloat* starPos = new GLfloat[vertices*3];
			GLfloat* texCoord = new GLfloat[vertices*2];
			int starPosIdx = 0;
			int texCoordIdx = 0;
			for(QVector<galParticle>::const_iterator si = m_vStars.constBegin(); si != m_vStars.constEnd(); si++)
			{
				// Calculate vRight and vUp vectors that are consistent between panes of the sky box.
				AuPoint vN(-(*si).pos);
				vN.Unit();
				vN.ArbitraryAxis(vRight);
				vUp = vN.Cross(vRight);
				vUp.Unit();

				//-------------------------------------------------------------
				//
				// vPoint3                vPoint2
				//         +------------+
				//         |            |
				//         |     +      |
				//         |    *si     |
				//         |            |
				//         +------------+
				// vPoint0                vPoint1
				//
				// Now, build a quad around the center point (*si) based on the vRight and vUp vectors.
				// This will guarantee that the quad will be orthogonal to the view.
				//
				//-------------------------------------------------------------
				//const double fAdjustedSize = .5 + (*si).Length() / 250;
				vPoint0 = (*si).pos + ((-vRight - vUp) * fAdjustedSize);
				vPoint1 = (*si).pos + (( vRight - vUp) * fAdjustedSize);
				vPoint2 = (*si).pos + (( vRight + vUp) * fAdjustedSize);
				vPoint3 = (*si).pos + ((-vRight + vUp) * fAdjustedSize);

				// Triangle 1
				starPos[starPosIdx++] = vPoint0.x;
				starPos[starPosIdx++] = vPoint0.y;
				starPos[starPosIdx++] = vPoint0.z;
				texCoord[texCoordIdx++] = 0;
				texCoord[texCoordIdx++] = 0;

				starPos[starPosIdx++] = vPoint3.x;
				starPos[starPosIdx++] = vPoint3.y;
				starPos[starPosIdx++] = vPoint3.z;
				texCoord[texCoordIdx++] = 0;
				texCoord[texCoordIdx++] = 1;

				starPos[starPosIdx++] = vPoint2.x;
				starPos[starPosIdx++] = vPoint2.y;
				starPos[starPosIdx++] = vPoint2.z;
				texCoord[texCoordIdx++] = 1;
				texCoord[texCoordIdx++] = 1;

				// Triangle 2
				starPos[starPosIdx++] = vPoint2.x;
				starPos[starPosIdx++] = vPoint2.y;
				starPos[starPosIdx++] = vPoint2.z;
				texCoord[texCoordIdx++] = 1;
				texCoord[texCoordIdx++] = 1;

				starPos[starPosIdx++] = vPoint1.x;
				starPos[starPosIdx++] = vPoint1.y;
				starPos[starPosIdx++] = vPoint1.z;
				texCoord[texCoordIdx++] = 1;
				texCoord[texCoordIdx++] = 0;

				starPos[starPosIdx++] = vPoint0.x;
				starPos[starPosIdx++] = vPoint0.y;
				starPos[starPosIdx++] = vPoint0.z;
				texCoord[texCoordIdx++] = 0;
				texCoord[texCoordIdx++] = 0;
			}

			AuGLVAO vao;
			vao.CreateGL(*ari.re, vertices, 2);

			vao.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			ari.re->glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
			ari.re->glBufferData(GL_ARRAY_BUFFER, vertices*3*sizeof(GLfloat), starPos, GL_STREAM_DRAW);
			ari.re->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

			// bind buffer for tex coords and copy data into buffer.
			ari.re->glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(1));
			ari.re->glBufferData(GL_ARRAY_BUFFER, vertices*2*sizeof(GLfloat), texCoord, GL_STREAM_DRAW);
			ari.re->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

			ari.re->glBindBuffer(GL_ARRAY_BUFFER, 0);

			vao.DrawGL(GL_TRIANGLES);
			vao.DeselectGL();

			// Delete the vertex array object. It is no longer needed because the display list has captured what it wants.
			vao.DeleteGL();

			delete [] starPos;
			delete [] texCoord;

			ari.re->GLSL().GalaxyCloudShader().Deselect();
		}

		// Draw the nebula.
		if(m_iParticles > 0)
		{
			GLuint nebTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Ring", ":data/txAsteroids.png");

			model.Translate(trans);
			RenderNebula(distance, *ari.re, projection, model, nebTex);
		}
	}

	ari.ResetViewport();

	// Disable blending and other attributes now that drawing is done
	ari.re->glDisable(GL_BLEND);
	ari.re->glDepthMask(GL_TRUE);

	if(mipMap)
		ari.re->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Cleanup frame buffer.
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+0, 0, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X+1, 0, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X+2, 0, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_CUBE_MAP_POSITIVE_X+3, 0, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_CUBE_MAP_POSITIVE_X+4, 0, 0);
	ari.re->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_CUBE_MAP_POSITIVE_X+5, 0, 0);

	ari.re->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldfbo);	// Unbind frame buffer

	ari.re->glDeleteFramebuffers(1, &fbo);
	//qDebug() << "auNebula::CreateSkyBox done";
}

void auNebula::RenderSkyBox(AuGLEffects& re, const AuMatrix4x4& projection, const AuMatrix4x4& model, float opacity)
{
	Q_UNUSED(projection);
	if(!m_SkyBoxVAO.VAO())
	{
		// The skybox shader calculates texture coordinates from the position values of the vertices.
		AuGlobeLLTriList tris;
		tris.CreateGlobeTriangles(8);

		// Store count of triangles in the vertex array object.
		int vertices = tris.count()*3;

		// Copy surface triangles into gl array buffers.
		GLfloat* position = new GLfloat[vertices*3];	// 'in vec3 position' variable in the vertex shader.

		double radius = FeetPerParsec();
		unsigned int pidx = 0;
		for(AuGlobeLLTriList::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
		{
			// Calculate triangle vertex coordinates.
			AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(radius);
			AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(radius);
			AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(radius);

			// Vertex 0
			position[pidx++] = (float)terCoord0.x;
			position[pidx++] = (float)terCoord0.y;
			position[pidx++] = (float)terCoord0.z;

			// Vertex 1
			position[pidx++] = (float)terCoord2.x;
			position[pidx++] = (float)terCoord2.y;
			position[pidx++] = (float)terCoord2.z;

			// Vertex 2
			position[pidx++] = (float)terCoord1.x;
			position[pidx++] = (float)terCoord1.y;
			position[pidx++] = (float)terCoord1.z;
		}

		// Generate vertex array object with a buffer for each array.
		m_SkyBoxVAO.CreateGL(re, vertices, 1);

		m_SkyBoxVAO.SelectGL();

		// bind buffer for positions and copy data into buffer.
		re.glBindBuffer(GL_ARRAY_BUFFER, m_SkyBoxVAO.Buffer(0));
		re.glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, GL_STATIC_DRAW);
		re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		re.glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_SkyBoxVAO.DeselectGL();

		// OpenGL has the data now. I can discard my local buffers.
		delete [] position;
	}

	// Modulate the color of the skybox with the opacity value. This is a lot less work than blending since nothing is already
	// on the screen that we care to blend with.
	re.GLSL().SkyBoxShader().Select(projection);
	re.GLSL().SkyBoxShader().SetModelView(model);
	re.GLSL().SkyBoxShader().SetOpacity(opacity);
	re.GLSL().SkyBoxShader().SetTexture(m_img);

	//re.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	re.glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	m_SkyBoxVAO.SelectGL();
	m_SkyBoxVAO.DrawGL(GL_TRIANGLES);
	m_SkyBoxVAO.DeselectGL();
	//re.glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//re.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	re.GLSL().SkyBoxShader().Deselect();
}

void auNebula::Swap(float num1, float num2)
{
	float fStore = num1;
	num1 = num2;
	num2 = fStore;
}

// 
void auNebula::ChooseOrder()
{
	// There are 24 different combinations we can make with the 4 numbers
	// Right now we are only doing 6 of those combinations.
	int iCombo = m_Dice.Roll(1, 6);
	switch(iCombo)
	{
		case 2:
			if(fabs(m_fC) > 1.125)
				Swap(m_fC, m_fD);
			break;
		case 3:
			Swap(m_fB, m_fC);
			break;
		case 4:
			if(m_fB > 0)
			{
				Swap(m_fB, m_fC);
				Swap(m_fB, m_fD);
			}
			break;
		case 5:
			Swap(m_fB, m_fD);
			Swap(m_fB, m_fC);
			break;
		case 6:
			Swap(m_fB, m_fD);
			break;
		default:
			break;
	}
}

// Create the Nebula model.
// Remember that the model will be viewed from six different angles,
// Front, Back, Top, Bottom, Left, and Right.  This will create the six textures
// to use for the sky box in the scene.  You can have all 4 values be randomly
// generated rather than use the method below.  Beware though, because most of the time
// the values converge and you end up with congested points rather than a cloudy
// like substance.  This current method tries to eliminate the possibility of that
// type of occurrence by generating 1 random number and perturbing that value
// for the other values in the algorithm.  This method has given the best
// results in terms of nebula that do not converge to congested points.
void auNebula::CreateNebula()
{
	AuPoint oldPos;
	AuPoint nuPos;
	// Note we generate random FLOATS because
	// a .1 difference between any of the numbers changes
	// the overall result.
	m_Dice.Seed(m_Seed);
	m_fB = m_Dice.RandomMinMax(-3, 3, m_fTotalMax);
	if(m_fB < 0 && m_fB > -0.5)
		m_fB = m_fB-m_Dice.RandomMinMax(.5, .875, m_fTotalMax);
	if(m_fB > 0 && m_fB < 0.5)
		m_fB = m_fB+m_Dice.RandomMinMax(.5, .875, m_fTotalMax);
	m_fA = fabs(m_fB)+1.5;
	if(m_fA > 3)
		m_fA = 3;
	float fRand = m_Dice.RandomMinMax(0, 2, m_fTotalMax);
	if(fRand <= 1)
		m_fA = -m_fA;
	if(fRand < .25)
	{
		m_fD = -m_fA;
		m_fC = -m_fB;
	}
	else if(fRand < .5)
	{
		m_fD = m_fA;
		m_fC = -m_fB;
	}
	else if(fRand < .75)
	{
		m_fD = -m_fA;
		m_fC = m_fB;
	}
	else
	{
		m_fD = m_fA;
		m_fC = m_fB;
	}

	// Now randomly order them, sometimes there is no change
	ChooseOrder();

	m_fMaxSpeed = 0;
	m_fMaxAngle = 0;
	for(int i=0; i<m_iParticles; i++)
	{
		nuPos.x = sin(m_fA * oldPos.y) - oldPos.z * cos(m_fB * oldPos.x);
		nuPos.y = oldPos.z * sin(m_fC * oldPos.x) - cos(m_fD * oldPos.y);
		nuPos.z = sin(oldPos.x);
		float speed = nuPos.Distance(oldPos);	// Distance between the current and last point
		float angle = 0;
		if(i != 0 && i != m_iParticles-2)
		{
			// Law of cosines
			// b^2 = a^2 + c^2 - 2ac * cos(B)
			float c = oldPos.Distance(nuPos);
			AuPoint nextPos;
			nextPos.x = sin(m_fA * nuPos.y) - nuPos.z * cos(m_fB * nuPos.x);
			nextPos.y = nuPos.z * sin(m_fC * nuPos.x) - cos(m_fD * nuPos.y);
			nextPos.z = sin(nuPos.x);
			float a = nuPos.Distance(nextPos);
			float b = oldPos.Distance(nextPos);
			float acosNum = ( b*b - a*a - c*c ) / (-2*a*c);
			angle = acos(acosNum);
		}
		nebParticle nuParticle;
		nuParticle.r = 1;
		nuParticle.g = 1;
		nuParticle.b = 1;
		nuParticle.pos = nuPos;
		nuParticle.speed = speed;
		nuParticle.angle = angle;
		if(m_fMaxSpeed < speed)
			m_fMaxSpeed = speed;
		if(m_fMaxAngle < angle)
			m_fMaxAngle = angle;
		m_Dimensions = m_Dimensions.Max(nuPos);
		m_vNebula.append(nuParticle);	// Add Particle to list
		oldPos = nuPos;				// Keep last position for next pass
	}

	// Create starry background for nebula
	if(m_iStars == 0)
	{
		// We are not drawing any background stars in our galaxy
		// Instead draw the gackground stars in the Universe
		// This number of stars is always the same
		for(int i=0; i<10000; i++)
		{
			// getRandomVector returns a number between -1 and 1
			AuPoint vect = GetRandomVector();
			m_vUniStars.append(vect);
		}
	}
	else
	{
		for (int i=0; i<m_iStars; i++)
		{
			// getRandomVector returns a number between -1 and 1
			galParticle nuParticle;
			nuParticle.pos = GetRandomVector();
			m_vStars.append(nuParticle);
		}
	}

	// Calculate Color
	CalculateColor();
}

void auNebula::CalculateColor()
{
	if(m_vNebula.isEmpty())
	{
		m_CloudColor = QColor(0,0,0);
		return;
	}

	// Calculate Base Color
	double r = 0;
	double g = 0;
	double b = 0;
	float fDiv = .01f;
	float fMod = .005f;
	float fFact = .25f;
	if(m_iColor == 0)			// Green and Blue
	{
		// skybox blue and light green, nebula teal.
		r = fFact*fDiv;
		g = fDiv;
		b = fMod;
	}
	else if(m_iColor == 1)		// Light Green and Light Blue
	{
		// skybox green and light blue, nebula teal.
		r = fFact*fDiv;
		g = fMod;
		b = fDiv;
	}
	else if(m_iColor == 2)		// Light Pink, Light Purple, and Blue
	{
		// skybox blue and purple, nebula magenta
		r = fDiv;
		g = fFact*fDiv;
		b = fMod;
	}
	else if(m_iColor == 3)		// Dark Pink, Dark Purple, and Red
	{
		// skybox red and purple, nebula magenta
		r = fMod;
		g = fFact*fDiv;
		b = fDiv;
	}
	else if(m_iColor == 4)		// Yellow-Green and Yellow-Orange
	{
		// skybox green and yellow, nebula yellow
		r = fDiv;
		g = fMod;
		b = fFact*fDiv;
	}
	else if(m_iColor == 5)		// Yellow-Green and Red-Orange
	{
		// skybox red and yellow, nebula yellow
		r = fMod;
		g = fDiv;
		b = fFact*fDiv;
	}

	float Rd = 0;
	float Gd = 0;
	float Bd = 0;
	//int Ad = 0;
	const float kR = 1.f;
	const float kG = 1.f;
	const float kB = 1.f;
	const float kA = 1.f;

	for(QVector<nebParticle>::iterator it = m_vNebula.begin(); it != m_vNebula.end(); it++)
	{
		float angleCol = (*it).angle/m_fMaxAngle;
		float speedCol = (*it).speed/m_fMaxSpeed;
		if(m_bReverseColor)
		{
			angleCol = 1-angleCol;
			speedCol = 1-speedCol;
		}

		if(m_iColor == 0)			// Green and Blue
		{
			(*it).r = angleCol*r;
			(*it).g = speedCol*g;
			(*it).b = b;
		}
		else if(m_iColor == 1)		// Light Green and Light Blue
		{
			(*it).r = angleCol*r;
			(*it).g = g;
			(*it).b = speedCol*b;
		}
		else if(m_iColor == 2)		// Light Pink, Light Purple, and Blue
		{
			(*it).r = speedCol*r;
			(*it).g = angleCol*g;
			(*it).b = b;
		}
		else if(m_iColor == 3)		// Dark Pink, Dark Purple, and Red
		{
			(*it).r = r;
			(*it).g = angleCol*g;
			(*it).b = speedCol*b;
		}
		else if(m_iColor == 4)		// Yellow-Green and Yellow-Orange
		{
			(*it).r = speedCol*r;
			(*it).g = g;
			(*it).b = angleCol*b;
		}
		else if(m_iColor == 5)		// Yellow-Green and Red-Orange
		{
			(*it).r = r;
			(*it).g = speedCol*g;
			(*it).b = angleCol*b;
		}

		if(Rd >= kR || Gd >= kG || Bd >= kB)
			continue;

		// Calculate source color.
		// Mimic glBlend with glBlendFunc(GL_SRC_ALPHA, GL_ONE)
		float Rs = (*it).r;
		float Gs = (*it).g;
		float Bs = (*it).b;
		float As = .01f;

		// GL_SRC_ALPHA source scaling factors.
		float sR = As/kA;
		float sG = As/kA;
		float sB = As/kA;

		Rd = Rs*sR + Rd;
		if(Rd > kR)
			Rd = kR;

		Gd = Gs*sG + Gd;
		if(Gd > kG)
			Gd = kG;

		Bd = Bs*sB + Bd;
		if(Bd > kB)
			Bd = kB;
	}

	//qDebug() << "cloudColor" << Rd << Gd << Bd;
	m_CloudColor.setRgbF(Rd, Gd, Bd);
}

void auNebula::RenderNebula(double distance, AuGLEffects& re, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		GLuint texture)
{
	if(m_vNebula.isEmpty())
		return;

	// Draw Nebula
	float fAdjustedSize = m_fParticleSize - m_fParticleSize*(distance/FARPOINT);

	re.GLSL().NebulaShader().Select();
	re.GLSL().NebulaShader().SetProjectionModelView(projection * model);
	re.GLSL().NebulaShader().SetTexture(texture);

	// Draw Nebula
	AuPoint vRight(model[0], model[4], model[8]);
	AuPoint vUp(model[1], model[5], model[9]);
	AuPoint vPoint0;
	AuPoint vPoint1;
	AuPoint vPoint2;
	AuPoint vPoint3;
	AuPoint vCenter;
	fAdjustedSize = 4.0f/300.0f;

	// Construct vertex, tex coord, and color data.
	const int vertices = m_vNebula.count() * 6;					// Number of vertices we will be creating for quads (2 tris each).
	GLfloat* nebPos = new GLfloat[vertices*3];					// vertices
	int nebPosIdx = 0;
	GLfloat* nebCol = new GLfloat[vertices*3];					// colors
	int nebColIdx = 0;
	GLfloat* nebTex = new GLfloat[vertices*2];					// texture coords
	int nebTexIdx = 0;
	for(QVector<nebParticle>::const_iterator ni = m_vNebula.constBegin(); ni != m_vNebula.constEnd(); ni++)
	{
		//-------------------------------------------------------------
		//
		// vPoint3                vPoint2
		//         +------------+
		//         |            |
		//         |     +      |
		//         |    *ni     |
		//         |            |
		//         +------------+
		// vPoint0                vPoint1
		//
		// Now, build a quad around the center point based on the vRight 
		// and vUp vectors. This will guarantee that the quad will be 
		// orthogonal to the view.
		//
		//-------------------------------------------------------------
		vPoint0 = (*ni).pos + ((-vRight - vUp) * fAdjustedSize );
		vPoint1 = (*ni).pos + (( vRight - vUp) * fAdjustedSize );
		vPoint2 = (*ni).pos + (( vRight + vUp) * fAdjustedSize );
		vPoint3 = (*ni).pos + ((-vRight + vUp) * fAdjustedSize );

		// Vertex
		nebPos[nebPosIdx++] = vPoint0.x;
		nebPos[nebPosIdx++] = vPoint0.y;
		nebPos[nebPosIdx++] = vPoint0.z;

		nebCol[nebColIdx++] = (*ni).r;
		nebCol[nebColIdx++] = (*ni).g;
		nebCol[nebColIdx++] = (*ni).b;

		nebTex[nebTexIdx++] = 0;
		nebTex[nebTexIdx++] = 0;

		// Vertex
		nebPos[nebPosIdx++] = vPoint3.x;
		nebPos[nebPosIdx++] = vPoint3.y;
		nebPos[nebPosIdx++] = vPoint3.z;

		nebCol[nebColIdx++] = (*ni).r;
		nebCol[nebColIdx++] = (*ni).g;
		nebCol[nebColIdx++] = (*ni).b;

		nebTex[nebTexIdx++] = 0;
		nebTex[nebTexIdx++] = 1;

		// Vertex
		nebPos[nebPosIdx++] = vPoint2.x;
		nebPos[nebPosIdx++] = vPoint2.y;
		nebPos[nebPosIdx++] = vPoint2.z;

		nebCol[nebColIdx++] = (*ni).r;
		nebCol[nebColIdx++] = (*ni).g;
		nebCol[nebColIdx++] = (*ni).b;

		nebTex[nebTexIdx++] = 1;
		nebTex[nebTexIdx++] = 1;

		// Vertex
		nebPos[nebPosIdx++] = vPoint2.x;
		nebPos[nebPosIdx++] = vPoint2.y;
		nebPos[nebPosIdx++] = vPoint2.z;

		nebCol[nebColIdx++] = (*ni).r;
		nebCol[nebColIdx++] = (*ni).g;
		nebCol[nebColIdx++] = (*ni).b;

		nebTex[nebTexIdx++] = 1;
		nebTex[nebTexIdx++] = 1;

		// Vertex
		nebPos[nebPosIdx++] = vPoint1.x;
		nebPos[nebPosIdx++] = vPoint1.y;
		nebPos[nebPosIdx++] = vPoint1.z;

		nebCol[nebColIdx++] = (*ni).r;
		nebCol[nebColIdx++] = (*ni).g;
		nebCol[nebColIdx++] = (*ni).b;

		nebTex[nebTexIdx++] = 1;
		nebTex[nebTexIdx++] = 0;

		// Vertex
		nebPos[nebPosIdx++] = vPoint0.x;
		nebPos[nebPosIdx++] = vPoint0.y;
		nebPos[nebPosIdx++] = vPoint0.z;

		nebCol[nebColIdx++] = (*ni).r;
		nebCol[nebColIdx++] = (*ni).g;
		nebCol[nebColIdx++] = (*ni).b;

		nebTex[nebTexIdx++] = 0;
		nebTex[nebTexIdx++] = 0;
	}

	// Generate vertex array(s) for this object. We only need one.
	AuGLVAO vao;
	vao.CreateGL(re, vertices, 3);

	vao.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
	re.glBufferData(GL_ARRAY_BUFFER, vertices*3*sizeof(GLfloat), nebPos, GL_STREAM_DRAW);
	re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for tex coords and copy data into buffer.
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(1));
	re.glBufferData(GL_ARRAY_BUFFER, vertices*2*sizeof(GLfloat), nebTex, GL_STREAM_DRAW);
	re.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for tex coords and copy data into buffer.
	re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(2));
	re.glBufferData(GL_ARRAY_BUFFER, vertices*3*sizeof(GLfloat), nebCol, GL_STREAM_DRAW);
	re.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	re.glBindBuffer(GL_ARRAY_BUFFER, 0);

	vao.DrawGL(GL_TRIANGLES);
	vao.DeselectGL();

	// Delete the vertex array object. It is no longer needed because the display list has captured what it wants.
	vao.DeleteGL();

	delete [] nebPos;
	delete [] nebTex;
	delete [] nebCol;

	re.GLSL().NebulaShader().Deselect();
}

void auNebula::RenderStars(float starDotSize, AuGLEffects& re, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		GLuint texture, const AuPoint& pos)
{
	float sds2 = starDotSize * starDotSize;
	re.glEnable(GL_PROGRAM_POINT_SIZE); 

	// Draw Universe Stars
	if(!m_vUniStars.isEmpty())
	{
		re.GLSL().FarStarShader().Select();
		re.GLSL().FarStarShader().SetProjectionModelView(projection * model);
		re.GLSL().FarStarShader().SetSize(sds2 * 1.5);
		re.GLSL().FarStarShader().SetTexture(texture);

		// Create vertex locations.
		int vertices = m_vUniStars.count();
		GLfloat* uniStarPos = new GLfloat[vertices*3];
		int uniStarPosIdx = 0;
		for(QVector<AuPoint>::const_iterator si = m_vUniStars.constBegin(); si != m_vUniStars.constEnd(); si++)
		{
			uniStarPos[uniStarPosIdx++] = (*si).x;
			uniStarPos[uniStarPosIdx++] = (*si).y;
			uniStarPos[uniStarPosIdx++] = (*si).z;
		}

		AuGLVAO vao;
		vao.CreateGL(re, vertices, 1);

		vao.SelectGL();

		// bind buffer for vertices and copy data into buffer.
		re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
		re.glBufferData(GL_ARRAY_BUFFER, vertices*3*sizeof(GLfloat), uniStarPos, GL_STREAM_DRAW);
		re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

		re.glBindBuffer(GL_ARRAY_BUFFER, 0);

		vao.DrawGL(GL_POINTS);
		vao.DeselectGL();

		// Delete the vertex array object. It is no longer needed because the display list has captured what it wants.
		vao.DeleteGL();

		delete [] uniStarPos;

		re.GLSL().FarStarShader().Deselect();
	}

	// Draw Galaxy Stars
	if(!m_vStars.isEmpty())
	{
		re.GLSL().GalaxyStarShader().Select();
		re.GLSL().GalaxyStarShader().SetProjection(projection);
		re.GLSL().GalaxyStarShader().SetModelView(model);
		re.GLSL().GalaxyStarShader().SetSize(starDotSize * 4);
		re.GLSL().GalaxyStarShader().SetTexture(texture);

		// Calculate distance at which intensity reaches the minimum.
		double gd = pos.Length();		// Distance to the galactic core, in sectors.
		double c = cos(pos.PolarIncline());
		double i = AuNoise::Cosine_Interpolate(m_CoreRadPc/10, m_GalaxyRadPc/10, c);
		//qDebug() << "gd dist to core in sectors" << gd << "c" << c << "i" << i;
		re.GLSL().GalaxyStarShader().SetIntensityDistance(gd + i, gd - i);		// Near and far distances for intensity ranges.
		float iMin = .05f;
		float iMax = .4f*sds2;
		float tMin = .25f;
		float tMax = .5f/sds2;
		re.GLSL().GalaxyStarShader().SetIntensityRanges(iMin,iMax, tMin,tMax);	// Ranges of intensity and transparency.

		// Create vertex locations.
		int vertices = m_vStars.count();
		GLfloat* galaxyStarPos = new GLfloat[vertices*3];
		int galaxyStarPosIdx = 0;
		for(QVector<galParticle>::const_iterator si = m_vStars.constBegin(); si != m_vStars.constEnd(); si++)
		{
			galaxyStarPos[galaxyStarPosIdx++] = (*si).pos.x;
			galaxyStarPos[galaxyStarPosIdx++] = (*si).pos.y;
			galaxyStarPos[galaxyStarPosIdx++] = (*si).pos.z;
		}

		AuGLVAO vao;
		vao.CreateGL(re, vertices, 1);

		vao.SelectGL();

		// bind buffer for vertices and copy data into buffer.
		re.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
		re.glBufferData(GL_ARRAY_BUFFER, m_vStars.size()*3*sizeof(GLfloat), galaxyStarPos, GL_STREAM_DRAW);
		re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

		re.glBindBuffer(GL_ARRAY_BUFFER, 0);

		vao.DrawGL(GL_POINTS);
		vao.DeselectGL();

		// Delete the vertex array object. It is no longer needed because the display list has captured what it wants.
		vao.DeleteGL();

		delete [] galaxyStarPos;

		re.GLSL().GalaxyStarShader().Deselect();
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 
}

AuPoint auNebula::GetRandomVector()
{
	AuPoint vVector;

	// Pick a random Z between -1.0f and 1.0f.
	vVector.z = m_Dice.RandomMinMax( -1.0f, 1.0f );

	// Get radius of this circle
	float radius = (float)sqrt(1 - vVector.z * vVector.z);

	// Pick a random point on a circle.
	float t = m_Dice.RandomMinMax( -PIf, PIf );

	// Compute matching X and Y for our Z.
	vVector.x = (float)cosf(t) * radius;
	vVector.y = (float)sinf(t) * radius;

	return vVector;
}

