//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmakerotatingbeacondlg.cpp,v 1.3 2022/02/19 18:25:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscmakerotatingbeacondlg.h"
//
#include "auglobal.h"
#include "aulightcolorwidget.h"
#include "aupushbutton.h"
#include "auscparts.h"
#include <QLabel>
#include <QPushButton>

AuScMakeRotatingBeaconDlg::AuScMakeRotatingBeaconDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Make Rotating Beacon"));
	setAuDlgIcon(QIcon(":data/MsgSOS.png"));

	QLabel* lbinfo = new QLabel(tr("Choose a color for the rotating beacon light."), this);
	lbinfo->setWordWrap(true);

	AuScRotatingBeaconLens light;
	QColor color = g_Au.Settings().readColorEntry("Widget/MakeRotatingBeaconDlg/Color", light.LightColor());

	// Light color hue selector.
	m_Color = new AuLightColorWidget(this);
	m_Color->SetLightColor(color);
	m_Color->setToolTip(tr("<html><b>Light Color</b><br>"
				"Sets the color of the light."
				"</html>"));

	QPushButton* pbdef = new QPushButton(tr("Default Color"), this);
	pbdef->setToolTip(tr("<html>"
				"Selects the default color for the light. "
				"</html>"));
	connect(pbdef, SIGNAL(clicked()), SLOT(DefaultColor()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Converts the selected parts into rotating beacon lenses."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoOk()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(pbdef);
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_Color);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScMakeRotatingBeaconDlg::~AuScMakeRotatingBeaconDlg()
{
}

void AuScMakeRotatingBeaconDlg::setVisible(bool visible)
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

void AuScMakeRotatingBeaconDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/MakeRotatingBeaconDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/MakeRotatingBeaconDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/MakeRotatingBeaconDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/MakeRotatingBeaconDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScMakeRotatingBeaconDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/MakeRotatingBeaconDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/MakeRotatingBeaconDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/MakeRotatingBeaconDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/MakeRotatingBeaconDlg/SizeY",sz.height());
}

void AuScMakeRotatingBeaconDlg::DoOk()
{
	g_Au.Settings().writeEntry(QString("Widget/MakeRotatingBeaconDlg/Color"), Color());
	accept();
}

QColor AuScMakeRotatingBeaconDlg::Color() const
{
	return m_Color->LightColor();
}

void AuScMakeRotatingBeaconDlg::DefaultColor()
{
	AuScRotatingBeaconLens light;
	m_Color->SetLightColor(light.LightColor());
}
