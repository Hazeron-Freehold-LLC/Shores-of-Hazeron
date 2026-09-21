//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubddesign.cpp,v 1.110 2022/04/12 17:09:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubddesign.h"
//
#include "aubdreport.h"
#include "aucombat.h"
#include "auglobal.h"
#include "auitem.h"
#include "auscparts.h"
#include "hashpack.h"
#include <QProgressDialog>
#include <QSqlQuery>

#define BDDESIGN_SERIALIZE_VER			4

static int s_AuBdDesignMetaType = qRegisterMetaType<AuBdDesign>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuBdDesignHash::LoadArchitect(const AuId& idArchitect)
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Create a query to select all of the architect's designs, except those that the architect has deleted.
	QString sq = QString("SELECT id,name,archname,state,textures, "
			"parts,modules,structure,iid, "
			"arenas,armor,auditoriums,bays,capacitor,hold,homesLarge,homesMedium,homesSmall,lifeSupport,lounges,offices,"
			"powerPlant,sensors,shields,shops,skeleton,stores, "
			"hullVolume "
			"FROM n_BdDesign WHERE state != 2 and ida=%1;").arg(idArchitect.GetID());
	AuBdDesign ob;
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		ob.SetId(qry.value(qv++).toUInt());
		ob.SetName(qry.value(qv++).toString());
		ob.SetArchitectId(idArchitect);
		ob.SetArchitectName(qry.value(qv++).toString());
		ob.SetState((AuDesign::STATE)qry.value(qv++).toUInt());

		// Textures
		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			if(!ob.ReadTextures(in))
			{
				in.device()->seek(0);
				ob.ReadTexturesOld(in);
			}
			QImage th;
			in >> th;
			ob.SetThumbnail(th);
		}

		// Parts
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray bac = qry.value(qvi).toByteArray();
			QByteArray ba;
			if(!bac.isEmpty())
				ba = AuUncompress(bac, __FILE__, __LINE__,QString("%1").arg(ob.Id()));
			if(ba.isEmpty())
			{
				qDebug() << "AuBdDesignHash::LoadArchitect Z_DATA_ERROR handled ok." << ob.Id() << ob.Name() << ob.ArchitectId()
					<< ob.ArchitectName();
				ba = bac;
			}
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *ob.m_Parts;
		}

		// System Modules
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> ob.m_Modules;
		}

		ob.m_Structure = qry.value(qv++).toUInt();
		ob.m_IndustryId = qry.value(qv++).toUInt();

		// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
		ob.m_Arena = qry.value(qv++).toUInt();
		ob.m_Armor = qry.value(qv++).toUInt();
		ob.m_Auditorium = qry.value(qv++).toUInt();
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			ob.m_Bays.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				quint16 partid;
				quint32 hullvol;

				in >> partid;
				in >> hullvol;

				ob.m_Bays.insert(partid, hullvol);
			}
		}
		ob.m_Capacitor = qry.value(qv++).toUInt();
		ob.m_Hold = qry.value(qv++).toUInt();
		ob.m_HomesLarge = qry.value(qv++).toUInt();
		ob.m_HomesMedium = qry.value(qv++).toUInt();
		ob.m_HomesSmall = qry.value(qv++).toUInt();
		ob.m_LifeSupport = qry.value(qv++).toUInt();
		ob.m_Lounge = qry.value(qv++).toUInt();
		ob.m_Offices = qry.value(qv++).toUInt();
		ob.m_PowerPlant = qry.value(qv++).toUInt();
		ob.m_Sensors = qry.value(qv++).toUInt();
		ob.m_Shields = qry.value(qv++).toUInt();
		ob.m_Shops = qry.value(qv++).toUInt();
		ob.m_Skeleton = qry.value(qv++).toUInt();
		ob.m_Store = qry.value(qv++).toUInt();

		// Factors gleaned by design analysis
		ob.m_HullVolume = qry.value(qv++).toUInt();
		//qDebug() << "AuBdDesignHash::LoadArchitect arch" << idArchitect << "loaded" << ob.Name() << ob.DescriptionOneLine();

		// Insert/update the data in this hash.
		iterator di = find(ob.Id());
		if(di == end())
			insert(pair<quint32, AuBdDesign>(ob.Id(), ob));
		else
			(*di).second = ob;
	}
}

void AuBdDesignHash::LoadSoloInit()
{
	clear();
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Create a query to select all of Haxus' designs that are final.
	AuId idArchitect(AuId::Haxus());
	QString sq = QString("SELECT id,name,archname,state,textures, "
			"parts,modules,structure,iid, "
			"arenas,armor,auditoriums,bays,capacitor,hold,homesLarge,homesMedium,homesSmall,lifeSupport,lounges,offices,"
			"powerPlant,sensors,shields,shops,skeleton,stores, "
			"hullVolume "
			"FROM n_BdDesign WHERE state = 1 and ida=%1;").arg(idArchitect.GetID());
	AuBdDesign ob;
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		ob.SetId(qry.value(qv++).toUInt());
		ob.SetName(qry.value(qv++).toString());
		ob.SetArchitectId(idArchitect);
		ob.SetArchitectName(qry.value(qv++).toString());
		ob.SetState((AuDesign::STATE)qry.value(qv++).toUInt());

		// Textures
		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			if(!ob.ReadTextures(in))
			{
				in.device()->seek(0);
				ob.ReadTexturesOld(in);
			}
			QImage th;
			in >> th;
			ob.SetThumbnail(th);
		}

		// Parts
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray bac = qry.value(qvi).toByteArray();
			QByteArray ba;
			if(!bac.isEmpty())
				ba = AuUncompress(bac, __FILE__, __LINE__,QString("%1").arg(ob.Id()));
			if(ba.isEmpty())
			{
				qDebug() << "AuBdDesignHash::LoadSoloInit Z_DATA_ERROR handled ok." << ob.Id() << ob.Name() << ob.ArchitectId()
					<< ob.ArchitectName();
				ba = bac;
			}
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *ob.m_Parts;
		}

		// System Modules
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> ob.m_Modules;
		}

		ob.m_Structure = qry.value(qv++).toUInt();
		ob.m_IndustryId = qry.value(qv++).toUInt();

		// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
		ob.m_Arena = qry.value(qv++).toUInt();
		ob.m_Armor = qry.value(qv++).toUInt();
		ob.m_Auditorium = qry.value(qv++).toUInt();
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			ob.m_Bays.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				quint16 partid;
				quint32 hullvol;

				in >> partid;
				in >> hullvol;

				ob.m_Bays.insert(partid, hullvol);
			}
		}
		ob.m_Capacitor = qry.value(qv++).toUInt();
		ob.m_Hold = qry.value(qv++).toUInt();
		ob.m_HomesLarge = qry.value(qv++).toUInt();
		ob.m_HomesMedium = qry.value(qv++).toUInt();
		ob.m_HomesSmall = qry.value(qv++).toUInt();
		ob.m_LifeSupport = qry.value(qv++).toUInt();
		ob.m_Lounge = qry.value(qv++).toUInt();
		ob.m_Offices = qry.value(qv++).toUInt();
		ob.m_PowerPlant = qry.value(qv++).toUInt();
		ob.m_Sensors = qry.value(qv++).toUInt();
		ob.m_Shields = qry.value(qv++).toUInt();
		ob.m_Shops = qry.value(qv++).toUInt();
		ob.m_Skeleton = qry.value(qv++).toUInt();
		ob.m_Store = qry.value(qv++).toUInt();

		// Factors gleaned by design analysis
		ob.m_HullVolume = qry.value(qv++).toUInt();
		//qDebug() << "AuBdDesignHash::LoadSoloInit arch" << idArchitect << "loaded" << ob.Name() << ob.DescriptionOneLine();

		// Insert/update the data in this hash.
		iterator di = find(ob.Id());
		if(di == end())
			insert(pair<quint32, AuBdDesign>(ob.Id(), ob));
		else
			(*di).second = ob;
	}
}

void AuBdDesignHash::Load(quint32 idDesign)
{
	AuBdDesign ob;
	ob.Load(idDesign);
	if(ob.Id() != idDesign)
		return;

	// Insert/update the data in this hash.
	iterator di = find(ob.Id());
	if(di == end())
		insert(pair<quint32, AuBdDesign>(ob.Id(), ob));
	else
		(*di).second = ob;
}

void AuBdDesignHash::operator<<(QDataStream& in)
{
	clear();
	quint16 cnt;
	in >> cnt;
	AuBdDesign ship;
	for(; cnt; cnt--)
	{
		ship << in;
		insert(pair<quint32, AuBdDesign>(ship.Id(), ship));
	}
}

void AuBdDesignHash::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	out << (quint16)size();
	for(const_iterator it = begin(); it != end(); it++)
		(*it).second >> out;
}

quint64 AuBdDesignHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuBdDesignHash);
	for(const_iterator it = cbegin(); it != cend(); it++)
		mem += sizeof(quint32) + (*it).second.MemoryUsage();
	return mem;
}

quint32 AuBdDesignHash::HighestId() const
{
	quint32 highest = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).first > highest)
			highest = (*it).first;
	return highest;
}

quint32 AuBdDesignHash::SoloInitNextId() const
{
	if(empty())
		return 1;

	// Prepare to search for a large gap in the existing ids. 10,000 should do it. That way when designs are added to the GS file,
	// they can be appended without causing a conflict.
	QMap<quint32,bool> sortedIds;
	for(const_iterator it = cbegin(); it != cend(); it++)
		sortedIds.insert((*it).first, true);

	// Starting at 1, look for the first gap of at least 10,000 unused ids.
	quint32 count = 10000;
	QMap<quint32, bool>::const_iterator si = sortedIds.cbegin();
	quint32 baseId = si.key() + 1;
	si++;
	for(; si != sortedIds.cend(); si++)
	{
		if(si.key() > baseId + count)
			break;
		baseId = si.key() + 1;	// Move up to the id after the map entry.
	}
	return baseId;
}

