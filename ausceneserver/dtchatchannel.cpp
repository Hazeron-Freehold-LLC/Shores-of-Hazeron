//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtchatchannel.cpp,v 1.990 2022/05/06 15:04:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtchatchannel.h"
//
#include "auarenaready.h"
#include "aubddesign.h"
#include "aubdreport.h"
#include "audbglobal.h"
#include "auitdesign.h"
#include "auitreport.h"
#include "aujoboffer.h"
#include "aulogincomm.h"
#include "auplayeronlinestatus.h"
#include "auscdesign.h"
#include "ausceneserverapp.h"
#include "auscparts.h"
#include "auscreport.h"
#include "auserverinterface.h"
#include "autransportertarget.h"
#include "daanimal.h"
#include "daarena.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesignbase.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "davessel.h"
#include "daworldll.h"
#include "dgdata.h"
#include "dtccpacket.h"
#include "hashpack.h"
#include <QDataStream>
#include <QUdpSocket>
#include <QVariant>

DtChatChannel::DtChatChannel(int socketDesc, QObject* parent) : QThread(parent)//, m_semAlive(1)
{
	m_AccountType = ACCT_Active;
	m_BadPackets = 0;
	m_BdDesignsRequested = false;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_confirmNum = 0;
	m_DesignStudioMode = DaDesignBase::InvalidMode;
	m_Disconnect = false;
	m_idPlayer = 0;
	m_ItDesignsRequested = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_ScDesignsRequested = false;
	m_socketDescriptor = socketDesc;
	m_ThreadId = 0;
	m_TxBufferFlush = false;
	//m_semAlive.acquire(1);			// I live...

	m_I = new AuServerInterface;
	connect(m_I, SIGNAL(AcceptTrade(AuTradeAgreement)),
			SIGNAL(AcceptTrade(AuTradeAgreement)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(AdminGoTo(AuId, AuId, quint8, bool, AuId)),
			SIGNAL(AdminGoTo(AuId, AuId, quint8, bool, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(AdminGoToGalaxyLoc(AuId, AuId, AuId, AuPoint)),
			SIGNAL(AdminGoToGalaxyLoc(AuId, AuId, AuId, AuPoint)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BdDesignExchangePublished(AuBdDesignExchangeEntry)),
			SIGNAL(BdDesignExchangePublished(AuBdDesignExchangeEntry)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BdDesignExchangeUnpublished(quint32)),
			SIGNAL(BdDesignExchangeUnpublished(quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BlueprintRatedBd(AuExchangeRatingRec)),
			SIGNAL(BlueprintRatedBd(AuExchangeRatingRec)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BlueprintRatedSc(AuExchangeRatingRec)),
			SIGNAL(BlueprintRatedSc(AuExchangeRatingRec)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
			SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
					QString, AuId, quint8)),
			SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
					QString, AuId, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
			SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
			SIGNAL(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			SIGNAL(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EmpireChgBdPref(quint32, int, quint32)),
			SIGNAL(EmpireChgBdPref(quint32, int, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EmpireChgFlag(quint32, QImage)),
			SIGNAL(EmpireChgFlag(quint32, QImage)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EmpireChgScPref(quint32, int, quint32)),
			SIGNAL(EmpireChgScPref(quint32, int, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			SIGNAL(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovBossChanged(quint32, AuId, AuId)),
			SIGNAL(GovBossChanged(quint32, AuId, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)),
			SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovDossierChangeState(quint32, quint32, AuTime, quint8, quint16, quint8)),
			SIGNAL(GovDossierChangeState(quint32, quint32, AuTime, quint8, quint16, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetAnnouncements(quint32, QString)),
			SIGNAL(GovSetAnnouncements(quint32, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
			SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetJoinPassword(quint32, QByteArray)),
			SIGNAL(GovSetJoinPassword(quint32, QByteArray)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetCommerce(quint32, QByteArray)),
			SIGNAL(GovSetCommerce(quint32, QByteArray)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetContrailColor(quint32, quint8, quint8)),
			SIGNAL(GovSetContrailColor(quint32, quint8, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetDefaultPoliticalStance(quint32, quint8)),
			SIGNAL(GovSetDefaultPoliticalStance(quint32, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString, quint8,
					quint8, QString, QString, QString)),
			SIGNAL(GovSetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString, quint8,
					quint8, QString, QString, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetTax(quint32, quint8, quint8, quint8)),
			SIGNAL(GovSetTax(quint32, quint8, quint8, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetThemeColor(quint32, quint8, QColor)),
			SIGNAL(GovSetThemeColor(quint32, quint8, QColor)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetWarCriteria(quint32, AuWarCriteria)),
			SIGNAL(GovSetWarCriteria(quint32, AuWarCriteria)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(LeaveEmpire(AuId, QString, quint32)),
			SIGNAL(LeaveEmpire(AuId, QString, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(NameSpecies(AuSpecies, QString, AuId)),
			SIGNAL(NameSpecies(AuSpecies, QString, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
			SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(OfficeRevoked(AuId, quint32)),
			SIGNAL(OfficeRevoked(AuId, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ProposeTrade(AuTradeAgreement)),
			SIGNAL(ProposeTrade(AuTradeAgreement)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ReleaseBerth(AuId, AuId)),
			SIGNAL(ReleaseBerth(AuId, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameAvatar(AuId, QString)),
			SIGNAL(RenameAvatar(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameCity(AuId, QString)),
			SIGNAL(RenameCity(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameSector(AuId, QString)),
			SIGNAL(RenameSector(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameSystem(AuId, QString)),
			SIGNAL(RenameSystem(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ScDesignExchangePublished(AuScDesignExchangeEntry)),
			SIGNAL(ScDesignExchangePublished(AuScDesignExchangeEntry)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ScDesignExchangeUnpublished(quint32)),
			SIGNAL(ScDesignExchangeUnpublished(quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
			SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
			SIGNAL(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
			Qt::QueuedConnection);
}

DtChatChannel::~DtChatChannel()
{
	delete m_I;
}

void DtChatChannel::run()
{
	m_ThreadId = currentThreadId();

	// Creating the socket on the stack automatically associates it with the
	// thread's event loop. This ensures that Qt will not try to deliver events
	// to our socket from the main thread while we are accessing it from
	// DtChatChannel::run().
	QTcpSocket tcpSocket;
	if(!tcpSocket.setSocketDescriptor(m_socketDescriptor))
	{
		emit error(tcpSocket.error());
		return;
	}

	m_tmConnect.Set();
	//qDebug("DtChatChannel::run - Connection established.");
	QString peerAddr = tcpSocket.peerAddress().toString();
	QString logmsg = tr("CHAT: Connected %1").arg(peerAddr);
	g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);

	// Login check interval must be longer than the cache decay time in
	// the login server.
	const int loginCheckInterval = 20 * 60;	// seconds between login checks
	AuElapsedTimer tmLoginCheck;
	while(tcpSocket.state() == QAbstractSocket::ConnectedState && !tcpSocket.waitForDisconnected(20))
	{
		// Still connected so check to see if we have anything to send.
		m_muTxBuffer.lock();
		if(!m_TxBuffer.isEmpty())
		{
			// There is data to be sent.
			// It is already formatted into packets with block sizes.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				int written = tcpSocket.write(m_TxBuffer);
				m_BytesTx += written;
				//qDebug("DtChatChannel::run - %d bytes written out of %d", written,
						//m_TxBuffer.count());
				m_TxBuffer.clear();

				// Sometimes we must flush the output stream.
				if(m_TxBufferFlush)
				{
					tcpSocket.flush();
					m_TxBufferFlush = false;
				}
			}
		}
		m_muTxBuffer.unlock();
		if(tcpSocket.state() != QAbstractSocket::ConnectedState)
			break;

		// Pull in any data that is waiting for us.
		// Note: If we wait for the signal, it appears to come from the gui
		// thread, which causes us grief. It's better for us to get the data
		// when it's there.
		RxData(tcpSocket);
		if(tcpSocket.state() != QAbstractSocket::ConnectedState)
			break;

		// Check to see if the login server still thinks this guy is logged in.
		if(!gp_AuGlobal)
			break;
		if(!tmLoginCheck.isValid() || tmLoginCheck.elapsed()/d_MSecPerSec > loginCheckInterval)
		{
			CheckLoginStatus();
			tmLoginCheck.restart();
			if(tcpSocket.state() != QAbstractSocket::ConnectedState)
				break;
		}

		// When a disconnect time is established, we have to eventually disconnect.
		if(m_Disconnect)
		{
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
		}
	}

	//qDebug("DtChatChannel::run - Player %d disconnected.", m_idPlayer);
	logmsg = tr("CHAT: thread exit %1").arg(peerAddr);
	if(gp_AuGlobal)
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
}

void DtChatChannel::RxData(QTcpSocket& tcpSocket)
{
	//qDebug("DtChatChannel::RxData");
	while(tcpSocket.bytesAvailable())
	{
		if(!m_PacketSize || m_PacketSizeBytesLeft)
		{
			int packetSizeBytesRead = AuUtil::ReadPacketSize(tcpSocket, m_PacketSize, m_PacketSizeBytesLeft);
			if(m_PacketSizeBytesLeft)
				break;					// We don't have all of the packet size bytes yet.
			m_BytesRx += packetSizeBytesRead;
		}

		// Wait until all of the bytes arrive.
		if(tcpSocket.bytesAvailable() < m_PacketSize)
			break;

		// It is now safe to read our data packet from the socket buffer.
		QByteArray block = tcpSocket.read(m_PacketSize);
		m_BytesRx += m_PacketSize;
		m_PacketSize = 0;
		ReadPacket(block, tcpSocket);	// Read the contents of the packet.
	}
}

void DtChatChannel::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	//qDebug("DtChatChannel::ReadPacket - %d bytes rx'd", ba.count());
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	QString logmsg;

	// The first byte is always the packet type.
	quint8 type;
	block >> type;
	switch((DtChatChannelPacket::PACKET)type)	// Generate warning when enum forgotten.
	{
	case DtChatChannelPacket::PI_TurboLift:
		{
			quint8 doWhat;
			block >> doWhat;
			if(doWhat == 0)
			{
				AuId shipId;
				quint16 liftId;
				QString stop;

				block >> shipId;
				block >> liftId;
				block >> stop;

				m_I->CallTurboLift(shipId, liftId, stop);
			}
		}
		break;

	case DtChatChannelPacket::PZ_TickTock:
		{
			AuElapsedTimer timeIt;
			timeIt.start();
			AuCommThought msg1(tr("Forcing TickTock of the Etherium"), m_I->AvatarId(), m_UnitName);
			g_Au.SendAtomChat(msg1);

			AuEther& ether = g_Au.LockEtheriumDelete();
			ether.TickTock();
			g_Au.UnlockEtheriumDelete();

			AuCommThought msg2(tr("Etherium TickTock finished in %L1 sec.").arg(timeIt.elapsed()/d_MSecPerSec),
					m_I->AvatarId(), m_UnitName);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case DtChatChannelPacket::PZ_Trade:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Propose a new trade agreement at a broker.
				AuTradeAgreement ta;
				AuId opBd;

				block >> ta;
				block >> opBd;

				TradeAgreementCreate(ta, opBd);
			}
			else if(doWhat == 1)
			{
				// Delete a trade agreement at a broker.
				AuTradeAgreement ta;
				AuId opBd;

				block >> ta;
				block >> opBd;

				TradeAgreementDelete(ta, opBd);
			}
			else if(doWhat == 2)
			{
				// Accept a trade agreement at a broker.
				AuTradeAgreement ta;
				AuId opBd;

				block >> ta;
				block >> opBd;

				TradeAgreementAccept(ta, opBd);
			}
			else if(doWhat == 3)
			{
				// Counter a trade proposal at a broker.
				AuTradeAgreement ta, tac;
				AuId opBd;

				block >> ta;
				block >> tac;
				block >> opBd;

				TradeAgreementCounter(ta, tac, opBd);
			}
			else if(doWhat == 4)
			{
				// Create a nu shipment at a broker.
				AuShipment shipment;
				AuId opBd;

				block >> shipment;
				block >> opBd;

				ShipmentCreate(shipment, opBd);
			}
			else if(doWhat == 5)
			{
				// Delete a shipment at a broker.
				AuShipment shipment;
				AuId opBd;

				block >> shipment;
				block >> opBd;

				ShipmentDelete(shipment, opBd);
			}
			else if(doWhat == 6)
			{
				// Refresh shipments at a broker.
				AuId opBd;
				bool sayResult;

				block >> opBd;
				block >> sayResult;

				ShipmentsRefresh(opBd, sayResult);
			}
			else if(doWhat == 7)
			{
				// Create a nu shipment aboard a station.
				AuShipment shipment;

				block >> shipment;

				ShipmentCreate(shipment);
			}
			else if(doWhat == 8)
			{
				// Delete a shipment aboard a station.
				AuShipment shipment;

				block >> shipment;

				ShipmentDelete(shipment);
			}
			else if(doWhat == 9)
			{
				// Refresh shipments aboard a station.
				bool sayResult;

				block >> sayResult;

				ShipmentsRefresh(sayResult);
			}
		}
		break;

	case DtChatChannelPacket::PZ_QueryAvatars:
		QueryAvatars();
		break;

	case DtChatChannelPacket::PTR_Place:
		SendTransporterPlaces();
		break;
	case DtChatChannelPacket::PTR_Ship:
		SendTransporterSpacecraft();
		break;
	case DtChatChannelPacket::PTR_Transporter:
		SendTransporterTransporters();
		break;

	case DtChatChannelPacket::PI_Insurer:
		{
			quint8 doWhat;
			block >> doWhat;
			if(doWhat == 0)
			{
				double amt;
				block >> amt;
				m_I->PayInsurer(amt);
			}
		}
		break;

	case DtChatChannelPacket::PC_PostBounty:			// Post a bounty on a DaUnit
		{
			AuId victimId;
			QString victimName;
			QString reason;
			double reward;

			block >> victimId;
			block >> victimName;
			block >> reason;
			block >> reward;

			m_I->PostBounty(victimId, victimName, reason, reward);
		}
		break;

	case DtChatChannelPacket::PC_Connect:	// Connection information.
		// Client sent us their login player id and confirmation number issued by the login server.
		// Client will send this packet more than once when they have created a nu character. The first time the unit id will
		// be 0. After their nu emperor/character is created, the client will send me another PC_Connect packet with the unit id.
		{
			// Read player id and login confirmation number from packet.
			quint32 idPlayer, confirmNum;
			AuId idUnit;
			block >> idPlayer;
			block >> confirmNum;
			block >> idUnit;
			//qDebug("PC_Connect: player %d unitId %d", idPlayer, idUnit.GetID());

			AuId idUnits[6];
			ACCOUNT acctType;
			AuTime acctExpiry;
			if(g_Au.IsPlayerLoggedIn(idPlayer, confirmNum, idUnits, acctType, acctExpiry))
			{
				m_AccountType = acctType;
				m_confirmNum = confirmNum;
				m_idPlayer = idPlayer;

				// Emit a signal to insure no other channels are open to the player.
				emit PlayerConnecting(idPlayer, (unsigned long)m_ThreadId);

				// Verify the client passed us a valid unit id. It must be listed in the array of six ids that belong to the
				// player, as returned by the login server.
				if(!idUnit.IsNull())
				{
					for(int iu=0; iu<6; iu++)
						if(idUnit == idUnits[iu])
						{
							SetCurrentPerson(idUnit);
							break;
						}
				}

				AuTime disconnectTime;
				if(g_Au.IsSceneWorker() && !UnitId().IsNull())
				{
					// Connect all comm channels the client wants open.
					quint8 channelsOpen;
					block >> channelsOpen;
					if(channelsOpen)
					{
						QString channelName;
						m_muChannels.lock();
						m_Channels.clear();		// Client sent us a complete list; discard any old channels in our list.
						for(int chan=0; chan<channelsOpen; chan++)
						{
							block >> channelName;
							if(!m_Channels.contains(channelName))
								m_Channels.append(channelName);
						}
						m_muChannels.unlock();
					}
					disconnectTime << block;

					// Squelch ids are sent right away so we don't send comm messages from them.
					{
						quint16 n;
						AuId key;
						bool val;

						m_SquelchIds.clear();
						block >> n;
						while(n--)
						{
							block >> key;
							block >> val;
							m_SquelchIds.insert(pair<AuId, bool>(key, val));
						}

						// Prevent dumb users from squelching themself.
						unordered_map<AuId, bool, AuIdHash>::iterator mi = m_SquelchIds.find(UnitId());
						if(mi != m_SquelchIds.end())
							m_SquelchIds.erase(mi);
					}

					// Emoji customizations.
					m_Emojis.LoadStandardEmojis();
					m_Emojis.LoadVisages(block);

					if(m_UnitName.isEmpty())
					{
						logmsg = tr("CHAT: DISCONNECT! Player %2 confirm %3, NO PERSON NAME.").arg(idPlayer).arg(confirmNum);
						g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);

						m_AccountType = ACCT_Count;
						m_confirmNum = 0;
						m_idPlayer = 0;
						m_I->SetAvatarId(AuId());
						Disconnect();
						break;
					}
				}

				if(disconnectTime.IsNull())
					logmsg = tr("CHAT: Connect player %2 confirm %3 unit %4 %5 type %6.")
						.arg(idPlayer)
						.arg(confirmNum).arg(UnitId().GetID()).arg(m_UnitName)
						.arg(AcctTypeName(acctType));
				else
					logmsg = tr("CHAT: Connect player %2 confirm %3 unit %4 %5 type %6 disconnectTime %7.")
						.arg(idPlayer)
						.arg(confirmNum).arg(UnitId().GetID()).arg(m_UnitName)
						.arg(AcctTypeName(acctType))
						.arg(disconnectTime.UTC().toString());
				g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);

				// Send chat messages to them that have accumulated since they disconnected.
				if(g_Au.IsSceneWorker())
				{
					// At initial login, the time of last disconnect will be invalid. Send the last 5 minutes worth of messages so
					// they can get caught up. Maybe they crashed or had to quickly relog for some reason.
					if(disconnectTime.IsNull())
						disconnectTime = AuTime::Now().AddMins(-5);

					if(!disconnectTime.IsNull() && disconnectTime < AuTime::Now())
					{
						const AuAtomChatList* chat = g_AuDb.LockRecentAtomChat();
						for(QList<AuAtomChat>::const_iterator it = chat->cbegin(); it != chat->cend(); it++)
							if((*it).TimeStamp() >= disconnectTime && (*it).CommPacket())
							{
								AuCommPacket* msg = (*it).CommPacket()->Clone();
								if(msg)
								{
									ChatToClient(msg);
									delete msg;
								}
							}
						g_AuDb.UnlockRecentAtomChat();
					}
				}
			}
			else
			{
				m_AccountType = ACCT_Count;
				m_confirmNum = 0;
				m_idPlayer = 0;
				m_I->SetAvatarId(AuId());
				Disconnect();

				logmsg = tr("CHAT: DISCONNECT! Chat Connect failed: player %2 confirm %3").arg(idPlayer).arg(confirmNum);
				g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			}
		}
		break;

	case DtChatChannelPacket::PC_Emoji:
		{
			// Emoji customizations.
			m_Emojis.LoadStandardEmojis();
			m_Emojis.LoadVisages(block);
		}
		break;

	case DtChatChannelPacket::PC_SquelchUpdate:	// Squelch list (hash)
		{
			quint16 n;
			AuId key;
			bool val;

			m_SquelchIds.clear();
			block >> n;
			while(n--)
			{
				block >> key;
				block >> val;
				m_SquelchIds.insert(pair<AuId, bool>(key, val));
			}

			// Prevent dumb users from squelching themself.
			unordered_map<AuId, bool, AuIdHash>::iterator mi = m_SquelchIds.find(UnitId());
			if(mi != m_SquelchIds.end())
				m_SquelchIds.erase(mi);
		}
		break;

	case DtChatChannelPacket::PC_SquelchAdd:	// Add an id to the Squelch list (hash)
		{
			AuId idSquelch;
			block >> idSquelch;

			// Prevent dumb users from squelching themself.
			if(idSquelch != UnitId())
				m_SquelchIds.insert(pair<AuId, bool>(idSquelch, false));
		}
		break;

	case DtChatChannelPacket::PC_SquelchDelete:	// Delete an id from the Squelch list (hash)
		{
			AuId idSquelch;

			block >> idSquelch;

			unordered_map<AuId, bool, AuIdHash>::iterator mi = m_SquelchIds.find(idSquelch);
			if(mi != m_SquelchIds.end())
				m_SquelchIds.erase(mi);
		}
		break;

	case DtChatChannelPacket::PS_StorySubmission:
		{
			quint8 doWhat;

			block >> doWhat;

			switch(doWhat)
			{
			case 0:		// Receive a script.
				{
					AuScript script;
					AuId forWho;

					block >> forWho;
					block >> script;

					m_I->StoryScriptSubmitted(forWho, script);
				}
				break;

			case 1:		// Clear custom script.
				{
					AuId forWho;

					block >> forWho;

					m_I->StoryClearScript(forWho);
				}
				break;

			default:
				qWarning() << "HACKER WARNING! DtChatChannel::ReadPacket" << UnitId() << "PS_StorySubmission bad doWhat" << doWhat;
				break;
			}
		}
		break;

	case DtChatChannelPacket::PZ_ResetStory:
		if(AcctIsAgent(m_AccountType))
		{
			AuId avatarId;
			quint32 scriptId;

			block >> avatarId;
			block >> scriptId;

			m_I->AgentResetStory(avatarId, scriptId);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_ResetStory "
						"person %2. NOT ADMIN!")
					.arg(m_idPlayer).arg(UnitId().GetID()));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_Capacitor:
		if(AcctIsAgent(m_AccountType))
		{
			quint8 fillOrDrain;	// 0=drain, 1=fill
			quint8 percent;

			block >> fillOrDrain;
			block >> percent;

			FillOrDrainCapacitor(fillOrDrain, percent);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_Capacitor "
						"person %2. NOT ADMIN!")
					.arg(m_idPlayer).arg(UnitId().GetID()));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_Encounter:
		if(AcctIsAgent(m_AccountType))
		{
			quint8 type;
			quint32 designId;

			block >> type;
			block >> designId;

			m_I->AgentEncounter(type, designId);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(),
					QString("CHAT: DISCONNECT! Player %1: Attempted PZ_AgentBits person %2. NOT ADMIN!")
					.arg(m_idPlayer).arg(UnitId().GetID()));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_AgentBits:
		if(AcctIsAgent(m_AccountType) || AcctIsMinion(m_AccountType))
		{
			quint8 agentBits;

			block >> agentBits;

			m_I->AgentSetBits(agentBits);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_AgentBits person %2. NOT ADMIN!")
					.arg(m_idPlayer).arg(UnitId().GetID()));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_KickStowaways:
		if(AcctIsAgent(m_AccountType))
		{
			m_I->KickStowaways();
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_KickStowaways person %2. NOT ADMIN!")
					.arg(m_idPlayer).arg(UnitId().GetID()));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_KillAnimals:
		//qDebug("PZ_KillAnimals:");
		{
			quint8 killWhat;
			block >> killWhat;	// 0=domestic animals, 1=wild animals
			if(AcctIsAgent(m_AccountType))
			{
				KillAnimals(killWhat);
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_KillAnimals person %2 killWhat %3. "
							"NOT ADMIN!").arg(m_idPlayer).arg(UnitId().GetID()).arg(killWhat));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_ChangeShip:
		//qDebug("PZ_ChangeShip:");
		{
			quint8 changeType;

			block >> changeType;

			if(changeType == 0 && AcctIsProgrammer(m_AccountType))
			{
				QString reason;

				block >> reason;

				m_I->AgentScuttleShip(reason);
			}
			else if(changeType == 1 && AcctIsArchitect(m_AccountType))
			{
				m_I->ToggleShipOwnership();
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_ChangeShip person %2. NOT ADMIN!")
						.arg(m_idPlayer).arg(UnitId().GetID()));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_SetDNA:
		//qDebug("PZ_SetDNA:");
		if(AcctIsProgrammer(m_AccountType))
		{
			AuId chgWho;
			AuDNABase* nuDNA;

			block >> chgWho;
			nuDNA = AuDNABase::ReadDynamic(block);
			if(nuDNA)
			{
				m_I->AgentSetDNA(chgWho, *nuDNA);
				delete nuDNA;
			}
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1: Attempted PZ_SetDNA person %2. "
						"NOT ADMIN!")
					.arg(m_idPlayer).arg(UnitId().GetID()));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_EnvironmentChange:
		//qDebug("PZ_EnvironmentChange:");
		{
			quint8 chgWhat;
			block >> chgWhat;	// 0=ground cover, 1=shrubbery, 2=trees, 3=set indigenous DNA to match agent,
								// 4=clear indigenous DNA, 5=clear rat DNA, 6=set citizen DNA to match agent
			qint8 floraZone;	// 0=polar, 1=temperate, 2=tropical
			block >> floraZone;
			if(AcctIsProgrammer(m_AccountType))
			{
				RegenDNA(chgWhat, floraZone);
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_EnvironmentChange chgWhat %4 "
							"floraZone %5. NOT ADMIN!")
						.arg(m_idPlayer).arg(UnitId().GetID()).arg(m_UnitName)
						.arg(chgWhat).arg(floraZone));
				Disconnect();
			}
		}
		break;
	case DtChatChannelPacket::PZ_GoToAtom:
		//qDebug("PZ_GoToAtom:");
		{
			AuId destAtomId;
			bool rideAlso;
			bool limboRescue;

			block >> destAtomId;
			block >> rideAlso;
			block >> limboRescue;

			if(AcctIsAgent(m_AccountType))
			{
				if(destAtomId != UnitId() &&						// Not agent's self
						(AcctIsArchitect(m_AccountType) ||
						 destAtomId != AuId::Haxus()))
					m_I->AgentGoToAtom(destAtomId, rideAlso, limboRescue && AcctIsArchitect(m_AccountType));
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(),
						QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_GoToAtom to atom %4 ride %5. "
							"NOT ADMIN!")
						.arg(m_idPlayer).arg(UnitId().GetID()).arg(m_UnitName)
						.arg(destAtomId.GetID()).arg(rideAlso?'Y':'N'));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_GoToGalaxyLoc:
		{
			quint8 galaxyEnum;
			AuPoint galaxyLoc;
			bool rideAlso;

			block >> galaxyEnum;
			block >> galaxyLoc;
			block >> rideAlso;

			if(AcctIsAgent(m_AccountType))
			{
				m_I->AgentGoToGalaxyLoc(galaxyEnum, galaxyLoc, rideAlso);
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(),
						QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_GoToGalaxyLoc to %4 ride %5. NOT ADMIN!")
						.arg(m_idPlayer)
						.arg(m_I->AvatarId().GetID())
						.arg(m_UnitName)
						.arg(GalaxyName((GALAXY)galaxyEnum))
						.arg(rideAlso?'Y':'N'));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_GoToLongLat:
		//qDebug("PZ_GoToLongLat:");
		{
			AuGlobeLLF ll;
			bool rideAlso;

			block >> ll;
			block >> rideAlso;

			m_I->AgentGoToLongLat(ll, rideAlso);
		}
		break;

	case DtChatChannelPacket::PZ_SummonPerson:
		//qDebug("PZ_SummonPerson:");
		{
			AuId atomId;
			bool rideAlso;
			block >> atomId;
			block >> rideAlso;
			if(AcctIsAgent(m_AccountType))
			{
				emit AdminSummon(atomId, m_I->AvatarId(), rideAlso?2:0);
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_SummonPerson to atom %4 ride %5. "
							"NOT ADMIN!")
						.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
						.arg(atomId.GetID()).arg(rideAlso?'Y':'N'));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_EmpireChg:
		if(AcctIsAgent(m_AccountType))
		{
			quint8 chgWhat;
			block >> chgWhat;
			switch(chgWhat)
			{
			case 0:
				// Pirate building pref.
				{
					quint32 empireId;
					quint16 iid;
					quint32 designId;
					block >> empireId;
					block >> iid;
					block >> designId;
					m_I->EmpireChangeBuildingPref(empireId, iid, designId);
				}
				break;
			case 1:
				// Pirate spacecraft pref.
				{
					quint32 empireId;
					quint8 scrole;
					quint32 designId;
					block >> empireId;
					block >> scrole;
					block >> designId;
					m_I->EmpireChangeSpacecraftPref(empireId, scrole, designId);
				}
				break;
			}
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(),
					QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_EmpireChg. NOT ADMIN!")
					.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_EmpireChgFlag:
		//qDebug("PZ_EmpireChgFlag:");
		{
			quint32 empireId;
			QImage flag;
			block >> empireId;
			block >> flag;
			if(AcctIsAgent(m_AccountType))
			{
				//qDebug("%x Player %d: PZ_EmpireChgFlag empire %d flag %s %dx%d",
						//(unsigned int)currentThreadId(), m_idPlayer,
						//empireId, flag.isNull()?"NOT VALID":"valid", flag.width(), flag.height());
				emit EmpireChgFlag(empireId, flag);

				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(empireId, empireName);
				g_Au.UnlockData();

				// Send the result on the empire channel.
				QString resultMsg = tr("%1 flag changed.").arg(empireName);
				AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), m_I->AvatarId(), m_UnitName, UnitSector(),
						UnitGalaxy(), AuCommStdChannel::Universe, AuCommStdChannel::Empire);
				empAnnounce.SetScopeData(empireId);
				g_Au.SendAtomChat(empAnnounce);

				// Notify agent since they may not be in that empire and won't get that message.
				AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
				g_Au.SendAtomChat(msg);
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(),
						QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_EmpireChgFlag empire %4 "
							"flag %5 %6x%7. NOT ADMIN!")
						.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
						.arg(empireId).arg(flag.isNull()?"NOT VALID":"valid").arg(flag.width()).arg(flag.height()));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_EmpireChgName:
		//qDebug("PZ_EmpireChgName:");
		{
			quint32 empireId;
			QString name;

			block >> empireId;
			block >> name;

			if(AcctIsAgent(m_AccountType))
			{
				// Insure there is not already an empire by that name. We don't allow duplicate empire names.
				const DgData& gd = g_Au.LockDataForRead();
				quint32 exemp = gd.EmpireId(name);	// returns 0 if the empire 'name' does not exist
				g_Au.UnlockData();
				if(!exemp)
				{
					//qDebug("%x Player %d: PZ_EmpireChgName empire %d name %s",
							//(unsigned int)currentThreadId(), m_idPlayer, empireId, (const char*)name.toAscii());
					emit EmpireChgName(empireId, name);

					QString empireName;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName(empireId, empireName);
					g_Au.UnlockData();

					// Send the result on the empire channel.
					QString resultMsg = tr("%1 name changed to %2.").arg(empireName).arg(name);
					AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), m_I->AvatarId(), m_UnitName, UnitSector(),
							UnitGalaxy(), AuCommStdChannel::Universe, AuCommStdChannel::Empire);
					empAnnounce.SetScopeData(empireId);
					g_Au.SendAtomChat(empAnnounce);

					// Notify agent since they may not be in that empire and won't get that message.
					AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
					g_Au.SendAtomChat(msg);
				}
				else
				{
					//qDebug("%x Player %d: PZ_EmpireChgName empire %d name %s NOT UNIQUE. Name unchanged.",
							//(unsigned int)currentThreadId(), m_idPlayer, empireId, (const char*)name.toAscii());
				}
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(),
						QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_EmpireChgName empire %4 name %5. NOT ADMIN!")
						.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
						.arg(empireId).arg(name));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_EmpireChgRole:
		//qDebug("PZ_EmpireChgRole:");
		{
			quint32 empireId;
			quint8 gameRole;

			block >> empireId;
			block >> gameRole;

			if(AcctIsAgent(m_AccountType))
			{
				emit EmpireChgRole(empireId, gameRole);

				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(empireId, empireName);
				g_Au.UnlockData();

				QString roleName;
				switch(gameRole)
				{
				case 0:
					roleName = "Normal Empire";
					break;
				case 1:
					roleName = "Pirate Empire";
					break;
				case 2:
					roleName = "Agent Empire";
					break;
				case 3:
					roleName = "Arena Empire";
					break;
				default:
					roleName = "Unknown";
					break;
				}

				// Send the result on the empire channel.
				QString resultMsg = tr("%1 role changed to %2.").arg(empireName).arg(roleName);
				AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), m_I->AvatarId(), m_UnitName, UnitSector(),
						UnitGalaxy(), AuCommStdChannel::Universe, AuCommStdChannel::Empire);
				empAnnounce.SetScopeData(empireId);
				g_Au.SendAtomChat(empAnnounce);

				// Notify agent since they may not be in that empire and won't get that message.
				AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
				g_Au.SendAtomChat(msg);
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(),
						QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_EmpireChgRole empire %4 role %5. NOT ADMIN!")
						.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
						.arg(empireId).arg(gameRole));
				Disconnect();
			}
		}
		break;

	case DtChatChannelPacket::PZ_EmpireChgEmperor:
		if(AcctIsAgent(m_AccountType))
		{
			quint32 empireId;
			AuId nuEmperor;
			QString nuEmpName;

			block >> empireId;
			block >> nuEmperor;
			block >> nuEmpName;

			AgentAssignEmperor(empireId, nuEmperor, nuEmpName);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 unit %2 %3: "
						"Attempted PZ_EmpireChgEmperor. NOT ADMIN!")
					.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_MakeCommodity:
		if(AcctIsProgrammer(m_AccountType) || AcctIsMinion(m_AccountType))
		{
			quint8 makeWhat;
			quint16 cid;
			quint8 quality;
			quint32 count;
			quint8 destGalaxy;
			AuPoint destGalaxyLoc;

			block >> cid;
			block >> quality;
			block >> count;
			block >> destGalaxy;
			block >> destGalaxyLoc;
			block >> makeWhat;

			AgentMakeCommodity(makeWhat, (COMMODITYID)cid, quality, count, (GALAXY)destGalaxy, destGalaxyLoc);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted PZ_MakeCommodity. NOT ADMIN!")
					.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PZ_Refuel:
		if(AcctIsAgent(m_AccountType) || AcctIsMinion(m_AccountType))
		{
			quint8 quality;

			block >> quality;

			if(quality)
				AgentRefuel(quality);
			else
				AgentRepair();
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Attempted Refuel. NOT ADMIN!")
					.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PU_SetAmmo:
		//qDebug("PU_SetAmmo:");
		{
			AuId unitId;
			quint16 ammoCID;

			block >> unitId;
			block >> ammoCID;

			m_I->SetUnitAmmo(unitId, (COMMODITYID)ammoCID);
		}
		break;
	case DtChatChannelPacket::PU_SetAmmoCount:
		//qDebug("PU_SetAmmoCount:");
		{
			AuId unitId;
			quint8 ammoCount;

			block >> unitId;
			block >> ammoCount;

			m_I->SetUnitAmmoCount(unitId, ammoCount);
		}
		break;
	case DtChatChannelPacket::PU_SetArmor:
		//qDebug("PU_SetArmor:");
		{
			AuId unitId;
			quint8 itemType;
			AuItem* item;

			block >> unitId;
			block >> itemType;
			item = AuItem::Create(itemType);
			if(item)
			{
				block >> *item;
				m_I->SetUnitArmor(unitId, *item);
				delete item;
			}
		}
		break;
	case DtChatChannelPacket::PU_SetClothing:
		//qDebug("PU_SetClothing:");
		{
			AuId unitId;
			quint8 itemType;
			AuItem* item;

			block >> unitId;
			block >> itemType;
			item = AuItem::Create(itemType);
			if(item)
			{
				block >> *item;
				m_I->SetUnitClothing(unitId, *item);
				delete item;
			}
		}
		break;
	case DtChatChannelPacket::PU_SetHelmet:
		//qDebug("PU_SetHelmet:");
		{
			AuId unitId;
			quint8 itemType;
			AuItem* item;

			block >> unitId;
			block >> itemType;
			item = AuItem::Create(itemType);
			if(item)
			{
				block >> *item;
				m_I->SetUnitHelmet(unitId, *item);
				delete item;
			}
		}
		break;
	case DtChatChannelPacket::PU_SetWeapon:
		//qDebug("PU_SetWeapon:");
		{
			AuId unitId;
			quint8 itemType;
			AuItem* item;

			block >> unitId;
			block >> itemType;
			item = AuItem::Create(itemType);
			if(item)
			{
				block >> *item;
				m_I->SetUnitWeapon(unitId, *item);
				delete item;
			}
		}
		break;

	case DtChatChannelPacket::PV_Dossier:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;
			quint32 empire;

			block >> doWhat;
			block >> empire;

			switch(doWhat)
			{
			case 0:		// Add a note
				{
					QString note;

					block >> note;

					m_I->GovDossierAddNote(empire, note);
				}
				break;

			case 1:		// Change state of a document.
				{
					AuTime docTime;	// Identifies the document.
					quint8 docType; // Identifies the document.
					quint16 docIndex;
					quint8 docState;

					block >> docTime;
					block >> docType;
					block >> docIndex;
					block >> docState;

					m_I->GovDossierDocState(empire, docTime, docType, docIndex, docState);
				}
				break;

			case 2:		// Send diplomatic message to another empire.
				{
					QString note;

					block >> note;

					m_I->GovDossierDispatchCourier(empire, note);
				}
				break;
			}
		}
		break;

	case DtChatChannelPacket::PV_ChangeTitle:
		//qDebug("PV_ReleaseTitle:");
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			switch(doWhat)
			{
			case 1:
				m_I->ClaimSpacecraftTitle();
				break;
			case 2:
				m_I->ReleaseSpacecraftTitle();
				break;
			case 3:
				{
					QString name;
					QImage flag;
					block >> name;
					block >> flag;
					m_I->CreateNewEmpire(name, flag);
				}
				break;
			case 4:
				{
					AuGearPos mediaPos;
					block >> mediaPos;
					m_I->CopySpacecraftTitle(mediaPos);
				}
				break;
			default:
				break;
			}
		}
		break;

	case DtChatChannelPacket::PV_Christen:
		//qDebug("PV_Christen:");
		if(g_Au.IsSceneWorker())
		{
			// Only the worker processes this message.
			QString shipName;
			block >> shipName;
			m_I->Christen(shipName);
		}
		break;

	case DtChatChannelPacket::PC_ChannelClosed:		// Client stopped listening on a comm channel.
		//qDebug("PC_ChannelClosed:");
		if(g_Au.IsSceneWorker())
		{
			// Only the worker processes this message. The master and
			// manager will receive this message but they ignore it.
			QString channelName;
			block >> channelName;
			m_muChannels.lock();
			int idx = m_Channels.indexOf(channelName);
			if(idx >= 0)
				m_Channels.removeAt(idx);
			m_muChannels.unlock();
		}
		break;
	case DtChatChannelPacket::PC_ChannelOpened:		// Client started listening on a comm channel.
		if(g_Au.IsSceneWorker())
		{
			// Only the worker processes this message. The master and manager will receive this message but they ignore it.
			QString channelName;

			block >> channelName;

			m_muChannels.lock();
			if(!channelName.isEmpty() && !m_Channels.contains(channelName))
				m_Channels.append(channelName);
			m_muChannels.unlock();
		}
		break;

	case DtChatChannelPacket::PC_HailStore:
		m_I->HailStore();
		break;

	case DtChatChannelPacket::PC_Chat:				// Chat message from client.
		//qDebug("PC_Chat:");
		if(g_Au.IsSceneWorker())
		{
			// Create an encryption key based on this user's login credentials.
			AuCommPacket* msg = AuCommPacket::RxDecrypt(block, AuUtil::NetCryptKey((quint16)m_confirmNum));
			if(!msg)
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Chat packet unrecognized! %1 unit %2 %3")
						.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
				m_BadPackets++;
				Disconnect();
				break;
			}

			// Impose a chat message frequency limit.
			// 'ChatContinued' messages are not limited by this filter for automated batches.
			if(!m_LastChat.isValid() ||
					m_LastChat.elapsed() >= 200 ||
					msg->Type() == AuCommPacket::ChatContinued)
			{
				m_LastChat.restart();
				// Universal announcements are sent by architects with a from id of 1.
				if(msg->FromId() == m_I->AvatarId() ||
						(msg->Type() == AuCommPacket::Announcement && AcctIsArchitect(m_AccountType)) ||
						(msg->Type() == AuCommPacket::Silverwill && AcctIsAgent(m_AccountType)))
				{
					// Architect's name sometimes changes when acting as entity or animal. 
					if(msg->FromName() == m_UnitName || AcctIsArchitect(m_AccountType))
					{
						// Propagate valid message to others on the same channel.
						if(msg->IsValid())
						{
							// The client cannot decide that a message is anonymous.
							if(!msg->IsSenderAnonymous())
								ChatReceivedFromClient(*msg);
							else
							{
								g_Au.LogConnect(auSceneServerApp->LoginName(),
										QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Anonymous Chat to channel %4 "
											"range %5 scopeData %6 type %7 '%8'")
										.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
										.arg(msg->ChannelName()).arg(msg->TransmissionRange()).arg(msg->ScopeData())
										.arg(msg->Type()).arg(msg->Text()));
								m_BadPackets++;
								Disconnect();
							}
						}
						else
						{
							if(g_Au.m_LogMessages & LOG_Error)
								g_Au.Log(QString("CHAT: ERROR! Player %1 unit %2 %3: Invalid Chat to channel %4 "
											"range %5 scopeData %6 type %7 '%8'")
										.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
										.arg(msg->ChannelName()).arg(msg->TransmissionRange()).arg(msg->ScopeData())
										.arg(msg->Type()).arg(msg->Text()));
							//g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 unit %2 %3: Invalid Chat to channel %4 "
										//"range %5 scopeData %6 type %7 '%8'")
									//.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
									//.arg(msg->ChannelName()).arg(msg->TransmissionRange()).arg(msg->ScopeData())
									//.arg(msg->Type()).arg(msg->Text()));
							//m_BadPackets++;
							//Disconnect();
						}
					}
					else
					{
						g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! From name forged! %1 unit %2 %3 "
									"msg %4 %5 %6 %7 %8 %9 %10")
								.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
								.arg(msg->FromId().GetID()).arg(msg->FromName()).arg(msg->ChannelName()).arg(msg->TransmissionRange())
								.arg(msg->ScopeData()).arg(msg->Type()).arg(msg->Text()));
						m_BadPackets++;
						Disconnect();
					}
				}
				else
				{
					g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 Unit %2 %3: Invalid Chat Sender Unit Id %4 to "
								"channel %5 range %6 scopeData %7 type %8 '%9'")
							.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName)
							.arg(msg->FromId().GetID()).arg(msg->ChannelName()).arg(msg->TransmissionRange()).arg(msg->ScopeData())
							.arg(msg->Type()).arg(msg->Text()));
					m_BadPackets++;
					Disconnect();
				}
			}
			else
			{
				qWarning() << currentThreadId() << "Chat message ignored. Message frequency exceeded." << m_idPlayer
					<< m_I->AvatarId() << m_UnitName << msg->FromId() << msg->FromName() << msg->ChannelName() << msg->TransmissionRange()
					<< msg->ScopeData() << msg->Type() << msg->Text();
			}
			delete msg;
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 Unexpected TCP packet type %2 size %3 unit %4 %5")
					.arg(m_idPlayer).arg(type).arg(ba.count()).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			m_BadPackets++;
			Disconnect();
		}
		break;

	case DtChatChannelPacket::PC_OpenChannelTo:		// Open channel to specific atom id.
		//qDebug("PC_OpenChannelTo:");
		// Only the worker processes this message. The master and manager will receive this message but they ignore it.
		if(g_Au.IsSceneWorker())
		{
			QString channelName;
			AuId toUnitId;

			block >> channelName;
			block >> toUnitId;

			m_muChannels.lock();
			if(!channelName.isEmpty() && !m_Channels.contains(channelName))
				m_Channels.append(channelName);
			m_muChannels.unlock();

			// Locate my sector and galaxy.
			AuId sectorId;
			quint8 galaxy = 0;
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(m_I->SystemId(), ether);
			if(system)
			{
				sectorId = system->ParentId();
				galaxy = system->Galaxy();
			}
			g_Au.UnlockEtheriumDelete();

			// Ok to propagate up when coming from this client connection.
			// Player channel opens to other players are always to 'Everyone' scope.
			emit OpenChannelTo(m_I->AvatarId(), m_UnitName, sectorId, galaxy, toUnitId, channelName, AuCommStdChannel::Everyone, true);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! Player %1 Unexpected TCP packet type %2 size %3 unit %4 %5")
					.arg(m_idPlayer).arg(type).arg(ba.count()).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			m_BadPackets++;
		}
		break;

	case DtChatChannelPacket::PN_Mission:
		//qDebug("PN_Mission:");
		if(g_Au.IsSceneWorker())
		{
			quint8 command;
			block >> command;
			switch(command)
			{
			case 0:				// Send mission to client.
				MissionSend(tcpSocket);
				break;
			case 1:				// Clear mission
				m_I->MissionClear();
				break;
			case 2:				// Append mission order.
				{
					quint8 orderType;

					block >> orderType;

					AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)orderType);
					if(order)
					{
						block >> *order;
						if(order->IsValid())
						{
							//qDebug() << "DtChatChannel::ReadPacket PN_Mission append order"
								//<< order->ListName() << order->ListDescription();
							m_I->MissionAppendOrder(*order);
						}
						delete order;
					}
				}
				break;
			case 3:				// Insert mission order.
				{
					quint8 beforeId;
					quint8 orderType;

					block >> beforeId;
					block >> orderType;

					AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)orderType);
					if(order)
					{
						block >> *order;
						if(order->IsValid())
							m_I->MissionInsertOrder(beforeId, *order);
						delete order;
					}
				}
				break;
			case 4:				// Delete mission order.
				{
					quint8 orderId;

					block >> orderId;

					m_I->MissionDeleteOrder(orderId);
				}
				break;
			case 5:				// Set mission type
				{
					quint8 missionType;
					block >> missionType;
					if(missionType <= 1)
						m_I->MissionSetType(missionType);
				}
				break;
			case 6:				// Set mission active
				{
					quint8 missionActive;
					block >> missionActive;
					m_I->MissionSetActive(missionActive?true:false);
				}
				break;
			case 7:				// mission order complete
				m_I->MissionOrderComplete();
				break;
			case 8:				// nu mission starting with attached order
				{
					quint8 orderType;

					block >> orderType;

					AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)orderType);
					if(order)
					{
						block >> *order;
						if(order->IsValid())
						{
							m_I->MissionClear();
							m_I->MissionAppendOrder(*order);
						}
						delete order;
					}
				}
				break;
			case 9:				// insert attached order as current mission order
				{
					quint8 orderType;

					block >> orderType;

					AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)orderType);
					if(order)
					{
						block >> *order;
						if(order->IsValid())
							m_I->MissionInsertOrder(0, *order);
						delete order;
					}
				}
				break;
			case 10:			// Burn a mission CD of the current mission.
				{
					QString missionName;
					AuGearPos mediaPos;
					block >> missionName;
					block >> mediaPos;

					m_I->MissionBurnCD(mediaPos, missionName);
				}
				break;
			case 11:			// nu mission
				{
					AuMission mission;
					mission << block;
					m_I->MissionNew(mission);
				}
				break;
			case 12:			// append mission
				{
					AuMission mission;
					mission << block;
					m_I->MissionAppend(mission);
				}
				break;
			case 13:			// Delete current mission order
				m_I->MissionDeleteCurrentOrder();
				break;
			case 14:			// Append mission order and activate
				{
					quint8 orderType;

					block >> orderType;

					AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)orderType);
					if(order)
					{
						block >> *order;
						if(order->IsValid())
						{
							m_I->MissionAppendOrder(*order);
							m_I->MissionSetActive(true);
						}
						delete order;
					}
				}
				break;
			case 15:			// append mission and activate
				{
					AuMission mission;
					mission << block;
					if(!mission.IsEmpty())
					{
						m_I->MissionAppend(mission);
						m_I->MissionSetActive(true);
					}
				}
				break;
			case 16:				// mission order complete and activate
				m_I->MissionOrderComplete();
				m_I->MissionSetActive(true);
				break;
			case 17:				// request sensor contacts useful for mission orders
				MissionSendSensorContacts(tcpSocket);
				break;
			case 18:				// Replace mission order, by id.
				{
					quint8 orderId;
					quint8 orderType;

					block >> orderId;
					block >> orderType;

					AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)orderType);
					if(order)
					{
						block >> *order;
						if(order->IsValid())
							m_I->MissionReplaceOrder(orderId, *order);
						delete order;
					}
				}
				break;
			}
		}
		break;

	case DtChatChannelPacket::PD_Position:
		{
			quint8 doWhat;
			block >> doWhat;
			switch(doWhat)
			{
			case 1:	// Restore avatar position.
				{
					AuId avatarId;
					quint8 number;
					block >> avatarId;
					block >> number;
					m_I->DesignerViewPositionLoad(avatarId, number);
				}
				break;
			case 2:	// Save avatar position.
				{
					AuId avatarId;
					quint8 number;
					block >> avatarId;
					block >> number;
					m_I->DesignerViewPositionSave(avatarId, number);
				}
				break;
			case 3:	// Position avatar to origin.
				{
					AuId avatarId;
					block >> avatarId;
					m_I->DesignerViewPositionOrigin(avatarId);
				}
				break;
				/*
			case 4:	// Restore grid position.
				{
					AuId avatarId;
					quint8 number;
					block >> avatarId;
					block >> number;
					DesignerGridPositionLoad(avatarId, number);
				}
				break;
				*/
			case 5:	// Save grid position.
				{
					quint8 number;
					AuMatrix4x4 gridM;
					block >> number;
					block >> gridM;
					m_I->DesignerGridPositionSave(number, gridM);
				}
				break;
				/*
			case 6:	// Position grid to origin.
				{
					AuId avatarId;
					block >> avatarId;
					DesignerGridPositionOrigin(avatarId);
				}
				break;
				*/
			case 7:	// Position avatar to grid plane.
				{
					AuId avatarId;
					AuDesignPosition pos;
					block >> avatarId;
					block >> pos;
					m_I->DesignerViewPositionGrid(avatarId, pos);
				}
				break;
			}
		}
		break;

	case DtChatChannelPacket::PD_BdLoaded:
	case DtChatChannelPacket::PD_ItLoaded:
	case DtChatChannelPacket::PD_ScLoaded:
		if(g_Au.IsSceneWorker())
		{
			QByteArray bcomp;

			block >> bcomp;

			if(bcomp.isEmpty())
				break;
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
			AuDesign* design = AuDesign::Unpack(ba);
			if(design)
			{
				m_I->DesignerLoadedFromFile(*design);
				delete design;
			}
		}
		break;

	case DtChatChannelPacket::PD_Redo:
		if(g_Au.IsSceneWorker())
			m_I->DesignerRedo();
		break;

	case DtChatChannelPacket::PD_Undo:
		if(g_Au.IsSceneWorker())
			m_I->DesignerUndo();
		break;

	case DtChatChannelPacket::PD_ScPart:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Add nu part;
				QByteArray ba;

				block >> ba;
				
				AuScPart* s = AuScPart::Unpack(ba);
				if(s)
				{
					m_I->DesignerPartAdd(*s);
					delete s;
				}
			}
			else if(doWhat == 1)
			{
				AuScIdSet partIds;
				bool seal;

				block >> partIds;
				block >> seal;

				m_I->DesignerPartsSetSeal(partIds, seal);
			}
			else if(doWhat == 2)
			{
				// Update part
				quint16 sid;
				QByteArray ba;

				block >> sid;
				block >> ba;
				
				AuScPart* s = AuScPart::Unpack(ba);
				if(s)
				{
					m_I->DesignerPartUpdate(sid, *s);
					delete s;
				}
			}
			else if(doWhat == 3)
			{
				// Copy selected parts.
				AuScSelection sel;
				AuMatrix4x4 t;

				block >> sel;
				block >> t;

				m_I->DesignerCopyParts(sel, t);
			}
			else if(doWhat == 4)
			{
				// Move selected parts.
				AuScSelection sel;
				AuMatrix4x4 t;

				block >> sel;
				block >> t;

				m_I->DesignerMoveParts(sel, t);
			}
			else if(doWhat == 5)
			{
				// Delete selected parts.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerDeleteParts(sel);
			}
			else if(doWhat == 6)
			{
				// Add parts.
				AuScParts parts;

				block >> parts;

				m_I->DesignerAddParts(parts);
			}
			else if(doWhat == 7)
			{
				// Copy selected part objects.
				AuScSelection sel;
				AuMatrix4x4 t;

				block >> sel;
				block >> t;

				m_I->DesignerCopyPartObjects(sel, t);
			}
			else if(doWhat == 8)
			{
				// Move selected parts.
				AuScSelection sel;
				AuMatrix4x4 t;

				block >> sel;
				block >> t;

				m_I->DesignerMovePartObjects(sel, t);
			}
			else if(doWhat == 9)
			{
				// Delete selected part objects.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerDeletePartObjects(sel);
			}
			else if(doWhat == 10)
			{
				// Add part objects.
				quint16 partId;
				AuScParts parts;

				block >> partId;
				block >> parts;

				m_I->DesignerAddPartObjects(partId, parts);
			}
			else if(doWhat == 11)
			{
				// Join selected parts.
				quint16 partId;
				AuScSelection sel;

				block >> partId;
				block >> sel;

				m_I->DesignerJoinParts(partId, sel);
			}
			else if(doWhat == 12)
			{
				// Reverse selected faces.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerReverseFaces(sel);
			}
			else if(doWhat == 13)
			{
				// Split selected faces.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerSplitFaces(sel);
			}
			else if(doWhat == 14)
			{
				// Flatten selected vertices of selected faces.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerFlattenVertices(sel);
			}
			else if(doWhat == 15)
			{
				// Smooth selected vertices of selected faces.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerSmooth(sel);
			}
			else if(doWhat == 16)
			{
				// Jig cut parts.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerJigCut(sel);
			}
			else if(doWhat == 17)
			{
				// Fill boundary on selected part.
				AuScSelection sel;
				AuPointArray boundary;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> boundary;
				block >> gridPlane;

				m_I->DesignerPartFillFace(sel, boundary, gridPlane);
			}
			else if(doWhat == 18)
			{
				// Set stop name.
				quint16 partId;
				QString name;

				block >> partId;
				block >> name;

				m_I->DesignerPartSetStopName(partId, name);
			}
			else if(doWhat == 19)
			{
				// Set part state.
				quint16 partId;
				quint8 partState;

				block >> partId;
				block >> partState;

				m_I->DesignerPartSetState(partId, partState);
			}
			else if(doWhat == 20)
			{
				// Set hidden
				AuScIdSet partIds;
				bool hide;

				block >> partIds;
				block >> hide;

				m_I->DesignerPartsSetHidden(partIds, hide);
			}
			else if(doWhat == 21)
			{
				// Set locked
				AuScIdSet partIds;
				bool lock;

				block >> partIds;
				block >> lock;

				m_I->DesignerPartsSetLocked(partIds, lock);
			}
			else if(doWhat == 22)
			{
				AuScIdSet partIds;
				bool omit;

				block >> partIds;
				block >> omit;

				m_I->DesignerPartsSetOmitted(partIds, omit);
			}
			else if(doWhat == 23)
			{
				// Add state
				quint16 partId;
				quint8 state;

				block >> partId;
				block >> state;

				m_I->DesignerPartAddState(partId, state);
			}
			else if(doWhat == 24)
			{
				// Add state
				quint16 partId;
				AuScFaceList faces;

				block >> partId;
				block >> faces;

				m_I->DesignerPartAddFaces(partId, faces);
			}
			else if(doWhat == 25)
			{
				// Delete state
				quint16 partId;
				quint8 state;

				block >> partId;
				block >> state;

				m_I->DesignerPartDeleteState(partId, state);
			}
			else if(doWhat == 26)
			{
				// Replace faces.
				quint16 partId;
				AuScSelection sel;
				AuScFaceList faces;

				block >> partId;
				block >> sel;
				block >> faces;

				m_I->DesignerPartReplaceFaces(partId, sel, faces);
			}
			else if(doWhat == 27)
			{
				// Replace faces.
				AuScSelection sel;
				AuPointArray cutLine;

				block >> sel;
				block >> cutLine;

				m_I->DesignerCutFaces(sel, cutLine);
			}
			else if(doWhat == 28)
			{
				// Fracture faces.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerFractureFaces(sel);
			}
			else if(doWhat == 29)
			{
				// Fuse faces.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerFuseFaces(sel);
			}
			else if(doWhat == 30)
			{
				// Divide faces.
				AuScSelection sel;
				quint8 divisions;

				block >> sel;
				block >> divisions;

				m_I->DesignerDivideFaces(sel, divisions);
			}
			else if(doWhat == 31)
			{
				AuScSelection sel;
				QColor color;

				block >> sel;
				block >> color;

				m_I->DesignerMakeNavLightLens(sel, color);
			}
			else if(doWhat == 32)
			{
				// Make door.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeDoor(sel);
			}
			else if(doWhat == 33)
			{
				// Set light lens properties.
				AuScSelection sel;
				QColor color;

				block >> sel;
				block >> color;

				m_I->DesignerPartsSetLightLensProperties(sel, color);
			}
			else if(doWhat == 34)
			{
				// Make landing gear.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeLandingGear(sel);
			}
			else if(doWhat == 35)
			{
				// Make rotating beacon.
				AuScSelection sel;
				QColor lightColor;
				AuPoint rotAxis;

				block >> sel;
				block >> lightColor;
				block >> rotAxis;

				m_I->DesignerMakeRotatingBeaconLens(sel, lightColor, rotAxis);
			}
			else if(doWhat == 36)
			{
				// Make strobe.
				AuScSelection sel;
				QColor lightColor;

				block >> sel;
				block >> lightColor;

				m_I->DesignerMakeStrobeLens(sel, lightColor);
			}
			else if(doWhat == 37)
			{
				// Make turbo lift.
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeTurboLift(sel);
			}
			else if(doWhat == 38)
			{
				// Make turret.
				AuScSelection sel;
				AuPoint basePivot, baseAxis;

				block >> sel;
				block >> basePivot;
				block >> baseAxis;

				m_I->DesignerMakeTurret(sel, basePivot, baseAxis);
			}
			else if(doWhat == 39)
			{
				// Make window.
				AuScSelection sel;
				float opacity;

				block >> sel;
				block >> opacity;

				m_I->DesignerMakeWindow(sel, opacity);
			}
			else if(doWhat == 40)
			{
				// Make street light lens.
				AuScSelection sel;
				QColor lightColor;

				block >> sel;
				block >> lightColor;

				m_I->DesignerMakeLightLens(sel, lightColor);
			}
			else if(doWhat == 41)
			{
				// Set light bulb properties.
				AuScSelection sel;
				QColor color;
				float kL;
				float kQ;
				float cosOuterCone;
				float cosInnerCone;

				block >> sel;
				block >> color;
				block >> kL;
				block >> kQ;
				block >> cosOuterCone;
				block >> cosInnerCone;

				m_I->DesignerPartsSetLightBulbProperties(sel, color, kL, kQ, cosOuterCone, cosInnerCone);
			}
			else if(doWhat == 42)
			{
				AuScSelection sel;
				quint8 gunnerPos;
				AuPoint gunnerLoc;
				AuPoint gunnerFacing;
				AuPoint corner1;
				AuPoint corner2;
				AuMatrix4x4 gridPlane;
				quint16 module;

				block >> sel;
				block >> gunnerPos;
				block >> gunnerLoc;
				block >> corner1;
				block >> corner2;
				block >> gridPlane;
				block >> module;
				block >> gunnerFacing;

				m_I->DesignerTurretStation(sel, gunnerPos, gunnerLoc, gunnerFacing, corner1, corner2, gridPlane,
						(COMMODITYID)module);
			}
			else if(doWhat == 43)
			{
				AuScSelection sel;
				AuPoint gunPivot;
				AuPoint gunAxis;
				AuPoint gunEmitLoc;
				AuPoint gunEmitVec;
				AuPoint gunSightLoc;

				block >> sel;
				block >> gunPivot;
				block >> gunAxis;
				block >> gunEmitLoc;
				block >> gunEmitVec;
				block >> gunSightLoc;

				m_I->DesignerTurretGun(sel, gunPivot, gunAxis, gunEmitLoc, gunEmitVec, gunSightLoc);
			}
			else if(doWhat == 44)
			{
				quint16 partId;
				float opacity;

				block >> partId;
				block >> opacity;

				m_I->DesignerPartSetOpacity(partId, opacity);
			}
			else if(doWhat == 45)
			{
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeHull(sel);
			}
			else if(doWhat == 46)
			{
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeRoom(sel);
			}
			else if(doWhat == 47)
			{
				quint16 partId;
				QString stop;

				block >> partId;
				block >> stop;

				m_I->DesignerPartAddStop(partId, stop);
			}
			else if(doWhat == 48)
			{
				quint16 partId;
				QString stop;

				block >> partId;
				block >> stop;

				m_I->DesignerPartDeleteStop(partId, stop);
			}
			else if(doWhat == 49)
			{
				quint16 partId;
				QString stop;

				block >> partId;
				block >> stop;

				m_I->DesignerPartSetStop(partId, stop);
			}
			else if(doWhat == 50)
			{
				AuScSelection sel;
				QColor color;

				block >> sel;
				block >> color;

				m_I->DesignerPaintFaces(sel, color);
			}
			else if(doWhat == 51)
			{
				AuScSelection sel;
				quint8 theme;
				quint8 opacity;

				block >> sel;
				block >> theme;
				block >> opacity;

				m_I->DesignerPaintFaces(sel, theme, opacity);
			}
			else if(doWhat == 52)
			{
				AuScSelection sel;
				QColor glow;

				block >> sel;
				block >> glow;

				m_I->DesignerGlowVertices(sel, glow);
			}
			else if(doWhat == 53)
			{
				AuScSelection sel;
				float shine, reflectivity;

				block >> sel;
				block >> shine;
				block >> reflectivity;

				m_I->DesignerPolishFaces(sel, shine, reflectivity);
			}
			else if(doWhat == 54)
			{
				AuScSelection sel;
				float opacity;

				block >> sel;
				block >> opacity;

				m_I->DesignerPartsSetFaceOpacity(sel, opacity);
			}
			else if(doWhat == 55)
			{
				AuScSelection sel;
				float selfIllum;

				block >> sel;
				block >> selfIllum;

				m_I->DesignerPartsSetFaceSelfIllumination(sel, selfIllum);
			}
			else if(doWhat == 56)
			{
				AuScSelection sel;
				AuPoint basePivot;
				AuPoint baseAxis;

				block >> sel;
				block >> basePivot;
				block >> baseAxis;

				m_I->DesignerTurretBase(sel, basePivot, baseAxis);
			}
			else if(doWhat == 57)
			{
				AuDesignTex texturePalette[SCTEXI_Count];
				for(int tpi=0; tpi<SCTEXI_Count; tpi++)
					block >> texturePalette[tpi];

				m_I->DesignerSetTexturePalette(texturePalette);
			}
			else if(doWhat == 58)
			{
				AuScSelection sel;
				quint8 texIndex;

				block >> sel;
				block >> texIndex;

				m_I->DesignerPartsTextureFaces(sel, (SC_TEXTURE_INDEX)texIndex);
			}
			else if(doWhat == 59)
			{
				AuScSelection sel;
				quint8 texIndex;
				quint16 texRepsAtEquator;
				double texSize;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> texIndex;
				block >> texRepsAtEquator;
				block >> texSize;
				block >> gridPlane;

				m_I->DesignerPartsTextureFacesCylindrical(sel, (SC_TEXTURE_INDEX)texIndex, texRepsAtEquator, texSize, gridPlane);
			}
			else if(doWhat == 60)
			{
				AuScSelection sel;
				quint8 texIndex;
				double texSize;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> texIndex;
				block >> texSize;
				block >> gridPlane;

				m_I->DesignerPartsTextureFacesProject(sel, (SC_TEXTURE_INDEX)texIndex, texSize, gridPlane);
			}
			else if(doWhat == 61)
			{
				AuScSelection sel;
				quint8 texIndex;
				quint16 texRepsAtEquator;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> texIndex;
				block >> texRepsAtEquator;
				block >> gridPlane;

				m_I->DesignerPartsTextureFacesSpherical(sel, (SC_TEXTURE_INDEX)texIndex, texRepsAtEquator, gridPlane);
			}
			else if(doWhat == 62)
			{
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeDecal(sel);
			}
			else if(doWhat == 63)
			{
				AuScSelection sel;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> gridPlane;

				m_I->DesignerCopyPartsSymmetrical(sel, gridPlane);
			}
			else if(doWhat == 64)
			{
				AuScSelection sel;
				QPointF vec;
				quint8 undoSeq;

				block >> sel;
				block >> vec;
				block >> undoSeq;

				m_I->DesignerPartsTextureMove(sel, vec, undoSeq);
			}
			else if(doWhat == 65)
			{
				AuScSelection sel;
				QPointF pivot;
				double angle;
				quint8 undoSeq;

				block >> sel;
				block >> pivot;
				block >> angle;
				block >> undoSeq;

				m_I->DesignerPartsTextureRotate(sel, pivot, angle, undoSeq);
			}
			else if(doWhat == 66)
			{
				AuScSelection sel;
				QPointF base;
				double scaleX, scaleY;
				quint8 undoSeq;

				block >> sel;
				block >> base;
				block >> scaleX;
				block >> scaleY;
				block >> undoSeq;

				m_I->DesignerPartsTextureStretch(sel, base, scaleX, scaleY, undoSeq);
			}
			else if(doWhat == 67)
			{
				AuScSelection sel;
				quint8 texIndex;
				double texSize;
				quint8 unwrap;
				float angleTol;
				AuMatrix4x4 gridPlane;
				bool gridAlign;
				quint8 alignAxis;

				block >> sel;
				block >> texIndex;
				block >> texSize;
				block >> unwrap;
				block >> angleTol;
				block >> gridPlane;
				block >> gridAlign;
				block >> alignAxis;

				m_I->DesignerPartsTextureUnwrap(sel, (SC_TEXTURE_INDEX)texIndex, texSize, (AuScFaceList::UNWRAP_METHOD)unwrap,
						angleTol, gridAlign, alignAxis, gridPlane);
			}
			else if(doWhat == 68)
			{
				AuScSelection partIds;

				block >> partIds;

				m_I->DesignerAssociateParts(partIds);
			}
			else if(doWhat == 69)
			{
				AuScSelection partIds;
				quint8 gravityType;
				AuPoint gravityUp;
				quint8 holdAccess;
				quint8 personnelAccess;

				block >> partIds;
				block >> gravityType;
				block >> gravityUp;
				block >> holdAccess;
				block >> personnelAccess;

				m_I->DesignerPartsSetVoidProperties(partIds, (GRAVITY)gravityType, gravityUp, (HOLD_ACCESS)holdAccess,
						personnelAccess);
			}
			else if(doWhat == 70)
			{
				AuScSelection sel;
				AuPoint v1,v2,v3,v4;

				block >> sel;
				block >> v1;
				block >> v2;
				block >> v3;
				block >> v4;

				m_I->DesignerPartFillQuad(sel, v1, v2, v3, v4);
			}
			else if(doWhat == 71)
			{
				AuScSelection sel;
				AuPoint v1,v2,v3;

				block >> sel;
				block >> v1;
				block >> v2;
				block >> v3;

				m_I->DesignerPartFillTriangle(sel, v1, v2, v3);
			}
			else if(doWhat == 72)
			{
				// Replace faces.
				AuScSelection sel;
				AuPointArray cutBnd;

				block >> sel;
				block >> cutBnd;

				m_I->DesignerPunchFaces(sel, cutBnd);
			}
			else if(doWhat == 73)
			{
				// Update hull doors
				AuScSelection sel;

				block >> sel;

				m_I->DesignerCheckDoors(sel);
			}
			else if(doWhat == 74)
			{
				AuScSelection partIds;

				block >> partIds;

				m_I->DesignerDissociateParts(partIds);
			}
			else if(doWhat == 75)
			{
				AuScSelection sel;

				block >> sel;

				m_I->DesignerGoToSelection(sel);
			}
			else if(doWhat == 76)
			{
				AuScSelection sel;
				AuPoint switch1, switch2;

				block >> sel;
				block >> switch1;
				block >> switch2;

				m_I->DesignerPartsMoveDoorSwitches(sel, switch1, switch2);
			}
			else if(doWhat == 77)
			{
				AuScSelection sel;
				AuPointArray padLocs;

				block >> sel;
				block >> padLocs;

				m_I->DesignerPartsTransporterPadLocs(sel, padLocs);
			}
			else if(doWhat == 78)
			{
				AuScSelection sel;
				AuPoint emitLoc;

				block >> sel;
				block >> emitLoc;

				m_I->DesignerPartsWeaponEmitLoc(sel, emitLoc);
			}
			else if(doWhat == 79)
			{
				AuScSelection sel;
				quint8 effect;

				block >> sel;
				block >> effect;

				m_I->DesignerPartsEffectFaces(sel, (SURFACE_EFFECT)effect);
			}
			else if(doWhat == 80)
			{
				AuScSelection sel;
				quint8 effect;
				quint16 texRepsAtEquator;
				double texSize;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> effect;
				block >> texRepsAtEquator;
				block >> texSize;
				block >> gridPlane;

				m_I->DesignerPartsEffectFacesCylindrical(sel, (SURFACE_EFFECT)effect, texRepsAtEquator, texSize, gridPlane);
			}
			else if(doWhat == 81)
			{
				AuScSelection sel;
				quint8 effect;
				double texSize;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> effect;
				block >> texSize;
				block >> gridPlane;

				m_I->DesignerPartsEffectFacesProject(sel, (SURFACE_EFFECT)effect, texSize, gridPlane);
			}
			else if(doWhat == 82)
			{
				AuScSelection sel;
				quint8 effect;
				quint16 texRepsAtEquator;
				AuMatrix4x4 gridPlane;

				block >> sel;
				block >> effect;
				block >> texRepsAtEquator;
				block >> gridPlane;

				m_I->DesignerPartsEffectFacesSpherical(sel, (SURFACE_EFFECT)effect, texRepsAtEquator, gridPlane);
			}
			else if(doWhat == 83)
			{
				AuScSelection sel;
				QPointF vec;
				quint8 undoSeq;

				block >> sel;
				block >> vec;
				block >> undoSeq;

				m_I->DesignerPartsEffectMove(sel, vec, undoSeq);
			}
			else if(doWhat == 84)
			{
				AuScSelection sel;
				QPointF pivot;
				double angle;
				quint8 undoSeq;

				block >> sel;
				block >> pivot;
				block >> angle;
				block >> undoSeq;

				m_I->DesignerPartsEffectRotate(sel, pivot, angle, undoSeq);
			}
			else if(doWhat == 85)
			{
				AuScSelection sel;
				QPointF base;
				double scaleX, scaleY;
				quint8 undoSeq;

				block >> sel;
				block >> base;
				block >> scaleX;
				block >> scaleY;
				block >> undoSeq;

				m_I->DesignerPartsEffectStretch(sel, base, scaleX, scaleY, undoSeq);
			}
			else if(doWhat == 86)
			{
				AuScSelection sel;
				quint8 effect;
				double texSize;
				quint8 unwrap;
				float angleTol;
				AuMatrix4x4 gridPlane;
				bool gridAlign;
				quint8 alignAxis;

				block >> sel;
				block >> effect;
				block >> texSize;
				block >> unwrap;
				block >> angleTol;
				block >> gridPlane;
				block >> gridAlign;
				block >> alignAxis;

				m_I->DesignerPartsEffectUnwrap(sel, (SURFACE_EFFECT)effect, texSize, (AuScFaceList::UNWRAP_METHOD)unwrap, angleTol,
						gridAlign, alignAxis, gridPlane);
			}
			else if(doWhat == 87)
			{
				AuScSelection partIds;
				quint8 isSunlit;
				quint8 initLight;

				block >> partIds;
				block >> isSunlit;
				block >> initLight;

				m_I->DesignerPartsSetRoomProperties(partIds, isSunlit, (INTERIOR_LIGHT)initLight);
			}
			else if(doWhat == 88)
			{
				AuScSelection partIds;
				quint8 isauto;
				quint8 initLocked;
				quint8 remoteSpacecraft;
				quint8 remoteVehicle;

				block >> partIds;
				block >> isauto;
				block >> initLocked;
				block >> remoteSpacecraft;
				block >> remoteVehicle;

				m_I->DesignerPartsSetDoorProperties(partIds, (Qt::CheckState)isauto, (Qt::CheckState)initLocked,
						(Qt::CheckState)remoteSpacecraft, (Qt::CheckState)remoteVehicle);
			}
			else if(doWhat == 89)
			{
				AuScSelection sel;

				block >> sel;

				m_I->DesignerMakeMesh(sel);
			}
			else if(doWhat == 90)
			{
				// Align turbo lift stops.
				AuScSelection sel;
				AuMatrix4x4 t;

				block >> sel;
				block >> t;

				m_I->DesignerMoveLiftStops(sel, t);
			}
			else if(doWhat == 91)
			{
				quint16 partId;
				QString oldStopName;
				QString nuStopName;

				block >> partId;
				block >> oldStopName;
				block >> nuStopName;

				m_I->DesignerPartRenameStop(partId, oldStopName, nuStopName);
			}
			else if(doWhat == 92)
			{
				AuScSelection sel;
				float intensity;

				block >> sel;
				block >> intensity;

				m_I->DesignerEffectIntensityVertices(sel, intensity);
			}
			else if(doWhat == 93)
			{
				AuScSelection partIds;
				quint8 grade;
				quint8 grass;
				quint8 pavement;
				quint8 rocks;
				quint8 shrubs;
				quint8 terBlend;
				quint8 trees;

				block >> partIds;
				block >> grade;
				block >> grass;
				block >> pavement;
				block >> rocks;
				block >> shrubs;
				block >> terBlend;
				block >> trees;

				m_I->DesignerPartsSetSiteProperties(partIds, grade, grass, pavement, rocks, shrubs, terBlend, trees);
			}
			else if(doWhat == 94)
			{
				// Load people in preview.
				AuSpecies species;
				block >> species;
				m_I->DesignerPreviewPeople(species);
			}
			else if(doWhat == 95)
			{
				// Set group
				AuScIdSet partIds;
				QString groupName;

				block >> partIds;
				block >> groupName;

				m_I->DesignerPartsSetGroup(partIds, groupName);
			}
		}
		break;

	case DtChatChannelPacket::PD_ScLibrary:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Store design
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuScDesign* sc = dynamic_cast<AuScDesign*>(s);
					if(sc)
						m_I->DesignerArchitectSaveSc(*sc);
					delete s;
				}
			}
			else if(doWhat == 1)
			{
				// List designs available.
				DesignerArchitectListSc();
			}
			else if(doWhat == 2)
			{
				// Delete design.
				quint32 designId;

				block >> designId;

				m_I->DesignerArchitectDeleteSc(AcctIsArchitect(m_AccountType), designId);
			}
			else if(doWhat == 3)
			{
				// Finalize assembly.
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuScDesign* sc = dynamic_cast<AuScDesign*>(s);
					if(sc)
					{
						m_I->DesignerArchitectFinalizeScAssembly(*sc);
						if((*sc).Id())
						{
							m_rwScDesigns.lockForWrite();
							m_ScDesignsInUse.insert(pair<quint32, bool>((*sc).Id(), true));
							m_rwScDesigns.unlock();
						}
					}
					delete s;
				}
			}
			else if(doWhat == 4)
			{
				// Finalize design.
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuScDesign* sc = dynamic_cast<AuScDesign*>(s);
					if(sc)
					{
						m_I->DesignerArchitectFinalizeScDesign(*sc);
						if((*sc).Id())
						{
							m_rwScDesigns.lockForWrite();
							m_ScDesignsInUse.insert(pair<quint32, bool>((*sc).Id(), true));
							m_rwScDesigns.unlock();
						}
					}
					delete s;
				}
			}
			else if(doWhat == 5)
			{
				// Load design from gear position.
				quint32 designId;
				AuGearPos pos;

				block >> designId;
				block >> pos;

				DesignerGearOpen(designId, pos);
			}
			else if(doWhat == 6)
			{
				// Preview current design.
				quint8 previewMode;
				AuDesign* design;
				quint32 genSeed;

				block >> previewMode;
				if(previewMode)
				{
					QByteArray bcomp;

					block >> bcomp;
					block >> genSeed;
					
					if(bcomp.isEmpty())
						break;
					QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
					design = AuDesign::Unpack(ba);
				}
				else
				{
					design = 0;
					genSeed = 0;
				}

				m_I->DesignerSetPreview(previewMode, design, genSeed);
				if(design)
					delete design;
			}
			else if(doWhat == 7 && AcctIsAgent(m_AccountType))
			{
				// Finalize design and assign a specific id.
				quint32 designId;
				QByteArray bcomp;

				block >> designId;
				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuScDesign* sc = dynamic_cast<AuScDesign*>(s);
					if(sc)
					{
						m_I->DesignerArchitectFinalizeScDesign(*sc, designId);
						if((*sc).Id())
						{
							m_rwScDesigns.lockForWrite();
							m_ScDesignsInUse.insert(pair<quint32, bool>((*sc).Id(), true));
							m_rwScDesigns.unlock();
						}
					}
					delete s;
				}
			}
		}
		break;

	case DtChatChannelPacket::PD_ScExchange:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Publish a design.
				AuScDesignExchangeEntry exchangeItem;

				block >> exchangeItem;

				m_I->ScDesignExchangePublish(exchangeItem);
			}
			else if(doWhat == 1)
			{
				// List published designs available.
				ScDesignExchangeList();
			}
			else if(doWhat == 2)
			{
				// Unpublish a design.
				quint32 designId;

				block >> designId;

				m_I->ScDesignExchangeUnpublish(AcctIsArchitect(m_AccountType), designId, false);
			}
			else if(doWhat == 3)
			{
				// Purchase a design.
				quint32 designId;
				AuGearPos mediaPos;

				block >> designId;
				block >> mediaPos;

				m_I->ScDesignExchangePurchase(designId, mediaPos);
			}
			else if(doWhat == 4)
			{
				// Store design from exchange to media at gear position.
				AuScDesignExchangeEntry exchangeItem;
				AuGearPos pos;

				block >> exchangeItem;
				block >> pos;

				if(m_DesignStudioMode != DaDesignBase::InvalidMode)
					m_I->DesignerGearSaveSc(exchangeItem, pos);
			}
			else if(doWhat == 5)
			{
				// Rate a design.
				quint32 designId;
				quint8 rating;
				QString comment;

				block >> designId;
				block >> rating;
				block >> comment;

				m_I->ScDesignExchangeRate(AuExchangeRatingRec(m_I->AvatarId(), m_UnitName, designId, rating, comment,
							AuTime::Now()));
			}
			else if(doWhat == 6)
			{
				// Report design ratings.
				quint32 designId;
				block >> designId;
				ScDesignExchangeReportRatings(designId);
			}

			// Do this as a more intelligent dialog to get only the exchange entries that have changed.
			else if(doWhat == 10)
			{
				// Client wants the table of contents: exchange ids and last update times.
				ScDesignExchangeToC();
			}
			else if(doWhat == 11)
			{
				// Client wants specific exchange records.
				QList<quint32> designIds;
				block >> designIds;
				ScDesignExchangeList(designIds);
			}
		}
		break;

	case DtChatChannelPacket::PD_ItLibrary:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Store design
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuItDesign* it = dynamic_cast<AuItDesign*>(s);
					if(it)
						m_I->DesignerArchitectSaveIt(*it);
					delete s;
				}
			}
			else if(doWhat == 1)
			{
				// List designs available.
				DesignerArchitectListIt();
			}
			else if(doWhat == 2)
			{
				// Delete design.
				quint32 designId;

				block >> designId;

				m_I->DesignerArchitectDeleteIt(AcctIsArchitect(m_AccountType), designId);
			}
			else if(doWhat == 3)
			{
				// Finalize assembly.
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuItDesign* it = dynamic_cast<AuItDesign*>(s);
					if(it)
					{
						m_I->DesignerArchitectFinalizeItAssembly(*it);
						if((*it).Id())
						{
							m_rwScDesigns.lockForWrite();
							m_ScDesignsInUse.insert(pair<quint32, bool>((*it).Id(), true));
							m_rwScDesigns.unlock();
						}
					}
					delete s;
				}
			}
			else if(doWhat == 4)
			{
				// Finalize design.
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuItDesign* it = dynamic_cast<AuItDesign*>(s);
					if(it)
					{
						m_I->DesignerArchitectFinalizeItDesign(*it);
						if((*it).Id())
						{
							m_rwScDesigns.lockForWrite();
							m_ScDesignsInUse.insert(pair<quint32, bool>((*it).Id(), true));
							m_rwScDesigns.unlock();
						}
					}
					delete s;
				}
			}
			else if(doWhat == 5 && AcctIsAgent(m_AccountType))
			{
				// Finalize design and assign a specific id.
				quint32 designId;
				QByteArray bcomp;

				block >> designId;
				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuItDesign* it = dynamic_cast<AuItDesign*>(s);
					if(it)
					{
						m_I->DesignerArchitectFinalizeItDesign(*it, designId);
						if((*it).Id())
						{
							m_rwScDesigns.lockForWrite();
							m_ScDesignsInUse.insert(pair<quint32, bool>((*it).Id(), true));
							m_rwScDesigns.unlock();
						}
					}
					delete s;
				}
			}
		}
		break;

	case DtChatChannelPacket::PD_BdLibrary:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Store design
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuBdDesign* sc = dynamic_cast<AuBdDesign*>(s);
					if(sc)
						m_I->DesignerArchitectSaveBd(*sc);
					delete s;
				}
			}
			else if(doWhat == 1)
			{
				// List designs available.
				DesignerArchitectListBd();
			}
			else if(doWhat == 2)
			{
				// Delete design.
				quint32 designId;

				block >> designId;

				m_I->DesignerArchitectDeleteBd(AcctIsArchitect(m_AccountType), designId);
			}
			else if(doWhat == 3)
			{
				// Finalize assembly.
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuBdDesign* sc = dynamic_cast<AuBdDesign*>(s);
					if(sc)
					{
						QSet<quint32> xrefs;
						m_I->DesignerArchitectFinalizeBdAssembly(*sc, xrefs);
						if((*sc).Id())
						{
							m_rwBdDesigns.lockForWrite();
							m_BdDesigns.insert(pair<quint32, bool>((*sc).Id(), true));
							for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
								m_BdDesignsInUse.insert(pair<quint32, bool>(*xi, true));
							m_rwBdDesigns.unlock();
						}
					}
					delete s;
				}
			}
			else if(doWhat == 4)
			{
				// Finalize design.
				QByteArray bcomp;

				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuBdDesign* sc = dynamic_cast<AuBdDesign*>(s);
					if(sc)
					{
						QSet<quint32> xrefs;
						m_I->DesignerArchitectFinalizeBdDesign(*sc, xrefs);
						if((*sc).Id())
						{
							m_rwBdDesigns.lockForWrite();
							m_BdDesigns.insert(pair<quint32, bool>((*sc).Id(), true));
							for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
								m_BdDesignsInUse.insert(pair<quint32, bool>(*xi, true));
							m_rwBdDesigns.unlock();
						}
					}
					delete s;
				}
			}
			else if(doWhat == 5 && AcctIsAgent(m_AccountType))
			{
				// Finalize design and assign a specific id.
				quint32 designId;
				QByteArray bcomp;

				block >> designId;
				block >> bcomp;
				
				if(bcomp.isEmpty())
					break;
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,m_UnitName);
				AuDesign* s = AuDesign::Unpack(ba);
				if(s)
				{
					AuBdDesign* sc = dynamic_cast<AuBdDesign*>(s);
					if(sc)
					{
						QSet<quint32> xrefs;
						m_I->DesignerArchitectFinalizeBdDesign(*sc, xrefs, designId);
						if((*sc).Id())
						{
							m_rwBdDesigns.lockForWrite();
							m_BdDesigns.insert(pair<quint32, bool>((*sc).Id(), true));
							for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
								m_BdDesignsInUse.insert(pair<quint32, bool>(*xi, true));
							m_rwBdDesigns.unlock();
						}
					}
					delete s;
				}
			}
		}
		break;

	case DtChatChannelPacket::PD_BdExchange:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Publish a design.
				AuBdDesignExchangeEntry exchangeItem;

				block >> exchangeItem;

				m_I->BdDesignExchangePublish(exchangeItem);
			}
			else if(doWhat == 1)
			{
				// List published designs available.
				BdDesignExchangeList();
			}
			else if(doWhat == 2)
			{
				// Unpublish a design.
				quint32 designId;

				block >> designId;

				m_I->BdDesignExchangeUnpublish(AcctIsArchitect(m_AccountType), designId, false);
			}
			else if(doWhat == 3)
			{
				// Purchase a design.
				quint32 designId;
				AuGearPos mediaPos;

				block >> designId;
				block >> mediaPos;

				m_I->BdDesignExchangePurchase(designId, mediaPos);
			}
			else if(doWhat == 4)
			{
				// Store design from exchange to media at gear position.
				AuBdDesignExchangeEntry exchangeItem;
				AuGearPos pos;

				block >> exchangeItem;
				block >> pos;

				if(m_DesignStudioMode != DaDesignBase::InvalidMode)
					m_I->DesignerGearSaveBd(exchangeItem, pos);
			}
			else if(doWhat == 5)
			{
				// Rate a design.
				quint32 designId;
				quint8 rating;
				QString comment;

				block >> designId;
				block >> rating;
				block >> comment;

				m_I->BdDesignExchangeRate(AuExchangeRatingRec(m_I->AvatarId(), m_UnitName, designId, rating, comment,
							AuTime::Now()));
			}
			else if(doWhat == 6)
			{
				// Report design ratings.
				quint32 designId;
				block >> designId;
				BdDesignExchangeReportRatings(designId);
			}

			// Do this as a more intelligent dialog to get only the exchange entries that have changed.
			else if(doWhat == 10)
			{
				// Client wants the table of contents: exchange ids and last update times.
				BdDesignExchangeToC();
			}
			else if(doWhat == 11)
			{
				// Client wants specific exchange records.
				QList<quint32> designIds;
				block >> designIds;
				BdDesignExchangeList(designIds);
			}
		}
		break;

	case DtChatChannelPacket::PD_BdDesign:						// Client request for an AuBdDesign record of data.
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;

			block >> designId;

			if(designId && !ClientHasBdDesign(designId))
				SendBdDesign(designId);
		}
		break;

	case DtChatChannelPacket::PD_ItDesign:						// Client request for an AuItDesign record of data.
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;

			block >> designId;

			if(designId && !ClientHasItDesign(designId))
				SendItDesign(designId);
		}
		break;

	case DtChatChannelPacket::PD_ScDesign:						// Client request for an AuScDesign record of data.
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;

			block >> designId;

			if(designId && !ClientHasScDesign(designId))
				SendScDesign(designId);
		}
		break;

	case DtChatChannelPacket::PD_BdDesignHere:
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;

			block >> designId;

			//qDebug() << "DtChatChannel::ReadPacket PD_BdDesignHere client has design" << designId;
			m_rwBdDesigns.lockForWrite();
			m_BdDesigns.insert(pair<quint32, bool>(designId, true));
			m_rwBdDesigns.unlock();
		}
		break;

	case DtChatChannelPacket::PD_ItDesignHere:
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;

			block >> designId;

			//qDebug() << "DtChatChannel::ReadPacket PD_ItDesignHere client has design" << designId;
			m_rwItDesigns.lockForWrite();
			m_ItDesigns.insert(pair<quint32, bool>(designId, true));
			m_rwItDesigns.unlock();
		}
		break;

	case DtChatChannelPacket::PD_ScDesignHere:
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;

			block >> designId;

			//qDebug() << "DtChatChannel::ReadPacket PD_ScDesignHere client has design" << designId;
			m_rwScDesigns.lockForWrite();
			m_ScDesigns.insert(pair<quint32, bool>(designId, true));
			m_rwScDesigns.unlock();
		}
		break;

	case DtChatChannelPacket::PD_BdErase:
		//qDebug("PD_BdErase:");
		// Server never gets this.
		g_Au.LogConnect(auSceneServerApp->LoginName(),
				QString("CHAT: ERROR! Received a PD_BdErase packet. Hacker? %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
		Disconnect();
		break;

	case DtChatChannelPacket::PD_ItErase:
		//qDebug("PD_ItErase:");
		// Server never gets this.
		g_Au.LogConnect(auSceneServerApp->LoginName(),
				QString("CHAT: ERROR! Received a PD_ItErase packet. Hacker? %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
		Disconnect();
		break;

	case DtChatChannelPacket::PD_ScErase:
		//qDebug("PD_ScErase:");
		// Server never gets this.
		g_Au.LogConnect(auSceneServerApp->LoginName(),
				QString("CHAT: ERROR! Received a PD_ScErase packet. Hacker? %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
		Disconnect();
		break;

	case DtChatChannelPacket::PD_Exit:
		//qDebug("PD_Exit:");
		if(g_Au.IsSceneWorker())
			DesignExit();
		break;

	case DtChatChannelPacket::PD_New:
		//qDebug("PD_New:");
		if(g_Au.IsSceneWorker())
		{
			QByteArray ba;

			block >> ba;

			AuDesign* design = AuDesign::Unpack(ba);
			if(design)
			{
				m_I->DesignerNew(*design);
				delete design;
			}
		}
		break;

	case DtChatChannelPacket::PD_Properties:
		//qDebug("PD_Properties:");
		if(g_Au.IsSceneWorker())
		{
			QByteArray ba;

			block >> ba;

			AuDesign* design = AuDesign::Unpack(ba);
			if(design)
			{
				m_I->DesignerSetProperties(*design);
				delete design;
			}
		}
		break;

	case DtChatChannelPacket::PD_Open:
		//qDebug("PD_Open:");
		if(g_Au.IsSceneWorker())
		{
			quint32 designId;
			quint8 designMode;

			block >> designId;
			block >> designMode;

			DesignOpen(designId, designMode);
		}
		break;
	case DtChatChannelPacket::PD_Gravity:
		//qDebug("PD_Gravity:");
		if(g_Au.IsSceneWorker())
		{
			bool gravOn;

			block >> gravOn;

			m_I->DesignerSetGravity(gravOn);
		}
		break;

	case DtChatChannelPacket::PE_ChangeFlag:
		{
			QImage flag;

			block >> flag;

			m_I->GovSetEmpireFlag(flag);
		}
		break;

	case DtChatChannelPacket::PE_Rename:
		{
			quint8 renameWhat;
			QString nuName;
			AuId capitolOpId;

			block >> renameWhat;
			block >> nuName;
			if(renameWhat == 5)
				block >> capitolOpId;	// Rename building can be done from capitol. Then capitolOpId is the building id to rename.

			if(m_I->Rename(renameWhat, nuName, capitolOpId) && renameWhat == 4)
				m_UnitName = nuName;
		}
		break;

	case DtChatChannelPacket::PE_GABalance:
		{
			AuId opBd;

			block >> opBd;

			m_I->GovtAcctRequestBalance(opBd);
		}
		break;

	case DtChatChannelPacket::PE_GADeposit:
		{
			double amount;
			AuId opBd;

			block >> amount;
			block >> opBd;

			if(amount >= 100)
				m_I->GovtAcctDepositFunds(amount, opBd);
		}
		break;

	case DtChatChannelPacket::PE_GAWithdraw:
		{
			double amount;
			AuId opBd;

			block >> amount;
			block >> opBd;

			if(amount >= 100)
				m_I->GovtAcctWithdrawFunds(amount, opBd);
		}
		break;

	case DtChatChannelPacket::PE_LeaveEmpire:
		//qDebug("PE_LeaveEmpire:");
		{
			// This is also used when kicking a citizen from an empire.
			quint32 idOldEmpire;
			AuId personId;
			QString personName;

			block >> personId;
			block >> personName;
			block >> idOldEmpire;

			if(m_I->AvatarId() == personId)
				m_I->GovLeaveEmpire(personId, personName, idOldEmpire);
			else
				m_I->GovKick(personId, personName, idOldEmpire);
		}
		break;

	case DtChatChannelPacket::PE_Report:
		{
			quint8 reportWhat;

			block >> reportWhat;

			switch(reportWhat)
			{
			case 0:
				{
					// Request for last posted city report.
					AuId cityId;

					block >> cityId;

					ReportCityLast(cityId);
				}
				break;
			case 1:
				{
					// Request for entire city report history.
					AuId cityId;

					block >> cityId;

					ReportCityHistory(cityId);
				}
				break;
			case 2:
				{
					// Request for last posted city report in batch.
					AuIdList cityIds;

					block >> cityIds;

					for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
						ReportCityLast(*ci);
				}
				break;
			case 3:
				{
					// Request for entire city report history in batch.
					AuIdList cityIds;

					block >> cityIds;

					for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
						ReportCityHistory(*ci);
				}
				break;
			case 4:
				{
					// Request for last city report by mail.
					AuIdList cityIds;

					block >> cityIds;

					for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
						m_I->MailCityLast(*ci);

					QString resultMsg;
					if(cityIds.count() == 1)
						resultMsg = tr("One city is sending me its last status report by mail.");
					else
						resultMsg = tr("%1 cities are sending me their last status report by mail.").arg(cityIds.count());
					AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
					g_Au.SendAtomChat(msg);
				}
				break;
			case 5:
				SendPlayerOnlineStatus();
				break;
			default:
				qWarning() << "PE_Report Invalid reportWhat" << reportWhat << "unit" << m_I->AvatarId() << m_UnitName;
				break;
			}
		}
		break;

	case DtChatChannelPacket::PI_Selfie:
		{
			QImage img;

			block >> img;

			m_I->Selfie(img);
		}
		break;

	case DtChatChannelPacket::PI_BankBorrow:
		{
			double amount;
			AuId opBd;

			block >> amount;
			block >> opBd;

			m_I->BankLoan(amount, opBd);
		}
		break;

	case DtChatChannelPacket::PI_BankPay:
		{
			double amount;
			AuId opBd;

			block >> amount;
			block >> opBd;

			m_I->BankPay(amount, opBd);
		}
		break;

	case DtChatChannelPacket::PI_AvatarSetting:
		{
			quint8 whichSetting;
			block >> whichSetting;
			switch(whichSetting)
			{
			case 0:	// name
				{
					QString babyName;
					block >> babyName;
					m_I->NameTheBaby(babyName);
				}
				break;
			case 1:	// auto eat
				{
					quint8 autoEat;
					block >> autoEat;
					m_I->SetAutoEat(autoEat);
				}
				break;
			case 2:	// hair style
				{
					quint8 haircut, shave;
					block >> haircut;
					block >> shave;
					m_I->SetHairStyle((AuHairStyle::STYLE)haircut, (AuHairStyle::SHAVE)shave);
				}
				break;
			case 3:	// dna4
				{
					AuDNA dna;
					block >> dna;
					m_I->SetDNA(dna);
				}
				break;
			case 4:	// Add domain system
				{
					AuId systemId;
					QString systemName;
					block >> systemId;
					block >> systemName;
					m_I->DomainAdd(systemId, systemName);
				}
				break;
			case 5:	// Remove domain system
				{
					AuId systemId;
					QString systemName;
					block >> systemId;
					block >> systemName;
					m_I->DomainRemove(systemId, systemName);
				}
				break;
			case 6:	// Add current system to domain
				m_I->DomainAddCurrent();
				break;
			}
		}
		break;

	case DtChatChannelPacket::PF_Station0 + DpStation::STATION_Nav:
		//qDebug("PI_ConsoleNav:");
		if(g_Au.IsSceneWorker())
		{
			// Client is restoring user settings on console.
			DaPerson* person = m_I->LockAvatar();
			if(person && person->IsNavStationPose())
				person->RxConsoleSettings(block);
			m_I->UnlockAvatar();
		}
		break;
	case DtChatChannelPacket::PF_Station0 + DpStation::STATION_Sensor:
		//qDebug("PI_ConsoleSensor:");
		if(g_Au.IsSceneWorker())
		{
			// Client is restoring user settings on console.
			DaPerson* person = m_I->LockAvatar();
			if(person)
				person->RxConsoleSettings(block);
			m_I->UnlockAvatar();
		}
		break;
	case DtChatChannelPacket::PF_Station0 + DpStation::STATION_Transporter:
		//qDebug("PI_ConsoleTransporter:");
		if(g_Au.IsSceneWorker())
		{
			// Client is restoring user settings on console.
			DaPerson* person = m_I->LockAvatar();
			if(person)
				person->RxConsoleSettings(block);
			m_I->UnlockAvatar();
		}
		break;
	case DtChatChannelPacket::PF_Station0 + DpStation::STATION_FireControl:
		//qDebug("PI_ConsoleFireControl:");
		if(g_Au.IsSceneWorker())
		{
			// Client is restoring user settings on console.
			DaPerson* person = m_I->LockAvatar();
			if(person)
				person->RxConsoleSettings(block);
			m_I->UnlockAvatar();
		}
		break;

	case DtChatChannelPacket::PI_Inscribe:
		if(g_Au.IsSceneWorker())
		{
			AuGearPos pos;
			QString inscription;

			block >> pos;
			block >> inscription;

			m_I->InscribeGearItem(pos, inscription);
		}
		break;

	case DtChatChannelPacket::PI_Message:
		if(g_Au.IsSceneWorker())
		{
			AuGearPos pos;
			QString message;

			block >> pos;
			block >> message;

			m_I->SetMessage(pos, message);
		}
		break;

	case DtChatChannelPacket::PI_SetPictureFrame:
		if(g_Au.IsSceneWorker())
		{
			AuGearPos pos;
			QString url;
			bool rotate;
			bool hideURL;

			block >> pos;
			block >> url;
			block >> rotate;
			block >> hideURL;

			m_I->SetPictureFrame(pos, url, hideURL, rotate);
		}
		break;

	case DtChatChannelPacket::PI_SetFlowerPot:
		if(g_Au.IsSceneWorker())
		{
			AuGearPos pos;
			quint8 plantSize;

			block >> pos;
			block >> plantSize;

			m_I->SetFlowerPot(pos, plantSize);
		}
		break;

	case DtChatChannelPacket::PI_Manipulate:
		{
			AuId itemId;
			quint8 doWhat;
			double amount;
			bool relativeToItem;
			AuMatrix4x4 t;
			QString room;

			block >> itemId;
			block >> doWhat;

			switch(doWhat)
			{
			// Move
			case 0:
				block >> amount;
				block >> relativeToItem;
				m_I->MoveItemRight(itemId, amount, relativeToItem);
				break;
			case 1:
				block >> amount;
				block >> relativeToItem;
				m_I->MoveItemLeft(itemId, amount, relativeToItem);
				break;
			case 2:
				block >> amount;
				block >> relativeToItem;
				m_I->MoveItemForward(itemId, amount, relativeToItem);
				break;
			case 3:
				block >> amount;
				block >> relativeToItem;
				m_I->MoveItemBackward(itemId, amount, relativeToItem);
				break;
			case 4:
				block >> amount;
				block >> relativeToItem;
				m_I->MoveItemUp(itemId, amount, relativeToItem);
				break;
			case 5:
				block >> amount;
				block >> relativeToItem;
				m_I->MoveItemDown(itemId, amount, relativeToItem);
				break;

			// Rotate
			case 6:
				block >> amount;
				block >> relativeToItem;
				m_I->RotateItemXPos(itemId, amount, relativeToItem);
				break;
			case 7:
				block >> amount;
				block >> relativeToItem;
				m_I->RotateItemXNeg(itemId, amount, relativeToItem);
				break;
			case 8:
				block >> amount;
				block >> relativeToItem;
				m_I->RotateItemYPos(itemId, amount, relativeToItem);
				break;
			case 9:
				block >> amount;
				block >> relativeToItem;
				m_I->RotateItemYNeg(itemId, amount, relativeToItem);
				break;
			case 10:
				block >> amount;
				block >> relativeToItem;
				m_I->RotateItemZPos(itemId, amount, relativeToItem);
				break;
			case 11:
				block >> amount;
				block >> relativeToItem;
				m_I->RotateItemZNeg(itemId, amount, relativeToItem);
				break;

			// Reset
			case 12:
				block >> t;
				block >> room;
				m_I->ResetItemTransform(itemId, t, room);
				break;

			// Take
			case 13:
				m_I->TakeItem(itemId);
				break;
			}
		}
		break;

	case DtChatChannelPacket::PI_Drop:
		//qDebug("PI_Drop:");
		if(g_Au.IsSceneWorker())
		{
			AuGearPos pos;

			block >> pos;

			m_I->DropGearItem(pos);
		}
		break;
	case DtChatChannelPacket::PI_GearItem:
		//qDebug("PI_GearItem:");
		if(g_Au.IsSceneWorker())
		{
			AuGearPos fromPos;
			quint8 doWhat;

			block >> fromPos;
			block >> doWhat;
			if(doWhat == 0)
			{
				// Move an item to a specific gear position, that contains an existing item.
				AuGearPos toPos;

				block >> toPos;

				m_I->MoveGearItemToItem(fromPos, toPos);
			}
			else if(doWhat == 1)
			{
				// Move an item to a top level gear slot of a specific type.
				quint8 toSlotType;

				block >> toSlotType;

				m_I->MoveGearItemToSlot(fromPos, toSlotType);
			}
			else if(doWhat == 2)
			{
				// Delete data from a storage device.
				QSet<COMMODITYID> patentCIDs;
				QSet<quint32> scDesigns;
				QSet<quint32> bdDesigns;
				QSet<QString> missions;
				QSet<QString> papers;
				quint16 c;
				// Patents
				block >> c;
				while(c--)
				{
					quint16 cid;
					block >> cid;
					patentCIDs.insert((COMMODITYID)cid);
				}
				// Sc designs
				block >> c;
				while(c--)
				{
					quint32 designId;
					block >> designId;
					scDesigns.insert(designId);
				}
				// Bd designs
				block >> c;
				while(c--)
				{
					quint32 designId;
					block >> designId;
					bdDesigns.insert(designId);
				}
				// Missions
				block >> c;
				while(c--)
				{
					QString missionName;
					block >> missionName;
					missions.insert(missionName);
				}
				block >> papers;

				m_I->GearDeleteStoredData(fromPos, patentCIDs, scDesigns, bdDesigns, missions, papers);
			}
			else if(doWhat == 3)
			{
				// Collect bounty on a gear item.
				m_I->CollectBounty(fromPos);
			}
		}
		break;
	case DtChatChannelPacket::PI_SendFunds:
		//qDebug("PI_SendFunds:");
		if(g_Au.IsSceneWorker())
		{
			AuId toId;
			QString toName;
			double amount;

			block >> toId;
			block >> toName;
			block >> amount;

			m_I->SendFunds(toId, toName, amount);
		}
		break;

	case DtChatChannelPacket::PI_Use:
		//qDebug("PI_Use:");
		if(g_Au.IsSceneWorker())
		{
			AuGearPos pos;

			block >> pos;

			m_I->UseGearItem(pos);
		}
		break;

	case DtChatChannelPacket::PI_GetFromCargo:
		//qDebug("PI_GetFromCargo:");
		if(g_Au.IsSceneWorker())
		{
			quint8 itemType;
			AuItem* item;

			block >> itemType;
			item = AuItem::Create(itemType);
			if(item)
			{
				block >> *item;

				m_I->GetFromCargo(*item);
				delete item;
			}
		}
		break;

	case DtChatChannelPacket::PI_PutIntoCargo:
		//qDebug("PI_PutIntoCargo:");
		if(g_Au.IsSceneWorker())
		{
			quint8 itemType;
			AuItem* item;

			block >> itemType;
			item = AuItem::Create(itemType);
			if(item)
			{
				block >> *item;

				m_I->PutIntoCargo(*item);
				delete item;
			}
		}
		break;

	case DtChatChannelPacket::PI_Ordnance:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Send a list of the ordnance available for loading.
				OrdnanceList();
			}
			else if(doWhat >= DaVehicle::RACK_Gun && doWhat <= DaVehicle::RACK_Ord1)
			{
				// Client wants to load a unit of ordnance.
				QByteArray ba;
				
				block >> ba;

				AuItem* loadItem = AuItem::Unpack(ba);
				if(loadItem)
				{
					m_I->OrdnanceLoad(doWhat, *loadItem);
					delete loadItem;
				}
			}
		}
		break;

	case DtChatChannelPacket::PI_TransferCargo:
		if(g_Au.IsSceneWorker())
		{
			quint8 doWhat;
			AuId otherShipId;

			block >> doWhat;
			block >> otherShipId;

			if(!otherShipId.IsNull())
			{
				if(doWhat == 1)
				{
					// Send money.
					double amount;
					block >> amount;
					m_I->TransferSendMoney(otherShipId, amount);
				}
				else if(doWhat == 2)
				{
					// Take money.
					double amount;
					block >> amount;
					m_I->TransferTakeMoney(otherShipId, amount);
				}
				else if(doWhat == 3)
				{
					// Send cargo item.
					quint8 itemType;
					AuItem* item;

					block >> itemType;
					item = AuItem::Create(itemType);
					if(item)
					{
						block >> *item;
						m_I->TransferSendCargo(otherShipId, *item);
						delete item;
					}
				}
				else if(doWhat == 4)
				{
					// Take cargo item.
					quint8 itemType;
					AuItem* item;

					block >> itemType;
					item = AuItem::Create(itemType);
					if(item)
					{
						block >> *item;
						m_I->TransferTakeCargo(otherShipId, *item);
						delete item;
					}
				}
				else if(doWhat == 5)
				{
					// Send cargo list.
					AuItemHash cargo;

					block >> cargo;

					m_I->TransferSendCargo(otherShipId, cargo);
				}
				else if(doWhat == 6)
				{
					// Take cargo list.
					AuItemHash cargo;

					block >> cargo;

					m_I->TransferTakeCargo(otherShipId, cargo);
				}
				else if(doWhat == 7)
				{
					// Send Citizen
					AuId citizenId, swapId;
					block >> citizenId;
					block >> swapId;

					AuIdList citizenIds, swapIds;
					citizenIds.append(citizenId);
					if(!swapId.IsNull())
						swapIds.append(swapId);
					m_I->TransferSendCitizens(otherShipId, citizenIds, swapIds);
				}
				else if(doWhat == 8)
				{
					// Take Citizen
					AuId citizenId, swapId;
					block >> citizenId;
					block >> swapId;

					AuIdList citizenIds, swapIds;
					citizenIds.append(citizenId);
					if(!swapId.IsNull())
						swapIds.append(swapId);
					m_I->TransferTakeCitizens(otherShipId, citizenIds, swapIds);
				}
				else if(doWhat == 9)
				{
					// Send citizens list.
					quint16 count;
					AuIdList citizenIds, swapIds;
					AuId id;
					
					block >> count;
					while(count--)
					{
						block >> id;
						citizenIds.append(id);
					}
					block >> count;
					while(count--)
					{
						block >> id;
						swapIds.append(id);
					}

					m_I->TransferSendCitizens(otherShipId, citizenIds, swapIds);
				}
				else if(doWhat == 10)
				{
					// Take citizens list.
					quint16 count;
					AuIdList citizenIds, swapIds;
					AuId id;
					
					block >> count;
					while(count--)
					{
						block >> id;
						citizenIds.append(id);
					}
					block >> count;
					while(count--)
					{
						block >> id;
						swapIds.append(id);
					}

					m_I->TransferTakeCitizens(otherShipId, citizenIds, swapIds);
				}
				else if(doWhat == 11)
				{
					// Send all items
					m_I->TransferSendItemsLoose(otherShipId);
					m_I->TransferSendItemsStatic(otherShipId);
				}
				else if(doWhat == 12)
				{
					// Send static items
					m_I->TransferSendItemsStatic(otherShipId);
				}
				else if(doWhat == 13)
				{
					// Send loose items
					m_I->TransferSendItemsLoose(otherShipId);
				}
				else if(doWhat == 14)
				{
					// Take all items
					m_I->TransferTakeItemsLoose(otherShipId);
					m_I->TransferTakeItemsStatic(otherShipId);
				}
				else if(doWhat == 15)
				{
					// Take static items
					m_I->TransferTakeItemsStatic(otherShipId);
				}
				else if(doWhat == 16)
				{
					// Take loose items
					m_I->TransferTakeItemsLoose(otherShipId);
				}
				else if(doWhat == 17)
				{
					// Query citizens for transfer.
					TransferListCitizens(otherShipId);
				}
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: ERROR! Received a PI_TransferCargo packet with NULL otherShip ID. "
							"Hacker? %1 unit %2 %3").arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
			}
		}
		break;

	case DtChatChannelPacket::PB_BdBuild:
		if(g_Au.IsSceneWorker())
		{
			quint8 industryId;
			quint32 designId;
			quint16 material;
			AuPoint buildLoc;
			float zRotation;
			AuId onPlatform;
			quint16 mpid;
			bool workerOk;
			quint8 buildWhat;
			QString cityName;
			QString systemName;
			QString sectorName;

			block >> industryId;
			if(industryId != IID_None)
			{
				// Not a preview.
				block >> designId;
				block >> material;
				block >> buildLoc;
				block >> zRotation;
				block >> onPlatform;
				block >> mpid;
				block >> workerOk;
				block >> buildWhat;
				if(buildWhat == 1)
				{
					// Founding a city.
					block >> cityName;
					block >> systemName;
					block >> sectorName;
					m_I->BdFoundCity((INDUSTRYID)industryId, designId, (COMMODITYID)material, buildLoc, zRotation, onPlatform, mpid,
							workerOk, cityName, systemName, sectorName);
				}
				else if(buildWhat == 2)
				{
					// Building a storage tank or warehouse.
					quint16 storeCID;
					block >> storeCID;
					m_I->BdBuildStorage((INDUSTRYID)industryId, designId, (COMMODITYID)material, buildLoc, zRotation, onPlatform,
							mpid, workerOk, (COMMODITYID)storeCID);
				}
				else
				{
					m_I->BdBuild((INDUSTRYID)industryId, designId, (COMMODITYID)material, buildLoc, zRotation, onPlatform, mpid,
							workerOk);
				}
			}
			else
			{
				// Preview
				quint8 previewWhat;
				block >> previewWhat;
				if(previewWhat == 0)
				{
					// Building
					block >> buildLoc;
					block >> zRotation;
					m_I->BdPreview(buildLoc, zRotation);
				}
				else if(previewWhat == 1)
				{
					// Fence
					AuPoint startLoc, endLoc;
					block >> startLoc;
					block >> endLoc;
					m_I->BdPreviewFence(startLoc, endLoc);
				}
			}
		}
		break;

	case DtChatChannelPacket::PB_PlanFence:
		if(g_Au.IsSceneWorker())
		{
			quint8 fenceStyle;
			AuPoint fenceStart;
			AuPoint fenceEnd;
			AuId onPlatform;
			quint32 customDesignId;
			quint16 mat;

			block >> fenceStart;
			block >> fenceEnd;
			block >> fenceStyle;
			block >> onPlatform;
			block >> customDesignId;
			block >> mat;

			m_I->BdFence(fenceStart, fenceEnd, fenceStyle, onPlatform, customDesignId, mat);
		}
		break;

	case DtChatChannelPacket::PB_PlanRoadGlobe:
		//qDebug() << "PB_PlanRoadGlobe";
		if(g_Au.IsSceneWorker())
		{
			quint8 cpid;
			block >> cpid;
			if(cpid >= CPID_DirtRoad)
			{
			}
			else if(cpid == 1)
			{
				// New style geo landscaper.
				AuPoint roadStart, roadEnd;
				float roadWidth;
				bool hideRocks;
				quint8 hideGrass;
				bool hideShrubs;
				bool hideTrees;
				quint8 pavement;

				block >> roadStart;
				block >> roadEnd;
				block >> roadWidth;
				block >> hideRocks;
				block >> hideGrass;
				block >> hideShrubs;
				block >> hideTrees;
				block >> pavement;

				m_I->GeoLandscaper(roadStart, roadEnd, roadWidth, hideRocks, hideGrass, hideShrubs, hideTrees,
						(WORLDTEX_TYPE)pavement);
			}
			else
			{
				// New style geo road.
				AuPoint roadStart, roadEnd;
				bool isSlab;
				float roadWidth;
				quint8 geoType;

				block >> roadStart;
				block >> roadEnd;
				block >> isSlab;
				block >> roadWidth;
				block >> geoType;

				m_I->GeoRoad(roadStart, roadEnd, isSlab, roadWidth, geoType);
			}
		}
		break;

	case DtChatChannelPacket::PB_Demolish:
		if(g_Au.IsSceneWorker())
		{
			quint8 demWhat;
			block >> demWhat;
			if(demWhat == 0)
			{
			}
			else if(demWhat == 1)
			{
				AuId bdId;
				block >> bdId;
				m_I->DemolishBd(bdId);
			}
		}
		break;

	case DtChatChannelPacket::PB_ClearLand:
		//qDebug("PB_ClearLand:");
		if(g_Au.IsSceneWorker())
		{
			quint8 clearWhat;
			block >> clearWhat;
			switch(clearWhat)
			{
			case 1:	// bd
				{
					AuId bdId;
					block >> bdId;
					m_I->DemolishBd(bdId);
				}
				break;
			case 2:	// dev
				{
				}
				break;
			case 3:	// geo
				{
					AuGlobeLLF geoLL;
					block >> geoLL;

					m_I->ClearGeo(geoLL);
				}
				break;
			}
		}
		break;

	case DtChatChannelPacket::PB_ChangeRoadConstruction:
		//qDebug("PB_ChangeRoadMaterial:");
		if(g_Au.IsSceneWorker())
		{
			AuId cityId;
			AuGlobeLLF roadDevLL;
			quint8 cpid;

			block >> cityId;
			block >> roadDevLL;
			block >> cpid;

			m_I->ChangeRoadConstruction(cityId, roadDevLL, (CONSTRUCTIONPROCESSID)cpid);
		}
		break;

	case DtChatChannelPacket::PJ_Accept:
		//qDebug("PJ_AcceptJob:");
		{
			AuJobOffer jobOffer;

			block >> jobOffer;

			m_I->JobAccepted(jobOffer);
		}
		break;

	case DtChatChannelPacket::PJ_Query:
		//qDebug("PJ_Query:");
		if(g_Au.IsSceneWorker())
			QueryJobs();
		break;

	case DtChatChannelPacket::PJ_QueryFilter:
		//qDebug("PJ_QueryFilter:");
		if(g_Au.IsSceneWorker())
		{
			quint8 filterCount;
			quint16 maxSectorsAway;

			block >> maxSectorsAway;
			block >> filterCount;

			QList<AuJob::TYPE> filterTypes;
			quint8 ui8;
			while(filterCount--)
			{
				block >> ui8;
				filterTypes.append((AuJob::TYPE)ui8);
			}

			if(maxSectorsAway > MaxJobSectorRange())
				maxSectorsAway = MaxJobSectorRange();

			QueryJobs(filterTypes, maxSectorsAway);
		}
		break;

	case DtChatChannelPacket::PS_CloseStory:
		//qDebug("PS_CloseStory:");
		{
			// This message is no longer used to just close stories. It is used to close any job.
			AuJobOffer jobToClose;

			block >> jobToClose;

			if(jobToClose.Job())
				m_I->CloseJob(jobToClose.Job());
		}
		break;
	case DtChatChannelPacket::PS_ChoiceMade:	// Text of a choice to client. Choice picked to server.
		//qDebug("PS_ChoiceMade:");
		{
			quint32 storyInstance;
			qint8 value;
			AuId offeredBy;

			block >> storyInstance;
			block >> value;
			block >> offeredBy;

			m_I->StoryChoicePicked(storyInstance, value, offeredBy);
		}
		break;
	case DtChatChannelPacket::PS_StoryHint:
		//qDebug("PS_StoryHint:");
		{
			quint32 storyInstance;
			block >> storyInstance;
			m_I->StorySendHint(storyInstance);
		}
		break;
	case DtChatChannelPacket::PS_StoryResendBlock:
		//qDebug("PS_StoryResendBlock:");
		{
			quint32 storyInstance;
			block >> storyInstance;
			m_I->StorySendCurrentBlock(storyInstance);
		}
		break;

	case DtChatChannelPacket::PM_Hangar:
		{
			quint8 hangarAction;
			block >> hangarAction;
			switch(hangarAction)
			{
			case 0:						// Query what ship of mine is stored here.
				HangarQuery();
				break;
			case 1:						// Mothball a nearby ship into my hangar.
				{
					AuId shipId;
					block >> shipId;
					HangarMothball(shipId);
				}
				break;
			case 2:						// Launch the ship from my hangar.
				{
					bool takeCaptBerth;
					block >> takeCaptBerth;
					HangarLaunch(takeCaptBerth);
				}
				break;
			case 3:
				// Client keep alive packets were squeezed in without restarting the servers. So they became hangar action 3.
				break;

			case 10:	// Repair Shop recover ships from stasis.
				m_I->RepairShopRecover();
				break;

			case 11:	// Agent recover ship from stasis.
				if(AcctIsAgent(m_AccountType))
				{
					AuId shipId;
					block >> shipId;
					m_I->AgentRecoverShip(shipId);
				}
				break;
			}
		}
		break;

	case DtChatChannelPacket::PM_CitizenEnabled:
		//qDebug("PM_CitizenEnabled:");
		{
			quint8 panelIndex;
			block >> panelIndex;
			if(panelIndex <= 99)
			{
			}
			else if(panelIndex == 101)
			{
				// Construction worker enable.
				quint8 enabled;
				AuId opBd;

				block >> enabled;
				block >> opBd;

				//qDebug() << "PM_CitizenEnabled" << enabled << "opBd" << opBd;
				m_I->BdConstructionEnabled(enabled, opBd);
			}
			else if(panelIndex == 102)
			{
				// Spacecraft factory Manufacturing worker enable.
				quint32 enabled;
				AuId opBd;

				block >> enabled;
				block >> opBd;

				m_I->BdManufactureSpacecraftCitizenEnabled(enabled, opBd);
			}
			else if(panelIndex == 103)
			{
				// Manufacturing worker enable.
				quint16 mpid;
				quint32 enabled;
				AuId opBd;

				block >> mpid;
				block >> enabled;
				block >> opBd;

				m_I->BdManufactureCitizenEnabledMPID(mpid, enabled, opBd);
			}
		}
		break;

	case DtChatChannelPacket::PM_Fetch:
		//qDebug("PM_Fetch:");
		{
			quint8 panelIndex;
			block >> panelIndex;
			if(panelIndex <= 99)
			{
			}
			else if(panelIndex == 101)
			{
				// Construction fetch.
				AuId opBd;

				block >> opBd;

				m_I->BdConstructionFetch(opBd);
			}
			else if(panelIndex == 102)
			{
				// Manufacture fetch.
				quint16 pindex;
				AuId opBd;

				block >> pindex;
				block >> opBd;

				m_I->BdManufactureFetch(pindex, opBd);
			}
			else if(panelIndex == 103)
			{
				// Manufacture fetch.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureFetchMPID(mpid, opBd);
			}
		}
		break;

	case DtChatChannelPacket::PM_Go:
		//qDebug("PM_Go:");
		{
			quint8 panelIndex;
			block >> panelIndex;
			if(panelIndex <= 99)
			{
			}
			else if(panelIndex == 101)
			{
				// Construction go.
				AuId opBd;

				block >> opBd;

				m_I->BdConstructionGo(opBd);
			}
			else if(panelIndex == 102)
			{
				// Manufacture go.
				quint16 pindex;
				AuId opBd;

				block >> pindex;
				block >> opBd;

				m_I->BdManufactureGo(pindex, opBd);
			}
			else if(panelIndex == 103)
			{
				// Manufacture go.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureGoMPID(mpid, opBd);
			}
		}
		break;

	case DtChatChannelPacket::PM_ProcessSelected:
		//qDebug("PM_ProcessSelected:");
		{
			quint8 panelIndex;
			block >> panelIndex;
			if(panelIndex == 0xff)
			{
				// Clears all manufacturing processes from a building.
				AuId opBd;

				block >> opBd;

				m_I->BdManufactureClear(opBd);
			}
			else if(panelIndex == 0xfe)
			{
				// Bd process priority lowered to the bottom.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureMinMPID(mpid, opBd);
			}
			else if(panelIndex == 0xfd)
			{
				// Bd process priority raised to the top.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureMaxMPID(mpid, opBd);
			}
			else if(panelIndex == 0xfc)
			{
				// Discard from a bd.
				QByteArray buyIt;
				AuId opBd;

				block >> buyIt;
				block >> opBd;

				AuItem* buyItem = AuItem::Unpack(buyIt);
				if(buyItem)
				{
					m_I->BdDiscardItem(*buyItem, opBd);
					delete buyItem;
				}
			}
			else if(panelIndex == 0xfb)
			{
				// Record one or more patents in the local city.
				AuKnownPatents patents;
				AuId opBd;

				block >> patents;
				block >> opBd;

				m_I->BdManufactureRecordPatents(patents, opBd);
			}
			else if(panelIndex == 0xfa)
			{
				// Request a copy of one or more patents recorded in the local city.
				QSet<COMMODITYID> patentCIDs;
				quint16 c;
				quint8 maxQ;
				AuGearPos mediaPos;
				AuId opBd;

				block >> c;
				while(c--)
				{
					quint16 cid;
					block >> cid;
					patentCIDs.insert((COMMODITYID)cid);
				}
				block >> maxQ;
				block >> mediaPos;
				block >> opBd;

				m_I->BdManufactureGetPatents(patentCIDs, maxQ, mediaPos, opBd);
			}
			else if(panelIndex == 0xf9)
			{
				// Select livestock DNA at a bd farm.
				AuSpecies species;
				AuId opBd;

				block >> species;
				block >> opBd;

				m_I->BdManufactureSetLivestock(species, opBd);
			}
			else if(panelIndex == 0xf8)
			{
				// Set warehouse commodity.
				quint16 cid;
				AuId opBd;

				block >> cid;
				block >> opBd;

				m_I->BdManufactureSetWarehouseStorage((COMMODITYID)cid, opBd);
			}
			else if(panelIndex == 0xf7)
			{
				// Buy from a story.
				QByteArray buyIt;
				QByteArray offerIt;
				AuId opBd;

				block >> buyIt;
				block >> offerIt;
				block >> opBd;

				AuItem* buyItem = AuItem::Unpack(buyIt);
				AuItem* offerItem = AuItem::Unpack(offerIt);
				if(buyItem && offerItem)
					m_I->BdStoreBuyItem(*buyItem, *offerItem, opBd);
				if(buyItem)
					delete buyItem;
				if(offerItem)
					delete offerItem;
			}
			else if(panelIndex == 0xf6)
			{
				// Record one or more patents in the empire.
				AuKnownPatents patents;

				block >> patents;

				m_I->EmpireRecordPatents(patents);
			}
			else if(panelIndex == 0xf5)
			{
				// Request a copy of one or more patents recorded in the empire.
				QSet<COMMODITYID> patentCIDs;
				quint16 c;
				quint8 maxQ;
				AuGearPos mediaPos;

				block >> c;
				while(c--)
				{
					quint16 cid;
					block >> cid;
					patentCIDs.insert((COMMODITYID)cid);
				}
				block >> maxQ;
				block >> mediaPos;

				m_I->EmpireGetPatents(patentCIDs, maxQ, mediaPos);
			}
			else if(panelIndex == 0xf4)
			{
				// Take from a bd.
				QByteArray buyIt;
				AuId opBd;

				block >> buyIt;
				block >> opBd;

				AuItem* buyItem = AuItem::Unpack(buyIt);
				if(buyItem)
				{
					m_I->BdTakeItem(*buyItem, opBd);
					delete buyItem;
				}
			}
			else if(panelIndex == 0xf3)
			{
				// Bd process priority lowered.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureLowerMPID(mpid, opBd);
			}
			else if(panelIndex == 0xf2)
			{
				// Bd process priority raised.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureRaiseMPID(mpid, opBd);
			}
			else if(panelIndex == 0xf1)
			{
				// Bd process added.
				quint16 mpid;
				quint8 minQ;
				quint32 workshops, stockLimit;
				AuId opBd;

				block >> mpid;
				block >> minQ;
				block >> workshops;
				block >> stockLimit;
				block >> opBd;

				m_I->BdManufactureAddMPID(mpid, minQ, workshops, stockLimit, opBd);
			}
			else if(panelIndex == 0xf0)
			{
				// Bd process deleted.
				quint16 mpid;
				AuId opBd;

				block >> mpid;
				block >> opBd;

				m_I->BdManufactureDeleteMPID(mpid, opBd);
			}
			else if(panelIndex == 102)
			{
				// Bd process selected.
				quint16 pindex;
				qint32 mpid;
				AuId opBd;

				block >> pindex;
				block >> mpid;
				block >> opBd;

				m_I->BdManufactureProcessSelected(pindex, mpid, opBd);
			}
			else
			{
			}
		}
		break;

	case DtChatChannelPacket::PM_ScFactory:
		//qDebug("PM_ScFactory:");
		{
			quint8 doWhat;
			block >> doWhat;

			// Bd spacecraft factory
			if(doWhat == 9)
			{
				// Delete fleet ship order.
				quint32 designId;
				AuId opBd;

				block >> designId;
				block >> opBd;

				m_I->BdManufactureSpacecraftOrderDelete(designId, opBd);
			}
			else if(doWhat == 10)
			{
				// Buy company ship.
				quint32 designId;
				AuGearPos pos;
				AuId opBd;

				block >> designId;
				block >> pos;
				block >> opBd;

				m_I->BdManufactureSpacecraftBuyCompanyShip(designId, pos, opBd);
			}
			else if(doWhat == 11)
			{
				// Cancel current running process.
				AuId opBd;

				block >> opBd;

				m_I->BdManufactureSpacecraftProcessCanceled(opBd);
			}
			else if(doWhat == 12)
			{
				// Commission ship.
				quint32 designId;
				AuGearPos pos;
				AuId opBd;

				block >> designId;
				block >> pos;
				block >> opBd;

				m_I->BdManufactureSpacecraftCommissionShip(designId, pos, opBd);
			}
			else if(doWhat == 13)
			{
				// Select a spacecraft design to manufacture, either the nu default design or a design to add to the build queue.
				quint32 designId;
				QString designName;
				quint8 designQ;
				AuId opBd;

				block >> designId;
				block >> designName;
				block >> designQ;
				block >> opBd;

				m_I->BdManufactureSpacecraftProcessDefaultSelected(designId, designName, designQ, opBd);
			}
			else if(doWhat == 14)
			{
				// Set company policy.
				bool companyOk;
				quint8 companyMaxQ;
				AuId opBd;

				block >> companyOk;
				block >> companyMaxQ;
				block >> opBd;

				m_I->BdManufactureSpacecraftSetCompanyPolicy(companyOk, companyMaxQ, opBd);
			}
			else if(doWhat == 15)
			{
				// Set minimum quality.
				quint8 minQ;
				AuId opBd;

				block >> minQ;
				block >> opBd;

				m_I->BdManufactureSpacecraftSetMinQ(minQ, opBd);
			}
		}
		break;

	case DtChatChannelPacket::PM_ProcessSetMinQ:
		//qDebug("PM_ProcessSetMinQ:");
		{
			quint8 panelIndex;

			block >> panelIndex;
			if(panelIndex <= 99)
			{
			}
			else if(panelIndex == 102)
			{
				// Sets minQ for all processes.
				quint8 minQ;
				AuId opBd;

				block >> minQ;
				block >> opBd;

				m_I->BdManufactureSetMinQ(minQ, opBd);
			}
			else if(panelIndex == 103)
			{
				quint16 mpid;
				quint8 minQ;
				AuId opBd;

				block >> mpid;
				block >> minQ;
				block >> opBd;

				m_I->BdManufactureProcessSetMinQMPID(mpid, minQ, opBd);
			}
			else if(panelIndex == 104)
			{
				quint16 mpid;
				quint32 stockLimit;
				AuId opBd;

				block >> mpid;
				block >> stockLimit;
				block >> opBd;

				m_I->BdManufactureProcessSetStockLimitMPID(mpid, stockLimit, opBd);
			}
		}
		break;

	case DtChatChannelPacket::PM_HandCraftGo:
		//qDebug("PM_HandCraftGo:");
		{
			qint32 mpid;
			block >> mpid;
			m_I->HandcraftGo(mpid);
		}
		break;

	case DtChatChannelPacket::PM_EnterStudio:
		//qDebug("PM_EnterStudio:");
		{
			quint16 panelIndex;
			QString password;

			block >> panelIndex;
			block >> password;

			DesignStudioEnter(panelIndex, password);
		}
		break;

	case DtChatChannelPacket::PM_CreateArena:
		//qDebug("PM_CreateArena:");
		{
			AuArenaReady arena;

			block >> arena;

			m_I->ArenaCreateNew(arena);
		}
		break;

	case DtChatChannelPacket::PM_JoinArena:
		//qDebug("PM_JoinArena:");
		{
			AuId arenaId;
			QString password;
			quint32 designId;
			quint8 team;

			block >> arenaId;
			block >> password;
			block >> designId;
			block >> team;

			m_I->ArenaJoin(arenaId, password, designId, team);
		}
		break;

	case DtChatChannelPacket::PM_ArenaExit:
		if(g_Au.IsSceneWorker())
			m_I->ArenaExit();
		break;

	case DtChatChannelPacket::PM_ArenaRecall:
		if(g_Au.IsSceneWorker())
			m_I->ArenaRecall();
		break;

	case DtChatChannelPacket::PM_ArenaStatus:
		if(g_Au.IsSceneWorker())
			m_I->ArenaStatus();
		break;

	case DtChatChannelPacket::PM_QueryArenas:
		if(g_Au.IsSceneWorker())
			ArenaSendList();
		break;

	case DtChatChannelPacket::PV_SetThemeColor:
		//qDebug("PV_SetThemeColor:");
		{
			quint8 theme;	// This is a AuScFace::COLOR_THEME enum value.
			QColor color;
			
			block >> theme;
			block >> color;

			m_I->GovSetThemeColor(theme, color);
		}
		break;

	case DtChatChannelPacket::PV_SetAnnouncements:
		//qDebug("PV_SetAnnouncements:");
		{
			QString announcements;
			
			block >> announcements;

			m_I->GovSetAnnouncements(announcements);
		}
		break;

	case DtChatChannelPacket::PV_WarCriteria:
		//qDebug("PV_WarCriteria:");
		{
			AuWarCriteria crit;
			
			block >> crit;

			m_I->GovSetWarCriteria(crit);
		}
		break;

	case DtChatChannelPacket::PV_SetCapital:
		//qDebug("PV_SetCapital:");
		{
			quint8 capitalType;
			block >> capitalType;
			if(capitalType == 2)
				m_I->GovQuitClaim();
			else
				m_I->GovSetCapital(capitalType);
		}
		break;

	case DtChatChannelPacket::PV_SetJoinPass:
		//qDebug("PV_SetCapital:");
		{
			QByteArray passCrypt;
			block >> passCrypt;
			m_I->GovSetJoinPassword(passCrypt);
		}
		break;

	case DtChatChannelPacket::PV_SetCommerce:
		//qDebug("PV_SetCommerce:");
		{
			QByteArray commerce;
			block >> commerce;
			m_I->GovSetCommerce(commerce);
		}
		break;

	case DtChatChannelPacket::PV_ContrailColor:
		//qDebug("PV_ContrailColor:");
		{
			quint8 hue, sat;
			block >> hue;
			block >> sat;
			m_I->GovSetContrailColor(hue, sat);
		}
		break;

	case DtChatChannelPacket::PV_SetDefStance:
		//qDebug("PV_SetDefStance:");
		{
			quint8 stance;
			block >> stance;
			m_I->GovSetDefaultPoliticalStance(stance);
		}
		break;

	case DtChatChannelPacket::PV_SetPilotPolicy:
		//qDebug("PV_SetPilotPolicy:");
		{
			quint8 whichPolicy;
			quint8 policyValue;

			block >> whichPolicy;
			block >> policyValue;

			m_I->GovSetPilotPolicy(whichPolicy, policyValue);
		}
		break;

	case DtChatChannelPacket::PV_SetShieldPolicy:
		//qDebug("PV_SetShieldPolicy:");
		{
			quint8 whichPolicy;
			quint8 policyValue;

			block >> whichPolicy;
			block >> policyValue;

			m_I->GovSetShieldPolicy(whichPolicy, policyValue);
		}
		break;

	case DtChatChannelPacket::PV_SetTax:
		//qDebug("PV_SetTax:");
		{
			quint8 income, sales, subsidy;

			block >> income;
			block >> sales;
			block >> subsidy;

			m_I->GovSetTax(income, sales, subsidy);
		}
		break;

	case DtChatChannelPacket::PV_SetPoliticalStance:
		//qDebug("PV_SetPoliticalStance:");
		{
			quint32 towardEmpire;
			quint8 stance, vassalTributePercent;
			block >> towardEmpire;
			block >> stance;
			block >> vassalTributePercent;
			m_I->GovSetPoliticalStance(towardEmpire, stance, vassalTributePercent);
		}
		break;

	case DtChatChannelPacket::PV_ReceiveShipFunds:	// Withdraw funds from ship's account.
		//qDebug("PV_ReceiveShipFunds:");
		{
			double amount;
			AuId idShip;
			block >> amount;
			block >> idShip;
			if(g_Au.IsSceneWorker())
				m_I->WithdrawShipFunds(amount, idShip);
		}
		break;

	case DtChatChannelPacket::PV_SendShipFunds:	// Deposit funds into ship's account.
		//qDebug("PV_SendShipFunds:");
		{
			double amount;
			AuId idShip;
			block >> amount;
			block >> idShip;
			if(g_Au.IsSceneWorker())
				m_I->DepositShipFunds(amount, idShip);
		}
		break;

	case DtChatChannelPacket::PV_ClaimReward:
		//qDebug("PV_ClaimReward:");
		{
			AuId opBd;

			block >> opBd;

			m_I->RedeemGovtDebt(opBd);
		}
		break;

	case DtChatChannelPacket::PV_AssignOffice:
		{
			AuId toWho;
			QString toName;
			AuOffice office;

			block >> toWho;
			block >> toName;
			block >> office;

			m_I->GovAssignOffice(toWho, toName, office);
		}
		break;

	case DtChatChannelPacket::PV_SetBoss:
		{
			AuId governorId;
			AuId bossId;

			block >> governorId;
			block >> bossId;

			m_I->GovSetBoss(governorId, bossId);
		}
		break;

	case DtChatChannelPacket::PV_ResignOffice:
		//qDebug("PV_ResignOffice:");
		{
			m_I->GovResignOffice();
		}
		break;

	case DtChatChannelPacket::PV_RevokeOffice:
		//qDebug("PV_RevokeOffice:");
		{
			AuId fromWho;
			QString fromName;

			block >> fromWho;
			block >> fromName;

			m_I->GovRevokeOffice(fromWho, fromName);
		}
		break;

	case DtChatChannelPacket::PV_KickBerthOccupant:
		//qDebug("PV_KickBerthOccupant:");
		{
			quint8 doWhat;
			block >> doWhat;
			switch(doWhat)
			{
			case 0:	// Kick berth occupant.
				{
					AuId kickId;
					block >> kickId;
					if(!kickId.IsNull())
						m_I->KickBerthOccupant(kickId);
				}
				break;
			case 1:	// Kick stowaways.
				m_I->KickStowaways();
				break;
			case 2:	// Kick cryo travellers.
				{
					AuSpecies species;
					quint32 count;
					block >> species;
					block >> count;
					m_I->KickCryoBerthOccupants(species, count);
				}
				break;
			case 3:	// Revive cryo traveller.
				{
					AuSpecies species;
					quint8 role;
					block >> species;
					block >> role;
					m_I->ReviveCryoBerthOccupant(species, role);
				}
				break;
			}
		}
		break;
	case DtChatChannelPacket::PV_ReconcileRoster:
		//qDebug("PV_ReconcileRoster:");
		m_I->ReconcileRoster();
		break;
	case DtChatChannelPacket::PV_ReleaseBerth:
		//qDebug("PV_ReleaseBerth:");
		// This release implies no tie to any ship afterwards by sending a ship id of 0.
		m_I->ReleaseBerth();
		break;
	case DtChatChannelPacket::PV_AcceptCrew:
		//qDebug("PV_AcceptCrew:");
		{
			quint8 ui8;
			block >> ui8;
			m_I->SetAcceptCrew(ui8);
		}
		break;
	case DtChatChannelPacket::PV_AcceptPassengers:
		//qDebug("PV_AcceptPassengers:");
		{
			quint8 ui8;
			block >> ui8;
			m_I->SetAcceptPassengers(ui8);
		}
		break;
	case DtChatChannelPacket::PV_AcceptTroops:
		//qDebug("PV_AcceptTroops:");
		{
			quint8 ui8;
			block >> ui8;
			m_I->SetAcceptTroops(ui8);
		}
		break;

	case DtChatChannelPacket::PV_AllowPassengerSeaChests:
		//qDebug("PV_AllowPassengerSeaChests:");
		{
			quint8 ui8;
			block >> ui8;
			m_I->SetAllowPassengerSeaChests(ui8);
		}
		break;

	case DtChatChannelPacket::PV_Specie:
		// Client sent us a nu name for a specie.
		{
			AuSpecies species;
			QString speciesName;
			bool isWildAnimal;

			block >> species;
			block >> speciesName;
			block >> isWildAnimal;

			m_I->SetSpeciesName(species, speciesName, isWildAnimal);
		}
		break;

	case DtChatChannelPacket::PV_SpeciesRequest:
		// Client wants us to send them a specie record.
		{
			AuSpecies species;
			block >> species;

			// Add the dna to our cache list so SendSpeciesRecord knows the client cares about it.
			// This could include dna that has never been named.
			m_muSpecies.lock();
			m_Species.insert(pair<AuSpecies, bool>(species, true));
			m_muSpecies.unlock();

			SendSpeciesRecord(species);
		}
		break;

	case DtChatChannelPacket::PT_GameQuit:
		m_I->GameQuit();
		break;

	case DtChatChannelPacket::PT_GameSelect:
		{
			// Client selected a game to play.
			quint8 gameType;
			block >> gameType;
			bool gameSelected = m_I->GameSelected(gameType);

			// Send a packet back to the person who selected the game, with the results.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PT_GameSelect;
			out << gameType;
			out << gameSelected;
			SendPacket(ba);
		}
		break;

	case DtChatChannelPacket::PT_GameStart:
		{
			// Client started the current game.
			quint8 gameType;
			QByteArray gameSettings;
			block >> gameType;
			block >> gameSettings;
			m_I->GameStart(gameType, gameSettings);
		}
		break;

	case DtChatChannelPacket::PT_GameTurn:
		{
			// Client sent their move for a turn.
			quint8 gameType;
			QByteArray gameTurn;
			block >> gameType;
			block >> gameTurn;
			m_I->GameTurn(gameType, gameTurn);
		}
		break;

	case DtChatChannelPacket::PTR_InputCoord:
		{
			AuPoint coord;

			block >> coord;

			m_I->TransporterCoord(coord);
		}
		break;

	case DtChatChannelPacket::PTR_InputLongLat:
		{
			AuGlobeLLF ll;

			block >> ll;

			m_I->TransporterLongLat(ll);
		}
		break;

	case DtChatChannelPacket::PB_RaiseFlag_Obsolete:
	case DtChatChannelPacket::PT_GameOver:
	case DtChatChannelPacket::PF_Station0:
	case DtChatChannelPacket::PF_Station1:
	//case DtChatChannelPacket::PF_Station2:
	case DtChatChannelPacket::PF_Station3:
	case DtChatChannelPacket::PF_Station4:
	//case DtChatChannelPacket::PF_Station5:
	case DtChatChannelPacket::PF_Station6:
	//case DtChatChannelPacket::PF_Station7:
	case DtChatChannelPacket::PF_Station8:
	case DtChatChannelPacket::PF_Station9:
	//case DtChatChannelPacket::PF_Station10:
	case DtChatChannelPacket::PF_Station11:
	case DtChatChannelPacket::PF_Station12:
	case DtChatChannelPacket::PF_Station13:
	case DtChatChannelPacket::PF_Station14:
	case DtChatChannelPacket::PF_Station15:
	case DtChatChannelPacket::PF_Station16:
	case DtChatChannelPacket::PF_Station17:
	case DtChatChannelPacket::PF_Station18:
	case DtChatChannelPacket::PF_Station19:
	case DtChatChannelPacket::P_Invalid:
#if defined(QT_NO_DEBUG)
	default:				// No default in debug build for compiler warnings. Compiler warnings wanted in release build.
#endif
		g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: ERROR! Player %1 unit %2 %3: Unknown TCP packet type %4 size %5 from %6. Hacker?")
			.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName).arg(type).arg(ba.count())
			.arg(tcpSocket.peerAddress().toString()));
		m_BadPackets++;
		break;
	}
}

