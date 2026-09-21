//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auunitorders.cpp,v 1.44 2022/02/19 18:17:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auunitorders.h"
//
#include "auapp.h"
#include "aucommpacket.h"
#include "auframe.h"
#include "augotoroom.h"
#include "aunpcresume.h"
#include "auselectedunit.h"
#include "autoolbutton.h"
#include "auview.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QAction>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>

void AuUnitOrdersDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleOrdersUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuUnitOrdersDock::AuUnitOrdersDock(QWidget* parent) :
	AuDockWidget(":data/CommMilitary.png", tr("<b>Unit Orders</b> (Shift+F3)"), tr("Units"), parent)
{
	m_UnitOrders = new AuUnitOrders(this);
	setWidget(m_UnitOrders);

	int dockSides = g_Au.Settings().readNumEntry("Dock/UnitOrdersAreas", (int)allowedAreas());
	setAllowedAreas((Qt::DockWidgetAreas)dockSides);
}

void AuUnitOrdersDock::SetAllowedAreas(Qt::DockWidgetAreas areas)
{
	setAllowedAreas(areas);
	g_Au.Settings().writeEntry("Dock/UnitOrdersAreas", (int)allowedAreas());
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuUnitTable::AuUnitTable(QWidget* parent) : QTableWidget(0,11,parent)
{
	//horizontalHeader()->hide();
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(6, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(7, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(8, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(9, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(10, QHeaderView::Interactive);
	QStringList headerLabels;
	headerLabels << tr("Rank");
	headerLabels << tr("Name");
	headerLabels << tr("S");
	headerLabels << tr("Location");
	headerLabels << tr("P");
	headerLabels << tr("Order");
	headerLabels << tr("H");
	headerLabels << tr("A");
	headerLabels << tr("C");
	headerLabels << tr("W");
	headerLabels << tr("Ammo");
	setHorizontalHeaderLabels(headerLabels);
	//setAlternatingRowColors(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setShowGrid(false);
	setSortingEnabled(true);
	setToolTip(tr("List of units that are able to accept your orders."));
	sortByColumn(0, Qt::DescendingOrder);
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

void AuUnitTable::mouseReleaseEvent(QMouseEvent* e)
{
	QTableWidget::mouseReleaseEvent(e);
	if(e->button() == Qt::RightButton)
	{
		QPoint pos(e->pos());
		QTableWidgetItem* clickedItem = itemAt(pos);
		if(clickedItem)
		{
			int row = clickedItem->row();
			QTableWidgetItem* nameItem = item(row, 1);
			if(nameItem)
			{
				AuId unitId(nameItem->data(Qt::UserRole).toUInt());
				emit UnitRightClicked(unitId);
			}
		}
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A QTableWidgetItem that sorts numerically, instead of alphabetically.

class AuTableWidgetRankItem : public QTableWidgetItem
{
public:
	AuTableWidgetRankItem(int rank, const QIcon& rankIcon, const QString& rankAbbr, const QString rankName) :
		QTableWidgetItem(rankIcon, rankAbbr)
	{
		setData(Qt::UserRole, rank);
		setToolTip(rankName);
	}

	bool operator<(const QTableWidgetItem& other) const
	{
		int myInt = data(Qt::UserRole).toInt();
		int otherInt = other.data(Qt::UserRole).toInt();
		return (myInt < otherInt) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuUnitOrders::AuUnitOrders(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	m_Channels = new QComboBox;
	m_Channels->addItem(AuCommStdChannel::ChannelIcon(AuCommStdChannel::CrewChannelName(), AuCommStdChannel::CrewMates),
			AuCommStdChannel::CrewChannelName());
	m_Channels->addItem(AuCommStdChannel::ChannelIcon(AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Audible),
			AuCommStdChannel::VoiceChannelName());
	m_Channels->setToolTip(tr("Select the channel for sending squad orders."));

	QLabel* lbselcount = new QLabel(this);
	lbselcount->setObjectName("lbselcount");
	lbselcount->setToolTip(tr("Assign the selected units to a squad by pressing <b>Shift+Number</b>.<br>"
				"Select a squad by pressing <b>Ctrl+Number</b>"));
	twcitizens = new AuUnitTable(this);
	connect(twcitizens, SIGNAL(itemSelectionChanged()), SLOT(SelectionChanged()));
	connect(twcitizens, SIGNAL(UnitRightClicked(AuId)), SLOT(UnitRightClicked(AuId)));

	AuIconButton* pbselofficers = new AuIconButton(":data/Officer.png", this);
	pbselofficers->setToolTip(tr("Select all officers that are listed."));
	connect(pbselofficers, SIGNAL(clicked()), SLOT(SelectOfficers()));

	AuIconButton* pbselcrew = new AuIconButton(":data/Crewman.png", this);
	pbselcrew->setToolTip(tr("Select all crew members that are listed."));
	connect(pbselcrew, SIGNAL(clicked()), SLOT(SelectCrew()));

	AuIconButton* pbseltroops = new AuIconButton(":data/Troop.png", this);
	pbseltroops->setToolTip(tr("Select all troops that are listed."));
	connect(pbseltroops, SIGNAL(clicked()), SLOT(SelectTroops()));

	AuIconButton* pbselpass = new AuIconButton(":data/Passenger.png", this);
	pbselpass->setToolTip(tr("Select all passengers that are listed."));
	connect(pbselpass, SIGNAL(clicked()), SLOT(SelectPassengers()));

	AuIconButton* pbselnone = new AuIconButton(":data/ShipRosterRemove.png", this);
	pbselnone->setToolTip(tr("Deselects all units that are listed."));
	connect(pbselnone, SIGNAL(clicked()), SLOT(DeselectAllUnits()));

	AuIconButton* pbattack = new AuIconButton(OrderIcon(ORDER_AttackTarget), this);
	pbattack->setObjectName("pbattack");
	pbattack->setToolTip(tr("<b>Attack Target</b><br>"
				"Order the selected units to attack the target you have sighted.<br><b>Ctrl+A</b>"));
	connect(pbattack, SIGNAL(clicked()), SLOT(AttackTarget()));

	AuIconButton* pbstations = new AuIconButton(OrderIcon(ORDER_BattleStation), this);
	pbstations->setObjectName("pbstations");
	pbstations->setToolTip(tr("<b>Stations</b><br>"
				"Order the selected units to operate the nearest control station or turret.<br><b>Ctrl+N</b>"));
	connect(pbstations, SIGNAL(clicked()), SLOT(BattleStations()));

	AuIconButton* pbberserk = new AuIconButton(PostureIcon(POSTURE_Berserk), this);
	pbberserk->setObjectName("pbberserk");
	pbberserk->setToolTip(tr("<b>Berserk Posture</b><br>"
				"Order the selected units to initiate attacks on all enemy units, including civilians and "
				"farm animals.<br><b>Ctrl+K</b>"));
	connect(pbberserk, SIGNAL(clicked()), SLOT(BerserkPosture()));

	AuIconButton* pbclosedoor = new AuIconButton(OrderIcon(ORDER_CloseDoor), this);
	pbclosedoor->setObjectName("pbclosedoor");
	pbclosedoor->setToolTip(tr("<b>Close Door</b><br>"
				"Order the selected units to close the door at their location.<br><b>Ctrl+C</b>"));
	connect(pbclosedoor, SIGNAL(clicked()), SLOT(CloseDoor()));

	AuIconButton* pbcomehere = new AuIconButton(OrderIcon(ORDER_ComeHere), this);
	pbcomehere->setObjectName("pbcomehere");
	pbcomehere->setToolTip(tr("<b>Come Here</b><br>"
				"Order the selected units to come to your location. They will not get in your vehicle.<br><b>Ctrl+H</b>"));
	connect(pbcomehere, SIGNAL(clicked()), SLOT(ComeHere()));

	AuIconButton* pbfollowme = new AuIconButton(OrderIcon(ORDER_FollowMe), this);
	pbfollowme->setObjectName("pbfollowme");
	pbfollowme->setToolTip(tr("<b>Follow Me</b><br>"
				"Order the selected units to follow you. They will not get in your vehicle.<br><b>Ctrl+M</b>"));
	connect(pbfollowme, SIGNAL(clicked()), SLOT(FollowMe()));

	AuIconButton* pbfollowtarget = new AuIconButton(OrderIcon(ORDER_FollowTarget), this);
	pbfollowtarget->setObjectName("pbfollowtarget");
	pbfollowtarget->setToolTip(tr("<b>Follow Target</b><br>"
				"Order the selected units to follow the target you have sighted.<br><b>Ctrl+F</b>"));
	connect(pbfollowtarget, SIGNAL(clicked()), SLOT(FollowTarget()));

	AuIconButton* pbgarrison = new AuIconButton(OrderIcon(ORDER_Garrison), this);
	pbgarrison->setObjectName("pbgarrison");
	pbgarrison->setToolTip(tr("<b>Garrison</b><br>"
				"Order the selected units to fortify their location. They will release their berth assignment "
				"and take up station at their present location.<br>"
				"%1 <b>Officers</b>, <b>Crew</b> and <b>Passengers</b> will try to join the roster of the ship they are aboard. "
				"If that fails, they will become citizens of their current city and vanish.<br>"
				"%1 <b>Officers</b> will take the con when assigned to a ship.<br>"
				"%1 <b>Troops</b> will join the roster of the ship they are aboard. "
				"If that fails, they will fortify their current base; "
				"troops will desert their posts after you are gone if ordered to garrison any other location."
				"<br><b>Ctrl+R</b>").arg(g_Au.MiddleDotSymbol()));
	connect(pbgarrison, SIGNAL(clicked()), SLOT(Garrison()));

	AuIconButton* pbgetin = new AuIconButton(OrderIcon(ORDER_GetIn), this);
	pbgetin->setObjectName("pbgetin");
	pbgetin->setToolTip(tr("<html><b>Get In</b><br>"
				"Order the selected units to get into your vehicle or spacecraft.<br>"
				"<b>Ctrl+I</b></html>"));
	connect(pbgetin, SIGNAL(clicked()), SLOT(GetIn()));

	AuIconButton* pbgetitem = new AuIconButton(OrderIcon(ORDER_GetItem), this);
	pbgetitem->setObjectName("pbgetitem");
	pbgetitem->setToolTip(tr("<html><b>Get Item or Dying Creature</b><br>"
				"Order the selected units to get the item or dying creature you have sighted.<br>"
				"%1 Units will only get dying creatures they can carry.<br>"
				"%1 Units will not get a dying creature when they are already carrying a dying creature.<br>"
				"%1 Units will not get dying city workers or storyline characters.<br>"
				"%1 Units will only get items they can use.<br>"
				"%1 Units will replace an item in their inventory if necessary to get the item you sighted.<br>"
				"<b>Ctrl+G</b></html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbgetitem, SIGNAL(clicked()), SLOT(GetItem()));

	AuIconButton* pbgetout = new AuIconButton(OrderIcon(ORDER_GetOut), this);
	pbgetout->setObjectName("pbgetout");
	pbgetout->setToolTip(tr("<html><b>Get Out or Dismount</b><br>"
				"Order the selected units to get out of their vehicle or spacecraft, or to dismount from their mount.<br>"
				"<b>Ctrl+O</b></html>"));
	connect(pbgetout, SIGNAL(clicked()), SLOT(GetOut()));

	AuIconButton* pbmountme = new AuIconButton(OrderIcon(ORDER_MountMe), this);
	pbmountme->setObjectName("pbmountme");
	pbmountme->setToolTip(tr("<html><b>Mount Me</b><br>"
				"Order the selected units to mount you.<br>"
				"%1 You must crouch to permit anyone to mount you for riding.<br>"
				"%1 You must assume the mating position to permit anyone to mount you for mating.<br>"
				"<b>Ctrl+U</b></html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbmountme, SIGNAL(clicked()), SLOT(MountMe()));

	AuIconButton* pbmounttarget = new AuIconButton(OrderIcon(ORDER_MountTarget), this);
	pbmounttarget->setObjectName("pbmounttarget");
	pbmounttarget->setToolTip(tr("<html><b>Mount Target</b><br>"
				"Order the selected units to mount the target you have sighted.<br>"
				"%1 Units will mount players who are not enemies.<br>"
				"%1 Units will mount other units who are assigned to the same ship as them and who are not enemies.<br>"
				"%1 Units will not mount animals.<br>"
				"%1 Players must crouch or assume the mating position to permit anyone to mount them.<br>"
				"<b>Ctrl+J</b></html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbmounttarget, SIGNAL(clicked()), SLOT(MountTarget()));

	AuIconButton* pbdroprider = new AuIconButton(OrderIcon(ORDER_DropRider), this);
	pbdroprider->setObjectName("pbdroprider");
	pbdroprider->setToolTip(tr("<b>Drop Rider</b><br>"
				"Order the selected units to drop their riders.<br><b>Ctrl+Q</b>"));
	connect(pbdroprider, SIGNAL(clicked()), SLOT(DropRider()));

	AuIconButton* pbgotosurgery = new AuIconButton(OrderIcon(ORDER_EnterSurgery), this);
	pbgotosurgery->setObjectName("pbgotosurgery");
	pbgotosurgery->setToolTip(tr("<b>Enter Surgery</b><br>"
				"Order the selected units to enter the nearest surgery unit.<br><b>Ctrl+Y</b>"));
	connect(pbgotosurgery, SIGNAL(clicked()), SLOT(EnterSurgery()));

	AuIconButton* pbtakecommand = new AuIconButton(OrderIcon(ORDER_TakeCommand), this);
	pbtakecommand->setObjectName("pbtakecommand");
	pbtakecommand->setToolTip(tr("<b>Take The Con</b><br>"
				"Order the selected officer to take the con, placing that officer in command of the ship. "
				"The officer in command carries out orders in the ship's mission.<br>"
				"%1 Officers must be aboard their assigned ship.<br>"
				"%1 Officers do not take a berth because of this command. The Garrison command does that."
				"<br><b>Ctrl+Z</b>").arg(g_Au.MiddleDotSymbol()));
	connect(pbtakecommand, SIGNAL(clicked()), SLOT(TakeCommand()));

	AuIconButton* pbrelcommand = new AuIconButton(OrderIcon(ORDER_ReleaseCommand), this);
	pbrelcommand->setObjectName("pbrelcommand");
	pbrelcommand->setToolTip(tr("<b>Release The Con</b><br>"
				"Order the selected officers to release the con, relieving those officers from commanding the ship. "
				"The officers keep their berths. They just stop carrying out orders in the ship's mission."
				"<br><b>Ctrl+X</b>"));
	connect(pbrelcommand, SIGNAL(clicked()), SLOT(ReleaseCommand()));

	AuIconButton* pbgotoroom = new AuIconButton(OrderIcon(ORDER_GoToRoom), this);
	pbgotoroom->setObjectName("pbgotoroom");
	pbgotoroom->setToolTip(tr("<b>Go To Room...</b><br>"
				"Order the selected units to go to a room by name.<br><b>Ctrl+T</b>"));
	connect(pbgotoroom, SIGNAL(clicked()), SLOT(GoToRoom()));

	AuIconButton* pbgoto = new AuIconButton(OrderIcon(ORDER_GoToTarget), this);
	pbgoto->setObjectName("pbgoto");
	pbgoto->setToolTip(tr("<b>Go To Target</b><br>"
				"Order the selected units to go to the target location you have sighted.<br><b>Ctrl+T</b>"));
	connect(pbgoto, SIGNAL(clicked()), SLOT(GoToTarget()));

	AuIconButton* pbopendoor = new AuIconButton(OrderIcon(ORDER_OpenDoor), this);
	pbopendoor->setObjectName("pbopendoor");
	pbopendoor->setToolTip(tr("<b>Open Door</b><br>"
				"Order the selected units to open the door at their location.<br><b>Ctrl+P</b>"));
	connect(pbopendoor, SIGNAL(clicked()), SLOT(OpenDoor()));

	AuIconButton* pbstay = new AuIconButton(OrderIcon(ORDER_Stay), this);
	pbstay->setObjectName("pbstay");
	pbstay->setToolTip(tr("<b>Stay</b><br>"
				"Order the selected units to stay and guard their location and stop manning any stations.<br>"
				"<b>Ctrl+S</b>"));
	connect(pbstay, SIGNAL(clicked()), SLOT(Stay()));

	AuIconButton* pboffense = new AuIconButton(PostureIcon(POSTURE_Offensive), this);
	pboffense->setObjectName("pboffense");
	pboffense->setToolTip(tr("<b>Offensive Posture</b><br>"
				"Order the selected units to initiate attacks on threats.<br><b>Ctrl+V</b>"));
	connect(pboffense, SIGNAL(clicked()), SLOT(OffensivePosture()));

	AuIconButton* pbdefarea = new AuIconButton(PostureIcon(POSTURE_DefendArea), this);
	pbdefarea->setObjectName("pbdefarea");
	pbdefarea->setToolTip(tr("<b>Defend Area Posture</b><br>"
				"Order the selected units to initiate attacks on threats that get close. "
				"Will not fire at wild animals when manning a turret.<br><b>Ctrl+D</b>"));
	connect(pbdefarea, SIGNAL(clicked()), SLOT(DefendAreaPosture()));

	AuIconButton* pbdefself = new AuIconButton(PostureIcon(POSTURE_DefendSelf), this);
	pbdefself->setObjectName("pbdefself");
	pbdefself->setToolTip(tr("<b>Defend Self Posture</b><br>"
				"Order the selected units to attack threats only if attacked.<br><b>Ctrl+E</b>"));
	connect(pbdefself, SIGNAL(clicked()), SLOT(DefendSelfPosture()));

	AuIconButton* pbholdfire = new AuIconButton(PostureIcon(POSTURE_HoldFire), this);
	pbholdfire->setObjectName("pbholdfire");
	pbholdfire->setToolTip(tr("<b>Hold Fire Posture</b><br>"
				"Order the selected units to hold their fire.<br><b>Ctrl+L</b>"));
	connect(pbholdfire, SIGNAL(clicked()), SLOT(HoldFirePosture()));

	QHBoxLayout* hlay = new QHBoxLayout();
	hlay->addStretch(1);
	hlay->addWidget(pbselofficers);
	hlay->addWidget(pbselcrew);
	hlay->addWidget(pbseltroops);
	hlay->addWidget(pbselpass);
	hlay->addWidget(pbselnone);
	hlay->addWidget(m_Channels);

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(pbberserk);
	blay->addWidget(pboffense);
	blay->addWidget(pbdefarea);
	blay->addWidget(pbdefself);
	blay->addWidget(pbholdfire);
	blay->addSpacing(6);
	blay->addStretch();
	blay->addWidget(pbattack);
	blay->addWidget(pbgoto);
	blay->addWidget(pbfollowtarget);
	blay->addWidget(pbmounttarget);
	blay->addSpacing(6);
	blay->addWidget(pbcomehere);
	blay->addWidget(pbfollowme);
	blay->addWidget(pbmountme);
	blay->addSpacing(6);
	blay->addWidget(pbgotoroom);
	blay->addWidget(pbgetin);
	blay->addWidget(pbgetout);
	blay->addWidget(pbdroprider);
	blay->addSpacing(6);
	blay->addWidget(pbgetitem);
	blay->addWidget(pbclosedoor);
	blay->addWidget(pbopendoor);
	blay->addSpacing(6);
	blay->addWidget(pbstations);
	blay->addWidget(pbgotosurgery);
	blay->addWidget(pbstay);
	blay->addWidget(pbgarrison);
	blay->addSpacing(6);
	blay->addWidget(pbtakecommand);
	blay->addWidget(pbrelcommand);

	QGridLayout* lay = new QGridLayout();
	int row = 0;
	lay->addWidget(lbselcount, row,0);
	lay->addLayout(hlay, row,1);
	row++;
	lay->addWidget(twcitizens, row,0, 1,2);
	row++;
	lay->addLayout(blay, row,0, 1,2);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuUnitOrders::~AuUnitOrders()
{
}

void AuUnitOrders::showEvent(QShowEvent* event)
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

	RestoreSettings();
	UpdateContents();
}

void AuUnitOrders::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();

	SaveSettings();
}

void AuUnitOrders::UpdateContents()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	//twcitizens->clearContents();
	//while(twcitizens->rowCount())
		//twcitizens->removeRow(0);
	twcitizens->setSortingEnabled(false);
	twcitizens->blockSignals(true);

	// Make a list of the atom ids already in the table for reconciling removed atoms at the end.
	// When the update pass is done, the 'intable' list will contain only ids of atoms that weren't updated. Those get removed.
	AuIdList intable;
	for(int row=0; row<twcitizens->rowCount(); row++)
	{
		QTableWidgetItem* item = twcitizens->item(row, 1);
		intable.append(AuId(item->data(Qt::UserRole).toInt()));
	}

	AuIdList citizensInTable;
	const AuSelectedUnitArray& selected = pView->SelectedUnits();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// Build a roster of the citizens in the scene.
		DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool isAgent = gd.ClientPlayerIsAgent();
			g_Au.UnlockData();

			AuId shipId;
			if(person->InArena())
			{
				const DaVessel* shipAboard = person->FindParentVesselInEnv();
				if(shipAboard && (shipAboard->EmpireId() == person->EmpireId() || isAgent))
					shipId = shipAboard->Id();
			}
			else if(isAgent && person->IsInSpacecraft())
			{
				const DaVessel* shipAboard = person->FindParentVesselInEnv();
				if(shipAboard)
					shipId = shipAboard->Id();
				else
					shipId = person->SpawnShip();
			}
			else
				shipId = person->SpawnShip();

			DaAtomList citizens;
			system->FindAIPeopleRecursive(shipId, citizens);
			system->FindOfficersRecursive(shipId, citizens);
			system->FindCrewRecursive(shipId, citizens);
			system->FindTroopsRecursive(shipId, citizens);
			system->FindPassengersRecursive(shipId, citizens);
#if !defined(QT_NO_DEBUG)
			if(dynamic_cast<DaCitizen*>(person->Parent()))
				citizens.AddIfUnique(person->Parent());
#endif

			// Construct null id value that we'll use for the 'no item' entries in our equipment lists.
			const AuItem nullItem(CID_Invalid, 1, 0, 0);

			for(DaAtomList::const_iterator ti = citizens.begin(); ti != citizens.end(); ti++)
			{
				DaSentient* citizen = dynamic_cast<DaSentient*>(*ti);
				if(!citizen)
					continue;
				if(!citizen->UnitCanBeSelected(person, false))
					continue;

				// Get species name and empire name.
				QString empireName;
				QString specieName = citizen->SpeciesName();	// Name of species, or a generic name when species not named.
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(citizen->EmpireId(), empireName);	// Name of empire of allegiance.
				g_Au.UnlockData();

				// Name and gender
				QString name = tr("%1<br>%2 %3<br>%4")
					.arg(citizen->Name())
					.arg(citizen->FemaleMale(true))
					.arg(specieName)
					.arg(citizen->IsMarried()?tr("Married"):tr("Single"));

				// Rank icon
				QString rankAbbr = citizen->RankAbbr();
				QIcon rankIcon = QIcon(citizen->RankIcon());
				QString rankName;
				if(!rankAbbr.isEmpty())
					rankName = tr("%1 (%2)").arg(citizen->RankName()).arg(rankAbbr);
				else
					rankName = tr("%1").arg(citizen->RankName());

				// Empire of allegiance
				rankName += "<br>" + empireName;

				int rank = citizen->Rank() + 1;
				if(citizen->IsNpcOfficer())
					rank += 30;	// Officer ranks count higher than the ranks of crew or troops.
				else if(citizen->IsNpcCrew())
					rank += 20;	// Crew ranks count higher than the ranks of troops.
				else if(citizen->IsNpcTroop())
					rank += 10;	// Troop ranks count higher than the ranks of civilians.
				NPC_ROLE npcRole = citizen->NpcRole();

				// Location
				QString location, locationShort;
				{
					if(citizen->IsAboardVehicle())
					{
						location = tr("Aboard %1").arg(citizen->Parent()->Name());
						locationShort = citizen->Parent()->Name();
					}
					else
					{
						switch(citizen->CurrentGeneralPose().Pose())
						{
						case AuPose::Init:
						case AuPose::Beckon:
						case AuPose::Bite:
						case AuPose::Block:
						case AuPose::Eat:
						case AuPose::Hug:
						case AuPose::Kick:
						case AuPose::Lash:
						case AuPose::Point:
						case AuPose::Psionic:
						case AuPose::Stomp:
						case AuPose::Strike:
						case AuPose::Throw:
						case AuPose::Upright:
						case AuPose::Wag:
						case AuPose::Wave:
						case AuPose::MouthPursed:
						case AuPose::MouthClosed:
						case AuPose::MouthOpen:
						case AuPose::MouthAgape:
						case AuPose::MouthOpenWide:
						case AuPose::MouthYawn:
						case AuPose::MouthBite:
						case AuPose::MouthLaugh:
						case AuPose::MouthYap:
						case AuPose::MouthWhisper:
						case AuPose::MouthTalk:
						case AuPose::MouthJabber:
						case AuPose::MouthYell:
						case AuPose::MouthSmile:
						case AuPose::MouthGrin:
						case AuPose::MouthSad:
						case AuPose::MouthFrown:
						case AuPose::MouthCry:
						case AuPose::MouthPerplexed:
						case AuPose::MouthGrim:
						case AuPose::MouthSardonic:
							// These are not "general" poses.
							break;
						case AuPose::Straddle:
						case AuPose::Spawn:
						case AuPose::EyesOnly:
							// These poses don't occur.
							break;
						case AuPose::GameTableSeat0_0:
						case AuPose::GameTableSeat0_45:
							location = tr("Game Table Chair 1");
							break;
						case AuPose::GameTableSeat1_0:
						case AuPose::GameTableSeat1_45:
							location = tr("Game Table Chair 2");
							break;
						case AuPose::GameTableSeat2_0:
						case AuPose::GameTableSeat2_45:
							location = tr("Game Table Chair 3");
							break;
						case AuPose::GameTableSeat3_0:
						case AuPose::GameTableSeat3_45:
							location = tr("Game Table Chair 4");
							break;
						case AuPose::ConsoleCommand:
						case AuPose::Command_Sit:
						case AuPose::Command_SitStationary:
						case AuPose::Command_Stand:
						case AuPose::Command_StandStationary:
							location = tr("Command Station");
							break;
						case AuPose::ConsoleNav:
						case AuPose::Nav_Sit:
						case AuPose::Nav_SitStationary:
						case AuPose::Nav_Stand:
						case AuPose::Nav_StandStationary:
							location = tr("Navigator Station");
							break;
						case AuPose::ConsoleEngine:
						case AuPose::Engine_Sit:
						case AuPose::Engine_SitStationary:
						case AuPose::Engine_Stand:
						case AuPose::Engine_StandStationary:
							location = tr("Engineer Station");
							break;
						case AuPose::ConsoleFireControl:
						case AuPose::FireControl_Sit:
						case AuPose::FireControl_SitStationary:
						case AuPose::FireControl_Stand:
						case AuPose::FireControl_StandStationary:
							location = tr("Fire Control Station");
							break;
						case AuPose::ConsoleSurgery:
						case AuPose::Medic_Sit:
						case AuPose::Medic_SitStationary:
						case AuPose::Medic_Stand:
						case AuPose::Medic_StandStationary:
							location = tr("Medical Station");
							break;
						case AuPose::ConsolePilot:
						case AuPose::Helm_Sit:
						case AuPose::Helm_SitStationary:
						case AuPose::Helm_Stand:
						case AuPose::Helm_StandStationary:
							location = tr("Helm");
							break;
						case AuPose::ConsolePower:
						case AuPose::Power_Sit:
						case AuPose::Power_SitStationary:
						case AuPose::Power_Stand:
						case AuPose::Power_StandStationary:
							location = tr("Power Relay Station");
							break;
						case AuPose::ConsoleSensor:
						case AuPose::Sensor_Sit:
						case AuPose::Sensor_SitStationary:
						case AuPose::Sensor_Stand:
						case AuPose::Sensor_StandStationary:
							location = tr("Sensor Station");
							break;
						case AuPose::ConsoleShield:
						case AuPose::Shield_Sit:
						case AuPose::Shield_SitStationary:
						case AuPose::Shield_Stand:
						case AuPose::Shield_StandStationary:
							location = tr("Shield Station");
							break;
						case AuPose::ConsoleSurgeryUnit:
						case AuPose::Patient_Sit:
						case AuPose::Patient_SitStationary:
						case AuPose::Patient_Stand:
						case AuPose::Patient_StandStationary:
						case AuPose::Patient_Prone:
							location = tr("Surgery Unit");
							break;
						case AuPose::ConsoleTransporter:
						case AuPose::Transporter_Sit:
						case AuPose::Transporter_SitStationary:
						case AuPose::Transporter_Stand:
						case AuPose::Transporter_StandStationary:
							location = tr("Transporter Station");
							break;
						case AuPose::Turret_Sit:
						case AuPose::Turret_SitStationary:
						case AuPose::Turret_SitSwivel3D:
						case AuPose::Turret_Stand:
						case AuPose::Turret_StandStationary:
							location = tr("Turret Station");
							break;
						case AuPose::Post_Sit:
						case AuPose::Post_Stand:
							location = tr("Guard Post");
							break;
						case AuPose::ConsoleTurret0:
							location = tr("Turret Starboard");
							break;
						case AuPose::ConsoleTurret45:
							location = tr("Turret Starboard Fore");
							break;
						case AuPose::ConsoleTurret90:
							location = tr("Turret Fore");
							break;
						case AuPose::ConsoleTurret135:
							location = tr("Turret Port Fore");
							break;
						case AuPose::ConsoleTurret180:
							location = tr("Turret Port");
							break;
						case AuPose::ConsoleTurret225:
							location = tr("Turret Port Quarter");
							break;
						case AuPose::ConsoleTurret270:
							location = tr("Turret Aft");
							break;
						case AuPose::ConsoleTurret315:
							location = tr("Turret Starboard Quarter");
							break;
						case AuPose::ConsoleTurretBottom:
							location = tr("Turret Below");
							break;
						case AuPose::ConsoleTurretTop:
							location = tr("Turret Aloft");
							break;
						case AuPose::Crouch:
							location = tr("Crouching");
							break;
						case AuPose::Prone:
							location = tr("Lying Prone");
							break;
						case AuPose::SitOnGround:
							location = tr("Sitting Down");
							break;
						case AuPose::Stand:
							location = tr("Standing");
							break;
						case AuPose::TreadWater:
							location = tr("Treading Water");
							break;
						case AuPose::Hover:
							location = tr("Hovering");
							break;
						case AuPose::Ladder:
							location = tr("Ladder");
							break;
						case AuPose::CrouchCreep:
							location = tr("Creeping Crouched");
							break;
						case AuPose::SitOnGroundCreep:
							location = tr("Creeping while Sitting");
							break;
						case AuPose::Dig:
							location = tr("Digging");
							break;
						case AuPose::Fly:
							location = tr("Flying");
							break;
						case AuPose::FlyFast:
							location = tr("Flying Fast");
							break;
						case AuPose::Glide:
							location = tr("Gliding");
							break;
						case AuPose::Jump:
							location = tr("Jumping");
							break;
						case AuPose::LadderClimb:
							location = tr("Climing Ladder");
							break;
						case AuPose::ProneCreep:
							location = tr("Creeping Prone");
							break;
						case AuPose::StandRun:
							location = tr("Running");
							break;
						case AuPose::StandWalk:
							location = tr("Walking");
							break;
						case AuPose::Swim:
							location = tr("Swimming");
							break;
						case AuPose::SwimFast:
							location = tr("Swimming Fast");
							break;
						case AuPose::DeadFaceDown:
							location = tr("Dead Facing Down");
							break;
						case AuPose::DeadFaceLeft:
							location = tr("Dead Facing Left");
							break;
						case AuPose::DeadFaceRight:
							location = tr("Dead Facing Right");
							break;
						case AuPose::DeadFaceUp:
							location = tr("Dead Facing Up");
							break;
						case AuPose::WritheFaceDown:
							location = tr("Dying Facing Down");
							break;
						case AuPose::WritheFaceLeft:
							location = tr("Dying Facing Left");
							break;
						case AuPose::WritheFaceRight:
							location = tr("Dying Facing Right");
							break;
						case AuPose::WritheFaceUp:
							location = tr("Dying Facing Up");
							break;
						case AuPose::ReceptiveMate:
							location = tr("Waiting to Have Sex");
							break;
						case AuPose::MateOnBottom:
							location = tr("Having Sex");
							break;
						case AuPose::MateOnTop:
							location = tr("Having Sex");
							break;
						case AuPose::BerthBottomFaceDown:
							location = tr("Bottom Bunk, Facing Down");
							break;
						case AuPose::BerthBottomFaceLeft:
							location = tr("Bottom Bunk, Facing Left");
							break;
						case AuPose::BerthBottomFaceRight:
							location = tr("Bottom Bunk, Facing Right");
							break;
						case AuPose::BerthBottomFaceUp:
							location = tr("Bottom Bunk, Facing Up");
							break;
						case AuPose::BerthTopFaceDown:
							location = tr("Top Bunk, Facing Down");
							break;
						case AuPose::BerthTopFaceLeft:
							location = tr("Top Bunk, Facing Left");
							break;
						case AuPose::BerthTopFaceRight:
							location = tr("Top Bunk, Facing Right");
							break;
						case AuPose::BerthTopFaceUp:
							location = tr("Top Bunk, Facing Up");
							break;
						}
					}

					if(citizen->IsInSpacecraft())
					{
						DaVessel* ship = citizen->FindParentVesselInEnv();
						if(ship)
						{
							if(!location.isEmpty())
								location += "<br>";
							location += tr("Aboard %1").arg(ship->NameWithEmpire());
							/*
							if(locationShort.isEmpty())
								locationShort = ship->Name();
							else
								locationShort += ", " + ship->Name();
								*/
							if(ship->IsDesignDataCacheValid())
							{
								ROOMTYPE room = citizen->ShipRoomType();
								if(room != ROOM_None)
								{
									QString desc(" " + ship->LocationDesc(*citizen));
									location += desc;
									if(!locationShort.isEmpty())
										locationShort += ", ";
									locationShort += desc;
								}
							}
						}
					}

					const DaWorld* planet = citizen->FindParentPlanetInEnv();
					if(planet)
					{
						if(!location.isEmpty())
							location += "<br>";
						location += planet->Name();

						AuMatrix4x4 tp;
						person->GetTransformRelativeToParent(tp, planet);
						AuPoint planetLoc = tp.Project(AuPoint(0,0));

						QString strLongLat;
						const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
						if(llworld)
						{
							AuGlobeLL ll;
							if(llworld->LLOfLoc(ll, planetLoc))
								strLongLat = ll.ToString();
						}
						location += "<br>" + strLongLat;

						if(!citizen->BdId().IsNull() && citizen->InIndustry() != IID_None)
						{
							// City name
							const DaBd* bd = planet->FindBd(citizen->BdId());
							if(bd)
							{
								if(!bd->CityName().isEmpty())
								{
									location += QString("<br>%1 %2").arg(bd->CityName()).arg(bd->Name());
									if(locationShort.isEmpty())
										locationShort = bd->Name() + ", " + bd->CityName();
								}
								else
								{
									location += QString("<br>%1").arg(bd->Name());
									if(locationShort.isEmpty())
										locationShort = bd->Name();
								}
							}
						}

						if(locationShort.isEmpty())
							locationShort = strLongLat + " " + planet->Name();
					}
				}

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
					citizen->CombatArmorEffectiveness(WM_Contact, WE_Generic, false, AuPoint(0,0), AuPoint(0,0), baseAbsorb,
							baseBlock, auxAbsorb, auxBlock);
					float absorbK = baseAbsorb + auxAbsorb;
					float blockK = baseBlock + auxBlock;
					status += tr("<br>Armor: %1")
						.arg(g_Au.FormatArmorFactors(absorbK, blockK));
				}

				// Order
				QString order;
				QIcon orderIcon;
				if(citizen->IsBodyDead())
				{
					order = tr("Dead");
					orderIcon = QIcon(":data/Bugle.png");
				}
				else if(citizen->IsBodyDyingOrDead())
				{
					if(dynamic_cast<DaCreature*>(citizen->Parent()))
					{
						order = tr("Stabilized");
						orderIcon = QIcon(":data/dp_SickBay.png");
					}
					else
					{
						order = tr("Dying");
						orderIcon = QIcon(":data/PostureBerserk.png");
					}
				}
				else
				{
					order = citizen->DescribeActivity();
					orderIcon = QIcon(OrderIcon(citizen->Order()));
				}

				// Posture
				QIcon posture;
				QString postureTip;
				switch(citizen->Posture())
				{
				case POSTURE_Berserk:
					posture = QIcon(PostureIcon(POSTURE_Berserk));
					postureTip = tr("Berserk<br>Will attack all potential threats, including civilians and farm animals.");
					break;
				case POSTURE_DefendSelf:
					posture = QIcon(PostureIcon(POSTURE_DefendSelf));
					postureTip = tr("Defend Self<br>Will not attack unless attacked.");
					break;
				case POSTURE_DefendArea:
					posture = QIcon(PostureIcon(POSTURE_DefendArea));
					postureTip = tr("Defend Area<br>"
							"Will attack threats that get close. Won't attack wild animals when manning a turret.");
					break;
				case POSTURE_HoldFire:
					posture = QIcon(PostureIcon(POSTURE_HoldFire));
					postureTip = tr("Hold Fire<br>Will not attack threats.");
					break;
				case POSTURE_Offensive:
					posture = QIcon(PostureIcon(POSTURE_Offensive));
					postureTip = tr("Offensive<br>Will attack threats.");
					break;
				}

				// Add the citizen to the roster on the dialog.
				AuToolButtonValue* cbArmor = 0;
				AuToolButtonValue* cbClothing = 0;
				AuToolButtonValue* cbHelmet = 0;
				AuToolButtonValue* cbWeapon = 0;
				AuToolButtonNumberValue* cnAmmo = 0;
				bool isSelected = selected.Contains(citizen->Id());
				citizensInTable.append(citizen->Id());
				if(intable.contains(citizen->Id()))
				{
					// The citizen is already in the table so just update the contents.
					for(int row=0; row<twcitizens->rowCount(); row++)
					{
						QTableWidgetItem* item = twcitizens->item(row, 1);
						if(citizen->Is(AuId(item->data(Qt::UserRole).toInt())))
						{
							// Found the citizen's row in the table.
							// Update the fields.
							QString citname = citizen->Name();
							if(!rankAbbr.isEmpty() && citname.left(rankAbbr.length()) == rankAbbr)
								citname = citname.mid(rankAbbr.length()+1);
							item->setText(citname);
							item->setToolTip(name);
							item->setSelected(isSelected);

							item = twcitizens->item(row, 0);
							item->setIcon(rankIcon);
							item->setData(Qt::UserRole, rank);	// column 0 stores rank value
							item->setText(rankAbbr);
							item->setToolTip(rankName);
							item->setSelected(isSelected);

							int col=2;
							item = twcitizens->item(row, col++);
							item->setData(Qt::UserRole, npcRole);	// column 2 stores npc role, for selecting by role
							item->setText(statusShort);
							item->setToolTip(status);
							item->setSelected(isSelected);

							item = twcitizens->item(row, col++);
							item->setText(locationShort);
							item->setToolTip(location);
							item->setSelected(isSelected);

							item = twcitizens->item(row, col++);
							item->setIcon(posture);
							item->setToolTip(postureTip);
							item->setSelected(isSelected);

							item = twcitizens->item(row, col++);
							item->setIcon(orderIcon);
							item->setText(order);
							item->setSelected(isSelected);

							// Cannot manipulate the inventory of avatars or passengers.
							if(!dynamic_cast<DaPerson*>(citizen) && !citizen->IsNpcPassenger())
							{
								QModelIndex idxHelmet = twcitizens->model()->index(row, col++);
								//twcitizens->item(idxHelmet.row(), idxHelmet.column())->setSelected(isSelected);
								cbHelmet = dynamic_cast<AuToolButtonValue*>(twcitizens->indexWidget(idxHelmet));

								QModelIndex idxArmor = twcitizens->model()->index(row, col++);
								//twcitizens->item(idxArmor.row(), idxArmor.column())->setSelected(isSelected);
								cbArmor = dynamic_cast<AuToolButtonValue*>(twcitizens->indexWidget(idxArmor));

								QModelIndex idxClothing = twcitizens->model()->index(row, col++);
								//twcitizens->item(idxClothing.row(), idxClothing.column())->setSelected(isSelected);
								cbClothing = dynamic_cast<AuToolButtonValue*>(twcitizens->indexWidget(idxClothing));

								QModelIndex idxWeapon = twcitizens->model()->index(row, col++);
								//twcitizens->item(idxWeapon.row(), idxWeapon.column())->setSelected(isSelected);
								cbWeapon = dynamic_cast<AuToolButtonValue*>(twcitizens->indexWidget(idxWeapon));

								QModelIndex idxAmmo = twcitizens->model()->index(row, col++);
								//twcitizens->item(idxAmmo.row(), idxAmmo.column())->setSelected(isSelected);
								cnAmmo = dynamic_cast<AuToolButtonNumberValue*>(twcitizens->indexWidget(idxAmmo));
							}
							break;
						}
					}

					// Remove the id from the 'intable' list. We'll remove the ones that don't get updated at the end.
					intable.Remove(citizen->Id());
				}
				else
				{
					// The citizen is not in the table so insert a new record.
					twcitizens->insertRow(0);

					int col=0;
					AuTableWidgetRankItem* twrank = new AuTableWidgetRankItem(rank, rankIcon, rankAbbr, rankName);
					twcitizens->setItem(0,col++, twrank);
					twrank->setSelected(isSelected);

					QString citname = citizen->Name();
					if(citname.left(rankAbbr.length()) == rankAbbr)
						citname = citname.mid(rankAbbr.length()+1);
					QTableWidgetItem* twname = new QTableWidgetItem(citname);
					twname->setData(Qt::UserRole, citizen->Id().GetID());
					twname->setToolTip(name);
					twcitizens->setItem(0,col++, twname);
					twname->setSelected(isSelected);

					QTableWidgetItem* twstat = new QTableWidgetItem(statusShort);
					twstat->setData(Qt::UserRole, npcRole);		// column 2 stores npc role, for selecting by role
					twstat->setToolTip(status);
					twcitizens->setItem(0,col++, twstat);
					twstat->setSelected(isSelected);

					QTableWidgetItem* twloc = new QTableWidgetItem(locationShort);
					twloc->setToolTip(location);
					twcitizens->setItem(0,col++, twloc);
					twloc->setSelected(isSelected);

					QTableWidgetItem* twposture = new QTableWidgetItem(posture, "");
					twposture->setToolTip(postureTip);
					twcitizens->setItem(0,col++, twposture);
					twposture->setSelected(isSelected);

					QTableWidgetItem* tworder = new QTableWidgetItem(orderIcon,order);
					twcitizens->setItem(0,col++, tworder);
					tworder->setSelected(isSelected);

					// Cannot manipulate the inventory of avatars or passengers.
					if(!dynamic_cast<DaPerson*>(citizen) && !citizen->IsNpcPassenger())
					{
						cbHelmet = new AuToolButtonValue(citizen->Id().GetID());
						cbHelmet->setToolTip(tr("<html>Helmet<br>"
									"When a new helmet is selected, current helmet is returned to the hold."
									"</html>"));
						cbHelmet->setObjectName(QString("helmet%1").arg(citizen->Id().GetID()));
						connect(cbHelmet, SIGNAL(ClickedValue(int)), SLOT(HelmetClicked(int)));
						QModelIndex idxHelmet = twcitizens->model()->index(0, col++);
						twcitizens->setIndexWidget(idxHelmet, cbHelmet);
						//twcitizens->item(idxHelmet.row(), idxHelmet.column())->setSelected(isSelected);

						cbArmor = new AuToolButtonValue(citizen->Id().GetID());
						cbArmor->setToolTip(tr("<html>Armor<br>"
									"When new armor is selected, current armor is returned to the hold."
									"</html>"));
						cbArmor->setObjectName(QString("armor%1").arg(citizen->Id().GetID()));
						connect(cbArmor, SIGNAL(ClickedValue(int)), SLOT(ArmorClicked(int)));
						QModelIndex idxArmor = twcitizens->model()->index(0, col++);
						twcitizens->setIndexWidget(idxArmor, cbArmor);
						//twcitizens->item(idxArmor.row(), idxArmor.column())->setSelected(isSelected);

						cbClothing = new AuToolButtonValue(citizen->Id().GetID());
						cbClothing->setToolTip(tr("<html>Clothing<br>"
									"When new clothing is selected, current clothing is returned to the hold."
									"</html>"));
						cbClothing->setObjectName(QString("clothing%1").arg(citizen->Id().GetID()));
						connect(cbClothing, SIGNAL(ClickedValue(int)), SLOT(ClothingClicked(int)));
						QModelIndex idxClothing = twcitizens->model()->index(0, col++);
						twcitizens->setIndexWidget(idxClothing, cbClothing);
						//twcitizens->item(idxClothing.row(), idxClothing.column())->setSelected(isSelected);

						cbWeapon = new AuToolButtonValue(citizen->Id().GetID());
						cbWeapon->setObjectName(QString("weapon%1").arg(citizen->Id().GetID()));
						cbWeapon->setToolTip(tr("<html>Weapon<br>"
									"When a new weapon is selected, current weapon and ammunition are returned to the hold. "
									"New ammunition appropriate to the weapon is grabbed."
									"</html>"));
						connect(cbWeapon, SIGNAL(ClickedValue(int)), SLOT(WeaponClicked(int)));
						QModelIndex idxWeapon = twcitizens->model()->index(0, col++);
						twcitizens->setIndexWidget(idxWeapon, cbWeapon);
						//twcitizens->item(idxWeapon.row(), idxWeapon.column())->setSelected(isSelected);

						cnAmmo = new AuToolButtonNumberValue(citizen->Id().GetID());
						cnAmmo->setToolTip(tr("<html>Ammunition<br>"
									"When new ammunition is selected, current ammunition is returned to the hold."
									"</html"));
						cnAmmo->setObjectName(QString("ammo%1").arg(citizen->Id().GetID()));
						cnAmmo->SpinBox()->setRange(0,5);
						connect(cnAmmo, SIGNAL(ClickedValue(int)), SLOT(AmmoClicked(int)));
						connect(cnAmmo, SIGNAL(CurrentNumberChanged(int,int)), SLOT(AmmoCountChanged(int,int)));
						QModelIndex idxAmmo = twcitizens->model()->index(0, col++);
						twcitizens->setIndexWidget(idxAmmo, cnAmmo);
						//twcitizens->item(idxAmmo.row(), idxAmmo.column())->setSelected(isSelected);
					}
				}

				// Get the current weapon in hand, which is invalid if my hand is empty.
				const AuGear* inv = citizen->LockGearForRead();
				const AuItem* gearItem = inv->ItemAt(citizen->CurrentItemPos());

				// Update contents of field widgets.
				if(cbWeapon)
				{
					if(gearItem && gearItem->IsWeapon())
					{
						cbWeapon->setIcon(QIcon(gearItem->BitmapName()));
						cbWeapon->setToolTip(QString("%1 Q%2")
							.arg(gearItem->ListName())
							.arg(gearItem->Quality()));
					}
					else
					{
						cbWeapon->setIcon(QIcon());
						cbWeapon->setToolTip(tr("No Weapon"));
					}
				}

				if(cnAmmo)
				{
					const AuItemWeaponAmmo* weapon;
					if(gearItem)
						weapon = dynamic_cast<const AuItemWeaponAmmo*>(gearItem);
					else
						weapon = 0;

					if(weapon)
					{
						// Count units of ammunition in my inventory that fit the weapon.
						COMMODITYID ammoCID = CID_Invalid;
						int ammoCount = 0;
						QString ammoTip;
						for(AuGear::const_iterator ii = inv->cbegin(); ii != inv->cend(); ii++)
						{
							// Skip non-ammo items.
							const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
							if(ammo)
							{
								if(weapon->Ammo1Type() != ammo->AmmoType())
									continue;
								if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
									continue;
								// Found ammo for current weapon.
								if(ammoCID == CID_Invalid)
								{
									ammoCID = ammo->CommodityId();
									ammoTip = QString("%1 Q%2")
										.arg(ammo->ListName())
										.arg(ammo->Quality());
								}
								if(ammoCID == ammo->CommodityId())
									ammoCount++;
							}
						}

						if(ammoCID != CID_Invalid)
						{
							cnAmmo->ToolButton()->setIcon(QIcon(g_Au.Commodity(ammoCID).BitmapName()));
							cnAmmo->SpinBox()->setValue(ammoCount);
							cnAmmo->ToolButton()->setToolTip(ammoTip);
						}
						else
						{
							cnAmmo->ToolButton()->setIcon(QIcon());
							cnAmmo->SpinBox()->setValue(0);
							cnAmmo->ToolButton()->setToolTip(tr("No Ammo"));
						}

						cnAmmo->ToolButton()->setEnabled(true);
						cnAmmo->SpinBox()->setEnabled(true);
					}
					else
					{
						cnAmmo->ToolButton()->setIcon(QIcon());
						cnAmmo->ToolButton()->setToolTip(tr("No Ammo"));
						cnAmmo->SpinBox()->setValue(0);

						cnAmmo->ToolButton()->setEnabled(false);
						cnAmmo->SpinBox()->setEnabled(false);
					}
				}

				if(cbHelmet)
				{
					const AuItem* gearItem = inv->HelmetItemWorn();
					if(gearItem)
					{
						cbHelmet->setIcon(QIcon(gearItem->BitmapName()));
						cbHelmet->setToolTip(QString("%1 Q%2")
							.arg(gearItem->ListName())
							.arg(gearItem->Quality()));
					}
					else
					{
						cbHelmet->setIcon(QIcon());
						cbHelmet->setToolTip(tr("No Helmet"));
					}
				}

				if(cbArmor)
				{
					const AuItem* gearItem = inv->ArmorItemWorn();
					if(gearItem)
					{
						cbArmor->setIcon(QIcon(gearItem->BitmapName()));
						cbArmor->setToolTip(QString("%1 Q%2")
							.arg(gearItem->ListName())
							.arg(gearItem->Quality()));
					}
					else
					{
						cbArmor->setIcon(QIcon());
						cbArmor->setToolTip(tr("No Armor"));
					}
				}

				if(cbClothing)
				{
					const AuItem* gearItem = inv->ClothingItemWorn();
					if(gearItem)
					{
						cbClothing->setIcon(QIcon(gearItem->BitmapName()));
						cbClothing->setToolTip(QString("%1 Q%2")
							.arg(gearItem->ListName())
							.arg(gearItem->Quality()));
					}
					else
					{
						cbClothing->setIcon(QIcon());
						cbClothing->setToolTip(tr("No Clothing"));
					}
				}
				citizen->UnlockGear();
			}
		}
	}
	pView->UnlockSceneAtom();

	// The view may have units that are selected that are not crew or troops.
	for(AuSelectedUnitArray::const_iterator si = selected.cbegin(); si != selected.cend(); si++)
	{
		if(citizensInTable.contains((*si).Id()))
			continue;	// The selected unit is a crew or troop. We've already handled that.

		if(intable.contains((*si).Id()))
		{
			// The selected unit is already in our table. Insure it appears selected.
			for(int row=0; row<twcitizens->rowCount(); row++)
			{
				QTableWidgetItem* item = twcitizens->item(row, 1);
				if(AuId(item->data(Qt::UserRole).toInt()) == (*si).Id())
				{
					// Found the unit's row in the table.
					// Update the fields.
					item->setText((*si).Name());
					item->setToolTip("");
					item->setSelected(true);

					item = twcitizens->item(row, 0);
					item->setData(Qt::UserRole, (*si).Rank());	// column 0 stores rank value
					item->setText((*si).RankAbbr());
					item->setToolTip((*si).RankName());
					item->setSelected(true);

					int col=2;
					item = twcitizens->item(row, col++);
					item->setData(Qt::UserRole, (*si).NpcRole());	// column 2 stores npc role, for selecting by role
					item->setText("");
					item->setToolTip("");
					item->setSelected(true);

					item = twcitizens->item(row, col++);
					item->setText("");
					item->setToolTip("");
					item->setSelected(true);

					item = twcitizens->item(row, col++);
					item->setText("");
					item->setToolTip("");
					item->setSelected(true);

					item = twcitizens->item(row, col++);
					item->setText("");
					item->setToolTip("");
					item->setSelected(true);
					break;
				}
			}

			intable.Remove((*si).Id());
		}
		else
		{
			// The selected unit is not in our table yet.
			twcitizens->insertRow(0);

			QIcon rankIcon(":data/ContactNew.png");
			int col=0;
			AuTableWidgetRankItem* twrank = new AuTableWidgetRankItem((*si).Rank(), rankIcon, (*si).RankAbbr(), (*si).RankName());
			twcitizens->setItem(0,col++, twrank);
			twrank->setSelected(true);

			QTableWidgetItem* twname = new QTableWidgetItem((*si).Name());
			twname->setData(Qt::UserRole, (*si).Id().GetID());
			twname->setToolTip("");
			twcitizens->setItem(0,col++, twname);
			twname->setSelected(true);

			QTableWidgetItem* twstat = new QTableWidgetItem("");
			twstat->setData(Qt::UserRole, (*si).NpcRole());		// column 2 stores npc role, for selecting by role
			twstat->setToolTip("");
			twcitizens->setItem(0,col++, twstat);
			twstat->setSelected(true);

			QTableWidgetItem* twloc = new QTableWidgetItem("");
			twloc->setToolTip("");
			twcitizens->setItem(0,col++, twloc);
			twloc->setSelected(true);

			QTableWidgetItem* twposture = new QTableWidgetItem("");
			twposture->setToolTip("");
			twcitizens->setItem(0,col++, twposture);
			twposture->setSelected(true);

			QTableWidgetItem* tworder = new QTableWidgetItem("");
			twcitizens->setItem(0,col++, tworder);
			tworder->setSelected(true);
		}
	}

	// Remove rows that no longer belong in the table.
	for(AuIdList::const_iterator ii = intable.cbegin(); ii != intable.cend(); ii++)
	{
		for(int row=0; row<twcitizens->rowCount(); row++)
		{
			QTableWidgetItem* item = twcitizens->item(row, 1);
			if(AuId(item->data(Qt::UserRole).toInt()) == *ii)
			{
				// Found the unit's row in the table. Remove it.
				twcitizens->removeRow(row);
				break;
			}
		}
	}

	twcitizens->setSortingEnabled(true);
	twcitizens->blockSignals(false);

	EnableControls();
}

QSize AuUnitOrders::sizeHint() const
{
	return QSize(1000,300);
}

void AuUnitOrders::EnableControls()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	const AuSelectedUnitArray& selected = pView->SelectedUnits();

	QLabel* lbselcount = findChild<QLabel*>("lbselcount");
	if(lbselcount)
	{
		if(selected.count() == 1)
			lbselcount->setText(tr("One Unit Selected"));
		else if(selected.count() > 1)
			lbselcount->setText(tr("%1 Units Selected").arg(selected.count()));
		else
			lbselcount->setText(tr("No Units Selected"));
	}

	bool okToIssueOrders = selected.isEmpty() ? false : true;

	const char* buttonNames[] =
	{
		"pbattack",
		"pbberserk",
		"pbclosedoor",
		"pbcomehere",
		"pbdefarea",
		"pbdefself",
		"pbfollowme",
		"pbfollowtarget",
		"pbgarrison",
		"pbgetin",
		"pbgetitem",
		"pbgetout",
		"pbgoto",
		"pbgotoroom",
		"pbgotosurgery",
		"pbholdfire",
		"pboffense",
		"pbopendoor",
		"pbrelcommand",
		"pbstations",
		"pbstay",
		"pbtakecommand",
		"pbmountme",
		"pbmounttarget",
		"pbdroprider",
	};
	int buttonNameCount = sizeof(buttonNames)/sizeof(const char*);
	for(int bn=0; bn<buttonNameCount; bn++)
	{
		AuIconButton* pb = findChild<AuIconButton*>(buttonNames[bn]);
		if(pb)
			pb->setEnabled(okToIssueOrders);
	}
}

void AuUnitOrders::AttackTarget()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_AttackTarget);
}

void AuUnitOrders::BattleStations()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_BattleStation);
}

void AuUnitOrders::CloseDoor()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_CloseDoor);
}

void AuUnitOrders::ComeHere()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_ComeHere);
}

void AuUnitOrders::FollowMe()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_FollowMe);
}

