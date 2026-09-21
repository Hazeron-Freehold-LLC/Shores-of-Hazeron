//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovcommerce.cpp,v 1.5 2022/02/19 18:14:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovcommerce.h"
//
#include "auapp.h"
#include "aucommodityfilter.h"
#include "auframe.h"
#include "auview.h"
#include <auglobal.h>
#include <daperson.h>
#include <dgdata.h>
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>

void AuGovCommerceDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernCommerceUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovCommerce::AuGovCommerce(QWidget* parent) : QWidget(parent)
{
	setObjectName("CommercePage");

	m_Filter = FILTER_All;
	m_FilterWidget = new AuCommodityFilter;
	connect(m_FilterWidget, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(FilterChanged(COMMODITYFILTER)));

	m_Tree = new QTreeView;
	m_Tree->setAlternatingRowColors(true);
	m_Tree->setEditTriggers(QTreeView::NoEditTriggers);
	m_Tree->setModel(&m_Model);
	m_Tree->setRootIsDecorated(false);
	m_Tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_Tree->setToolTip(tr("Export policies for commodities, citizens and services."));

	pbnoex = new QPushButton(tr("Empire Only"));
	pbnoex->setToolTip(tr("<html>Do not export to any other empire, regardless of political stance. "
				"Continue trading with members of our empire.</html>"));
	connect(pbnoex, SIGNAL(clicked()), SLOT(DoNotExport()));

	pbany = new QPushButton(tr("Export to Anyone"));
	pbany->setToolTip(tr("<html>Export to all other empires, regardless of political stance.</html>"));
	connect(pbany, SIGNAL(clicked()), SLOT(ExportToAnyone()));

	pbfriend = new QPushButton(tr("Export Only to Friends"));
	pbfriend->setToolTip(tr("<html>Export to empires designated as friends.</html>"));
	connect(pbfriend, SIGNAL(clicked()), SLOT(ExportToFriends()));

	pbneutral = new QPushButton(tr("Export to Friends or Neutrals"));
	pbneutral->setToolTip(tr("<html>Export to empires designated as friend or neutral.</html>"));
	connect(pbneutral, SIGNAL(clicked()), SLOT(ExportToNeutrals()));

	pbapply = new QPushButton(tr("Apply Commerce Changes"));
	pbapply->setToolTip(tr("<html>Commit changes to the export policies.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change trespass policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbapply, SIGNAL(clicked()), SLOT(ApplyChanges()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_FilterWidget, row,0, 1,4);
	row++;
	lay->addWidget(m_Tree, row,0, 1,4);
	row++;
	lay->addWidget(pbnoex, row,0);
	lay->addWidget(pbfriend, row,1);
	lay->addWidget(pbneutral, row,2);
	lay->addWidget(pbany, row,3);
	row++;
	lay->addWidget(pbapply, row,0, 1,4);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	connect(this, SIGNAL(CommerceChanged(QByteArray)), auApp, SLOT(GovSetCommerce(QByteArray)));
}

void AuGovCommerce::ApplyChanges()
{
	// Determine empire.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	unsigned int empire = 0;
	if(person)
		empire = person->EmpireId();
	pView->UnlockSceneAtom();
	if(empire == 0)
		return;

	// Get current commerce values for all commodities.
	QList<quint8> commerce;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.constBegin(); it != g_Au.m_CommodityArray.constEnd(); it++)
	{
		if((*it).IsObsolete())
			continue;	// Skip obsolete commodities.
		if((*it).m_id != CID_Sunlight &&						// spacecraft repairs
				(*it).m_id != CID_SupercooledLiquid &&			// spacecraft sales i.e. private
				(*it).m_id != CID_InfernoAtmosphere &&			// spacecraft requisition i.e. subsidized
				(*it).IsEnvironmentalFactor())
			continue;	// skip environmental factors
		EXPORT_POLICY com = gd.EmpireCommerce(empire, (*it).m_id);

		// Be sure our array is big enough by filling up to the cid index with default values.
		while(commerce.count() <= (*it).m_id)
			commerce.append(EXPOL_ToFriend);
		commerce[(*it).m_id] = com;
	}
	g_Au.UnlockData();

	// Get values from user interface, which only lists the items in the current filter.
	QModelIndex rootIdx;
	int rows = m_Model.rowCount(rootIdx);
	for(int row=0; row<rows; row++)
	{
		QModelIndex itemIdx0 = m_Model.index(row,0,rootIdx);
		COMMODITYID cid = (COMMODITYID)m_Model.data(itemIdx0, Qt::UserRole).toInt();
		QModelIndex itemIdx1 = m_Model.index(row,1,rootIdx);
		EXPORT_POLICY com = (EXPORT_POLICY)m_Model.data(itemIdx1, Qt::UserRole).toInt();
		Q_ASSERT(commerce.count() > cid);
		commerce[cid] = com;
	}

	// Put the Vector into a byte array because we can't queue it as a signal/slot parameter.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint32)commerce.count();
	for(QList<quint8>::const_iterator comIndex = commerce.constBegin(); comIndex != commerce.constEnd(); comIndex++)
		out << (quint8)(*comIndex);

	emit CommerceChanged(ba);
}

void AuGovCommerce::ListCommerce()
{
	m_Model.clear();

	// Create a root row in the model.
	QModelIndex rootIdx;
	QModelIndex itemIdx = m_Model.index(0,0,rootIdx);
	m_Model.insertColumns(0,3,itemIdx);
	m_Model.setHeaderData(0, Qt::Horizontal, tr("Commodity"));
	m_Model.setHeaderData(1, Qt::Horizontal, tr("Export Policy"));
	m_Model.setHeaderData(2, Qt::Horizontal, tr("Notes"));

	// Determine empire.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	unsigned int empire = 0;
	if(person)
		empire = person->EmpireId();
	pView->UnlockSceneAtom();
	if(empire == 0)
		return;

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_Filter);
	if(m_Filter == FILTER_All)
	{
		cidFilter.append(CID_Invalid);		// Used for export controls on citizens.
		cidFilter.append(CID_Sunlight);		// Used for export controls on repairs.
		cidFilter.append(CID_SupercooledLiquid);	// Used for export controls on private spacecraft.
		cidFilter.append(CID_InfernoAtmosphere);	// Used for export controls on subsidized spacecraft.
	}
	else if(m_Filter == FILTER_SpareParts)
		cidFilter.append(CID_Sunlight);		// Used for export controls on repairs.
	else if(m_Filter == FILTER_Misc)
	{
		cidFilter.append(CID_Invalid);		// Used for export controls on citizens.
		cidFilter.append(CID_SupercooledLiquid);	// Used for export controls on private spacecraft.
		cidFilter.append(CID_InfernoAtmosphere);	// Used for export controls on subsidized spacecraft.
	}

	// List all commodities and their commerce values.
	const DgData& gd = g_Au.LockDataForRead();
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.constBegin(); it != g_Au.m_CommodityArray.constEnd(); it++)
	{
		if((*it).IsObsolete())
			continue;	// Skip obsolete commodities.
		if((*it).m_id != CID_Sunlight &&						// spacecraft repairs
				(*it).m_id != CID_SupercooledLiquid &&			// spacecraft sales i.e. private
				(*it).m_id != CID_InfernoAtmosphere &&			// spacecraft requisition i.e. subsidized
				(*it).IsEnvironmentalFactor())
			continue;	// skip environmental factors
		if(DbCommodity::IsPatent((*it).m_id))
			continue;	// skip patents because they aren't manufactured in quantities.

		// Apply category filter to list.
		if(!cidFilter.contains((*it).m_id))
			continue;	// Skip item that is not included in the current filter.

		// Create a row in the model.
		QString notes;
		m_Model.insertRow(0, rootIdx);
		itemIdx = m_Model.index(0,0,rootIdx);
		QIcon icon;
		if((*it).m_id == CID_Invalid)
		{
			// The FIRST CID_Invalid entry is used to control passengers.
			if(it == g_Au.m_CommodityArray.constBegin())
			{
				m_Model.setData(itemIdx, tr("Citizens"));
				icon = QIcon(":data/CommCrew.png");
				notes = tr("Restricts boarding of crew and passengers.");
			}
			else
				continue;	// Subsequent CID_Invalid entries must be ignored.
		}
		else if((*it).m_id == CID_Sunlight)
		{
			// The CID_Sunlight entry is used to control repairs.
			m_Model.setData(itemIdx, tr("Spacecraft Repairs"));
			icon = QIcon(":data/MsgDamageReport.png");
			notes = tr("Restricts spacecraft repair services.");
		}
		else if((*it).m_id == CID_SupercooledLiquid)
		{
			// The CID_SupercooledLiquid entry is used to control private spacecraft.
			m_Model.setData(itemIdx, tr("Spacecraft Sales"));
			icon = QIcon(":data/c_Spacecraft.png");
			notes = tr("Restricts sales of company spacecraft.");
		}
		else if((*it).m_id == CID_InfernoAtmosphere)
		{
			// The CID_InfernoAtmosphere entry is used to control subsidized spacecraft.
			m_Model.setData(itemIdx, tr("Spacecraft Requisition"));
			icon = QIcon(":data/c_Spacecraft.png");
			notes = tr("Restricts captaincy of fleet spacecraft. Exception, stations are always Empire Only.");
		}
		else
		{
			m_Model.setData(itemIdx, (*it).m_name);
			icon = QIcon(g_Au.CommodityBitmapName((*it).m_id));
		}
		m_Model.setData(itemIdx, icon, Qt::DecorationRole);
		m_Model.setData(itemIdx, (*it).m_id, Qt::UserRole);

		itemIdx = m_Model.index(0,1,rootIdx);
		EXPORT_POLICY com = gd.EmpireCommerce(empire, (*it).m_id);
		m_Model.setData(itemIdx, com, Qt::UserRole);
		switch(com)
		{
		case EXPOL_ToNobody:
			m_Model.setData(itemIdx, tr("Empire Only"));
			break;
		case EXPOL_ToEnemy:
			m_Model.setData(itemIdx, tr("Export to Anyone"));
			break;
		case EXPOL_ToFriend:
			m_Model.setData(itemIdx, tr("Export Only to Friends"));
			break;
		case EXPOL_ToNeutral:
			m_Model.setData(itemIdx, tr("Export to Friends or Neutrals"));
			break;
		}

		if(!notes.isEmpty())
		{
			itemIdx = m_Model.index(0,2,rootIdx);
			m_Model.setData(itemIdx, notes);
		}
	}
	g_Au.UnlockData();

	m_Tree->resizeColumnToContents(0);
	m_Tree->resizeColumnToContents(1);
	m_Model.sort(0);
}

