//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscglowdlg.cpp,v 1.4 2022/02/19 18:25:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscglowdlg.h"
//
#include "auglobal.h"
#include "aupushbutton.h"
#include <QLabel>
#include <QPushButton>

AuScGlowDlg::AuScGlowDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Glow"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QColor col = g_Au.Settings().readColorEntry(QString("Widget/GlowDlg/Color"), QColor(255,255,255));

	QLabel* lbglow = new QLabel(tr("<html>"
				"<p>"
				"Choose a glow color. "
				"Glow color is applied at selected vertices of selected faces. "
				"</p>"

				"<p>"
				"Glow color is added to the surface in addition to light from other sources. "
				"The brightness of the color determines the amount of glow. "
				"For example, RGB(0,255,0) emits a bright green glow and RGB(0,20,0) emits a dim green glow. "
				"</p>"
				"</html>"), this);
	lbglow->setWordWrap(true);

	m_Color = new AuColorButton(this);
	m_Color->SetColor(col);

	QPushButton* pbnoglow = new QPushButton(tr("No Glow"), this);
	pbnoglow->setToolTip(tr("<html>"
				"Removes glow from the selected vertices of the selected faces."
				"</html>"));
	connect(pbnoglow, SIGNAL(clicked()), SLOT(DoNoGlow()));

	QPushButton* pbglow = new QPushButton(tr("Glow"), this);
	pbglow->setAutoDefault(true);
	pbglow->setDefault(true);
	pbglow->setToolTip(tr("<html>"
				"Applies glow color to the selected vertices of the selected faces."
				"</html>"));
	connect(pbglow, SIGNAL(clicked()), SLOT(DoGlow()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbnoglow);
	blay->addWidget(pbglow);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbglow);
	lay->addWidget(m_Color);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScGlowDlg::~AuScGlowDlg()
{
}

void AuScGlowDlg::setVisible(bool visible)
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

void AuScGlowDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GlowDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GlowDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GlowDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GlowDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScGlowDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GlowDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GlowDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GlowDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GlowDlg/SizeY",sz.height());
}

void AuScGlowDlg::DoGlow()
{
	g_Au.Settings().writeEntry("Widget/GlowDlg/Color", Color());

	accept();
}

void AuScGlowDlg::DoNoGlow()
{
	m_Color->SetColor(QColor(0,0,0));

	accept();
}

QColor AuScGlowDlg::Color() const
{
	return m_Color->Color();
}