void AuBdDesignHash::Merge(const AuBdDesignHash& mdh)
{
	// Add designs that are not already here.
	for(const_iterator it = mdh.cbegin(); it != mdh.cend(); it++)
	{
		if(find((*it).first) != end())
			continue;	// I already have that design.
		insert(pair<quint32, AuBdDesign>((*it).first, (*it).second));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdDesign::AuBdDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect) :
	AuDesign(idDesign, idArchitect, architect)
{
	m_HullVolume = 0;
	m_Parts = new AuScParts;
	m_Structure = BD_Building;
	m_IndustryId = IID_None;

	m_Arena = 0;
	m_Armor = 0;
	m_Auditorium = 0;
	m_Capacitor = 0;
	m_Hold = 0;
	m_HomesLarge = 0;
	m_HomesMedium = 0;
	m_HomesSmall = 0;
	m_LifeSupport = 0;
	m_Lounge = 0;
	m_Offices = 0;
	m_PowerPlant = 0;
	m_Sensors = 0;
	m_Shields = 0;
	m_Shops = 0;
	m_Skeleton = 0;
	m_Store = 0;

	// Build a reasonable default palette.
	quint16 texLibPal[SCTEXI_Count];
	AuTextureLibItem::InitDefaultBdPalette(texLibPal);
	AuDesignTex texturePalette[SCTEXI_Count];
	for(int ti=0; ti<SCTEXI_Count; ti++)
		texturePalette[ti].SetTexLibId(texLibPal[ti]);
	SetTexturePalette(texturePalette);
}

AuBdDesign::AuBdDesign()
{
	m_HullVolume = 0;
	m_Parts = new AuScParts;
	m_Structure = BD_Building;
	m_IndustryId = IID_None;

	m_Arena = 0;
	m_Armor = 0;
	m_Auditorium = 0;
	m_Capacitor = 0;
	m_Hold = 0;
	m_HomesLarge = 0;
	m_HomesMedium = 0;
	m_HomesSmall = 0;
	m_LifeSupport = 0;
	m_Lounge = 0;
	m_Offices = 0;
	m_PowerPlant = 0;
	m_Sensors = 0;
	m_Shields = 0;
	m_Shops = 0;
	m_Skeleton = 0;
	m_Store = 0;

	// Build a reasonable default palette.
	quint16 texLibPal[SCTEXI_Count];
	AuTextureLibItem::InitDefaultBdPalette(texLibPal);
	AuDesignTex texturePalette[SCTEXI_Count];
	for(int ti=0; ti<SCTEXI_Count; ti++)
		texturePalette[ti].SetTexLibId(texLibPal[ti]);
	SetTexturePalette(texturePalette);
}

AuBdDesign::~AuBdDesign()
{
	delete m_Parts;
}

AuBdDesign::AuBdDesign(const AuBdDesign& c) : AuDesign(c), m_Modules(c.m_Modules), m_Bays(c.m_Bays)
{
	m_HullVolume = c.m_HullVolume;
	m_Parts = new AuScParts(*c.m_Parts);
	m_Structure = c.m_Structure;
	m_IndustryId = c.m_IndustryId;

	m_Arena = c.m_Arena;
	m_Armor = c.m_Armor;
	m_Auditorium = c.m_Auditorium;
	m_Capacitor = c.m_Capacitor;
	m_Hold = c.m_Hold;
	m_HomesLarge = c.m_HomesLarge;
	m_HomesMedium = c.m_HomesMedium;
	m_HomesSmall = c.m_HomesSmall;
	m_LifeSupport = c.m_LifeSupport;
	m_Lounge = c.m_Lounge;
	m_Offices = c.m_Offices;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensors = c.m_Sensors;
	m_Shields = c.m_Shields;
	m_Shops = c.m_Shops;
	m_Skeleton = c.m_Skeleton;
	m_Store = c.m_Store;
}

AuBdDesign& AuBdDesign::operator=(const AuBdDesign& c)
{
	AuDesign::operator=(c);

	m_HullVolume = c.m_HullVolume;
	m_Modules = c.m_Modules;
	*m_Parts = *c.m_Parts;
	m_Structure = c.m_Structure;
	m_IndustryId = c.m_IndustryId;

	m_Arena = c.m_Arena;
	m_Armor = c.m_Armor;
	m_Auditorium = c.m_Auditorium;
	m_Bays = c.m_Bays;
	m_Capacitor = c.m_Capacitor;
	m_Hold = c.m_Hold;
	m_HomesLarge = c.m_HomesLarge;
	m_HomesMedium = c.m_HomesMedium;
	m_HomesSmall = c.m_HomesSmall;
	m_LifeSupport = c.m_LifeSupport;
	m_Lounge = c.m_Lounge;
	m_Offices = c.m_Offices;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensors = c.m_Sensors;
	m_Shields = c.m_Shields;
	m_Shops = c.m_Shops;
	m_Skeleton = c.m_Skeleton;
	m_Store = c.m_Store;
	return *this;
}

AuDesign* AuBdDesign::Clone() const
{
	return new AuBdDesign(*this);
}

void AuBdDesign::Load(quint32 idDesign)
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	QString sq = QString("SELECT name,ida,archname,state,textures, "
			"parts,modules,structure,iid, "
			"arenas,armor,auditoriums,bays,capacitor,hold,homesLarge,homesMedium,homesSmall,lifeSupport,lounges,offices,"
			"powerPlant,sensors,shields,shops,skeleton,stores, "
			"hullVolume "
			"FROM n_BdDesign WHERE id=%1;").arg(idDesign);
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	if(qry.next())
	{
		int qv=0;
		SetId(idDesign);
		SetName(qry.value(qv++).toString());
		SetArchitectId(AuId(qry.value(qv++).toUInt()));
		SetArchitectName(qry.value(qv++).toString());
		SetState((AuDesign::STATE)qry.value(qv++).toUInt());

		// Textures
		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			if(!ReadTextures(in))
			{
				in.device()->seek(0);
				ReadTexturesOld(in);
			}
			QImage th;
			in >> th;
			SetThumbnail(th);
		}

		// Parts
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray bac = qry.value(qvi).toByteArray();
			QByteArray ba;
			if(!bac.isEmpty())
				ba = AuUncompress(bac, __FILE__, __LINE__,QString("%1").arg(Id()));
			if(ba.isEmpty())
			{
				qDebug() << "AuBdDesign::Load Z_DATA_ERROR handled ok." << Id() << Name() << ArchitectId()
					<< ArchitectName();
				ba = bac;
			}
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *m_Parts;
		}

		// System Modules
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> m_Modules;
		}

		m_Structure = qry.value(qv++).toUInt();
		m_IndustryId = qry.value(qv++).toUInt();

		// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
		m_Arena = qry.value(qv++).toUInt();
		m_Armor = qry.value(qv++).toUInt();
		m_Auditorium = qry.value(qv++).toUInt();
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			m_Bays.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				quint16 partid;
				quint32 hullvol;

				in >> partid;
				in >> hullvol;

				m_Bays.insert(partid, hullvol);
			}
		}
		m_Capacitor = qry.value(qv++).toUInt();
		m_Hold = qry.value(qv++).toUInt();
		m_HomesLarge = qry.value(qv++).toUInt();
		m_HomesMedium = qry.value(qv++).toUInt();
		m_HomesSmall = qry.value(qv++).toUInt();
		m_LifeSupport = qry.value(qv++).toUInt();
		m_Lounge = qry.value(qv++).toUInt();
		m_Offices = qry.value(qv++).toUInt();
		m_PowerPlant = qry.value(qv++).toUInt();
		m_Sensors = qry.value(qv++).toUInt();
		m_Shields = qry.value(qv++).toUInt();
		m_Shops = qry.value(qv++).toUInt();
		m_Skeleton = qry.value(qv++).toUInt();
		m_Store = qry.value(qv++).toUInt();

		// Factors gleaned by design analysis
		m_HullVolume = qry.value(qv++).toUInt();
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
			qWarning() << "AuBdDesign::Load FAILED to SELECT design" << idDesign;
	}
}

const AuScParts& AuBdDesign::Parts() const
{
	return *m_Parts;
}

AuScParts& AuBdDesign::Parts()
{
	return *m_Parts;
}

void AuBdDesign::ClearParts()
{
	m_Parts->Clear();
	m_HullVolume = 0;
}

