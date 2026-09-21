//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftberths.cpp,v 1.12 2022/05/04 21:08:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftberths.h"
//
#include "auapp.h"
#include "auchristendialog.h"
#include "aucommpacket.h"
#include "auframe.h"
#include "auglobal.h"
#include "auroster.h"
#include "auspecies.h"
#include "autoolbutton.h"
#include "auview.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTimer>

void AuSpacecraftRosterDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftRosterUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftBerths::AuSpacecraftBerths(QWidget* parent) : QWidget(parent)
{
	m_CrewBerths = 0;
	m_PassengerBerths = 0;
	m_PersonGalaxy = GALAXY_Count;
	m_TroopBerths = 0;
	m_timer = 0;

	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	lwavail = new QListWidget;
	lwavail->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwavail->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwavail->setSelectionMode(QAbstractItemView::SingleSelection);
	lwavail->setSortingEnabled(false);
	lwavail->setToolTip(tr("List of available berths in the roster."));
	lwavail->setMinimumHeight(16);
	connect(lwavail, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));

	twroster = new QTableWidget(0,2);
	{
		twroster->setEditTriggers(QAbstractItemView::NoEditTriggers);
		twroster->setSelectionBehavior(QAbstractItemView::SelectRows);
		twroster->setSelectionMode(QAbstractItemView::SingleSelection);
		twroster->setShowGrid(false);
		twroster->setColumnCount(2);
		twroster->setColumnWidth(0,200);
		twroster->setSortingEnabled(true);
		twroster->sortByColumn(0, Qt::AscendingOrder);
		twroster->setToolTip(tr("List of occupied berths in the roster."));
		twroster->setMinimumHeight(32);
		connect(twroster, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));

		QHeaderView* hhdr = twroster->horizontalHeader();
		hhdr->setSectionResizeMode(0, QHeaderView::Interactive);
		hhdr->setSectionResizeMode(1, QHeaderView::Stretch);
		QStringList headerLabels;
		headerLabels << "Occupant";
		headerLabels << "Berth";
		twroster->setHorizontalHeaderLabels(headerLabels);

		QHeaderView* vhdr = twroster->verticalHeader();
		vhdr->setDefaultSectionSize(18);
		vhdr->hide();
	}

	twcryo = new QTableWidget(0,2);
	{
		twcryo->setEditTriggers(QAbstractItemView::NoEditTriggers);
		twcryo->setSelectionBehavior(QAbstractItemView::SelectRows);
		twcryo->setSelectionMode(QAbstractItemView::SingleSelection);
		twcryo->setShowGrid(false);
		twcryo->setColumnCount(2);
		twcryo->setColumnWidth(0,200);
		twcryo->setSortingEnabled(true);
		twcryo->sortByColumn(0, Qt::AscendingOrder);
		twcryo->setToolTip(tr("List of occupied cryo berths."));
		twcryo->setMinimumHeight(32);
		connect(twcryo, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));

		QHeaderView* hhdr = twcryo->horizontalHeader();
		hhdr->setSectionResizeMode(0, QHeaderView::Interactive);
		hhdr->setSectionResizeMode(1, QHeaderView::Stretch);
		QStringList headerLabels;
		headerLabels << "Cryo Travellers";
		headerLabels << "Count";
		twcryo->setHorizontalHeaderLabels(headerLabels);

		QHeaderView* vhdr = twcryo->verticalHeader();
		vhdr->setDefaultSectionSize(18);
		vhdr->hide();
	}

	QLabel* lbavail = new QLabel(tr("Available Berths"));
	QLabel* lbroster = new QLabel(tr("Occupied Berths"));
	lbcryo = new QLabel(tr("Cryo Berths"));

	pbrequest = new AuIconButton(QIcon(":data/RequestBerth.png"));
	pbrequest->setToolTip(tr("<b>Request Berth</b><br>"
				"Requests the selected berth. "
				"The Captain's berth is usually granted upon request. "
				"Requests for officer berths must be approved by the Captain. "
				"Policy settings determine if Crew, Passenger and Troop requests are accepted automatically "
				"or must be approved by a person in the ship's chain-of-command."));
	connect(pbrequest, SIGNAL(clicked()), SLOT(RequestSlot()));

	pbkick = new AuIconButton(QIcon(":data/CommKick.png"));
	if(isProgrammer)
		pbkick->setToolTip(tr("<html><b>Kick Occupant</b><br>"
					"Removes the selected person from the roster.<br>"
					"Only a person in the ship's chain-of-command may perform this action. "
					"Only the Captain may kick a fellow officer.<br>"
					"<b>Architect Power</b> Any roster position can be kicked <i>when architect powers are enabled</i>.</html>"));
	else if(isAgent)
		pbkick->setToolTip(tr("<html><b>Kick Occupant</b><br>"
					"Removes the selected person from the roster.<br>"
					"Only a person in the ship's chain-of-command may perform this action. "
					"Only the Captain may kick a fellow officer.<br>"
					"<b>Agent Power</b> Any roster position except captain of a company ship can be kicked "
					"<i>when agent powers are enabled</i>.</html>"));
	else
		pbkick->setToolTip(tr("<html><b>Kick Occupant</b><br>"
					"Removes the selected person from the roster.<br>"
					"Only a person in the ship's chain-of-command may perform this action. "
					"Only the Captain may kick a fellow officer.</html>"));
	connect(pbkick, SIGNAL(clicked()), SLOT(KickOccupant()));

	pbrecon = new AuIconButton(QIcon(":data/CommCrew.png"));
	if(isAgent)
		pbrecon->setToolTip(tr("<b>Reconcile Roster and Kick Stowaways</b><br>"
					"Reconciles the roster by kicking <i>crew</i>, <i>passengers</i>, <i>troops</i>, and <i>AI officers</i> from "
					"the roster who are not aboard the ship. "
					"<b>Does not remove the <i>Captain</i> or <i>human officers</i> from the roster, "
					"even if they aren't aboard.</b><br>"
					"Kicks stowaways from the ship by forcing off-line players who are not on the roster to recall home.<br>"
					"Only a person in the ship's chain-of-command may reconcile the roster and kick stowaways.<br>"
					"<b>Agent Power</b> Agents can kick stowaways from any ship. Agents kick all stowaways, "
					"whether or not they are on line. "
					"Agents are not considered stowaways while their agent powers are enabled."));
	else
		pbrecon->setToolTip(tr("<b>Reconcile Roster and Kick Stowaways</b><br>"
					"Reconciles the roster by kicking <i>crew</i>, <i>passengers</i>, <i>troops</i>, and <i>AI officers</i> from "
					"the roster who are not aboard the ship. "
					"<b>Does not remove the <i>Captain</i> or <i>human officers</i> from the roster, "
					"even if they aren't aboard.</b><br>"
					"Kicks stowaways from the ship by forcing off-line players who are not on the roster to recall home.<br>"
					"Only a person in the ship's chain-of-command may reconcile the roster and kick stowaways."));
	connect(pbrecon, SIGNAL(clicked()), SLOT(Reconcile()));

	pbkickcryo = new AuIconButton(QIcon(":data/CommKickCryo.png"));
	if(isAgent)
		pbkickcryo->setToolTip(tr("<html><b>Kick Cryo Travellers...</b><br>"
					"Removes one or more travellers from cryo berths.<br>"
					"Only a person in the ship's chain-of-command may perform this action.<br>"
					"<b>Agent Power</b> Any cryo traveller can be kicked <i>when agent powers are enabled</i>.</html>"));
	else
		pbkickcryo->setToolTip(tr("<html><b>Kill Cryo Travellers...</b><br>"
					"Removes one or more travellers from cryo berths.<br>"
					"Only a person in the ship's chain-of-command may perform this action. "
					"</html>"));
	connect(pbkickcryo, SIGNAL(clicked()), SLOT(KickCryoOccupant()));

	pbrevive = new AuIconButton(QIcon(":data/c_MedKit.png"));
	pbrevive->setToolTip(tr("<html><b>Revive Cryo Traveller...</b><br>"
				"Revives one of the selected travellers from their cryo berth. "
				"<br>"
				"The cryo traveller emerges with no equipment, standing at your location. "
				"You can choose if they become a crew member, a troop or a passenger. "
				"An appropriate empty berth must be available. "
				"<br>"
				"Only a person in the ship's chain-of-command may perform this action. "
				"</html>"));
	connect(pbrevive, SIGNAL(clicked()), SLOT(ReviveCryoOccupant()));

	// Create a layout for the row of available berth button.
	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lbavail);
	blay->addStretch(1);
	blay->addWidget(pbrequest);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* avlay = new QVBoxLayout;
	avlay->addLayout(blay);
	avlay->addWidget(lwavail);
	avlay->setMargin(0);
	avlay->setSpacing(2);

	QWidget* avwid = new QWidget;
	avwid->setLayout(avlay);

	// Create a layout for the row of occupied berth buttons.
	QHBoxLayout* olay = new QHBoxLayout;
	olay->addWidget(lbroster);
	olay->addStretch(1);
	olay->addWidget(pbkick);
	olay->addWidget(pbrecon);
	olay->setMargin(0);
	olay->setSpacing(2);

	QVBoxLayout* occlay = new QVBoxLayout;
	occlay->addLayout(olay);
	occlay->addWidget(twroster);
	occlay->setMargin(0);
	occlay->setSpacing(2);

	QWidget* occwid = new QWidget;
	occwid->setLayout(occlay);

	// Create a layout for the row of cryo berth buttons.
	QHBoxLayout* clay = new QHBoxLayout;
	clay->addWidget(lbcryo);
	clay->addStretch(1);
	clay->addWidget(pbkickcryo);
	clay->addWidget(pbrevive);
	clay->setMargin(0);
	clay->setSpacing(2);

	QVBoxLayout* cryolay = new QVBoxLayout;
	cryolay->addLayout(clay);
	cryolay->addWidget(twcryo);
	cryolay->setMargin(0);
	cryolay->setSpacing(2);

	QWidget* cryowid = new QWidget;
	cryowid->setLayout(cryolay);

	// Assemble the widgets into a splitter.
	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(avwid);
	m_Splitter->addWidget(occwid);
	m_Splitter->addWidget(cryowid);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	// Final main grid.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	connect(this, SIGNAL(KickBerthOccupant(AuId)), auApp, SLOT(KickBerthOccupant(AuId)));
	connect(this, SIGNAL(KickCryoBerthOccupants(AuSpecies, int)), auApp, SLOT(KickCryoBerthOccupants(AuSpecies, int)));
	connect(this, SIGNAL(ReconcileRoster()), auApp, SLOT(ReconcileRoster()));
	connect(this, SIGNAL(ReviveCryoBerthOccupant(AuSpecies, quint8)), auApp, SLOT(ReviveCryoBerthOccupant(AuSpecies, quint8)));
	connect(this, SIGNAL(SpeciesRequested(AuSpecies)), auApp, SLOT(SpeciesRequested(AuSpecies)));
	connect(this, SIGNAL(StowawaysKicked()), auApp, SLOT(KickStowaways()));

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/SpacecraftRoster/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuSpacecraftBerths::~AuSpacecraftBerths()
{
}

