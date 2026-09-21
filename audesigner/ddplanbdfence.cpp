//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanbdfence.cpp,v 1.15 2022/04/15 16:39:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddplanbdfence.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesignerview.h"
#include "augleffects.h"
#include "auscparts.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>

DdPlanBdFence::DdPlanBdFence(int fenceStyle, quint32 customDesignId, COMMODITYID constructionMaterial, AuDesignerView* view) :
	DdCommand(view)
{
	m_CustomDesignId = customDesignId;
	m_FenceStyle = fenceStyle;
	m_Material = constructionMaterial;
	m_ShiftDown = false;
	m_StartEstablished = false;
}

DdPlanBdFence::~DdPlanBdFence()
{
}

bool DdPlanBdFence::OnKeyUp(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;
	case Qt::Key_Shift:
		m_ShiftDown = false;
		break;
	}
	return true;
}

bool DdPlanBdFence::OnKeyDown(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;
	case Qt::Key_Up:
		{
			if(m_Material == CID_Wood)
				SetMaterial(CID_Lumber);
			else if(m_Material == CID_Lumber)
				SetMaterial(CID_Stone);
			else if(m_Material == CID_Stone)
				SetMaterial(CID_Metal);
			else if(m_Material == CID_Metal)
				SetMaterial(CID_Magmium);
			else if(m_Material == CID_Magmium)
				SetMaterial(CID_Vulcium);
			else if(m_Material == CID_Vulcium)
				SetMaterial(CID_Adamantine);
			else if(m_Material == CID_Adamantine)
				SetMaterial(CID_Wood);
		}
		break;
	case Qt::Key_Down:
		{
			if(m_Material == CID_Wood)
				SetMaterial(CID_Adamantine);
			else if(m_Material == CID_Adamantine)
				SetMaterial(CID_Vulcium);
			else if(m_Material == CID_Vulcium)
				SetMaterial(CID_Magmium);
			else if(m_Material == CID_Magmium)
				SetMaterial(CID_Metal);
			else if(m_Material == CID_Metal)
				SetMaterial(CID_Stone);
			else if(m_Material == CID_Stone)
				SetMaterial(CID_Lumber);
			else if(m_Material == CID_Lumber)
				SetMaterial(CID_Wood);
		}
		break;
	case Qt::Key_Home:
	case Qt::Key_Left:
	case Qt::Key_Right:
		break;
	case Qt::Key_Space:
		if(!m_StartLoc.IsZero())
			m_StartEstablished = !m_StartEstablished;
		break;
	case Qt::Key_G:
		{
			AuPoint postLoc;
			AuId platformId;
			if(m_pWnd->FindClosestFencePost(postLoc, platformId))
			{
				// A post was found.
				if(m_StartEstablished)
				{
					auApp->BdBuildFence(m_StartLoc, postLoc, m_FenceStyle, platformId, m_CustomDesignId, FenceMaterial());

					// Continue the next segment where this one left off.
					m_StartLoc = postLoc;
					m_StartEstablished = false;
				}
				else
				{
					// Start a segment at the post location.
					m_StartLoc = postLoc;
					m_StartEstablished = true;
				}
			}
		}
		break;
	case Qt::Key_Shift:
		m_ShiftDown = true;
		break;
	}
	return true;
}

void DdPlanBdFence::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
		ConstructFence(e->pos());
}

