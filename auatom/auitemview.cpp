//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemview.cpp,v 1.18 2022/05/09 16:24:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitemview.h"
//
#include "auatomrenderinfo.h"
#include "auglmodelmaker.h"
#include "auglobal.h"
#include <QDebug>
#include <QWheelEvent>

#define d_Ambient						(1)

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemView::AuItemView(QWidget* parent) : QOpenGLWidget(parent), m_Font(g_Au.TypefaceSans(), 22)
{
	m_BusyAngle = 0;
	m_BusyTex = 0;
	m_MM = new AuGLModelMaker(false),
	m_ModelCache = new AuGLModelCache();
	m_RenderInfo = new AuAtomRenderInfo(this, 0, *m_MM, *m_ModelCache, m_Font);

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_Item = 0;
	m_MouseButtons = Qt::NoButton;
}

AuItemView::~AuItemView()
{
	DeleteViewObjects();
	delete m_RenderInfo;
	delete m_ModelCache;
	delete m_MM;
}

void AuItemView::initializeGL()
{
	m_RenderInfo->InitializeGL();

	if(!m_BusyTex)
		m_BusyTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "BusyThread", ":data/Busy.png");

	// Configure for solid rendered display.
	m_RenderInfo->re->glFrontFace(GL_CW);						// Front faces are those drawn clockwise.
	m_RenderInfo->re->glCullFace(GL_BACK);						// Cull back-facing faces.
	m_RenderInfo->re->glEnable(GL_CULL_FACE);					// Enable face culling.
	m_RenderInfo->re->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);					// Enable depth calculations
	m_RenderInfo->re->glClearColor(.25, .25, .25, 1);

	ResetView();
}

void AuItemView::SetupLights(const AuMatrix4x4& model)
{
	Q_UNUSED(model);
	//m_RenderInfo->re->SetAmbientLight(d_Ambient, d_Ambient, d_Ambient, 1);

	m_RenderInfo->re->DisableSuns();
	m_RenderInfo->re->DisableLights();

	QColor sunColor(255, 234, 214);		// Yellow (Type G star)

	// Lights remain stationary while the model moves.
	// Set up some general lighting.
	const double ltdist = 100000.0;		// Light distance from model.
	double atten = 1.0/(ltdist*ltdist);
	AuPoint vl(-ltdist/2,ltdist/2,ltdist);
	vl.Unit(ltdist);
	m_RenderInfo->re->SetSun(0, atten, sunColor, vl.x, vl.y, vl.z, 1.0);

	m_AmbientUp = AuPoint(0,1,0);
}

void AuItemView::paintGL()
{
	//qDebug() << "AuItemView::paintGL";
	m_RenderInfo->re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	AuMatrix4x4 projection;
	if(rect().width() > 0)
		projection.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), .1, 1000);
	else
		projection.Perspective(45.0*DEG2RAD, 1, .1, 1000);

	SetupLights(m_Model);

	// Render item.
	if(m_Item)
	{
		bool waitingForModel = false;
		if(m_Item->CommodityId() == CID_Head)
		{
			// Gen4 heads take a lot of work to make.
			const AuItemDNA* idna = dynamic_cast<const AuItemDNA*>(m_Item);
			if(idna && idna->DNAWantsAnimaliaModel())
			{
				AuAnimalia* animalia = new AuAnimalia(*((const AuDNA*)idna->DNA()), idna->DNAChromatid(), idna->DNAHandedness(),
						idna->DNAGrowthStage());
				animalia->SetHairStyle(idna->DNAHair(), idna->DNAShave());
				animalia->Pose(AuId(1), AuPose::EyesOnly, false, 1);
				animalia->CreateClothingTex(*m_RenderInfo->re);
				m_RenderInfo->mm.Animalia(*m_RenderInfo->re, AuId(1), *animalia); 
				if(animalia->HasVAOs())
					m_Item->DNASetAnimaliaModel(animalia);
				else
				{
					waitingForModel = true;
					delete animalia;
				}
			}
		}

		if(!waitingForModel)
			m_Item->Render(projection, m_Model, *m_RenderInfo->re, m_RenderInfo->Models(), AuId(1), d_Ambient, d_Ambient, d_Ambient,
					m_AmbientUp, AuPoint(0,0,1));
		else
			ShowBusyIndicator();
	}
	update();
}

void AuItemView::wheelEvent(QWheelEvent *e)
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

		double dist = .10;
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

		double dist = .10;
		if(bfast)
			dist *= 10.0;

		AuPoint vec = m_Model.UnProjectVector(AuPoint(0,0,-dist));
		m_Model.Translate(vec);
	}

	QOpenGLWidget::wheelEvent(e);
}