void AuUnitOrders::FollowTarget()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_FollowTarget);
}

void AuUnitOrders::Garrison()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_Garrison);
}

void AuUnitOrders::GetIn()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_GetIn);
}

void AuUnitOrders::GetOut()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_GetOut);
}

void AuUnitOrders::EnterSurgery()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_EnterSurgery);
}

void AuUnitOrders::TakeCommand()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_TakeCommand);
}

void AuUnitOrders::ReleaseCommand()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_ReleaseCommand);
}

void AuUnitOrders::GoToRoom()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	AuGoToRoom dlg(pFrame);
	if(pFrame->ExecDialog(dlg) != QDialog::Accepted)
		return;
	QString roomName = dlg.RoomName();
	if(roomName.isEmpty())
		return;

	pView->SquadIssueOrder(ORDER_GoToRoom, roomName);
}

void AuUnitOrders::GoToTarget()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_GoToTarget);
}

void AuUnitOrders::OpenDoor()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_OpenDoor);
}

void AuUnitOrders::GetItem()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_GetItem);
}

void AuUnitOrders::Stay()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_Stay);
}

void AuUnitOrders::MountMe()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_MountMe);
}

void AuUnitOrders::MountTarget()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_MountTarget);
}

void AuUnitOrders::DropRider()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssueOrder(ORDER_DropRider);
}

