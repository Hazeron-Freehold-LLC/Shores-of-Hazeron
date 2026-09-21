//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdsite.cpp,v 1.8 2022/02/19 18:26:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddbdsite.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aubdsitedlg.h"
#include "auscparts.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QInputDialog>
#include <QMouseEvent>

DdBdSite::DdBdSite(AuDesignerView* view) : DdCommand(view)
{
	m_Blend = 0;
	m_Grade = 0;
	m_Rocks = 0;
	m_Round = false;
	m_Grass = 0;
	m_Shrubs = 0;
	m_Steps = 16;
	m_Trees = 0;
	m_Pavement = 0;
}

DdBdSite::~DdBdSite()
{

}

void DdBdSite::Start()
{
	DdCommand::Start();

	AuBdSiteDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
	{
		Terminate();
		return;
	}

	switch(dlg.Draw())
	{
	case 0:	// draw
		// Remember dialog options for MakeOutline after draw is complete.
		m_Blend = dlg.Blend();
		m_Grade = dlg.Grade();
		m_Grass = dlg.Grass();
		m_Rocks = dlg.Rocks();
		m_Shrubs = dlg.Shrubs();
		m_Trees = dlg.Trees();
		m_Pavement = dlg.Pavement();
		break;
	case 1:	// ag
		MakeLotAgricultural();
		Terminate();
		break;
	case 2:	// commercial
		MakeLotCommercial();
		Terminate();
		break;
	case 3:	// industrial
		MakeLotIndustrial();
		Terminate();
		break;
	case 4:	// residential
		MakeLotResidential();
		Terminate();
		break;
	case 5:	// shore
		MakeLotShore();
		Terminate();
		break;
	case 6:	// mine/well
		MakeLotMine();
		Terminate();
		break;
	}
}

QString DdBdSite::WikiTopic() const
{
	return "Designer_Menu#Site_2";
}

