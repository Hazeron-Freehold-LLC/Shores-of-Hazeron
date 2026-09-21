//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurequestupgrade.cpp,v 1.5 2022/02/19 18:16:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurequestupgrade.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "auframe.h"
#include "auservicedialog.h"
#include "auview.h"
#include "daperson.h"
#include "dasc.h"
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVariant>

AuRequestUpgrade::AuRequestUpgrade(AuServiceDialog* parent) : QWidget(parent)
{
	// Init default return values.
	m_System = SYS_Count;
	m_SystemDesc = "";
	m_SystemWhich = 0;

	QLabel* lbmod = new QLabel(tr("<html><p>Select a system module to replace. "
				"</p>"
				"<p>"
				"The spacecraft must be rigged for repairs: stopped, sensors off, shields off, power plant off, "
				"maneuver drive off and FTL drive off. "
				"</p>"
				"</html>"), this);
	lbmod->setWordWrap(true);

	// Place controls for listing spacecraft systems.
	QListWidget* lwd = new QListWidget(this);
	lwd->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwd->setObjectName("lwd");
	lwd->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwd->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(lwd, SIGNAL(itemSelectionChanged()), SLOT(SelectionChanged()));
	connect(lwd, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(ItemDoubleClicked(QListWidgetItem*)));

	QPushButton* pbok = new QPushButton(tr("Request Module Upgrade"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setEnabled(false);
	pbok->setObjectName("pbok");
	pbok->setToolTip(tr("<html>Request upgrade modules for the selected system. "
				"A city will respond with modules that are available for installation to the selected system."
				"You can choose which module upgrades to install. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(lbmod);
	lay->addWidget(lwd);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	UpdateContents();
}

// Update contents of dialog that might change while the dialog is up.
void AuRequestUpgrade::UpdateContents()
{
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;

	// Get list controls and selected item info for reselect after update.
	QListWidget* lwd = findChild<QListWidget*>("lwd");
	if(!lwd)
		return;

	// Lock the root and find the scene atom.
	QString shipname;
	QList<AuSystemInfo> systems;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			shipname = ship->NameWithEmpire();

			// List systems, current technologies, and types.
			// For upgrade purposes, we do not split out the CommandAndControl system into 'which' console.
			systems.append(AuSystemInfo(ScSystemName(SYS_CommandAndControl),
						SYS_CommandAndControl, 0,
						ship->SystemQuality(SYS_CommandAndControl),
						ship->SystemModule(SYS_CommandAndControl)));
			systems.append(AuSystemInfo(ScSystemName(SYS_Hull),
						SYS_Hull, 0,
						ship->SystemQuality(SYS_Hull),
						ship->SystemModule(SYS_Hull)));
			systems.append(AuSystemInfo(ScSystemName(SYS_PowerPlant),
						SYS_PowerPlant, 0,
						ship->SystemQuality(SYS_PowerPlant),
						ship->SystemModule(SYS_PowerPlant)));
			// Old style ships do not have a capacitor module.
			if(dynamic_cast<const DaSc*>(ship))
				systems.append(AuSystemInfo(ScSystemName(SYS_Capacitor),
							SYS_Capacitor, 0,
							ship->SystemQuality(SYS_Capacitor),
							ship->SystemModule(SYS_Capacitor)));

			// Now get info about each system.
			ship->ListSystems(systems);
		}
	}
	pView->UnlockSceneAtom();

	if(!systems.isEmpty())
	{
		QModelIndex curIdx = lwd->currentIndex();
		lwd->clear();

		// Insert an item for each system that can be upgraded, except individual consoles. Their system is upgraded as a whole.
		for(QList<AuSystemInfo>::const_iterator si = systems.constBegin(); si != systems.constEnd(); si++)
		{
			QString listDesc = tr("%1 %3 Q%4")
				.arg((*si).m_Desc)
				.arg(DbCommodity::QualityDesc((*si).m_Quality))
				.arg((*si).m_Quality);
			QPixmap pix = QPixmap((*si).SystemIconName()).scaled(24,24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			QListWidgetItem* lwi = new QListWidgetItem(QIcon(pix), listDesc, lwd);
			QVariant var;
			var.setValue(*si);
			lwi->setData(Qt::UserRole, var);
		}

		lwd->scrollTo(curIdx);
		lwd->setCurrentIndex(curIdx);
	}
	else
	{
		lwd->clear();
	}
}

void AuRequestUpgrade::SelectionChanged()
{
	QListWidget* lwd = findChild<QListWidget*>("lwd");
	if(!lwd)
		return;
	QPushButton* pbok = findChild<QPushButton*>("pbok");
	if(!pbok)
		return;

	QItemSelectionModel* sel = lwd->selectionModel();
	if(sel && sel->hasSelection())
	{
		// An item is selected.
		pbok->setEnabled(true);
	}
	else
	{
		// No items are selected.
		pbok->setEnabled(false);
	}
}

void AuRequestUpgrade::OkPressed()
{
	QListWidget* lwd = findChild<QListWidget*>("lwd");
	if(!lwd)
		return;
	QList<QListWidgetItem*> sel = lwd->selectedItems();
	if(sel.isEmpty() || sel.count() != 1)
		return;	// SelectionChanged already guaranteed this wouldn't happen.

	AuSystemInfo system = sel[0]->data(Qt::UserRole).value<AuSystemInfo>();
	m_System = system.m_System;
	m_SystemDesc = system.m_Desc;
	m_SystemWhich = system.m_Which;
	RequestUpgrade();
}

void AuRequestUpgrade::ItemDoubleClicked(QListWidgetItem* item)
{
	AuSystemInfo system = item->data(Qt::UserRole).value<AuSystemInfo>();
	m_System = system.m_System;
	m_SystemDesc = system.m_Desc;
	m_SystemWhich = system.m_Which;
	RequestUpgrade();
}

void AuRequestUpgrade::RequestUpgrade()
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	const AuView* view = auApp->Frame()->View();
	if(!view)
		return;

	AuCommRequestUpgradeSystem msg(SystemId(), SystemWhich(), SystemDesc(),
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);

	// Close the parent dialog.
	svcdlg->accept();
}
