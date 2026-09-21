//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daarena.h,v 1.45 2022/05/17 15:15:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaArena_H
#define DaArena_H

#include "daatom.h"

class AUATOM_DLL DaArena : public DaAtom  
{
public:
	enum MATCH_STATE
	{
		Join,
		Play,
		Aftermath,
	};
	enum MATCH_TYPE
	{
		SpacecraftDuel,
		SpacecraftTrial,
	};
	enum SPAWN_OPTION
	{
		No,
		Initial,
		Respawn,
	};

private:
	quint8								m_MatchState;			// A MATCH_STATE value.
	quint8								m_MatchType;			// A MATCH_TYPE value.
	bool								m_MaximizeShipQ;
	quint8								m_MaxShipQ;
	QString								m_Name;					// Name of arena instance. e.g. Name of the person who created it.
	bool								m_OpponentDesignsOk;	// True if opponents can choose their own designs.
	AuElapsedTimer						m_Position;
	quint32								m_ShipDesign;			// Ship design for all players, or default for players w/o designs.
	quint8								m_SpawnCrew;			// A SPAWN_OPTION value.
	quint8								m_SpawnOfficer;			// A SPAWN_OPTION value.
	quint8								m_SpawnPassengers;		// A SPAWN_OPTION value.
	quint8								m_SpawnTroops;			// A SPAWN_OPTION value.
	quint8								m_SpawnVehicles;		// A SPAWN_OPTION value.
	quint8								m_TeamShips;			// Initial number of ships per team when match started.
	quint8								m_Team1Ships;			// Total number of ships remaining to spawn for team 1.
	quint8								m_Team2Ships;			// Total number of ships remaining to spawn for team 2.

private:
	mutable QMutex						m_PasswordLock;
	QString								m_Password;				// Entry password.