void DdBdSite::UpdatePrompt() const
{
	QString prompt;
	if(m_Round)
	{
		if(m_Points.size() == 0)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Center of round site. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">G%1Round, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else if(m_Points.size() == 1)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Radius point of round site. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else
	{
		if(m_Points.size() == 0)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Start site shape. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">G%1Round, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else if(m_Points.size() < 3)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Continue site shape. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Continue site shape. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdBdSite::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	if(m_Points.empty() || m_Points.last() != loc)
		m_Points.append(loc);
	if(m_Round && m_Points.size() == 2)
	{
		// We have our two points to make a circle.
		AuPointArray pts;
		pts.MakeCircle(m_Points.first(), loc, m_Steps);
		if(MakeOutline(pts, m_Grade, m_Blend, m_Pavement, m_Rocks, m_Grass, m_Shrubs, m_Trees))
			Terminate();
	}
	else
		UpdatePrompt();
	return loc;
}

// Set appropriate device cursor.
void DdBdSite::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdBdSite::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;
	switch(e->key())
	{
	case Qt::Key_B:	// Back
		if(!m_Points.empty())
		{
			m_Points.pop_back();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_C:	// Cancel
		if(!m_Points.empty())
		{
			m_Points.clear();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_D:	// Done
		if(m_Round)
			break;
		if(MakeOutline(m_Points, m_Grade, m_Blend, m_Pavement, m_Rocks, m_Grass, m_Shrubs, m_Trees))
		{
			m_Points.clear();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_G:	// Round
		if(m_Points.size() == 0)
		{
			m_Round = !m_Round;
			if(m_Round)
			{
				// Round selected, ask for the number of circle sides.
				int lastSides = g_Au.Settings().readNumEntry("/Settings/SiteCircleSteps", 16);
				bool ok;
				int circleSides = QInputDialog::getInt(m_pWnd, g_Au.ApplicationName(),
						QObject::tr("Enter the number of sides around the circle."), lastSides, 3, 1000, 1, &ok);
				if(ok)
				{
					m_Steps = circleSides;
					g_Au.Settings().writeEntry("/Settings/SiteCircleSteps", m_Steps);
				}
				else
					m_Round = false;	// Put it back off.
			}
			UpdatePrompt();
		}
		break;

	default:
		break;
	}
	return false;
}

// Render "cursor" objects in the scene.
void DdBdSite::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
	if(!pde)
		return;

	// Get cursor location in coordinates relative to the design environment.
	AuPoint loc = m_pWnd->FromPixelToDesignPlane(sceneAtom, pxCursor, true, 0);

	// Cursor lines must be relative to scene atom.
	AuMatrix4x4 ts;
	sceneAtom->GetTransformRelativeToParent(ts, pde);
	ts = model * ts.Inverse();

	if(!m_Points.empty())
	{
		// Use line shader so lines will be super bright, even at night.
		re.GLSL().LineShader().Select();
		re.GLSL().LineShader().SetProjectionModelView(projection * ts);
		re.GLSL().LineShader().SetColor(.7f, 1, .7f);

		// Stairs conform exactly to the grid so the dimensions change between diagonal and straight.
		re.glDisable(GL_DEPTH_TEST);

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		if(m_Round)
		{
			AuPointArray pts;
			pts.MakeCircle(m_Points.last(), loc, m_Steps);
			vao.CreateForLines(re, pts, true, GL_STREAM_DRAW);
		}
		else
		{
			AuPointArray pts(m_Points);
			pts.append(loc);
			vao.CreateForLines(re, pts, true, GL_STREAM_DRAW);
		}
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		AuPoint vec = loc - m_Points.last();
		vec = m_pWnd->DesignPlane().UnProjectVector(vec);
		QString sloc = g_AuClient.FormatCoordinate(vec);

		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		re.RenderText(projection, ts, loc, sloc, g_Au.FontDesigner(), QColor(255,255,255), m_pWnd->size().height());
		re.glDisable(GL_BLEND);

		re.glEnable(GL_DEPTH_TEST);

		// Restore rendering states.
		re.GLSL().LineShader().Deselect();
	}

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdBdSite::MakeOutline(const AuPointArray& points, int gradeMode, int blend, int pavement, int rocks, int grass, int shrubs,
		int trees)
{
	if(points.size() < 3)
		return false;

	AuScSite mesh;
	mesh.SetGrade((AuScSite::GRADE)gradeMode);
	mesh.SetGrass(grass);
	mesh.SetPavement(pavement);
	mesh.SetRocks(rocks);
	mesh.SetShrubs(shrubs);
	mesh.SetTerrainBlend(blend);
	mesh.SetTrees(trees);
	mesh.MakeSite(points, m_pWnd->DesignPlane());
	if(!mesh.IsValid())
		return false;

	mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(mesh);
	return true;
}

void DdBdSite::MakeLotAgricultural()
{
	// Make the level building site.
	double bldWid2 = 13 * FeetPerMeter();	// 1/2 the width of the square building site.
	AuScSite bldsite;
	bldsite.SetGrade(AuScSite::Level);
	bldsite.SetGrass(0);	// No grass.
	bldsite.SetPavement(WORLDTEX_CobblestoneLight);
	bldsite.SetRocks(0);
	bldsite.SetShrubs(0);
	bldsite.SetTrees(0);
	AuPointArray bldpts;
	bldpts.append(AuPoint(-bldWid2, -bldWid2));
	bldpts.append(AuPoint(-bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, -bldWid2));
	bldsite.MakeSite(bldpts, m_pWnd->DesignPlane());
	bldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!bldsite.IsValid())
		return;

	// Make the yard around the building site.
	double yardWid2 = 12 * FeetPerMeter();	// 1/2 the width of the square yard site.
	AuScSite yardsite;
	yardsite.SetGrade(AuScSite::Level);	// Level but we will fade it off to the edges.
	yardsite.SetGrass(3);	// Lawn
	yardsite.SetPavement(WORLDTEX_ConstructionZone);	// No pavement
	yardsite.SetTerrainBlend(1);	// Roll
	yardsite.SetRocks(0);
	yardsite.SetShrubs(0);
	yardsite.SetTrees(1);
	AuPointArray yardpts;
	yardpts.append(AuPoint(-yardWid2, -yardWid2));
	yardpts.append(AuPoint(-yardWid2, yardWid2));
	yardpts.append(AuPoint(yardWid2, yardWid2));
	yardpts.append(AuPoint(yardWid2, -yardWid2));
	yardpts.append(AuPoint(-yardWid2, -yardWid2));
	yardpts.append(AuPoint(-bldWid2, -bldWid2));
	yardpts.append(AuPoint(bldWid2, -bldWid2));
	yardpts.append(AuPoint(bldWid2, bldWid2));
	yardpts.append(AuPoint(-bldWid2, bldWid2));
	yardpts.append(AuPoint(-bldWid2, -bldWid2));
	yardsite.MakeSite(yardpts, m_pWnd->DesignPlane());
	yardsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());

	// Add red glow to the edges of the yard site, to make the terrain leveling effect fade off. The terrain level effect drops off
	// as the red glow gets brighter.
	AuPointArray glowpts;
	glowpts.append(yardpts[0]);
	glowpts.append(yardpts[1]);
	glowpts.append(yardpts[2]);
	glowpts.append(yardpts[3]);
	yardsite.GlowVertexLocs(glowpts, QColor(128,0,0));
	if(!yardsite.IsValid())
		return;

	// Make the field around the yard site.
	double fieldWid2 = 50 * FeetPerMeter();	// 1/2 the width of the square field site.
	AuScSite fieldsite;
	fieldsite.SetGrade(AuScSite::Level);	// Level but we will fade it off to the edges.
	fieldsite.SetGrass(1);	// Natural grass
	fieldsite.SetPavement(WORLDTEX_DirtCrackedSmooth);
	fieldsite.SetTerrainBlend(1);	// Roll
	fieldsite.SetRocks(0);
	fieldsite.SetShrubs(0);
	fieldsite.SetTrees(0);
	AuPointArray fieldpts;
	fieldpts.append(AuPoint(-fieldWid2, -fieldWid2));
	fieldpts.append(AuPoint(-fieldWid2, fieldWid2));
	fieldpts.append(AuPoint(fieldWid2, fieldWid2));
	fieldpts.append(AuPoint(fieldWid2, -fieldWid2));
	fieldpts.append(AuPoint(-fieldWid2, -fieldWid2));
	fieldpts.append(AuPoint(-yardWid2, -yardWid2));
	fieldpts.append(AuPoint(yardWid2, -yardWid2));
	fieldpts.append(AuPoint(yardWid2, yardWid2));
	fieldpts.append(AuPoint(-yardWid2, yardWid2));
	fieldpts.append(AuPoint(-yardWid2, -yardWid2));
	fieldsite.MakeSite(fieldpts, m_pWnd->DesignPlane());
	fieldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());

	// Add red glow to the edges of the yard site, to make the terrain leveling effect fade off. The terrain level effect drops off
	// as the red glow gets brighter.
	fieldsite.GlowVertexLocs(glowpts, QColor(128,0,0));	// Continue yard .5 glow.
	glowpts.clear();
	glowpts.append(fieldpts[0]);
	glowpts.append(fieldpts[1]);
	glowpts.append(fieldpts[2]);
	glowpts.append(fieldpts[3]);
	fieldsite.GlowVertexLocs(glowpts, QColor(255,0,0));	// Fade off to 0 (full red glow) at the edges.
	if(!fieldsite.IsValid())
		return;

	// Move everything so the insertion point is at the front edge.
	AuMatrix4x4 m;
	m.Translate(0,-fieldWid2,0);
	bldsite.Transform(m);
	yardsite.Transform(m);
	fieldsite.Transform(m);

	// Add the parts.
	auApp->DesignerPartAdd(bldsite);
	auApp->DesignerPartAdd(yardsite);
	auApp->DesignerPartAdd(fieldsite);
}

void DdBdSite::MakeLotCommercial()
{
	// Make the level building site.
	double bldWid2 = 16 * FeetPerMeter();	// 1/2 the width of the square building site.
	AuScSite bldsite;
	bldsite.SetGrade(AuScSite::Level);
	bldsite.SetGrass(0);	// No grass.
	bldsite.SetPavement(WORLDTEX_ConcreteHeavy);
	bldsite.SetRocks(0);
	bldsite.SetShrubs(0);
	bldsite.SetTrees(0);
	AuPointArray bldpts;
	bldpts.append(AuPoint(-bldWid2, -bldWid2));
	bldpts.append(AuPoint(-bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, -bldWid2));
	bldsite.MakeSite(bldpts, m_pWnd->DesignPlane());
	bldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!bldsite.IsValid())
		return;

	// Move everything so the insertion point is at the front edge.
	AuMatrix4x4 m;
	m.Translate(0,-bldWid2,0);
	bldsite.Transform(m);

	// Add the parts.
	auApp->DesignerPartAdd(bldsite);
}

