//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucargotransfer.cpp,v 1.19 2022/05/04 21:19:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucargotransfer.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "daatomlist.h"
#include "daperson.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>

void AuCargoTransferDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCargoTransferUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCargoTransfer::AuCargoTransfer(QWidget* parent) : QWidget(parent)
{
	m_MyMoneyAmt = 0;
	m_OtherMoneyAmt = 0;
	m_timer = 0;

	m_GearFilter = FILTER_All;
	m_Filter = new AuCommodityFilter;
	connect(m_Filter, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(Filter(COMMODITYFILTER)));

	// Combo box for selecting other ships.
	m_OtherShips = new QComboBox(this);
	m_OtherShips->setToolTip(tr("<html>Select a spacecraft to transfer cargo to/from. "
				"Spacecraft listed are in range and you have authority to transfer cargo. "
				"Authority to transfer cargo is granted if any one of these conditions is true.<br>"
				"%1 Spacecraft is in your chain of command.<br>"
				"%1 Spacecraft is executing a <i>Stand Down</i> order.<br>"
				"%1 Spacecraft is new and the owning empire is yours, friendly, or vassal.<br>"
				"%1 Spacecraft is not new and it has no officers.<br>"
				"%1 Spacecraft is broadcasting an SOS for fuel."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	// Create widgets to display current ship info.
	{
		// Ship Name
		m_MyName = new QLabel(this);

		// Ship Description
		m_MyDesc = new QLabel(this);

		// Money
		m_MyMoney = new QPushButton(this);
		m_MyMoney->setToolTip(tr("<html>Sends money to the other ship. Your ship's account balance is shown.</html>"));
		connect(m_MyMoney, SIGNAL(clicked()), SLOT(SendMoney()));

		// Items
		m_MyItems = new QPushButton(tr("Send Items %1").arg(g_Au.DoubleAngleRightSymbol()), this);
		m_MyItems->setToolTip(tr("<html>Sends items to the other ship.</html>"));
		connect(m_MyItems, SIGNAL(clicked()), SLOT(SendItems()));

		// Citizens
		m_MyCitizens = new QTableWidget(0, 2, this);
		m_MyCitizens->setToolTip(tr("<html>Citizens aboard your ship. "
					"Double-click a citizen to transfer them to the other ship. "
					"If the other ship's berths are full, the citizen is swapped with the other ship's selected citizen or a "
					"citizen who volunteers. "
					"Citizens are only swapped with the same kind of citizens. "
					"Citizens will not transfer if they are prohibited from serving aboard the other ship."
					"</html>"));
		m_MyCitizens->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
		m_MyCitizens->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
		QStringList headerLabels;
		headerLabels << "Name";
		headerLabels << "Status";
		m_MyCitizens->setHorizontalHeaderLabels(headerLabels);
		m_MyCitizens->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_MyCitizens->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_MyCitizens->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_MyCitizens->setShowGrid(false);
		m_MyCitizens->setSortingEnabled(true);
		m_MyCitizens->sortByColumn(0, Qt::AscendingOrder);
		m_MyCitizens->verticalHeader()->hide();
		m_MyCitizens->verticalHeader()->setDefaultSectionSize(18);
		m_MyCitizens->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		connect(m_MyCitizens, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(SendCitizen(QTableWidgetItem*)));

		m_SendCitizens = new QPushButton(tr("Send Citizens %1").arg(g_Au.DoubleAngleRightSymbol()), this);
		m_SendCitizens->setToolTip(tr("<html>Transfers the selected citizens to the other ship. "
					"If the other ship's berths are full, the citizens are swapped with the other ship's selected citizens or "
					"citizens who volunteer. "
					"Citizens are only swapped with the same kind of citizens. "
					"Citizens will not transfer if they are prohibited from serving aboard the other ship."
					"</html>"));
		connect(m_SendCitizens, SIGNAL(clicked()), SLOT(SendCitizens()));

		// Cargo
		m_MyCargo = new QListWidget(this);
		m_MyCargo->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_MyCargo->setToolTip(tr("<html>Cargo aboard your ship. "
					"Double-click cargo to transfer a specific amount of it to the other ship.</html>"));
		connect(m_MyCargo, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(SendCargoItem(QListWidgetItem*)));
		connect(m_MyCargo, SIGNAL(itemSelectionChanged()), SLOT(MyCargoSelected()));

		m_SendCargo = new QPushButton(tr("Send Cargo %1").arg(g_Au.DoubleAngleRightSymbol()), this);
		m_SendCargo->setToolTip(tr("<html>Transfers the selected cargo to the other ship.</html>"));
		connect(m_SendCargo, SIGNAL(clicked()), SLOT(SendCargo()));

		m_SendSomeCargo = new QPushButton(tr("Send Some Cargo %1").arg(g_Au.DoubleAngleRightSymbol()), this);
		m_SendSomeCargo->setToolTip(tr("<html>Transfers some of the selected cargo to the other ship. "
					"Double-clicking an item also does this.</html>"));
		connect(m_SendSomeCargo, SIGNAL(clicked()), SLOT(SendSomeCargo()));
	}

	// Create widgets to display other ship info.
	{
		// Ship Name
		m_OtherName = new QLabel(this);

		// Ship Description
		m_OtherDesc = new QLabel(this);

		// Money
		m_OtherMoney = new QPushButton(this);
		m_OtherMoney->setToolTip(tr("<html>Takes money from the other ship. The other ship's account balance is shown.</html>"));
		connect(m_OtherMoney, SIGNAL(clicked()), SLOT(TakeMoney()));

		// Items
		m_OtherItems = new QPushButton(tr("%1 Take Items").arg(g_Au.DoubleAngleLeftSymbol()), this);
		m_OtherItems->setToolTip(tr("<html>Takes items from the other ship.</html>"));
		connect(m_OtherItems, SIGNAL(clicked()), SLOT(TakeItems()));

		// Citizens
		m_OtherCitizens = new QTableWidget(0, 2, this);
		m_OtherCitizens->setToolTip(tr("<html>Citizens aboard the other ship. "
					"Double-click a citizen to transfer them to your ship. "
					"If your ship's berths are full, the citizen is swapped with your ship's selected citizen or a "
					"citizen who volunteers. "
					"Citizens will not transfer if they are prohibited from serving aboard your ship."
					"</html>"));
		m_OtherCitizens->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
		m_OtherCitizens->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
		QStringList headerLabels;
		headerLabels << "Name";
		headerLabels << "Status";
		m_OtherCitizens->setHorizontalHeaderLabels(headerLabels);
		m_OtherCitizens->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_OtherCitizens->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_OtherCitizens->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_OtherCitizens->setShowGrid(false);
		m_OtherCitizens->setSortingEnabled(true);
		m_OtherCitizens->sortByColumn(0, Qt::AscendingOrder);
		m_OtherCitizens->verticalHeader()->hide();
		m_OtherCitizens->verticalHeader()->setDefaultSectionSize(18);
		m_OtherCitizens->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		connect(m_OtherCitizens, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(TakeCitizen(QTableWidgetItem*)));

		m_TakeCitizens = new QPushButton(tr("%1 Take Citizens").arg(g_Au.DoubleAngleLeftSymbol()), this);
		m_TakeCitizens->setToolTip(tr("<html>Transfers the selected citizens to your ship. "
					"If your ship's berths are full, the citizens are swapped with your ship's selected citizens or "
					"citizens who volunteer. "
					"Citizens are only swapped with the same kind of citizens. "
					"Citizens will not transfer if they are prohibited from serving aboard your ship."
					"</html>"));
		connect(m_TakeCitizens, SIGNAL(clicked()), SLOT(TakeCitizens()));

		// Cargo
		m_OtherCargo = new QListWidget(this);
		m_OtherCargo->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_OtherCargo->setToolTip(tr("<html>Cargo aboard the other ship. "
					"Double-click cargo to transfer a specific amount of it to your ship.</html>"));
		connect(m_OtherCargo, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(TakeCargoItem(QListWidgetItem*)));
		connect(m_OtherCargo, SIGNAL(itemSelectionChanged()), SLOT(OtherCargoSelected()));

		m_TakeCargo = new QPushButton(tr("%1 Take Cargo").arg(g_Au.DoubleAngleLeftSymbol()), this);
		m_TakeCargo->setToolTip(tr("<html>Transfers the selected cargo to your ship.</html>"));
		connect(m_TakeCargo, SIGNAL(clicked()), SLOT(TakeCargo()));

		m_TakeSomeCargo = new QPushButton(tr("%1 Take Some Cargo").arg(g_Au.DoubleAngleLeftSymbol()), this);
		m_TakeSomeCargo->setToolTip(tr("<html>Transfers a portion of the selected cargo to your ship. "
					"Double-clicking an item also does this.</html>"));
		connect(m_TakeSomeCargo, SIGNAL(clicked()), SLOT(TakeSomeCargo()));
	}
	
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_OtherShips, row,2, 1,2);
	row++;
	lay->addWidget(m_MyName, row,0, 1,2);
	lay->addWidget(m_OtherName, row,2, 1,2);
	row++;
	lay->addWidget(m_MyDesc, row,0, 1,2);
	lay->addWidget(m_OtherDesc, row,2, 1,2);
	row++;
	lay->addWidget(m_MyMoney, row,0, 1,2);
	lay->addWidget(m_OtherMoney, row,2, 1,2);
	row++;
	lay->addWidget(m_MyItems, row,0, 1,2);
	lay->addWidget(m_OtherItems, row,2, 1,2);
	row++;
	lay->addWidget(m_MyCitizens, row,0, 1,2);
	lay->addWidget(m_OtherCitizens, row,2, 1,2);
	row++;
	lay->addWidget(m_SendCitizens, row,0, 1,2);
	lay->addWidget(m_TakeCitizens, row,2, 1,2);
	row++;
	lay->addWidget(m_Filter, row,0, 1,4);
	row++;
	lay->addWidget(m_MyCargo, row,0, 1,2);
	//lay->addWidget(m_MyCargoDesc, row,1);
	lay->addWidget(m_OtherCargo, row,2, 1,2);
	//lay->addWidget(m_OtherCargoDesc, row,3);
	row++;
	lay->addWidget(m_SendCargo, row,0);
	lay->addWidget(m_SendSomeCargo, row,1);
	lay->addWidget(m_TakeCargo, row,2);
	lay->addWidget(m_TakeSomeCargo, row,3);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuCargoTransfer::~AuCargoTransfer()
{
}

void AuCargoTransfer::showEvent(QShowEvent* event)
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

void AuCargoTransfer::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	bool requestCitizensMyShip = false;
	bool requestCitizensOtherShip = false;

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaVessel* ship = person->FindParentVesselInEnv();
		const DaVessel* otherShip = 0;				// Will be set if we locate the other ship that is selected.
		if(ship && ship->InChainOfCommand(person))
		{
			// Person is aboard a spacecraft that is in their chain of command.
			m_MyName->setText(QString("<html><big><b>%1</b></big></html>").arg(ship->NameWithEmpire()));
			if(m_MyShipId != ship->Id())
			{
				m_MyShipId = ship->Id();
				requestCitizensMyShip = true;
			}

			// Show description.
			QString desc = QString("<html>");
			desc += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\">"
				"<tbody>";
			desc += "<tr>";
			if(ship->OfficerBerths() || ship->CrewBerths() || ship->TroopBerths() || ship->PassengerBerths())
			{
				// Berth Occupancy
				desc += "<td>";
				desc += tr("<b>Berths</b>");
				desc += "<div style=\"margin-left: 10px;\">";
				if(ship->OfficerBerths())
					desc += tr("Officer: %1 of %2 occupied<br>")
						.arg(ship->OfficerBerthsOccupied()).arg(ship->OfficerBerths());
				if(ship->CrewBerths())
					desc += tr("Crew: %1 of %2 occupied<br>")
						.arg(ship->CrewBerthsOccupied()).arg(ship->CrewBerths());
				if(ship->TroopBerths())
					desc += tr("Troop: %1 of %2 occupied<br>")
						.arg(ship->TroopBerthsOccupied()).arg(ship->TroopBerths());
				if(ship->PassengerBerths())
					desc += tr("Passenger: %1 of %2 occupied<br>")
						.arg(ship->PassengerBerthsOccupied()).arg(ship->PassengerBerths());
				desc = desc.left(desc.length()-4);	// Drop the trailing <br>
				desc += "</div>";
				desc += "</td>";
			}
			{
				// Cargo
				desc += "<td>";
				desc += tr("<b>Cargo</b>");
				desc += "<div style=\"margin-left: 10px;\">";
				desc += tr("Hold: %L1 of %L2 used<br>")
					.arg(ship->CargoTotalVolume())
					.arg(ship->CargoCapacityVolume());
				desc += tr("Fuel Cell: %L1 of %L2 used")
					.arg((int)ceil(ship->FuelQuantity()))
					.arg((int)ship->FuelCapacity());
				desc += "</div>";
				desc += "</td>";
			}
			desc += "</tr>";
			desc += "</tbody></table>";
			desc += QString("</html>");
			m_MyDesc->setText(desc);

			// Show money aboard ship.
			m_MyMoneyAmt = ship->AccountBalance();
			m_MyMoney->setText(QString("%L1%2 %3").arg(m_MyMoneyAmt,0,'f',0).arg(g_Au.CurrencySymbol())
					.arg(g_Au.DoubleAngleRightSymbol()));
			m_MyMoney->setEnabled(true);

			m_MyItems->setEnabled(true);

			// Show citizens aboard ship.
			//ListCitizens(m_MyCitizens, ship);	// Only lists citizens that are in my scene. :/
			m_MyCitizens->setEnabled(true);

			// Show cargo aboard ship.
			ListCargo(m_MyCargo, ship);
			m_MyCargo->setEnabled(true);

			// List ships in the solar system (and in the scene) that are in trade range.
			const DaAtom* system = ship->FindEnvironmentAtom();
			if(system)
			{
				// Start by getting a list of the ships already in the combo box.
				AuIdList shipsInBox;
				for(int row=0; row<m_OtherShips->count(); row++)
					shipsInBox.append(AuId(m_OtherShips->itemData(row).toInt()));

				// Get a list of all the ships in the solar system that could be in the combo box.
				DaAtomList shipsInSystem;
				system->FindVesselsRecursive(shipsInSystem);

				// List ships in range that are not already in the combo box.
				// Remove ships from shipsInBox who should remain in the combo box, so we can delete the ones that are gone.
				AuPoint shipLoc = ship->EnvLocation();
				AuPoint otherLoc;
				for(DaAtomList::const_iterator sisi = shipsInSystem.begin(); sisi != shipsInSystem.end(); sisi++)
				{
					DaVessel* shipInSystem = dynamic_cast<DaVessel*>(*sisi);
					if(!shipInSystem)
						continue;								// This should not happen due to the way the Find routines work.
					if(shipInSystem == ship)
						continue;								// Skip my own ship.

					// Determine if the person has the authority to transfer stuff to/from the other ship.
					int transferAuthority = 0;
					if(shipInSystem->Monster())
						transferAuthority = 5;					// Ship is having a fuel emergency.
					else if(shipInSystem->InChainOfCommand(person))
						transferAuthority = 1;					// Ships in my chain of command can be looted.
					else if(shipInSystem->MissionOrderType() == AuMissionOrder::StandDown)
						transferAuthority = 2;					// Ships that are standing down are eligible for looting.
					else if(shipInSystem->IsNew())
					{
						// The other ship is new. I can only get into it if its empire is friendly to mine.
						POLITICAL_STANCE stance = PoliticalStance(shipInSystem->EmpireId(), ship->EmpireId());
						if(stance == POL_Empire || stance == POL_Friend || stance == POL_Vassal)
							transferAuthority = 3;				// Ship's empire is friendly toward mine; I can loot it.
					}
					else if(shipInSystem->IsSpaceStation() && !shipInSystem->IsPrivatelyOwned())
						transferAuthority = 0;					// No transfer with subsidized station if not in chain of command.
					else if(!shipInSystem->HasOfficers())
						transferAuthority = 4;					// Ship is not new and it has no officers; person can loot it.
					if(!transferAuthority)
						continue;								// Person does not have authority to transfer stuff.

					// Determine range to the other ship.
					otherLoc = shipInSystem->EnvLocation();
					double distToShip = shipLoc.Distance(otherLoc);
					if(distToShip > ship->TransferRange(shipInSystem))
						continue;								// Skip ships that are not in trade range.
					//qDebug() << "shipInSystem" << shipInSystem->Id() << shipInSystem->Name() << "distToShip" << distToShip
						//<< "transfer" << ship->TransferRange(shipInSystem);

					// The combo box text shows the authority that permits the transfer.
					QString itemText;
					if(transferAuthority == 1)
						itemText = tr("%1, Chain-of-Command").arg(shipInSystem->SpotDescription(person->EmpireId()));
					else if(transferAuthority == 2)
						itemText = tr("%1, Standing Down").arg(shipInSystem->SpotDescription(person->EmpireId()));
					else if(transferAuthority == 3)
						itemText = tr("%1, New").arg(shipInSystem->SpotDescription(person->EmpireId()));
					else if(transferAuthority == 4)
						itemText = tr("%1, Leaderless").arg(shipInSystem->SpotDescription(person->EmpireId()));
					else if(transferAuthority == 5)
						itemText = tr("%1, Fuel Emergency").arg(shipInSystem->SpotDescription(person->EmpireId()));
					else
						itemText = tr("%1, Unknown").arg(shipInSystem->SpotDescription(person->EmpireId()));

					// Here if we have decided the ship is eligible for cargo transfer.
					int shipIdx = m_OtherShips->findData(QVariant(shipInSystem->Id().GetID()));
					if(shipIdx >= 0)
					{
						// The ship is already in the combo box.
						if(m_OtherShips->itemText(shipIdx) != itemText)
							m_OtherShips->setItemText(shipIdx, itemText);	// Update the text that changed.

						// Remove its id from the list so it is not deleted at the end.
						shipsInBox.Remove(shipInSystem->Id());
					}
					else
					{
						// The ship is not in the combo box.
						// Create a combo box entry for the ship or station.
						if(shipInSystem->IsSpaceStation())
							m_OtherShips->addItem(QIcon(":data/CommStation.png"), itemText, QVariant(shipInSystem->Id().GetID()));
						else
							m_OtherShips->addItem(QIcon(":data/c_Spacecraft.png"), itemText, QVariant(shipInSystem->Id().GetID()));
					}
				}

				// Finally, remove entries from the combo box that are still in our shipsInBox list.
				// Those ships are no longer valid.
				for(AuIdList::const_iterator sibi = shipsInBox.cbegin(); sibi != shipsInBox.cend(); sibi++)
				{
					int shipIdx = m_OtherShips->findData(QVariant((*sibi).GetID()));
					if(shipIdx >= 0)
						m_OtherShips->removeItem(shipIdx);
				}

				// Get pointer to current ship in combo box.
				if(m_OtherShips->currentIndex() >= 0)
				{
					AuId otherShipId = AuId(m_OtherShips->itemData(m_OtherShips->currentIndex()).toInt());
					for(DaAtomList::const_iterator sisi = shipsInSystem.begin(); sisi != shipsInSystem.end(); sisi++)
						if((*sisi)->Is(otherShipId))
						{
							otherShip = dynamic_cast<const DaVessel*>(*sisi);
							break;
						}
				}

				m_OtherShips->setEnabled(true);
			}
			else
			{
				// Not likely that the parent solar system could not be found.
				m_OtherShips->clear();
				m_OtherShips->setEnabled(false);
			}
		}
		else
		{
			// Not aboard a spacecraft or not in the chain of command of the spacecraft.
			if(ship)
				m_MyName->setText(tr("<html>%1 - Not In Command</html>").arg(ship->SpotDescription(person->EmpireId())));
			else
				m_MyName->setText(tr("<html>Not Aboard Spacecraft</html>"));
			m_MyDesc->clear();
			m_MyShipId = AuId();

			m_MyCargo->clear();
			m_MyCargo->setEnabled(false);
			//m_MyCargoDesc->setHtml("");

			m_MyMoneyAmt = 0;
			m_MyMoney->setText(QString("%2 %3").arg(g_Au.CurrencySymbol()).arg(g_Au.DoubleAngleRightSymbol()));
			m_MyMoney->setEnabled(false);

			m_MyItems->setEnabled(false);

			m_MyCitizens->clearContents();
			m_MyCitizens->setEnabled(false);

			m_OtherShips->clear();
			m_OtherShips->setEnabled(false);
		}

		// Show information about the selected other ship.
		if(otherShip)
		{
			// Person is aboard a spacecraft that is in their chain of command.
			m_OtherName->setText(QString("<html><big><b>%1</b></big></html>").arg(otherShip->NameWithEmpire()));
			if(m_OtherShipId != otherShip->Id())
			{
				m_OtherShipId = otherShip->Id();
				requestCitizensOtherShip = true;
			}

			// Show description.
			QString desc = QString("<html>");
			desc += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\">"
				"<tbody>";
			desc += "<tr>";
			if(otherShip->OfficerBerths() || otherShip->CrewBerths() || otherShip->TroopBerths() || otherShip->PassengerBerths())
			{
				// Berth Occupancy
				desc += "<td>";
				desc += tr("<b>Berths</b>");
				desc += "<div style=\"margin-left: 10px;\">";
				if(otherShip->OfficerBerths())
					desc += tr("Officer: %1 of %2 occupied<br>")
						.arg(otherShip->OfficerBerthsOccupied()).arg(otherShip->OfficerBerths());
				if(otherShip->CrewBerths())
					desc += tr("Crew: %1 of %2 occupied<br>")
						.arg(otherShip->CrewBerthsOccupied()).arg(otherShip->CrewBerths());
				if(otherShip->TroopBerths())
					desc += tr("Troop: %1 of %2 occupied<br>")
						.arg(otherShip->TroopBerthsOccupied()).arg(otherShip->TroopBerths());
				if(otherShip->PassengerBerths())
					desc += tr("Passenger: %1 of %2 occupied<br>")
						.arg(otherShip->PassengerBerthsOccupied()).arg(otherShip->PassengerBerths());
				desc = desc.left(desc.length()-4);	// Drop the trailing <br>
				desc += "</div>";
				desc += "</td>";
			}
			{
				// Cargo
				desc += "<td>";
				desc += tr("<b>Cargo</b>");
				desc += "<div style=\"margin-left: 10px;\">";
				desc += tr("Hold: %L1 of %L2 used<br>")
					.arg(otherShip->CargoTotalVolume())
					.arg(otherShip->CargoCapacityVolume());
				desc += tr("Fuel Cell: %L1 of %L2 used")
					.arg((int)ceil(otherShip->FuelQuantity()))
					.arg((int)otherShip->FuelCapacity());
				desc += "</div>";
				desc += "</td>";
			}
			desc += "</tr>";
			desc += "</tbody></table>";
			desc += QString("</html>");
			m_OtherDesc->setText(desc);

			// Show money aboard the other ship.
			m_OtherMoneyAmt = otherShip->AccountBalance();
			m_OtherMoney->setText(QString("%3 %L1%2").arg(m_OtherMoneyAmt,0,'f',0).arg(g_Au.CurrencySymbol())
					.arg(g_Au.DoubleAngleLeftSymbol()));
			m_OtherMoney->setEnabled(true);

			m_OtherItems->setEnabled(true);

			// Show citizens aboard the other ship.
			//ListCitizens(m_OtherCitizens, otherShip);	// Only lists citizens that are in my scene. :/
			m_OtherCitizens->setEnabled(true);

			// Show cargo aboard the other ship.
			ListCargo(m_OtherCargo, otherShip);
			m_OtherCargo->setEnabled(true);
		}
		else
		{
			m_OtherName->setText(tr("<html>No Other Ship Selected</html>"));
			m_OtherShipId = AuId(0);
			m_OtherDesc->clear();

			m_OtherCargo->clear();
			m_OtherCargo->setEnabled(false);
			//m_OtherCargoDesc->setHtml("");

			m_OtherMoneyAmt = 0;
			m_OtherMoney->setText(QString("%3 %2").arg(g_Au.CurrencySymbol()).arg(g_Au.DoubleAngleLeftSymbol()));
			m_OtherMoney->setEnabled(false);

			m_OtherItems->setEnabled(false);

			m_OtherCitizens->clearContents();
			m_OtherCitizens->setEnabled(false);
		}
	}
	pView->UnlockSceneAtom();

	if(requestCitizensMyShip)
		RequestMyCitizens();

	if(requestCitizensOtherShip)
		RequestOtherCitizens();
}

