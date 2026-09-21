//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftpolicy.cpp,v 1.12 2022/05/04 19:59:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftpolicy.h"
//
#include "auapp.h"
#include "auchristendialog.h"
#include "auglobal.h"
#include "auframe.h"
#include "auroster.h"
#include "aushiptitle.h"
#include "auview.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>

void AuSpacecraftPolicyDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftPolicyUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftPolicy::AuSpacecraftPolicy(QWidget* parent) : QWidget(parent)
{
	m_PersonGalaxy = GALAXY_Count;
	m_timer = 0;

	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	m_AlertCondition = new QComboBox(this);
	m_AlertCondition->addItem(tr("Condition Green"), ConditionGreen);
	m_AlertCondition->addItem(tr("Condition Yellow"), ConditionYellow);
	m_AlertCondition->addItem(tr("Condition Red"), ConditionRed);
	m_AlertCondition->setToolTip(tr("<html><b>Alert Condition</b><br>"
				"The current alert condition is indicated on alert condition status panels. "
				"Alert condition is automatically triggered by certain events.<br>"
				"%1 Red - Active mission order is Self Destruct.<br>"
				"%1 Red - Structural damage is 90% or greater.<br>"
				"%1 Red - Visible adverse condition, e.g. fire, acid.<br>"
				"%1 Red - Air pressure in all rooms is not breathable.<br>"
				"%1 Red - Worst system damaged is 100%.<br>"
				"%1 Yellow - Structural damage is 50% or greater.<br>"
				"%1 Yellow - Worst system damage is more than 50%.<br>"
				"Only a person in the ship's chain-of-command may change the alert condition. "
				"<b>On old style spacecraft, alert condition cannot be changed; "
				"it always reflects the conditions shown above.</b> "
				"<b>On new style spacecraft, events may trigger a higher alert condition; "
				"a person or officer with the con must lower it.</b> "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_AlertCondition, SIGNAL(currentIndexChanged(int)), SLOT(AlertConditionChanged(int)));

	m_PilotUpPolicies = new QComboBox(this);
	m_PilotUpPolicies->addItem(tr("Fly ship upright at all times."), UpAwayFromGravity);
	m_PilotUpPolicies->addItem(tr("Fly ship inverted in space."), UpTowardGravity);
	m_PilotUpPolicies->addItem(tr("Fly ship with port side toward the gravity well."), PortTowardGravity);
	m_PilotUpPolicies->addItem(tr("Fly ship with starboard side toward the gravity well."), StarboardTowardGravity);
	m_PilotUpPolicies->setToolTip(tr("<html><b>Ship Orientation Policy</b><br>"
				"This policy tells the pilot which direction to orient the up direction of the ship. "
				"This policy only applies when the ship is outside the <i>atmosphere range</i> of the nearest world, "
				"regardless of whether the world actually has an atmosphere. "
				"Only a person in the ship's chain-of-command may change this policy.</html>"));
	connect(m_PilotUpPolicies, SIGNAL(currentIndexChanged(int)), SLOT(PilotUpPolicyChanged(int)));

	m_ShieldPolicies = new QComboBox(this);
	m_ShieldPolicies->addItem(tr("Shields up at all times."), ShieldUp);
	m_ShieldPolicies->addItem(tr("Shields down at all times."), ShieldDown);
	m_ShieldPolicies->addItem(tr("Shields down unless any contacts detected."), ShieldUpIfAnyone);
	m_ShieldPolicies->addItem(tr("Shields down unless non-empire contacts detected."), ShieldUpIfForeign);
	m_ShieldPolicies->addItem(tr("Shields down unless neutral or enemy contacts detected."), ShieldUpIfNeutral);
	m_ShieldPolicies->addItem(tr("Shields down unless enemy contacts detected."), ShieldUpIfEnemy);
	m_ShieldPolicies->setToolTip(tr("<html><b>Shields Up Policy</b><br>"
				"The shield operator prefers to keep the shields down to save power. "
				"This policy tells the shield operator what conditions require the shields to be raised.<br>"
				"Only a person in the ship's chain-of-command may change this policy.</html>"));
	connect(m_ShieldPolicies, SIGNAL(currentIndexChanged(int)), SLOT(ShieldPolicyChanged(int)));

	m_ShieldFuelPolicies = new QComboBox(this);
	m_ShieldFuelPolicies->addItem(tr("Ignore fuel status when raising shields."), ShieldUpIgnoreFuel);
	m_ShieldFuelPolicies->addItem(tr("Do not raise shields when ship has no fuel."), ShieldUpUnlessNoFuel);
	m_ShieldFuelPolicies->setToolTip(tr("<html><b>Shields Fuel Policy</b><br>"
				"This policy tells the shield operator how to consider the ship's fuel status before raising the shields.<br>"
				"Only a person in the ship's chain-of-command may change this policy.</html>"));
	connect(m_ShieldFuelPolicies, SIGNAL(currentIndexChanged(int)), SLOT(ShieldFuelPolicyChanged(int)));

	m_AccessPolicy = new QComboBox(this);
	m_AccessPolicy->addItem(tr("Admit Commanders"), ACCESS_Command);
	m_AccessPolicy->addItem(tr("Admit Crew"), ACCESS_Crew);
	m_AccessPolicy->addItem(tr("Admit Crew and Passengers"), ACCESS_CrewAndPassengers);
	m_AccessPolicy->addItem(tr("Admit Empire"), ACCESS_Empire);
	m_AccessPolicy->addItem(tr("Admit Friends"), ACCESS_Friend);
	m_AccessPolicy->addItem(tr("Admit Neutrals"), ACCESS_Neutral);
	m_AccessPolicy->addItem(tr("Admit Everyone"), ACCESS_Enemy);
	m_AccessPolicy->setToolTip(tr("<html><b>Ship Access Policy</b><br>"
				"Controls who is automatically admitted to the ship, <u>from the outside</u>. "
				"<br>"
				"All access choices include <i>Commanders</i>, people in the ship's chain of command. "
				"<i>Empire</i>, <i>Friend</i> and <i>Neutral</i> access includes crew and passengers. "
				"<br>"
				"This policy applies in these situations. "
				"<br>"
				"%1 Manually opening hull doors. "
				"<br>"
				"%1 Remote door openers on vehicles. "
				"<br>"
				"%1 Remote door openers on spacecraft. "
				"<br>"
				"%1 Space vehicle recovery system. "
				"<br>"
				"%1 Transporter beam up. "
				"<br>"
				"Only a person in the ship's chain-of-command may change this policy. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_AccessPolicy, SIGNAL(currentIndexChanged(int)), SLOT(AccessPolicyChanged(int)));

	pchristen = new QPushButton(QIcon(":data/MsgChristen.png"), "");
	pchristen->setStyleSheet(QString("font-size: 20px;"));
	pchristen->setToolTip(tr("<small><small><b>Christen Spacecraft</b><br>"
				"Changes the name of the spacecraft you are aboard.<br>"
				"The Captain of the ship can change its name.<br>"
				"The name of a ship with no captain can be changed by someone in its chain-of-command.</small></small>"));
	connect(pchristen, SIGNAL(clicked(bool)), SLOT(Christen()));

	lbshipowner = new QLabel;

	pbclaimtitle = new QPushButton(tr("Claim Title..."));
	pbclaimtitle->hide();
	connect(pbclaimtitle, SIGNAL(clicked(bool)), SLOT(ClaimTitle()));
	if(isAgent)
		pbclaimtitle->setToolTip(tr("<html><b>Claim Title</b><br>"
					"Claims ownership of the spacecraft.<br>"
					"<b>Agent Power</b> enables agent to claim the title to any ship they are aboard."
					"</html>"));
	else
		pbclaimtitle->setToolTip(tr("<html><b>Claim Title</b><br>"
					"Claims ownership of the spacecraft."
					"</html>"));

	pbfreetitle = new QPushButton(tr("Relinquish Title..."));
	pbfreetitle->hide();
	connect(pbfreetitle, SIGNAL(clicked(bool)), SLOT(FreeTitle()));
	if(isAgent)
		pbfreetitle->setToolTip(tr("<html><b>Relinquish Title</b><br>"
					"Releases ownership of a company spacecraft. "
					"A spacecraft with a free title can be claimed by anyone aboard.<br>"
					"Only the owner of a company spacecraft can relinquish the title.<br>"
					"<b>Agent Power</b> enables agent to release the title to any ship they are aboard."
					"</html>"));
	else
		pbfreetitle->setToolTip(tr("<html><b>Relinquish Title</b><br>"
					"Releases ownership of a company spacecraft. "
					"A spacecraft with a free title can be claimed by anyone aboard.<br>"
					"Only the owner of a company spacecraft can relinquish the title."
					"</html>"));

	pbcopytitle = new QPushButton(tr("Copy Title..."));
	pbcopytitle->hide();
	connect(pbcopytitle, SIGNAL(clicked(bool)), SLOT(CopyTitle()));
	pbcopytitle->setToolTip(tr("<html>"
				"<p>"
				"<b>Copy Title</b><br>"
				"Creates a copy of the title to the spacecraft, onto a blank paper or a blank disk. "
				"</p>"
				"<p>"
				"A copy of the title is used to claim ownership of the spacecraft. "
				"When ownership is claimed using a title, all other copies of the title are annulled. "
				"</p>"
				"<p>"
				"Only the owner of a company spacecraft can make a copy of the title. "
				"</p>"
				"</html>"));

	ckcrewopen = new QCheckBox(tr("Accept Crew Berth Requests"));
	ckcrewopen->setToolTip(tr("<b>Accept Crew Berth Requests</b><br>"
				"When checked, requests for crew berths are accepted automatically. "
				"When unchecked, requests for crew berths must be accepted by a person in the ship's chain-of-command.<br>"
				"This setting is ignored while the ship's current mission order is <i>Board Crew</i>.<br>"
				"Only a person in the ship's chain-of-command may change this policy."));
	connect(ckcrewopen, SIGNAL(stateChanged(int)), SLOT(CrewChecked(int)));

	ckpassopen = new QCheckBox(tr("Accept Passenger Berth Requests"));
	connect(ckpassopen, SIGNAL(stateChanged(int)), SLOT(PassengerChecked(int)));
	ckpassopen->setToolTip(tr("<b>Accept Passenger Berth Requests</b><br>"
				"When checked, requests for passenger berths are accepted automatically. "
				"When unchecked, requests for passenger berths must be accepted by a person in the ship's chain-of-command.<br>"
				"This setting is ignored while the ship's current mission order is <i>Board Passengers</i>.<br>"
				"Only a person in the ship's chain-of-command may change this policy."));

	ckpassSeaChest = new QCheckBox(tr("Allow Sea Chest Access"));
	connect(ckpassSeaChest, SIGNAL(stateChanged(int)), SLOT(PassengerSeaChestChecked(int)));
	ckpassSeaChest->setToolTip(tr("<b>Allow Passenger Sea Chest Access</b><br>"
				"When checked, passengers can access their sea chest from their berth. "
				"When unchecked, passengers cannot access their sea chest.<br>"
				"Only a person in the ship's chain-of-command may change this policy."));

	cktroopopen = new QCheckBox(tr("Accept Troop Berth Requests"));
	connect(cktroopopen, SIGNAL(stateChanged(int)), SLOT(TroopChecked(int)));
	cktroopopen->setToolTip(tr("<b>Accept Troop Berth Requests</b><br>"
				"When checked, requests for troop berths are accepted automatically. "
				"When unchecked, requests for troop berths must be accepted by a person in the ship's chain-of-command.<br>"
				"This setting is ignored while the ship's current mission order is <i>Board Troops</i>.<br>"
				"Only a person in the ship's chain-of-command may change this policy."));

	m_AutoFetchVehicles = new QCheckBox(tr("Automatically Load Vehicles"));
	connect(m_AutoFetchVehicles, SIGNAL(stateChanged(int)), SLOT(AutoFetchVehiclesChecked(int)));
	m_AutoFetchVehicles->setToolTip(tr("<b>Automatically Load Vehicles</b><br>"
				"When checked, new vehicles are loaded when needed to fill empty parking spots. "
				"If the ship is on a road slab, vehicles are loaded from the city. "
				"If the ship is in trade range of a station, vehicles are loaded from all cities in the solar system. "
				"One vehicle is loaded per minute. "
				"Export policies of the vehicle's empire apply. "
				"<br>"
				"When unchecked, vehicles are not loaded automatically.<br>"
				"Only a person in the ship's chain-of-command may change this policy."));

	QHBoxLayout* tlay = new QHBoxLayout;
	tlay->addStretch();
	tlay->addWidget(lbshipowner);
	tlay->addStretch();
	tlay->addWidget(pbclaimtitle);
	tlay->addWidget(pbfreetitle);
	tlay->addWidget(pbcopytitle);

	// Policy buttons go in a grid
	int row = 0;
	QGridLayout* lay = new QGridLayout;
	lay->addWidget(pchristen, row,0, 1,4);
	row++;
	lay->addLayout(tlay, row,1, 1,3);
	row++;
	lay->addWidget(m_AlertCondition, row,0, 1,4);
	row++;
	lay->addWidget(m_AccessPolicy, row,0, 1,4);
	row++;
	lay->addWidget(ckcrewopen, row,0, 1,4);
	row++;
	lay->addWidget(ckpassopen, row,0, 1,4);
	row++;
	lay->addWidget(ckpassSeaChest, row,1, 1,3);
	row++;
	lay->addWidget(cktroopopen, row,0, 1,4);
	row++;
	lay->addWidget(m_AutoFetchVehicles, row,0, 1,4);
	row++;
	lay->addWidget(m_PilotUpPolicies, row,0, 1,4);
	row++;
	lay->addWidget(m_ShieldPolicies, row,0, 1,4);
	row++;
	lay->addWidget(m_ShieldFuelPolicies, row,0, 1,4);
	row++;
	lay->setRowStretch(row,1);
	lay->setMargin(0);
	lay->setSpacing(2);

	QWidget* wf = new QWidget();
	wf->setLayout(lay);

	QScrollArea* scroll = new QScrollArea;
	scroll->setWidget(wf);
	scroll->setWidgetResizable(true);

	QVBoxLayout* flay = new QVBoxLayout;
	flay->addWidget(scroll);
	flay->setMargin(4);
	flay->setSpacing(2);
	setLayout(flay);

	connect(this, SIGNAL(AcceptCrew(bool)), auApp, SLOT(AcceptCrew(bool)));
	connect(this, SIGNAL(AcceptPassengers(bool)), auApp, SLOT(AcceptPassengers(bool)));
	connect(this, SIGNAL(AcceptTroops(bool)), auApp, SLOT(AcceptTroops(bool)));
	connect(this, SIGNAL(AccessPolicy(int)), auApp, SLOT(AccessPolicy(int)));
	connect(this, SIGNAL(AlertCondition(int)), auApp, SLOT(AlertCondition(int)));
	connect(this, SIGNAL(AllowPassengerSeaChests(bool)), auApp, SLOT(AllowPassengerSeaChests(bool)));
	connect(this, SIGNAL(AutoFetchVehicles(bool)), auApp, SLOT(AutoFetchVehicles(bool)));
	connect(this, SIGNAL(PilotUpPolicy(int)), auApp, SLOT(PilotUpPolicy(int)));
	connect(this, SIGNAL(ShieldFuelPolicy(int)), auApp, SLOT(ShieldFuelPolicy(int)));
	connect(this, SIGNAL(ShieldUpPolicy(int)), auApp, SLOT(ShieldUpPolicy(int)));
}

AuSpacecraftPolicy::~AuSpacecraftPolicy()
{
}

void AuSpacecraftPolicy::EnableControls()
{
	// Determine if the person is in the chain-of-command of the ship.
	AuView* pView = auApp->Frame()->View();
	bool inCommand = pView->IsPersonInCommand();		// also true for admins

	// Enable/disable controls.
	ckcrewopen->setEnabled(inCommand);
	ckpassopen->setEnabled(inCommand);
	ckpassSeaChest->setEnabled(inCommand);
	cktroopopen->setEnabled(inCommand);

	m_AccessPolicy->setEnabled(inCommand);
	m_AlertCondition->setEnabled(inCommand);
	m_AutoFetchVehicles->setEnabled(inCommand);
	m_PilotUpPolicies->setEnabled(inCommand);
	m_ShieldFuelPolicies->setEnabled(inCommand);
	m_ShieldPolicies->setEnabled(inCommand);
	pchristen->setEnabled(inCommand);
}

void AuSpacecraftPolicy::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuSpacecraftPolicy::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Create list view entries from spaceship atom data.
	// We must use the scene atom because the root ship atom never does the
	// extra work of analyzing its design.
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		m_PersonId = person->Id();
		m_PersonName = person->Name();
		m_ShipId = AuId::NullId();
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			m_PersonSector = sector->Id();
			m_PersonGalaxy = sector->Galaxy();
		}

		// List items that relate to the ship the person is aboard.
		const DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			m_ShipId = ship->Id();
			// Update name of ship.
			QString nameText = QString("%1 %2 %3")
				.arg(ship->Id().StringId())
				.arg(g_Au.MiddleDotSymbol())
				.arg(ship->NameWithEmpire());
			pchristen->setText(nameText);
			if(ship->IsPrivatelyOwned())
			{
				if(ship->OwnerId().IsNull())
				{
					pbclaimtitle->show();
					pbfreetitle->hide();
					pbcopytitle->hide();

					lbshipowner->setText(QString("<html>Free Title</html>"));
				}
				else
				{
					pbclaimtitle->hide();
					pbfreetitle->show();
					pbcopytitle->show();

					lbshipowner->setText(QString("<html>Owned by %1 <i>%2</i></html>")
							.arg(ship->OwnerName())
							.arg(ship->OwnerId().StringId()));

					bool canFree = m_PersonId == ship->OwnerId() || person->HasAgentPower();
					pbfreetitle->setEnabled(canFree);
					pbcopytitle->setEnabled(m_PersonId == ship->OwnerId());
				}
			}
			else
			{
				QString ename;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(ship->EmpireId(), ename);
				g_Au.UnlockData();
				lbshipowner->setText(QString("<html>%1 Fleet</html>").arg(ename));

				pbclaimtitle->hide();
				pbfreetitle->hide();
				pbcopytitle->hide();
			}

			m_AccessPolicy->blockSignals(true);
			int defidx = m_AccessPolicy->findData(ship->AccessPolicy());
			m_AccessPolicy->setCurrentIndex(defidx);
			m_AccessPolicy->blockSignals(false);

			m_AlertCondition->blockSignals(true);
			defidx = m_AlertCondition->findData(ship->ApparentAlertCondition());
			m_AlertCondition->setCurrentIndex(defidx);
			m_AlertCondition->blockSignals(false);

			m_PilotUpPolicies->blockSignals(true);
			defidx = m_PilotUpPolicies->findData(ship->PilotUpPolicy());
			m_PilotUpPolicies->setCurrentIndex(defidx);
			m_PilotUpPolicies->blockSignals(false);

			m_ShieldPolicies->blockSignals(true);
			defidx = m_ShieldPolicies->findData(ship->ShieldUpPolicy());
			m_ShieldPolicies->setCurrentIndex(defidx);
			m_ShieldPolicies->blockSignals(false);

			m_ShieldFuelPolicies->blockSignals(true);
			defidx = m_ShieldFuelPolicies->findData(ship->ShieldFuelPolicy());
			m_ShieldFuelPolicies->setCurrentIndex(defidx);
			m_ShieldFuelPolicies->blockSignals(false);

			// Set check boxes to reflect automatic berth acceptance policies.
			if(ship->CanAcceptBerthType(AuRosterEntry::Crew))
				ckcrewopen->setCheckState(Qt::Checked);
			else
				ckcrewopen->setCheckState(Qt::Unchecked);
			if(ship->CanAcceptBerthType(AuRosterEntry::PassengerHuman))
				ckpassopen->setCheckState(Qt::Checked);
			else
				ckpassopen->setCheckState(Qt::Unchecked);

			if(ship->CanPassengersAccessSeaChest())
				ckpassSeaChest->setCheckState(Qt::Checked);
			else
				ckpassSeaChest->setCheckState(Qt::Unchecked);

			if(ship->CanAcceptBerthType(AuRosterEntry::Troop))
				cktroopopen->setCheckState(Qt::Checked);
			else
				cktroopopen->setCheckState(Qt::Unchecked);

			if(ship->AutoFetchVehicles())
				m_AutoFetchVehicles->setCheckState(Qt::Checked);
			else
				m_AutoFetchVehicles->setCheckState(Qt::Unchecked);
		}
		else
		{
			// Not aboard a spacecraft.
			pchristen->setText("");
			lbshipowner->setText("");
		}

	}
	pView->UnlockSceneAtom();
	EnableControls();
}

