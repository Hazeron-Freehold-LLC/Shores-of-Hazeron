//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctubingdlg.cpp,v 1.3 2022/02/19 18:26:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausctubingdlg.h"
//
#include "auclientglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

AuScTubingDlg::AuScTubingDlg(quint8 partType, QWidget* parent) : AuDialog(parent)
{
	m_Mode = DimensionsEntered;
	m_PartType = partType;

	setAuDlgTitle(tr("Tubing"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Circular or square tubing is made by entering the diameter and number of sides. "
				"Other tubing shapes are made by pressing <b>Draw Tube Shape</b>. "
				"</p>"

				"<p>"
				"Enter diameter, in units depending upon your <i>unit of measure</i> setting. "
				"Fractional values are ok."
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_DiameterEdit = new QLineEdit(g_Au.Settings().readEntry(QString("Widget/TubingDlg/Diameter%1").arg(partType), "1"), this);

	QLabel* lbspin = new QLabel(tr("Enter the number of sides, 4 for square tubing, 3 for Toblerones."), this);
	lbspin->setWordWrap(true);

	m_SidesSpin = new QSpinBox(this);
	m_SidesSpin->setRange(3, 100);
	m_SidesSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/TubingDlg/Sides%1").arg(partType), 12));

	QPushButton* pbdraw = new QPushButton(tr("Draw Tube Shape"), this);
	pbdraw->setToolTip(tr("<html>"
				"The shape of the tube will be determined by drawing its outline. "
				"The center of the grid corresponds to the tubing path. "
				"The grid center follows the tubing path as the tube is made. "
				"</html>"));
	connect(pbdraw, SIGNAL(clicked()), SLOT(DoDrawShape()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Creates circular tubing with the diameter and sides entered. "
				"The tubing will be centered on the path entered. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoDimensionsEntered()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbdraw);
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_DiameterEdit);
	lay->addWidget(lbspin);
	lay->addWidget(m_SidesSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScTubingDlg::~AuScTubingDlg()
{
}

void AuScTubingDlg::setVisible(bool visible)
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

void AuScTubingDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/TubingDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/TubingDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/TubingDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/TubingDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScTubingDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/TubingDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/TubingDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/TubingDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/TubingDlg/SizeY",sz.height());
}

void AuScTubingDlg::DoDimensionsEntered()
{
	g_Au.Settings().writeEntry(QString("Widget/TubingDlg/Diameter%1").arg(m_PartType), m_DiameterEdit->text());
	g_Au.Settings().writeEntry(QString("Widget/TubingDlg/Sides%1").arg(m_PartType), m_SidesSpin->value());
	m_Mode = DimensionsEntered;
	accept();
}

void AuScTubingDlg::DoDrawShape()
{
	m_Mode = DrawShape;
	accept();
}

double AuScTubingDlg::Diameter() const
{
	return m_DiameterEdit->text().toDouble() * g_AuClient.DesignerUnitScale();	// Convert to feet, which is our internal base unit.
}

int AuScTubingDlg::Sides() const
{
	return m_SidesSpin->value();
}

bool AuScTubingDlg::SmoothSides() const
{
	return true;
}
