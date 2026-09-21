//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscroompropsdlg.cpp,v 1.3 2022/02/19 18:25:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscroompropsdlg.h"
//
#include "auclientglobal.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScRoomPropsDlg::AuScRoomPropsDlg(QWidget* parent) : AuDialog(parent)
{
	bool lastSun = g_Au.Settings().readBoolEntry(QString("Widget/RoomPropsDlg/IsSunlit"), true);
	int lastLight = g_Au.Settings().readNumEntry(QString("Widget/RoomPropsDlg/InitialLighting"), INTLIGHT_Bright);
	if(lastLight < INTLIGHT_Dark || lastLight > INTLIGHT_Dim)
		lastLight = INTLIGHT_Bright;

	setAuDlgTitle(tr("Room Properties"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbsun = new QLabel(tr("<html>"
				"Rooms with windows or openings to the outside should permit sunlight to illuminate their interior. "
				"This affects the normal lighting state of the room geometry and its contents. "
				"Room lighting is not affected by opening or closing of doors. "
				"</html>"), this);
	lbsun->setWordWrap(true);
	m_IsSunlit = new QCheckBox(tr("Interior is sunlit."), this);
	m_IsSunlit->setCheckState(lastSun?Qt::Checked:Qt::Unchecked);

	QLabel* lblight = new QLabel(tr("<html>"
				"Configure the initial state of lighting in the room. "
				"Room lights will be set accordingly, when the design is manufactured. "
				"</html>"), this);
	lblight->setWordWrap(true);

	m_InitLight = new QComboBox(this);
	m_InitLight->addItem(tr("On"), INTLIGHT_Bright);
	m_InitLight->addItem(tr("Dim"), INTLIGHT_Dim);
	m_InitLight->addItem(tr("Off"), INTLIGHT_Dark);
	m_InitLight->setCurrentIndex(m_InitLight->findData(lastLight));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Lighting will be configured on the selected room(s). "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoProps()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbsun);
	lay->addWidget(m_IsSunlit);
	lay->addWidget(lblight);
	lay->addWidget(m_InitLight);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScRoomPropsDlg::~AuScRoomPropsDlg()
{
}

void AuScRoomPropsDlg::setVisible(bool visible)
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

void AuScRoomPropsDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/RoomPropsDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/RoomPropsDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/RoomPropsDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/RoomPropsDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScRoomPropsDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/RoomPropsDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/RoomPropsDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/RoomPropsDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/RoomPropsDlg/SizeY",sz.height());
}

void AuScRoomPropsDlg::DoProps()
{
	g_Au.Settings().writeEntry("Widget/RoomPropsDlg/IsSunlit", IsSunlit());
	g_Au.Settings().writeEntry("Widget/RoomPropsDlg/InitialLighting", InitialLighting());
	accept();
}

bool AuScRoomPropsDlg::IsSunlit() const
{
	return m_IsSunlit->checkState() == Qt::Checked;
}

INTERIOR_LIGHT AuScRoomPropsDlg::InitialLighting() const
{
	int li = m_InitLight->currentIndex();
	if(li < 0)
		return INTLIGHT_Bright;

	INTERIOR_LIGHT light = (INTERIOR_LIGHT)m_InitLight->itemData(li).toInt();
	return light;
}
