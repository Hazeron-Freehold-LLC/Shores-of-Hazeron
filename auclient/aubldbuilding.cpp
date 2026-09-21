//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldbuilding.cpp,v 1.24 2022/02/19 18:08:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldbuilding.h"
//
#include "auapp.h"
#include "aucommodityfilter.h"
#include "auframe.h"
#include "auglobal.h"
#include "aurenamebuilding.h"
#include "auscparts.h"
#include "autoolbutton.h"
#include "auview.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextBrowser>
#include <QTimer>
#include <QGridLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldBuilding::AuBldBuilding(QWidget* parent) : QWidget(parent)
{
	m_BuyItem = 0;
	m_Filter = FILTER_All;
	m_Timer = 0;

	m_Icon = new QLabel(this);

	m_Info = new QTextBrowser(this);
	m_Info->setOpenExternalLinks(true);
	m_Info->setReadOnly(true);

	AuIconButton* declareHome = new AuIconButton(QIcon(QPixmap(":data/SpawnTie.png").scaled(32,32)), this);
	declareHome->setIconSize(QSize(32,32));
	declareHome->setToolTip(tr("<html><p><b>Declare Home</b><br>"
				"Declares this building to be your home. "
				"A building must be friendly toward your empire to declare it your home."
				"</p>"
				"<p>"
				"Your home solar system is automatically part of your land. "
				"Buildings on your land are immune to abandonment decay for 30 days after this avatar was last on line. "
				"</p>"
				"<p>"
				"You appear near your current location when you die or recall home. "
				"The building must be friendly to your empire at the time. "
				"</p>"
				"<p>"
				"When recalling home, your berth aboard a spacecraft takes precedence over a building."
				"</p>"
				"</html>"));
	connect(declareHome, SIGNAL(clicked()), auApp->Frame(), SLOT(SpawnTie()));

	m_Rename = new AuIconButton(QIcon(":data/RenameBuilding.png"), this);
	m_Rename->setIconSize(QSize(32,32));
	m_Rename->setToolTip(tr("<html><b>Change Name</b><br>"
				"Changes the name of the building."
				"<br>"
				"<b>You must be in the chain of command of the building to change its name.</b>"
				"<html>"));
	connect(m_Rename, SIGNAL(clicked()), SLOT(Rename()));

	m_CommodityFilter = new AuCommodityFilter;
	connect(m_CommodityFilter, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(Filter(COMMODITYFILTER)));

	m_Cargo = new QListWidget(this);
	m_Cargo->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Cargo->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Cargo->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Cargo->setSortingEnabled(true);
	m_Cargo->setToolTip(tr("<html>"
				"Choose an item to take. "
				"<br>"
				"<b>You can take items because you are in the chain of command of this building.</b> "
				"</html>"));
	connect(m_Cargo, SIGNAL(itemSelectionChanged()), SLOT(CargoSelected()));

	m_CargoDesc = new QTextBrowser(this);
	m_CargoDesc->setReadOnly(true);

	// Quantity to purchase.
	m_CountLabel = new QLabel(tr("Amount"), this);
	m_CountLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_BuyCountSpin = new QSpinBox(this);
	m_BuyCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BuyCountSpin->selectAll();
	m_BuyCountSpin->setMinimum(1);
	m_BuyCountSpin->setMaximum(2000000000);
	m_BuyCountSpin->setValue(1);
	m_BuyCountSpin->setToolTip(tr("<html><b>Amount</b><br>"
				"Enter the amount of the item that you want to take or trash."
				"<br>"
				"<b>You can take items and trash items because you are in the chain of command of this building.</b> "
				"</html>"));
	connect(m_BuyCountSpin, SIGNAL(valueChanged(int)), SLOT(QuantityToBuyChanged(int)));

	m_OkButton = new QPushButton(QPixmap(":data/MsgCargoTake.png"), tr("Take Items"), this);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(OkPressed()));
	m_OkButton->setToolTip(tr("<html>Takes the selected items."
				"<br>"
				"<b>You can take items because you are in the chain of command of this building.</b> "
				"</html>"));

	m_TrashButton = new QPushButton(QPixmap(":data/CommJettison.png"), tr("Trash Items"), this);
	connect(m_TrashButton, SIGNAL(clicked()), SLOT(TrashPressed()));
	m_TrashButton->setToolTip(tr("<html>Discards the selected items."
				"<br>"
				"<b>You can trash items because you are in the chain of command of this building.</b> "
				"</html>"));

	// Build the layout.
	QHBoxLayout* cargolay = new QHBoxLayout;
	cargolay->addWidget(m_Cargo);
	cargolay->addWidget(m_CargoDesc);

	QHBoxLayout* tlay = new QHBoxLayout;
	tlay->addWidget(m_CountLabel);
	tlay->addWidget(m_BuyCountSpin);
	tlay->addWidget(m_OkButton);
	tlay->addWidget(m_TrashButton);

	QVBoxLayout* blay = new QVBoxLayout;
	blay->addWidget(m_Info);
	blay->addLayout(cargolay);
	blay->addWidget(m_CommodityFilter);
	blay->addLayout(tlay);

	// Build the layout.
	QGridLayout* lay = new QGridLayout();
	int row=0;
	lay->addWidget(m_Icon, row,0, 1,2);
	lay->addLayout(blay, row,2, 5,1);
	row++;
	lay->setRowStretch(row, 4);
	row++;
	lay->addWidget(declareHome, row,1, 1,1);
	row++;
	lay->setRowStretch(row, 1);
	row++;
	lay->addWidget(m_Rename, row,1, 1,1);
	lay->setColumnMinimumWidth(0, 20);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldBuilding::~AuBldBuilding()
{
	if(m_BuyItem)
		delete m_BuyItem;
}

