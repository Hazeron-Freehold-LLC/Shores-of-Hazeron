//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscaddstopdlg.cpp,v 1.3 2022/02/19 18:25:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscaddstopdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScAddStopDlg::AuScAddStopDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Add Turbo Lift Stop"));
	setAuDlgIcon(QIcon(":data/ScAddStop.png"));

	QLabel* lbinfo = new QLabel(tr("Enter the name of the stop at the turbo lift's current location. "
				"Each stop must have a unique name. "
				"If the stop already exists, it will be replaced."), this);
	lbinfo->setWordWrap(true);

	m_StopName = new QLineEdit("", this);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Accepts the name for the turbo lift stop."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_StopName);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScAddStopDlg::~AuScAddStopDlg()
{
}

void AuScAddStopDlg::setVisible(bool visible)
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

void AuScAddStopDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AddStopDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AddStopDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AddStopDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AddStopDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScAddStopDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AddStopDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AddStopDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AddStopDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AddStopDlg/SizeY",sz.height());
}

QString AuScAddStopDlg::StopName() const
{
	QString s = m_StopName->text();
	if(!IsNameValid(s))
		return "";
	return s;
}