void AuUnitOrders::RestoreSettings()
{
	// Restore the sizes of the table widget's columns 1 through 4, not 0 which is the automatically sized column.
	for(int col=0; col<twcitizens->columnCount(); col++)
	{
		if(col == 1)
			continue;	// skip the automatically sized column
		int colwid = g_Au.Settings().readNumEntry(QString("Widget/Orders/ColumnWid%1").arg(col),-123);
		if(colwid != -123)
			twcitizens->setColumnWidth(col, colwid);
	}
}

void AuUnitOrders::SaveSettings()
{
	// Save the sizes of the table widget's columns.
	for(int col=0; col<twcitizens->columnCount(); col++)
		g_Au.Settings().writeEntry(QString("Widget/Orders/ColumnWid%1").arg(col), twcitizens->columnWidth(col));
}

void AuUnitOrders::DefendAreaPosture()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssuePosture(POSTURE_DefendArea);
}

void AuUnitOrders::DefendSelfPosture()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssuePosture(POSTURE_DefendSelf);
}

void AuUnitOrders::HoldFirePosture()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssuePosture(POSTURE_HoldFire);
}

void AuUnitOrders::OffensivePosture()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssuePosture(POSTURE_Offensive);
}

void AuUnitOrders::BerserkPosture()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	pView->SquadIssuePosture(POSTURE_Berserk);
}

