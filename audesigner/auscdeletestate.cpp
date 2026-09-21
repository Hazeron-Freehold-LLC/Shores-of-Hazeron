//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdeletestate.cpp,v 1.2 2022/02/19 18:25:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscdeletestate.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

AuScDeleteState::AuScDeleteState(const QMap<quint8, QString>& deletableStates, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Delete State"));
	setAuDlgIcon(QIcon(":data/ScDeleteState.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Choose which of the states to delete. "
				"</p>"

				"<p>"
				"Doors do not allow the <i>Closed</i> state to be deleted. "
				"A door with no <i>Open</i> state will disappear when opened; "
				"it reappears when closed. "
				"</p>"

				"<p>"
				"Landing gear does not allow the <i>Extended</i> state to be deleted. "
				"Landing gear with no <i>Retracted</i> state will disappear when retracted; "
				"it reappears when extended. "
				"</p>"
				"</html>"), this);

	m_States = new QComboBox(this);
	for(QMap<quint8, QString>::const_iterator si = deletableStates.cbegin(); si != deletableStates.cend(); si++)
		m_States->addItem(*si, si.key());

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Deletes the state."));
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

AuScDeleteState::~AuScDeleteState()
{
}

void AuScDeleteState::setVisible(bool visible)
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

void AuScDeleteState::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScDeleteState/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScDeleteState/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScDeleteState/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScDeleteState/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScDeleteState::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScDeleteState/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScDeleteState/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScDeleteState/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScDeleteState/SizeY",sz.height());
}

quint8 AuScDeleteState::State() const
{
	return m_States->itemData(m_States->currentIndex()).toUInt();
}
