//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscstationdlg.cpp,v 1.3 2022/02/19 18:25:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscstationdlg.h"
//
#include "auglobal.h"
#include "auscparts.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScStationDlg::AuScStationDlg(SYSTEM system, bool isSc, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Station"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbmodules;
	QLabel* lbeffect;
	if(system != SYS_Count)
	{
		lbmodules = new QLabel(tr("Choose a module for the %1 controlled by the station.")
				.arg(ScSystemName(system)), this);

		m_Modules = new QComboBox(this);

		if(isSc)
		{
			for(int cid=0; cid<CID_Count; cid++)
				if(DbCommodity::IsScModule(system, (COMMODITYID)cid))
					m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);
		}
		else
		{
			for(int cid=0; cid<CID_Count; cid++)
				if(DbCommodity::IsBdModule(system, (COMMODITYID)cid))
					m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);
		}

		if(system == SYS_FTLDrive)
		{
			lbeffect = new QLabel(tr("Choose which surface effect will be animated when this FTL drive is active."), this);

			m_WhichEffect = new QComboBox(this);
			for(int s=0; s<4; s++)
				m_WhichEffect->addItem(tr("FTL Effect %1").arg(s+1), s);
		}
		else if(system == SYS_Transporter)
		{
			lbeffect = new QLabel(tr("Choose which surface effect will be animated when this transporter is active."), this);

			m_WhichEffect = new QComboBox(this);
			for(int s=0; s<10; s++)
				m_WhichEffect->addItem(tr("Transporter Effect %1").arg(s+1), s);
		}
		else if(system == SYS_Weapon)
		{
			lbeffect = new QLabel(tr("Choose which surface effect will be animated when this weapon is firing."), this);

			m_WhichEffect = new QComboBox(this);
			for(int s=0; s<10; s++)
				m_WhichEffect->addItem(tr("Weapon Effect %1").arg(s+1), s);
		}
		else
		{
			lbeffect = 0;
			m_WhichEffect = 0;
		}
	}
	else
	{
		lbmodules = 0;
		m_Modules = 0;
		lbeffect = 0;
		m_WhichEffect = 0;
	}

	QLabel* lbinfo = new QLabel(tr("Choose the body position of the station's operator."), this);
	lbinfo->setWordWrap(true);

	m_SitSwivel = new QRadioButton(tr("Sit Swivel %1 Operator can turn and look, like a swivel chair.").arg(g_Au.MiddleDotSymbol()),
			this);
	m_SitSwivel->setChecked(true);

	//m_SitSwivel3D = new QRadioButton(tr("Sit Swivel 3D %1 Operator's body turns in all 3D directions.").arg(g_Au.MiddleDotSymbol()),
			//this);

	m_SitStationary = new QRadioButton(tr("Sit Stationary %1 Operator cannot turn but can twist at the waist and look, "
				"like a bucket seat.").arg(g_Au.MiddleDotSymbol()), this);

	m_Stand = new QRadioButton(tr("Stand %1 Operator stands; can turn and look.").arg(g_Au.MiddleDotSymbol()), this);

	m_StandStationary = new QRadioButton(tr("Stand Stationary %1 Operator stands; cannot turn but can twist at the waist and look.")
			.arg(g_Au.MiddleDotSymbol()), this);

	m_PickLocation = new QPushButton(tr("Pick Operator Location"), this);
	m_PickLocation->setAutoDefault(true);
	m_PickLocation->setDefault(true);
	m_PickLocation->setToolTip(tr("<html>"
				"Accepts the choice of operator position and requests the operator's 3D location."
				"</html>"));
	connect(m_PickLocation, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_PickLocation);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	if(lbmodules)
	{
		lay->addWidget(lbmodules);
		lay->addWidget(m_Modules);
	}
	if(lbeffect && m_WhichEffect)
	{
		lay->addWidget(lbeffect);
		lay->addWidget(m_WhichEffect);
	}
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

AuScStationDlg::~AuScStationDlg()
{
}

void AuScStationDlg::setVisible(bool visible)
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

void AuScStationDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StationDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StationDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StationDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StationDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScStationDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StationDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StationDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StationDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StationDlg/SizeY",sz.height());
}

quint8 AuScStationDlg::Position() const
{
	if(m_SitSwivel->isChecked())
		return AuScStation::Operator_Sit;

	//if(m_SitSwivel3D->isChecked())
		//return AuScStation::Operator_SitSwivel3D;

	if(m_SitStationary->isChecked())
		return AuScStation::Operator_SitStationary;

	if(m_Stand->isChecked())
		return AuScStation::Operator_Stand;

	if(m_StandStationary->isChecked())
		return AuScStation::Operator_StandStationary;

	return AuScStation::Operator_Sit;
}

COMMODITYID AuScStationDlg::Module() const
{
	if(!m_Modules)
		return CID_Invalid;
	return (COMMODITYID)m_Modules->itemData(m_Modules->currentIndex()).toInt();
}

int AuScStationDlg::WhichEffect() const
{
	if(!m_WhichEffect)
		return 0;
	return m_WhichEffect->itemData(m_WhichEffect->currentIndex()).toInt();
}
