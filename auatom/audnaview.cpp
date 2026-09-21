//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnaview.cpp,v 1.32 2022/05/19 15:07:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audnaview.h"
//
#include "auatomrenderinfo.h"
#include "auglmodelmaker.h"
#include "auglobal.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QWheelEvent>

#define d_Ambient						(AuGLLight::MaxAmbientStarlight()/2)

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAView::AuDNAView(QWidget* parent) : QOpenGLWidget(parent), m_Font(g_Au.TypefaceSans(), 22)
{
	m_MM = new AuGLModelMaker(false);
	m_ModelCache = new AuGLModelCache();
	m_RenderInfo = new AuAtomRenderInfo(this, 0, *m_MM, *m_ModelCache, m_Font);

	setFocusPolicy(Qt::StrongFocus);
	setObjectName("AuDNAView");
	setAttribute(Qt::WA_OpaquePaintEvent);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_Female = 0;
	m_FemaleArmBent = false;
	m_FemaleArmor = CID_Invalid;
	if(g_Au.IsNudityOk())
		m_FemaleClothes = CID_Invalid;
	else
		m_FemaleClothes = CID_Underwear;
	m_FemaleDelete = 0;
	m_FemaleHair = AuHairStyle::Unkempt;
	m_FemaleHat = CID_Invalid;
	m_FemaleMouth = AuPose::MouthClosed;
	m_FemalePose = AuPose::Stand;
	m_FemaleShave = AuHairStyle::Unshaven;
	m_Male = 0;
	m_MaleArmBent = false;
	m_MaleArmor = CID_Invalid;
	if(g_Au.IsNudityOk())
		m_MaleClothes = CID_Invalid;
	else
		m_MaleClothes = CID_Underwear;
	m_MaleDelete = 0;
	m_MaleHair = AuHairStyle::Unkempt;
	m_MaleHat = CID_Invalid;
	m_MaleMouth = AuPose::MouthClosed;
	m_MalePose = AuPose::Stand;
	m_MaleShave = AuHairStyle::Unshaven;
	m_DirtTex = 0;
	m_GrassTex = 0;
	m_MeatImageTex = 0;
	m_MouseButtons = Qt::NoButton;
	m_ShowGround = 0;
}

AuDNAView::~AuDNAView()
{
	delete m_RenderInfo;
	delete m_MM;
	delete m_ModelCache;
}

void AuDNAView::initializeGL()
{
	//qDebug("AuDNAView::initializeGL");
	//QSurfaceFormat fmt = format();
	//qDebug() << "AuDNAView::initializeGL swapInt" << fmt.swapInterval() << "swapBehave" << fmt.swapBehavior()
		//<< "opaquePaint" << testAttribute(Qt::WA_OpaquePaintEvent);

	m_RenderInfo->InitializeGL();

	// Configure for solid rendered display.
	m_RenderInfo->re->glFrontFace(GL_CW);						// Front faces are those drawn clockwise.
	m_RenderInfo->re->glCullFace(GL_BACK);						// Cull back-facing faces.
	m_RenderInfo->re->glEnable(GL_CULL_FACE);					// Enable face culling.
	m_RenderInfo->re->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);					// Enable depth calculations
	m_RenderInfo->re->glClearColor(0.2f, 0.2f, 0.2f, 1.0);		// Let OpenGL clear to black

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

		m_Ground.CreateForModelTexFog(*m_RenderInfo->re, tris, GL_STATIC_DRAW);

		m_ConcreteTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Concrete", ":data/ConcreteHeavy.png",
				GL_LINEAR, GL_LINEAR, true);

		m_DirtTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Dirt", ":data/DirtCrackedModerate.png",
				GL_LINEAR, GL_LINEAR, true);

		m_GrassTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Grass", ":data/Grass.png",
				GL_LINEAR, GL_LINEAR, true);

		m_TarmacTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Tarmac", ":data/Tarmac.png",
				GL_LINEAR, GL_LINEAR, true);

		m_GridTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Grid", ":data/Grid.png",
				GL_LINEAR, GL_LINEAR, true);
	}
}