// Used by the client to send an array of bytes to the server.
void DtChatChannel::SendPacket(const QByteArray& ba, bool flush, bool usemutex)
{
	// The packet is already formatted. Add it to the transmit data stream.
	//qDebug("DtChatChannel::SendPacket - tx %d bytes", ba.count());
	if(usemutex)
	{
		// Most callers will use the mutex.
		m_muTxBuffer.lock();
		AuUtil::WritePacketSize(m_TxBuffer, ba.size());
		m_TxBuffer.append(ba);
		if(flush)
			m_TxBufferFlush = true;
		m_muTxBuffer.unlock();
	}
	else
	{
		// Don't use the mutex only if you've already locked it. SyncClient does
		// this so its stuff will be an uninterrupted stream in the buffer.
		AuUtil::WritePacketSize(m_TxBuffer, ba.size());
		m_TxBuffer.append(ba);
		if(flush)
			m_TxBufferFlush = true;
	}
}

void DtChatChannel::SendScDesign(unsigned int idDesign)
{
	AuScDesign design;
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool hasDesign = sdd.ScDesign(idDesign, design);	// Triggers load if it's not loaded.
	g_Au.UnlockScDesigns();
	//qDebug() << "DtChatChannel::SendScDesign" << idDesign << "hasDesign" << hasDesign;
	if(hasDesign)
	{
		// Remember the design id immediately so we know it's on the way to the client, in case they request it again before
		// we're done sending it.
		m_rwScDesigns.lockForWrite();
		m_ScDesigns.insert(pair<quint32, bool>(idDesign, true));
		m_rwScDesigns.unlock();

		m_muTxBuffer.lock();

		// Design Erase precedes design in case client already has it.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ScErase;
			out << design.Id();
			SendPacket(ba, false, false);
		}

		// Design rec is sent last so the client doesn't think it has the design until it has ALL of the design.
		// This is why it might ask a few times for the design before it's done receiving it.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ScDesign;
			design.Tx(out);
			SendPacket(ba, true, false);
		}

		m_muTxBuffer.unlock();
	}
	else
	{
		//qWarning("%lu Nonexistent spacecraft design %d requested", (unsigned long)currentThreadId(), idDesign);
	}
}

