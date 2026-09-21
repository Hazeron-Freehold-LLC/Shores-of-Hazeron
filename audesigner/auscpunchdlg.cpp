//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpunchdlg.cpp,v 1.3 2022/02/19 18:25:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpunchdlg.h"
//
#include "auclientglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

AuScPunchDlg::AuScPunchDlg(QWidget* parent) : AuDialog(parent)
{
	m_Mode = RadiusEntered;

	setAuDlgTitle(tr("Punch Round Hole"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("Enter the radius of the round hole, in units depending upon your <i>unit of measure</i> setting. "
				"This can be determined by snapping to part of the model."), this);
	lbinfo->setWordWrap(true);

	m_RadiusEdit = new QLineEdit(g_Au.Settings().readEntry(QString("Widget/PunchDlg/Radius"), "1"), this);

	QLabel* lbsteps = new QLabel(tr("Enter the number of steps around the hole."), this);
	lbsteps->setWordWrap(true);

	m_StepsSpin = new QSpinBox(this);
	m_StepsSpin->setRange(3, 1000);
	m_StepsSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/PunchDlg/Steps"), 10));

	QPushButton* pbpickit = new QPushButton(tr("Pick Radius"), this);
	pbpickit->setToolTip(tr("<html>"
				"Hole radius will be determined by entering a 3D location. "
				"The cursor can be used to snap to existing geometry. "
				"</html>"));
	connect(pbpickit, SIGNAL(clicked()), SLOT(DoPickRadius()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Cuts a round hole into faces that is the radius entered."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoRadiusEntered()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbpickit);
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_RadiusEdit);
	lay->addWidget(lbsteps);
	lay->addWidget(m_StepsSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScPunchDlg::~AuScPunchDlg()
{
}

void AuScPunchDlg::setVisible(bool visible)
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

void AuScPunchDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PunchDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PunchDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PunchDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PunchDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPunchDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PunchDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PunchDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PunchDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PunchDlg/SizeY",sz.height());
}

void AuScPunchDlg::DoRadiusEntered()
{
	g_Au.Settings().writeEntry(QString("Widget/PunchDlg/Radius"), m_RadiusEdit->text());
	g_Au.Settings().writeEntry(QString("Widget/PunchDlg/Steps"), m_StepsSpin->value());
	m_Mode = RadiusEntered;
	accept();
}

void AuScPunchDlg::DoPickRadius()
{
	g_Au.Settings().writeEntry(QString("Widget/PunchDlg/Steps"), m_StepsSpin->value());
	m_Mode = PickRadius;
	accept();
}

double AuScPunchDlg::Radius() const
{
	return m_RadiusEdit->text().toDouble() * g_AuClient.DesignerUnitScale();	// Convert to feet, which is our internal base unit.
}

quint16 AuScPunchDlg::Steps() const
{
	return m_StepsSpin->value();
}
