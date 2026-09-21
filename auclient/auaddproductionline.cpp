//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaddproductionline.cpp,v 1.14 2022/05/04 20:39:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auaddproductionline.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "dgdata.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

AuAddProductionLine::AuAddProductionLine(const AuId& capitolOpId, quint32 maxWorkshops, QWidget* parent) : AuDialog(parent)
{
	int minQ = g_Au.Settings().readNumEntry("Widget/AddProductionLine/MinQ",1);
	if(minQ < 0 || minQ > 255)
		minQ = 1;
	int curStockLimit = g_Au.Settings().readNumEntry("Widget/AddProductionLine/StockLimit",0);
	if(curStockLimit < 0)
		curStockLimit = 0;

	m_CapitolOpId = capitolOpId;
	setAuDlgTitle(tr("Add Production Line"));
	setAuDlgIcon(QIcon(":data/HammerAndSickle.png"));

	m_ProcCombo = new QComboBox(this);
	connect(m_ProcCombo, SIGNAL(currentIndexChanged(int)), SLOT(ProcessChanged(int)));

	m_ProcLabel = new QLabel(tr("Manufacturing Process"), this);

	lbWorkshops = new QLabel(tr("<html>"
				"Workshops"
				"</html>"), this);

	m_Workshops = new QSpinBox(this);
	m_Workshops->setRange(0, maxWorkshops);
	m_Workshops->setValue(maxWorkshops);
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
				"When set to zero, workers at this building do not work on the production line. "
				"Players in the chain-of-command of the city can fetch materials and run the production line. "
				"</p>"
				"<p>"
				"When developing patents, each workshop creates a prototype. "
				"Prototypes are tested for failure based on their quality. "
				"A patent is developed if any prototype passes. "
				"</p>"
				"</html>"));

	pbshop1 = new QPushButton(tr("1"), this);
	connect(pbshop1, SIGNAL(clicked()), SLOT(Shop1()));

	lbMinQ = new QLabel(tr("Production Quality"), this);

	m_MinQ = new QSpinBox(this);
	m_MinQ->setRange(1,255);
	m_MinQ->setValue(minQ);
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
	connect(m_MinQ, SIGNAL(valueChanged(int)), SLOT(MinQChanged(int)));

	lbStockLimit = new QLabel(tr("<html>"
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
				"<p>"
				"When developing patents, a one causes the process to pause once the empire has the patent, "
				"regardless of where it is developed. "
				"Zero or any number greater than one will result in the process running repeatedly. "
				"When higher quality components become available, a higher quality patent may be developed. "
				"</p>"
				"</html>"));
	connect(m_StockLimit, SIGNAL(valueChanged(int)), SLOT(StockLimitChanged(int)));

	QHBoxLayout* shoplay = new QHBoxLayout;
	shoplay->addWidget(m_Workshops, 1);
	shoplay->addWidget(pbshop1);

	pbstock0 = new QPushButton(tr("No Limit"), this);
	connect(pbstock0, SIGNAL(clicked()), SLOT(Stock0()));

	pbstock1 = new QPushButton(tr("1"), this);
	connect(pbstock1, SIGNAL(clicked()), SLOT(Stock1()));

	pbstock10 = new QPushButton(tr("10"), this);
	connect(pbstock10, SIGNAL(clicked()), SLOT(Stock10()));

	pbstock100 = new QPushButton(tr("100"), this);
	connect(pbstock100, SIGNAL(clicked()), SLOT(Stock100()));

	pbstock1000 = new QPushButton(tr("1000"), this);
	connect(pbstock1000, SIGNAL(clicked()), SLOT(Stock1000()));

	QHBoxLayout* stocklay = new QHBoxLayout;
	stocklay->addWidget(m_StockLimit, 1);
	stocklay->addWidget(pbstock0);
	stocklay->addWidget(pbstock1);
	stocklay->addWidget(pbstock10);
	stocklay->addWidget(pbstock100);
	stocklay->addWidget(pbstock1000);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QGridLayout* glay = new QGridLayout;
	int row=0;
	glay->addWidget(m_ProcLabel, row,0);
	glay->addWidget(m_ProcCombo, row,1);
	row++;
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

AuAddProductionLine::~AuAddProductionLine()
{
}

