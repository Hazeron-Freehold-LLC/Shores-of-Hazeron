//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtclientchat.h,v 1.17 2022/04/15 17:04:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtClientChat_H
#define DtClientChat_H

#include "auarenaready.h"
#include "aubddesign.h"
#include "aucityreport.h"
#include "auclientlib.h"
#include "aucommpacket.h"
#include "auelapsedtimer.h"
#include "auexchangerating.h"
#include "auid.h"
#include "auitdesign.h"
#include "aujoboffer.h"
#include "auplayeronlinestatus.h"
#include "auscdesign.h"
#include "aushipment.h"
#include "autransportertarget.h"
#include "dzspecies.h"
#include <QImage>
#include <QSemaphore>
#include <QTcpSocket>
#include <QThread>

class AuMission;

class AUCLIENT_DLL DtClientChat : public QThread
{
	Q_OBJECT;

public:
	DtClientChat(QObject* parent=0);
	~DtClientChat();

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	bool IsConnected() const
	{
		return m_isConnected;
	}

	void Kill()
	{
		if(Alive())
			m_semAlive.release(1);
	}

public slots:
	void TxData(QTcpSocket& tcpSocket);

signals:
	void ArenaReady(AuArenaReady arena);
	void AvatarsUpdated();
	void BrokerShipment(AuShipment shipment);
	void BrokerShipmentsClear();
	void BrokerTradeAgreement(AuTradeAgreement ta);
	void BrokerTradeAgreementsClear();

	void ChannelOpened(QString channelName, quint8 scope, QString narrDesc);
	void ChatReceived(QByteArray msg);

	void CityHistoryReportReady(AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId cityId, AuCityReport report);

	void Connected();

	void Disconnected();

	void EmpireRequested(quint32 empireId);

	void GameOver(quint8 gameType, QByteArray gameResult);
	void GameSelect(quint8 gameType, bool gameSelected);
	void GameTurn(AuId personsTurn);
	void HangarQueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar);
	void MissionSensorContactsReceived(QByteArray msg);

	void PlayersOnlineStatus(AuPlayerOnlineStatusVector players);

	void ReplaceSpecies(DzSpecies species);

	void TransferCitizensAvailable(QByteArray ba);

	void JobAvailable(AuJobOffer job);

	void OrdnanceReceived(AuId loaderId, QByteArray ord);

	void BdDesign(AuBdDesign rec);
	void BdDesignDelete(quint32 designId);
	void BdDesignNeeded(quint32 designId, quint8 designState);

	void BdExchangeBlueprintRating(quint32 designId, QString designName, AuExchangeBlueprintRating rating);
	void BdExchangeUpdated();
	void BdLibraryUpdated();

	void ItDesign(AuItDesign rec);
	void ItDesignDelete(quint32 designId);
	void ItDesignNeeded(quint32 designId, quint8 designState);
	void ItLibraryUpdated();

	void ScDesign(AuScDesign rec);
	void ScDesignDelete(quint32 designId);
	void ScDesignNeeded(quint32 designId, quint8 designState);

	void ScExchangeBlueprintRating(quint32 designId, QString designName, AuExchangeBlueprintRating rating);
	void ScExchangeUpdated();
	void ScLibraryUpdated();

	void TransporterPlace(AuTransporterTarget tt);
	void TransporterShip(AuId id, QString name, QString desc);
	void TransporterTransporter(AuId id, QString name, QString desc, quint16 whichTransporter);

protected:
	bool ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);
	virtual void run();

private slots:
	void NetworkError(QAbstractSocket::SocketError socketError);
	void RxData(QTcpSocket& tcpSocket);

private:
	int			m_BytesRx;				// Total bytes received from client
	int			m_BytesTx;				// Total bytes sent to client
	bool		m_isConnected;			// true while connected to a scene server
	AuElapsedTimer	m_LastComm;			// Last reception or transmission of any bytes. Used for keep-alives.
	quint32		m_PacketSize;			// TCP block size
	int			m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	QSemaphore	m_semAlive;				// Used to know when I'm supposed to die.
};

#endif // DtClientChat_H