// Render "cursor" objects in the scene.
void DdPlanBdFence::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson *pSceneAtom,
		const QPoint &pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	Q_UNUSED(snappedToExistingShape);
	DaAtom *pEnv = pSceneAtom->FindEnvironmentAtom();
	if(!pEnv)
		return;

	// On second end of a road segment, we can show some useful information.
	bool roadInfo = false;
	double roadLen = 0;
	double roadHeading = 0;
	double roadIncline = 0;

	const DaWorldLL* pTerrain;			// set by call to TestFenceLocation
	AuPoint fBuildLoc;				// set by call to TestFenceLocation
	DaBd* exBd;
	DaBd* onPlatform;
	QString cityName;
	int testloc = TestFenceLocation(pSceneAtom, pxCursor, pTerrain, fBuildLoc, exBd, &onPlatform, cityName);
	if(testloc != 1)
	{
		// Create a cursor to represent the construction area.
		double buildAlt = pTerrain->AltitudeMSL(fBuildLoc);
		AuPoint vUp = -pTerrain->GravityDir(fBuildLoc);			// Up vector with respect to gravity.

		// Get transforms needed to calculate the location of the vertices in the scene.
		AuMatrix4x4 tScene;
		pSceneAtom->GetTransformRelativeToParent(tScene, pEnv);
		tScene = tScene.Inverse();
		AuMatrix4x4 tTerr;
		pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
		AuMatrix4x4 t = tModel * tScene * tTerr;

		double Xwidth2 = 1;		// 1/2 the width of the X
		if(!m_StartEstablished || m_StartLoc.FuzzyEquals(fBuildLoc))
		{
			// Draw an X cursor at the potential start location.
			AuPoint vX;
			vUp.ArbitraryAxis(vX);									// Returns a unit vector.
			AuPoint vY = vUp.Cross(vX);								// Result will be a unit vector.
			AuPoint vXL = vX * Xwidth2;
			AuPoint vYL = vY * Xwidth2;
			QList<AuPoint> lvlPts;			// Terrain level perimeter points.
			lvlPts.append(fBuildLoc + vXL + vYL);
			lvlPts.append(fBuildLoc - vXL - vYL);
			lvlPts.append(fBuildLoc + vXL - vYL);
			lvlPts.append(fBuildLoc - vXL + vYL);

			// Use simple shader so lines will be super bright, even at night.
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(projection * t);

			// Draw lines at the cursor location.
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_BLEND);

			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if(testloc == 0)
				re.GLSL().LineShader().SetColor(.1f, .9f, .1f, .9f);
			else
				re.GLSL().LineShader().SetColor(.9f, .1f, .1f, .9f);

			// Compile a list of vertex locations.
			QList<AuPointAlpha> verts;
			QList<AuPoint>::const_iterator gpi = lvlPts.constBegin();
			for(; gpi != lvlPts.constEnd(); gpi++)
				verts << AuPointAlpha((*gpi).x, (*gpi).y, (*gpi).z);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
		else
		{
			// Draw a linear cursor from the start location to the potential end location.
			// Right/left designations are in the context of standing at m_StartLoc and looking down the road segment to fBuildLoc.
			double startAlt = pTerrain->AltitudeMSL(m_StartLoc);
			//AuPoint vUpStart = -pTerrain->GravityDir(m_StartLoc);				// Up vector with respect to terrain.
			AuPoint centerVec = fBuildLoc - m_StartLoc;
			double centerLen = centerVec.Length();
			Q_ASSERT(centerLen != 0);
			centerVec /= centerLen;

			// On second end of a road segment, we can show some useful information.
			AuPoint north, east, up;
			if(pTerrain->CompassVectors(m_StartLoc, north, east, up))
			{
				roadInfo = true;
				roadLen = centerLen / FeetPerMeter();	// In meters.
				AuPoint roadDir(centerVec);				// Unit vector

				// Calculate the heading using compass vectors.
				double rx = roadDir.Dot(east);
				double ry = roadDir.Dot(north);
				double rz = roadDir.Dot(up);
				roadHeading = PI/2. - AuPoint(rx, ry).ArcTangent_2D();
				if(roadHeading < 0)
					roadHeading += PI*2;
				roadHeading *= RAD2DEG;

				if(g_AuClient.RoadType() == 1)
					roadIncline = 0;
				else
					roadIncline = asin(rz) * RAD2DEG;
			}

			// Build road edge lines and center lines.
			// Start with the starting locations.
			QList<AuPoint> groundCenter;

			// Integrate along the length so we conform to the curvature of the world.
			double seglen = 1*FeetPerMeter();
			int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
			bool atEnd = false;
			for(int seg=0; seg<=segs && !atEnd; seg++)
			{
				double lenAlongSeg = seglen * seg;
				if(lenAlongSeg >= centerLen)
				{
					// We are at the end. Truncate the last segment for perfect ending.
					lenAlongSeg = centerLen;				
					atEnd = true;
				}
				double segRatio = lenAlongSeg / centerLen;
				double altAtSegLoc = startAlt + (buildAlt - startAlt) * segRatio;	// Linear alt change from one end to the other.
				AuPoint segLoc = m_StartLoc + centerVec * lenAlongSeg;
				pTerrain->ElevateLocMSL(segLoc, altAtSegLoc);

				if(onPlatform)
				{
					groundCenter.append(segLoc);
				}
				else
				{
					// Add ground level points.
					AuGlobeLL gndLL;
					if(!pTerrain->LLOfLoc(gndLL, segLoc))
						continue;
					AuPoint gndPt = pTerrain->GetTerrainLocation(gndLL);
					groundCenter.append(gndPt);
				}
			}

			// Use simple shader so lines will be super bright, even at night.
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(projection * t);

			// Draw lines at the cursor location.
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_BLEND);

			// Draw lines.
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.
			if(testloc == 0)
				re.GLSL().LineShader().SetColor(.1f, .9f, .1f, .9f);
			else
				re.GLSL().LineShader().SetColor(.9f, .1f, .1f, .9f);

			QList<AuPointAlpha> verts;
			for(QList<AuPoint>::const_iterator ri = groundCenter.constBegin(); ri != groundCenter.constEnd(); ri++)
				verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

			if(!verts.empty())
			{
				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
		}

		// Create another cursor to show the existing development that is affected.
		QColor hitColor;
		if(testloc == 0)
		{
			hitColor.setRedF(.1f);
			hitColor.setGreenF(.9f);
			hitColor.setBlueF(.1f);
			hitColor.setAlphaF(.9f);
		}
		else
		{
			hitColor.setRedF(.9f);
			hitColor.setGreenF(.1f);
			hitColor.setBlueF(.1f);
			hitColor.setAlphaF(.9f);
		}
		if(exBd)
		{
			re.GLSL().LineShader().SetColor(hitColor);
			exBd->RenderCursorHit_Shader(re, false);	// Building and site lines.
		}

		QColor roadColor(163, 104, 5, 100);				// orange
		if(pTerrain->HasSubatoms())
		{
			QColor buildingColor(136, 35, 214, 100);		// purple
			QColor landscapeColor(5, 163, 70, 100);			// green
			DaAtomList subs;
			pTerrain->ListSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				if(*it == exBd)
					continue;
				DaBd* bd = dynamic_cast<DaBd*>(*it);
				if(!bd)
					continue;
				if(bd->GravityNormal().Dot(vUp) <= 0)
					continue;
				switch(bd->IndustryId())
				{
				case IID_Road:
					re.GLSL().LineShader().SetColor(roadColor);
					break;
				case IID_Landscape:
					re.GLSL().LineShader().SetColor(landscapeColor);
					break;
				default:
					re.GLSL().LineShader().SetColor(buildingColor);
					break;
				}
				bd->RenderCursorHit_Shader(re, true);	// Site lines only.
			}
		}

		AuGlobeLLF hitRoad;	// We don't hit roads with fences.
		const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
		geof.RenderCursor_Shader(*pTerrain, re, fBuildLoc, hitRoad, hitColor, roadColor);
		pTerrain->UnlockGeologicalFeatures();

		re.glEnable(GL_DEPTH_TEST);
		re.glDisable(GL_BLEND);

		re.GLSL().LineShader().Deselect();
	}

	if(testloc != 0)
	{
		int secsTo = m_LastTime.elapsed()/1000;
		if(secsTo > 4)
			return;
		float msecsTo = m_LastTime.elapsed()/1000.0f;
		if(msecsTo < 0)
			msecsTo = 1;
		// We must calculate the pixel location of the text to draw text on
		// the screen using renderText, rather than at 3D Z-depth.
		float r = .9f;
		float g = .35f;
		float b = .1f;
		QFont font;
		font.setPointSize(10);
		font.setBold(true);

		QPoint cursorPos = m_pWnd->cursor().pos();
		cursorPos = glw->mapFromGlobal(cursorPos);
		QPoint drawPos = cursorPos+QPoint(0,36);
		float fAlpha = 1./(1+msecsTo);

		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		QColor tcolor;
		tcolor.setRgbF(0,0,0,fAlpha);
		re.RenderText(drawPos.x(), drawPos.y(), m_sReason, font, tcolor, glw->width(), glw->height(),
				Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, 200);
		tcolor.setRgbF(r,g,b,fAlpha);
		re.RenderText(drawPos.x()-1, drawPos.y()-1, m_sReason, font, tcolor, glw->width(), glw->height(),
				Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, 200);
		re.glDisable(GL_BLEND);
	}
	else if(roadInfo)
	{
		// We must calculate the pixel location of the text to draw text on
		// the screen using renderText, rather than at 3D Z-depth.
		float g = .9f;
		float r = .35f;
		float b = .1f;
		QFont font;
		font.setPointSize(10);

		QPoint cursorPos = m_pWnd->cursor().pos();
		cursorPos = glw->mapFromGlobal(cursorPos);
		QPoint drawPos = cursorPos+QPoint(0,36);

		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(roadInfo)
		{
			QString head;
			if(roadHeading < 22.5)
				head = "N";
			else if(roadHeading < 45+22.5)
				head = "NE";
			else if(roadHeading < 90+22.5)
				head = "E";
			else if(roadHeading < 135+22.5)
				head = "SE";
			else if(roadHeading < 180+22.5)
				head = "S";
			else if(roadHeading < 225+22.5)
				head = "SW";
			else if(roadHeading < 270+22.5)
				head = "W";
			else if(roadHeading < 315+22.5)
				head = "NW";
			else
				head = "N";

			QString sRoad;
			if(fabs(roadIncline) >= .05)
				sRoad = QObject::tr("%L1m Heading %5 %2%3 Incline %4%3")
					.arg(roadLen, 0,'f',1)
					.arg(roadHeading, 0,'f',1)
					.arg(g_Au.DegreeSymbol())
					.arg(roadIncline, 0,'f',1)
					.arg(head);
			else
				sRoad = QObject::tr("%L1m Heading %4 %2%3")
					.arg(roadLen, 0,'f',1)
					.arg(roadHeading, 0,'f',1)
					.arg(g_Au.DegreeSymbol())
					.arg(head);

			QColor tcolor;
			tcolor.setRgbF(0,0,0,1);
			re.RenderText(drawPos.x(), drawPos.y(), sRoad, font, tcolor, glw->width(), glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextSingleLine, 200);
			tcolor.setRgbF(r,g,b,1);
			re.RenderText(drawPos.x()-1, drawPos.y()-1, sRoad, font, tcolor, glw->width(), glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextSingleLine, 200);
			drawPos.setY(drawPos.y() + 16);
		}

		re.glDisable(GL_BLEND);
	}
}