void DtChatChannel::SendBdDesign(unsigned int idDesign)
{
	AuBdDesign design;
	const DgData& sdd = g_Au.LockBdDesignsForRead();
	bool hasDesign = sdd.BdDesign(idDesign, design);	// Triggers load if it's not loaded.
	g_Au.UnlockBdDesigns();
	//qDebug() << "DtChatChannel::SendBdDesign" << idDesign << "hasDesign" << hasDesign;
	if(hasDesign)
	{
		// Remember the design id immediately so we know it's on the way to the client, in case they request it again before
		// we're done sending it.
		m_rwBdDesigns.lockForWrite();
		m_BdDesigns.insert(pair<quint32, bool>(idDesign, true));
		m_rwBdDesigns.unlock();

		m_muTxBuffer.lock();

		// Design Erase precedes design in case client already has it.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_BdErase;
			out << design.Id();
			SendPacket(ba, false, false);
		}

		// Design rec is sent last so the client doesn't think it has the design until it has ALL of the design.
		// This is why it might ask a few times for the design before it's done receiving it.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_BdDesign;
			design.Tx(out);
			SendPacket(ba, true, false);
		}

		m_muTxBuffer.unlock();

		// Send xrefs.
		QSet<quint32> xrefs;
		design.ExternalReferences(xrefs);
		for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
			SendBdDesign(*xi);
	}
	else
	{
		//qWarning("%lu Nonexistent spacecraft design %d requested", (unsigned long)currentThreadId(), idDesign);
	}
}

