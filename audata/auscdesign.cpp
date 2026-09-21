//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdesign.cpp,v 1.226 2022/04/12 17:09:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscdesign.h"
//
#include "aucombat.h"
#include "auglobal.h"
#include "auitem.h"
#include "auscparts.h"
#include "auscreport.h"
#include "hashpack.h"
#include <QProgressDialog>
#include <QSqlQuery>

#define SCDESIGN_SERIALIZE_VER			0x06					// At present, this must fit into a nibble.

static int s_AuScDesignMetaType = qRegisterMetaType<AuScDesign>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScDesignHash::LoadArchitect(const AuId& idArchitect)
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Create a query to select all of the architect's designs, except those that the architect has deleted.
	QString sq = QString("SELECT id,name,archname,state,textures, "
			"parts,modules,landingOrient,stationOrient,streamline,structure,techLevelMin, "
			"armor,bays,capacitor,cryoBerths,ftlDrives,fuel,hold,lifeSupport,maneuverDrive,powerPlant,sensors,shields,skeleton, "
			"hullVolume "
			"FROM n_ScDesign WHERE state != 2 and ida=%1;").arg(idArchitect.GetID());
	AuScDesign ob;
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
				qDebug() << "AuScDesignHash::LoadArchitect Z_DATA_ERROR handled ok." << ob.Id() << ob.Name() << ob.ArchitectId()
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

		ob.m_LandingOrient = qry.value(qv++).toUInt();
		ob.m_StationOrient = qry.value(qv++).toUInt();
		ob.m_Streamline = qry.value(qv++).toUInt();
		ob.m_Structure = qry.value(qv++).toUInt();
		ob.m_MinQuality = qry.value(qv++).toInt();
		if(!ob.m_MinQuality)
			ob.m_MinQuality = 1;

		// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
		ob.m_Armor = qry.value(qv++).toUInt();
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
		ob.m_CryoBerths = qry.value(qv++).toUInt();
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			ob.m_FTLDrives.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				quint16 partid;
				quint32 hullvol;

				in >> partid;
				in >> hullvol;

				ob.m_FTLDrives.insert(partid, hullvol);
			}
		}
		ob.m_Fuel = qry.value(qv++).toUInt();
		ob.m_Hold = qry.value(qv++).toUInt();
		ob.m_LifeSupport = qry.value(qv++).toUInt();
		ob.m_ManeuverDrive = qry.value(qv++).toUInt();
		ob.m_PowerPlant = qry.value(qv++).toUInt();
		ob.m_Sensors = qry.value(qv++).toUInt();
		ob.m_Shields = qry.value(qv++).toUInt();
		ob.m_Skeleton = qry.value(qv++).toUInt();

		// Factors gleaned by design analysis
		ob.m_HullVolume = qry.value(qv++).toUInt();
		//qDebug() << "AuScDesignHash::LoadArchitect arch" << idArchitect << "loaded" << ob.Name() << ob.DescriptionOneLine();

		// Insert/update the data in this hash.
		iterator di = find(ob.Id());
		if(di == end())
			insert(pair<quint32, AuScDesign>(ob.Id(), ob));
		else
			(*di).second = ob;
	}
}

void AuScDesignHash::LoadSoloInit()
{
	clear();
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Create a query to select all of Haxus' designs that are final.
	AuId idArchitect(AuId::Haxus());
	QString sq = QString("SELECT id,name,archname,state,textures, "
			"parts,modules,landingOrient,stationOrient,streamline,structure,techLevelMin, "
			"armor,bays,capacitor,cryoBerths,ftlDrives,fuel,hold,lifeSupport,maneuverDrive,powerPlant,sensors,shields,skeleton, "
			"hullVolume "
			"FROM n_ScDesign WHERE state = 1 and ida=%1;").arg(idArchitect.GetID());
	AuScDesign ob;
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
				qDebug() << "AuScDesignHash::LoadSoloInit Z_DATA_ERROR handled ok." << ob.Id() << ob.Name() << ob.ArchitectId()
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

		ob.m_LandingOrient = qry.value(qv++).toUInt();
		ob.m_StationOrient = qry.value(qv++).toUInt();
		ob.m_Streamline = qry.value(qv++).toUInt();
		ob.m_Structure = qry.value(qv++).toUInt();
		ob.m_MinQuality = qry.value(qv++).toInt();
		if(!ob.m_MinQuality)
			ob.m_MinQuality = 1;

		// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
		ob.m_Armor = qry.value(qv++).toUInt();
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
		ob.m_CryoBerths = qry.value(qv++).toUInt();
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			ob.m_FTLDrives.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				quint16 partid;
				quint32 hullvol;

				in >> partid;
				in >> hullvol;

				ob.m_FTLDrives.insert(partid, hullvol);
			}
		}
		ob.m_Fuel = qry.value(qv++).toUInt();
		ob.m_Hold = qry.value(qv++).toUInt();
		ob.m_LifeSupport = qry.value(qv++).toUInt();
		ob.m_ManeuverDrive = qry.value(qv++).toUInt();
		ob.m_PowerPlant = qry.value(qv++).toUInt();
		ob.m_Sensors = qry.value(qv++).toUInt();
		ob.m_Shields = qry.value(qv++).toUInt();
		ob.m_Skeleton = qry.value(qv++).toUInt();

		// Factors gleaned by design analysis
		ob.m_HullVolume = qry.value(qv++).toUInt();
		//qDebug() << "AuScDesignHash::LoadSoloInit arch" << idArchitect << "loaded" << ob.Name() << ob.DescriptionOneLine();

		// Insert/update the data in this hash.
		iterator di = find(ob.Id());
		if(di == end())
			insert(pair<quint32, AuScDesign>(ob.Id(), ob));
		else
			(*di).second = ob;
	}
}

void AuScDesignHash::Load(quint32 idDesign)
{
	AuScDesign ob;
	ob.Load(idDesign);
	if(ob.Id() != idDesign)
		return;

	// Insert/update the data in this hash.
	iterator di = find(ob.Id());
	if(di == end())
		insert(pair<quint32, AuScDesign>(ob.Id(), ob));
	else
		(*di).second = ob;
}

void AuScDesignHash::operator<<(QDataStream& in)
{
	clear();
	quint16 cnt;
	in >> cnt;
	AuScDesign ship;
	for(; cnt; cnt--)
	{
		ship << in;
		insert(pair<quint32, AuScDesign>(ship.Id(), ship));
	}
}

void AuScDesignHash::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	out << (quint16)size();
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it).second >> out;
}

quint64 AuScDesignHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScDesignHash);
	for(const_iterator it = cbegin(); it != cend(); it++)
		mem += sizeof(quint32) + (*it).second.MemoryUsage();
	return mem;
}

quint32 AuScDesignHash::HighestId() const
{
	quint32 highest = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).first > highest)
			highest = (*it).first;
	return highest;
}

quint32 AuScDesignHash::SoloInitNextId() const
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