void AuCargoTransfer::ListCitizens(QTableWidget* table, const DaVessel* ship)
{
	// Start by getting a list of the citizens already in the table.
	AuIdList citizensInTable;
	for(int row=0; row < table->rowCount(); row++)
	{
		QTableWidgetItem* citItem0 = table->item(row, 0);
		if(citItem0)
			citizensInTable.append(AuId(citItem0->data(Qt::UserRole).toInt()));
	}

	// Get a list of all the citizens aboard who could be transferred.
	DaAtomList citizensAboard;
	ship->FindCrewRecursive(ship->Id(), citizensAboard);
	ship->FindOfficersRecursive(ship->Id(), citizensAboard);
	ship->FindPassengersRecursive(ship->Id(), citizensAboard);
	ship->FindTroopsRecursive(ship->Id(), citizensAboard);

	// We will work off the list of their ids due to a crash when transferring an officer to a station. I think the station was
	// out of 'scene' range for people aboard it so transferring the officer removed it from the local scene. At the next scene
	// sync, the corresponding atom is removed from the scene's atom data. I think that happens while updating the UI.
	AuIdList idsAboard;
	for(DaAtomList::const_iterator pai = citizensAboard.begin(); pai != citizensAboard.end(); pai++)
		idsAboard.append((*pai)->Id());

	// List citizens who are not already in the table.
	// Remove citizens from citizensInTable who should remain in the table, so we can delete the ones who are gone.
	table->setSortingEnabled(false);
	for(AuIdList::const_iterator ii = idsAboard.cbegin(); ii != idsAboard.cend(); ii++)
	{
		// Locate the citizen atom using its id. That way we won't die if it is no longer in the scene.
		// We use a non-recursive search so we don't grab citizens off of subordinate vessels.
		const DaCitizen* citizen = dynamic_cast<const DaCitizen*>(ship->FindNormalSubatomId(*ii));
		if(!citizen)
			continue;

		// Get the name of the specie, or a generic name when the specie is not named.
		QString specieName = citizen->SpeciesName();

		// Name
		QString name = tr("%1<br>%2 %3").arg(citizen->Name()).arg(specieName).arg(citizen->FemaleMale(true));

		// Promotion Goal
		if(citizen->PromotionGoal())
			name += tr("<br>%L1xp.<br>Next Promotion at %L2xp.")
				.arg(citizen->JobExperiencePoints()).arg(citizen->PromotionGoal());
		else
			name += tr("<br>%L1xp").arg(citizen->JobExperiencePoints());

		// Rank icon
		QIcon icon = QIcon(citizen->RankIcon());

		// Status
		QString status, statusShort;
		{
			if(citizen->CurrentHitPoints() < citizen->MaxHitPoints())
				status = tr("%1/%2hp").arg(citizen->CurrentHitPoints()).arg(citizen->MaxHitPoints());
			else
				status = tr("%1hp").arg(citizen->CurrentHitPoints());
			statusShort = status;
			if(citizen->CurrentHitPoints() <= 0)
				status = tr("Dying") + " " + status;
			else if(citizen->CurrentHitPoints() < citizen->MaxHitPoints())
				status = tr("Wounded") + " " + status;
			else
				status = tr("Unhurt") + " " + status;

			// Determine combat armor factors based on current armor.
			float baseAbsorb, baseBlock, auxAbsorb, auxBlock;
			citizen->CombatArmorEffectiveness(WM_Contact, WE_Generic, false, AuPoint(0,0), AuPoint(0,0), baseAbsorb, baseBlock,
					auxAbsorb, auxBlock);
			float absorbK = baseAbsorb + auxAbsorb;
			float blockK = baseBlock + auxBlock;
			status += tr("<br>Armor: %1")
				.arg(g_Au.FormatArmorFactors(absorbK, blockK));

			status += tr("<br>Alive Since %1").arg(citizen->CreatedTime().UTC().toLocalTime().toString());
		}

		// Add the citizen to the roster on the dialog.
		if(citizensInTable.contains(citizen->Id()))
		{
			// The citizen is already in the table so just update the contents.
			citizensInTable.Remove(citizen->Id());				// Remove their id from the list so they are not deleted at the end.

			for(int row=0; row<table->rowCount(); row++)
			{
				QTableWidgetItem* citItem0 = table->item(row, 0);
				if(citItem0 && citizen->Is(AuId(citItem0->data(Qt::UserRole).toInt())))
				{
					// Found the citizen's row in the table.
					// Update the fields.
					citItem0->setIcon(icon);
					citItem0->setText(citizen->Name());
					citItem0->setToolTip(name);

					int col=1;
					QTableWidgetItem* citItem1 = table->item(row, col++);
					if(citItem1)
					{
						citItem1->setText(statusShort);
						citItem1->setToolTip(status);
					}
					break;
				}
			}
		}
		else
		{
			// The citizen is not in the table so insert a new record.
			table->insertRow(0);

			int col=0;
			QTableWidgetItem* twname = new QTableWidgetItem(icon, citizen->Name());
			twname->setData(Qt::UserRole, citizen->Id().GetID());
			twname->setToolTip(name);
			table->setItem(0,col++, twname);

			QTableWidgetItem* twstat = new QTableWidgetItem(statusShort);
			twstat->setData(Qt::UserRole, citizen->AtomType());
			twstat->setToolTip(status);
			table->setItem(0,col++, twstat);
		}
	}
	table->setSortingEnabled(true);

	// Finally, remove entries from the table that are still in our citizensInTable list.
	// Those citizens were not found aboard.
	for(AuIdList::const_iterator piti = citizensInTable.cbegin(); piti != citizensInTable.cend(); piti++)
	{
		for(int row=0; row<table->rowCount(); row++)
		{
			QTableWidgetItem* citItem0 = table->item(row, 0);
			if(!citItem0 || AuId(citItem0->data(Qt::UserRole).toInt()) == *piti)
			{
				// Found the citizen's row in the table. Remove it.
				table->removeRow(row);
				break;
			}
		}
	}
}

