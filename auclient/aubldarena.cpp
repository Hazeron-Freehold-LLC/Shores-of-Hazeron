//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldarena.cpp,v 1.8 2022/02/19 18:08:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldarena.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "daarena.h"
#include "daperson.h"
#include "demanufacturingprocess.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>

AuBldArena::AuBldArena(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	connect(auApp, SIGNAL(ArenaReadySignal(AuArenaReady)), SLOT(ArenaReady(AuArenaReady)), Qt::QueuedConnection);

	m_Arenas = new QTableWidget(this);
	m_Arenas->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Arenas->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Arenas->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Arenas->setShowGrid(false);
	QStringList hls;
	//hls << tr("Arena Match") << tr("Type") << tr("Stage") << tr("Password");
	hls << tr("Arena Match") << tr("Type") << tr("Password");
	m_Arenas->setColumnCount(hls.count());
	m_Arenas->setHorizontalHeaderLabels(hls);
	m_Arenas->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_Arenas->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_Arenas->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	//m_Arenas->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
	m_Arenas->setColumnWidth(1, 150);
	//m_Arenas->setColumnWidth(2, 150);
	m_Arenas->setColumnWidth(2, 70);
	m_Arenas->sortByColumn(0, Qt::AscendingOrder);
	QHeaderView* vhdr = m_Arenas->verticalHeader();
	vhdr->hide();
	connect(m_Arenas, SIGNAL(itemSelectionChanged()), SLOT(MatchChanged()));
	m_Arenas->setToolTip(tr("<html>Select an arena match to join.</html>"));

	m_Teams = new QComboBox(this);
	m_Teams->setToolTip(tr("<html>Select a fleet to join.</html>"));

	m_MatchDesc = new QLabel(this);
	m_MatchDesc->setWordWrap(true);

	QLabel* joinlabel = new QLabel("<html><br><b>Join Match Options</b></html>", this);

	QLabel* hlabel = new QLabel("<html><br><b>New Match Options</b></html>", this);

	QLabel* passwordJoinLabel = new QLabel(tr("Join Password"), this);
	passwordJoinLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* passwordNewLabel = new QLabel(tr("New Password"), this);
	passwordNewLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_PasswordJoin = new QLineEdit(this);
	m_PasswordJoin->setToolTip(tr("<html>Enter the password needed to join the selected arena match. "
				"Control is enabled when the selected match requires a password. "
				"The password is case sensitive.</html>"));

	m_PasswordNew = new QLineEdit(this);
	m_PasswordNew->setToolTip(tr("<html>Enter the password others must enter to join the new arena match. "
				"The password is case sensitive.</html>"));

	m_JoinArenaButton = new QPushButton(tr("Join Match"), this);
	connect(m_JoinArenaButton, SIGNAL(clicked()), SLOT(JoinArena()));
	m_JoinArenaButton->setToolTip(tr("<html>Joins the selected arena match.</html>"));
	m_JoinArenaButton->setEnabled(false);

	m_NewMatchButton = new QPushButton(tr("New Match"), this);
	connect(m_NewMatchButton, SIGNAL(clicked()), SLOT(EnterArena()));
	m_NewMatchButton->setToolTip(tr("<html>Starts a new arena match using the new match options. "
				"This button will be disabled if you have not selected a spacecraft design."
				"</html>"));

	QToolButton* refreshButton = new QToolButton(this);
	refreshButton->setIcon(QIcon(":data/Sweep.png"));
	connect(refreshButton, SIGNAL(clicked()), SLOT(QueryArenas()));
	refreshButton->setToolTip(tr("<html>Refreshes the list of arena matches.</html>"));

	// Combo box for match type.
	DaArena::MATCH_TYPE defType = DaArena::SpacecraftTrial;
	m_MatchType = new QComboBox(this);
	m_MatchType->insertItem(0, DaArena::MatchTypeName(DaArena::SpacecraftDuel), QVariant(DaArena::SpacecraftDuel));
	m_MatchType->insertItem(0, DaArena::MatchTypeName(DaArena::SpacecraftTrial), QVariant(DaArena::SpacecraftTrial));
	m_MatchType->setCurrentIndex(m_MatchType->findData(QVariant(defType)));
	m_MatchType->setToolTip(tr("<html>Type of match to start.</html>"));
	connect(m_MatchType, SIGNAL(currentIndexChanged(int)), SLOT(MatchTypeChanged(int)));

	m_MatchTypeDesc = new QLabel(DaArena::MatchTypeDesc(defType), this);

	// Combo box for opponent limits.
	m_TeamSize = new QComboBox(this);
	for(int ships=1; ships<=3; ships++)
		m_TeamSize->insertItem(0, tr("%1 Spacecraft Per Fleet").arg(ships), QVariant(ships));
	m_TeamSize->setToolTip(tr("<html>Number of spacecraft per fleet. "
				"In a solo match, the AI opponent starts with this number of spacecraft while you get yours one by one.</html>"));

	// Combo box for ship design.
	m_ShipDesignsJoin = new QComboBox(this);
	m_ShipDesignsJoin->setToolTip(tr("<html>Spacecraft design to use when joining an arena match. "
				"Control is enabled when match options allow players to use their own spacecraft designs. "
				"Design is ignored if Q exceeds match limit; default design used instead. "
				"Designs in your inventory are listed.</html>"));

	// Combo box for ship design.
	m_ShipDesignsNew = new QComboBox(this);
	m_ShipDesignsNew->setToolTip(tr("<html>Spacecraft design to use when creating a new arena match. "
				"Everyone will use this design if players cannot use their own spacecraft designs. "
				"Designs in your inventory are listed.<br>"
				"<b>You must provide a spacecraft design to create a new match.</b></html>"));
	connect(m_ShipDesignsNew, SIGNAL(currentIndexChanged(int)), SLOT(ShipDesignNewChanged(int)));

	m_OpponentDesigns = new QCheckBox(tr("Players can use a different spacecraft design."), this);
	m_OpponentDesigns->setCheckState(Qt::Checked);
	m_OpponentDesigns->setToolTip(tr("<html>When unchecked, everyone will use the spacecraft design you select.</html>"));

	// Combo box for quality.
	m_MaxShipQ = new QComboBox(this);
	for(int tl=0; tl<=255; tl++)
		m_MaxShipQ->insertItem(0, tr("Max Player Design Q%1").arg(tl+1), QVariant(tl));
	m_MaxShipQ->setToolTip(tr("<html>Maximum quality allowed for player designs. "
				"This does not limit the quality of the spacecraft design chosen for the new match.</html>"));

	m_MaximizeShipQ = new QCheckBox(tr("Create all player spacecraft at Max Player Design Q."), this);
	m_MaximizeShipQ->setCheckState(Qt::Checked);
	m_MaximizeShipQ->setToolTip(tr("<html>When unchecked, player spacecraft are created at the Q of their design, "
				"which could be lower.</html>"));

	// Combo box for spawn option.
	m_SpawnOfficer = new QComboBox(this);
	m_SpawnOfficer->insertItem(0, tr("No Officer"), QVariant(DaArena::No));
	m_SpawnOfficer->insertItem(0, tr("Initial Officer"), QVariant(DaArena::Initial));
	m_SpawnOfficer->insertItem(0, tr("Respawn Officer"), QVariant(DaArena::Respawn));
	m_SpawnOfficer->setCurrentIndex(m_SpawnOfficer->findData(QVariant(DaArena::Initial)));
	m_SpawnOfficer->setToolTip(tr("<html>Spawn option for officer. "
				"In a Trial match, opposing ships are configured as target drones if they have no officers.</html>"));

	// Combo box for spawn option.
	m_SpawnCrew = new QComboBox(this);
	m_SpawnCrew->insertItem(0, tr("No Crew"), QVariant(DaArena::No));
	m_SpawnCrew->insertItem(0, tr("Initial Crew"), QVariant(DaArena::Initial));
	m_SpawnCrew->insertItem(0, tr("Respawn Crew"), QVariant(DaArena::Respawn));
	m_SpawnCrew->setCurrentIndex(m_SpawnCrew->findData(QVariant(DaArena::Initial)));
	m_SpawnCrew->setToolTip(tr("<html>Spawn option for crew members.</html>"));

	// Combo box for spawn option.
	m_SpawnTroops = new QComboBox(this);
	m_SpawnTroops->insertItem(0, tr("No Troops"), QVariant(DaArena::No));
	m_SpawnTroops->insertItem(0, tr("Initial Troops"), QVariant(DaArena::Initial));
	m_SpawnTroops->insertItem(0, tr("Respawn Troops"), QVariant(DaArena::Respawn));
	m_SpawnTroops->setCurrentIndex(m_SpawnTroops->findData(QVariant(DaArena::Initial)));
	m_SpawnTroops->setToolTip(tr("<html>Spawn option for troops.</html>"));

	// Combo box for spawn option.
	m_SpawnPassengers = new QComboBox(this);
	m_SpawnPassengers->insertItem(0, tr("No Passengers"), QVariant(DaArena::No));
	m_SpawnPassengers->insertItem(0, tr("Initial Passengers"), QVariant(DaArena::Initial));
	m_SpawnPassengers->insertItem(0, tr("Respawn Passengers"), QVariant(DaArena::Respawn));
	m_SpawnPassengers->setCurrentIndex(m_SpawnPassengers->findData(QVariant(DaArena::Initial)));
	m_SpawnPassengers->setToolTip(tr("<html>Spawn option for passengers.</html>"));

	// Combo box for spawn option.
	m_SpawnVehicles = new QComboBox(this);
	m_SpawnVehicles->insertItem(0, tr("No Vehicles"), QVariant(DaArena::No));
	m_SpawnVehicles->insertItem(0, tr("Initial Vehicles"), QVariant(DaArena::Initial));
	m_SpawnVehicles->insertItem(0, tr("Respawn Vehicles"), QVariant(DaArena::Respawn));
	m_SpawnVehicles->setCurrentIndex(m_SpawnVehicles->findData(QVariant(DaArena::Initial)));
	m_SpawnVehicles->setToolTip(tr("<html>Spawn option for vehicles.</html>"));

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(joinlabel, row,0, 1,3);
	lay->addWidget(refreshButton, row,2, 1,1);
	row++;
	lay->addWidget(m_Arenas, row,0, 1,3);
	row++;
	lay->addWidget(m_MatchDesc, row,0, 1,3);
	row++;
	lay->addWidget(m_ShipDesignsJoin, row,0, 1,2);
	row++;
	lay->addWidget(passwordJoinLabel, row,0, 1,1);
	lay->addWidget(m_PasswordJoin, row,1, 1,1);
	row++;
	lay->addWidget(m_Teams, row,1, 1,1);
	lay->addWidget(m_JoinArenaButton, row,2, 1,1);
	row++;
	lay->addWidget(hlabel, row,0, 1,3);
	row++;
	lay->addWidget(m_MatchType, row,0, 1,1);
	lay->addWidget(m_TeamSize, row,1, 1,1);
	row++;
	lay->addWidget(m_MatchTypeDesc, row,0, 1,2);
	row++;
	lay->addWidget(passwordNewLabel, row,0, 1,1);
	lay->addWidget(m_PasswordNew, row,1, 1,1);
	row++;
	lay->addWidget(m_ShipDesignsNew, row,0, 1,2);
	row++;
	lay->addWidget(m_OpponentDesigns, row,0, 1,2);
	row++;
	lay->addWidget(m_MaxShipQ, row,0, 1,1);
	lay->addWidget(m_MaximizeShipQ, row,1, 1,1);
	row++;
	lay->addWidget(m_SpawnOfficer, row,0, 1,1);
	lay->addWidget(m_SpawnCrew, row,1, 1,1);
	row++;
	lay->addWidget(m_SpawnTroops, row,0, 1,1);
	lay->addWidget(m_SpawnPassengers, row,1, 1,1);
	row++;
	lay->addWidget(m_SpawnVehicles, row,0, 1,1);
	lay->addWidget(m_NewMatchButton, row,2, 1,1);
	row++;
	lay->setRowStretch(row, 1);	// Scrunch everything to the top.
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldArena::~AuBldArena()
{
}

void AuBldArena::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(500);

	QueryArenas();
}

