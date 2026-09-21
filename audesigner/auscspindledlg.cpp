//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscspindledlg.cpp,v 1.3 2022/02/19 18:25:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscspindledlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScSpindleDlg::AuScSpindleDlg(quint8 partType, QWidget* parent) : AuDialog(parent)
{
	m_PartType = partType;

	setAuDlgTitle(tr("Spindle"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbaxis = new QLabel(tr("Choose the axis of the spindle."), this);
	lbaxis->setWordWrap(true);

	int lastAxis = g_Au.Settings().readNumEntry(QString("Widget/SpindleDlg/Axis%1").arg(m_PartType), 0);

	m_XAxis = new QRadioButton(tr("X Axis"), this);
	m_XAxis->setChecked(lastAxis == 0);

	m_YAxis = new QRadioButton(tr("Y Axis"), this);
	m_YAxis->setChecked(lastAxis == 1);

	m_ZAxis = new QRadioButton(tr("Z Axis"), this);
	m_ZAxis->setChecked(lastAxis == 2);

	QLabel* lbinfo = new QLabel(tr("Enter the number of steps around the spindle."), this);
	lbinfo->setWordWrap(true);

	m_StepsSpin = new QSpinBox(this);
	m_StepsSpin->setRange(3, 1000);
	m_StepsSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/SpindleDlg/Steps%1").arg(m_PartType), 10));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Spins the profile for the number of steps entered."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoStepsEntered()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QGridLayout* axislay = new QGridLayout;
	axislay->addWidget(m_XAxis, 0,1);
	axislay->addWidget(m_YAxis, 1,1);
	axislay->addWidget(m_ZAxis, 2,1);
	axislay->setColumnMinimumWidth(0, 20);
	axislay->setMargin(0);
	axislay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbaxis);
	lay->addLayout(axislay);
	lay->addWidget(lbinfo);
	lay->addWidget(m_StepsSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScSpindleDlg::~AuScSpindleDlg()
{
}

void AuScSpindleDlg::setVisible(bool visible)
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

void AuScSpindleDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SpindleDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SpindleDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SpindleDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SpindleDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScSpindleDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SpindleDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SpindleDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SpindleDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SpindleDlg/SizeY",sz.height());
}

void AuScSpindleDlg::DoStepsEntered()
{
	g_Au.Settings().writeEntry(QString("Widget/SpindleDlg/Steps%1").arg(m_PartType), m_StepsSpin->value());
	accept();
}

quint16 AuScSpindleDlg::Steps() const
{
	return m_StepsSpin->value();
}

int AuScSpindleDlg::Axis() const
{
	int axis;
	if(m_XAxis->isChecked())
		axis = 0;
	else if(m_YAxis->isChecked())
		axis = 1;
	else
		axis = 2;
	g_Au.Settings().writeEntry(QString("Widget/SpindleDlg/Axis%1").arg(m_PartType), axis);
	return axis;
}