void AuBldBuilding::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(3000);

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	UpdateContents();	// Don't wait for 3 seconds.
}

void AuBldBuilding::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();

	// Hide my widgets now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
	m_Info->setText("");
	m_InfoText = "";
}

void AuBldBuilding::Rename()
{
	AuRenameBuilding renameDlg(m_AssignedName, m_CapitolOpId, this);
	renameDlg.exec();
}

void AuBldBuilding::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}
	// Find out what my current livestock is.
	bool isUnderConstruction = false;
	INDUSTRYID iid = IID_None;
	QString name, indName, cityName;
	quint32 empireId = 0;
	quint32 homesLarge = 0;
	quint32 homesMedium = 0;
	quint32 homesSmall = 0;
	quint32 manufacturingJobs = 0;
	quint32 serviceJobs = 0;
	quint32 stores = 0;
	quint32 lounges = 0;
	quint32 proJobs = 0;
	quint16 residents = 0;
	quint16 residentsEmployed = 0;
	quint16 workers = 0;
	quint32 blueprintId = 0;
	AuScFence::STYLE fenceStyle = AuScFence::STYLE_Count;
	QString blueprintName;
	AuId archId;
	QString archName;
	quint32 storage = 0;
	quint8 qmade = 0;
	bool hasPower = false;
	AuId galaxyId;
	AuId sectorId;
	AuId opId;
	quint32 tradeByAir = 0;
	quint32 tradeByLand = 0;
	quint32 tradeBySea = 0;
	bool hasAirportAccess = false;
	bool hasWharfAccess = false;
	bool isMilitary = false;
	AuKnownPatents patents;
	QHash<COMMODITYID, quint32> rocks;

	QString qual;
	QMap<QString, QMap<CommoditySubKey, QString> > inven;		// Key is commodity name.
	QMap<QString, QString> bitmap;		// Key is commodity name.
	//QString reportHtml;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxyId = sector->FindParentGalaxy()->Id();
		}
		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp)
		{
			opId = bdOp->Id();
			isUnderConstruction = bdOp->IsUnderConstruction();
			m_AssignedName = bdOp->AssignedName();
			cityName = bdOp->CityName();
			indName = bdOp->IndustryName();
			empireId = bdOp->EmpireId();
			iid = bdOp->IndustryId();
			//const AuBdReport& report = bdOp->LockDesignAnalysis();
			//reportHtml = report.ReportAnalysis(AuDesign::Final);
			//bdOp->UnlockDesignAnalysis();
			bdOp->IndustryInfo(homesLarge, homesMedium, homesSmall, manufacturingJobs, serviceJobs, stores, lounges, proJobs);
			patents = bdOp->LockPatentsForRead();
			bdOp->UnlockPatents();
			residents = bdOp->Residents();
			residentsEmployed = bdOp->ResidentsEmployed();
			workers = bdOp->Workers();
			blueprintId = bdOp->DesignId();
			blueprintName = bdOp->DesignName();
			archId = bdOp->DesignArchitectId();
			archName = bdOp->DesignArchitectName();
			storage = bdOp->CargoCapacityVolume();
			qmade = bdOp->QualityMade();
			hasPower = bdOp->HasPower();
			hasAirportAccess = bdOp->HasAirportAccess();
			hasWharfAccess = bdOp->HasWharfAccess();
			tradeByAir = bdOp->TradeConnectionsByAir();
			tradeByLand = bdOp->TradeConnectionsByLand();
			tradeBySea = bdOp->TradeConnectionsBySea();
			isMilitary = bdOp->IsMilitary();
			if(iid == IID_Fence)
				fenceStyle = (AuScFence::STYLE)bdOp->FenceStyle();
			rocks = bdOp->LockRocksOnSite();
			bdOp->UnlockRocksOnSite();

			// Enable access to stored commodities to people in my chain of command, if I am not under construction.
			m_InCoC = !isUnderConstruction && bdOp->InChainOfCommand(person);
			if(m_InCoC)
				ListCargo(bdOp, m_Cargo);
			else
				m_Cargo->clear();

			const AuItemHash& inv = bdOp->LockCargoForRead();
			for(AuItemHash::const_iterator it = inv.cbegin(); it != inv.cend(); it++)
			{
				QMap<QString, QMap<CommoditySubKey, QString> >* pi = &inven;

				QString commod = (*it).second->ListName();
				if(!bitmap.contains(commod))
					bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

				QMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = pi->find(commod);
				if(iit == pi->end())
					iit = pi->insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

				if(iit != pi->end())
				{
					CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
					if((*it).second->EmpireOfOrigin())
					{
						// The item has a known empire of origin.
						QString empireOfOrigin;
						if((*it).second->EmpireOfOrigin() != bdOp->EmpireId())
						{
							// Item was created in another empire.
							const DgData& gd = g_Au.LockDataForRead();
							gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
							g_Au.UnlockData();
						}
						if(!empireOfOrigin.isEmpty())
						{
							// Item was created in another empire.
							(*iit).insert(key, QObject::tr("%1 Q%2 Made in %3").arg((*it).second->Count())
									.arg((*it).second->Quality())
									.arg(empireOfOrigin));
						}
						else
						{
							// Item was created in my empire.
							(*iit).insert(key, QObject::tr("%1 Q%2").arg((*it).second->Count()).arg((*it).second->Quality()));
						}
					}
					else
					{
						// The item does not have a known empire of origin. Probably hand made or a carcass.
						(*iit).insert(key, QObject::tr("<i>%1 Q%2</i>").arg((*it).second->Count()).arg((*it).second->Quality()));
					}
				}
			}
			bdOp->UnlockCargo();
		}
	}
	pView->UnlockSceneAtom();
	if(iid == IID_None)
	{
		m_Icon->setText("");
		m_Info->setText("");
		m_InfoText = "";
		return;
	}

	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, empireName);
	AuId empireCap;
	gd.EmpireCapital(empireId, empireCap);
	AuId galaxyCap = gd.EmpireGalaxyCapital(empireId, galaxyId);
	AuId sectorCap = gd.EmpireSectorCapital(empireId, sectorId);
	g_Au.UnlockData();

	QString info;
	if(!m_AssignedName.isEmpty())
		info = QString("<html><big><b>%1</b> <i>%2</i></big>")
			.arg(indName)
			.arg(m_AssignedName);
	else
		info = QString("<html><big><b>%1</b></big>")
			.arg(indName);

	if(iid == IID_Fence)
	{
		if(!empireName.isEmpty() && !cityName.isEmpty())
			info += tr("<br><b>%1</b> city <b>%2</b> fence/wall")
				.arg(empireName)
				.arg(cityName);
		else if(!empireName.isEmpty())
			info += QString("<br><b>%1</b> unincorporated fence/wall")
				.arg(empireName);
	}
	else if(isMilitary)
	{
		if(!empireName.isEmpty() && !cityName.isEmpty())
			info += tr("<br><b>%1</b> base <b>%2</b>")
				.arg(empireName)
				.arg(cityName);
		else if(!empireName.isEmpty())
			info += QString("<br><b>%1 base remnant</b>")
				.arg(empireName);
	}
	else
	{
		if(!empireName.isEmpty() && !cityName.isEmpty())
			info += tr("<br><b>%1</b> city <b>%2</b>")
				.arg(empireName)
				.arg(cityName);
		else if(!empireName.isEmpty())
			info += QString("<br><b>%1</b> unincorporated")
				.arg(empireName);

		if(opId == empireCap && opId == galaxyCap && opId == sectorCap)
			info += tr("<br><b>Empire, Galaxy and Sector Capital</b>");
		else if(opId == empireCap && opId == galaxyCap)
			info += tr("<br><b>Empire and Galaxy Capital</b>");
		else if(opId == empireCap && opId == sectorCap)
			info += tr("<br><b>Empire and Sector Capital</b>");
		else if(opId == galaxyCap && opId == sectorCap)
			info += tr("<br><b>Galaxy and Sector Capital</b>");
		else if(opId == empireCap)
			info += tr("<br><b>Empire Capital</b>");
		else if(opId == galaxyCap)
			info += tr("<br><b>Galaxy Capital</b>");
		else if(opId == sectorCap)
			info += tr("<br><b>Sector Capital</b>");
	}

	if(blueprintId)
	{
		info += "<br>";
		info += tr("Blueprint: <b>%1 %2</b>"
				"<br>"
				"Architect: <b>%3 <small><i>%4</i></small></b>")
			.arg(blueprintId)
			.arg(blueprintName)
			.arg(archName)
			.arg(archId.StringId());
	}
	else if(fenceStyle != AuScFence::STYLE_Count)
	{
		info += "<br>";
		info += tr("Style: <b>%2</b>")
			.arg(AuScFence::StyleName(fenceStyle));
	}

	info += tr("<hr>");
	if(isUnderConstruction)
	{
		m_Icon->setPixmap(QPixmap(":data/ConstructionSign.png").scaled(60,60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		info += tr("<span style=\"color: rgb(255, 128, 0);\">Under Construction</span>");

		{
			// Jobs/Workers
			info += "<br>";
			info += tr("Jobs: <b>1 Construction</b>");

			info += "<br>";
			if(workers)
				info += tr("Construction Workers: <b>%L1</b>")
					.arg(workers);
			else
				info += tr("Construction Workers: <b>None</b>");
		}
	}
	else
	{
		m_Icon->setPixmap(QPixmap(DbIndustry::BitmapName(iid)));
		info += tr("Quality of Construction: <b>Q%1</b>")
			.arg(qmade);

		if(homesLarge || homesMedium || homesSmall)
		{
			// Homes/Residents
			info += "<br>";
			if(isMilitary)
			{
				if(homesLarge && homesMedium && homesSmall)
				{
					info += tr("Barracks: <b>%L1 Large, %L2 Medium, %L3 Small</b>")
						.arg(homesLarge)
						.arg(homesMedium)
						.arg(homesSmall);
				}
				else if(homesLarge && homesMedium)
				{
					info += tr("Barracks: <b>%L1 Large, %L2 Medium</b>")
						.arg(homesLarge)
						.arg(homesMedium);
				}
				else if(homesMedium && homesSmall)
				{
					info += tr("Barracks: <b>%L1 Medium, %L2 Small</b>")
						.arg(homesMedium)
						.arg(homesSmall);
				}
				else if(homesLarge && homesSmall)
				{
					info += tr("Barracks: <b>%L1 Large, %L2 Small</b>")
						.arg(homesLarge)
						.arg(homesSmall);
				}
				else if(homesLarge)
				{
					info += tr("Barracks: <b>%L1 Large</b>")
						.arg(homesLarge);
				}
				else if(homesMedium)
				{
					info += tr("Barracks: <b>%L1 Medium</b>")
						.arg(homesMedium);
				}
				else
				{
					info += tr("Barracks: <b>%L1 Small</b>")
						.arg(homesSmall);
				}

				info += "<br>";
				if(residents)
				{
					info += tr("Troops Resident: <b>%L1</b>")
						.arg(residents);
					info += "<br>";
					info += tr("Troops Employed: <b>%L1</b>")
						.arg(residentsEmployed);
				}
				else
					info += tr("Troops Resident: <b>None</b>");
			}
			else
			{
				if(homesLarge && homesMedium && homesSmall)
				{
					info += tr("Homes: <b>%L1 Large, %L2 Medium, %L3 Small</b>")
						.arg(homesLarge)
						.arg(homesMedium)
						.arg(homesSmall);
				}
				else if(homesLarge && homesMedium)
				{
					info += tr("Homes: <b>%L1 Large, %L2 Medium</b>")
						.arg(homesLarge)
						.arg(homesMedium);
				}
				else if(homesMedium && homesSmall)
				{
					info += tr("Homes: <b>%L1 Medium, %L2 Small</b>")
						.arg(homesMedium)
						.arg(homesSmall);
				}
				else if(homesLarge && homesSmall)
				{
					info += tr("Homes: <b>%L1 Large, %L2 Small</b>")
						.arg(homesLarge)
						.arg(homesSmall);
				}
				else if(homesLarge)
				{
					info += tr("Homes: <b>%L1 Large</b>")
						.arg(homesLarge);
				}
				else if(homesMedium)
				{
					info += tr("Homes: <b>%L1 Medium</b>")
						.arg(homesMedium);
				}
				else
				{
					info += tr("Homes: <b>%L1 Small</b>")
						.arg(homesSmall);
				}

				info += "<br>";
				if(residents)
				{
					info += tr("Residents: <b>%L1</b>")
						.arg(residents);
					info += "<br>";
					info += tr("Residents Employed: <b>%L1</b>")
						.arg(residentsEmployed);
				}
				else
					info += tr("Residents: <b>None</b>");
			}
		}

		if(manufacturingJobs || proJobs || serviceJobs)
		{
			// Jobs/Workers
			info += "<br>";
			if(manufacturingJobs)
			{
				if(serviceJobs)
					info += tr("Jobs: <b>%L1 Manufacturing, %L2 Service</b>")
						.arg(manufacturingJobs)
						.arg(serviceJobs);
				else
					info += tr("Jobs: <b>%L1 Manufacturing</b>")
						.arg(manufacturingJobs);
			}
			else if(proJobs)
			{
				if(serviceJobs)
					info += tr("Jobs: <b>%L1 Professional, %L2 Service</b>")
						.arg(proJobs)
						.arg(serviceJobs);
				else
					info += tr("Jobs: <b>%L1 Professional</b>")
						.arg(proJobs);
			}
			else if(serviceJobs)
				info += tr("Jobs: <b>%L1 Service</b>")
					.arg(serviceJobs);

			info += "<br>";
			if(workers)
				info += tr("Workers: <b>%L1</b>")
					.arg(workers);
			else
				info += tr("Workers: <b>None</b>");

			// There could only be usable stores or lounges if there was at least one service job.
			if(lounges)
				info += tr("<br>Lounges: <b>%L1</b>")
					.arg(lounges);
			if(stores)
				info += tr("<br>Stores: <b>%L1</b>")
					.arg(stores);
		}

		info += "<br>";
		info += tr("Electricity: <b>%1</b>")
			.arg(hasPower?tr("Yes"):tr("No"));

		if(storage)
		{
			info += "<br>";
			info += tr("Commodity Storage: <b>Space for %L1 per Type</b>")
				.arg(storage);
		}
	}

	// Mines, drilling rigs, and wells report the rocks resources present on their site.
	if(!rocks.empty())
	{
		// Sort the list alphabetically by commodity name.
		QMap<QString, QHash<COMMODITYID, quint32>::const_iterator> sorted;
		for(QHash<COMMODITYID, quint32>::const_iterator ri = rocks.cbegin(); ri != rocks.cend(); ri++)
			sorted.insert(g_Au.CommodityName(ri.key()), ri);

		info += tr("<hr>");
		info += tr("<b>Mineral Resources Present</b>");
		for(QMap<QString, QHash<COMMODITYID, quint32>::const_iterator>::const_iterator si = sorted.cbegin(); si != sorted.cend();
				si++)
		{
			info += QString("<br><img height=\"16\" align=\"middle\" src=\"qrc%1\"> %2 %3")
				.arg(g_Au.CommodityBitmapName((*si).key()))
				.arg(g_Au.CommodityName((*si).key()))
				.arg((*si).value());
		}
	}

	info += tr("<hr>");
	if(hasAirportAccess || hasWharfAccess || tradeByLand || tradeBySea || tradeByAir)
	{
		if(tradeByLand)
			info += tr("<b>Connected by Land.</b> Connected for trade and power distribution with %L1 other buildings by land.<br>")
				.arg(tradeByLand);
		if(hasWharfAccess)
			info += tr("<b>Wharf Access.</b> Access to a wharf permits trade by sea with other buildings that have wharf "
					"access, on this world.<br>");
		if(tradeBySea)
			info += tr("<b>Connected by Sea.</b> Connected for trade with %L1 other buildings by sea.<br>")
				.arg(tradeBySea);
		if(hasAirportAccess)
			info += tr("<b>Airport Access.</b> Access to an airport permits trade by air and space with other buildings that "
					"have airport access, in this solar system. "
					"Airport access permits brokers to send shipments to outsystem cities.<br>");
		if(tradeByAir)
			info += tr("<b>Connected by Air.</b> Connected for trade with %L1 other buildings by air or space.<br>")
				.arg(tradeByAir);
	}
	else
	{
		// To trade connections.
		info += tr("<b>Isolated.</b> Not connected with any other buildings for trade or power distribution.");
	}

	if((!inven.empty() && !m_InCoC) || !patents.empty())
	{
		info += tr("<hr>");
		if(!inven.empty() && !m_InCoC)
		{
			// Not shown when person is in in c-o-c because the same information appears in the m_Cargo control.
			info += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			for(QMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = inven.cbegin(); iit != inven.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				info += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td>"
						"<td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			info += "</tbody></table>";
			if(!patents.empty())
				info += "<br><br>";
		}

		// Patents
		if(!patents.empty())
		{
			info += QObject::tr("Patents available to manufacturing processes.<br>");

			info += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(41, 41, 82);\"><tbody>");
			info += QObject::tr("<tr>"
					"<td><b>Patent</b></td>"
					"</tr>");
			QMap<QString, COMMODITYID> sortedPatents;
			for(QHash<COMMODITYID, quint8>::const_iterator vi = patents.cbegin(); vi != patents.cend(); vi++)
				if(!DbCommodity::IsInvalid(vi.key()))
					sortedPatents.insert(g_Au.CommodityName(vi.key()), vi.key());
			for(QMap<QString, COMMODITYID>::const_iterator si = sortedPatents.cbegin(); si != sortedPatents.cend(); si++)
				info += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%1\"> %2 Q%3</td></tr>")
					.arg(g_Au.CommodityBitmapName(*si))
					.arg(si.key())
					.arg(patents.PatentQuality(*si));
			info += "</tbody></table>";
			info += "<br>";
		}
	}
	info += "</html>";

	if(m_InfoText != info)
	{
		m_Info->setHtml(info);
		m_InfoText = info;
	}

	if(!m_InCoC || iid == IID_StreetLight)
	{
		m_Cargo->hide();
		m_CargoDesc->hide();
		m_CountLabel->hide();
		m_BuyCountSpin->hide();
		m_OkButton->hide();
		m_TrashButton->hide();
		m_CommodityFilter->hide();
	}
	else
	{
		m_Cargo->show();
		m_CargoDesc->show();
		m_CountLabel->show();
		m_BuyCountSpin->show();
		m_OkButton->show();
		m_TrashButton->show();
		m_CommodityFilter->show();
	}

	EnableControls();
}

