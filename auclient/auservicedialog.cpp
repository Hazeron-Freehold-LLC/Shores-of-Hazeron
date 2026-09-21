//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservicedialog.cpp,v 1.2 2022/02/19 18:16:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auservicedialog.h"
//
#include "aucommchannel.h"
#include "aurepairdamage.h"
#include "aurequestupgrade.h"
#include "auservicemisc.h"
#include "auservicerefit.h"
#include <auglobal.h>
#include <QGridLayout>
#include <QIcon>
#include <QStackedLayout>
#include <QTabBar>

AuServiceDialog::AuServiceDialog(AuCommChannel* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_Spacecraft.png"));
	setAuDlgTitle("Service Spacecraft");

	m_Pages = new QStackedLayout;

	m_Tabs = new QTabBar(this);
	connect(m_Tabs, SIGNAL(currentChanged(int)), m_Pages, SLOT(setCurrentIndex(int)));

	//
	int page = 0;
	AuServiceMisc* miscPage = new AuServiceMisc(this);
	m_Pages->insertWidget(page, miscPage);

	QIcon miscIcon(":data/Sweep.png");
	int itab = m_Tabs->insertTab(page, miscIcon, "Services");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Services</b><br>"
				"Request fuel, exterminator or house keeping services from a city. "
				"<i>Tips</i><br>"
				"%1 You must be an officer of the ship.<br>"
				"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	page++;

	//
	AuRepairDamage* repairDamage = new AuRepairDamage(this);
	m_Pages->insertWidget(page, repairDamage);

	QIcon repairDamageIcon(":data/MsgDamageReport.png");
	itab = m_Tabs->insertTab(page, repairDamageIcon, "Repair");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Repair</b><br>"
				"Request repairs from a city with an airport repair shop.<br>"
				"<i>Tips</i><br>"
				"%1 You must be an officer of the ship.<br>"
				"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	page++;

	//
	AuRequestUpgrade* modulesPage = new AuRequestUpgrade(this);
	m_Pages->insertWidget(page, modulesPage);

	QIcon modulesIcon(":data/ShipUpgrade.png");
	itab = m_Tabs->insertTab(page, modulesIcon, "Modules");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Modules</b><br>"
				"Request spacecraft system upgrades from a city with an airport repair shop.<br>"
				"<i>Tips</i><br>"
				"%1 You must be an officer of the ship.<br>"
				"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	page++;

	//
	AuServiceRefit* refitPage = new AuServiceRefit(this);
	m_Pages->insertWidget(page, refitPage);

	QIcon refitIcon(":data/ShipRefit.png");
	itab = m_Tabs->insertTab(page, refitIcon, "Refit");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Refit</b><br>"
				"Request refit from a city with an airport repair shop.<br>"
				"<i>Tips</i><br>"
				"%1 You must be an officer of the ship.<br>"
				"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	QGridLayout* lay = new QGridLayout();
	lay->addWidget(m_Tabs, 0, 0);
	lay->addLayout(m_Pages, 1, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuServiceDialog::~AuServiceDialog()
{
}

void AuServiceDialog::setVisible(bool visible)
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

void AuServiceDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ServiceDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ServiceDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ServiceDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ServiceDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuServiceDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ServiceDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ServiceDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ServiceDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ServiceDialog/SizeY",sz.height());
}

AuCommChannel* AuServiceDialog::Channel()
{
	return dynamic_cast<AuCommChannel*>(parent());
}
