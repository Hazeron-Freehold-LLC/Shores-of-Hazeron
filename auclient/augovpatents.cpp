//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovpatents.cpp,v 1.8 2022/05/04 21:27:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovpatents.h"
//
#include "auapp.h"
#include "aucommodityfilter.h"
#include "auglobal.h"
#include "auframe.h"
#include "aupatentsget.h"
#include "aupatentsrecord.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QToolButton>

void AuGovPatentsDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernCommerceUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovPatents::AuGovPatents(QWidget* parent) : QWidget(parent)
{
	m_PatentsGet = new QToolButton(this);
	m_PatentsGet->setIcon(QIcon(":data/PatentGet.png"));
	m_PatentsGet->setToolTip(tr("<html><b>Get Patent</b><br>"
				"Requests a copy of one or more patents. "
				"You must possess blank media of some kind. "
				"<br>"
				"<b>You must be in the chain of command of the city to get patents.</b>"
				"<br>"
				"<b>You cannot get patents from a city while it is occupied or under attack.</b>"
				"</html>"));
	connect(m_PatentsGet, SIGNAL(clicked()), SLOT(GetPatent()));

	m_PatentsRecord = new QToolButton(this);
	m_PatentsRecord->setIcon(QIcon(":data/PatentRecord.png"));
	m_PatentsRecord->setToolTip(tr("<html><b>Record Patent</b><br>"
				"Makes a record of a patent in the city. "
				"You must possess one or more patents. "
				"Patents can be recorded at any industry, even if the patent is not used by the industry. "
				"Patents are available to all industries in the city. "
				"<br>"
				"<b>When a new patent is developed, it is automatically recorded in the city.</b>"
				"<br>"
				"<b>You cannot record patents in a city while it is occupied or under attack.</b>"
				"</html>"));
	connect(m_PatentsRecord, SIGNAL(clicked()), SLOT(RecordPatent()));

	m_PatentList = new QListWidget(this);
	m_PatentList->setSelectionMode(QAbstractItemView::NoSelection);
	m_PatentList->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_PatentsGet);
	blay->addWidget(m_PatentsRecord);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addWidget(m_PatentList);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

void AuGovPatents::ListPatents()
{
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;

	AuKnownPatents patents;
	quint32 empire = pView->SceneAtomEmpire();
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpirePatents(empire, patents);
	g_Au.UnlockData();

	m_PatentList->setSortingEnabled(false);
	m_PatentList->clear();
	for(QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin(); pi != patents.cend(); pi++)
	{
		QString name = QString("%1 Q%2").arg(g_Au.CommodityName(pi.key())).arg(*pi);
		QListWidgetItem* lwi = new QListWidgetItem(QIcon(g_Au.CommodityBitmapName(pi.key())), name, m_PatentList);
		lwi->setData(Qt::UserRole, pi.key());
		lwi->setData(Qt::UserRole+1, *pi);
	}
	m_PatentList->setSortingEnabled(true);
	m_PatentList->sortItems();
}

void AuGovPatents::UpdateEmpireRecord()
{
	// If this widget is not visible, just clear the data.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovPatents::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_PatentList->clear();
	}
	else
	{
		//qDebug("AuGovPatents::UpdateEmpireRecord page is visible. Update contents.");
		ListPatents();
	}
}

void AuGovPatents::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	UpdateEmpireRecord();
}

void AuGovPatents::GetPatent()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a UI to get a list of the patents they wish to record.
	AuPatentsGet dlg(true, AuId(0), this);
	if(pFrame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	QSet<COMMODITYID> patents;
	dlg.Patents(patents);
	if(patents.empty())
		return;

	AuGearPos gearPos = dlg.MediaPos();
	if(!gearPos.IsValid())
		return;

	quint8 maxQ = dlg.QualityLimit();

	auApp->EmpirePatentsRequested(patents, maxQ, gearPos);
}

void AuGovPatents::RecordPatent()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Show a UI to get a list of the patents they wish to record.
	AuPatentsRecord dlg(this);
	if(pFrame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	// Record the patents, if any.
	AuKnownPatents patents;
	dlg.Patents(patents);
	auApp->EmpirePatentsRecord(patents);
}

QSize AuGovPatents::sizeHint() const
{
	return QSize(800,600);
}
