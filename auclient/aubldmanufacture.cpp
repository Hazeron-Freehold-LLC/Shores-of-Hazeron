//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldmanufacture.cpp,v 1.40 2022/05/05 23:01:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldmanufacture.h"
//
#include "auaddproductionline.h"
#include "auapp.h"
#include "auframe.h"
#include "aupatentsget.h"
#include "aupatentsrecord.h"
#include "aupatentstree.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "demanufacturingprocess.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdWorkshopSettings::AuBdWorkshopSettings(quint8 curMinQ, quint8 procMinQ, quint32 curWorkshops, quint32 maxWorkshops,
		quint32 curStockLimit, QWidget* parent) : AuDialog(parent)
{
	if(procMinQ < 1)
		procMinQ = 1;
	setAuDlgTitle(tr("Change Production Line"));
	setAuDlgIcon(QIcon(":data/HammerAndSickle.png"));

	QLabel* lbWorkshops = new QLabel(tr("<html>"
				"Workshops"
				"</html>"), this);

	m_Workshops = new QSpinBox(this);
	m_Workshops->setRange(0, maxWorkshops);
	m_Workshops->setValue(curWorkshops);
	m_Workshops->setToolTip(tr("<html>"
				"<p>"
				"<b>Workshops</b>"
				"<br>"
				"Maximum number is determined by the building blueprint. "
				"Mines, wells, and drilling rigs further limit workshops to resources present. "
				"</p>"
				"<p>"
				"Instead of <i>workshops</i>, some buildings report <i>assembly lines</i>, <i>auditoriums</i>, <i>fields</i>, "
				"or <i>generators</i>. "
				"</p>"
				"<p>"
				"When set to zero, workers at the building do not work on the production line. "
				"Then only players in the chain-of-command of the city can fetch materials and run the production line. "
				"</p>"
				"</html>"));

	QPushButton* pbshop1 = new QPushButton(tr("1"), this);
	connect(pbshop1, SIGNAL(clicked()), SLOT(Shop1()));

	QLabel* lbMinQ = new QLabel(tr("Production Quality"), this);

	m_MinQ = new QSpinBox(this);
	m_MinQ->setRange(procMinQ,255);
	m_MinQ->setValue(curMinQ);
	m_MinQ->setToolTip(tr("<html>"
				"<p>"
				"<b>Minimum Production Quality</b>"
				"<br>"
				"Sets the minimum production quality for commodities consumed by the production line, "
				"those listed in the first column. "
				"</p>"
				"<p>"
				"Quality of tools and other modifiers, listed in the second column, do not affect production quality, "
				"except when developing patents. "
				"</p>"
				"<p>"
				"Does not affect commodities fetched from the environment, as done by farming, logging, mining, "
				"drilling, refining, etc. "
				"Quality of those commodities is determined by the quality of natural resources present."
				"</p>"
				"<p>"
				"Certain resource gathering patents consume no commodities. "
				"Instead, they require a natural resource be present in the environment. "
				"The resulting quality of those patents is determined by the quality of the natural resource. "
				"</p>"
				"</html>"));

	QLabel* lbStockLimit = new QLabel(tr("<html>"
				"Stock Limit"
				"</html>"), this);

	m_StockLimit = new QSpinBox(this);
	m_StockLimit->setRange(0, 10000000);
	m_StockLimit->setValue(curStockLimit);
	m_StockLimit->setToolTip(tr("<html>"
				"<p>"
				"<b>Stock Limit</b>"
				"<br>"
				"When not zero, the process will pause when the stock at the building reaches or exceeds the limit. "
				"The process will resume when the building's stock drops below the limit. "
				"</p>"
				"<p>"
				"When set to zero, the process will fill all available storage in the building. "
				"</p>"
				"</html>"));

	QPushButton* pbstock0 = new QPushButton(tr("No Limit"), this);
	connect(pbstock0, SIGNAL(clicked()), SLOT(Stock0()));

	QPushButton* pbstock1 = new QPushButton(tr("1"), this);
	connect(pbstock1, SIGNAL(clicked()), SLOT(Stock1()));

	QPushButton* pbstock10 = new QPushButton(tr("10"), this);
	connect(pbstock10, SIGNAL(clicked()), SLOT(Stock10()));

	QPushButton* pbstock100 = new QPushButton(tr("100"), this);
	connect(pbstock100, SIGNAL(clicked()), SLOT(Stock100()));

	QPushButton* pbstock1000 = new QPushButton(tr("1000"), this);
	connect(pbstock1000, SIGNAL(clicked()), SLOT(Stock1000()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* shoplay = new QHBoxLayout;
	shoplay->addWidget(m_Workshops, 1);
	shoplay->addWidget(pbshop1);

	QHBoxLayout* stocklay = new QHBoxLayout;
	stocklay->addWidget(m_StockLimit, 1);
	stocklay->addWidget(pbstock0);
	stocklay->addWidget(pbstock1);
	stocklay->addWidget(pbstock10);
	stocklay->addWidget(pbstock100);
	stocklay->addWidget(pbstock1000);

	QGridLayout* glay = new QGridLayout;
	int row=0;
	glay->addWidget(lbWorkshops, row,0);
	glay->addLayout(shoplay, row,1);
	row++;
	glay->addWidget(lbMinQ, row,0);
	glay->addWidget(m_MinQ, row,1);
	row++;
	glay->addWidget(lbStockLimit, row,0);
	glay->addLayout(stocklay, row,1);
	row++;
	glay->setRowStretch(row,1);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(glay);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBdWorkshopSettings::~AuBdWorkshopSettings()
{
}

void AuBdWorkshopSettings::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuBdWorkshopSettings::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/WorkshopSettings/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/WorkshopSettings/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/WorkshopSettings/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/WorkshopSettings/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdWorkshopSettings::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/WorkshopSettings/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/WorkshopSettings/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/WorkshopSettings/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/WorkshopSettings/SizeY",sz.height());
}

quint8 AuBdWorkshopSettings::MinQ() const
{
	return m_MinQ->value();
}

quint32 AuBdWorkshopSettings::Workshops() const
{
	return m_Workshops->value();
}

quint32 AuBdWorkshopSettings::StockLimit() const
{
	return m_StockLimit->value();
}

void AuBdWorkshopSettings::Shop1()
{
	m_Workshops->setValue(1);
}

void AuBdWorkshopSettings::Stock0()
{
	m_StockLimit->setValue(0);
}

void AuBdWorkshopSettings::Stock1()
{
	m_StockLimit->setValue(1);
}

void AuBdWorkshopSettings::Stock10()
{
	m_StockLimit->setValue(10);
}

void AuBdWorkshopSettings::Stock100()
{
	m_StockLimit->setValue(100);
}

void AuBdWorkshopSettings::Stock1000()
{
	m_StockLimit->setValue(1000);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdManufacturePanel::AuBdManufacturePanel(QWidget* parent) : QWidget(parent)
{
	m_InCoC = false;
	m_ShopsEnabled = 0;
	m_ShopsMax = 0;
	m_StockLimit = 0;
	m_MPID = 0;
	m_CurMinQuality = 1;
	m_ProcMinQ = 1;

	// Make the controls.
	QSize delIconSize(6,6);
	m_DeleteProcess = new QToolButton(this);
	m_DeleteProcess->setIcon(QIcon(":data/Minus.png"));
	m_DeleteProcess->setIconSize(delIconSize);
	m_DeleteProcess->setToolTip(tr("<html><b>Delete...</b><br>"
				"Deletes a production line from the building. "
				"Materials and money are returned to the building's storage, space permitting. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_DeleteProcess, SIGNAL(clicked()), SLOT(DeleteProcess()));

	QSize priIconSize(6,6);
	m_MaxPriority = new QToolButton(this);
	m_MaxPriority->setIcon(QIcon(":data/hazLimitUp.png"));
	m_MaxPriority->setIconSize(priIconSize);
	m_MaxPriority->setToolTip(tr("<html><b>Raise to Top</b><br>"
				"Moves this production line to the top of the list. "
				"Workers are assigned to production lines from top to bottom. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_MaxPriority, SIGNAL(clicked()), SLOT(MaxPriority()));

	m_MinPriority = new QToolButton(this);
	m_MinPriority->setIcon(QIcon(":data/hazLimitDown.png"));
	m_MinPriority->setIconSize(priIconSize);
	m_MinPriority->setToolTip(tr("<html><b>Lower to Bottom</b><br>"
				"Moves this production line to the bottom of the list. "
				"Workers are assigned to production lines from top to bottom. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_MinPriority, SIGNAL(clicked()), SLOT(MinPriority()));

	m_RaisePriority = new QToolButton(this);
	m_RaisePriority->setIcon(QIcon(":data/hazHeaderSortUp.png"));
	m_RaisePriority->setIconSize(priIconSize);
	m_RaisePriority->setToolTip(tr("<html><b>Raise</b><br>"
				"Moves this production line up in the list. "
				"Workers are assigned to production lines from top to bottom. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_RaisePriority, SIGNAL(clicked()), SLOT(RaisePriority()));

	m_LowerPriority = new QToolButton(this);
	m_LowerPriority->setIcon(QIcon(":data/hazHeaderSortDown.png"));
	m_LowerPriority->setIconSize(priIconSize);
	m_LowerPriority->setToolTip(tr("<html><b>Lower</b><br>"
				"Moves this production line down in the list. "
				"Workers are assigned to production lines from top to bottom. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_LowerPriority, SIGNAL(clicked()), SLOT(LowerPriority()));

	QVBoxLayout* prilay = new QVBoxLayout;
	prilay->addWidget(m_DeleteProcess);
	prilay->addWidget(m_MaxPriority);
	prilay->addWidget(m_RaisePriority);
	prilay->addWidget(m_LowerPriority);
	prilay->addWidget(m_MinPriority);
	prilay->addStretch();

	m_LastOutput = new QLabel(this);
	m_LastOutput->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_LastOutput->setToolTip(tr("<html>Shows quality and amount of commodities made in the last run of the production line. "
				"Quality ranges from 1 to 255. "
				"A reason is displayed when a production line fails to complete properly.<br>"
				"Materials are not consumed when the output cannot be stored in the city's inventory.</html>"));

	m_Status = new QLabel(this);

	m_StatusIcon = new QLabel(this);

	m_MakeWhatIcon = new QLabel(this);

	m_MakeWhatName = new QLabel(this);
	m_MakeWhatName->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	m_MakeWhatName->setWordWrap(true);

	m_Components = new QListWidget(this);
	m_Components->setToolTip(tr("<html>Commodities consumed by the production line. "
				"All consumed commodities must be fetched before the production line run.</html>"));
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

	m_Workshops = new QPushButton(QIcon(":data/HammerAndSickle.png"), "", this);
	m_Workshops->setToolTip(tr("<html>"
				"<p>"
				"Changes workshops and minimum production quality assigned to this production line."
				"</p>"
				"<p>"
				"You must have governing authority to change this setting."
				"</p>"
				"</html>"));
	connect(m_Workshops, SIGNAL(clicked()), SLOT(WorkshopsClicked()));

	m_Fetch = new QPushButton(tr("Fetch"), this);
	m_Fetch->setToolTip(tr("<html>Fetches commodities from the city's inventory that are used by the production line. "
				"Commodities that can be used from your ready inventory, such as tools, are not fetched until the production line "
				"is run."
				"<br>"
				"Amount fetched is multiplied by the number of workers present. "
				"</html>"));
	connect(m_Fetch, SIGNAL(clicked(bool)), SLOT(FetchPushed()));

	m_Run = new QPushButton(tr("Run"), this);
	m_Run->setToolTip(tr("<html>Runs the production line. "
				"The production line will not run until all needed components are available. "
				"That means all commodities consumed by the production line must be fetched and "
				"all commodities <i>required</i> by the production line must be in the city's inventory or your ready inventory."
				"<br>"
				"%1 <b>Run</b> performs a <b>Fetch</b> first; you don't actually have to push the <b>Fetch</b> button.<br>"
				"%1 Tools in your ready inventory are used when <b>Run</b> is pressed.</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_Run, SIGNAL(clicked(bool)), SLOT(GoPushed()));

	QHBoxLayout* mlay = new QHBoxLayout;
	mlay->addWidget(m_MakeWhatIcon);
	mlay->addWidget(m_MakeWhatName, 1);

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(prilay, row,0, 3,1);
	lay->addLayout(mlay, row,1, 1,3);
	lay->addWidget(m_Fetch, row,4, 1,1);
	lay->addWidget(m_Run, row,5, 1,1);
	row++;
	lay->addWidget(m_Workshops, row,1, 1,5);
	row++;
	lay->addWidget(m_StatusIcon, row,1, 1,1);
	lay->addWidget(m_Status, row,2, 1,1);
	lay->addWidget(m_LastOutput, row,3, 1,3);
	row++;
	lay->setRowStretch(row,1);
	int rows = row+1;
	row=0;
	lay->addWidget(m_Components, row,6, rows,1);
	lay->addWidget(m_Modifiers, row,7, rows,1);

	lay->setColumnStretch(2,3);
	lay->setColumnStretch(6,2);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBdManufacturePanel::~AuBdManufacturePanel()
{
}

void AuBdManufacturePanel::SetChainOfCommand(bool inCoC)
{
	m_InCoC = inCoC;
}

void AuBdManufacturePanel::SetShopCount(quint32 shops)
{
	m_ShopsMax = shops;
}

void AuBdManufacturePanel::UpdateContents(const DaBd* bd, quint16 panelIndex, const AuId& capitolOpId)
{
	const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(bd->FindParentPlanetInEnv());
	if(!llworld)
		return;
	m_CapitolOpId = capitolOpId;

	// If the person has walked into a spacecraft factory, don't do anything.
	// The UI will change over to the spacecraft factory soon enough.
	INDUSTRYID iid = bd->IndustryId();
	if(iid == IID_SpacecraftFactory)
		return;

	const DeManufacturingProcessArray& mpa = bd->LockManufacturingProcessesForRead();
	if((quint32)panelIndex >= mpa.size())
	{
		bd->UnlockManufacturingProcesses();
		return;
	}

	// Remember manufacturing process settings.
	const DeManufacturingProcess& mp = mpa.FindProcess(panelIndex);
	m_ShopsEnabled = mp.ShopsEnabled();
	m_StockLimit = mp.StockLimit();
	quint32 workers = mp.WorkersPresent();
	m_MPID = mp.ManufacturingProcessId();

	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.constBegin();
			mpi < gd.m_bManufacturingProcessArray.constEnd(); mpi++)
	{
		if((*mpi).Id() != m_MPID)
			continue;
		COMMODITYID cid = (COMMODITYID)(*mpi).m_idc;
		m_MakeWhatIcon->setPixmap(QPixmap(g_Au.CommodityBitmapName(cid)).scaledToHeight(32, Qt::SmoothTransformation));
		m_MakeWhatName->setText(QString("<html><b>%1</b></html>").arg((*mpi).m_name));
		break;
	}

	int shopType = 0;	// 0=shop, 1=assembly line, 2=auditorium, 3=field, 4=generator
	const DbIndustry& dbi = gd.Industry(iid);
	if(dbi.DoesManufacture())
	{
		if(dbi.IndustryId() == IID_SpacecraftFactory ||
				dbi.IndustryId() == IID_AircraftFactory ||
				dbi.IndustryId() == IID_ShipYard ||
				dbi.IndustryId() == IID_VehicleFactory)
			shopType = 1;
		else
		{
			if(dbi.Auditorium() > 0)
				shopType = 2;
			else if(dbi.Field() > 0)
				shopType = 3;
			else if(dbi.Generator() >= 0)
				shopType = 4;
		}
	}
	g_Au.UnlockData();

	QString shopsText;
	switch(shopType)
	{
	default:
	case 0:	// shop
		if(m_ShopsEnabled == 1)
			shopsText = tr("1 Workshop");
		else if(m_ShopsEnabled > 1)
			shopsText = tr("%1 Workshops").arg(m_ShopsEnabled);
		else
			shopsText = tr("No Workshops");
		break;
	case 1:	// assembly line
		if(m_ShopsEnabled == 1)
			shopsText = tr("1 Assembly Line");
		else if(m_ShopsEnabled > 1)
			shopsText = tr("%1 Assembly Lines").arg(m_ShopsEnabled);
		else
			shopsText = tr("No Assembly Lines");
		break;
	case 2:	// auditorium
		if(m_ShopsEnabled == 1)
			shopsText = tr("1 Auditorium");
		else if(m_ShopsEnabled > 1)
			shopsText = tr("%1 Auditoriums").arg(m_ShopsEnabled);
		else
			shopsText = tr("No Auditoriums");
		break;
	case 3:	// field
		if(m_ShopsEnabled == 1)
			shopsText = tr("1 Field");
		else if(m_ShopsEnabled > 1)
			shopsText = tr("%1 Fields").arg(m_ShopsEnabled);
		else
			shopsText = tr("No Fields");
		break;
	case 4:	// generator
		if(m_ShopsEnabled == 1)
			shopsText = tr("1 Generator");
		else if(m_ShopsEnabled > 1)
			shopsText = tr("%1 Generators").arg(m_ShopsEnabled);
		else
			shopsText = tr("No Generators");
		break;
	}

	// Components and Modifiers
	// We'll do these simultaneously so we only have to loop through the
	// process components once.
	m_Components->clear();
	m_Modifiers->clearContents();
	while(m_Modifiers->rowCount())
		m_Modifiers->removeRow(0);
	for(DeProcessComponentArray::const_iterator pci = mp.Components().constBegin(); pci != mp.Components().constEnd(); pci++)
	{
		QIcon icon;
		if(!DbCommodity::IsObsolete((*pci).CommodityId()))
			icon = QIcon(QPixmap((*pci).CommodityBitmapName()).scaledToHeight(16, Qt::SmoothTransformation));
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
					text = tr("%1 Q%2")
						.arg((*pci).QtyApplied())
						.arg((*pci).Quality());
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
					text = tr("%1 of %2").arg((*pci).QtyApplied())
							.arg((*pci).QtyConsumed());
					//text.sprintf("%d of %d", (*pci).QtyApplied(),
					//		(*pci).QtyConsumed());
			}
			QListWidgetItem* lwi = new QListWidgetItem(icon, text, m_Components);
			lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
		}
		else
		{
			// Construction "Modifiers" are those components that are
			// not required OR that do not get consumed by the process.
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
			lwi->setSizeHint(QSize(1,16));
			lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
			m_Modifiers->setItem(0, col++, lwi);

			lwi = new QTableWidgetItem(QIcon(QPixmap(modicon).scaledToHeight(16, Qt::SmoothTransformation)), "");
			lwi->setSizeHint(QSize(1,16));
			lwi->setToolTip(tr("%1 %2").arg(g_Au.CommodityName((*pci).CommodityId())).arg(text));
			m_Modifiers->setItem(0, col++, lwi);

			if((*pci).QtyApplied())
			{
				// Component has been applied.
				QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(QPixmap(":data/mpReady.png")
							.scaledToHeight(16, Qt::SmoothTransformation)), "");
				lwi->setSizeHint(QSize(1,16));
				lwi->setToolTip(tr("%1 Ready").arg(g_Au.CommodityName((*pci).CommodityId())));
				m_Modifiers->setItem(0, col++, lwi);
			}
			else if((*pci).IsRequired())
			{
				// Missing required component is holding up the process.
				QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(QPixmap(":data/mpNotReady.png")
							.scaledToHeight(16, Qt::SmoothTransformation)), "");
				lwi->setSizeHint(QSize(1,16));
				lwi->setToolTip(tr("%1 Needed").arg(g_Au.CommodityName((*pci).CommodityId())));
				m_Modifiers->setItem(0, col++, lwi);
			}
		}
	}
	m_Modifiers->resizeColumnsToContents();

	// Last output quality x count.
	unsigned char lastQual = mp.LastOutputQuality();
	qint32 lastCount = mp.LastOutputQuantity();
	if(lastCount < 0)
	{
		// Negative last output quantity is an error code.
		if(lastCount == -1)
			m_LastOutput->setText(tr("No Empty Parking Spot"));
		else if(lastCount == -2)
			m_LastOutput->setText(tr("No Live Animal Or Animal DNA Sample Present"));
		else if(lastCount == -3)
			m_LastOutput->setText(tr("Inventory Full"));
		else if(lastCount == -4)
			m_LastOutput->setText(tr("Obsolete Product Discarded"));
		else if(lastCount == -26)
			m_LastOutput->setText(tr("Invention Prototype Failed"));
		else if(lastCount == -27)
			m_LastOutput->setText(tr("Patent Already Recorded at Q%1").arg(lastQual));
		else if(lastCount == -99)
			m_LastOutput->setText(tr("Unimplemented Game Object"));
		else
			m_LastOutput->setText(tr("No Output %1").arg(-lastCount));
	}
	else if(lastQual || lastCount)
		m_LastOutput->setText(tr("Produced %2 at Q%1").arg(lastQual).arg(lastCount));
	else
		m_LastOutput->setText("");

	// Status
	// Set the name of the commodity being produced.
	QString makeWhat = mp.CommodityName();

	// Production Q
	m_CurMinQuality = mp.MinimumQSet();
	m_ProcMinQ = mp.MinimumQRequired();

	if(m_CurMinQuality > 1)
		shopsText += tr(" Q%1").arg(m_CurMinQuality);

	if(m_StockLimit > 0)
		shopsText += tr(", Limit %L1").arg(m_StockLimit);

	if(workers == 1)
		shopsText += tr(", 1 Worker");
	else if(workers > 1)
		shopsText += tr(", %L1 Workers").arg(workers);
	else
		shopsText += tr(", No Workers");

	m_Workshops->setText(shopsText);

	QString status;
	if(mp.IsRunning())
	{
		unsigned int secs = mp.SecondsRemaining();
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
		m_StatusIcon->setPixmap(QPixmap(":data/mpRunning.png").scaledToHeight(16, Qt::SmoothTransformation));
		m_Fetch->setEnabled(false);
		m_Run->setEnabled(false);
	}
	else if(mp.CanRun())
	{
		unsigned int secs = mp.SecondsAccumulated();
		unsigned int hr = secs / (60*60);
		secs -= hr * (60*60);
		unsigned int min = secs / 60;
		secs -= min * 60;
		if(hr)
			status = QString("Ready %1:%2:%3").arg(hr).arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		else
			status = QString("Ready %1:%2").arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		m_StatusIcon->setPixmap(QPixmap(":data/mpReady.png").scaledToHeight(16, Qt::SmoothTransformation));
		m_Fetch->setEnabled(false);
		m_Run->setEnabled(m_InCoC || m_ShopsEnabled);
	}
	else
	{
		unsigned int secs = mp.SecondsAccumulated();
		unsigned int hr = secs / (60*60);
		secs -= hr * (60*60);
		unsigned int min = secs / 60;
		secs -= min * 60;
		if(hr)
			status = QString("Not Ready %1:%2:%3").arg(hr).arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		else
			status = QString("Not Ready %1:%2").arg(min, 2, 10, QChar('0'))
				.arg(secs, 2, 10, QChar('0'));
		m_StatusIcon->setPixmap(QPixmap(":data/mpNotReady.png").scaledToHeight(16, Qt::SmoothTransformation));
		m_Fetch->setEnabled(m_InCoC || m_ShopsEnabled);
		m_Run->setEnabled(m_InCoC || m_ShopsEnabled);	// Enable because player may have lacking component.
	}
	bd->UnlockManufacturingProcesses();
	m_DeleteProcess->setEnabled(m_InCoC);
	m_LowerPriority->setEnabled(m_InCoC);
	m_RaisePriority->setEnabled(m_InCoC);
	if(!makeWhat.isEmpty())
		status = makeWhat + '\n' + status;
	m_Status->setText(status);
	m_Workshops->setEnabled(m_InCoC);
}

void AuBdManufacturePanel::FetchPushed()
{
	emit Fetch(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::GoPushed()
{
	emit Go(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::LowerPriority()
{
	emit PriorityLowered(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::RaisePriority()
{
	emit PriorityRaised(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::MaxPriority()
{
	emit PriorityMax(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::MinPriority()
{
	emit PriorityMin(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::DeleteProcess()
{
	// Show confirmation.
	QString smsg = tr("<html>"
			"Confirm delete of production line. "
			"Materials and money are returned to the building's storage, space permitting. "
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	emit DeleteProcess(m_MPID, m_CapitolOpId);
}

void AuBdManufacturePanel::WorkshopsClicked()
{
	AuBdWorkshopSettings dlg(m_CurMinQuality, m_ProcMinQ, m_ShopsEnabled, m_ShopsMax, m_StockLimit, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	if(dlg.Workshops() != m_ShopsEnabled)
		emit CitizenEnabled(m_MPID, dlg.Workshops(), m_CapitolOpId);
	if(dlg.MinQ() != m_CurMinQuality)
		emit MinQSet(m_MPID, dlg.MinQ(), m_CapitolOpId);
	if(dlg.StockLimit() != m_StockLimit)
		emit StockLimitSet(m_MPID, dlg.StockLimit(), m_CapitolOpId);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldManufacture::AuBldManufacture(QWidget* parent) : QWidget(parent)
{
	m_ShopsCount = 0;
	m_timer = 0;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	m_ShopsLabel = new QLabel(tr("Workshops"), this);

	m_AddProcess = new QToolButton(this);
	m_AddProcess->setIcon(QIcon(":data/HammerAndSickle.png"));
	m_AddProcess->setToolTip(tr("<html><b>New...</b><br>"
				"Adds a new production line to the building. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_AddProcess, SIGNAL(clicked()), SLOT(AddProcess()));

	m_ClearProcesses = new QToolButton(this);
	m_ClearProcesses->setIcon(QIcon(":data/ExitGame.png"));
	m_ClearProcesses->setToolTip(tr("<html><b>Clear...</b><br>"
				"Removes all production lines from the building. "
				"<br>"
				"<b>You must be in the chain of command of the city to do this.</b>"
				"</html>"));
	connect(m_ClearProcesses, SIGNAL(clicked()), SLOT(ClearProcesses()));

	m_ProcessQ = new QToolButton(this);
	m_ProcessQ->setIcon(QIcon(":data/Q.png"));
	m_ProcessQ->setToolTip(tr("<html>"
				"<p>"
				"<b>Minimum Production Quality</b>"
				"<br>"
				"Sets the minimum production quality for commodities consumed by ALL production lines, "
				"those listed in the first column. "
				"</p>"
				"<p>"
				"Quality of tools and other modifiers, listed in the second column, do not affect production quality, "
				"except when developing patents. "
				"</p>"
				"<p>"
				"Does not affect commodities fetched from the environment, as done by farming, logging, mining, "
				"drilling, refining, etc. "
				"Quality of those commodities is determined by the quality of natural resources present."
				"</p>"
				"<p>"
				"Certain resource gathering patents consume no commodities. "
				"Instead, they require a natural resource be present in the environment. "
				"The resulting quality of those patents is determined by the quality of the natural resource. "
				"</p>"
				"</html>"));
	connect(m_ProcessQ, SIGNAL(clicked()), SLOT(ProcessQ()));

	// Add controls to get and record patents.
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

	m_PatentsTree = new QToolButton(this);
	m_PatentsTree->setIcon(QIcon(":data/OrderRead.png"));
	m_PatentsTree->setToolTip(tr("<html><b>Patents Reference</b><br>"
				"Displays a list of all patents and their dependencies. "
				"</html>"));
	connect(m_PatentsTree, SIGNAL(clicked()), SLOT(PatentsTree()));

	QHBoxLayout* play = new QHBoxLayout;
	play->addWidget(m_ShopsLabel);
	play->addStretch();
	play->addWidget(m_AddProcess);
	play->addWidget(m_ProcessQ);
	play->addWidget(m_ClearProcesses);
	play->addStretch();
	play->addWidget(m_PatentsGet);
	play->addWidget(m_PatentsRecord);
	play->addWidget(m_PatentsTree);
	play->setMargin(2);
	play->setSpacing(2);

	// Build the layout.
	QWidget* panels = new QWidget;
	m_PanelLay = new QVBoxLayout(panels);
	for(QList<AuBdManufacturePanel*>::iterator pi = m_Panels.begin(); pi != m_Panels.end(); pi++)
		m_PanelLay->addWidget(*pi);
	m_PanelLay->setSpacing(2);
	m_PanelLay->setMargin(2);
	panels->setLayout(m_PanelLay);

	m_PanelScroller = new QScrollArea;
	m_PanelScroller->setWidget(panels);
	m_PanelScroller->setWidgetResizable(true);

	QVBoxLayout* flay = new QVBoxLayout;
	flay->addLayout(play);
	flay->addWidget(m_PanelScroller);
	flay->setSpacing(2);
	flay->setMargin(0);
	setLayout(flay);

	connect(this, SIGNAL(AddProcess(quint16, quint8, quint32, quint32, AuId)),
			auApp, SLOT(BdManufactureAddMPID(quint16, quint8, quint32, quint32, AuId)));
}

AuBldManufacture::~AuBldManufacture()
{
}

void AuBldManufacture::showEvent(QShowEvent* event)
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

void AuBldManufacture::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();

	// Hide my panels now or they force the parent AuBld dialog to be too big even though this page isn't visible.
	m_PanelScroller->hide();
	m_PatentsGet->hide();
	m_PatentsRecord->hide();
	m_PatentsTree->hide();
}

void AuBldManufacture::UpdateContents()
{
	m_PanelScroller->show();

	QString industryName;
	AuKnownPatents patents;	// Patents available.

	// Lock the root and find the scene atom.
	int mpcount = 0;	// Count of manufacturing processes present.
	bool inCoC = 0;
	m_ShopsCount = 0;
	INDUSTRYID iid = IID_None;
	quint32 shops = 0;
	quint32 workers = 0;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp)
		{
			industryName = bdOp->IndustryName();
			iid = bdOp->IndustryId();
			// Get patents available at the industry. At a university, this will be all of them in town.
			bdOp->ListPatents(patents, bdOp->IndustryId());

			inCoC = bdOp->InChainOfCommand(person);
			m_ShopsCount = bdOp->ManufacturingJobs();

			// Get number of manufacturing process from the development.
			const DeManufacturingProcessArray& mpa = bdOp->LockManufacturingProcessesForRead();
			shops = mpa.ShopsEnabled();		// Total shops currently enabled between all jobs.
			workers = mpa.WorkersPresent();	// Total workers present between all jobs.
			mpcount = (int)mpa.size();
			bdOp->UnlockManufacturingProcesses();
			int pidx = 0;
			for(QList<AuBdManufacturePanel*>::iterator pi = m_Panels.begin(); pi != m_Panels.end(); pi++)
			{
				(*pi)->UpdateContents(bdOp, pidx, m_CapitolOpId);
				pidx++;
			}
		}
	}
	pView->UnlockSceneAtom();

	int shopType = 0;	// 0=shop, 1=assembly line, 2=auditorium, 3=field, 4=generator
	const DgData& gd = g_Au.LockDataForRead();
	const DbIndustry& dbi = gd.Industry(iid);
	if(dbi.DoesManufacture())
	{
		if(dbi.IndustryId() == IID_SpacecraftFactory ||
				dbi.IndustryId() == IID_AircraftFactory ||
				dbi.IndustryId() == IID_ShipYard ||
				dbi.IndustryId() == IID_VehicleFactory)
			shopType = 1;
		else
		{
			if(dbi.Auditorium() > 0)
				shopType = 2;
			else if(dbi.Field() > 0)
				shopType = 3;
			else if(dbi.Generator() >= 0)
				shopType = 4;
		}
	}
	g_Au.UnlockData();

	QString shopsText;
	switch(shopType)
	{
	case 0:	// shop
		if(m_ShopsCount == 1)
			shopsText = tr("1 Workshop");
		else if(m_ShopsCount)
			shopsText = tr("%L1 Workshops").arg(m_ShopsCount);
		else
			shopsText = tr("No Workshops");
		break;
	case 1:	// assembly line
		if(m_ShopsCount == 1)
			shopsText = tr("1 Assembly Line");
		else if(m_ShopsCount)
			shopsText = tr("%L1 Assembly Lines").arg(m_ShopsCount);
		else
			shopsText = tr("No Assembly Lines");
		break;
	case 2:	// auditorium
		if(m_ShopsCount == 1)
			shopsText = tr("1 Auditorium");
		else if(m_ShopsCount)
			shopsText = tr("%L1 Auditoriums").arg(m_ShopsCount);
		else
			shopsText = tr("No Auditoriums");
		break;
	case 3:	// field
		if(m_ShopsCount == 1)
			shopsText = tr("1 Field");
		else if(m_ShopsCount)
			shopsText = tr("%L1 Fields").arg(m_ShopsCount);
		else
			shopsText = tr("No Fields");
		break;
	case 4:	// generator
		if(m_ShopsCount == 1)
			shopsText = tr("1 Generator");
		else if(m_ShopsCount)
			shopsText = tr("%L1 Generators").arg(m_ShopsCount);
		else
			shopsText = tr("No Generators");
		break;
	}

	if(shops && m_ShopsCount == 1)
		shopsText += tr(", In Use");
	else if(shops == 1)
		shopsText += tr(", 1 in Use");
	else if(shops)
		shopsText += tr(", %L1 in Use").arg(shops);

	if(workers == 1)
		shopsText += tr(", 1 Worker");
	else if(workers)
		shopsText += tr(", %L1 Workers").arg(workers);

	m_ShopsLabel->setText(shopsText);

	// Create additional panels that are needed.
	int pidx = m_Panels.size();
	while(pidx < mpcount)
	{
		AuBdManufacturePanel* panel = new AuBdManufacturePanel();
		panel->SetShopCount(m_ShopsCount);
		connect(panel, SIGNAL(CitizenEnabled(quint16, quint32, AuId)),
				auApp, SLOT(BdManufactureCitizenEnabledMPID(quint16, quint32, AuId)));
		connect(panel, SIGNAL(DeleteProcess(quint16, AuId)), auApp, SLOT(BdManufactureDeleteMPID(quint16, AuId)));
		connect(panel, SIGNAL(Fetch(quint16, AuId)), auApp, SLOT(BdManufactureFetchMPID(quint16, AuId)));
		connect(panel, SIGNAL(Go(quint16, AuId)), auApp, SLOT(BdManufactureGoMPID(quint16, AuId)));
		connect(panel, SIGNAL(MinQSet(quint16, quint8, AuId)), auApp, SLOT(BdManufactureProcessSetMinQMPID(quint16, quint8, AuId)));
		connect(panel, SIGNAL(PriorityLowered(quint16, AuId)), auApp, SLOT(BdManufactureProcessLowerMPID(quint16, AuId)));
		connect(panel, SIGNAL(PriorityMax(quint16, AuId)), auApp, SLOT(BdManufactureProcessMaxMPID(quint16, AuId)));
		connect(panel, SIGNAL(PriorityMin(quint16, AuId)), auApp, SLOT(BdManufactureProcessMinMPID(quint16, AuId)));
		connect(panel, SIGNAL(PriorityRaised(quint16, AuId)), auApp, SLOT(BdManufactureProcessRaiseMPID(quint16, AuId)));
		connect(panel, SIGNAL(StockLimitSet(quint16, quint32, AuId)),
				auApp, SLOT(BdManufactureProcessSetStockLimitMPID(quint16, quint32, AuId)));
		m_Panels.append(panel);
		m_PanelLay->addWidget(panel);
		pidx++;
	}

	// Show/hide panels that are needed.
	pidx = 0;
	for(QList<AuBdManufacturePanel*>::iterator pi = m_Panels.begin(); pi != m_Panels.end(); pi++)
	{
		bool showIt = pidx < mpcount;	// Don't show the excess.
		if(showIt)
		{
			// Do not list patented processes unless all needed patents are available.
			const DgData& gd = g_Au.LockDataForRead();
			if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*pi)->ManufacturingProcessId(), patents))
				showIt = false;
			g_Au.UnlockData();
		}

		if(showIt)
		{
			(*pi)->show();
			(*pi)->SetChainOfCommand(inCoC);
			(*pi)->SetShopCount(m_ShopsCount);
		}
		else
			(*pi)->hide();
		pidx++;
	}

	m_AddProcess->setEnabled(inCoC && m_ShopsCount);
	m_ClearProcesses->setEnabled(inCoC && mpcount);
	m_ProcessQ->setEnabled(inCoC && mpcount);
	m_PatentsGet->setEnabled(inCoC);
	m_PatentsGet->show();
	m_PatentsRecord->show();
	m_PatentsTree->show();
}

void AuBldManufacture::GetPatent()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
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

void AuBldManufacture::RecordPatent()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
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

void AuBldManufacture::AddProcess()
{
	// Show UI to get mpid of new process.
	AuAddProductionLine dlg(m_CapitolOpId, m_ShopsCount, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	quint16 mpid = dlg.MPID();
	quint8 minQ = dlg.MinQ();
	quint32 shops = dlg.Workshops();
	quint32 stockLimit = dlg.StockLimit();
	if(mpid)
		emit AddProcess(mpid, minQ, shops, stockLimit, m_CapitolOpId);
}

void AuBldManufacture::ClearProcesses()
{
	QString smsg = tr("<html>"
			"Confirm delete of ALL production lines. "
			"Materials and money are returned to the building's storage, space permitting. "
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	auApp->BdManufactureProcessesClear(m_CapitolOpId);
}

void AuBldManufacture::ProcessQ()
{
	int defQ = g_Au.Settings().readNumEntry("Widget/WorkshopSettings/MinQ",1);
	if(defQ < 1)
		defQ = 1;
	else if(defQ > 255)
		defQ = 255;
	bool ok;
	int processQ = QInputDialog::getInt(this, tr("Minimum Quality"), tr("Enter the miminum quality for all processes."),
			defQ, 1,255, 1,&ok);
	if(!ok)
		return;
	g_Au.Settings().writeEntry("Widget/WorkshopSettings/MinQ", processQ);
	auApp->BdManufactureProcessesSetMinQ(processQ, m_CapitolOpId);
}

void AuBldManufacture::PatentsTree()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	AuPatentsTreeDlg dlg(this);
	pFrame->ExecDialog(dlg);
}

