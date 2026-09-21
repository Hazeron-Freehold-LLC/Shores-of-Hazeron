//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanclearglobe.cpp,v 1.16 2022/04/11 20:17:24 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddplanclearglobe.h"
//
#include "auapp.h"
#include "audesignerview.h"
#include "augleffects.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include <qcursor.h>
#include <QMouseEvent>
#include <QPainter>

DdPlanClearGlobe::DdPlanClearGlobe(AuDesignerView* view) : DdCommand(view)
{
	m_ClearCount = 0;
	m_ClearIndex = 0;
}

DdPlanClearGlobe::~DdPlanClearGlobe()
{
}

void DdPlanClearGlobe::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
		ClearLand();
}

void DdPlanClearGlobe::NextIndex()
{
	int oldidx = m_ClearIndex;
	m_ClearIndex++;
	if(m_ClearIndex >= m_ClearCount)
		m_ClearIndex = 0;
	if(oldidx != m_ClearIndex)
	{
		m_BdId = AuId();
		m_DevLL = AuGlobeLLF();
		m_GeoLL = AuGlobeLLF();
	}
}

void DdPlanClearGlobe::PrevIndex()
{
	int oldidx = m_ClearIndex;
	m_ClearIndex--;
	if(m_ClearIndex < 0)
	{
		if(m_ClearCount)
			m_ClearIndex = m_ClearCount - 1;
		else
			m_ClearIndex = 0;
	}
	if(oldidx != m_ClearIndex)
	{
		m_BdId = AuId();
		m_DevLL = AuGlobeLLF();
		m_GeoLL = AuGlobeLLF();
	}
}

void DdPlanClearGlobe::OnMouseWheel(QWheelEvent* e)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif
	if(zDelta > 0)
		NextIndex();
	else
		PrevIndex();
}

bool DdPlanClearGlobe::OnKeyDown(QKeyEvent* e)
{
	if(DdCommand::OnKeyDown(e))
		return true;
	switch(e->key())
	{
	case Qt::Key_Right:	// Skip next
		NextIndex();
		return true;

	case Qt::Key_Left:	// Skip prev
		PrevIndex();
		return true;

	default:
		break;
	}
	return false;
}

