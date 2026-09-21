//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auassignberth.cpp,v 1.2 2022/02/19 18:04:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auassignberth.h"
//
#include <auglobal.h>
#include <auroster.h>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

AuAssignBerth::AuAssignBerth(bool captainOk, bool officerOk, bool crewOk,
		bool passengerOk, bool troopOk, bool anyOk, QWidget* parent) :
	AuDialog(parent)
{
	// I know that anyOk is only true when requesting a berth.
	QLabel* lb;
	if(anyOk)
	{
		setAuDlgTitle(tr("Request Berth"));
		lb = new QLabel(tr("Select the type of berth to request."));
	}
	else
	{
		setAuDlgTitle(tr("Assign Berth"));
		lb = new QLabel(tr("Select the type of berth to assign."));
	}

	QListWidget* lwtypes = new QListWidget(this);
	lwtypes->setObjectName("lwtypes");
	lwtypes->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwtypes->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwtypes->setSelectionMode(QAbstractItemView::SingleSelection);
	lwtypes->setSortingEnabled(false);
	connect(lwtypes, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));
	connect(lwtypes, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(TypeDoubleClicked(QListWidgetItem*)));

	if(anyOk)
	{
		QIcon icon(":data/RequestBerth.png");
		QString text = tr("Any Berth");
		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwtypes);
		// NOTE: Invalid will denote the 'any berth' option.
		lwi->setData(Qt::UserRole, AuRosterEntry::Invalid);
	}
	if(captainOk)
	{
		QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::Captain));
		QString text = tr("Captain's Berth");
		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwtypes);
		lwi->setData(Qt::UserRole, AuRosterEntry::Captain);
	}
	if(officerOk)
	{
		QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::OfficerHuman));
		QString text = tr("Officer Berth");
		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwtypes);
		lwi->setData(Qt::UserRole, AuRosterEntry::OfficerHuman);
	}
	if(crewOk)
	{
		QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::Crew));
		QString text = tr("Crew Berth");
		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwtypes);
		lwi->setData(Qt::UserRole, AuRosterEntry::Crew);
	}
	if(passengerOk)
	{
		QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::PassengerHuman));
		QString text = tr("Passenger Berth");
		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwtypes);
		lwi->setData(Qt::UserRole, AuRosterEntry::PassengerHuman);
	}
	if(troopOk)
	{
		QIcon icon(AuRosterEntry::BerthIcon(AuRosterEntry::Troop));
		QString text = tr("Troop Berth");
		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwtypes);
		lwi->setData(Qt::UserRole, AuRosterEntry::Troop);
	}

	QPushButton* pbok;
	if(anyOk)
	{
		pbok = new QPushButton(tr("Request Berth"));
		pbok->setToolTip(tr("Request the type of berth that is selected."));
	}
	else
	{
		pbok = new QPushButton(tr("Assign Berth"));
		pbok->setToolTip(tr("Assign the type of berth that is selected."));
	}
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setObjectName("pbok");
	connect(pbok, SIGNAL(clicked()), SLOT(AssignBerth()));

	QGridLayout* lay = new QGridLayout();
	int row = 0;	// Using this makes inserting and rearranging rows easier.
	lay->addWidget(lb, row, 0);
	row++;
	lay->addWidget(lwtypes, row, 0);
	row++;
	lay->addWidget(pbok, row, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuAssignBerth::~AuAssignBerth()
{
}

void AuAssignBerth::EnableControls()
{
	bool enableok = false;
	QListWidget* lwtypes = findChild<QListWidget*>("lwtypes");
	if(lwtypes)
	{
		QItemSelectionModel* sel = lwtypes->selectionModel();
		if(sel && sel->hasSelection())
			enableok = true;
	}

	QPushButton* pbok = findChild<QPushButton*>("pbok");
	if(pbok)
		pbok->setEnabled(enableok);
}

void AuAssignBerth::AssignBerth()
{
	QListWidget* lwtypes = findChild<QListWidget*>("lwtypes");
	if(lwtypes)
	{
		// Get type so it can be retrieved after the widget is gone.
		QItemSelectionModel* sel = lwtypes->selectionModel();
		if(sel && sel->hasSelection())
		{
			QModelIndexList selRowIdxs = sel->selectedRows();
			Q_ASSERT(selRowIdxs.count());
			QModelIndex selIdx = selRowIdxs.first();
			m_TypeAssigned =
				(AuRosterEntry::TYPE)selIdx.data(Qt::UserRole).toInt();
			accept();
		}
	}
}

void AuAssignBerth::TypeDoubleClicked(QListWidgetItem* /*item*/)
{
	AssignBerth();
}