void AuScDesignHash::Merge(const AuScDesignHash& mdh)
{
	// Add designs that are not already here.
	for(const_iterator it = mdh.cbegin(); it != mdh.cend(); it++)
	{
		if(find((*it).first) != end())
			continue;	// I already have that design.
		insert(pair<quint32, AuScDesign>((*it).first, (*it).second));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScDesign::AuScDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect) :
	AuDesign(idDesign, idArchitect, architect)
{
	m_HullVolume = 0;
	m_LandingOrient = LORI_Belly;
	m_Parts = new AuScParts;
	m_StationOrient = SORI_Random;
	m_Streamline = SLINE_None;
	m_Structure = SC_Hull;
	m_MinQuality = 1;

	m_Armor = 0;
	m_Capacitor = 0;
	m_CryoBerths = 0;
	m_Fuel = 0;
	m_Hold = 0;
	m_LifeSupport = 0;
	m_ManeuverDrive = 0;
	m_PowerPlant = 0;
	m_Sensors = 0;
	m_Shields = 0;
	m_Skeleton = 0;

	// Build a reasonable default palette.
	quint16 texLibPal[SCTEXI_Count];
	AuTextureLibItem::InitDefaultScPalette(texLibPal);
	AuDesignTex texturePalette[SCTEXI_Count];
	for(int ti=0; ti<SCTEXI_Count; ti++)
		texturePalette[ti].SetTexLibId(texLibPal[ti]);
	SetTexturePalette(texturePalette);
}

AuScDesign::AuScDesign()
{
	m_HullVolume = 0;
	m_LandingOrient = LORI_Belly;
	m_Parts = new AuScParts;
	m_StationOrient = SORI_Random;
	m_Streamline = SLINE_None;
	m_Structure = SC_Hull;
	m_MinQuality = 1;

	m_Armor = 0;
	m_Capacitor = 0;
	m_CryoBerths = 0;
	m_Fuel = 0;
	m_Hold = 0;
	m_LifeSupport = 0;
	m_ManeuverDrive = 0;
	m_PowerPlant = 0;
	m_Sensors = 0;
	m_Shields = 0;
	m_Skeleton = 0;

	// Build a reasonable default palette.
	quint16 texLibPal[SCTEXI_Count];
	AuTextureLibItem::InitDefaultScPalette(texLibPal);
	AuDesignTex texturePalette[SCTEXI_Count];
	for(int ti=0; ti<SCTEXI_Count; ti++)
		texturePalette[ti].SetTexLibId(texLibPal[ti]);
	SetTexturePalette(texturePalette);
}

AuScDesign::~AuScDesign()
{
	delete m_Parts;
}

AuScDesign::AuScDesign(const AuScDesign& c) : AuDesign(c), m_Modules(c.m_Modules), m_Bays(c.m_Bays),
	m_FTLDrives(c.m_FTLDrives)
{
	m_HullVolume = c.m_HullVolume;
	m_LandingOrient = c.m_LandingOrient;
	m_Parts = new AuScParts(*c.m_Parts);
	m_StationOrient = c.m_StationOrient;
	m_Streamline = c.m_Streamline;
	m_Structure = c.m_Structure;
	m_MinQuality = c.m_MinQuality;

	m_Armor = c.m_Armor;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerths = c.m_CryoBerths;
	m_Fuel = c.m_Fuel;
	m_Hold = c.m_Hold;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensors = c.m_Sensors;
	m_Shields = c.m_Shields;
	m_Skeleton = c.m_Skeleton;
}

AuScDesign& AuScDesign::operator=(const AuScDesign& c)
{
	AuDesign::operator=(c);

	m_HullVolume = c.m_HullVolume;
	m_LandingOrient = c.m_LandingOrient;
	m_Modules = c.m_Modules;
	*m_Parts = *c.m_Parts;
	m_StationOrient = c.m_StationOrient;
	m_Streamline = c.m_Streamline;
	m_Structure = c.m_Structure;
	m_MinQuality = c.m_MinQuality;

	m_Armor = c.m_Armor;
	m_Bays = c.m_Bays;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerths = c.m_CryoBerths;
	m_FTLDrives = c.m_FTLDrives;
	m_Fuel = c.m_Fuel;
	m_Hold = c.m_Hold;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensors = c.m_Sensors;
	m_Shields = c.m_Shields;
	m_Skeleton = c.m_Skeleton;

	return *this;
}

AuDesign* AuScDesign::Clone() const
{
	return new AuScDesign(*this);
}

void AuScDesign::Load(quint32 idDesign)
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	QString sq = QString("SELECT name,ida,archname,state,textures, "
			"parts,modules,landingOrient,stationOrient,streamline,structure,techLevelMin, "
			"armor,bays,capacitor,cryoBerths,ftlDrives,fuel,hold,lifeSupport,maneuverDrive,powerPlant,sensors,shields,skeleton, "
			"hullVolume "
			"FROM n_ScDesign WHERE id=%1;").arg(idDesign);
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
				qDebug() << "AuScDesign::Load Z_DATA_ERROR handled ok." << Id() << Name() << ArchitectId()
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

		m_LandingOrient = qry.value(qv++).toUInt();
		m_StationOrient = qry.value(qv++).toUInt();
		m_Streamline = qry.value(qv++).toUInt();
		m_Structure = qry.value(qv++).toUInt();
		m_MinQuality = qry.value(qv++).toInt();
		if(!m_MinQuality)
			m_MinQuality = 1;

		// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
		m_Armor = qry.value(qv++).toUInt();
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
		m_CryoBerths = qry.value(qv++).toUInt();
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			m_FTLDrives.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				quint16 partid;
				quint32 hullvol;

				in >> partid;
				in >> hullvol;

				m_FTLDrives.insert(partid, hullvol);
			}
		}
		m_Fuel = qry.value(qv++).toUInt();
		m_Hold = qry.value(qv++).toUInt();
		m_LifeSupport = qry.value(qv++).toUInt();
		m_ManeuverDrive = qry.value(qv++).toUInt();
		m_PowerPlant = qry.value(qv++).toUInt();
		m_Sensors = qry.value(qv++).toUInt();
		m_Shields = qry.value(qv++).toUInt();
		m_Skeleton = qry.value(qv++).toUInt();

		// Factors gleaned by design analysis
		m_HullVolume = qry.value(qv++).toUInt();
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
			qWarning() << "AuScDesign::Load FAILED to SELECT design" << idDesign;
	}
}

const AuScParts& AuScDesign::Parts() const
{
	return *m_Parts;
}

AuScParts& AuScDesign::Parts()
{
	return *m_Parts;
}

void AuScDesign::ClearParts()
{
	m_Parts->Clear();
	m_HullVolume = 0;
}

