//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dadesignplane.cpp,v 1.158 2022/05/17 16:04:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dadesignplane.h"
//
#include "auatomrenderinfo.h"
#include "aubddesign.h"
#include "augleffects.h"
#include "auglobal.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

DaDesignPlane::DaDesignPlane(const AuId& owner)
{
	m_GravityOn = g_Au.DesignerGravityOn();;
	m_GridInc = FeetPerMeter()/10.;
	m_GridIncNoM = 10;
	m_GridIncOm = 10;
	m_GridOnBits = GRIDBIT_Horizontal|GRIDBIT_Vertical;
	m_GridRad = FeetPerMeter() * 50.;
	m_OwnerId = owner;
	m_RenderSolid = false;		// true for shaded render, false for lines
	m_RoomMode = 0;
	m_RoomId = 0;
}

DaDesignPlane::DaDesignPlane()
{
	m_GravityOn = g_Au.DesignerGravityOn();;
	m_GridInc = FeetPerMeter()/10.;
	m_GridIncNoM = 10;
	m_GridIncOm = 10;
	m_GridOnBits = GRIDBIT_Horizontal|GRIDBIT_Vertical;
	m_GridRad = FeetPerMeter() * 50.;
	m_RenderSolid = false;		// true for shaded render, false for lines
	m_RoomMode = 0;
	m_RoomId = 0;
}

DaDesignPlane::~DaDesignPlane()
{

}

DaDesignPlane::DaDesignPlane(const DaDesignPlane &rhs) : DaAtom(rhs)
{
	m_GravityOn = rhs.m_GravityOn;
	m_GridInc = rhs.m_GridInc;
	m_GridIncNoM = rhs.m_GridIncNoM;
	m_GridIncOm = rhs.m_GridIncOm;
	m_GridOnBits = rhs.m_GridOnBits;
	m_GridRad = rhs.m_GridRad;
	m_OwnerId = rhs.m_OwnerId;			// person atom who owns this design plane
	m_RenderSolid = rhs.m_RenderSolid;	// true for shaded render, false for lines
	m_RoomMode = rhs.m_RoomMode;
	m_RoomId = rhs.m_RoomId;
}

DaAtom *DaDesignPlane::Clone() const
{
	return new DaDesignPlane(*this);
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaDesignPlane::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
	quint8 bits = m_GridOnBits;
	if(m_RenderSolid)
		bits |= 0x10;	// turn on a bit for shaded render mode
	if(m_GravityOn)
		bits |= 0x20;	// turn on a bit for shaded render mode
	toClient << bits;
	m_OwnerId >> toClient;
	toClient << m_RoomId;
	toClient << m_RoomMode;
	toClient << m_GridInc;
	toClient << m_GridRad;
}

// Receive updates to this atom from a server.
void DaDesignPlane::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
	quint8 bits;
	fromServer >> bits;
	m_GridOnBits = bits&0x0f;
	m_RenderSolid = (bits & 0x10) ? true : false;
	m_GravityOn = (bits & 0x20) ? true : false;
	m_OwnerId << fromServer;
	fromServer >> m_RoomId;
	fromServer >> m_RoomMode;
	fromServer >> m_GridInc;
	fromServer >> m_GridRad;

	m_GravityOn = g_Au.DesignerGravityOn();	// New designer keeps gravity state in global settings.
}

// Refresh data from valid counterpart.
bool DaDesignPlane::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaDesignPlane* pA = (const DaDesignPlane*)pAtom;

	// In nu style designer, gravity state and grid visibility are handled entirely on the client.
	m_GravityOn = g_Au.DesignerGravityOn();	// New designer keeps gravity state in global settings.

	m_OwnerId = pA->m_OwnerId;
	m_RoomMode = pA->m_RoomMode;
	m_RoomId = pA->m_RoomId;

	return false;
}

void DaDesignPlane::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(projection);
	Q_UNUSED(sceneAtom);
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	// Do not render distant design planes.
	if(pass != PASS_Near)
		return;
	if(ari.textBrightness)
		renderLast.Insert(rmod, this);
}

