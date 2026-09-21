//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpscpower.cpp,v 1.8 2022/02/19 17:45:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpscpower.h"
//
#include "auatomrenderinfo.h"
#include "auscparts.h"
#include "dacreature.h"
#include "dasc.h"

DpScPower::DpScPower()
{
}

DpScPower::~DpScPower()
{
}

DpScPower::DpScPower(const DpScPower& c) : DpStationPower(c)
{
	m_Screen = c.m_Screen;
	m_ScreenVAO = c.m_ScreenVAO;
}

DpStation* DpScPower::Clone() const
{
	return new DpScPower(*this);
}

void DpScPower::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStationPower::CreateDeviceObjects(ari, parent, inBuilding);

	// Create display screen VAO once we have the screen quad.
	if(!m_ScreenVAO.VAO() && m_Screen.vll != m_Screen.vur)
	{
		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(m_Screen.vll, 0,0);
		tri0.m_V1 = AuPointTex(m_Screen.vul, 0,1);
		tri0.m_V2 = AuPointTex(m_Screen.vur, 1,1);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(m_Screen.vur, 1,1);
		tri1.m_V1 = AuPointTex(m_Screen.vlr, 1,0);
		tri1.m_V2 = AuPointTex(m_Screen.vll, 0,0);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		m_ScreenVAO.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
	}
}

void DpScPower::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStationPower::DeleteDeviceObjects(re);
	if(m_ScreenVAO.VAO())
		m_ScreenVAO.DeleteGL();
}

void DpScPower::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStationPower::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(!m_ScreenTex)
		return;

	// Render the display screen.
	if(m_ScreenVAO.VAO())
	{
		AuMatrix4x4 tcon = tModel * seatedCreature->Transform().Inverse();
		ari.re->GLSL().TextureShader().Select();
		ari.re->GLSL().TextureShader().SetProjectionModelView(projection * tcon);
		ari.re->GLSL().TextureShader().SetTexture(m_ScreenTex);
		ari.re->GLSL().TextureShader().SetColor(1,1,1,1);
		ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
		ari.re->glPolygonOffset(-1.f, -1.f);
		ari.re->glDepthFunc(GL_LEQUAL);
		m_ScreenVAO.SelectGL();
		m_ScreenVAO.DrawGL();
		m_ScreenVAO.DeselectGL();
		ari.re->glDepthFunc(GL_LESS);
		ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
		ari.re->GLSL().TextureShader().Deselect();
	}
	else if(m_Screen.vll == m_Screen.vur)
		seatedCreature->ConsoleScreenQuad(m_Screen);	 		// Get the screen quad from the seated creature, to make the VAO.
}

//static
void DpScPower::RenderEmpty(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari,
		const DaAtom& parent, bool inBuilding, const AuQuad& screen)
{
	Q_UNUSED(inBuilding);
	GLuint tex = CreateDisplayTexture(*ari.re, parent.Id());
	if(!tex)
		return;

	AuGLVAO vao;
	AuTriTex tri0;
	tri0.m_V0 = AuPointTex(screen.vll, 0,0);
	tri0.m_V1 = AuPointTex(screen.vul, 0,1);
	tri0.m_V2 = AuPointTex(screen.vur, 1,1);
	AuTriTex tri1;
	tri1.m_V0 = AuPointTex(screen.vur, 1,1);
	tri1.m_V1 = AuPointTex(screen.vlr, 1,0);
	tri1.m_V2 = AuPointTex(screen.vll, 0,0);
	QList<AuTriTex> tris;
	tris << tri0 << tri1;
	vao.CreateForTexture(*ari.re, tris, GL_STREAM_DRAW);
	if(!vao.VAO())
		return;

	ari.re->GLSL().TextureShader().Select();
	ari.re->GLSL().TextureShader().SetProjectionModelView(projection * tModel);
	ari.re->GLSL().TextureShader().SetTexture(tex);
	ari.re->GLSL().TextureShader().SetColor(1,1,1,1);
	ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
	ari.re->glPolygonOffset(-1.f, -1.f);
	ari.re->glDepthFunc(GL_LEQUAL);
	vao.SelectGL();
	vao.DrawGL();
	vao.DeselectGL();
	vao.DeleteGL();
	ari.re->glDepthFunc(GL_LESS);
	ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
	ari.re->GLSL().TextureShader().Deselect();
}

void DpScPower::PanelDimensions(double& width, double& height) const
{
	width = m_Screen.vll.Distance(m_Screen.vlr);
	height = m_Screen.vll.Distance(m_Screen.vul);
}

void DpScPower::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaCreature& seatedCreature) const
{
	if(m_Screen.vll == m_Screen.vur)
		return;
	AuMatrix4x4 tcon = tModel * seatedCreature.Transform().Inverse();
	AuQuad qscreen(m_Screen);
	qscreen.Transform(tcon);
	aControlPanels.append(qscreen);
	idPanels.append(0);
}

quint64 DpScPower::MemoryUsage() const
{
	quint64 mem = DpStationPower::MemoryUsage();
	mem += sizeof(m_Screen);
	mem += sizeof(m_ScreenVAO);
	return mem;
}
