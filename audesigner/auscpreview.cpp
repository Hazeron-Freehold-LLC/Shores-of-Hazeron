//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpreview.cpp,v 1.4 2022/02/19 18:25:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpreview.h"
//
#include "auglobal.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

AuScPreview::AuScPreview(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Spacecraft Preview Settings"));
	setAuDlgIcon(QIcon(":data/CommChannel.png"));

	int lastToD = g_Au.Settings().readNumEntry("Widget/BdPreview/ToD", 0);
	int angVal = lastToD;
	if(angVal > 180)
		angVal -= 360;
	else if(angVal < -180)
		angVal += 360;

	QLabel* lbtod = new QLabel(tr("Time of Day"), this);

	QSlider* rotangle = new QSlider(Qt::Horizontal, this);
	rotangle->setRange(-180, 180);
	rotangle->setValue(angVal);
	rotangle->setToolTip(tr("<html>"
				"Changes day/night lighting of the scene. "
				"Adjusts rotation of your planet on its axis. "
				"Adjusts shadow panel angle on a ringworld. "
				"</html>"));
	connect(rotangle, SIGNAL(valueChanged(int)), SLOT(TimeOfDaySliderChanged(int)));

	m_RotAngle = new QLabel(this);
	if(rotangle->value() > 0)
		m_RotAngle->setText(QString("+%1%2").arg(rotangle->value()).arg(g_Au.DegreeSymbol()));
	else
		m_RotAngle->setText(QString("%1%2").arg(rotangle->value()).arg(g_Au.DegreeSymbol()));

	QHBoxLayout* rlay = new QHBoxLayout;
	rlay->addWidget(rotangle);
	rlay->addWidget(m_RotAngle);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbtod);
	lay->addLayout(rlay);
	lay->addStretch();
	setAuDlgLayout(lay);

	float angle = angVal * DEG2RADf;
	emit TimeOfDayChanged(angle);
}

AuScPreview::~AuScPreview()
{
}

void AuScPreview::setVisible(bool visible)
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

void AuScPreview::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScPreview/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScPreview/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScPreview/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScPreview/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPreview::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScPreview/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScPreview/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScPreview/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScPreview/SizeY",sz.height());
}

void AuScPreview::CancelClicked()
{
	reject();
}

void AuScPreview::OkClicked()
{
	accept();
}

void AuScPreview::TimeOfDaySliderChanged(int sliderVal)
{
	g_Au.Settings().writeEntry("Widget/ScPreview/ToD", sliderVal);
	if(sliderVal > 0)
		m_RotAngle->setText(QString("+%1%2").arg(sliderVal).arg(g_Au.DegreeSymbol()));
	else
		m_RotAngle->setText(QString("%1%2").arg(sliderVal).arg(g_Au.DegreeSymbol()));
	float angle = sliderVal * DEG2RADf;
	emit TimeOfDayChanged(angle);
}
