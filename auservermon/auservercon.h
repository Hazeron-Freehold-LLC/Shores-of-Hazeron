//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservercon.h,v 1.58 2022/04/15 21:04:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServerCon_H
#define AuServerCon_H

#include "audice.h"
#include "dgempire.h"
#include "dtmgrclient.h"
#include <QMutex>
#include <QWidget>

class AuId;
class AuListWidget;
class AuPlayersTable;
class AuReportWidget;
class QLabel;
class QListWidgetItem;
class QPushButton;
class QResizeEvent;
class QToolButton;

class AuServerChoice
{
	QString								m_ListName;
	QString								m_NetName;
	quint16								m_NetPort;
	int									m_ServerMode;			// AuGlobal::MODE

public:
	inline AuServerChoice(int serverMode, const QString& listName, const QString& netName, quint16 netPort)
	{
		m_ListName = listName;
		m_NetName = netName;
		m_NetPort = netPort;
		m_ServerMode = serverMode;
	}

	inline AuServerChoice()
	{
		m_NetPort = 0;
		m_ServerMode = 0;
	}

	inline AuServerChoice(const AuServerChoice& c)
	{
		m_ListName = c.m_ListName;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_ServerMode = c.m_ServerMode;
	}

public:
	inline const AuServerChoice& operator=(const AuServerChoice& c)
	{
		m_ListName = c.m_ListName;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_ServerMode = c.m_ServerMode;
		return *this;
	}

public:
	inline const QString& ListName() const
	{
		return m_ListName;
	}

	inline const QString& NetName() const
	{
		return m_NetName;
	}

	inline quint16 NetPort() const
	{
		return m_NetPort;
	}

