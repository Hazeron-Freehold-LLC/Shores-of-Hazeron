//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpref.cpp,v 1.6 2022/02/19 18:20:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpref.h"
//

void AuScPref::operator>>(QDataStream& out) const
{
	out << (quint8)m_Blueprints.size();
	for(QList<quint32>::const_iterator bi = m_Blueprints.cbegin(); bi != m_Blueprints.cend(); bi++)
		out << *bi;
}

void AuScPref::operator<<(QDataStream& in)
{
	m_Blueprints.clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		quint32 blueprintId;
		in >> blueprintId;
		m_Blueprints.append(blueprintId);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScPrefs::operator>>(QDataStream& out) const
{
	out << (quint16)m_Prefs.size();
	for(QHash<SCROLE, AuScPref>::const_iterator pi = m_Prefs.cbegin(); pi != m_Prefs.cend(); pi++)
	{
		out << (quint8)pi.key();
		out << *pi;
	}
}

void AuScPrefs::operator<<(QDataStream& in)
{
	m_Prefs.clear();
	quint16 c;
	in >> c;
	while(c--)
	{
		quint8 role;
		AuScPref p;
		in >> role;
		in >> p;
		m_Prefs.insert((SCROLE)role, p);
	}
}

AuScPref AuScPrefs::PreferredBlueprints(SCROLE role) const
{
	QHash<SCROLE, AuScPref>::const_iterator pi = m_Prefs.find(role);
	if(pi != m_Prefs.cend())
		return *pi;
	return AuScPref();
}

void AuScPrefs::SetPreferredBlueprints(SCROLE role, const AuScPref& pref)
{
	m_Prefs.remove(role);
	m_Prefs.insert(role, pref);
}

bool AuScPrefs::CanMakePirateEncounters() const
{
	SCROLE needed[] =
	{
		SCROLE_Military,
	};
	int needcount = sizeof(needed) / sizeof(SCROLE);
	for(int ni=0; ni<needcount; ni++)
	{
		QHash<SCROLE, AuScPref>::const_iterator pi = m_Prefs.find(needed[ni]);
		if(pi == m_Prefs.cend())
			return false;					// No pref for the role.
		if((*pi).Blueprints().empty())
			return false;					// No blueprint for the pref? Can this happen?
	}
	return true;
}