void AuBldBuilding::Filter(COMMODITYFILTER filter)
{
	m_Filter = filter;
	UpdateContents();
}

void AuBldBuilding::CargoSelected()
{
	if(m_BuyItem)
	{
		delete m_BuyItem;
		m_BuyItem = 0;
	}
	m_EmpireOfOrigin = "";
	m_MaxCount = 0;

	if(m_Cargo)
	{
		QString cargotext;
		QItemSelectionModel* ism = m_Cargo->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
					QByteArray code = cur->data(Qt::UserRole).toByteArray();
					m_BuyItem = AuItem::Unpack(code);
					if(m_BuyItem)
					{
						QSize packSize = m_BuyItem->BitmapSizeToFit(100);
						cargotext = QString("<html>"
								"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
								"</html>")
							.arg(m_BuyItem->BitmapName())
							.arg(m_BuyItem->HtmlDesc())
							.arg(packSize.width())
							.arg(packSize.height());
						m_EmpireOfOrigin = m_BuyItem->EmpireNameOfOrigin();
						m_MaxCount = m_BuyItem->Count();
					}
				}
			}
		}
		m_CargoDesc->setHtml(cargotext);
	}

	if(m_BuyItem)
	{
		if(m_MaxCount)
			m_BuyCountSpin->setMaximum(m_MaxCount);
		else
			m_BuyCountSpin->setMaximum(2000000000);
	}

	EnableControls();
}

