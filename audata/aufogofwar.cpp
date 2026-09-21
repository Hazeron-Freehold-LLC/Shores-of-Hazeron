//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufogofwar.cpp,v 1.9 2022/05/17 18:23:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aufogofwar.h"
//

AuFogOfWarData::AuFogOfWarData()
{
}

AuFogOfWarData::AuFogOfWarData(const AuPointArray& fog)
{
	for(AuPointArray::const_iterator fi = fog.cbegin(); fi != fog.cend(); fi++)
		m_Fog.append(*fi);
}

void AuFogOfWarData::operator>>(QDataStream& out) const
{
	quint8 ver = 0;
	out << ver;
	out << m_Fog;
}

void AuFogOfWarData::operator<<(QDataStream& in)
{
	quint8 ver = 0;
	in >> ver;
	in >> m_Fog;
}

bool AuFogOfWarData::operator==(const AuFogOfWarData& c) const
{
	if(m_Fog != c.m_Fog)
		return false;
	return true;
}

bool AuFogOfWarData::Explore(const AuPoint& seaLevelLoc, double sightRadius)
{
	bool anyChange = false;

	double searchRadius = sightRadius + Radius() * .9;	// So sighting within 90% of the radius sees the spot.
	QList<AuPoint>::iterator fi = m_Fog.begin();
	while(fi != m_Fog.end())
	{
		double d = (*fi).Distance(seaLevelLoc);	// Distance from seaLevelLoc to sea level fog loc.
		if(d < searchRadius)
		{
			// Fog spot is within the search radius. Remove the fog spot.
			fi = m_Fog.erase(fi);
			anyChange = true;
		}
		else
			fi++;	// Fog spot is beyond the search radius. Skip it.
	}

	if(!anyChange)
		return false;

	m_LastChange.Set();
	return true;
}

bool AuFogOfWarData::ExploreAll()
{
	if(m_Fog.empty())
		return false;

	m_Fog.clear();
	m_LastChange.Set();
	return true;
}

bool AuFogOfWarData::IsExplored(const AuPoint& seaLevelLoc) const
{
	// Test for unknown spot at the location.
	if(!m_Fog.empty())
	{
		double radiusSq = Radius() * Radius();
		for(QList<AuPoint>::const_iterator fi = m_Fog.cbegin(); fi != m_Fog.cend(); fi++)
			if((*fi).Distance2(seaLevelLoc) < radiusSq)
				return false;	// Search location is not explored because it is in the radius of an unknown spot.
	}

	// Search location is explored because it is not blocked by any unkown spot.
	return true;
}

//static
double AuFogOfWarData::Radius()
{
	return 5280 / 2;	// 1/2 mile
}

bool AuFogOfWarData::Combine(const AuFogOfWarData& fog)
{
	bool anyChange = false;

	QList<AuPoint>::iterator fi = m_Fog.begin();
	while(fi != m_Fog.end())
	{
		// Search in 'fog' for 'fi'.
		if(fog.Contains(*fi))
			fi++;
		else
		{
			fi = m_Fog.erase(fi);
			anyChange = true;
		}
	}

	if(!anyChange)
		return false;

	m_LastChange.Set();
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuFogOfWar::AuFogOfWar()
{
}

AuFogOfWar::~AuFogOfWar()
{
}

AuFogOfWar::AuFogOfWar(const AuFogOfWar& c) : m_DataByAvatar(c.m_DataByAvatar), m_DataByEmpire(c.m_DataByEmpire)
{
}

void AuFogOfWar::operator>>(QDataStream& out) const
{
	quint8 ver = 0;
	out << ver;
	out << m_DataByAvatar;
	out << m_DataByEmpire;
}

void AuFogOfWar::operator<<(QDataStream& in)
{
	quint8 ver = 0;
	in >> ver;
	in >> m_DataByAvatar;
	in >> m_DataByEmpire;
}

bool AuFogOfWar::Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double sightRadius)
{
	bool anyChange = false;

	if(!avatarId.IsNull())
	{
		if(empireId)
		{
			// Avatar and empire are not null. Combine avatar into empire.
			anyChange = Combine(avatarId, empireId);
		}
		else
		{
			// No empire id so explore on behalf of empireless avatar.
			QHash<AuId, AuFogOfWarData>::iterator dai = m_DataByAvatar.find(avatarId);
			if(dai != m_DataByAvatar.cend())
				anyChange = (*dai).Explore(seaLevelLoc, sightRadius);
			return anyChange;
		}
	}

	if(empireId)
	{
		QHash<quint32, AuFogOfWarData>::iterator dei = m_DataByEmpire.find(empireId);
		if(dei != m_DataByEmpire.cend())
			if((*dei).Explore(seaLevelLoc, sightRadius))
				anyChange = true;
	}

	return anyChange;
}

