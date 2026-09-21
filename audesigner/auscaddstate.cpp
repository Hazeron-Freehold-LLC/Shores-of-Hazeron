//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscaddstate.cpp,v 1.2 2022/02/19 18:25:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscaddstate.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

AuScAddState::AuScAddState(const QMap<quint8, QString>& addableStates, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Add State"));
	setAuDlgIcon(QIcon(":data/ScAddState.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Choose from the available states to add."
				"</p>"
				"</html>"), this);

	m_States = new QComboBox(this);
	for(QMap<quint8, QString>::const_iterator si = addableStates.cbegin(); si != addableStates.cend(); si++)
		m_States->addItem(*si, si.key());

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Accepts the new state."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_States);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScAddState::~AuScAddState()
{
}

void AuScAddState::setVisible(bool visible)
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

void AuScAddState::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScAddState/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScAddState/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScAddState/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScAddState/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScAddState::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScAddState/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScAddState/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScAddState/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScAddState/SizeY",sz.height());
}

quint8 AuScAddState::State() const
{
	return m_States->itemData(m_States->currentIndex()).toUInt();
}
