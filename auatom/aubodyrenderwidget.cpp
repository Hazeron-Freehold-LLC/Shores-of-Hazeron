//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubodyrenderwidget.cpp,v 1.60 2022/05/16 23:20:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubodyrenderwidget.h"
//
#include "auanimalia.h"
#include "auelapsedtimer.h"
#include "augleffects.h"
#include "auglmodel.h"
#include "auglobal.h"
#include "dpbody.h"
#include <QMouseEvent>
#include <QWheelEvent>

AuBodyRenderWidget::AuBodyRenderWidget(QWidget *parent) : QOpenGLWidget(parent) 
{
	m_Animalia = 0;
	m_AnimaliaDel = 0;
	m_Animate = true;
	m_Armor = CID_Invalid;	// No armor.
	m_ModelCache = 0;
	m_texBackdrop = 0;
	m_texSpacecraft = 0;
	m_pBody = 0;
	m_pBodyDel = 0;
	m_pRE = 0;
	m_bLeftMousePushed = false;
	m_bRightMousePushed = false;
	m_bMiddleMousePushed = false;
	m_bShowScene = false;
	m_bDeleteDeviceContext = false;
	m_sFileName = "";
	m_sPath = "";
	m_Clothes = CID_Invalid;	// No clothing.
	m_Helmet = CID_Invalid;		// No helmet.
	m_HelmetVisor = 0;			// AuItem::State of the helmet. 0=visor open
	m_fRotation.z = 15*DEG2RAD;

	setMouseTracking(true);

	setAttribute(Qt::WA_OpaquePaintEvent);

	setProperty("cursor", QVariant(QCursor(Qt::OpenHandCursor)));

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	setSizePolicy(sizePolicy);
}

AuBodyRenderWidget::~AuBodyRenderWidget()
{
	//qDebug() << "~AuBodyRenderWidget";
	if(m_Animalia)
	{
		delete m_Animalia;
		m_Animalia = 0;
	}
	if(m_AnimaliaDel)
	{
		delete m_AnimaliaDel;
		m_AnimaliaDel = 0;
	}
	if(m_pBody)
	{
		delete m_pBody;
		m_pBody = 0;
	}
	if(m_pBodyDel)
	{
		delete m_pBodyDel;
		m_pBodyDel = 0;
	}
	if(m_ModelCache)
	{
		delete m_ModelCache;
		m_ModelCache = 0;
	}
	if(m_pRE)
	{
		delete m_pRE;
		m_pRE = 0;
	}
}

void AuBodyRenderWidget::wheelEvent(QWheelEvent *wheelEvent)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int numDegrees = wheelEvent->angleDelta().y() / 8;
#else
	int numDegrees = wheelEvent->delta() / 8;
#endif
	int numSteps = numDegrees / 15;
	if(numSteps > 0)
		m_fTranslation.z = m_fTranslation.z + 50.0f*DEG2RADf;
	else if(numSteps < 0)
		m_fTranslation.z = m_fTranslation.z - 50.0f*DEG2RADf;

	QOpenGLWidget::wheelEvent(wheelEvent);
}

void AuBodyRenderWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
	if(mouseEvent->button() == Qt::LeftButton)
		m_bLeftMousePushed = true;
	else if(mouseEvent->button() == Qt::RightButton)
		m_bRightMousePushed = true;
	else if(mouseEvent->button() == Qt::MidButton)
		m_bMiddleMousePushed = true;
	else
		return;

	if(m_bLeftMousePushed == true || m_bRightMousePushed == true || m_bMiddleMousePushed == true)
		setProperty("cursor", QVariant(QCursor(Qt::ClosedHandCursor)));
	else
		setProperty("cursor", QVariant(QCursor(Qt::OpenHandCursor)));
	posClicked = mouseEvent->pos();

	QOpenGLWidget::mousePressEvent(mouseEvent);
}

void AuBodyRenderWidget::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	QPoint curPos = mouseEvent->pos();
	int deltaX = curPos.x() - posClicked.x();
	int deltaY = curPos.y() - posClicked.y();
	if(m_bLeftMousePushed)
	{
		m_fRotation.z += deltaX * DEG2RADf;
		m_fRotation.x -= deltaY * DEG2RADf;
	}
	if(m_bRightMousePushed)
	{
		m_fTranslation.x += deltaX/100.;
		m_fTranslation.y -= deltaY/100.;
	}
	if(m_bMiddleMousePushed)
	{
		if(deltaX>0)
			m_fRotAngle += (float)PIf * 0.1f * .5;
		else if(deltaX<0)
			m_fRotAngle -= (float)PIf * 0.1f * .5;
	}
	posClicked = curPos;

	QOpenGLWidget::mouseMoveEvent(mouseEvent);
}

void AuBodyRenderWidget::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	if(mouseEvent->button() == Qt::LeftButton)
		m_bLeftMousePushed = false;
	if(mouseEvent->button() == Qt::RightButton)
		m_bRightMousePushed = false;

	if(m_bLeftMousePushed == true || m_bRightMousePushed == true)
		setProperty("cursor", QVariant(QCursor(Qt::ClosedHandCursor)));
	else
		setProperty("cursor", QVariant(QCursor(Qt::OpenHandCursor)));

	QOpenGLWidget::mouseReleaseEvent(mouseEvent);
}