void AuBldBuilding::ListCargo(DaBd* bdOp, QListWidget* lw)
{
	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_Filter);

	// Watch for changes.
	// Insure we have the empire and specie data loaded for all items in cargo, not including items in containers.
	{
		QStringList allCargo;
		const AuItemHash& cr = bdOp->LockCargoForRead();
		auApp->QueryEmpireAndSpecie(cr);
		for(AuItemHash::const_iterator cit = cr.begin(); cit != cr.end(); cit++)
		{
			if(!cidFilter.contains((*cit).second->CommodityId()))
				continue;
			QString text = (*cit).second->ListDesc();
			allCargo.append(text);
		}
		bdOp->UnlockCargo();
		if(allCargo == m_AllCargo)
			return;
		m_AllCargo = allCargo;
	}

	// List commodities.
	{
		lw->clear();

		const AuItemHash& cr = bdOp->LockCargoForRead();
		for(AuItemHash::const_iterator cit = cr.begin(); cit != cr.end(); cit++)
		{
			// Apply filter to list.
			if(!cidFilter.contains((*cit).second->CommodityId()))
				continue;

			QString text = (*cit).second->ListDesc();
			QByteArray code = (*cit).second->Pack();

			QPixmap px32 = QPixmap((*cit).second->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QIcon icon(px32);

			QListWidgetItem* lwi = new QListWidgetItem(icon, text, lw);
			lwi->setData(Qt::UserRole, code);
		}
		bdOp->UnlockCargo();
	}
}

