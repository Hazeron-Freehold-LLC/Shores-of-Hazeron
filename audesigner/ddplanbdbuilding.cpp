//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanbdbuilding.cpp,v 1.35 2022/04/15 16:39:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddplanbdbuilding.h"
//
#include "aubdstorage.h"
#include "auclientglobal.h"
#include "audesignerframe.h"
#include "augleffects.h"
#include "aunewcityname.h"
#include "audesignerview.h"
#include "audesignerapp.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

DdPlanBdBuilding::DdPlanBdBuilding(INDUSTRYID industryID, const AuBdReport& report, int constructionMaterial, quint16 mpid,
		bool enableWorker, const AuKnownPatents& patents, AuDesignerView* view) : DdCommand(view), m_ORpt(report)
{
	//qDebug() << "DdPlanBdBuilding::DdPlanBdBuilding build" << cp.DesignId() << "for industry" << iIndustryID;
	m_AlignRoadRot = 99.f;
	m_AlignRot = 99.f;
	m_AlignWhat = 0;
	m_BdVAOValid = false;
	m_eIndustryID = industryID;
	m_NorthZ = 0.f;
	m_PlatformAltMSL = 100 * FeetPerMeter();
	m_RotAligned = false;
	m_RotZ = 0.f;
	m_WorkerOk = enableWorker;

	SetMaterial(constructionMaterial);

	m_sReason = "";

	m_iProcessIndex = 0;
	int iIndex = 0;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.constBegin();
			mpi != gd.m_bManufacturingProcessArray.constEnd(); mpi++)
	{
		if((*mpi).m_idi != m_eIndustryID)
			continue;
		if(industryID == IID_Mine && !report.HasShop() && (*mpi).CommodityId() != CID_Stone)
			continue;	// Quarry can only make stone.
		// Do not list patented processes unless the patent is available in the city.
		if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*mpi).m_id, patents))
			continue;
		if((*mpi).m_id == mpid)
			m_iProcessIndex = iIndex;
		m_aProcesses.append(*mpi);
		m_aProcesses[iIndex].m_isDefault = true;	// DdPlanBdBuilding uses this to mark valid procs for mines and wells.
		iIndex++;
	}
	g_Au.UnlockData();

	AuPoint smax = report.SiteMax();
	AuPoint smin = report.SiteMin();
	m_SiteCenter = (smax + smin)/2;
	m_SiteRadius = (smax - smin)/2;

	// Insure we have the design locally, for the cursor. We'll be needing it soon enough if they actually build one.
	auApp->BdDesignNeeded(report.DesignId());
}

DdPlanBdBuilding::~DdPlanBdBuilding()
{
}

bool DdPlanBdBuilding::OnKeyDown(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;

	case Qt::Key_Space:
		if(m_AlignWhat == 0)
		{
			if(m_AlignRot != 99.f)
				AlignToExistingDevelopment();
			else
				AlignToExistingRoad();
			m_AlignWhat = 1;	// Align to road first next time.
		}
		else
		{
			if(m_AlignRoadRot != 99.f)
				AlignToExistingRoad();
			else
				AlignToExistingDevelopment();
			m_AlignWhat = 0;	// Align to building first next time.
		}
		break;

	case Qt::Key_Plus:
		{
		}
		break;
	case Qt::Key_Minus:
		{
		}
		break;
	case Qt::Key_Up:
		{
			if(m_SRpt.HullMaterial() == CID_Wood)
				SetMaterial(CID_Lumber);
			else if(m_SRpt.HullMaterial() == CID_Lumber)
				SetMaterial(CID_Stone);
			else if(m_SRpt.HullMaterial() == CID_Stone)
				SetMaterial(CID_Metal);
			else if(m_SRpt.HullMaterial() == CID_Metal)
				SetMaterial(CID_Magmium);
			else if(m_SRpt.HullMaterial() == CID_Magmium)
				SetMaterial(CID_Vulcium);
			else if(m_SRpt.HullMaterial() == CID_Vulcium)
				SetMaterial(CID_Adamantine);
			else if(m_SRpt.HullMaterial() == CID_Adamantine)
				SetMaterial(CID_Wood);
		}
		break;
	case Qt::Key_Down:
		{
			if(m_SRpt.HullMaterial() == CID_Wood)
				SetMaterial(CID_Adamantine);
			else if(m_SRpt.HullMaterial() == CID_Adamantine)
				SetMaterial(CID_Vulcium);
			else if(m_SRpt.HullMaterial() == CID_Vulcium)
				SetMaterial(CID_Magmium);
			else if(m_SRpt.HullMaterial() == CID_Magmium)
				SetMaterial(CID_Metal);
			else if(m_SRpt.HullMaterial() == CID_Metal)
				SetMaterial(CID_Stone);
			else if(m_SRpt.HullMaterial() == CID_Stone)
				SetMaterial(CID_Lumber);
			else if(m_SRpt.HullMaterial() == CID_Lumber)
				SetMaterial(CID_Wood);
		}
		break;
	case Qt::Key_Left:
		if(e->modifiers() & Qt::ControlModifier)
		{
			if(m_WorkerOk)
			{
				if(m_aProcesses.count() > 1)
				{
					// To prevent infinite looping, the max times we will decrement the process index is the number of array entries.
					for(int maxDec = m_aProcesses.count(); maxDec; maxDec--)
					{
						m_iProcessIndex--;
						if(m_iProcessIndex < 0)
							m_iProcessIndex = m_aProcesses.count()-1;

						// The m_isDefault flag tells us which processes are valid. The flag is only changed for mines and wells so
						// we will skip the processes that are invalid based on the resources present in the current tile.
						if(m_aProcesses[m_iProcessIndex].m_isDefault)
							break;
					}
				}
				else
					m_iProcessIndex = 0;
			}
		}
		else
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
		if(e->modifiers() & Qt::ControlModifier)
		{
			if(m_WorkerOk)
			{
				if(m_aProcesses.count() > 1)
				{
					// To prevent infinite looping, the max times we will increment the process index is the number of array entries.
					for(int maxDec = m_aProcesses.count(); maxDec; maxDec--)
					{
						m_iProcessIndex++;
						if(m_iProcessIndex >= m_aProcesses.count())
							m_iProcessIndex = 0;

						// The m_isDefault flag tells us which processes are valid. The flag is only changed for mines and wells so
						// we will skip the processes that are invalid based on the resources present in the current tile.
						if(m_aProcesses[m_iProcessIndex].m_isDefault)
							break;
					}
				}
				else
					m_iProcessIndex = 0;
			}
		}
		else
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
	case Qt::Key_Asterisk:	// Toggle worker
		{
			m_WorkerOk = !m_WorkerOk;

			// Update build toolbar with current worker enabled state.
			//DHH AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
			//DHH frame->SetWorkerEnabledForConstruction(m_WorkerOk);
		}
		break;
	}
	return true;
}