void AuSpacecraftBerths::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/SpacecraftRoster/Splitter", m_Splitter->saveState());
}

void AuSpacecraftBerths::EnableControls()
{
	// Determine if any of our lists have selections.
	bool selAvail = false;
	QItemSelectionModel* sel = lwavail->selectionModel();
	if(sel && sel->hasSelection())
		selAvail = true;

	bool selRoster = false;
	sel = twroster->selectionModel();
	if(sel && sel->hasSelection())
		selRoster = true;

	bool selCryo = false;
	sel = twcryo->selectionModel();
	if(sel && sel->hasSelection())
		selCryo = true;

	// Determine if the person is in the chain-of-command of the ship.
	AuView* pView = auApp->Frame()->View();
	bool inCommand = pView->IsPersonInCommand();		// also true for admins

	// Enable/disable controls.
	pbkick->setEnabled(selRoster && inCommand);
	pbrecon->setEnabled(inCommand);
	pbrequest->setEnabled(selAvail);
	pbrevive->setEnabled(selCryo && inCommand && (m_CrewBerths || m_TroopBerths || m_PassengerBerths));
	pbkickcryo->setEnabled(selCryo && inCommand);
}

void AuSpacecraftBerths::KickOccupant()
{
	QItemSelectionModel* sel = twroster->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		Q_ASSERT(selRowIdxs.count());
		QModelIndex selIdx = selRowIdxs.first();

		// Kick the occupant id.
		AuId kickId(selIdx.data(Qt::UserRole).toUInt());
		emit KickBerthOccupant(kickId);
	}
}

