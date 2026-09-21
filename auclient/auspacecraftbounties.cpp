//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftbounties.cpp,v 1.7 2022/05/04 21:48:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftbounties.h"
//
#include "auchristendialog.h"
#include "auapp.h"
#include <auglobal.h>
#include "auframe.h"
#include <auroster.h>
#include "auview.h"
#include <dagalaxy.h>
#include <daperson.h>
#include <dasector.h>
#include <dgdata.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTimer>

void AuSpacecraftBountiesDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftBountiesUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftBounties::AuSpacecraftBounties(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	QLabel* lbbounties = new QLabel(tr("Bounties For Destruction"));

	twbounties = new QTableWidget(0,2);
	{
		twbounties->setEditTriggers(QAbstractItemView::NoEditTriggers);
		twbounties->setSelectionBehavior(QAbstractItemView::SelectRows);
		twbounties->setSelectionMode(QAbstractItemView::NoSelection);
		twbounties->setShowGrid(false);
		twbounties->setColumnCount(2);
		twbounties->setColumnWidth(0,100);
		twbounties->setSortingEnabled(true);
		twbounties->sortByColumn(0, Qt::AscendingOrder);
		twbounties->setToolTip(tr("List of bounties posted for destruction of the ship."));

		QHeaderView* hhdr = twbounties->horizontalHeader();
		hhdr->setSectionResizeMode(0, QHeaderView::Interactive);
		hhdr->setSectionResizeMode(1, QHeaderView::Stretch);
		QStringList headerLabels;
		headerLabels << "Bounty";
		headerLabels << "Reason";
		twbounties->setHorizontalHeaderLabels(headerLabels);

		QHeaderView* vhdr = twbounties->verticalHeader();
		vhdr->setDefaultSectionSize(18);
		vhdr->hide();
	}

	// Final main grid.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbbounties);
	lay->addWidget(twbounties);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSpacecraftBounties::~AuSpacecraftBounties()
{
}

void AuSpacecraftBounties::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuSpacecraftBounties::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Create list view entries from spaceship atom data.
	// We must use the scene atom because the root ship atom never does the
	// extra work of analyzing its design.
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// List items that relate to the ship the person is aboard.
		const DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Bounties
			{
				// Make a list of the bounties already in the table for reconciling removed bounties at the end.
				// When the update is done, the 'intable' list will contain only bounties that weren't updated; those get removed.
				QStringList intable;
				for(int row=0; row<twbounties->rowCount(); row++)
				{
					QTableWidgetItem* twreward = twbounties->item(row, 0);
					QTableWidgetItem* twreason = twbounties->item(row, 1);
					QString key(twreward->text() + ":" + twreason->text());
					intable.append(key);
				}

				// List bounties.
				twbounties->setSortingEnabled(false);
				QTableWidgetItem* twreward = 0;
				QTableWidgetItem* twreason = 0;
				const AuConditionList& conditions = ship->LockConditions();
				for(AuConditionList::const_iterator it = conditions.constBegin(); it != conditions.constEnd(); it++)
				{
					const AuConBounty* conBounty = dynamic_cast<const AuConBounty*>(*it);
					if(!conBounty)
						continue;

					QString reward = QString("%L1%2").arg(conBounty->Reward(), 0,'f',0).arg(g_Au.CurrencySymbol());
					QString key(reward + ":" + conBounty->Reason());

					if(intable.contains(key))
					{
						// Remove the id from the 'intable' list. We'll remove the ones that don't get updated at the end.
						intable.removeAll(key);

						// No update of the list item is required.
						continue;
					}

					twreward = new QTableWidgetItem(QIcon(":data/JobBountyHunter.png"), reward);
					twreason = new QTableWidgetItem(conBounty->Reason());

					// Store key on column 0.
					twreward->setData(Qt::UserRole, key);

					int row = twbounties->rowCount();
					twbounties->insertRow(row);

					int col=0;
					twbounties->setItem(row,col++, twreward);
					twbounties->setItem(row,col++, twreason);
				}
				ship->UnlockConditions();

				// Remove rows that no longer belong in the table.
				for(QStringList::const_iterator ii = intable.constBegin(); ii != intable.constEnd(); ii++)
				{
					for(int row=0; row<twbounties->rowCount(); row++)
					{
						QTableWidgetItem* item = twbounties->item(row,0);
						if(item->data(Qt::UserRole).toString() == *ii)
						{
							// Found the row in the table. Remove it.
							twbounties->removeRow(row);
							break;
						}
					}
				}
				twbounties->resizeRowsToContents();
				twbounties->setSortingEnabled(true);
			}
		}
		else
		{
			// Not aboard a spacecraft.
			twbounties->clearContents();
		}
	}
	pView->UnlockSceneAtom();
}

QSize AuSpacecraftBounties::sizeHint() const
{
	return QSize(500,200);
}
