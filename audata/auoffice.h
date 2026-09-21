//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auoffice.h,v 1.27 2022/02/19 18:19:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuOffice_H
#define AuOffice_H

#include "audata.h"
#include "auhash.h"
#include "aupoint.h"
#include <QDataStream>
#include <QList>

class AUDATA_DLL AuOfficeDomainPlace
{
public:
	virtual ~AuOfficeDomainPlace()
	{
	}

public:
	virtual void operator>>(QDataStream& out) const = 0;

	virtual void operator<<(QDataStream& in) = 0;

public:
	virtual AuId AtomId() const = 0;

	virtual AuOfficeDomainPlace* Clone() const = 0;

	static AuOfficeDomainPlace* Create(quint8 type);

	// Return resource name of icon to use in lists.
	virtual QString Icon() const = 0;

	virtual bool IsPlace(const AuId& placeId) const = 0;

	virtual quint64 MemoryUsage() const = 0;

	virtual QString Name() const = 0;

	virtual quint32 TargetValue() const = 0;

	virtual quint8 Type() const = 0;
};

class AUDATA_DLL AuOfficeDomainAtom : public AuOfficeDomainPlace
{
	AuId								m_AtomId;
	QString								m_AtomName;

public:
	inline AuOfficeDomainAtom(const AuId& id, const QString& name)
	{
		m_AtomId = id;
		m_AtomName = name;
	}

	inline AuOfficeDomainAtom()
	{
	}

	virtual ~AuOfficeDomainAtom()
	{
	}

	inline AuOfficeDomainAtom(const AuOfficeDomainAtom& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomName = c.m_AtomName;
	}

public:
	virtual void operator>>(QDataStream& out) const
	{
		out << m_AtomId;
		out << m_AtomName;
	}

	virtual void operator<<(QDataStream& in)
	{
		in >> m_AtomId;
		in >> m_AtomName;
	}

public:
	virtual AuId AtomId() const
	{
		return m_AtomId;
	}

	inline const QString& AtomName() const
	{
		return m_AtomName;
	}

	virtual bool IsPlace(const AuId& placeId) const
	{
		return m_AtomId == placeId;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return AtomName();
	}
};

class AUDATA_DLL AuOfficeDomainCity : public AuOfficeDomainAtom
{
public:
	AuOfficeDomainCity(const AuId& id, const QString& name) : AuOfficeDomainAtom(id, name)
	{
	}

	AuOfficeDomainCity()
	{
	}

public:
	virtual AuOfficeDomainCity* Clone() const
	{
		return new AuOfficeDomainCity(*this);
	}

	virtual QString Icon() const
	{
		return ":data/City.png";
	}

	static quint8 PlaceType()
	{
		return 6;
	}

