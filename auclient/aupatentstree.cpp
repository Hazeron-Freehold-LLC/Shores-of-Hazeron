//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupatentstree.cpp,v 1.3 2022/05/05 23:21:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupatentstree.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QLabel>
#include <QProgressDialog>
#include <QVBoxLayout>

AuPatentsTree::AuPatentsTree(QWidget* parent) : QTreeWidget(parent)
{
	setColumnCount(3);

	QStringList hdr;
	hdr << tr("Patent");
	hdr << tr("Inventors");
	hdr << tr("Users");
	setHeaderLabels(hdr);

	header()->setSectionResizeMode(0, QHeaderView::Stretch);
	header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

void AuPatentsTree::mousePressEvent(QMouseEvent* event)
{
	QTreeWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuPatentsTree::mouseReleaseEvent(QMouseEvent* event)
{
	QTreeWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTreeWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

void AuPatentsTree::ListPatents()
{
	const DgData& gd = g_Au.LockDataForRead();
	QMap<QString, COMMODITYID> patents;	// key = name, value = commodity id
	for(DbCommodityArray::const_iterator ci = g_Au.m_CommodityArray.cbegin(); ci != g_Au.m_CommodityArray.cend(); ci++)
	{
		if(!DbCommodity::IsPatent((*ci).m_id))
			continue;

		// Skip patents that cannot be made. e.g. Commercial Ship Apparatus Patent
		bool canBeMade = false;
		for(DbManufacturingProcessArray::const_iterator mi = gd.m_bManufacturingProcessArray.cbegin();
				mi != gd.m_bManufacturingProcessArray.cend(); mi++)
			if((*mi).CommodityId() == (*ci).m_id && (*mi).IndustryId() != IID_None)
			{
				canBeMade = true;
				break;
			}
		if(!canBeMade)
			continue;

		patents.insert((*ci).m_name, (*ci).m_id);
	}

	QProgressDialog* prog = new QProgressDialog(this);
	prog->setWindowTitle(tr("Listing Patents"));
	prog->setWindowModality(Qt::WindowModal);
	prog->setValue(0);
	prog->setRange(0, (int)patents.size());
	prog->setLabelText(tr("Checking dependencies of %L1 patents...").arg(patents.size()));

	// Make a top level tree item of every patent.
	for(QMap<QString, COMMODITYID>::const_iterator pi = patents.cbegin(); pi != patents.cend(); pi++)
	{
		prog->setValue(prog->value()+1);

		// List industries that can invent the patent.
		QMap<QString, INDUSTRYID> inventors;
		bool invUniv = false;
		for(DbManufacturingProcessArray::const_iterator mi = gd.m_bManufacturingProcessArray.cbegin();
				mi != gd.m_bManufacturingProcessArray.cend(); mi++)
			if((*mi).CommodityId() == *pi && (*mi).IndustryId() != IID_None)
			{
				if((*mi).IndustryId() == IID_School)
					invUniv = true;
				else
					inventors.insert(gd.IndustryName((*mi).IndustryId()), (*mi).IndustryId());
			}

		// List industries that use the patent.
		QMap<QString, INDUSTRYID> inds;
		for(DbManufacturingProcessArray::const_iterator mi = gd.m_bManufacturingProcessArray.cbegin();
				mi != gd.m_bManufacturingProcessArray.cend(); mi++)
		{
			// Does it use the patent.
			for(DbManufacturingComponentArray::const_iterator ci = gd.m_bManufacturingComponentArray.cbegin();
					ci != gd.m_bManufacturingComponentArray.cend(); ci++)
			{
				if((*ci).m_idmp == (*mi).m_id)
				{
					// Found a component of the process.
					if((*ci).CommodityId() == *pi)
					{
						// It is the patent so the industry uses the patent.
						inds.insert(gd.IndustryName((*mi).IndustryId()), (*mi).IndustryId()); 
						break;
					}
				}
			}
		}

		// Make a comma separated list of the inventor names.
		QString invNames;
		if(invUniv)
			invNames = gd.IndustryName(IID_School);	// EVERY one should start with University.
		for(QMap<QString, INDUSTRYID>::const_iterator ii = inventors.cbegin(); ii != inventors.cend(); ii++)
		{
			if(invNames.isEmpty())
				invNames = ii.key();
			else
				invNames += QString(", %1").arg(ii.key());
		}

		// Make a comma separated list of the industry names.
		QString indNames;
		for(QMap<QString, INDUSTRYID>::const_iterator ii = inds.cbegin(); ii != inds.cend(); ii++)
		{
			if(indNames.isEmpty())
				indNames = ii.key();
			else
				indNames += QString(", %1").arg(ii.key());
		}

		QTreeWidgetItem* pitem = new QTreeWidgetItem(this);
		pitem->setIcon(0, QIcon(":data/c_Patent.png"));
		pitem->setText(0, pi.key());
		pitem->setText(1, invNames);
		pitem->setText(2, indNames);

		ListDependencies(pitem, gd, *pi);

		// Test for cancel button on progress dialog.
		auApp->processEvents();
		if(prog->wasCanceled())
			break;
	}
	g_Au.UnlockData();
	delete prog;
}

void AuPatentsTree::ListDependencies(QTreeWidgetItem* parent, const DgData& gd, COMMODITYID patentCID)
{
	QMap<QString, COMMODITYID> deps;
	for(DbManufacturingProcessArray::const_iterator pi = gd.m_bManufacturingProcessArray.cbegin();
			pi != gd.m_bManufacturingProcessArray.cend(); pi++)
	{
		if((*pi).CommodityId() == patentCID)
		{
			// Found a process that makes the patent.
			// Does it require any patents?
			for(DbManufacturingComponentArray::const_iterator ci = gd.m_bManufacturingComponentArray.cbegin();
					ci != gd.m_bManufacturingComponentArray.cend(); ci++)
			{
				if((*ci).m_idmp == (*pi).m_id)
				{
					// Found a component of the process.
					if((*ci).IsRequired() && DbCommodity::IsPatent((*ci).CommodityId()))
						deps.insert(g_Au.CommodityName((*ci).CommodityId()), (*ci).CommodityId());
				}
			}
		}
	}

	if(deps.empty())
		return;

	for(QMap<QString, COMMODITYID>::const_iterator pi = deps.cbegin(); pi != deps.cend(); pi++)
	{
		QTreeWidgetItem* pitem = new QTreeWidgetItem(parent);
		pitem->setIcon(0, QIcon(":data/c_Patent.png"));
		pitem->setText(0, pi.key());
		ListDependencies(pitem, gd, *pi);
	}
	parent->setExpanded(true);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPatentsTreeDlg::AuPatentsTreeDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Patents Reference"));
	setAuDlgIcon(QIcon(":data/OrderRead.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"All patents are listed here. "
				"This is a comprehensive reference; it is not affected by patents known to your empire. "
				"</p>"
				"<p>"
				"Dependencies are shown for patents that require other patents before they can be developed. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	AuPatentsTree* ptree = new AuPatentsTree(this);
	ptree->ListPatents();

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(ptree);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuPatentsTreeDlg::~AuPatentsTreeDlg()
{
}

void AuPatentsTreeDlg::setVisible(bool visible)
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

void AuPatentsTreeDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PatentsTreeDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PatentsTreeDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PatentsTreeDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PatentsTreeDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuPatentsTreeDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PatentsTreeDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PatentsTreeDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PatentsTreeDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PatentsTreeDlg/SizeY",sz.height());
}

QSize AuPatentsTreeDlg::sizeHint() const
{
	return QSize(1000,600);
}

