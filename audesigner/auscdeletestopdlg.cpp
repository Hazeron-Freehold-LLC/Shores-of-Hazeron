//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdeletestopdlg.cpp,v 1.2 2022/02/19 18:25:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscdeletestopdlg.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

AuScDeleteStopDlg::AuScDeleteStopDlg(const QStringList& deleteableStops, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Delete Turbo Lift Stop"));
	setAuDlgIcon(QIcon(":data/ScDeleteStop.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Choose which of the turbo lift stops to delete. "
				"</p>"

				"<p>"
				"The turbo lift part is not deleted even if the last stop is deleted. "
				"</p>"
				"</html>"), this);

	m_Stops = new QComboBox(this);
	for(QStringList::const_iterator si = deleteableStops.cbegin(); si != deleteableStops.cend(); si++)
		m_Stops->addItem(*si);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Deletes the turbo lift stop."));
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

AuScDeleteStopDlg::~AuScDeleteStopDlg()
{
}

void AuScDeleteStopDlg::setVisible(bool visible)
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

void AuScDeleteStopDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScDeleteStopDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScDeleteStopDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScDeleteStopDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScDeleteStopDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScDeleteStopDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScDeleteStopDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScDeleteStopDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScDeleteStopDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScDeleteStopDlg/SizeY",sz.height());
}

QString AuScDeleteStopDlg::StopName() const
{
	return m_Stops->itemText(m_Stops->currentIndex());
}