void DtChatChannel::SendItDesign(unsigned int idDesign)
{
	AuItDesign design;
	const DgData& sdd = g_Au.LockItDesignsForRead();
	bool hasDesign = sdd.ItDesign(idDesign, design);	// Triggers load if it's not loaded.
	g_Au.UnlockItDesigns();
	//qDebug() << "DtChatChannel::SendItDesign" << idDesign << "hasDesign" << hasDesign;
	if(hasDesign)
	{
		// Remember the design id immediately so we know it's on the way to the client, in case they request it again before
		// we're done sending it.
		m_rwItDesigns.lockForWrite();
		m_ItDesigns.insert(pair<quint32, bool>(idDesign, true));
		m_rwItDesigns.unlock();

		m_muTxBuffer.lock();

		// Design Erase precedes design in case client already has it.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ItErase;
			out << design.Id();
			SendPacket(ba, false, false);
		}

		// Design rec is sent last so the client doesn't think it has the design until it has ALL of the design.
		// This is why it might ask a few times for the design before it's done receiving it.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ItDesign;
			design.Tx(out);
			SendPacket(ba, true, false);
		}

		m_muTxBuffer.unlock();

		// Send xrefs.
		QSet<quint32> xrefs;
		design.ExternalReferences(xrefs);
		for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
			SendItDesign(*xi);
	}
	else
	{
		//qWarning("%lu Nonexistent spacecraft design %d requested", (unsigned long)currentThreadId(), idDesign);
	}
}