void AuCargoTransfer::ListCargo(QListWidget* list, const DaVessel* ship)
{
	// Start by getting a list of the cargo already in the list.
	// ListCargoItem removes cargo from cargoInList that should remain in the list, so we can delete the ones that are gone.
	AuItemHash cargoInList;
	for(int row=0; row < list->count(); row++)
	{
		QListWidgetItem* cargoItem = list->item(row);
		if(cargoItem)
		{
			AuItem* item = AuItem::Unpack(cargoItem->data(Qt::UserRole).toByteArray());
			cargoInList.insert(pair<COMMODITYID, AuItem*>(item->CommodityId(), item));
		}
	}

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_GearFilter);

	// List cargo that is not already in the list.
	list->setSortingEnabled(false);
	const AuItemHash& cr = ship->LockCargoForRead();
	for(AuItemHash::const_iterator it = cr.begin(); it != cr.end(); it++)
		if(cidFilter.contains((*it).second->CommodityId()))
			ListCargoItem(list, *(*it).second, cargoInList);
	ship->UnlockCargo();

	// List Hydrogen fuel.
	int fuelQty = (int)ship->FuelQuantity();
	if(fuelQty && cidFilter.contains(CID_Hydrogen))
	{
		unsigned int fuelQual = ship->FuelQuality();
		AuItem* fuelItem = AuItem::Create(CID_Hydrogen, fuelQual, ship->EmpireId(), fuelQty);
		if(fuelItem)
		{
			ListCargoItem(list, *fuelItem, cargoInList);
			delete fuelItem;
		}
	}

	list->setSortingEnabled(true);
	list->sortItems();

	// Finally, remove entries from the list that are still in our cargoInList list.
	// That cargo was not found aboard.
	for(AuItemHash::const_iterator cili = cargoInList.begin(); cili != cargoInList.end(); cili++)
	{
		QByteArray bacili = (*cili).second->Pack();
		for(int row=0; row<list->count(); row++)
		{
			QListWidgetItem* cargoItem = list->item(row);
			if(cargoItem && cargoItem->data(Qt::UserRole).toByteArray() == bacili)
			{
				// Found the cargo's row in the list. Remove it.
				delete list->takeItem(row);
				break;
			}
		}
	}
}