void AuBdDesign::DesignLoaded(const AuDesign& design)
{
	AuDesign::DesignLoaded(design);
	try
	{
		const AuBdDesign& c = dynamic_cast<const AuBdDesign&>(design);
		m_HullVolume = c.m_HullVolume;
		m_Modules = c.m_Modules;
		*m_Parts = *c.m_Parts;
		m_Structure = c.m_Structure;
		m_IndustryId = c.m_IndustryId;

		m_Arena = c.m_Arena;
		m_Armor = c.m_Armor;
		m_Auditorium = c.m_Auditorium;
		m_Bays = c.m_Bays;
		m_Capacitor = c.m_Capacitor;
		m_Hold = c.m_Hold;
		m_HomesLarge = c.m_HomesLarge;
		m_HomesMedium = c.m_HomesMedium;
		m_HomesSmall = c.m_HomesSmall;
		m_LifeSupport = c.m_LifeSupport;
		m_Lounge = c.m_Lounge;
		m_Offices = c.m_Offices;
		m_PowerPlant = c.m_PowerPlant;
		m_Sensors = c.m_Sensors;
		m_Shields = c.m_Shields;
		m_Shops = c.m_Shops;
		m_Skeleton = c.m_Skeleton;
		m_Store = c.m_Store;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuBdDesign::operator>>(QDataStream& out) const
{
	AuDesign::operator>>(out);

	quint8 ver = BDDESIGN_SERIALIZE_VER;
	out << ver;
	out << m_HullVolume;
	out << m_Modules;
	out << *m_Parts;
	out << m_Structure;
	out << m_IndustryId;
	out << m_Arena;
	out << m_Armor;
	out << m_Auditorium;
	out << (quint16)m_Bays.size();
	for(QHash<quint16,quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		out << bi.key();
		out << bi.value();
	}
	out << m_Capacitor;
	out << m_Hold;
	out << m_HomesLarge;
	out << m_HomesMedium;
	out << m_HomesSmall;
	out << m_LifeSupport;
	out << m_Lounge;
	out << m_Offices;
	out << m_PowerPlant;
	out << m_Sensors;
	out << m_Shields;
	out << m_Shops;
	out << m_Skeleton;
	out << m_Store;
}

void AuBdDesign::operator<<(QDataStream& in)
{
	AuDesign::operator<<(in);

	quint8 ver;
	in >> ver;
	in >> m_HullVolume;
	in >> m_Modules;
	in >> *m_Parts;
	in >> m_Structure;
	if(ver >= 3)
		in >> m_IndustryId;
	in >> m_Arena;
	in >> m_Armor;
	in >> m_Auditorium;

	quint16 c;
	m_Bays.clear();
	in >> c;
	while(c--)
	{
		quint16 partid;
		quint32 hullvol;

		in >> partid;
		in >> hullvol;

		m_Bays.insert(partid, hullvol);
	}
	in >> m_Capacitor;
	if(ver < 2)
	{
		quint32 fuel;
		in >> fuel;
		in >> m_Hold;
		m_Hold += fuel;
	}
	else
		in >> m_Hold;
	in >> m_HomesLarge;
	in >> m_HomesMedium;
	in >> m_HomesSmall;
	in >> m_LifeSupport;
	in >> m_Lounge;
	in >> m_Offices;
	in >> m_PowerPlant;
	in >> m_Sensors;
	in >> m_Shields;
	in >> m_Shops;
	if(ver >= 4)
		in >> m_Skeleton;
	else
		m_Skeleton = 0;
	in >> m_Store;
}

void AuBdDesign::Insert(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO n_BdDesign (id,name,ida,archname,state,textures, "
			"parts,modules,structure,iid, "
			"arenas,armor,auditoriums,bays,capacitor,hold,homesLarge,homesMedium,homesSmall,lifeSupport,lounges,offices,"
			"powerPlant,sensors,shields,shops,skeleton,stores,"
			"hullVolume"
			") VALUES ("
			"?,?,?,?,?,?, "
			"?,?,?,?, "
			"?,?,?,?,?,?,?,?,?,?,?,?, "
			"?,?,?,?,?,?, "
			"?);");
	qa.prepare(sq);
	qa.addBindValue(Id());
	qa.addBindValue(Name());
	qa.addBindValue(ArchitectId().GetID());
	qa.addBindValue(ArchitectName());
	qa.addBindValue(State());

	// Textures
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);

		WriteTextures(out);
		out << Thumbnail();

		qa.addBindValue(ba);
	}

	// Parts
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << *m_Parts;
		QByteArray bac = qCompress(ba);
		qa.addBindValue(bac);
	}

	// System Modules
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << m_Modules;
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_Structure);
	qa.addBindValue(m_IndustryId);

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	qa.addBindValue((qint32)m_Arena);
	qa.addBindValue((qint32)m_Armor);
	qa.addBindValue((qint32)m_Auditorium);
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << (quint16)m_Bays.size();
		for(QHash<quint16,quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		{
			out << bi.key();
			out << bi.value();
		}
		qa.addBindValue(ba);
	}
	qa.addBindValue((qint32)m_Capacitor);
	qa.addBindValue((qint32)m_Hold);
	qa.addBindValue((qint32)m_HomesLarge);
	qa.addBindValue((qint32)m_HomesMedium);
	qa.addBindValue((qint32)m_HomesSmall);
	qa.addBindValue((qint32)m_LifeSupport);
	qa.addBindValue((qint32)m_Lounge);
	qa.addBindValue((qint32)m_Offices);
	qa.addBindValue((qint32)m_PowerPlant);
	qa.addBindValue((qint32)m_Sensors);
	qa.addBindValue((qint32)m_Shields);
	qa.addBindValue((qint32)m_Shops);
	qa.addBindValue((qint32)m_Skeleton);
	qa.addBindValue((qint32)m_Store);

	// Factors gleaned by design analysis
	qa.addBindValue((qint32)m_HullVolume);

	if(!qa.exec())
		qWarning("ERROR! AuBdDesign::Insert %s FAILED '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1());
}

void AuBdDesign::Update(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("UPDATE n_BdDesign SET name=?, ida=?, archname=?, state=?, textures=?, "
			"parts=?, modules=?, structure=?, iid=?, "
			"arenas=?, armor=?, auditoriums=?, bays=?, capacitor=?, hold=?, homesLarge=?, homesMedium=?, homesSmall=?, "
			"lifeSupport=?, lounges=?, offices=?, powerPlant=?, sensors=?, shields=?, shops=?, skeleton=?, stores=?, hullVolume=? "
			"WHERE id=%1;")
		.arg(Id());
	qa.prepare(sq);
	qa.addBindValue(Name());
	qa.addBindValue(ArchitectId().GetID());
	qa.addBindValue(ArchitectName());
	qa.addBindValue(State());

	// Textures
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);

		WriteTextures(out);
		out << Thumbnail();

		qa.addBindValue(ba);
	}

	// Parts
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << *m_Parts;
		QByteArray bac = qCompress(ba);
		qa.addBindValue(bac);
	}

	// System Modules
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << m_Modules;
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_Structure);
	qa.addBindValue(m_IndustryId);

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	qa.addBindValue((qint32)m_Arena);
	qa.addBindValue((qint32)m_Armor);
	qa.addBindValue((qint32)m_Auditorium);
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << (quint16)m_Bays.size();
		for(QHash<quint16,quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		{
			out << bi.key();
			out << bi.value();
		}
		qa.addBindValue(ba);
	}
	qa.addBindValue((qint32)m_Capacitor);
	qa.addBindValue((qint32)m_Hold);
	qa.addBindValue((qint32)m_HomesLarge);
	qa.addBindValue((qint32)m_HomesMedium);
	qa.addBindValue((qint32)m_HomesSmall);
	qa.addBindValue((qint32)m_LifeSupport);
	qa.addBindValue((qint32)m_Lounge);
	qa.addBindValue((qint32)m_Offices);
	qa.addBindValue((qint32)m_PowerPlant);
	qa.addBindValue((qint32)m_Sensors);
	qa.addBindValue((qint32)m_Shields);
	qa.addBindValue((qint32)m_Shops);
	qa.addBindValue((qint32)m_Skeleton);
	qa.addBindValue((qint32)m_Store);

	// Factors gleaned by design analysis
	qa.addBindValue((qint32)m_HullVolume);

	if(!qa.exec())
		qWarning("ERROR! AuBdDesign::Update %s FAILED '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1());
}

void AuBdDesign::Tx(QDataStream& out) const
{
	AuDesign::Tx(out);

	quint8 ver = BDDESIGN_SERIALIZE_VER;
	out << ver;
	out << m_HullVolume;
	out << m_Modules;
	{
		// Store parts using a compressed byte array.
		QByteArray ba;
		QDataStream packout(&ba, QIODevice::WriteOnly);
		packout.setVersion(QDataStream::Qt_5_0);
		packout << *m_Parts;
		QByteArray bac = qCompress(ba);
		out << bac;
	}
	out << m_Structure;
	out << m_IndustryId;
	out << m_Arena;
	out << m_Armor;
	out << m_Auditorium;

	out << (quint16)m_Bays.size();
	for(QHash<quint16,quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		out << bi.key();
		out << bi.value();
	}
	out << m_Capacitor;
	out << m_Hold;
	out << m_HomesLarge;
	out << m_HomesMedium;
	out << m_HomesSmall;
	out << m_LifeSupport;
	out << m_Lounge;
	out << m_Offices;
	out << m_PowerPlant;
	out << m_Sensors;
	out << m_Shields;
	out << m_Shops;
	out << m_Skeleton;
	out << m_Store;
}

void AuBdDesign::Rx(QDataStream& in)
{
	AuDesign::Rx(in);

	quint8 ver;
	in >> ver;
	in >> m_HullVolume;
	in >> m_Modules;
	if(ver >= 1)
	{
		// Read parts from compressed byte array.
		QByteArray bac;
		in >> bac;
		QByteArray ba;
		if(!bac.isEmpty())
			ba = AuUncompress(bac, __FILE__, __LINE__,QString("%1").arg(Id()));
		QDataStream packin(ba);
		packin.setVersion(QDataStream::Qt_5_0);
		packin >> *m_Parts;
	}
	else
		in >> *m_Parts;
	in >> m_Structure;
	if(ver >= 3)
		in >> m_IndustryId;
	in >> m_Arena;
	in >> m_Armor;
	in >> m_Auditorium;

	quint16 c;
	m_Bays.clear();
	in >> c;
	while(c--)
	{
		quint16 partid;
		quint32 hullvol;

		in >> partid;
		in >> hullvol;

		m_Bays.insert(partid, hullvol);
	}
	in >> m_Capacitor;
	if(ver < 2)
	{
		float fuel;
		in >> fuel;
		in >> m_Hold;
		m_Hold += fuel;
	}
	else
		in >> m_Hold;
	in >> m_HomesLarge;
	in >> m_HomesMedium;
	in >> m_HomesSmall;
	in >> m_LifeSupport;
	in >> m_Lounge;
	in >> m_Offices;
	in >> m_PowerPlant;
	in >> m_Sensors;
	in >> m_Shields;
	in >> m_Shops;
	if(ver >= 4)
		in >> m_Skeleton;
	else
		m_Skeleton = 0;
	in >> m_Store;
}

void AuBdDesign::SetArenaVolume(quint32 arena)
{
	m_Arena = arena;
}

void AuBdDesign::SetAuditoriumVolume(quint32 auditorium)
{
	m_Auditorium = auditorium;
}

void AuBdDesign::SetLoungeVolume(quint32 lounge)
{
	m_Lounge = lounge;
}

void AuBdDesign::SetStoreVolume(quint32 store)
{
	m_Store = store;
}

