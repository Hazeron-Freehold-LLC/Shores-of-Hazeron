//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayerstable.h,v 1.5 2022/02/19 18:19:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPlayersTable_H
#define AuPlayersTable_H

#include "audata.h"
#include "auplayeronlinestatus.h"
#include <QTableWidget>
#include <QWidget>

class AuPlayerOnlineStatusVector;
class QTableWidgetItem;

class AUDATA_DLL AuPlayersTable : public QTableWidget
{
	Q_OBJECT;
	int									m_AvatarsOnline;
	AuPlayerOnlineStatusVector			m_Contents;
	bool								m_EmpiresAvailable;		// True if empire records can be referenced.
	bool								m_IsArchitect;
	QTableWidgetItem*					m_MousePressItem;		// Item under the mouse press.
	int									m_UpdateResult;			// Result of last call to Update.

public:
	AuPlayersTable(bool empiresAvailable, QWidget* parent=0);
	virtual ~AuPlayersTable()
	{
	}

	// Returns the number of players visibly on line.
	inline int AvatarsOnline() const
	{
		return m_AvatarsOnline;
	}

	// Updates m_AvatarsOnline value.
	// Returns: 0=no change, 1=updated ok, 2=at least one empire record was requested
	int Update(const AuPlayerOnlineStatusVector& players, bool isArchitect);

signals:
	void EmpireRequested(quint32 empireId);
	void ItemRightClicked(QTableWidgetItem*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // AuPlayersTable_H