// Render "cursor" objects in the scene.
void DdPlanClearGlobe::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson *pSceneAtom,
		const QPoint &pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	Q_UNUSED(glw);
	Q_UNUSED(snappedToExistingShape);
	DaAtom *pEnv = pSceneAtom->FindEnvironmentAtom();
	if (!pEnv)
		return;

	AuPoint fBuildLoc;
	DaBd* onPlatform = 0;
	const DaWorldLL* pTerrain = dynamic_cast<const DaWorldLL*>(m_pWnd->FindTerrainLocationAtPixel(pSceneAtom, pxCursor, fBuildLoc,
				&onPlatform));
	if(pTerrain)
	{
		AuGlobeLLF delLL;
		if(pTerrain->LLOfLoc(delLL, fBuildLoc))
		{
			// Check for Bd buildings.
			DaAtomList bds;
			pTerrain->FindBdsContaining(fBuildLoc, (FINDDEV)(FIND_All|FIND_SkipPlatforms), bds);	// Find bds but not platforms.
			if(bds.empty())
				pTerrain->FindBdsContaining(fBuildLoc, FIND_All, bds);								// No bds so look for platform.
			m_ClearCount = (int)bds.size();
			bool lookingForOldCur = !m_BdId.IsNull() || m_GeoLL != AuGlobeLLF();
			bool setCur = false;

			// Restore index to previous current bd if possible.
			if(!m_BdId.IsNull())
			{
				int idx = 0;
				for(DaAtomList::const_iterator ai = bds.cbegin(); ai != bds.cend(); ai++, idx++)
					if((*ai)->Is(m_BdId))
					{
						m_ClearIndex = idx;
						lookingForOldCur = false;	// We found it.
						break;
					}
			}

			if(!lookingForOldCur && m_ClearIndex < m_ClearCount)
			{
				// We have an index and we're not hoping to find a previous city or geo.
				DaBd* bd = (DaBd*)bds[m_ClearIndex];
				m_BdId = bd->Id();
				m_DevLL = AuGlobeLLF();
				m_GeoLL = AuGlobeLLF();
				setCur = true;
				bool okToClear = false;
				if(bd->IndustryId() == IID_Capitol)
				{
					if(!pTerrain->HasCityPolice(bd->CapitolId()))
						okToClear = true;					// Capitol can be cleared if it has no police.
				}
				else if(bd->IndustryId() == IID_BaseHeadquarters)
				{
					if(!pTerrain->HasAnyMilitaryNotHQ())
						okToClear = true;					// HQ can be cleared if it has no buildings.
				}
				else if(bd->IsPlatform())
				{
					// Platform can be cleared if it does not host any buildings.
					DaAtomList bds;
					pTerrain->FindBdsOnPlatform(bd->Id(), bds);
					if(bds.empty())
						okToClear = true;
				}
				else if(bd->IsHarshEnvironmentEnclosure())
				{
					// Harsh environment enclosure can be cleared if it does not contain any buildings.
					DaAtomList bds;
					pTerrain->FindBdsInsideEnclosure(bd->Id(), bds);
					if(bds.empty())
						okToClear = true;
				}
				else
					okToClear = true;

				// Get transforms needed to calculate the location of the vertices in the scene.
				AuMatrix4x4 tScene;
				pSceneAtom->GetTransformRelativeToParent(tScene, pEnv);
				tScene = tScene.Inverse();
				AuMatrix4x4 tTerr;
				pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
				AuMatrix4x4 model = tModel * tScene * tTerr;

				// Use simple shader so lines will be super bright, even at night.
				re.GLSL().LineShader().Select();
				re.GLSL().LineShader().SetProjectionModelView(projection * model);

				// Draw lines at the cursor location.
				re.glDisable(GL_DEPTH_TEST);
				re.glEnable(GL_BLEND);
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.

				QColor hitColor;
				if(okToClear)
				{
					hitColor.setRedF(.9f);
					hitColor.setGreenF(.35f);
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
				re.GLSL().LineShader().SetColor(hitColor);

				bd->RenderCursorHit_Shader(re, false);	// Building and site lines.

				re.glEnable(GL_DEPTH_TEST);
				re.glDisable(GL_BLEND);
				re.GLSL().LineShader().Deselect();
			}

			// Check for geo roads and craters.
			if(pTerrain->InChainOfCommand(pSceneAtom->Id()))
			{
				const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
				QList<const AuGeologicalFeature*> geos;
				geof.FindDeletablesContaining(*pTerrain, delLL, 0, geos);
				int clearBase = m_ClearCount;	// Number of items that are not geos.
				m_ClearCount += geos.size();

				// Restore index to previous current geo if possible.
				if(lookingForOldCur)
				{
					int geoidx = 0;
					for(QList<const AuGeologicalFeature*>::const_iterator gi = geos.cbegin(); gi != geos.cend(); gi++, geoidx++)
						if(m_GeoLL == (*gi)->LL())
						{
							m_ClearIndex = clearBase + geoidx;	// Found the geo we used to be indexing.
							lookingForOldCur = false;	// We found it.
							break;
						}
				}

				if(!lookingForOldCur && m_ClearIndex >= clearBase && m_ClearIndex < m_ClearCount)
				{
					const AuGeologicalFeature* gf = geos[m_ClearIndex - clearBase];
					if(gf)
					{
						m_BdId = AuId();
						m_DevLL = AuGlobeLLF();
						m_GeoLL = gf->LL();
						setCur = true;
						// Get transforms needed to calculate the location of the vertices in the scene.
						AuMatrix4x4 tScene;
						pSceneAtom->GetTransformRelativeToParent(tScene, pEnv);
						tScene = tScene.Inverse();
						AuMatrix4x4 tTerr;
						pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
						AuMatrix4x4 model = tModel * tScene * tTerr;

						QColor hitColor;
						hitColor.setRedF(.9f);
						hitColor.setGreenF(.35f);
						hitColor.setBlueF(.1f);
						hitColor.setAlphaF(.9f);

						gf->RenderCursor(*pTerrain, projection, model, re, fBuildLoc, AuGlobeLLF(), hitColor, hitColor);
					}
				}
				pTerrain->UnlockGeologicalFeatures();
			}
			if(!setCur)
			{
				m_ClearIndex = 0;
				m_BdId = AuId();
				m_DevLL = AuGlobeLLF();
				m_GeoLL = AuGlobeLLF();
			}
		}
	}
}

void DdPlanClearGlobe::ClearLand() const
{
	if(!m_BdId.IsNull())
		auApp->ClearBd(m_BdId);
	else if(m_GeoLL != AuGlobeLLF())
		auApp->ClearGeo(m_GeoLL);
}

// Set appropriate device cursor. Default sets Cross cursor if
// pSceneAtom != 0, otherwise sets Arrow cursor.
void DdPlanClearGlobe::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*pSceneAtom*/, const QPoint& /*pxCursor*/)
{
	if(m_Cursor.pixmap().isNull())
	{
		QPixmap pix(32,32);
		pix.fill(QColor(0,0,0,0));
		QPainter p(&pix);
		QPixmap pxdozer(":data/bp_ClearLand.png");
		if(!pxdozer.isNull())
		{
			p.setRenderHint(QPainter::SmoothPixmapTransform, true);
			p.translate(32,0);
			p.scale(-1,1);
			p.drawPixmap(0,0, 32,32, pxdozer);

			// Hot spot
			p.setPen(QColor(255,0,128));
			p.resetTransform();
			p.drawPoint(0,30);
			p.drawPoint(0,31);
			p.drawPoint(1,31);
		}
		p.end();
		if(!pix.isNull())
			m_Cursor = QCursor(pix, 0,31);
	}

	if(m_Cursor.pixmap().isNull())
		m_pWnd->setCursor(QCursor(Qt::CrossCursor));
	else
		m_pWnd->setCursor(m_Cursor);
}