void AuItemView::mousePressEvent(QMouseEvent* e)
{
	QPoint point = e->pos();
	m_MouseButtons = e->buttons();
	m_MouseLast = point;

	QOpenGLWidget::mousePressEvent(e);
}

void AuItemView::mouseMoveEvent(QMouseEvent* e)
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

void AuItemView::mouseDoubleClickEvent(QMouseEvent* e)
{
	Q_UNUSED(e);
}

AuPoint AuItemView::PointAtPixel(const QPoint& pixel) const
{
	// Configure the device transform for this window.
	QRect rcClient = rect();
	AuMatrix4x4 tDevice;
	tDevice.Scale(rcClient.width()/2.0, -rcClient.height()/2.0, rcClient.width()/2.0);
	tDevice.Translate(1.0, -1.0, 0.0);

	AuMatrix4x4 tPersp;
	if(rect().width() > 0)
		tPersp.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), .1, 1000);
	else
		tPersp.Perspective(45.0*DEG2RAD, 1, .1, 1000);
	tPersp *= m_Model;

	AuMatrix4x4 tDevicePersp = tDevice * tPersp;

	AuPoint fProj;
	tDevicePersp.UnProjectIntoPlane(fProj, pixel, AuMatrix4x4());
	return fProj;
}

void AuItemView::ResetView()
{
	AuPoint min,max;
	if(m_Item)
		m_Item->ModelExtents(min, max);

	double wid = max.x - min.x;
	double ht = max.z - min.z;
	double zDepth;
	if(wid > ht)
		zDepth = max.y + wid + .5;
	else
		zDepth = max.y + ht + .5;

	m_Model.LoadIdentity();
	m_Model.Translate((min.x + max.x)/2, -(min.z + max.z)/2, -zDepth);
	m_Model.RotateX(-90*DEG2RAD);
	m_Model.RotateZ(180*DEG2RAD);
}

// Deletes the heap objects but does not clean up the open gl objects.
void AuItemView::DeleteViewObjects()
{
	if(m_Item)
	{
		delete m_Item;
		m_Item = 0;
	}
}

void AuItemView::ViewItem(const AuItem& item)
{
	// Do nothing if this is not a change.
	if(m_Item && *m_Item == item)
		return;

	// Delete old render items.
	DeleteViewObjects();

	// View the new item.
	m_Item = item.Clone();

	update();
}

void AuItemView::ShowBusyIndicator()
{
	if(!m_RenderInfo->re)
		return;

	float rad = .025f;
	if(!m_BusyVAO.VAO())
	{
		// Create VAO of busy indicator.
		float s = 0;
		float t = 0;
		float u = .25;
		float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-rad,-rad,0, s,t);
		tri0.m_V1 = AuPointTex(-rad,rad,0, s,v);
		tri0.m_V2 = AuPointTex(rad,rad,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(rad,rad,0, u,v);
		tri1.m_V1 = AuPointTex(rad,-rad,0, u,t);
		tri1.m_V2 = AuPointTex(-rad,-rad,0, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		m_BusyVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}

	// Model view projection.
	AuMatrix4x4 model;
	model.Translate(.95, .925, 0);
	m_BusyAngle -= 1;
	while(m_BusyAngle < 360)
		m_BusyAngle += 360;
	model.RotateZ(m_BusyAngle*DEG2RAD);

	// Texture offset.
	int frame = (int)(m_BusyAngle/6) % 8;
	if(frame >= 4)
		frame = 7 - frame;
	float offset = frame * .25f;

	// Set up for additive blending.
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TexOffsetShader().Select();
	m_RenderInfo->re->GLSL().TexOffsetShader().SetProjectionModelView(model);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetTexture(m_BusyTex);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetColor(1,1,1, 1);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetDiscardAlpha(0);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetTextureOffset(offset, 0);

	m_BusyVAO.SelectGL();
	m_BusyVAO.DrawGL();
	m_BusyVAO.DeselectGL();

	m_RenderInfo->re->GLSL().TexOffsetShader().Deselect();

	float gldAspect = AuGLEffects::WindowAspectRatio(rect());
	float txht = rad;
	m_RenderInfo->RenderNumber(m_RenderInfo->mm.BusyThreadCount(), (.95f + rad) / gldAspect, .925f - txht/2.f, txht, 0,0,
			242.f/255.f, 243.f/255.f, 224.f/255.f, .5f);

	// Restore rendering states
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
	m_RenderInfo->re->glDisable(GL_BLEND);
}

