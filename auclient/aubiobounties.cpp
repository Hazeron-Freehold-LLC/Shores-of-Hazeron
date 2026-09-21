//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiobounties.cpp,v 1.11 2022/04/08 16:22:59 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubiobounties.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

AuBioBounties::AuBioBounties(QWidget *parent) : QWidget(parent)
{
	m_AccountBalance = 0;
	m_timer = 0;
	m_InsuranceClaim = 0;

	lbbounties = new QLabel(this);
	lbbounties->setWordWrap(true);

	m_BountyModel.insertColumns(0,4);
	m_BountyModel.setHeaderData(0, Qt::Horizontal, "Amount");
	m_BountyModel.setHeaderData(1, Qt::Horizontal, "Reason");
	m_BountyModel.setHeaderData(2, Qt::Horizontal, "Posted By");
	m_BountyModel.setHeaderData(3, Qt::Horizontal, "When");
	m_BountyModel.setSortRole(Qt::UserRole);

	// Create the widgets.

	m_BountyTree = new QTreeView(this);
	m_BountyTree->setAlternatingRowColors(true);
	m_BountyTree->setAllColumnsShowFocus(true);
	m_BountyTree->setModel(&m_BountyModel);
	m_BountyTree->setRootIsDecorated(false);
	m_BountyTree->setSortingEnabled(true);
	m_BountyTree->sortByColumn(0, Qt::AscendingOrder);
	m_BountyTree->setToolTip(tr("<html><b>Bounties</b><br>"
				"List of bounties placed on your head.</html>"));

	m_PayInsurer = new QPushButton(tr("Pay Insurance Claims"), this);
	m_PayInsurer->setToolTip(tr("<html><b>Pay Insurance Claims</b><br>"
				"The %1 posts bounties to settle insurance claims. "
				"Insurance claims against you can be removed by buying them off. "
				"</html>").arg(InsurerName()));
	connect(m_PayInsurer, SIGNAL(clicked()), SLOT(PayInsurer()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lbbounties);
	blay->addWidget(m_PayInsurer);

	// Layout the widgets.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addWidget(m_BountyTree);
	setLayout(lay);
}

AuBioBounties::~AuBioBounties()
{
}

QSize AuBioBounties::sizeHint() const
{
	return QSize(400,300);
}

void AuBioBounties::showEvent(QShowEvent* event)
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

void AuBioBounties::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Get the scene atom so we can show their information.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(!person)
	{
		view->UnlockSceneAtom();
		return;
	}

	// Bounties are only listed if there are any.
	QString sbounties;
	m_AccountBalance = person->AccountBalance();
	double bounty = person->Bounty();
	if(bounty > 0)
	{
		m_InsuranceClaim = person->InsuranceBounty();
		// Conditions list contains bounties on my head.
		sbounties = tr("Total Bounties <b>%L1%2</b>")
			.arg(bounty, 0,'f',0)
			.arg(g_Au.CurrencySymbol());

		bool anyAdded = false;

		// Get a list of the titleBriefs that are already in the tree control, that is, the title and brief concatenated.
		QStringList bountiesInList;
		for(int ir=0; ir<m_BountyModel.rowCount(); ir++)
		{
			QModelIndex itemIdx = m_BountyModel.index(ir,0);
			QString bounty = itemIdx.data(Qt::UserRole+1).toString();
			bountiesInList.append(bounty);
		}
		bool wasEmpty = bountiesInList.isEmpty();

		// List titles the person has open that are not already in the tree.
		// titles that belong in the tree will be removed from the 'titleBriefs' list so we know which to remove when we're done.
		QMultiMap<AuTime, AuBountyOnItem> bounties;
		person->Bounties(bounties);
		for(QMultiMap<AuTime, AuBountyOnItem>::const_iterator bri = bounties.constBegin(); bri != bounties.constEnd(); bri++)
		{
			//qDebug("AuJobsDialog::TimerUpdate open title %s", (const char*)title.toAscii());
			QString inlist = QString("%1%2%3").arg((*bri).Amount()).arg((*bri).PosterId().StringId()).arg((*bri).Reason());
			int tbi = bountiesInList.indexOf(inlist);
			if(tbi >= 0)
			{
				// titles listed in the control already contains the open title.
				// remove it from the titleBriefs list so we know which titles don't belong in the control when we're done.
				bountiesInList.removeAt(tbi);
			}
			else
			{
				double amt = (*bri).Amount();
				QString amount = QString("%L1%2").arg(amt, 0,'f',0).arg(g_Au.CurrencySymbol());
				QString reason = (*bri).Reason();
				QString byWho = QString("%1 %2").arg((*bri).PosterName()).arg((*bri).PosterId().StringId());
				QString when = bri.key().UTC().toString();

				m_BountyModel.insertRow(0);

				QModelIndex itemIdx = m_BountyModel.index(0,0);
				m_BountyModel.setData(itemIdx, amount);
				m_BountyModel.setData(itemIdx, amt, Qt::UserRole);	// sort
				m_BountyModel.setData(itemIdx, inlist, Qt::UserRole+1);	// in list key

				itemIdx = m_BountyModel.index(0,1);
				m_BountyModel.setData(itemIdx, reason);
				m_BountyModel.setData(itemIdx, reason, Qt::UserRole);	// sort

				itemIdx = m_BountyModel.index(0,2);
				m_BountyModel.setData(itemIdx, byWho);
				m_BountyModel.setData(itemIdx, byWho, Qt::UserRole);	// sort

				itemIdx = m_BountyModel.index(0,3);
				m_BountyModel.setData(itemIdx, when);
				m_BountyModel.setData(itemIdx, bri.key().MSecsSinceEpoch(), Qt::UserRole);	// sort

				anyAdded = true;
			}
		}

		// Now any titles remaining in the 'bountiesInList' list must be removed from the tree control.
		for(QStringList::const_iterator ti = bountiesInList.constBegin(); ti != bountiesInList.constEnd(); ti++)
		{
			for(int ir=0; ir<m_BountyModel.rowCount(); ir++)
			{
				QModelIndex itemIdx = m_BountyModel.index(ir,0);
				QString bounty = itemIdx.data(Qt::UserRole+1).toString();
				if(bounty == *ti)
				{
					// Remove the row from the control's data model.
					m_BountyModel.removeRow(ir);
					break;
				}
			}
		}

		if(anyAdded && wasEmpty)
		{
			m_BountyTree->resizeColumnToContents(0);
			m_BountyTree->resizeColumnToContents(1);
			m_BountyTree->resizeColumnToContents(2);
			m_BountyTree->sortByColumn(0, Qt::AscendingOrder);
		}
	}
	else
		m_InsuranceClaim = 0;

	view->UnlockSceneAtom();

	lbbounties->setText(sbounties);
	m_PayInsurer->setEnabled(m_InsuranceClaim > 0);
}

void AuBioBounties::PayInsurer()
{
	double defAmount;
	if(m_AccountBalance >= m_InsuranceClaim)
		defAmount = m_InsuranceClaim;
	else
		defAmount = m_AccountBalance;
	QString def = QString("%1").arg(defAmount, 0,'f',0);

	bool ok;
	QString amount = QInputDialog::getText(this, g_Au.ApplicationName(),
			tr("<html>"
				"Enter an amount to pay toward insurance claims, currently %L1%2. "
				"All or a portion of the amount can be paid. "
				"You currently have %L3%2."
				"</html>")
			.arg(m_InsuranceClaim, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(m_AccountBalance, 0,'f',0),
			QLineEdit::Normal, def, &ok).trimmed();
	if(!ok)
		return;

	double amountToPay = amount.toDouble();
	if(amountToPay <= 0)
		return;
	if(amountToPay > m_AccountBalance)
		return;

	auApp->PayInsurer(amountToPay);	// amount is trimmed to exact insurance amount on server
}

