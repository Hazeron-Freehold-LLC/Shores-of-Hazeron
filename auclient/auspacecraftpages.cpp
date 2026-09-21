//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftpages.cpp,v 1.6 2022/02/19 18:16:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftpages.h"
//
#include "auapp.h"
#include "audamagereport.h"
#include "auframe.h"
#include "auglobal.h"
#include "AuMissionTool.h"
#include "auscexchange.h"
#include "auspacecraftaccount.h"
#include "auspacecraftberths.h"
#include "auspacecraftberthsdialog.h"
#include "auspacecraftbounties.h"
#include "auspacecraftpolicy.h"
#include "auspacecraftshipments.h"
#include "auspacecraftspecs.h"
#include <QIcon>
#include <QKeyEvent>
#include <QStackedLayout>

void AuSpacecraftDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSpacecraftDockFree::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftTabs::AuSpacecraftTabs(QWidget *parent) : AuTabColumn(parent)
{
}

AuSpacecraftTabs::~AuSpacecraftTabs()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftPages::AuSpacecraftPages(bool freeAvatar, QWidget* parent) : QWidget(parent)
{
	m_Pages = new QStackedLayout();

	AuSpacecraftTabs* tabs = new AuSpacecraftTabs(this);
	connect(tabs, SIGNAL(CurrentChanged(int)), SLOT(CurrentTabChanged(int)));

	QGridLayout* lay = new QGridLayout;
	lay->addWidget(tabs, 0,0);
	lay->addLayout(m_Pages, 0,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	// Create all pages.
	if(!freeAvatar)
	{
		int tabIndex = tabs->AddTab(QIcon(":data/OrderTakeCommand.png"));
		tabs->SetTabToolTip(tabIndex, tr("Mission"));
		m_ShipMission = new AuMissionTool(this);
		m_Pages->addWidget(m_ShipMission);
	}
	else
		m_ShipMission = 0;

	if(!freeAvatar)
	{
		int tabIndex = tabs->AddTab(QIcon(":data/Captain.png"));
		tabs->SetTabToolTip(tabIndex, tr("Policy"));
		m_ShipPolicy = new AuSpacecraftPolicy(this);
		m_Pages->addWidget(m_ShipPolicy);
	}
	else
		m_ShipPolicy = 0;

	{
		int tabIndex = tabs->AddTab(QIcon(":data/CommCrew.png"));
		tabs->SetTabToolTip(tabIndex, tr("Roster"));
		m_ShipBerths = new AuSpacecraftBerths(this);
		m_Pages->addWidget(m_ShipBerths);
	}

	if(!freeAvatar)
	{
		int tabIndex = tabs->AddTab(QIcon(":data/c_Money.png"));
		tabs->SetTabToolTip(tabIndex, tr("Account"));
		m_ShipAccount = new AuSpacecraftAccount(this);
		m_Pages->addWidget(m_ShipAccount);
	}
	else
		m_ShipAccount = 0;

	{
		int tabIndex = tabs->AddTab(QIcon(":data/JobBountyHunter.png"));
		tabs->SetTabToolTip(tabIndex, tr("Bounties"));
		m_ShipBounties = new AuSpacecraftBounties(this);
		m_Pages->addWidget(m_ShipBounties);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/MsgDamageReport.png"));
		tabs->SetTabToolTip(tabIndex, tr("Damage"));
		m_ShipDamage = new AuDamageReport(this);
		m_Pages->addWidget(m_ShipDamage);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/c_Container.png"));
		tabs->SetTabToolTip(tabIndex, tr("Shipments"));
		m_ShipShipments = new AuSpacecraftShipments(this);
		m_Pages->addWidget(m_ShipShipments);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/c_ScDesignDrawing.png"));
		tabs->SetTabToolTip(tabIndex, tr("Specifications"));
		m_ShipSpecs = new AuSpacecraftSpecs(this);
		m_Pages->addWidget(m_ShipSpecs);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/Exchange.png"));
		tabs->SetTabToolTip(tabIndex, tr("Blueprint Exchange"));
		m_ShipDesignExchange = new AuScExchange(this);
		m_Pages->addWidget(m_ShipDesignExchange);
	}
}

AuSpacecraftPages::~AuSpacecraftPages()
{
}

void AuSpacecraftPages::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

void AuSpacecraftPages::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);
}

void AuSpacecraftPages::CurrentTabChanged(int idx)
{
	m_Pages->setCurrentIndex(idx);
}

QSize AuSpacecraftPages::sizeHint() const
{
	return QSize(640,300);
}

void AuSpacecraftPages::BrokerShipment(AuShipment shipment)
{
	if(m_ShipShipments)
		m_ShipShipments->Shipment(shipment);
}

void AuSpacecraftPages::BrokerShipmentsClear()
{
	if(m_ShipShipments)
		m_ShipShipments->ShipmentsClear();
}