void AuUnitOrders::DeselectAllUnits()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
	pView->DeselectAllUnits();
}

void AuUnitOrders::SelectionChanged()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Set the selection in the view to match my new selection state.
	pView->DeselectAllUnits();
	QItemSelectionModel* ism = twcitizens->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// rank is stored on column 0
				QTableWidgetItem* twrank = twcitizens->item((*si).row(), 0);
				int rank = twrank->data(Qt::UserRole).toInt();
				QString rankAbbr = twrank->text();
				QString rankName = twrank->toolTip();

				// atom id and name are stored on column 0
				QTableWidgetItem* twname = twcitizens->item((*si).row(), 1);
				AuId selId(twname->data(Qt::UserRole).toInt());

				// npc role is stored on column 2
				QTableWidgetItem* twstat = twcitizens->item((*si).row(), 2);
				NPC_ROLE npcRole = (NPC_ROLE)twstat->data(Qt::UserRole).toInt();

				pView->SelectUnit(selId, twname->text(), rank, rankAbbr, rankName, npcRole, true);
			}
	}
}

void AuUnitOrders::SelectOfficers()
{
	//qDebug("SelectOfficers");
	SelectType(ROLE_Officer);
}

void AuUnitOrders::SelectCrew()
{
	//qDebug("SelectCrew");
	SelectType(ROLE_Crew);
}

