//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdividedlg.cpp,v 1.3 2022/02/19 18:25:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscdividedlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

AuScDivideDlg::AuScDivideDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Divide Faces"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("Enter the number of times to divide the selected faces on each edge."), this);
	lbinfo->setWordWrap(true);

	m_DivisionsSpin = new QSpinBox(this);
	m_DivisionsSpin->setRange(1, 100);
	m_DivisionsSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/DivideDlg/Divisions"), 1));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Divides the selected faces for the number of times entered."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoDivisionsEntered()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_DivisionsSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScDivideDlg::~AuScDivideDlg()
{
}

void AuScDivideDlg::setVisible(bool visible)
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

void AuScDivideDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DivideDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DivideDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DivideDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DivideDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScDivideDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DivideDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DivideDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DivideDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DivideDlg/SizeY",sz.height());
}

void AuScDivideDlg::DoDivisionsEntered()
{
	g_Au.Settings().writeEntry(QString("Widget/DivideDlg/Divisions"), m_DivisionsSpin->value());
	accept();
}

quint8 AuScDivideDlg::Divisions() const
{
	return (quint8)m_DivisionsSpin->value();
}