void AuBldArena::UpdateContents()
{
	if(!isVisible())
	{
		// Hide my widgets now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
		QGridLayout* lay = (QGridLayout*)layout();
		for(int iw=0; iw<lay->count(); iw++)
			if(lay->itemAt(iw)->widget())
				lay->itemAt(iw)->widget()->hide();

		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Don't do anything if the combo box has its list open, otherwise it's disruptive.
	if(m_ShipDesignsJoin->view() && m_ShipDesignsJoin->view()->isVisible())
		return;
	if(m_ShipDesignsNew->view() && m_ShipDesignsNew->view()->isVisible())
		return;

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() == IID_Arena)
	{
		quint32 designJoinId;
		if(m_ShipDesignsJoin->currentIndex() >= 0)
			designJoinId = m_ShipDesignsJoin->itemData(m_ShipDesignsJoin->currentIndex()).toUInt();
		else
			designJoinId = 0;

		quint32 designNewId;
		if(m_ShipDesignsNew->currentIndex() >= 0)
			designNewId = m_ShipDesignsNew->itemData(m_ShipDesignsNew->currentIndex()).toUInt();
		else
			designNewId = 0;

		// List designs in my possession.
		m_ShipDesignsJoin->blockSignals(true);
		m_ShipDesignsNew->blockSignals(true);
		m_ShipDesignsJoin->clear();
		m_ShipDesignsNew->clear();

		QSet<int> designsListed;	// So we list each one only once.
		const AuGear* inv = person->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);
			const AuItemScDesign* scitem = dynamic_cast<const AuItemScDesign*>(gitem);
			if(scitem)
			{
				if(!scitem->Exchange().CanBeManufactured())
					continue;
				if(!scitem->Exchange().AccessibleTo(person->Id(), person->EmpireId()))
					continue;

				// Found an sc design disk in the person's ready inventory.
				int mpid = (int)scitem->Exchange().DesignId();
				if(!designsListed.contains(mpid))
				{
					QString text = tr("%1 %2 by %3, Q%4")
						.arg(scitem->Exchange().DesignId())
						.arg(scitem->Exchange().DesignName())
						.arg(scitem->Exchange().DesignArchitectName())
						.arg(scitem->Exchange().DesignQuality());
					m_ShipDesignsJoin->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, mpid);
					m_ShipDesignsNew->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, mpid);
					designsListed.insert(mpid);
					m_ShipDesignInfo.insert(scitem->Exchange().DesignId(),
							AuArenaDesignInfo(scitem->Exchange().DesignQuality(), true));
				}
				continue;
			}

			const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
			if(media)
			{
				for(AuScDesignExchange::const_iterator di = media->BlueprintsSc().cbegin();
						di != media->BlueprintsSc().cend(); di++)
				{
					if(!(*di).CanBeManufactured())
						continue;
					if(!(*di).AccessibleTo(person->Id(), person->EmpireId()))
						continue;

					// Found an sc design disk in the person's ready inventory.
					int mpid = (int)(*di).DesignId();
					if(!designsListed.contains(mpid))
					{
						QString text = tr("%1 %2 by %3, Q%4")
							.arg((*di).DesignId())
							.arg((*di).DesignName())
							.arg((*di).DesignArchitectName())
							.arg((*di).DesignQuality());
						m_ShipDesignsJoin->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, mpid);
						m_ShipDesignsNew->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, mpid);
						designsListed.insert(mpid);
						m_ShipDesignInfo.insert((*di).DesignId(), AuArenaDesignInfo((*di).DesignQuality(), true));
					}
				}
				continue;
			}
		}
		person->UnlockGear();

		int curIdx = m_ShipDesignsJoin->findData(designJoinId);
		if(curIdx >= 0)
			m_ShipDesignsJoin->setCurrentIndex(curIdx);

		curIdx = m_ShipDesignsNew->findData(designNewId);
		if(curIdx >= 0)
			m_ShipDesignsNew->setCurrentIndex(curIdx);
		else
			ShipDesignNewChanged(m_ShipDesignsNew->currentIndex());

		m_ShipDesignsJoin->blockSignals(false);
		m_ShipDesignsNew->blockSignals(false);

		m_NewMatchButton->setEnabled(m_ShipDesignsNew->currentIndex() != -1);
	}
	pView->UnlockSceneAtom();
}