bool AuFogOfWar::ExploreAll(const AuId& avatarId, quint32 empireId)
{
	bool anyChange = false;

	if(!avatarId.IsNull())
	{
		if(empireId)
		{
			// Avatar and empire are not null. Combine avatar into empire.
			anyChange = Combine(avatarId, empireId);
		}
		else
		{
			// No empire id so explore on behalf of empireless avatar.
			QHash<AuId, AuFogOfWarData>::iterator dai = m_DataByAvatar.find(avatarId);
			if(dai != m_DataByAvatar.cend())
				return (*dai).ExploreAll();
			else
			{
				AuFogOfWarData fow;	// Empty fow is fully explored.
				m_DataByAvatar.insert(avatarId, fow);
				return true;
			}
		}
	}

	if(empireId)
	{
		QHash<quint32, AuFogOfWarData>::iterator dei = m_DataByEmpire.find(empireId);
		if(dei != m_DataByEmpire.cend())
		{
			if((*dei).ExploreAll())
				anyChange = true;
		}
		else
		{
			AuFogOfWarData fow;	// Empty fow is fully explored.
			m_DataByEmpire.insert(empireId, fow);
			anyChange = true;
		}
	}

	return anyChange;
}

void AuFogOfWar::Init(const AuId& avatarId, quint32 empireId, const AuPointArray& fog)
{
	//qDebug() << "AuFogOfWar::Init fog" << fog.size();
	if(empireId)
	{
		// Init empire first in case we decide to combine the avatar's data.
		if(!m_DataByEmpire.contains(empireId))
		{
			AuFogOfWarData fow(fog);
			m_DataByEmpire.insert(empireId, fow);
		}

		if(!avatarId.IsNull())
		{
			// Avatar and empire are not null. Combine avatar into empire.
			Combine(avatarId, empireId);
		}
		return;
	}

	if(!avatarId.IsNull())
	{
		// No empire id so init empireless avatar.
		if(!m_DataByAvatar.contains(avatarId))
		{
			AuFogOfWarData fow(fog);
			m_DataByAvatar.insert(avatarId, fow);
		}
	}
}

bool AuFogOfWar::IsExplored(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc) const
{
	if(empireId)
	{
		QHash<quint32, AuFogOfWarData>::const_iterator dei = m_DataByEmpire.find(empireId);
		if(dei != m_DataByEmpire.cend())
			if((*dei).IsExplored(seaLevelLoc))
				return true;
	}
	else if(!avatarId.IsNull())
	{
		QHash<AuId, AuFogOfWarData>::const_iterator dai = m_DataByAvatar.find(avatarId);
		if(dai != m_DataByAvatar.cend())
			if((*dai).IsExplored(seaLevelLoc))
				return true;
	}
	return false;
}

bool AuFogOfWar::EmpireDeleted(quint32 empireId)
{
	QHash<quint32, AuFogOfWarData>::iterator dei = m_DataByEmpire.find(empireId);
	if(dei == m_DataByEmpire.end())
		return false;

	m_DataByEmpire.erase(dei);
	return true;
}

