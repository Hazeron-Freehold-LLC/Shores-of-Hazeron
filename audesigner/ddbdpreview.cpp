//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdpreview.cpp,v 1.18 2022/04/15 16:39:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddbdpreview.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "augleffects.h"
#include "auframe.h"
#include "aunewcityname.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

DdBdPreview::DdBdPreview(AuDesignerView* view) : DdCommand(view)
{
	//qDebug() << "DdBdPreview::DdBdPreview build" << cp.DesignId() << "for industry" << iIndustryID;
	m_AlignRot = 99.f;
	m_NorthZ = 0.f;
	m_PlatformAltMSL = 100 * FeetPerMeter();
	m_RotAligned = false;
	m_RotZ = 0.f;
	m_RptOk = false;
	m_sReason = "";
}

DdBdPreview::~DdBdPreview()
{
}

bool DdBdPreview::OnKeyDown(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;
	case Qt::Key_Space:
		AlignToExistingDevelopment();
		break;

	case Qt::Key_Left:
		{
			// Rotate
			if(e->modifiers() & Qt::ShiftModifier)
				m_RotZ += DEG2RADf;
			else if(g_AuClient.RoundingAngle() > 0)
				m_RotZ += g_AuClient.RoundingAngle() * DEG2RADf;
			else
				m_RotZ += 45.f * DEG2RADf;
		}
		break;
	case Qt::Key_Right:
		{
			// Rotate
			if(e->modifiers() & Qt::ShiftModifier)
				m_RotZ -= DEG2RADf;
			else if(g_AuClient.RoundingAngle() > 0)
				m_RotZ -= g_AuClient.RoundingAngle() * DEG2RADf;
			else
				m_RotZ -= 45.f * DEG2RADf;
		}
		break;
	}
	return true;
}

void DdBdPreview::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
	{
		// Construct the building.
		ConstructBuilding(e->pos());
	}
}

void DdBdPreview::OnMouseWheel(QWheelEvent* e)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif
	if(m_Rpt.Structure() == BD_AerialPlatform)
	{
		double increment;
		if(e->modifiers()&Qt::ShiftModifier)
			increment = 1 * FeetPerMeter();
		else
			increment = 100 * FeetPerMeter();

		if(zDelta > 0)
			m_PlatformAltMSL += increment;
		else
		{
			m_PlatformAltMSL -= increment;
			if(m_PlatformAltMSL < .1)
				m_PlatformAltMSL = .1;
		}
	}
}

// Create device objects used to render the cursor.
void DdBdPreview::CreateDeviceObjects(AuGLEffects& re)
{
	if(!m_BdVAO.VAO())
	{
		DaPerson* person;
		m_pWnd->LockSceneAtom(&person);
		if(person)
		{
			DaDesigner* de = person->FindParentDesignerInEnv();
			if(de)
			{
				const AuBdDesign* design = dynamic_cast<const AuBdDesign*>(de->LockDesignForRead());
				if(design)
				{
					if(!m_RptOk)
					{
						design->ReportDesign(m_Rpt);
						m_RptOk = true;
					}
					AuPointArray edges;
					design->Parts().BuildingCursorSegments(edges);
					de->UnlockDesign();
					m_BdVAO.CreateForLines(re, edges, false, GL_STATIC_DRAW);
				}
				else
					de->UnlockDesign();
			}
		}
		m_pWnd->UnlockSceneAtom();
	}
}

// Delete device objects used to render the cursor.
void DdBdPreview::DeleteDeviceObjects(AuGLEffects&)
{
	if(m_BdVAO.VAO())
		m_BdVAO.DeleteGL();
}

