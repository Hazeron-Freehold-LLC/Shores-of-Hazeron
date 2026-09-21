//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscopacitydlg.cpp,v 1.4 2022/02/19 18:25:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscopacitydlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>

AuScOpacityDlg::AuScOpacityDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Transparency"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("Choose the transparency for the faces."), this);
	lbinfo->setWordWrap(true);

	double opacity = g_Au.Settings().readNumEntry("Widget/OpacityDlg/Opacity", .25);

	QLabel* lbmin = new QLabel(tr("Opaque"), this);
	lbmin->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

	QLabel* lbmax = new QLabel(tr("Invisible"), this);
	lbmax->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

	m_TransparencySlider = new QSlider(Qt::Horizontal, this);
	m_TransparencySlider->setRange(0,255);
	m_TransparencySlider->setValue((int)(255 - opacity * 255));
	m_TransparencySlider->setToolTip(tr("<html><b>Transparency</b><br>"
				"Adjusts the transparency for the selected faces."
				"</html>"));
	connect(m_TransparencySlider, SIGNAL(valueChanged(int)), SLOT(TransparencyChanged(int)));

	m_TransparencyValue = new QLabel(QString("%1%").arg((int)(100 - opacity*100)), this);

	QPushButton* pbopaque = new QPushButton(tr("Opaque"), this);
	pbopaque->setToolTip(tr("<html>"
				"Sets the selected surfaces to opaque."
				"</html>"));
	connect(pbopaque, SIGNAL(clicked()), SLOT(DoOpaque()));

	QPushButton* pbtransparent = new QPushButton(tr("Transparent"), this);
	pbtransparent->setAutoDefault(true);
	pbtransparent->setDefault(true);
	pbtransparent->setToolTip(tr("<html>"
				"Applies the transparency to the selected faces."
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
	blay->addWidget(pbopaque);
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

AuScOpacityDlg::~AuScOpacityDlg()
{
}

void AuScOpacityDlg::setVisible(bool visible)
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

void AuScOpacityDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/OpacityDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/OpacityDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/OpacityDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/OpacityDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScOpacityDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/OpacityDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/OpacityDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/OpacityDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/OpacityDlg/SizeY",sz.height());
}

void AuScOpacityDlg::DoOpaque()
{
	m_TransparencySlider->blockSignals(true);
	m_TransparencySlider->setValue(0);
	m_TransparencySlider->blockSignals(false);

	accept();
}

void AuScOpacityDlg::DoTransparent()
{
	g_Au.Settings().writeEntry(QString("Widget/OpacityDlg/Opacity"), Opacity());
	accept();
}

float AuScOpacityDlg::Opacity() const
{
	return 1.f - (float)m_TransparencySlider->value() / 255.f;
}

void AuScOpacityDlg::TransparencyChanged(int value)
{
	int tv = value * 100 / 255;
	m_TransparencyValue->setText(QString("%1%").arg(tv));
}
