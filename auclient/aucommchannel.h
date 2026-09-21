//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommchannel.h,v 1.14 2022/02/19 17:54:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommChannel_H
#define AuCommChannel_H

#include "auclientlib.h"
#include "aucommpacket.h"
#include <QWidget>
#include <QDialog>

class AuComm;
class AuCommDialog;
class AuCommMission;
class AuItem;
class AuSpecies;
class QComboBox;
class QKeyEvent;
class QLabel;
class AuLineEdit;
class QSlider;
class QTableWidgetItem;
class QToolButton;
class QModelIndex;

class AUCLIENT_DLL AuCommChannel : public QWidget
{
	Q_OBJECT;
private:
	QString								m_AboutToSay;			// Last text in edit control when arrow pressed.
	AuCommStdChannel::RANGE				m_RangeSetting;
	QStringList							m_Said;					// History of messages actually said.
	int									m_SaidIndex;			// Moved by the up arrow and down arrow.
	AuCommStdChannel::SCOPE				m_Scope;				// Scope of this channel.
	int									m_SilverwillTaskId;		// Last Silverwill task id created from comm channel.
	QString								m_SilverwillTaskText;	// Last Silverwill task text created from comm channel.
	bool								m_UnseenChat;			// set true when chat message arrives while this tab is not visible

	// Widgets
	AuCommMission*						m_MissionDlg;
	QToolButton*						m_Range;
	AuLineEdit*							m_Say;
	AuCommDialog*						m_Transcript;

public:
	AuCommChannel(const QString& name, AuCommStdChannel::SCOPE scope, QWidget* parent=0);

	void AddBuddy(const AuId& playerId, const QString& playerName, const AuId& contactId, const QString& contactName);

	static bool AddContact(const AuId& contactId, const QString& contactName, bool bIgnore);

	void AddSquelch(const AuId& playerId, const QString& playerName, const AuId& contactId, const QString& contactName);

	void ChatReceived(const AuCommPacket& msg);

	void CloseChannel();

	void FocusChatInput();

	// Returns true if any message contains 'avatarName', not case sensitive.
	bool HasMessageAboutAvatar() const;

	inline bool HasUnseenChat() const
	{
		return m_UnseenChat;
	}

	bool IsMessageSelected() const;		// true if any message is selected

	inline bool IsNarrator() const
	{
		return m_Scope == AuCommStdChannel::Narrator;
	}

	static void NameSpecies(const AuSpecies& species, bool isWildAnimal, const AuId& sceneAtomId, QWidget* inputBoxParent);

	inline AuCommStdChannel::RANGE Range() const
	{
		return m_RangeSetting;
	}

	inline AuCommStdChannel::SCOPE Scope() const
	{
		return m_Scope;
	}

	QString SelectedMessageEmpireOfOrigin() const;

	// Caller must delete returned item.
	AuItem* SelectedMessageItem() const;

	// Caller must delete returned item.
	AuItem* SelectedMessageOfferItem() const;

	AuId SelectedMessageSenderId() const;

	QString SelectedMessageSenderName() const;

	AuCommPacket::TYPE SelectedMessageType() const;

	void SendChat(AuCommPacket& msg);

	void SendMailMessage(const AuId& playerId, const AuId& contactId, const QString& contactName);

	void SensorContactsReceived(QByteArray bablips);

	void SpotTargetOrTerrain(const QString& targetDesc, POLITICAL_STANCE stanceTowardTarget, bool targetIsCivilian);

public slots:
	void ChannelSpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian);
	void NotifyServer();
	void OpenChannelToSender();
	virtual void setVisible(bool visible);
	void StoryChoicePickedCleanup(quint32 storyInstance, qint8 choiceId, AuId fromId);

signals:
	void ChoicePicked(quint32, qint8, AuId);
	void HelpRequestAccepted(AuId reqId, QString reqName);
	void MessageSelectionChanged();
	void OpenChannelToSender(AuId senderId);
	void Steamworks();

protected slots:
	void Accept();
	void AcceptAddMissionOrder();
	void AcceptInsertMissionOrder();
	void AcceptMissionProperty();
	void AcceptNewMissionOrder();
	void AcceptNewMission();
	void Announce();
	void Announcement();
	void BeamOver();
	void Bid();
	void BidPickupShipment();
	void CityInventory();
	void ClearChatEntry();
	void Detonate();
	void DiplomaticMessage();
	void DownArrowPressed();
	void Emoji(QString emoji);
	void EmojiPressed();
	void ForSale();
	void HailRequest();
	void HailRequestComment();
	void HailReply();
	void ItemDescription();
	void LoadColonists();
	void MakeSilverwillNoteFromMessage();
	void MakeSilverwillTaskFromMessage();
	void MessageActivated(const QModelIndex& index);
	void MessageClicked();
	void MissionOrder();
	void NameTarget();
	void NewSilverwillTask();
	void OfferAvatarToSender();
	void OfferItemDescription();
	void PostBounty();
	void PostBountySpotted();
	void RangeClose();
	void RangeGalaxy();
	void RangeHail();
	void RangeMenu();
	void RangeSector();
	void RangeSystem();
	void RangeUniverse();
	void RequestBerth();
	void RequestService();
	void ReturnPressed();
	void RevokeOffer();
	void SelectionChanged();
	void SendFunds();
	void ShowLog();
	void SolicitSales();
	void TextChanged(const QString& text);
	void TeleportToSender();
	//void TroopOrders();
	void UnloadColonists();
	void UpArrowPressed();
	void ViewWebPageOfSender();

private:
	static void CloseChannel(const QString& channelName);

	void InitLog();

	void InviteTeleport(AuId& personId, QString& personName, AuId& personSectorId, GALAXY personGalaxy, AuId& contactId,
			QString& contactName);

	void InviteToEmpire(quint32 personEmpireId, AuId& personId, QString& personName, AuId& personSectorId, GALAXY personGalaxy,
			AuId& contactId, QString& contactName);

	QString LogFileName() const;

	void MissionOrder(const QString& toName, const AuId& toId, const AuId& toSystemId);

	void SetRange(AuCommStdChannel::RANGE range);
};

#endif // AuCommChannel_H
