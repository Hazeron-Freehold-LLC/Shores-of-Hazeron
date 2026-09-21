//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldspacecraftfactory.cpp,v 1.14 2022/05/04 19:53:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldspacecraftfactory.h"
//
#include "aubuyspacecraft.h"
#include "auapp.h"
#include "aucommissionspacecraft.h"
#include "audefaultspacecraft.h"
#include "auframe.h"
#include "aupatentsget.h"
#include "aupatentsrecord.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "demanufacturingprocess.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

AuBldSpacecraftFactory::AuBldSpacecraftFactory(QWidget* parent) : QWidget(parent)
{
	m_CitizenOk = false;
	m_CompanyMaxQCur = 255;
	m_CompanyOkCur = false;
	m_InitCombos = true;
	m_timer = 0;
	m_CurMinQuality = 1;
	m_PersonEmpire = 0;
	m_QRangeMax = 255;
	m_QRangeMin = 1;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Make the controls.
	m_Running = new QLabel(this);
	m_Running->setToolTip(tr("<html>Shows the current running manufacturing process.</html>"));

	m_RunningIcon = new QLabel(this);
	m_RunningIcon->setToolTip(tr("<html>Shows the current running manufacturing process.</html>"));

	m_Status = new QLabel(this);

	m_StatusIcon = new QLabel(this);

	m_Cancel = new QPushButton(tr("Stop..."), this);
	m_Cancel->setToolTip(tr("<html>Stops the running manufacturing process. "
				"Returns materials to the city stockpile, if space permits. "
				"</html>"));
	connect(m_Cancel, SIGNAL(clicked(bool)), SLOT(CancelPushed()));

	m_Fetch = new QPushButton(tr("Fetch"), this);
	m_Fetch->setToolTip(tr("<html>"
				"<p>"
				"Fetches commodities from the city's stockpile that are used by the current manufacturing process. "
				"Commodities that can be used from your ready gear, such as tools, are not fetched until the process is run."
				"</p>"
				"<p>"
				"When there is no current process, selects the first ship in the order queue. "
				"Selects the default process if no ship orders are waiting. "
				"</p>"
				"</html>"));
	connect(m_Fetch, SIGNAL(clicked(bool)), SLOT(FetchPushed()));

	m_Run = new QPushButton(tr("Run"), this);
	m_Run->setToolTip(tr("<html>"
				"<p>"
				"Runs the current process. "
				"The process will not run until all needed components are available. "
				"That means all commodities consumed by the process must be fetched and "
				"all commodities <i>required</i> by the process must be in the city's stockpile or your ready gear. "
				"</p>"
				"<p>"
				"When there is no current process, selects the first ship in the order queue. "
				"Selects the default process if no ship orders are waiting. "
				"</p>"
				"%1 <b>Run</b> performs a <b>Fetch</b> first; you don't really have to push the <b>Fetch</b> button. "
				"<br>"
				"%1 Tools in your ready gear are used when <b>Run</b> is pressed. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_Run, SIGNAL(clicked(bool)), SLOT(GoPushed()));

	m_EnableWorker = new QCheckBox(tr("Employ Citizen"), this);
	m_EnableWorker->setToolTip(
			tr("<html><b>Employ Citizen</b><br>"
				"Enables citizens of the city and players to fetch materials and to run the process. "
				"Citizens do not change the manufacturing process; they only run it.<br>"
				"When disabled, players must be in the chain-of-command of the city to fetch materials or run the process.<br>"
				"You must have governing authority to change this setting.</html>"));
	connect(m_EnableWorker, SIGNAL(stateChanged(int)), SLOT(CitizenEnableChanged(int)));

	m_LastOutput = new QLabel(this);
	m_LastOutput->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_LastOutput->setToolTip(tr("<html>Shows quality of the spacecraft or commodity made in the last run of the process. "
				"Quality ranges from 1 to 255. "
				"A reason is displayed when a process fails to complete properly.<br>"
				"Materials are not consumed when the ship cannot be made.</html>"));

	m_Components = new QListWidget(this);
	m_Components->setToolTip(tr("<html>Commodities consumed by the manufacturing process. "
				"All consumed commodities must be fetched before the process will run.</html>"));
	m_Components->setMinimumHeight(10);

	m_Modifiers = new QTableWidget(0,3,this);	// 0 rows, 3 columns
	m_Modifiers->setMinimumWidth(74);
	m_Modifiers->setMaximumWidth(74);
	m_Modifiers->setMinimumHeight(10);
	m_Modifiers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Modifiers->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Modifiers->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Modifiers->setShowGrid(false);
	m_Modifiers->horizontalHeader()->hide();
	m_Modifiers->horizontalHeader()->setDefaultSectionSize(24);
	m_Modifiers->horizontalHeader()->setMinimumSectionSize(24);
	m_Modifiers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_Modifiers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_Modifiers->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_Modifiers->verticalHeader()->hide();
	m_Modifiers->verticalHeader()->setDefaultSectionSize(18);
	m_Modifiers->setToolTip(tr("<html>Commodities that affect the production line without being consumed. "
				"Though not directly consumed, these commodities may wear out when used by the production line.<br>"
				"<i>Required</i> commodities must be available in your ready inventory or the city's inventory "
				"before the production line will run.</html>"));

	m_MakeWhat = new QPushButton(this);
	m_MakeWhat->setToolTip(tr("<html>"
				"<p>"
				"Selects the default manufacturing process to run. "
				"The <i>None</i> option lets the factory build ships to order. "
				"</p>"
				"<p>"
				"Default spacecraft are built as government-owned <i>fleet</i> vessels. "
				"Fleet ships offer a command option for players who cannot afford to buy a spacecraft. "
				"Cargo trade transactions using fleet spacecraft are subject to fleet tax. "
				"Fuel, repairs and munitions are free at friendly ports. "
				"</p>"
				"<p>"
				"<b>Changing this default does not interrupt the current running process.</b> "
				"</p>"
				"<p>"
				"You must have governing authority to select a default process. "
				"</p>"
				"</html>"));
	connect(m_MakeWhat, SIGNAL(clicked()), SLOT(ChooseDefaultProcess()));

	QLabel* lbMinQ = new QLabel(tr("Production Q"), this);
	lbMinQ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	// Spin box for quality.
	m_MinQ = new QSpinBox(this);
	m_MinQ->setRange(1,255);
	m_MinQ->setToolTip(tr("<html><p>Sets the minimum production quality for the next spacecraft that is built. "
				"<b>Does not affect the current running process.</b></p>"
				"<p>Establishes the minimum quality for commodities consumed by the process, those listed in the first column. "
				"Quality of tools and other modifiers, listed in the second column, do not affect production quality.</p>"
				"<p>Does not affect commodities fetched from the environment, as done by farming, logging, mining, "
				"drilling, refining, etc. "
				"Quality of those commodities is determined by the quality of the building, "
				"limited by the quality of the local natural resources.</p>"
				"<p>You must have governing authority to change this setting.</p>"
				"</html>"));
	connect(m_MinQ, SIGNAL(valueChanged(int)), SLOT(MinQChanged(int)));

	m_CompanyOk = new QCheckBox(tr("Build Company Spacecraft"), this);
	m_CompanyOk->setToolTip(
			tr("<html><b>Build Company Spacecraft</b><br>"
				"Enables players to purchase company spacecraft at this factory. "
				"The spacecraft is added to the order queue and it is built in turn.<br>"
				"You must have governing authority to change this setting.</html>"));
	connect(m_CompanyOk, SIGNAL(stateChanged(int)), SLOT(CompanyEnableChanged(int)));

	m_CompanyMaxQ = new QSpinBox(this);
	m_CompanyMaxQ->setRange(1,255);
	m_CompanyMaxQ->setToolTip(tr("<html>"
				"<p>"
				"Sets the maximum quality for company ship designs. "
				"Limits the maximum quality design that can be purchased as a company ship. "
				"Does not limit the maximum quality of components that are fetched to build the ship. "
				"</p>"

				"<p>"
				"Does not limit the quality of spacecraft commissioned for the fleet. "
				"</p>"

				"<p>Applies when a player buys a spacecraft. "
				"<b>Changes to this setting do not affect the running process or the ships already in the order queue.</b></p>"
				"<p>You must have governing authority to change this setting.</p>"
				"</html>"));
	m_CompanyMaxQ->setValue(m_CompanyMaxQCur);
	connect(m_CompanyMaxQ, SIGNAL(valueChanged(int)), SLOT(CompanyMaxQChanged(int)));

	m_CompanyQueue = new QListWidget(this);
	m_CompanyQueue->setToolTip(tr("<html>Queue of designs awaiting manufacture. "
				"Spacecraft will be built in turn until this list is empty. "
				"Regular production resumes after all spacecraft orders are completed. "
				"</html>"));
	m_CompanyQueue->setMinimumHeight(10);
	connect(m_CompanyQueue, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(QueueSelChanged(QListWidgetItem*, QListWidgetItem*)));

	m_Commission = new QPushButton(QIcon(":data/FleetShip.png"), tr("Commission Fleet Spacecraft..."), this);
	m_Commission->setToolTip(tr("<html>"
				"<p>"
				"Commissions a fleet spacecraft to be built using your governing authority. "
				"The design is added to the queue. "
				"Construction begins when the current running process is done. "
				"</p>"

				"<p>"
				"The ship appears on the ground if it is small enough; "
				"it may replace an inferior fleet ship, if no spaces are empty. "
				"It will appear in orbit overhead if it is a station or if it is too big to build on the ground. "
				"</p>"

				"<p>"
				"You must have governing authority to commission a spacecraft."
				"</p>"
				"</html>"));
	connect(m_Commission, SIGNAL(clicked(bool)), SLOT(Commission()));

	m_Delete = new QPushButton(QIcon(":data/ScDeleteStop.png"), tr("Cancel Fleet Spacecraft..."), this);
	m_Delete->setToolTip(tr("<html>"
				"<p>"
				"Cancels an order for a fleet spacecraft. "
				"Does not cancel orders for company spacecraft. "
				"</p>"

				"<p>"
				"You must have governing authority to cancel an order."
				"</p>"
				"</html>"));
	m_Delete->setEnabled(false);
	connect(m_Delete, SIGNAL(clicked(bool)), SLOT(Delete()));

	m_BuyCompany = new QPushButton(QIcon(":data/c_Spacecraft.png"), tr("Buy Company Spacecraft..."), this);
	m_BuyCompany->setToolTip(tr("<html>"
				"<p>"
				"Buys a spacecraft using your personal funds. "
				"The design is added to the queue. "
				"Construction begins when the current running process is done. "
				"</p>"

				"<p>"
				"You must be a member of an empire to buy spacecraft. "
				"</p>"

				"<p>"
				"When completed, mail is sent to the owner. "
				"The ship appears on the ground if it is small enough; "
				"it may replace an inferior fleet ship, if no spaces are empty. "
				"It will appear in orbit overhead if it is a station or if it is too big to build on the ground. "
				"Unlike fleet stations, company stations do not replace any stations in orbit. "
				"</p>"
				"</html>"));
	connect(m_BuyCompany, SIGNAL(clicked(bool)), SLOT(BuyOne()));

	// Add controls to get and record patents.
	QHBoxLayout* patlay = new QHBoxLayout;
	patlay->addWidget(lbMinQ);
	patlay->addWidget(m_MinQ);
	{
		m_PatentsGet = new QToolButton(this);
		m_PatentsGet->setIcon(QIcon(":data/PatentGet.png"));
		m_PatentsGet->setToolTip(tr("<html><b>Get Patent</b><br>"
					"Requests a copy of one or more patents. "
					"You must possess blank media of some kind. "
					"<br>"
					"<b>You must be in the chain of command of the city to get patents.</b>"
					"<br>"
					"<b>You cannot get patents from a city while it is occupied or under attack.</b>"
					"</html>"));
		connect(m_PatentsGet, SIGNAL(clicked()), SLOT(GetPatent()));

		m_PatentsRecord = new QToolButton(this);
		m_PatentsRecord->setIcon(QIcon(":data/PatentRecord.png"));
		m_PatentsRecord->setToolTip(tr("<html><b>Record Patent</b><br>"
					"Makes a record of a patent in the city. "
					"You must possess one or more patents. "
					"Patents can be recorded at any industry, even if the patent is not used by the industry. "
					"Patents are available to all industries in the city. "
					"<br>"
					"<b>When a new patent is developed, it is automatically recorded in the city.</b>"
					"<br>"
					"<b>You cannot record patents in a city while it is occupied or under attack.</b>"
					"</html>"));
		connect(m_PatentsRecord, SIGNAL(clicked()), SLOT(RecordPatent()));

		patlay->addStretch();
		patlay->addWidget(m_PatentsGet);
		patlay->addWidget(m_PatentsRecord);
		patlay->setMargin(2);
		patlay->setSpacing(2);
	}

	QGroupBox* gco = new QGroupBox(tr("Spacecraft Orders"));
	{
		QHBoxLayout* play = new QHBoxLayout;
		play->addWidget(m_CompanyOk);
		play->addWidget(m_CompanyMaxQ);
		play->setMargin(0);
		play->setSpacing(2);

		// Layout the group contents.
		QVBoxLayout* lay = new QVBoxLayout;
		lay->addLayout(play);
		lay->addWidget(m_BuyCompany);
		lay->addWidget(m_Commission);
		lay->addWidget(m_Delete);
		lay->addWidget(m_CompanyQueue);
		lay->setMargin(12);
		lay->setSpacing(2);
		gco->setLayout(lay);
	}

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_RunningIcon, row,0, 1,1);
	lay->addWidget(m_Running, row,1, 1,1);
	lay->addWidget(m_Cancel, row,2, 1,1);
	lay->addWidget(m_Fetch, row,3, 1,1);
	lay->addWidget(m_Run, row,4, 1,1);
	lay->addLayout(patlay, row,5, 1,2);
	row++;
	lay->addWidget(m_EnableWorker, row,0, 1,3);
	row++;
	lay->addWidget(m_StatusIcon, row,0, 1,1);
	lay->addWidget(m_Status, row,1, 1,1);
	lay->addWidget(m_LastOutput, row,2, 1,3);
	row++;
	lay->addWidget(m_MakeWhat, row,0, 1,5);
	row++;
	lay->addWidget(gco, row,0, 1,5);
	lay->setRowStretch(row,1);
	int rows = row;
	row=1;
	lay->addWidget(m_Components, row,5, rows,1);
	lay->addWidget(m_Modifiers, row,6, rows,1);
	lay->setColumnStretch(1,3);
	lay->setColumnStretch(5,2);
	lay->setColumnStretch(6,1);
	lay->setColumnMinimumWidth(3, 70);
	lay->setColumnMinimumWidth(4, 70);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldSpacecraftFactory::~AuBldSpacecraftFactory()
{
}

void AuBldSpacecraftFactory::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(500);

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	// Signal for UpdateContents to refill the combo boxes.
	// The person may have left the building and returned with more designs.
	m_InitCombos = true;
}

void AuBldSpacecraftFactory::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();

	// Hide my widget now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}

void AuBldSpacecraftFactory::UpdateContents()
{
	QString industryName;
	AuKnownPatents patents;	// Patents available.

	// Lock the root and find the scene atom.
	m_InCoC = false;
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person && (person->InIndustry() == IID_SpacecraftFactory || person->InIndustry() == IID_Capitol))
	{
		m_PersonEmpire = person->EmpireId();
		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp)
		{
			industryName = bdOp->IndustryName();
			// Get patents available at the industry. At a university, this will be all of them in town.
			bdOp->ListPatents(patents, bdOp->IndustryId());
			m_InCoC = bdOp->InChainOfCommand(person);

			DeManufacturingProcessArray& mpa = bdOp->LockManufacturingProcessesForWrite();
			if(!mpa.empty())
				UpdateContents(&mpa.FindProcess(0), bdOp);
			bdOp->UnlockManufacturingProcesses();
		}
	}
	view->UnlockSceneAtom();

	m_PatentsGet->setEnabled(m_InCoC);
	m_PatentsGet->show();
	m_PatentsRecord->show();
}

