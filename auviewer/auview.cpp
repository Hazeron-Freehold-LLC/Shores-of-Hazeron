//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auview.cpp,v 1.70 2022/05/09 16:55:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auview.h"
//
#include "auanimalia.h"
#include "auatomrenderinfo.h"
#include "dpbody.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QWheelEvent>

#define d_Ambient						(AuGLLight::MaxAmbientStarlight()/2)

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuView::AuView(QWidget* parent) : QOpenGLWidget(parent),
	m_Font(g_Au.TypefaceSans(), 22),
	m_MM(false),
	m_RenderInfo(this, 0, m_MM, m_ModelCache, m_Font)
{
	setFocusPolicy(Qt::StrongFocus);
	setObjectName("AuView");
	setAttribute(Qt::WA_OpaquePaintEvent);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_3dsModel = 0;
	m_3dsModelCID = CID_Invalid;
	m_3dsModelState = false;
	m_AnimalFemale = 0;
	m_AnimalFemaleArmor = CID_Invalid;
	m_AnimalFemaleClothes = CID_Invalid;
	m_AnimalFemaleDelete = 0;
	m_AnimalFemaleHair = AuHairStyle::Unkempt;
	m_AnimalFemaleHat = CID_Invalid;
	m_AnimalFemaleMouth = AuPose::MouthClosed;
	m_AnimalFemalePose = AuPose::Stand;
	m_AnimalFemaleShave = AuHairStyle::Unshaven;
	m_AnimalMale = 0;
	m_AnimalMaleArmor = CID_Invalid;
	m_AnimalMaleClothes = CID_Invalid;
	m_AnimalMaleDelete = 0;
	m_AnimalMaleHair = AuHairStyle::Unkempt;
	m_AnimalMaleHat = CID_Invalid;
	m_AnimalMaleMouth = AuPose::MouthClosed;
	m_AnimalMalePose = AuPose::Stand;
	m_AnimalMaleShave = AuHairStyle::Unshaven;
	m_Body = 0;
	m_DirtTex = 0;
	m_GrassTex = 0;
	m_Item = 0;
	m_MeatImageTex = 0;
	m_MeatJointFront = 0;
	m_MeatJointSide = 0;
	m_MeatOriginFront = 0;
	m_MeatOriginSide = 0;
	m_MeatProfileInput = InputNothing;
	m_MeatProfileFrontVAOValid = false;
	m_MeatProfileSideVAOValid = false;
	m_MouseButtons = Qt::NoButton;
	m_ShowGround = 0;
	m_ShowMeatImage = 1;
	m_ShowMode = 1;
	m_ViewMode = None;
}

AuView::~AuView()
{
}

void AuView::initializeGL()
{
	//qDebug("AuView::initializeGL");
	//QSurfaceFormat fmt = format();
	//qDebug() << "AuView::initializeGL swapInt" << fmt.swapInterval() << "swapBehave" << fmt.swapBehavior()
		//<< "opaquePaint" << testAttribute(Qt::WA_OpaquePaintEvent);

	m_RenderInfo.InitializeGL();

	// Configure for solid rendered display.
	m_RenderInfo.re->glFrontFace(GL_CW);						// Front faces are those drawn clockwise.
	m_RenderInfo.re->glCullFace(GL_BACK);						// Cull back-facing faces.
	m_RenderInfo.re->glEnable(GL_CULL_FACE);					// Enable face culling.
	m_RenderInfo.re->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	m_RenderInfo.re->glEnable(GL_DEPTH_TEST);					// Enable depth calculations
	m_RenderInfo.re->glClearColor(.2f, .2f, .2f, 1);			// Let OpenGL clear to black

	ResetView();

	// Create a VAO for the ground.
	{
		int meters = 3;
		int pixels = 160;										// pixel size of Grid.png file
		const float offset = 1.f/(pixels*2.f);					// offset by 1/2 pixel.
		const float s = offset;
		const float t = -offset;
		const float u = meters * 2 + offset;
		const float v = meters * 2 - offset;
		double grad = meters * FeetPerMeter();
		float S = 0;	// Shine
		AuPoint N(0,0,1);	// Normal

		AuTriTexNS tri0;
		tri0.m_V0 = AuPointTexNS(-grad,-grad,0, s,t, N,S);
		tri0.m_V1 = AuPointTexNS(-grad,grad,0, s,v, N,S);
		tri0.m_V2 = AuPointTexNS(grad,grad,0, u,v, N,S);
		AuTriTexNS tri1;
		tri1.m_V0 = AuPointTexNS(grad,grad,0, u,v, N,S);
		tri1.m_V1 = AuPointTexNS(grad,-grad,0, u,t, N,S);
		tri1.m_V2 = AuPointTexNS(-grad,-grad,0, s,t, N,S);
		QList<AuTriTexNS> tris;
		tris << tri0 << tri1;

		m_Ground.CreateForModelTexFog(*m_RenderInfo.re, tris, GL_STATIC_DRAW);

		m_ConcreteTex = m_RenderInfo.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Concrete", ":data/ConcreteHeavy.png",
				GL_LINEAR, GL_LINEAR, true);

		m_DirtTex = m_RenderInfo.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Dirt", ":data/DirtCrackedModerate.png",
				GL_LINEAR, GL_LINEAR, true);

		m_GrassTex = m_RenderInfo.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Grass", ":data/Grass.png",
				GL_LINEAR, GL_LINEAR, true);

		m_TarmacTex = m_RenderInfo.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Tarmac", ":data/Tarmac.png",
				GL_LINEAR, GL_LINEAR, true);

		m_GridTex = m_RenderInfo.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Grid", "mp/Grid.png",
				GL_LINEAR, GL_LINEAR, true);
	}

	// Objects used when displaying meat profile.
	{
		if(!m_MeatJointFront)
		{
			m_MeatJointFront = AuItem::Create(CID_Torch, 128, 0, 0);
			//m_MeatJointFront->SetState(1);
		}
		if(!m_MeatJointSide)
		{
			m_MeatJointSide = AuItem::Create(CID_SmallMissileBl, 128, 0, 0);
			//m_MeatJointSide->SetState(1);
		}

		if(!m_MeatOriginFront)
		{
			m_MeatOriginFront = AuItem::Create(CID_Candle, 128, 0, 0);
			m_MeatOriginFront->SetState(1);
		}
		if(!m_MeatOriginSide)
		{
			m_MeatOriginSide = AuItem::Create(CID_SmallRocketBl, 128, 0, 0);
			m_MeatOriginSide->SetState(1);
		}
	}
}

void AuView::SetupLights(const AuMatrix4x4& model)
{
	m_RenderInfo.re->SetAmbientLight(d_Ambient, d_Ambient, d_Ambient, 1);

	m_RenderInfo.re->DisableSuns();
	m_RenderInfo.re->DisableLights();

	QColor sunColor(255, 234, 214);		// Yellow (Type G star)

	bool lightsMoveWithModel = false;
	if(lightsMoveWithModel)
	{
		// Lights move with the model.
		// Set up some general lighting.
		const double ltdist = 100000.0;		// Light distance from model.
		double atten = 1.0/(ltdist*ltdist);
		AuPoint vl(ltdist/2,ltdist/2,ltdist);
		vl.Unit(ltdist);
		vl = model.Project(vl);
		m_RenderInfo.re->SetSun(0, atten, sunColor, vl.x, vl.y, vl.z, 1.0);

		m_AmbientUp = model.ProjectVector(AuPoint(0,0,1));
	}
	else
	{
		// Lights remain stationary while the model moves.
		// Set up some general lighting.
		const double ltdist = 100000.0;		// Light distance from model.
		double atten = 1.0/(ltdist*ltdist);
		AuPoint vl(-ltdist/2,ltdist/2,ltdist);
		vl.Unit(ltdist);
		m_RenderInfo.re->SetSun(0, atten, sunColor, vl.x, vl.y, vl.z, 1.0);

		m_AmbientUp = AuPoint(0,1,0);
	}
}

