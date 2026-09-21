//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovern.cpp,v 1.5 2022/02/19 18:15:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovern.h"
//
#include "auapp.h"
#include <auglobal.h>
#include "augovannouncements.h"
#include "augovcities.h"
#include "augovcommerce.h"
#include "augovdiplomacy.h"
#include "augoverntabs.h"
#include "augovpatents.h"
#include "augovpolicy.h"
#include "auempirecitizens.h"
#include "augovtaxes.h"
#include "auframe.h"
#include <QIcon>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QTimer>

void AuGovernDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovern::AuGovern(QWidget* parent) : QWidget(parent)
{
	m_Pages = new QStackedLayout();

	AuGovernTabs* tabs = new AuGovernTabs(this);
	connect(tabs, SIGNAL(CurrentChanged(int)), SLOT(CurrentTabChanged(int)));

	QGridLayout* lay = new QGridLayout;
	lay->addWidget(tabs, 0,0);
	lay->addLayout(m_Pages, 0,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	// Create all governance pages.
	{
		int tabIndex = tabs->AddTab(QIcon(":data/Speaker.png"));
		tabs->SetTabToolTip(tabIndex, tr("Announcements"));
		m_Announcements = new AuGovAnnouncements(this);
		m_Pages->addWidget(m_Announcements);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/CommGovernment.png"));
		tabs->SetTabToolTip(tabIndex, tr("Policy"));
		m_Policy = new AuGovPolicy(this);
		m_Pages->addWidget(m_Policy);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/CommDiplomacy.png"));
		tabs->SetTabToolTip(tabIndex, tr("Diplomacy"));
		m_Diplomacy = new AuGovDiplomacy(this);
		m_Pages->addWidget(m_Diplomacy);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/GovCommerce.png"));
		tabs->SetTabToolTip(tabIndex, tr("Commerce"));
		m_Commerce = new AuGovCommerce(this);
		m_Pages->addWidget(m_Commerce);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/c_Patent.png"));
		tabs->SetTabToolTip(tabIndex, tr("Patents"));
		m_Patents = new AuGovPatents(this);
		m_Pages->addWidget(m_Patents);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/CommCrew.png"));
		tabs->SetTabToolTip(tabIndex, tr("Citizens"));
		m_Citizens = new AuEmpireCitizens(this);
		m_Pages->addWidget(m_Citizens);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/City.png"));
		tabs->SetTabToolTip(tabIndex, tr("Cities and Bases"));
		m_Places = new AuGovCities(this);
		m_Pages->addWidget(m_Places);
	}

	{
		int tabIndex = tabs->AddTab(QIcon(":data/GovTaxes.png"));
		tabs->SetTabToolTip(tabIndex, tr("Taxes"));
		m_Taxes = new AuGovTaxes(this);
		m_Pages->addWidget(m_Taxes);
	}
}

AuGovern::~AuGovern()
{
}

void AuGovern::UpdateDatabase()
{
	AuFrame* frame = auApp->Frame();
	if(frame && frame->GetAndResetEmpireDataReceived())
	{
		//qDebug() << "AuGovern::UpdateDatabase";
		m_Announcements->UpdateEmpireRecord();

		m_Commerce->UpdateEmpireRecord();

		m_Places->UpdateEmpireRecord();

		m_Diplomacy->EmpireDataComplete();

		m_Patents->UpdateEmpireRecord();

		m_Policy->UpdateEmpireRecord();

		m_Citizens->UpdateEmpireRecord();

		m_Taxes->UpdateEmpireRecord();
	}
}

void AuGovern::CurrentTabChanged(int idx)
{
	m_Pages->setCurrentIndex(idx);
}

QSize AuGovern::sizeHint() const
{
	return QSize(640,300);
}