void DdBdSite::MakeLotIndustrial()
{
	// Make the level building site.
	double bldWid2 = 23 * FeetPerMeter();	// 1/2 the width of the square building site.
	AuScSite bldsite;
	bldsite.SetGrade(AuScSite::Level);
	bldsite.SetGrass(0);	// No grass.
	bldsite.SetPavement(WORLDTEX_ConcreteHeavy);
	bldsite.SetRocks(0);
	bldsite.SetShrubs(0);
	bldsite.SetTrees(0);
	AuPointArray bldpts;
	bldpts.append(AuPoint(-bldWid2, -bldWid2));
	bldpts.append(AuPoint(-bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, -bldWid2));
	bldsite.MakeSite(bldpts, m_pWnd->DesignPlane());
	bldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!bldsite.IsValid())
		return;

	// Move everything so the insertion point is at the front edge.
	AuMatrix4x4 m;
	m.Translate(0,-bldWid2,0);
	bldsite.Transform(m);

	// Add the parts.
	auApp->DesignerPartAdd(bldsite);
}

void DdBdSite::MakeLotResidential()
{
	// Make the level building site.
	double bldWid2 = 8 * FeetPerMeter();	// 1/2 the width of the square building site.
	AuScSite bldsite;
	bldsite.SetGrade(AuScSite::Level);
	bldsite.SetGrass(0);	// No grass.
	bldsite.SetPavement(WORLDTEX_CobblestoneLight);
	bldsite.SetRocks(0);
	bldsite.SetShrubs(0);
	bldsite.SetTrees(0);
	AuPointArray bldpts;
	bldpts.append(AuPoint(-bldWid2, -bldWid2));
	bldpts.append(AuPoint(-bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, -bldWid2));
	bldsite.MakeSite(bldpts, m_pWnd->DesignPlane());
	bldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!bldsite.IsValid())
		return;

	// Make the yard around the building site.
	double yardWid2 = 13 * FeetPerMeter();	// 1/2 the width of the square yard site.
	AuScSite yardsite;
	yardsite.SetGrade(AuScSite::Level);	// Level but we will fade it off to the edges.
	yardsite.SetGrass(3);	// Lawn
	yardsite.SetPavement(WORLDTEX_ConstructionZone);	// No pavement
	yardsite.SetTerrainBlend(1);	// Roll
	yardsite.SetRocks(0);
	yardsite.SetShrubs(0);
	yardsite.SetTrees(1);
	AuPointArray yardpts;
	yardpts.append(AuPoint(-yardWid2, -yardWid2));
	yardpts.append(AuPoint(-yardWid2, yardWid2));
	yardpts.append(AuPoint(yardWid2, yardWid2));
	yardpts.append(AuPoint(yardWid2, -yardWid2));
	yardpts.append(AuPoint(-yardWid2, -yardWid2));
	yardpts.append(AuPoint(-bldWid2, -bldWid2));
	yardpts.append(AuPoint(bldWid2, -bldWid2));
	yardpts.append(AuPoint(bldWid2, bldWid2));
	yardpts.append(AuPoint(-bldWid2, bldWid2));
	yardpts.append(AuPoint(-bldWid2, -bldWid2));
	yardsite.MakeSite(yardpts, m_pWnd->DesignPlane());
	yardsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());

	// Add red glow to the edges of the yard site, to make the terrain leveling effect fade off. The terrain level effect drops off
	// as the red glow gets brighter.
	AuPointArray glowpts;
	glowpts.append(yardpts[0]);
	glowpts.append(yardpts[1]);
	glowpts.append(yardpts[2]);
	glowpts.append(yardpts[3]);
	yardsite.GlowVertexLocs(glowpts, QColor(255,0,0));
	if(!yardsite.IsValid())
		return;

	// Move everything so the insertion point is at the front edge.
	AuMatrix4x4 m;
	m.Translate(0,-yardWid2,0);
	bldsite.Transform(m);
	yardsite.Transform(m);

	// Add the parts.
	auApp->DesignerPartAdd(bldsite);
	auApp->DesignerPartAdd(yardsite);
}