void DdPlanBdBuilding::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
	{
		// Determine the manufacturing process id for the new construction.
		quint16 mpid = 0;
		if(m_iProcessIndex >= 0 && m_iProcessIndex < m_aProcesses.count())
			mpid = m_aProcesses[m_iProcessIndex].m_id;

		// Start construction of the building.
		ConstructBuilding(e->pos(), mpid);
	}
}

void DdPlanBdBuilding::OnMouseMove(QMouseEvent* e)
{
	Q_UNUSED(e);
	m_TimeCursorMoved.restart();
	m_RocksOnSite.clear();
}

void DdPlanBdBuilding::OnMouseWheel(QWheelEvent* e)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif
	if(m_SRpt.Structure() == BD_AerialPlatform)
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
	else
	{
		if(zDelta > 0)
		{
			if(m_WorkerOk)
			{
				if(m_aProcesses.count() > 1)
				{
					// To prevent infinite looping, the max times we will dec the process index is the number of array entries.
					for(int maxDec = m_aProcesses.count(); maxDec; maxDec--)
					{
						m_iProcessIndex--;
						if(m_iProcessIndex < 0)
							m_iProcessIndex = m_aProcesses.count()-1;

						// The m_isDefault flag tells us which processes are valid. The flag is only changed for mines and wells so
						// we will skip the processes that are invalid based on the resources present in the current tile.
						if(m_aProcesses[m_iProcessIndex].m_isDefault)
							break;
					}
				}
				else
					m_iProcessIndex = 0;
			}
		}
		else if(zDelta < 0)
		{
			if(m_WorkerOk)
			{
				if(m_aProcesses.count() > 1)
				{
					// To prevent infinite looping, the max times we will inc the process index is the number of array entries.
					for(int maxDec = m_aProcesses.count(); maxDec; maxDec--)
					{
						m_iProcessIndex++;
						if(m_iProcessIndex >= m_aProcesses.count())
							m_iProcessIndex = 0;

						// The m_isDefault flag tells us which processes are valid. The flag is only changed for mines and wells so
						// we will skip the processes that are invalid based on the resources present in the current tile.
						if(m_aProcesses[m_iProcessIndex].m_isDefault)
							break;
					}
				}
				else
					m_iProcessIndex = 0;
			}
		}
	}
}

// Create device objects used to render the cursor.
void DdPlanBdBuilding::CreateDeviceObjects(AuGLEffects&)
{
}

// Delete device objects used to render the cursor.
void DdPlanBdBuilding::DeleteDeviceObjects(AuGLEffects&)
{
	if(m_BdVAO.VAO())
		m_BdVAO.DeleteGL();
	if(m_BdCoverageVAO.VAO())
		m_BdCoverageVAO.DeleteGL();
}