void AuBldSpacecraftFactory::UpdateContents(DeManufacturingProcess* mp, DaBd* scf)
{
	// Remember manufacturing process settings.
	m_CitizenOk = mp->ShopsEnabled();
	m_CompanyMaxQCur = scf->CompanyMaxQ();
	m_CompanyOkCur = scf->CompanyOk();
	int mpid = mp->ManufacturingProcessId();		// Negative design id or positive manufacturing process id.

	// Current running process.
	if(mpid < 0)
	{
		// Negative mpid is a ship design id.
		m_RunningIcon->setPixmap(QPixmap(g_Au.CommodityBitmapName(CID_ScDesignDisk)).scaled(24,24,
					Qt::KeepAspectRatio, Qt::SmoothTransformation));
		m_Running->setText(QString("%1 %2").arg(scf->MakingDesignId()).arg(scf->MakingDesignName()));
	}
	else
	{
		// No process is running.
		m_RunningIcon->setPixmap(QPixmap());
		m_Running->setText("");
	}

	// NOTE: 373 is the manufacturing process id for researching spacecraft factory technology.
	if(scf->DefaultDesignId() == 373)
	{
		m_MakeWhat->setIcon(QIcon(":data/mpNotReady.png"));
		m_MakeWhat->setText(tr("Change This To Something Else"));
	}
	else if(scf->DefaultDesignId())
	{
		m_MakeWhat->setIcon(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)));
		m_MakeWhat->setText(QString("%1 %2").arg(scf->DefaultDesignId()).arg(scf->DefaultDesignName()));
	}
	else
	{
		m_MakeWhat->setIcon(QIcon(":data/ScHide.png"));
		m_MakeWhat->setText(tr("Idle. No regular production."));
	}

	// Company
	{
		// Only a person who has an empire can buy a spacecraft.
		m_BuyCompany->setEnabled(m_CompanyOkCur && m_PersonEmpire);

		m_CompanyOk->blockSignals(true);
		m_CompanyOk->setCheckState(m_CompanyOkCur ? Qt::Checked : Qt::Unchecked);
		m_CompanyOk->blockSignals(false);

		m_CompanyMaxQ->blockSignals(true);
		m_CompanyMaxQ->setValue(m_CompanyMaxQCur);
		m_CompanyMaxQ->setEnabled(m_CompanyOkCur && m_InCoC);
		m_CompanyMaxQ->blockSignals(false);

		// Remember current selected item.
		int cur = m_CompanyQueue->currentRow();

		// List company designs in the queue.
		m_CompanyQueue->clear();
		for(QList<AuMakeDesign>::const_iterator qi = scf->DesignQueue().cbegin(); qi != scf->DesignQueue().cend(); qi++)
		{
			QPixmap px = QPixmap(g_Au.CommodityBitmapName(CID_ScDesignDisk)).scaled(32,32,
					Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QIcon icon(px);

			QString text;
			if((*qi).OwnerId().IsNull())
				text = QString("%1 %2")
					.arg((*qi).DesignId())
					.arg((*qi).DesignName());
			else
				text = QString("%1 %2 for %3 %4")
					.arg((*qi).DesignId())
					.arg((*qi).DesignName())
					.arg((*qi).OwnerName())
					.arg((*qi).OwnerId().StringId());

			QListWidgetItem* item = new QListWidgetItem(icon, text, m_CompanyQueue);
			item->setData(Qt::UserRole, (*qi).OwnerId().IsNull());	// true for fleet orders.
			item->setData(Qt::UserRole+1, (*qi).DesignId());
		}

		m_CompanyQueue->setCurrentRow(cur);
	}

	// Components and Modifiers
	// We'll do these simultaneously so we only have to loop through the process components once.
	m_Components->clear();
	m_Modifiers->clear();
	while(m_Modifiers->rowCount())
		m_Modifiers->removeRow(0);
	for(DeProcessComponentArray::const_iterator pci = mp->Components().cbegin(); pci != mp->Components().cend(); pci++)
	{
		QIcon icon((*pci).CommodityBitmapName());
		if((*pci).IsRequired() && (*pci).QtyConsumed() > 0)
		{
			// Construction "Materials" are those components that are
			// required AND that get consumed in the process. Some
			// required things might not be consumed but could be
			// required, such as a tool.
			QString text;
			if((*pci).QtyApplied() == 0)
			{
				// No components applied.
				text = tr("%1 needed").arg((*pci).QtyConsumed());
			}
			else if((*pci).QtyApplied() == (*pci).QtyConsumed())
			{
				// All components applied.
				if((*pci).Quality() > 0)
					text = tr("%1 Q%2").arg((*pci).QtyApplied()).arg((*pci).Quality());
				else
					text = tr("%1").arg((*pci).QtyApplied());
			}
			else
			{
				// Some components applied.
				if((*pci).Quality() > 0)
					text = tr("%1 of %2 Q%3").arg((*pci).QtyApplied())
						.arg((*pci).QtyConsumed()).arg((*pci).Quality());
				else
					text = tr("%1 of %2").arg((*pci).QtyApplied()).arg((*pci).QtyConsumed());
			}
			QListWidgetItem* lwi = new QListWidgetItem(icon, text, m_Components);
			lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
		}
		else
		{
			// Construction "Modifiers" are those components that are not required OR that do not get consumed by the process.
			QString text;
			QString modicon;
			if((*pci).EffectBitmapName())
				modicon = QString((*pci).EffectBitmapName());
			switch((*pci).Effect())
			{
			case CE_None:
				text = tr("Does Nothing");
				break;
			case CE_DecreasesTime:
				text = tr("Decreases Time");
				break;
			case CE_DecreasesMaterials:
				text = tr("Decreases Materials");
				break;
			case CE_Required:
				text = tr("Required");
				break;
			case CE_RequiredQ:
				text = tr("Required and Limits Quality"
						"<br>"
						"<small>Output quality cannot exceed the quality of this patent.</small>");
				break;
			case CE_IncreasesQuantity:
				text = tr("Increases Output");
				break;
			}

			m_Modifiers->insertRow(0);

			int col=0;
			QTableWidgetItem* lwi = new QTableWidgetItem(icon, "");
			lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
			m_Modifiers->setItem(0, col++, lwi);

			lwi = new QTableWidgetItem(QIcon(modicon), "");
			lwi->setToolTip(tr("%1 %2").arg(g_Au.CommodityName((*pci).CommodityId())).arg(text));
			m_Modifiers->setItem(0, col++, lwi);

			if((*pci).QtyApplied())
			{
				// Component has been applied.
				QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpReady.png"), "");
				lwi->setToolTip(tr("%1 Ready").arg(g_Au.CommodityName((*pci).CommodityId())));
				m_Modifiers->setItem(0, col++, lwi);
			}
			else if((*pci).IsRequired())
			{
				// Missing required component is holding up the process.
				QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpNotReady.png"), "");
				lwi->setToolTip(tr("%1 Needed").arg(g_Au.CommodityName((*pci).CommodityId())));
				m_Modifiers->setItem(0, col++, lwi);
			}
		}
	}
	m_Modifiers->resizeColumnsToContents();

	// Worker enabled checkbox.
	m_EnableWorker->blockSignals(true);
	m_EnableWorker->setCheckState(m_CitizenOk ? Qt::Checked : Qt::Unchecked);
	m_EnableWorker->blockSignals(false);

	// Last output quality x count.
	unsigned char lastQual = mp->LastOutputQuality();
	qint32 lastCount = mp->LastOutputQuantity();
	bool waitingToSpawn = false;
	if(lastCount < 0)
	{
		// Negative last output quantity is an error code returned by SpawnSc.
		// SpawnSc return code.
		switch(lastCount)
		{
		// Spawn on ground errors.
		case -11:
			m_LastOutput->setText(tr("No Parking Spot at Building"));
			waitingToSpawn = true;
			break;
		case -12:
			m_LastOutput->setText(tr("No Unobstructed Parking Spot"));
			waitingToSpawn = true;
			break;
		case -13:
			m_LastOutput->setText(tr("Parking Spot Not On World"));
			waitingToSpawn = true;
			break;

		// Spawn in orbit errors.
		case -21:
			m_LastOutput->setText(tr("Blocked by Unanalyzed Orbiting Spacecraft"));
			waitingToSpawn = true;
			break;
		case -22:
			m_LastOutput->setText(tr("Orbital Station Limit Reached"));
			waitingToSpawn = true;
			break;
		case -23:
			m_LastOutput->setText(tr("Orbital New Spacecraft Limit Reached"));
			waitingToSpawn = true;
			break;
		case -24:
			m_LastOutput->setText(tr("Orbital Parking Location Not Found"));
			waitingToSpawn = true;
			break;
		case -25:
			m_LastOutput->setText(tr("Orbital Station Needed"));
			waitingToSpawn = true;
			break;

		// Patent development errors.
		case -26:
			m_LastOutput->setText(tr("Invention Prototype Failed"));
			break;
		case -27:
			m_LastOutput->setText(tr("Patent Already Recorded at Q%1").arg(lastQual));
			break;

		// Empire ownership errors.
		case -40:
			m_LastOutput->setText(tr("Empire 0 Cannot Own Spacecraft"));
			waitingToSpawn = true;
			break;
		case -41:
			m_LastOutput->setText(tr("Empire 0 Cannot Build Spacecraft"));
			waitingToSpawn = true;
			break;

		case -99:
			m_LastOutput->setText(tr("Unimplemented Game Object"));
			break;
		default:
			m_LastOutput->setText(tr("No Output %1").arg(-lastCount));
			break;
		}
	}
	else if(lastQual || lastCount)
		m_LastOutput->setText(tr("Produced %2 at Q%1").arg(lastQual).arg(lastCount));
	else
		m_LastOutput->setText("");

	// Status
	// Production Q
	m_CurMinQuality = scf->MakeMinQ();
	quint8 tlRangeMin = 1;
	quint8 tlRangeMax = 255;
	if(tlRangeMax < tlRangeMin)
		tlRangeMax = tlRangeMin;
	if(m_QRangeMin != tlRangeMin || m_QRangeMax != tlRangeMax || m_MinQ->value() != m_CurMinQuality)
	{
		m_QRangeMin = tlRangeMin;
		m_QRangeMax = tlRangeMax;
		m_MinQ->blockSignals(true);
		m_MinQ->setRange(m_QRangeMin, m_QRangeMax);
		m_MinQ->setValue(m_CurMinQuality);
		m_MinQ->blockSignals(false);
	}

	QString status;
	if(waitingToSpawn)
	{
		status = tr("Waiting to Appear");
		m_StatusIcon->setPixmap(QPixmap(":data/c_SpacecraftWait.png"));
		//m_Cancel->setEnabled(false);
		m_Fetch->setEnabled(false);
		m_Run->setEnabled(false);
	}
	else if(mp->IsRunning())
	{
		unsigned int secs = mp->SecondsRemaining();
		unsigned int dy = secs / (60*60*24);
		if(dy)
			secs -= dy * (60*60*24);
		unsigned int hr = secs / (60*60);
		if(hr)
			secs -= hr * (60*60);
		unsigned int min = secs / 60;
		if(min)
			secs -= min * 60;
		if(dy)
			status = QString("Running %1 days %2 hr %3 min.").arg(dy).arg(hr).arg(min);
		else if(hr)
			status = QString("Running %1:%2:%3").arg(hr).arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		else
			status = QString("Running %1:%2").arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		m_StatusIcon->setPixmap(QPixmap(":data/mpRunning.png"));
		m_Cancel->setEnabled(m_InCoC);
		m_Fetch->setEnabled(false);
		m_Run->setEnabled(false);
	}
	else if(mp->CanRun())
	{
		unsigned int secs = mp->SecondsAccumulated();
		unsigned int dy = secs / (60*60*24);
		if(dy)
			secs -= dy * (60*60*24);
		unsigned int hr = secs / (60*60);
		if(hr)
			secs -= hr * (60*60);
		unsigned int min = secs / 60;
		if(min)
			secs -= min * 60;
		if(dy)
			status = QString("Ready %1 days %2 hr %3 min.").arg(dy).arg(hr).arg(min);
		else if(hr)
			status = QString("Ready %1:%2:%3").arg(hr).arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		else
			status = QString("Ready %1:%2").arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		m_StatusIcon->setPixmap(QPixmap(":/data/mpReady.png"));
		m_Cancel->setEnabled(m_InCoC);
		m_Fetch->setEnabled(false);
		m_Run->setEnabled(m_InCoC || m_EnableWorker->isChecked());
	}
	else
	{
		unsigned int secs = mp->SecondsAccumulated();
		unsigned int dy = secs / (60*60*24);
		if(dy)
			secs -= dy * (60*60*24);
		unsigned int hr = secs / (60*60);
		if(hr)
			secs -= hr * (60*60);
		unsigned int min = secs / 60;
		if(min)
			secs -= min * 60;
		if(dy)
			status = QString("Not Ready %1 days %2 hr %3 min.").arg(dy).arg(hr).arg(min);
		else if(hr)
			status = QString("Not Ready %1:%2:%3").arg(hr).arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		else
			status = QString("Not Ready %1:%2").arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		m_StatusIcon->setPixmap(QPixmap(":/data/mpNotReady.png"));
		m_Cancel->setEnabled(m_InCoC);
		m_Fetch->setEnabled(m_InCoC || m_EnableWorker->isChecked());
		m_Run->setEnabled(m_InCoC || m_EnableWorker->isChecked());	// Enable because player may have lacking component.
	}
	m_CompanyOk->setEnabled(m_InCoC);
	m_MakeWhat->setEnabled(m_InCoC);
	m_EnableWorker->setEnabled(m_InCoC);
	m_MinQ->setEnabled(m_InCoC);
	m_Status->setText(status);
}

