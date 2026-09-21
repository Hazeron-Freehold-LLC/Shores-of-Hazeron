//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscwindowjigdlg.cpp,v 1.3 2022/02/19 18:26:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscwindowjigdlg.h"
//
#include "auclientglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScWindowJigDlg::AuScWindowJigDlg(QWidget* parent) : AuDialog(parent)
{
	m_Mode = HeightEntered;

	setAuDlgTitle(tr("Window Jig"));
	setAuDlgIcon(QIcon(":data/ScJig.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>Enter the length of the jig, in units depending upon your <i>unit of measure</i> setting. "
				"This can be determined by referencing part of the model.</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_HeightEdit = new QLineEdit(g_Au.Settings().readEntry("Widget/WindowJigDlg/Height", "1"), this);

	QPushButton* pbgetht = new QPushButton(tr("Pick Length"), this);
	pbgetht->setToolTip(tr("<html>"
				"The jig length will be determined by entering a 3D location. "
				"The mouse can be used to snap to existing geometry. "
				"The jig will be straight up or down, to the length of the coordinate entered."
				"</html>"));
	connect(pbgetht, SIGNAL(clicked()), SLOT(DoPickHeight()));

	QPushButton* pbgetdir = new QPushButton(tr("Pick Direction"), this);
	pbgetdir->setToolTip(tr("<html>"
				"The jig direction will be determined by entering a 3D location. "
				"The mouse can be used to snap to existing geometry. "
				"The jig will go in the direction of the location entered."
				"</html>"));
	connect(pbgetdir, SIGNAL(clicked()), SLOT(DoPickVector()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Extrudes the boundary to the length entered. "
				"The jig will be straight up or down, to the length of the coordinate entered."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoHeightEntered()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbgetht);
	blay->addWidget(pbgetdir);
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_HeightEdit);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScWindowJigDlg::~AuScWindowJigDlg()
{
}

void AuScWindowJigDlg::setVisible(bool visible)
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

void AuScWindowJigDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/WindowJigDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/WindowJigDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/WindowJigDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/WindowJigDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScWindowJigDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/WindowJigDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/WindowJigDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/WindowJigDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/WindowJigDlg/SizeY",sz.height());
}

void AuScWindowJigDlg::DoHeightEntered()
{
	g_Au.Settings().writeEntry("Widget/WindowJigDlg/Height", m_HeightEdit->text());
	m_Mode = HeightEntered;
	accept();
}

void AuScWindowJigDlg::DoPickHeight()
{
	m_Mode = PickHeight;
	accept();
}

void AuScWindowJigDlg::DoPickVector()
{
	m_Mode = PickVector;
	accept();
}

double AuScWindowJigDlg::Height() const
{
	return m_HeightEdit->text().toDouble() * g_AuClient.DesignerUnitScale();	// Convert to feet, which is our internal base unit.
}
