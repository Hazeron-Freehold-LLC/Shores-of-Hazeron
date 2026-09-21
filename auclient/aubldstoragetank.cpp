//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldstoragetank.cpp,v 1.6 2022/02/19 18:09:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldstoragetank.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldStorageTank::AuBldStorageTank(QWidget* parent) : QWidget(parent)
{
	m_Timer = 0;

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Buildings store what they manufacture on-site. "
				"Buildings stop manufacturing when on-site storage is full. "
				"</p>"
				"<p>"
				"Warehouses and storage tanks increase storage capacity. "
				"This frees manufacturing potential and it accumulates large stocks of certain commodities. "
				"Using warehouses and storage tanks, manufactured output is moved and stored off-site, "
				"so manufacturing processes can continue."
				"</p>"
				"<p>"
				"Warehouses and storage tanks fill themselves by transferring commodities from other buildings. "
				"Warehouses store dry commodities and storage tanks store liquids and compressed gases. "
				"</p>"
				"<p>"
				"Commodities in warehouses and storage tanks are available to manufacturing and construction processes, "
				"and to brokers. "
				"When space traders ask to buy cargo, brokers sell commodities that are stored in warehouses and storage tanks. "
				"When brokers buy commodities, they store them in warehouses and storage tanks. "
				"</p>"
				"</html"), this);
	lbinfo->setWordWrap(true);

	m_StoreCombo = new QComboBox(this);
	m_StoreCombo->setToolTip(tr("<html>"
				"Choose which liquid or gas commodity to import and store in this storage tank. "
				"This does not limit the kinds of commodities stored in this storage tank by brokers. "
				"</html>"));
	connect(m_StoreCombo, SIGNAL(currentIndexChanged(int)), SLOT(CommoditySelectionChanged(int)));

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(lbinfo);
	lay->addWidget(m_StoreCombo);
	lay->addStretch();
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldStorageTank::~AuBldStorageTank()
{
}

void AuBldStorageTank::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(1000);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	UpdateContents();	// Don't wait.
}

void AuBldStorageTank::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();

	// Hide my widgets now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}

void AuBldStorageTank::UpdateContents()
{
	if(!isVisible())
		return;

	COMMODITYID storeCID = CID_Invalid;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp && bdOp->IndustryId() == IID_StorageTank)
			storeCID = bdOp->StoreCommodity();
	}
	pView->UnlockSceneAtom();

	m_StoreCombo->blockSignals(true);
	if(m_StoreCombo->count() == 0)
	{
		// First time through. Fill the combo box.
		QMap<QString, DbCommodityArray::const_iterator> sorted;
		for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.cbegin(); it != g_Au.m_CommodityArray.cend(); it++)
		{
			if(!(*it).CanBeCargo())
				continue;
			if((*it).IsPatent())
				continue;
			if((*it).IsObsolete())
				continue;
			if(!(*it).IsStorageLiquid())
				continue;
			sorted.insert((*it).m_name, it);
		}
		m_StoreCombo->addItem(tr("Import Nothing"), CID_Invalid);
		for(QMap<QString, DbCommodityArray::const_iterator>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
		{
			DbCommodityArray::const_iterator it = *si;
			m_StoreCombo->addItem(QPixmap((*it).BitmapName()), (*it).m_name, (*it).m_id);
		}
	}

	int cidindex = m_StoreCombo->findData(storeCID);
	m_StoreCombo->setCurrentIndex(cidindex);
	m_StoreCombo->blockSignals(false);
}

void AuBldStorageTank::CommoditySelectionChanged(int index)
{
	COMMODITYID storeCID = (COMMODITYID)m_StoreCombo->itemData(index).toUInt();
	auApp->BuildingStoreCommodity(storeCID, m_CapitolOpId);
}

