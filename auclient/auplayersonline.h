//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayersonline.h,v 1.8 2022/05/04 22:13:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPlayersOnline_H
#define AuPlayersOnline_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auplayeronlinestatus.h"
#include <QTableWidget>
#include <QWidget>

class AuIconButton;
class QLabel;
class QTimer;

class AUCLIENT_DLL AuPlayersOnlineTable : public QTableWidget
{
	Q_OBJECT;
	QTableWidgetItem*					m_MousePressItem;		// Item under the mouse press.

public:
	AuPlayersOnlineTable(QWidget* parent=0);
	virtual ~AuPlayersOnlineTable()
	{
	}

	virtual QSize sizeHint() const;

signals:
	void ItemRightClicked(QTableWidgetItem*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuPlayersOnlineWidget : public QWidget
{
	Q_OBJECT;
private:
	bool								m_AFK;
	QHash<AuId, QString>				m_PlayersOn;			// Players who were online and not afk at last update.
	QTimer*								m_Timer;				// UpdateContents timer

	// Widgets
	AuIconButton*						m_AFKCheck;
	QLabel*								m_PlayersLabel;
	AuPlayersOnlineTable*				m_Players;

public:
	AuPlayersOnlineWidget(QWidget* parent=0);
	virtual ~AuPlayersOnlineWidget();

	bool IsAFK() const;

public slots:
	void AFKClicked();
	void PlayersOnlineStatus(AuPlayerOnlineStatusVector players);
	void UpdateContents();

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void AddBuddy();
	void AddSquelch();
	void GoToAvatar();
	void OpenChannelToAvatar();
	void PlayerRightClicked(QTableWidgetItem* item);

signals:
	void AddBuddy(AuId id, QString name);
	void AddSquelch(AuId id, QString name);
	void AvatarOnLine(AuId id);
	void OpenChannelToAvatar(AuId id);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuPlayersOnlineDock : public AuDockWidget
{
public:
	AuPlayersOnlineWidget*				m_Content;

public:
	AuPlayersOnlineDock(QWidget* parent) : AuDockWidget(":data/ContactBuddies.png", tr("<b>Avatars Online</b>"),
			tr("Avatars Online"), parent)
	{
		m_Content = new AuPlayersOnlineWidget(this);
		setWidget(m_Content);
		int dockSides = g_Au.Settings().readNumEntry("Dock/AvatarsOnlineAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/AvatarsOnlineAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuPlayersOnline_H