void AuScDesign::DesignLoaded(const AuDesign& design)
{
	AuDesign::DesignLoaded(design);
	try
	{
		const AuScDesign& c = dynamic_cast<const AuScDesign&>(design);
		m_HullVolume = c.m_HullVolume;
		m_LandingOrient = c.m_LandingOrient;
		m_Modules = c.m_Modules;
		*m_Parts = *c.m_Parts;
		m_StationOrient = c.m_StationOrient;
		m_Streamline = c.m_Streamline;
		m_Structure = c.m_Structure;
		m_MinQuality = c.m_MinQuality;

		m_Armor = c.m_Armor;
		m_Bays = c.m_Bays;
		m_Capacitor = c.m_Capacitor;
		m_CryoBerths = c.m_CryoBerths;
		m_FTLDrives = c.m_FTLDrives;
		m_Fuel = c.m_Fuel;
		m_Hold = c.m_Hold;
		m_LifeSupport = c.m_LifeSupport;
		m_ManeuverDrive = c.m_ManeuverDrive;
		m_PowerPlant = c.m_PowerPlant;
		m_Sensors = c.m_Sensors;
		m_Shields = c.m_Shields;
		m_Skeleton = c.m_Skeleton;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuScDesign::operator>>(QDataStream& out) const
{
	AuDesign::operator>>(out);

	quint8 ver = (SCDESIGN_SERIALIZE_VER << 4);	// Bottom nibble used to be armor level 0 to 10.
	out << ver;
	out << m_HullVolume;
	out << m_LandingOrient;
	out << m_Modules;
	out << *m_Parts;
	out << m_StationOrient;
	out << m_Streamline;
	out << m_Structure;
	out << m_MinQuality;

	out << m_Armor;
	out << (quint16)m_Bays.size();
	for(QHash<quint16,quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		out << bi.key();
		out << bi.value();
	}
	out << m_Capacitor;
	out << m_CryoBerths;
	out << (quint16)m_FTLDrives.size();
	for(QHash<quint16,quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
	{
		out << fi.key();
		out << fi.value();
	}
	out << m_Fuel;
	out << m_Hold;
	out << m_LifeSupport;
	out << m_ManeuverDrive;
	out << m_PowerPlant;
	out << m_Sensors;
	out << m_Shields;
	out << m_Skeleton;
}

void AuScDesign::operator<<(QDataStream& in)
{
	AuDesign::operator<<(in);

	quint8 ver;
	in >> ver;
	ver = ver >> 4;

	if(ver == 0)
	{
		quint16 cryoBerths;
		in >> cryoBerths;
		in >> *m_Parts;
		in >> m_Modules;
		in >> m_Streamline;
		in >> m_Structure;
		in >> m_MinQuality;
		if(!m_MinQuality)
			m_MinQuality = 1;

		float a;
		quint8 ui8;
		quint16 ui16;
		bool b;

		in >> a;				// accel
		in >> ui16;				// crew berths
		in >> ui16;				// hull volume (was never calc'd)
		in >> ui16;				// officer berths
		in >> ui16;				// passenger berths
		in >> b;				// pirate ok
		in >> ui8;				// tech level
		in >> ui16;				// troop berths
		in >> ui16;				// turrets
		in >> ui8;				// weapon bays

		m_HullVolume = 0;

		m_Armor = 0;
		m_Bays.clear();
		m_Capacitor = 0;
		m_CryoBerths = cryoBerths;
		m_FTLDrives.clear();
		m_Fuel = 0;
		m_Hold = 0;
		m_LandingOrient = LORI_Belly;
		m_LifeSupport = 0;
		m_ManeuverDrive = 0;
		m_PowerPlant = 0;
		m_Sensors = 0;
		m_Shields = 0;
		m_Skeleton = 0;
		m_StationOrient = SORI_Random;
	}
	else
	{
		in >> m_HullVolume;

		if(ver >= 0x06)
			in >> m_LandingOrient;
		else
			m_LandingOrient = LORI_Belly;

		in >> m_Modules;
		in >> *m_Parts;

		if(ver >= 3)
			in >> m_StationOrient;
		else
			m_StationOrient = SORI_Random;

		in >> m_Streamline;
		in >> m_Structure;
		in >> m_MinQuality;
		if(!m_MinQuality)
			m_MinQuality = 1;

		if(ver >= 2)
			in >> m_Armor;
		else
			m_Armor = 0;

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
		in >> m_CryoBerths;
		m_FTLDrives.clear();
		in >> c;
		while(c--)
		{
			quint16 partid;
			quint32 hullvol;

			in >> partid;
			in >> hullvol;

			m_FTLDrives.insert(partid, hullvol);
		}
		in >> m_Fuel;
		in >> m_Hold;
		in >> m_LifeSupport;
		in >> m_ManeuverDrive;
		in >> m_PowerPlant;
		in >> m_Sensors;
		in >> m_Shields;
		if(ver >= 0x05)
			in >> m_Skeleton;
		else
			m_Skeleton = 0;
	}
}

void AuScDesign::Insert(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO n_ScDesign (id,name,ida,archname,state,textures, "
			"parts,modules,landingOrient,stationOrient,streamline,structure,techLevelMin, "
			"armor,bays,capacitor,cryoBerths,ftlDrives,fuel,hold,lifeSupport,maneuverDrive,powerPlant,sensors,shields,skeleton, "
			"hullVolume, techLevel"
			") VALUES ("
			"?,?,?,?,?,?, "
			"?,?,?,?,?,?,?, "
			"?,?,?,?,?,?,?,?,?,?,?,?,?, "
			"?, ?);");
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

	qa.addBindValue(m_LandingOrient);
	qa.addBindValue(m_StationOrient);
	qa.addBindValue(m_Streamline);
	qa.addBindValue(m_Structure);
	qa.addBindValue(m_MinQuality);

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	qa.addBindValue((qint32)m_Armor);
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
	qa.addBindValue((qint32)m_CryoBerths);
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << (quint16)m_FTLDrives.size();
		for(QHash<quint16,quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		{
			out << fi.key();
			out << fi.value();
		}
		qa.addBindValue(ba);
	}
	qa.addBindValue((qint32)m_Fuel);
	qa.addBindValue((qint32)m_Hold);
	qa.addBindValue((qint32)m_LifeSupport);
	qa.addBindValue((qint32)m_ManeuverDrive);
	qa.addBindValue((qint32)m_PowerPlant);
	qa.addBindValue((qint32)m_Sensors);
	qa.addBindValue((qint32)m_Shields);
	qa.addBindValue((qint32)m_Skeleton);

	// Factors gleaned by design analysis
	qa.addBindValue((qint32)m_HullVolume);
	qa.addBindValue(MinQ());

	if(!qa.exec())
		qWarning("ERROR! AuScDesign::Insert %s FAILED '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1());
}

void AuScDesign::Update(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("UPDATE n_ScDesign SET name=?, ida=?, archname=?, state=?, textures=?, "
			"parts=?, modules=?, landingOrient=?, stationOrient=?, streamline=?, structure=?, techLevelMin=?, "
			"armor=?, bays=?, capacitor=?, cryoBerths=?, ftlDrives=?, fuel=?, hold=?, lifeSupport=?, maneuverDrive=?, "
			"powerPlant=?, sensors=?, shields=?, skeleton=?, hullVolume=?, techLevel=? "
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

	qa.addBindValue(m_LandingOrient);
	qa.addBindValue(m_StationOrient);
	qa.addBindValue(m_Streamline);
	qa.addBindValue(m_Structure);
	qa.addBindValue(m_MinQuality);

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	qa.addBindValue((qint32)m_Armor);
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
	qa.addBindValue((qint32)m_CryoBerths);
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << (quint16)m_FTLDrives.size();
		for(QHash<quint16,quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		{
			out << fi.key();
			out << fi.value();
		}
		qa.addBindValue(ba);
	}
	qa.addBindValue((qint32)m_Fuel);
	qa.addBindValue((qint32)m_Hold);
	qa.addBindValue((qint32)m_LifeSupport);
	qa.addBindValue((qint32)m_ManeuverDrive);
	qa.addBindValue((qint32)m_PowerPlant);
	qa.addBindValue((qint32)m_Sensors);
	qa.addBindValue((qint32)m_Shields);
	qa.addBindValue((qint32)m_Skeleton);

	// Factors gleaned by design analysis
	qa.addBindValue((qint32)m_HullVolume);
	qa.addBindValue(MinQ());

	if(!qa.exec())
		qWarning("ERROR! AuScDesign::Update %s FAILED '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1());
}

void AuScDesign::Tx(QDataStream& out) const
{
	AuDesign::Tx(out);

	quint8 ver = (SCDESIGN_SERIALIZE_VER << 4);	// Bottom nibble used to be armor level 0 to 10.
	out << ver;
	out << m_HullVolume;
	out << m_LandingOrient;
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
	out << m_StationOrient;
	out << m_Streamline;
	out << m_Structure;
	out << m_MinQuality;
	out << m_Armor;

	out << (quint16)m_Bays.size();
	for(QHash<quint16,quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		out << bi.key();
		out << bi.value();
	}
	out << m_Capacitor;
	out << m_CryoBerths;
	out << (quint16)m_FTLDrives.size();
	for(QHash<quint16,quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
	{
		out << fi.key();
		out << fi.value();
	}
	out << m_Fuel;
	out << m_Hold;
	out << m_LifeSupport;
	out << m_ManeuverDrive;
	out << m_PowerPlant;
	out << m_Sensors;
	out << m_Shields;
	out << m_Skeleton;
}

// NOTE: This function is also used when loading from a file.
void AuScDesign::Rx(QDataStream& in)
{
	AuDesign::Rx(in);

	quint8 ver;
	in >> ver;
	ver = ver >> 4;

	if(ver == 0)
	{
		quint16 cryoBerths;
		in >> cryoBerths;
		in >> *m_Parts;
		in >> m_Modules;
		in >> m_Streamline;
		in >> m_Structure;
		in >> m_MinQuality;
		if(!m_MinQuality)
			m_MinQuality = 1;

		float a;
		quint8 ui8;
		quint16 ui16;
		bool b;

		in >> a;				// accel
		in >> ui16;				// crew berths
		in >> ui16;				// hull volume (was never calc'd)
		in >> ui16;				// officer berths
		in >> ui16;				// passenger berths
		in >> b;				// pirate ok
		in >> ui8;				// tech level
		in >> ui16;				// troop berths
		in >> ui16;				// turrets
		in >> ui8;				// weapon bays

		m_HullVolume = 0;

		m_Armor = 0;
		m_Bays.clear();
		m_Capacitor = 0;
		m_CryoBerths = cryoBerths;
		m_FTLDrives.clear();
		m_Fuel = 0;
		m_Hold = 0;
		m_LandingOrient = LORI_Belly;
		m_LifeSupport = 0;
		m_ManeuverDrive = 0;
		m_PowerPlant = 0;
		m_Sensors = 0;
		m_Shields = 0;
		m_Skeleton = 0;
		m_StationOrient = SORI_Random;
	}
	else
	{
		in >> m_HullVolume;

		if(ver >= 0x06)
			in >> m_LandingOrient;
		else
			m_LandingOrient = LORI_Belly;

		in >> m_Modules;
		if(ver >= 4)
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

		if(ver >= 3)
			in >> m_StationOrient;
		else
			m_StationOrient = SORI_Random;

		in >> m_Streamline;
		in >> m_Structure;
		in >> m_MinQuality;
		if(!m_MinQuality)
			m_MinQuality = 1;

		if(ver >= 2)
			in >> m_Armor;
		else
			m_Armor = 0;

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
		in >> m_CryoBerths;
		m_FTLDrives.clear();
		in >> c;
		while(c--)
		{
			quint16 partid;
			quint32 hullvol;

			in >> partid;
			in >> hullvol;

			m_FTLDrives.insert(partid, hullvol);
		}
		in >> m_Fuel;
		in >> m_Hold;
		in >> m_LifeSupport;
		in >> m_ManeuverDrive;
		in >> m_PowerPlant;
		in >> m_Sensors;
		in >> m_Shields;
		if(ver >= 5)
			in >> m_Skeleton;
		else
			m_Skeleton = 0;
	}
}

void AuScDesign::SetArmorVolume(quint32 armor)
{
	m_Armor = armor;
}

void AuScDesign::SetMinQ(quint8 qmin)
{
	Q_ASSERT(qmin);
	m_MinQuality = qmin;
}

void AuScDesign::SetStructure(SPACECRAFT_STRUCTURE structure)
{
	m_Structure = structure;
}

void AuScDesign::SetLandingOrientation(LANDING_ORIENT orient)
{
	m_LandingOrient = orient;
}

void AuScDesign::SetStationOrientation(STATION_ORIENT orient)
{
	m_StationOrient = orient;
}

void AuScDesign::SetStreamline(STREAMLINE streamline)
{
	m_Streamline = streamline;
}

bool AuScDesign::Validate()
{
	//qDebug() << "AuScDesign::Validate";
	bool isValid = AuDesign::Validate();

	//DHH Validate parts.
	//AuScParts	m_Parts;

	//DHH Validate modules.
	//AuScModules	m_Modules;

	return isValid;
}

// Returns the description as a single line.
// Format example: Streamlined Starship Q231 32m^3 (using cubed symbol instead of ^3).
QString AuScDesign::DescriptionOneLine() const
{
	QString cubedSym;
	if(g_Au.IsWebCGI())
		cubedSym = QString("³");
	else
		cubedSym = g_Au.CubedSymbol();

	if(m_Streamline)
		return QObject::tr("%5 %1 Q%2 %L3m%4")
			.arg(StructureName())
			.arg(MinQ())
			.arg(HullVolume())
			.arg(cubedSym)
			.arg(StreamlineName());
	else
		return QObject::tr("%1 Q%2 %L3m%4")
			.arg(StructureName())
			.arg(MinQ())
			.arg(HullVolume())
			.arg(cubedSym);
}

void AuScDesign::SetBayVolume(quint16 which, quint32 hullVol)
{
	m_Bays.insert(which, hullVol);
}

void AuScDesign::SetCapacitorVolume(quint32 hullVol)
{
	m_Capacitor = hullVol;
}

void AuScDesign::SetCryoBerthVolume(quint32 hullVol)
{
	m_CryoBerths = hullVol;
}

void AuScDesign::SetFTLDriveVolume(quint16 which, quint32 hullVol)
{
	m_FTLDrives.insert(which, hullVol);
}

void AuScDesign::SetFuelVolume(quint32 hullVol)
{
	m_Fuel = hullVol;
}

void AuScDesign::SetHoldVolume(quint32 hullVol)
{
	m_Hold = hullVol;
}

void AuScDesign::SetSkeletonVolume(quint32 vol)
{
	m_Skeleton = vol;
}

void AuScDesign::SetLifeSupportVolume(quint32 hullVol)
{
	m_LifeSupport = hullVol;
}

void AuScDesign::SetManeuverDriveVolume(quint32 hullVol)
{
	m_ManeuverDrive = hullVol;
}

void AuScDesign::SetModules(const AuScModules& modules)
{
	m_Modules = modules;
}

void AuScDesign::SetPowerPlantVolume(quint32 hullVol)
{
	m_PowerPlant = hullVol;
}

void AuScDesign::SetSensorVolume(quint32 hullVol)
{
	m_Sensors = hullVol;
}

void AuScDesign::SetShieldVolume(quint32 hullVol)
{
	m_Shields = hullVol;
}

quint16 AuScDesign::PartAdd(const AuScPart& part, AuScParts& partsAdded)
{
	quint16 partId = AuDesign::PartAdd(part, partsAdded);
	if(!partId)
		return 0;
	m_HullVolume = 0;
	return partId;
}

bool AuScDesign::PartObjectAdd(quint16 partId, const AuScPart& part, AuScParts& partsChanged)
{
	if(!AuDesign::PartObjectAdd(partId, part, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsCopy(const AuScSelection& partIds, AuScParts& partsAdded)
{
	if(!AuDesign::PartsCopy(partIds, partsAdded))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsCopySymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane, AuScParts& partsAdded)
{
	if(!AuDesign::PartsCopySymmetrical(partIds, gridPlane, partsAdded))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsTransform(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged, bool objectMode)
{
	if(!AuDesign::PartsTransform(partIds, t, partsChanged, objectMode))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsTransformLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged)
{
	if(!AuDesign::PartsTransformLiftStops(partIds, t, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsTransformCopy(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged)
{
	if(!AuDesign::PartsTransformCopy(partIds, t, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartDelete(quint16 partId, AuScParts& partsDeleted)
{
	if(!AuDesign::PartDelete(partId, partsDeleted))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartObjectDelete(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted)
{
	if(!AuDesign::PartObjectDelete(partId, selInfo, partsChanged, partsDeleted))
		return false;
	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartUpdate(quint16 partId, const AuScPart& part, AuScParts& partsUpdated)
{
	if(!AuDesign::PartUpdate(partId, part, partsUpdated))
		return false;
	m_HullVolume = 0;
	return true;
}

void AuScDesign::UndoRedoAdd(const AuScParts& parts)
{
	AuDesign::UndoRedoAdd(parts);
	m_HullVolume = 0;
}

void AuScDesign::UndoRedoDelete(const QList<quint16>& partIds)
{
	AuDesign::UndoRedoDelete(partIds);
	m_HullVolume = 0;
}

void AuScDesign::UndoRedoSwap(AuScParts& swapParts)
{
	AuDesign::UndoRedoSwap(swapParts);
	m_HullVolume = 0;
}

bool AuScDesign::PartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	if(!m_Parts->FillFace(sel, boundary, gridPlane, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartFillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, const AuPoint& v4,
		AuScParts& partsChanged)
{
	if(!m_Parts->FillQuad(sel, v1, v2, v3, v4, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartFillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
		AuScParts& partsChanged)
{
	if(!m_Parts->FillTriangle(sel, v1, v2, v3, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartSetOmitted(quint16 partId, bool omit, AuScParts& partsChanged)
{
	if(!m_Parts->SetOmitted(partId, omit, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartSetSealed(quint16 partId, bool seal, AuScParts& partsChanged)
{
	if(!m_Parts->SetSealed(partId, seal, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartDeleteState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	if(!m_Parts->DeleteState(partId, state, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartAddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged)
{
	if(!m_Parts->AddFaces(partId, faces, partsChanged))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeNavLightLens(const AuScSelection& sel, const QColor& color, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeNavLightLens(sel, color, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeDecal(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeDoor(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

/*
bool AuScDesign::PartsMakeHeadlightLens(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeHeadlightLens(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}
*/

bool AuScDesign::PartsMakeLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeLightLens(sel, lightColor, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeRoom(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeRoom(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeHull(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeMesh(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeMesh(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeLandingGear(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeRotatingBeaconLens(const AuScSelection& sel, const QColor& lightColor, const AuPoint& rotationAxis,
		AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeRotatingBeaconLens(sel, lightColor, rotationAxis, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeStrobeLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeStrobeLens(sel, lightColor, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeTurboLift(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeTurboLift(sel, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeTurret(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!m_Parts->MakeTurret(sel, basePivot, baseAxis, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

bool AuScDesign::PartsMakeWindow(const AuScSelection& sel, float opacity, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!m_Parts->MakeWindow(sel, opacity, partsAdded, partsDeleted))
		return false;

	m_HullVolume = 0;
	return true;
}

void AuScDesign::ShaderTexturePalette(COMMODITYID hullModule, AuGLEffects& re, AuTexturePalette& textures) const
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

void AuScDesign::SetProperties(const AuDesign& design)
{
	try
	{
		const AuScDesign& c = dynamic_cast<const AuScDesign&>(design);

		m_HullVolume = c.m_HullVolume;
		m_LandingOrient = c.m_LandingOrient;
		m_Modules = c.m_Modules;
		//m_Parts = c.m_Parts;
		m_StationOrient = c.m_StationOrient;
		m_Streamline = c.m_Streamline;
		m_Structure = c.m_Structure;
		m_MinQuality = c.m_MinQuality;

		m_Armor = c.m_Armor;
		m_Bays = c.m_Bays;
		m_Capacitor = c.m_Capacitor;
		m_CryoBerths = c.m_CryoBerths;
		m_FTLDrives = c.m_FTLDrives;
		m_Fuel = c.m_Fuel;
		m_Hold = c.m_Hold;
		m_LifeSupport = c.m_LifeSupport;
		m_ManeuverDrive = c.m_ManeuverDrive;
		m_PowerPlant = c.m_PowerPlant;
		m_Sensors = c.m_Sensors;
		m_Shields = c.m_Shields;
		m_Skeleton = c.m_Skeleton;

		AuDesign::SetProperties(design);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

bool AuScDesign::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops);
}

void AuScDesign::CalcHullVolume()
{
	//qDebug() << "AuScDesign::CalcHullVolume" << AuTime::Now();
	m_HullVolume = m_Parts->HullVolume();
	//qDebug() << "AuScDesign::CalcHullVolume result" << m_HullVolume << AuTime::Now();
}

void AuScDesign::RefreshObstructionsCache()
{
	m_Parts->RefreshObstructionsCache();
	CalcHullVolume();
	SyncBaysAndFTLDrives();
}

void AuScDesign::RefreshObstructionsCache(QProgressDialog& progress)
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

	progress.setLabelText(QObject::tr("Syncing equipment bays and FTL drives..."));
	SyncBaysAndFTLDrives();
	progress.setValue(partProg*2 + 1);
}

bool AuScDesign::RefreshObstructionsCacheIfNeeded()
{
	bool needed = m_Parts->RefreshObstructionsCacheIfNeeded();
	CalcHullVolume();
	SyncBaysAndFTLDrives();
	return needed;
}

void AuScDesign::SyncBaysAndFTLDrives()
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

	// Add ftl drives we are missing.
	for(QSet<quint16>::const_iterator fi = ftlDrives.cbegin(); fi != ftlDrives.cend(); fi++)
		if(!m_FTLDrives.contains(*fi))
			m_FTLDrives.insert(*fi, 0);

	// Remove ftl drives that don't belong there.
	QHash<quint16, quint32>::iterator fi = m_FTLDrives.begin();
	while(fi != m_FTLDrives.end())
		if(ftlDrives.contains(fi.key()))
			fi++;
		else
			fi = m_FTLDrives.erase(fi);
}

void AuScDesign::Bays(AuScPartSet& partIds) const
{
	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		partIds.insert(bi.key());
}

quint32 AuScDesign::BayVolume() const
{
	quint32 vol = 0;
	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		vol += *bi;
	return vol;
}

quint32 AuScDesign::BayVolume(quint16 which) const
{
	QHash<quint16, quint32>::const_iterator bi = m_Bays.find(which);
	if(bi == m_Bays.cend())
		return 0;
	return *bi;
}

double AuScDesign::BayVolumeCuFt() const
{
	double vol = 0;
	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		vol += *bi;
	return vol * CubicFeetPerCubicMeter();
}

double AuScDesign::BayVolumeCuFt(quint16 which) const
{
	QHash<quint16, quint32>::const_iterator bi = m_Bays.find(which);
	if(bi == m_Bays.cend())
		return 0;
	return *bi * CubicFeetPerCubicMeter();
}

void AuScDesign::FTLDrives(AuScPartSet& partIds) const
{
	for(QHash<quint16, quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		partIds.insert(fi.key());
}

quint32 AuScDesign::FTLDriveVolume() const
{
	quint32 vol = 0;
	for(QHash<quint16, quint32>::const_iterator bi = m_FTLDrives.cbegin(); bi != m_FTLDrives.cend(); bi++)
		vol += *bi;
	return vol;
}

quint32 AuScDesign::FTLDriveVolume(quint16 which) const
{
	QHash<quint16, quint32>::const_iterator fi = m_FTLDrives.find(which);
	if(fi == m_FTLDrives.cend())
		return 0;
	return *fi;
}

double AuScDesign::FTLDriveVolumeCuFt() const
{
	double vol = 0;
	for(QHash<quint16, quint32>::const_iterator bi = m_FTLDrives.cbegin(); bi != m_FTLDrives.cend(); bi++)
		vol += *bi;
	return vol * CubicFeetPerCubicMeter();
}

double AuScDesign::FTLDriveVolumeCuFt(quint16 which) const
{
	QHash<quint16, quint32>::const_iterator fi = m_FTLDrives.find(which);
	if(fi == m_FTLDrives.cend())
		return 0;
	return *fi * CubicFeetPerCubicMeter();
}

void AuScDesign::ReportDesign(AuScReport& report) const
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
		report.m_LandingOrient = m_LandingOrient;
		report.m_StationOrient = m_StationOrient;
		report.m_Streamline = m_Streamline;
		report.m_Structure = m_Structure;
	}

	// Volumes, in cubic meters.
	{
		report.m_Hull = AuScReportSysVol(HullModule(), m_MinQuality, HullVolume());
		m_Parts->HullExtents(report.m_HullMin, report.m_HullMax);

		report.m_ArmorVolume = ArmorVolume();

		for(QHash<quint16, quint32>::const_iterator fi = m_Bays.cbegin(); fi != m_Bays.cend(); fi++)
		{
			AuScParts::const_iterator pi = m_Parts->find(fi.key());
			if(pi  == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			report.m_Bays.insert(pi.key(), AuScReportSysVol((*pi)->Module(), m_MinQuality, *fi));
		}

		report.m_Capacitor = AuScReportSysVol(CapacitorModule(), m_MinQuality, CapacitorVolume());
		report.m_CryoBerthVolume = CryoBerthVolume();

		for(QHash<quint16, quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		{
			AuScParts::const_iterator pi = m_Parts->find(fi.key());
			if(pi  == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			report.m_FTLDrives.insert(pi.key(), AuScReportSysVol((*pi)->Module(), m_MinQuality, *fi));
		}

		report.m_FuelVolume = FuelVolume();
		report.m_HoldVolume = HoldVolume();
		report.m_LifeSupport = AuScReportSysVol(LifeSupportModule(), m_MinQuality, LifeSupportVolume());
		report.m_ManeuverDrive = AuScReportSysVol(ManeuverDriveModule(), m_MinQuality, ManeuverDriveVolume());
		report.m_PowerPlant = AuScReportSysVol(PowerPlantModule(), m_MinQuality, PowerPlantVolume());
		report.m_Sensor = AuScReportSysVol(SensorModule(), m_MinQuality, SensorVolume());
		report.m_Shield = AuScReportSysVol(ShieldModule(), m_MinQuality, ShieldVolume());
		report.m_SkeletonVolume = SkeletonVolume();

		AuScPartSet turrets;	// key is part id
		Turrets(turrets);
		for(QSet<quint16>::const_iterator ti = turrets.cbegin(); ti != turrets.cend(); ti++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*ti);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			quint16 roomId = RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);

			COMMODITYID turretMod = TurretModule(*ti);
			report.m_Turrets.insert(*ti, AuScReportTurret(turretMod, m_MinQuality, roomId));
		}
	}

	// Quality
	{
		report.m_Quality = MinQ();
		report.m_CnCModule = CommandAndControlModule();
		report.m_CnCQ = MinQ();
		report.m_MfgProc.InitSc(*this, report.m_Quality);	// Could be higher Q than used by QualityOfManufacturingProcess.
		if(State() != Final)
			report.m_MfgProc.ClearManufacturingProcessId();	// Prevent the thing from being manufactured.
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

			quint16 roomId = RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);

			COMMODITYID mod = (*pi)->Module();
			report.m_CommandStations.insert(pi.key(), AuScReportStation(mod, m_MinQuality, (*pi)->Type(), (*pi)->ReportDesc(),
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

			quint16 roomId = RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);

			COMMODITYID mod = (*pi)->Module();
			if(ScModuleSystem(mod) != SYS_Transporter)
				report.m_CrewStations.insert(pi.key(), AuScReportStation(mod, m_MinQuality, (*pi)->Type(), (*pi)->ReportDesc(),
							roomId, (*pi)->WhichEffect()));
			else
			{
				AuPointArray padLocs;
				(*pi)->PadLocs(padLocs);
				report.m_Transporters.insert(pi.key(),
						AuScReportTransporterStation(mod, m_MinQuality, (*pi)->Type(), (*pi)->ReportDesc(), padLocs.size(), roomId,
							(*pi)->WhichEffect()));
			}
		}

		stations.clear();
		DesignerStations(stations);
		for(QSet<quint16>::const_iterator si = stations.cbegin(); si != stations.cend(); si++)
		{
			AuScParts::const_iterator pi = m_Parts->find(*si);
			if(pi == m_Parts->cend())
				continue;
			if((*pi)->IsOmitted())
				continue;

			quint16 roomId = RoomVoidContainingLoc((*pi)->OperatorLoc(), .5);
			COMMODITYID mod = (*pi)->Module();
			report.m_DesignerStations.insert(pi.key(), AuScReportStation(mod, m_MinQuality, (*pi)->Type(), (*pi)->ReportDesc(),
						roomId, (*pi)->WhichEffect()));
		}

		ServicePanels(report.m_ServicePanels);

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
	{
		report.m_CaptBerths = CaptBerths();
		report.m_CrewBerths = CrewBerths();
		report.m_OfficerBerths = OfficerBerths();
		report.m_PassengerBerths = PassengerBerths();
		report.m_TroopBerths = TroopBerths();
	}

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

QString AuScDesign::ReportAnalysis() const
{
	AuScReport report;
	ReportDesign(report);
	return report.ReportAnalysis(State());
}

double AuScDesign::Mass() const
{
	return HullMass() + FuelMass() + EquipmentMass();
}

double AuScDesign::HullMass() const
{
	return HullDensity(ArmorLevel(), Streamline()) * HullVolumeCuFt();
}

double AuScDesign::EquipmentMass() const
{
	double mass = 0;

	// These values were taken from the old n_DetailType table, which expressed them in kg per unit.
	// e.g. A maneuver drive unit was 2250 kg. All units were 256 cubic feet.
	// Mass due to maneuver drives.
	if(ManeuverDriveVolume())
	{
		double mu = (2250 * PoundsPerKilogram()) / 256.;
		mass += mu * ManeuverDriveVolumeCuFt();
	}

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

	quint32 ftl = FTLDriveVolumeCuFt();
	if(ftl)
	{
		double mu = (700 * PoundsPerKilogram()) / 256.;
		mass += mu * ftl;
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

double AuScDesign::FuelMass() const
{
	return FuelDensity() * FuelVolumeCuFt();
}

quint32 AuScDesign::HoldCapacity() const
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

quint32 AuScDesign::FuelCapacity() const
{
	// Returns the number of commodity units that can fit in the fuel cells.
	if(!FuelVolume())
		return 0;						// No fuel cell space at all.

	// Calculate this the way the old ship designs did it, based on cubic feet.
	quint32 fuelVol = (quint32)FuelVolumeCuFt();
	if(fuelVol < 1)
		return 1;
	quint32 fuel = fuelVol;
	return fuel;
}

//static
double AuScDesign::HullSizeLimit()
{
	// USS Enterprise (NCC-1701-D) is 642.5m (2,108 ft) long x 463.73m (1,521.4 ft) wide x 195.26m (640.6 ft) tall.
	// Imperial star destroyer is 1,600m long. D:
	// Death Star is 160,000m long. D:
	// Smallest moon is 1,894m diameter.
	return (1600. * FeetPerMeter())/2.;
}

double AuScDesign::ManeuverAcceleration(quint8 maneuverDriveQ) const
{
	if(!HullVolume())
		return 0;
	const double tlBoost = QualityBoostF(maneuverDriveQ);
	return (ManeuverDriveVolumeCuFt() * ManeuverDriveForce()) / Mass() * tlBoost; 
}

// Force exerted by one cubic foot of maneuver drives, in lbs.
//static
double AuScDesign::ManeuverDriveForce()
{
	// Calculate force so a 2250kg gravity drive unit will have a maximum acceleration of 500m/s/s.
	// A drive unit is 256 cubic feet.
	//   F = ma
	// Where:
	//   m = 2250kg
	//   a = 500m/s/s
	return (double)((500 * FeetPerMeter()) * (2250 * PoundsPerKilogram())) / 256.;
}

// Returns power plant fuel burn rate in cubic feet per real world second.
// AuScDesign::PowerPerFuel returns the number of power units
// produced from each cubic foot of fuel that is burned.
double AuScDesign::FuelBurnRate() const
{
	// Calculate burn rate in cubic feet per real second.
	// A drive unit is 256 cubic feet.
	const double fpuday = 24.;	// Power plant fuel burn per Universe day.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600. / fpuday) / 256.;
	const double rBurnRate = PowerPlantVolumeCuFt() * rUnitRate;
	return rBurnRate;
}

// Power usage per real world second when holding
// - Gravity drives on HOLD
// - Life Support on
// - Sensors on
// - Shields on
double AuScDesign::PowerUsageHolding(bool shieldsUp, COMMODITYID maneuverDriveModule) const
{
	const double life = LifeSupportPowerRate();
	const double maneuver = ManeuverPowerRateHolding(maneuverDriveModule);
	const double sensor = SensorPowerRate();
	const double shield = shieldsUp ? ShieldPowerRate() : 0;
	const double idlepower = life + maneuver + sensor + shield;
	return idlepower;
}

// Power usage per real world second when idle
// - Gravity drives on
// - Life Support on
// - Sensors on
// - Shields on
double AuScDesign::PowerUsageIdling(bool shieldsUp, COMMODITYID maneuverDriveModule) const
{
	const double life = LifeSupportPowerRate();
	const double maneuver = ManeuverPowerRate(maneuverDriveModule);
	const double sensor = SensorPowerRate();
	const double shield = shieldsUp ? ShieldPowerRate() : 0;
	const double idlepower = life + maneuver + sensor + shield;
	return idlepower;
}

// Returns life support power use rate in units per real world second.
double AuScDesign::LifeSupportPowerRate() const
{
	// Each maneuver drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = LifeSupportVolumeCuFt() * rUnitRate;
	return rUseRate;
}

// Returns shield power use rate in units per real world second.
double AuScDesign::ShieldPowerRate() const
{
	// Each shield unit uses one power units per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = ShieldVolumeCuFt() * rUnitRate;
	return rUseRate;
}

// Returns sensor power use rate in units per real world second.
double AuScDesign::SensorPowerRate() const
{
	// Each sensor uses one power units per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = SensorVolumeCuFt() * rUnitRate;
	return rUseRate;
}

// Turn performance in radians/sec
double AuScDesign::TurnRate(quint8 maneuverDriveTL) const
{
	//DHH calculate this so the farthest point does not move faster than 1000fps.
	double rmaneuver = ManeuverAcceleration(maneuverDriveTL);
	double rdegpersec = rmaneuver / 25.;
	//qDebug("AuScDesign::TurnRate - %gdeg/sec maneuver %gft/sec/sec", rdegpersec, rmaneuver);
	double rturnrate = rdegpersec * DEG2RAD;
	return rturnrate;
}

double AuScDesign::SensorRange(quint8 sensorQ) const
{
	if(SensorVolume() <= 0)
		return 0;

	// A sensor unit is 256 cubic feet.
	const double tlBoost = QualityBoostF(sensorQ);
	return SensorVolumeCuFt()/256. * FeetPerMoonOrbit() / 2. * tlBoost;
}

// Returns maneuver drive fuel use rate in units per real world second.
// Only rocket drives use fuel.
double AuScDesign::ManeuverFuelRate(COMMODITYID maneuverDriveModule) const
{
	// Rocket drives do not use power.
	if(maneuverDriveModule != CID_RocketDriveModule)
		return 0.;

	// Each maneuver drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = ManeuverDriveVolumeCuFt() * rUnitRate / PowerPerFuel();
	return rUseRate;
}

// Returns maneuver drive power use rate in units per real world second.
double AuScDesign::ManeuverPowerRate(COMMODITYID maneuverDriveModule) const
{
	// Rocket drives do not use power.
	if(maneuverDriveModule == CID_RocketDriveModule)
		return 0.;

	// Each maneuver drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = ManeuverDriveVolumeCuFt() * rUnitRate;
	return rUseRate;
}

double AuScDesign::CapacitorMax(quint8 capacitorQ) const
{
	// These values were taken from the old DqDesignAnalysis, which expressed everything in system units.
	// All of the old system units were 256 cubic feet.
	const double tlBoost = QualityBoostF(capacitorQ);
	return CapacitorVolumeCuFt() / 256. * tlBoost;
}

// Returns the number of cubic feet of FTL drive units needed to engage.
double AuScDesign::FTLDriveUnitsToEngage() const
{
	// These values were taken from the old DqDesignAnalysis, which expressed everything in system units.
	// All of the old system units were 256 cubic feet.
	return pow(.75 * HullVolumeCuFt() * PI, 0.33333333) * 256.;
}

// Returns the highest setting available for the FTL drive.
// Returns -1 if power plant is inadequate to engage the FTL drive or if the FTL drive module is not recognized.
//static
int AuScDesign::MaxCapacitorFTLSetting(COMMODITYID ftlDriveMod, quint8 ftlDriveQ, double ftlDriveUnitsToEngageCuFt,
		double capacitorMax)
{
	const double tlBoost = QualityBoostF(ftlDriveQ);
	double powerToEngage = PowerNeededToEngageFTLDrive(ftlDriveUnitsToEngageCuFt);
	switch(ftlDriveMod)
	{
	case CID_Warp1FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 0;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp2FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 1;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp3FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 2;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp4FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 3;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp5FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 4;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp6FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 5;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp7FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 6;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp8FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 7;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_Warp9FTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage at warp factor 1.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;

			// Determine maximum warp factor based on maximum capacitor power and power needed to engage at warp factor 1.
			int warpFactor = (int)((capacitorMax / ftlPowerNeeded - 1) / 2);

			// Limit max warp factor based on warp module.
			const int maxWarpFactor = 8;
			if(warpFactor > maxWarpFactor)
				return maxWarpFactor;

			return warpFactor;
		}
		break;
	case CID_WormholePFTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage.
			// All polarities take the same power on wormhole drives.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;
			// Only positive polarity.
			return 0;
		}
		break;
	case CID_WormholePNFTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage.
			// All polarities take the same power on wormhole drives.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;
			// Positive and negative polarity.
			return 1;
		}
		break;
	case CID_WormholePNNFTLDriveModule:
		{
			// Determine if there is enough capacitor power to engage.
			// All polarities take the same power on wormhole drives.
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;
			// Positive, negative and neutral polarity, for intergalactic wormholes.
			return 2;
		}
		break;
	default:
		// Unrecognized FTL drive module.
		{
			double ftlPowerNeeded = powerToEngage / tlBoost;
			if(ftlPowerNeeded >= capacitorMax)
				return -1;
			return 0;
		}
		break;
	}
	return -1;
}

//static
double AuScDesign::HullDensityFactor(double armorLevel, STREAMLINE streamline)
{
	// Hull is relatively cheap, in terms of mass.
	// Streamlining induces an increase in density.
	double streamFact;
	switch(streamline)
	{
	default:
	case SLINE_None:
	case SLINE_Count:
		streamFact = 1.;
		break;
	case SLINE_Atmospheric:
		streamFact = 1.1;
		break;
	case SLINE_SeaSurface:
		streamFact = 1.2;
		break;
	case SLINE_Submersible:
		streamFact = 1.3;
		break;
	case SLINE_Subterranean:
		streamFact = 1.4;
		break;
	}

	// Armor increases hull mass a lot.
	// The base density is linearly modified by armorLevel.
	return streamFact * (armorLevel + 1);
}

// Quantity of power needed to activate the FTL drive one time.
double AuScDesign::PowerNeededToEngageFTLDrive() const
{
	return PowerNeededToEngageFTLDrive(FTLDriveUnitsToEngage());
}

//static
double AuScDesign::PowerNeededToEngageFTLDrive(double ftlDriveUnitsToEngageCuFt)
{
	// Drive units to engage actually returns cubic feet. An old unit was 256 cubic feet.
	return -.5 + ftlDriveUnitsToEngageCuFt / 256.;
}

// Returns cubic feet of life support needed.
double AuScDesign::LifeSupportNeeded(quint8 lifeSupportQ) const
{
	// A spacecraft must have certain minimum components.
	int berths = CaptBerths() + CrewBerths() + OfficerBerths() + PassengerBerths() + TroopBerths();
	const double tlBoost = QualityBoostF(lifeSupportQ);
	double lifeSupportNeeded = berths * CubicFeetPerCubicMeter() / tlBoost;	// 1 cubic meter of ls per berth.
	if(lifeSupportNeeded < CubicFeetPerCubicMeter())
		lifeSupportNeeded = CubicFeetPerCubicMeter();	// At least one cubic meter of life support is needed.
	return lifeSupportNeeded;
}

quint16 AuScDesign::RoomVoidContainingLocLive(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->RoomVoidContainingLoc(loc, doorStates, landingGearStates, liftStops);
}

quint16 AuScDesign::RoomVoidContainingLocLive(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->RoomVoidContainingLoc(loc, elevate, doorStates, landingGearStates, liftStops);
}

bool AuScDesign::IsInsideObstructionModel(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	return m_Parts->IsInsideObstructionModel(loc, doorStates, landingGearStates);
}

bool AuScDesign::RandomRoomLocation(quint16 roomId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->RandomRoomLocation(roomId, voidLoc, doorStates, landingGearStates, liftStops);
}

bool AuScDesign::RandomRoomLocation(const QString& roomName, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->RandomRoomLocation(roomName, voidLoc, doorStates, landingGearStates, liftStops);
}

quint16 AuScDesign::RandomRoomVoidLocation(AuPoint& voidLoc, quint8 personnelAccess, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->RandomRoomVoidLocation(voidLoc, personnelAccess, doorStates, landingGearStates, liftStops);
}

bool AuScDesign::IsInsideHull(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->IsInsideHull(loc, doorStates, landingGearStates, liftStops);
}

bool AuScDesign::FinalizeAssembly(QStringList& msgs)
{
	// Finalized designs are getting prepared to be new entries in the library. That means an id of 0.
	SetId(0);
	SetState(FinalAssembly);

	// Design report checks to see if the design is ok.
	AuScReport report;
	ReportDesign(report);
	return report.OkForAssembly(msgs, *this);
}

bool AuScDesign::FinalizeDesign(QStringList& msgs, QProgressDialog& progress)
{
	// Finalized designs are getting prepared to be new entries in the library. That means an id of 0.
	SetId(0);
	SetState(Final);

	int partProg = m_Parts->size();
	int lastProg = partProg * 3 + 3;
	progress.setMaximum(lastProg);

	// Remove parts and part states that are not needed in the finalized design.
	progress.setLabelText(QObject::tr("Finalizing parts..."));
	m_Parts->Finalize(true, &progress);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg);	// Done with first pass through parts.

	// Create obstruction model.
	progress.setLabelText(QObject::tr("Updating obstructions..."));
	m_Parts->RefreshObstructionsCacheForFinalize(progress);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*2);	// Done with second pass through parts.

	progress.setLabelText(QObject::tr("Calculating hull volume..."));
	m_HullVolume = m_Parts->HullVolume(progress);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3);	// Done with third pass through parts.

	progress.setLabelText(QObject::tr("Syncing equipment bays and FTL drives..."));
	SyncBaysAndFTLDrives();
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3 + 1);

	// Design report checks to see if the design is ok.
	progress.setLabelText(QObject::tr("Analyzing spacecraft blueprint..."));
	AuScReport report;
	ReportDesign(report);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3 + 2);

	bool isOk = report.OkToManufacture(msgs, this);
	progress.setValue(lastProg);	// Done.
	return isOk;
}

void AuScDesign::FinalizePreview()
{
	// Remove parts and part states that are not needed in the finalized design.
	// Applies hull surface effect textures, for damage and FTL operation.
	m_Parts->Finalize(true, 0);

	// Create obstruction model.
	RefreshObstructionsCacheIfNeeded();
}

// NOTE: If you change this function be sure to change AuScReport::ListComponents.
void AuScDesign::ListComponents(DeProcessComponentArray& pc) const
{
	int commandStations = CommandAndControlStations();
	if(commandStations)
		pc.AddRequirement(CommandAndControlModule(), CE_Required, commandStations);

	int lifeSupportUnits = (int)ceil(m_LifeSupport * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(LifeSupportModule(), CE_Required, lifeSupportUnits);

	int driveUnits = (int)ceil(m_ManeuverDrive * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_Modules.ManeuverDriveModule(), CE_Required, driveUnits);

	int powerPlantUnits = (int)ceil(m_PowerPlant * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_Modules.PowerPlantModule(), CE_Required, powerPlantUnits);

	int capacitorUnits = (int)ceil(m_Capacitor * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_Modules.CapacitorModule(), CE_Required, capacitorUnits);

	int sensorUnits = (int)ceil(m_Sensors * CubicFeetPerCubicMeter() / 256.f);
	if(sensorUnits)
		pc.AddRequirement(SensorModule(), CE_Required, sensorUnits);

	int shieldUnits = (int)ceil(m_Shields * CubicFeetPerCubicMeter() / 256.f);
	if(shieldUnits)
		pc.AddRequirement(ShieldModule(), CE_Required, shieldUnits);

	for(QHash<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		int bayHits = (int)ceil(*bi * CubicFeetPerCubicMeter() / 256.f);
		COMMODITYID bmod = BayModule(bi.key());
		pc.AddRequirement(bmod, CE_Required, bayHits);
	}

	for(QHash<quint16, quint32>::const_iterator bi = m_FTLDrives.cbegin(); bi != m_FTLDrives.cend(); bi++)
	{
		int ftlHits = (int)ceil(*bi * CubicFeetPerCubicMeter() / 256.f);
		COMMODITYID bmod = FTLDriveModule(bi.key());
		pc.AddRequirement(bmod, CE_Required, ftlHits);
	}

	m_Parts->ListComponents(pc, Streamline(), ArmorLevel(), HullMaterial());
}

bool AuScDesign::PartsUpdateHullDoors(const AuScSelection& partIds, AuScParts& partsChanged)
{
	if(!AuDesign::PartsUpdateHullDoors(partIds, partsChanged))
		return false;
	m_HullVolume = 0;
	return true;
}

quint16 AuScDesign::VoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, quint16 exceptVoidId) const
{
	return m_Parts->VoidContainingLoc(loc, doorStates, landingGearStates, liftStops, exceptVoidId);
}

bool AuScDesign::Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, bool canUnlockDoors,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->Route(route, fromLoc, toLoc, LandingOrientation(), canUnlockDoors, doorStates, landingGearStates, liftStops);
}

bool AuScDesign::ClosestEntrance(AuPoint& entranceLoc, quint16& doorId, quint16& ladderId, const AuPoint& loc,
		bool useGroundLevelDoors, bool hullDoorsUsable, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->ClosestEntrance(entranceLoc, doorId, ladderId, loc, LandingOrientation(), useGroundLevelDoors, hullDoorsUsable,
			doorStates, landingGearStates, liftStops);
}

bool AuScDesign::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	return m_Parts->Sight(eyeLoc, sightLoc, sightPart, doorStates, landingGearStates, liftStops, sightSurfaceNormal);
}

double AuScDesign::HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, AuPoint& floorN, bool& inGravityWell,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	inGravityWell = false;
	double floorHt = 0;
	quint16 roomId = VoidContainingLoc(loc, doorStates, landingGearStates, liftStops, 0);
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
			quint16 froomId = VoidContainingLoc(floc, doorStates, landingGearStates, liftStops, roomId);
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

double AuScDesign::HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	double ceilingHt = 0;
	quint16 roomId = VoidContainingLoc(loc, doorStates, landingGearStates, liftStops, 0);
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
			quint16 froomId = VoidContainingLoc(floc, doorStates, landingGearStates, liftStops, roomId);
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

bool AuScDesign::PathLocInsideDoor(quint16 doorId, AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
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
		//if(IsInsideHull(pathLoc, doorStates, landingGearStates, liftStops))
		if(m_Parts->RoomVoidContainingLoc(loc, .1, doorStates, landingGearStates, liftStops))
			return true;
		loc = seg.Loc2();
		//if(IsInsideHull(pathLoc, doorStates, landingGearStates, liftStops))
		if(m_Parts->RoomVoidContainingLoc(loc, .1, doorStates, landingGearStates, liftStops))
			return true;
	}
	return false;
}

//static
double AuScDesign::MaxVolume()
{
	return 255 * 255 * 1600;
}

bool AuScDesign::FindLadderToGrab(const AuPoint& fromLoc, AuMatrix4x4& mLadder, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->FindLadderToGrab(fromLoc, mLadder, doorStates, landingGearStates, liftStops);
}

bool AuScDesign::FindLadderToGrabAt(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return m_Parts->FindLadderToGrabAt(fromLoc, ladderPathLoc, mLadder, doorStates, landingGearStates, liftStops);
}

double AuScDesign::GroundHeightAboveCG(const AuLandingGearStateHash& landingGearStates) const
{
	return m_Parts->GroundHeightAboveCG(landingGearStates, LandingOrientation());
}

unsigned int AuScDesign::CompressedSizeLimit() const
{
	return AuScReport::CompressedSizeLimit();
}

void AuScDesign::ContrailEmitters(QMap<AuPoint, float>& contrails) const
{
	Parts().ContrailEmitters(contrails);
	if(contrails.empty() && Structure() != SC_Station)
	{
		// Default emitter position and size are based on the size of this design.
		AuPoint min, max;
		Parts().HullExtents(min, max);

		// Calculate center of default emitter.
		AuPoint c((min.x + max.x)/2, min.y, (min.z + max.z)/2);

		// Calculate size of default emitter.
		float s = (max.x - min.x)/4.;
		if(s < FeetPerMeter())
			s = (float)FeetPerMeter();

		contrails.insert(c, s);
	}
}

quint64 AuScDesign::MemoryUsage() const
{
	quint64 mem = AuDesign::MemoryUsage();
	mem += sizeof(m_HullVolume);
	mem += sizeof(m_LandingOrient);
	mem += sizeof(m_Modules);
	mem += m_Parts->MemoryUsage();
	mem += sizeof(m_StationOrient);
	mem += sizeof(m_Streamline);
	mem += sizeof(m_Structure);
	mem += sizeof(m_MinQuality);

	mem += sizeof(m_Armor);
	mem += (sizeof(quint16) + sizeof(quint32)) * m_Bays.capacity();
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_CryoBerths);
	mem += (sizeof(quint16) + sizeof(quint32)) * m_FTLDrives.capacity();
	mem += sizeof(m_Fuel);
	mem += sizeof(m_Hold);
	mem += sizeof(m_LifeSupport);
	mem += sizeof(m_ManeuverDrive);
	mem += sizeof(m_PowerPlant);
	mem += sizeof(m_Sensors);
	mem += sizeof(m_Shields);
	mem += sizeof(m_Skeleton);
	return mem;
}

bool AuScDesign::IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	return m_Parts->IsOnLadderTo(fromLoc, ladderPathLoc, doorStates, landingGearStates);
}

int AuScDesign::CommandAndControlStations() const
{
	AuScPartSet stations;
	m_Parts->CommandStations(stations);
	m_Parts->ControlStations(stations);
	return stations.size();
}