void AuBldArena::EnterArena()
{
	AuArenaReady newArena;

	newArena.m_MatchType = m_MatchType->itemData(m_MatchType->currentIndex()).toUInt();
	newArena.m_MaximizeShipQ = m_MaximizeShipQ->checkState() == Qt::Checked;
	newArena.m_MaxShipQ = m_MaxShipQ->itemData(m_MaxShipQ->currentIndex()).toUInt();
	newArena.m_OpponentDesignsOk = m_OpponentDesigns->checkState() == Qt::Checked;
	newArena.m_Password = m_PasswordNew->text();
	newArena.m_ShipDesign = m_ShipDesignsNew->itemData(m_ShipDesignsNew->currentIndex()).toUInt();
	newArena.m_SpawnCrew = m_SpawnCrew->itemData(m_SpawnCrew->currentIndex()).toUInt();
	newArena.m_SpawnOfficer = m_SpawnOfficer->itemData(m_SpawnOfficer->currentIndex()).toUInt();
	newArena.m_SpawnPassengers = m_SpawnPassengers->itemData(m_SpawnPassengers->currentIndex()).toUInt();
	newArena.m_SpawnTroops = m_SpawnTroops->itemData(m_SpawnTroops->currentIndex()).toUInt();
	newArena.m_SpawnVehicles = m_SpawnVehicles->itemData(m_SpawnVehicles->currentIndex()).toUInt();
	newArena.m_TeamShips = m_TeamSize->itemData(m_TeamSize->currentIndex()).toUInt();
	newArena.m_Team1Ships = newArena.m_TeamShips;
	newArena.m_Team2Ships = newArena.m_TeamShips;

	if(newArena.m_ShipDesign)
		auApp->CreateArena(newArena);
}

