//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscsketch.cpp,v 1.9 2022/02/19 18:27:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscsketch.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>

DdScSketch::DdScSketch(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
}

QString DdScSketch::WikiTopic() const
{
	return "Designer_Menu#Sketch";
}

void DdScSketch::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Corner of the sketch. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Opposite corner of the sketch. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScSketch::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		// Slam the point to the plane.
		m_Corner = m_pWnd->DesignPlane().UnProject(loc);
		m_Corner.z = 0;
		m_Corner = m_pWnd->DesignPlane().Project(m_Corner);

		m_GetPoint = 1;
		UpdatePrompt();
		return m_Corner;
	}
	else
	{
		AuPoint cloc = m_pWnd->DesignPlane().UnProject(loc);
		cloc.z = 0;
		cloc = m_pWnd->DesignPlane().Project(cloc);

		// Ask for a picture file name.
		// Load last known file folder.
		QString lastFile = g_Au.Settings().readEntry("/Settings/LastSketch", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();
		QFileInfo lastFileInfo(lastFile);
		if(!lastFileInfo.exists())
			lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

		QString file = QFileDialog::getOpenFileName(m_pWnd, QObject::tr("Select Sketch Image"), lastFile, "", 0,
				g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
		if(!file.isEmpty())
		{
			// Store last known file folder.
			g_Au.Settings().writeEntry("/Settings/LastSketch", file);

			QImage imSketch;
			if(imSketch.load(file))
			{
				if(MakeIt(cloc, file))
					Terminate();
			}
			else
			{
				QMessageBox::critical(m_pWnd, g_Au.ApplicationName(),
						QObject::tr("<html>"
							"<p>"
							"The image could not be loaded. "
							"Many image formats are supported but some JPEG sub-formats give us trouble. "
							"</p>"

							"<p>"
							"If the image can be loaded with a paint program, "
							"try saving it as a different format of image such as PNG or BMP. "
							"Then try again. "
							"</p>"
							"</html>"),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			}
		}

		return cloc;
	}
}

void DdScSketch::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	ts = ts.Inverse();

	AuMatrix4x4 pmv = projection * model * ts;

	if(m_GetPoint == 1)
	{
		// Slam the point to the plane.
		AuPoint cloc = m_pWnd->DesignPlane().UnProject(m_Corner);
		loc = m_pWnd->DesignPlane().UnProject(loc);
		loc.z = 0;

		AuPoint ll = loc.Min(cloc);
		AuPoint ur = loc.Max(cloc);
		AuPoint ul(ll.x, ur.y, 0);
		AuPoint lr(ur.x, ll.y, 0);

		AuPointArray verts;
		verts.append(m_pWnd->DesignPlane().Project(ll));
		verts.append(m_pWnd->DesignPlane().Project(ul));
		verts.append(m_pWnd->DesignPlane().Project(ur));
		verts.append(m_pWnd->DesignPlane().Project(lr));

		AuGLVAO vao;
		vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,1,1);

			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();

			// Restore rendering states.
			re.GLSL().LineShader().Deselect();
			re.glDisable(GL_LINE_SMOOTH);
			re.glEnable(GL_DEPTH_TEST);
		}
	}

	// Draw the blip in pixel space.
	DrawBlip(pmv, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdScSketch::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_C:	// Restart
		m_GetPoint = 0;
		UpdatePrompt();
		break;

	default:
		break;
	}

	return false;
}

bool DdScSketch::MakeIt(const AuPoint& corner, const QString& fileName)
{
	if(fileName.isEmpty())
		return false;

	AuScSketch sketch;
	sketch.MakeScreen(m_Corner, corner, m_pWnd->DesignPlane());
	sketch.SetFilePath(fileName);
	if(!sketch.IsValid())
		return false;

	sketch.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(sketch);
	return true;
}
