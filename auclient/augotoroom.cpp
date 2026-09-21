//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augotoroom.cpp,v 1.6 2022/02/19 18:10:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augotoroom.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "davessel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AuGoToRoom::AuGoToRoom(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Go To Room"));
	setAuDlgIcon(QIcon(OrderIcon(ORDER_GoToRoom)));
	QString lastName = g_Au.Settings().readEntry("Widget/GoToRoom/RoomName","");

	QLabel* lb = new QLabel(tr("<html><p>"
				"Enter a room name for the selected units to go. "
				"To carry out this order, they must be <u>aboard a spacecraft where they hold a berth</u>. "
				"</p>"
				"<p>"
				"Units will look for a room with a name that exactly matches. "
				"If no exact match is found, a room with a partial match will be accepted. "
				"The partial match check is not case sensitive. "
				"e.g. <i>bri</i> would match <i>Bridge</i> and <i>Brig</i>. "
				"</p>"
				"</html>"), this);
	lb->setWordWrap(true);

	m_RoomNames = new QComboBox(this);
	m_RoomNames->setEditable(true);
	m_RoomNames->setToolTip(tr("<html>"
				"The list contains the room names of the ship you are aboard, "
				"if you are on the roster or if you are in the ship's chain of command. "
				"Those names are followed by a list of generic room names. "
				"</html>"));
	ListRoomNames(m_RoomNames);
	if(!lastName.isEmpty())
		m_RoomNames->setCurrentText(lastName);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Accepts the name or name fragment as the destination for the selected units."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->addWidget(m_RoomNames);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGoToRoom::~AuGoToRoom()
{
}

void AuGoToRoom::setVisible(bool visible)
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

void AuGoToRoom::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GoToRoom/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GoToRoom/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GoToRoom/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GoToRoom/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGoToRoom::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GoToRoom/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GoToRoom/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GoToRoom/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GoToRoom/SizeY",sz.height());
}

void AuGoToRoom::ListRoomNames(QComboBox* cb)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// List specific room names first.
	QStringList roomNames;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaCapsule* v = person->FindParentCapsuleInEnv();
		if(v && (v->Is(person->AssignedShip()) || v->InChainOfCommand(person)))
			v->ListRoomNames(roomNames);
	}
	pView->UnlockSceneAtom();
	roomNames.sort(Qt::CaseInsensitive);

	// List generic room names.
	QStringList rmnames;
	for(int rt=ROOM_None+1; rt<=ROOM_WeaponBay; rt++)
	{
		QString rn(RoomTypeNameInSpacecraft((ROOMTYPE)rt));
		if(rn.isEmpty())
			continue;
		if(roomNames.contains(rn))
			continue;
		if(rmnames.contains(rn))
			continue;
		rmnames.append(rn);
	}
	rmnames.sort(Qt::CaseInsensitive);
	roomNames.append(rmnames);

	// Put the names into the combo box.
	for(QStringList::const_iterator rni = roomNames.cbegin(); rni != roomNames.cend(); rni++)
		cb->addItem(*rni);
}

QString AuGoToRoom::RoomName() const
{
	QString rn(m_RoomNames->currentText());
	g_Au.Settings().writeEntry("Widget/GoToRoom/RoomName", rn);
	return rn;
}
