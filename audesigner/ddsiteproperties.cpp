//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsiteproperties.cpp,v 1.6 2022/02/19 18:27:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsiteproperties.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "ausitepropertiesdlg.h"
#include "audesignerview.h"
#include "auscparts.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdSiteProperties::DdSiteProperties(AuDesignerView* view) : DdCommand(view)
{
}

DdSiteProperties::~DdSiteProperties()
{

}

QString DdSiteProperties::WikiTopic() const
{
	return "Designer_Menu#Site";
}

void DdSiteProperties::Start()
{
	DdCommand::Start();

	// Configure defaults.
	AuScSite def;
	quint8 grade = def.Grade();
	quint8 grass = def.Grass();
	quint8 pavement = def.Pavement();
	quint8 rocks = def.Rocks();
	quint8 shrubs = def.Shrubs();
	quint8 terBlend = def.TerrainBlend();
	quint8 trees = def.Trees();

	// Get current properties from selection.
	DaPerson* person;
	m_pWnd->LockSceneAtom(&person);
	if(person)
	{
		const DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartSelectionSiteProperties(m_pWnd->DesignerPartSelection(), grade, grass, pavement, rocks, shrubs, terBlend,
					trees);
	}
	m_pWnd->UnlockSceneAtom();

	AuSitePropertiesDlg dlg(grade, grass, pavement, rocks, shrubs, terBlend, trees, m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		if(SetProperties(dlg.Grade(), dlg.Grass(), dlg.Pavement(), dlg.Rocks(), dlg.Shrubs(), dlg.Blend(), dlg.Trees()))
			Terminate();
	}
	else
		Terminate();
}

void DdSiteProperties::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Configure site properties. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdSiteProperties::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	return loc;
}

// Set appropriate device cursor.
void DdSiteProperties::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdSiteProperties::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;
	return false;
}

// Render "cursor" objects in the scene.
void DdSiteProperties::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdSiteProperties::SetProperties(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend,
		quint8 trees)
{
	auApp->DesignerPartsSetSiteProperties(m_pWnd->DesignerPartSelection(), grade, grass, pavement, rocks, shrubs, terBlend, trees);
	return true;
}
