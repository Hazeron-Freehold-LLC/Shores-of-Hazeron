//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscchangestopdlg.cpp,v 1.2 2022/02/19 18:25:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscchangestopdlg.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

AuScChangeStopDlg::AuScChangeStopDlg(const QStringList& deleteableStops, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Change Turbo Lift Stop"));
	setAuDlgIcon(QIcon(":data/ScChangeStop.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Choose which of the turbo lift stops to move the car. "
				"</p>"
				"</html>"), this);

	m_Stops = new QComboBox(this);
	for(QStringList::const_iterator si = deleteableStops.cbegin(); si != deleteableStops.cend(); si++)
		m_Stops->addItem(*si);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Change the turbo lift car to a different stop."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_Stops);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScChangeStopDlg::~AuScChangeStopDlg()
{
}

void AuScChangeStopDlg::setVisible(bool visible)
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

void AuScChangeStopDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScChangeStopDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScChangeStopDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScChangeStopDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScChangeStopDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScChangeStopDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScChangeStopDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScChangeStopDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScChangeStopDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScChangeStopDlg/SizeY",sz.height());
}

QString AuScChangeStopDlg::StopName() const
{
	return m_Stops->itemText(m_Stops->currentIndex());
}