void AuUnitOrders::SelectPassengers()
{
	//qDebug("SelectPassengers");
	SelectType(ROLE_Passenger);
}

void AuUnitOrders::SelectTroops()
{
	//qDebug("SelectTroops");
	SelectType(ROLE_Troop);
}

void AuUnitOrders::SelectType(int selRole)
{
	//qDebug("SelectCrew");
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	for(int row=0; row<twcitizens->rowCount(); row++)
	{
		// npc role is stored on column 2
		QTableWidgetItem* twstat = twcitizens->item(row, 2);
		NPC_ROLE npcRole = (NPC_ROLE)twstat->data(Qt::UserRole).toInt();
		if(selRole == npcRole)
		{
			// rank is stored on column 0
			QTableWidgetItem* twrank = twcitizens->item(row, 0);
			int rank = twrank->data(Qt::UserRole).toInt();
			QString rankAbbr = twrank->text();
			QString rankName = twrank->toolTip();

			// atom id and name are stored on column 0
			QTableWidgetItem* twname = twcitizens->item(row, 1);
			AuId selId(twname->data(Qt::UserRole).toInt());

			pView->SelectUnit(selId, twname->text(), rank, rankAbbr, rankName, npcRole, true);
		}
	}
}

QString AuUnitOrders::OrderChannel() const
{
	int chanIdx = m_Channels->currentIndex();
	if(chanIdx >= 0)
		return m_Channels->itemText(chanIdx);
	return AuCommStdChannel::CrewChannelName();
}

