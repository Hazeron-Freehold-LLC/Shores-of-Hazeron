//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpref.cpp,v 1.17 2022/02/19 18:19:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdpref.h"
//

void AuBlueprintPref::operator>>(QDataStream& out) const
{
	out << m_BlueprintId;
	out << m_BuildEnvBits;
}

void AuBlueprintPref::operator<<(QDataStream& in)
{
	in >> m_BlueprintId;
	in >> m_BuildEnvBits;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuBdPref::operator>>(QDataStream& out) const
{
	out << (quint8)m_Blueprints.size();
	for(QList<AuBlueprintPref>::const_iterator bi = m_Blueprints.cbegin(); bi != m_Blueprints.cend(); bi++)
		out << *bi;
}

void AuBdPref::operator<<(QDataStream& in)
{
	m_Blueprints.clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		AuBlueprintPref bp;
		in >> bp;
		m_Blueprints.append(bp);
	}
}

void AuBdPref::FilterEnvironment(quint8 envBits)
{
	QList<AuBlueprintPref>::iterator bi = m_Blueprints.begin();
	while(bi != m_Blueprints.end())
		if((*bi).BuildEnvironmentOk(envBits))
			bi++;							// Keep blueprints that can be built in the environment.
		else
			bi = m_Blueprints.erase(bi);	// Remove blueprints that cannot be built in the environment.
}

void AuBdPref::RemoveBlueprint(quint32 blueprintId)
{
	for(QList<AuBlueprintPref>::iterator bi = m_Blueprints.begin(); bi != m_Blueprints.end(); bi++)
		if((*bi).BlueprintId() == blueprintId)
		{
			m_Blueprints.erase(bi);
			return;
		}
}

quint8 AuBdPref::BuildEnvironmentBits() const
{
	quint8 envBits = 0;
	for(QList<AuBlueprintPref>::const_iterator bi = m_Blueprints.cbegin(); bi != m_Blueprints.cend(); bi++)
		envBits |= (*bi).BuildEnvironmentBits();
	return envBits;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuBdPrefs::operator>>(QDataStream& out) const
{
	out << (quint16)m_Prefs.size();
	for(QHash<INDUSTRYID, AuBdPref>::const_iterator pi = m_Prefs.cbegin(); pi != m_Prefs.cend(); pi++)
	{
		out << (quint8)pi.key();
		out << *pi;
	}
}

void AuBdPrefs::operator<<(QDataStream& in)
{
	m_Prefs.clear();
	quint16 c;
	in >> c;
	while(c--)
	{
		quint8 i;
		AuBdPref p;
		in >> i;
		in >> p;
		m_Prefs.insert((INDUSTRYID)i, p);
	}
}

AuBdPref AuBdPrefs::PreferredBlueprints(INDUSTRYID industryId, quint8 envBits) const
{
	AuBdPref pref = PreferredBlueprints(industryId);
	pref.FilterEnvironment(envBits);
	return pref;
}

AuBdPref AuBdPrefs::PreferredBlueprints(INDUSTRYID industryId) const
{
	QHash<INDUSTRYID, AuBdPref>::const_iterator pi = m_Prefs.find(industryId);
	if(pi != m_Prefs.cend())
		return *pi;
	return AuBdPref();
}

void AuBdPrefs::SetPreferredBlueprints(INDUSTRYID industryId, const AuBdPref& pref)
{
	m_Prefs.remove(industryId);
	m_Prefs.insert(industryId, pref);
}

bool AuBdPrefs::CanMakePirateBases(const QList<INDUSTRYID>& baseBuildings) const
{
	// Make a list of unique industry ids that are needed.
	QSet<INDUSTRYID> needed;
	for(QList<INDUSTRYID>::const_iterator bi = baseBuildings.cbegin(); bi != baseBuildings.cend(); bi++)
		needed.insert(*bi);

	// Insure the preferences have blueprints for the industries that are needed, in all environments.
	for(QSet<INDUSTRYID>::const_iterator ni = needed.cbegin(); ni != needed.cend(); ni++)
	{
		QHash<INDUSTRYID, AuBdPref>::const_iterator pi = m_Prefs.find(*ni);
		if(pi == m_Prefs.cend())
			return false;	// No pref for the industry.
		if(!(*pi).HasAllEnvironments())
			return false;	// Pref does not have blueprints for every environment.
	}
	return true;
}
