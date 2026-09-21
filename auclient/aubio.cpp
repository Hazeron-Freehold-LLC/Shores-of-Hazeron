//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubio.cpp,v 1.13 2022/02/19 18:08:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubio.h"
//
#include "aubioaccount.h"
#include "aubioavatar.h"
#include "aubiobiology.h"
#include "aubiobounties.h"
#include "aubiodomain.h"
#include "aubiojobs.h"
#include "aubioskills.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include <QGridLayout>
#include <QIcon>
#include <QStackedLayout>
#include <QTabBar>

void AuBioDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCharacterUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBioPages::AuBioPages(QWidget* parent) : QWidget(parent)
{
	m_Pages = new QStackedLayout;

	m_Tabs = new QTabBar(this);
	connect(m_Tabs, SIGNAL(currentChanged(int)), m_Pages, SLOT(setCurrentIndex(int)));

	int pageIndex = 0;

	m_AvatarPage = new AuBioAvatar(this);
	m_Pages->insertWidget(pageIndex, m_AvatarPage);

	QIcon avatarIcon(":data/c_VacSuitHelmet.png");
	int itab = m_Tabs->insertTab(pageIndex, avatarIcon, tr("Avatar"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Avatar</b><br>"
			"Shows information about your avatar."
			"</html>"));

	pageIndex++;

	m_SkillsPage = new AuBioSkills(this);
	m_Pages->insertWidget(pageIndex, m_SkillsPage);

	QIcon skillsIcon(":data/Brain.png");
	itab = m_Tabs->insertTab(pageIndex, skillsIcon, tr("Skills"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Skills</b><br>"
			"Shows skills learned by your avatar."
			"</html>"));

	pageIndex++;

	m_BiologyPage = new AuBioBiology(this);
	m_Pages->insertWidget(pageIndex, m_BiologyPage);

	QIcon biologyIcon(":data/DNA.png");
	itab = m_Tabs->insertTab(pageIndex, biologyIcon, tr("Biology"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Biology</b><br>"
			"Shows information about your avatar's physical body."
			"</html>"));

	pageIndex++;

	m_DomainPage = new AuBioDomain(this);
	m_Pages->insertWidget(pageIndex, m_DomainPage);

	QIcon sysIcon(":data/AvatarDomain.png");
	itab = m_Tabs->insertTab(pageIndex, sysIcon, tr("Land"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Land</b><br>"
			"Shows the land of your avatar."
			"</html>"));

	pageIndex++;

	m_JobsPage = new AuBioJobs(this);
	m_Pages->insertWidget(pageIndex, m_JobsPage);

	QIcon jobsIcon(":data/OrderRead.png");
	itab = m_Tabs->insertTab(pageIndex, jobsIcon, tr("Stories"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Stories</b><br>"
			"Shows information about your active stories and jobs."
			"</html>"));

	pageIndex++;

	m_BountiesPage = new AuBioBounties(this);
	m_Pages->insertWidget(pageIndex, m_BountiesPage);

	QIcon bountyIcon(":data/JobBountyHunter.png");
	itab = m_Tabs->insertTab(pageIndex, bountyIcon, tr("Bounties"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Bounties</b><br>"
			"Shows rewards for killing your avatar."
			"</html>"));

	pageIndex++;

	m_AccountPage = new AuBioAccount(this);
	m_Pages->insertWidget(pageIndex, m_AccountPage);

	QIcon accountIcon(":data/Login.png");
	itab = m_Tabs->insertTab(pageIndex, accountIcon, tr("Account"));
	m_Tabs->setTabToolTip(itab, tr("<html><b>Account</b><br>"
			"Manages your login account. "
			"</html>"));

	QGridLayout* lay = new QGridLayout();
	lay->addWidget(m_Tabs, 0, 0);
	lay->addLayout(m_Pages, 1, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuBioPages::~AuBioPages()
{
}