void AuGovCommerce::DoNotExport()
{
	SetSelectedItems(EXPOL_ToNobody);
}

void AuGovCommerce::ExportToAnyone()
{
	SetSelectedItems(EXPOL_ToEnemy);
}

void AuGovCommerce::ExportToFriends()
{
	SetSelectedItems(EXPOL_ToFriend);
}

void AuGovCommerce::ExportToNeutrals()
{
	SetSelectedItems(EXPOL_ToNeutral);
}

void AuGovCommerce::SetSelectedItems(EXPORT_POLICY com)
{
	QItemSelectionModel* ism = m_Tree->selectionModel();
	if(ism)
	{
		QModelIndex rootIdx;
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator it = sel.constBegin(); it != sel.constEnd(); it++)
		{
			QModelIndex itemIdx = m_Model.index((*it).row(),1,rootIdx);
			m_Model.setData(itemIdx, com, Qt::UserRole);
			switch(com)
			{
			case EXPOL_ToNobody:
				m_Model.setData(itemIdx, tr("Empire Only"));
				break;
			case EXPOL_ToEnemy:
				m_Model.setData(itemIdx, tr("Export to Anyone"));
				break;
			case EXPOL_ToFriend:
				m_Model.setData(itemIdx, tr("Export Only to Friends"));
				break;
			case EXPOL_ToNeutral:
				m_Model.setData(itemIdx, tr("Export to Friends or Neutrals"));
				break;
			}
		}
	}
}

void AuGovCommerce::FilterChanged(COMMODITYFILTER filter)
{
	m_Filter = filter;
	ListCommerce();
}

void AuGovCommerce::UpdateEmpireRecord()
{
	// If this widget is not visible, just clear the data.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovCommerce::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_Model.clear();
	}
	else
	{
		//qDebug("AuGovCommerce::UpdateEmpireRecord page is visible. Update contents.");
		ListCommerce();

		AuView* pView = auApp->Frame()->View();
		AuId personId = pView->SceneAtomId();
		quint32 personEmpireId = pView->SceneAtomEmpire();
		const DgData& gd = g_Au.LockDataForRead();
		bool enablePolicies = gd.EmpireCitizenCanSetCommerce(personEmpireId, personId);
		g_Au.UnlockData();

		// Enable/disable controls appropriately.
		pbany->setEnabled(enablePolicies);
		pbapply->setEnabled(enablePolicies);
		pbfriend->setEnabled(enablePolicies);
		pbneutral->setEnabled(enablePolicies);
		pbnoex->setEnabled(enablePolicies);
	}
}

void AuGovCommerce::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	UpdateEmpireRecord();
}