void AuBodyRenderWidget::initializeGL()
{
	//qDebug() << "AuBodyRenderWidget::initializeGL";
	Q_ASSERT(!m_pRE);
	m_pRE = new AuGLEffects;
	m_pRE->InitializeGL(g_Au.MaxLights());

	// Set up the rendering context, define display lists etc.:
	m_pRE->glClearColor(0.0, 0.0, 0.0, 1.0);

	m_ModelCache = new AuGLModelCache;
}

void AuBodyRenderWidget::resizeGL(int w, int h)
{
	//qDebug() << "AuBodyRenderWidget::resizeGL" << w << h;
	// setup viewport, projection etc.:
	m_pRE->glViewport(0, 0, (GLint)w, (GLint)h);
}

void AuBodyRenderWidget::paintGL()
{
	//qDebug() << "AuBodyRenderWidget::paingGL" << m_texBackdrop << m_texSpacecraft << g_Au.CurrentUTC().time().msec();
	if(isHidden())
		return;		// No need to render when hidden

	if(!m_texBackdrop)
		m_texBackdrop = m_pRE->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "B0", ":data/Backdrop.png");

	if(!m_texSpacecraft)
		m_texSpacecraft = m_pRE->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "S0MetalLit", ":data/txSpacecraft0Lit.png");

	if(!m_BackdropVAO.VAO())
	{
		float s = 0;
		float t = 0;
		float u = 1;
		float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-1,-1,0, s,t);
		tri0.m_V1 = AuPointTex(-1,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,-1,0, u,t);
		tri1.m_V2 = AuPointTex(-1,-1,0, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_BackdropVAO.CreateForTexture(*m_pRE, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 model;
	model.Translate(m_fTranslation.x,m_fTranslation.y,m_fTranslation.z-12);
	model.RotateAxis(-90*DEG2RAD, 1,0,0);
	model.RotateAxis(180*DEG2RAD, 0,0,1);

	// Set up some general lighting.
	float sunDist = 1000;
	AuPoint sunLoc(0, 0, sunDist);
	double atten = 1.0/(sunDist*sunDist);
	sunLoc = model.ProjectVector(sunLoc);
	m_pRE->SetSun(0, atten, QColor(220,220,220), sunLoc.x, sunLoc.y, sunLoc.z, 100.0);

	AuMatrix4x4 projection;
	if(rect().width() > 0)
		projection.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), 2, 100000);
	else
		projection.Perspective(45.0*DEG2RAD, 1, 2, 100000);

	m_pRE->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pRE->glFrontFace(GL_CW);			// Front faces are those drawn clockwise.
	m_pRE->glCullFace(GL_BACK);		// Cull back-facing faces.
	m_pRE->glEnable(GL_CULL_FACE);		// Enable face culling.
	m_pRE->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	m_pRE->glDisable(GL_DEPTH_TEST);

	// Draw backdrop using simple shader.
	m_pRE->GLSL().TextureShader().Select();
	m_pRE->GLSL().TextureShader().SetProjectionModelView(AuMatrix4x4());
	m_pRE->GLSL().TextureShader().SetTexture(m_texBackdrop);
	m_pRE->GLSL().TextureShader().SetColor(1,1,1,1);

	m_BackdropVAO.SelectGL();
	m_BackdropVAO.DrawGL();
	m_BackdropVAO.DeselectGL();

	m_pRE->GLSL().TextureShader().Deselect();
	
	m_pRE->glEnable(GL_DEPTH_TEST);
	//m_pRE->glShadeModel(GL_SMOOTH);

	// Shut off all the directional/point lights and the ambient light.
	m_pRE->SetAmbientLight(1,1,1,1);
	m_pRE->DisableSuns();
	m_pRE->DisableLights();

	AuMatrix4x4 rotm;
	rotm.Rotate(m_fRotation);

	model *= rotm;

	AuPoint ambientUp = model.ProjectVector(0,0,1);

	// Render the model of the body.
	if(m_pBody)
	{
		// Old style body removes the helmet entirely when the visor is open.
		if(m_HelmetVisor)
			m_pBody->CreateDeviceObjects(*m_pRE, *m_ModelCache, m_Clothes, m_Helmet, 1);
		else
			m_pBody->CreateDeviceObjects(*m_pRE, *m_ModelCache, m_Clothes, CID_Invalid, 1);

		// Stolen from DaCreature::BodyPoseAdjustTransform
		AuMatrix4x4 tBody(model);
		switch(CurrentGeneralPose().Pose())
		{
		default:
			break;
		case AuPose::EyesOnly:
			//tBody.RotateX(CurrentHeadTilt());
			break;
		case AuPose::BerthBottomFaceLeft:
		case AuPose::BerthTopFaceLeft:
		case AuPose::DeadFaceLeft:
		case AuPose::WritheFaceLeft:
			tBody.RotateY(PI/2.);
			break;
		case AuPose::BerthBottomFaceRight:
		case AuPose::BerthTopFaceRight:
		case AuPose::DeadFaceRight:
		case AuPose::WritheFaceRight:
			tBody.RotateY(-PI/2.);
			break;
		case AuPose::BerthBottomFaceUp:
		case AuPose::BerthTopFaceUp:
		case AuPose::DeadFaceUp:
		case AuPose::WritheFaceUp:
		case AuPose::ReceptiveMate:
		case AuPose::MateOnBottom:
		case AuPose::Patient_Prone:
			tBody.RotateY(PI);
			break;
		}
		m_pBody->RenderSkin(projection, tBody, *m_pRE, ambientUp, false);
	}
	else if(m_Animalia)
	{
		if(!m_Animalia->HasVAOs())
		{
			m_Animalia->CreateClothingTex(*m_pRE);
			m_Animalia->CreateAnimalVAO(*m_pRE, m_Id.GetID());
		}
		m_Animalia->RenderAnimal(projection, model, *m_pRE, false, m_HelmetVisor, ambientUp);
	}

	// Scene
	if(m_bShowScene)
	{
		if(!m_ShipDeckVAO.VAO())
			m_ShipDeckVAO = CreateDeckScene();

		RenderDeckScene(projection, model);
	}

	// Blended pass
	if(m_Animalia)
		m_Animalia->RenderAnimal(projection, model, *m_pRE, true, m_HelmetVisor, ambientUp);

	// Cleanup old models.
	if(m_AnimaliaDel)
	{
		m_AnimaliaDel->DeleteGL();
		delete m_AnimaliaDel;
		m_AnimaliaDel = 0;
	}
	if(m_pBodyDel)
	{
		m_pBodyDel->DeleteDeviceObjects(*m_pRE);
		delete m_pBodyDel;
		m_pBodyDel = 0;
	}

	if(m_Animate)
		update();
}

