//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdooraccessdlg.cpp,v 1.5 2022/02/19 18:25:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscdooraccessdlg.h"
//
#include "auclientglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

AuScDoorAccessDlg::AuScDoorAccessDlg(Qt::CheckState isAuto, Qt::CheckState initLock, Qt::CheckState remoteSpacecraft,
		Qt::CheckState remoteVehicle, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Door Access"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"This window does two things. "
				"It configures settings on doors. "
				"<b>It also analyzes doors to see if they appear to be hull doors, changing them accordingly.</b>"
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	QLabel* lbdoorstd = new QLabel(tr("Standard Door"), this);

	QLabel* lbdoorstdico = new QLabel(this);
	lbdoorstdico->setPixmap(QPixmap(":data/ScDoorStd.png"));

	QLabel* lbdoorauto = new QLabel(tr("Automatic Door"), this);

	QLabel* lbdoorautoico = new QLabel(this);
	lbdoorautoico->setPixmap(QPixmap(":data/ScDoorAuto.png"));

	QLabel* lbdoorhull = new QLabel(tr("Hull Door"), this);

	QLabel* lbdoorhullico = new QLabel(this);
	lbdoorhullico->setPixmap(QPixmap(":data/ScDoorHull.png"));

	QLabel* lbdoorhulllift = new QLabel(tr("Hull Lift Door"), this);

	QLabel* lbdoorhullliftico = new QLabel(this);
	lbdoorhullliftico->setPixmap(QPixmap(":data/ScDoorHullLift.png"));

	QLabel* lbdoorlift = new QLabel(tr("Lift Door"), this);

	QLabel* lbdoorliftico = new QLabel(this);
	lbdoorliftico->setPixmap(QPixmap(":data/ScDoorLift.png"));

	QLabel* lbauto = new QLabel(tr("<html>"
				"<p>"
				"Automatic doors open when someone is near them and they close when the way is clear. "
				"Locked doors do not open automatically but they can close automatically. "
				"<b>Hull and lift doors are not affected by this automatic door setting.</b> "
				"</p>"
				"</html>"), this);
	lbauto->setWordWrap(true);
	m_IsAuto = new QCheckBox(tr("Door opens and closes automatically."), this);
	m_IsAuto->setTristate(isAuto == Qt::PartiallyChecked);
	m_IsAuto->setCheckState(isAuto);

	QLabel* lblock = new QLabel(tr("<html>"
				"<p>"
				"Doors can be locked at the time the design is manufactured. "
				"Locked doors can be opened by any officer, crew or troop of the ship, "
				"and by anyone in the ship's chain of command. "
				"</p>"
				"<p>"
				"Hull door locks control access from the outside. "
				"Hull doors are never treated as locked from the inside. "
				"</p>"
				"<p>"
				"While a ship is executing a <i>stand down</i> order, all doors are treated as unlocked. "
				"</p>"
				"<p>"
				"Access policy of the vessel also controls who can open hull doors, from the outside; "
				"the default access policy admits friends, empire members and chain of command. "
				"Door locks take precedence over access policy. "
				"</p>"
				"</html>"), this);
	lblock->setWordWrap(true);
	m_InitLocked = new QCheckBox(tr("Door is locked when design is manufactured."), this);
	m_InitLocked->setTristate(initLock == Qt::PartiallyChecked);
	m_InitLocked->setCheckState(initLock);

	QLabel* lbremotes = new QLabel(tr("<html>"
				"<p>"
				"Doors can be operated remotely by spacecraft and vehicles. "
				"Helm station and vehicles provide a remote door switch control for this purpose. "
				"When a remote door switch is activated, the closest remotely operable door is operated. "
				"</p>"
				"<p>"
				"Access policy of the vessel also controls who can open hull doors, from the outside; "
				"the default access policy admits friends, empire members and chain of command. "
				"</p>"
				"</html>"), this);
	lbremotes->setWordWrap(true);
	m_RemoteSpacecraft = new QCheckBox(tr("Door responds to spacecraft door remotes."), this);
	m_RemoteSpacecraft->setTristate(remoteSpacecraft == Qt::PartiallyChecked);
	m_RemoteSpacecraft->setCheckState(remoteSpacecraft);
	m_RemoteVehicle = new QCheckBox(tr("Door responds to vehicle door remotes."), this);
	m_RemoteVehicle->setTristate(remoteVehicle == Qt::PartiallyChecked);
	m_RemoteVehicle->setCheckState(remoteVehicle);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Access settings will be configured on selected doors. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoIt()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbinfo, row,0, 1,3);
	row++;
	lay->addWidget(lbdoorstdico, row,1, 1,1);
	lay->addWidget(lbdoorstd, row,2, 1,1);
	row++;
	lay->addWidget(lbdoorautoico, row,1, 1,1);
	lay->addWidget(lbdoorauto, row,2, 1,1);
	row++;
	lay->addWidget(lbdoorhullico, row,1, 1,1);
	lay->addWidget(lbdoorhull, row,2, 1,1);
	row++;
	lay->addWidget(lbdoorhullliftico, row,1, 1,1);
	lay->addWidget(lbdoorhulllift, row,2, 1,1);
	row++;
	lay->addWidget(lbdoorliftico, row,1, 1,1);
	lay->addWidget(lbdoorlift, row,2, 1,1);
	row++;
	lay->setRowMinimumHeight(row, 10);
	row++;
	lay->addWidget(lbauto, row,0, 1,3);
	row++;
	lay->addWidget(m_IsAuto, row,1, 1,2);
	row++;
	lay->setRowMinimumHeight(row, 10);
	row++;
	lay->addWidget(lblock, row,0, 1,3);
	row++;
	lay->addWidget(m_InitLocked, row,1, 1,2);
	row++;
	lay->addWidget(lbremotes, row,0, 1,3);
	row++;
	lay->addWidget(m_RemoteSpacecraft, row,1, 1,2);
	row++;
	lay->addWidget(m_RemoteVehicle, row,1, 1,2);
	row++;
	lay->addLayout(blay, row,0, 1,3);
	lay->setColumnMinimumWidth(0, 20);
	lay->setColumnStretch(2, 20);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScDoorAccessDlg::~AuScDoorAccessDlg()
{
}

void AuScDoorAccessDlg::setVisible(bool visible)
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

void AuScDoorAccessDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DoorAccessDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DoorAccessDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DoorAccessDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DoorAccessDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScDoorAccessDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DoorAccessDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DoorAccessDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DoorAccessDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DoorAccessDlg/SizeY",sz.height());
}

void AuScDoorAccessDlg::DoIt()
{
	accept();
}

Qt::CheckState AuScDoorAccessDlg::IsAutomatic() const
{
	return m_IsAuto->checkState();
}

Qt::CheckState AuScDoorAccessDlg::InitLocked() const
{
	return m_InitLocked->checkState();
}

Qt::CheckState AuScDoorAccessDlg::RemoteSpacecraft() const
{
	return m_RemoteSpacecraft->checkState();
}

Qt::CheckState AuScDoorAccessDlg::RemoteVehicle() const
{
	return m_RemoteVehicle->checkState();
}
