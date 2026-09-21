//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augotoroom.h,v 1.6 2022/02/19 17:56:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGoToRoom_H
#define AuGoToRoom_H

#include "auclientlib.h"
#include "audialog.h"

class QComboBox;

class AUCLIENT_DLL AuGoToRoom : public AuDialog
{
	Q_OBJECT;
	QComboBox*							m_RoomNames;

public:
	AuGoToRoom(QWidget* parent=0);
	virtual ~AuGoToRoom();

	QString RoomName() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListRoomNames(QComboBox* cb);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGoToRoom_H