AuGLVAO AuBodyRenderWidget::CreateDeckScene()
{
	//qDebug() << "AuBodyRenderWidget::CreateDeckScene";
	AuGLVAO vao;
	float fs,ft,fu,fv;
	ShipTexture(ST_IntFloor, fs, ft, fu, fv);
	float cs,ct,cu,cv;
	ShipTexture(ST_IntCeiling, cs, ct, cu, cv);
	float iws,iwt,iwu,iwv;
	ShipTexture(ST_IntWallCargo, iws, iwt, iwu, iwv);
	float irs,irt,iru,irv;
	ShipTexture(ST_IntWallFinished, irs, irt, iru, irv);
	float iwfs,iwft,iwfu,iwfv;
	ShipTexture(ST_IntWallFillCargo, iwfs, iwft, iwfu, iwfv);
	float irfs,irft,irfu,irfv;
	ShipTexture(ST_IntWallFill, irfs, irft, irfu, irfv);
	float ws,wt,wu,wv;
	ShipTexture(ST_HullWallWindow, ws, wt, wu, wv);
	float cds,cdt,cdu,cdv;
	ShipTexture(ST_CrewDoorClosed, cds, cdt, cdu, cdv);
	float ods,odt,odu,odv;
	ShipTexture(ST_CrewDoorOpen, ods, odt, odu, odv);
	float hds,hdt,hdu,hdv;
	ShipTexture(ST_HullDoorClosed, hds, hdt, hdu, hdv);

	double dgrid2 = DeckGridSpacing() * 2;

	QList<AuTriTexNS> tris;

	AuPoint n(0,0,1);	// Surface normal;
	float shine = 1;

	for(int ix=-2; ix<2; ix++)
	{
		double rx = DeckGridSpacing() * ix;
		double rx2 = rx + DeckGridSpacing();
		for(int iy=-2; iy<2; iy++)
		{
			double ry = DeckGridSpacing() * iy;
			double ry2 = ry + DeckGridSpacing();

			// Floor quad.
			tris << AuTriTexNS(AuPointTexNS(AuPoint(rx,ry,0), QPointF(fs,ft), n, shine),
					AuPointTexNS(AuPoint(rx,ry2,0), QPointF(fs,fv), n, shine),
					AuPointTexNS(AuPoint(rx2,ry2,0), QPointF(fu,fv), n, shine));

			tris << AuTriTexNS(AuPointTexNS(AuPoint(rx2,ry2,0), QPointF(fu,fv), n, shine),
					AuPointTexNS(AuPoint(rx2,ry,0), QPointF(fu,ft), n, shine),
					AuPointTexNS(AuPoint(rx,ry,0), QPointF(fs,ft), n, shine));

			// Ceiling quad.
			tris << AuTriTexNS(AuPointTexNS(AuPoint(rx,ry2, DeckHeight()), QPointF(cs,ct), n, shine),
					AuPointTexNS(AuPoint(rx,ry, DeckHeight()), QPointF(cs,cv), n, shine),
					AuPointTexNS(AuPoint(rx2,ry, DeckHeight()), QPointF(cu,cv), n, shine));

			tris << AuTriTexNS(AuPointTexNS(AuPoint(rx2,ry, DeckHeight()), QPointF(cu,cv), n, shine),
					AuPointTexNS(AuPoint(rx2,ry2, DeckHeight()), QPointF(cu,ct), n, shine),
					AuPointTexNS(AuPoint(rx,ry2, DeckHeight()), QPointF(cs,ct), n, shine));
		}
	}

	// Solid wall in front of person.
	{
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-dgrid2,dgrid2,0), QPointF(irs,irt), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,dgrid2, DeckHeight()), QPointF(irs,irv), n, shine),
				AuPointTexNS(AuPoint(-2,dgrid2, DeckHeight()), QPointF(iru,irv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-2,dgrid2, DeckHeight()), QPointF(iru,irv), n, shine),
				AuPointTexNS(AuPoint(-2,dgrid2,0), QPointF(iru,irt), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,dgrid2,0), QPointF(irs,irt), n, shine));

		// Hull Door
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-2,dgrid2,0), QPointF(hds,hdt), n, shine),
				AuPointTexNS(AuPoint(-2,dgrid2, DeckHeight()-1), QPointF(hds,hdv), n, shine),
				AuPointTexNS(AuPoint(2,dgrid2, DeckHeight()-1), QPointF(hdu,hdv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(2,dgrid2, DeckHeight()-1), QPointF(hdu,hdv), n, shine),
				AuPointTexNS(AuPoint(2,dgrid2,0), QPointF(hdu,hdt), n, shine),
				AuPointTexNS(AuPoint(-2,dgrid2,0), QPointF(hds,hdt), n, shine));

		// Window above door
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-2,dgrid2, DeckHeight()-1), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(-2,dgrid2, DeckHeight()), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(0,dgrid2, DeckHeight()), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(0,dgrid2, DeckHeight()), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(0,dgrid2, DeckHeight()-1), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(-2,dgrid2, DeckHeight()-1), QPointF(ws,wt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(0,dgrid2, DeckHeight()-1), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(0,dgrid2, DeckHeight()), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(2,dgrid2, DeckHeight()), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(2,dgrid2, DeckHeight()), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(2,dgrid2, DeckHeight()-1), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(0,dgrid2, DeckHeight()-1), QPointF(ws,wt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(2,dgrid2,0), QPointF(irs,irt), n, shine),
				AuPointTexNS(AuPoint(2,dgrid2, DeckHeight()), QPointF(irs,irv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2, DeckHeight()), QPointF(iru,irv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,dgrid2, DeckHeight()), QPointF(iru,irv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2,0), QPointF(iru,irt), n, shine),
				AuPointTexNS(AuPoint(2,dgrid2,0), QPointF(irs,irt), n, shine));
	}

	// Solid wall to left of person.
	{
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2,0), QPointF(irs,irt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, DeckHeight()), QPointF(irs,irv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2, DeckHeight()), QPointF(iru,irv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2, DeckHeight()), QPointF(iru,irv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2,0), QPointF(iru,irt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2,0), QPointF(irs,irt), n, shine));

		// Door
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2,0), QPointF(cds,cdt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2, DeckHeight()), QPointF(cds,cdv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2, DeckHeight()), QPointF(cdu,cdv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2, DeckHeight()), QPointF(cdu,cdv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2,0), QPointF(cdu,cdt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2,0), QPointF(cds,cdt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2,0), QPointF(irfs,irft), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2, DeckHeight()), QPointF(irfs,irfv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(), DeckHeight()), QPointF(irfu,irfv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(), DeckHeight()), QPointF(irfu,irfv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(),0), QPointF(irfu,irft), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2,0), QPointF(irfs,irft), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(),0), QPointF(irs,irt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(), DeckHeight()), QPointF(irs,irv), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(), DeckHeight()), QPointF(iru,irv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(), DeckHeight()), QPointF(iru,irv), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(),0), QPointF(iru,irt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(),0), QPointF(irs,irt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(),0), QPointF(irs,irt), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(), DeckHeight()), QPointF(irs,irv), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,dgrid2, DeckHeight()), QPointF(iru,irv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-dgrid2,dgrid2, DeckHeight()), QPointF(iru,irv), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,dgrid2,0), QPointF(iru,irt), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(),0), QPointF(irs,irt), n, shine));
	}

	// Back side of solid wall to left of person.
	{
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2,0), QPointF(iws,iwt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2, DeckHeight()), QPointF(iws,iwv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, DeckHeight()), QPointF(iwu,iwv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, DeckHeight()), QPointF(iwu,iwv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2,0), QPointF(iwu,iwt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2,0), QPointF(iws,iwt), n, shine));

		// Door
		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2,0), QPointF(ods,odt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2, DeckHeight()), QPointF(ods,odv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2, DeckHeight()), QPointF(odu,odv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2, DeckHeight()), QPointF(odu,odv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-2,0), QPointF(odu,odt), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2,0), QPointF(ods,odt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(),0), QPointF(iwfs,iwft), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(), DeckHeight()), QPointF(iwfs,iwfv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2, DeckHeight()), QPointF(iwfu,iwfv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2, DeckHeight()), QPointF(iwfu,iwfv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),2,0), QPointF(iwfu,iwft), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(),0), QPointF(iwfs,iwft), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(),0), QPointF(iws,iwt), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(), DeckHeight()), QPointF(iws,iwv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(), DeckHeight()), QPointF(iwu,iwv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(), DeckHeight()), QPointF(iwu,iwv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),DeckGridSpacingHalf(),0), QPointF(iwu,iwt), n, shine),
				AuPointTexNS(AuPoint(-dgrid2,DeckGridSpacingHalf(),0), QPointF(iws,iwt), n, shine));
	}

	// Window wall behind person 
	{
		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,-dgrid2,0), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2, 2), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, 2), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, 2), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2,0), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2,0), QPointF(ws,wt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,-dgrid2,2), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2, 6), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, 6), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, 6), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2,2), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2,2), QPointF(ws,wt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,-dgrid2,6), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2, DeckHeight()), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, DeckHeight()), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2, DeckHeight()), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(-DeckGridSpacingHalf(),-dgrid2,6), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2,6), QPointF(ws,wt), n, shine));
	}

	// Window wall to the right of person 
	{
		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,dgrid2,0), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2, 2), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2, 2), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,-dgrid2, 2), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2,0), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2,0), QPointF(ws,wt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,dgrid2,2), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2, 6), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2, 6), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,-dgrid2, 6), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2,2), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2,2), QPointF(ws,wt), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,dgrid2,6), QPointF(ws,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2, DeckHeight()), QPointF(ws,wv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2, DeckHeight()), QPointF(wu,wv), n, shine));

		tris << AuTriTexNS(AuPointTexNS(AuPoint(dgrid2,-dgrid2, DeckHeight()), QPointF(wu,wv), n, shine),
				AuPointTexNS(AuPoint(dgrid2,-dgrid2,6), QPointF(wu,wt), n, shine),
				AuPointTexNS(AuPoint(dgrid2,dgrid2,6), QPointF(ws,wt), n, shine));
	}

	vao.CreateForModelTexFog(*m_pRE, tris, GL_STATIC_DRAW);
	return vao;
}