void AuView::paintGL()
{
	//qDebug() << "AuView::paintGL";
	m_RenderInfo.re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(m_ViewMode == None)
		return;

	if(m_ViewMode == MeatProfile)
	{
		// Set up for a parallel projection.
		AuMatrix4x4 projection;
		if(rect().width() > 0)
			projection.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), 1, 10000);
		else
			projection.Perspective(45.0*DEG2RAD, 1, 1, 10000);

		if(m_ShowMeatImage && !m_MeatProfileImage.isEmpty())
		{
			if(!m_MeatImageTex)
			{
				// Load the meat profile image.
				QImage meatImage(m_MeatProfileImage);
				if(!meatImage.isNull())
				{
					m_MeatImageTex = m_RenderInfo.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, m_MeatProfileImage, meatImage,
							GL_LINEAR, GL_LINEAR, true);

					// Create a new VAO for the image.
					if(m_MeatImageVAO.VAO())
						m_MeatImageVAO.DeleteGL();

					m_MeatImageSize = meatImage.size();
					const float s = 0;
					const float t = 0;
					const float u = 1;
					const float v = 1;
					double wid2 = meatImage.width()/2;
					double ht2 = meatImage.height()/2;

					AuTriTex tri0;
					tri0.m_V0 = AuPointTex(-wid2,-ht2,0, s,t);
					tri0.m_V1 = AuPointTex(-wid2,ht2,0, s,v);
					tri0.m_V2 = AuPointTex(wid2,ht2,0, u,v);
					AuTriTex tri1;
					tri1.m_V0 = AuPointTex(wid2,ht2,0, u,v);
					tri1.m_V1 = AuPointTex(wid2,-ht2,0, u,t);
					tri1.m_V2 = AuPointTex(-wid2,-ht2,0, s,t);

					QList<AuTriTex> tris;
					tris << tri0 << tri1;

					m_MeatImageVAO.CreateForTexture(*m_RenderInfo.re, tris, GL_STATIC_DRAW);

					ResetView();
				}
			}

			// Paint the meat image VAO.
			if(m_MeatImageTex && m_MeatImageVAO.VAO())
			{
				m_RenderInfo.re->GLSL().TextureShader().SetProjectionModelView(projection * m_Model);
				m_RenderInfo.re->GLSL().TextureShader().SetTexture(m_MeatImageTex);
				m_RenderInfo.re->GLSL().TextureShader().SetColor(1,1,1,1);

				// Render the model.
				m_MeatImageVAO.SelectGL();
				m_MeatImageVAO.DrawGL();
				m_MeatImageVAO.DeselectGL();

				m_RenderInfo.re->GLSL().TextureShader().Deselect();
			}
		}

		// Render meat profiles.
		{
			// Front
			if(m_MeatProfileFront.count() <= 1)
			{
				if(m_MeatProfileFrontVAO.VAO())
					m_MeatProfileFrontVAO.DeleteGL();
			}
			else
			{
				if(!m_MeatProfileFrontVAO.VAO() || !m_MeatProfileFrontVAOValid)
				{
					if(m_MeatProfileFrontVAO.VAO())
						m_MeatProfileFrontVAO.DeleteGL();

					// Create a VAO of the current meat profile.
					// Store count of vertices in the vertex array object.
					int vertices = m_MeatProfileFront.count();

					// Copy vertices into gl array buffer.
					// This array buffer corresponds literally to the 'in vec4 position' variable in the vertex shader.
					GLfloat* position = new GLfloat[vertices*4];

					unsigned int pidx = 0;
					for(AuPointArray::const_iterator ti = m_MeatProfileFront.constBegin(); ti != m_MeatProfileFront.constEnd();
							ti++)
					{
						position[pidx++] = (float)(*ti).x;
						position[pidx++] = (float)(*ti).y;
						position[pidx++] = (float)(*ti).z;
						position[pidx++] = 1.f;				// Alpha
					}

					// Generate vertex array object with a buffer for each array.
					m_MeatProfileFrontVAO.CreateGL(*m_RenderInfo.re, vertices, 1);

					m_MeatProfileFrontVAO.SelectGL();

					// bind buffer for positions and copy data into buffer.
					m_RenderInfo.re->glBindBuffer(GL_ARRAY_BUFFER, m_MeatProfileFrontVAO.Buffer(0));
					m_RenderInfo.re->glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, GL_STATIC_DRAW);
					m_RenderInfo.re->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

					m_RenderInfo.re->glBindBuffer(GL_ARRAY_BUFFER, 0);

					m_MeatProfileFrontVAO.DeselectGL();
					m_MeatProfileFrontVAOValid = true;

					// OpenGL has the data now. I can discard my local buffers.
					delete [] position;
				}

				// Paint the meat image VAO.
				if(m_MeatProfileFrontVAO.VAO())
				{
					m_RenderInfo.re->glDisable(GL_DEPTH_TEST);

					m_RenderInfo.re->GLSL().LineShader().SetProjectionModelView(projection * m_Model);
					if(m_MeatProfileFront.first() == m_MeatProfileFront.last())
						m_RenderInfo.re->GLSL().LineShader().SetColor(.1,1,.1,1);
					else
						m_RenderInfo.re->GLSL().LineShader().SetColor(1,.1,.1,1);

					// Render the model.
					m_MeatProfileFrontVAO.SelectGL();
					m_MeatProfileFrontVAO.DrawGL(GL_LINE_STRIP);
					m_MeatProfileFrontVAO.DeselectGL();

					m_RenderInfo.re->GLSL().LineShader().Deselect();

					m_RenderInfo.re->glEnable(GL_DEPTH_TEST);
				}
			}

			// Side
			if(m_MeatProfileSide.count() <= 1)
			{
				if(m_MeatProfileSideVAO.VAO())
					m_MeatProfileSideVAO.DeleteGL();
			}
			else
			{
				if(!m_MeatProfileSideVAO.VAO() || !m_MeatProfileSideVAOValid)
				{
					if(m_MeatProfileSideVAO.VAO())
						m_MeatProfileSideVAO.DeleteGL();

					// Create a VAO of the current meat profile.
					// Store count of vertices in the vertex array object.
					int vertices = m_MeatProfileSide.count();

					// Copy vertices into gl array buffer.
					// This array buffer corresponds literally to the 'in vec4 position' variable in the vertex shader.
					GLfloat* position = new GLfloat[vertices*4];

					unsigned int pidx = 0;
					for(AuPointArray::const_iterator ti = m_MeatProfileSide.constBegin(); ti != m_MeatProfileSide.constEnd();
							ti++)
					{
						position[pidx++] = (float)(*ti).x;
						position[pidx++] = (float)(*ti).y;
						position[pidx++] = (float)(*ti).z;
						position[pidx++] = 1.f;				// Alpha
					}

					// Generate vertex array object with a buffer for each array.
					m_MeatProfileSideVAO.CreateGL(*m_RenderInfo.re, vertices, 1);

					m_MeatProfileSideVAO.SelectGL();

					// bind buffer for positions and copy data into buffer.
					m_RenderInfo.re->glBindBuffer(GL_ARRAY_BUFFER, m_MeatProfileSideVAO.Buffer(0));
					m_RenderInfo.re->glBufferData(GL_ARRAY_BUFFER, pidx*sizeof(GLfloat), position, GL_STATIC_DRAW);
					m_RenderInfo.re->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

					m_RenderInfo.re->glBindBuffer(GL_ARRAY_BUFFER, 0);

					m_MeatProfileSideVAO.DeselectGL();
					m_MeatProfileSideVAOValid = true;

					// OpenGL has the data now. I can discard my local buffers.
					delete [] position;
				}

				// Paint the meat image VAO.
				if(m_MeatProfileSideVAO.VAO())
				{
					m_RenderInfo.re->glDisable(GL_DEPTH_TEST);

					m_RenderInfo.re->GLSL().LineShader().SetProjectionModelView(projection * m_Model);
					if(m_MeatProfileSide.first() == m_MeatProfileSide.last())
						m_RenderInfo.re->GLSL().LineShader().SetColor(.1,1,.1,1);
					else
						m_RenderInfo.re->GLSL().LineShader().SetColor(1,.1,.1,1);

					// Render the model.
					m_MeatProfileSideVAO.SelectGL();
					m_MeatProfileSideVAO.DrawGL(GL_LINE_STRIP);
					m_MeatProfileSideVAO.DeselectGL();

					m_RenderInfo.re->GLSL().LineShader().Deselect();

					m_RenderInfo.re->glEnable(GL_DEPTH_TEST);
				}
			}
		}

		{
			// Render meat profile joints.
			SetupLights(m_Model);

			for(AuPointArray::const_iterator ji = m_MeatJointsFront.cbegin(); ji != m_MeatJointsFront.cend(); ji++)
			{
				if(ji == m_MeatJointsFront.cbegin())
				{
					float itemScale = 40;
					AuMatrix4x4 m;
					m.Translate(*ji);
					m.Scale(itemScale, itemScale, itemScale);
					m_MeatOriginFront->Render(projection, m_Model * m, *m_RenderInfo.re, m_RenderInfo.Models(), AuId(1), d_Ambient,
							d_Ambient, d_Ambient, m_AmbientUp, AuPoint(0,0,1), AuPoint(0,0), 1, 0, itemScale);
				}
				else
				{
					float itemScale = 40;
					AuMatrix4x4 m;
					m.Translate(*ji);
					m.Scale(itemScale, itemScale, itemScale);
					m_MeatJointFront->Render(projection, m_Model * m, *m_RenderInfo.re, m_RenderInfo.Models(), AuId(1), d_Ambient,
							d_Ambient, d_Ambient, m_AmbientUp, AuPoint(0,0,1), AuPoint(0,0), 1, 0, itemScale);
				}
			}
			for(AuPointArray::const_iterator ji = m_MeatJointsSide.cbegin(); ji != m_MeatJointsSide.cend(); ji++)
			{
				if(ji == m_MeatJointsSide.cbegin())
				{
					float itemScale = 40;
					AuMatrix4x4 m;
					m.Translate(*ji);
					m.Scale(itemScale, itemScale, itemScale);
					m_MeatOriginSide->Render(projection, m_Model * m, *m_RenderInfo.re, m_RenderInfo.Models(), AuId(1), d_Ambient,
							d_Ambient, d_Ambient, m_AmbientUp, AuPoint(0,0,1), AuPoint(0,0), 1, 0, itemScale);
				}
				else
				{
					float itemScale = 40;
					AuMatrix4x4 m;
					m.Translate(*ji);
					m.Scale(itemScale, itemScale, itemScale);
					m_MeatJointSide->Render(projection, m_Model * m, *m_RenderInfo.re, m_RenderInfo.Models(), AuId(1), d_Ambient,
							d_Ambient, d_Ambient, m_AmbientUp, AuPoint(0,0,1), AuPoint(0,0), 1, 0, itemScale);
				}
			}
		}
	}
	else
	{
		AuMatrix4x4 projection;
		if(rect().width() > 0)
			projection.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), 2, 10000);
		else
			projection.Perspective(45.0*DEG2RAD, 1, 2, 10000);

		SetupLights(m_Model);

		AuTime when(AuTime::Now());
		float cgHeightFemale;
		if(m_AnimalFemale)
			cgHeightFemale = m_AnimalFemale->CGHeight();
		else
			cgHeightFemale = 0;
		float cgHeightMale;
		if(m_AnimalMale)
			cgHeightMale = m_AnimalMale->CGHeight();
		else
			cgHeightMale = 0;
		switch(m_ViewMode)
		{
		case None:
		case MeatProfile:
			// These modes were already handled.
			break;

		case Animalia:
			// Opaque pass
			if(m_AnimalFemale)
			{
				if(!m_AnimalFemale->HasVAOs())
				{
					m_AnimalFemale->CreateClothingTex(*m_RenderInfo.re);
					m_AnimalFemale->CreateAnimalVAO(*m_RenderInfo.re);
				}
				AuMatrix4x4 m(m_Model);
				m.Translate(-1,0,0);
				m_AnimalFemale->RenderAnimal(projection, m, *m_RenderInfo.re, false, 1, m_AmbientUp);
			}
			if(m_AnimalMale)
			{
				if(!m_AnimalMale->HasVAOs())
				{
					m_AnimalMale->CreateClothingTex(*m_RenderInfo.re);
					m_AnimalMale->CreateAnimalVAO(*m_RenderInfo.re);
				}
				AuMatrix4x4 m(m_Model);
				m.Translate(1,0,0);
				m_AnimalMale->RenderAnimal(projection, m, *m_RenderInfo.re, false, 1, m_AmbientUp);
			}

			// Transparent pass
			if(m_AnimalFemale)
			{
				AuMatrix4x4 m(m_Model);
				m.Translate(-1,0,0);
				m_AnimalFemale->RenderAnimal(projection, m, *m_RenderInfo.re, true, 1, m_AmbientUp);
			}
			if(m_AnimalMale)
			{
				AuMatrix4x4 m(m_Model);
				m.Translate(1,0,0);
				m_AnimalMale->RenderAnimal(projection, m, *m_RenderInfo.re, true, 1, m_AmbientUp);
			}
			break;

		case DNAPair:
			// Solid pass
			if(m_AnimalFemale)
			{
				if(!m_AnimalFemale->HasVAOs())
				{
					m_AnimalFemale->CreateClothingTex(*m_RenderInfo.re);
					m_AnimalFemale->CreateAnimalVAO(*m_RenderInfo.re);
				}
				AuMatrix4x4 m(m_Model);
				m.Translate(-.5*FeetPerMeter(),0,cgHeightFemale);
				m_AnimalFemale->RenderAnimal(projection, m, *m_RenderInfo.re, false, 1, m_AmbientUp);
			}
			if(m_AnimalMale)
			{
				if(!m_AnimalMale->HasVAOs())
				{
					m_AnimalMale->CreateClothingTex(*m_RenderInfo.re);
					m_AnimalMale->CreateAnimalVAO(*m_RenderInfo.re);
				}
				AuMatrix4x4 m(m_Model);
				m.Translate(.5*FeetPerMeter(),0,cgHeightMale);
				m_AnimalMale->RenderAnimal(projection, m, *m_RenderInfo.re, false, 1, m_AmbientUp);
			}

			// Transparent pass.
			if(m_AnimalFemale)
			{
				AuMatrix4x4 m(m_Model);
				m.Translate(-.5*FeetPerMeter(),0,cgHeightFemale);
				m_AnimalFemale->RenderAnimal(projection, m, *m_RenderInfo.re, true, 1, m_AmbientUp);
			}
			if(m_AnimalMale)
			{
				AuMatrix4x4 m(m_Model);
				m.Translate(.5*FeetPerMeter(),0,cgHeightMale);
				m_AnimalMale->RenderAnimal(projection, m, *m_RenderInfo.re, true, 1, m_AmbientUp);
			}

			// Cleanup
			if(m_AnimalFemaleDelete)
			{
				m_AnimalFemaleDelete->DeleteGL();
				delete m_AnimalFemaleDelete;
				m_AnimalFemaleDelete = 0;
			}
			if(m_AnimalMaleDelete)
			{
				m_AnimalMaleDelete->DeleteGL();
				delete m_AnimalMaleDelete;
				m_AnimalMaleDelete = 0;
			}
			break;

		case Item:
			if(m_Item)
			{
				// Render item.
				AuMatrix4x4 m;
				m.Translate(0,0, g_Au.Commodity(m_Item->CommodityId()).m_insertHeight);
				m_Item->Render(projection, m_Model * m, *m_RenderInfo.re, m_RenderInfo.Models(), AuId(1), d_Ambient, d_Ambient,
						d_Ambient, m_AmbientUp, AuPoint(0,0,1));
			}
			break;

		case Body:
			if(m_Body)
			{
				// Render body.
				AuMatrix4x4 m;
				m.Translate(0,0,m_Body->CGHeight(true));

				float speedEncumberance = 1;	// none
				m_Body->CreateDeviceObjects(*m_RenderInfo.re, m_RenderInfo.Models(), CID_Invalid, CID_Invalid, speedEncumberance);
				m_Body->AnimatePose(false, speedEncumberance);
				m_Body->RenderSkin(projection, m_Model * m, *m_RenderInfo.re, m_AmbientUp, speedEncumberance);
			}
			break;

		case Commodity:
			/*
			if(m_3dsModelCID != CID_Invalid)
			{
				if(!m_3dsModel)
				{
					QString modelFile = g_Au.CommodityModel(m_3dsModelCID);
					m_3dsModel = m_RenderInfo.re->LoadModelGLSL(modelFile, true);    // Does not create the OpenGL display list.
				}
				else if(!m_3dsModel->HasGLModel())
					m_3dsModel->GLModel(*m_RenderInfo.re);

				if(m_3dsModel && m_3dsModel->HasGLModel())
				{
					AuPoint min,max;
					m_3dsModel->GetExtents(min, max);

					AuMatrix4x4 m;
					m.Translate(0,0,-min.z);

					m_3dsModel->RenderGLSLModelTex(projection, m_Model * m, *m_RenderInfo.re, d_Ambient, d_Ambient, d_Ambient,
							m_AmbientUp, m_3dsModelState);
				}
			}
			*/
			break;

		case ModelFile:
			/*
			if(!m_3dsModelFile.isEmpty())
			{
				if(!m_3dsModel)
					m_3dsModel = m_RenderInfo.re->LoadModelGLSL(m_3dsModelFile, true);    // Does not create OpenGL display list.
				else if(!m_3dsModel->HasGLModel())
					m_3dsModel->GLModel(*m_RenderInfo.re);

				if(m_3dsModel && m_3dsModel->HasGLModel())
				{
					AuPoint min,max;
					m_3dsModel->GetExtents(min, max);

					AuMatrix4x4 m;
					m.Translate(0,0,-min.z);

					m_3dsModel->RenderGLSLModelTex(projection, m_Model * m, *m_RenderInfo.re, d_Ambient, d_Ambient, d_Ambient,
							m_AmbientUp, m_3dsModelState);
				}
			}
			*/
			break;
		}

		// Paint the ground VAO.
		if(m_ShowGround && m_Ground.VAO())
		{
			m_RenderInfo.re->GLSL().ModelTexShader().Select(projection, *m_RenderInfo.re);
			if(m_ShowGround == 6)
			{
				AuMatrix4x4 mwall(m_Model);
				mwall.RotateX(-PI/2);
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(mwall);
			}
			else
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(m_Model);

			if(m_ShowGround == 5 || m_ShowGround == 6 || m_ShowGround == 7)
			{
				m_RenderInfo.re->glEnable(GL_BLEND);
				m_RenderInfo.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				m_RenderInfo.re->glDepthMask(GL_FALSE);
				m_RenderInfo.re->GLSL().ModelTexShader().SetGlowAlpha(0, .75f, 0, .5f);
			}
			else
				m_RenderInfo.re->GLSL().ModelTexShader().SetGlowAlpha(0, 0, 0, 1);

			if(m_ShowGround == 1)
				m_RenderInfo.re->GLSL().ModelTexShader().SetTexture(m_DirtTex);
			else if(m_ShowGround == 2)
				m_RenderInfo.re->GLSL().ModelTexShader().SetTexture(m_GrassTex);
			else if(m_ShowGround == 3)
				m_RenderInfo.re->GLSL().ModelTexShader().SetTexture(m_ConcreteTex);
			else if(m_ShowGround == 4)
				m_RenderInfo.re->GLSL().ModelTexShader().SetTexture(m_TarmacTex);
			else
				m_RenderInfo.re->GLSL().ModelTexShader().SetTexture(m_GridTex);

			// Ambient light
			m_RenderInfo.re->GLSL().ModelTexShader().SetAmbientLight(d_Ambient, d_Ambient, d_Ambient);
			m_RenderInfo.re->GLSL().ModelTexShader().SetAmbientLightUp(m_AmbientUp.x, m_AmbientUp.y, m_AmbientUp.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in eye space.
				const AuGLSun& sun = m_RenderInfo.re->Sun(sunIndex);

				// Configure shader for object space lighting.
				m_RenderInfo.re->GLSL().ModelTexShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR,
						sun.m_ColorG, sun.m_ColorB, sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<m_RenderInfo.re->MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in eye space.
				const AuGLLight& light = m_RenderInfo.re->Light(lightIndex);

				// Configure shader for object space lighting.
				m_RenderInfo.re->GLSL().ModelTexShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ,
						light.m_ColorR, light.m_ColorG, light.m_ColorB, light.m_Position.x, light.m_Position.y, light.m_Position.z,
						light.m_cosOuter, light.m_cosInner, light.m_Direction.x, light.m_Direction.y, light.m_Direction.z);
			}

			// Render the model.
			m_Ground.SelectGL();
			m_Ground.DrawGL();
			if(m_ShowGround == 5)
			{
				AuMatrix4x4 mwall(m_Model);
				mwall.RotateX(PI);
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(mwall);
				m_Ground.DrawGL();
			}
			else if(m_ShowGround == 6)
			{
				AuMatrix4x4 mwall(m_Model);
				mwall.RotateX(PI/2);
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(mwall);
				m_Ground.DrawGL();
			}
			else if(m_ShowGround == 7)
			{
				AuMatrix4x4 mwall(m_Model);
				mwall.RotateX(PI/2);
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(mwall);
				m_Ground.DrawGL();
				mwall.RotateX(PI/2);
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(mwall);
				m_Ground.DrawGL();
				mwall.RotateX(PI/2);
				m_RenderInfo.re->GLSL().ModelTexShader().SetModelView(mwall);
				m_Ground.DrawGL();
			}
			m_Ground.DeselectGL();

			m_RenderInfo.re->GLSL().ModelTexShader().Deselect();
			if(m_ShowGround == 5 || m_ShowGround == 6 || m_ShowGround == 7)
			{
				m_RenderInfo.re->glDepthMask(GL_TRUE);
				m_RenderInfo.re->glDisable(GL_BLEND);
			}
		}
	}

	update();
}