void DtChatChannel::DesignExit()
{
	m_I->DesignExit();
	m_DesignStudioMode = DaDesignBase::InvalidMode;
}

void DtChatChannel::DesignOpen(quint32 designId, int designMode)
{
	if(!designId)
		return;

	if(m_DesignStudioMode != DaDesignBase::InvalidMode)
	{
		m_DesignStudioMode = m_I->DesignOpen(designId, designMode);
	}
	else if(AcctIsProgrammer(m_AccountType))
	{
		// Not in a design studio.
		DaPerson* person = m_I->LockAvatar();
		if(person)
		{
			// Programmer-level admin spawns ship into existence.
			QString resultMsg;
			DaWorld* planet = person->FindParentPlanetInEnv();
			DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
			if(llworld)
			{
				quint8 errcode;
				DaVessel* ship = llworld->AgentSpawnSpacecraft(person, designId, errcode);

				if(ship)
				{
					// When architect powers are specifically enabled, the ship is randomly named.
					if(person->HasArchitectPower())
					{
						QString randomName = g_Au.Dice().RandomShipName();
						ship->SetName(randomName);
						ship->SetModified();
						resultMsg = tr("Spacecraft created %1 named %2.").arg(ship->Id().GetID()).arg(ship->Name());
					}
					else
						resultMsg = tr("Spacecraft created %1 %2.").arg(ship->Id().GetID()).arg(ship->Name());
					g_AuDb.PostCriticalUpdates();	// Save this right away so it appears in the database.
				}
				else
				{
					switch(errcode)
					{
					case 1:
						resultMsg = tr("An unlikely error occurred.");
						break;
					case 2:
						resultMsg = tr("Library is loading the spacecraft blueprint. Try again shortly.");
						break;
					case 6:
						resultMsg = tr("Spawn location was blocked.");
						break;
					case 27:
						resultMsg = tr("Blueprint not valid for manufacture.");
						break;
					}
				}
			}
			else
				resultMsg = tr("I must move to a world to create spacecraft.");

			if(!resultMsg.isEmpty())
			{
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		m_I->UnlockAvatar();
	}
	else
	{
		qWarning() << "HACKER WARNING! DtChatChannel::DesignOpen" << m_I->AvatarId() << "not in designer." << designId;
		//Disconnect();
	}
}

void DtChatChannel::CheckLoginStatus()
{
	// The confirmation number is never zero after a player has successfully logged in to this server by sending a PL_Login packet.
	// However, the client connects to the master server to get empire data and then has lots of settings to configure
	// before they finally send us the PL_Login packet. During that time the confirmation number is still zero. We must not call
	// IsPlayerLoggedIn with a zero confirmation number or it will disconnect nu users while they are configuring their settings,
	// typically when setting up a nu empire because of the large number of options.
	if(!m_confirmNum)
		return;
	AuId pIdUnit[6];
	AuTime acctExpiry;
	if(!g_Au.IsPlayerLoggedIn(m_idPlayer, m_confirmNum, pIdUnit, m_AccountType, acctExpiry))
	{
		g_Au.LogConnect(auSceneServerApp->LoginName(), QString("CHAT: DISCONNECT! DtChatChannel::CheckLoginStatus %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName));
		Disconnect();	// This player is no longer logged in or the confirmation number is incorrect.
	}
}

void DtChatChannel::Disconnect(AuId idAtom)
{
	if(UnitId() == idAtom)
	{
		QString logmsg = QString("CHAT: Disconnect. Player %1 unit %2 %3")
			.arg(m_idPlayer).arg(m_I->AvatarId().GetID()).arg(m_UnitName);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		Disconnect();
	}
}

void DtChatChannel::Disconnect()
{
	// Signal the run loop to disconnect.
	m_Disconnect = true;
}

// Disconnect this channel if it is connected to 'playerId'.
void DtChatChannel::DisconnectPlayer(quint32 playerId, unsigned long exceptThreadId)
{
	if(m_idPlayer == playerId && exceptThreadId != (unsigned long)m_ThreadId)
	{
		QString logmsg = QString("CHAT: Disconnect. Player %1 except %2 thread %3")
			.arg(playerId).arg((unsigned long)exceptThreadId).arg((unsigned long)m_ThreadId);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		Disconnect();
	}
}

bool DtChatChannel::IsCommChannelOpen(const QString& channelName)
{
	m_muChannels.lock();
	bool isOpen = m_Channels.contains(channelName);
	m_muChannels.unlock();
	return isOpen;
}

bool DtChatChannel::OpenCommChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const QString& narrDesc)
{
	if(channelName.isEmpty())
		return false;
	m_muChannels.lock();
	if(!m_Channels.contains(channelName))
	{
		// We must add the channel to the list of open channels immediately rather
		// than wait for the client to respond to the open. Otherwise, it is
		// difficult for local atoms such as cities to open a channel and
		// immediately broadcast on it.
		m_Channels.append(channelName);
		m_muChannels.unlock();

		// The client will not respond with a PC_ChannelOpened message but that won't cause us any trouble.
		//qDebug("DtChatChannel::OpenCommChannel - Sending open packet for "
				//"channel %s.", (const char*)channelName.toAscii());
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_ChannelOpened;
		out << channelName;
		out << (quint8)scope;
		out << narrDesc;
		SendPacket(ba);
		return true;
	}
	else
	{
		m_muChannels.unlock();
		return false;
	}
}

void DtChatChannel::QueryJobs()
{
	QMultiMap<int, AuJobOffer> jobsToStart;						// Key is sector range so they are sent closest to farthest.
	m_I->QueryJobs(jobsToStart);

	// Send the info to the player. The map is sorted by sector range so they will get the close jobs before the far jobs.
	for(QMultiMap<int, AuJobOffer>::const_iterator si = jobsToStart.cbegin(); si != jobsToStart.cend(); si++)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PJ_Query;			// Reply to query from client with a job offer.
		out << *si;
		SendPacket(ba);
	}
}

void DtChatChannel::QueryJobs(const QList<AuJob::TYPE>& filterTypes, int maxSectorsAway)
{
	QMultiMap<int, AuJobOffer> jobsToStart;						// Key is sector range so they are sent closest to farthest.
	m_I->QueryJobs(jobsToStart, filterTypes, maxSectorsAway);

	// Send the info to the player. The map is sorted by sector range so they will get the close jobs before the far jobs.
	for(QMultiMap<int, AuJobOffer>::const_iterator si = jobsToStart.cbegin(); si != jobsToStart.cend(); si++)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PJ_Query;			// Reply to query from client with a job offer.
		out << *si;
		SendPacket(ba);
	}
}

void DtChatChannel::DesignStudioEnter(quint16 panelIndex, const QString& password)
{
	quint8 designStudioMode = m_I->DesignStudioEnter(panelIndex, password);
	if(designStudioMode != DaDesignBase::InvalidMode)
		EnteredDesigner(m_I->AvatarId(), designStudioMode);
}

void DtChatChannel::RegenDNA(int chgWhat, int floraZone)
{
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		if(chgWhat == 5)
		{
			// Clear rat dna on current ship.
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				ship->ClearRatSpecies();
				ship->SetModified();

				// Report the result as a log message.
				QString resultMsg = tr("Rat DNA of %1 cleared.").arg(ship->NameWithEmpire());
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		else
		{
			DaWorld* planet = person->FindParentPlanetInEnv();
			if(planet)
			{
				if(chgWhat >= 0 && chgWhat <= 2)
				{
					QString resultMsg;
					DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
					if(llworld)
					{
						int resourceZone = llworld->ResourceZone(person->GlobeLL().Longitude());
						llworld->RegenPlantDNA(resourceZone, chgWhat, floraZone);

						// Report the result as a log message.
						if(floraZone == 0)
							resultMsg = tr("Polar plant DNA of %1 zone %2 regenerated.").arg(planet->Name())
								.arg(resourceZone+1);
						else if(floraZone == 1)
							resultMsg = tr("Temperate plant DNA of %1 zone %2 regenerated.").arg(planet->Name())
								.arg(resourceZone+1);
						else
							resultMsg = tr("Tropical plant DNA of %1 zone %2 regenerated.").arg(planet->Name())
								.arg(resourceZone+1);
					}
					else
						resultMsg = tr("Unable to regen plant DNA of %1.").arg(planet->Name());
					AuCommThought msg(resultMsg, person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(chgWhat == 3)
				{
					QString resultMsg;
					DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
					if(llworld)
					{
						// Change indigenous DNA in scene atom's current zone to match the scene atom.
						llworld->SetWorkerSpecies(person->Species());
						planet->SetModified();

						// Report the result as a log message.
						QString specieName = person->SpeciesName();
						resultMsg = tr("Indigenous DNA of %1 changed to %3.").arg(planet->Name())
							.arg(specieName);
					}
					else
						resultMsg = tr("Unable to change indigenous DNA of %1.").arg(planet->Name());
					AuCommThought msg(resultMsg, person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(chgWhat == 4)
				{
					QString resultMsg;
					DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
					if(llworld)
					{
						// Clear the indigenous DNA in the scene atom's current zone.
						AuSpecies species;	// Construct default species.
						llworld->SetWorkerSpecies(species);
						planet->SetModified();

						// Report the result as a log message.
						resultMsg = tr("Indigenous DNA of %1 cleared.").arg(planet->Name());
					}
					else
						resultMsg = tr("Unable to clear indigenous DNA of %1.").arg(planet->Name());
					AuCommThought msg(resultMsg, person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(chgWhat == 6)
				{
					// Change citizen DNA in scene atom's current city to match the scene atom.
					if(!person->BdId().IsNull())
					{
						DaBd* bd = planet->FindBd(person->BdId());
						if(bd)
						{
							bd->SetResidentSpecies(person->Species());
							bd->SetModified();

							// Report the result as a log message.
							QString specieName = person->SpeciesName();
							QString resultMsg = tr("Resident DNA of %1 changed to %2.").arg(bd->Name()).arg(specieName);
							AuCommThought msg(resultMsg, person->Id(), person->Name());
							g_Au.SendAtomChat(msg);
						}
					}
				}
			}
		}
	}
	m_I->UnlockAvatar();
}

void DtChatChannel::MissionSend(QTcpSocket& tcpSocket)
{
	// Reply with the mission packet.
	bool okToSend = false;
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PN_Mission;
	out << (quint8)0;	// mission

	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		// Found the scene atom and it's a person.
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && (ship->IsCrewOrTroop(person->Id()) || ship->InChainOfCommand(person)))
		{
			// The person is a crew member of the spacecraft they are aboard.
			okToSend = true;
			ship->Id() >> out;			// For positive association of the mission to the appropriate ship.
			const AuMission& m = ship->LockMission();
			m >> out;
			ship->UnlockMission();

			// Must call SetModified on the ship otherwise the person who queried for this mission might not get it.
			// The mission gets stored on an intermediate copy of the ship atom on the client. The mission data does not
			// propagate to the view's version of the atom through Refresh until the atom has changed. This probably should
			// be addressed on the client but, then again, this solves the problem at the expense of one extra frame of ship
			// data when it otherwise might be silent.
			ship->SetModified(DaAtom::NOTIFY_Scene);
		}
	}
	m_I->UnlockAvatar();

	if(okToSend)
	{
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
	}
}

void DtChatChannel::MissionSendSensorContacts(QTcpSocket& tcpSocket)
{
	AuSensorBlipList sensorBlips;
	AuPoint sensorEnvLoc;
	double sensorRange = m_I->SensorContacts(sensorBlips, sensorEnvLoc);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PN_Mission;
	out << (quint8)1;	// sensor contacts
	sensorBlips.TxRangeLimited(out, sensorEnvLoc, sensorRange);

	if(sensorRange > 0)
	{
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
	}
}

// Process a chat packet received from the client on this connection.
// Send it to the local application if appropriate.
void DtChatChannel::ChatReceivedFromClient(AuCommPacket& msg)
{
	//qDebug() << "DtChatChannel::ChatReceivedFromClient" << msg.Type() << msg.Text();
	if((AcctIsTrial(m_AccountType) || AcctIsExpired(m_AccountType)) && msg.Type() == AuCommPacket::OfferAvatar)
	{
		// Trial account is not allowed to transfer their avatar anywhere.
		AuCommThought msg(tr("I must activate my account before I can give away an avatar."), m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
		return;
	}

	// Verify that the sender has the authority to transmit on the channel.
	bool okToSend = true;
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		// Scope determines what is required to send most messages.
		switch(msg.Scope())
		{
		case AuCommStdChannel::Agent:
			// Channel is restricted to agents, programmers and architects only.
			okToSend = AcctIsAgent(m_AccountType);
			break;
		case AuCommStdChannel::Programmer:
			// Channel is restricted to programmers and architects only.
			okToSend = AcctIsProgrammer(m_AccountType);
			break;
		case AuCommStdChannel::Architect:
			// Channel is restricted to architects only.
			okToSend = AcctIsArchitect(m_AccountType);
			break;
		case AuCommStdChannel::CrewMates:
			// Sender must be on a crew.
			if(person->SpawnShip().IsNull() || person->SpawnShip() != msg.FromShipId())
				okToSend = false;	// person is not a crew member or they're spoofing the ship id in the message
			else if(person->InArena() && msg.Type() == AuCommPacket::HailRequest &&
					msg.TransmissionRange() > AuCommStdChannel::SolarSystem)
				okToSend = false;	// Cheesy way to prevent hail on crew channel from arena from summoning private dead ship.
			break;
		case AuCommStdChannel::Intercom:
			// Sender must be aboard a spacecraft.
			if(!person->IsInsideSpacecraft())
				okToSend = false;	// person is not inside a spacecraft
			break;
		case AuCommStdChannel::Fleet:
		case AuCommStdChannel::Squadron:
			// Sender must be a government official or they need to be a ship's officer.
			if(!AuRosterEntry::IsOfficer(person->SpawnShipBerthType()))
			{
				// Person is not a ship's officer or captain.
				const DgData& gd = g_Au.LockDataForRead();
				if(!gd.EmpireCitizenCanReceiveFleetMessages(person->EmpireId(), person->Id()))
					okToSend = false;
				g_Au.UnlockData();
			}
			break;

		case AuCommStdChannel::Company:
			// Anyone can send on the Company channel. Their id will be the owner id in the message or the owner id of the ship
			// they command, if they are an officer of a private ship and that ship is found in the environment.
			// Recipients must be officers of ships with that owner, or the owner.
			break;

		case AuCommStdChannel::Diplomacy:
			if(msg.ToEmpireId())
			{
				// City surrender messages are targeted at a particular empire on the diplomacy channel.
				if(person->EmpireId() != msg.FromEmpireId())
					okToSend = false;	// Person is not from the empire identified in the message.
			}
			else
			{
				const DgData& gd = g_Au.LockDataForRead();
				if(!gd.EmpireCitizenCanUseDiplomacyChannel(person->EmpireId(), person->Id()))
					okToSend = false;
				g_Au.UnlockData();
			}
			break;

		case AuCommStdChannel::Empire:
		case AuCommStdChannel::Friends:
			// Sender must be in the empire identified in the message.
			if(person->EmpireId() != msg.FromEmpireId())
				okToSend = false;	// person is not from the empire id in the message
			break;

		case AuCommStdChannel::Government:
			// Sender must be in the empire identified in the message.
			if(person->EmpireId() != msg.FromEmpireId())
				okToSend = false;	// person is not from the empire id in the message
			else
			{
				const DgData& gd = g_Au.LockDataForRead();
				if(!gd.IsEmpireGovernor(person->EmpireId(), person->Id()))
					okToSend = false;
				g_Au.UnlockData();
			}
			break;

		case AuCommStdChannel::Narrator:
			// Sender must be in the story instance identified in the message.
			if(!person->IsInStoryInstance(msg.StoryInstance()))
				okToSend = false;	// person is not in the story instance associated with the message
			break;

		case AuCommStdChannel::Audible:
		case AuCommStdChannel::Thoughts:
		case AuCommStdChannel::Help:
			break;

		case AuCommStdChannel::Everyone:
			// Sender must be a government official on the Diplomacy channel.
			//DHH Why is this test here? Aren't all messages on the Diplomacy channel in Diplomacy scope?
			if(msg.ChannelName() == AuCommStdChannel::DiplomacyChannelName())
			{
				const DgData& gd = g_Au.LockDataForRead();
				if(!gd.IsEmpireGovernor(person->EmpireId(), person->Id()))
					okToSend = false;	// person is not a member of the government
				g_Au.UnlockData();
			}
			break;

		case AuCommStdChannel::ScopeTypes:
			// msg.Scope() never returns this.
			break;
		}

		switch(msg.Type())
		{
		case AuCommPacket::AcceptBerthCaptain:
		case AuCommPacket::AcceptBerthCaptainSlot:
		case AuCommPacket::AcceptBerthCrew:
		case AuCommPacket::AcceptBerthOfficer:
		case AuCommPacket::AcceptBerthPassenger:
		case AuCommPacket::AcceptBerthTroop:
			// Person cannot accept berths while in an arena.
		case AuCommPacket::InviteTeleport:
		case AuCommPacket::AcceptTeleport:
			// Person cannot invite or accept teleport while in an arena.
			if(person->InArena())
				okToSend = false;
			break;

		case AuCommPacket::InviteToEmpire:
			{
				const DgData& gd = g_Au.LockDataForRead();
				if(!gd.EmpireCitizenCanInviteCitizens(person->EmpireId(), person->Id()))
					okToSend = false;						// Person does not have the authority to transmit the message.
				g_Au.UnlockData();
			}
			break;

		default:
			break;
		}

		// Entity and animal agent bits cause the chat message to become anonymous.
		if(person->m_AgentBits & (AGENT_Entity|AGENT_Animal))
			msg.SetAnonymous(true);
	}
	else
		okToSend = false;

	if(okToSend)
		person->PoseMouthForSaying(msg, 0, m_Emojis);
	m_I->UnlockAvatar();
	if(!okToSend)
		return;

	// Bundle the message into a byte array so it can be emitted as a signal.
	QByteArray bamsg;
	QDataStream out(&bamsg, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)msg.Type();
	out << msg;
	emit ChatReceived(bamsg);

	if(g_Au.m_LogMessages & LOG_Chat)
	{
		// Log chat messages to their own log file.
		QString logmsg;
		logmsg = tr("From Player %1 Unit %8 %9 Channel %2 Range %3 Scope %4 Data %5 Type %6: %7")
				.arg(m_idPlayer)
				.arg(msg.ChannelName())
				.arg(AuCommStdChannel::RangeName(msg.TransmissionRange()))
				.arg(AuCommStdChannel::ScopeName(msg.Scope()))
				.arg(msg.ScopeData())
				.arg(msg.TypeName())
				.arg(msg.Text())
				.arg(msg.FromId().GetID())
				.arg(msg.FromName());
		QString chatLogFile = QString("chat.%1.log").arg(auSceneServerApp->LoginName());
		g_Au.Log(chatLogFile, logmsg);
	}
}

void DtChatChannel::SetPersonSystem(AuId systemId, quint32 playerId)
{
	if(m_idPlayer == playerId)
		m_I->SetSystemId(systemId);
}

// Process a chat packet received from the local app.
// Send it to the client on this connection if appropriate.
void DtChatChannel::ChatToClients(QByteArray bamsg)
{
	if(UnitId().IsNull())
		return;

	// Extract the packet from the byte array.
	QDataStream in(bamsg);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 msgType;
	in >> msgType;
	AuCommPacket* msg = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
	if(!msg)
	{
		qWarning() << "DtChatChannel::ChatToClients ERROR unpacking type" << msgType << "bamsg" << bamsg.count() << "bytes";
		return;
	}
	in >> *msg;

	ChatToClient(msg);

	delete msg;
}

// Process a chat packet received from the local app.
// Send it to the client on this connection if appropriate.
void DtChatChannel::ChatToClient(AuCommPacket* msg)
{
	//qDebug() << "DtChatChannel::ChatToClients" << msg->Type() << "from" << msg->FromId() << msg->FromName()
		//<< "range" << msg->TransmissionRange() << msg->Text();
	if(msg->Text() == "")
		return;	// Could be AuCommContactShip message, which is not visible to players.

	// Skip this client if the message has a specific recipient and this client is not the recipient or the sender.
	AuId toId = msg->ToId();									// Some messages are addressed to a specific recipient.
	if(!toId.IsNull() && toId != UnitId() && msg->FromId() != UnitId())
		return;

	// Check to see if the sender is in the recipient's squelch list.
	if(m_SquelchIds.find(msg->FromId()) != m_SquelchIds.cend())
		return;

	AuCommStdChannel::RANGE senderActualRange = msg->TransmissionRange();
	const AuCommStdChannel::SCOPE scope = msg->Scope();
	if(!msg->IsThought())
	{
		// Determine the actual range of the sender from me.
		// Log messages always appear to come from close range, which is the hard-coded range for the Log message type.
		// Pick up info about the sender while we're at it, for scope test.
		quint32 empireId = 0;
		bool aboardSame = false;
		bool crewOk = false;
		bool diplomatOk = false;
		bool empireOk = false;
		bool fleetOk = false;
		bool companyOk = false;
		bool friendOk = false;
		bool inStory = false;

		DaPerson* pto = m_I->LockAvatar();
		DaSolarSystem* system = pto ? pto->FindParentSolarSystem() : 0;
		if(pto && system)
		{
			// Found ME!
			empireId = pto->EmpireId();

			// Check for crew membership.
			crewOk = pto->SpawnShip() == msg->FromShipId();

			// Check for common empire.
			empireOk = pto->EmpireId() == msg->FromEmpireId();

			// Check for recipient being a diplomat.
			quint32 toEmpireId = msg->ToEmpireId();
			if(toEmpireId)
			{
				// City surrender messages are targeted at a particular empire on the diplomacy channel.
				if(empireId == toEmpireId || empireId == msg->FromEmpireId())
				{
					const DgData& gd = g_Au.LockDataForRead();
					diplomatOk = gd.EmpireCitizenCanAcceptCitySurrender(empireId, pto->Id());
					g_Au.UnlockData();
				}
			}
			else
			{
				const DgData& gd = g_Au.LockDataForRead();
				diplomatOk = gd.IsEmpireGovernor(empireId, pto->Id());
				g_Au.UnlockData();
			}

			// Check for recipient being eligible for fleet messages.
			// Fleet scope requires the recipient to be a spacecraft officer that is in the same empire as the sender.
			// The recipient may also be a government offical higher than the rank of mayor.
			if(pto->EmpireId() == msg->FromEmpireId())
			{
				if(AuRosterEntry::IsOfficer(pto->SpawnShipBerthType()))
					fleetOk = true;
				else
				{
					const DgData& gd = g_Au.LockDataForRead();
					fleetOk = gd.EmpireCitizenCanReceiveFleetMessages(empireId, pto->Id());
					g_Au.UnlockData();
				}
			}

			// Check for recipient being eligible for company messages.
			if(pto->Is(msg->FromOwnerId()))
				companyOk = true;			// Recipient is the owner.
			else if(pto->SpawnShipIsPrivate() && AuRosterEntry::IsOfficer(pto->SpawnShipBerthType()))
			{
				// Officer of a private ship. We must find the ship in the system.
				const DaAtom* envAtom = pto->FindEnvironmentAtom();
				const DaVessel* ship = dynamic_cast<const DaVessel*>(envAtom->FindIdRecursive(pto->SpawnShip()));
				if(ship && ship->OwnerId() == msg->FromOwnerId())
					companyOk = true;
			}

			// Check for recipent being eligible for friend messages.
			POLITICAL_STANCE stanceTo = PoliticalStance(msg->FromEmpireId(), pto->EmpireId());
			POLITICAL_STANCE stanceFrom = PoliticalStance(pto->EmpireId(), msg->FromEmpireId());
			if((stanceTo == POL_Friend || stanceTo == POL_Empire || stanceTo == POL_Vassal) &&
					(stanceFrom == POL_Friend || stanceFrom == POL_Empire || stanceFrom == POL_Vassal))
				friendOk = true;

			// Check to see if the person is in the story.
			// Everyone is considered to be in story instance 1. It is used for conversations with AI citizens. Those messages
			// have a ToId so the recipient is filtered on that.
			inStory = pto->IsInStoryInstance(msg->StoryInstance()) || msg->StoryInstance() == 1; 

			const DaSector* mySector = pto->FindParentSector();
			if(mySector)
			{
				// Found my sector.
				if(msg->FromId() == UnitId())
				{
					// Sender always receives their own messages at close range.
					senderActualRange = AuCommStdChannel::Close;
					if(pto->IsAboardVehicle() || pto->IsInsideSpacecraft())
						aboardSame = true;					// So intercom channel will work.
				}
				else if(mySector->Is(msg->FromSectorId()))
				{
					// The message originated in my sector.
					const DaAtom* pfrom = system->FindIdRecursive(msg->FromId());
					if(pfrom)
					{
						// Found the sender of the message in my solar system.
						bool calcActualDistance = false;
						if(dynamic_cast<const DaArena*>(pfrom))
						{
							// Message is from an arena. They don't communicate outside of themselves.
							// Client does not receive this message if they are not in the arena.
							if(pto->FindParentArena() == pfrom)
								senderActualRange = AuCommStdChannel::Hail;		// Always at hail range from the arena itself.
							else
								senderActualRange = AuCommStdChannel::InvalidRange;	// Not in the arena.
						}
						else
						{
							// Sender is not a city so it must be a unit. No other atoms send chat.
							// Sender has queried their environment.
							if(pto->IsAboardVehicle() && pfrom->IsAboardVehicle() && pto->Parent() == pfrom->Parent())
							{
								// Aboard same vehicle as sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else if(pto->IsAboardVehicle() && dynamic_cast<const DaVehicle*>(pfrom) &&
									pto->Parent() == pfrom)
							{
								// Aboard vehicle that is the sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else if(pto->IsInsideCapsule() && pfrom->IsInsideCapsule() &&
									pto->FindParentCapsuleInEnv() == pfrom->FindParentCapsuleInEnv())
							{
								// Aboard same capsule as sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else if(pto->IsInsideCapsule() && dynamic_cast<const DaCapsule*>(pfrom) &&
									pto->FindParentCapsuleInEnv() == pfrom)
							{
								// Aboard capsule that is the sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else
								calcActualDistance = true;
						}

						if(scope == AuCommStdChannel::Audible)
						{
							// Calculate precise distance between the sender and the client's unit.
							AuPoint fromloc = pfrom->EnvLocation();
							AuPoint toloc = pto->EnvLocation();
							double dist = fromloc.Distance(toloc);

							// Audible message range is limited by the listener's DNA, not regular close range.
							float hearingRange = pto->HearingRange();
							if(dist < hearingRange)
							{
								// Test audibility between sender and receiver, modulated over the audible distance.
								// SetSoundHeard multiplies aud by 20 to get an integer result, so less than .05 is pointless.
								float aud = pfrom->RelativeAudioGain(pto) * (1. - dist / hearingRange);
								if(aud >= .05f)
								{
									// Set sound heard so the listener gets a visual indication of direction and audibility.
									AuPoint soundDir = pto->EnvTransform().UnProject(fromloc);	// Relative to pto
									soundDir.Unit();
									pto->SetSoundHeard(soundDir, aud);

									// Louder than 20% is intelligible; quieter gets the audibility indicator but not the text.
									if(aud > .2f)
										senderActualRange = AuCommStdChannel::Close;		// Parties are within close range.
									else
										senderActualRange = AuCommStdChannel::InvalidRange;	// Cannot make out what I said.
								}
								else
									senderActualRange = AuCommStdChannel::InvalidRange;	// Cannot hear me at all.
							}
							else
								senderActualRange = AuCommStdChannel::InvalidRange;	// Cannot hear me.
						}
						else if(calcActualDistance)
						{
							// Calculate precise distance between the sender and the client's unit.
							AuPoint fromloc = pfrom->EnvLocation();
							AuPoint toloc = pto->EnvLocation();
							double dist = fromloc.Distance(toloc);
							if(AuCommStdChannel::IsWithinRange(dist, AuCommStdChannel::Close))
								senderActualRange = AuCommStdChannel::Close;		// Parties are within close range.
							else if(AuCommStdChannel::IsWithinRange(dist, AuCommStdChannel::Hail))
								senderActualRange = AuCommStdChannel::Hail;			// Parties are within hail range.
							else
								senderActualRange = AuCommStdChannel::SolarSystem;	// Parties are within system range.
						}
					}
					else if(msg->Type() == AuCommPacket::CitySurrenderAccepted)
					{
						// Sender of the surrender accepted message could be a pirate emperor who does not really exist.
						// This could result in a person in the wrong system receiving this response but it's ok if they are
						// otherwise eligible to receive the message.
						if(DgEmpire::IsPirate(msg->FromEmpireId()))
							senderActualRange = AuCommStdChannel::SolarSystem;	// Message originated in this solar system.
						else
							senderActualRange = AuCommStdChannel::Sector;	// Message originated in a different solar system.
					}
					else
						senderActualRange = AuCommStdChannel::Sector;	// Message originated in a different solar system.
				}
				else
				{
					// The message originated in a different sector.
					const DaGalaxy* galaxy = mySector->FindParentGalaxy();
					if(!galaxy)
						senderActualRange = AuCommStdChannel::InvalidRange;		// Could not find my galaxy?
					else if(galaxy->Galaxy() != msg->FromGalaxy())
						senderActualRange = AuCommStdChannel::Universe;		// Message originated in a distant galaxy.
					else
					{
						const DaSector* fromSector =
							dynamic_cast<const DaSector*>(galaxy->FindNormalSubatomId(msg->FromSectorId()));
						if(fromSector && fromSector->Parent() == mySector->Parent())
						{
							// Both sectors are in the same galaxy.
							// Check distance from my sector to the sector that originated the message.
							// An adjacent sector will be 10 parsecs away; I'll check 11 in case of fuzzy errors.
							AuPoint vecToOrig = mySector->Location() - fromSector->Location();
							if(fabs(vecToOrig.x) < 11 && fabs(vecToOrig.y) < 11 && fabs(vecToOrig.z) < 11)
								senderActualRange = AuCommStdChannel::Sector;	// Message originated within a sector.
							else
								senderActualRange = AuCommStdChannel::Galaxy;	// Message originated in a distant sector.
						}
						else
							senderActualRange = AuCommStdChannel::Galaxy;		// Message originated in a distant sector.
					}
				}
			}
			else
				senderActualRange = AuCommStdChannel::InvalidRange;		// Could not find my sector?
		}
		else
			senderActualRange = AuCommStdChannel::InvalidRange;				// Could not find me or I haven't queried env yet.
		m_I->UnlockAvatar();
		if(senderActualRange == AuCommStdChannel::InvalidRange)
			return;			// Sender range could not be determined, or I am ignoring them, or I cannot hear them.

		// Messages with an associated story instance are not received by anyone else.
		// These messages can arrive on any channel, regardless of scope.
		if(!inStory && msg->StoryInstance())
			return;

		// Test scope of message against this recipient.
		switch(scope)
		{
		case AuCommStdChannel::Agent:
			if(!AcctIsAgent(m_AccountType))
				return;											// Agent scope requires agent, programmer or architect.
			break;
		case AuCommStdChannel::Programmer:
			if(!AcctIsProgrammer(m_AccountType))
				return;											// Programmer scope requires programmer or architect.
			break;
		case AuCommStdChannel::Architect:
			if(!AcctIsArchitect(m_AccountType))
				return;											// Architect scope requires architect.
			break;
		case AuCommStdChannel::Fleet:
		case AuCommStdChannel::Squadron:
			if(!fleetOk)
				return;
			break;

		case AuCommStdChannel::Company:
			if(!companyOk)
				return;
			break;

		case AuCommStdChannel::CrewMates:						// Crew channel requires crew fellowship
			if(!crewOk)
				return;
			break;
		case AuCommStdChannel::Diplomacy:						// Diplomacy requires the person to hold some office.
			if(!diplomatOk)
				return;											// Person is not a member of any government.
			break;
		case AuCommStdChannel::Empire:							// Empire channel requires same empire.
			if(!empireOk)
				return;											// Empire allegiance did not match exactly.
			break;
		case AuCommStdChannel::Government:						// Government requires person to hold office and be in same empire.
			if(!empireOk || !diplomatOk)
				return;
			break;
		case AuCommStdChannel::Audible:							// Already tested audibility.
		case AuCommStdChannel::Everyone:						// Everyone scope is limited only by range
		case AuCommStdChannel::Narrator:						// Narrative requires same story instance. Already checked.
		case AuCommStdChannel::Thoughts:
		case AuCommStdChannel::Help:
			break;
		case AuCommStdChannel::Friends:							// Friend channel requires same empire or friendly empire.
			if(!friendOk)
				return;
			break;
		case AuCommStdChannel::Intercom:						// Intercom channel requires same parent ship.
			if(!aboardSame)
				return;											// Scene client is not aboard the same ship or vehicle.
			break;
		case AuCommStdChannel::ScopeTypes:
			// msg->Scope() never returns this.
			break;
		}

		// Determine if client is capable of receiving the message based on range.
		// The Intercom channel has already checked based on common parent ship or vehicle only.
		// Log messages are sent directly to an atom id, which has already been checked. Range is irrelevant.
		// Narrative messages are received based on participation in the message's story instance. Range is irrelevant.
		if(scope != AuCommStdChannel::Intercom && !msg->IsNarratorMessage() && senderActualRange > msg->TransmissionRange())
			return;												// Sender range is beyond the transmission range of the message.
	}

	// Now test to see if the client is monitoring the channel.
	if(!IsCommChannelOpen(msg->ChannelName()))
	{
		// The client does not have the channel open.
		if(scope == AuCommStdChannel::Agent || scope == AuCommStdChannel::Help)
		{
			// Agent channels and the Help channel are only opened automatically for agents, programmers and architects.
			if(AcctIsAgent(m_AccountType))
				OpenCommChannel(msg->ChannelName(), scope, "");	// Open the channel for the agent, programmer or architect.
			else
				return;											// The channel does not open automatically. Skip the client.
		}
		else if(scope == AuCommStdChannel::Programmer)
		{
			// Programmer channel are only opened automatically for programmers and architects.
			if(AcctIsProgrammer(m_AccountType))
				OpenCommChannel(msg->ChannelName(), scope, "");	// Open the channel for the programmer or architect.
			else
				return;											// The channel does not open automatically. Skip the client.
		}
		else if(scope == AuCommStdChannel::Architect)
		{
			// Architect channel are only opened automatically for architects.
			if(AcctIsArchitect(m_AccountType))
				OpenCommChannel(msg->ChannelName(), scope, "");	// Open the channel for the architect.
			else
				return;											// The channel does not open automatically. Skip the client.
		}
		else if(AuCommStdChannel::IsOpenedAutomatically(msg->ChannelName(), senderActualRange, AcctIsMinion(m_AccountType)))
		{
			// We can open the channel automatically for them.
			// Automatically opened channels get the default scope for that channel.
			// Only standard channels automatically open.
			OpenCommChannel(msg->ChannelName(), AuCommStdChannel::Scope(msg->ChannelName()), "");
		}
		else if(scope == AuCommStdChannel::Narrator)
		{
			// We can open the channel automatically for them.
			// Channels opened under due to narration are identified as narrator scope channels.
			OpenCommChannel(msg->ChannelName(), scope, g_Au.StoryBrief(msg->StoryInstance()));
		}
		else
			return;												// The channel does not open automatically. Skip the client.
	}

	// Send specie information associated with message items, before the message.
	if(msg->Item())
		SendSpeciesRecordIfNeeded(msg->Item()->DNASpecies());
	if(msg->OfferItem())
		SendSpeciesRecordIfNeeded(msg->OfferItem()->DNASpecies());

	// Set the reception range in the message with the actual range that we determined.
	// That helps the client show the message according to actual range. The transmission range is irrelevant to the client.
	// Announcements always appear to come from Close range so they get a big font.
	// Silverwill messages always appear to come from Universe range so they get a tiny font.
	if(msg->Type() == AuCommPacket::Announcement)
		msg->SetReceptionRange(AuCommStdChannel::Close);
	else if(msg->Type() == AuCommPacket::Silverwill)
		msg->SetReceptionRange(AuCommStdChannel::Universe);
	else
		msg->SetReceptionRange(senderActualRange);

	// At this point, anonymous messages must hide the sender's actual identity. This copy will be sent to the client who could
	// discover that information.
	if(msg->IsSenderAnonymous())
		msg->SetFromId(AuId(1));

	// Create an encryption key based on this user's login credentials.
	quint16 wKey = AuUtil::NetCryptKey((quint16)m_confirmNum);

	// If we made it this far, send the chat message to the scene client.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PC_Chat;
	msg->TxEncrypt(out, wKey);
	SendPacket(ba);
}

void DtChatChannel::OpenChannelToUnit(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId,
		QString channelName, quint8 scope)
{
	if(UnitId() == toUnitId)
	{
		// Check my squelch list. Don't let squelched people open channels to me.
		if(m_SquelchIds.find(fromUnitId) == m_SquelchIds.cend())
		{
			if(OpenCommChannel(channelName, (AuCommStdChannel::SCOPE)scope, ""))
			{
				// Announce who opened the channel, on that channel.
				AuCommChannelOpened msg(m_I->AvatarId(), m_UnitName, channelName, fromUnitId, fromName, fromSector, (GALAXY)fromGalaxy,
						AuCommStdChannel::Universe, (AuCommStdChannel::SCOPE)scope);
				ChatToClient(&msg);
			}
		}
	}
}

// 0=domestic animals, 1=wild animals
void DtChatChannel::KillAnimals(quint8 killWhat)
{
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		DaAtomList sceneAtoms;
		person->ListSceneAtoms(sceneAtoms);
		for(DaAtomList::iterator sai = sceneAtoms.begin(); sai != sceneAtoms.end(); sai++)
		{
			DaAnimal* animal = dynamic_cast<DaAnimal*>(*sai);
			if(!animal)
				continue;
			if(animal->IsMounted())
				continue;	// Skip mounted animals.
			switch(killWhat)
			{
			case 0:			// kill domestic animals
				if(!animal->IsWild())
					animal->Kill();
				break;
			case 1:			// kill wild animals
				if(animal->IsWild())
					animal->Kill();
				break;
			}
		}
	}
	m_I->UnlockAvatar();
}

void DtChatChannel::SendSpeciesRecordIfNeeded(const AuSpecies& species)
{
	// Assume the default constructed DNA is not valid. It's definitely not likely.
	if(species.IsNull())
		return;

	// We only do this if the DNA is not already in our cache list, which indicates they already have the DNA.
	bool sendIt = false;
	m_muSpecies.lock();
	if(m_Species.find(species) == m_Species.end())
	{
		// Add the dna to our cache list so SendSpeciesRecord knows the client cares about it.
		m_Species.insert(pair<AuSpecies, bool>(species, true));
		sendIt = true;
	}
	m_muSpecies.unlock();

	// Send the species to the client.
	if(sendIt)
		SendSpeciesRecord(species);
}

// Sends a species record to the client if it is in their current cache.
void DtChatChannel::SendSpeciesRecord(AuSpecies species)
{
	//qDebug() << "DtChatChannel::SendSpeciesRecord" << species;
	m_muSpecies.lock();
	if(m_Species.find(species) != m_Species.end())
	{
		// The client has the species in their cache so they care about this species.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_Specie;

		const DgData& gd = g_Au.LockDataForRead();
		DzSpeciesHash::const_iterator si = gd.m_Species.find(species);
		if(si != gd.m_Species.cend())
		{
			// The species has been named. Send the species record.
			(*si).second >> out;
		}
		else
		{
			// The species has not been named. Send an empty record.
			DzSpecies nonameSpecies(species, "", AuId(0));
			nonameSpecies >> out;
		}
		g_Au.UnlockData();

		SendPacket(ba);
	}
	m_muSpecies.unlock();
}

// Agent Assign Emperor:
//   'empireId' is empire to change.
//   'toWho' is nu emperor
void DtChatChannel::AgentAssignEmperor(quint32 empireId, const AuId& toWho, const QString& toName)
{
	AuOffice office;
	office.SetupEmperor();
	emit OfficeAssigned(toWho, empireId, office);

	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, empireName);
	g_Au.UnlockData();

	// Send the result on the empire channel.
	QString resultMsg = tr("%1 designated as emperor of %2.").arg(toName).arg(empireName);
	AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), m_I->AvatarId(), m_UnitName, UnitSector(), UnitGalaxy(),
			AuCommStdChannel::Universe, AuCommStdChannel::Empire);
	empAnnounce.SetScopeData(empireId);
	g_Au.SendAtomChat(empAnnounce);

	// Notify agent since they may not be in that empire and won't get that message.
	AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
	g_Au.SendAtomChat(msg);
}

// This function arrives from a signal emitted by an atom when it wants to remind everyone in a game whose turn it is.
// The signal goes to all chat channels. I have to decide if it's going to my scene atom.
void DtChatChannel::GameTurn(AuId shipId, quint16 whichTable, AuId personsTurn)
{
	// Games not permitted inside design studio.
	if(m_DesignStudioMode != DaDesignBase::InvalidMode)
		return;

	bool notifyGameTurn = false;
	DaPerson* person = m_I->LockAvatar();
	if(person && person->IsGameTablePose() && person->ParentId() == shipId && person->StationId() == whichTable)
		notifyGameTurn = true;	// My guy is at the game table in question.
	m_I->UnlockAvatar();

	if(notifyGameTurn)
	{
		// Send a packet to the person, informing them of whose turn it is. It may not be their turn.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PT_GameTurn;
		out << personsTurn;
		SendPacket(ba);
	}
}

// This function arrives from a signal emitted by an atom when a game has terminated. The game no longer exists. All information
// needed to inform the players of the result is in the gameResult.
// The signal goes to all chat channels. I have to decide if it's going to my scene atom.
void DtChatChannel::GameOver(AuId shipId, quint16 whichTable, quint8 gameType, QByteArray gameResult)
{
	// Games not permitted inside design studio.
	if(m_DesignStudioMode != DaDesignBase::InvalidMode)
		return;

	bool notifyGameOver = false;
	DaPerson* person = m_I->LockAvatar();
	if(person && person->IsGameTablePose() && person->ParentId() == shipId && person->StationId() == whichTable)
		notifyGameOver = true;	// My guy is at the game table in question.
	m_I->UnlockAvatar();

	if(notifyGameOver)
	{
		// Send a packet to the person, informing them of whose turn it is. It may not be their turn.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PT_GameOver;
		out << gameType;
		out << gameResult;
		SendPacket(ba);
	}
}

void DtChatChannel::AgentMakeCommodity(int makeWhat, COMMODITYID cid, quint8 quality, quint32 count, GALAXY destGalaxy,
		const AuPoint& destGalaxyLoc)
{
	DaPerson* person = m_I->LockAvatar();
	if(person)
		person->AgentMakeCommodity(makeWhat, cid, quality, count, destGalaxy, destGalaxyLoc);
	m_I->UnlockAvatar();
}

void DtChatChannel::AgentRefuel(quint8 quality)
{
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		// Refuel the person's parent vehicle or spacecraft.
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
		if(vehicle)
		{
			// Refuel the vehicle.
			AuItem* fuel = AuItem::Create(vehicle->FuelType(), quality, 0);
			if(fuel)
			{
				vehicle->Refuel(*fuel);
				delete fuel;
			}

			QString resultMsg = tr("%1 refueled with Q%2 %3.").arg(vehicle->Name())
				.arg(quality)
				.arg(g_Au.CommodityName(vehicle->FuelType()));
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
		else if(person->IsInSpacecraft())
		{
			// Refuel the spacecraft.
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				ship->Refuel(quality);
				ship->SetModified();

				QString resultMsg = tr("%1 refueled with Q%2 Hydrogen.").arg(ship->NameWithEmpire()).arg(quality);
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		else
		{
			QString resultMsg = tr("Nothing refueled.");
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	m_I->UnlockAvatar();
}

void DtChatChannel::AgentRepair()
{
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		// Refuel the person's parent vehicle or spacecraft.
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
		if(vehicle)
		{
			// Repair the vehicle.
			vehicle->ResetHitPoints();
			vehicle->SetModified();

			QString resultMsg = tr("%1 repaired.").arg(vehicle->Name());
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
		else if(person->IsInSpacecraft())
		{
			// Repair the spacecraft.
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				ship->RepairAll();
				ship->ResetRepairTime();
				ship->SetModified();

				QString resultMsg = tr("%1 repaired.").arg(ship->NameWithEmpire());
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		else
		{
			QString resultMsg = tr("Nothing repaired.");
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	m_I->UnlockAvatar();
}

void DtChatChannel::SceneAtomChanged(AuId oldPersonId, AuId nuPersonId)
{
	if(UnitId() == oldPersonId)
		SetCurrentPerson(nuPersonId);
}

void DtChatChannel::SetCurrentPerson(const AuId& personId)
{
	// Set the unit id and blank the name.
	m_I->SetAvatarId(personId);
	m_UnitName = "";
	if(!m_I->AvatarId().IsNull())
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		const DaPerson* person = dynamic_cast<const DaPerson*>(ether.FindNormalAtom(m_I->AvatarId()));
		if(person)
		{
			// Get the assigned name of the person atom. This is the real name of an agent, not incognito names like 'Entity'.
			m_UnitName = person->AssignedName();

			// Check to see if the person is in a design studio instance, for the proper state of our local flag.
			// Can't call InDesignEnvironment because person may not have queried their environment yet.
			const DaDesignBase* pde = person->FindParentDesignBaseInEnv();
			if(pde)
				m_DesignStudioMode = pde->DesignMode();
			else
				m_DesignStudioMode = DaDesignBase::InvalidMode;

			// Determine the current solar system of the person.
			const DaSolarSystem* system = person->FindParentSolarSystem();
			if(system)
				m_I->SetSystemId(system->Id());
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void DtChatChannel::HangarLaunch(bool takeCaptBerth)
{
	if(m_I->HangarLaunch(takeCaptBerth))
		HangarQuery();											// Update client with nu storage status.
}

void DtChatChannel::HangarMothball(const AuId& shipId)
{
	if(m_I->HangarMothball(shipId))
		HangarQuery();											// Update client with nu storage status.
}

void DtChatChannel::HangarQuery()
{
	//qDebug() << "DtChatChannel::HangarQuery";
	QString shipDesc;
	quint32 shipDesign = 0;
	quint32 shipsInHangar = 0;
	m_I->HangarQuery(shipDesc, shipDesign, shipsInHangar);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PM_Hangar;
	out << shipDesc;
	out << shipDesign;
	out << shipsInHangar;
	SendPacket(ba);
}

// Responds to client request for all cities that are in range of the transporter room in which they are standing.
void DtChatChannel::SendTransporterPlaces()
{
	QList<AuTransporterTarget> places;
	m_I->TransporterPlaces(places);
	if(!places.empty())
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_Place;
		out << (quint16)places.size();
		for(QList<AuTransporterTarget>::const_iterator pi = places.cbegin(); pi != places.cend(); pi++)
			out << *pi;
		SendPacket(ba);
	}
}

// Responds to client request for all spacecraft that are in range of the transporter room in which they are standing.
void DtChatChannel::SendTransporterSpacecraft()
{
	QList<AuTransporterShip> ships;
	m_I->TransporterSpacecraft(ships);
	if(!ships.isEmpty())
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_Ship;
		out << (quint16)ships.size();
		for(QList<AuTransporterShip>::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
			out << *si;
		SendPacket(ba);
	}
}

// Responds to client request for all transporters that are in range of the transporter room in which they are standing.
void DtChatChannel::SendTransporterTransporters()
{
	QList<AuTransporterSystem> trans;
	m_I->TransporterTransporters(trans);
	if(!trans.isEmpty())
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_Transporter;
		out << (quint16)trans.size();
		for(QList<AuTransporterSystem>::const_iterator ti = trans.cbegin(); ti != trans.cend(); ti++)
			out << *ti;
		SendPacket(ba);
	}
}

GALAXY DtChatChannel::UnitGalaxy() const
{
	GALAXY galaxy = GALAXY_Count;
	DaPerson* person = m_I->LockAvatar();
	if(person)
		galaxy = person->Galaxy();
	m_I->UnlockAvatar();
	return galaxy;
}

AuId DtChatChannel::UnitSector() const
{
	AuId sectorId;
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		const DaSector* sector = person->FindParentSector();
		if(sector)
			sectorId = sector->Id();
	}
	m_I->UnlockAvatar();
	return sectorId;
}

void DtChatChannel::FillOrDrainCapacitor(quint8 fillOrDrain, quint8 percent)
{
	QString resultMsg;

	QString crewMsg;
	AuId crewId;
	QString crewName;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	AuId shipId;

	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			if(fillOrDrain)
			{
				// Fill capacitor
				int oldCap = ship->CapacitorPercent();
				float capToAdd = ship->CapacitorMax() * (float)percent / 100.f;
				ship->ChargeCapacitor(capToAdd);
				ship->SetModified();
				int nuCap = ship->CapacitorPercent();
				int chgCap = nuCap - oldCap;

				resultMsg = tr("%1 capacitor charged at %2 of %3 (%4%).")
					.arg(ship->NameWithEmpire())
					.arg(ship->CapacitorLevel())
					.arg(ship->CapacitorMax())
					.arg((int)ship->CapacitorPercent());

				DaSentient* engineer = ship->AIEngineOperator();
				if(engineer)
				{
					if(chgCap)
						crewMsg = tr("Sir, we just had a %2% surge in capacitor power! Capacitor is now at %1%. "
								"I can't explain where the surge came from.")
							.arg((int)ship->CapacitorPercent())
							.arg(chgCap);
					else
						crewMsg = tr("Sir, we just had a minor spike in capacitor power! Capacitor is now at %1%. "
								"I can't identify the source of the spike.")
							.arg((int)ship->CapacitorPercent());
					crewId = engineer->Id();
					crewName = engineer->Name();
					sectorId = ship->FindParentSector()->Id();
					galaxy = ship->Galaxy();
					shipId = ship->Id();
				}
			}
			else
			{
				// Drain capacitor
				float capToTake = ship->CapacitorLevel() * (float)percent / 100.f;
				ship->DischargeCapacitor(capToTake);
				ship->SetModified();

				resultMsg = tr("%1 capacitor discharged at %2 of %3 (%4%).")
					.arg(ship->NameWithEmpire())
					.arg(ship->CapacitorLevel())
					.arg(ship->CapacitorMax())
					.arg((int)ship->CapacitorPercent());

				DaSentient* engineer = ship->AIEngineOperator();
				if(engineer)
				{
					crewMsg = tr("Sir, we just had a %2% drain on our capacitor power! Capacitor is now at %1%. "
							"I don't understand where the power went.")
						.arg((int)ship->CapacitorPercent())
						.arg(percent);
					crewId = engineer->Id();
					crewName = engineer->Name();
					sectorId = ship->FindParentSector()->Id();
					galaxy = ship->Galaxy();
					shipId = ship->Id();
				}
			}
		}
		else
		{
			resultMsg = tr("You must board a spacecraft to %1 a capacitor.").arg(fillOrDrain?tr("fill"):tr("drain"));
		}
	}
	m_I->UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		if(!crewMsg.isEmpty())
			resultMsg += tr(" The ship's engineer %1 alerted the crew.").arg(crewName);
		AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
	}

	if(!crewMsg.isEmpty())
	{
		AuCommChat msg(crewMsg, AuCommStdChannel::CrewChannelName(), crewId, crewName, sectorId, galaxy,
				AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
		msg.SetScopeData(shipId.GetID());	// Crew channel needs ship id to resolve scope of message.
		g_Au.SendAtomChat(msg);
	}
}

// Null 'cityId' is current city.
void DtChatChannel::ReportCityLast(const AuId& cityId)
{
	// Try to find the city locally.
	bool haveReport = false;
	bool havePrevReport = false;
	bool personHasAgentPower = false;
	quint32 personEmpire = 0;
	AuCityReport report, prevReport;

	bool foundCity = m_I->ReportCityLast(cityId, haveReport, report, havePrevReport, prevReport, personHasAgentPower, personEmpire);

	if(haveReport)
		SendCityLastReport(cityId, report, havePrevReport ? &prevReport : 0);
	else if(!foundCity && !cityId.IsNull())
		emit CityReportRequested(cityId, (quint8)0, m_I->AvatarId(), personHasAgentPower?(quint32)0:personEmpire);	// 0 skips empire ck
}

void DtChatChannel::CityLastReportReady(AuId unitId, AuId cityId, AuCityReport report, bool havePrevReport, AuCityReport prevReport)
{
	// All chat channels receive this signal. The unitId says who it's for.
	if(unitId != m_I->AvatarId())
		return;	// Report is not for me.

	SendCityLastReport(cityId, report, havePrevReport ? &prevReport : 0);
}

void DtChatChannel::SendCityLastReport(const AuId& cityId, const AuCityReport& report, const AuCityReport* prevReport)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PE_Report;
	out << (quint8)0;					// 0 = city report.
	out << cityId;
	out << report;
	if(prevReport)
	{
		out << (quint8)1;
		out << *prevReport;
	}
	else
		out << (quint8)0;
	SendPacket(ba);
}

// Null 'cityId' is current city.
void DtChatChannel::ReportCityHistory(const AuId& cityId)
{
	// Try to find the city locally.
	bool haveReport = false;
	bool personHasAgentPower = false;
	quint32 personEmpire = 0;
	AuCityReportHistory report;
	bool foundCity = m_I->ReportCityHistory(cityId, haveReport, report, personHasAgentPower, personEmpire);

	if(haveReport)
		SendCityReportHistory(cityId, report);
	else if(!foundCity && !cityId.IsNull())
		emit CityReportRequested(cityId, (quint8)1, m_I->AvatarId(), personHasAgentPower?(quint32)0:personEmpire);	// 0 skips empire ck
}

void DtChatChannel::CityHistoryReportReady(AuId unitId, AuId cityId, AuCityReportHistory report)
{
	// All chat channels receive this signal. The unitId says who it's for.
	if(unitId != m_I->AvatarId())
		return;	// Report is not for me.

	SendCityReportHistory(cityId, report);
}

void DtChatChannel::SendCityReportHistory(const AuId& cityId, const AuCityReportHistory& report)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PE_Report;
	out << (quint8)1;					// 1 = city report history.
	out << cityId;
	out << report;
	SendPacket(ba);
}

void DtChatChannel::OrdnanceList()
{
	// Get the list of ordnance available.
	AuId loaderId;	// 'loaderId' is the id of the atom providing the ordnance.
	AuItemHash ordnance;
	m_I->OrdnanceList(loaderId, ordnance);
	if(!ordnance.empty())
	{
		// Send the list of ordnance to the client.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Ordnance;
		out << loaderId;
		out << ordnance;
		SendPacket(ba);
	}
}

void DtChatChannel::ArenaSendList()
{
	unordered_map<AuId, AuArenaReady, AuIdHash> arenas;
	m_I->ArenaList(arenas);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PM_QueryArenas;

	out << (quint16)arenas.size();
	for(unordered_map<AuId, AuArenaReady, AuIdHash>::const_iterator ai = arenas.cbegin(); ai != arenas.cend(); ai++)
	{
		// Don't send the password to the client.
		AuArenaReady nopw((*ai).second);
		if(!nopw.m_Password.isEmpty())
			nopw.m_Password = "*";
		out << nopw;
	}

	SendPacket(ba);
}

void DtChatChannel::QueryAvatars()
{
	AuId unitIds[6];
	QString unitNames[6];
	AuLoginComm::ListPlayerUnits(m_idPlayer, m_confirmNum, unitIds, unitNames);
	//qDebug() << "DtChatChannel::QueryAvatars" << m_idPlayer << m_confirmNum
		//<< unitIds[0] << unitNames[0]
		//<< unitIds[1] << unitNames[1]
		//<< unitIds[2] << unitNames[2]
		//<< unitIds[3] << unitNames[3]
		//<< unitIds[4] << unitNames[4]
		//<< unitIds[5] << unitNames[5];

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PZ_QueryAvatars;
	for(int ia=0; ia<6; ia++)
	{
		out << unitIds[ia];
		out << unitNames[ia];
	}
	SendPacket(ba);
}

void DtChatChannel::DesignerArchitectListSc()
{
	// Tell the db thread to load all designs by this architect, so they are ready for them.
	g_Au.ScDesignsNeeded(m_I->AvatarId());
	if(!m_ScDesignsRequested)
	{
		m_ScDesignsRequested = true;	// We don't need to notify them more than once per server.
		AuCommThought msg(QObject::tr("A librarian is retrieving my spacecraft designs. "
					"If some designs are not present, I should wait a few minutes then try again."), m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
		return;
	}

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_ScLibrary;
	out << (quint8)0;					// Sending design list.

	m_rwScDesigns.lockForWrite();
	const DgData& sdd = g_Au.LockScDesignsForRead();
	// Count the designs we will send.
	quint16 c = 0;
	for(AuScDesignHash::const_iterator di = sdd.m_ScDesigns->cbegin(); di != sdd.m_ScDesigns->cend(); di++)
		if((*di).second.ArchitectId() == m_I->AvatarId() && !(*di).second.IsDeleted())
			c++;

	// Send the count.
	out << c;

	// Send the list of designs.
	for(AuScDesignHash::const_iterator di = sdd.m_ScDesigns->cbegin(); di != sdd.m_ScDesigns->cend(); di++)
		if((*di).second.ArchitectId() == m_I->AvatarId() && !(*di).second.IsDeleted())
		{
			AuScReport report;
			(*di).second.ReportDesign(report);

			out << report;
			out << (quint8)(*di).second.State();
			out << (*di).second.Thumbnail();

			m_ScDesignsInUse.insert(pair<quint32, bool>((*di).first, true));
		}

	g_Au.UnlockScDesigns();
	m_rwScDesigns.unlock();

	SendPacket(ba);
}

void DtChatChannel::ScDesignExchangeList()
{
	// Get my empire.
	quint32 empireId = 0;
	DaPerson* person = m_I->LockAvatar();
	if(person)
		empireId = person->EmpireId();
	m_I->UnlockAvatar();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);

	const DgData& sdl = g_Au.LockScExchangeForRead();
	AuScDesignExchange lib;
	sdl.m_ScExchange->ListAndRate(lib, m_I->AvatarId(), empireId, *sdl.m_ScExchangeRatings);	// List designs available to me.
	g_Au.UnlockScExchange();

	// Send the count.
	out << (quint32)lib.size();

	const DgData& sdd = g_Au.LockScDesignsForRead();

	// Send the list of library entries and thumbnails if available.
	for(AuScDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
	{
		out << *li;

		AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find((*li).DesignId());
		if(di != sdd.m_ScDesigns->cend())
			out << (*di).second.Thumbnail();
		else
		{
			g_Au.ScDesignNeeded((*li).DesignId());				// Tell global data we need the design, for when they query again.
			out << QImage();
		}
	}

	g_Au.UnlockScDesigns();

	// Send compressed array of data.
	QByteArray bacomp = qCompress(ba);
	//qDebug() << "DtChatChannel::ScDesignExchangeList bytes" << ba.size() << "compressed to" << bacomp.size();
	if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am receiving the spacecraft blueprint exchange; the data is %L1 bytes. "
					"The Spacecraft Exchange window may have requested it. "
					"I won't receive chat messages until it arrives."
					).arg(bacomp.size()), m_I->AvatarId(), m_UnitName);
		ChatToClient(&msg);	// Send this message to the scene client immediately so it gets there before the exchange data packet.
	}

	QByteArray bapkt;
	QDataStream outpacket(&bapkt, QIODevice::WriteOnly);
	outpacket.setVersion(QDataStream::Qt_4_1);
	outpacket << (quint8)DtChatChannelPacket::PD_ScExchange;
	outpacket << (quint8)2;					// Sending compressed published library list.
	outpacket << bacomp;
	SendPacket(bapkt);

	if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am done receiving the spacecraft blueprint exchange."), m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
	}
}

