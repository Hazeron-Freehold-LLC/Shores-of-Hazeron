//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscspherelatlong.cpp,v 1.7 2022/02/19 18:27:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscspherelatlong.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auscspherelatlongdlg.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScSphereLatLong::DdScSphereLatLong(quint8 partType, AuDesignerView* view) : DdCommand(view)
{
	m_PartType = partType;
}

DdScSphereLatLong::~DdScSphereLatLong()
{

}

QString DdScSphereLatLong::WikiTopic() const
{
	return "Designer#Sphere_Lon.2FLat";
}

void DdScSphereLatLong::UpdatePrompt() const
{
	QString prompt;
	if(m_Points.empty())
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Sphere center. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Sphere radius. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScSphereLatLong::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Points.empty())
	{
		m_Points.append(ploc);
		UpdatePrompt();
	}
	else
	{
		if(loc != m_Points.first())
		{
			AuScSphereLatLongDlg dlg(AuScMesh::TypeStatic(), m_pWnd);
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				double radius = m_Points.first().Distance(loc);
				if(MakeSphere(m_Points.first(), radius, dlg.LongitudeSteps(), dlg.LatitudeSteps()))
				{
					m_Points.clear();
					UpdatePrompt();
				}
			}
		}
	}
	return ploc;
}

// Set appropriate device cursor.
void DdScSphereLatLong::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScSphereLatLong::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_C:	// Cancel
		if(!m_Points.empty())
		{
			m_Points.clear();
			UpdatePrompt();
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

// Render "cursor" objects in the scene.
void DdScSphereLatLong::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	// Use line shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * ts);
	re.GLSL().LineShader().SetColor(.7f, 1, .7f);

	// Stairs conform exactly to the grid so the dimensions change between diagonal and straight.
	re.glDisable(GL_DEPTH_TEST);

	if(!m_Points.empty())
	{
		// Make a VAO of the vertex locations.
		AuPointArray pts(m_Points);
		pts.append(loc);
		AuGLVAO vao;
		vao.CreateForLines(re, pts, false, GL_STREAM_DRAW);
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
	}

	re.glEnable(GL_DEPTH_TEST);

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScSphereLatLong::MakeSphere(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps)
{
	if(radius <= 0)
		return false;
	if(lonSteps < 3)
		return false;
	if(latSteps < 2)
		return false;
	if(m_Points.empty())
		return false;

	if(m_PartType == AuScHull::TypeStatic())
	{
		AuScHull mesh;
		mesh.MakeSphereLongLat(m_pWnd->DesignPlane().UnProject(center), radius, lonSteps, latSteps, m_pWnd->DesignPlane());
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScRoom::TypeStatic())
	{
		AuScRoom mesh;
		mesh.MakeSphereLongLat(m_pWnd->DesignPlane().UnProject(center), radius, lonSteps, latSteps, m_pWnd->DesignPlane());
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScLift::TypeStatic())
	{
		AuScLift mesh;
		mesh.MakeSphereLongLat(m_pWnd->DesignPlane().UnProject(center), radius, lonSteps, latSteps, m_pWnd->DesignPlane());
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScLandingGear::TypeStatic())
	{
		AuScLandingGear mesh;
		mesh.MakeSphereLongLat(m_pWnd->DesignPlane().UnProject(center), radius, lonSteps, latSteps, m_pWnd->DesignPlane());
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else
	{
		AuScMesh mesh;
		mesh.MakeSphereLongLat(m_pWnd->DesignPlane().UnProject(center), radius, lonSteps, latSteps, m_pWnd->DesignPlane());
		if(!mesh.IsValid())
			return false;

		quint16 partId = m_pWnd->DesignerObjectModeTargetPart();
		if(partId)
		{
			// Add the faces of the mesh to the face mode target mesh.
			AuScFaceList faces;
			mesh.Faces(faces);
			auApp->DesignerPartAddFaces(partId, faces);
		}
		else
		{
			// Create a new mesh.
			mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
			auApp->DesignerPartAdd(mesh);
		}
		return true;
	}
}
