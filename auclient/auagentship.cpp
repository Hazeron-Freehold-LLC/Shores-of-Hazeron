//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentship.cpp,v 1.7 2022/02/19 18:04:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentship.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "davehicle.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

AuAgentShip::AuAgentShip(QWidget* parent) : QWidget(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	bool isArchitect = gd.ClientPlayerIsArchitect();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	QLabel* loadLabel = 0;
	QPushButton* pbloadcrew = 0;
	QPushButton* pbloadofficer = 0;
	QPushButton* pbloadtroops = 0;
	QPushButton* pbloadpassengers = 0;
	QPushButton* pbrecover = 0;
	QPushButton* pbscuttle = 0;
	QPushButton* pbownership = 0;
	QPushButton* pbpirateequip = 0;
	QPushButton* pbdrain = 0;
	QPushButton* pbfill = 0;
	if(isProgrammer || isMinion)
	{
		if(isProgrammer)
		{
			pbrecover = new QPushButton("Recover Ship...");
			pbrecover->setToolTip("<html>Recovers a spacecraft from stasis. "
					"</html>");
			connect(pbrecover, SIGNAL(clicked()), SLOT(RecoverShip()));

			pbscuttle = new QPushButton("Scuttle Ship...");
			pbscuttle->setToolTip("<html>Scuttles the spacecraft you are aboard. "
					"All players aboard, online or not, are safely recalled home before the ship is destroyed. "
					"This includes the agent who initiates the scuttle.</html>");
			connect(pbscuttle, SIGNAL(clicked()), SLOT(ScuttleShip()));

			pbownership = new QPushButton("Toggle Company vs Fleet");
			connect(pbownership, SIGNAL(clicked()), SLOT(ToggleOwnership()));
			pbownership->setToolTip(QString("<html>Changes the ownership type of the ship you are aboard. "
						"You must be aboard a spacecraft.</html>"));

			pbdrain = new QPushButton("Drain Capacitor...");
			connect(pbdrain, SIGNAL(clicked()), SLOT(DrainCapacitor()));
			pbdrain->setToolTip(QString("<html>Drains the capacitor of the spacecraft that the agent is currently aboard.</html>"));

			pbfill = new QPushButton("Fill Capacitor...");
			connect(pbfill, SIGNAL(clicked()), SLOT(FillCapacitor()));
			pbfill->setToolTip(QString("<html>Fills the capacitor of the spacecraft that the agent is currently aboard.</html>"));
		}

		if(!auApp->IsSingleUserDesignerMode())
		{
			loadLabel = new QLabel("Roster");

			pbloadcrew = new QPushButton("Load Crew");
			connect(pbloadcrew, SIGNAL(clicked()), SLOT(LoadCrew()));
			pbloadcrew->setToolTip(QString("<html>Loads crew to fill all empty crew berths of the spacecraft you are aboard. "
						"Crew will match your DNA and empire. "
						"You must be aboard a spacecraft.</html>"));

			pbloadtroops = new QPushButton("Load Troops");
			connect(pbloadtroops, SIGNAL(clicked()), SLOT(LoadTroops()));
			pbloadtroops->setToolTip(QString("<html>Loads troops to fill all empty troop berths of the spacecraft you are aboard. "
						"Troops will match your DNA and empire. "
						"You must be aboard a spacecraft.</html>"));

			pbloadofficer = new QPushButton("Load Officer");
			connect(pbloadofficer, SIGNAL(clicked()), SLOT(LoadOfficer()));
			pbloadofficer->setToolTip(QString("<html>Loads an officer to fill an empty officer berth of the spacecraft you are "
						"aboard. "
						"The officer will match your DNA and empire. "
						"You must be aboard a spacecraft.</html>"));

			pbloadpassengers = new QPushButton("Load Passengers");
			connect(pbloadpassengers, SIGNAL(clicked()), SLOT(LoadPassengers()));
			pbloadpassengers->setToolTip(QString("<html>Loads passengers to fill all empty passengers berths of the spacecraft you "
						"are aboard. "
						"Passengers will match your DNA and empire. "
						"You must be aboard a spacecraft.</html>"));

			if(isArchitect)
			{
				pbpirateequip = new QPushButton("Pirate Equip");
				connect(pbpirateequip, SIGNAL(clicked()), SLOT(PirateEquip()));
				pbpirateequip->setToolTip(QString("<html>Equips the spacecraft you are aboard the way pirates get equipped.<br>"
							"%1 Capacitor charged.<br>"
							"%1 Maneuver drive turned on.<br>"
							"%1 Interior lighting set dim.<br>"
							"%1 Fuel cells filled.<br>"
							"%1 Shields charged.<br>"
							"%1 Moorings cast off.<br>"
							"%1 <i>Where made</i> set to current location.<br>"
							"%1 Harvester bays refitted with weapon modules.<br>"
							"%1 Half of hold filled with ammo needed by weapon bays and turrets.<br>"
							"%1 Officer loaded if a berth is available and no officer is already aboard.<br>"
							"%1 Crew and troops loaded into all available berths.<br>"
							"%1 Empire set to your current empire.<br>"
							"%1 Christened with random name.</html>")
						.arg(g_Au.MiddleDotSymbol()));
			}
		}
	}

	QPushButton* pbclearrat = 0;
	if(!isMinion)
	{
		pbclearrat = new QPushButton("Clear Rat DNA...");
		connect(pbclearrat, SIGNAL(clicked()), SLOT(ClearRatDNA()));
	}

	QPushButton* pbrefuel = new QPushButton("Refuel...");
	connect(pbrefuel, SIGNAL(clicked()), SLOT(Refuel()));
	pbrefuel->setToolTip(QString("<html>Refuels the vehicle or spacecraft that the agent is currently aboard.</html>"));

	QPushButton* pbrepair = new QPushButton("Repair");
	connect(pbrepair, SIGNAL(clicked()), SLOT(Repair()));
	pbrepair->setToolTip(QString("<html>Repairs the vehicle or spacecraft that the agent is currently aboard. "
				"Concludes repair time on a spacecraft already undergoing repairs.</html>"));

	QLabel* svlabel = new QLabel("Ships and Vehicles");

	QLabel* slabel = 0;
	if(!isMinion)
		slabel = new QLabel("Ships");

	// Construct layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;

	if(loadLabel)
	{
		lay->addWidget(loadLabel, row,1, 1,2);
		row++;
		lay->addWidget(pbloadofficer, row,2, 1,4);
		lay->addWidget(pbloadcrew, row,6, 1,4);
		row++;
		lay->addWidget(pbloadtroops, row,2, 1,4);
		lay->addWidget(pbloadpassengers, row,6, 1,4);
		row++;
	}


	lay->addWidget(svlabel, row,1);
	row++;

	lay->addWidget(pbrefuel, row,2, 1,4);
	lay->addWidget(pbrepair, row,6, 1,4);
	row++;

	if(slabel)
	{
		lay->addWidget(slabel, row,1);
		row++;
	}

	if(pbownership)
	{
		lay->addWidget(pbownership, row,2, 1,8);
		row++;
	}

	if(pbclearrat)
	{
		lay->addWidget(pbclearrat, row,2, 1,8);
		row++;
	}

	if(pbfill && pbdrain)
	{
		lay->addWidget(pbfill, row,2, 1,4);
		lay->addWidget(pbdrain, row,6, 1,4);
		row++;
	}

	if(pbpirateequip)
	{
		lay->addWidget(pbpirateequip, row,2, 1,8);
		row++;
	}

	if(pbscuttle && pbrecover)
	{
		lay->addWidget(pbrecover, row,2, 1,4);
		lay->addWidget(pbscuttle, row,6, 1,4);
		row++;
	}

	lay->setRowStretch(row++, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuAgentShip::~AuAgentShip()
{
}

void AuAgentShip::setVisible(bool visible)
{
	QWidget::setVisible(visible);
}

void AuAgentShip::ToggleOwnership()
{
	auApp->ShipToggleOwnership();
}

void AuAgentShip::ScuttleShip()
{
	AuAgentScuttle scuttleShip(this);
	if(scuttleShip.exec() == QDialog::Accepted)
	{
		QString reason = scuttleShip.Reason();
		auApp->ShipScuttle(reason);
	}
}

void AuAgentShip::RecoverShip()
{
	QString lastRecover = g_Au.Settings().readEntry("Widget/Admin/RecoverShip", "");
	bool ok;
	QString sid = QInputDialog::getText(this, tr("Recover Spacecraft"),
			tr("Enter the atom id of the spacecraft to recover from stasis. It can be a string or numeric id."),
			QLineEdit::Normal, lastRecover, &ok);
	if(!ok)
		return;
	AuId shipId(sid);
	if(shipId.IsNull())
		return;
	g_Au.Settings().writeEntry("Widget/Admin/RecoverShip", sid);
	auApp->ShipRecover(shipId);
}

void AuAgentShip::Refuel()
{
	// Determine current fuel quality.
	quint8 fuelQ = 0;	// 0 = n/a
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		const DaVehicle* veh = dynamic_cast<const DaVehicle*>(person->Parent());
		if(!veh)
		{
			const DaVessel* ves = person->FindParentVesselInEnv();
			if(ves)
				fuelQ = ves->FuelQuality();
		}
	}
	view->UnlockSceneAtom();

	if(fuelQ)
	{
		// Fuel Q is relevant.
		bool ok;
		QString qual = QInputDialog::getText(this, tr("Fuel Quality"), tr("Enter the quality of the fuel.<br>"
					"Current fuel quality is shown."),
				QLineEdit::Normal, QString("%1").arg(fuelQ), &ok);
		if(!ok)
			return;
		fuelQ = QVariant(qual).toUInt();
	}
	else
		fuelQ = 1;	// Fuel Q is n/a but we can't send a zero.

	auApp->ShipRefuel(fuelQ);
}

void AuAgentShip::Repair()
{
	auApp->ShipRepair();
}

void AuAgentShip::DrainCapacitor()
{
	bool ok;
	quint8 percentCap = (quint8)QInputDialog::getInt(this, tr("Drain Capacitor..."),
			tr("Enter the percent of the current capacitor power level to drain. "
				"AI engineer alerts the crew of this event on the crew channel."), 100, 1, 100, 1, &ok);
	if(!ok)
		return;
	auApp->ShipDrainCapacitor(percentCap);
}

void AuAgentShip::FillCapacitor()
{
	bool ok;
	quint8 percentCap = (quint8)QInputDialog::getInt(this, tr("Fill Capacitor..."),
			tr("Enter the percent of the capacitor to fill. "
				"AI engineer alerts the crew of this event on the crew channel."), 100, 1, 100, 1, &ok);
	if(!ok)
		return;
	auApp->ShipFillCapacitor(percentCap);
}

void AuAgentShip::LoadOfficer()
{
	auApp->ShipLoadOfficer();
}

void AuAgentShip::LoadCrew()
{
	auApp->ShipLoadCrew();
}

void AuAgentShip::LoadTroops()
{
	auApp->ShipLoadTroops();
}

void AuAgentShip::LoadPassengers()
{
	auApp->ShipLoadPassengers();
}

void AuAgentShip::PirateEquip()
{
	auApp->ShipPirateEquip();
}

void AuAgentShip::ClearRatDNA()
{
	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("The rat DNA in your current spacecraft will be cleared, like it was never set.<br><br>"
			"<b>Are you sure you want to do this?<b>"),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->ShipClearRatDNA();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAgentScuttle::AuAgentScuttle(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/FleetShipFinal.png"));
	setAuDlgTitle("<b>Scuttle Ship</b>");

	QLabel* lb0 = new QLabel(tr("<html>The ship you are aboard will be destroyed. "
				"All avatars aboard the ship will safely recall home before the ship is destroyed. "
				"Avatars aboard who hold a berth on the ship will release their berth.<br><br>"
				"<b>Company ships are destroyed permanently with no way to recover them.</b><br><br>"
				"Enter a reason here, to be included in mail sent to people who are aboard:"), this);
	lb0->setWordWrap(true);

	m_Reason = new QLineEdit(this);
	m_Reason->setToolTip(tr("<html>Enter a reason for scuttling the ship. "
			"This will be included in a mail message sent to everyone aboard. "
			"If blank, the message will say, 'No reason given.'</html>"));

	// Ok button that makes only one
	QPushButton* pbok = new QPushButton(tr("Scuttle Ship"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>Scuttles the ship.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	// Construct final overall layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lb0, row,0);
	row++;
	lay->addWidget(m_Reason, row,0);
	row++;
	lay->addWidget(pbok, row,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuAgentScuttle::~AuAgentScuttle()
{
}

QString AuAgentScuttle::Reason() const
{
	QString reason = m_Reason->text().trimmed();
	if(!reason.isEmpty() && reason.right(1) == ".")
		reason = reason.left(reason.length()-1).trimmed();
	return reason;
}