// Set appropriate device cursor. Default sets Cross cursor if
// pSceneAtom != 0, otherwise sets Arrow cursor.
void DdPlanBdFence::SetCursor(const AuMatrix4x4& tCamera, DaPerson* sceneAtom, const QPoint& pxCursor)
{
	Q_UNUSED(tCamera);
	const DaWorldLL* pTerrain;
	AuPoint fBuildLoc;
	DaBd* exBd;
	DaBd* onPlatform;
	QString cityName;
	int testloc = TestFenceLocation(sceneAtom, pxCursor, pTerrain, fBuildLoc, exBd, &onPlatform, cityName);
	if(testloc == 0)
	{
		// Location is good.
		m_pWnd->setCursor(QCursor(Qt::CrossCursor));
	}
	else
	{
		QString sLastReason = m_sReason;
		// Location is bad. Show a cursor that hints at the reason.
		m_sReason = DaWorldLL::TestBdResultText(testloc);
		if(m_sReason != sLastReason)
			m_LastTime = AuElapsedTimer();
		QPixmap pix(DaWorldLL::TestBdResultPixmap(testloc));
		if(pix.isNull())
			m_pWnd->setCursor(QCursor(Qt::ForbiddenCursor));
		else
			m_pWnd->setCursor(QCursor(pix, 0,0));
	}
}