void AuView::wheelEvent(QWheelEvent *e)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif

	if(zDelta > 0)
	{
		bool bfast = false;
		if(e->modifiers() & Qt::ControlModifier)
			bfast = true;

		double dist = 1.0;
		if(bfast)
			dist *= 10.0;

		if(m_ViewMode == MeatProfile)
			dist *= 10;

		AuPoint vec = m_Model.UnProjectVector(AuPoint(0,0,dist));
		m_Model.Translate(vec);
	}
	else if(zDelta < 0)
	{
		bool bfast = false;
		if(e->modifiers() & Qt::ControlModifier)
			bfast = true;

		double dist = 1.0;
		if(bfast)
			dist *= 10.0;

		if(m_ViewMode == MeatProfile)
			dist *= 10;

		AuPoint vec = m_Model.UnProjectVector(AuPoint(0,0,-dist));
		m_Model.Translate(vec);
	}

	QOpenGLWidget::wheelEvent(e);
}

void AuView::mousePressEvent(QMouseEvent* e)
{
	QPoint point = e->pos();
	m_MouseButtons = e->buttons();
	m_MouseLast = point;

	if(m_MouseButtons == Qt::LeftButton && m_ViewMode == MeatProfile)
	{
		switch(m_MeatProfileInput)
		{
		case InputNothing:
			break;
		case InputFrontProfile:
			m_MeatProfileFront.append(PointAtPixel(e->pos()));
			m_MeatProfileFrontVAOValid = false;
			break;
		case InputFrontJoints:
			m_MeatJointsFront.append(PointAtPixel(e->pos()));
			break;
		case InputSideProfile:
			m_MeatProfileSide.append(PointAtPixel(e->pos()));
			m_MeatProfileSideVAOValid = false;
			break;
		case InputSideJoints:
			m_MeatJointsSide.append(PointAtPixel(e->pos()));
			break;
		}
	}

	QOpenGLWidget::mousePressEvent(e);
}

