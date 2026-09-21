//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausccopysymmetrical.cpp,v 1.3 2022/02/19 18:25:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausccopysymmetrical.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScCopySymmetrical::AuScCopySymmetrical(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Copy Symmetrical"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbfig = new QLabel(this);
	lbfig->setPixmap(QPixmap(":data/ScFigCopySymmetrical.png"));

	QLabel* lbaxis = new QLabel(tr("<html>"
				"Choose the axis of the symmetrical copy. "
				"The selected parts will be copied along the chosen axis. "
				"</html>"), this);
	lbaxis->setWordWrap(true);

	int lastAxis = g_Au.Settings().readNumEntry(QString("Widget/CopySymmetrical/Axis"), 0);

	m_XAxis = new QRadioButton(tr("X Axis"), this);
	m_XAxis->setChecked(lastAxis == 0);

	m_YAxis = new QRadioButton(tr("Y Axis"), this);
	m_YAxis->setChecked(lastAxis == 1);

	m_ZAxis = new QRadioButton(tr("Z Axis"), this);
	m_ZAxis->setChecked(lastAxis == 2);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Copies the selected parts along the chosen axis. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoAxis()));

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

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbfig, row,0, 3,1);
	lay->addWidget(lbaxis, row,1);
	row++;
	lay->addLayout(axislay, row,1);
	row++;
	lay->addLayout(blay, row,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScCopySymmetrical::~AuScCopySymmetrical()
{
}

void AuScCopySymmetrical::setVisible(bool visible)
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

void AuScCopySymmetrical::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CopySymmetrical/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CopySymmetrical/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CopySymmetrical/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CopySymmetrical/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScCopySymmetrical::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CopySymmetrical/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CopySymmetrical/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CopySymmetrical/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CopySymmetrical/SizeY",sz.height());
}

void AuScCopySymmetrical::DoAxis()
{
	accept();
}

int AuScCopySymmetrical::Axis() const
{
	int axis;
	if(m_XAxis->isChecked())
		axis = 0;
	else if(m_YAxis->isChecked())
		axis = 1;
	else
		axis = 2;
	g_Au.Settings().writeEntry(QString("Widget/CopySymmetrical/Axis"), axis);
	return axis;
}
