//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daatom.h,v 1.907 2022/05/17 15:18:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaAtom_H
#define DaAtom_H

#include "auarenaready.h"
#include "auatom.h"
#include "auatompos.h"
#include "aucityreport.h"
#include "aucombat.h"
#include "audnabase.h"
#include "audomain.h"
#include "auelapsedtimer.h"
#include "auether.h"
#include "auid.h"
#include "auitem.h"
#include "aujoboffer.h"
#include "aumatrix4x4.h"
#include "auoffice.h"
#include "auqueryenvironment.h"
#include "aushipment.h"
#include "austarmapdata.h"
#include "auwormhole.h"
#include "dttock.h"
#include <QList>
#include <QOpenGLWidget>
#include <map>

// Enable the MEM_TRACE_ATOM define to override new and delete for all atoms.
//#define MEM_TRACE_ATOM

class AuAtomRenderInfo;
class AuBox;
class AuCommAcceptBerth;
class AuCommodityExchange;
class AuCommPacket;
class AuEffect;
class AuEffectArray;
class AuEther;
class AuGearPos;
class AuGLEffects;
class AuGLModelMaker;
class AuSyncController;
class AuSyncControllerList;
class AuPose;
class AuQuadArray;
class AuRoadMap;
class AuScripts;
class AuSensorBlipList;
class AuStories;
class DaArena;
class DaAsteroid;
class DaAtom;
class DaAtomList;
class DaBd;
class DaCapsule;
class DaCreature;
class DaDesigner;
class DaDesignBase;
class DaDesignPlane;
class DaEffect;
class DaGalaxy;
class DaNpc;
class DaPerson;
class DaSector;
class DaSolarSystem;
class DaStar;
class DaTeleporter;
class DaUnit;
class DaUniverse;
class DaVessel;
class DaWorld;
class DaWorldLL;
class DgData;
class DiControlInputArray;
class DiPanelControl;
class QDataStream;
class QDebug;
class QOpenGLWidget;
class QSqlDatabase;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#define ATOM_SOUNDS		2

#define c_ArenaRingRadius				(FeetPerOrbit()/2)						// Radius of battle ring inside arena
#define c_ArenaBoundsRadius				(c_ArenaRingRadius+FeetPerMoonOrbit())	// Radius of outer bounds in arena.
#define d_GroundTolerance				.25						// Distance to floor that is considered "on the ground".

// These can be OR'd together when calling LoadSubatoms.
// Some are mutually exclusive. Be sure you know what you're doing. ;)
enum LOADSUBATOMS
{
	LOAD_NoSubatoms	= 0x00,
	LOAD_All		= 0x7f,
	LOAD_Systems	= 0x01,	// Solar systems only. Don't load children of solar systems.
	LOAD_Moons		= 0x03,	// All bodies down to moons are loaded, INCLUDING systems.
	LOAD_GamePieces	= 0x04,	// Things such as people, animals, items, and spacecraft.
	LOAD_StarField	= 0x80,	// Flag to identify objects loaded but not managed.
};