void AuView::mouseMoveEvent(QMouseEvent* e)
{
	QOpenGLWidget::mouseMoveEvent(e);

	float xdelta = e->pos().x() - m_MouseLast.x();
	float ydelta = e->pos().y() - m_MouseLast.y();
	if(xdelta == 0 && ydelta == 0)
		return;

	AuPoint loc = m_Model.Translation();

	if(m_MouseButtons == Qt::LeftButton)
	{
		if(m_ViewMode == MeatProfile)
		{
			// Trace the profile.
			//DHH
		}
		else
		{
			// Rotate.
			if(ydelta != 0)
			{
				float yang = ydelta * DEG2RAD;

				AuMatrix4x4 m;
				m.Translate(loc);
				m.RotateX(yang);
				m.Translate(-loc);

				m_Model = m * m_Model;
			}

			if(xdelta != 0)
			{
				float xang = xdelta * DEG2RAD;

				/*
				AuMatrix4x4 m;
				m.Translate(loc);
				m.RotateY(xang);
				m.Translate(-loc);

				m_Model = m * m_Model;
				*/

				m_Model.RotateZ(xang);
			}
		}
	}
	else if(m_MouseButtons == Qt::RightButton)
	{
		// Pan.
		if(ydelta != 0)
		{
			float dist = ydelta/100.f;

			if(m_ViewMode == MeatProfile)
				dist *= 100;

			AuMatrix4x4 m;
			m.Translate(loc);
			m.Translate(0,-dist,0);
			m.Translate(-loc);

			m_Model = m * m_Model;
		}

		if(xdelta != 0)
		{
			float dist = xdelta/100.f;

			if(m_ViewMode == MeatProfile)
				dist *= 100;

			AuMatrix4x4 m;
			m.Translate(loc);
			m.Translate(dist,0,0);
			m.Translate(-loc);

			m_Model = m * m_Model;
		}
	}

	m_MouseLast = e->pos();
}