void DtChatChannel::DesignerGearOpen(quint32 designId, const AuGearPos& pos)
{
	if(m_DesignStudioMode == DaDesignBase::InvalidMode)
		return;
	quint8 designStudioMode = m_I->DesignerGearOpen(designId, pos);
	if(designStudioMode == DaDesignBase::InvalidMode)
		return;
	m_DesignStudioMode = designStudioMode;
}

void DtChatChannel::ShipmentCreate(const AuShipment& shipment, const AuId& opBd)
{
	if(m_I->ShipmentCreate(shipment, opBd))
		ShipmentsRefresh(opBd, false);
}

void DtChatChannel::ShipmentDelete(const AuShipment& shipment, const AuId& opBd)
{
	if(m_I->ShipmentDelete(shipment, opBd))
		ShipmentsRefresh(opBd, false);
}

void DtChatChannel::ShipmentsRefresh(const AuId& opBd, bool sayResult)
{
	AuShipmentList shipments;
	AuTradeAgreementList tas;
	int result = m_I->BrokerShipments(shipments, tas, opBd);

	// Transmit the shipments even if empty so the client knows it is empty.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PZ_Trade;
	out << (quint8)0;					// Sending shipments list.
	out << shipments;
	SendPacket(ba);

	// Transmit the trade agreements even if empty so the client knows it is empty.
	QByteArray bat;
	QDataStream outt(&bat, QIODevice::WriteOnly);
	outt.setVersion(QDataStream::Qt_4_1);
	outt << (quint8)DtChatChannelPacket::PZ_Trade;
	outt << (quint8)1;					// Sending trade agreements list.
	outt << tas;
	SendPacket(bat);

	if(!sayResult)
		return;

	// Notify the player of the result. Usually receipt of the information is sufficient.
	QString resultMsg;
	if(result == 3)
		resultMsg = tr("Only authorized government officials can view shipments and trade agreements.");
	else if(shipments.empty() && tas.empty())
		resultMsg = tr("The broker has no trade agreements or shipments.");
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
	}
}