	inline int ServerMode() const
	{
		return m_ServerMode;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuServerCon : public QWidget
{
	Q_OBJECT;
	enum UNIVERSE_DB_QUERY
	{
		Q_GameData,
		Q_LimboCheck,
		Q_AtomId,
		Q_AvatarName,
		Q_SpacecraftName,
		Q_SystemId,
		Q_SystemName,
		Q_WorldId,
		Q_WorldName,
	};

private:
	QHash<int, AuElapsedTimer>	m_Alert;			// key = row index, value = when alert
	QList<DtMgrClient*>		m_Connections;
	QMutex					m_ConnectionsLock;
	bool					m_DbFailed;				// True if database connection fails, so we stop trying.
	QList<AuServerChoice>	m_DebugServers;
	AuDice					m_Dice;
	QImage					m_Fire;
	QHash<int, QIcon>		m_FireIcons;
	bool					m_GdLoaded;				// True after game data has been loaded.
	bool					m_OperateOnSelectedOnly;
	int						m_PingIcon;
	int						m_PlayerCount;
	AuPlayersTable*			m_PlayersOn;
	QList<AuServerChoice>	m_PublicServers;
	AuReportWidget*			m_Report;				// Report widget in the frame.

	// Widgets
	QLabel*					m_ConnectionsInfo;
	AuListWidget*			m_ConnectionsList;
	QLabel*					m_PlayersInfo;

public:
	AuServerCon(AuReportWidget* report, AuPlayersTable* playersOn, QWidget* parent=0);
	virtual ~AuServerCon();

	void DisconnectAll();

signals:
	void Broadcast(QString serverName, quint16 serverPort, QString message);

	// 'galaxy' is a GALAXY enum value.
	void CreateSectorAt(QString serverName, quint16 serverPort, quint8 galaxy, double sectorX, double sectorY, double sectorZ);

	void DebugMessages(QString serverName, quint16 serverPort, quint32 debugMsgBits);
	void DebugSwitch(QString serverName, quint16 serverPort, quint8 specificDebug, quint8 value, quint32 atomId);
	void Disconnect(QString serverName, quint16 serverPort);
	void FinalizeSystem(QString serverName, quint16 serverPort, quint32 atomId);
	void FindUnitId(QString serverName, quint16 serverPort, quint32 unitId);
	void FindUnitName(QString serverName, quint16 serverPort, QString unitName);
	void LimboRescue(QString serverName, quint16 serverPort, quint32 atomId);
	void LimboRescueList(QString serverName, quint16 serverPort, QList<quint32> atomIds);
	void Probe(QString serverName, quint16 serverPort);
	void ReassignAtom(QString mgrServerName, quint16 mgrServerPort, quint32 atomId, QString wrkServerName, quint16 wrkServerPort);
	void ReloadScript(QString serverName, quint16 serverPort, quint32 scriptId);	// 'scriptId' 0 reloads all scripts
	void RequestMemoryUsage(QString serverName, quint16 serverPort);
	void RequestReloadData(QString serverName, quint16 serverPort);
	void RequestResourceReport(QString serverName, quint16 serverPort, quint8 specificReport, quint32 atomId);
	void RestoreEmpire(QString serverName, quint16 serverPort, DgEmpire empire);
	void RevokeAtom(QString serverName, quint16 serverPort, quint32 atomId);
	void ServerBroadcast(QString serverName, quint16 serverPort, QString text);
	void ServerSelected(int mode);	// 'mode' is the AuGlobal::MODE of the selected server.
	void SetAtomModified(QString serverName, quint16 serverPort, quint32 atomId, bool recursive);
	void SetControlDecay(QString serverName, quint16 serverPort, quint16 decayMs);
	void SetMaxHistory(int width);
	void SetUdpDecay(QString serverName, quint16 serverPort, quint16 decayMs);
	void Shutdown(QString serverName, quint16 serverPort, QString warnMsg, QString haltMsg, qint16 shutdownSecs);
	void ShutdownHalt(QString serverName, quint16 serverPort, QString haltMsg);
	void StartPolling(int intervalMs, QString serverName, quint16 serverPort);
	void StopPolling(QString serverName, quint16 serverPort);

protected slots:
	void AllServers();
	void AtomQuery();
	void AvatarQuery();
	void ConnectDebugGrid();
	void ConnectDebugSceneWorkers();
	void ConnectDebugWorker();
	void ConnectGrid();
	void ConnectPublicWorker();
	void ConnectSceneWorkers();
	void ConnectionClicked(QListWidgetItem* item);
	void ConnectionClosed();
	void CreateSectorAt();
	void DeselectAll();
	void Disconnect();
	void FinalizeSectorOrSystem();
	void FindId();
	void FindName();
	void HaltShutdown();
	void LimboCheck();
	void LimboRescue();
	void LimboRescueList();
	void LogMessages();
	void ManageAccounts();
	void MemoryUsageReport(QString serverName, quint16 serverPort, QStringList report);
	void MonitorAnswered(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
			unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock);
	void ProbeAnswered(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
			unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock);
	void ProbeServers();
	void ReassignSystem();
	void ReloadGameData();
	void ReloadStatic();
	void ReportAtom();
	void ReportMemory();
	void ReportResources();
	void ResourceReport(AuResourceReport report);
	void ResourceReportAtom(AuResourceReportAtom report);
	void RestoreEmpire();
	void RevokeSectorOrSystem();
	void ServerBroadcast();
	void ServerDisableAll();
	void ServerDisableAnimalAI();
	void ServerDisableAnimalEnc();
	void ServerDisableCharAI();
	void ServerDisableCitizenPath();
	void ServerDisableCityScan();
	void ServerDisableCopAI();
	void ServerDisableCreatureDecay();
	void ServerDisableCrewAI();
	void ServerDisableIndiAI();
	void ServerDisableJobPosting();
	void ServerDisableMovement();
	void ServerDisableOffAI();
	void ServerDisablePassAI();
	void ServerDisablePirateEnc();
	void ServerDisableReportOff();
	void ServerDisableSetModified();
	void ServerDisableShipScan();
	void ServerDisableStationResupply();
	void ServerDisableStationScan();
	void ServerDisableSystemEmission();
	void ServerDisableTockTransactions();
	void ServerDisableTroopAI();
	void ServerDisableTroopResupply();
	void ServerDisableUnitDecay();
	void ServerDisableWorkerAI();
	void ServerEnableAll();
	void ServerEnableAnimalAI();
	void ServerEnableAnimalEnc();
	void ServerEnableCharAI();
	void ServerEnableCitizenPath();
	void ServerEnableCityScan();
	void ServerEnableCopAI();
	void ServerEnableCreatureDecay();
	void ServerEnableCrewAI();
	void ServerEnableIndiAI();
	void ServerEnableJobPosting();
	void ServerEnableMovement();
	void ServerEnableOffAI();
	void ServerEnablePassAI();
	void ServerEnablePirateEnc();
	void ServerEnableReportOff();
	void ServerEnableSetModified();
	void ServerEnableShipScan();
	void ServerEnableStationResupply();
	void ServerEnableStationScan();
	void ServerEnableSystemEmission();
	void ServerEnableTockTransactions();
	void ServerEnableTroopAI();
	void ServerEnableTroopResupply();
	void ServerEnableUnitDecay();
	void ServerEnableWorkerAI();
	void SetAtomModified();
	void SetControlDecay();
	void SetUDPDecay();
	void ShutdownServer();
	void SpacecraftQuery();
	void Start10Sec();
	void Start15Sec();
	void Start1Sec();
	void Start2Sec();
	void Start30Sec();
	void Start3Sec();
	void Start5Sec();
	void Start60Sec();
	void StopMonitoring();
	void StoryReloadAllScripts();
	void StoryReloadScript();
	void SystemIdQuery();
	void SystemNameQuery();
	void TickClearAtomIdExclusions();
	void TickClearAtomTypeExclusions();
	void TickDisable();
	void TickEnable();
	void TickEnableAtomId();
	void TickEnableAtomType();
	void TickExcludeAtomId();
	void TickExcludeAtomType();
	void TimerUpdate();
	void TockClearAtomIdExclusions();
	void TockClearAtomTypeExclusions();
	void TockDisable();
	void TockEnable();
	void TockEnableAtomId();
	void TockEnableAtomType();
	void TockExcludeAtomId();
	void TockExcludeAtomType();
	void UnitSearchResult(QString serverName, quint16 serverPort, quint32 unitId, QString unitName);
	void WorldIdQuery();
	void WorldNameQuery();

protected:
	quint32 AtomId(const QString& atomIdS) const;
	int ConnectToServer(int serverMode, const QString& listName, const QString& netName, quint16 netPort, int sortOrder,
			int msConnectDelay);
	QIcon FireIcon();
	QString MakeServerTitle(const QString& curTitle, const QString& suffix) const;
	virtual void resizeEvent(QResizeEvent*);
	QString ResourceReportCommon(const AuResourceReport& report);
	void Servers(QList<int>& rows, QStringList& serverTitles, QStringList& serverNames, QList<quint16>& serverPorts) const;
	void ShowMenu(const QList<int>& rows);
	void StartMonitor(int secs);
	void StartMonitoringAll();
	void StopMonitoringAll();
	QString PingIcon(unsigned int offset) const;
	QStringList UniverseDbQuery(UNIVERSE_DB_QUERY q, const AuId& atomId, const QString& name);
	QListWidgetItem* Update(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
			unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock);
};

#endif // AuServerCon_H