void AuDNAView::SetupLights(const AuMatrix4x4& model)
{
	m_RenderInfo->re->SetAmbientLight(d_Ambient, d_Ambient, d_Ambient, 1);

	m_RenderInfo->re->DisableSuns();
	m_RenderInfo->re->DisableLights();

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
		m_RenderInfo->re->SetSun(0, atten, sunColor, vl.x, vl.y, vl.z, 1.0);

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
		m_RenderInfo->re->SetSun(0, atten, sunColor, vl.x, vl.y, vl.z, 1.0);

		m_AmbientUp = AuPoint(0,1,0);
	}
}

void AuDNAView::paintGL()
{
	//qDebug() << "AuDNAView::paintGL";
	m_RenderInfo->re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	AuMatrix4x4 projection;
	if(rect().width() > 0)
		projection.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), 2, 10000);
	else
		projection.Perspective(45.0*DEG2RAD, 1, 2, 10000);

	SetupLights(m_Model);

	float cgHeightFemale;
	if(m_Female)
		cgHeightFemale = m_Female->CGHeight();
	else
		cgHeightFemale = 0;
	float cgHeightMale;
	if(m_Male)
		cgHeightMale = m_Male->CGHeight();
	else
		cgHeightMale = 0;

	// Solid pass
	AuElapsedTimer when(AuElapsedTimer::Now());
	if(m_Female)
	{
		if(!m_Female->HasVAOs())
		{
			m_Female->CreateClothingTex(*m_RenderInfo->re);
			m_Female->CreateAnimalVAO(*m_RenderInfo->re);
		}
		m_Female->AnimatePose(AuId(Female), when);
		AuMatrix4x4 m(m_Model);
		m.Translate(-.5*FeetPerMeter(),0,cgHeightFemale);
		m_Female->RenderAnimal(projection, m, *m_RenderInfo->re, false, 1, m_AmbientUp);
	}
	if(m_Male)
	{
		if(!m_Male->HasVAOs())
		{
			m_Male->CreateClothingTex(*m_RenderInfo->re);
			m_Male->CreateAnimalVAO(*m_RenderInfo->re);
		}
		m_Male->AnimatePose(AuId(Male), when);
		AuMatrix4x4 m(m_Model);
		m.Translate(.5*FeetPerMeter(),0,cgHeightMale);
		m_Male->RenderAnimal(projection, m, *m_RenderInfo->re, false, 1, m_AmbientUp);
	}

	// Transparent pass.
	if(m_Female)
	{
		AuMatrix4x4 m(m_Model);
		m.Translate(-.5*FeetPerMeter(),0,cgHeightFemale);
		m_Female->RenderAnimal(projection, m, *m_RenderInfo->re, true, 1, m_AmbientUp);
	}
	if(m_Male)
	{
		AuMatrix4x4 m(m_Model);
		m.Translate(.5*FeetPerMeter(),0,cgHeightMale);
		m_Male->RenderAnimal(projection, m, *m_RenderInfo->re, true, 1, m_AmbientUp);
	}

	// Cleanup
	if(m_FemaleDelete)
	{
		m_FemaleDelete->DeleteGL();
		delete m_FemaleDelete;
		m_FemaleDelete = 0;
	}
	if(m_MaleDelete)
	{
		m_MaleDelete->DeleteGL();
		delete m_MaleDelete;
		m_MaleDelete = 0;
	}

	// Paint the ground VAO.
	if(m_ShowGround && m_Ground.VAO())
	{
		m_RenderInfo->re->GLSL().ModelTexShader().Select(projection, *m_RenderInfo->re);
		if(m_ShowGround == 6)
		{
			AuMatrix4x4 mwall(m_Model);
			mwall.RotateX(-PI/2);
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(mwall);
		}
		else
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(m_Model);

		if(m_ShowGround == 5 || m_ShowGround == 6 || m_ShowGround == 7)
		{
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_RenderInfo->re->glDepthMask(GL_FALSE);
			m_RenderInfo->re->GLSL().ModelTexShader().SetGlowAlpha(0, .75f, 0, .5f);
		}
		else
			m_RenderInfo->re->GLSL().ModelTexShader().SetGlowAlpha(0, 0, 0, 1);

		if(m_ShowGround == 1)
			m_RenderInfo->re->GLSL().ModelTexShader().SetTexture(m_DirtTex);
		else if(m_ShowGround == 2)
			m_RenderInfo->re->GLSL().ModelTexShader().SetTexture(m_GrassTex);
		else if(m_ShowGround == 3)
			m_RenderInfo->re->GLSL().ModelTexShader().SetTexture(m_ConcreteTex);
		else if(m_ShowGround == 4)
			m_RenderInfo->re->GLSL().ModelTexShader().SetTexture(m_TarmacTex);
		else
			m_RenderInfo->re->GLSL().ModelTexShader().SetTexture(m_GridTex);

		// Ambient light
		m_RenderInfo->re->GLSL().ModelTexShader().SetAmbientLight(d_Ambient, d_Ambient, d_Ambient);
		m_RenderInfo->re->GLSL().ModelTexShader().SetAmbientLightUp(m_AmbientUp.x, m_AmbientUp.y, m_AmbientUp.z);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in eye space.
			const AuGLSun& sun = m_RenderInfo->re->Sun(sunIndex);

			// Configure shader for object space lighting.
			m_RenderInfo->re->GLSL().ModelTexShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR,
					sun.m_ColorG, sun.m_ColorB, sun.m_LocX, sun.m_LocY, sun.m_LocZ, sun.m_DiskRadius);
		}

		// Point lights
		for(unsigned int lightIndex=0; lightIndex<m_RenderInfo->re->MaxLights(); lightIndex++)
		{
			// Light location was stored in eye space. Shaders want it in eye space.
			const AuGLLight& light = m_RenderInfo->re->Light(lightIndex);

			// Configure shader for object space lighting.
			m_RenderInfo->re->GLSL().ModelTexShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ,
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
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(mwall);
			m_Ground.DrawGL();
		}
		else if(m_ShowGround == 6)
		{
			AuMatrix4x4 mwall(m_Model);
			mwall.RotateX(PI/2);
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(mwall);
			m_Ground.DrawGL();
		}
		else if(m_ShowGround == 7)
		{
			AuMatrix4x4 mwall(m_Model);
			mwall.RotateX(PI/2);
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(mwall);
			m_Ground.DrawGL();
			mwall.RotateX(PI/2);
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(mwall);
			m_Ground.DrawGL();
			mwall.RotateX(PI/2);
			m_RenderInfo->re->GLSL().ModelTexShader().SetModelView(mwall);
			m_Ground.DrawGL();
		}
		m_Ground.DeselectGL();

		m_RenderInfo->re->GLSL().ModelTexShader().Deselect();
		if(m_ShowGround == 5 || m_ShowGround == 6 || m_ShowGround == 7)
		{
			m_RenderInfo->re->glDepthMask(GL_TRUE);
			m_RenderInfo->re->glDisable(GL_BLEND);
		}
	}

	update();
}

