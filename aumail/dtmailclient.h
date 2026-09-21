//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmailclient.h,v 1.22 2022/04/15 20:37:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtMailClient_H
#define DtMailClient_H

#include "auelapsedtimer.h"
#include "auid.h"
#include "aumaillib.h"
#include "aumailmsg.h"
#include <QSemaphore>
#include <QTcpSocket>
#include <QThread>

class AuMailMsg;

class AUMAIL_DLL DtMailClient : public QThread
{
	Q_OBJECT;

	int									m_BytesRx;			// Total bytes received from client
	int									m_BytesTx;			// Total bytes sent to client
	bool								m_DeferredToWorker;	// True if we have contacted the master to be directed to a worker.
	QList<quint32>						m_DeleteQueue;
	AuId								m_idPerson;			// atom id of current person who's mail is being managed
	AuElapsedTimer						m_LastRx;			// Time of last mail fetch.
	bool								m_MailRequested;	// True when we have been told to check the mail.
	QString								m_MailServerName;
	quint16								m_MailServerPort;
	bool								m_MailVisible;		// True when mail window is visible.
	QSemaphore							m_semAlive;			// Used to know when I'm supposed to die.
	bool								m_UpLongEnough;
	AuElapsedTimer						m_UpTime;

public:
	DtMailClient(QObject* parent=0);
	~DtMailClient();

	inline bool Alive() const
	{
		return !m_semAlive.available();
	}

	// 'oldMailIds' is an output parameter, filled with the ids of mail messages that are deleted
	static void DeleteOldMail(const AuId& toId, QList<quint32>& oldMailIds);

	inline void Kill()
	{
		m_semAlive.release(1);
	}

	// Loads a mail message from the local cache.
	// 'msgId' is an input parameter to specify which message to load.
	// 'msg' is an output parameter, only valid if true is returned.
	// Returns true on success.
	static bool LoadMessage(quint32 msgId, AuMailMsg& msg);

	inline void RequestMessages()
	{
		m_MailRequested = true;
		m_UpLongEnough = true;
	}

public slots:
	void MailVisibilityChanged(bool isVisible);
	void SelectPerson(AuId id);

protected:
	void LoadMessageCache();
	int ReadPacket(const QByteArray& ba);
	virtual void run();
	void RxMail(const AuId& personId);
	void RxMailSolo(const AuId& personId);
	void StoreMessageInCache(const AuMailMsg& msg);
	void TxDeletions();
	void TxMail();

private slots:
	void NetworkError(QAbstractSocket::SocketError socketError);

signals:
	void CachedMessageReceived(AuMailMsg msg);
	void DeferFailed();
	void LoginError();
	void MailBatchEnd();
	void MailBatchStart(int count);
	void MessageDiscarded();
	void NewMessageReceived(AuMailMsg msg);
	void VersionError();
};

#endif // DtMailClient_H