	// Transient data that is not stored in the db.
private:
	mutable QReadWriteLock				m_CommLock;				// Locks m_ShipsHailed and m_tmChannelsMonitored.
	QSet<AuId>							m_ShipsHailed;			// Ships that have hailed during join state.
	AuElapsedTimer						m_tmChannelsMonitored;	// For keeping my comm channels open.

private:
	AuGLVAO								m_ShellRing;			// Vertex array object of a ring of my shell.

public:
	DaArena(const QString& name);
	DaArena();
	virtual ~DaArena();

protected:
	DaArena(const DaArena &rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual DaAtom* Clone() const;

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	virtual QString Description() const;

	inline QString EntryPassword() const
	{
		m_PasswordLock.lock();
		QString s(m_Password);
		m_PasswordLock.unlock();
		return s;
	}

	virtual DaArena* FindParentArena();

	virtual const DaArena* FindParentArena() const;

	virtual DaBd* FindParentBdInEnv();

	virtual const DaBd* FindParentBdInEnv() const;

	virtual DaCapsule* FindParentCapsuleInEnv();

	virtual const DaCapsule* FindParentCapsuleInEnv() const;

	virtual DaDesignBase* FindParentDesignBaseInEnv();

	virtual const DaDesignBase* FindParentDesignBaseInEnv() const;

	virtual DaDesigner* FindParentDesignerInEnv();

	virtual const DaDesigner* FindParentDesignerInEnv() const;

	virtual DaWorld* FindParentPlanetInEnv();

	virtual const DaWorld* FindParentPlanetInEnv() const;

	virtual DaSolarSystem* FindParentSolarSystemInEnv();

	virtual const DaSolarSystem* FindParentSolarSystemInEnv() const;

	virtual DaStar* FindParentStarInEnv();

	virtual const DaStar* FindParentStarInEnv() const;

	virtual DaVessel* FindParentVesselInEnv();

	virtual const DaVessel* FindParentVesselInEnv() const;

	virtual DaWorldLL* FindParentWorldInEnv();

	virtual const DaWorldLL* FindParentWorldInEnv() const;

	inline bool HasHailed(const AuId& shipId) const
	{
		// Once the match starts, all ships are considered to have hailed.
		if(MatchState() != Join)
			return true;
		m_CommLock.lockForRead();
		bool v = m_ShipsHailed.contains(shipId);
		m_CommLock.unlock();
		return v;
	}

	inline MATCH_STATE MatchState() const
	{
		return (MATCH_STATE)m_MatchState;
	}

	inline QString MatchStateDesc() const
	{
		return MatchStateDesc(MatchState());
	}

	static QString MatchStateDesc(MATCH_STATE state);

	inline QString MatchStateName() const
	{
		return MatchStateName(MatchState());
	}

	inline static QString MatchStateName(MATCH_STATE state)
	{
		QString states[] =
		{
			tr("Join"),
			tr("In Play"),
			tr("Aftermath"),
		};
		if(state < 0 || state >= sizeof(states)/sizeof(QString))
			return tr("Invalid Match State %1").arg(state);
		return states[state];
	}

	inline MATCH_TYPE MatchType() const
	{
		return (MATCH_TYPE)m_MatchType;
	}

	inline QString MatchTypeDesc() const
	{
		return MatchTypeDesc(MatchType());
	}

	static QString MatchTypeDesc(MATCH_TYPE type);

	inline QString MatchTypeName() const
	{
		return MatchTypeName(MatchType());
	}

	inline static QString MatchTypeName(MATCH_TYPE type)
	{
		QString types[] =
		{
			tr("Spacecraft Duel"),
			tr("Spacecraft Trial"),
		};
		if(type < 0 || (unsigned int)type >= sizeof(types)/sizeof(QString))
			return tr("Invalid Match Type %1").arg(type);
		return types[type];
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// This function is called when a chat message is received on a channel that is being monitored by this atom.
	// This function is also called when orders are issued to units via chat message.
	// 'firstInGroup' is true when this unit is the first in a group of similar units to receive this order message, so only one
	// unit acknowledges the order instead of a large group of them.
	// 'countSupplied' is used when boarding crew and passengers so multiple cities can take a crack at supplying some citizens
	// until the request is filled.
	virtual void RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup=true);

	inline void SetMatchType(MATCH_TYPE mtype)
	{
		m_MatchType = mtype;
	}

	inline void SetMaximizeShipQ(bool maximize)
	{
		m_MaximizeShipQ = maximize;
	}

	inline void SetMaxShipQ(quint8 maxShipQ)
	{
		Q_ASSERT(maxShipQ);
		m_MaxShipQ = maxShipQ;
	}

	inline void SetOpponentDesignsOk(bool opponentDesignsOk)
	{
		m_OpponentDesignsOk = opponentDesignsOk;
	}

	inline void SetPassword(const QString& password)
	{
		m_PasswordLock.lock();
		m_Password = password;
		m_PasswordLock.unlock();
	}

	inline void SetShipDesign(quint32 designId)
	{
		m_ShipDesign = designId;
	}

	inline void SetShipsPerTeam(quint8 ships)
	{
		m_Team1Ships = ships;
		m_Team2Ships = ships;
	}

	inline void SetSpawnCrew(SPAWN_OPTION spawn)
	{
		m_SpawnCrew = spawn;
	}

	inline void SetSpawnOfficer(SPAWN_OPTION spawn)
	{
		m_SpawnOfficer = spawn;
	}

	inline void SetSpawnPassengers(SPAWN_OPTION spawn)
	{
		m_SpawnPassengers = spawn;
	}

	inline void SetSpawnTroops(SPAWN_OPTION spawn)
	{
		m_SpawnTroops = spawn;
	}

	inline void SetSpawnVehicles(SPAWN_OPTION spawn)
	{
		m_SpawnVehicles = spawn;
	}

	void SetupArena(const AuArenaReady& arena);

	// Total ships per team in the match.
	inline int TeamShips() const
	{
		return m_TeamShips;
	}

	// Team 1 ships remaining to spawn.
	inline int Team1Ships() const
	{
		return m_Team1Ships;
	}

	// Team 2 ships remaining to spawn.
	inline int Team2Ships() const
	{
		return m_Team2Ships;
	}

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual bool CanRenderEnvironment() const
	{
		return true;
	}

	virtual bool IsEthereal() const
	{
		return true;
	}

	// Instance universe's prevent some top-down recursive searches from entering the instance.
	virtual bool IsInstanceUniverse() const
	{
		return true;
	}

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);
	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);
	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	virtual void RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, DaPerson* sceneAtom);

public:
	static DaArena* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.

	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

protected:
	void CreateShellRingVAO(AuOpenGLFunctions& GLf);

	void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns, QList<AuGLLight>& lights) const;

	bool IsSpawnLocationBlocked(const AuPoint& loc, const AuId& ignoreId, double spawnRad) const;

	void PositionTeamMembers(quint8 team, bool emptyShipsOnly);

	void RenderFieldMarkers(const AuMatrix4x4& projection, const AuMatrix4x4& model, float textBrightness, AuGLEffects& re);

	void RenderShell(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight);

	void SetupLights(DaPerson* sceneAtom, const AuMatrix4x4& model, AuGLEffects& re);

	// 'errcode' is an output parameter. It will be set to one of the following values:
	// 0 = success
	// 1 = an unlikely error occurred
	// 2 = design not loaded, try again
	// 3 = quality design is too high for arena rules
	// 4 = design not ok to manufacture
	DaVessel* SpawnSpacecraft(quint8 team, quint32 designId, quint8& errcode);

	bool SpawnTeamShip(quint8 team);
};

#endif // DaArena_H
