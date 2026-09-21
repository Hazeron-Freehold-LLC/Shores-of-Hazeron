//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplangeolandscape.cpp,v 1.15 2022/04/15 16:39:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddplangeolandscape.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "augleffects.h"
#include "audesignerview.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>

DdPlanGeoLandscape::DdPlanGeoLandscape(int type, AuDesignerView* view) : DdCommand(view)
{
	m_CtrlDown = false;
	m_CursorCtrlDown = false;
	m_LandscapeType = type;
	m_ShiftDown = false;
	m_StartEstablished = false;
}

DdPlanGeoLandscape::~DdPlanGeoLandscape()
{
}

bool DdPlanGeoLandscape::OnKeyUp(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;
	case Qt::Key_Control:
		m_CtrlDown = false;
		break;
	case Qt::Key_Shift:
		m_ShiftDown = false;
		break;
	}
	return true;
}

bool DdPlanGeoLandscape::OnKeyDown(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;
	case Qt::Key_Left:
	case Qt::Key_Right:
		if(!m_StartEstablished)
		{
			int roadType = g_AuClient.RoadType();
			if(roadType)
				g_AuClient.SetRoadType(0);	// road
			else
				g_AuClient.SetRoadType(1);	// slab
			m_Cursor = QCursor();
		}
		break;
	case Qt::Key_Space:
		if(!m_StartLoc.IsZero())
			m_StartEstablished = !m_StartEstablished;
		break;
	case Qt::Key_Control:
		m_CtrlDown = true;
		break;
	case Qt::Key_Shift:
		m_ShiftDown = true;
		break;
	}
	return true;
}

void DdPlanGeoLandscape::OnMouseWheel(QWheelEvent* /*e*/)
{
	if(!m_StartEstablished)
	{
		int roadType = g_AuClient.RoadType();
		if(roadType)
			g_AuClient.SetRoadType(0);	// road
		else
			g_AuClient.SetRoadType(1);	// slab
		m_Cursor = QCursor();
	}
}

void DdPlanGeoLandscape::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
		ConstructRoad(e->pos());
}

