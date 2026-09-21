//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdtreedlg.cpp,v 1.3 2022/02/19 18:25:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdtreedlg.h"
//
#include "auclientglobal.h"
#include "auscparts.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuBdTreeDlg::AuBdTreeDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Tree"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("Choose the kind of tree, based on the resource it produces. "
				"The actual tree DNA will be determined by the environment in which the building is constructed. "), this);
	lbinfo->setWordWrap(true);

	m_ResourceChoices = new QComboBox(this);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setToolTip(tr("<html>"
				"Accepts the chosen tree resource. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoOk()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_ResourceChoices);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBdTreeDlg::~AuBdTreeDlg()
{
}

void AuBdTreeDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListResources();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuBdTreeDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuildingTreeDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuildingTreeDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuildingTreeDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuildingTreeDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdTreeDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuildingTreeDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuildingTreeDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuildingTreeDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuildingTreeDlg/SizeY",sz.height());
}

void AuBdTreeDlg::DoOk()
{
	accept();
}

COMMODITYID AuBdTreeDlg::PlantCID() const
{
	COMMODITYID cid = (COMMODITYID)m_ResourceChoices->itemData(m_ResourceChoices->currentIndex()).toInt();
	return cid;
}

void AuBdTreeDlg::ListResources()
{
	m_ResourceChoices->addItem(QIcon(":data/c_FarmMachinery.png"), tr("Agriculture"), CID_FarmMachinery);
	m_ResourceChoices->addItem(QIcon(":data/Game.png"), tr("Random"), CID_Invalid);
	for(DbCommodityArray::const_iterator ci = g_Au.m_CommodityArray.cbegin(); ci != g_Au.m_CommodityArray.cend(); ci++)
		if(DbCommodity::IsTreeResource((*ci).m_id))
			m_ResourceChoices->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);
}