void AuBdDesign::SetArmorVolume(quint32 armor)
{
	m_Armor = armor;
}

void AuBdDesign::SetStructure(BUILDING_STRUCTURE structure)
{
	m_Structure = structure;
}

void AuBdDesign::SetIndustryId(INDUSTRYID iid)
{
	m_IndustryId = iid;
}

bool AuBdDesign::Validate()
{
	//qDebug() << "AuBdDesign::Validate";
	bool isValid = AuDesign::Validate();

	//DHH Validate parts.
	//AuScParts	m_Parts;

	//DHH Validate modules.
	//AuBdModules	m_Modules;

	return isValid;
}

QString AuBdDesign::MaterialName() const
{
	return g_Au.CommodityName(HullModuleMaterial(HullModule()));
}

// Returns the description as a single line.
QString AuBdDesign::DescriptionOneLine() const
{
	QString cubedSym;
	if(g_Au.IsWebCGI())
		cubedSym = QString("³");
	else
		cubedSym = g_Au.CubedSymbol();

	return QObject::tr("%1 %2 %L3m%4")
		.arg(MaterialName())
		.arg(StructureName())
		.arg(HullVolume())
		.arg(cubedSym);
}

void AuBdDesign::SetBayVolume(quint16 which, quint32 hullVol)
{
	m_Bays.insert(which, hullVol);
}

void AuBdDesign::SetCapacitorVolume(quint32 hullVol)
{
	m_Capacitor = hullVol;
}

void AuBdDesign::SetHoldVolume(quint32 hullVol)
{
	m_Hold = hullVol;
}

void AuBdDesign::SetSkeletonVolume(quint32 vol)
{
	m_Skeleton = vol;
}

void AuBdDesign::SetLifeSupportVolume(quint32 hullVol)
{
	m_LifeSupport = hullVol;
}

void AuBdDesign::SetModules(const AuBdModules& modules)
{
	m_Modules = modules;
}

void AuBdDesign::SetPowerPlantVolume(quint32 hullVol)
{
	m_PowerPlant = hullVol;
}

void AuBdDesign::SetHomeLargeVolume(quint32 homesVol)
{
	m_HomesLarge = homesVol;
}

void AuBdDesign::SetHomeMediumVolume(quint32 homesVol)
{
	m_HomesMedium = homesVol;
}

void AuBdDesign::SetHomeSmallVolume(quint32 homesVol)
{
	m_HomesSmall = homesVol;
}

void AuBdDesign::SetOfficeVolume(quint32 officeVol)
{
	m_Offices = officeVol;
}

void AuBdDesign::SetShopVolume(quint32 shopVol)
{
	m_Shops = shopVol;
}

void AuBdDesign::SetSensorVolume(quint32 hullVol)
{
	m_Sensors = hullVol;
}

void AuBdDesign::SetShieldVolume(quint32 hullVol)
{
	m_Shields = hullVol;
}

quint16 AuBdDesign::PartAdd(const AuScPart& part, AuScParts& partsAdded)
{
	quint16 partId = AuDesign::PartAdd(part, partsAdded);
	if(!partId)
		return 0;
	m_HullVolume = 0;
	return partId;
}