void AuBldArena::JoinArena()
{
	AuId arenaId;
	QTableWidgetItem* ci = m_Arenas->currentItem();
	if(ci)
	{
		QTableWidgetItem* twname = m_Arenas->item(ci->row(), 0);
		if(twname)
			arenaId = AuId(twname->data(Qt::UserRole).toUInt());
	}

	QString passwordJoin = m_PasswordJoin->text();

	quint32 shipDesignJoin = m_ShipDesignsJoin->itemData(m_ShipDesignsJoin->currentIndex()).toUInt();

	quint8 teamJoin = m_Teams->itemData(m_Teams->currentIndex()).toUInt();

	if(!arenaId.IsNull())
		auApp->JoinArena(arenaId, passwordJoin, shipDesignJoin, teamJoin);
}

void AuBldArena::MatchChanged()
{
	QString teamName;
	bool joinEnable = false;
	bool passEnable = false;
	bool designsEnable = false;
	quint8 matchState = DaArena::Aftermath;
	QString matchDesc;
	QTableWidgetItem* ci = m_Arenas->currentItem();
	if(ci)
	{
		QTableWidgetItem* twname = m_Arenas->item(ci->row(), 0);
		if(twname)
		{
			AuId id = AuId(twname->data(Qt::UserRole).toUInt());
			joinEnable = !id.IsNull();
			passEnable = twname->data(Qt::UserRole+1).toBool();
			designsEnable = twname->data(Qt::UserRole+2).toBool();
			teamName = twname->data(Qt::UserRole+3).toString();
			matchState = twname->data(Qt::UserRole+4).toUInt();
			matchDesc = twname->toolTip();
		}
	}

	m_JoinArenaButton->setEnabled(joinEnable);
	m_PasswordJoin->setEnabled(passEnable);
	m_ShipDesignsJoin->setEnabled(designsEnable);
	m_MatchDesc->setText(matchDesc);

	int curteam = m_Teams->currentIndex();
	m_Teams->clear();
	//m_Teams->addItem(tr("Observer"), DgEmpire::ArenaObserverId());
	if(matchState == DaArena::Join)
	{
		m_Teams->addItem(teamName, DgEmpire::ArenaTeam1Id());
		m_Teams->addItem(tr("Challenger"), DgEmpire::ArenaTeam2Id());
	}
	if(curteam < 0)
		curteam = m_Teams->findData(QVariant(DgEmpire::ArenaTeam2Id()));
	m_Teams->setCurrentIndex(curteam);
}

