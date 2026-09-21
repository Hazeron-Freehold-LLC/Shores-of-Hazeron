//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupatentsrecord.cpp,v 1.4 2022/02/19 18:16:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupatentsrecord.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

AuPatentsRecord::AuPatentsRecord(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Record Patents"));
	setAuDlgIcon(QIcon(":data/c_Patent.png"));

	m_PatentList = new QListWidget(this);
	m_PatentList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_PatentList->setSpacing(2);
	connect(m_PatentList, SIGNAL(itemSelectionChanged()), SLOT(PatentSelectionChanged()));
	ListPatents();

	m_CountLabel = new QLabel(tr("No patents selected."), this);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Records the selected patents."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_PatentList);
	lay->addWidget(m_CountLabel);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuPatentsRecord::~AuPatentsRecord()
{
}

void AuPatentsRecord::setVisible(bool visible)
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

void AuPatentsRecord::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/RecordPatents/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/RecordPatents/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/RecordPatents/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/RecordPatents/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuPatentsRecord::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/RecordPatents/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/RecordPatents/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/RecordPatents/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/RecordPatents/SizeY",sz.height());
}

void AuPatentsRecord::Patents(AuKnownPatents& patents) const
{
	QItemSelectionModel* sel = m_PatentList->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		for(QModelIndexList::const_iterator si = selRowIdxs.cbegin(); si != selRowIdxs.cend(); si++)
		{
			COMMODITYID patentCID  = (COMMODITYID)(*si).data(Qt::UserRole).toInt();
			quint8 patentQ  = (*si).data(Qt::UserRole+1).toUInt();
			patents.RecordPatent(patentCID, patentQ);
		}
	}
}

void AuPatentsRecord::ListPatents()
{
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;
	AuKnownPatents patents;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		person->ListPatents(patents);
	pView->UnlockSceneAtom();

	m_PatentList->setSortingEnabled(false);
	for(QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin(); pi != patents.cend(); pi++)
	{
		// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
		// Lists items in top-level containers.
		// Does not list containers that are not empty.
		if(!DbCommodity::IsPatent(pi.key()))
			continue;
		QString name = QString("%1 Q%2").arg(g_Au.CommodityName(pi.key())).arg(*pi);
		QListWidgetItem* lwi = new QListWidgetItem(QIcon(g_Au.CommodityBitmapName(pi.key())), name, m_PatentList);
		lwi->setData(Qt::UserRole, pi.key());
		lwi->setData(Qt::UserRole+1, *pi);
	}
	m_PatentList->setSortingEnabled(true);
	m_PatentList->sortItems();
}

void AuPatentsRecord::PatentSelectionChanged()
{
	int countSel = 0;
	QItemSelectionModel* sel = m_PatentList->selectionModel();
	if(sel)
		countSel = sel->selectedIndexes().count();
	else
		countSel = 0;
	if(countSel == 0)
		m_CountLabel->setText(tr("No patents selected."));
	else if(countSel == 1)
		m_CountLabel->setText(tr("One patent selected."));
	else
		m_CountLabel->setText(tr("%1 patents selected.").arg(countSel));
}