CONSTRUCTIONPROCESSID DdPlanBdFence::ConstructionProcessId() const
{
	switch((AuScFence::STYLE)m_FenceStyle)
	{
	case AuScFence::Posts:
		return CPID_FencePosts;
	case AuScFence::PostsT:
		return CPID_FencePostsT;
	case AuScFence::Wire:
		return CPID_FenceWire;
	case AuScFence::WireT:
		return CPID_FenceWireT;
	case AuScFence::Range:
		return CPID_FenceRange;
	case AuScFence::RangeT:
		return CPID_FenceRangeT;
	case AuScFence::ChainLink:
		return CPID_FenceChainLink;
	case AuScFence::ChainLinkRazorWire:
		return CPID_FenceChainLinkRazorWire;
	case AuScFence::SplitRail:
		return CPID_FenceSplitRail;
	case AuScFence::WoodYard:
		return CPID_FenceWoodYard;
	case AuScFence::SandBagWall:
		return CPID_FenceSandBagWall;
	case AuScFence::BrickWall:
		return CPID_FenceBrickWall;
	case AuScFence::StoneWall:
		return CPID_FenceStoneWall;
	case AuScFence::Custom:
		return CPID_FenceCustom;
	case AuScFence::STYLE_Count:
		break;
	}
	return CPID_FencePosts;
}