void AuBodyRenderWidget::RenderDeckScene(const AuMatrix4x4& projection, const AuMatrix4x4& model)
{
	if(!m_ShipDeckVAO.VAO())
		return;

	AuMatrix4x4 tModel(model);
	if(m_pBody)
	{
		switch(m_pBody->CurrentGeneralPose().Pose())
		{
		case AuPose::Fly:
		case AuPose::FlyFast:
		case AuPose::Glide:
		case AuPose::Hover:
		case AuPose::Swim:
		case AuPose::SwimFast:
			// These poses need to be lifted off the floor.
			tModel.Translate(0,0,-DeckHeight()/2);	// Put them half-way between the floor and the ceiling.
			break;
		default:
			{
				double waistHt = m_pBody->CGHeight(true) + 2./12.;	// Lower floor 2" below feet.
				tModel.Translate(0,0,-waistHt);
			}
			break;
		}
	}
	else if(m_Animalia)
	{
		switch(m_Animalia->CurrentGeneralPose().Pose())
		{
		case AuPose::Fly:
		case AuPose::FlyFast:
		case AuPose::Glide:
		case AuPose::Hover:
		case AuPose::Swim:
		case AuPose::SwimFast:
			// These poses need to be lifted off the floor.
			tModel.Translate(0,0,-DeckHeight()/2);	// Put them half-way between the floor and the ceiling.
			break;
		default:
			{
				double waistHt = m_Animalia->CGHeight() + 2./12.;	// Lower floor 2" below feet.
				tModel.Translate(0,0,-waistHt);
			}
			break;
		}
	}

	m_pRE->GLSL().ModelTexShader().Select(projection, *m_pRE);
	m_pRE->GLSL().ModelTexShader().SetModelView(tModel);
	m_pRE->GLSL().ModelTexShader().SetTexture(m_texSpacecraft);

	// Ambient light
	m_pRE->GLSL().ModelTexShader().SetAmbientLight(1,1,1);
	AuPoint ambientUp = tModel.ProjectVector(0, 0, 1);
	m_pRE->GLSL().ModelTexShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Point lights
	for(unsigned int lightIndex=0; lightIndex<m_pRE->MaxLights(); lightIndex++)
		m_pRE->GLSL().ModelTexShader().SetLight(lightIndex, false, 0, 0, 0,0,0, 0,0,0, 0,0, 0,0,1);

	// Render the model.
	m_pRE->GLSL().ModelTexShader().SetGlowAlpha(0,0,0, 1);

	m_ShipDeckVAO.SelectGL();
	m_ShipDeckVAO.DrawGL();
	m_ShipDeckVAO.DeselectGL();

	m_pRE->GLSL().ModelTexShader().Deselect();
}

