//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentdialog.cpp,v 1.5 2022/02/19 18:03:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentdialog.h"
//
#include "auagentdesign.h"
#include "auagentempire.h"
#include "auagentencounters.h"
#include "auagentenvironment.h"
#include "auagentplayers.h"
#include "auagentpower.h"
#include "auagentship.h"
#include "auagentteleport.h"
#include <auglobal.h>
#include <autabcolumn.h>
#include <dgdata.h>
#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QStackedLayout>

AuAgentDialog::AuAgentDialog(QWidget* parent) : AuDialog(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	bool isArchitect = gd.ClientPlayerIsArchitect();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	QString title;
	QIcon icon;
	if(isArchitect)
	{
		icon = QIcon(":data/Architect.png");
		title = "Architect";
	}
	else if(isProgrammer)
	{
		icon = QIcon(":data/Programmer.png");
		title = "Programmer";
	}
	else if(isMinion)
	{
		icon = QIcon(":data/Minion.png");
		title = "Minion";
	}
	else
	{
		icon = QIcon(":data/Agent.png");
		title = "Agent";
	}
	setAuDlgIcon(icon);
	setAuDlgTitle(QString("<b>%1</b> (Shift+F11)").arg(title));

	// Create base widgets needed for tabbed pages structure.
	m_Pages = new QStackedLayout;
	m_Tabs = new AuTabColumn(this);
	connect(m_Tabs, SIGNAL(CurrentChanged(int)), m_Pages, SLOT(setCurrentIndex(int)));

	// Add pages and tabs.
	int itab;

	// Power
	m_PagePower = new AuAgentPower(this);
	m_Pages->insertWidget(m_Pages->count(), m_PagePower);
	itab = m_Tabs->AddTab(icon, title);
	m_Tabs->SetTabToolTip(itab, QString("<html><b>%1</b><br>"
			"Agent information and settings."
			"</html>").arg(title));

	if(!isMinion)
	{
		// Players
		m_PagePlayers = new AuAgentPlayers(this);
		m_Pages->insertWidget(m_Pages->count(), m_PagePlayers);
		QIcon iconRoster(":data/CommCrew.png");
		itab = m_Tabs->AddTab(iconRoster, tr("Players"));
		m_Tabs->SetTabToolTip(itab, QString("<html><b>Players Online</b><br>"
				"Avatars who are online right now."
				"</html>"));

		connect(m_PagePlayers, SIGNAL(AgentActionComplete()), SLOT(hide()));

		// Teleport
		m_PageTeleport = new AuAgentTeleport(this);
		m_Pages->insertWidget(m_Pages->count(), m_PageTeleport);
		QIcon iconTeleport(":data/MsgTransport.png");
		itab = m_Tabs->AddTab(iconTeleport, tr("Teleport"));
		m_Tabs->SetTabToolTip(itab, QString("<html><b>Teleport</b><br>"
				"Summon avater or go to avater or city."
				"</html>"));

		connect(m_PageTeleport, SIGNAL(AgentActionComplete()), SLOT(hide()));
	}
	else
	{
		m_PagePlayers = 0;
		m_PageTeleport = 0;
	}

	// Ship/vehicle
	m_PageShip = new AuAgentShip(this);
	m_Pages->insertWidget(m_Pages->count(), m_PageShip);
	QIcon iconShip(":data/c_Spacecraft.png");
	itab = m_Tabs->AddTab(iconShip, tr("Spacecraft"));
	m_Tabs->SetTabToolTip(itab, QString("<html><b>Spacecraft</b><br>"
			"Load crew or make changes to the agent's current spacecraft or vehicle."
			"</html>"));

	if(!isMinion)
	{
		// Design
		m_PageDesign = new AuAgentDesign(this);
		m_Pages->insertWidget(m_Pages->count(), m_PageDesign);
		QIcon iconDesign(":data/c_ScDesignDisk.png");
		itab = m_Tabs->AddTab(iconDesign, tr("Design"));
		m_Tabs->SetTabToolTip(itab, QString("<html><b>Design</b><br>"
				"Load a ship design or publicize designs."
				"</html>"));

		connect(m_PagePlayers, SIGNAL(AvatarSelected(AuId, QString)), m_PageTeleport, SLOT(AvatarSelected(AuId, QString)));

		// Empire
		m_PageEmpire = new AuAgentEmpire(this);
		m_Pages->insertWidget(m_Pages->count(), m_PageEmpire);
		QIcon iconEmpire(":data/CommGovernment.png");
		itab = m_Tabs->AddTab(iconEmpire, tr("Empire"));
		m_Tabs->SetTabToolTip(itab, QString("<html><b>Empire</b><br>"
				"Edit empires."
				"</html>"));

		connect(m_PageEmpire, SIGNAL(EmpireDataRequested()), m_PageTeleport, SLOT(EmpireDataInvalid()));
		connect(m_PageTeleport, SIGNAL(EmpireDataRequested()), m_PageEmpire, SLOT(EmpireDataInvalid()));

		// Encounters
		m_PageEncounters = new AuAgentEncounters(this);
		m_Pages->insertWidget(m_Pages->count(), m_PageEncounters);
		QIcon iconEncounters(":data/Rat.png");
		itab = m_Tabs->AddTab(iconEncounters, tr("Encounters"));
		if(isProgrammer)
			m_Tabs->SetTabToolTip(itab, QString("<html><b>Encounters</b><br>"
					"Creature and ship encounters."
					"</html>"));
		else
			m_Tabs->SetTabToolTip(itab, QString("<html><b>Encounters</b><br>"
					"Creature encounters."
					"</html>"));

		// Environment
		m_PageEnvironment = new AuAgentEnvironment(this);
		m_Pages->insertWidget(m_Pages->count(), m_PageEnvironment);
		QIcon iconEnvironment(":data/DNA.png");
		itab = m_Tabs->AddTab(iconEnvironment, tr("Environment"));
		m_Tabs->SetTabToolTip(itab, QString("<html><b>Environment</b><br>"
				"Environment query and modification."
				"</html>"));
	}
	else
	{
		m_PageDesign = 0;
		m_PageEmpire = 0;
		m_PageEncounters = 0;
		m_PageEnvironment = 0;
	}

	// Layout the widgets.
	QGridLayout* lay = new QGridLayout();
	lay->addWidget(m_Tabs, 0, 0);
	lay->addLayout(m_Pages, 0, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuAgentDialog::~AuAgentDialog()
{
}

void AuAgentDialog::keyPressEvent(QKeyEvent* event)
{
	// Don't close when Esc pressed.
	if(event->key() != Qt::Key_Escape)
		AuDialog::keyPressEvent(event);
}

void AuAgentDialog::SetGoToDestination(const AuId& id, const QString& name, int destType)
{
	if(m_PageTeleport)
		m_PageTeleport->SetGoToDestination(id, name, destType);
}

void AuAgentDialog::SceneAtomChanged()
{
	// agent power page needs to pick up agent bits
	if(m_PagePower->isVisible())
		m_PagePower->VisibilityRestored();
}

void AuAgentDialog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();

		if(m_PagePower->isVisible())
			m_PagePower->VisibilityRestored();
		if(m_PageEmpire && m_PageEmpire->isVisible())
			m_PageEmpire->VisibilityRestored();
		if(m_PageTeleport && m_PageTeleport->isVisible())
			m_PageTeleport->VisibilityRestored();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuAgentDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/Agent/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/Agent/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/Agent/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/Agent/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuAgentDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/Agent/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/Agent/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/Agent/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/Agent/SizeY",sz.height());
}

void AuAgentDialog::EmpireDataComplete()
{
	if(m_PageEmpire)
		m_PageEmpire->EmpireDataComplete();
	if(m_PageTeleport)
		m_PageTeleport->EmpireDataComplete();
}