void AuAddProductionLine::setVisible(bool visible)
{
	if(visible)
	{
		ListProcesses();
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuAddProductionLine::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AddProductionLine/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AddProductionLine/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AddProductionLine/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AddProductionLine/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuAddProductionLine::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AddProductionLine/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AddProductionLine/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AddProductionLine/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AddProductionLine/SizeY",sz.height());
}

void AuAddProductionLine::ListProcesses()
{
	m_ProcCombo->blockSignals(true);
	QMap<QString, quint16> processes;
	AuView* pView = auApp->Frame()->View();
	int listResult = 0;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp)
			listResult = bdOp->ListAddableProcesses(processes);
	}
	pView->UnlockSceneAtom();
	// Add non-patent development processes first.
	for(QMap<QString, quint16>::const_iterator pi = processes.cbegin(); pi != processes.cend(); pi++)
	{
		DbManufacturingProcess bMan;
		const DgData& gd = g_Au.LockDataForRead();
		bool hasProc = gd.ManufacturingProcess(*pi, bMan);
		g_Au.UnlockData();
		if(!hasProc)
			continue;
		if(bMan.IsRnD())
			continue;
		m_ProcCombo->addItem(QIcon(g_Au.CommodityBitmapName(bMan.CommodityId())), bMan.m_name, bMan.Id());
	}
	// Add patent development processes last.
	for(QMap<QString, quint16>::const_iterator pi = processes.cbegin(); pi != processes.cend(); pi++)
	{
		DbManufacturingProcess bMan;
		const DgData& gd = g_Au.LockDataForRead();
		bool hasProc = gd.ManufacturingProcess(*pi, bMan);
		g_Au.UnlockData();
		if(!hasProc)
			continue;
		if(!bMan.IsRnD())
			continue;
		m_ProcCombo->addItem(QIcon(g_Au.CommodityBitmapName(bMan.CommodityId())), bMan.m_name, bMan.Id());
	}
	bool hideEm = true;
	switch(listResult)
	{
	default:
	case 0:
		hideEm = false;
		break;
	case 1:
		m_ProcLabel->setText(tr("<p>"
					"Production lines cannot be added while the building is under construction."
					"</p>"
					"<p>"
					"Complete construction of the building."
					"</p>"));
		break;
	case 2:
		m_ProcLabel->setText(tr("<p>"
					"Production lines cannot be added to a spacecraft factory in this way."
					"</p>"));
		break;
	case 3:
		m_ProcLabel->setText(tr("<p>"
					"No unassigned workshop is available. "
					"A production line cannot be added. "
					"</p>"
					"<p>"
					"Deassign a workshop from a production line or delete a production line."
					"</p>"));
		break;
	case 4:
		m_ProcLabel->setText(tr("<p>"
					"No other manufacturing processes exist. "
					"A production line cannot be added. "
					"</p>"
					"<p>"
					"Additional manufacturing processes could become available as a result of patent research. "
					"</p>"));
		break;
	}

	if(hideEm)
	{
		m_ProcCombo->hide();
		lbWorkshops->hide();
		m_Workshops->hide();
		pbshop1->hide();
		lbMinQ->hide();
		m_MinQ->hide();
		lbStockLimit->hide();
		m_StockLimit->hide();
		pbstock0->hide();
		pbstock1->hide();
		pbstock10->hide();
		pbstock100->hide();
		pbstock1000->hide();
	}
	m_ProcCombo->blockSignals(false);
	ProcessChanged(0);
}

quint16 AuAddProductionLine::MPID() const
{
	int ci = m_ProcCombo->currentIndex();
	if(ci < 0)
		return 0;
	return m_ProcCombo->itemData(ci).toUInt();
}

quint8 AuAddProductionLine::MinQ() const
{
	return m_MinQ->value();
}

void AuAddProductionLine::MinQChanged(int value)
{
	g_Au.Settings().writeEntry("Widget/AddProductionLine/MinQ", value);
}

quint32 AuAddProductionLine::StockLimit() const
{
	return m_StockLimit->value();
}

void AuAddProductionLine::StockLimitChanged(int value)
{
	quint16 mpid = m_ProcCombo->currentData().toUInt();
	DbManufacturingProcess bMan;
	const DgData& gd = g_Au.LockDataForRead();
	bool hasProc = gd.ManufacturingProcess(mpid, bMan);
	g_Au.UnlockData();
	if(hasProc && !bMan.IsRnD())
		g_Au.Settings().writeEntry("Widget/AddProductionLine/StockLimit", value);
}

quint32 AuAddProductionLine::Workshops() const
{
	return m_Workshops->value();
}

void AuAddProductionLine::Shop1()
{
	m_Workshops->setValue(1);
}

void AuAddProductionLine::Stock0()
{
	m_StockLimit->setValue(0);
}

void AuAddProductionLine::Stock1()
{
	m_StockLimit->setValue(1);
}

void AuAddProductionLine::Stock10()
{
	m_StockLimit->setValue(10);
}

void AuAddProductionLine::Stock100()
{
	m_StockLimit->setValue(100);
}

void AuAddProductionLine::Stock1000()
{
	m_StockLimit->setValue(1000);
}

void AuAddProductionLine::ProcessChanged(int index)
{
	if(index < 0)
		return;
	quint16 mpid = m_ProcCombo->itemData(index).toUInt();
	DbManufacturingProcess bMan;
	const DgData& gd = g_Au.LockDataForRead();
	bool hasProc = gd.ManufacturingProcess(mpid, bMan);
	g_Au.UnlockData();
	if(!hasProc)
		return;	// wut?
	if(bMan.IsRnD())
	{
		m_Workshops->setValue(1);
		m_StockLimit->setValue(1);
	}
	else
	{
		int curStockLimit = g_Au.Settings().readNumEntry("Widget/AddProductionLine/StockLimit",0);
		if(curStockLimit < 0)
			curStockLimit = 0;
		m_StockLimit->setValue(curStockLimit);
	}
}

