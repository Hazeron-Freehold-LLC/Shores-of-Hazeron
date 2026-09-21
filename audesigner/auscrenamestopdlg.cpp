//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscrenamestopdlg.cpp,v 1.2 2022/02/19 18:25:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscrenamestopdlg.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScRenameStopDlg::AuScRenameStopDlg(const QStringList& deleteableStops, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Rename Turbo Lift Stop"));
	setAuDlgIcon(QIcon(":data/ScRenameStop.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Choose which of the turbo lift stops to rename. "
				"</p>"
				"</html>"), this);

	m_Stops = new QComboBox(this);
	for(QStringList::const_iterator si = deleteableStops.cbegin(); si != deleteableStops.cend(); si++)
		m_Stops->addItem(*si);

	m_NewName = new QLineEdit(this);
	m_NewName->setToolTip(tr("Enter a new name for the stop. "
				"The new name must not match any existing stop of the lift. "));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Renames the turbo lift stop."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_Stops);
	lay->addWidget(m_NewName);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScRenameStopDlg::~AuScRenameStopDlg()
{
}

void AuScRenameStopDlg::setVisible(bool visible)
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

void AuScRenameStopDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/RenameStopDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/RenameStopDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/RenameStopDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/RenameStopDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScRenameStopDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/RenameStopDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/RenameStopDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/RenameStopDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/RenameStopDlg/SizeY",sz.height());
}

QString AuScRenameStopDlg::OldStopName() const
{
	return m_Stops->itemText(m_Stops->currentIndex());
}

QString AuScRenameStopDlg::NewStopName() const
{
	return m_NewName->text().trimmed();
}