void DdBdSite::MakeLotShore()
{
	// Make the level building site.
	double bldWid2 = 23 * FeetPerMeter();	// 1/2 the width of the square building site.
	double inset = 4 * FeetPerMeter();
	AuScSite bldsite;
	bldsite.SetGrade(AuScSite::Level);
	bldsite.SetGrass(0);	// No grass.
	bldsite.SetRocks(0);
	bldsite.SetShrubs(0);
	bldsite.SetTrees(0);
	bldsite.SetPavement(WORLDTEX_ConcreteHeavy);
	AuPointArray bldpts;
	bldpts.append(AuPoint(-bldWid2, -bldWid2));
	bldpts.append(AuPoint(-bldWid2, inset));
	bldpts.append(AuPoint(bldWid2, inset));
	bldpts.append(AuPoint(bldWid2, -bldWid2));
	bldsite.MakeSite(bldpts, m_pWnd->DesignPlane());
	bldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!bldsite.IsValid())
		return;

	double harborDepth = 10 * FeetPerMeter();
	AuScSite harborsite;
	harborsite.SetGrade(AuScSite::Cut);
	harborsite.SetGrass(0);	// No grass.
	harborsite.SetPavement(WORLDTEX_ConstructionZone);	// No pavement
	harborsite.SetRocks(1);
	harborsite.SetShrubs(0);
	harborsite.SetTrees(0);
	AuPointArray harborpts;
	harborpts.append(AuPoint(-bldWid2, inset, -harborDepth));
	harborpts.append(AuPoint(-bldWid2, bldWid2, -harborDepth));
	harborpts.append(AuPoint(bldWid2, bldWid2, -harborDepth));
	harborpts.append(AuPoint(bldWid2, inset, -harborDepth));
	harborsite.MakeSite(harborpts, m_pWnd->DesignPlane());
	harborsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!harborsite.IsValid())
		return;

	double blendWid = 5 * FeetPerMeter();
	AuScSite blendsite;
	blendsite.SetGrade(AuScSite::Cut);
	blendsite.SetGrass(0);	// No grass.
	blendsite.SetPavement(WORLDTEX_ConstructionZone);	// No pavement
	blendsite.SetTerrainBlend(1);	// Roll
	blendsite.SetRocks(1);
	blendsite.SetShrubs(0);
	blendsite.SetTrees(1);
	AuPointArray blendpts;
	blendpts.append(harborpts[0] + AuPoint(-blendWid, blendWid));
	blendpts.append(harborpts[1] + AuPoint(-blendWid, blendWid));
	blendpts.append(harborpts[2] + AuPoint(blendWid, blendWid));
	blendpts.append(harborpts[3] + AuPoint(blendWid, blendWid));
	blendpts.append(harborpts[3]);
	blendpts.append(harborpts[2]);
	blendpts.append(harborpts[1]);
	blendpts.append(harborpts[0]);
	blendsite.MakeSite(blendpts, m_pWnd->DesignPlane());
	blendsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());

	// Add red glow to the edges of the yard site, to make the terrain leveling effect fade off. The terrain level effect drops off
	// as the red glow gets brighter.
	AuPointArray glowpts;
	glowpts.append(harborpts[0]);
	glowpts.append(harborpts[1]);
	glowpts.append(harborpts[2]);
	glowpts.append(harborpts[3]);
	blendsite.GlowVertexLocs(glowpts, QColor(255,0,0));
	if(!blendsite.IsValid())
		return;

	// Move everything so the insertion point is at the back edge, which is the land side.
	AuMatrix4x4 m;
	m.Translate(0,bldWid2,0);
	bldsite.Transform(m);
	harborsite.Transform(m);
	blendsite.Transform(m);

	// Add the parts.
	auApp->DesignerPartAdd(bldsite);
	auApp->DesignerPartAdd(harborsite);
	auApp->DesignerPartAdd(blendsite);
}