// Render "cursor" objects in the scene.
void DdBdPreview::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	Q_UNUSED(snappedToExistingShape);
	//qDebug() << "DdBdPreview::RenderCursor cursor at" << pxCursor;
	DaAtom *pEnv = pSceneAtom->FindEnvironmentAtom();
	if(!pEnv)
		return;

	const DaWorldLL* pTerrain;			// set by call to TestBuildLocation
	AuPoint fBuildLoc;				// set by call to TestBuildLocation
	DaBd* exBd;
	m_AlignRot = 99.f;				// Invalid until set by existing development in TestBuildLocation.
	int testloc = TestBuildLocation(pSceneAtom, pxCursor, pTerrain, fBuildLoc, exBd);

	// Render lines of building site meshes.
	if(testloc != 1)
	{
		// Create a cursor to represent the construction area.
		QColor cursorColor;
		if(testloc == 0)
		{
			cursorColor.setRedF(.1f);
			cursorColor.setGreenF(.9f);
			cursorColor.setBlueF(.1f);
			cursorColor.setAlphaF(.9f);
		}
		else
		{
			cursorColor.setRedF(.9f);
			cursorColor.setGreenF(.1f);
			cursorColor.setBlueF(.1f);
			cursorColor.setAlphaF(.9f);
		}

		// Get transforms needed to calculate the location of the vertices in the scene.
		AuMatrix4x4 tScene;
		pSceneAtom->GetTransformRelativeToParent(tScene, pEnv);
		tScene = tScene.Inverse();
		AuMatrix4x4 tTerr;
		pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
		AuMatrix4x4 model = tModel * tScene * tTerr;

		AuPoint siteCen(m_Rpt.SiteCenter());
		AuMatrix4x4 mBld;
		pTerrain->BuildingConstructionTransform(fBuildLoc, ZRotation(), 0, siteCen, mBld);

		// Use simple shader so lines will be super bright, even at night.
		re.GLSL().LineShader().Select();
		re.GLSL().LineShader().SetColor(cursorColor);

		// Draw lines at the cursor location.
		re.glDisable(GL_DEPTH_TEST);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.

		if(m_BdVAO.VAO())
		{
			re.GLSL().LineShader().SetProjectionModelView(projection * model * mBld);
			m_BdVAO.SelectGL();
			m_BdVAO.DrawGL(GL_LINES);
			m_BdVAO.DeselectGL();
		}
		re.GLSL().LineShader().SetProjectionModelView(projection * model);

		// Render forward facing indicator.
		{
			double len = 20*FeetPerMeter();
			double aw2 = (2*FeetPerMeter())/2;	// 1/2 the width of the arrow head
			double al = 6*FeetPerMeter();
			double cr = FeetPerMeter();
			AuPointAlpha dl = mBld.Project(AuPoint(-cr, 0, 0));
			AuPointAlpha dr = mBld.Project(AuPoint(cr, 0, 0));
			AuPointAlpha db = mBld.Project(AuPoint(0, -cr, 0));
			AuPointAlpha df = mBld.Project(AuPoint(0, cr, 0));
			AuPointAlpha f = mBld.Project(AuPoint(0, len, 0));
			AuPointAlpha fl = mBld.Project(AuPoint(-aw2, len, 0));
			AuPointAlpha fp = mBld.Project(AuPoint(0, len + al, 0));
			AuPointAlpha fr = mBld.Project(AuPoint(aw2, len, 0));

			QList<AuPointAlpha> verts;
			verts << AuPointAlpha(df);
			verts << AuPointAlpha(dl);
			verts << AuPointAlpha(db);
			verts << AuPointAlpha(dr);
			verts << AuPointAlpha(df);
			verts << AuPointAlpha(f);
			verts << AuPointAlpha(fl);
			verts << AuPointAlpha(fp);
			verts << AuPointAlpha(fr);
			verts << AuPointAlpha(f);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(re, verts, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
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

			// Calculate the Z rotation needed to align with the building.
			AuGlobeLLF buildLL;
			if(pTerrain->LLOfLoc(buildLL, fBuildLoc))
				m_AlignRot = exBd->AlignmentRot(*pTerrain, buildLL);
		}

		QColor roadColor(163, 104, 5, 100);						// orange
		QColor alignRoadColor(255, 163, 8, 100);				// bright orange
		if(pTerrain->HasSubatoms())
		{
			QColor buildingColor(136, 35, 214, 100);			// purple
			QColor landscapeColor(5, 163, 70, 100);				// green
			DaAtomList subs;
			pTerrain->ListSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				if(*it == exBd)
					continue;
				DaBd* bd = dynamic_cast<DaBd*>(*it);
				if(!bd)
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
				bd->RenderCursorHit_Shader(re, false);	// Site lines only.
			}
		}

		// Show roads.
		AuGlobeLLF alignRoad;
		if(m_AlignRot == 99.f)
		{
			// Try aligning to a road.
			AuGlobeLLF alignLL;
			if(pTerrain->LLOfLoc(alignLL, fBuildLoc))
			{
				const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
				m_AlignRot = geof.AlignmentRot(*pTerrain, alignLL, 0, alignRoad);	// Returns 99.f if none is found.
				pTerrain->UnlockGeologicalFeatures();
			}
		}
		const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
		geof.RenderCursor_Shader(*pTerrain, re, fBuildLoc, alignRoad, alignRoadColor, roadColor);
		pTerrain->UnlockGeologicalFeatures();

		re.glEnable(GL_DEPTH_TEST);
		re.glDisable(GL_BLEND);
		re.GLSL().LineShader().Deselect();
	}

	if(testloc == 0)
	{
		// Location is good.
		QPoint cursorPos = m_pWnd->cursor().pos();
		cursorPos = glw->mapFromGlobal(cursorPos);
		QPoint drawPos = cursorPos+QPoint(0,32);

		QFont font;
		font.setPointSize(10);
		font.setBold(true);

		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		{
			double altMSL = pTerrain->AltitudeMSL(fBuildLoc) / FeetPerMeter();
			QString sOffset;
			sOffset = QObject::tr("%L1m MSL").arg(altMSL, 0,'f',1);

			QColor tcolor;
			tcolor.setRgbF(0,0,0,1);
			re.RenderText(drawPos.x(), drawPos.y(), sOffset, font, tcolor, glw->width(), glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextSingleLine, 200);
			tcolor.setRgbF(.1f,.9f,.1f,1);
			re.RenderText(drawPos.x()-1, drawPos.y()-1, sOffset, font, tcolor, glw->width(), glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextSingleLine, 200);
		}
		re.glDisable(GL_BLEND);
	}
	else
	{
		// There is a problem.
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
		QPoint drawPos = cursorPos+QPoint(0,32);
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
}

