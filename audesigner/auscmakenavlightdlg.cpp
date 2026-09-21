//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmakenavlightdlg.cpp,v 1.4 2022/02/19 18:25:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscmakenavlightdlg.h"
//
#include "auglobal.h"
#include "aulightcolorwidget.h"
#include "aupushbutton.h"
#include <QLabel>
#include <QPushButton>

AuScMakeNavLightDlg::AuScMakeNavLightDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Make Navigation Light"));
	setAuDlgIcon(QIcon(":data/ScNavLight.png"));

	QLabel* lbinfo = new QLabel(tr("Choose a color for the navigation light lenses."), this);
	lbinfo->setWordWrap(true);

	QColor color = g_Au.Settings().readColorEntry("Widget/MakeNavLightDlg/Color", QColor(255,255,255));

	// Light color hue selector.
	m_Color = new AuLightColorWidget(this);
	m_Color->SetLightColor(color);
	m_Color->setToolTip(tr("<html><b>Light Color</b><br>"
				"Sets the color of the light."
				"</html>"));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Converts the selected parts into nav light lenses."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoOk()));

	QHBoxLayout* blay = new QHBoxLayout;
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

AuScMakeNavLightDlg::~AuScMakeNavLightDlg()
{
}

void AuScMakeNavLightDlg::setVisible(bool visible)
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

void AuScMakeNavLightDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/MakeNavLightDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/MakeNavLightDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/MakeNavLightDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/MakeNavLightDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScMakeNavLightDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/MakeNavLightDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/MakeNavLightDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/MakeNavLightDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/MakeNavLightDlg/SizeY",sz.height());
}

void AuScMakeNavLightDlg::DoOk()
{
	g_Au.Settings().writeEntry(QString("Widget/MakeNavLightDlg/Color"), Color());
	accept();
}

QColor AuScMakeNavLightDlg::Color() const
{
	return m_Color->LightColor();
}