// Removes cargo from cargoInList that should remain in the list, so we can delete the ones that are gone.
void AuCargoTransfer::ListCargoItem(QListWidget* list, const AuItem& item, AuItemHash& cargoInList)
{
	// Add the cargo item to the list on the dialog.
	if(cargoInList.ContainsItem(item))
	{
		// The cargo is already in the list.
		cargoInList.RemoveItem(item);							// Remove the item from the list so it is not deleted at the end.
	}
	else
	{
		// The cargo is not in the list so insert a new record.
		QString text = item.ListDesc();

		QPixmap px32 = QPixmap(item.BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QIcon icon(px32);

		QListWidgetItem* lwi = new QListWidgetItem(icon, text, list);
		lwi->setData(Qt::UserRole, item.Pack());
		lwi->setToolTip(item.HtmlToolTip());
	}
}

void AuCargoTransfer::SendCargo()
{
	AuItemHash cargo;
	for(int row=0; row<m_MyCargo->count(); row++)
	{
		QListWidgetItem* cargoItem = m_MyCargo->item(row);
		if(cargoItem && cargoItem->isSelected())
		{
			AuItem* item = AuItem::Unpack(cargoItem->data(Qt::UserRole).toByteArray());
			cargo.insert(pair<COMMODITYID, AuItem*>(item->CommodityId(), item));
		}
	}

	auApp->TransferSendCargo(m_OtherShipId, cargo);
}

void AuCargoTransfer::SendCargoItem(QListWidgetItem* cargoItem)
{
	if(!cargoItem)
		return;

	// Ask the user how much of that cargo to transfer.
	AuItem* item = AuItem::Unpack(cargoItem->data(Qt::UserRole).toByteArray());
	if(!item)
		return;

	quint32 amountToTransfer = item->Count();
	bool ok = true;
	if(item->Count() > 1)
		amountToTransfer = (quint32)QInputDialog::getInt(this, g_Au.ApplicationName(),
				tr("Enter the amount of %1 to send.").arg(item->ListDesc()), item->Count(), 1, item->Count(), 1, &ok);
	if(ok && amountToTransfer)
	{
		// We got a value that is acceptable. Transfer the cargo.
		item->SetCount(amountToTransfer);
		auApp->TransferSendCargoItem(m_OtherShipId, *item);
	}

	delete item;
}

void AuCargoTransfer::SendCitizen(QTableWidgetItem* item)
{
	if(!item)
		return;

	// Get the id and type of the citizen that was double-clicked.
	QTableWidgetItem* citItem0 = m_MyCitizens->item(item->row(), 0);
	if(!citItem0)
		return;
	QTableWidgetItem* citItem1 = m_MyCitizens->item(item->row(), 1);
	if(!citItem1)
		return;
	AuId citizenId = AuId(citItem0->data(Qt::UserRole).toInt());
	int citizenType = citItem1->data(Qt::UserRole).toInt();

	// Locate a selected citizen on the other side of the same type for swap if needed.
	AuId swapId;
	for(int row=0; row<m_OtherCitizens->rowCount(); row++)
	{
		QTableWidgetItem* ocitItem0 = m_OtherCitizens->item(row, 0);
		if(!ocitItem0)
			continue;
		QTableWidgetItem* ocitItem1 = m_OtherCitizens->item(row, 1);
		if(!ocitItem1)
			continue;

		if(ocitItem0->isSelected() && ocitItem1->data(Qt::UserRole) == citizenType)
		{
			swapId = AuId(ocitItem0->data(Qt::UserRole).toInt());
			break;
		}
	}

	auApp->TransferSendCitizen(m_OtherShipId, citizenId, swapId);
}

void AuCargoTransfer::SendCitizens()
{
	// List the ids of the selected citizens.
	AuIdList citizenIds;
	for(int row=0; row<m_MyCitizens->rowCount(); row++)
	{
		QTableWidgetItem* citItem0 = m_MyCitizens->item(row, 0);
		if(citItem0 && citItem0->isSelected())
			citizenIds.append(AuId(citItem0->data(Qt::UserRole).toInt()));
	}

	// List the ids of the selected other citizens, for swapping if needed.
	AuIdList swapIds;
	for(int row=0; row<m_OtherCitizens->rowCount(); row++)
	{
		QTableWidgetItem* ocitItem0 = m_OtherCitizens->item(row, 0);
		if(ocitItem0 && ocitItem0->isSelected())
			swapIds.append(AuId(ocitItem0->data(Qt::UserRole).toInt()));
	}

	auApp->TransferSendCitizens(m_OtherShipId, citizenIds, swapIds);
	RequestMyCitizens();
	RequestOtherCitizens();
}

void AuCargoTransfer::SendMoney()
{
	// Ask the user how much of that money to transfer.
	bool ok;
	double amountToTransfer = QInputDialog::getDouble(this, g_Au.ApplicationName(),
			tr("Enter the amount of money to send."), m_MyMoneyAmt, 1, m_MyMoneyAmt, 0, &ok);
	if(ok)
	{
		// We got a value that is acceptable. Transfer the money.
		auApp->TransferSendMoney(m_OtherShipId, amountToTransfer);
	}
}

void AuCargoTransfer::TakeCargo()
{
	AuItemHash cargo;
	for(int row=0; row<m_OtherCargo->count(); row++)
	{
		QListWidgetItem* cargoItem = m_OtherCargo->item(row);
		if(cargoItem && cargoItem->isSelected())
		{
			AuItem* item = AuItem::Unpack(cargoItem->data(Qt::UserRole).toByteArray());
			cargo.insert(pair<COMMODITYID, AuItem*>(item->CommodityId(), item));
		}
	}

	auApp->TransferTakeCargo(m_OtherShipId, cargo);
}

void AuCargoTransfer::TakeCargoItem(QListWidgetItem* cargoItem)
{
	if(!cargoItem)
		return;

	// Ask the user how much of that cargo to transfer.
	AuItem* item = AuItem::Unpack(cargoItem->data(Qt::UserRole).toByteArray());
	if(!item)
		return;

	quint32 amountToTransfer = item->Count();
	bool ok = true;
	if(item->Count() > 1)
		amountToTransfer = (quint32)QInputDialog::getInt(this, g_Au.ApplicationName(),
				tr("Enter the amount of %1 to take.").arg(item->ListDesc()), item->Count(), 1, item->Count(), 1, &ok);
	if(ok && amountToTransfer)
	{
		// We got a value that is acceptable. Transfer the cargo.
		item->SetCount(amountToTransfer);
		auApp->TransferTakeCargoItem(m_OtherShipId, *item);
	}

	delete item;
}

void AuCargoTransfer::TakeCitizen(QTableWidgetItem* item)
{
	if(!item)
		return;

	// Get the id and type of the citizen that was double-clicked.
	QTableWidgetItem* ocitItem0 = m_OtherCitizens->item(item->row(), 0);
	if(!ocitItem0)
		return;
	QTableWidgetItem* ocitItem1 = m_OtherCitizens->item(item->row(), 1);
	if(!ocitItem1)
		return;

	AuId citizenId = AuId(ocitItem0->data(Qt::UserRole).toInt());
	int citizenType = ocitItem1->data(Qt::UserRole).toInt();

	// Locate a selected citizen on the other side of the same type for swap if needed.
	AuId swapId;
	for(int row=0; row<m_MyCitizens->rowCount(); row++)
	{
		QTableWidgetItem* citItem0 = m_MyCitizens->item(row, 0);
		if(!citItem0)
			continue;
		QTableWidgetItem* citItem1 = m_MyCitizens->item(row, 1);
		if(!citItem1)
			continue;

		if(citItem0->isSelected() && citItem1->data(Qt::UserRole) == citizenType)
		{
			swapId = AuId(citItem0->data(Qt::UserRole).toInt());
			break;
		}
	}

	auApp->TransferTakeCitizen(m_OtherShipId, citizenId, swapId);
}

void AuCargoTransfer::TakeCitizens()
{
	// List the ids of the selected citizens.
	AuIdList citizenIds;
	for(int row=0; row<m_OtherCitizens->rowCount(); row++)
	{
		QTableWidgetItem* ocitItem0 = m_OtherCitizens->item(row, 0);
		if(ocitItem0 && ocitItem0->isSelected())
			citizenIds.append(AuId(ocitItem0->data(Qt::UserRole).toInt()));
	}

	// List the ids of the selected other citizens, for swapping if needed.
	AuIdList swapIds;
	for(int row=0; row<m_MyCitizens->rowCount(); row++)
	{
		QTableWidgetItem* citItem0 = m_MyCitizens->item(row, 0);
		if(citItem0->isSelected())
			swapIds.append(AuId(citItem0->data(Qt::UserRole).toInt()));
	}

	auApp->TransferTakeCitizens(m_OtherShipId, citizenIds, swapIds);
	RequestMyCitizens();
	RequestOtherCitizens();
}

void AuCargoTransfer::TakeMoney()
{
	// Ask the user how much of that money to transfer.
	bool ok;
	double amountToTransfer = QInputDialog::getDouble(this, g_Au.ApplicationName(),
			tr("Enter the amount of money to take."), m_OtherMoneyAmt, 1, m_OtherMoneyAmt, 0, &ok);
	if(ok)
	{
		// We got a value that is acceptable. Transfer the money.
		auApp->TransferTakeMoney(m_OtherShipId, amountToTransfer);
	}
}

void AuCargoTransfer::SendSomeCargo()
{
	for(int row=0; row<m_MyCargo->count(); row++)
	{
		QListWidgetItem* cargoItem = m_MyCargo->item(row);
		if(cargoItem && cargoItem->isSelected())
		{
			SendCargoItem(cargoItem);
			break;
		}
	}
}

void AuCargoTransfer::TakeSomeCargo()
{
	for(int row=0; row<m_OtherCargo->count(); row++)
	{
		QListWidgetItem* cargoItem = m_OtherCargo->item(row);
		if(cargoItem && cargoItem->isSelected())
		{
			TakeCargoItem(cargoItem);
			break;
		}
	}
}

void AuCargoTransfer::MyCargoSelected()
{
	//QString cargotext;
	QListWidgetItem* curcargo = m_MyCargo->currentItem();
	if(curcargo)
	{
		AuItem* item = AuItem::Unpack(curcargo->data(Qt::UserRole).toByteArray());
		if(item)
		{
			/*
			QSize packSize = item->BitmapSizeToFit(100);
			cargotext = QString("<html>"
					"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
					"</html>")
				.arg(item->BitmapName())
				.arg(item->HtmlDesc())
				.arg(packSize.width())
				.arg(packSize.height());
				*/
			delete item;
		}
	}
	//m_MyCargoDesc->setHtml(cargotext);
}

void AuCargoTransfer::OtherCargoSelected()
{
	//QString cargotext;
	QListWidgetItem* curcargo = m_OtherCargo->currentItem();
	if(curcargo)
	{
		AuItem* item = AuItem::Unpack(curcargo->data(Qt::UserRole).toByteArray());
		if(item)
		{
			/*
			QSize packSize = item->BitmapSizeToFit(100);
			cargotext = QString("<html>"
					"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
					"</html>")
				.arg(item->BitmapName())
				.arg(item->HtmlDesc())
				.arg(packSize.width())
				.arg(packSize.height());
				*/
			delete item;
		}
	}
	//m_OtherCargoDesc->setHtml(cargotext);
}

void AuCargoTransfer::Filter(COMMODITYFILTER filter)
{
	m_GearFilter = filter;
	UpdateContents();
}

void AuCargoTransfer::SendItems()
{
	QString msg = tr("<html><b>Send Items</b>"
			"<p>"
			"This function is useful when moving items to a ship, that has a design similar to this ship's design. "
			"When items are transferred to the other spacecraft, their relative positions do not change. "
			"If the other ship's design is not the same as this ship, items could move to inaccessible locations aboard that ship. "
			"</p>"
			"</html>");
	QMessageBox mb(QMessageBox::NoIcon, g_Au.ApplicationName(), msg, QMessageBox::Cancel, this);
	mb.setIconPixmap(QPixmap(auApp->Icon32()));
	QAbstractButton* sendAll = mb.addButton(tr("Send All Items"), QMessageBox::ActionRole);
	QAbstractButton* sendStatic = mb.addButton(tr("Send Only Static Display Items"), QMessageBox::ActionRole);
	QAbstractButton* sendLoose = mb.addButton(tr("Send Only Loose Items"), QMessageBox::ActionRole);
	mb.exec();
	if(mb.clickedButton() == sendAll)
		auApp->TransferSendItemsAll(m_OtherShipId);
	else if(mb.clickedButton() == sendStatic)
		auApp->TransferSendItemsStatic(m_OtherShipId);
	else if(mb.clickedButton() == sendLoose)
		auApp->TransferSendItemsLoose(m_OtherShipId);
}

void AuCargoTransfer::TakeItems()
{
	QString msg = tr("<html><b>Take Items</b>"
			"<p>"
			"This function is useful when moving items from a ship, that has a design similar to this ship's design. "
			"When items are transferred from the other spacecraft, their relative positions do not change. "
			"If the other ship's design is not the same as this ship, items could move to inaccessible locations aboard this ship. "
			"</p>"
			"</html>");
	QMessageBox mb(QMessageBox::NoIcon, g_Au.ApplicationName(), msg, QMessageBox::Cancel, this);
	mb.setIconPixmap(QPixmap(auApp->Icon32()));
	QAbstractButton* takeAll = mb.addButton(tr("Take All Items"), QMessageBox::ActionRole);
	QAbstractButton* takeStatic = mb.addButton(tr("Take Only Static Display Items"), QMessageBox::ActionRole);
	QAbstractButton* takeLoose = mb.addButton(tr("Take Only Loose Items"), QMessageBox::ActionRole);
	mb.exec();
	if(mb.clickedButton() == takeAll)
		auApp->TransferTakeItemsAll(m_OtherShipId);
	else if(mb.clickedButton() == takeStatic)
		auApp->TransferTakeItemsAll(m_OtherShipId);
	else if(mb.clickedButton() == takeLoose)
		auApp->TransferTakeItemsAll(m_OtherShipId);
}

void AuCargoTransfer::TransferCitizensAvailable(const QByteArray& ba)
{
	// Decode the list of all the citizens aboard who could be transferred.
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	AuId shipId;
	in >> shipId;

	// Based on ship id, decide which table we are updating.
	QTableWidget* table;
	if(shipId == m_MyShipId)
		table = m_MyCitizens;
	else
		table = m_OtherCitizens;

	// Start by getting a list of the citizens already in the table.
	AuIdList citizensInTable;
	for(int row=0; row < table->rowCount(); row++)
	{
		QTableWidgetItem* citItem0 = table->item(row, 0);
		if(citItem0)
			citizensInTable.append(AuId(citItem0->data(Qt::UserRole).toInt()));
	}

	// List citizens who are not already in the table.
	if(shipId == m_MyShipId || shipId == m_OtherShipId)
	{
		// The list is appropriate for my ship id or the other ship id.
		table->setSortingEnabled(false);
		AuId citizenId;
		do
		{
			in >> citizenId;
			if(!citizenId.IsNull())
			{
				QString name, desc, icon, status, statusShort;
				quint8 atomType;
				in >> name;
				in >> atomType;
				in >> desc;
				in >> icon;
				in >> status;
				in >> statusShort;

				// Add the citizen to the roster on the dialog.
				if(citizensInTable.contains(citizenId))
				{
					// The citizen is already in the table so just update the contents.
					// Remove citizens from citizensInTable who should remain in the table, so we can delete the ones who are gone.
					citizensInTable.Remove(citizenId);			// Remove their id from the list so they are not deleted at the end.

					for(int row=0; row<table->rowCount(); row++)
					{
						QTableWidgetItem* citItem0 = table->item(row, 0);
						if(citItem0 && citizenId == AuId(citItem0->data(Qt::UserRole).toInt()))
						{
							// Found the citizen's row in the table.
							// Update the fields.
							citItem0->setIcon(QIcon(icon));
							citItem0->setText(name);
							citItem0->setToolTip(name + desc);

							int col=1;
							QTableWidgetItem* citItem1 = table->item(row, col++);
							if(citItem1)
							{
								citItem1->setText(statusShort);
								citItem1->setToolTip(status);
							}
							break;
						}
					}
				}
				else
				{
					// The citizen is not in the table so insert a new record.
					table->insertRow(0);

					int col=0;
					QTableWidgetItem* twname = new QTableWidgetItem(QIcon(icon), name);
					twname->setData(Qt::UserRole, citizenId.GetID());
					twname->setToolTip(name + desc);
					table->setItem(0,col++, twname);

					QTableWidgetItem* twstat = new QTableWidgetItem(statusShort);
					twstat->setData(Qt::UserRole, atomType);
					twstat->setToolTip(status);
					table->setItem(0,col++, twstat);
				}
			}
		}
		while(!citizenId.IsNull());
		table->setSortingEnabled(true);
	}

	// Finally, remove entries from the table that are still in our citizensInTable list.
	// Those citizens were not found aboard or they belong to a different ship than the one displayed.
	for(AuIdList::const_iterator piti = citizensInTable.cbegin(); piti != citizensInTable.cend(); piti++)
	{
		for(int row=0; row<table->rowCount(); row++)
		{
			QTableWidgetItem* citItem0 = table->item(row, 0);
			if(!citItem0 || AuId(citItem0->data(Qt::UserRole).toInt()) == *piti)
			{
				// Found the citizen's row in the table. Remove it.
				table->removeRow(row);
				break;
			}
		}
	}
}

void AuCargoTransfer::RequestMyCitizens()
{
	auApp->TransferRequestCitizens(m_MyShipId);
}

void AuCargoTransfer::RequestOtherCitizens()
{
	auApp->TransferRequestCitizens(m_OtherShipId);
}

QSize AuCargoTransfer::sizeHint() const
{
	return QSize(800,600);
}