void AuView::mouseDoubleClickEvent(QMouseEvent* e)
{
	if(m_ViewMode == MeatProfile)
	{
		AuPoint o = m_Model.Translation();

		AuPoint nuo = PointAtPixel(e->pos());

		m_Model.LoadIdentity();
		m_Model.Translate(-nuo.x, -nuo.y, o.z);
	}
}

AuPoint AuView::PointAtPixel(const QPoint& pixel) const
{
	// Configure the device transform for this window.
	QRect rcClient = rect();
	AuMatrix4x4 tDevice;
	tDevice.Scale(rcClient.width()/2.0, -rcClient.height()/2.0, rcClient.width()/2.0);
	tDevice.Translate(1.0, -1.0, 0.0);

	AuMatrix4x4 tPersp;
	if(rect().width() > 0)
		tPersp.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), 1, 10000);
	else
		tPersp.Perspective(45.0*DEG2RAD, 1, 1, 10000);
	tPersp *= m_Model;

	AuMatrix4x4 tDevicePersp = tDevice * tPersp;

	AuPoint fProj;
	tDevicePersp.UnProjectIntoPlane(fProj, pixel, AuMatrix4x4());
	return fProj;
}

void AuView::ResetView()
{
	m_Model.LoadIdentity();
	switch(m_ViewMode)
	{
	case None:
		break;

	case Animalia:
	case DNAPair:
	case ModelFile:
		m_Model.Translate(0,-3,-8);
		m_Model.RotateX(-90*DEG2RAD);
		m_Model.RotateZ(180*DEG2RAD);
		break;

	case Body:
		m_Model.Translate(0,-3,-8);
		m_Model.RotateX(-90*DEG2RAD);
		m_Model.RotateZ(180*DEG2RAD);
		break;

	case Commodity:
		m_Model.Translate(0,-5,-30);
		m_Model.RotateX(-90*DEG2RAD);
		m_Model.RotateZ(180*DEG2RAD);
		break;

	case Item:
		m_Model.Translate(0,-3,-8);
		m_Model.RotateX(-90*DEG2RAD);
		m_Model.RotateZ(180*DEG2RAD);
		break;

	case MeatProfile:
		if(m_MeatImageSize.width() > 0)
			m_Model.Translate(0,0,-m_MeatImageSize.width());
		break;
	}
}

// Deletes the heap objects but does not clean up the open gl objects.
void AuView::DeleteViewObjects()
{
	if(m_AnimalFemale)
	{
		delete m_AnimalFemale;
		m_AnimalFemale = 0;
	}

	if(m_AnimalMale)
	{
		delete m_AnimalMale;
		m_AnimalMale = 0;
	}

	if(m_Body)
	{
		delete m_Body;
		m_Body = 0;
	}

	if(m_Item)
	{
		delete m_Item;
		m_Item = 0;
	}

	if(m_3dsModel)
	{
		delete m_3dsModel;
		m_3dsModel = 0;
	}

	m_3dsModelCID = CID_Invalid;
	m_3dsModelFile = "";
}

void AuView::ViewAnimal(const AuDNA& dna, GENDER gender, HANDEDNESS handedness)
{
	if(gender == Male)
	{
		if(m_AnimalMale && m_AnimalMale->Is(dna, AuDNA::Alpha, handedness))
			return;
		//else if(!m_AnimalMale && !m_AnimalFemale)
			//DeleteViewObjects();	// Delete old render items from different mode.

		// View the new animal.
		m_AnimalMale = new AuAnimalia(dna, AuDNA::Alpha, handedness, Mature);
		m_AnimalMale->Pose(AuId(Male), AuPose::Stand, true, 1);

		AuItem hat(m_AnimalMaleHat, 192, 0, 0);
		m_AnimalMale->SetHat(&hat);
		AuItem clothes(m_AnimalMaleClothes, 192, 0, 0);
		m_AnimalMale->SetClothing(&clothes);
		AuItem armor(m_AnimalMaleArmor, 192, 0, 0);
		m_AnimalMale->SetArmor(&armor);
	}
	else
	{
		if(m_AnimalFemale && m_AnimalFemale->Is(dna, AuDNA::Beta, handedness))
			return;
		//else if(!m_AnimalMale && !m_AnimalFemale)
			//DeleteViewObjects();	// Delete old render items from different mode.

		// View the new animal.
		m_AnimalFemale = new AuAnimalia(dna, AuDNA::Beta, handedness, Mature);
		m_AnimalFemale->Pose(AuId(Female), AuPose::Stand, true, 1);

		AuItem hat(m_AnimalFemaleHat, 192, 0, 0);
		m_AnimalFemale->SetHat(&hat);
		AuItem clothes(m_AnimalFemaleClothes, 192, 0, 0);
		m_AnimalFemale->SetClothing(&clothes);
		AuItem armor(m_AnimalFemaleArmor, 192, 0, 0);
		m_AnimalFemale->SetArmor(&armor);
	}
	m_ViewMode = Animalia;
	update();
}

void AuView::ViewItem(COMMODITYID cid)
{
	// Do nothing if this is not a change.
	if((m_Item && m_Item->CommodityId() == cid) ||
			(m_3dsModel && m_3dsModelCID == cid))
		return;

	// Delete old render items.
	DeleteViewObjects();

	// View the new item.
	if(DbCommodity::IsVehicle(cid))
	{
		m_3dsModelCID = cid;
		m_ViewMode = Commodity;
	}
	else
	{
		m_Item = AuItem::Create(cid, 128, 0, 0);
		m_ViewMode = Item;
	}
	update();
}