void AuSpacecraftBerths::Reconcile()
{
	emit ReconcileRoster();
	emit StowawaysKicked();
}

void AuSpacecraftBerths::RequestSlot()
{
	QItemSelectionModel* sel = lwavail->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		Q_ASSERT(selRowIdxs.count());
		QModelIndex selIdx = selRowIdxs.first();

		// Get governing credentials of person.
		AuView* pView = auApp->Frame()->View();
		quint32 personEmpire = pView->SceneAtomEmpire();

		// The berth type was stored on the list widget item.
		AuRosterEntry::TYPE type = (AuRosterEntry::TYPE)selIdx.data(Qt::UserRole).toInt();
		switch(type)
		{
		case AuRosterEntry::Invalid:
		case AuRosterEntry::OfficerAI:
		case AuRosterEntry::PassengerAI:
			break;
		case AuRosterEntry::Captain:
			{
				AuCommRequestBerthCaptain msg(personEmpire, AuCommStdChannel::IntercomChannelName(), m_PersonId, m_PersonName,
						m_PersonSector, m_PersonGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				auApp->SendChatMessage(msg);
			}
			break;
		case AuRosterEntry::CaptainSlot:
			{
				// No credentials needed for captain slot.
				AuCommRequestBerthCaptainSlot msg(AuCommStdChannel::IntercomChannelName(), m_PersonId, m_PersonName, m_PersonSector,
						m_PersonGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				auApp->SendChatMessage(msg);
			}
			break;
		case AuRosterEntry::Crew:
			{
				AuCommRequestBerthCrew msg(personEmpire, AuCommStdChannel::IntercomChannelName(), m_PersonId, m_PersonName,
						m_PersonSector, m_PersonGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				auApp->SendChatMessage(msg);
			}
			break;
		case AuRosterEntry::OfficerHuman:
			{
				AuCommRequestBerthOfficer msg(personEmpire, AuCommStdChannel::IntercomChannelName(), m_PersonId, m_PersonName,
						m_PersonSector, m_PersonGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				auApp->SendChatMessage(msg);
			}
			break;
		case AuRosterEntry::PassengerHuman:
			{
				AuCommRequestBerthPassenger msg(personEmpire, AuCommStdChannel::IntercomChannelName(), m_PersonId, m_PersonName,
						m_PersonSector, m_PersonGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				auApp->SendChatMessage(msg);
			}
			break;
		case AuRosterEntry::Troop:
			{
				AuCommRequestBerthTroop msg(personEmpire, AuCommStdChannel::IntercomChannelName(), m_PersonId, m_PersonName,
						m_PersonSector, m_PersonGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				auApp->SendChatMessage(msg);
			}
			break;
		}
	}
}

void AuSpacecraftBerths::showEvent(QShowEvent* event)
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

void AuSpacecraftBerths::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Remember the selected items.
	AuRosterEntry::TYPE selType = AuRosterEntry::Invalid;
	QItemSelectionModel* sel = lwavail->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		Q_ASSERT(selRowIdxs.count());
		QModelIndex selIdx = selRowIdxs.first();
		selType = (AuRosterEntry::TYPE)selIdx.data(Qt::UserRole).toInt();
	}
	AuId selOccupant;
	sel = twroster->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		Q_ASSERT(selRowIdxs.count());
		QModelIndex selIdx = selRowIdxs.first();
		selOccupant = AuId(selIdx.data(Qt::UserRole).toUInt());
	}
	AuSpecies selSpecies;
	sel = twcryo->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		Q_ASSERT(selRowIdxs.count());
		QModelIndex selIdx = selRowIdxs.first();
		selSpecies = selIdx.data(Qt::UserRole).value<AuSpecies>();
	}

	// Clear the current contents.
	lwavail->clear();

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
			// Determine which berths are available.
			if(ship->IsDesignDataCacheValid())
			{
				bool captavail = !ship->CaptainsBerthOccupied();
				unsigned int officeravail = ship->OfficerBerthsAvailable();
				m_CrewBerths = ship->CrewBerthsAvailable();
				m_PassengerBerths = ship->PassengerBerthsAvailable();
				m_TroopBerths = ship->TroopBerthsAvailable();

				// List available berths.
				if(captavail)
				{
					QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::Captain));
					QListWidgetItem* lwi = new QListWidgetItem(icon, "Captain", lwavail);
					lwi->setData(Qt::UserRole, AuRosterEntry::Captain);
					lwi->setToolTip(tr("<b>Captain's Berth</b><br>"
								"The Captain's berth is granted upon request. "
								"The Captain's berth is available even if the ship has no actual berths. "
								"A ship that has officers will only grant the Captain's berth to a person in the ship's "
								"chain-of-command."));
					if(selType == AuRosterEntry::Captain || selType == AuRosterEntry::CaptainSlot)
						lwi->setSelected(true);
				}
				if(officeravail)
				{
					QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::OfficerHuman));
					QString text = tr("%1 Officer Berths").arg(officeravail);
					QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwavail);
					lwi->setData(Qt::UserRole, AuRosterEntry::OfficerHuman);
					lwi->setToolTip(tr("<b>Officer Berth</b><br>"
								"Requests for officer berths must be accepted by the Captain."));
					if(selType == AuRosterEntry::OfficerHuman)
						lwi->setSelected(true);
				}
				if(m_CrewBerths)
				{
					QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::Crew));
					QString text = tr("%1 Crew Berths").arg(m_CrewBerths);
					QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwavail);
					lwi->setData(Qt::UserRole, AuRosterEntry::Crew);
					lwi->setToolTip(tr("<b>Crew Berth</b><br>"
								"Crew berth acceptance policy enables requests to be accepted automatically, otherwise "
								"requests must be accepted by someone in the ship's chain-of-command."));
					if(selType == AuRosterEntry::Crew)
						lwi->setSelected(true);
				}
				if(m_PassengerBerths)
				{
					QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::PassengerHuman));
					QString text = tr("%1 Passenger Berths").arg(m_PassengerBerths);
					QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwavail);
					lwi->setData(Qt::UserRole, AuRosterEntry::PassengerHuman);
					lwi->setToolTip(tr("<b>Passenger Berth</b><br>"
								"Passenger berth acceptance policy enables requests to be accepted automatically, otherwise "
								"requests must be accepted by someone in the ship's chain-of-command."));
					if(selType == AuRosterEntry::PassengerHuman)
						lwi->setSelected(true);
				}
				if(m_TroopBerths)
				{
					QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::Troop));
					QString text = tr("%1 Troop Berths").arg(m_TroopBerths);
					QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwavail);
					lwi->setData(Qt::UserRole, AuRosterEntry::Troop);
					lwi->setToolTip(tr("<b>Troop Berth</b><br>"
								"Troop berth acceptance policy enables requests to be accepted automatically, otherwise "
								"requests must be accepted by someone in the ship's chain-of-command."));
					if(selType == AuRosterEntry::Troop)
						lwi->setSelected(true);
				}
			}

			// Roster
			{
				// Make a list of the atom ids already in the table for reconciling removed atoms at the end.
				// When the update is done, the 'intable' list will contain only ids that weren't updated; those get removed.
				AuIdList intable;
				for(int row=0; row<twroster->rowCount(); row++)
				{
					QTableWidgetItem* item = twroster->item(row, 0);
					if(item)
						intable.append(AuId(item->data(Qt::UserRole).toUInt()));
				}

				// List occupied berths.
				twroster->setSortingEnabled(false);
				twroster->disconnect(SIGNAL(itemSelectionChanged()));
				QTableWidgetItem* twname = 0;
				QTableWidgetItem* twroom = 0;
				const AuRoster& roster = ship->LockRoster();
				for(AuRoster::const_iterator it = roster.constBegin(); it != roster.constEnd(); it++)
				{
					if(intable.contains((*it).m_OccId))
					{
						// Remove the id from the 'intable' list. We'll remove the ones that don't get updated at the end.
						intable.Remove((*it).m_OccId);
						continue;	// No update of the list item is required.
					}

					QString roomName = ship->BerthRoomName(*it);
					if(roomName.isEmpty())
						continue;

					twname = new QTableWidgetItem(QIcon((*it).OccupantIcon()), (*it).OccupantName());
					twroom = new QTableWidgetItem(QIcon((*it).BerthIcon()), roomName);

					// Store atom id on column 0.
					twname->setData(Qt::UserRole, (*it).m_OccId.GetID());

					int row = twroster->rowCount();
					twroster->insertRow(row);

					int col=0;
					twroster->setItem(row,col++, twname);
					twroster->setItem(row,col++, twroom);
					if(selOccupant == (*it).m_OccId)
						twroster->selectRow(row);
				}
				ship->UnlockRoster();

				// Remove rows that no longer belong in the table.
				for(AuIdList::const_iterator ii = intable.constBegin(); ii != intable.constEnd(); ii++)
				{
					for(int row=0; row<twroster->rowCount(); row++)
					{
						QTableWidgetItem* item = twroster->item(row,0);
						if(AuId(item->data(Qt::UserRole).toUInt()) == *ii)
						{
							// Found the unit's row in the table. Remove it.
							twroster->removeRow(row);
							break;
						}
					}
				}
				twroster->resizeRowsToContents();
				twroster->setSortingEnabled(true);
				connect(twroster, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));
			}

			// Cryo Berths
			{
				QString cryoText;
				unsigned int cryoBerths = ship->CryoBerths();
				if(cryoBerths)
				{
					unsigned int cryoocc = ship->CryoBerthsOccupied();
					if(cryoBerths == 1)
						cryoText = tr("1 Cryo Berth, %2").arg(cryoocc?tr("Occupied"):tr("Unoccupied"));
					else if(cryoocc)
						cryoText = tr("%1 Cryo Berths, %2 Occupied").arg(cryoBerths).arg(cryoocc);
					else
						cryoText = tr("%1 Cryo Berths, Unoccupied").arg(cryoBerths);
				}
				else
					cryoText = tr("No Cryo Berths");
				lbcryo->setText(cryoText);

				// Make a list of the species already in the table for reconciling removed atoms at the end.
				// When the update is done, the 'intable' list will contain only ids that weren't updated; those get removed.
				QList<AuSpecies> intable;	// species
				for(int row=0; row<twcryo->rowCount(); row++)
				{
					QTableWidgetItem* item = twcryo->item(row, 0);
					if(item)
						intable.append(item->data(Qt::UserRole).value<AuSpecies>());
				}

				// List occupied cryo berths.
				twcryo->setSortingEnabled(false);
				twcryo->disconnect(SIGNAL(itemSelectionChanged()));
				QTableWidgetItem* twname = 0;
				QTableWidgetItem* twcnt = 0;
				const AuCryoBerths& cryo = ship->LockCryoBerths();
				for(AuCryoBerths::const_iterator it = cryo.constBegin(); it != cryo.constEnd(); it++)
				{
					const DgData& gd = g_Au.LockDataForRead();
					QString specieName = gd.SpeciesName(it.key(), true);
					bool requestSpecie = !gd.HasSpecies(it.key());
					g_Au.UnlockData();
					if(requestSpecie)
						emit SpeciesRequested(it.key());

					if(intable.contains(it.key()))
					{
						// The dna is already in the table.
						for(int row=0; row<twcryo->rowCount(); row++)
						{
							twname = twcryo->item(row, 0);
							if(twname->data(Qt::UserRole).value<AuSpecies>() == it.key())
							{
								// Found the corresponding table row.
								// Update specie name and number of occupants.
								twname->setText(specieName);
								twname->setData(Qt::UserRole+1, *it);
								twcnt = twcryo->item(row, 1);
								twcnt->setText(QString("%L1").arg(*it));
								break;
							}
						}

						// Remove the dna from the 'intable' list. We'll remove the ones that don't get updated at the end.
						intable.removeAll(it.key());
					}
					else
					{
						// Create a new table row.
						twname = new QTableWidgetItem(QIcon(":data/CryoPassenger.png"), specieName);
						twcnt = new QTableWidgetItem(QString("%L1").arg(*it));

						// Store dna string on column 0.
						twname->setData(Qt::UserRole, QVariant::fromValue(it.key()));
						twname->setData(Qt::UserRole+1, *it);

						int row = twcryo->rowCount();
						twcryo->insertRow(row);

						int col=0;
						twcryo->setItem(row,col++, twname);
						twcryo->setItem(row,col++, twcnt);
						if(selSpecies == it.key())
							twcryo->selectRow(row);
					}
				}
				ship->UnlockCryoBerths();

				// Remove rows that no longer belong in the table.
				for(QList<AuSpecies>::const_iterator ii = intable.constBegin(); ii != intable.constEnd(); ii++)
				{
					for(int row=0; row<twcryo->rowCount(); row++)
					{
						QTableWidgetItem* item = twcryo->item(row, 0);
						if(item->data(Qt::UserRole).value<AuSpecies>() == *ii)
						{
							// Found the dna's row in the table. Remove it.
							twcryo->removeRow(row);
							break;
						}
					}
				}
				twcryo->resizeRowsToContents();
				twcryo->setSortingEnabled(true);
				connect(twcryo, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));
			}
		}
		else
		{
			// Not aboard a spacecraft.
			lwavail->clear();
			twroster->clearContents();
			twcryo->clearContents();
			m_CrewBerths = 0;
			m_PassengerBerths = 0;
			m_TroopBerths = 0;
		}

	}
	pView->UnlockSceneAtom();
	EnableControls();
}