void DdPlanBdFence::ConstructFence(const QPoint& pxCursor)
{
	if(!m_StartEstablished)
	{
		// This is the first location of the road segment.
		DaPerson* person;
		m_pWnd->LockSceneAtom(&person);

		AuIdList idBuildList;
		QStringList sBuildList;

		const DaWorldLL* pTerrain;			// set by call to TestFenceLocation
		AuPoint fBuildLoc;				// set by call to TestFenceLocation
		DaBd* exBd;
		DaBd* onPlatform;
		QString cityName;
		int testloc = TestFenceLocation(person, pxCursor, pTerrain, fBuildLoc, exBd, &onPlatform, cityName);
		AuId platformId;
		if(onPlatform)
			platformId = onPlatform->Id();
		if(testloc == 0)
		{
			m_StartEstablished = true;
			m_StartLoc = fBuildLoc;
			m_pWnd->UnlockSceneAtom();
		}
		else
		{
			// Location is not ok.
			m_pWnd->UnlockSceneAtom();
			m_LastTime.restart();
		}
	}
	else
	{
		// This is a subsequent segment location. Try to create a road section from this point to the start location.
		DaPerson* person;
		m_pWnd->LockSceneAtom(&person);

		const DaWorldLL* pTerrain;			// set by call to TestFenceLocation
		AuPoint fBuildLoc;				// set by call to TestFenceLocation
		DaBd* exBd;
		DaBd* onPlatform;
		QString cityName;
		int testloc = TestFenceLocation(person, pxCursor, pTerrain, fBuildLoc, exBd, &onPlatform, cityName);
		AuId platformId;
		if(onPlatform)
			platformId = onPlatform->Id();
		if(testloc == 0)
		{
			m_pWnd->UnlockSceneAtom();
			auApp->BdBuildFence(m_StartLoc, fBuildLoc, m_FenceStyle, platformId, m_CustomDesignId, FenceMaterial());

			// Continue the next segment where this one left off.
			m_StartLoc = fBuildLoc;
			m_StartEstablished = false;
		}
		else
		{
			// Location is not ok.
			m_pWnd->UnlockSceneAtom();
			m_LastTime.restart();
		}
	}
}