void DaDesignPlane::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(sceneAtom);
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	// Base atom class needs to render special effects.
	//DaAtom::RenderLast(sceneAtom, projection, model, re, glw, altView, textBrightness, fade, font);
	const DaDesignBase* env = dynamic_cast<const DaDesignBase*>(Parent());
	if(!env)
		return;

	switch(env->DesignMode())
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		break;
	case DaDesignBase::DESIGN_Spacecraft:
		{
			const DaDesigner* envsc = dynamic_cast<const DaDesigner*>(env);
			if(envsc->IsPreview())
				break;
			RenderScGrid(projection, rmod.Matrix(ParentId()), *ari.re, ari.glw, ari.textBrightness, g_Au.FontDesigner());
		}
		break;
	case DaDesignBase::DESIGN_Building:
		{
			const DaDesigner* envsc = dynamic_cast<const DaDesigner*>(env);
			if(envsc->IsPreview())
				break;
			RenderBdGrid(projection, rmod.Matrix(ParentId()), *ari.re, ari.glw, ari.textBrightness, g_Au.FontDesigner(),
					envsc->BuildingStructure());
		}
		break;
	case DaDesignBase::DESIGN_Item:
		{
			const DaDesigner* envsc = dynamic_cast<const DaDesigner*>(env);
			if(envsc->IsPreview())
				break;
			RenderItGrid(projection, rmod.Matrix(ParentId()), *ari.re, ari.glw, ari.textBrightness, g_Au.FontDesigner(),
					envsc->ItemCommodity());
		}
		break;
	//case DaDesignBase::DESIGN_Vehicle:
		//break;
	}
}

