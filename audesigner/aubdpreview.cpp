//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpreview.cpp,v 1.4 2022/02/19 18:25:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdpreview.h"
//
#include "auglobal.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

AuBdPreview::AuBdPreview(bool hasPower, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Building Preview Settings"));
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

	QCheckBox* ckpower = new QCheckBox(tr("Buildings Have Power"), this);
	ckpower->setChecked(hasPower);
	ckpower->setToolTip(tr("<html>"
				"<p>"
				"Switches power on/off to all buildings. "
				"</p>"
				"<p>"
				"Window lights animate when a building has power and it is dark. "
				"Buildings on a ringworld have power, regardless of this setting. "
				"Window light animation happens much faster in preview than it does in the game. "
				"</p>"
				"<p>"
				"In the game, a building must be occupied by residents or workers for lights to be on. "
				"Occupancy depends upon building usage. "
				"In preview, buildings are lit if they could potentially be occupied by residents or workers. "
				"</p>"
				"</html>"));
	connect(ckpower, SIGNAL(stateChanged(int)), SLOT(HasPowerStateChanged(int)));

	QHBoxLayout* rlay = new QHBoxLayout;
	rlay->addWidget(rotangle);
	rlay->addWidget(m_RotAngle);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbtod);
	lay->addLayout(rlay);
	lay->addWidget(ckpower);
	lay->addStretch();
	setAuDlgLayout(lay);

	float angle = angVal * DEG2RADf;
	emit TimeOfDayChanged(angle);
}

AuBdPreview::~AuBdPreview()
{
}

void AuBdPreview::setVisible(bool visible)
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

void AuBdPreview::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BdPreview/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BdPreview/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BdPreview/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BdPreview/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdPreview::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BdPreview/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BdPreview/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BdPreview/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BdPreview/SizeY",sz.height());
}

void AuBdPreview::CancelClicked()
{
	reject();
}

void AuBdPreview::OkClicked()
{
	accept();
}

void AuBdPreview::TimeOfDaySliderChanged(int sliderVal)
{
	g_Au.Settings().writeEntry("Widget/BdPreview/ToD", sliderVal);
	if(sliderVal > 0)
		m_RotAngle->setText(QString("+%1%2").arg(sliderVal).arg(g_Au.DegreeSymbol()));
	else
		m_RotAngle->setText(QString("%1%2").arg(sliderVal).arg(g_Au.DegreeSymbol()));
	float angle = sliderVal * DEG2RADf;
	emit TimeOfDayChanged(angle);
}

void AuBdPreview::HasPowerStateChanged(int checkState)
{
	emit HasPowerChanged(checkState == Qt::Checked);
}
