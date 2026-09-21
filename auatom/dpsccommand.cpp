//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpsccommand.cpp,v 1.9 2022/02/19 17:45:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpsccommand.h"
//
#include "auatomrenderinfo.h"
#include "auscparts.h"
#include "dacreature.h"
#include "dasc.h"

DpScCommand::DpScCommand()
{
}

DpScCommand::~DpScCommand()
{
}

DpScCommand::DpScCommand(const DpScCommand& c) : DpStationEmpty(c)
{
	m_Screen = c.m_Screen;
	m_ScreenVAO = c.m_ScreenVAO;
}

DpStation* DpScCommand::Clone() const
{
	return new DpScCommand(*this);
}

void DpScCommand::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStationEmpty::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());

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

//static
GLuint DpScCommand::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("CCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/txCnCConsole.bmp");
}

void DpScCommand::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStationEmpty::DeleteDeviceObjects(re);
	if(m_ScreenVAO.VAO())
		m_ScreenVAO.DeleteGL();
}

void DpScCommand::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStationEmpty::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
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
void DpScCommand::RenderEmpty(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari,
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

void DpScCommand::PanelDimensions(double& width, double& height) const
{
	width = m_Screen.vll.Distance(m_Screen.vlr);
	height = m_Screen.vll.Distance(m_Screen.vul);
}

void DpScCommand::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
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

quint64 DpScCommand::MemoryUsage() const
{
	quint64 mem = DpStationEmpty::MemoryUsage();
	mem += sizeof(m_Screen);
	mem += sizeof(m_ScreenVAO);
	return mem;
}