void DaDesignPlane::RenderScGrid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, QOpenGLWidget* glw,
		float textBrightness, const QFont& font)
{
	if(!m_GridOnBits)
		return;

	if(m_GridInc <= 0 || m_GridRad <= 0)
		return;

	AuMatrix4x4 gridM = model * Transform();

	//int gridIncs = m_GridRad / m_GridInc;
	int gridIncs = 500;
	double gridRad = gridIncs * m_GridInc;

	//re.glDisable(GL_DEPTH_TEST);	// So the grid lines will be visible no matter what.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	float axisAlpha = .5f;	// Alpha of axis lines.
	float ba = .5f;	// Alpha of bounding box lines.
	float ga = .2f;	// Alpha of grid lines.

	// Calculate colors of bright and dim lines.
	QColor xBase(g_Au.DesignerGridXColor());
	float xh = xBase.hueF();
	float xs = xBase.saturationF();
	float xv = xBase.valueF() * textBrightness;
	QColor xBright;
	xBright.setHsvF(xh, xs, xv);

	QColor yBase(g_Au.DesignerGridYColor());
	float yh = yBase.hueF();
	float ys = yBase.saturationF();
	float yv = yBase.valueF() * textBrightness;
	QColor yBright;
	yBright.setHsvF(yh, ys, yv);

	QColor zBase(g_Au.DesignerGridZColor());
	float zh = zBase.hueF();
	float zs = zBase.saturationF();
	float zv = zBase.valueF() * textBrightness;
	QColor zBright;
	zBright.setHsvF(zh, zs, zv);

	QColor gridBase(g_Au.DesignerGridColor());
	float gh = gridBase.hueF();
	float gs = gridBase.saturationF();
	float gv = gridBase.valueF() * textBrightness;
	QColor gBright;
	gBright.setHsvF(gh, gs, gv);
	QColor gMedium;
	gMedium.setHsvF(gh, gs, gv * .75f);

	re.GLSL().LineShader().Select();

	AuGLVAO vao;
	AuPointArray verts;

	// Bounding box.
	{
		re.GLSL().LineShader().SetProjectionModelView(projection * model);
		re.GLSL().LineShader().SetColor(1.f, 1.f, 0.f, ba);

		double top, bottom;
		if(m_GridRad == AuBdDesign::SiteSizeLimitBase(BD_Building))
		{
			top = AuBdDesign::SiteSizeLimitHeight(BD_Building);
			bottom = AuBdDesign::SiteSizeLimitDepth(BD_Building);
		}
		else if(m_GridRad == AuBdDesign::HullSizeLimitBase(BD_Building))
		{
			top = AuBdDesign::HullSizeLimitHeight(BD_Building);
			bottom = AuBdDesign::HullSizeLimitDepth(BD_Building);
		}
		else
		{
			top = m_GridRad;
			bottom = m_GridRad;
		}

		// Top of cube
		verts.append(AuPoint(m_GridRad, m_GridRad, top));
		verts.append(AuPoint(m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, m_GridRad, top));
		verts.append(AuPoint(m_GridRad, m_GridRad, top));

		// Z=0 plane
		verts.append(AuPoint(m_GridRad, m_GridRad, 0.));
		verts.append(AuPoint(m_GridRad, -m_GridRad, 0.));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, 0.));
		verts.append(AuPoint(-m_GridRad, m_GridRad, 0.));
		verts.append(AuPoint(m_GridRad, m_GridRad, 0.));

		// Bottom of cube
		verts.append(AuPoint(m_GridRad, m_GridRad, -bottom));
		verts.append(AuPoint(m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, m_GridRad, -bottom));
		verts.append(AuPoint(m_GridRad, m_GridRad, -bottom));

		// Display the VAO once and clean it up.
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// 3 remaining edges of extents cube and the origin marker.
		double oRad = FeetPerMeter() / 2;
		verts.clear();
		verts.append(AuPoint(m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, m_GridRad, -bottom));
		// Origin marker.
		verts.append(AuPoint(-oRad,0,0));
		verts.append(AuPoint(oRad,0,0));
		verts.append(AuPoint(0,-oRad,0));
		verts.append(AuPoint(0,oRad,0));
		verts.append(AuPoint(0,0,-oRad));
		verts.append(AuPoint(0,0,oRad));
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Z plane grid.
	{
		int brightInterval = m_GridIncOm * m_GridIncNoM;

		// Draw dim gridlines at small intervals, skipping large intervals for bright grid lines.
		{
			re.GLSL().LineShader().SetProjectionModelView(projection * gridM);
			re.GLSL().LineShader().SetColor(gMedium.redF(), gMedium.greenF(), gMedium.blueF(), ga);

			// Vertical Lines
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix%m_GridIncOm != 0 && ix%brightInterval != 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy%m_GridIncOm != 0 && iy%brightInterval != 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Draw medium lines at next intervals but not bright intervals.
		{
			re.GLSL().LineShader().SetColor(gBright.redF(), gBright.greenF(), gBright.blueF(), ga);

			// Vertical Lines except axis line.
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix && ix%m_GridIncOm == 0 && ix%brightInterval != 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines except axis line.
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy && iy%m_GridIncOm == 0 && iy%brightInterval != 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Draw brightest lines at large intervals.
		{
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), ga);

			// Vertical Lines except axis line.
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix && ix%brightInterval == 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines except axis line.
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy && iy%brightInterval == 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Primary axis lines are colored according to their plane.
		{
			// Positive side is smoothed for slightly thicker appearance than the negative side.
			re.glEnable(GL_LINE_SMOOTH);	// This helps them to look thicker.
			re.glDisable(GL_BLEND);

			// +X axis
			re.GLSL().LineShader().SetColor(xBright.redF(), xBright.greenF(), xBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(gridRad, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// +Y axis
			re.GLSL().LineShader().SetColor(yBright.redF(), yBright.greenF(), yBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(0, gridRad, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// +Z axis
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(0, 0, gridRad));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// Negative side is not smoothed for slightly thinner appearance than the positive side.
			re.glEnable(GL_BLEND);
			re.glDisable(GL_LINE_SMOOTH);

			// -X axis
			re.GLSL().LineShader().SetColor(xBright.redF(), xBright.greenF(), xBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(-gridRad, 0, 0));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// -Y axis
			re.GLSL().LineShader().SetColor(yBright.redF(), yBright.greenF(), yBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(0, -gridRad, 0));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// -Z axis
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(0, 0, -gridRad));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}

	re.GLSL().LineShader().Deselect();

	// Draw direction indicator text at 3D extents.
	{
		QColor textBright(255,255,255);

		AuPoint loc = AuPoint(0, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Fore"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(0, gridRad, 6);
		re.RenderText(projection, gridM, loc, tr("+Y"), font, yBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(0, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Aft"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(0, -gridRad, 6);
		re.RenderText(projection, gridM, loc, tr("-Y"), font, yBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(-m_GridRad, 0, 6);
		re.RenderText(projection, model, loc, tr("Port"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(-gridRad, 0, 6);
		re.RenderText(projection, gridM, loc, tr("-X"), font, xBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(m_GridRad, 0, 6);
		re.RenderText(projection, model, loc, tr("Starboard"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(gridRad, 0, 6);
		re.RenderText(projection, gridM, loc, tr("+X"), font, xBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(-m_GridRad, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Port Quarter"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(m_GridRad, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Starboard Quarter"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(-m_GridRad, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Port Fore"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(m_GridRad, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Starboard Fore"), font, textBright, glw->height(), Qt::AlignHCenter);
	}

	re.glDisable(GL_BLEND);
	//re.glEnable(GL_DEPTH_TEST);
}

void DaDesignPlane::RenderBdGrid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, QOpenGLWidget* glw,
		float textBrightness, const QFont& font, BUILDING_STRUCTURE bdStruct)
{
	if(!m_GridOnBits)
		return;

	if(m_GridInc <= 0 || m_GridRad <= 0)
		return;

	AuMatrix4x4 gridM = model * Transform();

	//int gridIncs = m_GridRad / m_GridInc;
	int gridIncs = 500;
	double gridRad = gridIncs * m_GridInc;

	//re.glDisable(GL_DEPTH_TEST);	// So the grid lines will be visible no matter what.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	float axisAlpha = .5f;	// Alpha of axis lines.
	float ba = .5f;	// Alpha of bounding box lines.
	float ga = .2f;	// Alpha of grid lines.

	// Calculate colors of bright and dim lines.
	QColor xBase(g_Au.DesignerGridXColor());
	float xh = xBase.hueF();
	float xs = xBase.saturationF();
	float xv = xBase.valueF() * textBrightness;
	QColor xBright;
	xBright.setHsvF(xh, xs, xv);

	QColor yBase(g_Au.DesignerGridYColor());
	float yh = yBase.hueF();
	float ys = yBase.saturationF();
	float yv = yBase.valueF() * textBrightness;
	QColor yBright;
	yBright.setHsvF(yh, ys, yv);

	QColor zBase(g_Au.DesignerGridZColor());
	float zh = zBase.hueF();
	float zs = zBase.saturationF();
	float zv = zBase.valueF() * textBrightness;
	QColor zBright;
	zBright.setHsvF(zh, zs, zv);

	QColor gridBase(g_Au.DesignerGridColor());
	float gh = gridBase.hueF();
	float gs = gridBase.saturationF();
	float gv = gridBase.valueF() * textBrightness;
	QColor gBright;
	gBright.setHsvF(gh, gs, gv);
	QColor gMedium;
	gMedium.setHsvF(gh, gs, gv * .75f);

	re.GLSL().LineShader().Select();

	AuGLVAO vao;
	AuPointArray verts;

	// Bounding box.
	{
		re.GLSL().LineShader().SetProjectionModelView(projection * model);
		re.GLSL().LineShader().SetColor(1.f, 1.f, 0.f, ba);

		double top, bottom;
		if(m_GridRad == AuBdDesign::SiteSizeLimitBase(bdStruct))
		{
			top = AuBdDesign::SiteSizeLimitHeight(bdStruct);
			bottom = AuBdDesign::SiteSizeLimitDepth(bdStruct);
		}
		else if(m_GridRad == AuBdDesign::HullSizeLimitBase(bdStruct))
		{
			top = AuBdDesign::HullSizeLimitHeight(bdStruct);
			bottom = AuBdDesign::HullSizeLimitDepth(bdStruct);
		}
		else
		{
			top = m_GridRad;
			bottom = m_GridRad;
		}

		// Top of cube
		verts.append(AuPoint(m_GridRad, m_GridRad, top));
		verts.append(AuPoint(m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, m_GridRad, top));
		verts.append(AuPoint(m_GridRad, m_GridRad, top));

		// Z=0 plane
		verts.append(AuPoint(m_GridRad, m_GridRad, 0.));
		verts.append(AuPoint(m_GridRad, -m_GridRad, 0.));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, 0.));
		verts.append(AuPoint(-m_GridRad, m_GridRad, 0.));
		verts.append(AuPoint(m_GridRad, m_GridRad, 0.));

		// Bottom of cube
		verts.append(AuPoint(m_GridRad, m_GridRad, -bottom));
		verts.append(AuPoint(m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, m_GridRad, -bottom));
		verts.append(AuPoint(m_GridRad, m_GridRad, -bottom));

		// Display the VAO once and clean it up.
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// 3 remaining edges of extents cube and the origin marker.
		double oRad = FeetPerMeter() / 2;
		verts.clear();
		verts.append(AuPoint(m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, m_GridRad, -bottom));
		// Origin marker.
		verts.append(AuPoint(-oRad,0,0));
		verts.append(AuPoint(oRad,0,0));
		verts.append(AuPoint(0,-oRad,0));
		verts.append(AuPoint(0,oRad,0));
		verts.append(AuPoint(0,0,-oRad));
		verts.append(AuPoint(0,0,oRad));
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Z plane grid.
	{
		int brightInterval = m_GridIncOm * m_GridIncNoM;

		// Draw dim gridlines at small intervals, skipping large intervals for bright grid lines.
		{
			re.GLSL().LineShader().SetProjectionModelView(projection * gridM);
			re.GLSL().LineShader().SetColor(gMedium.redF(), gMedium.greenF(), gMedium.blueF(), ga);

			// Vertical Lines
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix%m_GridIncOm != 0 && ix%brightInterval != 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy%m_GridIncOm != 0 && iy%brightInterval != 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Draw medium lines at next intervals but not bright intervals.
		{
			re.GLSL().LineShader().SetColor(gBright.redF(), gBright.greenF(), gBright.blueF(), ga);

			// Vertical Lines except axis line.
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix && ix%m_GridIncOm == 0 && ix%brightInterval != 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines except axis line.
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy && iy%m_GridIncOm == 0 && iy%brightInterval != 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Draw brightest lines at large intervals.
		{
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), ga);

			// Vertical Lines except axis line.
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix && ix%brightInterval == 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines except axis line.
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy && iy%brightInterval == 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Primary axis lines are colored according to their plane.
		{
			// Positive side is smoothed for slightly thicker appearance than the negative side.
			re.glEnable(GL_LINE_SMOOTH);	// This helps them to look thicker.
			re.glDisable(GL_BLEND);

			// +X axis
			re.GLSL().LineShader().SetColor(xBright.redF(), xBright.greenF(), xBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(gridRad, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// +Y axis
			re.GLSL().LineShader().SetColor(yBright.redF(), yBright.greenF(), yBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(0, gridRad, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// +Z axis
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(0, 0, gridRad));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// Negative side is not smoothed for slightly thinner appearance than the positive side.
			re.glEnable(GL_BLEND);
			re.glDisable(GL_LINE_SMOOTH);

			// -X axis
			re.GLSL().LineShader().SetColor(xBright.redF(), xBright.greenF(), xBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(-gridRad, 0, 0));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// -Y axis
			re.GLSL().LineShader().SetColor(yBright.redF(), yBright.greenF(), yBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(0, -gridRad, 0));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// -Z axis
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(0, 0, -gridRad));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}

	re.GLSL().LineShader().Deselect();

	// Draw direction indicator text at 3D extents.
	{
		QColor textBright(255,255,255);

		AuPoint loc = AuPoint(0, m_GridRad, 6);
		switch(bdStruct)
		{
		case BD_Invalid:
		case BD_Biodome:
		case BD_Bridge:
		case BD_Fence:
		case BD_SeaPlatform:
		case BD_AerialPlatform:
		case BD_GroundPlatform:
		case BD_RoadEnclosure:
		case BD_SeaSurfaceBuilding:
		case BD_SpaceElevator:
		case BD_Statue:
		case BD_Tunnel:
		case BD_StreetLight:
		case BD_Count:
			re.RenderText(projection, model, loc,
					tr("%1\nFront").arg(BuildingStructureName(bdStruct)), font, textBright, glw->height(), Qt::AlignHCenter);
			break;
		case BD_Building:
			re.RenderText(projection, model, loc,
					tr("%1\nFront\nStreet Side / Open Water Side for Coastal Buildings").arg(BuildingStructureName(bdStruct)), font,
					textBright, glw->height(), Qt::AlignHCenter);
			break;
		}
		loc = AuPoint(0, gridRad, 6);
		re.RenderText(projection, gridM, loc, tr("+Y"), font, yBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(0, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Back"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(0, -gridRad, 6);
		re.RenderText(projection, gridM, loc, tr("-Y"), font, yBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(-m_GridRad, 0, 6);
		re.RenderText(projection, model, loc, tr("Left"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(-gridRad, 0, 6);
		re.RenderText(projection, gridM, loc, tr("-X"), font, xBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(m_GridRad, 0, 6);
		re.RenderText(projection, model, loc, tr("Right"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(gridRad, 0, 6);
		re.RenderText(projection, gridM, loc, tr("+X"), font, xBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(-m_GridRad, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Back Left"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(m_GridRad, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Back Right"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(-m_GridRad, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Front Left"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(m_GridRad, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Front Right"), font, textBright, glw->height(), Qt::AlignHCenter);
	}

	re.glDisable(GL_BLEND);
	//re.glEnable(GL_DEPTH_TEST);
}

void DaDesignPlane::RenderItGrid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, QOpenGLWidget* glw,
		float textBrightness, const QFont& font, COMMODITYID cid)
{
	if(!m_GridOnBits)
		return;

	if(m_GridInc <= 0 || m_GridRad <= 0)
		return;

	AuMatrix4x4 gridM = model * Transform();

	//int gridIncs = m_GridRad / m_GridInc;
	int gridIncs = 500;
	double gridRad = gridIncs * m_GridInc;

	//re.glDisable(GL_DEPTH_TEST);	// So the grid lines will be visible no matter what.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	float axisAlpha = .5f;	// Alpha of axis lines.
	//float ba = .5f;	// Alpha of bounding box lines.
	float ga = .2f;	// Alpha of grid lines.

	// Calculate colors of bright and dim lines.
	QColor xBase(g_Au.DesignerGridXColor());
	float xh = xBase.hueF();
	float xs = xBase.saturationF();
	float xv = xBase.valueF() * textBrightness;
	QColor xBright;
	xBright.setHsvF(xh, xs, xv);

	QColor yBase(g_Au.DesignerGridYColor());
	float yh = yBase.hueF();
	float ys = yBase.saturationF();
	float yv = yBase.valueF() * textBrightness;
	QColor yBright;
	yBright.setHsvF(yh, ys, yv);

	QColor zBase(g_Au.DesignerGridZColor());
	float zh = zBase.hueF();
	float zs = zBase.saturationF();
	float zv = zBase.valueF() * textBrightness;
	QColor zBright;
	zBright.setHsvF(zh, zs, zv);

	QColor gridBase(g_Au.DesignerGridColor());
	float gh = gridBase.hueF();
	float gs = gridBase.saturationF();
	float gv = gridBase.valueF() * textBrightness;
	QColor gBright;
	gBright.setHsvF(gh, gs, gv);
	QColor gMedium;
	gMedium.setHsvF(gh, gs, gv * .75f);

	re.GLSL().LineShader().Select();

	AuGLVAO vao;
	AuPointArray verts;

	// Bounding box.
	/*DHH It
	{
		re.GLSL().LineShader().SetProjectionModelView(projection * model);
		re.GLSL().LineShader().SetColor(1.f, 1.f, 0.f, ba);

		double top, bottom;
		if(m_GridRad == AuBdDesign::SiteSizeLimitBase(bdStruct))
		{
			top = AuBdDesign::SiteSizeLimitHeight(bdStruct);
			bottom = AuBdDesign::SiteSizeLimitDepth(bdStruct);
		}
		else if(m_GridRad == AuBdDesign::HullSizeLimitBase(bdStruct))
		{
			top = AuBdDesign::HullSizeLimitHeight(bdStruct);
			bottom = AuBdDesign::HullSizeLimitDepth(bdStruct);
		}
		else
		{
			top = m_GridRad;
			bottom = m_GridRad;
		}

		// Top of cube
		verts.append(AuPoint(m_GridRad, m_GridRad, top));
		verts.append(AuPoint(m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, m_GridRad, top));
		verts.append(AuPoint(m_GridRad, m_GridRad, top));

		// Z=0 plane
		verts.append(AuPoint(m_GridRad, m_GridRad, 0.));
		verts.append(AuPoint(m_GridRad, -m_GridRad, 0.));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, 0.));
		verts.append(AuPoint(-m_GridRad, m_GridRad, 0.));
		verts.append(AuPoint(m_GridRad, m_GridRad, 0.));

		// Bottom of cube
		verts.append(AuPoint(m_GridRad, m_GridRad, -bottom));
		verts.append(AuPoint(m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, m_GridRad, -bottom));
		verts.append(AuPoint(m_GridRad, m_GridRad, -bottom));

		// Display the VAO once and clean it up.
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// 3 remaining edges of extents cube and the origin marker.
		double oRad = FeetPerMeter() / 2;
		verts.clear();
		verts.append(AuPoint(m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, -m_GridRad, -bottom));
		verts.append(AuPoint(-m_GridRad, m_GridRad, top));
		verts.append(AuPoint(-m_GridRad, m_GridRad, -bottom));
		// Origin marker.
		verts.append(AuPoint(-oRad,0,0));
		verts.append(AuPoint(oRad,0,0));
		verts.append(AuPoint(0,-oRad,0));
		verts.append(AuPoint(0,oRad,0));
		verts.append(AuPoint(0,0,-oRad));
		verts.append(AuPoint(0,0,oRad));
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}
	*/

	// Z plane grid.
	{
		int brightInterval = m_GridIncOm * m_GridIncNoM;

		// Draw dim gridlines at small intervals, skipping large intervals for bright grid lines.
		{
			re.GLSL().LineShader().SetProjectionModelView(projection * gridM);
			re.GLSL().LineShader().SetColor(gMedium.redF(), gMedium.greenF(), gMedium.blueF(), ga);

			// Vertical Lines
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix%m_GridIncOm != 0 && ix%brightInterval != 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy%m_GridIncOm != 0 && iy%brightInterval != 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Draw medium lines at next intervals but not bright intervals.
		{
			re.GLSL().LineShader().SetColor(gBright.redF(), gBright.greenF(), gBright.blueF(), ga);

			// Vertical Lines except axis line.
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix && ix%m_GridIncOm == 0 && ix%brightInterval != 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines except axis line.
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy && iy%m_GridIncOm == 0 && iy%brightInterval != 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Draw brightest lines at large intervals.
		{
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), ga);

			// Vertical Lines except axis line.
			verts.clear();
			for(int ix=-gridIncs; ix<=gridIncs; ix++)
			{
				if(ix && ix%brightInterval == 0)
				{
					double rx = m_GridInc * ix;
					verts.append(AuPoint(rx, gridRad, 0));
					verts.append(AuPoint(rx, -gridRad, 0));
				}
			}

			// Horizontal Lines except axis line.
			for(int iy=-gridIncs; iy<=gridIncs; iy++)
			{
				if(iy && iy%brightInterval == 0)
				{
					double ry = m_GridInc * iy;
					verts.append(AuPoint(gridRad, ry, 0));
					verts.append(AuPoint(-gridRad, ry, 0));
				}
			}

			// Display the VAO once and clean it up.
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Primary axis lines are colored according to their plane.
		{
			// Positive side is smoothed for slightly thicker appearance than the negative side.
			re.glEnable(GL_LINE_SMOOTH);	// This helps them to look thicker.
			re.glDisable(GL_BLEND);

			// +X axis
			re.GLSL().LineShader().SetColor(xBright.redF(), xBright.greenF(), xBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(gridRad, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// +Y axis
			re.GLSL().LineShader().SetColor(yBright.redF(), yBright.greenF(), yBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(0, gridRad, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// +Z axis
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), 1);
			verts.clear();
			verts.append(AuPoint(0, 0, 0));
			verts.append(AuPoint(0, 0, gridRad));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// Negative side is not smoothed for slightly thinner appearance than the positive side.
			re.glEnable(GL_BLEND);
			re.glDisable(GL_LINE_SMOOTH);

			// -X axis
			re.GLSL().LineShader().SetColor(xBright.redF(), xBright.greenF(), xBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(-gridRad, 0, 0));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// -Y axis
			re.GLSL().LineShader().SetColor(yBright.redF(), yBright.greenF(), yBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(0, -gridRad, 0));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// -Z axis
			re.GLSL().LineShader().SetColor(zBright.redF(), zBright.greenF(), zBright.blueF(), axisAlpha);
			verts.clear();
			verts.append(AuPoint(0, 0, -gridRad));
			verts.append(AuPoint(0, 0, 0));
			vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}

	re.GLSL().LineShader().Deselect();

	// Draw direction indicator text at 3D extents.
	{
		QColor textBright(255,255,255);

		AuPoint loc = AuPoint(0, m_GridRad, 6);
		switch(cid)
		{
		case CID_Invalid:
			re.RenderText(projection, model, loc,
					tr("Item\nFront"), font, textBright, glw->height(), Qt::AlignHCenter);
			break;
		default:
			re.RenderText(projection, model, loc,
					tr("%1\nFront").arg(g_Au.CommodityName(cid)), font, textBright, glw->height(), Qt::AlignHCenter);
			break;
		}
		loc = AuPoint(0, gridRad, 6);
		re.RenderText(projection, gridM, loc, tr("+Y"), font, yBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(0, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Back"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(0, -gridRad, 6);
		re.RenderText(projection, gridM, loc, tr("-Y"), font, yBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(-m_GridRad, 0, 6);
		re.RenderText(projection, model, loc, tr("Left"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(-gridRad, 0, 6);
		re.RenderText(projection, gridM, loc, tr("-X"), font, xBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(m_GridRad, 0, 6);
		re.RenderText(projection, model, loc, tr("Right"), font, textBright, glw->height(), Qt::AlignHCenter);
		loc = AuPoint(gridRad, 0, 6);
		re.RenderText(projection, gridM, loc, tr("+X"), font, xBright, glw->height(), Qt::AlignHCenter|Qt::AlignTop);

		loc = AuPoint(-m_GridRad, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Back Left"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(m_GridRad, -m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Back Right"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(-m_GridRad, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Front Left"), font, textBright, glw->height(), Qt::AlignHCenter);

		loc = AuPoint(m_GridRad, m_GridRad, 6);
		re.RenderText(projection, model, loc, tr("Front Right"), font, textBright, glw->height(), Qt::AlignHCenter);
	}

	re.glDisable(GL_BLEND);
	//re.glEnable(GL_DEPTH_TEST);
}

// Insert first time database record.
void DaDesignPlane::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO a_DesignPlane (id,idp,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
				"timeModified,timeTock,timeDeath, gravity,onBits,owner,render,roomid,roomMode,inc,rad) "
				"VALUES (?,?,?,?,?,?,?,?,?,?,?,?, ?,?,?,?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaDesignPlane values.
	qa.addBindValue(m_GravityOn);
	qa.addBindValue(m_GridOnBits);
	qa.addBindValue(m_OwnerId.GetID());
	qa.addBindValue(m_RenderSolid);
	qa.addBindValue(m_RoomId);
	qa.addBindValue(m_RoomMode);
	qa.addBindValue(m_GridInc);
	qa.addBindValue(m_GridRad);

	if(!qa.exec())
		qWarning("DaDesignPlane::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaDesignPlane::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_DesignPlane SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, "
				"timeModified=?, timeTock=?, timeDeath=?, gravity=?, onBits=?, render=?, "
				"roomid=?, roomMode=?, inc=?, rad=? WHERE id=%2;").arg(Id().GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaDesignPlane values.
	qa.addBindValue(m_GravityOn);
	qa.addBindValue(m_GridOnBits);
	qa.addBindValue(m_RenderSolid);
	qa.addBindValue(m_RoomId);
	qa.addBindValue(m_RoomMode);
	qa.addBindValue(m_GridInc);
	qa.addBindValue(m_GridRad);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaDesignPlane::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaDesignPlane::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaDesignPlane* DaDesignPlane::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,"
				"timeTock,timeDeath, gravity,onBits,owner,render,roomid,roomMode,inc,rad FROM a_DesignPlane WHERE ");
	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaDesignPlane *pnu = new DaDesignPlane;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			// DaDesignPlane values.
			pnu->m_GravityOn = qa.value(qv++).toBool();
			pnu->m_GridOnBits = qa.value(qv++).toInt();
			pnu->m_OwnerId = AuId(qa.value(qv++).toUInt());
			pnu->m_RenderSolid = qa.value(qv++).toBool();
			pnu->m_RoomId = qa.value(qv++).toInt();
			pnu->m_RoomMode = qa.value(qv++).toInt();
			pnu->m_GridInc = qa.value(qv++).toDouble();
			pnu->m_GridRad = qa.value(qv++).toDouble();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

void DaDesignPlane::DecrementRoomId()
{
	m_RoomId--;
	//qDebug("DaDesignPlane::DecrementRoomId - SetModified");
	SetModified();
}

void DaDesignPlane::IncrementRoomId()
{
	m_RoomId++;
	//qDebug("DaDesignPlane::IncrementRoomId - SetModified");
	SetModified();
}

void DaDesignPlane::EnableHorizontalLines()
{
	m_GridOnBits |= GRIDBIT_Horizontal;
	SetModified();
}

void DaDesignPlane::EnableDiagonal45Lines()
{
	m_GridOnBits |= GRIDBIT_Diagonal45;
	SetModified();
}

void DaDesignPlane::EnableVerticalLines()
{
	m_GridOnBits |= GRIDBIT_Vertical;
	SetModified();
}

void DaDesignPlane::EnableDiagonal135Lines()
{
	m_GridOnBits |= GRIDBIT_Diagonal135;
	SetModified();
}

void DaDesignPlane::DisableHorizontalLines()
{
	m_GridOnBits &= ~GRIDBIT_Horizontal;
	SetModified();
}

void DaDesignPlane::DisableDiagonal45Lines()
{
	m_GridOnBits &= ~GRIDBIT_Diagonal45;
	SetModified();
}

void DaDesignPlane::DisableVerticalLines()
{
	m_GridOnBits &= ~GRIDBIT_Vertical;
	SetModified();
}

void DaDesignPlane::DisableDiagonal135Lines()
{
	m_GridOnBits &= ~GRIDBIT_Diagonal135;
	SetModified();
}

bool DaDesignPlane::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(cameraEnvLoc);
	// Design plane only appears in the scene when it is owned by the scene atom.
	Q_ASSERT(sceneAtom);
	return sceneAtom->Is(m_OwnerId);
}

quint64 DaDesignPlane::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaDesignPlane) - sizeof(DaAtom);
	return mem;
}

DaAtom::ATOM_TYPE DaDesignPlane::AtomType() const
{
	return ATOM_DesignPlane;
}

bool DaDesignPlane::IsTicker() const
{
	return false;
}

void DaDesignPlane::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << m_GravityOn;
	out << m_GridInc;
	out << m_GridIncNoM;
	out << m_GridIncOm;
	out << m_GridOnBits;
	out << m_GridRad;
	out << m_OwnerId;
	out << m_RenderSolid;
	out << m_RoomMode;
	out << m_RoomId;
}

void DaDesignPlane::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	in >> m_GravityOn;
	in >> m_GridInc;
	in >> m_GridIncNoM;
	in >> m_GridIncOm;
	in >> m_GridOnBits;
	in >> m_GridRad;
	in >> m_OwnerId;
	in >> m_RenderSolid;
	in >> m_RoomMode;
	in >> m_RoomId;
}

