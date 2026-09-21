//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscstationgunnerdlg.cpp,v 1.3 2022/02/19 18:25:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscstationgunnerdlg.h"
//
#include "auglobal.h"
#include "auscparts.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScStationGunnerDlg::AuScStationGunnerDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Turret Gunner"));
	setAuDlgIcon(QIcon(":data/dp_TurretTopBottom.png"));

	SYSTEM system = SYS_Turret;
	QLabel* lbmodules = new QLabel(tr("Choose a module for the %1 controlled by the station.")
			.arg(ScSystemName(system)), this);

	m_Modules = new QComboBox(this);

	for(int cid=0; cid<CID_Count; cid++)
		if(DbCommodity::IsScModule(system, (COMMODITYID)cid))
			m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);

	QLabel* lbinfo = new QLabel(tr("Choose the body position of the turret's gunner."), this);
	lbinfo->setWordWrap(true);

	m_SitSwivel = new QRadioButton(tr("Sit Swivel %1 Gunner can turn and look, like a swivel chair.").arg(g_Au.MiddleDotSymbol()),
			this);
	m_SitSwivel->setChecked(true);

	//m_SitSwivel3D = new QRadioButton(tr("Sit Swivel 3D %1 Gunner's body turns in all 3D directions.").arg(g_Au.MiddleDotSymbol()),
			//this);

	m_SitStationary = new QRadioButton(tr("Sit Stationary %1 Gunner cannot turn but can twist at the waist and look, "
				"like a bucket seat.").arg(g_Au.MiddleDotSymbol()), this);

	m_Stand = new QRadioButton(tr("Stand %1 Gunner stands; can turn and look.").arg(g_Au.MiddleDotSymbol()), this);

	m_StandStationary = new QRadioButton(tr("Stand Stationary %1 Gunner stands; cannot turn but can twist at the waist and look.")
			.arg(g_Au.MiddleDotSymbol()), this);

	m_PickLocation = new QPushButton(tr("Pick Gunner Location"), this);
	m_PickLocation->setAutoDefault(true);
	m_PickLocation->setDefault(true);
	m_PickLocation->setToolTip(tr("<html>"
				"Accepts the choice of gunner position and requests the gunner's 3D location."
				"</html>"));
	connect(m_PickLocation, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_PickLocation);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbmodules);
	lay->addWidget(m_Modules);
	lay->addWidget(lbinfo);
	lay->addWidget(m_SitSwivel);
	//lay->addWidget(m_SitSwivel3D);
	lay->addWidget(m_SitStationary);
	lay->addWidget(m_Stand);
	lay->addWidget(m_StandStationary);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScStationGunnerDlg::~AuScStationGunnerDlg()
{
}

void AuScStationGunnerDlg::setVisible(bool visible)
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

void AuScStationGunnerDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StationGunnerDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StationGunnerDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StationGunnerDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StationGunnerDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScStationGunnerDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StationGunnerDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StationGunnerDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StationGunnerDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StationGunnerDlg/SizeY",sz.height());
}

quint8 AuScStationGunnerDlg::Position() const
{
	if(m_SitSwivel->isChecked())
		return AuScTurret::Gunner_Sit;

	//if(m_SitSwivel3D->isChecked())
		//return AuScTurret::Gunner_SitSwivel3D;

	if(m_SitStationary->isChecked())
		return AuScTurret::Gunner_SitStationary;

	if(m_Stand->isChecked())
		return AuScTurret::Gunner_Stand;

	if(m_StandStationary->isChecked())
		return AuScTurret::Gunner_StandStationary;

	return AuScTurret::Gunner_Sit;
}

COMMODITYID AuScStationGunnerDlg::Module() const
{
	if(!m_Modules)
		return CID_Invalid;
	return (COMMODITYID)m_Modules->itemData(m_Modules->currentIndex()).toInt();
}