// Render "cursor" objects in the scene.
void DdPlanBdBuilding::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& camera, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuAtomRenderInfo& ari, bool snappedToExistingShape)
{
	Q_UNUSED(snappedToExistingShape);
	//qDebug() << "DdPlanBdBuilding::RenderCursor cursor at" << pxCursor;
	DaAtom *pEnv = sceneAtom->FindEnvironmentAtom();
	if(!pEnv)
		return;

	const DaWorldLL* pTerrain;			// set by call to TestBuildLocation
	AuPoint fBuildLoc;				// set by call to TestBuildLocation
	DaBd* exBd;
	m_AlignRot = 99.f;				// Invalid until set by existing development in TestBuildLocation.
	m_AlignRoadRot = 99.f;			// Invalid until set by existing development in TestBuildLocation.
	DaBd* onPlatform;
	QString cityName;
	int testloc = TestBuildLocation(sceneAtom, pxCursor, pTerrain, fBuildLoc, exBd, &onPlatform, cityName);

	// Render lines of building site meshes.
	AuMatrix4x4 mBld;
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
		sceneAtom->GetTransformRelativeToParent(tScene, pEnv);
		tScene = tScene.Inverse();
		AuMatrix4x4 tTerr;
		pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
		AuMatrix4x4 model = camera * tScene * tTerr;

		AuPoint siteCen(m_SRpt.SiteCenter());
		pTerrain->BuildingConstructionTransform(fBuildLoc, ZRotation(), onPlatform, siteCen, mBld);
		AuPoint vUp = -pTerrain->GravityDir(mBld.Project(siteCen));	// Up vector with respect to terrain.

		// Use simple shader so lines will be super bright, even at night.
		ari.re->GLSL().LineShader().Select();
		ari.re->GLSL().LineShader().SetColor(cursorColor);

		// Draw lines at the cursor location.
		ari.re->glDisable(GL_DEPTH_TEST);
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.

		if(m_BdVAO.VAO() && m_BdVAOValid)
		{
			// We have built the VAO of the design so we can reuse it.
			ari.re->GLSL().LineShader().SetProjectionModelView(projection * model * mBld);
			DaBd bd;
			bd.SetIndustry(m_eIndustryID);	// So police will show coverage.
			bd.SetDesign(m_SRpt);
			bd.RenderCursorHit_Shader(m_BdVAO, m_BdCoverageVAO, *ari.re, false, GL_STATIC_DRAW);	// Building and site edge lines.
		}
		else
		{
			if(m_BdVAO.VAO())
				m_BdVAO.DeleteGL();
			if(m_BdCoverageVAO.VAO())
				m_BdCoverageVAO.DeleteGL();

			// Display the building as a cursor, if we have the design.
			const DgData& sdd = g_Au.LockBdDesignsForRead();
			bool hasDesign = sdd.BdDesignIsLoaded(m_SRpt.DesignId());
			g_Au.UnlockBdDesigns();
			if(hasDesign)
			{
				// The design is available so we can have it draw itself.
				ari.re->GLSL().LineShader().SetProjectionModelView(projection * model * mBld);
				DaBd bd;
				bd.SetIndustry(m_eIndustryID);	// So police will show coverage.
				bd.SetDesign(m_SRpt);
				bd.RenderCursorHit_Shader(m_BdVAO, m_BdCoverageVAO, *ari.re, false, GL_STATIC_DRAW);	// Bd and site edge lines.
				m_BdVAOValid = true;
			}
			else
			{
				// The design is not ready. Render cursor lines based on the extents of the model, which the report tells us. The
				// design should be ready soon.
				ari.re->GLSL().LineShader().SetProjectionModelView(projection * model);
				AuPoint bl = mBld.Project(AuPoint(m_SRpt.HullMin().x, m_SRpt.HullMin().y, 0));
				AuPoint ml = mBld.Project(AuPoint(m_SRpt.HullMin().x, 0, 0));
				AuPoint fl = mBld.Project(AuPoint(m_SRpt.HullMin().x, m_SRpt.HullMax().y, 0));
				AuPoint fm = mBld.Project(AuPoint(0, m_SRpt.HullMax().y, 0));
				AuPoint fr = mBld.Project(AuPoint(m_SRpt.HullMax().x, m_SRpt.HullMax().y, 0));
				AuPoint mr = mBld.Project(AuPoint(m_SRpt.HullMax().x, 0, 0));
				AuPoint br = mBld.Project(AuPoint(m_SRpt.HullMax().x, m_SRpt.HullMin().y, 0));
				AuPoint bm = mBld.Project(AuPoint(0, m_SRpt.HullMin().y, 0));

				QList<AuPoint> lvlPts;			// Terrain level perimeter points.
				lvlPts.append(bl);
				lvlPts.append(ml);
				lvlPts.append(fl);
				lvlPts.append(fm);
				lvlPts.append(fr);
				lvlPts.append(fr);
				lvlPts.append(mr);
				lvlPts.append(br);
				lvlPts.append(bm);
				lvlPts.append(bl);

				AuPoint bbl = mBld.Project(AuPoint(m_SRpt.SiteMin().x, m_SRpt.SiteMin().y, 0));
				AuPoint bml = mBld.Project(AuPoint(m_SRpt.SiteMin().x, 0, 0));
				AuPoint bfl = mBld.Project(AuPoint(m_SRpt.SiteMin().x, m_SRpt.SiteMax().y, 0));
				AuPoint bfm = mBld.Project(AuPoint(0, m_SRpt.SiteMax().y, 0));
				AuPoint bfr = mBld.Project(AuPoint(m_SRpt.SiteMax().x, m_SRpt.SiteMax().y, 0));
				AuPoint bmr = mBld.Project(AuPoint(m_SRpt.SiteMax().x, 0, 0));
				AuPoint bbr = mBld.Project(AuPoint(m_SRpt.SiteMax().x, m_SRpt.SiteMin().y, 0));
				AuPoint bbm = mBld.Project(AuPoint(0, m_SRpt.SiteMin().y, 0));

				QList<AuPoint> bldPts;			// Building zone perimeter points.
				AuGlobeLL ll;
				if(pTerrain->LLOfLoc(ll, bbl))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bml))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bfl))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bfm))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bfr))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bmr))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bbr))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(pTerrain->LLOfLoc(ll, bbm))
					bldPts.append(pTerrain->GetTerrainLocation(ll));
				if(bldPts.size() == 8)
					bldPts.append(bldPts.first());
				else
					bldPts.clear();

				// Draw building level perimeter.
				QList<AuPointAlpha> verts;
				AuGLVAO vao;
				if(!bldPts.isEmpty())
				{
					// Build radius is larger than the terrain leveling radius. Draw the build radius using a thin line weight.
					verts.clear();
					for(QList<AuPoint>::const_iterator lpi = bldPts.constBegin(); lpi != bldPts.constEnd(); lpi++)
						verts << AuPointAlpha((*lpi).x, (*lpi).y, (*lpi).z);

					// Make a VAO of the vertex locations.
					vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					if(vao.VAO())
					{
						vao.SelectGL();
						vao.DrawGL(GL_LINE_STRIP);
						vao.DeselectGL();
						vao.DeleteGL();
					}
				}

				// Draw the terrain leveling radius.
				verts.clear();
				for(QList<AuPoint>::const_iterator lpi = lvlPts.constBegin(); lpi != lvlPts.constEnd(); lpi++)
					verts << AuPointAlpha((*lpi).x, (*lpi).y, (*lpi).z);

				// Make a VAO of the vertex locations.
				vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}

				// Draw the rotation/front-facing indicator.
				verts.clear();
				AuPoint frontLoc = mBld.Project(AuPoint(0,FeetPerMeter()*2,0));
				verts << AuPointAlpha(fBuildLoc.x, fBuildLoc.y, fBuildLoc.z);
				verts << AuPointAlpha(frontLoc.x, frontLoc.y, frontLoc.z);

				// Make a VAO of the vertex locations.
				vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);

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

		// Render forward facing indicator.
		ari.re->GLSL().LineShader().SetProjectionModelView(projection * model);
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
			vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);
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
			ari.re->GLSL().LineShader().SetColor(hitColor);
			exBd->RenderCursorHit_Shader(*ari.re, false);	// Building and site edge lines.

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
				AuPoint bdUp = bd->Transform().ProjectVector(bd->GravityNormal());
				if(bdUp.Dot(vUp) <= 0)
					continue;
				switch(bd->IndustryId())
				{
				case IID_Road:
					ari.re->GLSL().LineShader().SetColor(roadColor);
					break;
				case IID_Landscape:
					ari.re->GLSL().LineShader().SetColor(landscapeColor);
					break;
				default:
					ari.re->GLSL().LineShader().SetColor(buildingColor);
					break;
				}
				bd->RenderCursorHit_Shader(*ari.re, true);	// Site edge lines only.
			}
		}

		// Show roads.
		AuGlobeLLF alignRoad;
		// Try aligning to a road.
		AuGlobeLLF alignLL;
		if(pTerrain->LLOfLoc(alignLL, fBuildLoc))
		{
			const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
			m_AlignRoadRot = geof.AlignmentRot(*pTerrain, alignLL, 0, alignRoad);	// Returns 99.f if none is found.
			pTerrain->UnlockGeologicalFeatures();
		}
		const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
		geof.RenderCursor_Shader(*pTerrain, *ari.re, fBuildLoc, alignRoad, alignRoadColor, roadColor);
		pTerrain->UnlockGeologicalFeatures();

		ari.re->glEnable(GL_DEPTH_TEST);
		ari.re->glDisable(GL_BLEND);
		ari.re->GLSL().LineShader().Deselect();
	}

	QPoint cursorPos = m_pWnd->cursor().pos();
	cursorPos = ari.glw->mapFromGlobal(cursorPos);
	if(testloc == 0)
	{
		// Location is good.
		QPoint drawPos = cursorPos+QPoint(0,32);

		QFont font;
		font.setPointSize(10);
		font.setBold(true);

		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// City name with jurisdiction at the proposed site.
		if(!cityName.isEmpty())
		{
			QFont fontCity;
			fontCity.setPointSize(12);
			fontCity.setBold(true);
			QPoint namePos = cursorPos+QPoint(0,-32);
			QColor tcolor;
			tcolor.setRgbF(0,0,0,1);
			ari.re->RenderText(namePos.x(), namePos.y(), cityName, fontCity, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignBottom|Qt::AlignHCenter|Qt::TextWordWrap, 200);
			tcolor.setRgbF(1,1,1,1);
			ari.re->RenderText(namePos.x()-1, namePos.y()-1, cityName, fontCity, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignBottom|Qt::AlignHCenter|Qt::TextWordWrap, 200);
		}

		if(m_WorkerOk && !m_aProcesses.isEmpty())
		{
			// We must calculate the pixel location of the text to draw text on
			// the screen using renderText, rather than at 3D Z-depth.
			QColor tcolor;
			tcolor.setRgbF(0,0,0,1);
			ari.re->RenderText(drawPos.x(), drawPos.y(), m_sProcessName, font, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, 200);
			tcolor.setRgbF(.1f,.9f,.1f,1);
			ari.re->RenderText(drawPos.x()-1, drawPos.y()-1, m_sProcessName, font, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, 200);
			drawPos.setY(drawPos.y() + 16);
		}

		{
			double altMSL = pTerrain->AltitudeMSL(fBuildLoc) / FeetPerMeter();	// in meters
			QString sOffset;
			if(m_SRpt.IsPlatform())
			{
				// Also show space above for buildings. We require a 100m margin between the top of the tallest building and the
				// margin of the atmosphere.
				double distAbove = pTerrain->AtmosphereHeightMSL() / FeetPerMeter() - altMSL - 100;
				sOffset = QObject::tr("%L1m MSL, %L2m On Top").arg(altMSL, 0,'f',1).arg(distAbove, 0,'f',1);
			}
			else
				sOffset = QObject::tr("%L1m MSL").arg(altMSL, 0,'f',1);

			QColor tcolor;
			tcolor.setRgbF(0,0,0,1);
			ari.re->RenderText(drawPos.x(), drawPos.y(), sOffset, font, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextSingleLine, 200);
			tcolor.setRgbF(.1f,.9f,.1f,1);
			ari.re->RenderText(drawPos.x()-1, drawPos.y()-1, sOffset, font, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextSingleLine, 200);
		}
		ari.re->glDisable(GL_BLEND);

		// Mines, wells and drilling rigs show the resources on the site.
		if(m_eIndustryID == IID_Mine || m_eIndustryID == IID_Well || m_eIndustryID == IID_DrillingRig)
		{
			if(m_RocksOnSite.empty() && m_TimeCursorMoved.elapsed() > 250)
			{
				// Survey the site for rocks.
				DaBd bd;
				bd.SetId(AuId(1));
				bd.SetIndustry(m_eIndustryID);	// So police will show coverage.
				bd.SetDesign(m_SRpt);
				bd.SetPosAndOrient(AuAtomPos(AuId(0), mBld));
				bd.SurveySite(*pTerrain, m_RocksOnSite);
				//qDebug() << "DdPlanBdBuilding::RenderCursor site survey" << m_RocksOnSite.size();

				// Update current mining/drilling process.
				if(m_eIndustryID == IID_Mine)
				{
					int procIdx = 0;
					int mineralIdx = -1;
					int defIdx = -1;
					for(DbManufacturingProcessArray::iterator it = m_aProcesses.begin(); it != m_aProcesses.end(); it++, procIdx++)
					{
						if(DbCommodity::IsPatent((*it).CommodityId()))
							(*it).m_isDefault = true;	// Patent processes are always valid
						else
						{
							(*it).m_isDefault = m_RocksOnSite.contains((*it).CommodityId());
							if((*it).m_isDefault)
							{
								if((*it).CommodityId() == CID_Stone)
									defIdx = procIdx;		// Remember Stone process.
								else
									mineralIdx = procIdx;	// Remember non-Stone process.
							}
						}
					}

					if(!m_aProcesses[m_iProcessIndex].m_isDefault || m_aProcesses[m_iProcessIndex].CommodityId() == CID_Stone)
					{
						// Current mine process is not valid for the terrain tile or current process makes stone.
						if(mineralIdx != -1)
							m_iProcessIndex = mineralIdx;
						else if(defIdx != -1)
							m_iProcessIndex = defIdx;
					}
				}
				else if(m_eIndustryID == IID_Well || m_eIndustryID == IID_DrillingRig)
				{
					int procIdx = 0;
					int mineralIdx = -1;
					int defIdx = -1;
					for(DbManufacturingProcessArray::iterator it = m_aProcesses.begin(); it != m_aProcesses.end(); it++, procIdx++)
					{
						if(DbCommodity::IsPatent((*it).CommodityId()))
							(*it).m_isDefault = true;	// Patent processes are always valid
						else
						{
							(*it).m_isDefault = m_RocksOnSite.contains((*it).CommodityId());
							if((*it).m_isDefault)
							{
								if((*it).CommodityId() == CID_Water)
									defIdx = procIdx;
								else
									mineralIdx = procIdx;
							}
						}
					}

					if(!m_aProcesses[m_iProcessIndex].m_isDefault || m_aProcesses[m_iProcessIndex].CommodityId() == CID_Water)
					{
						// Current well process is not valid for the terrain tile or current process makes water.
						if(mineralIdx != -1)
							m_iProcessIndex = mineralIdx;
						else if(defIdx != -1)
							m_iProcessIndex = defIdx;
					}
				}
			}

			// Show survey results.
			if(!m_RocksOnSite.empty())
			{
				QPointF curpos = PixelToViewport(ari.glw, pxCursor);
				float sizeX = .06f;
				float sizeY = sizeX;
				float xpos = curpos.x() * ari.glw->width()/ari.glw->height() + sizeX*2;
				float ypos = curpos.y() - sizeY*2.25f;
				float rowStep = sizeY;
				for(QHash<COMMODITYID, quint32>::const_iterator ri = m_RocksOnSite.cbegin(); ri != m_RocksOnSite.cend(); ri++)
				{
					GLuint tex = ari.CommodityTextureN(ri.key());
					if(!tex)
						continue;
					ari.RenderSprite(xpos, ypos, sizeX, sizeY, tex, 1,1,1,1);
					ari.RenderNumber(*ri, xpos + sizeX/2, ypos, sizeY/2, 0, 0, 1,1,1,1);
					ypos -= rowStep;
				}
			}
		}
	}
	else
	{
		// There is a problem.
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// City name with jurisdiction at the proposed site.
		if(!cityName.isEmpty())
		{
			QFont fontCity;
			fontCity.setPointSize(12);
			fontCity.setBold(true);
			QPoint namePos = cursorPos+QPoint(0,-32);
			QColor tcolor;
			tcolor.setRgbF(0,0,0,1);
			ari.re->RenderText(namePos.x(), namePos.y(), cityName, fontCity, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignBottom|Qt::AlignHCenter|Qt::TextWordWrap, 200);
			tcolor.setRgbF(1,1,1,1);
			ari.re->RenderText(namePos.x()-1, namePos.y()-1, cityName, fontCity, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignBottom|Qt::AlignHCenter|Qt::TextWordWrap, 200);
		}

		int secsTo = m_LastTime.elapsed()/1000;
		if(secsTo <= 4)
		{
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

			QPoint drawPos = cursorPos+QPoint(0,32);
			float fAlpha = 1./(1+msecsTo);

			QColor tcolor;
			tcolor.setRgbF(0,0,0,fAlpha);
			ari.re->RenderText(drawPos.x(), drawPos.y(), m_sReason, font, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, 200);
			tcolor.setRgbF(r,g,b,fAlpha);
			ari.re->RenderText(drawPos.x()-1, drawPos.y()-1, m_sReason, font, tcolor, ari.glw->width(), ari.glw->height(),
					Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, 200);
		}
		ari.re->glDisable(GL_BLEND);
	}
}