void AuSpacecraftPolicy::CrewChecked(int state)
{
	emit AcceptCrew(state == Qt::Checked);
}

void AuSpacecraftPolicy::PassengerChecked(int state)
{
	emit AcceptPassengers(state == Qt::Checked);
}

void AuSpacecraftPolicy::PassengerSeaChestChecked(int state)
{
	emit AllowPassengerSeaChests(state == Qt::Checked);
}

void AuSpacecraftPolicy::AutoFetchVehiclesChecked(int state)
{
	emit AutoFetchVehicles(state == Qt::Checked);
}
void AuSpacecraftPolicy::TroopChecked(int state)
{
	emit AcceptTroops(state == Qt::Checked);
}

void AuSpacecraftPolicy::Christen()
{
	if(m_ShipId.IsNull())
		return;
	// We must ask the view to release keyboard grab.
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	AuChristenDialog dlg(m_ShipId, pFrame);
	int result = pFrame->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		QString name = dlg.Name().trimmed();
		auApp->ShipChristen(name);
	}
}

void AuSpacecraftPolicy::ClaimTitle()
{
	// We must ask the view to release keyboard grab.
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	QString smsg = tr("<html>"
			"Confirm claim of title to the spacecraft. "
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = pFrame->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	auApp->ShipClaimTitle();
}

void AuSpacecraftPolicy::FreeTitle()
{
	// We must ask the view to release keyboard grab.
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	QString smsg = tr("<html>"
			"<p>Confirm release of title to the spacecraft.</p>"
			"<p><b>Warning: Destruction of a spacecraft is permanent while it has no human officer.</b></p>"
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = pFrame->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	auApp->ShipReleaseTitle();
}

void AuSpacecraftPolicy::AlertConditionChanged(int index)
{
	if(index < 0)
		return;

	ALERT_CONDITION alertCondition = (ALERT_CONDITION)m_AlertCondition->itemData(index).toInt();
	emit AlertCondition(alertCondition);
}

void AuSpacecraftPolicy::PilotUpPolicyChanged(int index)
{
	if(index < 0)
		return;

	PILOT_UP_POLICY pilotUpPolicy = (PILOT_UP_POLICY)m_PilotUpPolicies->itemData(index).toInt();
	emit PilotUpPolicy(pilotUpPolicy);
}

void AuSpacecraftPolicy::ShieldPolicyChanged(int index)
{
	if(index < 0)
		return;

	SHIELD_POLICY shieldPolicy = (SHIELD_POLICY)m_ShieldPolicies->itemData(index).toInt();
	emit ShieldUpPolicy(shieldPolicy);
}

void AuSpacecraftPolicy::ShieldFuelPolicyChanged(int index)
{
	if(index < 0)
		return;

	SHIELDFUEL_POLICY shieldFuelPolicy = (SHIELDFUEL_POLICY)m_ShieldFuelPolicies->itemData(index).toInt();
	emit ShieldFuelPolicy(shieldFuelPolicy);
}

void AuSpacecraftPolicy::AccessPolicyChanged(int index)
{
	if(index < 0)
		return;

	ACCESS_POLICY accessPolicy = (ACCESS_POLICY)m_AccessPolicy->itemData(index).toInt();
	emit AccessPolicy(accessPolicy);
}

void AuSpacecraftPolicy::CopyTitle()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	AuShipTitle dlg(pFrame);
	int result = pFrame->ExecDialog(dlg);
	if(result != QDialog::Accepted)
		return;
	AuGearPos mediaPos = dlg.MediaPos();
	if(mediaPos == AuGearPos())
		return;
	auApp->ShipCopyTitle(mediaPos);
}

