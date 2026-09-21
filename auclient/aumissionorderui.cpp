//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumissionorderui.cpp,v 1.2 2022/02/19 18:16:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumissionorderui.h"
//
#include "auglobal.h"
#include "aumissionorderwidget.h"
#include <QGridLayout>
#include <QPushButton>

AuMissionOrderUi::AuMissionOrderUi(const AuId& authorization, const QString& title, const AuId& solarSystem,
		const AuSensorBlipList& blips, const AuPoint& galaxyLoc, const AuPoint& sectorLoc, const AuPoint& systemLoc,
		const AuMissionOrder* order, QWidget* parent) : AuDialog(parent)
{
	//qDebug("AuMissionOrderUi system %d", solarSystem.GetID());
	setAuDlgIcon(QIcon(":data/GovSpacecraft.png"));
	setAuDlgTitle(title);

	m_MO = new AuMissionOrderWidget(authorization, solarSystem, blips, galaxyLoc, sectorLoc, systemLoc, order);
	connect(m_MO, SIGNAL(ValidityChanged()), SLOT(EnableControls()));

	m_Ok = new QPushButton(tr("Ok Order"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(AcceptOrder()));
	m_Ok->setToolTip(tr("Accepts the order."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_MO, row,0);
	row++;
	lay->addWidget(m_Ok, row,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuMissionOrderUi::~AuMissionOrderUi()
{
}

void AuMissionOrderUi::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		QTimer::singleShot(0, [=](){RestoreGeometry();});
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

QSize AuMissionOrderUi::sizeHint() const
{
	return QSize(400,200);
}

void AuMissionOrderUi::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/MissionOrderUi/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/MissionOrderUi/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/MissionOrderUi/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/MissionOrderUi/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuMissionOrderUi::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/MissionOrderUi/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/MissionOrderUi/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/MissionOrderUi/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/MissionOrderUi/SizeY",sz.height());
}

void AuMissionOrderUi::AcceptOrder()
{
	accept();
}

void AuMissionOrderUi::EnableControls()
{
	m_Ok->setEnabled(m_MO->IsContentValid());
}

AuMissionOrder* AuMissionOrderUi::Order() const
{
	return m_MO->Order();
}