void AuView::ViewModel(const QString& fileName)
{
	// Delete old render items.
	DeleteViewObjects();

	// View the new model.
	m_3dsModelFile = fileName;
	m_ViewMode = ModelFile;
	update();
}

void AuView::ViewBody(int gender, bool isLefty, const DhDNA& dna)
{
	// Do nothing if this is not a change.
	if(m_Body && m_Body->DNA() == dna &&
			m_Body->Sex() == gender &&
			m_Body->IsLeftHanded() == isLefty)
		return;

	// Delete old render items.
	DeleteViewObjects();

	// View the new body.
	m_Body = new DpBody(gender, isLefty, dna);
	float speedEncumberance = 1;	// none
	m_Body->Pose(AuPose::Stand, false, speedEncumberance);
	m_ViewMode = Body;
	update();
}

void AuView::ItemToggleState()
{
	if(m_Item)
		m_Item->SetState(!m_Item->State());
	else if(m_3dsModelCID != CID_Invalid)
		m_3dsModelState = !m_3dsModelState;
}

void AuView::ModelToggleState()
{
	if(!m_3dsModelFile.isEmpty())
		m_3dsModelState = !m_3dsModelState;
}

void AuView::BodyStand()
{
	if(m_Body)
	{
		float speedEncumberance = 1;	// none
		m_Body->Pose(AuPose::Stand, false, speedEncumberance);
	}
}

void AuView::BodyWalk()
{
	if(m_Body)
	{
		float speedEncumberance = 1;	// none
		m_Body->Pose(AuPose::StandWalk, false, speedEncumberance);
	}
}

void AuView::BodyRun()
{
	if(m_Body)
	{
		float speedEncumberance = 1;	// none
		m_Body->Pose(AuPose::StandRun, false, speedEncumberance);
	}
}

void AuView::ViewLines()
{
	m_ShowMode = 0;
}

void AuView::ViewLinesAndSolid()
{
	m_ShowMode = 2;
}

void AuView::ViewSolid()
{
	m_ShowMode = 1;
}

void AuView::ViewSolidMeshAndIntersections()
{
	m_ShowMode = 3;
}

void AuView::MeatProfileInputFrontJoints()
{
	m_MeatProfileInput = InputFrontJoints;
}

void AuView::MeatProfileInputFrontProfile()
{
	m_MeatProfileInput = InputFrontProfile;
}

void AuView::MeatProfileInputSideJoints()
{
	m_MeatProfileInput = InputSideJoints;
}

void AuView::MeatProfileInputSideProfile()
{
	m_MeatProfileInput = InputSideProfile;
}

void AuView::MeatProfileStopInput()
{
	m_MeatProfileInput = InputNothing;
}

void AuView::MeatImageToggle()
{
	if(m_ShowMeatImage == 1)
		m_ShowMeatImage = 0;
	else
		m_ShowMeatImage = 1;
}

void AuView::DirtToggle()
{
	if(m_ShowGround == 1)
		m_ShowGround = 0;
	else
		m_ShowGround = 1;
}

void AuView::GrassToggle()
{
	if(m_ShowGround == 2)
		m_ShowGround = 0;
	else
		m_ShowGround = 2;
}

void AuView::ConcreteToggle()
{
	if(m_ShowGround == 3)
		m_ShowGround = 0;
	else
		m_ShowGround = 3;
}

void AuView::TarmacToggle()
{
	if(m_ShowGround == 4)
		m_ShowGround = 0;
	else
		m_ShowGround = 4;
}

void AuView::MeatProfileEnd()
{

	switch(m_MeatProfileInput)
	{
	case InputNothing:
	case InputFrontJoints:
	case InputSideJoints:
		break;
	case InputFrontProfile:
		if(!m_MeatProfileFront.isEmpty())
		{
			m_MeatProfileFront.append(m_MeatProfileFront.first());
			m_MeatProfileFrontVAOValid = false;
		}
		break;
	case InputSideProfile:
		if(!m_MeatProfileSide.isEmpty())
		{
			m_MeatProfileSide.append(m_MeatProfileSide.first());
			m_MeatProfileSideVAOValid = false;
		}
		break;
	}
}

void AuView::MeatProfileBackspace()
{
	switch(m_MeatProfileInput)
	{
	case InputNothing:
		break;
	case InputFrontProfile:
		if(!m_MeatProfileFront.isEmpty())
		{
			m_MeatProfileFront.removeLast();
			m_MeatProfileFrontVAOValid = false;
		}
		break;
	case InputFrontJoints:
		if(!m_MeatJointsFront.isEmpty())
			m_MeatJointsFront.removeLast();
		break;
	case InputSideProfile:
		if(!m_MeatProfileSide.isEmpty())
		{
			m_MeatProfileSide.removeLast();
			m_MeatProfileSideVAOValid = false;
		}
		break;
	case InputSideJoints:
		if(!m_MeatJointsSide.isEmpty())
			m_MeatJointsSide.removeLast();
		break;
	}

}

