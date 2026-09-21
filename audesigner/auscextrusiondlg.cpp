//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscextrusiondlg.cpp,v 1.4 2022/02/19 18:25:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscextrusiondlg.h"
//
#include "auclientglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

AuScExtrusionDlg::AuScExtrusionDlg(quint8 partType, QWidget* parent) : AuDialog(parent)
{
	m_Mode = HeightEntered;
	m_PartType = partType;

	QString lastHeight = g_Au.Settings().readEntry(QString("Widget/ExtrusionDlg/Height%1").arg(partType), "1");
	int lastSteps = g_Au.Settings().readNumEntry(QString("Widget/ExtrusionDlg/Steps%1").arg(partType), 1);
	if(lastSteps < 1)
		lastSteps = 1;

	setAuDlgTitle(tr("Extrusion"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbsteps = new QLabel(tr("Enter the number of steps. The height of the extrusion will be divided evenly into steps."),
				this);
	lbsteps->setWordWrap(true);

	m_StepsSpin = new QSpinBox(this);
	m_StepsSpin->setMinimum(1);
	m_StepsSpin->setValue(lastSteps);
	m_StepsSpin->setToolTip(tr("<html>"
				"Extrusion Steps"
				"</html>"));

	QLabel* lbinfo = new QLabel(tr("Enter the height of the extrusion, in units depending upon your <i>unit of measure</i> setting. "
				"This can be determined by snapping to part of the model."), this);
	lbinfo->setWordWrap(true);

	m_HeightEdit = new QLineEdit(lastHeight, this);

	QPushButton* pbgetht = new QPushButton(tr("Pick Height"), this);
	pbgetht->setToolTip(tr("<html>"
				"The extrusion height will be determined by entering a 3D location. "
				"The cursor can be used to snap to existing geometry. "
				"The extrusion will be straight up or down, to the height of the coordinate entered."
				"</html>"));
	connect(pbgetht, SIGNAL(clicked()), SLOT(DoPickHeight()));

	QPushButton* pbgetdir = new QPushButton(tr("Pick Direction"), this);
	pbgetdir->setToolTip(tr("<html>"
				"The extrusion direction will be determined by entering a 3D location. "
				"The cursor can be used to snap to existing geometry. "
				"The extrusion will go in the direction of the location entered."
				"</html>"));
	connect(pbgetdir, SIGNAL(clicked()), SLOT(DoPickVector()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Extrudes the boundary to the height entered. "
				"The extrusion will be straight up or down, to the height of the coordinate entered."
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
	lay->addWidget(lbsteps);
	lay->addWidget(m_StepsSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScExtrusionDlg::~AuScExtrusionDlg()
{
}

void AuScExtrusionDlg::setVisible(bool visible)
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

void AuScExtrusionDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ExtrusionDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ExtrusionDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ExtrusionDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ExtrusionDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScExtrusionDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ExtrusionDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ExtrusionDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ExtrusionDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ExtrusionDlg/SizeY",sz.height());
}

void AuScExtrusionDlg::DoHeightEntered()
{
	g_Au.Settings().writeEntry(QString("Widget/ExtrusionDlg/Steps%1").arg(m_PartType), m_StepsSpin->value());
	g_Au.Settings().writeEntry(QString("Widget/ExtrusionDlg/Height%1").arg(m_PartType), m_HeightEdit->text());
	m_Mode = HeightEntered;
	accept();
}

void AuScExtrusionDlg::DoPickHeight()
{
	g_Au.Settings().writeEntry(QString("Widget/ExtrusionDlg/Steps%1").arg(m_PartType), m_StepsSpin->value());
	m_Mode = PickHeight;
	accept();
}

void AuScExtrusionDlg::DoPickVector()
{
	g_Au.Settings().writeEntry(QString("Widget/ExtrusionDlg/Steps%1").arg(m_PartType), m_StepsSpin->value());
	m_Mode = PickVector;
	accept();
}

double AuScExtrusionDlg::Height() const
{
	return m_HeightEdit->text().toDouble() * g_AuClient.DesignerUnitScale();	// Convert to feet, which is our internal base unit.
}

int AuScExtrusionDlg::Steps() const
{
	return m_StepsSpin->value();
}