// Set appropriate device cursor. Default sets Cross cursor if pSceneAtom != 0, otherwise sets Arrow cursor.
void DdPlanBdBuilding::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* pSceneAtom, const QPoint& pxCursor)
{
	const DaWorldLL* pTerrain;
	AuPoint fBuildLoc;
	DaBd* exBd;
	DaBd* onPlatform;
	QString cityName;
	int testloc = TestBuildLocation(pSceneAtom, pxCursor, pTerrain, fBuildLoc, exBd, &onPlatform, cityName);
	if(testloc == 0)
	{
		// Location is good.
		Q_ASSERT(pTerrain);
		AuGlobeLLF buildLL;
		pTerrain->LLOfLoc(buildLL, fBuildLoc);
		AuPoint siteCen(m_SRpt.SiteCenter());
		AuMatrix4x4 mBld;
		pTerrain->BuildingConstructionTransform(fBuildLoc, ZRotation(), onPlatform, siteCen, mBld);
		m_NorthZ = pTerrain->BuildingConstructionNorthAngle(mBld.Project(siteCen), onPlatform);

		QPixmap pix(64,32);
		pix.fill(QColor(0,0,0,0));
		QPainter p(&pix);

		// Show the building material on the cursor.
		QPixmap pxmat(g_Au.CommodityBitmapName(m_SRpt.HullMaterial()));
		if(!pxmat.isNull())
		{
			p.setRenderHint(QPainter::SmoothPixmapTransform, true);
			p.drawPixmap(0,0, 32,32, pxmat);	// Entire cursor
			if(m_WorkerOk && m_iProcessIndex >= 0 && m_iProcessIndex < m_aProcesses.count())
			{
				// Worker is enabled and process index is within a valid range.
				// Update valid processes for mines and wells and treatment plants based on resources and terrain under the
				// cursor at this time.
				/*
				if(m_eIndustryID == IID_Mine)
				{
					int procIdx = 0;
					int mineralIdx = -1;
					int defIdx = -1;
					for(DbManufacturingProcessArray::iterator it = m_aProcesses.begin(); it != m_aProcesses.end(); it++, procIdx++)
					{
						if(DbCommodity::IsPatent((*it).CommodityId()))
							(*it).m_isDefault = true;	// Patent processes are always valid
						else
						{
							//(*it).m_isDefault = pTerrain->TestRockResource(mBld, mBld.Project(m_SiteCenter), m_SiteRadius,
									//(*it).CommodityId());
							(*it).m_isDefault = m_RocksOnSite.contains((*it).CommodityId());
							if((*it).m_isDefault)
							{
								if((*it).CommodityId() == CID_Stone)
									defIdx = procIdx;		// Remember Stone process.
								else
									mineralIdx = procIdx;	// Remember non-Stone process.
							}
						}
					}

					if(!m_aProcesses[m_iProcessIndex].m_isDefault ||
							(m_aProcesses[m_iProcessIndex].CommodityId() == CID_Stone && buildLL != m_LastBuildLL))
					{
						// Current mine process is not valid for the terrain tile or
						// current process makes stone and the tile has changed.
						qDebug() << "SetCursor" << mineralIdx << defIdx;
						if(mineralIdx != -1)
							m_iProcessIndex = mineralIdx;
						else if(defIdx != -1)
							m_iProcessIndex = defIdx;
					}
					m_LastBuildLL = buildLL;
				}
				else if(m_eIndustryID == IID_Well || m_eIndustryID == IID_DrillingRig)
				{
					int procIdx = 0;
					int mineralIdx = -1;
					int defIdx = -1;
					for(DbManufacturingProcessArray::iterator it = m_aProcesses.begin(); it != m_aProcesses.end(); it++, procIdx++)
					{
						if(DbCommodity::IsPatent((*it).CommodityId()))
							(*it).m_isDefault = true;	// Patent processes are always valid
						else
						{
							/+
							COMMODITYID testCID = (*it).CommodityId();
							if(testCID == CID_NaturalGas)
								testCID = CID_CrudeOil;  // Natural gas is found where oil is present.
							// Water can be drilled anywhere but environmental factors affect the success of that endeavor.
							(*it).m_isDefault = testCID == CID_Water || pTerrain->TestRockResource(mBld, mBld.Project(m_SiteCenter),
									m_SiteRadius, testCID);
									+/
							(*it).m_isDefault = m_RocksOnSite.contains((*it).CommodityId());
							if((*it).m_isDefault)
							{
								if((*it).CommodityId() == CID_Water)
									defIdx = procIdx;
								else
									mineralIdx = procIdx;
							}
						}
					}

					if(!m_aProcesses[m_iProcessIndex].m_isDefault ||
							(m_aProcesses[m_iProcessIndex].CommodityId() == CID_Water && buildLL != m_LastBuildLL))
					{
						// Current well process is not valid for the terrain tile or
						// current process makes water and the tile has changed.
						if(mineralIdx != -1)
							m_iProcessIndex = mineralIdx;
						else if(defIdx != -1)
							m_iProcessIndex = defIdx;
					}

					m_LastBuildLL = buildLL;
				}
				else*/ if(m_eIndustryID == IID_TreatmentPlant)
				{
					//qDebug() << "DdPlanBdBuilding:: IID_TreatmentPlant" << waterCoverage
						//<< "orbitZone" << pTerrain->OrbitZone();
					bool hasLiquid = pTerrain->TestForLiquid(mBld, mBld.Project(m_SiteCenter), m_SiteRadius);
					int defIdx = -1;
					if(!hasLiquid)
					{
						// Melting ice is the only non-patent process available to treatment plants entirely on land.
						int procIdx = 0;
						for(DbManufacturingProcessArray::iterator it = m_aProcesses.begin();
								it != m_aProcesses.end(); it++, procIdx++)
						{
							if(DbCommodity::IsPatent((*it).CommodityId()))
								(*it).m_isDefault = true;	// Patent processes are always valid
							else
							{
								(*it).m_isDefault = ((*it).m_id == MPID_WaterUsingIce) ? true : false;
								if((*it).m_isDefault)
									defIdx = procIdx;
							}
						}
					}
					else
					{
						// Treatment plant in water processes.
						int procIdx = 0;
						for(DbManufacturingProcessArray::iterator it = m_aProcesses.begin();
								it != m_aProcesses.end(); it++, procIdx++)
						{
							if(DbCommodity::IsPatent((*it).CommodityId()))
								(*it).m_isDefault = true;	// Patent processes are always valid
							else if((*it).m_id == MPID_WaterUsingIce)
								(*it).m_isDefault = true;	// Water from Ice process is valid but it's not the default.
							else if(pTerrain->IsGasGiant())
							{
								(*it).m_isDefault = ((*it).m_id == MPID_Myrathane || (*it).m_id == MPID_Flomentum) ? true : false;
								if((*it).m_isDefault)
									defIdx = procIdx;
							}
							else
							{
								// Orbit zone determines the type of natural treatment process available.
								if(pTerrain->OrbitZone() == OZONE_Inferno)
									(*it).m_isDefault = ((*it).m_id == MPID_Magmex) ? true : false;
								else if(pTerrain->OrbitZone() == OZONE_Frigid)
									(*it).m_isDefault = ((*it).m_id == MPID_Myrathane) ? true : false;
								else
									(*it).m_isDefault = (*it).m_id == MPID_WaterUsingEnvironment ? true : false;
								if((*it).m_isDefault)
									defIdx = procIdx;
							}
						}
					}

					if(!m_aProcesses[m_iProcessIndex].m_isDefault ||
							(m_aProcesses[m_iProcessIndex].m_id == MPID_WaterUsingIce && buildLL != m_LastBuildLL))
					{
						// Current treatment process is not valid for the terrain tile or
						// current process makes water from ice and the tile has changed.
						if(defIdx != -1)
							m_iProcessIndex = defIdx;
					}

					m_LastBuildLL = buildLL;
				}

				if(m_iProcessIndex >= 0 && m_iProcessIndex < m_aProcesses.count())
				{
					m_sProcessName = m_aProcesses[m_iProcessIndex].m_name;
					QPixmap pxproc(g_Au.CommodityBitmapName(m_aProcesses[m_iProcessIndex].CommodityId()));
					if(!pxproc.isNull())
					{
						p.setRenderHint(QPainter::SmoothPixmapTransform, true);
						p.drawPixmap(32,0,32,32, pxproc);	// Entire cursor
					}

					// Update build toolbar with current manufacturing process ID. It changes for mines and wells as the cursor
					// moves so we'll do it for everybody here. The build toolbar won't do anything if this is not a change.
					//DHH AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
					//DHH frame->SetManufacturingProcessForConstruction(m_aProcesses[m_iProcessIndex].m_id);
				}
			}
		}

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
int DdPlanBdBuilding::TestBuildLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
		DaBd*& exBd, DaBd** onPlatform, QString& cityName) const
{
	exBd = 0;

	// Where is the cursor on the terrain?
	*onPlatform = 0;
	pTerrain = dynamic_cast<const DaWorldLL*>(m_pWnd->FindTerrainLocationAtPixel(sceneAtom, pxCursor, fBuildLoc, onPlatform));
	if(!pTerrain)
		return 1;
	if(*onPlatform)
	{
		// Raise up a little on platforms so floor polygons at Z=0 in the building do not collide with the platform deck polygons.
		AuPoint platLoc = (*onPlatform)->Transform().UnProject(fBuildLoc);	// into the space of the platform
		platLoc.z += .1;	// 
		fBuildLoc = (*onPlatform)->Transform().Project(platLoc);
	}
	switch(m_SRpt.Structure())
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
	return pTerrain->TestBdLocation(m_eIndustryID, sceneAtom->EmpireId(), &m_SRpt, m_SRpt.HullMaterial(), fBuildLoc, 0,
			ZRotation(), *onPlatform, exBd, 0, cityName);
}

void DdPlanBdBuilding::ConstructBuilding(const QPoint& pxCursor, quint16 mpid)
{
	// Preliminary test for valid build location.
	const DaWorldLL* terr;
	AuPoint fBuildLoc;
	DaPerson* person;
	m_pWnd->LockSceneAtom(&person);
	DaBd* exBd;
	DaBd* onPlatform;
	QString cityName;
	int testloc = TestBuildLocation(person, pxCursor, terr, fBuildLoc, exBd, &onPlatform, cityName);
	AuId platformId;
	if(onPlatform)
		platformId = onPlatform->Id();
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
		//qDebug("DdPlanBdBuilding::ConstructBuilding - Build location rejected: TestBuildLocation returned %d", testloc);
		return;
	}
	m_pWnd->UnlockSceneAtom();

	if(m_eIndustryID == IID_Capitol)
	{
		// Get a name for the new city.
		AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
		AuNewCityName dlgCityName(CITY_Capitol, frame);
		int result = frame->ExecDialog(dlgCityName);
		if(result != QDialog::Accepted)
		{
			// User canceled.
			Terminate();
			return;
		}
		QString cityName = dlgCityName.CityName();
		QString systemName = dlgCityName.SolarSystemName();
		QString sectorName = dlgCityName.SectorName();

		// Found the city by making the capitol building.
		auApp->BdFoundCity(m_eIndustryID, m_SRpt.DesignId(), m_SRpt.HullMaterial(), fBuildLoc, ZRotation(), platformId, mpid,
				m_WorkerOk, cityName, systemName, sectorName);
	}
	else if(m_eIndustryID == IID_BaseHeadquarters)
	{
		// Get a name for the new base.
		AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
		AuNewCityName dlgCityName(CITY_MilitaryBase, frame);
		int result = frame->ExecDialog(dlgCityName);
		if(result != QDialog::Accepted)
		{
			// User canceled.
			Terminate();
			return;
		}
		QString cityName = dlgCityName.CityName();
		QString systemName = dlgCityName.SolarSystemName();
		QString sectorName = dlgCityName.SectorName();

		// Found the city by making the capitol building.
		auApp->BdFoundCity(m_eIndustryID, m_SRpt.DesignId(), m_SRpt.HullMaterial(), fBuildLoc, ZRotation(), platformId, mpid,
				m_WorkerOk, cityName, systemName, sectorName);
	}
	else if(m_eIndustryID == IID_Warehouse)
	{
		// Get the commodity to store in the warehouse.
		AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
		AuBdStorage dlg(m_eIndustryID, frame);
		int result = frame->ExecDialog(dlg);
		if(result != QDialog::Accepted)
		{
			// User canceled.
			Terminate();
			return;
		}

		//qDebug() << "DdPlanBdBuilding::ConstructBuilding mpid" << mpid << "workerOk" << m_WorkerOk;
		auApp->BdBuildStorage(m_eIndustryID, m_SRpt.DesignId(), m_SRpt.HullMaterial(), fBuildLoc, ZRotation(), platformId, mpid,
				m_WorkerOk, dlg.StoreCommodity());
	}
	else if(m_eIndustryID == IID_StorageTank)
	{
		// Get the commodity to store in the storage tank.
		AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
		AuBdStorage dlg(m_eIndustryID, frame);
		int result = frame->ExecDialog(dlg);
		if(result != QDialog::Accepted)
		{
			// User canceled.
			Terminate();
			return;
		}

		//qDebug() << "DdPlanBdBuilding::ConstructBuilding mpid" << mpid << "workerOk" << m_WorkerOk;
		auApp->BdBuildStorage(m_eIndustryID, m_SRpt.DesignId(), m_SRpt.HullMaterial(), fBuildLoc, ZRotation(), platformId, mpid,
				m_WorkerOk, dlg.StoreCommodity());
	}
	else
	{
		//qDebug() << "DdPlanBdBuilding::ConstructBuilding mpid" << mpid << "workerOk" << m_WorkerOk;
		auApp->BdBuild(m_eIndustryID, m_SRpt.DesignId(), m_SRpt.HullMaterial(), fBuildLoc, ZRotation(), platformId, mpid,
				m_WorkerOk);
	}
}