void AuBodyRenderWidget::setVisible(bool visible)
{
	QOpenGLWidget::setVisible(visible);
	//if(visible)
		//update();
}

void AuBodyRenderWidget::SetBody(const DhDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, COMMODITYID clothing,
		COMMODITYID helmet, quint8 visorState)
{
	if(m_pBodyDel)
		qWarning() << "warning: AuBodyRenderWidget::SetBody leaked some memory";
	m_AnimaliaDel = m_Animalia;	// This will leak memory if SetBody is called more than once per render frame.
	m_Animalia = 0;
	m_pBodyDel = m_pBody;	// This will leak memory if SetBody is called more than once per render frame.
	m_pBody = new DpBody(chroma == AuDNABase::Alpha ? Male : Female, hand == LeftHanded, dna);
	m_Armor = CID_Invalid;
	m_Clothes = clothing;
	m_Helmet = helmet;
	m_HelmetVisor = visorState;
}

bool AuBodyRenderWidget::IsChange(const AuDNA& dna, AuDNABase::CHROMATID chroma, COMMODITYID clothing, COMMODITYID armor,
		COMMODITYID helmet, AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave) const
{
	if(!m_Animalia || m_pBody)
		return true;
	if(m_Animalia->Chromatid() != chroma)
		return true;
	if(m_Animalia->WornClothing() != clothing)
		return true;
	if(m_Animalia->WornArmor() != armor)
		return true;
	if(m_Animalia->WornHat() != helmet)
		return true;
	if(m_Animalia->Haircut() != haircut)
		return true;
	if(m_Animalia->Shave() != shave)
		return true;
	if(m_Animalia->DNA() != dna)
		return true;
	return false;
}