void DtChatChannel::SendPlayerOnlineStatus()
{
	AuPlayerOnlineStatusVector players;
	auSceneServerApp->AllPlayersOnlineStatus(players, AcctIsAgent(m_AccountType));
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PE_Report;
	out << (quint8)2;					// 2 = players online status
	players >> out;
	SendPacket(ba);
}

void DtChatChannel::DesignerArchitectListBd()
{
	// Tell the db thread to load all designs by this architect, so they are ready for them.
	g_Au.BdDesignsNeeded(m_I->AvatarId());
	if(!m_BdDesignsRequested)
	{
		m_BdDesignsRequested = true;	// We don't need to notify them more than once per server.
		AuCommThought msg(QObject::tr("A librarian is retrieving my building designs. "
					"If some designs are not present, I should wait a few minutes then try again."), m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
		return;
	}

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_BdLibrary;
	out << (quint8)0;					// Sending design list.

	m_rwBdDesigns.lockForWrite();
	const DgData& sdd = g_Au.LockBdDesignsForRead();
	// Count the designs we will send.
	quint16 c = 0;
	for(AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->cbegin(); di != sdd.m_BdDesigns->cend(); di++)
		if((*di).second.ArchitectId() == m_I->AvatarId() && !(*di).second.IsDeleted())
			c++;

	// Send the count.
	out << c;

	// Send the list of designs.
	for(AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->cbegin(); di != sdd.m_BdDesigns->cend(); di++)
		if((*di).second.ArchitectId() == m_I->AvatarId() && !(*di).second.IsDeleted())
		{
			AuBdReport report;
			(*di).second.ReportDesign(report);

			out << report;
			out << (quint8)(*di).second.State();
			out << (*di).second.Thumbnail();

			m_BdDesignsInUse.insert(pair<quint32, bool>((*di).first, true));
			QSet<quint32> xrefs;
			(*di).second.ExternalReferences(xrefs);
			for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
				m_BdDesignsInUse.insert(pair<quint32, bool>(*xi, true));
		}

	g_Au.UnlockBdDesigns();
	m_rwBdDesigns.unlock();

	SendPacket(ba);
}

void DtChatChannel::BdDesignExchangeList()
{
	// Get my empire.
	quint32 empireId = 0;
	DaPerson* person = m_I->LockAvatar();
	if(person)
		empireId = person->EmpireId();
	m_I->UnlockAvatar();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);

	const DgData& sdl = g_Au.LockBdExchangeForRead();
	AuBdDesignExchange lib;
	sdl.m_BdExchange->ListAndRate(lib, m_I->AvatarId(), empireId, *sdl.m_BdExchangeRatings);	// List designs available to me.
	g_Au.UnlockBdExchange();

	// Send the count.
	out << (quint32)lib.size();

	const DgData& sdd = g_Au.LockBdDesignsForRead();

	// Send the list of library entries and thumbnails if available.
	for(AuBdDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
	{
		out << *li;

		AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find((*li).DesignId());
		if(di != sdd.m_BdDesigns->cend())
			out << (*di).second.Thumbnail();
		else
		{
			g_Au.BdDesignNeeded((*li).DesignId());				// Tell global data we need the design, for when they query again.
			out << QImage();
		}
	}

	g_Au.UnlockBdDesigns();

	// Send compressed array of data.
	QByteArray bacomp = qCompress(ba);
	//qDebug() << "DtChatChannel::BdDesignExchangeList bytes" << ba.size() << "compressed to" << bacomp.size();
	if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am receiving the building blueprint exchange; the data is %L1 bytes. "
					"The Construction or Building Exchange window may have requested it. "
					"I won't receive chat messages until it arrives."
					).arg(bacomp.size()), m_I->AvatarId(), m_UnitName);
		ChatToClient(&msg);	// Send this message to the scene client immediately so it gets there before the exchange data packet.
	}

	QByteArray bapkt;
	QDataStream outpacket(&bapkt, QIODevice::WriteOnly);
	outpacket.setVersion(QDataStream::Qt_4_1);
	outpacket << (quint8)DtChatChannelPacket::PD_BdExchange;
	outpacket << (quint8)2;					// Sending compressed published library list.
	outpacket << bacomp;
	SendPacket(bapkt);

	if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am done receiving the building blueprint exchange."), m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
	}
}