void AuUnitOrders::AmmoClicked(int unitId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a menu of the choices available.
	QMenu menu(this);

	QAction* cur = 0;
	QAction* none = 0;
	QHash<COMMODITYID, QAction*> cargo;

	// List other helmets that are available.
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DaSentient* citizen = dynamic_cast<const DaSentient*>(system->FindIdRecursive(AuId(unitId)));
			if(citizen)
			{
				// Get the current weapon in hand, which is invalid if my hand is empty.
				const AuGear* inv = citizen->LockGearForRead();
				const AuItem* gearItem = inv->ItemAt(citizen->CurrentItemPos());

				const AuItemWeaponAmmo* weapon;
				if(gearItem)
					weapon = dynamic_cast<const AuItemWeaponAmmo*>(gearItem);
				else
					weapon = 0;

				if(weapon)
				{
					// Find ammunition in my inventory that fits the weapon.
					COMMODITYID ammoCID = CID_Invalid;
					for(AuGear::const_iterator ii = inv->cbegin(); ii != inv->cend(); ii++)
					{
						// Skip non-ammo items.
						const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
						if(ammo)
						{
							if(weapon->Ammo1Type() != ammo->AmmoType())
								continue;
							if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
								continue;
							// Found ammo for current weapon.
							if(ammoCID == CID_Invalid)
							{
								QString text = QString("%1 Q%2")
									.arg(ammo->ListName())
									.arg(ammo->Quality());
								cur = menu.addAction(QIcon(ammo->BitmapName()), text);
								menu.addSeparator();
								ammoCID = ammo->CommodityId();
								break;
							}
						}
					}

					if(citizen->IsInsideSpacecraft())
					{
						const DaVessel* ship = citizen->FindParentVesselInEnv();
						if(ship && (ship->Is(citizen->AssignedShip()) ||
									citizen->ShipRoomType() == ROOM_Barracks ||
									citizen->ShipRoomType() == ROOM_Hold))
						{
							// The ship's cargo is accessible.
							const AuItemHash& cr = ship->LockCargoForRead();
							for(AuItemHash::const_iterator ci = cr.begin(); ci != cr.end(); ci++)
							{
								if(!citizen->CanUseGear(*(*ci).second, AuGearSlot::Hand))
									continue;	// Citizen is not permitted to use the item.
								if((*ci).second->CommodityId() == ammoCID)
									continue;	// Skip ammo that is the same as my current ammo.
								if(cargo.contains((*ci).second->CommodityId()))
									continue;	// Only need one of each type.

								// Skip non-ammo items.
								const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>((*ci).second);
								if(ammo)
								{
									if(weapon->Ammo1Type() != ammo->AmmoType())
										continue;	// Skip ammo that does not fit my current weapon.
									if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
										continue;

									cargo.insert(ammo->CommodityId(), menu.addAction(QIcon(ammo->BitmapName()), ammo->ListName()));
								}
							}
							ship->UnlockCargo();
						}
					}

					none = menu.addAction(QIcon(), tr("No Ammo"));
				}
				citizen->UnlockGear();
			}
		}
	}
	pView->UnlockSceneAtom();

	// Did we add anything to the menu at all. If anything is added, we add a No Ammo entry.
	if(!none)
		return;

	QAction* action = menu.exec(QCursor::pos());
	if(!action || action == cur)
		return;

	if(action == none)
	{
		auApp->SetUnitAmmo(AuId(unitId), CID_Invalid);
		return;
	}

	for(QHash<COMMODITYID, QAction*>::iterator sel = cargo.begin(); sel != cargo.end(); sel++)
		if(*sel == action)
		{
			auApp->SetUnitAmmo(AuId(unitId), sel.key());
			break;
		}
}