void AuView::MeatProfileChooseImage()
{
	QString lastFile = g_Au.Settings().readEntry("/MeatProfileImage", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("Image File (*.png *.bmp *.jpg)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Most Any Image File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("/MeatProfileImage", loadFile);
	m_MeatProfileImage = loadFile;
}

void AuView::NewMeatProfile()
{
	m_MeatProfileFront.clear();
	m_MeatJointsFront.clear();
	m_MeatProfileSide.clear();
	m_MeatJointsSide.clear();
	m_MeatProfileImage = "";

	m_ViewMode = MeatProfile;
	update();
}

void AuView::SaveMeatProfile(const QString& fileName)
{
	if(m_ViewMode != MeatProfile)
		return;

	QFile file(fileName);
	if(file.open(QFile::WriteOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_4_1);

		ds << (quint8)MEATPROFILE_FILE_TYPE;
		ds << (quint8)MEATPROFILE_FILE_VER;

		// The meat profile data.
		ds << m_MeatProfileFront;
		ds << m_MeatJointsFront;
		ds << m_MeatProfileSide;
		ds << m_MeatJointsSide;

		// Additional information.
		ds << m_MeatProfileImage;
		ds << m_Model;

		file.close();
	}
}

void AuView::SaveMeatLine(const QString& fileName)
{
	if(m_ViewMode != MeatProfile)
		return;

	QFile file(fileName);
	if(file.open(QFile::WriteOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_4_1);

		ds << (quint8)MEATLINE_FILE_TYPE;
		ds << (quint8)MEATLINE_FILE_VER;

		ds << m_MeatProfileFront;
		ds << m_MeatJointsFront;

		file.close();
	}
}

void AuView::LoadMeatProfile(const QString& fileName)
{
	QFile file(fileName);
	if(file.open(QFile::ReadOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_4_1);

		quint8 fileType, fileVer;
		ds >> fileType;
		ds >> fileVer;

		if(fileType == MEATPROFILE_FILE_TYPE && fileVer == MEATPROFILE_FILE_VER)
		{
			// The meat profile data.
			ds >> m_MeatProfileFront;
			ds >> m_MeatJointsFront;
			ds >> m_MeatProfileSide;
			ds >> m_MeatJointsSide;

			// Additional information.
			ds >> m_MeatProfileImage;
			ds >> m_Model;

			m_ViewMode = MeatProfile;
			update();
			m_MeatProfileFrontVAOValid = false;
			m_MeatProfileSideVAOValid = false;
		}

		file.close();
	}
}

void AuView::LoadMeatLine(const QString& fileName)
{
	QFile file(fileName);
	if(file.open(QFile::ReadOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_4_1);

		quint8 fileType, fileVer;
		ds >> fileType;
		ds >> fileVer;

		if(fileType == MEATLINE_FILE_TYPE && fileVer == MEATLINE_FILE_VER)
		{
			// The meat profile data.
			if(m_MeatProfileInput == InputSideProfile)
			{
				ds >> m_MeatProfileSide;
				ds >> m_MeatJointsSide;
			}
			else
			{
				ds >> m_MeatProfileFront;
				ds >> m_MeatJointsFront;
			}

			m_ViewMode = MeatProfile;
			update();
			m_MeatProfileFrontVAOValid = false;
			m_MeatProfileSideVAOValid = false;
		}

		file.close();
	}
}

void AuView::MeatProfileClear()
{
	switch(m_MeatProfileInput)
	{
	case InputNothing:
		{
			QString smsg = tr("<html>The front and side meat profiles will be lost. The joints will not change.</html>");
			if(QMessageBox::critical(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
				return;
			//m_MeatJointsFront.clear();
			m_MeatProfileFront.clear();
			m_MeatProfileFrontVAOValid = false;
			//m_MeatJointsSide.clear();
			m_MeatProfileSide.clear();
			m_MeatProfileSideVAOValid = false;
		}
		break;

	case InputFrontJoints:
		{
			QString smsg = tr("<html>The front joints will be lost.</html>");
			if(QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
				return;
			m_MeatJointsFront.clear();
		}
		break;

	case InputFrontProfile:
		{
			QString smsg = tr("<html>The front meat profile will be lost.</html>");
			if(QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
				return;
			m_MeatProfileFront.clear();
			m_MeatProfileFrontVAOValid = false;
		}
		break;

	case InputSideJoints:
		{
			QString smsg = tr("<html>The side joints will be lost.</html>");
			if(QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
				return;
			m_MeatJointsSide.clear();
		}
		break;

	case InputSideProfile:
		{
			QString smsg = tr("<html>The side meat profile will be lost.</html>");
			if(QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
				return;
			m_MeatProfileSide.clear();
			m_MeatProfileSideVAOValid = false;
		}
		break;
	}
}

void AuView::MeatProfileScale()
{
	switch(m_MeatProfileInput)
	{
	case InputNothing:
	case InputFrontJoints:
	case InputSideJoints:
		break;

	case InputFrontProfile:
		if(!m_MeatJointsFront.isEmpty())
		{
			QString smsg = tr("<html>The front profile will be scaled.</html>");
			QString scale = QInputDialog::getText(this, g_Au.ApplicationName(), tr("Enter a scale vector."),
					QLineEdit::Normal, "1, 1");
			if(scale.isEmpty())
				return;

			AuPoint scl(scale);
			if(scl.x == 0)
				scl.x = 1;
			if(scl.y == 0)
				scl.y = 1;

			AuPoint origin = m_MeatJointsFront[0];
			for(AuPointArray::iterator pi = m_MeatProfileFront.begin(); pi != m_MeatProfileFront.end(); pi++)
			{
				*pi -= origin;
				(*pi).x *= scl.x;
				(*pi).y *= scl.y;
				*pi += origin;
			}
			m_MeatProfileFrontVAOValid = false;
		}
		break;

	case InputSideProfile:
		if(!m_MeatJointsSide.isEmpty())
		{
			QString smsg = tr("<html>The side profile will be scaled.</html>");
			QString scale = QInputDialog::getText(this, g_Au.ApplicationName(), tr("Enter a scale vector."),
					QLineEdit::Normal, "1, 1");
			if(scale.isEmpty())
				return;

			AuPoint scl(scale);
			if(scl.x == 0)
				scl.x = 1;
			if(scl.y == 0)
				scl.y = 1;

			AuPoint origin = m_MeatJointsSide[0];
			for(AuPointArray::iterator pi = m_MeatProfileSide.begin(); pi != m_MeatProfileSide.end(); pi++)
			{
				*pi -= origin;
				(*pi).x *= scl.x;
				(*pi).y *= scl.y;
				*pi += origin;
			}
			m_MeatProfileSideVAOValid = false;
		}
		break;
	}
}

void AuView::ViewRestore()
{
	QByteArray ba;
	g_Au.Settings().readEntry("/ViewMatrix", ba);

	QDataStream ds(ba);
	ds.setVersion(QDataStream::Qt_4_1);

	ds >> m_Model;

	update();
}

void AuView::ViewSave()
{
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.setVersion(QDataStream::Qt_4_1);

	ds << m_Model;

	g_Au.Settings().writeEntry("/ViewMatrix", ba);
	//qDebug() << "View matrix saved";
}

void AuView::SetDNAPair(const AuDNA& dna)
{
	// Male
	HANDEDNESS handedness = RightHanded;
	if(m_AnimalMale && !m_AnimalMale->Is(dna, AuDNA::Alpha, handedness))
	{
		m_AnimalMaleDelete = m_AnimalMale;
		m_AnimalMale = 0;
	}
	if(!m_AnimalMale)
	{
		m_AnimalMale = new AuAnimalia(dna, AuDNA::Alpha, handedness, Mature);
		m_AnimalMale->Pose(AuId(Male), m_AnimalMalePose, true, 1);
		m_AnimalMale->SetHairStyle(m_AnimalMaleHair, m_AnimalMaleShave);
		m_AnimalMale->PoseMouth(AuId(Male), m_AnimalMaleMouth);

		AuItem hat(m_AnimalMaleHat, 192, 0, 0);
		m_AnimalMale->SetHat(&hat);
		AuItem clothes(m_AnimalMaleClothes, 192, 0, 0);
		m_AnimalMale->SetClothing(&clothes);
		AuItem armor(m_AnimalMaleArmor, 192, 0, 0);
		m_AnimalMale->SetArmor(&armor);
	}

	// Female
	handedness = LeftHanded;
	if(m_AnimalFemale && !m_AnimalFemale->Is(dna, AuDNA::Beta, handedness))
	{
		m_AnimalFemaleDelete = m_AnimalFemale;
		m_AnimalFemale = 0;
	}
	if(!m_AnimalFemale)
	{
		m_AnimalFemale = new AuAnimalia(dna, AuDNA::Beta, handedness, Mature);
		m_AnimalFemale->Pose(AuId(Female), m_AnimalFemalePose, true, 1);
		m_AnimalFemale->SetHairStyle(m_AnimalFemaleHair, m_AnimalFemaleShave);
		m_AnimalFemale->PoseMouth(AuId(Female), m_AnimalFemaleMouth);

		AuItem hat(m_AnimalFemaleHat, 192, 0, 0);
		m_AnimalFemale->SetHat(&hat);
		AuItem clothes(m_AnimalFemaleClothes, 192, 0, 0);
		m_AnimalFemale->SetClothing(&clothes);
		AuItem armor(m_AnimalFemaleArmor, 192, 0, 0);
		m_AnimalFemale->SetArmor(&armor);
	}

	m_DNA = dna;
	if(m_ViewMode != DNAPair)
	{
		m_ViewMode = DNAPair;
		ResetView();
	}
	update();
}

void AuView::SetDNAPair(const AuDNA& dna, int modelchg)
{
	// Male
	HANDEDNESS handedness = RightHanded;
	if(m_AnimalMale && (modelchg & 0x01))
	{
		m_AnimalMaleDelete = m_AnimalMale;
		m_AnimalMale = 0;
	}
	if(!m_AnimalMale)
	{
		m_AnimalMale = new AuAnimalia(dna, AuDNA::Alpha, handedness, Mature);
		m_AnimalMale->Pose(AuId(Male), m_AnimalMalePose, true, 1);

		AuItem hat(m_AnimalMaleHat, 192, 0, 0);
		m_AnimalMale->SetHat(&hat);
		AuItem clothes(m_AnimalMaleClothes, 192, 0, 0);
		m_AnimalMale->SetClothing(&clothes);
		AuItem armor(m_AnimalMaleArmor, 192, 0, 0);
		m_AnimalMale->SetArmor(&armor);
	}
	m_AnimalMale->SetDNA(dna, AuDNA::Alpha);
	m_AnimalMale->SetHairStyle(m_AnimalMaleHair, m_AnimalMaleShave);
	m_AnimalMale->PoseMouth(AuId(Male), m_AnimalMaleMouth);

	// Female
	handedness = LeftHanded;
	if(m_AnimalFemale && (modelchg & 0x02))
	{
		m_AnimalFemaleDelete = m_AnimalFemale;
		m_AnimalFemale = 0;
	}
	if(!m_AnimalFemale)
	{
		m_AnimalFemale = new AuAnimalia(dna, AuDNA::Beta, handedness, Mature);
		m_AnimalFemale->Pose(AuId(Female), m_AnimalFemalePose, true, 1);

		AuItem hat(m_AnimalFemaleHat, 192, 0, 0);
		m_AnimalFemale->SetHat(&hat);
		AuItem clothes(m_AnimalFemaleClothes, 192, 0, 0);
		m_AnimalFemale->SetClothing(&clothes);
		AuItem armor(m_AnimalFemaleArmor, 192, 0, 0);
		m_AnimalFemale->SetArmor(&armor);
	}
	m_AnimalFemale->SetDNA(dna, AuDNA::Beta);
	m_AnimalFemale->SetHairStyle(m_AnimalFemaleHair, m_AnimalFemaleShave);
	m_AnimalFemale->PoseMouth(AuId(Female), m_AnimalFemaleMouth);

	m_DNA = dna;
	if(m_ViewMode != DNAPair)
	{
		m_ViewMode = DNAPair;
		ResetView();
	}
	update();
}

void AuView::SetMaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave)
{
	if(m_AnimalMaleHair == cut && m_AnimalMaleShave == shave)
		return;
	m_AnimalMaleHair = cut;
	m_AnimalMaleShave = shave;
	if(m_AnimalMale)
	{
		m_AnimalMaleDelete = m_AnimalMale;
		m_AnimalMale = 0;
	}

	HANDEDNESS handedness = RightHanded;

	m_AnimalMale = new AuAnimalia(m_DNA, AuDNA::Alpha, handedness, Mature);
	m_AnimalMale->Pose(AuId(Male), m_AnimalMalePose, true, 1);
	m_AnimalMale->SetHairStyle(m_AnimalMaleHair, m_AnimalMaleShave);
	m_AnimalMale->PoseMouth(AuId(Male), m_AnimalMaleMouth);

	AuItem hat(m_AnimalMaleHat, 192, 0, 0);
	m_AnimalMale->SetHat(&hat);
	AuItem clothes(m_AnimalMaleClothes, 192, 0, 0);
	m_AnimalMale->SetClothing(&clothes);
	AuItem armor(m_AnimalMaleArmor, 192, 0, 0);
	m_AnimalMale->SetArmor(&armor);

	update();
}

void AuView::SetFemaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave)
{
	if(m_AnimalFemaleHair == cut && m_AnimalFemaleShave == shave)
		return;
	m_AnimalFemaleHair = cut;
	m_AnimalFemaleShave = shave;
	if(m_AnimalFemale)
	{
		m_AnimalFemaleDelete = m_AnimalFemale;
		m_AnimalFemale = 0;
	}

	HANDEDNESS handedness = LeftHanded;

	m_AnimalFemale = new AuAnimalia(m_DNA, AuDNA::Beta, handedness, Mature);
	m_AnimalFemale->Pose(AuId(Female), m_AnimalFemalePose, true, 1);
	m_AnimalFemale->SetHairStyle(m_AnimalFemaleHair, m_AnimalFemaleShave);
	m_AnimalFemale->PoseMouth(AuId(Female), m_AnimalFemaleMouth);

	AuItem hat(m_AnimalFemaleHat, 192, 0, 0);
	m_AnimalFemale->SetHat(&hat);
	AuItem clothes(m_AnimalFemaleClothes, 192, 0, 0);
	m_AnimalFemale->SetClothing(&clothes);
	AuItem armor(m_AnimalFemaleArmor, 192, 0, 0);
	m_AnimalFemale->SetArmor(&armor);

	update();
}

void AuView::SetMaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes)
{
	if(m_AnimalMaleHat == hat &&
			m_AnimalMaleArmor == armor &&
			m_AnimalMaleClothes == clothes)
		return;
	m_AnimalMaleHat = hat;
	m_AnimalMaleArmor = armor;
	m_AnimalMaleClothes = clothes;
	if(m_AnimalMale)
	{
		m_AnimalMaleDelete = m_AnimalMale;
		m_AnimalMale = 0;
	}

	HANDEDNESS handedness = RightHanded;

	m_AnimalMale = new AuAnimalia(m_DNA, AuDNA::Alpha, handedness, Mature);
	m_AnimalMale->Pose(AuId(Male), m_AnimalMalePose, true, 1);
	m_AnimalMale->SetHairStyle(m_AnimalMaleHair, m_AnimalMaleShave);
	m_AnimalMale->PoseMouth(AuId(Male), m_AnimalMaleMouth);

	AuItem hatI(m_AnimalMaleHat, 192, 0, 0);
	m_AnimalMale->SetHat(&hatI);
	AuItem clothesI(m_AnimalMaleClothes, 192, 0, 0);
	m_AnimalMale->SetClothing(&clothesI);
	AuItem armorI(m_AnimalMaleArmor, 192, 0, 0);
	m_AnimalMale->SetArmor(&armorI);

	update();
}

void AuView::SetFemaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes)
{
	if(m_AnimalFemaleHat == hat &&
			m_AnimalFemaleArmor == armor &&
			m_AnimalFemaleClothes == clothes)
		return;
	m_AnimalFemaleHat = hat;
	m_AnimalFemaleArmor = armor;
	m_AnimalFemaleClothes = clothes;
	if(m_AnimalFemale)
	{
		m_AnimalFemaleDelete = m_AnimalFemale;
		m_AnimalFemale = 0;
	}

	HANDEDNESS handedness = LeftHanded;

	m_AnimalFemale = new AuAnimalia(m_DNA, AuDNA::Beta, handedness, Mature);
	m_AnimalFemale->Pose(AuId(Female), m_AnimalFemalePose, true, 1);
	m_AnimalFemale->SetHairStyle(m_AnimalFemaleHair, m_AnimalFemaleShave);
	m_AnimalFemale->PoseMouth(AuId(Female), m_AnimalFemaleMouth);

	AuItem hatI(m_AnimalFemaleHat, 192, 0, 0);
	m_AnimalFemale->SetHat(&hatI);
	AuItem clothesI(m_AnimalFemaleClothes, 192, 0, 0);
	m_AnimalFemale->SetClothing(&clothesI);
	AuItem armorI(m_AnimalFemaleArmor, 192, 0, 0);
	m_AnimalFemale->SetArmor(&armorI);
	update();
}

void AuView::SetFemaleMouthPose(const AuPose& face)
{
	if(m_AnimalFemaleMouth == face)
		return;
	m_AnimalFemaleMouth = face;
	if(!m_AnimalFemale)
		return;
	m_AnimalFemale->PoseMouth(AuId(Female), face);
	update();
}

void AuView::SetFemalePose(const AuPose& pose)
{
	if(m_AnimalFemalePose == pose)
		return;
	m_AnimalFemalePose = pose;
	if(!m_AnimalFemale)
		return;
	m_AnimalFemale->Pose(AuId(Female), pose, true, 1);
	update();
}

void AuView::SetMaleMouthPose(const AuPose& face)
{
	if(m_AnimalMaleMouth == face)
		return;
	m_AnimalMaleMouth = face;
	if(!m_AnimalMale)
		return;
	m_AnimalMale->PoseMouth(AuId(Male), face);
	update();
}

void AuView::SetMalePose(const AuPose& pose)
{
	if(m_AnimalMalePose == pose)
		return;
	m_AnimalMalePose = pose;
	if(!m_AnimalMale)
		return;
	m_AnimalMale->Pose(AuId(Male), pose, true, 1);
	update();
}

void AuView::SetShowGround(int showWhat)
{
	m_ShowGround = showWhat;
	update();
}
