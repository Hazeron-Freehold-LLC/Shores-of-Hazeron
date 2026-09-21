//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerbar.cpp,v 1.9 2022/02/19 18:25:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignerbar.h"
//
#include "auclientglobal.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignerview.h"
#include "dadesignbase.h"
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>

AuDesignerBar::AuDesignerBar(QWidget* parent) : QToolBar(parent)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = pFrame->ViewWidget();

	setObjectName("DesignerBar");
	setWindowTitle(tr("Designer Input Bar"));

	int dockSides = g_Au.Settings().readNumEntry("Dock/DesignerBarAreas", (int)allowedAreas());
	setAllowedAreas((Qt::ToolBarAreas)dockSides);

	m_InputEdit = new QLineEdit(this);
	m_InputEdit->setToolTip(tr("<html>"
				"<b>Coordinate Input</b><br>"
				"Coordinates can be entered here when the current design command requests a location. "
				"A location can also be entered by clicking with the mouse cursor in the view window.<br>"
				"Selected parts can be moved or copied when no design command is requesting a location.<br"
				"<i>Tips:</i><br>"
				"%1 Characters typed in the view window are sent to this control when a design command is expecting a location.<br>"
				"%1 Cartesian coordinates are entered as <b>X,Y,Z</b> values, "
				"which are positive or negative offsets along the <b>X</b>, <b>Y</b> and <b>Z</b> axes.<br>"
				"%1 Polar coordinates are entered as <b>pD,A,I</b> values: <b>D</b> is distance from the origin; "
				"<b>A</b> is angle around the Z axis, positive counterclockwise and negative clockwise; "
				"<b>I</b> is angle of inclination off the XY plane, positive above and negative below.<br>"
				"%1 Coordinates are relative to the location and orientation of the grid unless specified otherwise.<br>"
				"%1 Coordinates prefixed with <b>a</b> are <i>absolute</i>. "
				"Absolute coordinates are relative to the native coordinate system of the design, "
				"ignoring the grid. e.g. <i>a1.25,2.3,-4.5</i> or <i>ap2.5,-30,10</i>.<br>"
				"%1 Distances are entered in units depending upon the <i>unit of measure</i> setting.<br>"
				"%1 Angles are entered in degrees.<br>"
				"%1 Omitted values are 0. e.g. <i>1,,2</i> is the same as <i>1,0,2</i>; "
				"<i>p2</i> is the same as <i>p2,0,0</i>; <i>a,3</i> is the same as <i>a0,3,0</i>.<br>"
				"%1 <b>Move parts</b> by entering a coordinate here, when no command is waiting for input. "
				"The parts that are selected will be moved using the vector entered.<br>"
				"%1 <b>Copy parts</b> by entering a coordinate here while holding the <i>Ctrl</i> key, "
				"when no command is waiting for input. "
				"The parts that are selected will be copied and moved using the vector entered.<br>"
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_InputEdit->setMinimumWidth(150);
	m_InputEdit->setMaximumWidth(200);

	m_InputPrompt = new QLabel(this);
	QSizePolicy spol = m_InputPrompt->sizePolicy();
	spol.setHorizontalPolicy(QSizePolicy::Expanding);
	m_InputPrompt->setSizePolicy(spol);

	m_DesignProps = new QAction(QIcon(":data/CommManifest.png"), tr("Design Properties"), this);
	m_DesignProps->setToolTip(tr("<html>"
				"<b>Design Properties</b><br>"
				"Adjusts properties of the design. "
				"Reports analysis of design. "
				"</html>"));
	connect(m_DesignProps, SIGNAL(triggered()), view, SLOT(DesignerProperties()));

	m_RefreshObstructions = new QAction(QIcon(":data/Cycle.png"), tr("Refresh Obstructions"), this);
	m_RefreshObstructions->setToolTip(tr("<html>"
				"<b>Refresh Obstructions</b><br>"
				"Refreshes obstructions model. "
				"Most changes to the design invalidate the obstructions model. "
				"The obstructions model is needed for solids to obstruct movement. "
				"It is also needed by the <i>Design Properties</i> to report the design analysis. "
				"</html>"));
	connect(m_RefreshObstructions, SIGNAL(triggered()), auApp, SLOT(DesignerRefreshObstructions()));

	m_SolidsBlockPath = new QAction(QIcon(":data/ScMoveBlock.png"), tr("Solids Obstruct Movement"), this);
	m_SolidsBlockPath->setToolTip(tr("<html>"
				"<b>Solids Obstruct Movement</b><br>"
				"Enables solid parts of the model to block your movement. "
				"The obstructions model is needed for solids to obstruct movement. "
				"<br>"
				"%1 When checked, solid parts block movement of your avatar.<br>"
				"%1 When unchecked, solid parts do not block movement of your avatar."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_SolidsBlockPath->setCheckable(true);
	m_SolidsBlockPath->setChecked(g_Au.DesignerBlockPaths());
	connect(m_SolidsBlockPath, SIGNAL(triggered()), SLOT(SolidsBlockPath()));

	m_GravityToggle = new QAction(QIcon(":data/dp_Gravity.png"), tr("Gravity On/Off"), this);
	m_GravityToggle->setToolTip(tr("<html>"
				"<b>Gravity</b><br>"
				"Enables gravity inside room voids. "
				"<br>"
				"%1 When checked, your avatar is affected by gravity inside room voids.<br>"
				"%1 When unchecked, gravity does not affect your avatar."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_GravityToggle->setCheckable(true);
	m_GravityToggle->setChecked(g_Au.DesignerGravityOn());
	connect(m_GravityToggle, SIGNAL(triggered()), SLOT(GravityToggle()));

	m_PreviewToggle = new QAction(QIcon(":data/MsgSolicitSales.png"), tr("Preview On/Off"), this);
	m_PreviewToggle->setToolTip(tr("<html>"
				"<b>Preview...</b><br>"
				"Enables preview of design as it will exist and operate in the game environment. "
				"All editing operations are disabled while previewing a design, for everyone in the design studio. "
				"<br>"
				"%1 When checked, a preview of the design is displayed for inspection and testing. "
				"<br>"
				"%1 When unchecked, the parts of the design are displayed for editing."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_PreviewToggle->setCheckable(true);
	m_PreviewToggle->setChecked(false);
	connect(m_PreviewToggle, SIGNAL(triggered()), SLOT(PreviewToggle()));

	m_PreviewSettings = new QAction(QIcon(":data/c_Sunlight.png"), tr("Preview Sun Position..."), this);
	m_PreviewSettings->setToolTip(tr("<html>"
				"<b>Preview Sun Position...</b><br>"
				"Changes the sun position in the design preview. "
				"</html>"));
	connect(m_PreviewSettings, SIGNAL(triggered()), SLOT(PreviewSettings()));

	m_PreviewBd = new QAction(QIcon(":data/ConstructionSign.png"), tr("Construct Building..."), this);
	m_PreviewBd->setToolTip(tr("<html>"
				"<b>Construct Building...</b><br>"
				"Constructs the building design on the terrain. "
				"</html>"));
	connect(m_PreviewBd, SIGNAL(triggered()), SLOT(PreviewBd()));

	m_PreviewPeople = new QAction(QIcon(":data/CommCrew.png"), tr("People"), this);
	m_PreviewPeople->setToolTip(tr("<html>"
				"<p>"
				"<b>Preview People</b><br>"
				"Loads crew, troops, and passengers to fill non-officer berths aboard the spacecraft in the preview. "
				"Loads workers and troops to fill posts at buildings in the preview. "
				"</p>"
				"<p>"
				"People can be selected and given simple movement orders. "
				"This shows if NPCs can find their way through the blueprint. "
				"</p>"
				"</html>"));
	connect(m_PreviewPeople, SIGNAL(triggered()), SLOT(PreviewPeople()));

	addAction(m_DesignProps);
	addAction(m_RefreshObstructions);
	addAction(m_SolidsBlockPath);
	addAction(m_GravityToggle);
	addAction(m_PreviewToggle);
	addAction(m_PreviewSettings);
	addAction(m_PreviewBd);
	addAction(m_PreviewPeople);
	addWidget(m_InputEdit);
	addWidget(m_InputPrompt);
	setIconSize(QSize(16,16));
}

AuDesignerBar::~AuDesignerBar()
{
}

void AuDesignerBar::changeEvent(QEvent* event)
{
	QToolBar::changeEvent(event);
	if(event->type() == QEvent::ActivationChange)
	{
		// Configure transparency state of this widget.
		if(isFloating())
		{
			float opacity = (float)g_Au.WindowOpacity(isActiveWindow())/100.f;
			setWindowOpacity(opacity);
		}
		else
		{
			// This appears to have no effect when docked but here goes anyway.
			setWindowOpacity(1);
		}
	}
}

// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
bool AuDesignerBar::KeyReceived(QKeyEvent* e)
{
	switch(e->key())
	{
	default:
		return false;
	case Qt::Key_A:						// Absolute coordinate
	case Qt::Key_P:						// Polar coordinate
	case Qt::Key_R:						// Relative to grid.
		{
			//qDebug() << "AuDesignerBar::KeyReceived apr" << m_InputEdit->selectionStart();
			// Delete selected text, if any.
			if(m_InputEdit->selectionStart() >= 0)
				m_InputEdit->del();

			// Append the character to the text.
			QString text = m_InputEdit->text();
			text.append(QChar(e->key()).toLower());
			m_InputEdit->setText(text);
		}
		break;
	case Qt::Key_Comma:
	case Qt::Key_Minus:
	case Qt::Key_Period:
	case Qt::Key_Space:
	case Qt::Key_0:
	case Qt::Key_1:
	case Qt::Key_2:
	case Qt::Key_3:
	case Qt::Key_4:
	case Qt::Key_5:
	case Qt::Key_6:
	case Qt::Key_7:
	case Qt::Key_8:
	case Qt::Key_9:
		{
			//qDebug() << "AuDesignerBar::KeyReceived coord" << m_InputEdit->selectionStart();
			// Delete selected text, if any.
			if(m_InputEdit->selectionStart() >= 0)
				m_InputEdit->del();

			// Append the character to the text.
			QString text = m_InputEdit->text();
			text.append(e->key());
			m_InputEdit->setText(text);
		}
		break;
	case Qt::Key_Enter:
	case Qt::Key_Return:
		{
			//qDebug() << "AuDesignerBar::KeyReceived enter" << m_InputEdit->text();
			QString text = m_InputEdit->text();
			if(!text.isEmpty())
			{
				emit CoordinateEntered(text, e->modifiers());

				// Select the text so typing a new coordinate will overwrite it.
				// Leaving the coordinate in the box enable the user to repeat the coordinate entry easily.
				m_InputEdit->selectAll();
			}
		}
		break;
	case Qt::Key_Backspace:
		{
			// Move cursor to the end before backspacing.
			//qDebug() << "AuDesignerBar::KeyReceived backspace" << m_InputEdit->text();
			if(m_InputEdit->selectionStart() < 0)
				m_InputEdit->setCursorPosition(m_InputEdit->text().length());
			m_InputEdit->backspace();
		}
		break;
	}
	return true;
}

void AuDesignerBar::SetInputPrompt(QString prompt)
{
	//qDebug() << "AuDesignerBar::SetInputPrompt" << prompt;
	if(m_InputPrompt->text() != prompt)
		m_InputPrompt->setText(prompt);
}

void AuDesignerBar::keyPressEvent(QKeyEvent* e)
{
	//qDebug() << "AuDesignerBar::keyPressEvent";
	// We must process Enter and Return to accept the contents of the input edit control.
	if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
		KeyReceived(e);
}

void AuDesignerBar::EnableControls()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = pFrame->ViewWidget();
	bool preview = view->IsDesignPreview();
	DaDesignBase::DESIGNMODE designMode = (DaDesignBase::DESIGNMODE)view->DesignMode();
	bool bdpreviewok = preview && designMode == DaDesignBase::DESIGN_Building;	// Ok to place building design on preview terrain.
	bool dok = !preview && designMode > DaDesignBase::DESIGN_Unknown;			// True if mode is valid for designing things.
	m_DesignProps->setEnabled(dok);
	m_GravityToggle->setEnabled(dok);
	m_InputEdit->setEnabled(dok);
	m_PreviewBd->setEnabled(bdpreviewok);
	m_PreviewPeople->setEnabled(preview);
	m_PreviewSettings->setEnabled(preview);
	m_RefreshObstructions->setEnabled(dok);
	m_SolidsBlockPath->setEnabled(dok);

	// Hide/show/disable actions that depend on design mode.
	// WARNING: You can hide menu items but you cannot hide menus. The menu system goes haywire.
	// For consistency, hide menu items according to design mode. Don't hide according to sub mode conditions; disable instead.
	// Menu items have already been enabled|disabled. Don't enable controls here based on design mode; only disable them.
	switch(designMode)
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		break;
	case DaDesignBase::DESIGN_Spacecraft:
		break;
	case DaDesignBase::DESIGN_Building:
		break;
	case DaDesignBase::DESIGN_Item:
		m_GravityToggle->setEnabled(false);
		break;
	//case DaDesignBase::DESIGN_Vehicle:
		//break;
	}
}

void AuDesignerBar::SolidsBlockPath()
{
	auDesignerApp->DesignerFrame()->DesignerSetSolidsBlockPath(m_SolidsBlockPath->isChecked());
}

void AuDesignerBar::SetSolidsBlockPath(bool block)
{
	if(m_SolidsBlockPath->isChecked() != block)
	{
		m_SolidsBlockPath->blockSignals(true);
		m_SolidsBlockPath->setChecked(block);
		m_SolidsBlockPath->blockSignals(false);
	}
}

void AuDesignerBar::GravityToggle()
{
	auDesignerApp->DesignerFrame()->DesignerSetGravityOn(m_GravityToggle->isChecked());
}

void AuDesignerBar::SetGravity(bool g)
{
	if(m_GravityToggle->isChecked() != g)
	{
		m_GravityToggle->blockSignals(true);
		m_GravityToggle->setChecked(g);
		m_GravityToggle->blockSignals(false);
	}
}

void AuDesignerBar::PreviewSettings()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	pFrame->ViewWidget()->DesignerPreviewSettings();
}

void AuDesignerBar::PreviewBd()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	pFrame->ViewWidget()->DesignerPreviewBd();
}

void AuDesignerBar::PreviewPeople()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	pFrame->ViewWidget()->DesignerPreviewPeople();
}

void AuDesignerBar::PreviewToggle()
{
	auDesignerApp->DesignerFrame()->DesignerSetPreview(m_PreviewToggle->isChecked());
}

void AuDesignerBar::SetPreview(bool p)
{
	if(m_PreviewToggle->isChecked() != p)
	{
		m_PreviewToggle->blockSignals(true);
		m_PreviewToggle->setChecked(p);
		m_PreviewToggle->blockSignals(false);
	}
}
