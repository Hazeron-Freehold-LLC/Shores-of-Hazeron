//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auviewitem.cpp,v 1.3 2022/02/19 18:42:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auviewitem.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>

AuViewItem::AuViewItem(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_Crate.png"));
	setAuDlgTitle("<b>View Item or Vehicle</b>");

	COMMODITYID lastCID = (COMMODITYID)g_Au.Settings().readNumEntry("Widget/ViewItem/CID", CID_Invalid);
	if(lastCID <= CID_Invalid || lastCID >= CID_Count)
		lastCID = CID_StoneFirePit;

	// Combo box lists all possible commodities, including vehicles.
	QMap<QString, COMMODITYID> sortedCIDs;	// commodities sorted alphabetically
	for(DbCommodityArray::const_iterator cit = g_Au.m_CommodityArray.constBegin(); cit != g_Au.m_CommodityArray.constEnd(); cit++)
		if((*cit).m_id != CID_Invalid && (*cit).m_id != CID_Money)
			sortedCIDs.insert((*cit).m_name, (*cit).m_id);

	m_Commodity = new QComboBox(this);
	for(QMap<QString, COMMODITYID>::const_iterator sci = sortedCIDs.constBegin(); sci != sortedCIDs.constEnd(); sci++)
		m_Commodity->addItem(QIcon(g_Au.CommodityBitmapName(*sci)), sci.key(), QVariant(*sci));
	m_Commodity->setCurrentIndex(m_Commodity->findData(QVariant(lastCID)));
	m_Commodity->setToolTip(tr("<html>Item or vehicle to view."));

	// Ok button
	QPushButton* pbok = new QPushButton(tr("View Item/Vehicle"), this);
	pbok->setToolTip(tr("<html>Views a the item specified.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	// Construct final overall layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_Commodity, row,0);
	row++;
	lay->addWidget(pbok, row,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuViewItem::~AuViewItem()
{
}

COMMODITYID AuViewItem::CommodityId() const
{
	COMMODITYID cid = (COMMODITYID)(m_Commodity->itemData(m_Commodity->currentIndex()).toInt());
	g_Au.Settings().writeEntry("Widget/ViewItem/CID", cid);
	return cid;
}