bool AuBdDesign::PartObjectAdd(quint16 partId, const AuScPart& part, AuScParts& partsChanged)
{
	if(!AuDesign::PartObjectAdd(partId, part, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsCopy(const AuScSelection& partIds, AuScParts& partsAdded)
{
	if(!AuDesign::PartsCopy(partIds, partsAdded))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsCopySymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane, AuScParts& partsAdded)
{
	if(!AuDesign::PartsCopySymmetrical(partIds, gridPlane, partsAdded))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsTransform(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged, bool objectMode)
{
	if(!AuDesign::PartsTransform(partIds, t, partsChanged, objectMode))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsTransformLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged)
{
	if(!AuDesign::PartsTransformLiftStops(partIds, t, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsTransformCopy(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged)
{
	if(!AuDesign::PartsTransformCopy(partIds, t, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartDelete(quint16 partId, AuScParts& partsDeleted)
{
	if(!AuDesign::PartDelete(partId, partsDeleted))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartObjectDelete(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted)
{
	if(!AuDesign::PartObjectDelete(partId, selInfo, partsChanged, partsDeleted))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartUpdate(quint16 partId, const AuScPart& part, AuScParts& partsUpdated)
{
	if(!AuDesign::PartUpdate(partId, part, partsUpdated))
		return false;
	m_HullVolume = 0;
	return true;
}

void AuBdDesign::UndoRedoAdd(const AuScParts& parts)
{
	AuDesign::UndoRedoAdd(parts);
	m_HullVolume = 0;
}

void AuBdDesign::UndoRedoDelete(const QList<quint16>& partIds)
{
	AuDesign::UndoRedoDelete(partIds);
	m_HullVolume = 0;
}

void AuBdDesign::UndoRedoSwap(AuScParts& swapParts)
{
	AuDesign::UndoRedoSwap(swapParts);
	m_HullVolume = 0;
}

bool AuBdDesign::PartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	if(!m_Parts->FillFace(sel, boundary, gridPlane, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartFillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, const AuPoint& v4,
		AuScParts& partsChanged)
{
	if(!m_Parts->FillQuad(sel, v1, v2, v3, v4, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartFillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
		AuScParts& partsChanged)
{
	if(!m_Parts->FillTriangle(sel, v1, v2, v3, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartSetOmitted(quint16 partId, bool omit, AuScParts& partsChanged)
{
	if(!m_Parts->SetOmitted(partId, omit, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartSetSealed(quint16 partId, bool seal, AuScParts& partsChanged)
{
	if(!m_Parts->SetSealed(partId, seal, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartDeleteState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	if(!m_Parts->DeleteState(partId, state, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartAddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged)
{
	if(!m_Parts->AddFaces(partId, faces, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeNavLightLens(const AuScSelection& sel, const QColor& color, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeNavLightLens(sel, color, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeDecal(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeDoor(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

/*
bool AuBdDesign::PartsMakeHeadlightLens(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeHeadlightLens(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}
*/

bool AuBdDesign::PartsMakeLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeLightLens(sel, lightColor, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeRoom(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeRoom(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeHull(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeMesh(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeMesh(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeLandingGear(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeRotatingBeaconLens(const AuScSelection& sel, const QColor& lightColor, const AuPoint& rotationAxis,
		AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeRotatingBeaconLens(sel, lightColor, rotationAxis, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeStrobeLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeStrobeLens(sel, lightColor, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeTurboLift(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeTurboLift(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeTurret(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeTurret(sel, basePivot, baseAxis, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuBdDesign::PartsMakeWindow(const AuScSelection& sel, float opacity, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeWindow(sel, opacity, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

void AuBdDesign::ShaderTexturePalette(COMMODITYID hullModule, AuGLEffects& re, AuTexturePalette& textures) const
{
	// Hull plates. Grab this one first in case the custom texture is bad. Then we'll use this one as the fall back.
	GLuint hullPlate;
	switch(hullModule)
	{
	default:
	case CID_WoodHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullWood());
		break;
	case CID_LumberHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullLumber());
		break;
	case CID_StoneHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullStone());
		break;
	case CID_MetalHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullMetal());
		break;
	case CID_MagmiumHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullMagmium());
		break;
	case CID_VulciumHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullVulcium());
		break;
	case CID_AdamantineHullModule:
		hullPlate = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullAdamantine());
		break;
	}

	// Fill the textures array.
	for(int ti=0; ti<SCTEXI_Count; ti++)
	{
		if(!TexturePaletteImage(ti).isNull())
		{
			// Use a custom texture image.
			QString sid = QString("ScC%1").arg(TexturePaletteImage(ti).cacheKey());
			textures[ti] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TexturePaletteImage(ti), GL_LINEAR, GL_LINEAR, true);
		}
		else
		{
			// Use one of our palette textures.
			if(ti == SCTEXI_Material)
				textures[ti] = hullPlate;
			else
				textures[ti] = AuTextureLibItem::LibraryTexture(re, TexturePaletteId(ti));
		}
	}
}

void AuBdDesign::SetProperties(const AuDesign& design)
{
	try
	{
		const AuBdDesign& c = dynamic_cast<const AuBdDesign&>(design);

		m_HullVolume = c.m_HullVolume;
		m_Modules = c.m_Modules;
		//m_Parts = c.m_Parts;
		m_Structure = c.m_Structure;
		m_IndustryId = c.m_IndustryId;
		m_Arena = c.m_Arena;
		m_Armor = c.m_Armor;
		m_Auditorium = c.m_Auditorium;
		m_Bays = c.m_Bays;
		m_Capacitor = c.m_Capacitor;
		m_Hold = c.m_Hold;
		m_HomesLarge = c.m_HomesLarge;
		m_HomesMedium = c.m_HomesMedium;
		m_HomesSmall = c.m_HomesSmall;
		m_LifeSupport = c.m_LifeSupport;
		m_Lounge = c.m_Lounge;
		m_Offices = c.m_Offices;
		m_PowerPlant = c.m_PowerPlant;
		m_Sensors = c.m_Sensors;
		m_Shields = c.m_Shields;
		m_Shops = c.m_Shops;
		m_Skeleton = c.m_Skeleton;
		m_Store = c.m_Store;
		AuDesign::SetProperties(design);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

bool AuBdDesign::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
		const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const
{
	return m_Parts->IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal, doorStates, AuLandingGearStateHash(), liftStops);
}

void AuBdDesign::CalcHullVolume()
{
	//qDebug() << "AuBdDesign::CalcHullVolume" << AuTime::Now();
	m_HullVolume = m_Parts->HullVolume();
	//qDebug() << "AuBdDesign::CalcHullVolume result" << m_HullVolume << AuTime::Now();
}

void AuBdDesign::RefreshObstructionsCache()
{
	m_Parts->RefreshObstructionsCache();
	CalcHullVolume();
	SyncBays();
}

void AuBdDesign::RefreshObstructionsCache(QProgressDialog& progress)
{
	int partProg = m_Parts->size();
	int lastProg = partProg * 2 + 1;
	progress.setMaximum(lastProg);

	progress.setLabelText(QObject::tr("Updating obstructions..."));
	m_Parts->RefreshObstructionsCache(progress);
	if(progress.wasCanceled())
		return;
	progress.setValue(partProg);

	progress.setLabelText(QObject::tr("Calculating hull volume..."));
	m_HullVolume = m_Parts->HullVolume(progress);
	if(progress.wasCanceled())
		return;
	progress.setValue(partProg*2);

	progress.setLabelText(QObject::tr("Syncing equipment bays..."));
	SyncBays();
	progress.setValue(partProg*2 + 1);
}

bool AuBdDesign::RefreshObstructionsCacheIfNeeded()
{
	bool needed = m_Parts->RefreshObstructionsCacheIfNeeded();
	CalcHullVolume();
	SyncBays();
	return needed;
}

void AuBdDesign::SyncBays()
{
	// Insure we have entries for bays and ftl drives added and that we don't have leftovers from those removed.
	AuScPartSet bays, ftlDrives;
	m_Parts->BaysAndFTLDrives(bays, ftlDrives);	// Get ids of the bays and ftl drives in the design.

	// Add bays we are missing.
	for(QSet<quint16>::const_iterator bi = bays.cbegin(); bi != bays.cend(); bi++)
		if(!m_Bays.contains(*bi))
			m_Bays.insert(*bi, 0);

	// Remove bays that don't belong there.
	QHash<quint16, quint32>::iterator bi = m_Bays.begin();
	while(bi != m_Bays.end())
		if(bays.contains(bi.key()))
			bi++;
		else
			bi = m_Bays.erase(bi);
}

void AuBdDesign::Bays(AuScPartSet& partIds) const
{
	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		partIds.insert(bi.key());
}

quint32 AuBdDesign::BayVolume() const
{
	quint32 vol = 0;
	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		vol += *bi;
	return vol;
}

quint32 AuBdDesign::BayVolume(quint16 which) const
{
	QHash<quint16, quint32>::const_iterator bi = m_Bays.find(which);
	if(bi == m_Bays.cend())
		return 0;
	return *bi;
}

double AuBdDesign::BayVolumeCuFt() const
{
	double vol = 0;
	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		vol += *bi;
	return vol * CubicFeetPerCubicMeter();
}

double AuBdDesign::BayVolumeCuFt(quint16 which) const
{
	QHash<quint16, quint32>::const_iterator bi = m_Bays.find(which);
	if(bi == m_Bays.cend())
		return 0;
	return *bi * CubicFeetPerCubicMeter();
}

void AuBdDesign::ReportDesign(AuBdReport& report) const
{
	// Design
	{
		report.m_ArchitectId = ArchitectId();
		report.m_ArchitectName = ArchitectName();
		report.m_DesignDesc = Description();
		report.m_DesignId = Id();
		report.m_DesignName = Name();

		// List architects of parts that are not the design architect.
		m_Parts->PartArchitects(ArchitectId(), report.m_PartArchitects);
	}

	// Structure
	{
		report.m_Structure = m_Structure;
		report.m_IndustryId = m_IndustryId;
	}

	// Volumes, in cubic meters.
	{
		report.m_Hull = AuScReportSysVol(HullModule(), 1, HullVolume());
		m_Parts->HullExtents(report.m_HullMin, report.m_HullMax);
		m_Parts->SiteExtents(report.m_SiteMin, report.m_SiteMax);

		report.m_ArenaVolume = ArenaVolume();
		report.m_ArmorVolume = ArmorVolume();
		report.m_AuditoriumVolume = AuditoriumVolume();

		for(QHash<quint16, quint32>::const_iterator fi = m_Bays.cbegin(); fi != m_Bays.cend(); fi++)
		{
			AuScParts::const_iterator pi = m_Parts->find(fi.key());
			if(pi  == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			report.m_Bays.insert(pi.key(), AuScReportSysVol((*pi)->Module(), 1, *fi));
		}

		report.m_Capacitor = AuScReportSysVol(CapacitorModule(), 1, CapacitorVolume());
		report.m_FieldArea = (quint32)(FieldArea() * SquareMetersPerSquareFoot());
		report.m_HoldVolume = HoldVolume();
		report.m_HomeLargeVolume = HomeLargeVolume();
		report.m_HomeMediumVolume = HomeMediumVolume();
		report.m_HomeSmallVolume = HomeSmallVolume();
		report.m_LifeSupport = AuScReportSysVol(LifeSupportModule(), 1, LifeSupportVolume());
		report.m_LoungeVolume = LoungeVolume();
		report.m_OfficeVolume = OfficeVolume();
		report.m_PowerPlant = AuScReportSysVol(PowerPlantModule(), 1, PowerPlantVolume());
		report.m_Sensor = AuScReportSysVol(SensorModule(), 1, SensorVolume());
		report.m_Shield = AuScReportSysVol(ShieldModule(), 1, ShieldVolume());
		report.m_ShopVolume = ShopVolume();
		report.m_SkeletonVolume = SkeletonVolume();
		report.m_StoreVolume = StoreVolume();

		AuScPartSet turrets;	// key is part id
		Turrets(turrets);
		for(QSet<quint16>::const_iterator ti = turrets.cbegin(); ti != turrets.cend(); ti++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*ti);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			quint16 roomId = AuDesign::RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);

			COMMODITYID turretMod = TurretModule(*ti);
			report.m_Turrets.insert(*ti, AuScReportTurret(turretMod, 1, roomId));
		}
	}

	// Quality
	{
		report.m_Quality = 1;
		report.m_ConProc.InitBd(*this);
		if(State() != Final)
			report.m_ConProc.ClearDesignId();	// Prevent the thing from being manufactured.
	}

	// Stations and equipment.
	{
		// Capt Chairs
		AuScPartSet captChairs;
		CommandStations(captChairs);
		for(QSet<quint16>::const_iterator si = captChairs.cbegin(); si != captChairs.cend(); si++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*si);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			quint16 roomId = AuDesign::RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);

			COMMODITYID mod = (*pi)->Module();
			report.m_CommandStations.insert(pi.key(), AuScReportStation(mod, 1, (*pi)->Type(), (*pi)->ReportDesc(),
						roomId, (*pi)->WhichEffect()));
		}

		// Crew Stations
		AuScPartSet stations;
		ControlStations(stations);
		for(QSet<quint16>::const_iterator si = stations.cbegin(); si != stations.cend(); si++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*si);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			quint16 roomId = AuDesign::RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);

			COMMODITYID mod = (*pi)->Module();
			if(BdModuleSystem(mod) != SYS_Transporter)
				report.m_CrewStations.insert(pi.key(), AuScReportStation(mod, 1, (*pi)->Type(), (*pi)->ReportDesc(),
							roomId, (*pi)->WhichEffect()));
			else
			{
				AuPointArray padLocs;
				(*pi)->PadLocs(padLocs);
				report.m_Transporters.insert(pi.key(),
						AuScReportTransporterStation(mod, 1, (*pi)->Type(), (*pi)->ReportDesc(), padLocs.size(), roomId,
							(*pi)->WhichEffect()));
			}
		}

		StatusDisplays(report.m_StatusDisplays);

		TurboLifts(report.m_Lifts);

		GameTables(report.m_GameTables);
		CitizenPosts(report.m_CitizenPosts);
		GuardPosts(report.m_GuardPosts);
		LivestockPosts(report.m_LivestockPosts);

		AuScPartSet launchers;
		VehicleLaunchers(launchers);
		for(QSet<quint16>::const_iterator si = launchers.cbegin(); si != launchers.cend(); si++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*si);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;
			report.m_VehicleLaunchers.insert(pi.key());
		}

		AuScPartSet vls;
		VehicleLandingSystems(vls);
		for(QSet<quint16>::const_iterator si = vls.cbegin(); si != vls.cend(); si++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*si);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;
			report.m_VehicleLandingSystems.insert(pi.key());
		}
	}

	// Berths
	report.m_Beds = Beds();

	// Parking
	{
		report.m_ParkGround = GroundVehicleParkingSpots();
		report.m_ParkRocket = RocketParkingSpots();
		report.m_ParkSpace = SpaceVehicleParkingSpots();
		report.m_ParkSpacecraft = SpacecraftParkingSpots();
		report.m_ParkWater = WaterVehicleParkingSpots();
	}

	// Amenities
	report.m_HasBar = m_Parts->HasBar();
	report.m_HasEnvironmentVoid = m_Parts->HasEnvironmentVoid();
	report.m_HasPantry = m_Parts->HasPantry();

	// Calculate values that are based on system volumes.
	report.UpdateCalculatedValues();

	// Assemblies report their parts list.
	if(State() != Final)
		report.m_PartsList = m_Parts->ReportParts(ArchitectId());

	// Faces/triangles.
	report.m_FaceCount = m_Parts->CountFaces();
	report.m_TriangleCount = m_Parts->CountTriangles();
}

QString AuBdDesign::ReportAnalysis() const
{
	AuBdReport report;
	ReportDesign(report);
	return report.ReportAnalysis(State());
}

double AuBdDesign::Mass() const
{
	return HullMass() + EquipmentMass();
}

double AuBdDesign::HullMass() const
{
	return HullDensity(ArmorLevel()) * HullVolumeCuFt();
}

double AuBdDesign::EquipmentMass() const
{
	double mass = 0;

	// These values were taken from the old n_DetailType table, which expressed them in kg per unit.
	// e.g. A maneuver drive unit was 2250 kg. All units were 256 cubic feet.
	if(PowerPlantVolume())
	{
		double mu = (800 * PoundsPerKilogram()) / 256.;
		mass += mu * PowerPlantVolumeCuFt();
	}

	if(CapacitorVolume())
	{
		double mu = (800 * PoundsPerKilogram()) / 256.;
		mass += mu * CapacitorVolumeCuFt();
	}

	quint32 bay = BayVolumeCuFt();
	if(bay)
	{
		double mu = (1800 * PoundsPerKilogram()) / 256.;
		mass += mu * bay;
	}

	if(SensorVolume())
	{
		double mu = (450 * PoundsPerKilogram()) / 256.;
		mass += mu * SensorVolumeCuFt();
	}

	if(ShieldVolume())
	{
		double mu = (450 * PoundsPerKilogram()) / 256.;
		mass += mu * ShieldVolumeCuFt();
	}

	return mass;
}

quint32 AuBdDesign::HoldCapacity() const
{
	// Returns the number of commodity units that can fit in the hold.
	if(!HoldVolume())
		return 0;						// No hold space at all.

	// Calculate this the way the old ship designs did it, based on cubic feet.
	quint32 holdVol = (quint32)HoldVolumeCuFt();
	if(holdVol < 3)
		return 1;
	quint32 hold = holdVol / 3;
	return hold;
}

//static
double AuBdDesign::HullSizeLimitHeight(BUILDING_STRUCTURE structure)
{
	// Burj Khalifa is 828m to the tip.
	switch(structure)
	{
	case BD_Invalid:
	case BD_Count:
		break;
	case BD_Biodome:
	case BD_SeaPlatform:
	case BD_AerialPlatform:
	case BD_GroundPlatform:
		return (1200. * FeetPerMeter());
	case BD_Bridge:
	case BD_Building:
	case BD_RoadEnclosure:
	case BD_SeaSurfaceBuilding:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_Tunnel:
	case BD_StreetLight:
		return (1000. * FeetPerMeter());
	case BD_Fence:
		return (100. * FeetPerMeter());
	}
	return 1;
}

double AuBdDesign::HullSizeLimitDepth(BUILDING_STRUCTURE structure)
{
	switch(structure)
	{
	case BD_Invalid:
	case BD_Count:
		break;
	case BD_Biodome:
	case BD_AerialPlatform:
	case BD_GroundPlatform:
	case BD_SeaPlatform:
		return (200. * FeetPerMeter());
	case BD_Bridge:
	case BD_Building:
	case BD_RoadEnclosure:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_Tunnel:
		return (100. * FeetPerMeter());
	case BD_SeaSurfaceBuilding:
		return (400. * FeetPerMeter());
	case BD_Fence:
	case BD_StreetLight:
		return (10. * FeetPerMeter());
	}
	return 1;
}

double AuBdDesign::HullSizeLimitBase(BUILDING_STRUCTURE structure)
{
	switch(structure)
	{
	case BD_Invalid:
	case BD_Count:
		break;
	case BD_Biodome:
	case BD_SeaPlatform:
	case BD_AerialPlatform:
	case BD_GroundPlatform:
		return (1000. * FeetPerMeter()) / 2;
	case BD_Bridge:
	case BD_Building:
	case BD_Fence:
	case BD_RoadEnclosure:
	case BD_SeaSurfaceBuilding:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_StreetLight:
	case BD_Tunnel:
		return (500. * FeetPerMeter()) / 2;
	}
	return 1;
}

double AuBdDesign::SiteSizeLimitHeight(BUILDING_STRUCTURE structure)
{
	switch(structure)
	{
	case BD_Invalid:
	case BD_Count:
		break;
	case BD_Biodome:
	case BD_SeaPlatform:
	case BD_AerialPlatform:
	case BD_GroundPlatform:
	case BD_Bridge:
	case BD_Building:
	case BD_Fence:
	case BD_RoadEnclosure:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_StreetLight:
	case BD_Tunnel:
		return (50. * FeetPerMeter());
	case BD_SeaSurfaceBuilding:
		return (1. * FeetPerMeter());
	}
	return 1;
}

double AuBdDesign::SiteSizeLimitDepth(BUILDING_STRUCTURE structure)
{
	switch(structure)
	{
	case BD_Invalid:
	case BD_Count:
		break;
	case BD_Biodome:
	case BD_SeaPlatform:
	case BD_AerialPlatform:
	case BD_GroundPlatform:
	case BD_Bridge:
	case BD_Building:
	case BD_Fence:
	case BD_RoadEnclosure:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_StreetLight:
	case BD_Tunnel:
		return (50. * FeetPerMeter());
	case BD_SeaSurfaceBuilding:
		return (1. * FeetPerMeter());
	}
	return 1;
}

double AuBdDesign::SiteSizeLimitBase(BUILDING_STRUCTURE structure)
{
	switch(structure)
	{
	case BD_Invalid:
	case BD_Count:
		break;
	case BD_Biodome:
	case BD_SeaPlatform:
	case BD_AerialPlatform:
	case BD_GroundPlatform:
		return (1020. * FeetPerMeter()) / 2;
	case BD_Bridge:
	case BD_Building:
	case BD_RoadEnclosure:
	case BD_SeaSurfaceBuilding:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_StreetLight:
	case BD_Tunnel:
		return (510. * FeetPerMeter()) / 2;
	case BD_Fence:
		return 200. * FeetPerMeter();
	}
	return 1;
}

// Returns shield power use rate in units per real world second.
double AuBdDesign::ShieldPowerRate() const
{
	// Each shield unit uses one power units per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = ShieldVolumeCuFt() * rUnitRate;
	return rUseRate;
}

// Returns sensor power use rate in units per real world second.
double AuBdDesign::SensorPowerRate() const
{
	// Each sensor uses one power units per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = SensorVolumeCuFt() * rUnitRate;
	return rUseRate;
}

double AuBdDesign::SensorRange(quint8 sensorQ) const
{
	if(SensorVolume() <= 0)
		return 0;

	// A sensor unit is 256 cubic feet.
	const double tlBoost = QualityBoostF(sensorQ);
	return SensorVolumeCuFt()/256. * FeetPerMoonOrbit() / 2. * tlBoost;
}

double AuBdDesign::CapacitorMax(quint8 capacitorQ) const
{
	// These values were taken from the old DqDesignAnalysis, which expressed everything in system units.
	// All of the old system units were 256 cubic feet.
	const double tlBoost = QualityBoostF(capacitorQ);
	double cap = CapacitorVolumeCuFt() / 256. * tlBoost;
	// Buildings need at least two units of capacitor to import and use power for lights.
	if(cap < 2)
		cap = 2;
	return cap;
}

//static
double AuBdDesign::HullDensityFactor(double armorLevel)
{
	// Armor increases hull mass a lot.
	// The base density is linearly modified by armorLevel.
	return (armorLevel + 1);
}

quint16 AuBdDesign::RoomVoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RoomVoidContainingLoc(loc, doorStates, AuLandingGearStateHash(), liftStops);
}

quint16 AuBdDesign::RoomVoidContainingLoc(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RoomVoidContainingLoc(loc, elevate, doorStates, AuLandingGearStateHash(), liftStops);
}

bool AuBdDesign::IsInsideObstructionModel(const AuPoint& loc, const AuDoorStateHash& doorStates) const
{
	return m_Parts->IsInsideObstructionModel(loc, doorStates, AuLandingGearStateHash());
}

bool AuBdDesign::RandomRoomLocation(quint16 roomId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RandomRoomLocation(roomId, voidLoc, doorStates, AuLandingGearStateHash(), liftStops);
}

bool AuBdDesign::RandomRoomLocation(const QString& roomName, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RandomRoomLocation(roomName, voidLoc, doorStates, AuLandingGearStateHash(), liftStops);
}

quint16 AuBdDesign::RandomRoomVoidLocation(AuPoint& voidLoc, quint8 personnelAccess, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RandomRoomVoidLocation(voidLoc, personnelAccess, doorStates, AuLandingGearStateHash(), liftStops);
}

quint16 AuBdDesign::RandomBuildZoneLocation(AuPoint& zoneLoc) const
{
	return m_Parts->RandomBuildZoneLocation(zoneLoc);
}

quint16 AuBdDesign::RandomEnvironmentVoidLocation(AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RandomEnvironmentVoidLocation(voidLoc, doorStates, AuLandingGearStateHash(), liftStops);
}

quint16 AuBdDesign::RandomSiteLocation(AuPoint& siteLoc, const AuDoorStateHash& doorStates) const
{
	return m_Parts->RandomSiteLocation(siteLoc, doorStates, AuLandingGearStateHash());
}

bool AuBdDesign::IsInsideHull(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->IsInsideHull(loc, doorStates, AuLandingGearStateHash(), liftStops);
}

bool AuBdDesign::FinalizeAssembly(QStringList& msgs)
{
	// Finalized designs are getting prepared to be new entries in the library. That means an id of 0.
	SetId(0);
	SetState(FinalAssembly);

	// Design report checks to see if the design is ok.
	AuBdReport report;
	ReportDesign(report);
	return report.OkForAssembly(msgs, *this);
}

bool AuBdDesign::FinalizeDesign(QStringList& msgs, QProgressDialog& progress)
{
	// Finalized designs are getting prepared to be new entries in the library. That means an id of 0.
	SetId(0);
	SetState(Final);

	int partProg = m_Parts->size();
	int lastProg = partProg * 3 + 3;
	progress.setMaximum(lastProg);

	// Remove parts and part states that are not needed in the finalized design.
	progress.setLabelText(QObject::tr("Finalizing parts..."));
	m_Parts->Finalize(false, &progress);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg);	// Done with first pass through parts.

	// Create obstruction model.
	progress.setLabelText(QObject::tr("Updating obstructions..."));
	m_Parts->RefreshObstructionsCacheForFinalize(progress);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*2);	// Done with second pass through parts.

	progress.setLabelText(QObject::tr("Calculating building volume..."));
	m_HullVolume = m_Parts->HullVolume(progress);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3);	// Done with third pass through parts.

	progress.setLabelText(QObject::tr("Syncing equipment bays..."));
	SyncBays();
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3 + 1);

	// Design report checks to see if the design is ok.
	progress.setLabelText(QObject::tr("Analyzing building blueprint..."));
	AuBdReport report;
	ReportDesign(report);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3 + 2);

	bool isOk = report.OkToConstruct(msgs, this);
	progress.setValue(lastProg);	// Done.
	return isOk;
}

void AuBdDesign::FinalizePreview()
{
	// Remove parts and part states that are not needed in the finalized design.
	// Applies hull surface effect textures, for damage and FTL operation.
	m_Parts->Finalize(false, 0);

	// Create obstruction model.
	RefreshObstructionsCacheIfNeeded();
}

// NOTE: If you change this function be sure to change AuBdReport::ListComponents.
void AuBdDesign::ListComponents(DeProcessComponentArray& pc) const
{
	// Ascribe parts based on hull volumes.
	if(Structure() == BD_AerialPlatform)
	{
		quint32 gravCouplings = HullVolume() / 1000;	// 1 grav coupling per 1000 cubic meters of hull
		if(!gravCouplings)
			gravCouplings = 1;
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * gravCouplings);
		pc.AddRequirement(CID_GravCoupling, CE_Required, 1 * gravCouplings);
		pc.AddRequirement(CID_Metal, CE_Required, 3 * gravCouplings);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * gravCouplings);
		pc.AddRequirement(CID_Plastic, CE_Required, 1 * gravCouplings);
	}

	// Life Support
	int lifeSupportUnits = (int)ceil(m_LifeSupport * CubicFeetPerCubicMeter() / 256.f);
	if(lifeSupportUnits)
	{
		//pc.AddRequirement(CID_Air, CE_Required, 10 * lifeSupportUnits);
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_Metal, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_Plastic, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_Textiles, CE_Required, 1 * lifeSupportUnits);
		if(m_Modules.LifeSupportModule() == CID_LifeSupportGravityModule)
			pc.AddRequirement(CID_GravCoupling, CE_Required, 1 * lifeSupportUnits);
	}

	int powerPlantUnits = (int)ceil(m_PowerPlant * CubicFeetPerCubicMeter() / 256.f);
	if(powerPlantUnits)
	{
		if(PowerPlantModule() == CID_NuclearPowerPlantModule ||
				PowerPlantModule() == CID_SolarPowerPlantModule)
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * powerPlantUnits);
		pc.AddRequirement(CID_Metal, CE_Required, 1 * powerPlantUnits);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * powerPlantUnits);
		if(PowerPlantModule() == CID_NuclearPowerPlantModule)
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * powerPlantUnits);
	}

	int capacitorUnits = (int)ceil(m_Capacitor * CubicFeetPerCubicMeter() / 256.f);
	if(capacitorUnits)
	{
		if(PowerPlantModule() == CID_NuclearPowerPlantModule ||
				PowerPlantModule() == CID_SolarPowerPlantModule)
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * capacitorUnits);
		pc.AddRequirement(CID_Metal, CE_Required, 2 * capacitorUnits);
	}

	int sensorUnits = (int)ceil(m_Sensors * CubicFeetPerCubicMeter() / 256.f);
	if(sensorUnits)
	{
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 3 * sensorUnits);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1 * sensorUnits);
		pc.AddRequirement(CID_Plastic, CE_Required, 1 * sensorUnits);
	}

	int shieldUnits = (int)ceil(m_Shields * CubicFeetPerCubicMeter() / 256.f);
	if(shieldUnits)
	{
		COMMODITYID shieldMod = ShieldModule();
		if(shieldMod == CID_KineticShieldModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 5 * shieldUnits);
			pc.AddRequirement(CID_IoTube, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_MagmiumHeatSink, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * shieldUnits);
		}
		else if(shieldMod == CID_CloakingShieldModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 5 * shieldUnits);
			pc.AddRequirement(CID_IoTube, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_TesloidDissipator, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_VulciumHeatSink, CE_Required, 1 * shieldUnits);
		}
		else	// CID_EnergyShieldModule
		{
			pc.AddRequirement(CID_CryoHeatSink, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 5 * shieldUnits);
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * shieldUnits);
		}
	}

	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		int bayHits = (int)ceil(*bi * CubicFeetPerCubicMeter() / 256.f);
		COMMODITYID bmod = BayModule(bi.key());
		if(bmod == CID_AtmosphereCondenserBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_ProspectorBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_SiphonBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_WildcatterBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_ParticleCollectorBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_TesloidDissipator, CE_Required, 1 * bayHits);
		}
		else if(bmod == CID_TractorBeamBayModule)
		{
			pc.AddRequirement(CID_CryoHeatSink, CE_Required, 1 * bayHits);
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_IoTube, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else	// CID_WeaponBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * bayHits);
			pc.AddRequirement(CID_HeavyWeapon, CE_Required, 1 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
	}

	// Process is built using CID_Lumber for easy change later. Nothing else uses CID_Lumber as a component so it won't overlap
	// any other components.
	m_Parts->ListComponents(pc, SLINE_None, ArmorLevel(), CID_Lumber);
}

bool AuBdDesign::PartsUpdateHullDoors(const AuScSelection& partIds, AuScParts& partsChanged)
{
	if(!AuDesign::PartsUpdateHullDoors(partIds, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

quint16 AuBdDesign::VoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops, quint16 exceptVoidId) const
{
	return m_Parts->VoidContainingLoc(loc, doorStates, AuLandingGearStateHash(), liftStops, exceptVoidId);
}

bool AuBdDesign::Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, bool canUnlockDoors,
		const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const
{
	return m_Parts->Route(route, fromLoc, toLoc, LORI_Belly, canUnlockDoors, doorStates, AuLandingGearStateHash(), liftStops);
}

bool AuBdDesign::ClosestEntrance(AuPoint& entranceLoc, quint16& doorId, quint16& ladderId, const AuPoint& loc,
		bool useGroundLevelDoors, bool hullDoorsUsable, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const
{
	return m_Parts->ClosestEntrance(entranceLoc, doorId, ladderId, loc, LORI_Belly, useGroundLevelDoors, hullDoorsUsable,
			doorStates, AuLandingGearStateHash(), liftStops);
}

bool AuBdDesign::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	return m_Parts->Sight(eyeLoc, sightLoc, sightPart, doorStates, AuLandingGearStateHash(), liftStops, sightSurfaceNormal);
}

bool AuBdDesign::SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart) const
{
	return m_Parts->SightBuildZone(eyeLoc, sightLoc, sightPart);
}

double AuBdDesign::HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, AuPoint& floorN, bool& inGravityWell,
		const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const
{
	inGravityWell = false;
	double floorHt = 0;
	quint16 roomId = VoidContainingLoc(loc, doorStates, liftStops, 0);
	if(roomId)
	{
		// We found a room void at 'loc'. Follow gravity down to the floor, through multiple room voids if necessary.
		AuPoint floc(loc);		// loc from which we are falling. It will walk downward to the lowest floor.
		quint16 proomId = 0;	// previous lower room id, to prevent oscillation.

		// We will only try a few times so we don't get stuck forever.
		for(int tries=0; tries<10; tries++)
		{
			// Locate the room that contains the fall loc.
			AuScParts::const_iterator ri = m_Parts->find(roomId);
			if(ri == m_Parts->cend())
				break;	// wut?

			// First room determine if the loc is in a gravity well.
			if(tries == 0 && (*ri)->GravityType() == G_Zero)
				inGravityWell = true;

			// How far can floc fall in the room?
			const AuMatrix4x4& stopM = (*ri)->StopMatrixAt(ri.key(), liftStops);
			double roomFloorHt = (*ri)->HeightAboveFloor(floc, gravityN, stopM, floorN);

			// Walk downward until we find no other room voids.
			floc -= (*ri)->GravityUp(gravityN, stopM) * roomFloorHt;		// Fall to the floor in 'roomId'.

			// Account for barriers between loc and the floor loc.
			m_Parts->BlockSegment(loc, floc, floorN);

			// Look for a room void at the floor loc, besides 'roomId'.
			quint16 froomId = VoidContainingLoc(floc, doorStates, liftStops, roomId);
			if(!froomId || froomId == proomId)
				break;			// We didn't move into a different room or we oscillated back to the previous room due to grav.

			// Found another room at the floor loc besides roomId.
			proomId = roomId;
			roomId = froomId;
		}
		floorHt = floc.Distance(loc);
	}
	return floorHt;
}

double AuBdDesign::HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	double ceilingHt = 0;
	quint16 roomId = VoidContainingLoc(loc, doorStates, liftStops, 0);
	if(roomId)
	{
		// We found a room void at 'loc'. Follow gravity up to the ceiling, through multiple room voids if necessary.
		AuPoint floc(loc);		// loc from which we are climbing. It will walk upward to the highest ceiling.
		quint16 proomId = 0;	// previous upper room id, to prevent oscillation.

		// We will only try a few times so we don't get stuck forever.
		for(int tries=0; tries<10; tries++)
		{
			// Locate the room that contains the climb loc.
			AuScParts::const_iterator ri = m_Parts->find(roomId);
			if(ri == m_Parts->end())
				break;	// wut?

			// How far can floc climb in the room?
			const AuMatrix4x4& stopM = (*ri)->StopMatrixAt(ri.key(), liftStops);
			double roomCeilingHt = (*ri)->HeightOfCeiling(floc, gravityN, stopM);

			// Walk upward until we find no other room voids.
			floc += (*ri)->GravityUp(gravityN, stopM) * roomCeilingHt;		// Climb to the ceiling in 'roomId'.

			// Look for a room void at the ceiling loc, besides 'roomId'.
			quint16 froomId = VoidContainingLoc(floc, doorStates, liftStops, roomId);
			if(!froomId || froomId == proomId)
				break;			// We oscillated back to the previous room.

			// Found another room at the ceiling loc besides roomId.
			proomId = roomId;
			roomId = froomId;
		}

		ceilingHt = floc.Distance(loc);
	}
	return ceilingHt;
}

bool AuBdDesign::PathLocInsideDoor(quint16 doorId, AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	// Look for a path segment that is associated with the open state of the door.
	for(AuScParts::const_iterator pi = m_Parts->cbegin(); pi != m_Parts->cend(); pi++)
	{
		if(!(*pi)->IsPath())
			continue;
		if((*pi)->AssociatedPartId() != doorId)
			continue;
		if((*pi)->AssociatedPartState() != AuScDoor::Open)
			continue;
		const AuScPathSeg& seg = (*pi)->PathSegment(0);
		loc = seg.Loc1();
		//if(IsInsideHull(pathLoc, doorStates, AuLandingGearStateHash(), liftStops))
		if(m_Parts->RoomVoidContainingLoc(loc, .1, doorStates, AuLandingGearStateHash(), liftStops))
			return true;
		loc = seg.Loc2();
		//if(IsInsideHull(pathLoc, doorStates, AuLandingGearStateHash(), liftStops))
		if(m_Parts->RoomVoidContainingLoc(loc, .1, doorStates, AuLandingGearStateHash(), liftStops))
			return true;
	}
	return false;
}

//static
double AuBdDesign::MaxVolume()
{
	return 255 * 255 * 1600;	// Same as spacecraft design at Q255.
}

bool AuBdDesign::FindLadderToGrab(const AuPoint& fromLoc, AuMatrix4x4& mLadder, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->FindLadderToGrab(fromLoc, mLadder, doorStates, AuLandingGearStateHash(), liftStops);
}

bool AuBdDesign::FindLadderToGrabAt(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder,
		const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const
{
	return m_Parts->FindLadderToGrabAt(fromLoc, ladderPathLoc, mLadder, doorStates, AuLandingGearStateHash(), liftStops);
}

double AuBdDesign::GroundHeightAboveCG() const
{
	return m_Parts->GroundHeightAboveCG(AuLandingGearStateHash(), LORI_Belly);
}

quint16 AuBdDesign::EnvironmentVoidContainingLocLive(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->EnvironmentVoidContainingLoc(loc, doorStates, AuLandingGearStateHash(), liftStops);
}

quint16 AuBdDesign::RoomVoidContainingLocLive(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RoomVoidContainingLoc(loc, doorStates, AuLandingGearStateHash(), liftStops);
}

quint16 AuBdDesign::RoomVoidContainingLocLive(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
		const AuLiftHash& liftStops) const
{
	return m_Parts->RoomVoidContainingLoc(loc, elevate, doorStates, AuLandingGearStateHash(), liftStops);
}

quint32 AuBdDesign::FieldArea() const
{
	return m_Parts->FieldArea();
}

//static
quint32 AuBdDesign::AreaFirstField()
{
	return 4000;
}

//static
quint32 AuBdDesign::VolumeFirstArena()
{
	return 5000;
}

//static
quint32 AuBdDesign::VolumeFirstAuditorium()
{
	// Cubic meters for first auditorium. 50ft x 100ft x 16ft = 80000cu ft = 2265 cu m.
	return 2265/2;
}

//static
quint32 AuBdDesign::VolumeFirstGenerator()
{
	// Cubic meters for first generator. 50ft x 100ft x 32ft = 160000cu ft = 4531 cu m.
	return 4531/2;
}

//static
quint32 AuBdDesign::VolumeFirstHomeSmall()
{
	// Cubic meters per small home for one single person.
	return 57;
}

//static
quint32 AuBdDesign::VolumeFirstHomeMedium()
{
	// Cubic meters per medium home for one single person. 500sq ft x 8 ft = 4000cu ft = 113.3 cu m.
	return 114;
}

//static
quint32 AuBdDesign::VolumeFirstHomeLarge()
{
	// Cubic meters per large home for one single person. 1000sq ft x 8 ft = 8000cu ft = 226.5 cu m.
	return 228;
}

//static
quint32 AuBdDesign::VolumeFirstLounge()
{
	// Cubic meters for first store. 75ft x 50ft x 12ft = 45000cu ft = 1274 cu m.
	return 1274/2;
}

//static
quint32 AuBdDesign::VolumeFirstOffice()
{
	// Cubic meters per office for one single person. 100sq ft x 12 ft = 800cu ft = 34.0 cu m.
	return 34;
}

//static
quint32 AuBdDesign::VolumeFirstShop()
{
	// Cubic meters per shop for one single person. 200sq ft x 16 ft = 3200cu ft = 90.6 cu m.
	return 91;
}

//static
quint32 AuBdDesign::VolumeFirstStore()
{
	// Cubic meters for first store. 50ft x 50ft x 16ft = 40000cu ft = 1133 cu m.
	return 1133/2;
}

//static
quint32 AuBdDesign::AreaAddField()
{
	return AreaFirstField();
}

//static
quint32 AuBdDesign::VolumeAddHomeSmall()
{
	return VolumeFirstHomeSmall();
}

//static
quint32 AuBdDesign::VolumeAddHomeMedium()
{
	return VolumeFirstHomeMedium();
}

//static
quint32 AuBdDesign::VolumeAddHomeLarge()
{
	return VolumeFirstHomeLarge();
}

//static
quint32 AuBdDesign::VolumeAddArena()
{
	// Cubic meters per football field. 189m x 156m x 48m = 1415232sq m (Roman Colosseum).
	return VolumeFirstArena();
}

//static
quint32 AuBdDesign::VolumeAddAuditorium()
{
	return VolumeFirstAuditorium();
}

//static
quint32 AuBdDesign::VolumeAddGenerator()
{
	return VolumeFirstGenerator();
}

//static
quint32 AuBdDesign::VolumeAddLounge()
{
	return VolumeFirstLounge();
}

//static
quint32 AuBdDesign::VolumeAddOffice()
{
	return VolumeFirstOffice();
}

//static
quint32 AuBdDesign::VolumeAddShop()
{
	return VolumeFirstShop();
}

//static
quint32 AuBdDesign::VolumeAddStore()
{
	return VolumeFirstStore();
}

float AuBdDesign::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const
{
	return m_Parts->BuildingSiteInfluence(loc, siteLoc, terBlend);
}

float AuBdDesign::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
		const AuZoneResource& res) const
{
	return m_Parts->BuildingSiteInfluence(loc, siteLoc, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub,
			orchardTree, terBlend, res);
}

bool AuBdDesign::InBuildingSite(const AuPoint& loc) const
{
	return m_Parts->InBuildingSite(loc);
}

bool AuBdDesign::InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const
{
	return m_Parts->InBuildingSite(loc, siteGround, hideGrass);
}

void AuBdDesign::BuildZoneCheckLocs(AuPointArray& locs) const
{
	m_Parts->BuildZoneCheckLocs(locs);
}

unsigned int AuBdDesign::CompressedSizeLimit() const
{
	return AuBdReport::CompressedSizeLimit();
}

quint64 AuBdDesign::MemoryUsage() const
{
	quint64 mem = AuDesign::MemoryUsage();
	mem += sizeof(m_HullVolume);
	mem += sizeof(m_Modules);
	mem += m_Parts->MemoryUsage();
	mem += sizeof(m_Structure);
	mem += sizeof(m_IndustryId);

	mem += sizeof(m_Arena);
	mem += sizeof(m_Armor);
	mem += sizeof(m_Auditorium);
	mem += (sizeof(quint16) + sizeof(quint32)) * m_Bays.capacity();
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_Hold);
	mem += sizeof(m_HomesLarge);
	mem += sizeof(m_HomesMedium);
	mem += sizeof(m_HomesSmall);
	mem += sizeof(m_LifeSupport);
	mem += sizeof(m_Lounge);
	mem += sizeof(m_Offices);
	mem += sizeof(m_PowerPlant);
	mem += sizeof(m_Sensors);
	mem += sizeof(m_Shields);
	mem += sizeof(m_Shops);
	mem += sizeof(m_Skeleton);
	mem += sizeof(m_Store);
	return mem;
}

bool AuBdDesign::IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, const AuDoorStateHash& doorStates) const
{
	return m_Parts->IsOnLadderTo(fromLoc, ladderPathLoc, doorStates, AuLandingGearStateHash());
}

void AuBdDesign::ListEnvironmentVoids(AuScParts& envVoids) const
{
	m_Parts->ListEnvironmentVoids(envVoids);
}

quint16 AuBdDesign::InBuildZone(const AuPoint& loc) const
{
	return m_Parts->InBuildZone(loc);
}

bool AuBdDesign::CreateFence(const AuSegmentArray& fenceSegs, int fenceStyle, quint32 designId, const QString& designName,
		const QString& designDesc, const AuId& archId, const QString& archName)
{
	SetArchitectId(archId);
	SetArchitectName(archName);
	SetDescription(designDesc);
	SetId(designId);
	SetIndustryId(IID_Fence);
	SetName(designName);
	SetState(Final);
	SetStructure(BD_Fence);

	// Create fence parts from the segments.
	for(AuSegmentArray::const_iterator fi = fenceSegs.cbegin(); fi != fenceSegs.cend(); fi++)
	{
		AuScFence f;
		f.MakeFence((AuScFence::STYLE)fenceStyle, designId, (*fi).m_fLoc1, (*fi).m_fLoc2);
		m_Parts->AddPart(f);
	}

	// Perform a limited version of Finalize.
	// Remove parts and part states that are not needed in the finalized design.
	m_Parts->Finalize(false, 0);
	m_HullVolume = (quint32)ceil(m_Parts->FenceVolume());
	//qDebug() << "AuBdDesign::CreateFence" << fenceStyle << "fenceSegs" << fenceSegs.size() << "hullVol" << m_HullVolume;

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	m_Arena = 0;
	m_Armor = 0;
	m_Auditorium = 0;
	m_Capacitor = 0;
	m_Hold = 0;
	m_HomesLarge = 0;
	m_HomesMedium = 0;
	m_HomesSmall = 0;
	m_LifeSupport = 0;
	m_Lounge = 0;
	m_Offices = 0;
	m_PowerPlant = 0;
	m_Sensors = 0;
	m_Shields = 0;
	m_Shops = 0;
	m_Store = 0;
	m_Skeleton = m_HullVolume;

	// We're good if we ended up with at least one valid fence segment.
	return !m_Parts->empty();
}
