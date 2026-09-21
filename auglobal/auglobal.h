//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglobal.h,v 1.385 2022/05/19 16:12:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGlobal_H
#define AuGlobal_H

#include "au3dsglsldna.h"
#include "auarenaready.h"
#include "auatomticktock.h"
#include "aucommstdchannel.h"
#include "audice.h"
#include "auelapsedtimer.h"
#include "auemoji.h"
#include "augloballib.h"
#include "auglobell.h"
#include "auhash.h"
#include "auid.h"
#include "aumailmsg.h"
#include "ausettings.h"
#include "austarmapdata.h"
#include "autime.h"
#include "dbcommodity.h"

using namespace std;
#include <unordered_map>

class AuAtomChatList;
class AuAtomManagerDispatch;
class AuCommPacket;
class AuEther;
class AuExchangeRatingRec;
class AuGLModelCache;
class AuJobOffers;
class AuScripts;
class AuShipPathHash;
class AuStories;
class AuStory;
class DaAtom;
class DgData;
class DtSceneServer;
class QUrl;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuHengeDestination
{
public:
	AuGlobeLLF							m_HengeLL;
	AuId								m_SectorId;
	AuId								m_SystemId;
	QString								m_SystemName;
	quint8								m_Sun;
	QString								m_WorldCatName;			// Catalog name for identification. Can't use AuId here.
	QString								m_WorldName;			// Designated world name.
	quint8								m_WorldType;			// DaAtom::Type

	// Information useful for stories looking for henge destinations.
	bool								m_Breathable;			// True if the destination world's atmosphere is breathable.
	quint8								m_OrbitZone;			// Orbit zone of the destination world.
	bool								m_OutsideTown;			// True if henge is far from any developments.

public:	// Constructors
	inline AuHengeDestination(const AuId& sectorId, const AuId& systemId, const QString& systemName, quint8 sun,
			const QString& worldCatName, const QString& worldName, quint8 worldType, const AuGlobeLLF& hengeLL, bool breathable,
			ORBITZONE orbitZone, bool outsideTown)
	{
		m_HengeLL = hengeLL;
		m_SectorId = sectorId;
		m_SystemId = systemId;
		m_SystemName = systemName;
		m_Sun = sun;
		m_WorldCatName = worldCatName;
		m_WorldName = worldName;
		m_WorldType = worldType;

		m_Breathable = breathable;
		m_OrbitZone = orbitZone;
		m_OutsideTown = outsideTown;
	}

	inline AuHengeDestination()
	{
		m_Breathable = 0;
		m_OrbitZone = OZONE_Inferno;
		m_OutsideTown = false;
		m_Sun = 0;
		m_WorldType = 0;
	}

	inline AuHengeDestination(const AuHengeDestination& c)
	{
		m_Breathable = c.m_Breathable;
		m_HengeLL = c.m_HengeLL;
		m_OrbitZone = c.m_OrbitZone;
		m_OutsideTown = c.m_OutsideTown;
		m_SectorId = c.m_SectorId;
		m_SystemId = c.m_SystemId;
		m_SystemName = c.m_SystemName;
		m_Sun = c.m_Sun;
		m_WorldCatName = c.m_WorldCatName;
		m_WorldName = c.m_WorldName;
		m_WorldType = c.m_WorldType;
	}

public:	// Operators
	inline const AuHengeDestination& operator=(const AuHengeDestination& c)
	{
		m_Breathable = c.m_Breathable;
		m_HengeLL = c.m_HengeLL;
		m_OrbitZone = c.m_OrbitZone;
		m_OutsideTown = c.m_OutsideTown;
		m_SectorId = c.m_SectorId;
		m_SystemId = c.m_SystemId;
		m_SystemName = c.m_SystemName;
		m_Sun = c.m_Sun;
		m_WorldCatName = c.m_WorldCatName;
		m_WorldName = c.m_WorldName;
		m_WorldType = c.m_WorldType;
		return *this;
	}