void AuBldArena::QueryArenas()
{
	m_Arenas->clearContents();
	m_MatchDesc->setText("");
	auApp->QueryArenas();
}

void AuBldArena::ArenaReady(AuArenaReady arena)
{
	m_Arenas->insertRow(0);

	int col=0;
	QString text = tr("%1 Arena %2").arg(arena.m_Name).arg(arena.m_Id.StringId());
	QIcon icon(":data/c_Spacecraft.png");
	QTableWidgetItem* twname = new QTableWidgetItem(icon, text);
	twname->setData(Qt::UserRole, arena.m_Id.GetID());
	twname->setData(Qt::UserRole+1, !arena.m_Password.isEmpty());
	twname->setData(Qt::UserRole+2, arena.m_OpponentDesignsOk);
	twname->setData(Qt::UserRole+3, arena.m_Name);
	twname->setData(Qt::UserRole+4, arena.m_MatchState);
	twname->setToolTip(tr("<html><b>%1</b><br>"
				"%20 Alpha Fleet Spacecraft <b>%2 of %13 Awaiting Commander</b><br>"
				"%20 Zulu Fleet Spacecraft <b>%3 of %13 Awaiting Commander</b><br>"
				"%20 Design <b>%12</b><br>"
				"%20 Player Designs Allowed <b>%4</b><br>"
				"%20 Max Player Design <b>Q%5</b><br>"
				"%20 Player Spacecraft Created <b>%6</b><br>"
				"%20 Officer <b>%7</b><br>"
				"%20 Crew <b>%8</b><br>"
				"%20 Troops <b>%9</b><br>"
				"%20 Passengers <b>%10</b><br>"
				"%20 Vehicles <b>%11</b>"
				"</html>")
			.arg(text)
			.arg(arena.m_Team1Ships)
			.arg(arena.m_Team2Ships)
			.arg(arena.m_OpponentDesignsOk ? tr("Yes") : tr("No"))
			.arg(arena.m_MaxShipQ)
			.arg(arena.m_MaximizeShipQ ? tr("<b>At Max Q</b>") : tr("<b>At Design Q</b>"))
			.arg(arena.m_SpawnOfficer == 0 ? tr("No") : arena.m_SpawnOfficer == 1 ? tr("Initial") : tr("Respawn"))
			.arg(arena.m_SpawnCrew == 0 ? tr("No") : arena.m_SpawnCrew == 1 ? tr("Initial") : tr("Respawn"))
			.arg(arena.m_SpawnTroops == 0 ? tr("No") : arena.m_SpawnTroops == 1 ? tr("Initial") : tr("Respawn"))
			.arg(arena.m_SpawnPassengers == 0 ? tr("No") : arena.m_SpawnPassengers == 1 ? tr("Initial") : tr("Respawn"))
			.arg(arena.m_SpawnVehicles == 0 ? tr("No") : arena.m_SpawnVehicles == 1 ? tr("Initial") : tr("Respawn"))
			.arg(arena.m_ShipDesignDesc)
			.arg(arena.m_TeamShips)
			.arg(g_Au.MiddleDotSymbol()));
	m_Arenas->setItem(0,col++, twname);

	QTableWidgetItem* twtype = new QTableWidgetItem(DaArena::MatchTypeName((DaArena::MATCH_TYPE)arena.m_MatchType));
	twtype->setToolTip(DaArena::MatchTypeDesc((DaArena::MATCH_TYPE)arena.m_MatchType));
	m_Arenas->setItem(0,col++, twtype);

	//QTableWidgetItem* twstate = new QTableWidgetItem(DaArena::MatchStateName((DaArena::MATCH_STATE)arena.m_MatchState));
	//twstate->setToolTip(DaArena::MatchStateDesc((DaArena::MATCH_STATE)arena.m_MatchState));
	//m_Arenas->setItem(0,col++, twstate);

	if(!arena.m_Password.isEmpty())
		text = "Yes";
	else
		text = "No";
	QTableWidgetItem* twpw = new QTableWidgetItem(text);
	twpw->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
	twpw->setToolTip(tr("<html<b>Password Requirement</b><br>"
				"Shows whether a password is required to join."
				"</html"));
	m_Arenas->setItem(0,col++, twpw);
}

void AuBldArena::MatchTypeChanged(int index)
{
	DaArena::MATCH_TYPE type = (DaArena::MATCH_TYPE)m_MatchType->itemData(index).toUInt();
	m_MatchTypeDesc->setText(DaArena::MatchTypeDesc(type));
}

void AuBldArena::ShipDesignNewChanged(int index)
{
	quint32 designNewId = m_ShipDesignsNew->itemData(index).toUInt();
	quint8 designQ = m_ShipDesignInfo[designNewId].m_DesignQ;
	int tlidx = m_MaxShipQ->findData(designQ);
	m_MaxShipQ->setCurrentIndex(tlidx);
}