// Tests 'pxCursor' for ok to build.
// Sets 'pTerrain' to the terrain patch.
// Sets 'fBuildLoc' to the terrain location.
int DdPlanBdFence::TestFenceLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
		DaBd*& exBd, DaBd** onPlatform, QString& cityName) const
{
	exBd = 0;

	// Where is the cursor on the terrain?
	*onPlatform = 0;
	pTerrain = dynamic_cast<const DaWorldLL*>(m_pWnd->FindTerrainLocationAtPixel(sceneAtom, pxCursor, fBuildLoc, onPlatform));
	if(!pTerrain)
		return 1;

	// Choose material for build test. It just determines if the road can be built on an inferno.
	COMMODITYID mat = FenceMaterial();

	// When start location has been established, we need to pass the starting location also.
	// We may also need to constrain the angle.
	if(m_StartEstablished && !m_StartLoc.FuzzyEquals(fBuildLoc))
	{
		// Start location has been established. Are we constraining the angle?
		if(m_ShiftDown)
		{
			// Constrain fBuildLoc to be on an even 5 degree angle relative to the start location.
			AuPoint bvec(fBuildLoc - m_StartLoc);
			if(*onPlatform)
			{
				AuPoint pvec = (*onPlatform)->Transform().UnProjectVector(bvec);	// In the space of the platform.
				double ang = pvec.ArcTangent_2D();

				// Round to the road angle constraint set by the user.
				double angdeg = ang * RAD2DEG;
				angdeg = FROUND(angdeg, g_AuClient.RoundingAngle());
				double nang = angdeg * DEG2RAD;

				pvec = pvec.RotateZ(nang - ang);
				fBuildLoc = m_StartLoc + (*onPlatform)->Transform().ProjectVector(pvec);
			}
			else
			{
				AuPoint vUp(-pTerrain->GravityDir(m_StartLoc));		// Unit vector pointing up vs gravity.
				AuPoint vX;
				vUp.ArbitraryAxis(vX);								// Returns a unit vector.
				AuPoint vY = vUp.Cross(vX);							// Result will be a unit vector.
				double x = bvec.Dot(vX);
				double y = bvec.Dot(vY);
				double ang = atan2(y,x);							// Angle to fBuildLoc in the plane of vUp.

				// Round to the road angle constraint set by the user.
				double angdeg = ang * RAD2DEG;
				angdeg = FROUND(angdeg, g_AuClient.RoundingAngle());
				ang = angdeg * DEG2RAD;

				double blen = bvec.Length();
				x = cos(ang) * blen;
				y = sin(ang) * blen;
				AuPoint bloc = m_StartLoc + vX * x + vY * y;		// Constrained point in the plane of vUp at alt of m_StartLoc.
				AuGlobeLLF bll;
				if(pTerrain->LLOfLoc(bll, bloc))
					fBuildLoc = pTerrain->GetTerrainLocation(bll);
			}
		}
		return pTerrain->TestBdLocation(IID_Fence, sceneAtom->EmpireId(), 0, mat, fBuildLoc, 1, 0, *onPlatform, exBd, &m_StartLoc,
				cityName);
	}
	else
		return pTerrain->TestBdLocation(IID_Fence, sceneAtom->EmpireId(), 0, mat, fBuildLoc, 1, 0, *onPlatform, exBd, 0, cityName);
}

COMMODITYID DdPlanBdFence::FenceMaterial() const
{
	switch((AuScFence::STYLE)m_FenceStyle)
	{
	case AuScFence::Posts:
	case AuScFence::Wire:
	case AuScFence::Range:
	case AuScFence::SplitRail:
		return CID_Wood;
	case AuScFence::WoodYard:
		return CID_Lumber;
	case AuScFence::PostsT:
	case AuScFence::WireT:
	case AuScFence::RangeT:
	case AuScFence::ChainLink:
	case AuScFence::ChainLinkRazorWire:
		return CID_Metal;
	case AuScFence::SandBagWall:
		return CID_Textiles;
	case AuScFence::BrickWall:
	case AuScFence::StoneWall:
		return CID_Stone;
	case AuScFence::Custom:
		return m_Material;
	case AuScFence::STYLE_Count:
		break;
	}
	return CID_Invalid;
}

void DdPlanBdFence::SetMaterial(int constructionMaterial)
{
	m_Material = (COMMODITYID)constructionMaterial;
}

void DdPlanBdFence::SetConstructionProcess(const AuBdReport& report)
{
	//qDebug() << "DdPlanBdFence::SetConstructionProcess" << report.DesignId();
	m_CustomDesignId = report.DesignId();
}
