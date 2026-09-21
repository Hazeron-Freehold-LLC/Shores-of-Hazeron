//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldcapitol.cpp,v 1.12 2022/02/19 18:09:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldcapitol.h"
//
#include "auapp.h"
#include "aubldbank.h"
#include "aubldbroker.h"
#include "aubldbuilding.h"
#include "aubldconstruct.h"
#include "aubldinfo.h"
#include "aubldmanufacture.h"
#include "aubldshed.h"
#include "aubldspacecraftfactory.h"
#include "aubldstoragetank.h"
#include "aubldwarehouse.h"
#include "auframe.h"
#include "aurenamecity.h"
#include "autablewidgetitem.h"
#include "auview.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStackedLayout>
#include <QTabBar>
#include <QTableWidget>
#include <QToolButton>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldCapitol::AuBldCapitol(QWidget* parent) : QWidget(parent)
{
	m_Mode = Init;

	m_CityName = new QPushButton(this);
	m_CityName->setToolTip(tr("Changes the name of the city."));
	connect(m_CityName, SIGNAL(clicked(bool)), SLOT(RenameCity()));

	m_WorldName = new QLabel(this);

	m_Info = new QLabel(this);

	m_Buildings = new QTableWidget(this);
	m_Buildings->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Buildings->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Buildings->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Buildings->setShowGrid(false);
	QStringList hls;
	hls << tr("Building") << tr("Location") << tr("Distance") << tr("Info");
	m_Buildings->setColumnCount(hls.count());
	m_Buildings->setHorizontalHeaderLabels(hls);
	m_Buildings->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_Buildings->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_Buildings->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	m_Buildings->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	m_Buildings->setColumnWidth(0, 200);
	m_Buildings->setColumnWidth(1, 120);
	m_Buildings->setColumnWidth(2, 80);
	m_Buildings->sortByColumn(0, Qt::AscendingOrder);
	QHeaderView* vhdr = m_Buildings->verticalHeader();
	vhdr->hide();
	connect(m_Buildings, SIGNAL(itemSelectionChanged()), SLOT(BuildingChanged()));
	m_Buildings->setToolTip(tr("<html>"
				"Select a building to manage. "
				"Buildings in the jurisdiction of the city are shown. "
				"Add <i>police stations</i> to expand the city's jurisdiction. "
				"</html>"));

	m_Pages = new QStackedLayout;

	// Create a tab bar for each MODE we will need.
	// Every mode has more than one page therefore every mode needs a tab bar.

	// Bank MODE
	banktabs = new QTabBar(this);
	banktabs->hide();
	connect(banktabs, SIGNAL(currentChanged(int)), SLOT(BankTabChanged(int)));

	// Broker MODE
	brokertabs = new QTabBar(this);
	brokertabs->hide();
	connect(brokertabs, SIGNAL(currentChanged(int)), SLOT(BrokerTabChanged(int)));

	// Building
	bldtabs = new QTabBar(this);
	bldtabs->hide();
	connect(bldtabs, SIGNAL(currentChanged(int)), SLOT(BuildingTabChanged(int)));

	// Construction
	contabs = new QTabBar(this);
	contabs->hide();
	connect(contabs, SIGNAL(currentChanged(int)), SLOT(ConstructionTabChanged(int)));

	// Manufacture
	mfgtabs = new QTabBar(this);
	mfgtabs->hide();
	connect(mfgtabs, SIGNAL(currentChanged(int)), SLOT(ManufacturingTabChanged(int)));

	// Shed MODE
	shedtabs = new QTabBar(this);
	shedtabs->hide();
	connect(shedtabs, SIGNAL(currentChanged(int)), SLOT(ShedTabChanged(int)));

	// SpacecraftFactory MODE
	scfacttabs = new QTabBar(this);
	scfacttabs->hide();
	connect(scfacttabs, SIGNAL(currentChanged(int)), SLOT(SpacecraftFactoryTabChanged(int)));

	// StorageTank MODE
	stanktabs = new QTabBar(this);
	stanktabs->hide();
	connect(stanktabs, SIGNAL(currentChanged(int)), SLOT(StorageTankTabChanged(int)));

	// Warehouse MODE
	waretabs = new QTabBar(this);
	waretabs->hide();
	connect(waretabs, SIGNAL(currentChanged(int)), SLOT(WarehouseTabChanged(int)));

	QGridLayout* tabsPagesLay = new QGridLayout;
	tabsPagesLay->addWidget(banktabs, 0,0);
	tabsPagesLay->addWidget(bldtabs, 0,0);
	tabsPagesLay->addWidget(brokertabs, 0,0);
	tabsPagesLay->addWidget(contabs, 0,0);
	tabsPagesLay->addWidget(mfgtabs, 0,0);
	tabsPagesLay->addWidget(scfacttabs, 0,0);
	tabsPagesLay->addWidget(shedtabs, 0,0);
	tabsPagesLay->addWidget(stanktabs, 0,0);
	tabsPagesLay->addWidget(waretabs, 0,0);
	tabsPagesLay->addLayout(m_Pages, 1,0);
	tabsPagesLay->setMargin(2);
	tabsPagesLay->setSpacing(0);
	setLayout(tabsPagesLay);

	QWidget* pagePane = new QWidget;
	pagePane->setLayout(tabsPagesLay);

	// Construction page is always alone because Bd buildings cannot do anything else while under construction.
	contabs->addTab(QIcon(":data/ConstructionSign.png"), tr("Construction"));
	m_BldConstruct = new AuBldConstruct(this);
	m_ConstructionPage = m_Pages->addWidget(m_BldConstruct);

	// Bank tab
	banktabs->addTab(QIcon(":data/bBank.png"), tr("Bank"));
	m_BldBank = new AuBldBank(this);
	m_BankPage = m_Pages->addWidget(m_BldBank);

	// Broker tab
	brokertabs->addTab(QIcon(":data/bBroker.png"), tr("Broker"));
	m_BldBroker = new AuBldBroker(this);
	m_BrokerPage = m_Pages->addWidget(m_BldBroker);

	// Shed tab
	shedtabs->addTab(QIcon(":data/bShed.png"), tr("Shed"));
	m_BldShed = new AuBldShed(this);
	m_ShedPage = m_Pages->addWidget(m_BldShed);

	// SpacecraftFactory tab
	scfacttabs->addTab(QIcon(":data/bSpacecraftFactory.png"), tr("Spacecraft Factory"));
	m_BldScFact = new AuBldSpacecraftFactory(this);
	m_ScFactPage = m_Pages->addWidget(m_BldScFact);

	// StorageTank tab
	stanktabs->addTab(QIcon(":data/bStorageTank.png"), tr("Storage Tank"));
	m_BldStorageTank = new AuBldStorageTank(this);
	m_StorageTankPage = m_Pages->addWidget(m_BldStorageTank);

	// Warehouse tab
	waretabs->addTab(QIcon(":data/bWarehouse.png"), tr("Warehouse"));
	m_BldWarehouse = new AuBldWarehouse(this);
	m_WarehousePage = m_Pages->addWidget(m_BldWarehouse);

	// Manufacture tab
	// Not needed by buildings that do not manufacture anything.
	banktabs->addTab(QIcon(":data/HammerAndSickle.png"), tr("Manufacture"));
	mfgtabs->addTab(QIcon(":data/HammerAndSickle.png"), tr("Manufacture"));
	m_BldManufacture = new AuBldManufacture(this);
	m_ManufacturePage = m_Pages->addWidget(m_BldManufacture);

	// Building page provides information about the building itself. It is useful at all buildings.
	banktabs->addTab(QIcon(":data/City.png"), tr("Building"));
	bldtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	brokertabs->addTab(QIcon(":data/City.png"), tr("Building"));
	contabs->addTab(QIcon(":data/City.png"), tr("Building"));
	mfgtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	scfacttabs->addTab(QIcon(":data/City.png"), tr("Building"));
	shedtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	stanktabs->addTab(QIcon(":data/City.png"), tr("Building"));
	waretabs->addTab(QIcon(":data/City.png"), tr("Building"));
	m_BldBuilding = new AuBldBuilding(this);
	m_BuildingPage = m_Pages->addWidget(m_BldBuilding);

	// Info page provides information about the building window. It is useful at all buildings.
	banktabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	bldtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	brokertabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	contabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	mfgtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	scfacttabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	shedtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	stanktabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	waretabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	m_BldInfo = new AuBldInfo(true, this);
	m_InfoPage = m_Pages->addWidget(m_BldInfo);

	// Assemble the widgets into a splitter.
	QSplitter* splitter = new QSplitter;
	splitter->setOrientation(Qt::Vertical);
	splitter->addWidget(m_Buildings);
	splitter->addWidget(pagePane);
	splitter->setCollapsible(0, false);						// Prevent pane from collapsing to 0. People lose it.
	splitter->setCollapsible(1, false);						// Prevent pane from collapsing to 0. People lose it.

	m_DemolishButton = new QToolButton(this);
	m_DemolishButton->setIcon(QIcon(":data/bp_ClearLand.png"));
	m_DemolishButton->setToolTip(tr("<html><b>Demolish...</b><br>"
				"Destroys the selected building. "
				"</html>"));
	m_DemolishButton->setAutoRaise(true);
	m_DemolishButton->setEnabled(false);
	connect(m_DemolishButton, SIGNAL(clicked(bool)), SLOT(Demolish()));

	m_Report = new QToolButton(this);
	m_Report->setIcon(QIcon(":data/MailCityStatus.png"));
	m_Report->setToolTip(tr("<html><b>Recent Report</b><br>"
				"Requests the most recent report from the city. "
				"</html>"));
	m_Report->setAutoRaise(true);
	connect(m_Report, SIGNAL(clicked(bool)), SLOT(ReportCityLast()));

	m_DeclareEmpireCapital = new QToolButton(this);
	m_DeclareEmpireCapital->setIcon(QIcon(":data/GovPolicy.png"));
	m_DeclareEmpireCapital->setToolTip(tr("<html><b>Declare Empire Capital...</b><br>"
				"Designates the current city to be the capital of the empire.<br>"
				"<i>Tips</i><br>"
				"%1 A city can be an empire capital, a galaxy capital and a sector capital.<br>"
				"%1 Only a governor with appropriate authority can declare the empire capital.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_DeclareEmpireCapital, SIGNAL(clicked(bool)), SLOT(DeclareEmpireCapital()));

	m_DeclareGalaxyCapital = new QToolButton(this);
	m_DeclareGalaxyCapital->setIcon(QIcon(":data/RangeGalaxy.png"));
	m_DeclareGalaxyCapital->setToolTip(tr("<html><b>Declare Galaxy Capital...</b><br>"
				"Designates the current city to be the capital of the galaxy.<br>"
				"<i>Tips</i><br>"
				"%1 A city can be an empire capital, a galaxy capital and a sector capital.<br>"
				"%1 Only a governor with appropriate authority can declare a galaxy capital.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_DeclareGalaxyCapital, SIGNAL(clicked(bool)), SLOT(DeclareGalaxyCapital()));

	m_DeclareSectorCapital = new QToolButton(this);
	m_DeclareSectorCapital->setIcon(QIcon(":data/Sector.png"));
	m_DeclareSectorCapital->setToolTip(tr("<html><b>Declare Sector Capital...</b><br>"
				"Designates the current city to be the capital of the sector.<br>"
				"<i>Tips</i><br>"
				"%1 A city can be an empire capital, a galaxy capital and a sector capital.<br>"
				"%1 Only a governor with appropriate authority can declare a sector capital.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_DeclareSectorCapital, SIGNAL(clicked(bool)), SLOT(DeclareSectorCapital()));

	QToolButton* refresh = new QToolButton(this);
	refresh->setIcon(QIcon(":data/Sweep.png"));
	refresh->setToolTip(tr("<html><b>Refresh</b><br>"
				"Refreshes the list of buildings. "
				"</html>"));
	refresh->setAutoRaise(true);
	connect(refresh, SIGNAL(clicked(bool)), SLOT(ListBuildings()));

	QHBoxLayout* tblay = new QHBoxLayout;
	tblay->addWidget(m_CityName);
	tblay->addWidget(m_WorldName);
	tblay->addStretch(10);
	tblay->addWidget(m_DemolishButton);
	tblay->addStretch(1);
	tblay->addWidget(refresh);
	tblay->addWidget(m_DeclareEmpireCapital);
	tblay->addWidget(m_DeclareGalaxyCapital);
	tblay->addWidget(m_DeclareSectorCapital);
	tblay->addWidget(m_Report);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(tblay);
	lay->addWidget(m_Info);
	lay->addWidget(splitter, 10);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldCapitol::~AuBldCapitol()
{
}

void AuBldCapitol::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	ListBuildings();
	BuildingChanged();
}

void AuBldCapitol::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widget now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();

	AuView* pView = auApp->Frame()->View();
	pView->SetSelectedBd(AuId(0));
}

void AuBldCapitol::ListBuildings()
{
	m_CityName->setText("");
	m_WorldName->setText("");
	bool hasInfo = false;
	bool hasSelected = false;

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() == IID_Capitol && !person->BdId().IsNull())
	{
		const DaWorldLL* planet = dynamic_cast<const DaWorldLL*>(person->FindParentPlanetInEnv());
		const DaBd* bd = planet ? dynamic_cast<const DaBd*>(planet->FindIdRecursive(person->BdId())) : 0;
		if(bd)
		{
			m_CityName->setText(bd->CityName());
			m_WorldName->setText(planet->Name());
			if(bd->InChainOfCommand(person))
				m_CapitolId = bd->CapitolId();
			else
				m_CapitolId = AuId(0);

			AuPoint personLoc;
			person->GetLocationRelativeToParent(personLoc, planet);

			int selcolumn = 0;
			QTableWidgetItem* selitem = m_Buildings->currentItem();
			if(selitem)
			{
				selcolumn = selitem->column();
				selitem = 0;
			}
			m_Buildings->blockSignals(true);
			m_Buildings->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			int trow=0;

			DaAtomList bdCity;
			planet->FindBdCity(bdCity, bd->Id());
			m_Buildings->setRowCount((int)bdCity.size());
			for(DaAtomList::const_iterator ci = bdCity.cbegin(); ci != bdCity.cend(); ci++)
			{
				const DaBd* bdc = (const DaBd*)*ci;
				QString name(bdc->AssignedName());
				QString indName(bdc->IndustryName());
				if(bdc->Is(m_SelectedId))
					hasSelected = true;

				// Industry
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(QIcon(DbIndustry::BitmapName(bdc->IndustryId())), indName);
				if(selcolumn == col && bdc->Is(m_SelectedId))
					selitem = twi;
				twi->setData(Qt::UserRole, QVariant::fromValue(bdc->Id()));
				m_Buildings->setItem(trow, col++, twi);

				// Location
				twi = new QTableWidgetItem(bdc->GlobeLL().ToString());
				if(selcolumn == col && bdc->Is(m_SelectedId))
					selitem = twi;
				twi->setData(Qt::UserRole, QVariant::fromValue(bdc->Id()));
				m_Buildings->setItem(trow, col++, twi);

				// Distance
				AuPoint bldLoc(bdc->Location());
				twi = new AuTableWidgetDistanceItem(bldLoc.Distance(personLoc));
				if(selcolumn == col && bdc->Is(m_SelectedId))
					selitem = twi;
				twi->setData(Qt::UserRole, QVariant::fromValue(bdc->Id()));
				m_Buildings->setItem(trow, col++, twi);

				// Info
				QString info(name);
				if(bdc->IsUnderConstruction())
				{
					if(!info.isEmpty())
						info += ", ";
					info += bdc->IsConstructionWorkerEnabled() ? tr("Under Construction") : tr("Construction Halted");
				}
				twi = new QTableWidgetItem(info);
				if(selcolumn == col && bdc->Is(m_SelectedId))
					selitem = twi;
				twi->setData(Qt::UserRole, QVariant::fromValue(bdc->Id()));
				m_Buildings->setItem(trow, col++, twi);

				m_Buildings->setRowHeight(trow, 18);
				trow++;
			}

			m_Buildings->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill

			// Reselect the current building.
			m_Buildings->setRangeSelected(QTableWidgetSelectionRange(0, 0, m_Buildings->rowCount(), m_Buildings->columnCount()),
					false);
			if(selitem)
				m_Buildings->setCurrentItem(selitem);

			m_Buildings->blockSignals(false);

			m_Info->setText(tr("%L1 buildings").arg(bdCity.size()));
			hasInfo = true;
		}
	}
	pView->UnlockSceneAtom();
	m_Report->setEnabled(!m_CapitolId.IsNull());
	if(!hasInfo)
		m_Info->setText("");
	if(!hasSelected)
	{
		m_SelectedId = AuId(0);
		m_SelectedName = "";
	}
}

void AuBldCapitol::BuildingTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::ConstructionTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ConstructionPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::BankTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BankPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_ManufacturePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::BrokerTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BrokerPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::ManufacturingTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ManufacturePage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::ShedTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ShedPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::SpacecraftFactoryTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ScFactPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::StorageTankTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_StorageTankPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::WarehouseTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_WarehousePage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBldCapitol::Demolish()
{
	if(m_CapitolId.IsNull())
		return;
	if(m_SelectedId.IsNull() || m_SelectedName == "")
		return;

	QString smsg = tr("<html><p>Are you sure you want to demolish the %1 at %2?</p></html>")
		.arg(m_SelectedName)
		.arg(m_SelectedLL.ToString());
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result == QMessageBox::Yes)
		auApp->BuildingDemolish(m_SelectedId);
}

void AuBldCapitol::ReportCityLast()
{
	if(m_CapitolId.IsNull())
		return;
	auApp->ReportCityLast(m_CapitolId);
}

void AuBldCapitol::DeclareEmpireCapital()
{
	if(m_CapitolId.IsNull())
		return;
	AuFrame* frame = auApp->Frame();
	QString smsg = tr("<html>Confirm declaration of empire capital.</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, frame);
	int result = frame->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	auApp->GovSetCapital(1);
}

void AuBldCapitol::DeclareGalaxyCapital()
{
	if(m_CapitolId.IsNull())
		return;
	AuFrame* frame = auApp->Frame();
	QString smsg = tr("<html>Confirm declaration of galaxy capital.</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, frame);
	int result = frame->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	auApp->GovSetCapital(3);
}

void AuBldCapitol::DeclareSectorCapital()
{
	if(m_CapitolId.IsNull())
		return;
	AuFrame* frame = auApp->Frame();
	QString smsg = tr("<html>Confirm declaration of sector capital.</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, frame);
	int result = frame->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	auApp->GovSetCapital(0);
}

void AuBldCapitol::RenameCity()
{
	if(m_CapitolId.IsNull())
		return;
	AuRenameCity renameDlg(CITY_Capitol, this);
	renameDlg.exec();
}

void AuBldCapitol::BuildingChanged()
{
	//qDebug() << "AuBldCapitol::BuildingChanged";
	m_SelectedId = AuId(0);
	m_SelectedName = "";

	AuId bdId;
	QItemSelectionModel* ism = m_Buildings->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(sel.isEmpty())
		{
			m_BldBank->ClearCapitolOp();
			m_BldBroker->ClearCapitolOp();
			m_BldBuilding->ClearCapitolOp();
			m_BldConstruct->ClearCapitolOp();
			m_BldManufacture->ClearCapitolOp();
			m_BldScFact->ClearCapitolOp();
			m_BldShed->ClearCapitolOp();
			m_BldStorageTank->ClearCapitolOp();
			m_BldWarehouse->ClearCapitolOp();
		}
		else
		{
			for(QModelIndexList::const_iterator si = sel.constBegin(); si != sel.constEnd(); si++)
				if((*si).isValid())
				{
					// Building id is stored on column 0.
					QTableWidgetItem* item = m_Buildings->item((*si).row(), 0);
					if(item)
						bdId = item->data(Qt::UserRole).value<AuId>();
					break;
				}
		}
	}

	AuView* pView = auApp->Frame()->View();
	MODE modeNeeded = Info;	// Set to the mode needed by the scene atom. Info is always available.
	if(!bdId.IsNull())
	{
		// What building is there?
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person && person->InIndustry() == IID_Capitol && !person->BdId().IsNull())
		{
			const DaWorldLL* planet = dynamic_cast<const DaWorldLL*>(person->FindParentPlanetInEnv());
			const DaBd* bd = planet ? dynamic_cast<const DaBd*>(planet->FindIdRecursive(bdId)) : 0;
			if(bd)
			{
				m_SelectedId = bd->Id();
				m_SelectedName = bd->IndustryName();
				m_SelectedLL = bd->GlobeLL();
				m_BldBank->SetCapitolOp(bdId);
				m_BldBroker->SetCapitolOp(bdId);
				m_BldBuilding->SetCapitolOp(bdId);
				m_BldConstruct->SetCapitolOp(bdId, bd->IndustryId(), bd->GlobeLL());	// The LL is really just for roads.
				m_BldManufacture->SetCapitolOp(bdId);
				m_BldScFact->SetCapitolOp(bdId);
				m_BldShed->SetCapitolOp(bdId);
				m_BldStorageTank->SetCapitolOp(bdId);
				m_BldWarehouse->SetCapitolOp(bdId);
				switch(bd->IndustryId())
				{
				case IID_Bank:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else
						modeNeeded = Bank;
					break;
				case IID_Broker:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else
						modeNeeded = Broker;
					break;
				case IID_Shed:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else
						modeNeeded = Shed;
					break;
				case IID_SpacecraftFactory:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else
						modeNeeded = SpacecraftFactory;
					break;
				case IID_StorageTank:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else
						modeNeeded = StorageTank;
					break;
				case IID_Warehouse:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else
						modeNeeded = Warehouse;
					break;
				default:
					if(bd->IsUnderConstruction())
						modeNeeded = Construction;
					else if(DbIndustry::DoesManufacture(bd->IndustryId()))
						modeNeeded = Manufacture;
					else
						modeNeeded = Building;
					break;
				}
			}
		}
		pView->UnlockSceneAtom();
	}

	pView->SetSelectedBd(m_SelectedId);
	m_DemolishButton->setEnabled(!m_SelectedId.IsNull());

	// Do nothing if the mode hasn't changed.
	if(modeNeeded == m_Mode)
		return;

	int selPage = m_Pages->currentIndex();
	int selTabIndex = 0;
	QTabBar* showTabs = 0;
	switch(modeNeeded)
	{
	case Init:
	case Info:
		selPage = m_InfoPage;
		break;
	case Construction:
		selPage = m_ConstructionPage;
		showTabs = contabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ConstructionPage;
			selTabIndex = 0;
		}
		break;
	case Building:
		showTabs = bldtabs;
		//if(selPage == m_InfoPage)
			//selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else
		{
			selPage = m_BuildingPage;
			selTabIndex = 0;
		}
		break;
	case Bank:
		showTabs = banktabs;
		if(selPage == m_ManufacturePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 3;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_BankPage;
			selTabIndex = 0;
		}
		break;
	case Broker:
		showTabs = brokertabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_BrokerPage;
			selTabIndex = 0;
		}
		break;
	case Manufacture:
		showTabs = mfgtabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ManufacturePage;
			selTabIndex = 0;
		}
		break;
	case SpacecraftFactory:
		showTabs = scfacttabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ScFactPage;
			selTabIndex = 0;
		}
		break;
	case Shed:
		showTabs = shedtabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ShedPage;
			selTabIndex = 0;
		}
		break;
	case StorageTank:
		showTabs = stanktabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_StorageTankPage;
			selTabIndex = 0;
		}
		break;
	case Warehouse:
		showTabs = waretabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		//else if(selPage == m_InfoPage)
			//selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_WarehousePage;
			selTabIndex = 0;
		}
		break;
	}

	if(banktabs != showTabs)
		banktabs->hide();
	if(bldtabs != showTabs)
		bldtabs->hide();
	if(brokertabs != showTabs)
		brokertabs->hide();
	if(contabs != showTabs)
		contabs->hide();
	if(mfgtabs != showTabs)
		mfgtabs->hide();
	if(scfacttabs != showTabs)
		scfacttabs->hide();
	if(shedtabs != showTabs)
		shedtabs->hide();
	if(stanktabs != showTabs)
		stanktabs->hide();
	if(waretabs != showTabs)
		waretabs->hide();
	if(showTabs)
	{
		showTabs->show();
		showTabs->setCurrentIndex(selTabIndex);
	}
	m_Pages->setCurrentIndex(selPage);
	m_Mode = modeNeeded;
}

void AuBldCapitol::Shipment(AuShipment shipment)
{
	m_BldBroker->Shipment(shipment);
}

void AuBldCapitol::ShipmentsClear()
{
	m_BldBroker->ShipmentsClear();
}

void AuBldCapitol::TradeAgreement(AuTradeAgreement ta)
{
	m_BldBroker->TradeAgreement(ta);
}

void AuBldCapitol::TradeAgreementsClear()
{
	m_BldBroker->TradeAgreementsClear();
}