void AuSpacecraftBerths::KickCryoOccupant()
{
	QItemSelectionModel* sel = twcryo->selectionModel();
	if(!sel || !sel->hasSelection())
		return;

	QModelIndexList selRowIdxs = sel->selectedRows();
	Q_ASSERT(selRowIdxs.count());
	QModelIndex selIdx = selRowIdxs.first();

	unsigned int count = selIdx.data(Qt::UserRole+1).toUInt();
	bool ok;
	int howMany = QInputDialog::getInt(this, tr("Kick Cryo Travellers"), tr("How many cryo travellers are to be kicked?"),
			count, 1, count, 1, &ok);
	if(!ok || !howMany)
		return;

	emit KickCryoBerthOccupants(selIdx.data(Qt::UserRole).value<AuSpecies>(), howMany);
}

void AuSpacecraftBerths::ReviveCryoOccupant()
{
	if(!m_CrewBerths && !m_TroopBerths && !m_PassengerBerths)
		return;
	QItemSelectionModel* sel = twcryo->selectionModel();
	if(!sel || !sel->hasSelection())
		return;

	QModelIndexList selRowIdxs = sel->selectedRows();
	Q_ASSERT(selRowIdxs.count());
	QModelIndex selIdx = selRowIdxs.first();

	// Get role.
	QStringList roles;
	//roles << tr("No Role");	// This would make a worker. What for?
	if(m_CrewBerths)
		roles << tr("Crew");
	if(m_TroopBerths)
		roles << tr("Troop");
	if(m_PassengerBerths)
		roles << tr("Passenger");
	bool ok;
	QString roleName = QInputDialog::getItem(this, tr("Revive Cryo Traveller"),
			tr("<html>"
				"Choose a role for the cryo traveller. Roles are listed that have an "
				"<br>"
				"empty berth available aboard the ship.</html>"), roles, 0, false, &ok);
	if(!ok)
		return;
	quint8 role;	// 0=none, 1=crew, 2=troop, 3=passenger.
	if(roleName == tr("Crew"))
		role = 1;
	else if(roleName == tr("Troop"))
		role = 2;
	else if(roleName == tr("Passenger"))
		role = 3;
	else
		return;

	emit ReviveCryoBerthOccupant(selIdx.data(Qt::UserRole).value<AuSpecies>(), (quint8)role);
}

QSize AuSpacecraftBerths::sizeHint() const
{
	return QSize(500,600);
}

