//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscstationmedicdlg.cpp,v 1.3 2022/02/19 18:25:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscstationmedicdlg.h"
//
#include "auglobal.h"
#include "auscparts.h"
#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScStationMedicDlg::AuScStationMedicDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Medic Station"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	SYSTEM system = SYS_SurgeryUnit;
	QLabel* lbmodules = new QLabel(tr("Choose a module for the %1 controlled by the station.")
			.arg(ScSystemName(system)), this);

	m_Modules = new QComboBox(this);

	for(int cid=0; cid<CID_Count; cid++)
		if(DbCommodity::IsScModule(system, (COMMODITYID)cid))
			m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);

	QVBoxLayout* medicRadios = new QVBoxLayout;
	{
		QLabel* lbinfo = new QLabel(tr("Choose the body position of the medic operating the station."), this);
		lbinfo->setWordWrap(true);

		m_MedicSitSwivel = new QRadioButton(tr("Sit Swivel %1 Operator can turn and look, like a swivel chair.")
				.arg(g_Au.MiddleDotSymbol()),
				this);
		m_MedicSitSwivel->setChecked(true);

		m_MedicSitStationary = new QRadioButton(tr("Sit Stationary %1 Operator cannot turn but can twist at the waist and look, "
					"like a bucket seat.").arg(g_Au.MiddleDotSymbol()), this);

		m_MedicStand = new QRadioButton(tr("Stand %1 Operator stands; can turn and look.").arg(g_Au.MiddleDotSymbol()), this);

		m_MedicStandStationary =
			new QRadioButton(tr("Stand Stationary %1 Operator stands; cannot turn but can twist at the waist and look.")
					.arg(g_Au.MiddleDotSymbol()), this);

		medicRadios->addWidget(lbinfo);
		medicRadios->addWidget(m_MedicSitSwivel);
		medicRadios->addWidget(m_MedicSitStationary);
		medicRadios->addWidget(m_MedicStand);
		medicRadios->addWidget(m_MedicStandStationary);
		medicRadios->setSpacing(2);
		medicRadios->setMargin(0);

		QButtonGroup* bg = new QButtonGroup(this);
		bg->addButton(m_MedicSitSwivel);
		bg->addButton(m_MedicSitStationary);
		bg->addButton(m_MedicStand);
		bg->addButton(m_MedicStandStationary);
	}

	QVBoxLayout* patientRadios = new QVBoxLayout;
	{
		QLabel* lbinfo = new QLabel(tr("Choose the body position of the station's patient."), this);
		lbinfo->setWordWrap(true);

		m_PatientProne = new QRadioButton(tr("Prone %1 Patient lies prone.")
				.arg(g_Au.MiddleDotSymbol()),
				this);
		m_PatientProne->setChecked(true);

		m_PatientSitSwivel = new QRadioButton(tr("Sit Swivel %1 Patient can turn and look, like a swivel chair.")
				.arg(g_Au.MiddleDotSymbol()),
				this);

		m_PatientSitStationary = new QRadioButton(tr("Sit Stationary %1 Patient cannot turn but can twist at the waist and look, "
					"like a bucket seat.").arg(g_Au.MiddleDotSymbol()), this);

		m_PatientStand = new QRadioButton(tr("Stand %1 Patient stands; can turn and look.").arg(g_Au.MiddleDotSymbol()), this);

		m_PatientStandStationary =
			new QRadioButton(tr("Stand Stationary %1 Patient stands; cannot turn but can twist at the waist and look.")
					.arg(g_Au.MiddleDotSymbol()),
					this);

		patientRadios->addWidget(lbinfo);
		patientRadios->addWidget(m_PatientProne);
		patientRadios->addWidget(m_PatientSitSwivel);
		patientRadios->addWidget(m_PatientSitStationary);
		patientRadios->addWidget(m_PatientStand);
		patientRadios->addWidget(m_PatientStandStationary);
		patientRadios->setSpacing(2);
		patientRadios->setMargin(0);

		QButtonGroup* bg = new QButtonGroup(this);
		bg->addButton(m_PatientProne);
		bg->addButton(m_PatientSitSwivel);
		bg->addButton(m_PatientSitStationary);
		bg->addButton(m_PatientStand);
		bg->addButton(m_PatientStandStationary);
	}

	m_PickLocation = new QPushButton(tr("Pick Medic and Patient Locations"), this);
	m_PickLocation->setAutoDefault(true);
	m_PickLocation->setDefault(true);
	m_PickLocation->setToolTip(tr("<html>"
				"Accepts the choice of medic and patient positions and requests the medic's and patient's 3D locations."
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
	lay->addLayout(medicRadios);
	lay->addLayout(patientRadios);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScStationMedicDlg::~AuScStationMedicDlg()
{
}

void AuScStationMedicDlg::setVisible(bool visible)
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

void AuScStationMedicDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StationMedicDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StationMedicDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StationMedicDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StationMedicDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScStationMedicDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StationMedicDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StationMedicDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StationMedicDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StationMedicDlg/SizeY",sz.height());
}

quint8 AuScStationMedicDlg::MedicPosition() const
{
	if(m_MedicSitSwivel->isChecked())
		return AuScStation::Operator_Sit;

	if(m_MedicSitStationary->isChecked())
		return AuScStation::Operator_SitStationary;

	if(m_MedicStand->isChecked())
		return AuScStation::Operator_Stand;

	if(m_MedicStandStationary->isChecked())
		return AuScStation::Operator_StandStationary;

	return AuScStation::Operator_Sit;
}

quint8 AuScStationMedicDlg::PatientPosition() const
{
	if(m_PatientProne->isChecked())
		return AuScStationMedic::Patient_Prone;

	if(m_PatientSitSwivel->isChecked())
		return AuScStationMedic::Patient_Sit;

	if(m_PatientSitStationary->isChecked())
		return AuScStationMedic::Patient_SitStationary;

	if(m_PatientStand->isChecked())
		return AuScStationMedic::Patient_Stand;

	if(m_PatientStandStationary->isChecked())
		return AuScStationMedic::Patient_StandStationary;

	return AuScStationMedic::Patient_Sit;
}

COMMODITYID AuScStationMedicDlg::Module() const
{
	if(!m_Modules)
		return CID_Invalid;
	return (COMMODITYID)m_Modules->itemData(m_Modules->currentIndex()).toInt();
}
