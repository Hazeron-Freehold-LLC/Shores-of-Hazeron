//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auknownpatents.cpp,v 1.12 2022/02/19 18:20:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auknownpatents.h"
//
#include "auglobal.h"
#include "dgdata.h"

AuKnownPatents::AuKnownPatents()
{
}

AuKnownPatents::~AuKnownPatents()
{
}

AuKnownPatents::AuKnownPatents(const AuKnownPatents& c) : m_Patents(c.m_Patents)
{
}

const AuKnownPatents& AuKnownPatents::operator=(const AuKnownPatents& c)
{
	m_Patents = c.m_Patents;
	return *this;
}

bool AuKnownPatents::operator==(const AuKnownPatents& c) const
{
	if(m_Patents != c.m_Patents)
		return false;
	return true;
}

void AuKnownPatents::operator>>(QDataStream& out) const
{
	quint16 c = m_Patents.size();
	out << c;
	for(QHash<COMMODITYID, quint8>::const_iterator pi = m_Patents.cbegin(); pi != m_Patents.cend(); pi++)
	{
		out << (quint16)pi.key();
		out << *pi;
	}
}

void AuKnownPatents::operator<<(QDataStream& in)
{
	m_Patents.clear();
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 p;
		quint8 q;
		in >> p;
		in >> q;
		if(DbCommodity::IsPatent((COMMODITYID)p))
			m_Patents.insert((COMMODITYID)p, q);
	}
}

bool AuKnownPatents::RecordPatent(COMMODITYID patent, quint8 quality)
{
	if(!DbCommodity::IsPatent(patent))
		return false;
	QHash<COMMODITYID, quint8>::iterator pi = m_Patents.find(patent);
	if(pi == m_Patents.end())
	{
		// Patent is not already known.
		m_Patents.insert(patent, quality);
		return true;
	}
	else if(quality > *pi)
	{
		// Patent is already known at a lower quality.
		*pi = quality;
		return true;
	}
	// Patent is already known at the same or higher quality.
	return false;
}

void AuKnownPatents::ListIndustryPatents(AuKnownPatents& patents, INDUSTRYID execIndustry) const
{
	const DgData& gd = g_Au.LockDataForRead();
	ListIndustryPatents(patents, execIndustry, gd);
	g_Au.UnlockData();
}

void AuKnownPatents::ListIndustryPatents(AuKnownPatents& patents, INDUSTRYID execIndustry, const DgData& gd) const
{
	for(QHash<COMMODITYID, quint8>::const_iterator pi = m_Patents.cbegin(); pi != m_Patents.cend(); pi++)
		if(gd.IsIndustryPatent(pi.key(), execIndustry))
			patents.RecordPatent(pi.key(), *pi);
}

QString AuKnownPatents::HtmlListContents() const
{
	// Alphabetize
	QMap<QString, COMMODITYID> patents;
	for(QHash<COMMODITYID, quint8>::const_iterator pi = m_Patents.cbegin(); pi != m_Patents.cend(); pi++)
		patents.insert(g_Au.CommodityName(pi.key()), pi.key());

	// Format
	QString cont = QObject::tr("<hr><img height=\"16\" src=\"qrc:data/Folder.png\"> Patents <b>%1</b>").arg(m_Patents.size());
	for(QMap<QString, COMMODITYID>::const_iterator ai = patents.cbegin(); ai != patents.cend(); ai++)
	{
		QHash<COMMODITYID, quint8>::const_iterator pi = m_Patents.find(*ai);
		cont += QString("<br>%3 <img height=\"16\" src=\"qrc:data/c_Patent.png\"> %1 Q%2")
			.arg(g_Au.CommodityName(pi.key()))
			.arg(*pi)
			.arg(g_Au.MiddleDotSymbol());
	}
	return cont;
}

bool AuKnownPatents::RemovePatent(COMMODITYID patent)
{
	QHash<COMMODITYID, quint8>::iterator pi = m_Patents.find(patent);
	if(pi == m_Patents.end())
		return false;
	m_Patents.erase(pi);
	return true;
}

quint64 AuKnownPatents::MemoryUsage() const
{
	quint64 mem = sizeof(AuKnownPatents);
	mem += (sizeof(COMMODITYID) + sizeof(quint8)) * m_Patents.capacity();
	return mem;
}

int AuKnownPatents::RecordPatents(const AuKnownPatents& o)
{
	int recorded = 0;
	for(QHash<COMMODITYID, quint8>::const_iterator pi = o.m_Patents.cbegin(); pi != o.m_Patents.cend(); pi++)
		if(RecordPatent(pi.key(), *pi))
			recorded++;
	return recorded;
}

bool AuKnownPatents::HasWarpTechnology() const
{
	return m_Patents.contains(CID_PatentMakingWarp1FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp2FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp3FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp4FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp5FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp6FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp7FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp8FTLDriveModule) ||
		m_Patents.contains(CID_PatentMakingWarp9FTLDriveModule);
}

unsigned int AuKnownPatents::WarpTechnology() const
{
	if(m_Patents.contains(CID_PatentMakingWarp9FTLDriveModule))
		return 9;
	if(m_Patents.contains(CID_PatentMakingWarp8FTLDriveModule))
		return 8;
	if(m_Patents.contains(CID_PatentMakingWarp7FTLDriveModule))
		return 7;
	if(m_Patents.contains(CID_PatentMakingWarp6FTLDriveModule))
		return 6;
	if(m_Patents.contains(CID_PatentMakingWarp5FTLDriveModule))
		return 5;
	if(m_Patents.contains(CID_PatentMakingWarp4FTLDriveModule))
		return 4;
	if(m_Patents.contains(CID_PatentMakingWarp3FTLDriveModule))
		return 3;
	if(m_Patents.contains(CID_PatentMakingWarp2FTLDriveModule))
		return 2;
	if(m_Patents.contains(CID_PatentMakingWarp1FTLDriveModule))
		return 1;
	return 0;
}
