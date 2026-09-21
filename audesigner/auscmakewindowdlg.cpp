//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmakewindowdlg.cpp,v 1.4 2022/02/19 18:25:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscmakewindowdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

AuScMakeWindowDlg::AuScMakeWindowDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Make Window"));
	setAuDlgIcon(QIcon(":data/ScWindow.png"));

	QLabel* lbinfo = new QLabel(tr("Choose the transparency of the window."), this);
	lbinfo->setWordWrap(true);

	double opacity = g_Au.Settings().readNumEntry("Widget/MakeWindowDlg/Opacity", .25);

	QLabel* lbmin = new QLabel(tr("Translucent"), this);
	lbmin->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

	QLabel* lbmax = new QLabel(tr("Invisible"), this);
	lbmax->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

	m_TransparencySlider = new QSlider(Qt::Horizontal, this);
	m_TransparencySlider->setRange(0,255);
	m_TransparencySlider->setValue((int)(255 - opacity * 255));
	m_TransparencySlider->setToolTip(tr("<html><b>Window Transparency</b><br>"
				"Adjusts the transparency for the selected faces."
				"</html>"));
	connect(m_TransparencySlider, SIGNAL(valueChanged(int)), SLOT(TransparencyChanged(int)));

	m_TransparencyValue = new QLabel(QString("%1%").arg((int)(100 - opacity*100)), this);

	QPushButton* pbtranslucent = new QPushButton(tr("Translucent"), this);
	pbtranslucent->setToolTip(tr("<html>"
				"Converts the selected parts into translucent windows."
				"</html>"));
	connect(pbtranslucent, SIGNAL(clicked()), SLOT(DoTranslucent()));

	QPushButton* pbtransparent = new QPushButton(tr("Transparent"), this);
	pbtransparent->setAutoDefault(true);
	pbtransparent->setDefault(true);
	pbtransparent->setToolTip(tr("<html>"
				"Converts the selected parts into transparent windows."
				"</html>"));
	connect(pbtransparent, SIGNAL(clicked()), SLOT(DoTransparent()));

	QHBoxLayout* slblay = new QHBoxLayout;
	slblay->addWidget(lbmin);
	slblay->addStretch();
	slblay->addWidget(lbmax);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(m_TransparencySlider);
	slay->addWidget(m_TransparencyValue);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbtranslucent);
	blay->addWidget(pbtransparent);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addLayout(slblay);
	lay->addLayout(slay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScMakeWindowDlg::~AuScMakeWindowDlg()
{
}

void AuScMakeWindowDlg::setVisible(bool visible)
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

void AuScMakeWindowDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/MakeWindowDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/MakeWindowDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/MakeWindowDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/MakeWindowDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScMakeWindowDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/MakeWindowDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/MakeWindowDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/MakeWindowDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/MakeWindowDlg/SizeY",sz.height());
}

void AuScMakeWindowDlg::DoTranslucent()
{
	m_TransparencySlider->blockSignals(true);
	m_TransparencySlider->setValue(0);
	m_TransparencySlider->blockSignals(false);

	accept();
}

void AuScMakeWindowDlg::DoTransparent()
{
	g_Au.Settings().writeEntry(QString("Widget/MakeWindowDlg/Opacity"), Opacity());
	accept();
}

float AuScMakeWindowDlg::Opacity() const
{
	return 1.f - (float)m_TransparencySlider->value() / 255.f;
}

void AuScMakeWindowDlg::TransparencyChanged(int value)
{
	int tv = value * 100 / 255;
	m_TransparencyValue->setText(QString("%1%").arg(tv));
}