void AuUnitOrders::AmmoCountChanged(int count, int unitId)
{
	// A unit can only carry up to five clips of ammo.
	if(count < 0 || count > 5)
		return;
	QString widgetName = QString("ammo%1").arg(unitId);
	AuToolButtonNumberValue* cnAmmo = findChild<AuToolButtonNumberValue*>(widgetName);
	if(cnAmmo)
		auApp->SetUnitAmmoCount(AuId(unitId), count);
}

void AuUnitOrders::ArmorClicked(int unitId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a menu of the choices available.
	QMenu menu(this);

	QAction* cur = 0;
	QAction* none = 0;
	QHash<QAction*, QByteArray> cargo;

	// List other helmets that are available.
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DaSentient* citizen = dynamic_cast<const DaSentient*>(system->FindIdRecursive(AuId(unitId)));
			if(citizen)
			{
				// Current item.
				const AuGear* inv = citizen->LockGearForRead();
				const AuItem* gearItem = inv->ArmorItemWorn();
				if(gearItem)
				{
					QString text = QString("%1 Q%2")
						.arg(gearItem->ListName())
						.arg(gearItem->Quality());
					cur = menu.addAction(QIcon(gearItem->BitmapName()), text);
					menu.addSeparator();
				}

				if(citizen->IsInsideSpacecraft())
				{
					const DaVessel* ship = citizen->FindParentVesselInEnv();
					if(ship && (ship->Is(citizen->AssignedShip()) ||
								citizen->ShipRoomType() == ROOM_Barracks ||
								citizen->ShipRoomType() == ROOM_Hold))
					{
						// The ship's cargo is accessible.
						const AuItemHash& cr = ship->LockCargoForRead();
						for(AuItemHash::const_iterator ci = cr.begin(); ci != cr.end(); ci++)
						{
							if(!(*ci).second->IsBodyArmor())
								continue;
							if(!citizen->CanUseGear(*(*ci).second, AuGearSlot::BodyWear))
								continue;	// Citizen is not permitted to wear the item at that inventory position.

							if(gearItem && gearItem->IsItem(*(*ci).second))
								continue;	// Skip helmet that is identical to my current helmet.

							QString text = QString("%1 Q%2")
								.arg((*ci).second->ListName())
								.arg((*ci).second->Quality());
							cargo.insert(menu.addAction(QIcon((*ci).second->BitmapName()), text), (*ci).second->Pack());
						}
						ship->UnlockCargo();
					}
				}
				citizen->UnlockGear();

				none = menu.addAction(QIcon(), tr("No Armor"));
			}
		}
	}
	pView->UnlockSceneAtom();

	QAction* action = menu.exec(QCursor::pos());
	if(!action || action == cur)
		return;

	if(action == none)
	{
		auApp->SetUnitArmor(AuId(unitId), AuItem(CID_Invalid, 1, 0, 0));
		return;
	}

	QHash<QAction*, QByteArray>::iterator sel = cargo.find(action);
	if(sel == cargo.end())
		return;

	AuItem* item = AuItem::Unpack(*sel);
	if(item)
	{
		auApp->SetUnitArmor(AuId(unitId), *item);
		delete item;
	}
}