// This may be signaled to us from an atom who enters a designer station on a ship.
void DtChatChannel::EnteredDesigner(AuId personId, int designStudioMode)
{
	if(personId != m_I->AvatarId())
		return;
	//qDebug() << "DtChatChannel::EnteredDesigner" << personId << designStudioMode;
	m_DesignStudioMode = designStudioMode;
	if(m_DesignStudioMode == DaDesignBase::DESIGN_Unknown)
	{
		// Initial entry into designer.
		if(!m_BdDesignsRequested)
		{
			// Tell the db thread to load all designs by this architect, so they are ready for them.
			g_Au.BdDesignsNeeded(m_I->AvatarId());
			m_BdDesignsRequested = true;	// We don't need to do this more than once per server.
		}

		if(!m_ScDesignsRequested)
		{
			// Tell the db thread to load all designs by this architect, so they are ready for them.
			g_Au.ScDesignsNeeded(m_I->AvatarId());
			m_ScDesignsRequested = true;	// We don't need to do this more than once per server.
		}
	}
}

void DtChatChannel::ListDesignsInUse(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns)
{
	m_rwBdDesigns.lockForRead();
	for(unordered_map<quint32, bool, AuDesignIdHash>::const_iterator bdi = m_BdDesigns.cbegin(); bdi != m_BdDesigns.cend(); bdi++)
		bdDesigns.insert((*bdi).first);
	for(unordered_map<quint32, bool, AuDesignIdHash>::const_iterator bdi = m_BdDesignsInUse.cbegin();
			bdi != m_BdDesignsInUse.cend(); bdi++)
		bdDesigns.insert((*bdi).first);
	m_rwBdDesigns.unlock();

	m_rwScDesigns.lockForRead();
	for(unordered_map<quint32, bool, AuDesignIdHash>::const_iterator sci = m_ScDesigns.cbegin(); sci != m_ScDesigns.cend(); sci++)
		scDesigns.insert((*sci).first);
	for(unordered_map<quint32, bool, AuDesignIdHash>::const_iterator sci = m_ScDesignsInUse.cbegin();
			sci != m_ScDesignsInUse.cend(); sci++)
		scDesigns.insert((*sci).first);
	m_rwScDesigns.unlock();
}

// on station
void DtChatChannel::ShipmentCreate(const AuShipment& shipment)
{
	if(m_I->ShipmentCreate(shipment))
		ShipmentsRefresh(false);
}

// on station
void DtChatChannel::ShipmentDelete(const AuShipment& shipment)
{
	if(m_I->ShipmentDelete(shipment))
		ShipmentsRefresh(false);
}

// on station
void DtChatChannel::ShipmentsRefresh(bool sayResult)
{
	AuShipmentList shipments;
	bool isStation = false;
	int result = m_I->Shipments(shipments, isStation);

	// Transmit the shipments even if empty so the client knows it is empty.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PZ_Trade;
	out << (quint8)0;					// Sending shipments list.
	out << shipments;
	SendPacket(ba);

	if(!sayResult)
		return;

	QString resultMsg;
	if(result == 3)
		resultMsg = tr("I have to be in the chain of command to view shipments.");
	else if(shipments.empty())
	{
		if(isStation)
			resultMsg = tr("The space station has no shipments scheduled.");
		else
			resultMsg = tr("Only a space station can schedule shipments.");
	}

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
	}
}

void DtChatChannel::BdDesignExchangeReportRatings(quint32 designId)
{
	AuExchangeBlueprintRating rating;
	const DgData& sdl = g_Au.LockBdExchangeForRead();
	bool hasRating = sdl.m_BdExchangeRatings->Rating(designId, rating);
	QString designName = sdl.m_BdExchange->DesignName(designId);
	g_Au.UnlockBdExchange();
	if(!hasRating)
		return;

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_BdExchange;
	out << (quint8)1;					// Sending ratings.
	out << designId;
	out << designName;
	out << rating;
	SendPacket(ba);
}

void DtChatChannel::ScDesignExchangeReportRatings(quint32 designId)
{
	AuExchangeBlueprintRating rating;
	const DgData& sdl = g_Au.LockScExchangeForRead();
	bool hasRating = sdl.m_ScExchangeRatings->Rating(designId, rating);
	QString designName = sdl.m_ScExchange->DesignName(designId);
	g_Au.UnlockScExchange();
	if(!hasRating)
		return;

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_ScExchange;
	out << (quint8)1;					// Sending ratings.
	out << designId;
	out << designName;
	out << rating;
	SendPacket(ba);
}

void DtChatChannel::DesignerArchitectListIt()
{
	// Tell the db thread to load all designs by this architect, so they are ready for them.
	g_Au.ItDesignsNeeded(m_I->AvatarId());
	if(!m_ItDesignsRequested)
	{
		m_ItDesignsRequested = true;	// We don't need to notify them more than once per server.
		AuCommThought msg(QObject::tr("A librarian is retrieving my spacecraft designs. "
					"If some designs are not present, I should wait a few minutes then try again."), m_I->AvatarId(), m_UnitName);
		g_Au.SendAtomChat(msg);
		return;
	}

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_ItLibrary;
	out << (quint8)0;					// Sending design list.

	m_rwItDesigns.lockForWrite();
	const DgData& sdd = g_Au.LockItDesignsForRead();
	// Count the designs we will send.
	quint16 c = 0;
	for(AuItDesignHash::const_iterator di = sdd.m_ItDesigns->cbegin(); di != sdd.m_ItDesigns->cend(); di++)
		if((*di).second.ArchitectId() == m_I->AvatarId() && !(*di).second.IsDeleted())
			c++;

	// Send the count.
	out << c;

	// Send the list of designs.
	for(AuItDesignHash::const_iterator di = sdd.m_ItDesigns->cbegin(); di != sdd.m_ItDesigns->cend(); di++)
		if((*di).second.ArchitectId() == m_I->AvatarId() && !(*di).second.IsDeleted())
		{
			AuItReport report;
			(*di).second.ReportDesign(report);

			out << report;
			out << (quint8)(*di).second.State();
			out << (*di).second.Thumbnail();

			m_ItDesignsInUse.insert(pair<quint32, bool>((*di).first, true));
		}

	g_Au.UnlockItDesigns();
	m_rwItDesigns.unlock();

	SendPacket(ba);
}

void DtChatChannel::TradeAgreementCreate(const AuTradeAgreement& ta, const AuId& opBd)
{
	if(m_I->TradeAgreementCreate(ta, opBd))
		ShipmentsRefresh(opBd, false);
}

void DtChatChannel::TradeAgreementDelete(const AuTradeAgreement& ta, const AuId& opBd)
{
	if(m_I->TradeAgreementDelete(ta, opBd))
		ShipmentsRefresh(opBd, false);
}

void DtChatChannel::TradeAgreementAccept(const AuTradeAgreement& ta, const AuId& opBd)
{
	if(m_I->TradeAgreementAccept(ta, opBd))
		ShipmentsRefresh(opBd, false);
}

void DtChatChannel::TradeAgreementCounter(const AuTradeAgreement& proposal, const AuTradeAgreement& counter, const AuId& opBd)
{
	if(m_I->TradeAgreementCounter(proposal, counter, opBd))
		ShipmentsRefresh(opBd, false);
}

void DtChatChannel::BdDesignExchangeToC()
{
	// Get my empire.
	quint32 empireId = 0;
	AuId personId;
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		personId = person->Id();
		empireId = person->EmpireId();
	}
	m_I->UnlockAvatar();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);

	const DgData& sdl = g_Au.LockBdExchangeForRead();
	AuBdDesignExchange lib;
	sdl.m_BdExchange->ListAndRate(lib, personId, empireId, *sdl.m_BdExchangeRatings);	// List designs available to me.
	g_Au.UnlockBdExchange();

	QByteArray bapkt;
	QDataStream outpacket(&bapkt, QIODevice::WriteOnly);
	outpacket.setVersion(QDataStream::Qt_4_1);
	outpacket << (quint8)DtChatChannelPacket::PD_BdExchange;
	outpacket << (quint8)10;				// Sending ToC

	// Send the count.
	outpacket << (quint32)lib.size();

	// Send the list of design ids and last modified times. The client will then request the specific entries needed.
	for(AuBdDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
	{
		outpacket << (*li).DesignId();
		outpacket << (*li).LastMod();
	}

	SendPacket(bapkt);
}

void DtChatChannel::BdDesignExchangeList(const QList<quint32>& designIds)
{
	// Get my empire.
	quint32 empireId = 0;
	AuId personId;
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		personId = person->Id();
		empireId = person->EmpireId();
	}
	m_I->UnlockAvatar();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);

	const DgData& sdl = g_Au.LockBdExchangeForRead();
	AuBdDesignExchange lib;
	sdl.m_BdExchange->ListAndRate(lib, personId, empireId, *sdl.m_BdExchangeRatings);	// List designs available to me.
	g_Au.UnlockBdExchange();

	const DgData& sdd = g_Au.LockBdDesignsForRead();

	// Count the entries with thumbnails. The client will cache this so we want to send the full monty.
	quint32 cnt = 0;
	quint32 cntwaiting = 0;
	for(QList<quint32>::const_iterator bi = designIds.cbegin(); bi != designIds.cend(); bi++)
		if(lib.contains(*bi))
		{
			AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find(*bi);
			if(di != sdd.m_BdDesigns->cend())
				cnt++;
			else
			{
				g_Au.BdDesignNeeded(*bi);			// Tell global data we need the design, for when they query again.
				cntwaiting++;
			}
		}
	out << cnt;

	// Send the list of library entries and thumbnails.
	for(QList<quint32>::const_iterator bi = designIds.cbegin(); bi != designIds.cend(); bi++)
	{
		AuBdDesignExchange::const_iterator li = lib.find(*bi);
		if(li != lib.cend())
		{
			AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find(*bi);
			if(di != sdd.m_BdDesigns->cend())
			{
				out << *li;
				out << (*di).second.Thumbnail();
			}
		}
	}

	g_Au.UnlockBdDesigns();

	// Send compressed array of data.
	QByteArray bacomp = qCompress(ba);
	if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am receiving the building blueprint exchange; the data is %L1 bytes. "
					"I won't receive chat messages until it is done."
					).arg(bacomp.size()), personId, m_UnitName);
		ChatToClient(&msg);	// Send this message to the scene client immediately so it gets there before the exchange data packet.
	}

	QByteArray bapkt;
	QDataStream outpacket(&bapkt, QIODevice::WriteOnly);
	outpacket.setVersion(QDataStream::Qt_4_1);
	outpacket << (quint8)DtChatChannelPacket::PD_BdExchange;
	outpacket << (quint8)11;				// Sending compressed partial published library list.
	outpacket << bacomp;
	SendPacket(bapkt);

	if(cntwaiting)
	{
		AuCommThought msg(tr("I received %L2 building blueprint exchange entries. "
					"The server is loading %L1 additional entries. "
					"I must refresh the building blueprint exchange to get them.")
				.arg(cntwaiting)
				.arg(cnt), personId, m_UnitName);
		ChatToClient(&msg);
	}
	else if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am done receiving the building blueprint exchange."), personId, m_UnitName);
		g_Au.SendAtomChat(msg);
	}
	else if(cnt == 1)
	{
		AuCommThought msg(tr("I received a building blueprint exchange entry."), personId, m_UnitName);
		ChatToClient(&msg);
	}
	else if(cnt)
	{
		AuCommThought msg(tr("I received %L1 building blueprint exchange entries.").arg(cnt), personId, m_UnitName);
		ChatToClient(&msg);
	}
}

void DtChatChannel::ScDesignExchangeToC()
{
	// Get my empire.
	quint32 empireId = 0;
	AuId personId;
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		personId = person->Id();
		empireId = person->EmpireId();
	}
	m_I->UnlockAvatar();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);

	const DgData& sdl = g_Au.LockScExchangeForRead();
	AuScDesignExchange lib;
	sdl.m_ScExchange->ListAndRate(lib, personId, empireId, *sdl.m_ScExchangeRatings);	// List designs available to me.
	g_Au.UnlockScExchange();

	QByteArray bapkt;
	QDataStream outpacket(&bapkt, QIODevice::WriteOnly);
	outpacket.setVersion(QDataStream::Qt_4_1);
	outpacket << (quint8)DtChatChannelPacket::PD_ScExchange;
	outpacket << (quint8)10;				// Sending ToC

	// Send the count.
	outpacket << (quint32)lib.size();

	// Send the list of design ids and last modified times. The client will then request the specific entries needed.
	for(AuScDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
	{
		outpacket << (*li).DesignId();
		outpacket << (*li).LastMod();
	}

	SendPacket(bapkt);
}

void DtChatChannel::ScDesignExchangeList(const QList<quint32>& designIds)
{
	// Get my empire.
	quint32 empireId = 0;
	AuId personId;
	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		personId = person->Id();
		empireId = person->EmpireId();
	}
	m_I->UnlockAvatar();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);

	const DgData& sdl = g_Au.LockScExchangeForRead();
	AuScDesignExchange lib;
	sdl.m_ScExchange->ListAndRate(lib, personId, empireId, *sdl.m_ScExchangeRatings);	// List designs available to me.
	g_Au.UnlockScExchange();

	const DgData& sdd = g_Au.LockScDesignsForRead();

	// Count the entries with thumbnails. The client will cache this so we want to send the full monty.
	quint32 cnt = 0;
	quint32 cntwaiting = 0;
	for(QList<quint32>::const_iterator bi = designIds.cbegin(); bi != designIds.cend(); bi++)
		if(lib.contains(*bi))
		{
			AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find(*bi);
			if(di != sdd.m_ScDesigns->cend())
				cnt++;
			else
			{
				g_Au.ScDesignNeeded(*bi);			// Tell global data we need the design, for when they query again.
				cntwaiting++;
			}
		}
	out << cnt;

	// Send the list of library entries and thumbnails.
	for(QList<quint32>::const_iterator bi = designIds.cbegin(); bi != designIds.cend(); bi++)
	{
		AuScDesignExchange::const_iterator li = lib.find(*bi);
		if(li != lib.cend())
		{
			AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find(*bi);
			if(di != sdd.m_ScDesigns->cend())
			{
				out << *li;
				out << (*di).second.Thumbnail();
			}
		}
	}

	g_Au.UnlockScDesigns();

	// Send compressed array of data.
	QByteArray bacomp = qCompress(ba);
	if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am receiving the spacecraft blueprint exchange; the data is %L1 bytes. "
					"I won't receive chat messages until it is done."
					).arg(bacomp.size()), personId, m_UnitName);
		ChatToClient(&msg);	// Send this message to the scene client immediately so it gets there before the exchange data packet.
	}

	QByteArray bapkt;
	QDataStream outpacket(&bapkt, QIODevice::WriteOnly);
	outpacket.setVersion(QDataStream::Qt_4_1);
	outpacket << (quint8)DtChatChannelPacket::PD_ScExchange;
	outpacket << (quint8)11;				// Sending compressed partial published library list.
	outpacket << bacomp;
	SendPacket(bapkt);

	if(cntwaiting)
	{
		AuCommThought msg(tr("I received %L2 spacecraft blueprint exchange entries. "
					"The server is loading %L1 additional entries. "
					"I must refresh the spacecraft blueprint exchange to get them.")
				.arg(cntwaiting)
				.arg(cnt), personId, m_UnitName);
		ChatToClient(&msg);
	}
	else if(bacomp.size() > 5000000)
	{
		AuCommThought msg(tr("I am done receiving the spacecraft blueprint exchange."), personId, m_UnitName);
		g_Au.SendAtomChat(msg);
	}
	else if(cnt == 1)
	{
		AuCommThought msg(tr("I received a spacecraft blueprint exchange entry."), personId, m_UnitName);
		ChatToClient(&msg);
	}
	else if(cnt)
	{
		AuCommThought msg(tr("I received %L1 spacecraft blueprint exchange entries.").arg(cnt), personId, m_UnitName);
		ChatToClient(&msg);
	}
}

const AuId& DtChatChannel::UnitId() const
{
	return m_I->AvatarId();
}

void DtChatChannel::TransferListCitizens(const AuId& shipId)
{
	QByteArray bacit;
	m_I->TransferListCitizens(shipId, bacit);
	if(!bacit.isEmpty())
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << (quint8)0;
		out << bacit;
		SendPacket(ba);
	}
}