void DdPlanBdBuilding::AlignToExistingDevelopment()
{
	//qDebug() << "DdPlanBdBuilding::AlignToExistingDevelopment" << m_AlignRot;
	if(m_AlignRot != 99.f)
	{
		m_RotZ = m_AlignRot;
		if(DbIndustry::IsCoastal(m_eIndustryID))
			m_RotZ += PIf;
		m_RotAligned = true;
	}
}

void DdPlanBdBuilding::AlignToExistingRoad()
{
	//qDebug() << "DdPlanBdBuilding::AlignToExistingRoad" << m_AlignRot;
	if(m_AlignRoadRot != 99.f)
	{
		m_RotZ = m_AlignRoadRot;
		m_RotAligned = true;
	}
}

void DdPlanBdBuilding::SetConstructionProcess(const AuBdReport& report)
{
	m_ORpt = report;
	SetMaterial(m_SRpt.HullMaterial());
	m_BdVAOValid = false;

	// Insure we have the design locally, for the cursor. We'll be needing it soon enough if they actually build one.
	auApp->BdDesignNeeded(report.DesignId());
}

void DdPlanBdBuilding::SetMaterial(int constructionMaterial)
{
	m_SRpt = m_ORpt;
	m_SRpt.SetConstructionMaterial((COMMODITYID)constructionMaterial);
}