	virtual quint32 TargetValue() const
	{
		return 3;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

class AUDATA_DLL AuOfficeDomainSector : public AuOfficeDomainAtom
{
public:
	AuOfficeDomainSector(const AuId& id, const QString& name) : AuOfficeDomainAtom(id, name)
	{
	}

	AuOfficeDomainSector()
	{
	}

public:
	virtual AuOfficeDomainSector* Clone() const
	{
		return new AuOfficeDomainSector(*this);
	}

	virtual QString Icon() const
	{
		return ":data/RangeSector.png";
	}

	static quint8 PlaceType()
	{
		return 3;
	}

	virtual quint32 TargetValue() const
	{
		return 25;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

class AUDATA_DLL AuOfficeDomainSolarSystem : public AuOfficeDomainAtom
{
public:
	AuOfficeDomainSolarSystem(const AuId& id, const QString& name) : AuOfficeDomainAtom(id, name)
	{
	}

	AuOfficeDomainSolarSystem()
	{
	}

public:
	virtual AuOfficeDomainSolarSystem* Clone() const
	{
		return new AuOfficeDomainSolarSystem(*this);
	}

	virtual QString Icon() const
	{
		return ":data/RangeSystem.png";
	}

	static quint8 PlaceType()
	{
		return 4;
	}

	virtual quint32 TargetValue() const
	{
		return 12;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

class AUDATA_DLL AuOfficeDomainWorld : public AuOfficeDomainAtom
{
public:
	AuOfficeDomainWorld(const AuId& id, const QString& name) : AuOfficeDomainAtom(id, name)
	{
	}

	AuOfficeDomainWorld()
	{
	}

public:
	virtual AuOfficeDomainWorld* Clone() const
	{
		return new AuOfficeDomainWorld(*this);
	}

	virtual QString Icon() const
	{
		return ":data/RangeHail.png";
	}

	static quint8 PlaceType()
	{
		return 5;
	}

	virtual quint32 TargetValue() const
	{
		return 6;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

class AUDATA_DLL AuOfficeDomainGalaxy : public AuOfficeDomainAtom
{
public:
	AuOfficeDomainGalaxy(const AuId& id, const QString& name) : AuOfficeDomainAtom(id, name)
	{
	}

	AuOfficeDomainGalaxy()
	{
	}

public:
	virtual AuOfficeDomainGalaxy* Clone() const
	{
		return new AuOfficeDomainGalaxy(*this);
	}

	virtual QString Icon() const
	{
		return ":data/RangeGalaxy.png";
	}

	static quint8 PlaceType()
	{
		return 2;
	}

	virtual quint32 TargetValue() const
	{
		return 50;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

class AUDATA_DLL AuOfficeDomainUniverse : public AuOfficeDomainAtom
{
public:
	AuOfficeDomainUniverse() : AuOfficeDomainAtom(AuId::Universe(), "Universe")
	{
	}

public:
	virtual AuOfficeDomainUniverse* Clone() const
	{
		return new AuOfficeDomainUniverse(*this);
	}

	virtual QString Icon() const
	{
		return ":data/RangeUniverse.png";
	}

	static quint8 PlaceType()
	{
		return 1;
	}

	virtual quint32 TargetValue() const
	{
		return 100;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

/*
class AUDATA_DLL AuOfficeDomainSphere : public AuOfficeDomainPlace
{
	AuPoint								m_GalaxyLoc;			// Center location, in parsecs.
	float								m_Radius;				// Radius, in parsecs.

public:
	AuOfficeDomainSphere(const AuPoint& galaxyLoc, float radiusPc)
	{
		m_GalaxyLoc = galaxyLoc;
		m_Radius = radiusPc;
	}

	virtual ~AuOfficeDomainSphere()
	{
	}

public:
	virtual void operator>>(QDataStream& out) const
	{
		out << (float)m_GalaxyLoc.x;
		out << (float)m_GalaxyLoc.y;
		out << (float)m_GalaxyLoc.z;
		out << m_Radius;
	}

	virtual void operator<<(QDataStream& in)
	{
		float x,y,z;
		in >> x;
		in >> y;
		in >> z;
		m_GalaxyLoc = AuPoint(x,y,z);
		in >> m_Radius;
	}

public:
	virtual AuOfficeDomainPlace* Clone() const
	{
		return new AuOfficeDomainSphere(*this);
	}

	static quint8 PlaceType()
	{
		return 7;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};

class AUDATA_DLL AuOfficeDomainCube : public AuOfficeDomainPlace
{
	AuPoint								m_GalaxyLoc;			// Center location, in parsecs.
	float								m_Radius;				// Radius, in parsecs.

public:
	AuOfficeDomainCube(const AuPoint& galaxyLoc, float radiusPc)
	{
		m_GalaxyLoc = galaxyLoc;
		m_Radius = radiusPc;
	}

	virtual ~AuOfficeDomainCube()
	{
	}

public:
	virtual void operator>>(QDataStream& out) const
	{
		out << (float)m_GalaxyLoc.x;
		out << (float)m_GalaxyLoc.y;
		out << (float)m_GalaxyLoc.z;
		out << m_Radius;
	}

	virtual void operator<<(QDataStream& in)
	{
		float x,y,z;
		in >> x;
		in >> y;
		in >> z;
		m_GalaxyLoc = AuPoint(x,y,z);
		in >> m_Radius;
	}

public:
	virtual AuOfficeDomainPlace* Clone() const
	{
		return new AuOfficeDomainCube(*this);
	}

	static quint8 PlaceType()
	{
		return 8;
	}

	virtual quint8 Type() const
	{
		return PlaceType();
	}
};
*/

inline QDataStream& operator>>(QDataStream& in, AuOfficeDomainPlace& o)
{
	o << in;
	return in;
}

inline QDataStream& operator<<(QDataStream& out, const AuOfficeDomainPlace& o)
{
	o >> out;
	return out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuOfficeDomain : public QList<AuOfficeDomainPlace*>
{
public:
	inline AuOfficeDomain()
	{
	}

	inline ~AuOfficeDomain()
	{
		Clear();
	}

	AuOfficeDomain(const AuOfficeDomain& c) : QList<AuOfficeDomainPlace*>()
	{
		for(const_iterator ci = c.cbegin(); ci != c.cend(); ci++)
			Add(**ci);
	}

public:
	const AuOfficeDomain& operator=(const AuOfficeDomain& c)
	{
		Clear();
		for(const_iterator ci = c.cbegin(); ci != c.cend(); ci++)
			Add(**ci);

		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline void Add(const AuOfficeDomainPlace& place)
	{
		append(place.Clone());
	}

	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	inline void DeletePlace(const AuId& placeId)
	{
		iterator pi = begin();
		while(pi != end())
		{
			if((*pi)->IsPlace(placeId))
			{
				delete *pi;
				pi = erase(pi);
			}
			else
				pi++;
		}
	}

	inline bool IsPlaceInDomain(const AuId& placeId) const
	{
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
			if((*pi)->IsPlace(placeId))
				return true;
		return false;
	}

	quint64 MemoryUsage() const;

	inline quint32 TargetValue() const
	{
		quint32 tv = 0;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
			tv += (*pi)->TargetValue();
		return tv;
	}
};

inline QDataStream& operator>>(QDataStream& in, AuOfficeDomain& o)
{
	o << in;
	return in;
}

inline QDataStream& operator<<(QDataStream& out, const AuOfficeDomain& o)
{
	o >> out;
	return out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuOffice
{
friend class AuGovAssignOffice;
private:
	// Id of avatar who delegated this office to the avatar. An avatar who holds an office assigned by id 0 is at the top of the
	// hierarchy, considered to be the "emperor". Emperors override a lot of the "can do" settings to prevent stuck situations.
	AuId								m_AssignedBy;
	QString								m_Title;				// Title of office.

	// Empire wide permissions.
	bool								m_CanAcceptCitySurrender : 1;	// Can accept cities surrendered to the empire.
	bool								m_CanDelegatePermissions : 1;
	bool								m_CanInvite : 1;		// Can invite people to join the empire.
	bool								m_CanKick : 1;			// Can kick citizens the empire.
	bool								m_CanManageDossier : 1;	// Can modify dossier of known empires.
	bool								m_CanManageTrade : 1;					// Can modify trade routes.
	bool								m_CanSendEmpireMail : 1;	// Can broadcast mail to all empire members.
	bool								m_CanSetAnnouncements : 1;	// Can set empire wide announcements.
	bool								m_CanSetCapital : 1;
	bool								m_CanSetColorTheme : 1;
	bool								m_CanSetCommerce : 1;
	bool								m_CanSetContrailColor : 1;
	bool								m_CanSetEmpireFlag : 1;
	bool								m_CanSetJoinPassword : 1;
	bool								m_CanSetPoliticalStance : 1;	// Political stance toward other empires.
	bool								m_CanSetTax : 1;
	bool								m_CanSetTitle : 1;		// Their own title of office.
	bool								m_CanSetWarCriteria : 1;	// Can set empire wide automatic war on neutrals criteria.
	bool								m_CanUseDiplomacyChannel : 1;

	// Domain wide permissions.
	bool								m_CanAccessGovtAccount : 1;
	bool								m_CanBuild : 1;
	bool								m_CanLoadOfficers : 1;
	bool								m_CanSetGalaxyCapital : 1;
	bool								m_CanSetMfg : 1;
	bool								m_CanSetMfgSpacecraft : 1;
	bool								m_CanSetPlaceNames : 1;
	bool								m_CanSetResearch : 1;
	bool								m_CanSetSectorCapital : 1;
	bool								m_RxFleetMessages : 1;	// Receives mail when a ship sends a log or routine message.
	bool								m_RxSensorScans : 1;	// Receives mail when a sensor scan is sent to command.

	// Domain
	AuOfficeDomain						m_Domain;				// Their domain. None if empty.

public:
	inline AuOffice()
	{
		m_CanAcceptCitySurrender = false;
		m_CanAccessGovtAccount = false;
		m_CanBuild = false;
		m_CanDelegatePermissions = true;
		m_CanInvite = false;
		m_CanKick = false;
		m_CanLoadOfficers = false;
		m_CanManageDossier = false;
		m_CanManageTrade = false;
		m_CanSendEmpireMail = false;
		m_CanSetAnnouncements = false;
		m_CanSetCapital = false;
		m_CanSetColorTheme = false;
		m_CanSetCommerce = false;
		m_CanSetContrailColor = false;
		m_CanSetEmpireFlag = false;
		m_CanSetGalaxyCapital = false;
		m_CanSetJoinPassword = false;
		m_CanSetMfg = false;
		m_CanSetMfgSpacecraft = false;
		m_CanSetPlaceNames = false;
		m_CanSetPoliticalStance = false;
		m_CanSetResearch = false;
		m_CanSetSectorCapital = false;
		m_CanSetTax = false;
		m_CanSetTitle = false;
		m_CanSetWarCriteria = false;
		m_CanUseDiplomacyChannel = false;
		m_RxFleetMessages = false;
		m_RxSensorScans = false;
	}

	AuOffice(const AuOffice& c);

public:
	const AuOffice& operator=(const AuOffice& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline void AddDomain(const AuOfficeDomainPlace& place)
	{
		m_Domain.Add(place);
	}

	inline const AuId& BossId() const
	{
		return m_AssignedBy;
	}

	// True if this citizen is a governor and they have the authority to accept surrendered cities into the empire.
	inline bool CanAcceptCitySurrender() const
	{
		return m_CanAcceptCitySurrender;
	}

	inline bool CanAccessGovtAccount() const
	{
		return m_CanAccessGovtAccount;
	}

	// True if the holder of this office has the authority to delegate their power.
	inline bool CanDelegate() const
	{
		// Emperor must have this right or they can't modify their own settings.
		if(IsEmperor())
			return true;
		return m_CanDelegatePermissions;
	}

	inline bool CanInviteCitizens() const
	{
		return m_CanInvite;
	}

	inline bool CanKickCitizens() const
	{
		return m_CanKick;
	}

	inline bool CanLoadOfficer(const AuIdList& placeIds) const
	{
		if(!m_CanLoadOfficers)
			return false;
		return IsPlaceHierarchyInDomain(placeIds);
	}

	inline bool CanManageDossier() const
	{
		// This setting was added late so all emperors need to be able to modify it. They won't otherwise have permission.
		if(IsEmperor())
			return true;
		return m_CanManageDossier;
	}

	inline bool CanManageTrade() const
	{
		// This setting was added late so all emperors need to be able to modify it. They won't otherwise have permission.
		if(IsEmperor())
			return true;
		return m_CanManageTrade;
	}

	inline bool CanReceiveFleetMessages() const
	{
		return m_RxFleetMessages;
	}

	inline bool CanSendEmpireMail() const
	{
		return m_CanSendEmpireMail;
	}

	inline bool CanSetAnnouncements() const
	{
		return m_CanSetAnnouncements;
	}

	inline bool CanSetCapital() const
	{
		return m_CanSetCapital;
	}

	inline bool CanSetColorTheme() const
	{
		return m_CanSetColorTheme;
	}

	inline bool CanSetCommerce() const
	{
		return m_CanSetCommerce;
	}

	inline bool CanSetContrailColor() const
	{
		return m_CanSetContrailColor;
	}

	inline bool CanSetEmpireFlag() const
	{
		return m_CanSetEmpireFlag;
	}

	// Returns true for emperor and anyone with permission to set sector capitals.
	inline bool CanSetGalaxyCapital() const
	{
		return m_CanSetGalaxyCapital;
	}

	inline bool CanSetGalaxyCapital(const AuIdList& placeIds) const
	{
		if(!m_CanSetGalaxyCapital)
			return false;
		return IsPlaceHierarchyInDomain(placeIds);
	}

	inline bool CanSetJoinPassword() const
	{
		return m_CanSetJoinPassword;
	}

	inline bool CanSetPlaceNames() const
	{
		return m_CanSetPlaceNames;
	}

	inline bool CanSetPoliticalStance() const
	{
		return m_CanSetPoliticalStance;
	}

	// Returns true for emperor and anyone with permission to set sector capitals.
	inline bool CanSetSectorCapital() const
	{
		return m_CanSetSectorCapital;
	}

	inline bool CanSetSectorCapital(const AuIdList& placeIds) const
	{
		if(!m_CanSetSectorCapital)
			return false;
		return IsPlaceHierarchyInDomain(placeIds);
	}

	inline bool CanSetTax() const
	{
		return m_CanSetTax;
	}

	inline bool CanSetWarCriteria() const
	{
		// This setting was added late so all emperors need to be able to modify it. They won't otherwise have permission.
		if(IsEmperor())
			return true;
		return m_CanSetWarCriteria;
	}

	inline bool CanUseDiplomacyChannel() const
	{
		return m_CanUseDiplomacyChannel;
	}

	inline const AuOfficeDomain& Domain() const
	{
		return m_Domain;
	}

	bool FilterAssignment(AuOffice& office) const;

	inline bool IsEmperor() const
	{
		return m_AssignedBy.IsNull();
	}

	inline bool IsPlaceInDomain(const AuId& placeId) const
	{
		return m_Domain.IsPlaceInDomain(placeId);
	}

	inline bool IsPlaceHierarchyInDomain(const AuIdList& placeIds) const
	{
		for(AuIdList::const_iterator pi = placeIds.cbegin(); pi != placeIds.cend(); pi++)
			if(IsPlaceInDomain(*pi))
				return true;
		return false;
	}

	quint64 MemoryUsage() const;

	inline bool ReceivesSensorScans() const
	{
		return m_RxSensorScans;
	}

	// Returns true if the domain is empty on return.
	bool RevokeDomain(const AuId& placeId);

	void SetBossId(const AuId& bossId);

	inline void SetTitleOfOffice(const QString& title)
	{
		m_Title = title;
	}

	// Configures settings for an emperor.
	void SetupEmperor();

	inline quint32 TargetValue() const
	{
		return m_Domain.TargetValue();
	}

	inline const QString& TitleOfOffice() const
	{
		return m_Title;
	}

	// Returns the id of the universe atom, if the universe is in this office's domain.
	inline AuId UniverseDomainId() const
	{
		return AuId::Universe();
	}

	// Returns the name of the universe atom, if the universe is in this office's domain.
	inline QString UniverseDomainName() const
	{
		return "Universe";
	}
};

Q_DECLARE_METATYPE(AuOffice)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator>>(QDataStream& in, AuOffice& o)
{
	o << in;
	return in;
}

inline QDataStream& operator<<(QDataStream& out, const AuOffice& o)
{
	o >> out;
	return out;
}

#endif // AuOffice_H