void AuBldSpacecraftFactory::CitizenEnableChanged(int state)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	bool citizenOk = (state == Qt::Checked) ? true : false;
	if(citizenOk != m_CitizenOk)
		auApp->BdManufactureSpacecraftCitizenEnabled(citizenOk, m_CapitolOpId);
}

void AuBldSpacecraftFactory::FetchPushed()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	auApp->BdManufactureFetch(0, m_CapitolOpId);
}

void AuBldSpacecraftFactory::GoPushed()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	auApp->BdManufactureGo(0, m_CapitolOpId);
}

void AuBldSpacecraftFactory::MinQChanged(int value)
{
	quint8 minQ = (quint8)value;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(minQ != m_CurMinQuality)
	{
		m_CurMinQuality = minQ;
		auApp->BdSpacecraftProcessSetMinQ(minQ, m_CapitolOpId);
	}
}

void AuBldSpacecraftFactory::BuyOne()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(!m_PersonEmpire)
		return;

	// Company manufacture can be limited to a maximum quality.
	quint8 maxQ = m_CompanyMaxQ->value();
	AuBuySpacecraft dlg(maxQ, this);
	if(dlg.exec() == QDialog::Accepted && dlg.GearPos().IsValid())
	{
		// Queue a company spacecraft manufacturing process.
		auApp->BdSpacecraftProcessBuyCompanyShip(dlg.DesignId(), dlg.GearPos(), m_CapitolOpId);
	}
}