// Set appropriate device cursor. Default sets Cross cursor if pSceneAtom != 0, otherwise sets Arrow cursor.
void DdBdPreview::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* pSceneAtom, const QPoint& pxCursor)
{
	const DaWorldLL* pTerrain;
	AuPoint fBuildLoc;
	DaBd* exBd;
	int testloc = TestBuildLocation(pSceneAtom, pxCursor, pTerrain, fBuildLoc, exBd);
	if(testloc == 0)
	{
		// Location is good.
		Q_ASSERT(pTerrain);
		AuGlobeLLF buildLL;
		pTerrain->LLOfLoc(buildLL, fBuildLoc);
		AuPoint siteCen(m_Rpt.SiteCenter());
		AuMatrix4x4 mBld;
		pTerrain->BuildingConstructionTransform(fBuildLoc, ZRotation(), 0, siteCen, mBld);
		m_NorthZ = pTerrain->BuildingConstructionNorthAngle(mBld.Project(siteCen), 0);

		QPixmap pix(64,32);
		pix.fill(QColor(0,0,0,0));
		QPainter p(&pix);

		// Set the cursor using the picture.
		p.end();
		if(pix.isNull())
			m_pWnd->setCursor(QCursor(Qt::CrossCursor));
		else
			m_pWnd->setCursor(QCursor(pix, 0,0));
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

// Tests 'pxCursor' for ok to build.
// Sets 'pTerrain' to the terrain patch.
// Sets 'fBuildLoc' to the terrain location.
// Sets 'mat' to the building material to use.
// Sets 'levels' to the number of existing building levels at the location,
//  whether or not construction has been completed on all levels.
// Returns:
//  0 = ok
//  1 = build location world not found
int DdBdPreview::TestBuildLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
		DaBd*& exBd) const
{
	exBd = 0;

	// Where is the cursor on the terrain?
	pTerrain = dynamic_cast<const DaWorldLL*>(m_pWnd->FindTerrainLocationAtPixel(sceneAtom, pxCursor, fBuildLoc));
	if(!pTerrain)
		return 1;
	switch(m_Rpt.Structure())
	{
	case BD_Invalid:
	case BD_Biodome:
	case BD_Bridge:
	case BD_Building:
	case BD_Fence:
	case BD_GroundPlatform:
	case BD_RoadEnclosure:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_Tunnel:
	case BD_StreetLight:
	case BD_Count:
		break;
	case BD_SeaPlatform:
	case BD_SeaSurfaceBuilding:
		pTerrain->ElevateLocMSL(fBuildLoc, .1);	// Place it just above sea level, so it doesn't think it is under water.
		break;
	case BD_AerialPlatform:
		pTerrain->ElevateLocMSL(fBuildLoc, m_PlatformAltMSL);	// Place it at the selected flying altitude.
		break;
	}
	return pTerrain->TestBdPreviewLocation(&m_Rpt, fBuildLoc, ZRotation(), exBd);
}

void DdBdPreview::ConstructBuilding(const QPoint& pxCursor)
{
	// Preliminary test for valid build location.
	const DaWorldLL* terr;
	AuPoint fBuildLoc;
	DaPerson* person;
	m_pWnd->LockSceneAtom(&person);
	DaBd* exBd;
	int testloc = TestBuildLocation(person, pxCursor, terr, fBuildLoc, exBd);
	AuGlobeLLF buildLL;
	if(!terr || !terr->LLOfLoc(buildLL, fBuildLoc) || testloc != 0)
	{
		if(testloc == 12 && exBd)
		{
			// Blocked by existing development.
			// Calculate the Z rotation needed to align with the blocking development.
			m_AlignRot = exBd->AlignmentRot(*terr, buildLL);

			// Align to the existing development.
			AlignToExistingDevelopment();
		}

		m_pWnd->UnlockSceneAtom();
		m_LastTime.restart();
		//qDebug("DdBdPreview::ConstructBuilding - Build location rejected: TestBuildLocation returned %d", testloc);
		return;
	}
	m_pWnd->UnlockSceneAtom();

	auApp->BdPreview(fBuildLoc, ZRotation());
}

void DdBdPreview::AlignToExistingDevelopment()
{
	//qDebug() << "DdBdPreview::AlignToExistingDevelopment" << m_AlignRot;
	if(m_AlignRot != 99.f)
	{
		m_RotZ = m_AlignRot;

		//if(DnBuilding::IsCoastal(m_eIndustryID))
			//m_RotZ += PIf;

		m_RotAligned = true;
	}
}