void AuUnitOrders::ClothingClicked(int unitId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a menu of the choices available.
	QMenu menu(this);

	QAction* cur = 0;
	QAction* none = 0;
	QHash<QAction*, QByteArray> cargo;

	// List other helmets that are available.
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DaSentient* citizen = dynamic_cast<const DaSentient*>(system->FindIdRecursive(AuId(unitId)));
			if(citizen)
			{
				// Current item.
				const AuGear* inv = citizen->LockGearForRead();
				const AuItem* gearItem = inv->ClothingItemWorn();
				if(gearItem)
				{
					QString text = QString("%1 Q%2")
						.arg(gearItem->ListName())
						.arg(gearItem->Quality());
					cur = menu.addAction(QIcon(gearItem->BitmapName()), text);
					menu.addSeparator();
				}

				if(citizen->IsInsideSpacecraft())
				{
					const DaVessel* ship = citizen->FindParentVesselInEnv();
					if(ship && (ship->Is(citizen->AssignedShip()) ||
								citizen->ShipRoomType() == ROOM_Stateroom ||
								citizen->ShipRoomType() == ROOM_CaptainsCabin ||
								citizen->ShipRoomType() == ROOM_CrewCabin ||
								citizen->ShipRoomType() == ROOM_Hold))
					{
						// The ship's cargo is accessible.
						const AuItemHash& cr = ship->LockCargoForRead();
						for(AuItemHash::const_iterator ci = cr.begin(); ci != cr.end(); ci++)
						{
							if(!(*ci).second->IsClothing())
								continue;
							if(!citizen->CanUseGear(*(*ci).second, AuGearSlot::BodyWear))
								continue;	// Citizen is not permitted to wear the item at that inventory position.

							if(gearItem && gearItem->IsItem(*(*ci).second))
								continue;	// Skip helmet that is identical to my current helmet.

							QString text = QString("%1 Q%2")
								.arg((*ci).second->ListName())
								.arg((*ci).second->Quality());
							cargo.insert(menu.addAction(QIcon((*ci).second->BitmapName()), text), (*ci).second->Pack());
						}
						ship->UnlockCargo();
					}
				}
				citizen->UnlockGear();

				none = menu.addAction(QIcon(), tr("No Clothing"));
			}
		}
	}
	pView->UnlockSceneAtom();

	QAction* action = menu.exec(QCursor::pos());
	if(!action || action == cur)
		return;

	if(action == none)
	{
		auApp->SetUnitClothing(AuId(unitId), AuItem(CID_Invalid, 1, 0, 0));
		return;
	}

	QHash<QAction*, QByteArray>::iterator sel = cargo.find(action);
	if(sel == cargo.end())
		return;

	AuItem* item = AuItem::Unpack(*sel);
	if(item)
	{
		auApp->SetUnitClothing(AuId(unitId), *item);
		delete item;
	}
}

void AuUnitOrders::HelmetClicked(int unitId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a menu of the choices available.
	QMenu menu(this);

	QAction* cur = 0;
	QAction* none = 0;
	QHash<QAction*, QByteArray> cargo;

	// List other helmets that are available.
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DaSentient* citizen = dynamic_cast<const DaSentient*>(system->FindIdRecursive(AuId(unitId)));
			if(citizen)
			{
				// Current item.
				const AuGear* inv = citizen->LockGearForRead();
				const AuItem* gearItem = inv->HelmetItemWorn();
				if(gearItem)
				{
					QString text = QString("%1 Q%2")
						.arg(gearItem->ListName())
						.arg(gearItem->Quality());
					cur = menu.addAction(QIcon(gearItem->BitmapName()), text);
					menu.addSeparator();
				}

				if(citizen->IsInsideSpacecraft())
				{
					const DaVessel* ship = citizen->FindParentVesselInEnv();
					if(ship && (ship->Is(citizen->AssignedShip()) ||
								citizen->ShipRoomType() == ROOM_Barracks ||
								citizen->ShipRoomType() == ROOM_Hold))
					{
						// The ship's cargo is accessible.
						const AuItemHash& cr = ship->LockCargoForRead();
						for(AuItemHash::const_iterator ci = cr.begin(); ci != cr.end(); ci++)
						{
							if(!(*ci).second->IsHelmet())
								continue;
							if(!citizen->CanUseGear(*(*ci).second, AuGearSlot::Head))
								continue;	// Citizen is not permitted to wear the item at that inventory position.

							if(gearItem && gearItem->IsItem(*(*ci).second))
								continue;	// Skip helmet that is identical to my current helmet.

							QString text = QString("%1 Q%2")
								.arg((*ci).second->ListName())
								.arg((*ci).second->Quality());
							cargo.insert(menu.addAction(QIcon((*ci).second->BitmapName()), text), (*ci).second->Pack());
						}
						ship->UnlockCargo();
					}
				}
				citizen->UnlockGear();

				none = menu.addAction(QIcon(), tr("No Helmet"));
			}
		}
	}
	pView->UnlockSceneAtom();

	QAction* action = menu.exec(QCursor::pos());
	if(!action || action == cur)
		return;

	if(action == none)
	{
		auApp->SetUnitHelmet(AuId(unitId), AuItem(CID_Invalid, 1, 0, 0));
		return;
	}

	QHash<QAction*, QByteArray>::iterator sel = cargo.find(action);
	if(sel == cargo.end())
		return;

	AuItem* item = AuItem::Unpack(*sel);
	if(item)
	{
		auApp->SetUnitHelmet(AuId(unitId), *item);
		delete item;
	}
}

void AuUnitOrders::WeaponClicked(int unitId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a menu of the choices available.
	QMenu menu(this);

	QAction* cur = 0;
	QAction* none = 0;
	QHash<QAction*, QByteArray> cargo;

	// List other weapons that are available.
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DaSentient* citizen = dynamic_cast<const DaSentient*>(system->FindIdRecursive(AuId(unitId)));
			if(citizen)
			{
				// Current item.
				const AuGear* inv = citizen->LockGearForRead();
				const AuItem* gearItem = inv->ItemAt(citizen->CurrentItemPos());
				if(gearItem && gearItem->IsWeapon())
				{
					QString text = QString("%1 Q%2")
						.arg(gearItem->ListName())
						.arg(gearItem->Quality());
					cur = menu.addAction(QIcon(gearItem->BitmapName()), text);
					menu.addSeparator();
				}

				if(citizen->IsInsideSpacecraft())
				{
					const DaVessel* ship = citizen->FindParentVesselInEnv();
					if(ship && (ship->Is(citizen->AssignedShip()) ||
								citizen->ShipRoomType() == ROOM_Barracks ||
								citizen->ShipRoomType() == ROOM_Hold))
					{
						// The ship's cargo is accessible.
						const AuItemHash& cr = ship->LockCargoForRead();
						for(AuItemHash::const_iterator ci = cr.begin(); ci != cr.end(); ci++)
						{
							if(!citizen->CanUseGear(*(*ci).second, AuGearSlot::Hand))
								continue;	// Citizen is not permitted to wear the item at that inventory position.

							if(!(*ci).second->IsWeapon())
								continue;	// Skip non-weapon items.
							if((*ci).second->IsDetonatorWeapon() || (*ci).second->IsFuseWeapon())
								continue;	// Skip detonator and fuse weapons. AI can't use them safely.

							if(gearItem && gearItem->IsItem(*(*ci).second))
								continue;	// Skip helmet that is identical to my current helmet.

							QString text = QString("%1 Q%2")
								.arg((*ci).second->ListName())
								.arg((*ci).second->Quality());
							cargo.insert(menu.addAction(QIcon((*ci).second->BitmapName()), text), (*ci).second->Pack());
						}
						ship->UnlockCargo();
					}
				}
				citizen->UnlockGear();

				none = menu.addAction(QIcon(), tr("No Weapon"));
			}
		}
	}
	pView->UnlockSceneAtom();

	QAction* action = menu.exec(QCursor::pos());
	if(!action || action == cur)
		return;

	if(action == none)
	{
		auApp->SetUnitWeapon(AuId(unitId), AuItem(CID_Invalid, 1, 0, 0));
		return;
	}

	QHash<QAction*, QByteArray>::iterator sel = cargo.find(action);
	if(sel == cargo.end())
		return;

	AuItem* item = AuItem::Unpack(*sel);
	if(item)
	{
		auApp->SetUnitWeapon(AuId(unitId), *item);
		delete item;
	}
}

void AuUnitOrders::UnitRightClicked(AuId unitId)
{
	QMenu menu(this);
	QAction* resume = menu.addAction(QIcon(":data/c_Certificate.png"), tr("View Resume..."));

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	if(action == resume)
		ViewResume(unitId);
}

void AuUnitOrders::ViewResume(AuId unitId)
{
	AuFrame* frame = auApp->Frame();
	if(!frame)
		return;
	AuView* view = frame->View();
	if(!view)
		return;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		// Build a roster of the citizens in the scene.
		DaAtom* system = person->FindEnvironmentAtom();
		if(system)
		{
			const DaSentient* unit = dynamic_cast<const DaSentient*>(system->FindIdRecursive(unitId));
			if(unit)
			{
				AuNpcResume* dlg = new AuNpcResume(*unit, frame);
				dlg->show();
			}
		}
	}
	view->UnlockSceneAtom();
}