void AuDNAView::wheelEvent(QWheelEvent *e)
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

		AuPoint vec = m_Model.UnProjectVector(AuPoint(0,0,-dist));
		m_Model.Translate(vec);
	}

	QOpenGLWidget::wheelEvent(e);
}

void AuDNAView::mousePressEvent(QMouseEvent* e)
{
	QPoint point = e->pos();
	m_MouseButtons = e->buttons();
	m_MouseLast = point;

	QOpenGLWidget::mousePressEvent(e);
}

void AuDNAView::mouseMoveEvent(QMouseEvent* e)
{
	QOpenGLWidget::mouseMoveEvent(e);

	float xdelta = e->pos().x() - m_MouseLast.x();
	float ydelta = e->pos().y() - m_MouseLast.y();
	if(xdelta == 0 && ydelta == 0)
		return;

	AuPoint loc = m_Model.Translation();

	if(m_MouseButtons == Qt::LeftButton)
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
	else if(m_MouseButtons == Qt::RightButton)
	{
		// Pan.
		if(ydelta != 0)
		{
			float dist = ydelta/100.f;

			AuMatrix4x4 m;
			m.Translate(loc);
			m.Translate(0,-dist,0);
			m.Translate(-loc);

			m_Model = m * m_Model;
		}

		if(xdelta != 0)
		{
			float dist = xdelta/100.f;

			AuMatrix4x4 m;
			m.Translate(loc);
			m.Translate(dist,0,0);
			m.Translate(-loc);

			m_Model = m * m_Model;
		}
	}

	m_MouseLast = e->pos();
}

void AuDNAView::ResetView()
{
	m_Model.LoadIdentity();
	m_Model.Translate(0,-3,-8);
	m_Model.RotateX(-90*DEG2RAD);
	m_Model.RotateZ(180*DEG2RAD);
}