void DdBdSite::MakeLotMine()
{
	// Make the level building site.
	double bldWid2 = 10 * FeetPerMeter();	// 1/2 the width of the square building site.
	AuScSite bldsite;
	bldsite.SetGrade(AuScSite::Level);
	bldsite.SetGrass(0);	// No grass.
	bldsite.SetPavement(WORLDTEX_CobblestoneLight);
	bldsite.SetRocks(0);
	bldsite.SetShrubs(0);
	bldsite.SetTrees(0);
	AuPointArray bldpts;
	bldpts.append(AuPoint(-bldWid2, -bldWid2));
	bldpts.append(AuPoint(-bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, bldWid2));
	bldpts.append(AuPoint(bldWid2, -bldWid2));
	bldsite.MakeSite(bldpts, m_pWnd->DesignPlane());
	bldsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	if(!bldsite.IsValid())
		return;

	// Make the yard around the building site.
	double yardWid2 = 20 * FeetPerMeter();	// 1/2 the width of the square yard site.
	AuScSite yardsite;
	yardsite.SetGrade(AuScSite::Level);	// Level but we will fade it off to the edges.
	yardsite.SetGrass(0);	// No grass.
	yardsite.SetPavement(WORLDTEX_RockTan);
	yardsite.SetTerrainBlend(1);	// Roll
	yardsite.SetRocks(1);
	yardsite.SetShrubs(0);
	yardsite.SetTrees(0);
	AuPointArray yardpts;
	yardpts.append(AuPoint(-yardWid2, -yardWid2));
	yardpts.append(AuPoint(-yardWid2, yardWid2));
	yardpts.append(AuPoint(yardWid2, yardWid2));
	yardpts.append(AuPoint(yardWid2, -yardWid2));
	yardpts.append(AuPoint(-yardWid2, -yardWid2));
	yardpts.append(AuPoint(-bldWid2, -bldWid2));
	yardpts.append(AuPoint(bldWid2, -bldWid2));
	yardpts.append(AuPoint(bldWid2, bldWid2));
	yardpts.append(AuPoint(-bldWid2, bldWid2));
	yardpts.append(AuPoint(-bldWid2, -bldWid2));
	yardsite.MakeSite(yardpts, m_pWnd->DesignPlane());
	yardsite.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());

	// Add red glow to the edges of the yard site, to make the terrain leveling effect fade off. The terrain level effect drops off
	// as the red glow gets brighter.
	AuPointArray glowpts;
	glowpts.append(yardpts[0]);
	glowpts.append(yardpts[1]);
	glowpts.append(yardpts[2]);
	glowpts.append(yardpts[3]);
	yardsite.GlowVertexLocs(glowpts, QColor(255,0,0));
	if(!yardsite.IsValid())
		return;

	// Move everything so the insertion point is at the front edge.
	AuMatrix4x4 m;
	m.Translate(0,-yardWid2,0);
	bldsite.Transform(m);
	yardsite.Transform(m);

	// Add the parts.
	auApp->DesignerPartAdd(bldsite);
	auApp->DesignerPartAdd(yardsite);
}