void AuBldSpacecraftFactory::Commission()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuCommissionSpacecraft dlg(this);
	if(dlg.exec() == QDialog::Accepted && dlg.GearPos().IsValid())
	{
		// Queue a fleet spacecraft manufacturing process.
		auApp->BdSpacecraftProcessCommissionShip(dlg.DesignId(), dlg.GearPos(), m_CapitolOpId);
	}
}

void AuBldSpacecraftFactory::CancelPushed()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	QString smsg = tr("<html>"
			"<p>"
			"The current running process will be terminated. "
			"</p>"
			"<p>"
			"Parts consumed by manufacturing processes are returned to the city stockpile, if space permits. "
			"Excess parts are lost. "
			"</p>"
			"<p>"
			"Funds spent to purchase a company ship are returned to the buyer. "
			"They will receive a receipt by mail. "
			"</p>"
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, auApp->Frame());
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	auApp->BdSpacecraftProcessCanceled(m_CapitolOpId);
}

void AuBldSpacecraftFactory::CompanyEnableChanged(int state)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	bool companyOk = (state == Qt::Checked) ? true : false;
	quint8 maxQ = m_CompanyMaxQ->value();
	if(companyOk != m_CompanyOkCur)
	{
		m_CompanyOkCur = companyOk;
		auApp->BdSpacecraftProcessSetCompanyPolicy(companyOk, maxQ, m_CapitolOpId);
	}
}

