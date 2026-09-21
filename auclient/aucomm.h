//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucomm.h,v 1.10 2022/02/19 17:54:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCOMM_H
#define AUCOMM_H

#include "auclientlib.h"
#include <aucommpacket.h>
#include <audockwidget.h>
#include <auglobal.h>

class AuCommChannel;
class AuCommPacket;
class AuCommTabsColumn;
class QGridLayout;
class QStackedLayout;
class QToolButton;

class AUCLIENT_DLL AuChannel
{
public:
	QString								m_Name;					// Channel name.
	QString								m_NarrDesc;				// Description for narrator channels.
	AuCommStdChannel::SCOPE				m_Scope;

	AuChannel(const QString& name, AuCommStdChannel::SCOPE scope, const QString& narrDesc)
	{
		m_Name = name;
		m_NarrDesc = narrDesc;
		m_Scope = scope;
	}

	AuChannel()
	{
		m_Scope = AuCommStdChannel::Everyone;
	}

	virtual ~AuChannel()
	{
	}

	AuChannel(const AuChannel& c)
	{
		m_Name = c.m_Name;
		m_NarrDesc = c.m_NarrDesc;
		m_Scope = c.m_Scope;
	}

	virtual void operator<<(QDataStream& in)
	{
		quint8 ui8;

		in >> m_Name;
		in >> m_NarrDesc;
		in >> ui8;
		m_Scope = (AuCommStdChannel::SCOPE)ui8;
	}

	virtual void operator>>(QDataStream& out) const
	{
		out << m_Name;
		out << m_NarrDesc;
		out << (quint8)m_Scope;
	}

	void SensorContactsReceived(QByteArray bablips);
};

inline QDataStream& operator<<(QDataStream& out, const AuChannel& channel)
{
	channel >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuChannel& channel)
{
	channel << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuComm : public QWidget
{
	Q_OBJECT;
private:
	QList<AuChannel>					m_OpenChannels;			// Channels open right now. Determines restore order next time.

	// Widgets
	QGridLayout*						m_Lay;
	QStackedLayout* 					m_Pages;
	AuCommTabsColumn* 					m_TabsCol;				// Tab column.

public:
	AuComm(QWidget* parent=0);
	~AuComm();

	AuCommChannel* Channel(const QString& name) const;

	void ChatReceived(AuCommPacket& msg);

	AuCommChannel* CreateChannel(const QString& name, AuCommStdChannel::SCOPE scope, bool selectNewTab,
			const QString& narrDesc=QString(""));

	void ListOpenChannels(QStringList& channelNames);

	void SendChat(AuCommPacket& msg);

	void SensorContactsReceived(QByteArray bablips);

public slots:
	void AddBuddy(AuId id, QString name);
	void AddSquelch(AuId id, QString name);
	void Animate();
	void FocusChatInput();
	void SpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian, bool logOnly);

signals:
	void ChannelSpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian);
	void ChoicePicked(quint32 storyInstance, qint8 value, AuId offeredBy);

public slots:
	void OpenChannelTo(AuId senderId);

protected slots:
	void ChannelTabMoved(int fromIndex, int toIndex);
	void ChoicePickedInChannel(quint32 storyInstance, qint8 value, AuId offeredBy);
	void CloseChannel(int tabIdx);
	void CloseChannel(QString channelName);
	void CloseNamedChannels();
	void CurrentTabChanged(int idx);
	void EnableControls();
	void FloatChannel(int tabIdx);
	void OpenChannel();
	void OpenStdChannel();
	void RestoreChannel(AuCommChannel* channel);

protected:
	void Narrate(const AuCommPacket& msg);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCommDock : public AuDockWidget
{
public:
	AuComm*								m_Comm;

public:
	AuCommDock(QWidget* parent) : AuDockWidget(":data/Comm.png", tr("<b>Comm</b>"), tr("Comm"), parent)
	{
		SetWikiTopic("Communication");
		m_Comm = new AuComm(this);
		setWidget(m_Comm);

		int dockSides = g_Au.Settings().readNumEntry("Dock/CommAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/CommAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AUCOMM_H
