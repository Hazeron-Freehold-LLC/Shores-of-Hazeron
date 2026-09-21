//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommdialog.h,v 1.7 2022/02/19 17:54:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommDialog_H
#define AuCommDialog_H

#include "auatomchat.h"
#include "auclientlib.h"
#include "auglobal.h"
#include <QQueue>
#include <QTableWidget>
#include <QTimer>

class AuRefitShip;
class AuShipment;

class AUCLIENT_DLL AuCommDialog : public QTableWidget
{
	Q_OBJECT;
private:
	QTableWidgetItem*					m_MousePressItem;		// Item under the mouse press.
	QQueue<AuAtomChat>					m_MsgQueue;				// Queue of messages waiting to be added to the table.
	AuCommPacket*						m_SelectedMsg;			// A copy of the currently selected message.
	QTimer*								m_Timer;				// Timer for filling the table from the queue.

public:
	AuCommDialog(bool isNarrator, QWidget* parent=0);
	virtual ~AuCommDialog();

	void ChatReceived(const AuCommPacket& msg);

	// Returns true if any message contains 'avatarName', not case sensitive.
	bool HasMessageAbout(const QString& avatarName) const;

	bool IsMessageSelected() const;

	AuId SelectedMessageCityId() const;

	void SelectedMessageCityIds(AuIdList& cityIds) const;

	COMMODITYID SelectedMessageCommodityId() const;

	unsigned int SelectedMessageCount() const;

	quint32 SelectedMessageDesignId() const;

	QString SelectedMessageEmpireOfOrigin() const;

	double SelectedMessageEstCost() const;

	quint32 SelectedMessageEstSeconds() const;

	GALAXY SelectedMessageFromGalaxy() const;

	AuId SelectedMessageFromId() const;

	QString SelectedMessageFromName() const;

	quint32 SelectedMessageFromEmpireId() const;

	// Only for HailReply message. 0 for all others.
	AuId SelectedMessageFromSystem() const;

	QImage SelectedMessageImage() const;

	quint32 SelectedMessageInstallSecs() const;

	// Allocated on the heap. Caller must delete. May return 0.
	AuItem* SelectedMessageItem() const;

	quint8 SelectedMessageMinQ() const;

	QString SelectedMessageOfferEmpireOfOrigin() const;

	// Allocated on the heap. Caller must delete. May return 0.
	AuItem* SelectedMessageOfferItem() const;

	void SelectedMessageMissingParts(QList<COMMODITYID>& noQ, QList<COMMODITYID>& shortQ, QList<COMMODITYID>& noSupply,
			QList<COMMODITYID>& enough) const;

	// Only for MissionNew message. Returns empty mission for all others.
	// Mission name is returned.
	QString SelectedMessageMission(AuMission& mission) const;

	// Only for MissionOrder messages. Returns 0 for all others.
	const AuMissionOrder* SelectedMessageMissionOrder() const;

	void SelectedMessageRefit(AuRefitShip& refit) const;

	quint8 SelectedMessageMissionProperty() const;

	quint32 SelectedMessageToEmpireId() const;

	quint8 SelectedMessageMissionValue() const;

	// From AuCommPreBoard: 1=1stCl pass, 2=2ndCl pass, 3=3rdCl pass, 4=troop, 5=crew
	quint8 SelectedMessagePreBoardBerthType() const;

	quint32 SelectedMessagePrice() const;

	quint16 SelectedMessageRepairSecs() const;

	AuId SelectedMessageRepairShipId() const;

	bool SelectedMessageShipment(AuShipment& shipment) const;

	quint8 SelectedMessageSystem() const;

	QString SelectedMessageSystemDesc() const;

	quint16 SelectedMessageSystemWhich() const;

	QString SelectedMessageText() const;

	AuCommPacket::TYPE SelectedMessageType() const;

signals:
	void ItemRightClicked(QTableWidgetItem*);
	void MessageSelected(QByteArray msg);

protected:
	void AddMessage(AuAtomChat msg);
	virtual void hideEvent(QHideEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual int sizeHintForRow(int row) const;

protected slots:
	void DequeueMessage();
	void ItemSelectionChanged();
};

#endif // AuCommDialog_H