void AuBldBuilding::EnableControls()
{
	// Enable/disabled controls.
	bool enable = m_BuyItem;	// We are open and an item is selected to buy.
	m_BuyCountSpin->setEnabled(enable);
	m_OkButton->setEnabled(enable);
	m_TrashButton->setEnabled(enable);
}

void AuBldBuilding::QuantityToBuyChanged(int buyCount)
{
	Q_UNUSED(buyCount);
	EnableControls();
}

void AuBldBuilding::TrashPressed()
{
	if(!m_BuyItem)
		return;
	int buyCount = m_BuyCountSpin->value();
	if(buyCount <= 0 || buyCount > (int)m_BuyItem->Count())
		return;

	quint32 nuCount = m_BuyItem->Count() - buyCount;
	m_BuyItem->SetCount(buyCount);
	auApp->BuildingTrashItem(*m_BuyItem, m_CapitolOpId);
	m_BuyItem->SetCount(nuCount);
}

void AuBldBuilding::OkPressed()
{
	if(!m_BuyItem)
		return;
	int buyCount = m_BuyCountSpin->value();
	if(buyCount > 100)
		buyCount = 100;	// Limit the amount that can be taken in one go. Someone tried 353482 cryozine. D:
	if(buyCount <= 0 || buyCount > (int)m_BuyItem->Count())
		return;

	quint32 nuCount = m_BuyItem->Count() - buyCount;
	m_BuyItem->SetCount(buyCount);
	auApp->BuildingTakeItem(*m_BuyItem, m_CapitolOpId);
	m_BuyItem->SetCount(nuCount);
}