// Deletes the heap objects but does not clean up the open gl objects.
void AuDNAView::DeleteViewObjects()
{
	if(m_Female)
	{
		delete m_Female;
		m_Female = 0;
	}

	if(m_Male)
	{
		delete m_Male;
		m_Male = 0;
	}
}

void AuDNAView::ViewRestore()
{
	QByteArray ba;
	g_Au.Settings().readEntry("/ViewMatrix", ba);

	QDataStream ds(ba);
	ds.setVersion(QDataStream::Qt_4_1);

	ds >> m_Model;

	update();
}

void AuDNAView::ViewSave()
{
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.setVersion(QDataStream::Qt_4_1);

	ds << m_Model;

	g_Au.Settings().writeEntry("/ViewMatrix", ba);
	//qDebug() << "View matrix saved";
}

void AuDNAView::SetDNAPair(const AuDNA& dna)
{
	// Male
	HANDEDNESS handedness = RightHanded;
	if(m_Male && !m_Male->Is(dna, AuDNA::Alpha, handedness))
	{
		m_MaleDelete = m_Male;
		m_Male = 0;
	}
	if(!m_Male)
	{
		m_Male = new AuAnimalia(dna, AuDNA::Alpha, handedness, Mature);
		m_Male->Pose(AuId(Male), m_MalePose, m_MaleArmBent, 1);
		m_Male->SetHairStyle(m_MaleHair, m_MaleShave);
		m_Male->PoseMouth(AuId(Male), m_MaleMouth);

		AuItem hat(m_MaleHat, 192, 0, 0);
		m_Male->SetHat(&hat);
		AuItem clothes(m_MaleClothes, 192, 0, 0);
		m_Male->SetClothing(&clothes);
		AuItem armor(m_MaleArmor, 192, 0, 0);
		m_Male->SetArmor(&armor);
	}

	// Female
	handedness = LeftHanded;
	if(m_Female && !m_Female->Is(dna, AuDNA::Beta, handedness))
	{
		m_FemaleDelete = m_Female;
		m_Female = 0;
	}
	if(!m_Female)
	{
		m_Female = new AuAnimalia(dna, AuDNA::Beta, handedness, Mature);
		m_Female->Pose(AuId(Female), m_FemalePose, m_FemaleArmBent, 1);
		m_Female->SetHairStyle(m_FemaleHair, m_FemaleShave);
		m_Female->PoseMouth(AuId(Female), m_FemaleMouth);

		AuItem hat(m_FemaleHat, 192, 0, 0);
		m_Female->SetHat(&hat);
		AuItem clothes(m_FemaleClothes, 192, 0, 0);
		m_Female->SetClothing(&clothes);
		AuItem armor(m_FemaleArmor, 192, 0, 0);
		m_Female->SetArmor(&armor);
	}

	m_DNA = dna;
	//ResetView();
	update();
}

void AuDNAView::SetDNAPair(const AuDNA& dna, int modelchg)
{
	// Male
	HANDEDNESS handedness = RightHanded;
	if(m_Male && (modelchg & 0x01))
	{
		m_MaleDelete = m_Male;
		m_Male = 0;
	}
	if(!m_Male)
	{
		m_Male = new AuAnimalia(dna, AuDNA::Alpha, handedness, Mature);
		m_Male->Pose(AuId(Male), m_MalePose, m_MaleArmBent, 1);

		AuItem hat(m_MaleHat, 192, 0, 0);
		m_Male->SetHat(&hat);
		AuItem clothes(m_MaleClothes, 192, 0, 0);
		m_Male->SetClothing(&clothes);
		AuItem armor(m_MaleArmor, 192, 0, 0);
		m_Male->SetArmor(&armor);
	}
	m_Male->SetDNA(dna, AuDNA::Alpha);
	m_Male->SetHairStyle(m_MaleHair, m_MaleShave);
	m_Male->PoseMouth(AuId(Male), m_MaleMouth);

	// Female
	handedness = LeftHanded;
	if(m_Female && (modelchg & 0x02))
	{
		m_FemaleDelete = m_Female;
		m_Female = 0;
	}
	if(!m_Female)
	{
		m_Female = new AuAnimalia(dna, AuDNA::Beta, handedness, Mature);
		m_Female->Pose(AuId(Female), m_FemalePose, m_FemaleArmBent, 1);

		AuItem hat(m_FemaleHat, 192, 0, 0);
		m_Female->SetHat(&hat);
		AuItem clothes(m_FemaleClothes, 192, 0, 0);
		m_Female->SetClothing(&clothes);
		AuItem armor(m_FemaleArmor, 192, 0, 0);
		m_Female->SetArmor(&armor);
	}
	m_Female->SetDNA(dna, AuDNA::Beta);
	m_Female->SetHairStyle(m_FemaleHair, m_FemaleShave);
	m_Female->PoseMouth(AuId(Female), m_FemaleMouth);

	m_DNA = dna;
	//ResetView();
	update();
}

