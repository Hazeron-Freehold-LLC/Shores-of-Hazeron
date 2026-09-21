//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovchooseboss.cpp,v 1.3 2022/02/19 18:10:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovchooseboss.h"
//
#include "auglobal.h"
#include "auoffice.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QTreeWidget>
#include <QPushButton>

AuGovChooseBoss::AuGovChooseBoss(const DgEmpire& empire, const AuId& governorId, QWidget* parent) : AuDialog(parent),
	m_Empire(empire)
{
	setAuDlgTitle(tr("Choose Boss"));
	setAuDlgIcon(QIcon(":data/CommGovernment.png"));

	m_GovernTree = new QTreeWidget(this);
	m_GovernTree->setColumnCount(2);
	QStringList citizensHdr;
	citizensHdr << tr("Government");
	citizensHdr << tr("Id");
	m_GovernTree->setHeaderLabels(citizensHdr);
	connect(m_GovernTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(SelectionChanged()));
	connect(m_GovernTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(Accept()));

	m_OkButton = new QPushButton(tr("Ok"), this);
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	m_OkButton->setToolTip(tr("<html>Accepts the chosen boss.</html>"));
	connect(m_OkButton, SIGNAL(clicked()), SLOT(Accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_GovernTree, row,0, 1,2);
	row++;
	lay->addWidget(m_OkButton, row,1);
	lay->setColumnStretch(0, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	FillGovernTree(m_GovernTree, governorId);
}

AuGovChooseBoss::~AuGovChooseBoss()
{
}

void AuGovChooseBoss::setVisible(bool visible)
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

void AuGovChooseBoss::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GovChooseBoss/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GovChooseBoss/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GovChooseBoss/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GovChooseBoss/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGovChooseBoss::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GovChooseBoss/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GovChooseBoss/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GovChooseBoss/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GovChooseBoss/SizeY",sz.height());
}

void AuGovChooseBoss::SelectionChanged()
{
	m_OkButton->setEnabled(m_GovernTree->currentItem());
}

void AuGovChooseBoss::Accept()
{
	const QTreeWidgetItem* citizen = m_GovernTree->currentItem();
	if(!citizen)
		return;

	m_Boss = AuId(citizen->data(0, Qt::UserRole).toUInt());

	accept();
}

void AuGovChooseBoss::FillGovernTree(QTreeWidget* tree, const AuId& governorId)
{
	// Add items for the government.
	AuId emperorId(m_Empire.EmperorId());
	QSet<AuId> alreadyListed;
	if(!emperorId.IsNull())
	{
		// Prevent adding the governor and any of his subordinates.
		alreadyListed.insert(governorId);

		QTreeWidgetItem* emperor = new QTreeWidgetItem(tree);
		emperor->setExpanded(true);
		emperor->setIcon(0, QIcon(":data/CommGovernment.png"));
		emperor->setText(0, m_Empire.CitizenNameWithTitle(emperorId));
		emperor->setData(0, Qt::UserRole, emperorId.GetID());
		emperor->setText(1, emperorId.StringId());
		alreadyListed.insert(emperorId);

		AddSubordinates(emperor, alreadyListed);

		// List governors with broken trees. They are the ones who are left.
		QMultiMap<QString, AuId> govStragglers;
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Empire.m_Citizens.cbegin();
				ci != m_Empire.m_Citizens.cend(); ci++)
			if(!alreadyListed.contains((*ci).first) && (*ci).second.HasOffice())
				govStragglers.insert((*ci).second.NameWithTitle(), (*ci).first);
		for(QMultiMap<QString, AuId>::const_iterator si = govStragglers.cbegin(); si != govStragglers.cend(); si++)
		{
			QTreeWidgetItem* citizen = new QTreeWidgetItem(emperor);
			citizen->setIcon(0, QIcon(":data/CommGovernment.png"));
			citizen->setText(0, si.key());
			citizen->setData(0, Qt::UserRole, (*si).GetID());
			citizen->setText(1, (*si).StringId());
		}
	}
}

// Recursive
void AuGovChooseBoss::AddSubordinates(QTreeWidgetItem* boss, QSet<AuId>& alreadyListed)
{
	// Get the list of names and ids of all citizens who are subordinate to the boss.
	AuId bossId = AuId(boss->data(0, Qt::UserRole).toUInt());
	QMultiMap<QString, AuId> subs;
	m_Empire.Subordinates(bossId, subs);

	// Create a tree entry under boss for each citizen.
	for(QMultiMap<QString, AuId>::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
	{
		if(alreadyListed.contains(*si))
			continue;

		QTreeWidgetItem* sub = new QTreeWidgetItem(boss);
		sub->setExpanded(true);
		sub->setIcon(0, QIcon(":data/CommGovernment.png"));
		sub->setText(0, si.key());
		sub->setData(0, Qt::UserRole, (*si).GetID());
		alreadyListed.insert(*si);

		// Add the citizen's subordinates, if any.
		AddSubordinates(sub, alreadyListed);
	}
}
