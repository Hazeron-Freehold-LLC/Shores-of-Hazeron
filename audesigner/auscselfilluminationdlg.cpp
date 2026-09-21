//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscselfilluminationdlg.cpp,v 1.4 2022/02/19 18:25:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscselfilluminationdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>

AuScSelfIlluminationDlg::AuScSelfIlluminationDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Self Illumination"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	int emission = g_Au.Settings().readNumEntry("Widget/SelfIlluminationDlg/SelfIllumination", 0);
	if(emission < 0)
		emission = 0;
	else if(emission > 100)
		emission = 100;

	QLabel* lbselfillum = new QLabel(tr("Enter the self illumination of the surface."), this);
	lbselfillum->setWordWrap(true);

	QLabel* lbmin = new QLabel(tr("None"), this);
	lbmin->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

	QLabel* lbmax = new QLabel(tr("Full"), this);
	lbmax->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

	m_SelfIllumSlider = new QSlider(Qt::Horizontal, this);
	m_SelfIllumSlider->setRange(0,100);
	m_SelfIllumSlider->setValue(emission);
	m_SelfIllumSlider->setToolTip(tr("<html><b>Self Illumination</b><br>"
				"<p>"
				"Adjusts self illumination. "
				"</html>"));
	connect(m_SelfIllumSlider, SIGNAL(valueChanged(int)), SLOT(SelfIlluminationChanged(int)));

	m_SelfIllumValue = new QLabel(QString("%1%").arg(emission), this);

	QPushButton* pbnoemit = new QPushButton(tr("None"), this);
	pbnoemit->setToolTip(tr("<html>"
				"Removes self illumination from the selected faces."
				"</html>"));
	connect(pbnoemit, SIGNAL(clicked()), SLOT(DoNoSelfIllumination()));

	QPushButton* pbemit = new QPushButton(tr("Illuminate"), this);
	pbemit->setAutoDefault(true);
	pbemit->setDefault(true);
	pbemit->setToolTip(tr("<html>"
				"Applies self illumination to the selected faces."
				"</html>"));
	connect(pbemit, SIGNAL(clicked()), SLOT(DoSelfIllumination()));

	QHBoxLayout* slblay = new QHBoxLayout;
	slblay->addWidget(lbmin);
	slblay->addStretch();
	slblay->addWidget(lbmax);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(m_SelfIllumSlider);
	slay->addWidget(m_SelfIllumValue);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbnoemit);
	blay->addWidget(pbemit);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbselfillum);
	lay->addLayout(slblay);
	lay->addLayout(slay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScSelfIlluminationDlg::~AuScSelfIlluminationDlg()
{
}

void AuScSelfIlluminationDlg::setVisible(bool visible)
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

void AuScSelfIlluminationDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SelfIlluminationDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SelfIlluminationDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SelfIlluminationDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SelfIlluminationDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScSelfIlluminationDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SelfIlluminationDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SelfIlluminationDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SelfIlluminationDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SelfIlluminationDlg/SizeY",sz.height());
}

void AuScSelfIlluminationDlg::DoNoSelfIllumination()
{
	m_SelfIllumSlider->blockSignals(true);
	m_SelfIllumSlider->setValue(0);
	m_SelfIllumSlider->blockSignals(false);

	m_SelfIllumValue->setText("0%");

	accept();
}

void AuScSelfIlluminationDlg::DoSelfIllumination()
{
	g_Au.Settings().writeEntry("Widget/SelfIlluminationDlg/SelfIllumination", m_SelfIllumSlider->value());

	accept();
}

float AuScSelfIlluminationDlg::SelfIllumination() const
{
	int val = m_SelfIllumSlider->value();
	if(val == 100)
		return 1;
	return (float)val/100;
}

void AuScSelfIlluminationDlg::SelfIlluminationChanged(int value)
{
	m_SelfIllumValue->setText(QString("%1%").arg(value));
}