bool AuFogOfWar::NeedsInit(const AuId& avatarId, quint32 empireId) const
{
	if(empireId)
	{
		// The empire id is not null. It MUST have a record.
		// Avatar id is not needed when using an empire id.
		if(!m_DataByEmpire.contains(empireId))
			return true;	// We have no data for the empire so Init is needed.
	}
	else if(!avatarId.IsNull())
	{
		// Avatar id provided but no empire id.
		if(!m_DataByAvatar.contains(avatarId))
			return true;	// We have no data for the avatar so Init is needed.
	}
	return false;
}

bool AuFogOfWar::Combine(const AuId& avatarId, quint32 empireId)
{
	QHash<AuId, AuFogOfWarData>::iterator dai = m_DataByAvatar.find(avatarId);
	if(dai == m_DataByAvatar.end())
		return false;
	QHash<quint32, AuFogOfWarData>::iterator dei = m_DataByEmpire.find(empireId);
	if(dei == m_DataByEmpire.end())
		return false;

	(*dei).Combine(*dai);
	m_DataByAvatar.erase(dai);
	return true;
}

bool AuFogOfWar::HasChanged(const AuId& avatarId, const quint32 empireId, const AuElapsedTimer& tmLastKnownChange) const
{
	if(empireId)
	{
		QHash<quint32, AuFogOfWarData>::const_iterator dei = m_DataByEmpire.find(empireId);
		if(dei != m_DataByEmpire.cend())
			return (*dei).LastChanged() >= tmLastKnownChange;	// We found the empire so that's all we care about.
	}

	QHash<AuId, AuFogOfWarData>::const_iterator dai = m_DataByAvatar.find(avatarId);
	if(dai == m_DataByAvatar.cend())
		return false;

	return (*dai).LastChanged() >= tmLastKnownChange;
}

void AuFogOfWar::Tx(QDataStream& toClient, const AuId& avatarId, quint32 empireId)
{
	QHash<quint32, AuFogOfWarData>::const_iterator dei = m_DataByEmpire.find(empireId);
	QHash<AuId, AuFogOfWarData>::const_iterator dai;
	if(dei == m_DataByEmpire.cend())
		dai = m_DataByAvatar.find(avatarId);	// Only if empire does not exist.

	quint8 hasWhat = 0;
	if(dei != m_DataByEmpire.cend())
		hasWhat |= 0x01;
	else if(dai != m_DataByAvatar.cend())
		hasWhat |= 0x02;
	toClient << hasWhat;

	if(hasWhat & 0x01)
	{
		toClient << empireId;
		toClient << *dei;
	}

	if(hasWhat & 0x02)
	{
		toClient << avatarId;
		toClient << *dai;
	}
}

void AuFogOfWar::Rx(QDataStream& fromServer)
{
	quint8 hasWhat = 0;
	fromServer >> hasWhat;

	m_DataByEmpire.clear();
	if(hasWhat & 0x01)
	{
		quint32 empireId;
		AuFogOfWarData data;

		fromServer >> empireId;
		fromServer >> data;

		//qDebug() << "AuFogOfWar::Rx empire" << empireId << "data" << data.Fog().size();
		m_DataByEmpire.insert(empireId, data);
	}

	m_DataByAvatar.clear();
	if(hasWhat & 0x02)
	{
		AuId avatarId;
		AuFogOfWarData data;

		fromServer >> avatarId;
		fromServer >> data;

		//qDebug() << "AuFogOfWar::Rx avatar" << avatarId << "data" << data.Fog().size();
		m_DataByAvatar.insert(avatarId, data);
	}
}

void AuFogOfWar::GetData(const AuId& avatarId, const quint32 empireId, AuFogOfWarData& fow) const
{
	if(empireId)
	{
		QHash<quint32, AuFogOfWarData>::const_iterator dei = m_DataByEmpire.find(empireId);
		if(dei != m_DataByEmpire.cend())
		{
			fow = *dei;
			return;
		}
	}

	if(!avatarId.IsNull())
	{
		QHash<AuId, AuFogOfWarData>::const_iterator dai = m_DataByAvatar.find(avatarId);
		if(dai != m_DataByAvatar.cend())
			fow = *dai;
	}
}

