//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausclaunchtubejigdlg.cpp,v 1.3 2022/02/19 18:25:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausclaunchtubejigdlg.h"
//
#include "auclientglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScLaunchTubeJigDlg::AuScLaunchTubeJigDlg(QWidget* parent) : AuDialog(parent)
{
	m_Mode = HeightEntered;

	setAuDlgTitle(tr("Launch Tube Jig"));
	setAuDlgIcon(QIcon(":data/ScJig.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>Enter the length of the jig, in units depending upon your <i>unit of measure</i> setting. "
				"This can be determined by referencing part of the model.</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_HeightEdit = new QLineEdit(g_Au.Settings().readEntry("Widget/LaunchTubeJigDlg/Height", "1"), this);

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
				"Extrudes the shape to the length entered. "
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

AuScLaunchTubeJigDlg::~AuScLaunchTubeJigDlg()
{
}

void AuScLaunchTubeJigDlg::setVisible(bool visible)
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

void AuScLaunchTubeJigDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/LaunchTubeJigDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/LaunchTubeJigDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/LaunchTubeJigDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/LaunchTubeJigDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScLaunchTubeJigDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/LaunchTubeJigDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/LaunchTubeJigDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/LaunchTubeJigDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/LaunchTubeJigDlg/SizeY",sz.height());
}

void AuScLaunchTubeJigDlg::DoHeightEntered()
{
	g_Au.Settings().writeEntry("Widget/LaunchTubeJigDlg/Height", m_HeightEdit->text());
	m_Mode = HeightEntered;
	accept();
}

void AuScLaunchTubeJigDlg::DoPickHeight()
{
	m_Mode = PickHeight;
	accept();
}

void AuScLaunchTubeJigDlg::DoPickVector()
{
	m_Mode = PickVector;
	accept();
}

double AuScLaunchTubeJigDlg::Height() const
{
	return m_HeightEdit->text().toDouble() * g_AuClient.DesignerUnitScale();	// Convert to feet, which is our internal base unit.
}