void AuDNAView::SetMaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave)
{
	if(m_MaleHair == cut && m_MaleShave == shave)
		return;
	m_MaleHair = cut;
	m_MaleShave = shave;
	if(m_Male)
	{
		m_MaleDelete = m_Male;
		m_Male = 0;
	}

	HANDEDNESS handedness = RightHanded;

	m_Male = new AuAnimalia(m_DNA, AuDNA::Alpha, handedness, Mature);
	m_Male->Pose(AuId(Male), m_MalePose, m_MaleArmBent, 1);
	m_Male->SetHairStyle(m_MaleHair, m_MaleShave);
	m_Male->PoseMouth(AuId(Male), m_MaleMouth);

	AuItem hat(m_MaleHat, 192, 0, 0);
	m_Male->SetHat(&hat);
	AuItem clothes(m_MaleClothes, 192, 0, 0);
	m_Male->SetClothing(&clothes);
	AuItem armor(m_MaleArmor, 192, 0, 0);
	m_Male->SetArmor(&armor);

	update();
}

void AuDNAView::SetFemaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave)
{
	if(m_FemaleHair == cut && m_FemaleShave == shave)
		return;
	m_FemaleHair = cut;
	m_FemaleShave = shave;
	if(m_Female)
	{
		m_FemaleDelete = m_Female;
		m_Female = 0;
	}

	HANDEDNESS handedness = LeftHanded;

	m_Female = new AuAnimalia(m_DNA, AuDNA::Beta, handedness, Mature);
	m_Female->Pose(AuId(Female), m_FemalePose, m_FemaleArmBent, 1);
	m_Female->SetHairStyle(m_FemaleHair, m_FemaleShave);
	m_Female->PoseMouth(AuId(Female), m_FemaleMouth);

	AuItem hat(m_FemaleHat, 192, 0, 0);
	m_Female->SetHat(&hat);
	AuItem clothes(m_FemaleClothes, 192, 0, 0);
	m_Female->SetClothing(&clothes);
	AuItem armor(m_FemaleArmor, 192, 0, 0);
	m_Female->SetArmor(&armor);

	update();
}

void AuDNAView::SetMaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes)
{
	if(clothes == CID_Invalid && !g_Au.IsNudityOk())
		clothes = CID_Underwear;

	if(m_MaleHat == hat &&
			m_MaleArmor == armor &&
			m_MaleClothes == clothes)
		return;
	m_MaleHat = hat;
	m_MaleArmor = armor;
	m_MaleClothes = clothes;
	if(m_Male)
	{
		m_MaleDelete = m_Male;
		m_Male = 0;
	}

	HANDEDNESS handedness = RightHanded;

	m_Male = new AuAnimalia(m_DNA, AuDNA::Alpha, handedness, Mature);
	m_Male->Pose(AuId(Male), m_MalePose, m_MaleArmBent, 1);
	m_Male->SetHairStyle(m_MaleHair, m_MaleShave);
	m_Male->PoseMouth(AuId(Male), m_MaleMouth);

	AuItem hatI(m_MaleHat, 192, 0, 0);
	m_Male->SetHat(&hatI);
	AuItem clothesI(m_MaleClothes, 192, 0, 0);
	m_Male->SetClothing(&clothesI);
	AuItem armorI(m_MaleArmor, 192, 0, 0);
	m_Male->SetArmor(&armorI);

	update();
}