	inline bool operator==(const AuHengeDestination& c) const
	{
		if(m_SectorId != c.m_SectorId)
			return false;
		if(m_SystemId != c.m_SystemId)
			return false;
		if(m_WorldCatName != c.m_WorldCatName)
			return false;
		if(m_WorldType != c.m_WorldType)
			return false;
		if(m_Sun != c.m_Sun)
			return false;
		if(m_Breathable != c.m_Breathable)
			return false;
		if(m_OrbitZone != c.m_OrbitZone)
			return false;
		if(m_OutsideTown != c.m_OutsideTown)
			return false;
		if(m_HengeLL != c.m_HengeLL)
			return false;
		if(m_SystemName != c.m_SystemName)
			return false;
		if(m_WorldName != c.m_WorldName)
			return false;
		return true;
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	// Compares location and ids, but not time updated or names.
	inline bool Is(const AuHengeDestination& c) const
	{
		if(m_SectorId != c.m_SectorId)
			return false;
		if(m_SystemId != c.m_SystemId)
			return false;
		// Sun index does not need to be compared because we are comparing atom ids of the destination world.
		// Sun index is not compared because that is data the henge destination signal sends back to the henge origin world, who
		// did not initially know the sun index in the destination solar system.
		//if(m_Sun != c.m_Sun)
			//return false;
		if(m_WorldCatName != c.m_WorldCatName)
			return false;
		if(m_HengeLL != c.m_HengeLL)
			return false;
		return true;
	}

	inline bool IsHabitable() const
	{
		return m_Breathable && m_OrbitZone == OZONE_Habitable;
	}

	inline bool IsOutsideTown() const
	{
		return m_OutsideTown;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuHengeDestination& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHengeDestination& o)
{
	o << in;
	return in;
}

// Windows seems to want this.
inline unsigned int qHash(const AuHengeDestination& key)
{
	return key.m_SystemId.GetID();
}

class AUGLOBAL_DLL AuHengeDestinations : public QList<AuHengeDestination>
{
public:
	void Update(const AuHengeDestination& hd);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// This class is used to maintain data related to a comm connection monitored by
// a local atom that is not a player's unit. An example of this is when a city
// monitores the "Trade" channel to buy and sell cargo.
// NOTE: These connections are not stored in the sql database so a server
// handoff will cause the affected atoms to lose connections that were
// initiated by a client player.

class AUGLOBAL_DLL AuCommChannelConnection
{
public:
	AuId								m_idAtom;				// Atom monitoring the channel.
	AuElapsedTimer						m_LastUse;				// Last use time for connect decay logic
	AuCommStdChannel::SCOPE				m_Scope;				// Scope of this channel.

public:
	AuCommChannelConnection()
	{
		m_Scope = AuCommStdChannel::Everyone;
	}

	AuCommChannelConnection(const AuCommChannelConnection& c)
	{
		m_idAtom = c.m_idAtom;
		m_LastUse = c.m_LastUse;
		m_Scope = c.m_Scope;
	}

	AuCommChannelConnection& operator=(const AuCommChannelConnection& c)
	{
		m_idAtom = c.m_idAtom;
		m_LastUse = c.m_LastUse;
		m_Scope = c.m_Scope;
		return *this;
	}

	inline bool IsMonitoredBy(const AuId& id) const
	{
		return m_idAtom == id;
	}

	inline bool IsTooOld() const
	{
		// NOTE: City channel last-use times are refreshed in DaCity::Tock. The decay time interval needs to be harmonious with
		// the interval that cities refresh the channel.
		// NOTE: Stations must refresh monitoring the "Hail" channel often enough to keep it alive also.
		return (!m_LastUse.isValid() || m_LastUse.elapsed()/d_MSecPerSec > (20 * 60 + 15)) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// This class is used to queue "open chat channel" requests sent by local atoms.

class AUGLOBAL_DLL AuOpenChannel
{
public:
	QString								m_ChannelName;			// Name of the channel to open.
	bool								m_ConnectBeyondSystem;	// True to connect the channel to atoms outside the solar system.
	GALAXY								m_FromGalaxy;
	QString								m_FromName;
	AuId								m_FromSector;
	AuId								m_idFrom;				// Atom opening the channel.
	AuId								m_idTo;					// Atom to whom the channel is being opened.
	AuCommStdChannel::SCOPE				m_Scope;				// Scope of the channel.

public:
	AuOpenChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const AuId& idFrom, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, const AuId& idTo, bool connectBeyondSolarSystem)
	{
		m_ChannelName = channelName;
		m_ConnectBeyondSystem = connectBeyondSolarSystem;
		m_FromGalaxy = fromGalaxy;
		m_FromName = fromName;
		m_FromSector = fromSector;
		m_idFrom = idFrom;
		m_idTo = idTo;
		m_Scope = scope;
	}
	AuOpenChannel()
	{
		m_ConnectBeyondSystem = false;
		m_FromGalaxy = GALAXY_Count;
		m_Scope = AuCommStdChannel::Everyone;
	}

	AuOpenChannel(const AuOpenChannel& c)
	{
		m_ChannelName = c.m_ChannelName;
		m_ConnectBeyondSystem = c.m_ConnectBeyondSystem;
		m_FromGalaxy = c.m_FromGalaxy;
		m_FromName = c.m_FromName;
		m_FromSector = c.m_FromSector;
		m_idFrom = c.m_idFrom;
		m_idTo = c.m_idTo;
		m_Scope = c.m_Scope;
	}

	AuOpenChannel& operator=(const AuOpenChannel& c)
	{
		m_ChannelName = c.m_ChannelName;
		m_ConnectBeyondSystem = c.m_ConnectBeyondSystem;
		m_FromGalaxy = c.m_FromGalaxy;
		m_FromName = c.m_FromName;
		m_FromSector = c.m_FromSector;
		m_idFrom = c.m_idFrom;
		m_idTo = c.m_idTo;
		m_Scope = c.m_Scope;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// From OpenAL documentation on the default Inverse Distance Rolloff Clamped (AL_INVERSE_DISTANCE_CLAMPED) model.
//   distance = max(distance, AL_REFERENCE_DISTANCE)
//   distance = min(distance, AL_MAX_DISTANCE)
//   gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance – AL_REFERENCE_DISTANCE))
class AuSoundOverride
{
private:	// Properties
	QString								m_FileName;				// Name of a .wav file that overrides the default sound.
	double								m_ReferenceDistance;	// In feet.
	float								m_RolloffFactor;

public:	// Constructors
	inline AuSoundOverride(const QString& fileName, double referenceDistance=FeetPerMeter(), float rolloffFactor=1.f)
	{
		m_FileName = fileName;
		m_ReferenceDistance = referenceDistance;
		m_RolloffFactor = rolloffFactor;
	}

	inline AuSoundOverride()
	{
		m_ReferenceDistance = FeetPerMeter();
		m_RolloffFactor = 1;
	}

	inline AuSoundOverride(const AuSoundOverride& c)
	{
		m_FileName = c.m_FileName;
		m_ReferenceDistance = c.m_ReferenceDistance;
		m_RolloffFactor = c.m_RolloffFactor;
	}

public:	// Operators
	inline const AuSoundOverride& operator=(const AuSoundOverride& c)
	{
		m_FileName = c.m_FileName;
		m_ReferenceDistance = c.m_ReferenceDistance;
		m_RolloffFactor = c.m_RolloffFactor;
		return *this;
	}

	inline bool operator==(const AuSoundOverride& c) const
	{
		if(m_FileName != c.m_FileName)
			return false;
		if(m_ReferenceDistance != c.m_ReferenceDistance)
			return false;
		if(m_RolloffFactor != c.m_RolloffFactor)
			return false;
		return true;
	}

	inline bool operator!=(const AuSoundOverride& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline void Clear()
	{
		m_FileName = QString();
		m_ReferenceDistance = FeetPerMeter();
		m_RolloffFactor = 1;
	}

	inline const QString& FileName() const
	{
		return m_FileName;
	}

	// Returns reference distance, in feet.
	inline double ReferenceDistance() const
	{
		return m_ReferenceDistance;
	}

	inline float RolloffFactor() const
	{
		return m_RolloffFactor;
	}

	inline void SetFileName(const QString& fileName)
	{
		m_FileName = fileName;
	}

	inline void SetReferenceDistance(double refDist)
	{
		m_ReferenceDistance = refDist;
	}

	inline void SetRolloffFactor(float rolloff)
	{
		m_RolloffFactor = rolloff;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuGlobal
{
public:
	enum MODE
	{
		LoginMaster,
		LoginWorker,
		MailClient,
		MailMaster,
		MailWorker,
		SceneClient,
		SceneManager,
		SceneMaster,
		SceneWorker,
		WebCGI,
		SceneSolo,
	};

	enum RATING
	{
		Everyone,					// No mating. No nudity. e.g. Steam version.
		Teen,						// No mating. Nudity ok.
		Mature,						// Mating ok. Nudity only while mating.
		Adult,						// Mating ok. Nudity ok.
	};

public:
	DbCommodityArray	m_CommodityArray;	// Commodities data gets loaded (in DgData) and then it never changes. No mutex needed.

	// Public for convenience. Use the atom chat mutex to access channel open queue.
	QList<AuOpenChannel>				m_OpenChannels;			// Channel open requests initiated by local atoms.

private:
	QString								m_AppName;
	QString								m_BaseDir;				// e.g. ~/Shores of Hazeron
	int									m_BuildingViewRange;	// Meters range that buildings are visible.
	int									m_BuildingViewRangeFt;	// Feet range that buildings are visible.
	quint64								m_BuildingViewRangeFt2;	// Feet squared range that buildings are visible.
	bool								m_BumpMapSeas;			// True to use bump mapping on seas.
	QString								m_CacheDir;				// e.g. /home/daveh/Shores of Hazeron/Temporary
	QString								m_ClientChannelServer;	// Name of boss server.
	quint16								m_ChatPort;				// Only used on the scene worker server, for chat traffic.
	QString								m_Company;
	quint32								m_ControlAverageSpanMs;	// Timespan over which to average mouse control inputs. 0 to 500ms.
	quint32								m_ControlDecayMargin;	// Milliseconds margin used when calculating control decay.
	DgData*								m_Data;					// Data used by all applications to varying degrees. May be 0.
	int									m_DetailedTerrainRange;	// Meters range to generate detailed terrain.
	int									m_DetailedTerrainRangeFt;	// Feet range to generate detailed terrain.
	int									m_DetailedTerrainRangeFt2;	// Feet squared range to generate detailed terrain.
	AuEmojiList							m_Emojis;
	QString								m_GameSaveDir;			// e.g. ~/Shores of Hazeron/Saved Games
	int									m_GrassViewRange;		// Meters range that grass is visible.
	int									m_GrassViewRangeFt;		// Feet range that grass is visible.
	int									m_GrassViewRangeFt2;	// Feet squared range that grass is visible.
	int									m_LeftRightCursor;		// Hand cursor 0=handedness, 1=left, 2=right
	int									m_LeftRightOrientation;	// Orientation of screen elements 0=handedness, 1=left, 2=right
	bool								m_MailCityReportAutoDelete;		// true to autodelete old city reports
	int									m_MailCityReportOldAge;			// age to autodelete old city reports, in hours
	bool								m_MailFleetReportAutoDelete;	// true to autodelete old fleet reports
	int									m_MailFleetReportOldAge;		// age to autodelete old city reports, in hours
	int									m_MailNotificationMode;			// 0=off, 1=startup, 2=intervals
	bool								m_MailOfficerReportAutoDelete;	// true to autodelete old officer reports
	int									m_MailOfficerReportOldAge;		// age to autodelete old city reports, in hours
	QString								m_PictureDir;			// e.g. ~/Shores of Hazeron/Pictures
	RATING								m_Rating;
	int									m_RockViewRange;		// Meters range that grass is visible.
	int									m_RockViewRangeFt;		// Feet range that grass is visible.
	int									m_RockViewRangeFt2;		// Feet squared range that grass is visible.
	int									m_ShrubViewRange;		// Meters range that grass is visible.
	int									m_ShrubViewRangeFt;		// Feet range that grass is visible.
	int									m_ShrubViewRangeFt2;	// Feet squared range that grass is visible.
	int									m_TreeViewRange;		// Meters range that grass is visible.
	int									m_TreeViewRangeFt;		// Feet range that grass is visible.
	int									m_TreeViewRangeFt2;		// Feet squared range that grass is visible.
	int									m_WorkerThreadCount;	// Maximum number of worker threads permitted.
	AuDice								m_Dice;					// Random number generator;
	int									m_DivTexels;			// Texels per terrain division in planet surface texture.
	bool								m_EnableJoystick[6];	// Six joysticks should be enough.
	QFont								m_FontAtom;				// On-screen font for atoms, e.g. city beacons and dude names.
	QFont								m_FontDesigner;			// On-screen font for designer.
	float								m_SoundEffectsGain;		// Overall gain for sound effects.
	QHash<int, float>					m_SoundGain;			// Sound gain setting when not 1. Key is SOUND_EFFECT.
	QHash<int, AuSoundOverride>			m_SoundOverrides;		// Sound overrides, when overridden. Key is SOUND_EFFECT.
	mutable QString						m_TypefaceSans;			// Name of default sans serif font.
	mutable QString						m_TypefaceSerif;		// Name of default serif font.
	quint32								m_UdpDecayMargin;		// Milliseconds margin used when calculating udp packet decay.
	bool								m_UseNativeColorDialog;
	bool								m_UseNativeFileDialog;
	bool								m_UseNativeFontDialog;

	int									m_DrawBody;				// 0=no, 1=yes, 2=headless
	int									m_DrawClouds;			// 0=no, 1=thin, 2=thick, 3=thickest
	bool								m_DrawGun;
	bool								m_DrawFramedPictures;	// Download pictures for picture frames.
	bool								m_DrawHelmetHUDAlways;	// When true, helmet HUD appears even if screen elements are off.
	bool								m_DrawMiniMe;			// Show mini me on screen next to health bars.
	bool								m_DrawNebulaCloud;
	bool								m_DrawSpecks;
	bool								m_DrawSkyBox;
	bool								m_SkyBoxMipMap;
	unsigned int						m_SkyBoxPaneSize;		// Pixel edge size of sky box panes.
	bool								m_DrawWeatherEffects;
	int									m_FramedPictureSize;	// Maximum size of a framed picture.
	int									m_ResourceIconSize;		// modifier for the top down view sprite sizes. Percentage of size.
	int									m_BodyStatSize;			// modifier for the body/vehicle status bar sizes. % of max size.
	double								m_FieldOfViewXCos;		// Cosine of 1/2 X field of view angle. For clipping, not stored.
	double								m_FieldOfViewY;			// Y field of view angle, in radians.
	QString								m_LoginMasterName;
	QByteArray							m_MailBuffer;			// Stream buffer for mail Tcp packets.
	QString								m_MailMasterName;
	quint16								m_ManagerPort;			// Port on which we listen for server manager connections.
	MODE								m_Mode;					// mode of operation
	QMutex								m_muMail;
	QMutex								m_muMailBuffer;
	QMutex								m_muMailDelQ;
	QString								m_NetName;				// Internet resolvable name to this computer.
	unsigned int						m_OpacityActive;		// Opacity of active window. 0-100
	unsigned int						m_OpacityInactive;		// Opacity of inactive window. 0-100
	QReadWriteLock						m_rwData;				// Guards most of the data in DgData
	QReadWriteLock						m_rwBdDesigns;			// Guards the bd building design data in DgData
	QReadWriteLock						m_rwBdExchange;			// Guards the bd library data in DgData
	QReadWriteLock						m_rwItDesigns;			// Guards the it item design data in DgData
	QReadWriteLock						m_rwScDesigns;			// Guards the sc spacecraft design data in DgData
	QReadWriteLock						m_rwScExchange;			// Guards the sc library data in DgData
	QString								m_SceneMasterName;
	quint16								m_ScenePort;
	QString								m_SceneServerName;
	quint8								m_SceneServerMode;		// 0=master, 1=manager, 2=worker
	qint64								m_ServerMSecSkew;		// milliseconds skew = serverTime - localTime.
	quint16								m_ServerPort;
	AuSettings							m_Settings;

	bool								m_DesignerFindEdges;
	bool								m_DesignerFindFaces;
	bool								m_DesignerFindVertices;
	QColor								m_DesignerBackColor;	// Color of back faces in object mode.
	bool								m_DesignerBlockPaths;
	bool								m_DesignerGravityOn;
	QColor								m_DesignerGridColor;	// Color of design grid lines.
	QColor								m_DesignerGridXColor;	// Color of X axis line and text
	QColor								m_DesignerGridYColor;	// Color of Y axis line and text
	QColor								m_DesignerGridZColor;	// Color of Z axis line and text
	QColor								m_DesignerLadderColor;	// Color of ladder paths.
	QColor								m_DesignerLineColor;	// Color of edge lines when not selected.
	QColor								m_DesignerSelColor;		// Color of edge lines when selected.
	int									m_DesignerVertexSize;	// Size of sprites at selected vertex in designer, in pixels.
	QColor								m_DesignerWalkColor;	// Color of walk paths.
	int									m_GroundCoverQty;		// Maximum number of ground cover plants per terrain tile.
	int									m_GroundTextureSize;	// Edge size of each square WORLDTEX_TYPE texture.
	unsigned int						m_MaxLights;			// Maximum number of simultaneous point light sources.
	int									m_PlantBranchMaxSegs;	// Max tree branch segments in a plant texture.
	int									m_PlantTextureSize;		// Edge size of each square plant specie texture.
	float								m_TreeTrunkMinDia;		// Minimum diameter branch to make a 3D model.
	int									m_TreeTrunkFaces;		// Faces in each tree trunk segment.
	int									m_TreeTrunkMaxSegs;		// Mac number of tree trunk segments before branches begin anyway.

	mutable QReadWriteLock				m_rwModelsGL;			// Guards m_ModelsGL.
	AuGLModelCache*						m_ModelsGL;				// Geometry for collisions, etc.

	mutable QReadWriteLock				m_rwModels;				// Guards m_Models, m_FootOffset and m_HandOffset.
	QMap<Au3dsGLSLDNA, AuPoint> 		m_FootOffset;			// Foot offset for this dna
	QMap<Au3dsGLSLDNA, AuPoint> 		m_HandOffset;			// Hand offset for this dna

private:
	AuAtomManagerDispatch*				m_AtomManagerDispatch;

protected:
	AuEther*							m_Etherium;				// The active/live etherium to atoms stored locally. 0 in web apps.

private:
	// Arenas
	QReadWriteLock						m_ArenaLock;			// Controls m_Arenas.
	unordered_map<AuId, AuArenaReady, AuIdHash>	m_Arenas;		// Arenas waiting for players.

private:
	// Henge destinations.
	QReadWriteLock						m_rwHengeDestinations;
	AuHengeDestinations					m_HengeDestinations;	// Destination worlds of ringworld henge portals.

private:
	QMutex								m_muAtomChannels;		// Guards m_CommChannels.
	unordered_multimap<QString, AuCommChannelConnection, AuStringHash> m_CommChannels;	// Atom comm channels, key=name.

private:
	// Tickers get very high frequency processing, for movement physics and very little else.
	mutable QReadWriteLock				m_AtomTickerLock;
	mutable bool						m_AtomTickerReleased;
	AuAtomTickerHash					m_AtomTickers;			// Atoms who want calls to Tick.

private:
	// Tockers get variable frequency processing. e.g. automatic doors, bleeding, environmental effects, mfg processes.
	mutable QReadWriteLock				m_AtomTockerLock;
	mutable bool						m_AtomTockerReleased;
	AuAtomTockerHash					m_AtomTockers;			// Atoms who want calls to Tock.

private:
	mutable QMutex						m_Net1Lock;
	int									m_Net1Downloading;
	int									m_Net1Uploading;

private:
	mutable QMutex						m_Net2Lock;
	int									m_Net2Downloading;
	int									m_Net2Uploading;

private:
	mutable QMutex						m_Net3Lock;
	int									m_Net3Downloading;
	int									m_Net3Uploading;

private:
	mutable QMutex						m_Net4Lock;
	int									m_Net4Downloading;
	int									m_Net4Uploading;

public:
	// Debug switches.
	quint32			m_DebugSwitches;	// Bit switches; see usage. No enum to prevent rebuilds when I decide to use one.
	bool			m_EnableAnimalAI;
	bool			m_EnableAnimalEncounters;
	bool			m_EnableCharacterAI;
	bool			m_EnableCitizenPathing;
	bool			m_EnableCopAI;
	bool			m_EnableCreatureDecay;
	bool			m_EnableCrewAI;
	bool			m_EnableIndigenousEncounters;
	bool			m_EnableIndigenousAI;
	bool			m_EnableJobPosting;
	bool			m_EnableMovement;
	bool			m_EnableOfficerAI;
	bool			m_EnablePassengerAI;
	bool			m_EnablePirateEncounters;
	bool			m_EnableReportOfficers;
	bool			m_EnableSensorScanByCity;
	bool			m_EnableSensorScanByShip;
	bool			m_EnableSensorScanByStation;
	bool			m_EnableSetModified;
	bool			m_EnableStationResupply;
	bool			m_EnableSystemEmission;
	bool			m_EnableTick;
	bool			m_EnableTock;
	bool			m_EnableTroopAI;
	bool			m_EnableTroopResupply;
	bool			m_EnableUnitDecay;
	bool			m_EnableWorkerAI;
	AuIdList		m_ExcludeFromTick;
	AuIdList		m_ExcludeFromTock;
	QList<int>		m_ExcludeFromTickType;						// Atom type to exclude from tick.
	QList<int>		m_ExcludeFromTockType;						// Atom type to exclude from tock.
	FILE*			m_LogConnectFile;
	QMutex			m_LogConnectFileLock;
	FILE*			m_LogFile;
	QMutex			m_LogFileLock;
	quint32			m_LogMessages;								// Bits are masked LOG_MESSAGE_BITS values.

private:
	// Spacecraft position reports.
	mutable QMutex						m_muScPosLock;
	QMultiHash<AuId, AuStarMapScPos>	m_ScPos;	// Position reports from all spacecraft. These die of old age. Key is system id.

public:
	AuGlobal(const QString& appName,
			const QString& company, MODE mode, DgData* data=0, AuEther* etherium=0,
			QSettings::Format format=QSettings::NativeFormat);

	AuGlobal(const QString& appName, const QString& settingsAppName,
			const QString& company, MODE mode, DgData* data=0, AuEther* etherium=0,
			QSettings::Format format=QSettings::NativeFormat);

	virtual ~AuGlobal();

public:
	virtual const char* Address() const;

	// Adds 'atomId' to the tick queue.
	virtual void AddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId);

	inline AuAtomManagerDispatch* AtomManagerDispatch()
	{
		return m_AtomManagerDispatch;
	}

	int AtomManagerCount() const;

	virtual void ClearEtherium()
	{
	}

	// You must override this function to enable the DtTrash thread to delete atoms.
	// Returns the number of atoms remaining in the trash after deleting.
	virtual size_t DeleteEtheriumTrashAtom()
	{
		return 0;
	}

	// Fills 'emojis' with the current emojis list.
	inline const AuEmojiList& Emojis() const
	{
		return m_Emojis;
	}

	// Returns true if 'atomId' is currently in the tick or tock queue.
	bool InTickTockQueue(const AuId& atomId);

	void LoadDefaultEmojis();

	void LoadEmojis(const QByteArray& ba);

	inline const unordered_map<AuId, AuArenaReady, AuIdHash>& LockArenasForRead()
	{
		m_ArenaLock.lockForRead();
		return m_Arenas;
	}

	inline unordered_map<AuId, AuArenaReady, AuIdHash>& LockArenasForWrite()
	{
		m_ArenaLock.lockForWrite();
		return m_Arenas;
	}

	inline const AuAtomTickerHash& LockAtomTickers() const
	{
		m_AtomTickerLock.lockForRead();
		return m_AtomTickers;
	}

	// Locks the Etherium delete lock and returns a reference to the Etherium.
	// The delete lock insures that no atoms in the ether will be deleted during the lock.
	// The delete lock is a soft/readonly lock. Only the trash thread ever locks it for write, just before deleting an atom.
	// NOTE: Must override this to actually lock the lock. Can't do it here due to circular dependency.
	virtual AuEther& LockEtheriumDelete()
	{
		return *m_Etherium;
	}

	bool LockNextTicker(const AuId& prevAtomId, AuId& atomId, AuId& systemId);

	inline QMultiHash<AuId, AuStarMapScPos>& LockScPositionReports()
	{
		m_muScPosLock.lock();
		return m_ScPos;
	}

	bool LockTicker(const AuId& atomId);

	// Open the file 'fileName' as the log file. The file is kept open for the duration of the program.
	// The file name typically looks like this "serverName.log".
	bool OpenLogFile(const QString& fileName);

	// Opens the file 'fileName' as the connections log file. The file is kept open for the duration of the program.
	// The file name typically looks like this "serverName.connect".
	bool OpenLogConnectFile(const QString& fileName);

	virtual void RateBlueprintBd(const AuExchangeRatingRec& rating)
	{
		Q_UNUSED(rating);
		Q_ASSERT(false);	// Must override to use.
	}

	virtual void RateBlueprintSc(const AuExchangeRatingRec& rating)
	{
		Q_UNUSED(rating);
		Q_ASSERT(false);	// Must override to use.
	}

	// Release 'atomId' from the tick queue.
	void ReleaseTicker(const AuId& atomId) const;

	// Release 'atomId' from the tick and tock queues.
	void ReleaseTickTock(const AuId& atomId) const;

	void RemoveReleasedTickers();

private:
	// Remove 'atomId' from the tick queue.
	// BEWARE: This could remove a locked atom queue item.
	//void RemoveTicker(const AuId& atomId);

public:
	// Replaces emoji tags in 'raw'. E.g. :alarm clock: might be replaced with ⏰, if 'alarm clock' is a tag in the emojis list.
	QString ReplaceEmojiTags(const QString& raw) const;

	inline void SetAtomManagerDispatch(AuAtomManagerDispatch* amd)
	{
		m_AtomManagerDispatch = amd;
	}

	void SetEmojis(const AuEmojiList& emojis);

	// Splits 'textWithEmojis' into a string list of words and/or emojis.
	QStringList SplitWordsAndEmojis(const QString& textWithEmojis) const;

	void StoreEmojis(QByteArray& ba) const;

	// The return value is in 1/2ms increments.
	unsigned int SumOfAverageTickTimes() const;

	// The return value is in 1/2ms increments.
	unsigned int SumOfAverageTockTimes() const;

	void TallyAtomTickerSystemTimes(QHash<AuId, unsigned int>& systemTotals,
			QHash<AuId, unsigned int>& systemTotalsCount,
			QHash<AuId, unsigned int>& systemMax,
			QHash<AuId, unsigned int>& systemMaxCount,
			QHash<AuId, unsigned int>& systemCycle,
			QHash<AuId, unsigned int>& systemCycleCount,
			QHash<AuId, unsigned int>& systemCycleMax,
			QHash<AuId, unsigned int>& systemCycleMaxCount) const;

	// Does not test for deadlocks.
	void TallyAtomTockerSystemStatus(QHash<AuId, unsigned int>& systemTotals) const;

	// Does test for deadlocks.
	void TallyAtomTockerSystemTimes(QHash<AuId, unsigned int>& systemTotals,
			QHash<AuId, unsigned int>& systemMax) const;

	// Does test for deadlocks.
	void TallyAtomTockerSystemTimes(QHash<AuId, unsigned int>& systemTotals,
			QHash<AuId, unsigned int>& systemTotalsCount,
			QHash<AuId, unsigned int>& systemMax,
			QHash<AuId, unsigned int>& systemMaxCount) const;

	inline void UnlockArenas()
	{
		m_ArenaLock.unlock();
	}

	virtual void UnlockEtheriumDelete()
	{
	}

	inline void UnlockScPositionReports()
	{
		m_muScPosLock.unlock();
	}

	void UnlockTicker(const AuId& atomId);

	void UnlockTicker(const AuId& atomId, unsigned int msElapsed);

	inline void UnlockAtomTickers() const
	{
		m_AtomTickerLock.unlock();
	}

	// Changes the 'systemId' in existing ticker. Adds ticker if it is new.
	virtual void UpdateOrAddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId);

	// Changes the 'systemId' in the ticker and restores it from being released.
	// Returns true if the atom ticker is updated, false if it does not exist.
	bool UpdateTicker(const AuId& atomId, const AuId& systemId) const;

	bool UseNativeColorDialog() const;

	bool UseNativeFileDialog() const;

	bool UseNativeFontDialog() const;

	// Adds 'atomId' to the tock queue.
	virtual void AddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs);

	inline const AuAtomTockerHash& LockAtomTockers() const
	{
		m_AtomTockerLock.lockForRead();
		return m_AtomTockers;
	}

	bool LockNextTocker(const AuId& prevAtomId, AuId& atomId, AuId& systemId);

	bool LockTocker(const AuId& atomId);

	// Release 'atomId' from the tock queue.
	void ReleaseTocker(const AuId& atomId) const;

	//void RemoveReleasedTockers();

private:
	// Remove 'atomId' from the tock queue.
	// BEWARE: This could remove a locked atom queue item.
	//void RemoveTocker(const AuId& atomId);

public:
	void UnlockTocker(const AuId& atomId);

	void UnlockTocker(const AuId& atomId, unsigned int msElapsed);

	inline void UnlockAtomTockers() const
	{
		m_AtomTockerLock.unlock();
	}

	// Changes the 'systemId' and/or 'desiredCycleMs' in the tocker. Adds tocker if it is new.
	virtual void UpdateOrAddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs);

	// Changes the 'systemId' and/or 'desiredCycleMs' in the tocker and restores it from being released.
	// Returns true if the atom tocker is updated, false if it does not exist.
	bool UpdateTocker(const AuId& atomId, const AuId& systemId, unsigned int desiredCycleMs) const;

	inline const QString& ApplicationName() const
	{
		return m_AppName;
	}

	virtual void AtomAdded(const AuId& /*id*/)
	{
		Q_ASSERT(false);	// You must override this function to use it. See AuDbGlobal
	}

	virtual void AtomChanged(const AuId& /*id*/)
	{
		Q_ASSERT(false);	// You must override this function to use it. See AuDbGlobal
	}

	virtual void AtomDeleted(const AuId& /*id*/)
	{
		Q_ASSERT(false);	// You must override this function to use it. See AuDbGlobal
	}

	virtual void AtomDeparted(const AuId& /*id*/)
	{
		Q_ASSERT(false);	// You must override this function to use it. See AuDbGlobal
	}

	inline int BuildingViewRange() const
	{
		return m_BuildingViewRange;
	}

	inline int BuildingViewRangeFt() const
	{
		return m_BuildingViewRangeFt;
	}

	inline quint64 BuildingViewRangeFt2() const
	{
		return m_BuildingViewRangeFt2;
	}

	inline int BumpMapSeas() const
	{
		return m_BumpMapSeas;
	}

	// Overridden in the client to get images for picture frames off the internet.
	virtual QImage DownloadImage(const QUrl& pictureURL)
	{
		Q_UNUSED(pictureURL);
		return QImage();
	}

	QString GameName() const;

	inline int Net1Downloading() const
	{
		m_Net1Lock.lock();
		int v = m_Net1Downloading;
		m_Net1Lock.unlock();
		return v;
	}

	inline int Net1Uploading() const
	{
		m_Net1Lock.lock();
		int v = m_Net1Uploading;
		m_Net1Lock.unlock();
		return v;
	}

	inline int Net2Downloading() const
	{
		m_Net2Lock.lock();
		int v = m_Net2Downloading;
		m_Net2Lock.unlock();
		return v;
	}

	inline int Net2Uploading() const
	{
		m_Net2Lock.lock();
		int v = m_Net2Uploading;
		m_Net2Lock.unlock();
		return v;
	}

	inline int Net3Downloading() const
	{
		m_Net3Lock.lock();
		int v = m_Net3Downloading;
		m_Net3Lock.unlock();
		return v;
	}

	inline int Net3Uploading() const
	{
		m_Net3Lock.lock();
		int v = m_Net3Uploading;
		m_Net3Lock.unlock();
		return v;
	}

	inline int Net4Downloading() const
	{
		m_Net4Lock.lock();
		int v = m_Net4Downloading;
		m_Net4Lock.unlock();
		return v;
	}

	inline int Net4Uploading() const
	{
		m_Net4Lock.lock();
		int v = m_Net4Uploading;
		m_Net4Lock.unlock();
		return v;
	}

	// Derived class with access to DgData must return the empire name.
	// A blank string is returned when the empireId is 0.
	// 'Forgotten Empire N' is returned when empireId is non-zero but the empire record cannot be found.
	virtual QString EmpireName(quint32 /*empireId*/) const
	{
		Q_ASSERT(false);	// You must override this function to use it. See AuDbGlobal
		return "";
	}

	inline const QFont& FontAtom() const
	{
		return m_FontAtom;
	}

	inline const QFont& FontDesigner() const
	{
		return m_FontDesigner;
	}

	// Returns m_CacheDir;
	QString CacheApp() const;

	// Returns m_CacheDir/Solo if IsClientSolo, otherwise returns m_CacheDir.
	QString CacheGame() const;

	// Returns dir for mail cache. Differs between solo and mmo.
	QString CacheMail() const;

	// Returns m_CacheDir/net.
	inline QString CacheNet() const
	{
		return m_CacheDir + "/net";
	}

	// Returns m_CacheDir/Solo
	QString CacheSolo() const;

	inline void ClearAllCacheFiles()
	{
		ClearCacheFiles(0);
		ClearCacheFiles(1);
	}

	// 'which' 0 = m_CacheDir, 1 = m_CacheDir/Solo
	void ClearCacheFiles(int which);

	// Clears m_CacheDir/Solo
	inline void ClearSoloCacheFiles()
	{
		ClearCacheFiles(1);
	}

	inline QString BaseDir() const
	{
		return m_BaseDir;
	}

	inline QString GameSaveDir() const
	{
		return m_GameSaveDir;
	}

	inline QString PictureDir() const
	{
		return m_PictureDir;
	}

	// Commodities
	inline const DbCommodity& Commodity(COMMODITYID cid) const
	{
		// Redirect obsolete computer to new computer, to prevent crash.
		if(cid == CID_ComputerObsolete)
			cid = CID_Computer;

		Q_ASSERT(cid < m_CommodityArray.count());
		Q_ASSERT(m_CommodityArray[cid].m_id == cid);
		return m_CommodityArray[cid];
	}

	COMMODITYID CommodityId(const QString& name) const;

	// Safely returns the name of the icon image associated with a commodity id.
	// Returns empty string when the commodity id is invalid.
	inline QString CommodityBitmapName(COMMODITYID cid) const
	{
		if(cid >= CID_Invalid && cid < m_CommodityArray.count())
			return Commodity(cid).BitmapName();

		// Return empty string when the commodity id is invalid.
		qWarning() << "AuGlobal::CommodityBitmapName cid" << cid << "INVALID!";
		return QString("");
	}

	// Safely returns the path to the 3ds model associated with a commodity id.
	// Returns empty string when the commodity id is invalid.
	inline QString CommodityModel(COMMODITYID cid) const
	{
		if(cid >= CID_Invalid && cid < m_CommodityArray.count())
			return Commodity(cid).ModelName();

		// Return empty string when the commodity id is invalid.
		qWarning() << "AuGlobal::CommodityModel cid" << cid << "INVALID!";
		return QString("");
	}

	QString CommodityName(COMMODITYID cid) const;

	// Returns the base cost of a commodity modified by quality.
	inline float CommodityQualityCost(COMMODITYID idCommodity, quint8 quality) const
	{
		return m_CommodityArray.CommodityQualityCost(idCommodity, quality);
	}

	inline quint32 ControlAverageSpanMs() const
	{
		return m_ControlAverageSpanMs;
	}

	inline quint32 ControlDecayMargin() const
	{
		return m_ControlDecayMargin;
	}

	// Creates the CacheApp() directory if it does not already exist.
	// Returns true if the directory exists, whether or not it was created by this call.
	// Returns false if the directory could not be created.
	bool CreateCacheApp() const;

	// Creates the CacheGame() directory if it does not already exist.
	// Returns true if the directory exists, whether or not it was created by this call.
	// Returns false if the directory could not be created.
	bool CreateCacheGame() const;

	// Creates the GameSaveDir() directory if it does not already exist.
	// Returns true if the directory exists, whether or not it was created by this call.
	// Returns false if the directory could not be created.
	bool CreateGameSaveDir() const;

	// Creates the CacheSolo() directory if it does not already exist.
	// Returns true if the directory exists, whether or not it was created by this call.
	// Returns false if the directory could not be created.
	bool CreateCacheSolo() const;

	// Decodes a single UTC string in 'encodedUTC' into local datetime format.
	// 'encodedUTC' is expected to be of the same form as returned by EncodeUTCString.
	// If the format looks wrong, 'encodedUTC' is returned unchanged.
	QString DecodeUTCString(const QString& encodedUTC) const;

	// Decodes all UTC strings in 'encodedUTCs' into local datetime format.
	QString DecodeUTCStrings(const QString& encodedUTCs) const;

	inline int DetailedTerrainRange() const
	{
		return m_DetailedTerrainRange;
	}

	inline int DetailedTerrainRangeFt() const
	{
		return m_DetailedTerrainRangeFt;
	}

	inline int DetailedTerrainRangeFt2() const
	{
		return m_DetailedTerrainRangeFt2;
	}

	inline int GrassViewRange() const
	{
		return m_GrassViewRange;
	}

	inline int GrassViewRangeFt() const
	{
		return m_GrassViewRangeFt;
	}

	inline int GrassViewRangeFt2() const
	{
		return m_GrassViewRangeFt2;
	}

	inline int LeftRightCursor() const
	{
		return m_LeftRightCursor;
	}

	inline int LeftRightOrientation() const
	{
		return m_LeftRightOrientation;
	}

	inline bool MailCityReportAutoDelete() const
	{
		return m_MailCityReportAutoDelete;
	}

	inline int MailCityReportOldAge() const
	{
		return m_MailCityReportOldAge;
	}

	inline bool MailFleetReportAutoDelete() const
	{
		return m_MailFleetReportAutoDelete;
	}

	inline int MailFleetReportOldAge() const
	{
		return m_MailFleetReportOldAge;
	}

	inline int MailNotificationMode() const
	{
		return m_MailNotificationMode;
	}

	inline bool MailOfficerReportAutoDelete() const
	{
		return m_MailOfficerReportAutoDelete;
	}

	inline int MailOfficerReportOldAge() const
	{
		return m_MailOfficerReportOldAge;
	}

	static void MemoryInfo();

	inline int RockViewRange() const
	{
		return m_RockViewRange;
	}

	inline int RockViewRangeFt() const
	{
		return m_RockViewRangeFt;
	}

	inline int RockViewRangeFt2() const
	{
		return m_RockViewRangeFt2;
	}

	inline int ShrubViewRange() const
	{
		return m_ShrubViewRange;
	}

	inline int ShrubViewRangeFt() const
	{
		return m_ShrubViewRangeFt;
	}

	inline int ShrubViewRangeFt2() const
	{
		return m_ShrubViewRangeFt2;
	}

	inline int TreeViewRange() const
	{
		return m_TreeViewRange;
	}

	inline int TreeViewRangeFt() const
	{
		return m_TreeViewRangeFt;
	}

	inline int TreeViewRangeFt2() const
	{
		return m_TreeViewRangeFt2;
	}

	// Returns name of default sans serif typeface.
	const QString& TypefaceSans() const;

	// Returns name of default serif typeface.
	const QString& TypefaceSerif() const;

	// Adds a bid from the sender id on the named channel.
	// Revokes all previous bids by the sender on the channel.
	// NOTE: Does not verify the comm packet actually represents a bid. Caller does that.
	virtual void BidPlaced(const AuCommPacket* /*msgBid*/)
	{
		Q_ASSERT(false);
	}

	virtual void BidRevoked(const AuId& /*idFrom*/, const QString& /*channelName*/)
	{
		Q_ASSERT(false);
	}

	inline const QString& ClientChannelServer() const	// Boss server for client chan
	{
		return m_ClientChannelServer;
	}

	// DivTexels is the number of pixels in the surface texture of a planet per terrain long lat division, as returned by
	// DaPlanet::LongitudeDivisions and DaPlanet::LatitudeDivisions.
	// DivTexels does not apply to gas giants.
	// Due to smoothing applied when resizing the result, 3 pixels per division makes the locator map and transporter display look
	// nicer than the raw result. More than 3 produces miniscule improvement, at best.
	// DivTexels must not exceed 17 or the largest spherical non-gasgiant planet will exceed 4096 pixels width, which would require
	// stripping of the texture like a ringworld. Planets aren't prepared for stripping.
	// DivTexels must not exceed 7 or the largest ringworld will exceed 4096 pixels height. Texture size overall limit is 4096x4096.
	inline int DivTexels() const
	{
		return m_DivTexels;
	}

	inline QString EncodeUTCString() const
	{
		return EncodeUTCString(CoordinatedTime());
	}

	inline static QString EncodeUTCString(const AuTime& utcDateTime)
	{
		return QString("UTC:%1").arg(utcDateTime.UTC().toTime_t(), 8, 16, QChar('0'));
	}

	// Checks for pending Bid that corresponds to the msg,
	// expected to be an Accept or Bid message.
	// Return false if there is no match or the buyer has no active bid.
	virtual bool HasPendingBid(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending bid that corresponds to the msg, expected to be an AcceptBarter or BidBarter message.
	// Return false if there is no match or the buyer has no active bid.
	virtual bool HasPendingBidBarter(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	virtual bool HasPendingBidShipment(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending colonist notices that corresponds to the colonist message.
	// Return false if there is no match or the requestor has no active colonist notices.
	virtual bool HasPendingColonists(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending crew notices that corresponds to the crew message.
	// Return false if there is no match or the requestor has no active crew notices.
	virtual bool HasPendingCrew(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending cryo notices that corresponds to the crew message.
	// Return false if there is no match or the requestor has no active crew notices.
	virtual bool HasPendingCryo(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending officer notices that corresponds to the officer message.
	// Return false if there is no match or the requestor has no active officer notices.
	virtual bool HasPendingOfficers(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending troop notices that corresponds to the troop message.
	// Return false if there is no match or the requestor has no active troop notices.
	virtual bool HasPendingTroops(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	// Checks for pending OfferRepairXXX that corresponds to the msg, expected to be an AcceptRepair message.
	// Return false if there is no match or the buyer has no active bid.
	virtual bool HasPendingRepairOffer(const AuCommPacket* /*msg*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	inline quint32 UdpDecayMargin() const
	{
		return m_UdpDecayMargin;
	}

	inline quint16 ChatPort() const
	{
		return m_ChatPort;
	}

	inline const QString& Company() const
	{
		return m_Company;
	}

	inline static QString CopyrightSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xa9));
#else
#if defined(UNICODE)
		return QString("©");
#else
		return QString::fromLocal8Bit("©");
#endif
#endif
	}

	inline static QString CubedSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xb3));
#else
#if defined(UNICODE)
		return QString("³");
#else
		return QString::fromLocal8Bit("³");
#endif
#endif
	}

	inline static QString CurrencySymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xa2));
#else
#if defined(UNICODE)
		return QString("¢");
#else
		return QString::fromLocal8Bit("¢");
#endif
#endif
	}

	// Returns the milliseconds difference between the server clock and the local clock.
	// offset = serverTime - localTime.
	inline qint64 ServerTimeOffset() const
	{
		return m_ServerMSecSkew;
	}

	// Current UTC.
	// On the client, the result is adjusted using the clock skew difference between client and server.
	// On the server, the server skew value is always 0.
	inline QDateTime CurrentUTC() const
	{
		// The server skew value is always 0 on the server itself.
		if(m_ServerMSecSkew)
		{
			// Get local date and time from the system clock.
			// Return the time syncronized with the server. It will be off by the lag time between the client and the server.
			return QDateTime::currentDateTimeUtc().addMSecs(m_ServerMSecSkew);
		}
		else
		{
			// Get local date and time from the system clock.
			return QDateTime::currentDateTimeUtc();
		}
	}

	// Current AuTime.
	// On the client, the result is coordinated with the server using the clock skew difference between client and server.
	// On the server, the server skew value is always 0.
	inline AuTime CoordinatedTime() const
	{
		// The server skew value is always 0 on the server itself.
		if(m_ServerMSecSkew)
		{
			// Get local date and time from the system clock.
			// Return the time syncronized with the server. It will be off by the lag time between the client and the server.
			return AuTime::Now().AddMSecs(m_ServerMSecSkew);
		}
		else
		{
			// Get local date and time from the system clock.
			return AuTime::Now();
		}
	}

	virtual void DecayActiveBids()
	{
		Q_ASSERT(false);
	}

	void DecayArenas();

	virtual void DecayAtomChat()
	{
		Q_ASSERT(false);
	}

	void DecayChannelMonitors();

	inline static QString DegreeSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xb0));
#else
#if defined(UNICODE)
		return QString("°");
#else
		return QString::fromLocal8Bit("°");
#endif
#endif
	}

	inline QColor DesignerBackColor() const
	{
		return m_DesignerBackColor;
	}

	QColor DesignerDefaultBackColor() const;

	QColor DesignerDefaultGridColor() const;

	QColor DesignerDefaultGridXColor() const;

	QColor DesignerDefaultGridYColor() const;

	QColor DesignerDefaultGridZColor() const;

	QColor DesignerDefaultLadderPathColor() const;

	QColor DesignerDefaultLineColor() const;

	QColor DesignerDefaultSelColor() const;

	QColor DesignerDefaultWalkPathColor() const;

	inline bool DesignerFindEdges() const
	{
		return m_DesignerFindEdges;
	}

	inline bool DesignerFindFaces() const
	{
		return m_DesignerFindFaces;
	}

	inline bool DesignerFindVertices() const
	{
		return m_DesignerFindVertices;
	}

	inline bool DesignerGravityOn() const
	{
		return m_DesignerGravityOn;
	}

	inline QColor DesignerGridColor() const
	{
		return m_DesignerGridColor;
	}

	inline QColor DesignerGridXColor() const
	{
		return m_DesignerGridXColor;
	}

	inline QColor DesignerGridYColor() const
	{
		return m_DesignerGridYColor;
	}

	inline QColor DesignerGridZColor() const
	{
		return m_DesignerGridZColor;
	}

	inline QColor DesignerLadderPathColor() const
	{
		return m_DesignerLadderColor;
	}

	inline QColor DesignerLineColor() const
	{
		return m_DesignerLineColor;
	}

	inline QColor DesignerSelColor() const
	{
		return m_DesignerSelColor;
	}

	inline int DesignerVertexSize() const
	{
		return m_DesignerVertexSize;
	}

	inline QColor DesignerWalkPathColor() const
	{
		return m_DesignerWalkColor;
	}

	inline AuDice& Dice()
	{
		return m_Dice;
	}

	inline static QString DoubleAngleLeftSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xab));
#else
#if defined(UNICODE)
		return QString("«");
#else
		return QString::fromLocal8Bit("«");
#endif
#endif
	}

	inline static QString DoubleAngleRightSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xbb));
#else
#if defined(UNICODE)
		return QString("»");
#else
		return QString::fromLocal8Bit("»");
#endif
#endif
	}

	inline static QString FilzlausSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xa4));
#else
#if defined(UNICODE)
		return QString("¤");
#else
		return QString::fromLocal8Bit("¤");
#endif
#endif
	}

	static QString FormatArmorFactors(float absorb, float block);

	// 'amount' is the number in cronos.
	static QString FormatMoney(qint32 amount);

	// 'amount' is the number in cronos.
	static QString FormatMoney(quint32 amount);

	// 'amount' is the number in cronos.
	// 'decimals' is the number of digits after the decimal point.
	static QString FormatMoney(double amount, int decimals=0);

	unsigned int GenConfirmation();	// Generate valid confirmation num.

	inline const QString& InternetName() const
	{
		return m_NetName;
	}

	inline int BodyStatSize() const
	{
		return m_BodyStatSize;
	}

	// 0=no, 1=yes, 2=headless
	inline int DrawBodyModel() const
	{
		return m_DrawBody;
	}

	inline int DrawClouds() const
	{
		return m_DrawClouds;
	}

	inline bool DrawFramedPictures() const
	{
		return m_DrawFramedPictures;
	}

	inline bool DrawGun() const
	{
		return m_DrawGun;
	}

	inline bool DrawHelmetHUDAlways() const
	{
		return m_DrawHelmetHUDAlways;
	}

	inline bool DrawMiniMe() const
	{
		return m_DrawMiniMe;
	}

	inline bool DrawNebulaCloud() const
	{
		return m_DrawNebulaCloud;
	}

	inline bool DrawSpecks() const
	{
		return m_DrawSpecks;
	}

	inline bool DrawSkyBox() const
	{
		return m_DrawSkyBox;
	}

	inline bool SkyBoxMipMap() const
	{
		return m_SkyBoxMipMap;
	}

	inline unsigned int SkyBoxPaneSize() const
	{
		return m_SkyBoxPaneSize;
	}

	inline bool DrawWeatherEffects() const
	{
		return m_DrawWeatherEffects;
	}

	inline double FieldOfViewXCos() const
	{
		return m_FieldOfViewXCos;
	}

	inline double FieldOfViewY() const
	{
		return m_FieldOfViewY;
	}

	inline int FramedPictureSize() const
	{
		return m_FramedPictureSize;
	}

	inline int ResourceIconSize() const
	{
		return m_ResourceIconSize;
	}

	inline bool IsJoystickEnabled(int joyIndex) const
	{
		Q_ASSERT(joyIndex >= 0);
		Q_ASSERT(joyIndex < 6);
		return m_EnableJoystick[joyIndex];
	}

	inline bool IsLoginMaster() const
	{
		return m_Mode == LoginMaster;
	}

	inline bool IsLoginWorker() const
	{
		return m_Mode == LoginWorker;
	}

	inline bool IsMailClient() const
	{
		return m_Mode == MailClient;
	}

	inline bool IsMailMaster() const
	{
		return m_Mode == MailMaster;
	}

	inline bool IsMailWorker() const
	{
		return m_Mode == MailWorker;
	}

	// Used by other servers to query the login server.
	static bool IsPlayerLoggedIn(unsigned int idPlayer, unsigned int confirmNum)
	{
		return IsPlayerLoggedIn(idPlayer, confirmNum, 0, 0, 0);
	}

	// 'pIdUnit' returns the units controlled by idPlayer. It must point to an array big enough for 6 AuIds.
	// 'acctType' returns the type of account idPlayer holds. 0=emperor, 1=closed, 2=server, 3=admin, 4=programmer
	static bool IsPlayerLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId pIdUnit[6], ACCOUNT& acctType, AuTime& expiry)
	{
		return IsPlayerLoggedIn(idPlayer, confirmNum, pIdUnit, &acctType, &expiry);
	}

private:
	// 'pIdUnit' returns the units controlled by idPlayer. It must point to an array big enough for 6 AuIds.
	// 'acctType' returns the type of account idPlayer holds. 0=emperor, 1=closed, 2=server, 3=admin, 4=programmer
	// Both must be 0 or both must be non-null.
	static bool IsPlayerLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId* pIdUnit, ACCOUNT* acctType, AuTime* expiry);

public:
	// True if mating is ok.
	// Nudity is implicitly ok while mating.
	inline bool IsMatingOk() const
	{
		return m_Rating == Mature || m_Rating == Adult;
	}

	// True if nudity is ok all the time.
	// Nudity is implicitly ok while mating.
	inline bool IsNudityOk() const
	{
		return m_Rating == Adult || m_Rating == Teen;
	}

	// True when running scene client on line or solo.
	inline bool IsSceneApp() const
	{
		return IsSceneClient() || IsSceneSolo();
	}

	// True when running scene client on line.
	inline bool IsSceneClient() const
	{
		return m_Mode == SceneClient;
	}

	inline bool IsSceneManager() const
	{
		return m_Mode == SceneManager;
	}

	inline bool IsSceneMaster() const
	{
		return m_Mode == SceneMaster;
	}

	// True when running scene client solo.
	inline bool IsSceneSolo() const
	{
		return m_Mode == SceneSolo;
	}

	inline bool IsSceneWorker() const
	{
		return m_Mode == SceneWorker;
	}

	virtual bool IsStoryActive(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return false;
	}

	inline bool IsWebCGI() const
	{
		return m_Mode == WebCGI;
	}

	// 'unitId' is any unit owned by a player.
	// Fills the array idUnits with the player's current unit ids.
	// Queries the login server to do this.
	// Returns true if the player record was found and unit ids returned.
	static bool ListPlayerUnits(const AuId& unitId, AuId* idUnits);

	virtual AuAtomChatList* LockAtomChat()
	{
		Q_ASSERT(false);
		return 0;
	}

	inline unordered_multimap<QString, AuCommChannelConnection, AuStringHash>& LockAtomCommChannels()
	{
		m_muAtomChannels.lock();
		return m_CommChannels;	// Atom comm channels.
	}
	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline const DgData& LockBdDesignsForRead()
	{
		Q_ASSERT(m_Data);
		m_rwBdDesigns.lockForRead();
		return *m_Data;
	}

	// The data address is the same as LockDataForWrite but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline DgData& LockBdDesignsForWrite()
	{
		Q_ASSERT(m_Data);
		m_rwBdDesigns.lockForWrite();
		return *m_Data;
	}

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline const DgData& LockBdExchangeForRead()
	{
		Q_ASSERT(m_Data);
		m_rwBdExchange.lockForRead();
		return *m_Data;
	}

	// The data address is the same as LockDataForWrite but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline DgData& LockBdExchangeForWrite()
	{
		Q_ASSERT(m_Data);
		m_rwBdExchange.lockForWrite();
		return *m_Data;
	}

	inline const DgData& LockDataForRead()
	{
		Q_ASSERT(m_Data);
		m_rwData.lockForRead();
		return *m_Data;
	}

	inline DgData& LockDataForWrite()
	{
		Q_ASSERT(m_Data);
		m_rwData.lockForWrite();
		return *m_Data;
	}

	inline const AuHengeDestinations& LockHengeDestinations()
	{
		m_rwHengeDestinations.lockForRead();
		return m_HengeDestinations;
	}

	inline AuHengeDestinations& LockHengeDestinationsForWrite()
	{
		m_rwHengeDestinations.lockForWrite();
		return m_HengeDestinations;
	}

	virtual const AuJobOffers* LockJobOffersForRead()
	{
		Q_ASSERT(false);
		return 0;	//DHH You must override this function to use it. See AuDbGlobal.
	}

	virtual AuJobOffers* LockJobOffersForWrite()
	{
		Q_ASSERT(false);
		return 0;	//DHH You must override this function to use it. See AuDbGlobal.
	}

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline const DgData& LockItDesignsForRead()
	{
		Q_ASSERT(m_Data);
		m_rwItDesigns.lockForRead();
		return *m_Data;
	}

	// The data address is the same as LockDataForWrite but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline DgData& LockItDesignsForWrite()
	{
		Q_ASSERT(m_Data);
		m_rwItDesigns.lockForWrite();
		return *m_Data;
	}

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline const DgData& LockScDesignsForRead()
	{
		Q_ASSERT(m_Data);
		m_rwScDesigns.lockForRead();
		return *m_Data;
	}

	// The data address is the same as LockDataForWrite but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline DgData& LockScDesignsForWrite()
	{
		Q_ASSERT(m_Data);
		m_rwScDesigns.lockForWrite();
		return *m_Data;
	}

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline const DgData& LockScExchangeForRead()
	{
		Q_ASSERT(m_Data);
		m_rwScExchange.lockForRead();
		return *m_Data;
	}

	// The data address is the same as LockDataForWrite but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	inline DgData& LockScExchangeForWrite()
	{
		Q_ASSERT(m_Data);
		m_rwScExchange.lockForWrite();
		return *m_Data;
	}

	virtual const AuScripts* LockScripts()
	{
		Q_ASSERT(false);
		return 0;	//DHH You must override this function to use it. See AuDbGlobal.
	}

	virtual const AuStories* LockStoriesForRead() const
	{
		Q_ASSERT(false);
		return 0;	//DHH You must override this function to use it. See AuDbGlobal.
	}

	virtual AuStories* LockStoriesForWrite()
	{
		Q_ASSERT(false);
		return 0;	//DHH You must override this function to use it. See AuDbGlobal.
	}

	virtual AuStory* LockStory(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);
		return 0;	//DHH You must override this function to use it. See AuDbGlobal.
	}

	inline QByteArray& LockMailBuffer(const char* /*who*/)
	{
		//qDebug("LockMailBuffer - %s", who);
		m_muMailBuffer.lock();
		//qDebug("LockMailBuffer - %s acquired", who);
		return m_MailBuffer;
	}

	inline const AuGLModelCache& LockGLModelsForRead()
	{
		m_rwModelsGL.lockForRead();
		return *m_ModelsGL;
	}

	inline AuGLModelCache& LockGLModelsForWrite()
	{
		m_rwModelsGL.lockForWrite();
		return *m_ModelsGL;
	}

	// Adds a timestamped message to a file. e.g. stdout, stderr, m_LogFile, m_LogConnectFile.
	static void LogFile(FILE* fp, const QString& message);

	// Adds a timestamped message to stdout.
	inline void Log(const QString& message)
	{
		m_LogFileLock.lock();
		if(m_LogFile)
			LogFile(m_LogFile, message);
		else
			LogFile(stdout, message);
		m_LogFileLock.unlock();
	}

	// Adds 'message' to stderr or m_LogFile with a leading date/time stamp and 'error: '.
	inline void Error(const QString& message)
	{
		QString errMsg = QString("error: %1").arg(message);
		Log(errMsg);
	}

	// Adds 'message' to stdout or m_LogFile with a leading date/time stamp and 'warning: '.
	inline void Warning(const QString& message)
	{
		QString warnMsg = QString("warning: %1").arg(message);
		Log(warnMsg);
	}

	// Adds a timestamped message to a log file.
	static void Log(const QString& fileName, const QString& message);

	// Adds a timestamped message to the connect log file.
	inline void LogConnect(const QString& fileName, const QString& message)
	{
		m_LogConnectFileLock.lock();
		if(m_LogConnectFile)
			LogFile(m_LogConnectFile, message);
		else
			Log(fileName, message);
		m_LogConnectFileLock.unlock();
	}

	inline const QString& LoginMasterName() const
	{
		return m_LoginMasterName;
	}

	inline const QString& MailMasterName() const
	{
		return m_MailMasterName;
	}

	inline quint16 ManagerPort() const
	{
		return m_ManagerPort;
	}

	inline static QString MiddleDotSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xb7));
#else
#if defined(UNICODE)
		return QString("·");
#else
		return QString::fromLocal8Bit("·");
#endif
#endif
	}

	inline MODE Mode() const
	{
		return m_Mode;
	}

	// BTW 'idMonitoringAtom' is never a person atom.
	void MonitorChannel(const QString& channelName, const AuId& idMonitoringAtom);

	virtual AuId NextAtomId(unsigned int /*howMany*/=1)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return AuId();
	}

	virtual quint32 NextBuildingDesignId(unsigned int /*howmany*/=1)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return 0;
	}

	virtual quint32 NextEmpireId()
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return 0;
	}

	virtual quint32 NextItemDesignId(unsigned int /*howmany*/=1)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return 0;
	}

	// Used by the solo client for solo mail.
	virtual quint32 NextMailMessageId()
	{
		Q_ASSERT(false);	// You must override this to use it. See AuClientGlobal.
		return 0;
	}

	virtual quint32 NextModelId()
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return 0;
	}

	virtual quint32 NextSpaceShipDesignId(unsigned int /*howmany*/=1)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return 0;
	}

	virtual quint32 NextStoryId(unsigned int /*howmany*/=1)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return 0;
	}

	// Queues a channel to be opened to another atom.
	virtual void OpenChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const AuId& idFrom, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, const AuId& idTo, bool connectBeyondSolarSystem)
	{
		Q_ASSERT(false);
		Q_UNUSED(channelName);
		Q_UNUSED(scope);
		Q_UNUSED(idFrom);
		Q_UNUSED(fromName);
		Q_UNUSED(fromSector);
		Q_UNUSED(fromGalaxy);
		Q_UNUSED(idTo);
		Q_UNUSED(connectBeyondSolarSystem);
	}

	inline int GroundCoverQty() const
	{
		return m_GroundCoverQty;
	}

	inline unsigned int MaxLights() const
	{
		return m_MaxLights;
	}

	inline int GroundTextureSize() const
	{
		return m_GroundTextureSize;
	}

	inline int PlantBranchMaxSegs() const
	{
		return m_PlantBranchMaxSegs;
	}

	inline int PlantTextureSize() const
	{
		return m_PlantTextureSize;
	}

	inline RATING Rating() const
	{
		return m_Rating;
	}

	static QString RatingDescription(RATING rating);

	inline float TreeTrunkMinDia() const
	{
		return m_TreeTrunkMinDia;
	}

	inline int TreeTrunkFaces() const
	{
		return m_TreeTrunkFaces;
	}

	inline int TreeTrunkMaxSegs() const
	{
		return m_TreeTrunkMaxSegs;
	}

	virtual void PlayerChanged(unsigned int /*idPlayer*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
	}

	virtual void ReloadStory(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
	}

	virtual void RemoveStory(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
	}

	void ResetClientChannelServer()
	{
		m_ClientChannelServer = SceneMasterName();
	}

	// Reset scene server values to return to communication with the master scene server.
	void ResetSceneServer();

	inline const QString& SceneMasterName() const
	{
		return m_SceneMasterName;
	}

	inline const QString& SceneServer() const		// Name of current scene server.
	{
		return m_SceneServerName;
	}


	// On scene client, tells us what kind of server we are connected to.
	// 0=master, 1=manager, 2=worker
	inline quint8 SceneServerMode() const
	{
		return m_SceneServerMode;
	}

	inline quint16 ScenePort() const
	{
		return m_ScenePort;
	}

	inline static QString SectionSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xa7));
#else
#if defined(UNICODE)
		return QString("§");
#else
		return QString::fromLocal8Bit("§");
#endif
#endif
	}

	virtual void SendAtomChat(const AuCommPacket& /*msg*/)
	{
		Q_ASSERT(false);
	}

	// 'fromSystem', 'fromWorld', and 'fromGalaxyLoc' can be 0. They are not needed specifically by the mail. They are helpful at
	// the receiving end to enable right-click popup menu items that issue mission travel orders.
	// 'galaxy' is a GALAXY enum value, it can be GALAXY_Count when unknown
	void SendMail(unsigned int idPlayer, unsigned int confirmNum, const AuId& idFromAtom, const QString& nameFrom,
			const AuId& idToAtom, AuMailMsg::MSGTYPE msgType, const QString& subject, const QString& bodyText,
			const AuId& fromSystemId, const QString& fromSystem, const AuId& fromWorldId, const QString& fromWorld,
			GALAXY fromGalaxy, const AuPoint& fromGalaxyLoc);

	// 'fromSystem', 'fromWorld', and 'fromGalaxyLoc' can be 0. They are not needed specifically by the mail. They are helpful at
	// the receiving end to enable right-click popup menu items that issue mission travel orders.
	// 'galaxy' is a GALAXY enum value, it can be GALAXY_Count when unknown
	void SendBulkMail(unsigned int idPlayer, unsigned int confirmNum, const AuId& idFromAtom, const QString& nameFrom,
			const AuIdList& recipients, AuMailMsg::MSGTYPE msgType, const QString& subject, const QString& bodyText,
			const AuId& fromSystemId, const QString& fromSystem, const AuId& fromWorldId, const QString& fromWorld,
			GALAXY fromGalaxy, const AuPoint& fromGalaxyLoc);

	inline quint16 ServerPort() const
	{
		return m_ServerPort;
	}

	void ServerTimeUpdated(const AuTime& serverUTC);

	void SetBumpMapSeas(bool bumpMapSeas);

	void SetChatPort(quint16 chatPort);

	void SetMode(MODE mode);

	void SetRating(RATING rating);

	// This is the overall sound volume for all sounds.
	void SetSoundEffectsGain(float gain);

	inline float SoundEffectsGain() const
	{
		return m_SoundEffectsGain;
	}

	// 'sound' is a SOUND_EFFECT
	void SetSoundGain(int sound, float gain);

	inline float SoundGain(int sound) const
	{
		QHash<int, float>::const_iterator gi = m_SoundGain.find(sound);
		if(gi == m_SoundGain.cend())
			return 1.f;
		return *gi;
	}

	void SetSoundOverride(int sound, const AuSoundOverride& sndOver);

	inline bool SoundOverride(int sound, AuSoundOverride& sndOver) const
	{
		QHash<int, AuSoundOverride>::const_iterator oi = m_SoundOverrides.find(sound);
		if(oi == m_SoundOverrides.cend())
			return false;
		sndOver = *oi;
		return true;
	}

	// Tests the building designs lock for deadlock.
	inline bool TestBdLock()
	{
		if(m_rwBdDesigns.tryLockForWrite(3000))	// Attempt to acquire the lock but don't wait forever.
		{
			m_rwBdDesigns.unlock();				// Let it go as soon as I get it.
			return true;						// I am not deadlocked.
		}
		return false;							// I failed to get the lock in the timeout period.
	}

	// Tests the data lock for deadlock.
	inline bool TestDataLock()
	{
		if(m_rwData.tryLockForWrite(3000))	// Attempt to acquire the lock but don't wait forever.
		{
			m_rwData.unlock();				// Let it go as soon as I get it.
			return true;					// I am not deadlocked.
		}
		return false;						// I failed to get the lock in the timeout period.
	}

	// Tries to lock the hash lock for write; gives up after 'timeout' milliseconds.
	// Returns -1 if the lock cannot be obtained within the 'timeout' period.
	// Returns the amount of time it took to get the lock, if the lock is obtained within the 'timeout' period.
	virtual int TestEtheriumLock(int timeout)
	{
		Q_UNUSED(timeout);
		return 0;
	}

	// Tests the etherium trash lock for deadlock.
	virtual bool TestEtheriumTrashLock()
	{
		return true;
	}

	// Tests the building designs lock for deadlock.
	inline bool TestItLock()
	{
		if(m_rwItDesigns.tryLockForWrite(3000))	// Attempt to acquire the lock but don't wait forever.
		{
			m_rwItDesigns.unlock();				// Let it go as soon as I get it.
			return true;						// I am not deadlocked.
		}
		return false;							// I failed to get the lock in the timeout period.
	}

	// Tests the spacecraft designs lock for deadlock.
	inline bool TestScLock()
	{
		if(m_rwScDesigns.tryLockForWrite(3000))	// Attempt to acquire the lock but don't wait forever.
		{
			m_rwScDesigns.unlock();				// Let it go as soon as I get it.
			return true;						// I am not deadlocked.
		}
		return false;							// I failed to get the lock in the timeout period.
	}

	void DecayNet1Downloading(int howMuch);

	void DecayNet1Uploading(int howMuch);

	void DecayNet2Downloading(int howMuch);

	void DecayNet2Uploading(int howMuch);

	void DecayNet3Downloading(int howMuch);

	void DecayNet3Uploading(int howMuch);

	void DecayNet4Downloading(int howMuch);

	void DecayNet4Uploading(int howMuch);

	void SetNet1Downloading(int downloading);

	void SetNet1Uploading(int uploading);

	void SetNet2Downloading(int downloading);

	void SetNet2Uploading(int uploading);

	void SetNet3Downloading(int downloading);

	void SetNet3Uploading(int uploading);

	void SetNet4Downloading(int downloading);

	void SetNet4Uploading(int uploading);

	void SetClientChannelServer(const QString& serverName);

	void SetControlAverageSpanMs(quint32 controlAverageSpanMs);

	void SetControlDecayMargin(quint32 controlDecayMargin);

	void SetDetailedTerrainRange(int detailedTerrainRange);

	void SetGrassViewRange(int viewRange);

	void SetLeftRightCursor(int value);

	void SetLeftRightOrientation(int value);

	void SetMailCityReportAutoDelete(bool value);

	void SetMailCityReportOldAge(int value);

	void SetMailFleetReportAutoDelete(bool value);

	void SetMailFleetReportOldAge(int value);

	void SetMailNotificationMode(int value);

	void SetMailOfficerReportAutoDelete(bool value);

	void SetMailOfficerReportOldAge(int value);

	void SetRockViewRange(int viewRange);

	void SetShrubViewRange(int viewRange);

	void SetBuildingViewRange(int viewRange);

	void SetTreeViewRange(int viewRange);

	void SetDivTexels(int divTexels);

	void SetEnableJoystick(int joyIndex, bool enableJoystick);

	void SetFontAtom(const QFont& font);

	void SetFontDesigner(const QFont& font);

	void SetInternetName(const QString& netName);

	void SetManagerPort(quint16 mgrPort);

	void SetTypefaceSans(const QString& fontName);

	void SetTypefaceSerif(const QString& fontName);

	void SetUdpDecayMargin(quint32 udpDecayMargin);

	// Redirects login clients to an alternate server farm.
	// Currently a debug server farm is hosted on Huey, Dewey and Louie, with Dewey hosting the master servers.
	// e.g. SetLoginMaster("Dewey.Hazeron.net");
	void SetLoginMaster(const QString& serverNameOrIP);

	// Redirects mail clients to an alternate server farm.
	// Currently a debug server farm is hosted on Huey, Dewey and Louie, with Dewey hosting the master servers.
	// e.g. SetMailMaster("Dewey.Hazeron.net");
	void SetMailMaster(const QString& serverNameOrIP);

	// Redirects scene clients to an alternate server farm.
	// Currently a debug server farm is hosted on Huey, Dewey and Louie, with Dewey hosting the master servers.
	// e.g. SetSceneMaster("Dewey.Hazeron.net");
	void SetSceneMaster(const QString& serverNameOrIP);

	void SetUseNativeColorDialog(bool value);

	void SetUseNativeFileDialog(bool value);

	void SetUseNativeFontDialog(bool value);

	inline bool DesignerBlockPaths() const
	{
		return m_DesignerBlockPaths;
	}

	inline void DesignerSetBlockPaths(bool block)
	{
		m_DesignerBlockPaths = block;
		Settings().writeEntry("Designer/BlockPaths", block);
	}

	inline void DesignerSetFindEdges(bool findEm)
	{
		m_DesignerFindEdges = findEm;
		Settings().writeEntry("Designer/FindEdges", findEm);
	}

	inline void DesignerSetFindFaces(bool findEm)
	{
		m_DesignerFindFaces = findEm;
		Settings().writeEntry("Designer/FindFaces", findEm);
	}

	inline void DesignerSetFindVertices(bool findEm)
	{
		m_DesignerFindVertices = findEm;
		Settings().writeEntry("Designer/FindVertices", findEm);
	}

	inline void DesignerSetGravityOn(bool gravOn)
	{
		m_DesignerGravityOn = gravOn;
		Settings().writeEntry("Designer/GravityOn", gravOn);
	}

	void SetDesignerBackColor(const QColor& col);

	void SetDesignerGridColor(const QColor& col);

	void SetDesignerGridXColor(const QColor& col);

	void SetDesignerGridYColor(const QColor& col);

	void SetDesignerGridZColor(const QColor& col);

	void SetDesignerLadderPathColor(const QColor& col);

	void SetDesignerLineColor(const QColor& col);

	void SetDesignerSelColor(const QColor& col);

	void SetDesignerVertexSize(int vertexSize);

	void SetDesignerWalkPathColor(const QColor& col);

	void SetModelCache(AuGLModelCache* modelsGL);

	void SetGroundCoverQty(int groundCoverQty);

	void SetMaxLights(unsigned int maxLights);

	void SetGroundTextureSize(int texSize);

	void SetPlantBranchMaxSegs(int maxSegs);

	void SetPlantTextureSize(int texSize);

	void SetTreeTrunkMinDia(float minDia);

	void SetTreeTrunkFaces(int faces);

	void SetTreeTrunkMaxSegs(int maxSegs);

	void SetSceneServer(const QString& serverName, quint16 serverPort, unsigned int serverMode);

	// E.g. /home/daveh/Shores of Hazeron/Temporary
	void SetCacheApp(const QString& dir);

	void SetGameSaveDir(const QString& dir);

	void SetPictureDir(const QString& dir);

	void SetBaseDir(const QString& dir);

	inline AuSettings& Settings()
	{
		return m_Settings;
	}

	void SetBodyStatSize(int bodyStatSize);

	void SetDrawBodyModel(int drawBody);

	void SetDrawClouds(int drawClouds);

	void SetDrawFramedPictures(bool drawFramedPictures);

	void SetDrawGun(bool drawGun);

	void SetDrawHelmetHUDAlways(bool drawHelmetHUDAlways);

	void SetDrawMiniMe(bool drawMiniMe);

	void SetDrawNebulaCloud(bool drawNebulaCloud);

	void SetDrawSpecks(bool drawNebulaCloud);

	void SetDrawSkyBox(bool drawSkyBox);

	inline void SetFieldOfViewXCos(double fovXCos)
	{
		m_FieldOfViewXCos = fovXCos;
	}

	void SetFieldOfViewY(double fovy);

	void SetSkyBoxMipMap(bool mipMap);

	void SetSkyBoxPaneSize(unsigned int paneSize);

	void SetDrawWeatherEffects(bool drawWeatherEffects);

	void SetFramedPictureSize(int framedPictureSize);

	void SetResourceIconSize(int resourceIconSize);

	void SetWindowOpacity(bool active, unsigned int percent);

	// 0=automatic
	void SetWorkerThreadCount(int threadCount);

	// You must override this to use it. See AuDbGlobal.
	virtual void BdDesignAdded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdDesignChanged(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdDesignDeactivated(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdDesignDeleted(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdDesignNeeded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdDesignsNeeded(const AuId& architectId)
	{
		Q_UNUSED(architectId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdExchangeEntryAdded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdExchangeEntryChanged(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void BdExchangeEntryDeleted(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ItDesignAdded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ItDesignChanged(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ItDesignDeactivated(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ItDesignDeleted(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ItDesignNeeded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ItDesignsNeeded(const AuId& architectId)
	{
		Q_UNUSED(architectId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScDesignAdded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScDesignChanged(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScDesignDeactivated(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScDesignDeleted(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScDesignNeeded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScDesignsNeeded(const AuId& architectId)
	{
		Q_UNUSED(architectId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScExchangeEntryAdded(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScExchangeEntryChanged(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	// You must override this to use it. See AuDbGlobal.
	virtual void ScExchangeEntryDeleted(quint32 designId)
	{
		Q_UNUSED(designId);
	}

	inline static QString SquaredSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xb2));
#else
#if defined(UNICODE)
		return QString("²");
#else
		return QString::fromLocal8Bit("²");
#endif
#endif
	}

	void StopMonitoringChannel(const QString& channelName, const AuId& idMonitoringAtom);

	void StopMonitoringChannels(const AuId& idMonitoringAtom);

	virtual void StoryAdded(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
	}

	virtual QString StoryAuthor(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return "";
	}

	virtual QString StoryBrief(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return "";
	}

	virtual void StoryChanged(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
	}

	virtual void StoryDeleted(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
	}

	virtual QString StoryTitle(quint32 /*storyInstance*/)
	{
		Q_ASSERT(false);	// You must override this to use it. See AuDbGlobal.
		return "";
	}

	static bool TestConfirmationValidity(unsigned int confirmNum);

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	// Returns 0 if the mutex could not be locked.
	inline const DgData* TryLockBdDesignsForRead()
	{
		Q_ASSERT(m_Data);
		if(m_rwBdDesigns.tryLockForRead())
			return m_Data;										// Lock was obtained.
		return 0;												// Failed to obtain the lock.
	}

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	// Returns 0 if the mutex could not be locked.
	inline const DgData* TryLockItDesignsForRead()
	{
		Q_ASSERT(m_Data);
		if(m_rwItDesigns.tryLockForRead())
			return m_Data;										// Lock was obtained.
		return 0;												// Failed to obtain the lock.
	}

	// The data address is the same as LockDataForRead but the mutex is different. Be sure to use the correct one based on the
	// data you are accessing.
	// Returns 0 if the mutex could not be locked.
	inline const DgData* TryLockScDesignsForRead()
	{
		Q_ASSERT(m_Data);
		if(m_rwScDesigns.tryLockForRead())
			return m_Data;										// Lock was obtained.
		return 0;												// Failed to obtain the lock.
	}

	virtual void UnlockAtomChat()
	{
		Q_ASSERT(false);
	}

	inline void UnlockAtomCommChannels()
	{
		m_muAtomChannels.unlock();
	}

	inline void UnlockBdDesigns()
	{
		m_rwBdDesigns.unlock();
	}

	inline void UnlockBdExchange()
	{
		m_rwBdExchange.unlock();
	}

	inline void UnlockData()
	{
		m_rwData.unlock();
	}

	inline void UnlockHengeDestinations()
	{
		m_rwHengeDestinations.unlock();
	}

	inline void UnlockItDesigns()
	{
		m_rwItDesigns.unlock();
	}

	virtual void UnlockJobOffers()
	{
	}

	inline void UnlockScDesigns()
	{
		m_rwScDesigns.unlock();
	}

	inline void UnlockScExchange()
	{
		m_rwScExchange.unlock();
	}

	virtual void UnlockScripts()
	{
	}

	virtual void UnlockStories()
	{
	}

	virtual void UnlockStory(quint32 /*storyInstance*/)
	{
	}

	inline void UnlockMailBuffer(const char* /*who*/)
	{
		//qDebug("UnlockMailBuffer - %s", who);
		m_muMailBuffer.unlock();
	}

	inline void UnlockGLModels()
	{
		m_rwModelsGL.unlock();
	}

	virtual const char* WebAddress() const
	{
		return "Hazeron.com";
	}

	inline unsigned int WindowOpacity(bool active) const
	{
		if(active)
			return m_OpacityActive;
		else
			return m_OpacityInactive;
	}

	// Returns a number from 1 to QThread::idealThreadCount().
	inline int WorkerThreadCount() const
	{
		// Determine the ideal thread count.
		int idealThreadCount = QThread::idealThreadCount();
		if(idealThreadCount <= 0)
			return 1;

		// If worker thread count is in the valid range, return it.
		if(m_WorkerThreadCount > 0 && m_WorkerThreadCount <= idealThreadCount)
			return m_WorkerThreadCount;

		// Worker thread count is invalid so calculate an optimum value.
		if(idealThreadCount > 2)
			return idealThreadCount - 2;						// Reserve 2 threads for gui and network.
		else if(idealThreadCount > 1)
			return idealThreadCount - 1;						// Reserve 1 thread for gui.
		return idealThreadCount;
	}

	// Returns the literal setting, which might be 0.
	inline int WorkerThreadCountSetting() const
	{
		return m_WorkerThreadCount;
	}

	// Returns the number of threads available to the worker thread pool. This is the WorkerThreadCount minus atom manager threads.
	int WorkerThreadPoolCount() const;

	inline AuPoint FootOffset(const Au3dsGLSLDNA& dna) const
	{
		m_rwModels.lockForRead();
		QMap<Au3dsGLSLDNA, AuPoint>::const_iterator itFoot =  m_FootOffset.find(dna);
		if(itFoot != m_FootOffset.constEnd())
		{
			AuPoint offset = itFoot.value();
			m_rwModels.unlock();
			return offset;
		}
		m_rwModels.unlock();
		return AuPoint(0,0);
	}

	void SetFootOffset(const Au3dsGLSLDNA& dna, const AuPoint& footOffset);

	inline AuPoint HandOffset(const Au3dsGLSLDNA& dna) const
	{
		m_rwModels.lockForRead();
		QMap<Au3dsGLSLDNA, AuPoint>::const_iterator itHand =  m_HandOffset.find(dna);
		if(itHand != m_HandOffset.constEnd())
		{
			AuPoint offset = itHand.value();
			m_rwModels.unlock();
			return offset;
		}
		m_rwModels.unlock();
		return AuPoint(0,0);
	}

	void SetHandOffset(Au3dsGLSLDNA dna, AuPoint handOffset);

protected:
	void Init(MODE mode, const QString& appName, const QString& company, DgData* data, AuEther* etherium);
};

extern AUGLOBAL_DLL AuGlobal*				gp_AuGlobal;			// This is exported from my DLL.
#define g_Au		(*gp_AuGlobal)	// Enables use of global data by reference.

#endif // AuGlobal_H
