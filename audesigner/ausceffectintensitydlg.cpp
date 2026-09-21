//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectintensitydlg.cpp,v 1.2 2022/02/19 18:25:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausceffectintensitydlg.h"
//
#include "auglobal.h"
#include "aupushbutton.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScEffectIntensityDlg::AuScEffectIntensityDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Effect Intensity"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	float intensity = g_Au.Settings().readNumEntry(QString("Widget/EffectIntensityDlg/Intensity"), 1.);

	QLabel* lbintensity = new QLabel(tr("<html>"
				"<p>"
				"Enter an intensity for the effect texture layer, in the range of 0 to 1. "
				"Effect intensity is applied at selected vertices of selected faces. "
				"</p>"
				"</html>"), this);
	lbintensity->setWordWrap(true);

	QString curval = QString("%1").arg(intensity, 0,'f',2);
	m_Intensity = new QLineEdit(curval, this);
	m_Intensity->setToolTip(tr("<html>"
				"Enter an intensity value from 0 to 1. "
				"</html>"));

	QPushButton* pbok = new QPushButton(tr("Set Effect Intensity"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Applies effect intensity to the selected vertices of the selected faces."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoIt()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbintensity);
	lay->addWidget(m_Intensity);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScEffectIntensityDlg::~AuScEffectIntensityDlg()
{
}

void AuScEffectIntensityDlg::setVisible(bool visible)
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

void AuScEffectIntensityDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/EffectIntensityDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/EffectIntensityDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/EffectIntensityDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/EffectIntensityDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScEffectIntensityDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/EffectIntensityDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/EffectIntensityDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/EffectIntensityDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/EffectIntensityDlg/SizeY",sz.height());
}

void AuScEffectIntensityDlg::DoIt()
{
	g_Au.Settings().writeEntry("Widget/EffectIntensityDlg/Intensity", Intensity());
	accept();
}

float AuScEffectIntensityDlg::Intensity() const
{
	float intensity = m_Intensity->text().toFloat();
	if(intensity <= 0)
		return 0;
	else if(intensity >= 1)
		return 1;
	return intensity;
}