void AuBodyRenderWidget::SetAnimalia(const AuDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, COMMODITYID clothing,
		COMMODITYID armor, COMMODITYID helmet, quint8 visorState, AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave)
{
	if(m_Animalia)
	{
		// Is the body changing enough to need a new model?
		if(m_Animalia->Chromatid() != chroma ||
				m_Animalia->DNA() != dna ||
				m_Animalia->Haircut() != haircut ||
				m_Animalia->Shave() != shave ||
				m_Animalia->WornArmor() != armor ||
				m_Animalia->WornClothing() != clothing ||
				m_Animalia->WornHat() != helmet)
		{
			if(m_AnimaliaDel)
				qWarning() << "warning: AuBodyRenderWidget::SetAnimalia leaked some memory";
			m_AnimaliaDel = m_Animalia;	// This will leak memory if SetBody is called more than once per render frame.
			m_Animalia = 0;
		}
	}

	if(!m_Animalia)
	{
		m_Animalia = new AuAnimalia(dna, chroma, hand, Mature);
		m_Animalia->SetArmor(armor);
		m_Animalia->SetClothing(clothing);
		m_Animalia->SetHat(helmet);
		m_Animalia->SetHairStyle(haircut, shave);
	}

	m_pBodyDel = m_pBody;	// This will leak memory if SetBody is called more than once per render frame.
	m_pBody = 0;
	m_Armor = armor;
	m_Clothes = clothing;
	m_Helmet = helmet;
	m_HelmetVisor = visorState;
}

void AuBodyRenderWidget::SetShowScene(bool show)
{
	m_bShowScene = show;
}

void AuBodyRenderWidget::SetId(const AuId& id)
{
	m_Id = id;
}

void AuBodyRenderWidget::SetHandedness(HANDEDNESS hand)
{
	if(m_pBody)
		m_pBody->SetLeftHanded(hand == LeftHanded);
	else if(m_Animalia)
		m_Animalia->SetHandedness(hand);
}

void AuBodyRenderWidget::SetPose(const AuPose& pose, bool bendDominantArm, const AuPose& mouth)
{
	m_BendArm = bendDominantArm;
	if(m_pBody)
		m_pBody->Pose(pose, m_BendArm, 1);
	else if(m_Animalia)
	{
		m_Animalia->Pose(AuId(0), pose, m_BendArm, 1);
		m_Animalia->PoseMouth(AuId(0), mouth);
	}
}

void AuBodyRenderWidget::Animate()
{
	if(m_pBody)
		m_pBody->AnimatePose(m_BendArm, 1);
	else if(m_Animalia)
		m_Animalia->AnimatePose(AuId(0), AuElapsedTimer::Now());
}

AuPose AuBodyRenderWidget::CurrentGeneralPose() const
{
	if(m_pBody)
		return m_pBody->CurrentGeneralPose();
	else if(m_Animalia)
		return m_Animalia->CurrentGeneralPose();
	return AuPose::Stand;
}