void AuBldSpacecraftFactory::CompanyMaxQChanged(int value)
{
	quint8 maxQ = (quint8)value;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	bool companyOk = m_CompanyOk->isChecked();
	if(maxQ != m_CompanyMaxQCur)
	{
		m_CompanyMaxQCur = maxQ;
		auApp->BdSpacecraftProcessSetCompanyPolicy(companyOk, maxQ, m_CapitolOpId);
	}
}

void AuBldSpacecraftFactory::ChooseDefaultProcess()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuDefaultSpacecraft dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{
		quint32 designId = dlg.DesignId();
		QString designName = dlg.DesignName();
		quint8 designQ = dlg.DesignQ();
		auApp->BdSpacecraftProcessDefaultSelected(designId, designName, designQ, m_CapitolOpId);
	}
}

void AuBldSpacecraftFactory::Delete()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	QListWidgetItem* current = m_CompanyQueue->currentItem();
	if(!current)
		return;
	if(!current->data(Qt::UserRole).toBool())
		return;

	quint32 designId = current->data(Qt::UserRole+1).toUInt();

	QString smsg = tr("<html>Confirm delete of order for fleet spacecraft design %1.</html>").arg(designId);
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, auApp->Frame());
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	auApp->BdSpacecraftOrderDelete(designId, m_CapitolOpId);
}

void AuBldSpacecraftFactory::QueueSelChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	Q_UNUSED(previous);
	bool enableDelete = false;
	if(current && m_InCoC)
		enableDelete = current->data(Qt::UserRole).toBool();
	m_Delete->setEnabled(enableDelete);
}

void AuBldSpacecraftFactory::GetPatent()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a UI to get a list of the patents they wish to record.
	AuPatentsGet dlg(false, m_CapitolOpId, this);
	if(pFrame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	QSet<COMMODITYID> patents;
	dlg.Patents(patents);
	if(patents.empty())
		return;

	AuGearPos gearPos = dlg.MediaPos();
	if(!gearPos.IsValid())
		return;

	quint8 maxQ = dlg.QualityLimit();

	auApp->BdPatentsRequested(patents, maxQ, gearPos, m_CapitolOpId);
}

void AuBldSpacecraftFactory::RecordPatent()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a UI to get a list of the patents they wish to record.
	AuPatentsRecord dlg(this);
	if(pFrame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	// Record the patents, if any.
	AuKnownPatents patents;
	dlg.Patents(patents);
	auApp->BdPatentsRecord(patents, m_CapitolOpId);
}
