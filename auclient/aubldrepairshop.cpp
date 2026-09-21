//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldrepairshop.cpp,v 1.6 2022/02/19 18:09:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldrepairshop.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldRepairShop::AuBldRepairShop(QWidget* parent) : QWidget(parent)
{
	QLabel* lbico = new QLabel(this);
	lbico->setPixmap(QPixmap(":data/bAirportRepairShop.png"));

	m_Info = new QLabel(this);
	m_Info->setWordWrap(true);

	m_Recover = new QPushButton(QIcon(":data/MsgSOS.png"), tr("Recover Spacecraft"));
	m_Recover->setToolTip(tr("<html><b>Recover Spacecraft</b><br>"
				"Recovers spacecraft from stasis.<br>"
				"%1 City empire repair policy must permit recovery.<br>"
				"%1 Spacecraft will be moored in orbit above this repair shop."
				"<br>"
				"%1 Spacecraft will announce on the <i>Crew</i> channel."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_Recover, SIGNAL(clicked()), SLOT(Recover()));

	QHBoxLayout* ilay = new QHBoxLayout();
	ilay->addWidget(lbico);
	ilay->addWidget(m_Info);

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(m_Recover);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(ilay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldRepairShop::~AuBldRepairShop()
{
}

void AuBldRepairShop::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	m_Info->setText(tr("<html>"
				"<p>Spacecraft enter <i>stasis</i> when destroyed, "
				"if an avatar holds an officer rank berth on the roster. "
				"Spacecraft remain in stasis where they were destroyed.</p>"

				"<p>Avatar officers can restore their ships from stasis at a repair shop.</p>"

				"<p>The ship must be in the same dimension as the shop.</p>"

				"<p>Recovery time equals the time it would take to manufacture the design new. "
				"The time starts when the ship enters stasis, not when recovery operations begin.</p>"

				"<p>Cargo, vehicles, creatures and loose items were removed when the ship was destroyed. "
				"Static displayed items remain intact. "
				"Systems are often heavily damaged, due to the ship's destruction.</p>"

				"<p>The recovery team performs basic repairs so the wreckage can hold orbit. "
				"One life support unit is repaired if none are functioniong, so the crew can recall to their berths. "
				"Control stations are repaired. "
				"Power plant is repaired up to 5% and the capacitor is fully charged. "
				"Maneuver drive is repaired up to 5%.</p>"

				"<p>Recovered wrecks are moored in orbit above the repair shop. "
				"Hull doors are closed and locked. "
				"Navigation route is cleared. "
				"Mission is deactivated.</p>"

				"<p>Wrecks announce the recovery result on the <i>Crew</i> channel.</p>"
				"</html>"));
}

void AuBldRepairShop::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widget now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();

	m_Info->setText("");
}

void AuBldRepairShop::Recover()
{
	auApp->RepairShopRecover();
}