int AuBodyRenderWidget::AvatarCheck() const
{
	int dnaCheck = 0;
	if(m_pBody)
	{
		// Check ability points and hit points.
		if(!m_pBody->IsAbilityPointsValidForPerson())
			dnaCheck |= 0x0001;
		if(!m_pBody->IsHitPointsValidForPerson())
			dnaCheck |= 0x0002;

		// Check body geometry in my standing pose.
		AuPose savePose = m_pBody->CurrentGeneralPose();
		AuPose standPose;
		if(m_pBody->DNA().BodyCarriage == 0 || m_pBody->DNA().BodyCarriage == 1)
			standPose = AuPose::Crouch;
		else
			standPose = AuPose::Stand;
		m_pBody->Pose(standPose, true, 1);
		if(!m_pBody->IsCameraHeightValidForPerson())
			dnaCheck |= 0x0004;
		if(!m_pBody->IsCgHeightValidForPerson())
			dnaCheck |= 0x0008;
		if(!m_pBody->IsHeadHeightValidForPerson())
			dnaCheck |= 0x0010;

		// Check body geometry when sitting at a console.
		m_pBody->Pose(AuPose::ConsoleCommand, true, 1);
		if(!m_pBody->IsCameraHeightValidForPerson())
			dnaCheck |= 0x0020;
		if(!m_pBody->IsHeadHeightValidForPerson())
			dnaCheck |= 0x0040;

		m_pBody->Pose(savePose, true, 1);
	}
	else if(m_Animalia)
	{
		dnaCheck = m_Animalia->AvatarCheck();

		// We must also check the opposite chromatid.
		AuDNA::CHROMATID otherChroma;
		if(m_Animalia->Chromatid() == AuDNABase::Alpha)
			otherChroma = AuDNABase::Beta;
		else
			otherChroma = AuDNABase::Alpha;
		AuAnimalia* otherHalf = new AuAnimalia(m_Animalia->DNA(), otherChroma, m_Animalia->Handedness(), Mature);
		int otherCheck = otherHalf->AvatarCheck();
		delete otherHalf;

		dnaCheck |= otherCheck << 16;
	}
	else
		dnaCheck = 0x8000;
	return dnaCheck;
}

void AuBodyRenderWidget::ResetView()
{
	m_fRotation = AuPoint(0,0,15*DEG2RAD);
    m_fTranslation = AuPoint(0,0,0);
}

const DhDNA& AuBodyRenderWidget::DNA3() const
{
	Q_ASSERT(m_pBody);
	return m_pBody->DNA();
}

const AuDNA& AuBodyRenderWidget::DNA4() const
{
	Q_ASSERT(m_Animalia);
	return m_Animalia->DNA();
}

float AuBodyRenderWidget::CGHeight() const
{
	if(m_pBody)
		return m_pBody->CGHeight(true);
	else if(m_Animalia)
		return m_Animalia->CGHeight();
	return 0;
}

float AuBodyRenderWidget::EyeHeight() const
{
	if(m_pBody)
		return m_pBody->CGHeight(true) + m_pBody->CameraLocation().z;
	else if(m_Animalia)
		return m_Animalia->CGHeight() + m_Animalia->CameraLocation().z;
	return 0;
}

float AuBodyRenderWidget::HeadHeight() const
{
	if(m_pBody)
		return m_pBody->CGHeight(true) + m_pBody->HeadLocation().z;
	else if(m_Animalia)
		return m_Animalia->CGHeight() + m_Animalia->HeadLocation().z;
	return 0;
}

int AuBodyRenderWidget::MaxHitPoints() const
{
	if(m_pBody)
		return m_pBody->MaxHitPoints();
	else if(m_Animalia)
		return m_Animalia->MaxHitPoints();
	return 0;
}

int AuBodyRenderWidget::AbilityPointsUsed() const
{
	if(m_pBody)
		return m_pBody->AbilityPointsUsed();
	else if(m_Animalia)
		return m_Animalia->AbilityPointsUsed();
	return 0;
}