enum RENDERPASS
{
	PASS_None,							// Invalid state. Used for objects that have not been rendered in a given pass.
	PASS_Firmament,						// Objects beyond the solar system. Sky box, sector stars, nebula.
	PASS_Far,							// Solar system objects that can be seen far away.
	PASS_Near,							// Solar system objects that can be seen nearby away.
	PASS_FarBlend,						// Solar system objects that can be seen far away.
	PASS_NearBlend,						// Solar system objects that can be seen nearby away.
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuControlButton
{
public:
	AuId								m_AtomId;
	qint16								m_AtomPanel;
	quint8								m_AtomPanelControlId;
	bool								m_NeedsTarget;
	AuPoint								m_ScreenPos;			// GL unit coord, as if screen is -1 to 1 in both X and Y axes.
	QString								m_Text;
	GLuint								m_Texture;

public:
	inline AuControlButton(const QString& text, const AuId& atomId, qint16 panelId, qint8 controlId, GLuint texture,
			bool needsTarget=false)
	{
		m_AtomId = atomId;
		m_AtomPanel = panelId;
		m_AtomPanelControlId = controlId;
		m_NeedsTarget = needsTarget;
		m_Text = text;
		m_Texture = texture;
	}

	inline AuControlButton()
	{
		m_AtomPanel = 0;
		m_AtomPanelControlId = 0;
		m_NeedsTarget = false;
		m_Texture = 0;
	}

	inline AuControlButton(const AuControlButton& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomPanel = c.m_AtomPanel;
		m_AtomPanelControlId = c.m_AtomPanelControlId;
		m_NeedsTarget = c.m_NeedsTarget;
		m_ScreenPos = c.m_ScreenPos;
		m_Text = c.m_Text;
		m_Texture = c.m_Texture;
	}

public:	// Operators
	inline const AuControlButton& operator=(const AuControlButton& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomPanel = c.m_AtomPanel;
		m_AtomPanelControlId = c.m_AtomPanelControlId;
		m_NeedsTarget = c.m_NeedsTarget;
		m_ScreenPos = c.m_ScreenPos;
		m_Text = c.m_Text;
		m_Texture = c.m_Texture;
		return *this;
	}

	inline bool operator==(const AuControlButton& c) const
	{
		if(m_AtomId != c.m_AtomId)
			return false;
		if(m_AtomPanel != c.m_AtomPanel)
			return false;
		if(m_AtomPanelControlId != c.m_AtomPanelControlId)
			return false;
		if(m_NeedsTarget != c.m_NeedsTarget)
			return false;
		if(m_ScreenPos != c.m_ScreenPos)
			return false;
		if(m_Text != c.m_Text)
			return false;
		if(m_Texture != c.m_Texture)
			return false;
		return true;
	}

	inline bool operator!=(const AuControlButton& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline bool NeedsTarget() const
	{
		return m_NeedsTarget;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stores the input arguments for a call to CombatApplyDamage.

class AuDamageToApply
{
public:
	AuCombatAttacker					m_Attacker;
	AuPoint								m_HitLoc;
	AuCombatResult						m_Result;
	AuCombatWeapon						m_Weapon;
	AuPoint								m_WpnLoc;

public:
	inline AuDamageToApply(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, const AuCombatResult& result) :
		m_Attacker(attacker), m_HitLoc(hitLoc), m_Result(result), m_Weapon(weapon), m_WpnLoc(wpnLoc)
	{
	}

	inline AuDamageToApply(const AuDamageToApply& c) :
		m_Attacker(c.m_Attacker), m_HitLoc(c.m_HitLoc), m_Result(c.m_Result), m_Weapon(c.m_Weapon), m_WpnLoc(c.m_WpnLoc)
	{
	}

	inline const AuDamageToApply& operator=(const AuDamageToApply& c)
	{
		m_Attacker = c.m_Attacker;
		m_HitLoc = c.m_HitLoc;
		m_Result = c.m_Result;
		m_Weapon = c.m_Weapon;
		m_WpnLoc = c.m_WpnLoc;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBayFired
{
public:
	AuItem*								m_Ammo;					// Ammo fired for weapons. n/a for harvesters.
	AuId								m_Credit;
	AuPoint								m_EmitLoc;
	quint8								m_HarvestIndex;			// Index into commodity arrays for harvesters. See global Harvest
																// functions in audef.h
	AuPoint								m_LocOnTarget;			// Location relative to target, for missile targetting.
	AuId								m_Target;
	AuPoint								m_TargetLoc;
	quint8								m_TractorState;			// 0=not a tractor, 1=firing, >=2 =attached
	float								m_UnitsFired;			// Units fired, energy units for energy weapons and harvesters.
	AuTime								m_When;

public:
	inline AuBayFired()
	{
		m_Ammo = 0;
		m_HarvestIndex = 0;
		m_TractorState = 0;
		m_UnitsFired = 0;
	}

	inline ~AuBayFired()
	{
		if(m_Ammo)
			delete m_Ammo;
	}

	inline AuBayFired(const AuBayFired& c)
	{
		if(c.m_Ammo)
			m_Ammo = c.m_Ammo->Clone();
		else
			m_Ammo = 0;

		m_Credit = c.m_Credit;
		m_EmitLoc = c.m_EmitLoc;
		m_HarvestIndex = c.m_HarvestIndex;
		m_LocOnTarget = c.m_LocOnTarget;
		m_Target = c.m_Target;
		m_TargetLoc = c.m_TargetLoc;
		m_TractorState = c.m_TractorState;
		m_UnitsFired = c.m_UnitsFired;
		m_When = c.m_When;
	}

	inline const AuBayFired& operator=(const AuBayFired& c)
	{
		if(m_Ammo)
			delete m_Ammo;
		if(c.m_Ammo)
			m_Ammo = c.m_Ammo->Clone();
		else
			m_Ammo = 0;

		m_Credit = c.m_Credit;
		m_EmitLoc = c.m_EmitLoc;
		m_HarvestIndex = c.m_HarvestIndex;
		m_LocOnTarget = c.m_LocOnTarget;
		m_Target = c.m_Target;
		m_TargetLoc = c.m_TargetLoc;
		m_TractorState = c.m_TractorState;
		m_UnitsFired = c.m_UnitsFired;
		m_When = c.m_When;

		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		quint8 hasWhat = 0;
		if(m_Ammo)
			hasWhat |= 0x01;
		if(!m_Credit.IsNull())
			hasWhat |= 0x02;
		if(!m_EmitLoc.IsZero())
			hasWhat |= 0x04;
		if(m_HarvestIndex)
			hasWhat |= 0x08;
		if(!m_LocOnTarget.IsZero())
			hasWhat |= 0x10;
		if(!m_Target.IsNull() || !m_TargetLoc.IsZero())
			hasWhat |= 0x20;
		if(m_TractorState)
			hasWhat |= 0x40;

		out << hasWhat;

		if(hasWhat & 0x01)
			out << m_Ammo->Pack();

		if(hasWhat & 0x02)
			out << m_Credit;

		if(hasWhat & 0x04)
			out << m_EmitLoc;

		if(hasWhat & 0x08)
			out << m_HarvestIndex;

		if(hasWhat & 0x10)
			out << m_LocOnTarget;

		if(hasWhat & 0x20)
		{
			out << m_Target;
			out << m_TargetLoc;
		}

		if(hasWhat & 0x40)
			out << m_TractorState;

		out << m_UnitsFired;
		out << m_When;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 hasWhat;
		in >> hasWhat;

		if(hasWhat & 0x01)
		{
			QByteArray ba;
			in >> ba;
			m_Ammo = AuItem::Unpack(ba);
		}
		else
			m_Ammo = 0;

		if(hasWhat & 0x02)
			in >> m_Credit;
		else
			m_Credit = AuId(0);

		if(hasWhat & 0x04)
			in >> m_EmitLoc;
		else
			m_EmitLoc = AuPoint(0);

		if(hasWhat & 0x08)
			in >> m_HarvestIndex;
		else
			m_HarvestIndex = 0;

		if(hasWhat & 0x10)
			in >> m_LocOnTarget;
		else
			m_LocOnTarget = AuPoint();

		if(hasWhat & 0x20)
		{
			in >> m_Target;
			in >> m_TargetLoc;
		}
		else
		{
			m_Target = AuId();
			m_TargetLoc = AuPoint();
		}

		if(hasWhat & 0x40)
			in >> m_TractorState;
		else
			m_TractorState = 0;

		in >> m_UnitsFired;
		in >> m_When;
	}

public:	// Methods
	// Returns true if this is ready to start emitting.
	// Returns false if this was fired too recently to start emitting yet, or if it is a tractor beam that is already emitting.
	inline bool IsReadyToEmit() const
	{
		// Waiting for two seconds to pass since fired.
		return m_When.ElapsedSec() >= 2;
	}

	inline bool IsTractor() const
	{
		return m_TractorState;
	}

	inline bool IsTractorAttached() const
	{
		return m_TractorState >= 2;
	}

	// Returns true if this should be destroyed upon return.
	inline bool StartEmitting()
	{
		// If this is not a tractor beam, it is removed as soon as the mission starts.
		if(!IsTractor())
			return true;

		// Tractor beam switches to attached state.
		if(m_TractorState < 2)
			m_TractorState = 2;
		else if(m_TractorState < 5)
			m_TractorState++;
		return false;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuBayFired& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBayFired& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Helper class to store model view transformation matrix and other information about an atom, so we only calculate them once per
// render pass.

class AuModelMatrix
{
	AuMatrix4x4							m_Model;				// Model view matrix in this render pass.
	double								m_ViewDepth;
	bool								m_ViewDepthKnown;
	AuPoint								m_ViewLoc;
	bool								m_ViewLocKnown;

public:	// Constructors
	inline AuModelMatrix(const AuMatrix4x4& model) : m_Model(model)
	{
		m_ViewDepth = 0;
		m_ViewDepthKnown = false;
		m_ViewLocKnown = false;
	}

	inline AuModelMatrix(const AuModelMatrix& c) : m_Model(c.m_Model)
	{
		m_ViewDepth = c.m_ViewDepth;
		m_ViewDepthKnown = c.m_ViewDepthKnown;
		m_ViewLoc = c.m_ViewLoc;
		m_ViewLocKnown = c.m_ViewLocKnown;
	}

public:	// Operators
	inline const AuModelMatrix& operator=(const AuModelMatrix& c)
	{
		m_Model = c.m_Model;
		m_ViewDepth = c.m_ViewDepth;
		m_ViewDepthKnown = c.m_ViewDepthKnown;
		m_ViewLoc = c.m_ViewLoc;
		m_ViewLocKnown = c.m_ViewLocKnown;
		return *this;
	}

public:	// Methods
	inline const AuMatrix4x4& Matrix() const
	{
		return m_Model;
	}

	// Negative is into the screen. Positive is behind the viewer.
	inline double ViewDepth()
	{
		if(!m_ViewDepthKnown)
		{
			m_ViewDepth = ViewLoc().z;
			m_ViewDepthKnown = true;
		}
		return m_ViewDepth;
	}

	inline const AuPoint& ViewLoc()
	{
		if(!m_ViewLocKnown)
		{
			m_ViewLoc = m_Model.Translation();
			m_ViewLocKnown = true;
		}
		return m_ViewLoc;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuModelMatrixHash : public QHash<AuId, AuModelMatrix>
{
	double								m_FarDistance;			// Distance beyond which objects are rendered in the far pass.

public:	// Constructors
	AuModelMatrixHash();

	inline AuModelMatrixHash(const AuModelMatrixHash& c) : QHash<AuId, AuModelMatrix>(c)
	{
		m_FarDistance = c.m_FarDistance;
	}

public:	// Methods
	inline double FarDistance() const
	{
		return m_FarDistance;
	}

	inline const AuMatrix4x4& Matrix(const AuId& id) const
	{
		const_iterator mi = find(id);
		if(mi != cend())
			return (*mi).Matrix();
		return AuMatrix4x4::IdentityMatrix();
	}

	inline bool RenderThisPass(RENDERPASS pass, double viewDistance) const
	{
		switch(pass)
		{
		case PASS_None:
		case PASS_Firmament:
			break;
		case PASS_Far:
		case PASS_FarBlend:
			return viewDistance > FarDistance();
		case PASS_Near:
		case PASS_NearBlend:
			return viewDistance <= FarDistance();
		}
		return false;
	}

	/*
	inline void SetFarDistance(double far)
	{
		m_FarDistance = far;	// Windows gave a syntax error on this semicolon. :/
	}
	*/

	inline const AuPoint& ViewLoc(const AuId& id)
	{
		iterator mi = find(id);
		if(mi != end())
			return (*mi).ViewLoc();
		return AuPoint::Null();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Helper class to store renderLast information.

class AuRenderLast
{
public:
	DaAtom*		m_Atom;
	AuMatrix4x4	m_Model;

	inline AuRenderLast()
	{
		m_Atom = 0;
	}

	inline AuRenderLast(DaAtom* atom, const AuMatrix4x4& model)
	{
		m_Atom = atom;
		m_Model = model;
	}

	inline AuRenderLast(const AuRenderLast& c)
	{
		m_Atom = c.m_Atom;
		m_Model = c.m_Model;
	}

	inline const AuRenderLast& operator=(const AuRenderLast& c)
	{
		m_Atom = c.m_Atom;
		m_Model = c.m_Model;
		return *this;
	}
};

// The key is the negative eye distance squared, for sorting.
class AuRenderLastMap : public multimap<double, DaAtom*>
{
public:
	// Eye distance is calculated using the model transformation matrix in 'renderLast'.
	inline void Insert(AuModelMatrix& mv, DaAtom* atom)
	{
		const AuPoint& loc = mv.ViewLoc();		// -Z is farther from the eye into the scene.
		Insert(loc.Length2(), atom);				// Objects farther from the eye are sorted first, whether in front or in back.
	}

	void Insert(AuModelMatrixHash& rmod, DaAtom* atom);

	// 'eyeDistance' is the distance squared.
	// This is the typical result of projecting a location through the model view matrix and then taking the squared length of the
	// result.
	inline void Insert(double eyeDistance2, DaAtom* atom)
	{
		if(!HasAtom(atom))
			insert(pair<double, DaAtom*>(-eyeDistance2, atom));
	}

	inline bool HasAtom(const DaAtom* atom) const
	{
		for(const_iterator it = begin(); it != end(); it++)
			if((*it).second == atom)
				return true;
		return false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaAtom : public QObject
{
	Q_OBJECT;
friend class DaAtomList;
friend class DaEffect;					// Needs access to m_Effects.
friend class DtTrash;

public:
	enum ATOM_STATE
	{
		Normal,				// Atom normal state of existence.
		Dead,				// Atom is waiting to be removed from the atom tree permanently.
		Traveling,			// Atom is traveling to a nu parent atom, due to solar system change or spawn.
		Dormant,			// Atom exists but it is ignored by everything looking for normal atoms.
		ClientReparent_Deprecated,		// Normal atom. Client has changed parent, waiting for server to finalize.
		New,				// New atom not ready for processing. Not "normal" to be mostly ignored. Constructor default.
		Trash,				// Atom is on a one way trip to the trash bin and ultimate deletion.
	};

	// Do not change the values of this enum. Add new atom types at the end. Be sure to add to AtomTypeName.
	enum ATOM_TYPE
	{
		ATOM_Invalid = 0,
		ATOM_Atom,
		ATOM_Animal,
		ATOM_Arena,
		ATOM_Asteroid,
		ATOM_Avatar,
		ATOM_Bd,
		ATOM_Capsule,
		ATOM_CitCharacter_Deprecated,
		ATOM_CitCrew_Deprecated,
		ATOM_CitIndigenous_Deprecated = 10,
		ATOM_Citizen,
		ATOM_CitOfficer_Deprecated,
		ATOM_CitPassenger_Deprecated,
		ATOM_CitTroop_Deprecated,
		ATOM_CitWorker_Deprecated,
		ATOM_Creature,
		ATOM_DesignBase,
		ATOM_Designer,
		ATOM_DesignPlane,
		ATOM_Effect = 20,
		ATOM_Galaxy,
		ATOM_Item,
		ATOM_Missile = 23,
		ATOM_Npc = 24,
		ATOM_Sc = 25,
		ATOM_Sector,
		ATOM_Sentient,
		ATOM_SolarSystem,
		ATOM_Star,
		ATOM_Teleporter = 30,
		ATOM_Unit,
		ATOM_Universe,
		ATOM_Vehicle,
		ATOM_Vessel,
		ATOM_WorldGasGiant,
		ATOM_WorldGlobe,
		ATOM_World,
		ATOM_WorldLL,
		ATOM_WorldRing,
		ATOM_WorldRingSection = 40,
		ATOM_WorldSphere,
		ATOM_Count					// Update P_SyncComplete in dtsspacket.h if this exceeds 50.
	};

	// Constants for determining object collision results when an atom is moving.
	enum COLLISION_TYPE
	{
		CT_Miss,			// Moving atom is not obstructed.
		CT_HitParent,		// Moving atom collides with surface of the parent.
		CT_HitPeer,			// Moving atom collides with surface of a peer atom.
		CT_JoinedPeer,		// Moving atom joins a peer without hitting it. Moving atom becomes subordinate to peer.
		CT_HitPeerShield,	// Moving atom collides with surface of a peer atom's shield bubble.
	};

	enum SET_MODIFIED_BITS
	{
		NOTIFY_Database = 0x01,
		NOTIFY_Scene = 0x02,
		NOTIFY_All = 0x03,
	};

private:
	quint8								m_AnyScene;				// Set when I appear in a scene. Decremented in Tock. Not stored.
	bool								m_AtRest;				// true if atom is not moving. Not affected by gravity when at rest.
	AuId								m_Id;					// Unique identifier of this atom.
	COLLISION_TYPE						m_MoveResult;			// COLLISION_TYPE result of my last move. Useful for AI pathing.
	ATOM_STATE							m_State;
	quint8								m_Ticked;				// Set in Tick. Decremented in Tock. Not stored.
	AuTime								m_tmCreated;			// Time this atom was Created. Not guarded because it never changes.

private:
	mutable QReadWriteLock				m_DamageToApplyLock;	// Guards m_DamageToApply during Think.
	QList<AuDamageToApply>				m_DamageToApply;		// Damage to be applied during Tick. Not stored.

private:
	mutable QReadWriteLock				m_DeathLock;			// Guards m_tmDeath.
	AuTime								m_tmDeath;				// Time this atom will die. 0 = immortal

protected:
	mutable QReadWriteLock				m_EffectsLock;			// Guards m_Effects and m_tmEffectsChange
	AuEffectArray*						m_Effects;				// Only allocated when needed.
	AuTime								m_tmEffectsChange;

private:
	// Environment information gathered by calling CreateEnvironmentQuery. Not present on all atoms. Check DoesQueryEnvironment.
	mutable QReadWriteLock				m_EnvironLock;
	mutable AuQueryEnvironment*			m_Environ;
	mutable int							m_EnvironState;			// 0=Not current, 1=current, 2=not current; renew in progress
	AuPoint								m_EnvVector;			// Movement vector relative to the environment.
	mutable float						m_Illumination;			// Light intensity when scene atom is within atmosphere range of a
																// planet, even if it doesn't have an atmosphere. Otherwise -1.

private:
	mutable QReadWriteLock				m_EnvTransformLock;		// Guards m_EnvTransform and m_EnvTransformValid.
	mutable AuMatrix4x4					m_EnvTransform;			// World transformation of this Atom relative to its environment.
	mutable bool						m_EnvTransformValid;	// true while the m_EnvTransform is valid.

private:
	mutable QReadWriteLock				m_ModifyLock;			// Guards m_tmModify.
	AuTime								m_tmModify;				// Time this atom was last Modified.

private:
	mutable QReadWriteLock				m_PosLock;
	AuPoint								m_OrientRotation;		// Rotation to which we are orienting. Not stored in db.
	AuAtomPos							m_Pos;
	AuElapsedTimer						m_PosChange;
	AuPoint								m_Vector;				// Movement vector relative to me. Length is speed in fps.

	mutable AuId						m_Controller;
	AuSyncControllerList*				m_SyncController;		// Syncs to be propagated to the controller.
	mutable AuElapsedTimer				m_tmControlled;			// Time this atom was last controlled by a client. Not stored in db.

private:
	mutable QReadWriteLock				m_EtherLock;
	AuEther*							m_Ether;				// My personal ether. Might not be the global etherium.

private:
	mutable QReadWriteLock				m_TickLock;				// Guards m_tmTicked.
	AuElapsedTimer						m_tmTicked;				// Time this atom was last Ticked. Nominally 30 Ticks per second.

private:
	mutable QReadWriteLock				m_TockLock;				// Guards m_tmTocked.
	AuTime								m_tmTocked;				// Time this atom was last Tocked. Nominally 1 Tock per second.

protected:
	double								m_MiniScale;			// Used when displaying mini me on the screen.

	// OpenAL
	unsigned int						m_Audio[ATOM_SOUNDS];	// sound sources
	int									m_FirstActiveAudioIdx;	// index of first active source in m_Audio, ATOM_SOUNDS if none
	int									m_Sound[ATOM_SOUNDS];	// current SOUND_EFFECT

public:	// Construction
	inline DaAtom()
	{
		m_AnyScene = 0;
		m_AtRest = true;
		m_Effects = 0;
		m_Environ = 0;
		m_EnvironState = 0;	// not current
		m_EnvTransformValid = false;
		m_Ether = 0;
		m_Illumination = 0;
		m_MiniScale = 1;
		m_MoveResult = CT_Miss;
		m_State = New;
		m_SyncController = 0;
		m_Ticked = 0;

		m_FirstActiveAudioIdx = ATOM_SOUNDS;
		for(int cs=0; cs<ATOM_SOUNDS; cs++)
		{
			m_Audio[cs] = 0;
			m_Sound[cs] = 0/*SOUND_None*/;
		}
	}

	virtual ~DaAtom();

protected:
	DaAtom(const DaAtom& rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Call this after deserializing.
	// Returns this atom pointer if this atom is to be kept as is.
	// Returns a nu atom pointer of an atom that is to replace this atom. This atom is deleted by this call.
	// Returns 0 if this atom is obsolete. This atom is deleted by this call.
	virtual DaAtom* UpgradeOrDelete()
	{
		return this;
	}

public:
	// 'why' is non-zero for system generated bounties. See DaPerson::AddBounty for meaning.
	virtual void AddBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward, int why)
	{
		Q_UNUSED(posterId);
		Q_UNUSED(posterName);
		Q_UNUSED(reason);
		Q_UNUSED(reward);
		Q_UNUSED(why);
	}

	// Add a visual and/or audio effect to this atom.
	// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
	// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
	virtual void AddEffect(AuEffect& effect);

	// Adjusts 'terAltMSL' based on my developments at or near 'll'.
	// Returns the adjusted altitude, which might not be different from 'terAltMSL'.
	virtual double AdjustAltMSLForDevelopments(const DaWorldLL& world, double terAltMSL, const AuGlobeLL& ll) const
	{
		Q_UNUSED(world);
		Q_UNUSED(ll);
		return terAltMSL;
	}

	// Adjusts 'terAltMSL' based on my developments at or near 'll'.
	// Returns the adjusted altitude, which might not be different from 'terAltMSL'.
	// Sets 'hide' bools to indicate affect on natural resources by developments at 'll'.
	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	// 'farmShrub' or 'orchardTree' will be set accordingly if the location falls within the area of a farm or orchard that is
	//   growing something, otherwise set to CID_Invalid.
	virtual double AdjustAltMSLForDevelopments(const DaWorldLL& world, double terAltMSL, const AuGlobeLL& ll, bool& hideRocks,
			int& hideGrass, bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub,
			double& farmShrubMSL, COMMODITYID& orchardTree, double& orchardTreeMSL) const
	{
		Q_UNUSED(world);
		Q_UNUSED(terAltMSL);
		Q_UNUSED(ll);
		Q_UNUSED(hideRocks);
		Q_UNUSED(hideGrass);
		Q_UNUSED(hideShrubs);
		Q_UNUSED(hideTrees);
		Q_UNUSED(pavementType);
		Q_UNUSED(farmShrub);
		Q_UNUSED(farmShrubMSL);
		Q_UNUSED(orchardTree);
		Q_UNUSED(orchardTreeMSL);
		return terAltMSL;
	}

	// Adjusts 'navLoc' so it is not inside the atmosphere/photosphere of any planets/stars or other objects a navigator would
	// perceive.
	// Result will be a close orbit radius outside of those.
	// 'shipLoc' is the reference loc for deciding where to move 'navLoc'. It is probably the location of a navigator's ship.
	// 'shipLoc' and 'navLoc' are in solar system coordinates.
	// Recursive.
	virtual void AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const;

	virtual quint8 AgentBits() const
	{
		return 0;
	}

	inline bool AgentAnimal() const
	{
		return AgentBits() & AGENT_Animal;
	}

	inline bool AgentEntity() const
	{
		return AgentBits() & AGENT_Entity;
	}

	inline bool AgentIncognito() const
	{
		return AgentBits() & AGENT_Incognito;
	}

	inline bool AgentInvincible() const
	{
		return AgentBits() & AGENT_Invincible;
	}

	inline bool AgentInvisible() const
	{
		return AgentBits() & AGENT_Invisible;
	}

	inline bool AgentPowerEnabled() const
	{
		return AgentBits() & AGENT_Enable;
	}

	// Returns the amount of ambient light added by this atom (city or building) at a location.
	// Does not include environmental ambient light such as starlight or sunlight.
	virtual float AmbientLightEmitted(const AuGlobeLLF& ll) const
	{
		Q_UNUSED(ll);
		return 0;
	}

	// Returns asteroid belt density at 'envLoc', for particle effects.
	// To be clear, an asteroid belt is a solar system object that occupies a primary orbit. The atoms there are DaWorldGlobes
	// that respond with BODY_Planetoid to their BodyType function.
	// This is not the same as the DaAsteroid atom. They are not belt asteroids.
	virtual float AsteroidBeltDensityRecursive(const AuPoint& envLoc) const;

	inline ATOM_STATE AtomState() const
	{
		return m_State;
	}

	virtual void AvatarAccountActiveRecursive(const AuIdList& land);

	// Atoms that are bodies in a solar system return an appropriate type here.
	virtual BODY_TYPE BodyType() const
	{
		return BODY_TYPE_Count;
	}

	inline QString BodyTypeName() const
	{
		return ::BodyTypeName(BodyType());
	}

	virtual DaAtom* Clone() const = 0;

	void ConnectSignals();

	// Creates an atom on the heap of 'atomType'.
	// Returns the atom or 0 on failure.
	static DaAtom* Create(ATOM_TYPE atomType);

	// Called the first time this object exists in the universe.
	// Allocates the next available atom id from the database.
	void Create();

protected:
	// Called the first time this object exists in the universe.
	// DaGalaxy uses this one to pass a predetermined atom id. Nobody else probably has a need to use it. Don't do it.
	void Create(const AuId& myNewId);

public:
	// Returns a brief name for qDebug messages
	virtual QString DebugName() const;

	void InvalidateEnvironment() const;

	void InvalidateEnvironmentRecursive() const;

	// Forces recreate of environment query. Calls CreateEnvironmentQuery. 
	// Used to be ::QueryEnvironment
	void UpdateEnvironment();

	// This always returns a non-zero pointer and locks the environment. If the current environment is not valid, then it is
	// updated with a call to CreateEnvironmentQuery.
	// Caller must unlock the environment.
	const AuQueryEnvironment* LockEnvironmentQuery() const;

	inline void UnlockEnvironmentQuery() const
	{
		m_EnvironLock.unlock();
	}

	// Called when my parent Bd building is surrendered to another empire.
	virtual void BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire)
	{
		Q_UNUSED(surrenderingEmpire);
		Q_UNUSED(toEmpire);
	}

	// Returns the vector from my atom origin to the eye camera location of my body model.
	// Beware that on the server the HeadOffset is returned, which differs from the client.
	// This works for the server for aiming purposes, since the server does not load head geometry, but the client often needs the
	// exact eye camera location.
	virtual AuPoint CameraOffset() const
	{
		return AuPoint(0,0);
	}

	virtual bool CanGrabLadderOnMyBd() const
	{
		return false;
	}

	virtual bool CanGrabLadderOnMyShip() const
	{
		return false;
	}

	// Returns the vector from my atom location to the CG of my body model.
	// In a typical standing pose, this would be the reverse of the CameraOffset.
	// In a straddle pose, the CG is at my body's pivot point so I don't move wierdly when looking up and down on a mount.
	virtual AuPoint CGOffset() const
	{
		return AuPoint(0,0);
	}

	// Fills 'places' and the ids of the places up my parent chain to the universe.
	// Maximum return list would be moonId, planetId, systemId, sectorId, galaxyId, universeId.
	void ChainOfPlaces(AuIdList& places) const;

	// Does two things:
	//  1) Changes ownership of private property 'propertyId' 'propertyName' to 'ownerId' 'ownerName'.
	//  2) Nullifies all outstanding ownership titles for 'propertyId', including the one used to claim ownership if any.
	virtual void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire);

	void ClearPendingDamage();

	// Apply damage due to collision of this atom with another atom.
	// 'myChange' is my movement vector change due to the collision.
	// 'myHitLoc' is the collision location relative to this atom.
	// 'hit' is the atom that I have hit.
	// 'hitChange' is the hit atom's movement vector change due to the collision.
	// 'hitFromLoc' is my location, relative to the hit atom.
	// 'hitResultVec' is the resulting movement vector relative to the hit atom.
	// 'hitShield' is true if the shield bubble was hit on the 'hit' atom.
	void Collide(const AuPoint& myChange, const AuPoint& myHitLoc, DaAtom* hit, const AuPoint& hitChange, const AuPoint& hitFromLoc,
			const AuPoint& hitResultVec, bool hitShield);

	// 'berthType' is a AuRosterEntry::TYPE
	virtual bool CommAcceptBerth(int berthType, const AuCommAcceptBerth& msg)
	{
		Q_UNUSED(berthType);
		Q_UNUSED(msg);
		return false;
	}

	// Returns my name when I am talking on a comm channel.
	// Base class returns Name().
	virtual QString CommName() const;

	// Returns the nearest common parent in the parenting chain between this atom and 'otherAtom'.
	// If a common parent atom is already known, it can be passed in 'knownCommonParent'.
	// If 'knownCommonParent' is 0, the environment atom is tried.
	DaAtom* CommonParent(const DaAtom* otherAtom, const DaAtom* knownCommonParent) const;

	// Locates an effect atom subordinate to this atom to which effects may be added. Creates a nu effect atom if none is
	// already available.
	DaEffect* CreateEffect();

	static void DbFindAtom(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& atomId, bool reportGear,
			bool reportMemory, const char* prefixStep);
	static bool DbFindParent(QSqlDatabase& dbUniverse, const AuId& atomId, const AuId& findParentId);
	static void DbFindPerson(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& personName, bool reportGear,
			bool reportMemory, const char* prefixStep);
	static void DbFindSpacecraft(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& shipName, bool reportGear,
			bool reportMemory, const char* prefixStep);
	static void DbItemContent(QStringList& textLines, const AuItemContent& content, const char* prefix, const char* prefixStep);
	static QStringList DbLimboCheck(QSqlDatabase& dbUniverse, const char* prefixStep);
	static QStringList DbLostShips(QSqlDatabase& dbUniverse, AuIdList& limboRescueMe);
	static AuId DbQueryAtom(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& atomId, bool includeGear,
			bool includeMemory, const char* prefix, const char* prefixStep);
	static void DbQuerySystem(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& systemName, const char* prefixStep);
	static void DbQuerySystem(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& systemId, const char* prefixStep);
	static void DbQueryWorld(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& worldName, const char* prefixStep);
	static void DbQueryWorld(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& worldId, const char* prefixStep);

	// Returns false if the atom is destroyed rather than depart a solar system.
	virtual bool DepartingSolarSystem()
	{
		return true;
	}

	void DesignSetGravity(bool gravOn);

	virtual void DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc);

	// Calculates the distance between this atom and 'otherAtom' using the CommonParent. It is more work but produces a more
	// accurate result, especially when I am moving through a solar system aboard a spacecraft.
	double DistanceToAtom(const DaAtom* otherAtom, const DaAtom* knownCommonParent) const;

	// Calculates the squared distance between this atom and 'otherAtom' using the CommonParent. It is more work but produces a more
	// accurate result, especially when I am moving through a solar system aboard a spacecraft.
	// This is slightly faster than Distance( because it performs one less square root calculation to determine the final distance.
	double DistanceToAtom2(const DaAtom* otherAtom, const DaAtom* knownCommonParent) const;

	// On the client, the read/write lock is not needed because multiple threads aren't hitting me.
	inline const AuEffectArray* EffectsOnClient() const
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_Effects;
	}

	// On the client, the read/write lock is not needed because multiple threads aren't hitting me.
	inline AuEffectArray* EffectsOnClient()
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_Effects;
	}

	// Notification that an empire was deleted.
	virtual void EmpireDeleted(quint32 empireId)
	{
		Q_UNUSED(empireId);
	}

	virtual quint32 EmpireId() const
	{
		return 0;
	}

	// Returns the vertical speed at which this atom starts to take falling damage, in feet per second.
	virtual double FallSafeSpeed() const
	{
		return 0;
	}

	// Determines the effects of a fall. Can be called on client.
	// 'oldCgAlt' is my old vertical altitude.
	// 'nuCgAlt' would be my nu vertical altitude if my fall was not stopped.
	// 'stopMove' is the vertical move distance that was prevented.
	// 'elapsedMs' is the milliseconds of time during which the stop happened.
	// 'stopReason' is 0=hit building floor, 1=hit building ceiling, 2=hit terrain
	virtual void FallStopped(double oldCgAlt, double nuCgAlt, double stopMove, int elapsedMs, int stopReason);

	// Applies the effects of a fall.
	void Fell(quint32 hits, int reason);

	// Returns current flight instrument readings:
	//  'centerOffset' specifies an offset to this atom's location for position calculations.
	//  'mode' specifies relativity desired: 0=auto, 1=planet, 2=system, 3=sector, 4=star
	// Readings are returned in the following parameters:
	//  'speed' overall speed in fps, always positive or zero.
	//  'speedF' forward speed in fps, negative is backward
	//  'speedL' lateral speed in fps, negative is left
	//  'speedV' vertical speed in fps, negative is down
	//  'altitude' is altitude above reference in feet. MSL altitude in planet mode.
	//  'pitch' is pitch angle
	//  'roll' is roll angle
	//  'heading' is heading angle
	//  'longitude' is real longitude angle. Negative is west; positive is east. This matches AuPoint::ArcTangent_2D.
	//  'latitude' is real latitude angle. Negative is south; positive is north. This matches AuPoint::PolarIncline.
	// returns actual mode of reference.
	// returns 0 if 'mode' produced no reference target; only speed values will be valid.
	int FlightInstruments(const AuPoint& centerOffset, int mode, double& speed, double& speedF, double& speedL, double& speedV,
			double& altitude, double& pitch, double& roll, double& heading, double& longitude, double& latitude) const;

	// Forces controller to accept in-motion.
	void ForceMotion();

	// Forces controller to accept atom position.
	void ForcePos(const AuAtomPos& pos);

	// Forces controller to accept atom position, in-motion, and movement vector.
	void ForcePosMove(const AuAtomPos& pos, const AuPoint& vector);

	// Forces controller to accept atom position and at-rest.
	void ForcePosStop(const AuAtomPos& pos);

	// Forces controller to accept a movement vector and in-motion.
	void ForceVector(const AuPoint& movementVector);

	// Returns the base location beneath my CG, where I would settle to the ground if upright.
	AuPoint BaseLoc() const;

	// True if this is an agent who has their powers enabled right now.
	virtual bool HasAgentPower() const
	{
		return false;
	}

	// Non recursive check of my subatoms.
	bool HasConsoleOperator(quint16 stationId) const;

	virtual bool HasSensors() const
	{
		return false;
	}

	// 'effect' is a SOUND_EFFECT
	bool HasSoundEffect(int effect);

	// 'effect' is a VISUAL_EFFECT
	bool HasVisualEffect(int effect);

	virtual AuPoint HeadOffset() const
	{
		return AuPoint(0,0);
	}

	// Returns the highest orbit index of a world subatom that is at 'orbitRadiusAu'. Only one level of depth. I should be a sun.
	int HighestWorldOrbitIndex(double orbitRadiusAu) const;

	// Called on solar system objects after the system is finalized, so orbiting bodies can position themselves appropriately
	// without waiting for the next tick or tock cycle.
	virtual void InitSystemPosition()
	{
	}

	// Used for determination of color highlighting based on political stance toward this unit.
	// Civilian units use a different color palette than military units.
	virtual bool IsCivilian() const
	{
		return true;
	}

	// Test for console manned by a person.
	bool IsConsoleManned(quint16 stationId) const;
	bool IsConsoleSeatManned(quint16 stationId, unsigned int seatIndex) const;

	// Test for console occupied by anyone, including people and ai controlled citizens.
	bool IsConsoleOccupied(quint16 stationId) const;
	bool IsConsoleSeatOccupied(quint16 stationId, unsigned int seatIndex) const;

	inline bool IsEnvTransformValid() const
	{
		return m_EnvTransformValid;
	}

	// Returns true for atoms that do not obstruct the movement of anything.
	virtual bool IsEthereal() const = 0;

	// Returns true if this subatom is visible when the scene atom is outside the atmosphere and beyond far viewing distance of
	// my parent world atom.
	virtual bool IsVisibleFarWorld() const
	{
		return DoesQueryEnvironment() && !IsInAtmosphere();
	}

	// Returns true if this subatom is visible when the scene atom is outside the atmosphere but within near viewing distance of
	// my parent world atom.
	virtual bool IsVisibleNearWorld() const
	{
		return DoesQueryEnvironment() && !IsInAtmosphere();
	}

	// Recursively invalidates the system transform of this atom and all its subatoms.
	void InvalidateEnvTransform();

	// Returns true if this atoms position is currently controlled by a client. The client determines all transformation and
	// parent changes for smoother client-side movement.
	bool IsClientControlled();
	bool IsClientControlledNoLock();	// Expects m_PosLock to already be locked.

	inline bool IsClientControlledNoReset() const
	{
		m_PosLock.lockForRead();
		bool is = !m_Controller.IsNull() && m_tmControlled.isValid() && m_tmControlled.elapsed() < 1500;
		m_PosLock.unlock();
		return is;
	}

	// Returns true if 'c' is controlling me right now.
	inline bool IsController(const AuId& c)
	{
		bool clearC = false;
		m_PosLock.lockForRead();
		bool isc = m_Controller == c && m_tmControlled.isValid();
		if(isc && m_tmControlled.elapsed() > 1500)
		{
			// Turn off the controlled time so we stop calling elapsed(), in case the controller got left on.
			isc = false;
			clearC = true;
		}
		if(clearC)
			SetControlledNoLock(AuId());
		m_PosLock.unlock();
		if(clearC)
			ApplySync();				// The server applies any remaining syncs, to be sure I am where I'm supposed to be.
		return isc;
	}

	inline bool IsControllerNoReset(const AuId& c) const
	{
		m_PosLock.lockForRead();
		bool isc = m_Controller == c && m_tmControlled.isValid() && m_tmControlled.elapsed() < 1500;
		m_PosLock.unlock();
		return isc;
	}

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
	// 'gunnerPosture' is the posture of the gunner.
	// 'targetLoc' is an output parameter relative to my parent world.
	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const
	{
		Q_UNUSED(gunnerEmpire);
		Q_UNUSED(gunnerPosture);
		Q_UNUSED(gunLoc);
		Q_UNUSED(gunRange);
		Q_UNUSED(targetLoc);
		return false;
	}

	// Determines my turret or weapon firing domain.
	// Sets 'worldId' to my parent world id if my domain is not space.
	// Returns 0=underground, 1=underwater, 2=atmosphere, 3=space
	virtual int GunnerDomain(AuId& worldId) const;

	// Returns true if I am a potential target to the gunner.
	bool IsGunnerDomainTarget(int gunnerDomain, const AuId& gunnerWorldId, const AuId& gunnerCapsuleId) const;

	// Returns true if this atom is climbing a ladder.
	virtual bool IsClimbingLadder() const
	{
		return false;
	}

	// True if this atom's empire is a "pirate" empire.
	virtual bool IsPirate() const
	{
		return false;
	}

	// Return true if this atom can be targetted by a turret on a spacecraft.
	// 'shipId' is the ship the gunner is aboard firing a turret.
	// 'gunnerDomain' 0=underground, 1=undersea, 2=atmo, 3=space
	// 'gunnerWorldId' is gunner's world id, if not in space
	// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
	// 'gunSysVec' imposes a hemisphere restriction to turret coverage, 0,0 ignores this restriction.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const
	{
		Q_UNUSED(shipId);
		Q_UNUSED(gunnerDomain);
		Q_UNUSED(gunnerWorldId);
		Q_UNUSED(gunnerEmpire);
		Q_UNUSED(gunnerPosture);
		Q_UNUSED(gunSysLoc);
		Q_UNUSED(gunRange);
		Q_UNUSED(gunSysVec);
		Q_UNUSED(targetSysLoc);
		return false;
	}

	// 'shipId' is the ship the gunner is aboard firing a turret.
	// 'gunnerDomain' 0=underground, 1=undersea, 2=atmo, 3=space
	// 'gunnerWorldId' is gunner's world id, if not in space
	virtual bool IsShipWeaponTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, AuPoint& targetSysLoc) const
	{
		Q_UNUSED(shipId);
		Q_UNUSED(gunnerDomain);
		Q_UNUSED(gunnerWorldId);
		Q_UNUSED(gunnerEmpire);
		Q_UNUSED(gunnerPosture);
		Q_UNUSED(shipSysLoc);
		Q_UNUSED(gunRange);
		Q_UNUSED(targetSysLoc);
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual void MovedByTurboLift();

	// Paints this atom onto a surface map of a planet.
	virtual void PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const
	{
		Q_UNUSED(world);
		Q_UNUSED(p);
		Q_UNUSED(width);
		Q_UNUSED(height);
	}

	virtual bool SetGeneralPose(const AuPose& pose, quint16 stationId, bool setModified=true)
	{
		Q_UNUSED(pose);
		Q_UNUSED(stationId);
		Q_UNUSED(setModified);
		return false;
	}

	inline AuEther* Ether() const
	{
		m_EtherLock.lockForRead();
		AuEther* ether = m_Ether;
		m_EtherLock.unlock();
		return ether;
	}

	inline void SetEther(AuEther* ether)
	{
		m_EtherLock.lockForWrite();
		m_Ether = ether;
		m_EtherLock.unlock();
	}

	virtual void SetVehicleSeatIndex(quint8 seatIndex)
	{
		Q_UNUSED(seatIndex);
	}

	// Head tilts at base of head. 0 = level
	// Restricts head tilt if in a turret.
	// Returns true if head tilt actually changes.
	virtual bool TiltHeadTo(float angle)
	{
		Q_UNUSED(angle);
		return false;
	}

	virtual TRANSPORTER_TARGET TransporterTarget() const
	{
		return TT_Invalid;
	}

	virtual QString TransporterTargetName() const;

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const
	{
		Q_UNUSED(shipSysLoc);
		Q_UNUSED(harvesterModule);
		return EnvLocation();
	}

	// Sets 'attacker' and 'weapon' to appropriate values when this atom collides with another atom, for damage determination.
	// 'hitShield' is 0=No, 1=I hit shield, 2=My shield was hit.
	// 'kineticEnergy' is the amount of kinetic energy of the impact. K = 1/2 * m * v^2
	// Base class sets up for bludgeon damage.
	// Returns false if no damage results to the target.
	virtual bool WeaponOnCollision(AuCombatAttacker& attacker, AuCombatWeapon& weapon, int hitShield, double kineticEnergy) const;

	// Checks political stance of 'myEmpireId' toward 'otherEmpireId'. Makes first contact if empires did not previously know
	// about each other. Makes dossier entries.
	POLITICAL_STANCE PoliticalStanceMakeContact(quint32 myEmpireId, const DaAtom* other, quint32 otherEmpireId) const;

	// Non-recursive tally of atoms that contribute population to a world.
	virtual quint32 Population() const
	{
		return 0;
	}

	// 'empireId' is the empire of the atom checking to transport.
	// 'loc' is relative to 'world' atom that is the parent of this atom.
	// Returns true if 'loc' is inside my active shield bubble and it has enough power to block transporters.
	virtual bool ShieldsBlockPeerTransport(quint32 empireId, const AuPoint& loc) const
	{
		Q_UNUSED(empireId);
		Q_UNUSED(loc);
		return false;
	}

	// Returns payment.
	virtual quint32 ShipmentDeliver(const AuItemTEUContainer& shipment)
	{
		Q_UNUSED(shipment);
		return 0;
	}

	// Returns true if this atom or any of its subatoms is changed as a result of this call.
	virtual bool SolarSystemNameChanged(const QString& oldName, const QString& nuName, bool notifyDb)
	{
		Q_UNUSED(oldName);
		Q_UNUSED(nuName);
		Q_UNUSED(notifyDb);
		return false;
	}

	virtual void PrepareForScan()
	{
	}

	// Used on client to produce the scan report information available by selecting stars, planets and solar systems on the
	// system scan report dialog.
	// 'scanEx is 2 when the system has been scanned, 1 when the system has been explored, 0 when only observed from afar.
	virtual QString SystemScanReport(int scanEx) const
	{
		Q_UNUSED(scanEx);
		return "";
	}

	// Used on client to produce the scan report information available by selecting stars, planets and solar systems on the
	// star map scan report xml export.
	virtual void SystemScanReportXML(QXmlStreamWriter& stream, int scanEx) const
	{
		Q_UNUSED(stream);
		Q_UNUSED(scanEx);
	}

	// Used by system survey scan report to make a list of the best resources on a star or world.
	// 'atmRes', 'hydRes', 'bioRes' and 'geoRes' are filled with all the natural resources this atom (world or star) has to
	// offer. Key is quality, value is COMMODITYID.
	virtual void SystemScanResources(QMultiMap<quint8, COMMODITYID>& atmRes,
			QMultiMap<quint8, COMMODITYID>& hydRes,
			QMultiMap<quint8, COMMODITYID>& geoRes,
			QMultiMap<quint8, COMMODITYID>& bioRes) const
	{
		Q_UNUSED(atmRes);
		Q_UNUSED(hydRes);
		Q_UNUSED(geoRes);
		Q_UNUSED(bioRes);
	}

	// Returns true if this atom has any subatoms, regardless of their current state.
	bool HasSubatoms() const;

	// 'hit' is the atom that was hit.
	// 'hitLoc' is the point of collision, relative to this atom.
	// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
	// 'hitShield' is true if the shield bubble around the atom was hit.
	// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
	// occurred. It is simply the reflected movement vector when the hit occurred.
	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	// Tests for presense of another identical meat pile that is too close to loc.
	bool OkToDropMeat(const AuPoint& loc, COMMODITYID cidMeat, const AuDNABase* dna) const;

	bool OkToTick(const AuId& inSystemId) const;

	bool OkToTock(const AuId& inSystemId) const;

	// Return true if this atom causes automatic doors to remain open simply by being near them.
	virtual bool KeepsAutomaticDoorsOpen() const
	{
		return false;
	}

	// Return true if this atom causes automatic doors to open when moving near them.
	virtual bool OpensAutomaticDoors() const
	{
		return false;
	}

	// Packs this atom's item type and data into a byte array using operator>>. Use Unpack to restore.
	QByteArray Pack() const;

	// Called when this atom is credited with killing a unit.
	// All unit kills go through here.
	// This is where empires declare mutual enemies based on unit kills. See DaUnit::UnitKilled.
	virtual void UnitKilled(DaUnit* /*victim*/)
	{
	}

	// Restores an atom from a byte array that was stored using Pack.
	// The returned pointer is allocated on the heap and becomes the property of the caller.
	static DaAtom* Unpack(const QByteArray& ba);

	void CreateForScan();

	virtual void CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const
	{
		Q_UNUSED(world);
		Q_UNUSED(roadMap);
	}

	// NOTE: pAtom is not a const but it must not be altered by this call. Call pAtom->RefreshComplete to do post refresh stuff.
	// 'ari' is non-zero when refreshing the atom used in the view. The OpenGL context is current in this case.
	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Called after this atom is used to refresh another atom, to clean up things that only get refreshed once, e.g. effects.
	virtual void RefreshComplete();

	// NOTE: Rendering states must already be configured. Check actual usage of this function.
	// 'isTarget' is true when rendering the stats of a targeted atom, not the scene atom or its mount.
	// 'targetPart' is the sightedPart returned by SightTarget.
	// 'stance' is the political stance of the scene atom toward this atom.
	virtual void RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
			POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
			const AuMatrix4x4& tDevOrtho, float hudIntensity)
	{
		Q_UNUSED(left);
		Q_UNUSED(bottom);
		Q_UNUSED(wid);
		Q_UNUSED(ht);
		Q_UNUSED(isTarget);
		Q_UNUSED(targetPart);
		Q_UNUSED(stance);
		Q_UNUSED(ari);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(tDevPersp);
		Q_UNUSED(tDevOrtho);
		Q_UNUSED(hudIntensity);
	}

	// Paints this atom onto a surface map of a planet, if appropriate.
	// 'pixelsPerFoot' is the number of pixels of texture width per foot of circumference at the equator of the globe.
	virtual void RenderOnSurfaceTexture(AuGLEffects& re, float radiansPerFootX, float radiansPerFootY) const
	{
		Q_UNUSED(re);
		Q_UNUSED(radiansPerFootX);
		Q_UNUSED(radiansPerFootY);
	}

	// Returns parameter values needed by shaders when this atom is rendered in a scene viewed by 'sceneAtom'.
	// 'this' atom could be the 'sceneAtom'.
	virtual void SceneRenderParameters(const DaAtom* sceneAtom, float& ambientR, float& ambientG, float& ambientB,
			float& fogDensity, float& fogR, float& fogG, float& fogB, bool viewIsTopDown, bool& sunsBlocked) const;

	// Called whenever this atom's position is directly updated by a client.
	// Call with null 'controller' to turn off.
	void SetControlled(const AuId& controller);
	void SetControlledNoLock(const AuId& controller);	// Expects m_PosLock to already be locked.

	inline void SetDormant()
	{
		if(!IsDead())
		{
			m_State = Dormant;
			TickTock();
		}
	}

	// Called whenever this atom appears in a scene.
	// Atoms can use this for decay or AI logic.
	inline void SetInScene()
	{
		// This function may be called by multiple threads simultaneously on DaUniverse, DaGalaxy, and DaSector atoms.
		bool wasInScene = IsInAnyScene();
		m_AnyScene = 3;	// Tock decrements this by one so it will take about 30 seconds to be 0.
		if(!wasInScene)
			TickTock();	// Many atoms animate slowly when not in any scene. They need to be awakened.
	}

	inline void SetNormal()
	{
		if(!IsDead())
			m_State = Normal;
	}

	inline void SetTrash()
	{
		m_State = Trash;
		TickTock();
	}

	inline void SetTraveling()
	{
		if(!IsDead())
		{
			m_State = Traveling;
			TickTock();
		}
	}

	inline double Speed() const	// Forward/reverse speed
	{
		m_PosLock.lockForRead();
		double v = m_Vector.Length();
		m_PosLock.unlock();
		return v;
	}

	inline double ForwardSpeed() const
	{
		return Vector().y;
	}

	inline double LateralSpeed() const
	{
		return Vector().x;
	}

	inline double VerticalSpeed() const
	{
		return Vector().z;
	}

	virtual void TestHullStress()
	{
	}

	// This function is only called on atoms who are directly subordinate to the same parent atom as the missile.
	// 'missileLoc' is the location of the missile relative to our parent atom.
	// 'proximity' is the distance within which the missile will detonate.
	// Returns 0=not hit, 1=hit, 2=hit shield
	virtual int TestMissileHit(const AuPoint& missileLoc, float proximity) const
	{
		Q_UNUSED(missileLoc);
		Q_UNUSED(proximity);
		return 0;
	}

	// Returns true if I appear to have been Ticked recently.
	inline bool IsTicking() const
	{
		// m_Ticked is set to 2 in Tick.
		// m_Ticked is decremented during each call to :Tock, nominally 10 seconds apart.
		return m_Ticked;
	}

	// Returns true if this atom must not be Ticked under any circumstances. i.e. traveling, dormant, aboard ship w/o design cache.
	bool MustNotTickTock() const;

	// Only atoms that return true to IsTicker will have their Tick function called.
	// Only relevant for atoms that can be subordinate to a solar system. Only solar system contents are ticked.
	virtual bool IsTicker() const;

	// Primary physics and other high priority processing. Target frequency is 30Hz.
	// Current solar system is locked for write when called.
	// Returns false if this atom is to be deleted upon return.
	// NOTE: Solar systems stop ticking subatoms three minutes after their contents are no longer active. i.e. not needed
	// by players on line or officers.
	// This function is only called on atoms that return true to IsTicker.
	// Not recursive.
	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	// Move due to movement vector, i.e. gravity, inertia. This requires up-to-date environment query.
	// Movement is performed before moving subatoms due to EnvTransform.
	virtual void TickMovement(const AuElapsedTimer& last);

	virtual void TickOnClient(const AuElapsedTimer& last);

	// Updates Tick/Tock queue based on current state/needs of this atom. Atoms in motion are always added to the Tick queue.
	// Not recursive.
	void TickTock() const;

	// This is the recursive version of TickTock(). Calls TickTock on myself. Calls TickTockRecursive on all my subatoms, regardless
	// of my own tick/tock needs.
	void TickTockRecursive() const;

	// Only atoms that return non-zero to IsTocker will have their Tock function called.
	// Only relevant for atoms that can be subordinate to a solar system. Only solar system contents are tocked.
	// The number returned is the number of milliseconds desired between calls to Tock.
	virtual unsigned int IsTocker() const;

	// Low priority processing. Target frequency is based on the value returned by IsTocker.
	// Not recursive.
	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	// This function is called when a chat message is received on a channel that is being monitored by this atom.
	// This function is also called when orders are issued to units via chat message.
	// 'firstInGroup' is true when this unit is the first in a group of similar units to receive this order message, so only one
	// unit acknowledges the order instead of a large group of them.
	// 'countSupplied' is used when boarding crew and passengers so multiple cities can take a crack at supplying some citizens
	// until the request is filled.
	virtual void RxChatMessage(const AuCommPacket* /*msg*/, quint32& /*countSupplied*/, bool /*firstInGroup*/=true)
	{
	}

	// These functions are used when the server transmits this atom to a client.
	// The server maintains the time each client was last updated so atoms can
	// be more efficient about what is transmitted. This requires derived
	// classes to monitor change times of specific properties or groups of
	// properties. A bitmask byte can then be put in the buffer to indicate
	// what is being updated.
	void RxUpdateId(QDataStream& fromServer);	// Rx atom id only
	void RxUpdateBase(QDataStream& fromServer);	// Rx remainder of DaAtom
												// portion only
	// Rx the rest, after parenting
	virtual void RxUpdate(QDataStream& /*fromServer*/, bool /*isFirstUpdate*/)
	{
		// DaAtom has all been received by now.
	}
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	inline QString AtomTypeName() const
	{
		return AtomTypeName(AtomType());
	}

	static QString AtomTypeName(ATOM_TYPE atomType);

public:
	bool Is(const AuId &id) const		// true if this Atom IS 'id'.
	{
		return (m_Id == id) ? true : false;
	}

	// Returns true if this atom has been in any client scene within the past 30 seconds, approximately.
	inline bool IsInAnyScene() const
	{
		// m_AnyScene is set to 3 in SetInScene.
		// m_AnyScene is decremented during each call to :Tock, nominally 10 seconds apart.
		return m_AnyScene;
	}

	// Returns true if this atom appears to be in transition between systems. Client shows shooting stars wait animation when the
	// scene atom returns true to this call. Recurses up the atom tree if parent is a creature or vehicle or if this atom is
	// subordinate to a spaceship at any level.
	bool IsInTransition() const;

	// Returns true for units whose empire is dormant due to nobody being on line.
	// Returns true if this atom is currently prevented from attacking others and protected from attack.
    virtual bool IsNoncombatant() const
    {
        return false;
    }

	// true if this atom is riding on a creature, in any capacity, either as a passenger or as a carried body or equipment.
	bool IsRider() const;

	// Kills this atom and all subatoms recursively.
	// The database is signalled to delete the corresponding records.
	// Most normal atom kills are done this way.
	virtual void Kill();

	// Kills a subatom. It will be deleted during the next Tick cycle.
	// Recursive.
	void KillSubatom(const AuId &id);

	// Kills all subatoms. They will be deleted during the next Tick cycle.
	// Recursive.
	void KillSubatoms();

	// Recursively trashes atoms !InStarField.
	void TrashNotInStarField();

	// Trashes a subatom recursively and removes it now.
	void TrashSubatom(const AuId &id);

	// Trashes all subatoms recursively and removes them now.
	void TrashSubatoms();

	// Causes this atom to stop dying.
	void StopDying()
	{
		m_DeathLock.lockForWrite();
		m_tmDeath.SetNull();	// Set death time to null to keep living.
		m_DeathLock.unlock();
	}

	// This atom is to die by the time tDeath, but may die sooner.
	inline void DieBy(const AuTime& tDeath)
	{
		m_DeathLock.lockForWrite();
		if(m_tmDeath.IsNull() || m_tmDeath > tDeath)
			m_tmDeath = tDeath;
		m_DeathLock.unlock();
	}

	// Returns -1 if 't' is earlier than this atom's modify time, 0 if equal, 1 if t is past.
	inline int CompareModifyTime(const AuTime& t) const
	{
		int result;
		m_ModifyLock.lockForRead();
		if(m_tmModify < t)
			result = -1;
		else if(m_tmModify == t)
			result = 0;
		else
			result = 1;
		m_ModifyLock.unlock();
		return result;
	}

	// 'notify' is a bit mask: b0=store in database (calls g_Au.AtomChanged(); b1=update scene clients (calls SetModifyTime()).
	virtual void SetModified(SET_MODIFIED_BITS notifyBits=NOTIFY_All);

	// Returns Name() with special characters substituted, e.g. '&' becomes '&amp;'.
	inline QString HtmlName() const
	{
		return HtmlName(Name());
	}

	static QString HtmlName(const QString& text);

	// Returns true for Dead atoms.
	// Returns true for Trash atoms even though they may not have been 'Killed', per se. Trash atoms are ignored even more so than
	// dead atoms in that they do not get Ticked, whereas Dead atoms do get ticked in order to become Trash.
	// Atoms are marked as trash when they depart the local data, when the local atom gets moved to the trash. In that case the
	// atom is trashed but not killed.
	inline bool IsDead() const
	{
		return m_State == Dead || m_State == Trash;
	}

	inline bool IsDormant() const
	{
		return m_State == Dormant;
	}

	inline bool IsDying() const
	{
		// If a time of death has been set, this atom is dying.
		m_DeathLock.lockForRead();
		bool is = !m_tmDeath.IsNull();
		m_DeathLock.unlock();
		return is;
	}

	// 'sceneAtom' is the atom who is viewing the scene.
	// 'cameraEnvLoc' is the location of the scene camera, relative to the environment.
	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
	{
		Q_UNUSED(sceneAtom);
		Q_UNUSED(cameraEnvLoc);
		// Atoms who are dead or being transferred do not appear in any scene.
		// This test is always made before IsInScene is called. It is redundant here.
		//return IsNormal();
		return true;
	}

	inline bool IsNormal() const
	{
		return m_State == Normal;
	}

	virtual bool IsStaticDisplay() const
	{
		return false;
	}

	inline bool IsTrash() const
	{
		return m_State == Trash;
	}

	inline bool IsTrashOrInvalid() const
	{
		return m_State >= Trash;
	}

	inline bool IsTraveling() const
	{
		return m_State == Traveling;
	}

	virtual QString Name() const		// Returns a brief name for listing
	{									// purposes. e.g. combo boxes and lists
		return("Atom");
	}

	// Base atom class does not store a name.
	virtual void SetName(const QString& /*nuName*/)
	{
	}

	virtual QString SpotDescription(quint32 /*spotterEmpire*/) const
	{
		return Name();
	}

	QString SpottedLocation() const;

	virtual QString TargetDesc(const AuPoint& spottedLocation, quint32 spotterEmpire, AuId& targetId,
			POLITICAL_STANCE& stanceToTarget) const;

	QString StringId() const		// Report the id of this Atom as a string.
	{
		return m_Id.StringId();
	}

	virtual QString Description() const;	// Report a longer description of this Atom for examination purposes.

	const AuId &Id() const
	{
		return(m_Id);
	}

	void SetId(const AuId &idNew)
	{
		m_Id = idNew;
	}

	inline AuId ParentId() const
	{
		m_PosLock.lockForRead();
		AuId v(m_Pos.ParentId());
		m_PosLock.unlock();
		return v;
	}

public:
	DaAtom* Parent() const;

	// Returns my location relative to my parent atom.
	inline AuPoint Location() const
	{
		m_PosLock.lockForRead();
		AuPoint l(m_Pos.Translation());
		m_PosLock.unlock();
		return l;
	}

	inline AuPoint OrientRotation() const
	{
		m_PosLock.lockForRead();
		AuPoint r(m_OrientRotation);
		m_PosLock.unlock();
		return r;
	}

	inline AuPoint Rotation() const
	{
		m_PosLock.lockForRead();
		AuPoint r(m_Pos.Rotation());
		m_PosLock.unlock();
		return r;
	}

	inline AuPoint EnvLocation() const
	{
		return EnvTransform().Translation();
	}

	// Returns the transformation matrix relative to this atom's parent solar system.
	virtual AuMatrix4x4 EnvTransform() const;

	// Returns the location to use when targeting this atom with a weapon, relative to this atom.
	virtual AuPoint TargetLocation() const
	{
		return AuPoint(0,0);
	}

	inline AuMatrix4x4 Transform() const
	{
		m_PosLock.lockForRead();
		AuMatrix4x4 t(m_Pos.Transform());
		m_PosLock.unlock();
		return t;
	}

	inline AuTime DeathTime() const	// Access to scheduled time of death.
	{
		m_DeathLock.lockForRead();
		AuTime t(m_tmDeath);
		m_DeathLock.unlock();
		return t;
	}

	inline AuTime ModifyTime() const	// Access to time last modified.
	{
		m_ModifyLock.lockForRead();
		AuTime t(m_tmModify);
		m_ModifyLock.unlock();
		return t;
	}

	inline const AuTime& CreatedTime() const	// Access to time created.
	{
		return m_tmCreated;
	}

	inline void SetCreatedTime(const AuTime& tmCreated)
	{
		m_tmCreated = tmCreated;
	}

	inline void SetCreatedTime(const QDateTime& tmCreated)
	{
		m_tmCreated = tmCreated;
	}

	inline void SetDeathTime(const AuTime& tmDeath)
	{
		m_DeathLock.lockForWrite();
		m_tmDeath = tmDeath;
		m_DeathLock.unlock();
	}

	inline void SetDeathTime(const QDateTime& tmDeath)
	{
		m_DeathLock.lockForWrite();
		m_tmDeath = tmDeath;
		m_DeathLock.unlock();
	}

	inline void SetModifyTime(const AuTime& tmModify)
	{
		m_ModifyLock.lockForWrite();
		m_tmModify = tmModify;
		m_ModifyLock.unlock();
	}

	inline void SetModifyTime(const QDateTime& tmModify)
	{
		m_ModifyLock.lockForWrite();
		m_tmModify = tmModify;
		m_ModifyLock.unlock();
	}

	inline void SetTickTime(const AuElapsedTimer& tmTick)
	{
		m_TickLock.lockForWrite();
		m_tmTicked = tmTick;
		m_TickLock.unlock();
	}

	inline void SetTockTime(const AuTime& tmTock)
	{
		m_TockLock.lockForWrite();
		m_tmTocked = tmTock;
		m_TockLock.unlock();
	}

	inline void SetTockTime(const QDateTime& tmTock)
	{
		m_TockLock.lockForWrite();
		m_tmTocked = tmTock;
		m_TockLock.unlock();
	}

	inline AuElapsedTimer TickTime() const	// Access to time last modified.
	{
		m_TickLock.lockForRead();
		AuElapsedTimer t(m_tmTicked);
		m_TickLock.unlock();
		return t;
	}

	inline AuTime TockTime() const	// Access to time last modified.
	{
		m_TockLock.lockForRead();
		AuTime t(m_tmTocked);
		m_TockLock.unlock();
		return t;
	}

	inline AuAtomPos Pos() const
	{
		m_PosLock.lockForRead();
		AuAtomPos t(m_Pos);
		m_PosLock.unlock();
		return t;
	}

	inline AuElapsedTimer PosChangeTime() const
	{
		m_PosLock.lockForRead();
		AuElapsedTimer t(m_PosChange);
		m_PosLock.unlock();
		return t;
	}

	virtual void FindAIPeopleRecursive(const AuId& shipAssigned, DaAtomList& aipeople) const;

	virtual const DaPerson* FindAIPersonRecursive() const;

	virtual void FindAnimalsRecursive(DaAtomList& animals) const;

	// Find an IsNormal asteroid that is targetted on 'targetId'.
	virtual const DaAsteroid* FindAsteroidRecursive(const AuId& targetId) const;

	virtual void FindAsteroidsRecursive(DaAtomList& asteroids) const;

	// Not a recursive search.
	DaBd* FindBd(const AuId& bdId) const;

	// Lists all buildings subordinae to this atom.
	// Not recursive.
	void FindBds(DaAtomList& bds) const;

	// Lists all military buildings subordinate to this atom.
	// Not recursive.
	void FindBdBase(DaAtomList& bdBase) const;

	// Lists all buildings subordinate to this atom that have a capitol building of 'capitolBdId', also lists the capitol bd.
	// Not recursive.
	void FindBdCity(DaAtomList& bdCity, const AuId& capitolBdId) const;

	// Searches subatoms for a military HQ building.
	// Not recursive.
	DaBd* FindBdHQ() const;

	virtual void FindBdRecursive(DaAtomList& bds) const;

	virtual void FindBdRecursive(DaAtomList& bds, quint32 empireId) const;

	virtual void FindBeaconsRecursive(DaAtomList& cities) const;

	// Non-recursive search of my subatoms for a vessel.
	DaCapsule* FindCapsule() const;

	virtual void FindCapsulesRecursive(DaAtomList& capsules) const;

	// Recursively finds all Bd city capitols and Bd HQs.
	virtual void FindCitiesBdRecursive(DaAtomList& cities) const;

	// Recursively finds all Bd city capitols and Bd HQs that belong to a particular empire.
	virtual void FindCitiesBdRecursive(DaAtomList& cities, quint32 empireId) const;

	// Recursively finds bds that belong to empireId.
	// 'supplyCities' is filled with bds that could supply a spacecraft.
	// 'repairCities' is filled with bds that could supply and repair a spacecraft.
	// 'troopCities' is filled with bds that could deploy troops to a spacecraft.
	virtual void FindCitiesBdRecursive(DaAtomList& supplyCities, DaAtomList& repairCities, DaAtomList& troopCities,
			quint32 empireId);

	// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	virtual void FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc);

	virtual void FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& gunSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	// Recursively locates the best target for a guided missile to lock on to.
	// 'launcherId' is the atom who actually launched the missile. This atom is never considered to be a target even when friendly
	// fire is enabled.
	// Should probably start this search at the solar system atom.
	// 'bestAtom' should be 0 on first call.
	// 'bestDistance' should be the maximum range of the sensor lock on first call. It can be -1 to ignore maximum range.
	// 'bestSysLoc' is an output parameter related to the atom that is returned.
	// This base class implementation checks all subatoms. Does not consider this atom to be a target.
	virtual const DaAtom* FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
			quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
			double& bestDistance) const;

	// 'telLoc' is an output parameter in the coordinate system of this atom.
	// 'telDistance' is an output parameter.
	const DaTeleporter* FindClosestOriginTeleporter(const AuPoint& nearLoc, AuPoint& telLoc, double& telDistance) const;

	// 'loc' is relative to this atom.
	// 'personAtom' is a return value.
	// 'personRange' is a return value. It is the squared distance to the personAtom.
	void FindClosestPersonOnLineRecursive(const AuPoint& loc, DaPerson*& personAtom, double& personRange) const;

	virtual void FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
			const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
			const DaVessel*& enemyVessel, double& vesselRange,
			const DaVessel*& nonEnemyStation, double& stationRange,
			const DaBd*& enemyMilBd, double& milRange,	// Enemy military building
			const DaBd*& anyBd, double& bdRange) const;

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'shipId' is the ship the gunner is aboard firing a turret.
	// 'gunnerDomain' 0=underground, 1=undersea, 2=atmo, 3=space
	// 'gunnerWorldId' is gunner's world id, if not in space
	// 'gunSysVec' imposes a hemisphere restriction to turret coverage, 0,0 ignores this restriction.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// Find atoms that might cause shields to be raised by 'shipId'.
	virtual void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	// 'gunnerDomain' 0=underground, 1=undersea, 2=atmo, 3=space
	// 'gunnerWorldId' is gunner's world id, if not in space
	virtual void FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// Finds any creature recursively.
	virtual DaCreature* FindCreatureRecursive();

	virtual void FindCrewRecursive(const AuId& shipAssigned, DaAtomList& crewMembers) const;

	// Non recursive search for normal item subatoms that are not statically displayed.
	void FindItems(DaAtomList& items) const;

	virtual void FindItemsRecursive(DaAtomList& items, bool includeStaticDisplayedItems) const;

	// Non recursive search for normal item subatoms that are statically displayed.
	void FindItemsStatic(DaAtomList& items) const;

	// Finds !IsDead people, which gets IsNormal and IsTraveling people.
	virtual void FindLivePeopleRecursive(DaAtomList& people);

	// Finds DaWorldLL atoms that are subordinate to this atom and IsNormal. Not recursive.
	void FindMoons(DaAtomList& planets) const;

	// Find a star or world that has natural resource commodity 'resCID' at minimum quality 'minQ' in the environment.
	// Not recursive.
	virtual bool FindNaturalResource(COMMODITYID resCID, quint8 minQ) const
	{
		Q_UNUSED(resCID);
		Q_UNUSED(minQ);
		return false;
	}

	// Returns any npc who is in a normal state and whose story instance is still active.
    virtual DaNpc* FindNpcRecursive() const;

    virtual DaNpc* FindNpcRecursive(const QString& charName, quint32 storyInstance) const;

	virtual void FindNpcsRecursive(DaAtomList& npcs) const;

	virtual const DaNpc* FindOfficerRecursive() const;

	virtual void FindOfficersRecursive(const AuId& shipAssigned, DaAtomList& officers) const;

	// Recursively finds all DaWorld atoms that are IsNormal.
	virtual void FindPlanetsRecursive(DaAtomList& planets) const;

	virtual DaArena* FindParentArena();

	virtual const DaArena* FindParentArena() const;

	virtual const DaAtom* FindParentAtomId(const AuId& parentId) const;

	virtual DaBd* FindParentBdInEnv();

	virtual const DaBd* FindParentBdInEnv() const;

	virtual DaCapsule* FindParentCapsuleInEnv();

	virtual const DaCapsule* FindParentCapsuleInEnv() const;

	virtual DaDesignBase* FindParentDesignBaseInEnv();

	virtual const DaDesignBase* FindParentDesignBaseInEnv() const;

	virtual DaDesigner* FindParentDesignerInEnv();

	virtual const DaDesigner* FindParentDesignerInEnv() const;

	virtual DaGalaxy *FindParentGalaxy();

	virtual const DaGalaxy *FindParentGalaxy() const;

	virtual DaWorld* FindParentPlanetInEnv();

	virtual const DaWorld* FindParentPlanetInEnv() const;

	virtual DaSector* FindParentSector();

	virtual const DaSector* FindParentSector() const;

	virtual DaSolarSystem* FindParentSolarSystem();

	virtual const DaSolarSystem* FindParentSolarSystem() const;

	virtual DaSolarSystem* FindParentSolarSystemInEnv();

	virtual const DaSolarSystem* FindParentSolarSystemInEnv() const;

	virtual DaStar* FindParentStarInEnv();

	virtual const DaStar* FindParentStarInEnv() const;

	virtual DaUniverse *FindParentUniverse();

	virtual const DaUniverse *FindParentUniverse() const;

	virtual DaVessel* FindParentVessel();

	virtual const DaVessel* FindParentVessel() const;

	virtual DaVessel* FindParentVesselInEnv();

	virtual const DaVessel* FindParentVesselInEnv() const;

	virtual DaWorldLL* FindParentWorldInEnv();

	virtual const DaWorldLL* FindParentWorldInEnv() const;

	// Find parent world that is not a moon. If my parent world is a moon, finds its parent instead.
	virtual DaWorldLL* FindParentWorldNotSatelliteInEnv();

	// Find parent world that is not a moon. If my parent world is a moon, finds its parent instead.
	virtual const DaWorldLL* FindParentWorldNotSatelliteInEnv() const;

	virtual void FindShipMatesRecursive(const AuId& shipAssigned, DaAtomList& mates) const;

	// Recursively finds all DaTeleporter atoms that are IsNormal.
	virtual void FindTeleportersRecursive(DaAtomList& teleporters) const;

	// Called by stations and cities.
	// Searches recursively for any trespassing cities or stations.
	virtual const DaAtom* FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const;

	DaDesigner* FindDesigner(quint16 studioId) const;

	DaDesignPlane* FindDesignPlane(const AuId& owner) const;

	virtual const DaWorld* FindDestinationWorld(const AuId& id, const QString& name) const;

	DaGalaxy* FindGalaxy(GALAXY galaxy) const;

	virtual void FindPassengersRecursive(const AuId& shipAssigned, DaAtomList& passengers) const;

	const DaPerson* FindPersonOnLine() const;

	virtual const DaPerson* FindPersonOnLineRecursive() const;

	// Finds person atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
	virtual DaPerson* FindPersonRecursive() const;

	// Recursive search for people atoms who are 'normal' and on line or AI controlled. Includes this atom if I am a person.
	virtual void FindPeopleAwakeRecursive(DaAtomList& people);

	// Finds people atoms subordinate to this atom that are 'normal'. Not recursive. Does not include this atom.
	void FindPeopleOnLine(DaAtomList& people);

	// Recursive search for people atoms who are 'normal' and on line. Includes this atom if I am a person.
	virtual void FindPeopleOnLineRecursive(DaAtomList& people) const;

	virtual void FindPeopleOnLineRecursiveInStory(DaAtomList& people, quint32 storyInstance);

	// Finds IsNormal people.
	virtual void FindPeopleRecursive(DaAtomList& people) const;

	// Finds all stations that are commanded by pirates, i.e. returned true to IsSpaceStation() && IsPirateCommander().
	virtual void FindPirateStationsRecursive(DaAtomList& stations) const;

	virtual void FindTroopsRecursive(const AuId& shipAssigned, DaAtomList& troops) const;

	virtual void FindTroopsRecursive(quint32 empireId, DaAtomList& troops);

	virtual void FindVehiclesRecursive(DaAtomList& vehicles) const;

	// Find vehicles assigned to a particular ship.
	virtual void FindVehiclesRecursive(DaAtomList& vehicles, const AuId& motherShipId) const;

	// Non-recursive search of my subatoms for a vessel.
	DaVessel* FindVessel() const;

	virtual void FindVesselsRecursive(DaAtomList& spaceships) const;

	// This is not recursive.
	void FindCitizens(DaAtomList& citizens);

	virtual void FindCitizensRecursive(DaAtomList& citizens) const;

	DaStar* FindStar() const;

	// Searches subatoms for a normal atom with the 'id'. Does not compare the id of 'this' atom. Not recursive.
	DaAtom* FindNormalSubatomId(const AuId& id) const;

	// Searches subatoms for any atom with the 'id'. Does not compare the id of 'this' atom. Not recursive.
	DaAtom* FindSubatomId(const AuId& id) const;

	// Searches recursively for a normal atom with id.
	DaAtom* FindIdRecursive(const AuId &id) const;

	DaAtom* FindLiveIdRecursive(const AuId &id);

	virtual void ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	// List the station ids of consoles, turrets, surgery units, and troop stations that are destinations of AI controlled citizens.
	// Citizens are checked subordinate to this atom, not recursive.
	void ListDestinationConsoles(QSet<quint16>& consoles) const;

	void ListOccupiedSurgeryUnits(QSet<quint16>& consoles) const;

	void ListPlanets(DaAtomList &al) const;	// List planets subordinate to this atom. Not recursive.

	void ListStars(DaAtomList &al) const;	// List stars subordinate to this atom. Not recursive.

	// List the station ids of consoles and turrets occupied by anyone who is not invisible, including AI controlled citizens.
	// 'consoles' key is station id, value is seat index.
	void ListVisiblyOccupiedConsoles(const DaPerson* sceneAtom, QSet<quint16>& consoles) const;

	// Returns true for atoms that persist unconditionally in the scene between scene syncs e.g. sectors, wormholes, etc.
	virtual bool IsPermanentInScene() const
	{
		// Most atoms are not permanent in the scene.
		return false;
	}

	virtual DaAtom *FindClosestAtom(const QPoint &pxFind,
			const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			const AuMatrix4x4 &tModel) const;

	// Called on the scene atom.
	DaAtom* FindControlAtPixel(const QPoint &pxFind,
			const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			qint16& idPanel, AuPoint& fPolyLoc, double& zDepth) const;

	// Called on the scene atom.
	DaAtom* FindAtomAtPixel(const QPoint &pxFind,
			const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			unsigned int& uiPoly, AuPoint &fPolyLoc) const;

	DaBd* FindPlatformLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp, const AuMatrix4x4& tDeviceOrtho,
			AuPoint& platformLoc) const;

	DaWorldLL* FindTerrainLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp, const AuMatrix4x4& tDeviceOrtho,
			AuPoint& terrainLoc) const;

	// Finds the atom that is nearest to this atom in this atom's look direction and in front of this atom's eye location.
	// Returns sighted atom, or 0 if none sighted.
	// When an atom is sighted:
	//   'sightedLoc' set to the 3D location on the atom that was sighted. Relative to this atom.
	//   'sightedPart' set to the atom-specific part that was sighted, as returned by SightTargetDetails.
	//   'sightedShield' is set true if the shield bubble was sighted.
	virtual DaAtom* SightTargetInSight(AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield) const
	{
		return SightTarget(sightedLoc, sightedPart, sightedShield, AuPoint(0,0), AuPoint(0, 1000*FeetPerMeter(), 0));
	}

	// Finds the atom that is nearest to this atom between 'eyeLoc' and 'lookLoc', which are relative to this atom.
	// Returns sighted atom, or 0 if none sighted.
	// When an atom is sighted:
	//   'sightedLoc' set to location on sighted atom between 'eyeLoc' and 'lookLoc'. Relative to this atom.
	//   'sightedPart' set to the atom-specific part that was sighted, as returned by SightTargetDetails.
	//   'sightedShield' is set true if the shield bubble was sighted.
	inline DaAtom* SightTarget(AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, const AuPoint& eyeLoc,
			const AuPoint& lookLoc) const
	{
		// Go all the way to the environment atom, which is usually a solar system but sometimes it's a design environment or
		// other instance universe.
		// In the local scene, this will only include the visible atoms anyway.
		const DaAtom* envAtom = FindEnvironmentAtom();
		if(!envAtom)
			return 0;	// No environment atom?

		// Ask the environment to find the nearest solid atom that is in front of my eye.
		// Use the reverse transformation back up to the environment atom.
		sightedLoc = lookLoc;
		return envAtom->SightTarget(this, eyeLoc, EnvTransform().Inverse(), 0, sightedLoc, sightedPart, sightedShield, 0);
	}

	// 'sightingAtom' is the atom doing the sighting, such as the scene atom or an AI dude. The sightingAtom and all of its
	//   subatoms are ignored when calling this function.
	// 'eyeLoc' is the eye location relative to the sightingAtom.
	// 'tModel' is the model projection to transform geometry into the model space of the sightingAtom.
	// 'pBest' is the best target found so far.
	// 'sightedLoc' is the location on the pBest atom that is sighted, relative to sightingAtom.
	// 'sightedPart' is the sighted part of the target atom, as returned by SightTargetDetails.
	// 'sightedShield' is set true if the shield bubble was sighted.
	// 'sightedSurfaceNormal' is optional, currently only used to determine how to orient to the sighted location below a falling
	//   atom. If atoms don't orient to this atom, 'sightedSurfaceNormal' may be ignored.
	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	// Returns the atom Type() of my parent world, at any depth.
	inline ATOM_TYPE WorldAtomType() const
	{
		ATOM_TYPE v = (ATOM_TYPE)LockEnvironmentQuery()->WorldAtomType();
		UnlockEnvironmentQuery();
		return v;
	}

	// The parent world of this atom was captured by 'byEmpire'.
	virtual void WorldCaptured(quint32 byEmpire);

protected:
	// Called by atoms who want to be affected by solar radiation damage when too close to a sun.
	// Call this with the parent star atom.
	// 'dmgPerSecond' is hit points of damage to inflict per second at the radius of the sun/black hole, increases farther in.
	void ApplySolarRadiation(const DaStar* parentStar, float dmgPerSecond);

private:
	// 'attacker' is the atom that is doing the attacking.
	// 'targetLoc' is the center of the blast, relative to this atom.
	// 'weapon' and 'sub' identify the weapon that is used.
	// 'observeProtection' is true to skip damage to atoms that are IsNoncombatant.
	void AttackArea(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuItem& weaponIt, INVENTORYSUB sub,
			bool observeProtection);

	// 'attacker' is the atom that is doing the attacking.
	// 'defender' is the atom that is under attack. Atom may be null because area attack does not have to be directed at a specific
	//   atom, just the location is essential.
	// 'weapon' describes the weapon that is used.
	// 'observeProtection' is true to skip damage to atoms that are IsNoncombatant.
	// 'result' returns the results of the attack against the specific 'defender' atom, if it is not null.
	void AttackArea(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuCombatWeapon& weapon,
			bool observeProtection, AuCombatResult& result);

protected:
	// 'attacker' is the atom that is doing the attacking.
	// 'defender' is the atom that is under attack.
	// 'weapon' and 'sub' identify the weapon that is used.
	// 'observeProtection' is true to skip damage to atoms that are IsNoncombatant when projecting area effects.
	// 'hitShield' is true if the shield bubble on the defender was hit.
	// Returns true if the target atom is killed. This is a temporal death, not necessarily an atom Kill.
	bool AttackTarget(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuItem& weaponIt, INVENTORYSUB sub,
			bool observeProtection);

public:
	// 'attacker' is the atom that is doing the attacking.
	// 'defender' is the atom that is under attack.
	// 'weapon' describes the weapon that is used.
	// 'observeProtection' is true to skip damage to atoms that are IsNoncombatant when projecting area effects.
	// 'hitShield' is true if the shield bubble on the defender was hit.
	// 'result' returns the results of the attack.
	// Returns true if the target atom is killed. This is a temporal death, not necessarily an atom Kill.
	bool AttackTarget(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuCombatWeapon& weapon,
			bool observeProtection, AuCombatResult& result);

protected:
	DaAtom* FindControlAtPixel(const DaAtom* sceneAtom, const QPoint &pxFind,
			const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			const AuMatrix4x4 &tModel, DaAtom *pBest, double &rBestZDepth,
			qint16& idPanel, AuPoint& fPolyLoc);
	DaAtom* FindAtomAtPixel(const QPoint &pxFind,
			const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			const AuMatrix4x4 &tModel, DaAtom *pBest, double &rBestZDepth,
			unsigned int &uiPoly, AuPoint &fPolyLoc, const DaAtom *pSceneAtom);

	// Formats a message reporting that damage was received from an atom named 'attackerName'.
	static QString FormatDamageReceived(int criticalHits, WEAPONEFFECT effect1, int damage1, int armor1,
			WEAPONEFFECT effect2, int damage2, int armor2, const QString& attackerName);
public:
	// Finds the next atom "up" in scale that can render the environment which contains this atom.
	inline DaAtom* FindEnvironmentAtom() const
	{
		const DaAtom* pAtom = this;
		DaAtom* parent = 0;
		do
		{
			parent = pAtom->Parent();
			if(parent)
			{
				if(parent->CanRenderEnvironment())
					return(parent);
				pAtom = parent;
			}
		}
		while(parent);
		return 0;
	}

	inline bool GetLocationRelativeToParent(AuPoint &fLoc, const DaAtom *pParent) const
	{
		AuMatrix4x4 tModel;
		if(GetTransformRelativeToParent(tModel, pParent))
		{
			// "I" found the parent above me. Return my location in the model
			// projection relative to the parent.
			fLoc = tModel.Project(AuPoint(0,0));
			return true;
		}
		return false;
	}

	// Returns the location of 'atom' relative to this atom.
	// Returns false if 'atom' is not subordinate to the same environment as this atom, recursively.
	bool GetRelativeLocation(const DaAtom& atom, AuPoint& loc) const;

	// Returns the transformation matrix of 'atom' relative to this atom.
	// Returns false if 'atom' is not subordinate to the same environment as this atom, recursively.
	bool GetRelativeTransform(const DaAtom& atom, AuMatrix4x4& t) const;

	virtual bool GetTransformRelativeToParent(AuMatrix4x4 &tModel, const DaAtom *pParent) const;

	inline bool ListPathFromParent(AuIdList &idPath, const DaAtom *pParent) const
	{
		if(Is(pParent->Id()))
		{
			// "I" am the desired parent atom. Return the path as is.
			return true;
		}

		const DaAtom* parent = Parent();
		if(!parent)
			return false;	// Didn't find the real parent.

		// "I" am not the desired parent so prepend my id onto the path list
		// passed and work my way up the chain.
		idPath.insert(idPath.begin(), Id());
		return parent->ListPathFromParent(idPath, pParent);
	}

	bool ListAtomsFromParent(DaAtomList &aPath, const DaAtom *pParent) const;

	// Lists all subatom pointers recursively into one linear list.
	void ListSubatomsRecursive(DaAtomList& atoms) const;

	// Lists IsNormal subatom pointers recursively into one linear list.
	void ListNormalSubatomsRecursive(DaAtomList& atoms) const;

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	virtual bool InStarField() const
	{
		// True if this atom is in the star field.
		return false;
	}

	// Returns false if a known component of the starfield is missing. i.e. a sector isn't loaded yet that we need.
	virtual bool ListStarFieldSubatoms(const DaAtom *pSceneAtom, DaAtomList &sceneAtoms) const;

	// Called on environment atom to recursively cause all sentient atoms in the conversation address cone of 'talker' to start
	// listening on the Voice comm channel.
	virtual void StartVoiceConversationRecursive(const DaPerson& talker) const;

	// Changes the position and/or the parent of this atom. The ether is notified to track parent changes.
	// Does not change this atom's orient rotation.
	void ChangePos(const AuAtomPos& pos);
	bool ChangePosNotParent(const AuAtomPos& pos);	// No change if parent in pos is not my current parent. Returns true on success.
	void ChangePosSync(const AuAtomPos& pos, const AuSyncController& sync);
	bool ChangePosSyncNotParent(const AuAtomPos& pos, const AuSyncController& sync);	// No change if parent changes.

	// Changes the position and/or the parent of this atom. The ether is notified to track parent changes.
	// Changes this atom's orient rotation to match the new transform rotation.
	void ChangePosAndOrient(const AuAtomPos& pos);
	bool ChangePosAndOrientNotParent(const AuAtomPos& pos);
	void ChangePosAndOrientSync(const AuAtomPos& pos);
	bool ChangePosAndOrientSyncNotParent(const AuAtomPos& pos);

	// SetPos should ONLY be used when this atom has not been added to the ether, unless called by the ether.
	// ChangePos should be used after this atom has been added to the ether, to properly track parent changes.
	void SetPos(const AuAtomPos& pos);											// Asserts that m_Ether is 0
	void SetPosByEther(const AuAtomPos& pos, const AuEther* ether);				// Asserts that m_Ether is 'ether'

	// SetPosAndOrient should ONLY be used when this atom has not been added to the ether, unless called by the ether.
	// ChangePosAndOrient should be used after this atom has been added to the ether, to properly track parent changes.
	void SetPosAndOrient(const AuAtomPos& pos);									// Asserts that m_Ether is 0
	void SetPosAndOrientByEther(const AuAtomPos& pos, const AuEther* ether);	// Asserts that m_Ether is 'ether'

	// Interaction with atoms subordinate to this atom.
	// 'elapsedMs' is the time elapsed to move the distance, for collision calculations. 0 ignores collisions.
	bool Move(const AuPoint& vMove, int elapsedMs);

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	// Physics
	// Base returns acceleration due to gravity.
	virtual AuPoint Acceleration() const;

	// The base class implementation causes this atom to float with its CG right at the water line. To float lower than that,
	// return a buoyancy of zero at the appropriate depth.
	// If I float, buoyancy is modulated as my CG gets closer to the water line, so I don't bob in the water.
	// 'qe' is the current environment. It has been locked.
	// Returns 0 for things that neither ascend nor descend in water.
	// Returns positive number for atoms that ascend in water.
	// Returns negative number for atoms that sink in water. Max negative is -1, for 1G downward acceleration.
	// NOTE: This atom IsInWater.
	virtual float Buoyancy(const AuQueryEnvironment& qe) const;

	virtual bool CanBeHarvested(COMMODITYID moduleInUse) const
	{
		Q_UNUSED(moduleInUse);
		return false;
	}

	// True if this atom is permitted to go below the surface of the terrain.
	// This could be any hard surface, such as ground terrain or the floor of
	// a spacecraft or building.
	virtual bool CanBeUnderGround() const
	{
		return false;
	}

	// Atoms such as cities, planets and stars cannot be kicked around.
	virtual bool CanSetInMotion() const
	{
		// New atoms can be set in motion so SetInMotion and SetNormal do not have an order dependency when creating a nu atom.
		return IsNormal() || m_State == New;
	}

	// In square feet. Used by drag calculation.
	// X is lateral
	// Y is forward
	// Z is vertical
	// See http://www.newton.dep.anl.gov/askasci/phy99/phy99x88.htm
	virtual AuPoint CrossSectionalArea() const
	{
		return AuPoint(1,1,1);
	}

	// Return true for atoms that require CreateEnvironmentQuery information.
	// Atoms that can have a movement vector must return true.
	// This base class default returns false;
	virtual bool DoesQueryEnvironment() const
	{
		return false;
	}

	// See usage. Creatures at stations need to return true so they orient to the floor of rooms with natural gravity, instead of
	// to the natural gravity, when natural gravity does not match the room gravity vector.
	virtual bool DoGetRoomUp() const
	{
		return false;
	}

	// Returns atmospheric drag.
	AuPoint Drag() const;

	// Used by drag calculation. Returns 0 < DC <= 1
	// See http://www.newton.dep.anl.gov/askasci/phy99/phy99x88.htm
	virtual float DragCoefficient() const
	{
		// Calculated such that the terminal velocity is 120mph in a standard
		// atmosphere and gravity for an object of unit cross-sectional area.
		// 
		//  D = 1/2 CpAv*v
		//
		//  C = (D * 2) / (pAv*v)
		//
		//  C = drag coefficient of .041322314
		//  D = drag force of 32ft/sec/sec (standard gravity)
		//  p = fluid density of .05 for standard 50% atmosphere density
		//  A = cross-sectional area of 1
		//  v = velocity of 176ft/sec (120mph)
		return (float)(GravityStandard() * 2) / (float)(.05f * 1 * 176 * 176);
	}

	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	virtual float Density() const
	{
		return (float)DensityOfWater();
	}

	// 'vector' is my movement vector per second.
	// 'rsec' is the number of seconds in the movement period.
	virtual void FilterFinalMoveVector(AuPoint& vector) const;

	virtual void FilterPostMoveVector(AuPoint& vector) const;

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const
	{
		return AuPoint(0,0,0);
	}

	// Returns the GALAXY that contains me, or my GALAXY if I am a galaxy.
	virtual GALAXY Galaxy() const;

	// Returns the force of gravity in ft/sec/sec projected by this atom on
	// another atom that is 'cgDistance' feet away.
	virtual double Gravity(double /*cgDistance*/) const
	{
		return 0;
	}

	// Returns a unit vector pointing down with respect to gravity at a specific location.
	// This is the direction of the gravitational force, if this atom exerts any.
	virtual AuPoint GravityDir(const AuPoint& /*loc*/) const
	{
		return AuPoint(0,0,-1);
	}

	// Returns false if this atom is moving. An atom that is not moving will
	// be ignored when applying gravitational forces, etc.
	inline bool IsAtRest() const
	{
		return m_AtRest;
	}

	// Test for movement beyond a very slow speed.
	// Returns true if this atom is moving enough to call it motion.
	// Returns false if this atom is at rest or moving so slowly that it can be considered stopped.
	inline bool IsMoving() const
	{
		if(IsAtRest())
			return false;
		AuPoint v(Vector());
		if(fabs(v.x) > 1)
			return true;
		if(fabs(v.y) > 1)
			return true;
		if(fabs(v.z) > 1)
			return true;
		return false;
	}

	// Returns the mass of this atom, in pounds.
	virtual float Mass() const
	{
		return 0;
	}

	// Momentum is movement vector multiplied by mass.
	inline AuPoint Momentum() const
	{
		return Vector() * Mass();
	}

	// Returns the result of my last attempt to move.
	inline COLLISION_TYPE MoveResult() const
	{
		return m_MoveResult;
	}

	// Orient and OrientationSpeed have no effect unless this function returns true.
	virtual bool OkToOrient() const;

	// If this atom is "at rest", its rotation is changed immediately and true is returned.
	// If this atom is not "at rest", its m_OrientRotation is set and false is returned.
	// Always returns false if parent is a vehicle or an animal.
	virtual bool Orient(const AuPoint& atLoc, const AuPoint& terNormal);

	// Return the number of degrees per second this atom rotates when orienting to terrain.
	virtual double OrientationSpeed() const
	{
		return 90*DEG2RAD;
	}

	virtual void SetAtRest();

	// Applies current controller sync requirements to myself. This is called when the controller stops controlling me, to insure
	// any remaining syncs get applied to myself.
	// Clears my sync controller queue.
	void ApplySync();

	// Discards controller sync.
	void ClearSync();

	// Collects current controller sync requirements. This is the client side, picking up server-applied SyncController values.
	// Changes passed values to reflect current controller sync requirements.
	// Clears my sync controller queue.
	void ControllerSync(AuAtomPos& pos, bool& atRest, AuPoint& vec, float& headTilt, AuPose& pose, quint16& stationId,
			quint8& seatIndex);

	// Adds a 'sync' for some change that must be propagated to the controller of this atom (scene atom or driver).
	// This is the server side, called when some control must be syncronized with the client and not overridden by client-side
	// movement smoothing algorithms.
	void SyncController(const AuSyncController& sync);
	void SyncControllerNoLock(const AuSyncController& sync);	// m_PosLock is expected to be locked already

	void SetInMotion();

	inline void SetMoveResult(COLLISION_TYPE moveResult)
	{
		m_MoveResult = moveResult;
	}

	inline void SetOrientRotation(const AuPoint& rot)
	{
		m_PosLock.lockForWrite();
		m_OrientRotation = rot;
		m_PosLock.unlock();
	}

	// Returns the current movement vector, in feet per second.
	inline AuPoint Vector() const
	{
		m_PosLock.lockForRead();
		AuPoint v(m_Vector);
		m_PosLock.unlock();
		return v;
	}

	inline bool IsMini() const
	{
		return m_MiniScale != 1;
	}

	inline double MiniScale() const
	{
		return m_MiniScale;
	}

	inline void ResetMiniScale()
	{
		m_MiniScale = 1;
	}

	inline void SetMiniScale(double scale)
	{
		m_MiniScale = scale;
	}

	inline void SetVector(const AuPoint& movementVector)
	{
		m_PosLock.lockForWrite();
		m_Vector = movementVector;
		m_PosLock.unlock();
	}

	void SetVectorSync(const AuPoint& movementVector);

	// Atoms that return true here are an obstacle to movement of 'toWho'.
	// 'toWho' atom is !IsEthereal.
	virtual bool IsObstacle(const DaAtom* toWho) const
	{
		Q_UNUSED(toWho);
		return false;
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual double ObstacleRadius() const
	{
		return 0.;
	}

	// Called by spacecraft to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	// A return value of 0 means this atom is not avoided by spacecraft.
	virtual double PilotAvoidRadius() const
	{
		return 0.;
	}

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning non-zero here has an effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		return 0;
	}

	// User interface helpers.
	virtual float GroundHeightAboveCG() const
	{
		// Override this function for objects whose center-of-gravity is above
		// the ground.
		//DHH This must be improved eventually to allow objects lying on
		// their sides.
		return 0;
	}
	virtual double GroundClimbMax() const
	{
		// Override to return maximum amount of ground climb this unit can
		// achieve.
		return 0.0;
	}

	inline bool EnvironmentQueryValid() const
	{
		//m_EnvironLock.lockForRead();
		bool v = m_Environ ? true : false;
		//m_EnvironLock.unlock();
		return v;
	}

	// Calculates lighting in environment query data.
	void QueryEnvironmentLighting() const;

	// Calculate environment vector in environment query data when movement relative to the environment is needed.
	// Not called on the client.
	// Movement vector of atoms is not sent to the client. Therefore, the environment movement vector calc can only be performed
	// on the server. DaPerson::TxUpdate only sends the environment movement vector when needed by the client.
	void QueryEnvironmentVector();

	// Returns the id of the DaBd site that contains this atom.
	// If I am on a geo road, the id of the world is returned.
	inline AuId BdId() const
	{
		AuId v(LockEnvironmentQuery()->m_BdId);
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns the id of the DaBd environment enclosure that contains this atom, if any. e.g. platform or biodome.
	// IsInsideEnclosure simply tests this id for null.
	inline AuId EnclosureId() const
	{
		AuId v(LockEnvironmentQuery()->EnclosureId());
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns the CapitolId of the DaBd site that contains this atom.
	// The DaBd city id is the CapitolId of the DaBd site where I am at. It will be 0 at a DaBd that is not part of a city.
	// If I am on a geo road, the id of the DaBd city whose jurisdiction contains me is returned.
	inline AuId InBdCity() const
	{
		AuId v(LockEnvironmentQuery()->m_BdCityId);
		UnlockEnvironmentQuery();
		return v;
	}

	inline quint32 InCityEmpire() const
	{
		Q_ASSERT(!BdId().IsNull());	// Must be in a city or at a building. At a geo road, empire 0 is returned.
		quint32 v = LockEnvironmentQuery()->m_CityEmpire;
		UnlockEnvironmentQuery();
		return v;
	}

	inline ATMTYPE InAtmosphereType() const
	{
		ATMTYPE v = LockEnvironmentQuery()->AtmosphereType();
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns true if this unit is within "atmosphere range" of a planetary body, whether or not that
	// body actually has an atmosphere. 'm_fAtmDensity' is your indicator of atmosphere presense.
	inline bool IsInAtmosphere() const
	{
		bool v = (LockEnvironmentQuery()->m_Environment & EB_InAtmosphere) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// True if FindParentPlanet returned something.
	inline bool IsNearPlanet() const
	{
		bool v = LockEnvironmentQuery()->OrbitZone() != OZONE_DeepSpace;
		UnlockEnvironmentQuery();
		return v;
	}

	// NOTE: This will return -1 if the atom is not near a planet. i.e. it
	//       could be subordinate to a star or solar system but far from any
	//       planet.
	inline ORBITZONE OrbitZone() const
	{
		ORBITZONE v = LockEnvironmentQuery()->OrbitZone();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool PlanetHasAtmosphere() const
	{
		bool v = (LockEnvironmentQuery()->m_Environment & EB_Atmosphere) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	virtual bool IsInVacuum() const
	{
		if(IsAirBreathable())
			return false;	// On a world or in spacecraft or building.
		if(IsInAtmosphere() && AtmosphereDensity() > 0)
			return false;	// In atmosphere of a world.
		return true;
	}

	inline double FallDistance() const
	{
		double v = LockEnvironmentQuery()->m_FallDist;
		UnlockEnvironmentQuery();
		return v;
	}

	// Depth of the eye below the water surface.
	// 0 if eye is above the water surface
	inline float SeaDepth() const
	{
		float v = LockEnvironmentQuery()->m_fSeaDepth;
		UnlockEnvironmentQuery();
		return v;
	}

	inline AuPoint GravityNormal() const
	{
		AuPoint v(LockEnvironmentQuery()->GravityNormal());
		UnlockEnvironmentQuery();
		return v;
	}

	void SetGravityNormal(const AuPoint& gN);

	void SetTerrainNormal(const AuPoint& tN);

	inline AuPoint TerrainNormal() const
	{
		AuPoint v = LockEnvironmentQuery()->m_terNormal;
		UnlockEnvironmentQuery();
		return v;
	}

	// The vector returned is this atom's movement vector with respect to its environment.
	inline AuPoint EnvironmentVector() const
	{
		m_EnvironLock.lockForRead();
		AuPoint v(m_EnvVector);
		m_EnvironLock.unlock();
		return v;
	}

	// This is the result of calling DaWorld::QuerySunlight at my location on my parent world.
	// m_Illumination is 0 or more when this atom is inside atmosphere range of a planet, whether or not the planet has any
	// atmosphere density.
	// CAVEAT: QueryEnvironmentLighting must be called before calling this function.
	inline float Illumination() const
	{
		m_EnvironLock.lockForRead();
		float v = m_Illumination;
		m_EnvironLock.unlock();
		return v;
	}

	// CAVEAT: QueryEnvironmentLighting must be called before calling this function.
	inline bool IsDark() const
	{
		return IsDark(Illumination());
	}

	inline static bool IsDark(float illumination)
	{
		return illumination < .3f;
	}

	// Returns air density from immediate environment.
	// Should be the same as AtmosphereDensity when not in a vehicle or spacecraft.
	inline float AirDensity() const
	{
		float v = LockEnvironmentQuery()->m_fAirDensity;	// 0..1
		UnlockEnvironmentQuery();
		return v;
	}

	// Distance from CG to ceiling.
	inline float CeilingDistance() const
	{
		float v = LockEnvironmentQuery()->m_fCeilingDist;
		UnlockEnvironmentQuery();
		return v;
	}

	inline double CGAltitude() const
	{
		double v = LockEnvironmentQuery()->m_fCGAltitude;
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns atmosphere density of planet adjusted for altitude. 0..1
	inline float AtmosphereDensity() const
	{
		float v = LockEnvironmentQuery()->m_fAtmDensity;	// 0..1
		UnlockEnvironmentQuery();
		return v;
	}

	inline QColor ColorOfAtmosphere() const
	{
		QColor v = LockEnvironmentQuery()->m_colAtmosphere;
		UnlockEnvironmentQuery();
		return v;
	}

	inline AuGlobeLLF GlobeLL() const
	{
		AuGlobeLLF v = LockEnvironmentQuery()->m_GlobeLL;
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns true if this unit is inside the atmosphere of a planet with a
	// breathable atmosphere, in a spacecraft with functioning life support, or
	// in a building or pressurized road with functioning life support.
	inline bool IsAirBreathable() const
	{
		bool v = LockEnvironmentQuery()->IsAirBreathable();
		UnlockEnvironmentQuery();
		return v;
	}

	// Specifically on the ground in a forest, not in the air.
	inline bool IsOnForest() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Forest ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// Fairly loose test for being on the ground or against something run into.
	// Use IsTouchingGround() to be more precise.
	// false means not on any ground surface
	inline bool IsOnGround() const
	{
		if(MoveResultBlocked())
			return true;
		// false means not touching any ground surface
		bool v = LockEnvironmentQuery()->IsOnGround();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnHardFloor() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_HardFloor ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnMetal() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Metal ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnPavement() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Pavement ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns true if this atom is in the area of a road, could be under construction. Does not imply ground contact.
	inline bool IsOnRoad() const
	{
		bool v = (LockEnvironmentQuery()->m_Environment & EB_Road) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnRock() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Rock ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnSand() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Sand ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnSavanna() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Savanna ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsOnSnow() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_Snow ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// True means I hit my parent or a peer atom while moving and I am not on any ground surface.
	inline bool MoveResultBlocked() const
	{
		return m_MoveResult == CT_HitParent || m_MoveResult == CT_HitPeer;
	}

	inline bool IsOnWoodFloor() const
	{
		bool v = LockEnvironmentQuery()->m_OnGround == OG_WoodFloor ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// Precise test for touching the ground.
	// Use IsOnGround() or the IsOnXXX functions for a looser test for being near the ground.
	inline bool IsTouchingGround() const
	{
		if(MoveResultBlocked())
			return true;
		if(!IsOnGround())
			return false;			// Not even close to the ground.
		// 1/2" from the ground is close enough to consider it "touching"
		return (FallDistance() < .083/2.) ? true : false;
	}

	inline ON_GROUND OnGround() const
	{
		// NOTE: Never returns OG_WaterSoundOnly.
		ON_GROUND v = LockEnvironmentQuery()->m_OnGround;
		UnlockEnvironmentQuery();
		return v;
	}

	inline float AltitudeAGL() const
	{
		float v = LockEnvironmentQuery()->m_fAltitudeAGL;
		UnlockEnvironmentQuery();
		return v;
	}

	// Altitude of my CG location.
	inline float AltitudeMSL() const
	{
		float v = LockEnvironmentQuery()->m_fAltitudeMSL;
		UnlockEnvironmentQuery();
		return v;
	}

	// In an arena instance. An arena will return true only if it is subordinate to an arena at some level.
	bool InArena() const;

	// In an arena instance as an observer.
	bool InArenaAsObserver() const;

	// In an arena instance when/where engines cannot be used.
	bool InArenaCannotAccelerate() const;

	// In an arena instance when/where weapons cannot be used.
	bool InArenaCannotFireWeapons() const;

	// In an arena instance but outside the outermost bounds allowed.
	inline bool InArenaOutsideBounds() const
	{
		if(!InArena())
			return false;
		return EnvLocation().Length2() > c_ArenaBoundsRadius*c_ArenaBoundsRadius;
	}

	// In an arena instance but outside the field.
	inline bool InArenaOutsideRing() const
	{
		if(!InArena())
			return false;
		return EnvLocation().Length2() > c_ArenaRingRadius*c_ArenaRingRadius;
	}

	// At the sidelines in an arena instance.
	inline bool InArenaSidelines() const
	{
		if(!InArena())
			return false;
		double elen2 = EnvLocation().Length2();
		return elen2 > c_ArenaRingRadius*c_ArenaRingRadius &&	// Outside the combat sphere
			elen2 < c_ArenaBoundsRadius*c_ArenaBoundsRadius;	// Inside the arena structure sphere.
	}

	// True if this atom is inside the Schwarzschild radius of a black hole.
	inline bool InBlackHole() const
	{
		bool v = LockEnvironmentQuery()->m_Environment & EB_EventHorizon;
		UnlockEnvironmentQuery();
		return v;
	}

	// True if this atom is in the gravitational influence a black hole or neutron star.
	inline bool InBlackHoleGravity() const
	{
		bool v = LockEnvironmentQuery()->m_Environment & EB_BlackHoleGravity;
		UnlockEnvironmentQuery();
		return v;
	}

	inline INDUSTRYID InIndustry() const
	{
		INDUSTRYID v = LockEnvironmentQuery()->m_InIndustry;
		UnlockEnvironmentQuery();
		return v;
	}

	// Only valid if InIndustry is not IID_None.
	// Only valid on globe world.
	inline AuGlobeLLF InIndustryLL() const
	{
		AuGlobeLLF v = LockEnvironmentQuery()->m_InIndustryLL;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool InDesignEnvironment() const
	{
		bool v = LockEnvironmentQuery()->InDesignEnvironment();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool InDesignPreview() const
	{
		bool v = LockEnvironmentQuery()->InDesignPreview();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool InDesignerNotPreview() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		bool v = qe->InDesignEnvironment() && !qe->InDesignPreview();
		UnlockEnvironmentQuery();
		return v;
	}

	// This check includes above/below hatch checks for old style ship and building designs.
	inline bool InLadderWell() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		bool v = qe->InLadderWell();
		UnlockEnvironmentQuery();
		return v;
	}

	// True if this atom is inside the photon sphere of a black hole.
	inline bool InPhotonSphere() const
	{
		bool v = LockEnvironmentQuery()->m_Environment & EB_PhotonSphere;
		UnlockEnvironmentQuery();
		return v;
	}

	// True if this atom is in pathable space. Could be aboard a ship or at a building site, potentially on the roof or grounds
	// within TerrainLevelRadius of a building at ground level. True result means AI can use AuPath to find its way around.
	inline bool InPathableSpace() const
	{
		bool v = LockEnvironmentQuery()->m_Environment & EB_Pathable;
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns true if this unit is inside the transporter room of a capsule.
	bool InTransporterRoom() const;

	// Returns true if I am inside an environment void of a biodome or platform.
	inline bool IsInsideEnclosure() const
	{
		bool v = LockEnvironmentQuery()->IsInsideEnclosure();
		UnlockEnvironmentQuery();
		return v;
	}

	// true if parent atom is a vehicle, regardless of whether it is enclosed
	// or not. See IsInVehicle and IsOnVehicle to be that specific.
	inline bool IsAboardVehicle() const
	{
		bool v = LockEnvironmentQuery()->IsAboardVehicle();
		UnlockEnvironmentQuery();
		return v;
	}

	// true if inside an enclosed vehicle like suv or space fighter.
	// false in open vehicles like dirt bike and sail boat.
	inline bool IsInVehicle() const
	{
		bool v = LockEnvironmentQuery()->IsInVehicle();
		UnlockEnvironmentQuery();
		return v;
	}

	// true if on an open vehicle like dirt bike or sail boat.
	// false inside enclosed vehicles like suv and space fighter.
	inline bool IsOnVehicle() const
	{
		bool v = (LockEnvironmentQuery()->m_Environment & EB_OnVehicle) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// True if the insertion point of this unit is on the water surface.
	inline bool IsOnWater() const
	{
		bool v = LockEnvironmentQuery()->IsOnWater();
		UnlockEnvironmentQuery();
		return v;
	}

	// cg underwater and wet, i.e. not in a vehicle or spacecraft
	inline bool IsCGSubmerged() const
	{
		bool v = LockEnvironmentQuery()->IsCGSubmerged();
		UnlockEnvironmentQuery();
		return v;
	}

	// mouth underwater and wet, i.e. not in a vehicle or spacecraft
	inline bool IsMouthSubmerged() const
	{
		const unsigned int crit = EB_MouthUnderwater|EB_InWater;
		// This is an AND query. All of the bits must be set.
		bool v = ((LockEnvironmentQuery()->m_Environment & crit) == crit) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// eye underwater and wet, i.e. not in a vehicle or spacecraft
	inline bool IsEyeSubmerged() const
	{
		const unsigned int crit = EB_EyeUnderwater|EB_InWater;
		// This is an AND query. All of the bits must be set.
		bool v = ((LockEnvironmentQuery()->m_Environment & crit) == crit) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsUnderGround() const
	{
		return AltitudeAGL() < 0;
	}

	// true even if inside a spacecraft or vehicle
	inline bool IsEyeUnderWater() const
	{
		bool v = LockEnvironmentQuery()->IsEyeUnderWater();
		UnlockEnvironmentQuery();
		return v;
	}

	// true even if inside a spacecraft or vehicle
	inline bool IsCGUnderWater() const
	{
		bool v = (LockEnvironmentQuery()->m_Environment & EB_CGUnderwater) ? true : false;
		UnlockEnvironmentQuery();
		return v;
	}

	// true when there is a gravitational force affecting this atom
	inline bool IsInGravity() const
	{
		bool v = LockEnvironmentQuery()->IsInGravity();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsInGravityWell() const
	{
		bool v = LockEnvironmentQuery()->IsInGravityWell();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsInsideBd() const
	{
		bool v = LockEnvironmentQuery()->IsInsideBd();
		UnlockEnvironmentQuery();
		return v;
	}

	// 0 indicates this atom is not in a room of a Bd building.
	// N indicates the AuScPart room id in which this atom is standing.
	inline quint16 BdRoomId() const
	{
		quint16 v = LockEnvironmentQuery()->BdRoomId();
		UnlockEnvironmentQuery();
		return v;
	}

	// true if at the site of a DaBd, may not be subordinate to it.
	inline bool IsAtBd() const
	{
		bool v = LockEnvironmentQuery()->IsAtBd();
		UnlockEnvironmentQuery();
		return v;
	}

	// true if subordinate to a DaBd at any child level
	inline bool IsInBd() const
	{
		bool v = LockEnvironmentQuery()->IsInBd();
		UnlockEnvironmentQuery();
		return v;
	}

	void CapsuleLocation(quint16& roomId, ROOMTYPE& roomType, QString& roomName) const;

	inline quint16 CapsuleRoomId() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		quint16 v;
		if(qe->IsInCapsule())
			v = qe->m_CapRoomId;
		else
			v = 0;
		UnlockEnvironmentQuery();
		return v;
	}

	virtual QString CapsuleRoomName() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		QString v;
		if(qe->IsInCapsule())
			v = qe->m_CapRoomName;
		UnlockEnvironmentQuery();
		return v;
	}

	inline ROOMTYPE CapsuleRoomType() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		ROOMTYPE v;
		if(qe->IsInCapsule())
			v = qe->m_CapRoomType;
		else
			v = ROOM_None;
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsInCapsule() const
	{
		// true if subordinate to a DaBd or DaVessel at any child level
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		bool v = qe->IsInCapsule();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsInsideCapsule() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		bool v = qe->IsInsideCapsule();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsAtGeo() const
	{
		// true if at the site of a DaBd, may not be subordinate to it.
		bool v = LockEnvironmentQuery()->IsAtGeo();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsInsideSpacecraft() const
	{
		bool v = LockEnvironmentQuery()->IsInsideSpacecraft();
		UnlockEnvironmentQuery();
		return v;
	}

	inline bool IsInSpacecraft() const
	{
		// true if subordinate to a spacecraft at any child level
		bool v = LockEnvironmentQuery()->IsInSpacecraft();
		UnlockEnvironmentQuery();
		return v;
	}

	// True if this unit is in at least 2" of water whether or not this unit is under water.
	// Never true if this unit is inside a vehicle or spacecraft.
	// The minimal water depth helps things to be considered in the water closer to where the player sees the water.
	// Due to tessellation of the water surface, the water effect is not visible where it is extremely shallow, particularly
	// between the vertices of the water surface model.
	// This is essentially a fudge factor to err on the side of safety for the player, so they don't get fried in lava when they
	// don't think they are standing in lava. I have seen lava pools so shallow you never see the surface effect so you can't tell
	// they are there at all.
	bool IsInVisibleWater() const;

	// True if this unit is in water whether or not it is under water.
	// This could be water, lava, or supercooled liquid.
	// Never true if this unit is inside a vehicle or spacecraft.
	inline bool IsInWater() const
	{
		bool v = LockEnvironmentQuery()->IsInWater();
		UnlockEnvironmentQuery();
		return v;
	}

	// True if this unit IsInWater that is lava.
	bool IsInLava() const;

	// True if this unit IsInWater that is supercooled liquid.
	bool IsInSupercooledLiquid() const;

	bool OnRingworld() const;

	inline bool OnStairs() const
	{
		bool v = LockEnvironmentQuery()->OnStairs();
		UnlockEnvironmentQuery();
		return v;
	}

	inline float WaterDepth() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		float v = qe->IsInWater() ? qe->m_WaterDepth : 0;
		UnlockEnvironmentQuery();
		return v;
	}

	// 'oldloc' and 'nuloc' are relative to my parent atom.
	// 'hit', 'hitLoc' and 'hitNormal' are output parameters set if I hit another atom.
	// 'hitLoc' and 'hitNormal' are in the space of the atom that was hit.
	COLLISION_TYPE CheckCollision(const AuPoint& oldloc, const AuPoint& nuloc, DaAtom*& hit, AuPoint& hitLoc, AuPoint& hitNormal);

protected:
	// 'foldloc' and 'fnuloc' are relative to pParent.
	// 'fMoveExt' is relative to pParent. It is the rough extents box of the moving atom.
	// 'pHit', 'hitLoc' and 'hitNormal' are output parameters set if I hit another atom.
	virtual COLLISION_TYPE CheckCollisionRelativeToAtom(DaAtom* pParent, const AuPoint &foldloc, const AuPoint &fnuloc,
			DaAtom*& pHit, AuPoint& hitLoc, AuPoint& hitNormal, const AuBox& fMoveExt);

public:
	// Override to decide when a subatom has departed this atom. Used by AuStory when placing nu characters, so a building's site
	// is not considered a departure from the building.
	virtual bool CheckChildDeparture(const AuPoint& oldLoc, const AuPoint& nuLoc, const DaAtom* who) const
	{
		return CheckChildExit(oldLoc, nuLoc, who);
	}

	// Override to decide when to release control of a subatom that is moving.
	virtual bool CheckChildExit(const AuPoint& /*oldLoc*/, const AuPoint& /*nuLoc*/, const DaAtom* /*who*/) const
	{
		return false;
	}

	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);

	virtual bool RoughExtents(AuBox &fExt) const;

	// Returns extents of shields if they are active/present and able to block weapons.
	// Otherwise returns RoughExtents.
	virtual bool RoughExtentsWithShields(AuBox &fExt) const;

	// Specifically used for movement path testing.
	// Margins are applied where needed to prevent eye camera penetration into objects.
	// Not precise enough for gun sight testing. SightTarget functions exist for that.
	// 'hitLoc' is an output parameter. It is the location on the surface that is hit.
	// 'hitNormal' is an output parameter. It is the normal to the surface that is hit.
	// 'toWho' is the atom attempting to move. Spacecraft automatically open doors when appropriate.
	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho)
	{
		Q_UNUSED(oloc);
		Q_UNUSED(nloc);
		Q_UNUSED(hitLoc);
		Q_UNUSED(hitNormal);
		Q_UNUSED(toWho);
		return false;
	}

	// Only valid when IsInSpacecraft is true.
	inline quint16 ShipRoomId() const
	{
		quint16 v = LockEnvironmentQuery()->VesselRoomId();
		UnlockEnvironmentQuery();
		return v;
	}

	// Only valid when IsInBd is true.
	// Only needed by ScDesign functions. They group multiple room voids by name, so they can be treated as one.
	inline QString BdRoomName() const
	{
		QString v = LockEnvironmentQuery()->BdRoomName();
		UnlockEnvironmentQuery();
		return v;
	}

	// Only valid when IsInSpacecraft is true.
	// Only needed by ScDesign functions. They group multiple room voids by name, so they can be treated as one.
	inline QString ShipRoomName() const
	{
		QString v = LockEnvironmentQuery()->VesselRoomName();
		UnlockEnvironmentQuery();
		return v;
	}

	inline ROOMTYPE ShipRoomType() const
	{
		ROOMTYPE v = LockEnvironmentQuery()->VesselRoomType();
		UnlockEnvironmentQuery();
		return v;
	}

	inline quint16 TransporterSystemWhich() const
	{
		const AuQueryEnvironment* qe = LockEnvironmentQuery();
		quint16 v = qe->VesselRoomId();
		UnlockEnvironmentQuery();
		return v;
	}

	// Returns the which id of the transporter system I am nearest, or operating.
	// On new AuDesign designs, this is the closest transporter station system id, if InTransporterRoom is true.
	virtual quint16 AtTransporterWhich() const;

public:
	// Lists ids of all IsNormal subatoms. Not recursive.
	void ListNormalSubatomIds(AuIdList& ids) const;

	void ListNormalSubatoms(DaAtomList& atoms) const;

	// Lists ids of all subatoms. Not recursive.
	void ListSubatomIds(AuIdList& ids) const;

	// Lists subatom pointers. Not recursive.
	void ListSubatoms(DaAtomList& atoms) const;

	// Returns the number of subatoms that are not IsDead. Not recursive.
	int SubatomCount() const;

	// 'attacker' is the atom that is causing the damage. The attacker id should be NULL when the attacker is not a DaUnit, like
	//   when it is the environment or a sun or something.
	// 'weapon' describes the weapon that was used to inflict the damage.
	// 'wpnLoc' is the location of the weapon that is attacking this atom, relative to this atom.
	// 'hitLoc' is the location of the hit on this atom, relative to this atom.
	// 'result' contains the results of the attack, except the IsDestroyed state; this function call sets that.
	// Sets the IsDestroyed result in 'result', if this atom is killed. This is temporal death, not necessarily atomic death.
	// IsDestroyed result is not modified if this atom is already dead.
	// The only thing this base class function does is sends a log message to 'whoId', if it's not NULL. DISABLED.
	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	// Queues a call to CombatApplyDamage to occur during Tick.
	void CombatApplyDamageLater(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, const AuCombatResult& result);

	// Query to determine this atom's armor absorption, based on weapon 'mode', 'effect' and hit direction.
	// 'mode' is the mode of delivery of the weapon effect.
	// 'effect' is the weapon effect, may be WE_Generic for generic assessment. Those types are never actually
	//   damage types inflicted on an atom, just categories of weapon effect.
	// 'hitShield' is true if the shield bubble was hit.
	// 'wpnLoc' is the location of the weapon that is attacking this atom, relative to this atom.
	// 'hitLoc' is the location of the hit on this atom, relative to this atom.
	// 'baseAbsorb' and 'baseBlock' are output parameters, set to the armor absorption potential vs 'mode' 'effect' attributed to
	//   the base object, like physical body or ship hull.
	// 'auxAbsorb' and 'auxBlock' are output parameters, set to the armor absorption potential vs 'mode' 'effect' attributed to
	//   auxiliary armor, like body armor and energy shields.
	// 'baseAbsorb' and 'auxAbsorb' are percentages targetting the range 0-1 but no clamping is performed.
	// 'baseBlock' and 'auxBlock' are hit points in the range 0-n.
	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
	{
		Q_UNUSED(mode);
		Q_UNUSED(effect);
		Q_UNUSED(hitShield);
		Q_UNUSED(wpnLoc);
		Q_UNUSED(hitLoc);
		baseAbsorb = 0;
		baseBlock = 0;
		auxAbsorb = 0;
		auxBlock = 0;
	}

	// User interface functions.
	virtual bool IsControl(const DaAtom* /*sceneAtom*/) const
	{
		// Returns true if this atom is a control for Enum/ProcessControlInputs.
		return false;
	}
	virtual void EnumControlInputs(DiControlInputArray &) const
	{
		// parameters: controls array
		// A scene atom should fill this array with control inputs it can accept.
	}

	// parameters: controls array, model matrix, panel id array, isInWater
	// 'isInWater' and 'cgHeight' are used by worlds to determine if water can be foraged. 'cgHeight' is the height above the
	// ground of the CG of the scene atom when this function is called, i.e. GroundHeightAboveCG().
	// Fill the array with control panel quads for determining hit locations.
	virtual void EnumControlPanels(AuQuadArray& controlPanels, const AuMatrix4x4& model, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const
	{
		Q_UNUSED(controlPanels);
		Q_UNUSED(model);
		Q_UNUSED(idPanels);
		Q_UNUSED(sceneAtom);
	}

	// Fills 'controls' with controls that could be activated on 'panelId'. 'panelLoc' is the panel polygon location returned
	// by FindControlAt.
	// Used to construct a UI of possible inputs when the scene atom looks at a panel. e.g. open door, lock door, forage seed.
	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
	{
		Q_UNUSED(panelId);
		Q_UNUSED(controls);
		Q_UNUSED(ari);
	}

	// 'gearPos' refers to the atom's inventory. 'chSelValue' is a count associated with that.
	// 'msElapsed' is used when 'cc' is client or solo, to rotate the atom more promptly.
	virtual void ProcessControlInput(const DiControlInputArray& /*aActiveControls*/, DaPerson* /*pSceneAtom*/,
			const AuGearPos& /*gearPos*/, qint8 /*chSelValue*/, CONTROL_CONTEXT /*cc*/, int /*msElapsed*/)
	{
	}

	// parameters: panel id, panel location, active controls array, continuous feedback test
	// This is how mouse control inputs activate controls in the control input array.
	// Controls in 'controls' are activated accordingly.
	// Some panels don't need to be selected and can simply react to the input, such as a light switch.
	// 'bContinuous' is true when the caller is testing for controls that provide continuous feedback while the button is depressed.
	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint& panelLoc, DiControlInputArray& controls, bool bContinuous)
	{
		Q_UNUSED(idPanel);
		Q_UNUSED(panelLoc);
		Q_UNUSED(controls);
		Q_UNUSED(bContinuous);
	}

public:
	virtual bool CanBeSelected() const;	// Default returns true if the atom has non-zero rough extents.

	// Return true if this atom can be sighted when targeted with a weapon or tool by 'toWho'.
	// If false is returned, this atom will not act as an obstacle to gunfire from 'toWho'.
	// 'toWho' may be null.
	virtual bool IsSightTarget(const DaAtom* toWho) const
	{
		Q_UNUSED(toWho);
		return false;
	}

	void EnumSelectionSurfaces(AuQuadArray &aSelectionSurfaces, const AuMatrix4x4& model) const;

	// Rendering functions.
	virtual bool CanRenderEnvironment() const
	{
		return false;
	}

	// Instance universe's prevent some top-down recursive searches from entering the instance.
	virtual bool IsInstanceUniverse() const
	{
		return false;
	}

	virtual AuPoint SceneLookAtPoint(const DaAtom *pLookAt) const
	{
		if (pLookAt)
		{
			AuPoint flook;
			pLookAt->GetLocationRelativeToParent(flook, this);
			return flook;
		}
		return AuPoint(0,0);
	}

	// Returns the location of the view camera, relative to this atom. Introduced to determine IsInScene distances from the
	// scene atom, which could be viewing the world through a remote turret camera.
	virtual AuPoint SceneCameraLocation() const
	{
		return AuPoint(0,0);
	}

	// Adjust camera location for a subatom.
	// 'subatom' is one of my subatoms.
	// 'camera' is the matrix to the camera relative to the subatom.
	// Returns true if 'camera' is modified.
	virtual bool ConstrainCamera(const DaAtom* /*subatom*/, AuMatrix4x4& /*camera*/) const
	{
		return false;
	}

	// Returns 0 to 1.
	virtual float RelativeAudioGain(const DaPerson* sceneAtom) const;

	// Renders the environment for a subatom of the environment who is rendering a scene.
	// 'ari' is the atom render information
	// 'model' is the model view matrix.
	// 'sceneAtom' is the person atom who is the scene atom.
	virtual void RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, DaPerson* sceneAtom)
	{
		Q_UNUSED(projection);
		Q_UNUSED(farProjection);
		Q_UNUSED(model);
		Q_UNUSED(ari);
		Q_UNUSED(sceneAtom);
	}

	// Calculates model view matrices of each of my subatoms. 'model' is my model view matrix. Recursive.
	void CollectModelMatrices(const AuMatrix4x4& model, AuModelMatrixHash& rmod);

	// Renders this object in an environment.
	// Places this atom's pointer in the 'renderLast' map if RenderLast needs to be called after everyone else is done rendering.
	// This is needed for visual effects and transparency rendering sort order.
	// Objects are rendered in two passes. The far pass is rendered first, with a large near clip distance. Only large astronomical
	// bodies are rendered in this pass. After the far pass, the Z buffer is cleared and all remaining objects are rendered.
	// The 'renderLast' objects are rendered after their respective far and near passes.
	// 'projection' is the view projection matrix.
	// 'ari' is the atom render information.
	// 'rmod' contains the current model view matrices of all atoms in the scene, gathered using CollectModelMatrices.
	// 'sceneAtom' is the person atom who is the scene atom.
	// 'renderLast' is the map of atoms that must be rendered last. This atom adds itself to the renderLast map if needed.
	// 'pass' specifies which rendering pass we are painting, either PASS_Far or PASS_Near.
	// 'shadSc' is filled with atoms who can be rendered using the Sc shader.
	// 'shadScFog' is filled with atoms who can be rendered using the ScFog shader.
	// 'shadScGlobe' is filled with atoms who can be rendered using the ScGlobe shader.
	// 'shadScGlobeFog' is filled with atoms who can be rendered using the ScGlobeFog shader.
	// 'shadScRingworld' is filled with atoms who can be rendered using the ScRingworld shader.
	// 'shadScRingworldFog' is filled with atoms who can be rendered using the ScRingworldFog shader.
	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	// Second pass rendering function for things that need to be drawn again in a given RENDERPASS, such as visual effects.
	// Not recursive.
	// 'projection' is the view projection matrix.
	// 'ari' is the atom render information
	// 'rmod' contains the current model view matrices of all atoms in the scene, gathered using CollectModelMatrices.
	// 'sceneAtom' is the person atom who is the scene atom.
	// 'shadSc' is filled with atoms who can be rendered using the Sc shader.
	// 'shadScFog' is filled with atoms who can be rendered using the ScFog shader.
	// 'shadScGlobe' is filled with atoms who can be rendered using the ScGlobe shader.
	// 'shadScGlobeFog' is filled with atoms who can be rendered using the ScGlobeFog shader.
	// 'shadScRingworld' is filled with atoms who can be rendered using the ScRingworld shader.
	// 'shadScRingworldFog' is filled with atoms who can be rendered using the ScRingworldFog shader.
	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	// Atoms who add themselves to 'shadSc' in RenderEnvironmentObject must override this.
	virtual void RenderSc(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
	{
		Q_UNUSED(ari);
		Q_UNUSED(rmod);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pass);
	}

	// Atoms who add themselves to 'shadScFog' in RenderEnvironmentObject must override this.
	virtual void RenderScFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
	{
		Q_UNUSED(ari);
		Q_UNUSED(rmod);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pass);
	}

	// Atoms who add themselves to 'shadScGlobe' in RenderEnvironmentObject must override this.
	virtual void RenderScGlobe(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
	{
		Q_UNUSED(ari);
		Q_UNUSED(rmod);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pass);
	}

	// Atoms who add themselves to 'shadScGlobeFog' in RenderEnvironmentObject must override this.
	virtual void RenderScGlobeFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
	{
		Q_UNUSED(ari);
		Q_UNUSED(rmod);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pass);
	}

	// Atoms who add themselves to 'shadScRingworld' in RenderEnvironmentObject must override this.
	virtual void RenderScRingworld(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
	{
		Q_UNUSED(ari);
		Q_UNUSED(rmod);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pass);
	}

	// Atoms who add themselves to 'shadScRingworldFog' in RenderEnvironmentObject must override this.
	virtual void RenderScRingworldFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom,
			RENDERPASS pass)
	{
		Q_UNUSED(ari);
		Q_UNUSED(rmod);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pass);
	}

	// Collects lights from all scene atoms with a light that is on.
	// 'person' is the person for whom the scene is rendered.
	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);
	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
	// 'scannerLoc' is the location of the atom who is performing the scan, relative to the solar sytem.
	// 'scannerId' is the id of the atom who is performing the scan. Currently either a ship or a city.
	// 'scannerEmpire' is the empire id of the atom who is performing the scan.
	// 'scannerSensorRange' is the range in feet of the scanner atom's sensors.
	// 'scannerWeaponRange' is the range in feet of the scanner atom's bay/base weapons.
	// 'scanContent' is a bitmask of data that is sought in the scan
	//   b0 = sensor console blips
	//   b1 = fire control console blips
	//   b2 = transporter console blips
	//   b3 = locked-on blip
	//   b4 = AI pilot obstacle blips
	//   b5 = AI turret gunner target blips
	//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
	// 'lockedBlip' is the id of the currently locked sensor blip
	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld,
			float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	// Returns the sensor range in feet, adjusted for damage.
	virtual float SensorRange() const
	{
		return 0;
	}

public:
	virtual void Insert(QSqlDatabase& /*db*/) const	// Insert first time record.
	{
	}
	virtual void StoredInDb()					// Called when this atom has been inserted or updated in the database.
	{
	}
	virtual void Update(QSqlDatabase& /*db*/) const	// Update existing record.
	{
	}

protected:
	void RenderSc(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom,
			DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog, RENDERPASS pass);

	// Probably should only be used when reestablishing the state of this variable when loading from a database or stream.
	void SetAtRestState(bool atRest);

	void SetRenderLights(AuGLEffects& re, const QList<AuGLSun>& suns, const QList<AuGLLight>& lights, float ambientLight);

	// I am the target of an attack. Determine attack result.
	// Does not apply the result. Simply determines what the result would be. The logical next step would be CombatApplyDamage.
	// Input parameters:
	//   'weaponLoc' and 'hitLoc' are relative to this atom.
	//   'weapon' describes the attacking weapon.
	// Output parameters:
	//   'result' configured with output parameters. IsDestroyed not determined yet; done by subsequent call to CombatApplyDamage.
	void TargetAttacked(const AuPoint& weaponLoc, const AuPoint& hitLoc, bool hitShield, const AuCombatWeapon& weapon,
			AuCombatResult& result) const;

	// Returns a vector relative to this atom, in miles per hour.
	AuPoint Wind() const;

signals:
	// WARNING! High frequency signals can overrun the signal/slot queue. Don't use signals here that will be emitted frequently.
	void AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName, quint8 personGender);

	void ArenaReady(AuArenaReady arena);

	// Emitted daily by avatars whose account is active, in DaPerson::SetAccountActive.
	// Avatar might not be on line.
	void AvatarAccountActive(AuIdList land);

	void AvatarBorn(AuId motherAvatar, AuId babyAvatar);

	// This is a low frequency signal, like once every ten minutes or so.
	// It is used to control the noncombatant status of empires.
	// 'personId' is the person atom who is on line.
	// 'empireId' is the id of personId's empire.
	void AvatarOnLine(AuId personId, quint32 empireId);

	// 'why' is passed to AddBounty.
	void BountyPosted(AuId posterId, QString posterName, AuId victim, QString reason, double reward, int why);

	void CityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, AuId cityId);

	void CityCreated(quint32 empireId,
			quint8 cityType, AuId cityId, QString cityName,
			AuId founderId, QString founderName,
			AuId planetId, QString planetName,
			AuId systemId, QString systemName, AuPoint systemLoc,
			AuId sectorId, QString sectorName,
			AuId galaxyId, quint8 galaxy);

	void CityDecayed(quint32 empireId, AuId cityId);

	void CityFacilitiesChanged(AuId cityId, AuCityIndustryHash indries);

	void CityStatusChanged(AuId cityId, int alertCondition, qint8 morale);

	// Signal emitted only on the client to propagate to the server a collision resulting from a client-side move.
	void CollisionOnClient(AuId myId, AuPoint myChange, AuPoint myHitLoc, AuId hitId, AuPoint hitChange, AuPoint hitFromLoc,
			AuPoint hitResultVec, bool hitShield);

	void CreateEmpire();

	void CreditCityBank(float fAmount, AuId cityId);

	void DeadheadedOnClient(AuId systemId, AuId moveId, AuPoint moveLoc);

	void DemiavatarDied(AuId avatar);

	void EmpireCityWorkerSpeciesChanged(quint32 empireId, AuId cityId, AuSpecies workerSpecies);

	void EmpireRecordPatent(quint32 empireId, quint16 patentCID, quint8 patentQ);

	void EmpireRequested(quint32 empireId);								// Only emitted on the client.

	void EnteredDesigner(AuId personId, int designMode);

	// Signal emitted only on the client to propagate to the server a client-side move into a wormhole.
	void EnteredWormholeOnClient(AuId travId, AuId systemId, AuWormhole wh, float entryAngle);

	// 'fellId' is the atom who fell and is getting damaged.
	// 'hitsDamage' is the number of hit points of collision damage to apply to the falling atom.
	// 'stopReason' is 0=hit building floor, 1=hit building ceiling, 2=hit terrain
	void FellOnClient(AuId fellId, quint32 hitsDamage, int stopReason);

	// 'empireId' is the empire storing the document in their dossier.
	// 'dosEmpire' is the empire in the dossier.
	// 'packedDoc' is the document to store in the dossier, created using the Pack function of AuDocument.
	void GovDossierAddDocument(quint32 empireId, quint32 dosEmpire, QByteArray packedDoc);

	// 'domain' is NULL is setting empire capital, galaxy for galaxy capital, and sector for sector capital.
	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapital(quint32 empire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType);

	void HengeActivatedSignal(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreatedSignal(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivatedSignal(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	// Signaled when a world has created a destination henge portal. It is a confirmation of sorts that the destination system
	// of a henge portal actually has a world for the henge destination portal.
	// Story engine uses this information to locate destination henge portals.
	void HengeDestinationSignal(AuId fromSectorId, AuId fromSysId, QString fromSysName, quint8 fromSun, QString fromWorldCatName,
			QString fromWorldName, quint8 fromWorldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideCity);

	// Emitted when an atom has entered the active teleporter at the center of a henge.
	// 'teleportId' is the atom to teleport. It could be my parent if I am riding a mount.
	void HengeTeleportSignal(AuId teleportId, AuId fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels,
			AuId destSysId, quint8 destSun, quint8 destArc);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void OpenChannelTo(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
			quint8 scope, bool propagateUp);

	void OperateTurboLift(AuId shipId, AuId personId, quint16 liftId);

	// Emit this signal to do two things:
	//  1) Change ownership of private property 'propertyId' 'propertyName' to 'ownerId' 'ownerName'. e.g. spacecraft title.
	//  2) Voids all existing ownership titles for 'propertyId'.
	void ClaimTitle(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	// 'encSectorId' is the sector that contains the vessel who triggered the pirate base encounter.
	// 'encSystemId' is the solar system that contains the vessel who triggered the pirate base encounter.
	void SectorPirateBaseEncounter(AuId encSectorId, AuId encSystemId);

	// 'fromGalaxy' is a GALAXY value.
	void SendFunds(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	void ShipIt(AuItemTEUContainer shipment);

	// Emitted when the name of a spacecraft changes so people who hold a berth aboard it can be updated.
	void SpacecraftNameChanged(AuId shipId, QString nuName);

	// Emitted when an originating stargate is closed, so the destination negative stargate can be closed also.
	void StargateClosed(AuId fromGalaxyId, AuPoint fromGalaxyLoc);

	// Emitted when an originating stargate is opened, with the destination galaxy location of the destination solar system, in pc.
	void StargateOpened(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuTime expire);

	void SystemWarpSignature(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	// Notification from a broker when a shipment under a trade agreement is ready to ship.
	// 'readyCapitolId' says whether this is being sent by the origin or destination of the trade agreement.
	void TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta);

	// Response from a broker when ReadyToShip is received and he is ready to ship. The broker who sent the ReadyToShip signal ships
	// when they receive this signal at their end.
	// 'shipperCapitolId' says whether this is being sent by the origin or destination of the trade agreement.
	void TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta);

	// Signal emitted by a city atom when it detects the emission from a distant solar system that consumed power.
	// 'signalStrength' is the amount of the warp signature that was detected.
	// 'detectingEmpireId' is the empire id of the city that detected the emission from system 'systemId'.
	void WarpSignatureDetected(AuId systemId, quint32 signalStrength, quint32 detectingEmpireId);

	// Emitted when a game is finished.
	void GameOver(AuId shipId, quint16 whichTable, quint8 gameType, QByteArray gameResult);

	// Emitted periodically, not just when the turn changes.
	void GameTurnPoll(AuId shipId, quint16 whichTable, AuId personsTurn);

	void OfficerKilled(AuId homePlanetId);

	void PostJobOffer(AuJobOffer jobOffer);
	void ReleaseBerthSignal(AuId myId, AuId exceptShipId);				// Releases my berth from all ships except 'exceptShipId'.

	// 'stationType' is a DpStation::STATION_TYPE
	void RestoreConsoleSettings(quint8 stationType, QByteArray consoleSettings);

	// Sets political stance of 'fromEmpire' toward 'toEmpire' as a result of initial contact between units of two empires.
	// 'vassalTributePercent' is relevant when becoming a vassal.
	// 'reason' is an AuDocument::Type() value, for the dossier.
	// 'fromUnit' is the unit in 'fromEmpire' who triggered this stance setting.
	// 'fromStance' is the stance taken by the fromEmpire.
	// 'toStance' is the stance taken by the toEmpire.
	// 'toUnit' is the unit in 'fromEmpire' who triggered this stance setting.
	// 'galaxy', 'sector', 'system' and 'world' identify where this incident occurred.
	void SetPoliticalStance(quint32 toEmpire, quint32 fromEmpire, quint8 fromStance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world) const;

	void SolarSystemExplored(AuId id, quint32 empireId);
	void SolarSystemFinal(AuId id);
	void SolarSystemReadyToDecay(AuId id);
	void SolarSystemScanned(AuId id, quint32 empireId, quint8 genHab, quint32 genSeed);

	void SpacecraftPositionReport(AuId id, AuId systemId, AuStarMapSpacecraft pos);

	// 'galaxy' is a GALAXY enum value
	void Spawn(AuId personOrShip, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arenaExit, quint8 galaxy);

	void TransporterCoord(AuPoint coord);

	void TransporterLongLat(AuGlobeLLF ll);

	// when traveling via intergalactic wormhole, 'destSystem' is a galaxy id and 'destSector' is the universe id.
	void Travel(AuId id, AuId origSystem, AuId destSystem, AuId destSector);

	void TravelGalaxyLoc(AuId id, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void TravelStargate(AuId id, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

private:
	AuQueryEnvironment* CreateEnvironmentQuery() const;

public:
	virtual quint32 BuyCost(const AuId& idFromWho, const AuItem& item)
	{
		Q_UNUSED(idFromWho);
		Q_UNUSED(item);
		return 0;
	}

	// Gets a current tally of commodities I can supply.
	virtual void CommoditySupplies(AuCommodityExchange& cex) const
	{
		Q_UNUSED(cex);
	}

	virtual double CommunityAccountBalance() const
	{
		return 0;
	}

	virtual void CommunityAddCommodity(const AuItem& iNew)
	{
		Q_UNUSED(iNew);
	}

	virtual void CommunityCreditAccount(double amount, AuBank::CATEGORY cat)
	{
		Q_UNUSED(amount);
		Q_UNUSED(cat);
	}

	virtual void CommunityDebitAccount(double amount, AuBank::CATEGORY cat)
	{
		Q_UNUSED(amount);
		Q_UNUSED(cat);
	}

	// Fills 'items' with all cargo available to this building, tallied from all ground connection in my community.
	// 'empireAsking' is used to check commerce restrictions.
	virtual void CommunityInventoryAvailable(AuItemHash& items, quint32 empireAsking) const
	{
		Q_UNUSED(items);
		Q_UNUSED(empireAsking);
	}

	virtual void CommunityRemoveInventory(const AuItem& items)
	{
		Q_UNUSED(items);
	}

	virtual quint32 CommunityTestFitCommodity(const AuItem& offer) const
	{
		Q_UNUSED(offer);
		return 0;
	}

	// These interfaces enable commodities to be imported between DaBd objects.
	virtual void DischargeStoredPower(quint32 units)
	{
		Q_UNUSED(units);
	}

	virtual const AuItemHash* LockCommoditiesForRead()
	{
		return 0;
	}

	virtual AuItemHash* LockCommoditiesForWrite()
	{
		return 0;
	}

	virtual bool RemoveCommodityNoLock(const AuItem& take, quint32 howMany)
	{
		Q_UNUSED(take);
		Q_UNUSED(howMany);
		return false;
	}

	virtual quint32 SellCost(const AuId& idToWho, const AuItem& item)
	{
		Q_UNUSED(idToWho);
		Q_UNUSED(item);
		return 0;
	}

	virtual quint32 StoredPowerLevel() const
	{
		return 0;
	}

	virtual void UnlockCommoditiesForRead()
	{
	}

	virtual void UnlockCommoditiesForWrite(bool anyChange)
	{
		Q_UNUSED(anyChange);
	}

#if defined(MEM_TRACE_ATOM)
public:
	void* operator new(size_t size);
	void operator delete(void * p);
	static quint64 AllocatedCount();
	static quint64 FreedCount();
#endif
};

inline QDataStream& operator<<(QDataStream& out, const DaAtom& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DaAtom& o)
{
	o << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const DaAtom& a);

class DqAtomLoc
{
public:
	DaAtom	*m_pA;
	AuPoint	m_fLocation;
};

class DqAtomLocArray : public QList<DqAtomLoc>
{
};

#endif // DaAtom_H
