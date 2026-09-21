//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausettingsdialog.cpp,v 1.4 2022/02/19 18:26:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausettingsdialog.h"
//
#include "ausetcomm.h"
#include "ausetcontrols.h"
#include "ausetdesigner.h"
#include "ausetoverlay.h"
#include "ausetprogram.h"
#include "ausetrender.h"
#include "ausetsound.h"
#include "ausetwindows.h"
#include <auglobal.h>
#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QTabBar>

AuSettingsDialog::AuSettingsDialog(bool showF12, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Settings.png"));
	if(showF12)
		setAuDlgTitle("<b>Settings</b> (Shift+F12)");
	else
		setAuDlgTitle("<b>Settings</b>");
	SetWikiTopic("Settings");

	// Create base widgets needed for tabbed pages structure.
	m_Pages = new QStackedLayout;
	m_Tabs = new QTabBar(this);
	connect(m_Tabs, SIGNAL(currentChanged(int)), m_Pages, SLOT(setCurrentIndex(int)));

	// Add pages and tabs.
	int itab;

	// Overlay
	AuSetOverlay* pageOverlay = new AuSetOverlay(this);
	m_Pages->insertWidget(m_Pages->count(), pageOverlay);
	QIcon iconOverlay(":data/OrderTV.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconOverlay, "Overlay");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Overlay</b><br>"
			"Overlay settings affect the things that are overlayed onto the scene."
			"</html>"));

	// Comm
	AuSetComm* pageComm = new AuSetComm(this);
	m_Pages->insertWidget(m_Pages->count(), pageComm);
	QIcon iconComm(":data/Comm.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconComm, "Comm");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Comm</b><br>"
			"Communication settings affect comm channel messages."
			"</html>"));
	connect(pageComm, SIGNAL(MessageLimitChanged(int)), SIGNAL(MessageLimitChanged(int)));	// Propagate the signal up.

	// Controls
	AuSetControls* pageControls = new AuSetControls(this);
	m_Pages->insertWidget(m_Pages->count(), pageControls);
	QIcon iconControls(":data/CommDetonate.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconControls, "Controls");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Controls</b><br>"
			"Control settings affect input devices and bindings for keyboard, mouse and joystick actions."
			"</html>"));
	connect(pageControls, SIGNAL(ControlsChanged()), SIGNAL(ControlsChanged()));	// Propagate the signal up.

	// Designer
	AuSetDesigner* pageDesigner = new AuSetDesigner(this);
	m_Pages->insertWidget(m_Pages->count(), pageDesigner);
	QIcon iconDesigner(":data/SoHDesignerIcon.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconDesigner, "Designer");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Designer</b><br>"
			"Designer settings affect the designer."
			"</html>"));

	// Scene
	m_PageScene = new AuSetRender(showF12, this);
	m_Pages->insertWidget(m_Pages->count(), m_PageScene);
	QIcon iconScene(":data/WorldPlanet.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconScene, "Scene");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Scene</b><br>"
			"Scene settings affect things that are drawn in the scene."
			"</html>"));

	// Sound
	AuSetSound* pageSound = new AuSetSound(this);
	m_Pages->insertWidget(m_Pages->count(), pageSound);
	QIcon iconSound(":data/CommHail.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconSound, "Sound");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Sound</b><br>"
			"Sound settings affect things that you hear."
			"</html>"));
	connect(pageSound, SIGNAL(SoundGainChanged(int)), SIGNAL(SoundGainChanged(int)));
	connect(pageSound, SIGNAL(SoundOverrideChanged(int)), SIGNAL(SoundOverrideChanged(int)));

	// Windows
	AuSetWindows* pageWindows = new AuSetWindows(this);
	m_Pages->insertWidget(m_Pages->count(), pageWindows);
	QIcon iconWindows(":data/ToggleAll.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconWindows, "Windows");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Window</b><br>"
			"Window settings affect the appearance of user interface windows."
			"</html>"));

	// Program
	AuSetProgram* pageProgram = new AuSetProgram(this);
	m_Pages->insertWidget(m_Pages->count(), pageProgram);
	QIcon iconProgram(":data/Settings.png");
	itab = m_Tabs->insertTab(m_Tabs->count(), iconProgram, "Program");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Program</b><br>"
			"Program settings affect the way the program runs."
			"</html>"));

	// Layout the widgets.
	QGridLayout* lay = new QGridLayout();
	lay->addWidget(m_Tabs, 0, 0);
	lay->addLayout(m_Pages, 1, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuSettingsDialog::~AuSettingsDialog()
{
}

void AuSettingsDialog::keyPressEvent(QKeyEvent* event)
{
	// Don't close when Esc pressed.
	if(event->key() != Qt::Key_Escape)
		AuDialog::keyPressEvent(event);
}

void AuSettingsDialog::SetOpenGLInfo(int textureUnits, int uniformLocs)
{
	m_PageScene->SetOpenGLInfo(textureUnits, uniformLocs);
}
