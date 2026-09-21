//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscspherelatlongdlg.cpp,v 1.3 2022/02/19 18:25:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscspherelatlongdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

AuScSphereLatLongDlg::AuScSphereLatLongDlg(quint8 partType, QWidget* parent) : AuDialog(parent)
{
	m_PartType = partType;

	setAuDlgTitle(tr("Sphere Longitude/Latitude"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lblon = new QLabel(tr("Enter the number of longitude steps around the equator."), this);
	lblon->setWordWrap(true);

	m_LonStepsSpin = new QSpinBox(this);
	m_LonStepsSpin->setRange(3, 1000);
	m_LonStepsSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/SphereLongLat/LongitudeSteps%1").arg(m_PartType), 10));

	QLabel* lblat = new QLabel(tr("Enter the number of latitude steps from pole to pole."), this);
	lblat->setWordWrap(true);

	m_LatStepsSpin = new QSpinBox(this);
	m_LatStepsSpin->setRange(3, 1000);
	m_LatStepsSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/SphereLongLat/LatitudeSteps%1").arg(m_PartType), 10));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Creates a sphere."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lblon);
	lay->addWidget(m_LonStepsSpin);
	lay->addWidget(lblat);
	lay->addWidget(m_LatStepsSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScSphereLatLongDlg::~AuScSphereLatLongDlg()
{
}

void AuScSphereLatLongDlg::setVisible(bool visible)
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

void AuScSphereLatLongDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SphereLongLat/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SphereLongLat/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SphereLongLat/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SphereLongLat/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScSphereLatLongDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SphereLongLat/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SphereLongLat/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SphereLongLat/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SphereLongLat/SizeY",sz.height());
}

void AuScSphereLatLongDlg::OkPressed()
{
	g_Au.Settings().writeEntry(QString("Widget/SphereLongLat/LatitudeSteps%1").arg(m_PartType), m_LatStepsSpin->value());
	g_Au.Settings().writeEntry(QString("Widget/SphereLongLat/LongitudeSteps%1").arg(m_PartType), m_LonStepsSpin->value());
	accept();
}

quint16 AuScSphereLatLongDlg::LatitudeSteps() const
{
	return m_LatStepsSpin->value();
}

quint16 AuScSphereLatLongDlg::LongitudeSteps() const
{
	return m_LonStepsSpin->value();
}