void AuDNAView::SetFemaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes)
{
	if(clothes == CID_Invalid && !g_Au.IsNudityOk())
		clothes = CID_Underwear;

	if(m_FemaleHat == hat &&
			m_FemaleArmor == armor &&
			m_FemaleClothes == clothes)
		return;
	m_FemaleHat = hat;
	m_FemaleArmor = armor;
	m_FemaleClothes = clothes;
	if(m_Female)
	{
		m_FemaleDelete = m_Female;
		m_Female = 0;
	}

	HANDEDNESS handedness = LeftHanded;

	m_Female = new AuAnimalia(m_DNA, AuDNA::Beta, handedness, Mature);
	m_Female->Pose(AuId(Female), m_FemalePose, m_FemaleArmBent, 1);
	m_Female->SetHairStyle(m_FemaleHair, m_FemaleShave);
	m_Female->PoseMouth(AuId(Female), m_FemaleMouth);

	AuItem hatI(m_FemaleHat, 192, 0, 0);
	m_Female->SetHat(&hatI);
	AuItem clothesI(m_FemaleClothes, 192, 0, 0);
	m_Female->SetClothing(&clothesI);
	AuItem armorI(m_FemaleArmor, 192, 0, 0);
	m_Female->SetArmor(&armorI);
	update();
}

void AuDNAView::SetFemaleMouthPose(const AuPose& face)
{
	if(m_FemaleMouth == face)
		return;
	m_FemaleMouth = face;
	if(!m_Female)
		return;
	m_Female->PoseMouth(AuId(Female), face);
	update();
}

void AuDNAView::SetFemalePose(const AuPose& pose, bool armBent)
{
	if(m_FemalePose == pose && m_FemaleArmBent == armBent)
		return;
	m_FemalePose = pose;
	m_FemaleArmBent = armBent;
	if(!m_Female)
		return;
	m_Female->Pose(AuId(Female), pose, m_FemaleArmBent, 1);
	m_Female->PoseMouth(AuId(Female), m_FemaleMouth);
	update();
}

void AuDNAView::SetMaleMouthPose(const AuPose& face)
{
	if(m_MaleMouth == face)
		return;
	m_MaleMouth = face;
	if(!m_Male)
		return;
	m_Male->PoseMouth(AuId(Male), face);
	update();
}

void AuDNAView::SetMalePose(const AuPose& pose, bool armBent)
{
	if(m_MalePose == pose && m_MaleArmBent == armBent)
		return;
	m_MalePose = pose;
	m_MaleArmBent = armBent;
	if(!m_Male)
		return;
	m_Male->Pose(AuId(Male), pose, m_MaleArmBent, 1);
	m_Male->PoseMouth(AuId(Male), m_MaleMouth);
	update();
}

void AuDNAView::SetShowGround(int showWhat)
{
	m_ShowGround = showWhat;
	update();
}

int AuDNAView::AbilityPointsUsedFemale()
{
	if(!m_Female)
		return 0;
	return m_Female->AbilityPointsUsed();
}

int AuDNAView::AbilityPointsUsedMale()
{
	if(!m_Male)
		return 0;
	return m_Male->AbilityPointsUsed();
}

float AuDNAView::CameraHeightFemale()
{
	if(!m_Female)
		return 0;
	return m_Female->CameraLocation().z;
}

float AuDNAView::CameraHeightMale()
{
	if(!m_Male)
		return 0;
	return m_Male->CameraLocation().z;
}

float AuDNAView::CgHeightFemale()
{
	if(!m_Female)
		return 0;
	return m_Female->CGHeight();
}

float AuDNAView::CgHeightMale()
{
	if(!m_Male)
		return 0;
	return m_Male->CGHeight();
}

float AuDNAView::HeadHeightFemale()
{
	if(!m_Female)
		return 0;
	return m_Female->HeadLocation().z;
}

float AuDNAView::HeadHeightMale()
{
	if(!m_Male)
		return 0;
	return m_Male->HeadLocation().z;
}

int AuDNAView::HitPointsFemale()
{
	if(!m_Female)
		return 0;
	return m_Female->MaxHitPoints();
}

int AuDNAView::HitPointsMale()
{
	if(!m_Male)
		return 0;
	return m_Male->MaxHitPoints();
}

int AuDNAView::AvatarCheckFemale()
{
	if(!m_Female)
		return 0x8000;
	return m_Female->AvatarCheck();
}

int AuDNAView::AvatarCheckMale()
{
	if(!m_Male)
		return 0x8000;
	return m_Male->AvatarCheck();
}

