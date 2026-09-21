//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscconsolemedicdlg.cpp,v 1.3 2022/02/19 18:25:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscconsolemedicdlg.h"
//
#include "auglobal.h"
#include "auscparts.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScConsoleMedicDlg::AuScConsoleMedicDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Medic Station"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbmodules = new QLabel(tr("Choose a module for the %1 controlled by the console.")
			.arg(ScSystemName(SYS_SurgeryUnit)), this);

	m_Modules = new QComboBox(this);

	for(int cid=0; cid<CID_Count; cid++)
		if(DbCommodity::IsScModule(SYS_SurgeryUnit, (COMMODITYID)cid))
			m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);

	QLabel* lbinfo = new QLabel(tr("Choose the body position of the station's patient."), this);
	lbinfo->setWordWrap(true);

	m_PatientProne = new QRadioButton(tr("Prone %1 Patient lies prone.")
			.arg(g_Au.MiddleDotSymbol()),
			this);
	m_PatientProne->setChecked(true);
	connect(m_PatientProne, SIGNAL(clicked()), SLOT(EnableControls()));

	m_PatientSitSwivel = new QRadioButton(tr("Sit Swivel %1 Patient can turn and look, like a swivel chair.")
			.arg(g_Au.MiddleDotSymbol()),
			this);
	connect(m_PatientSitSwivel, SIGNAL(clicked()), SLOT(EnableControls()));

	m_PatientSitStationary = new QRadioButton(tr("Sit Stationary %1 Patient cannot turn but can twist at the waist and look, "
				"like a bucket seat.").arg(g_Au.MiddleDotSymbol()), this);
	connect(m_PatientSitStationary, SIGNAL(clicked()), SLOT(EnableControls()));

	m_PatientStand = new QRadioButton(tr("Stand %1 Patient stands; can turn and look.").arg(g_Au.MiddleDotSymbol()), this);
	connect(m_PatientStand, SIGNAL(clicked()), SLOT(EnableControls()));

	m_PatientStandStationary =
		new QRadioButton(tr("Stand Stationary %1 Patient stands; cannot turn but can twist at the waist and look.")
				.arg(g_Au.MiddleDotSymbol()),
				this);
	connect(m_PatientStandStationary, SIGNAL(clicked()), SLOT(EnableControls()));

	m_PickLocation = new QPushButton(tr("Pick Patient Location"), this);
	m_PickLocation->setAutoDefault(true);
	m_PickLocation->setDefault(true);
	m_PickLocation->setToolTip(tr("<html>"
				"Accepts the choice of patient position and requests the patient's 3D location."
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
	lay->addWidget(m_PatientProne);
	lay->addWidget(m_PatientSitSwivel);
	lay->addWidget(m_PatientSitStationary);
	lay->addWidget(m_PatientStand);
	lay->addWidget(m_PatientStandStationary);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScConsoleMedicDlg::~AuScConsoleMedicDlg()
{
}

void AuScConsoleMedicDlg::setVisible(bool visible)
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

void AuScConsoleMedicDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ConsoleMedicDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ConsoleMedicDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ConsoleMedicDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ConsoleMedicDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScConsoleMedicDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ConsoleMedicDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ConsoleMedicDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ConsoleMedicDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ConsoleMedicDlg/SizeY",sz.height());
}

void AuScConsoleMedicDlg::EnableControls()
{
	m_PickLocation->setEnabled(m_PatientSitSwivel->isChecked() ||
			m_PatientProne->isChecked() ||
			m_PatientSitStationary->isChecked() ||
			m_PatientStand->isChecked() ||
			m_PatientStandStationary->isChecked());
}

quint8 AuScConsoleMedicDlg::PatientPosition() const
{
	if(m_PatientProne->isChecked())
		return AuScConsoleMedic::Patient_Prone;

	if(m_PatientSitSwivel->isChecked())
		return AuScConsoleMedic::Patient_Sit;

	if(m_PatientSitStationary->isChecked())
		return AuScConsoleMedic::Patient_SitStationary;

	if(m_PatientStand->isChecked())
		return AuScConsoleMedic::Patient_Stand;

	if(m_PatientStandStationary->isChecked())
		return AuScConsoleMedic::Patient_StandStationary;

	return AuScConsoleMedic::Patient_Sit;
}

COMMODITYID AuScConsoleMedicDlg::Module() const
{
	return (COMMODITYID)m_Modules->itemData(m_Modules->currentIndex()).toInt();
}