//static
void AuBodyRenderWidget::ShipTexture(SHIP_TEXTURE tx, float& s, float& t, float& u, float& v)
{
	class TexPos
	{
	public:
		SHIP_TEXTURE texId;
		int ox, oy;			// pixel coordinate of lower left corner of
							// texture (pos y goes up)
		int sx, sy;			// size of texture in pixels

		TexPos(SHIP_TEXTURE shipTexture, int offsetX, int offsetY, int sizeX, int sizeY)
		{
			texId = shipTexture;
			ox = offsetX;
			oy = offsetY;
			sx = sizeX;
			sy = sizeY;
		}
	};

	const TexPos texPos[] =
	{
		// NOTE: Y goes UP. To use a top relative Y (like from gimp), subtract from 4096. e.g. 12 pixels from the top is 4096-12.
		// NOTE: values are based on a texture that is 128x4096. Our current texture is 256x8192, which is 2x all these values.
		TexPos(ST_Berth, 0, 480, 128, 32),
		TexPos(ST_BerthBottom, 0, 480, 24, 12),
		TexPos(ST_LargeDoorClosed, 0, 2176, 128, 128),
		TexPos(ST_LargeDoorHalfOpen, 0, 2048, 128, 128),
		TexPos(ST_LargeDoorOpen, 0, 1920, 128, 128),
		TexPos(ST_LargeCrewDoorClosed, 0, 2560, 128, 128),
		TexPos(ST_LargeCrewDoorHalfOpen, 0, 2432, 128, 128),
		TexPos(ST_LargeCrewDoorOpen, 0, 2304, 128, 128),
		TexPos(ST_LargeHullDoorClosed, 0, 2944, 128, 128),
		TexPos(ST_LargeHullDoorHalfOpen, 0, 2816, 128, 128),
		TexPos(ST_LargeHullDoorOpen, 0, 2688, 128, 128),
		TexPos(ST_CrewDoorClosed, 0, 1536, 64, 128),
		TexPos(ST_CrewDoorHalfOpen, 64, 1536, 64, 128),
		TexPos(ST_CrewDoorOpen, 0, 1408, 64, 128),
		TexPos(ST_CrewHatchClosed, 0, 896, 64, 64),
		TexPos(ST_CrewHatchHalfOpen, 64, 960, 64, 64),
		TexPos(ST_CrewHatchOpen, 0, 960, 64, 64),
		TexPos(ST_GravityDrive, 32, 4096-3520, 32, 32),
		TexPos(ST_HullBottom, 0, 3456, 128, 128),
		TexPos(ST_HullDoorClosed, 0, 1792, 64, 128),
		TexPos(ST_HullDoorHalfOpen, 64, 1792, 64, 128),
		TexPos(ST_HullDoorOpen, 0, 1664, 64, 128),
		TexPos(ST_HullHatchClosed, 0, 1024, 64, 64),
		TexPos(ST_HullHatchHalfOpen, 64, 1088, 64, 64),
		TexPos(ST_HullHatchOpen, 0, 1088, 64, 64),
		TexPos(ST_HullTop, 0, 3584, 128, 128),
		TexPos(ST_HullTopBottomWindow, 0, 3200, 128, 128),
		TexPos(ST_HullWall, 0, 3712, 128, 128),
		TexPos(ST_HullWallWindow, 0, 3328, 128, 128),
		TexPos(ST_IntCeiling, 0, 0, 128, 128),
		TexPos(ST_IntDoorClosed, 0, 4096-2816, 64, 128),
		TexPos(ST_IntDoorHalfOpen, 64, 4096-2816, 64, 128),
		TexPos(ST_IntDoorOpen, 0, 4096-2944, 64, 128),
		TexPos(ST_IntFloor, 0, 3840, 128, 128),
		TexPos(ST_IntHatchClosed, 0, 4096-3328, 64, 64),
		TexPos(ST_IntHatchHalfOpen, 64, 4096-3264, 64, 64),
		TexPos(ST_IntHatchOpen, 0, 4096-3264, 64, 64),
		TexPos(ST_IntWallEndCap, 96, 4096-2944, 32, 128),
		TexPos(ST_IntWallFinished, 0, 3968, 128, 128),
		TexPos(ST_IntWallFill, 64, 4096-2944, 32, 128),
		TexPos(ST_IntWallCargo, 0, 256, 128, 128),
		TexPos(ST_IntWallFillCargo, 96, 4096-2688, 32, 128),
		TexPos(ST_IntWallEquipment, 0, 128, 128, 128),
		TexPos(ST_IntWallFillEquipment, 64, 4096-2688, 32, 128),
		TexPos(ST_Ladder, 0, 4096-3456, 64, 128),
		TexPos(ST_LadderPole, 0, 4096-3456, 11, 128),
		TexPos(ST_LifeSupport, 64, 4096-3552, 32, 32),
		TexPos(ST_PowerPlant, 32, 4096-3488, 32, 32),
		TexPos(ST_Railing, 96, 4096-3488+16, 32, 16),
		TexPos(ST_RailingPost, 96, 4096-3488, 16, 16),
		TexPos(ST_Seat, 64, 4096-3520, 32, 32),
		TexPos(ST_Sensor, 0, 4096-3584, 32, 128),
		TexPos(ST_SensorTop, 0, 4096-3584, 2, 2),
		TexPos(ST_Shield, 32, 4096-3584, 32, 32),
		TexPos(ST_Surgery, 64, 4096-3456, 64, 128),
		TexPos(ST_TransporterPad, 64, 4096-3488, 32, 32),
		TexPos(ST_Turret, 0, 3072, 128, 128),
		TexPos(ST_WeaponBay, 64, 4096-3584, 32, 32),
		TexPos(ST_FTLDrive, 32, 4096-3552, 32, 32),
		TexPos(ST_StairCasing, 0, 416, 128, 32),
		TexPos(ST_StairRiser, 0, 412, 128, 36),					// Casing with a strip of tread at the BOTTOM edge, for stair rises.
		TexPos(ST_StairTread, 0, 384, 128, 32),
	};
	int texPosCount = sizeof(texPos)/sizeof(TexPos);
	if(tx > texPosCount)
		tx = ST_Seat;	// This will stand out.
	Q_ASSERT(texPos[tx].texId == tx);	// Verify data validity.

	// Calculate texture coordinates.
	const double maxx = 128;
	const double maxy = 4096;
	if(tx == ST_IntCeiling)
	{
		s = 1-(texPos[tx].ox + .5) / maxx;
		u = 1-(texPos[tx].ox + texPos[tx].sx - .5) / maxx;
	}
	else
	{
		s = (texPos[tx].ox + .5) / maxx;
		u = (texPos[tx].ox + texPos[tx].sx - .5) / maxx;
	}
	
	t = (texPos[tx].oy + .5) / maxy;
	v = (texPos[tx].oy + texPos[tx].sy - .5) / maxy;
}

