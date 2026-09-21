//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumail.h,v 1.42 2022/04/15 20:37:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMail_H
#define AuMail_H

#include "aucontact.h"
#include "auelapsedtimer.h"
#include "auid.h"
#include "aumaillib.h"
#include "aumailmsg.h"
#include "aupoint.h"
#include <QStandardItemModel>
#include <QWidget>

class AuMailNew;
class AuMailTreeView;
class DtMailClient;
class QComboBox;
class QLabel;
class QProgressBar;
class QSplitter;
class QTextBrowser;
class QTimer;

class AUMAIL_DLL AuMail : public QWidget
{
	Q_OBJECT;
	enum NODE_TYPE
	{
	NODE_MailMessage,					// A mail message.
	NODE_Individual,					// Parent node of mail messages from individuals, including bank and library.
	NODE_Empire,						// Parent node of city hierarchy, contains NODE_Sector nodes.
	NODE_Sector,						// City hierarchy, contains NODE_System nodes.
	NODE_System,						// City hierarchy, contains NODE_World nodes.
	NODE_World,							// City hierarchy, contains NODE_City nodes.
	NODE_City,							// City hierarchy. Parent node of each city's mail messages.
	NODE_Officers,						// Parent node of NODE_Officer nodes.
	NODE_Officer,						// Parent node of officer mail messages.
	NODE_Fleet,							// Parent node of NODE_Spacecraft nodes.
	NODE_Spacecraft,					// Parent node of spacecraft mail messages.
	NODE_Government,					// Parent node of NODE_Department nodes.
	NODE_Department,					// Parent node of government mail messages.
	NODE_Galaxy,						// City hierarchy, contains NODE_Sector nodes.
	};

private:
	quint8								m_ActivityCount;		// For activity indicator.
	AuElapsedTimer						m_DeleteTimer;
	int									m_idMessage;			// id of message whose body is displayed
	AuId								m_idPerson;				// id of person whose mail is displayed
	QStandardItemModel					m_Model;				// Data is stored on items in column 0 UserRole positions, at the
																// following offsets:
										// UserRole+0 = Type of node, NODE_TYPE.
										// UserRole+1 = Mail message id for NODE_MailMessage; sender id when the item contains
										//              NODE_MailMessages directly, not in sub nodes; 0 for all others.
										// UserRole+2 = Priority for NODE_MailMessage.
										// UserRole+3 = From solar system id.
										// UserRole+4 = From solar system name.
										// UserRole+5 = From world id.
										// UserRole+6 = From world name.
										// UserRole+7 = From galaxy coordinate X, in parsecs (pc).
										// UserRole+8 = From galaxy coordinate Y, in parsecs (pc).
										// UserRole+9 = From galaxy coordinate Z, in parsecs (pc).
	AuElapsedTimer						m_MsgActivity;			// Reset whenever we receive messages, even if they are discarded.
	QTimer*								m_timer;
	DtMailClient*						m_tMailClient;
	bool								m_UpdateIcons;			// Set true when the priority icons need updating.

	// Widgets
	QLabel*								m_ActivityIndicator;
	QProgressBar*						m_BatchProgress;		// Allocated when needed
	AuMailTreeView*						m_Mail;
	QTextBrowser*						m_MessageText;
	QComboBox*							m_PlayerCombo;
	QSplitter*							m_Splitter;

public:
	AuMail(QWidget* parent=0);
	~AuMail();

	virtual QSize sizeHint() const;

	// Deletes all local mail files that were sent to 'id'.
	static void DeleteLocalMail(const AuId& id);

protected:
	QModelIndex AddTreeItem(const QString& itemName, const QIcon& icon, NODE_TYPE type, const QModelIndex& parentIdx);

	QModelIndex AddTreeItem(const QString& itemName, const QIcon& icon, NODE_TYPE type, const QModelIndex& parentIdx,
			const AuId& fromSystemId, const QString& fromSystem, const AuId& fromWorldId, const QString& fromWorld,
			float fromGalaxyX, float fromGalaxyY, float fromGalaxyZ);

	bool DeleteMessage(const QModelIndex& idx);

	// Deletes old mail that is supposed to be automatically deleted, according to global settings.
	void DeleteOldMail();

	// Recursive, pass parIdx = QModelIndex() to start at the root.
	QModelIndex FindMail(quint32 findMsgId, const QModelIndex& parIdx);

	virtual void hideEvent(QHideEvent* event);

	void MessageReceived(const AuMailMsg& msg);

	void RestoreGeometry();
	void SaveGeometry();
	virtual void showEvent(QShowEvent* event);
	void UpdateModelIcons();

public slots:
	void AvatarsUpdated();
	void DeleteMessage();
	void DeleteMessages();
	void MailBatchEnd();
	void MailBatchStart(int count);
	void NewMessage();
	void RequestMessages();
	virtual void setVisible(bool visible);
	void ReplyToMessage();
	void ForwardMessage();
	void AddBuddy();
	void AddSquelch();
	void ManageContacts();
	void StopThreads();
	void SetRecipient(AuId personId);

protected slots:
	void CachedMessageReceived(AuMailMsg msg);
	void LoginError();
	void MailSettings();
	void MessageRightClicked(const QModelIndex& idx);
	void MessageSelectionChanged();
	void MessageDiscarded();
	void NewMessageReceived(AuMailMsg msg);
	void PersonChanged(int index);
	void ServerError();
	void SplitterMoved(int pos, int index);
	void UpdateContents();
	void VersionError();

signals:
	void AlertMessageReceived(AuMailMsg msg);
	void GoDirectToCoord(AuPoint galaxyLoc);
	void GoDirectToSystem(AuId fromSystemId, QString fromSystem, AuPoint galaxyLoc);
	void GoLandAtCity(AuId fromCityId, QString fromCity);
	void GoOrbit(AuId fromWorldId, QString fromWorld);
	void GoOrbitCity(AuId fromCityId, QString fromCity);
	void GoViaWormholes(AuId fromSystemId, QString fromSystem);
	void PersonSelected(AuId);
	void UpdateSquelch(AuId idContact, bool bDelete);
	void UpdateSquelches(AuIdList idContacts, bool bDelete);
	void VisibilityChanged(bool isVisible);
};

#endif // AuMail_H
