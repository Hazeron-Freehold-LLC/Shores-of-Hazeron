//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdstorage.cpp,v 1.3 2022/02/19 18:25:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdstorage.h"
//
#include "auglobal.h"
#include "dgdata.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AuBdStorage::AuBdStorage(INDUSTRYID iid, QWidget* parent) : AuDialog(parent)
{
	m_IID = iid;
	const DgData& gd = g_Au.LockDataForRead();
	QString iname = gd.IndustryName(iid);
	g_Au.UnlockData();

	setAuDlgTitle(tr("%1 Storage").arg(iname));
	setAuDlgIcon(QIcon(DbIndustry::BitmapName(iid)));

	int lastCID = g_Au.Settings().readNumEntry(QString("Widget/BuildingStorage/%1").arg(m_IID), CID_Invalid);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Warehouses and storage tanks fill up by importing commodities manufactured at buildings. "
				"Warehouses store dry commodities and storage tanks store liquids and compressed gases. "
				"</p>"
				"<p>"
				"Commodities at warehouses and storage tanks are available to manufacturing and construction processes. "
				"</p>"
				"<p>"
				"When space traders ask to buy cargo, brokers sell commodities that are stored in warehouses and storage tanks. "
				"</p>"
				"</html"), this);
	lbinfo->setWordWrap(true);

	if(m_IID == IID_StorageTank)
	{
		m_StoreCombo = new QComboBox(this);
		m_StoreCombo->setToolTip(tr("<html>"
					"Choose which liquid or gas commodity to import and store in the new storage tank."
					"</html>"));
	}
	else
	{
		m_StoreCombo = new QComboBox(this);
		m_StoreCombo->setToolTip(tr("<html>"
					"Choose which dry commodity to import and store in the new warehouse."
					"</html>"));
	}

	// Fill the combo box with appropriate commodity choices.
	QMap<QString, DbCommodityArray::const_iterator> sorted;
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.cbegin(); it != g_Au.m_CommodityArray.cend(); it++)
	{
		if(!(*it).CanBeCargo())
			continue;
		if((*it).IsPatent())
			continue;
		if((*it).IsObsolete())
			continue;
		if(m_IID == IID_StorageTank)
		{
			if(!(*it).IsStorageLiquid())
				continue;
		}
		else
		{
			if((*it).IsStorageLiquid())
				continue;
		}
		sorted.insert((*it).m_name, it);
	}
	m_StoreCombo->addItem(tr("Import Nothing"), CID_Invalid);
	for(QMap<QString, DbCommodityArray::const_iterator>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
	{
		DbCommodityArray::const_iterator it = *si;
		m_StoreCombo->addItem(QPixmap((*it).BitmapName()), (*it).m_name, (*it).m_id);
	}
	int cidindex = m_StoreCombo->findData(lastCID);
	m_StoreCombo->setCurrentIndex(cidindex);


	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Accepts the commodity to store. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(lbinfo);
	lay->addWidget(m_StoreCombo);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setSpacing(2);
	lay->setMargin(2);
	setAuDlgLayout(lay);
}

AuBdStorage::~AuBdStorage()
{
}

void AuBdStorage::setVisible(bool visible)
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

void AuBdStorage::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuildingStorage/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuildingStorage/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuildingStorage/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuildingStorage/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdStorage::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuildingStorage/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuildingStorage/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuildingStorage/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuildingStorage/SizeY",sz.height());
	g_Au.Settings().writeEntry(QString("Widget/BuildingStorage/%1").arg(m_IID), StoreCommodity());
}

COMMODITYID AuBdStorage::StoreCommodity() const
{
	int index = m_StoreCombo->currentIndex();
	if(index < 0)
		return CID_Invalid;
	return (COMMODITYID)m_StoreCombo->itemData(index).toUInt();
}
