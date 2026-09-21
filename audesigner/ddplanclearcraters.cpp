//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanclearcraters.cpp,v 1.11 2022/04/11 20:17:21 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddplanclearcraters.h"
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

DdPlanClearCraters::DdPlanClearCraters(AuDesignerView* view) : DdCommand(view)
{
	m_ClearCount = 0;
	m_ClearIndex = 0;
}

DdPlanClearCraters::~DdPlanClearCraters()
{
}

void DdPlanClearCraters::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
		ClearLand();
}

void DdPlanClearCraters::NextIndex()
{
	int oldidx = m_ClearIndex;
	m_ClearIndex++;
	if(m_ClearIndex >= m_ClearCount)
		m_ClearIndex = 0;
	if(oldidx != m_ClearIndex)
		m_GeoLL = AuGlobeLLF();
}

void DdPlanClearCraters::PrevIndex()
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
		m_GeoLL = AuGlobeLLF();
}

void DdPlanClearCraters::OnMouseWheel(QWheelEvent* e)
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

bool DdPlanClearCraters::OnKeyDown(QKeyEvent* e)
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
void DdPlanClearCraters::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson *pSceneAtom,
		const QPoint &pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	Q_UNUSED(glw);
	Q_UNUSED(snappedToExistingShape);
	DaAtom *pEnv = pSceneAtom->FindEnvironmentAtom();
	if (!pEnv)
		return;

	AuPoint fBuildLoc;
	const DaWorldLL *pTerrain = dynamic_cast<const DaWorldLL*>(m_pWnd->FindTerrainLocationAtPixel(pSceneAtom, pxCursor, fBuildLoc));
	if(pTerrain)
	{
		AuGlobeLLF delLL;
		if(pTerrain->LLOfLoc(delLL, fBuildLoc))
		{
			// Check for craters.
			m_ClearCount = 0;
			bool lookingForOldCur = m_GeoLL != AuGlobeLLF();
			bool setCur = false;
			if(pTerrain->InChainOfCommand(pSceneAtom->Id()))
			{
				const AuGeologicalFeatures& geof = pTerrain->LockGeologicalFeatures();
				QList<const AuGeologicalFeature*> geos;
				geof.FindDeletableCratersContaining(*pTerrain, delLL, 0, geos);
				m_ClearCount += geos.size();

				// Restore index to previous current geo if possible.
				if(lookingForOldCur)
				{
					int geoidx = 0;
					for(QList<const AuGeologicalFeature*>::const_iterator gi = geos.cbegin(); gi != geos.cend(); gi++, geoidx++)
						if(m_GeoLL == (*gi)->LL())
						{
							m_ClearIndex = geoidx;	// Found the geo we used to be indexing.
							lookingForOldCur = false;	// We found it.
							break;
						}
				}

				// Get transforms needed to calculate the location of the vertices in the scene.
				AuMatrix4x4 tScene;
				pSceneAtom->GetTransformRelativeToParent(tScene, pEnv);
				tScene = tScene.Inverse();
				AuMatrix4x4 tTerr;
				pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
				AuMatrix4x4 model = tModel * tScene * tTerr;

				// Render current crater.
				const AuGeologicalFeature* gfcur = 0;
				if(!lookingForOldCur && m_ClearIndex >= 0 && m_ClearIndex < m_ClearCount)
				{
					gfcur = geos[m_ClearIndex];
					if(gfcur)
					{
						m_GeoLL = gfcur->LL();
						setCur = true;
						QColor hitColor;
						hitColor.setRedF(.9f);
						hitColor.setGreenF(.35f);
						hitColor.setBlueF(.1f);
						hitColor.setAlphaF(1);
						gfcur->RenderCursor(*pTerrain, projection, model, re, fBuildLoc, AuGlobeLLF(), hitColor, hitColor);
					}
				}

				// Render all craters in the area, to make them easy to find.
				QList<const AuGeologicalFeature*> geosNear;
				geof.FindDeletableCratersContaining(*pTerrain, delLL, 500, geosNear);
				for(QList<const AuGeologicalFeature*>::const_iterator gi = geosNear.cbegin(); gi != geosNear.cend(); gi++)
				{
					const AuGeologicalFeature* gf = *gi;
					if(gf == gfcur)
						continue;	// Already rendered that one.
					QColor craterColor(163, 104, 5, 100);						// orange
					gf->RenderCursor(*pTerrain, projection, model, re, fBuildLoc, AuGlobeLLF(), craterColor, craterColor);
				}
				pTerrain->UnlockGeologicalFeatures();
			}
			if(!setCur)
			{
				m_ClearIndex = 0;
				m_GeoLL = AuGlobeLLF();
			}
		}
	}
}

void DdPlanClearCraters::ClearLand() const
{
	auApp->ClearGeo(m_GeoLL);
}

// Set appropriate device cursor. Default sets Cross cursor if
// pSceneAtom != 0, otherwise sets Arrow cursor.
void DdPlanClearCraters::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*pSceneAtom*/, const QPoint& /*pxCursor*/)
{
	if(m_Cursor.pixmap().isNull())
	{
		QPixmap pix(32,32);
		pix.fill(QColor(0,0,0,0));
		QPainter p(&pix);
		QPixmap pxdozer(":data/bp_ClearCraters.png");
		if(!pxdozer.isNull())
		{
			p.setRenderHint(QPainter::SmoothPixmapTransform, true);
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