// Render "cursor" objects in the scene.
void DdPlanGeoLandscape::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson *pSceneAtom,
		const QPoint &pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	Q_UNUSED(snappedToExistingShape);
	DaAtom *pEnv = pSceneAtom->FindEnvironmentAtom();
	if(!pEnv)
		return;

	const DaWorldLL* pTerrain;			// set by call to TestRoadLocation
	AuPoint fBuildLoc;				// set by call to TestRoadLocation
	DaBd* exBd;
	QString cityName;
	int testloc = TestRoadLocation(pSceneAtom, pxCursor, pTerrain, fBuildLoc, exBd, cityName);
	if(testloc != 1)
	{
		// Create a cursor to represent the construction area.
		double buildAlt = pTerrain->AltitudeMSL(fBuildLoc);
		AuPoint vUp = -pTerrain->GravityDir(fBuildLoc);			// Up vector with respect to gravity.
		double roadWidth2;
		if(g_AuClient.RoadType() == 1)
			roadWidth2 = g_AuClient.SlabWidthInFeet() / 2.;
		else
			roadWidth2 = g_AuClient.RoadWidthInFeet() / 2.;

		// Get transforms needed to calculate the location of the vertices in the scene.
		AuMatrix4x4 tScene;
		pSceneAtom->GetTransformRelativeToParent(tScene, pEnv);
		tScene = tScene.Inverse();
		AuMatrix4x4 tTerr;
		pTerrain->GetTransformRelativeToParent(tTerr, pEnv);
		AuMatrix4x4 t = tModel * tScene * tTerr;

		if(!m_StartEstablished || m_StartLoc.FuzzyEquals(fBuildLoc))
		{
			// Draw a circular cursor at the potential start location.
			double radTZ = roadWidth2 + 10*FeetPerMeter();		// Radius of transition zone perimeter.
			double radTZInt[3];
			radTZInt[0] = roadWidth2*.75 + radTZ*.25;			// Radius 1/4 through terrain transition zone for building.
			radTZInt[1] = roadWidth2*.5 + radTZ*.5;				// Radius 1/2 through terrain transition zone for building.
			radTZInt[2] = roadWidth2*.25 + radTZ*.75;			// Radius 3/4 through terrain transition zone for building.
			AuPoint vX;
			vUp.ArbitraryAxis(vX);									// Returns a unit vector.
			AuPoint vY = vUp.Cross(vX);								// Result will be a unit vector.
			AuPoint vXL = vX * roadWidth2;
			AuPoint vYL = vY * roadWidth2;
			AuPoint vXTZ = vX * radTZ;
			AuPoint vYTZ = vY * radTZ;
			AuPoint vXTZInt[3];
			AuPoint vYTZInt[3];
			for(int tzi=0; tzi<3; tzi++)
			{
				vXTZInt[tzi] = vX * radTZInt[tzi];
				vYTZInt[tzi] = vY * radTZInt[tzi];
			}
			QList<AuPoint> lvlPts;			// Terrain level perimeter points.
			QList<AuPoint> gndPts;			// Ground level perimeter points, at outside edge of transition zone.
			QList<AuPoint> intPts[3];		// Intermediate perimeters, between building level perimeter and ground level perimeter.
			for(int ipt=0; ipt<32; ipt++)
			{
				float ang = (ipt*PIf*2)/32;
				float ca = cosf(ang);
				float sa = sinf(ang);

				// Calculate terrain level perimeter point.
				AuPoint lvlPt = fBuildLoc + vXL*ca + vYL*sa;
				lvlPts.append(lvlPt);

				// Calculate ground level perimeter point at outside edge of transition zone.
				AuPoint gndPt = fBuildLoc + vXTZ*ca + vYTZ*sa;
				AuGlobeLL gndLL;
				if(!pTerrain->LLOfLoc(gndLL, gndPt))
					continue;
				gndPt = pTerrain->GetTerrainLocation(gndLL);
				gndPts.append(gndPt);

				// Calculate intermediate blended points.
				for(int tzi=0; tzi<3; tzi++)
				{
					// Calculate blended point that is between building level altitude and actual terrain level altitude.
					AuPoint intPt = fBuildLoc + vXTZInt[tzi]*ca + vYTZInt[tzi]*sa;
					AuGlobeLL intLL;
					if(!pTerrain->LLOfLoc(intLL, intPt))
						continue;
					double terAlt = pTerrain->TerrainAltitudeMSL(intLL);

					// Calculate blended altitude.
					double buildFract = (3.-tzi)/4.;
					double terFract = (tzi+1.)/4.;
					double intAlt = buildAlt*buildFract + terAlt*terFract;

					intPt = pTerrain->LocOfLL(intLL, intAlt);
					intPts[tzi].append(intPt);
				}
			}
			lvlPts.append(lvlPts.first());

			// Cross slabs.
			if(g_AuClient.RoadType() == 1)
				lvlPts.append(lvlPts[lvlPts.count()/2]);

			gndPts.append(gndPts.first());
			for(int tzi=0; tzi<3; tzi++)
				intPts[tzi].append(intPts[tzi].first());

			// Use simple shader so lines will be super bright, even at night.
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(projection * t);

			// Draw lines at the cursor location.
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_BLEND);

			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if(testloc == 0)
				re.GLSL().LineShader().SetColor(.1f, .1f, .1f, .7f);
			else
				re.GLSL().LineShader().SetColor(.5f, .1f, .1f, .7f);

			// Compile a list of vertex locations.
			QList<AuPointAlpha> verts;
			QList<AuPoint>::const_iterator gpi = lvlPts.constBegin();
			QList<AuPoint>::const_iterator ipi = intPts[0].constBegin();
			for(; gpi != lvlPts.constEnd() && ipi != intPts[0].constEnd(); gpi++, ipi++)
			{
				verts << AuPointAlpha((*gpi).x, (*gpi).y, (*gpi).z);
				verts << AuPointAlpha((*ipi).x, (*ipi).y, (*ipi).z);
			}

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

			// Draw intermediate perimeters to show terrain transition.
			for(int tzi=0; tzi<3; tzi++)
			{
				// Draw blended level perimeter, which is the midpoint of the transition zone.
				verts.clear();
				for(QList<AuPoint>::const_iterator ipi = intPts[tzi].constBegin(); ipi != intPts[tzi].constEnd(); ipi++)
					verts << AuPointAlpha((*ipi).x, (*ipi).y, (*ipi).z);

				// Make a VAO of the vertex locations.
				vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}

				if(tzi == 2)
					break;

				// Draw legs between ground level perimeter and first intermediate level perimeter.
				verts.clear();
				QList<AuPoint>::const_iterator gpi = intPts[tzi].constBegin();
				QList<AuPoint>::const_iterator ipi = intPts[tzi+1].constBegin();
				for(; gpi != intPts[tzi].constEnd() && ipi != intPts[tzi+1].constEnd(); gpi++, ipi++)
				{
					verts << AuPointAlpha((*gpi).x, (*gpi).y, (*gpi).z);
					verts << AuPointAlpha((*ipi).x, (*ipi).y, (*ipi).z);
				}

				// Make a VAO of the vertex locations.
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

			// Draw legs between last blended intermediate level perimeter and level perimeter.
			verts.clear();
			ipi = intPts[2].constBegin();
			QList<AuPoint>::const_iterator lpi = gndPts.constBegin();
			for(; ipi != intPts[2].constEnd() && lpi != gndPts.constEnd(); ipi++, lpi++)
			{
				verts << AuPointAlpha((*ipi).x, (*ipi).y, (*ipi).z);
				verts << AuPointAlpha((*lpi).x, (*lpi).y, (*lpi).z);
			}

			// Make a VAO of the vertex locations.
			vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// Draw building level perimeter.
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.
			if(testloc == 0)
				re.GLSL().LineShader().SetColor(.1f, .9f, .1f, .9f);
			else
				re.GLSL().LineShader().SetColor(.9f, .1f, .1f, .9f);

			// Draw the terrain leveling radius using a thick line weight.
			verts.clear();
			for(QList<AuPoint>::const_iterator lpi = lvlPts.constBegin(); lpi != lvlPts.constEnd(); lpi++)
				verts << AuPointAlpha((*lpi).x, (*lpi).y, (*lpi).z);

			// Make a VAO of the vertex locations.
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

			// Build road edge lines and center lines.
			// Start with the starting locations.
			QList<AuPoint> roadCenter;
			QList<AuPoint> roadLeft;
			QList<AuPoint> roadRight;
			QList<AuPoint> roadLegs;	// Lines across the road to make a grid.
			QList<AuPoint> groundCenter;
			QList<AuPoint> groundLeft;
			QList<AuPoint> groundRight;
			QList<AuPoint> intPtsLeft[3];
			QList<AuPoint> intPtsRight[3];
			QList<AuPoint> intLegsLeft;
			QList<AuPoint> intLegsRight;

			// Integrate along the length so we conform to the curvature of the world.
			double seglen = 10*FeetPerMeter();
			int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
			bool atEnd = false;
			for(int seg=0; seg<=segs && !atEnd; seg++)
			{
				double lenAlongSeg = seglen * seg;
				if(lenAlongSeg >= centerLen)
				{
					// We are at the end. Truncate the last segment for perfect ending.
					//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
					lenAlongSeg = centerLen;				
					atEnd = true;
				}
				double segRatio = lenAlongSeg / centerLen;
				double altAtSegLoc = startAlt + (buildAlt - startAlt) * segRatio;	// Linear alt change from one end to the other.
				AuPoint segLoc = m_StartLoc + centerVec * lenAlongSeg;
				AuPoint segUp = -pTerrain->GravityDir(segLoc);
				pTerrain->ElevateLocMSL(segLoc, altAtSegLoc);

				AuPoint segRight = centerVec.Cross(segUp);
				segRight.Unit();

				// Add road level points.
				roadCenter.append(segLoc);
				roadLeft.append(segLoc - segRight*roadWidth2);
				roadRight.append(segLoc + segRight*roadWidth2);

				// Add pairs to draw them as GL_LINES.
				roadLegs.append(roadLeft.last());
				roadLegs.append(roadCenter.last());
				roadLegs.append(roadCenter.last());
				roadLegs.append(roadRight.last());

				// Add ground level points.
				AuGlobeLL gndLL;
				if(!pTerrain->LLOfLoc(gndLL, roadCenter.last()))
					continue;
				AuPoint gndPt = pTerrain->GetTerrainLocation(gndLL);
				groundCenter.append(gndPt);

				if(!pTerrain->LLOfLoc(gndLL, roadLeft.last()))
					continue;
				gndPt = pTerrain->GetTerrainLocation(gndLL);
				groundLeft.append(gndPt);

				if(!pTerrain->LLOfLoc(gndLL, roadRight.last()))
					continue;
				gndPt = pTerrain->GetTerrainLocation(gndLL);
				groundRight.append(gndPt);

				// Calculate intermediate blended points.
				intLegsLeft.append(roadLeft.last());
				intLegsRight.append(roadRight.last());
				for(int tzi=0; tzi<3; tzi++)
				{
					// Calculate blended point that is between building level altitude and actual terrain level altitude.
					double buildFract = (3.-tzi)/4.;
					double terFract = (tzi+1.)/4.;
					double intDist = (10*FeetPerMeter()) * terFract;

					// Left
					{
						AuPoint intPt = roadLeft.last() - segRight*intDist;
						AuGlobeLL intLL;
						if(!pTerrain->LLOfLoc(intLL, intPt))
							continue;
						intPt = pTerrain->GetTerrainLocation(intLL);
						double terAlt = pTerrain->AltitudeMSL(intPt);

						// Calculate blended altitude.
						double intAlt = pTerrain->AltitudeMSL(roadLeft.last())*buildFract + terAlt*terFract;

						pTerrain->ElevateLocMSL(intPt, intAlt);
						intPtsLeft[tzi].append(intPt);

						// Leg point. We add two because we draw the entire array as GL_LINES.
						// We are ending one seg and starting the next.
						intLegsLeft.append(intPt);
						intLegsLeft.append(intPt);
					}

					// Right
					{
						AuPoint intPt = roadRight.last() + segRight*intDist;
						AuGlobeLL intLL;
						if(!pTerrain->LLOfLoc(intLL, intPt))
							continue;
						intPt = pTerrain->GetTerrainLocation(intLL);
						double terAlt = pTerrain->AltitudeMSL(intPt);

						// Calculate blended altitude.
						double intAlt = pTerrain->AltitudeMSL(roadRight.last())*buildFract + terAlt*terFract;

						pTerrain->ElevateLocMSL(intPt, intAlt);
						intPtsRight[tzi].append(intPt);

						// Leg point. We add two because we draw the entire array as GL_LINES.
						// We are ending one seg and starting the next.
						intLegsRight.append(intPt);
						intLegsRight.append(intPt);
					}
				}

				// Calculate final leg points.
				// Left
				{
					AuPoint intPt = roadLeft.last() - segRight*(10*FeetPerMeter());
					AuGlobeLL intLL;
					if(!pTerrain->LLOfLoc(intLL, intPt))
						continue;
					intPt = pTerrain->GetTerrainLocation(intLL);
					intLegsLeft.append(intPt);
				}

				// Right
				{
					AuPoint intPt = roadRight.last() + segRight*(10*FeetPerMeter());
					AuGlobeLL intLL;
					if(!pTerrain->LLOfLoc(intLL, intPt))
						continue;
					intPt = pTerrain->GetTerrainLocation(intLL);
					intLegsRight.append(intPt);
				}
			}

			// Return value tells us if the center or either edge deviates from the terrain too much.
			bool centerOk, leftOk, rightOk;
			/*
			if(testloc >= 21 && testloc <= 27)
			{
				int badBits = testloc - 20;
				centerOk = (badBits&0x02) ? false : true;
				leftOk = (badBits&0x04) ? false : true;
				rightOk = (badBits&0x01) ? false : true;
			}
			else*/ if(testloc != 0)
			{
				centerOk = false;
				leftOk = false;
				rightOk = false;
			}
			else
			{
				centerOk = true;
				leftOk = true;
				rightOk = true;
			}

			// Use simple shader so lines will be super bright, even at night.
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(projection * t);

			// Draw lines at the cursor location.
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_BLEND);

			// Draw lines.
			// Left
			{
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.
				if(leftOk)
					re.GLSL().LineShader().SetColor(.1f, .9f, .1f, .9f);
				else
					re.GLSL().LineShader().SetColor(.9f, .1f, .1f, .9f);

				// Compile a list of vertex locations.
				QList<AuPointAlpha> verts;
				for(QList<AuPoint>::const_iterator ri = roadLeft.constBegin(); ri != roadLeft.constEnd(); ri++)
					verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

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

				verts.clear();
				for(QList<AuPoint>::const_iterator ri = groundLeft.constBegin(); ri != groundLeft.constEnd(); ri++)
					verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

				// Make a VAO of the vertex locations.
				vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}

				// Draw transition zone
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//if(leftOk)
					re.GLSL().LineShader().SetColor(.1f, .1f, .1f, .7f);
				//else
					//re.GLSL().LineShader().SetColor(.5f, .1f, .1f, .7f);
				for(int tzi=0; tzi<3; tzi++)
				{
					verts.clear();
					for(QList<AuPoint>::const_iterator ri = intPtsLeft[tzi].constBegin(); ri != intPtsLeft[tzi].constEnd(); ri++)
						verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

					// Make a VAO of the vertex locations.
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

				// Transition zone legs.
				verts.clear();
				for(QList<AuPoint>::const_iterator ri = intLegsLeft.constBegin(); ri != intLegsLeft.constEnd(); ri++)
					verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

				// Make a VAO of the vertex locations.
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

			// Center
			{
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.
				if(centerOk)
					re.GLSL().LineShader().SetColor(.1f, .9f, .1f, .9f);
				else
					re.GLSL().LineShader().SetColor(.9f, .1f, .1f, .9f);

				if(g_AuClient.RoadType() == 1)
				{
					// X on slabs.
					// Compile a list of vertex locations.
					QList<AuPointAlpha> verts;
					verts << AuPointAlpha(roadRight.first().x, roadRight.first().y, roadRight.first().z);
					verts << AuPointAlpha(roadLeft.last().x, roadLeft.last().y, roadLeft.last().z);
					verts << AuPointAlpha(roadRight.last().x, roadRight.last().y, roadRight.last().z);
					verts << AuPointAlpha(roadLeft.first().x, roadLeft.first().y, roadLeft.first().z);
					verts << AuPointAlpha(roadRight.first().x, roadRight.first().y, roadRight.first().z);

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
				else
				{
					// Grid on road strips.
					// Compile a list of vertex locations.
					QList<AuPointAlpha> verts;
					for(QList<AuPoint>::const_iterator ri = roadCenter.constBegin(); ri != roadCenter.constEnd(); ri++)
						verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

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

					verts.clear();
					for(QList<AuPoint>::const_iterator ri = roadLegs.constBegin(); ri != roadLegs.constEnd(); ri++)
						verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

					// Make a VAO of the vertex locations.
					vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					if(vao.VAO())
					{
						vao.SelectGL();
						vao.DrawGL(GL_LINES);
						vao.DeselectGL();
						vao.DeleteGL();
					}

					verts.clear();
					for(QList<AuPoint>::const_iterator ri = groundCenter.constBegin(); ri != groundCenter.constEnd(); ri++)
						verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

					// Make a VAO of the vertex locations.
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

			// Right
			{
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend keeps it nice and bright.
				if(rightOk)
					re.GLSL().LineShader().SetColor(.1f, .9f, .1f, .9f);
				else
					re.GLSL().LineShader().SetColor(.9f, .1f, .1f, .9f);

				// Compile a list of vertex locations.
				QList<AuPointAlpha> verts;
				for(QList<AuPoint>::const_iterator ri = roadRight.constBegin(); ri != roadRight.constEnd(); ri++)
					verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

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

				verts.clear();
				for(QList<AuPoint>::const_iterator ri = groundRight.constBegin(); ri != groundRight.constEnd(); ri++)
					verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

				// Make a VAO of the vertex locations.
				vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}

				// Draw transition zone
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//if(rightOk)
					re.GLSL().LineShader().SetColor(.1f, .1f, .1f, .7f);
				//else
					//re.GLSL().LineShader().SetColor(.5f, .1f, .1f, .7f);
				for(int tzi=0; tzi<3; tzi++)
				{
					verts.clear();
					for(QList<AuPoint>::const_iterator ri = intPtsRight[tzi].constBegin(); ri != intPtsRight[tzi].constEnd(); ri++)
						verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

					// Make a VAO of the vertex locations.
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

				// Transition zone legs.
				verts.clear();
				for(QList<AuPoint>::const_iterator ri = intLegsRight.constBegin(); ri != intLegsRight.constEnd(); ri++)
					verts << AuPointAlpha((*ri).x, (*ri).y, (*ri).z);

				// Make a VAO of the vertex locations.
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
		}

		// Create another cursor to show the existing development that is affected.
		if(exBd)
		{
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
			re.GLSL().LineShader().SetColor(hitColor);
			exBd->RenderCursorHit_Shader(re, false);	// Building and site lines.
		}

		if(pTerrain->HasSubatoms())
		{
			QColor buildingColor(136, 35, 214, 100);		// purple
			QColor roadColor(163, 104, 5, 100);				// orange
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

// Set appropriate device cursor. Default sets Cross cursor if
// pSceneAtom != 0, otherwise sets Arrow cursor.
void DdPlanGeoLandscape::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* pSceneAtom, const QPoint& pxCursor)
{
	//qDebug() << "DdPlanGeoLandscape::SetCursor" << g_AuClient.RoadType();
	const DaWorldLL* pTerrain;			// set by call to TestRoadLocation
	AuPoint fBuildLoc;				// set by call to TestRoadLocation
	DaBd* exBd;
	QString cityName;
	int testloc = TestRoadLocation(pSceneAtom, pxCursor, pTerrain, fBuildLoc, exBd, cityName);
	if(testloc == 0)
	{
		// Location is good. Show the building material on the cursor.
		if(m_Cursor.pixmap().isNull() || m_CtrlDown != m_CursorCtrlDown)
		{
			QPixmap pix;
			if(!m_CtrlDown)
			{
				switch(m_LandscapeType)
				{
				case 0:
					pix.load(":data/bp_RoadClearing.bmp");
					break;
				case 1:
					pix.load(":data/bp_RoadDefoliate.bmp");
					break;
				case 2:
					pix.load(":data/bp_RoadIrrigate.bmp");
					break;
				}
				if(g_AuClient.RoadType())
					pix = pix.scaledToWidth(32);
				if(!pix.isNull())
					m_Cursor = QCursor(pix, 0,0);
			}
			m_CursorCtrlDown = m_CtrlDown;
		}

		if(m_Cursor.pixmap().isNull())
			m_pWnd->setCursor(QCursor(Qt::CrossCursor));
		else
			m_pWnd->setCursor(m_Cursor);
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
int DdPlanGeoLandscape::TestRoadLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL*& pTerrain,
		AuPoint& fBuildLoc, DaBd*& exBd, QString& cityName) const
{
	exBd = 0;

	// Where is the cursor on the terrain?
	pTerrain = dynamic_cast<const DaWorldLL*>(m_pWnd->FindTerrainLocationAtPixel(sceneAtom, pxCursor, fBuildLoc));
	if(!pTerrain)
		return 1;

	float roadWidth2;
	if(g_AuClient.RoadType() == 1)
		roadWidth2 = g_AuClient.SlabWidthInFeet() / 2.;
	else
		roadWidth2 = g_AuClient.RoadWidthInFeet() / 2.;

	// Choose material for build test. It just determines if the road can be built on an inferno.
	COMMODITYID mat = CID_Stone;	// n/a for landscapers

	// When start location has been established, we need to pass the starting location also.
	// We may also need to constrain the angle.
	if(m_StartEstablished && !m_StartLoc.FuzzyEquals(fBuildLoc))
	{
		// Start location has been established. Are we constraining the angle?
		if(m_ShiftDown)
		{
			// Constrain fBuildLoc to be on an even 5 degree angle relative to the start location.
			AuPoint vUp(-pTerrain->GravityDir(m_StartLoc));			// Unit vector pointing up vs gravity.
			AuPoint vX;
			vUp.ArbitraryAxis(vX);									// Returns a unit vector.
			AuPoint vY = vUp.Cross(vX);								// Result will be a unit vector.
			AuPoint bvec(fBuildLoc - m_StartLoc);
			double x = bvec.Dot(vX);
			double y = bvec.Dot(vY);
			double ang = atan2(y,x);								// Angle to fBuildLoc in the plane of vUp.

			// Round to the road angle constraint set by the user.
			double angdeg = ang * RAD2DEG;
			angdeg = FROUND(angdeg, g_AuClient.RoundingAngle());
			ang = angdeg * DEG2RAD;

			double blen = bvec.Length();
			x = cos(ang) * blen;
			y = sin(ang) * blen;
			AuPoint bloc = m_StartLoc + vX * x + vY * y;			// Constrained point in the plane of vUp at alt of m_StartLoc.
			AuGlobeLLF bll;
			if(pTerrain->LLOfLoc(bll, bloc))
				fBuildLoc = pTerrain->GetTerrainLocation(bll);
		}

		// In slab mode, all locations are forced to the same altitude.
		if(g_AuClient.RoadType())
			pTerrain->ElevateLocMSL(fBuildLoc, pTerrain->AltitudeMSL(m_StartLoc));

		return pTerrain->TestBdLocation(IID_Landscape, sceneAtom->EmpireId(), 0, mat, fBuildLoc, roadWidth2, 0, 0, exBd,
				&m_StartLoc, cityName);
	}
	else
		return pTerrain->TestBdLocation(IID_Landscape, sceneAtom->EmpireId(), 0, mat, fBuildLoc, roadWidth2, 0, 0, exBd, 0,
				cityName);
}

void DdPlanGeoLandscape::ConstructRoad(const QPoint& pxCursor)
{
	if(!m_StartEstablished)
	{
		// This is the first location of the road segment.
		DaPerson* person;
		m_pWnd->LockSceneAtom(&person);

		AuIdList idBuildList;
		QStringList sBuildList;

		const DaWorldLL* pTerrain;			// set by call to TestRoadLocation
		AuPoint fBuildLoc;				// set by call to TestRoadLocation
		DaBd* exBd;
		QString cityName;
		int testloc = TestRoadLocation(person, pxCursor, pTerrain, fBuildLoc, exBd, cityName);
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

		const DaWorldLL* pTerrain;			// set by call to TestRoadLocation
		AuPoint fBuildLoc;				// set by call to TestRoadLocation
		DaBd* exBd;
		QString cityName;
		int testloc = TestRoadLocation(person, pxCursor, pTerrain, fBuildLoc, exBd, cityName);
		if(testloc == 0)
		{
			m_pWnd->UnlockSceneAtom();

			double roadWidth;
			if(g_AuClient.RoadType() == 1)
				roadWidth = g_AuClient.SlabWidthInMeters() * FeetPerMeter();
			else
				roadWidth = g_AuClient.RoadWidthInMeters() * FeetPerMeter();

			bool hideRocks;
			int hideGrass;
			bool hideShrubs;
			bool hideTrees;
			WORLDTEX_TYPE pavement = WORLDTEX_ConstructionZone;	// Leaves the natural terrain texture unchanged.
			if(m_LandscapeType == 0)
			{
				// Clearing
				hideRocks = false;
				hideGrass = 2;	// mowed
				hideShrubs = true;
				hideTrees = true;
			}
			else if(m_LandscapeType == 1)
			{
				// Defoliate
				hideRocks = false;
				hideGrass = 1;	// removed
				hideShrubs = true;
				hideTrees = true;
			}
			else if(m_LandscapeType == 2)
			{
				// Irrigate
				hideRocks = true;
				hideGrass = 3;	// lawn
				hideShrubs = false;
				hideTrees = false;
			}
			else
			{
				// wut?
				hideRocks = false;
				hideGrass = 0;
				hideShrubs = false;
				hideTrees = false;
			}
			auApp->GeoLandscape(m_StartLoc, fBuildLoc, roadWidth, hideRocks, hideGrass, hideShrubs, hideTrees, pavement);

			// Continue the next segment where this one left off.
			m_StartLoc = fBuildLoc;
		}
		else
		{
			// Location is not ok.
			m_pWnd->UnlockSceneAtom();
			m_LastTime.restart();
		}
	}
}
