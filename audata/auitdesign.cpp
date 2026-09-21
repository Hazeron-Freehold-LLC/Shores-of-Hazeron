//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitdesign.cpp,v 1.16 2022/04/12 17:09:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitdesign.h"
//
#include "aucombat.h"
#include "auglobal.h"
#include "auitem.h"
#include "auscparts.h"
#include "auitreport.h"
#include "hashpack.h"
#include <QProgressDialog>
#include <QSqlQuery>

#define ITDESIGN_SERIALIZE_VER			0x00

static int s_AuItDesignMetaType = qRegisterMetaType<AuItDesign>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuItDesignHash::LoadArchitect(const AuId& idArchitect)
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Create a query to select all of the architect's designs, except those that the architect has deleted.
	QString sq = QString("SELECT id,name,archname,state,textures, "
			"parts,cid,groundOrient "
			"FROM n_ItDesign WHERE state != 2 and ida=%1;").arg(idArchitect.GetID());
	AuItDesign ob;
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
				qDebug() << "AuItDesignHash::LoadArchitect Z_DATA_ERROR handled ok." << ob.Id() << ob.Name() << ob.ArchitectId()
					<< ob.ArchitectName();
				ba = bac;
			}
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *ob.m_Parts;
		}

		ob.m_CommodityId = (COMMODITYID)qry.value(qv++).toInt();
		ob.m_GroundOrient = (GROUND_ORIENT)qry.value(qv++).toUInt();
		//qDebug() << "AuItDesignHash::LoadArchitect arch" << idArchitect << "loaded" << ob.Name() << ob.DescriptionOneLine();

		// Insert/update the data in this hash.
		iterator di = find(ob.Id());
		if(di == end())
			insert(pair<quint32, AuItDesign>(ob.Id(), ob));
		else
			(*di).second = ob;
	}
}

void AuItDesignHash::LoadSoloInit()
{
	clear();
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Create a query to select all of Haxus' designs that are final.
	AuId idArchitect(AuId::Haxus());
	QString sq = QString("SELECT id,name,archname,state,textures, "
			"parts,cid,groundOrient "
			"FROM n_ItDesign WHERE state = 1 and ida=%1;").arg(idArchitect.GetID());
	AuItDesign ob;
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
				qDebug() << "AuItDesignHash::LoadSoloInit Z_DATA_ERROR handled ok." << ob.Id() << ob.Name() << ob.ArchitectId()
					<< ob.ArchitectName();
				ba = bac;
			}
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *ob.m_Parts;
		}

		ob.m_CommodityId = (COMMODITYID)qry.value(qv++).toInt();
		ob.m_GroundOrient = (GROUND_ORIENT)qry.value(qv++).toUInt();
		//qDebug() << "AuItDesignHash::LoadSoloInit arch" << idArchitect << "loaded" << ob.Name() << ob.DescriptionOneLine();

		// Insert/update the data in this hash.
		iterator di = find(ob.Id());
		if(di == end())
			insert(pair<quint32, AuItDesign>(ob.Id(), ob));
		else
			(*di).second = ob;
	}
}

void AuItDesignHash::Load(quint32 idDesign)
{
	AuItDesign ob;
	ob.Load(idDesign);
	if(ob.Id() != idDesign)
		return;

	// Insert/update the data in this hash.
	iterator di = find(ob.Id());
	if(di == end())
		insert(pair<quint32, AuItDesign>(ob.Id(), ob));
	else
		(*di).second = ob;
}

void AuItDesignHash::operator<<(QDataStream& in)
{
	clear();
	quint16 cnt;
	in >> cnt;
	AuItDesign ship;
	for(; cnt; cnt--)
	{
		ship << in;
		insert(pair<quint32, AuItDesign>(ship.Id(), ship));
	}
}

void AuItDesignHash::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	out << (quint16)size();
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it).second >> out;
}

quint64 AuItDesignHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuItDesignHash);
	for(const_iterator it = cbegin(); it != cend(); it++)
		mem += sizeof(quint32) + (*it).second.MemoryUsage();
	return mem;
}

quint32 AuItDesignHash::HighestId() const
{
	quint32 highest = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).first > highest)
			highest = (*it).first;
	return highest;
}

quint32 AuItDesignHash::SoloInitNextId() const
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

void AuItDesignHash::Merge(const AuItDesignHash& mdh)
{
	// Add designs that are not already here.
	for(const_iterator it = mdh.cbegin(); it != mdh.cend(); it++)
	{
		if(find((*it).first) != end())
			continue;	// I already have that design.
		insert(pair<quint32, AuItDesign>((*it).first, (*it).second));
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItDesign::AuItDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect) :
	AuDesign(idDesign, idArchitect, architect)
{
	m_CommodityId = CID_Invalid;
	m_GroundOrient = GORI_ZPosUp;
	m_Parts = new AuScParts;

	// Build a reasonable default palette.
	quint16 texLibPal[SCTEXI_Count];
	AuTextureLibItem::InitDefaultScPalette(texLibPal);
	AuDesignTex texturePalette[SCTEXI_Count];
	for(int ti=0; ti<SCTEXI_Count; ti++)
		texturePalette[ti].SetTexLibId(texLibPal[ti]);
	SetTexturePalette(texturePalette);
}

AuItDesign::AuItDesign()
{
	m_CommodityId = CID_Invalid;
	m_GroundOrient = GORI_ZPosUp;
	m_Parts = new AuScParts;

	// Build a reasonable default palette.
	quint16 texLibPal[SCTEXI_Count];
	AuTextureLibItem::InitDefaultScPalette(texLibPal);
	AuDesignTex texturePalette[SCTEXI_Count];
	for(int ti=0; ti<SCTEXI_Count; ti++)
		texturePalette[ti].SetTexLibId(texLibPal[ti]);
	SetTexturePalette(texturePalette);
}

AuItDesign::~AuItDesign()
{
	delete m_Parts;
}

AuItDesign::AuItDesign(const AuItDesign& c) : AuDesign(c)
{
	m_CommodityId = c.m_CommodityId;
	m_GroundOrient = c.m_GroundOrient;
	m_Parts = new AuScParts(*c.m_Parts);
}

AuItDesign& AuItDesign::operator=(const AuItDesign& c)
{
	AuDesign::operator=(c);
	m_CommodityId = c.m_CommodityId;
	m_GroundOrient = c.m_GroundOrient;
	*m_Parts = *c.m_Parts;
	return *this;
}

AuDesign* AuItDesign::Clone() const
{
	return new AuItDesign(*this);
}

void AuItDesign::Load(quint32 idDesign)
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	QString sq = QString("SELECT name,ida,archname,state,textures, "
			"parts,cid,groundOrient "
			"FROM n_ItDesign WHERE id=%1;").arg(idDesign);
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
				qDebug() << "AuItDesign::Load Z_DATA_ERROR handled ok." << Id() << Name() << ArchitectId()
					<< ArchitectName();
				ba = bac;
			}
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *m_Parts;
		}

		m_CommodityId = (COMMODITYID)qry.value(qv++).toInt();
		m_GroundOrient = (GROUND_ORIENT)qry.value(qv++).toUInt();
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
			qWarning() << "AuItDesign::Load FAILED to SELECT design" << idDesign;
	}
}

const AuScParts& AuItDesign::Parts() const
{
	return *m_Parts;
}

AuScParts& AuItDesign::Parts()
{
	return *m_Parts;
}

void AuItDesign::ClearParts()
{
	m_Parts->Clear();
}

void AuItDesign::DesignLoaded(const AuDesign& design)
{
	AuDesign::DesignLoaded(design);
	try
	{
		const AuItDesign& c = dynamic_cast<const AuItDesign&>(design);
		m_CommodityId = c.m_CommodityId;
		m_GroundOrient = c.m_GroundOrient;
		*m_Parts = *c.m_Parts;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuItDesign::operator>>(QDataStream& out) const
{
	AuDesign::operator>>(out);
	out << (quint8)ITDESIGN_SERIALIZE_VER;
	out << (quint16)m_CommodityId;
	out << (quint8)m_GroundOrient;
	out << *m_Parts;
}

void AuItDesign::operator<<(QDataStream& in)
{
	AuDesign::operator<<(in);
	quint8 ver;
	in >> ver;
	quint16 cid;
	in >> cid;
	m_CommodityId = (COMMODITYID)cid;
	quint8 gori;
	in >> gori;
	m_GroundOrient = (GROUND_ORIENT)gori;
	in >> *m_Parts;
}

void AuItDesign::Insert(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO n_ItDesign (id,name,ida,archname,state,textures, "
			"parts,cid,groundOrient "
			") VALUES ("
			"?,?,?,?,?,?, "
			"?,?,? "
			");");
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

	qa.addBindValue(m_CommodityId);
	qa.addBindValue(m_GroundOrient);

	if(!qa.exec())
		qWarning() << "error: AuItDesign::Insert failed" << Name() << sq;
}

void AuItDesign::Update(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("UPDATE n_ItDesign SET name=?, ida=?, archname=?, state=?, textures=?, "
			"parts=?, cid=?, groundOrient=? "
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

	qa.addBindValue(m_CommodityId);
	qa.addBindValue(m_GroundOrient);

	if(!qa.exec())
		qWarning() << "error: AuItDesign::Update failed" << Name() << sq;
}

void AuItDesign::Tx(QDataStream& out) const
{
	AuDesign::Tx(out);
	out << (quint8)ITDESIGN_SERIALIZE_VER;
	out << (quint16)m_CommodityId;
	out << (quint8)m_GroundOrient;
	{
		// Store parts using a compressed byte array.
		QByteArray ba;
		QDataStream packout(&ba, QIODevice::WriteOnly);
		packout.setVersion(QDataStream::Qt_5_0);
		packout << *m_Parts;
		QByteArray bac = qCompress(ba);
		out << bac;
	}
}

// NOTE: This function is also used when loading from a file.
void AuItDesign::Rx(QDataStream& in)
{
	AuDesign::Rx(in);
	quint8 ver;
	in >> ver;
	quint16 cid;
	in >> cid;
	m_CommodityId = (COMMODITYID)cid;
	quint8 gori;
	in >> gori;
	m_GroundOrient = (GROUND_ORIENT)gori;
	in >> *m_Parts;
}

void AuItDesign::SetCommodityId(COMMODITYID cid)
{
	m_CommodityId = cid;
}

void AuItDesign::SetGroundOrientation(GROUND_ORIENT gori)
{
	m_GroundOrient = gori;
}

bool AuItDesign::Validate()
{
	//qDebug() << "AuItDesign::Validate";
	bool isValid = AuDesign::Validate();

	//DHH Validate parts.
	//AuScParts	m_Parts;

	//DHH Validate modules.
	//AuScModules	m_Modules;

	return isValid;
}

// Returns the description as a single line.
// Format example: Streamlined Starship Q231 32m^3 (using cubed symbol instead of ^3).
QString AuItDesign::DescriptionOneLine() const
{
	return StructureName();
}

void AuItDesign::ShaderTexturePalette(COMMODITYID hullModule, AuGLEffects& re, AuTexturePalette& textures) const
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

void AuItDesign::SetProperties(const AuDesign& design)
{
	try
	{
		const AuItDesign& c = dynamic_cast<const AuItDesign&>(design);
		m_CommodityId = c.m_CommodityId;
		m_GroundOrient = c.m_GroundOrient;
		//m_Parts = c.m_Parts;
		AuDesign::SetProperties(design);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuItDesign::RefreshObstructionsCache()
{
	m_Parts->RefreshObstructionsCache();
}

void AuItDesign::RefreshObstructionsCache(QProgressDialog& progress)
{
	int partProg = m_Parts->size();
	int lastProg = partProg;
	progress.setMaximum(lastProg);

	progress.setLabelText(QObject::tr("Updating obstructions..."));
	m_Parts->RefreshObstructionsCache(progress);
	if(progress.wasCanceled())
		return;
	progress.setValue(partProg);
}

bool AuItDesign::RefreshObstructionsCacheIfNeeded()
{
	bool needed = m_Parts->RefreshObstructionsCacheIfNeeded();
	return needed;
}

void AuItDesign::ReportDesign(AuItReport& report) const
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
		report.m_CommodityId = m_CommodityId;
		report.m_GroundOrient = m_GroundOrient;
	}

	// Volumes, in cubic meters.
	{
		m_Parts->HullExtents(report.m_HullMin, report.m_HullMax);
	}

	// Calculate values that are based on system volumes.
	report.UpdateCalculatedValues();

	// Assemblies report their parts list.
	if(State() != Final)
		report.m_PartsList = m_Parts->ReportParts(ArchitectId());

	// Faces/triangles.
	report.m_FaceCount = m_Parts->CountFaces();
	report.m_TriangleCount = m_Parts->CountTriangles();
}

QString AuItDesign::ReportAnalysis() const
{
	AuItReport report;
	ReportDesign(report);
	return report.ReportAnalysis(State());
}

bool AuItDesign::FinalizeAssembly(QStringList& msgs)
{
	// Finalized designs are getting prepared to be new entries in the library. That means an id of 0.
	SetId(0);
	SetState(FinalAssembly);

	// Design report checks to see if the design is ok.
	AuItReport report;
	ReportDesign(report);
	return report.OkForAssembly(msgs, *this);
}

bool AuItDesign::FinalizeDesign(QStringList& msgs, QProgressDialog& progress)
{
	// Finalized designs are getting prepared to be new entries in the library. That means an id of 0.
	SetId(0);
	SetState(Final);

	int partProg = m_Parts->size();
	int lastProg = partProg * 2 + 2;
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

	// Design report checks to see if the design is ok.
	progress.setLabelText(QObject::tr("Analyzing spacecraft blueprint..."));
	AuItReport report;
	ReportDesign(report);
	if(progress.wasCanceled())
		return false;
	progress.setValue(partProg*3 + 1);

	bool isOk = report.OkToManufacture(msgs, this);
	progress.setValue(lastProg);	// Done.
	return isOk;
}

void AuItDesign::FinalizePreview()
{
	// Remove parts and part states that are not needed in the finalized design.
	// Applies hull surface effect textures, for damage and FTL operation.
	m_Parts->Finalize(true, 0);

	// Create obstruction model.
	RefreshObstructionsCacheIfNeeded();
}

void AuItDesign::ListComponents(DeProcessComponentArray& pc) const
{
	Q_UNUSED(pc);
}

bool AuItDesign::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart,
		AuPoint& sightSurfaceNormal) const
{
	AuDoorStateHash doorStates;
	AuLandingGearStateHash landingGearStates;
	AuLiftHash liftStops;
	return m_Parts->Sight(eyeLoc, sightLoc, sightPart, doorStates, landingGearStates, liftStops, sightSurfaceNormal);
}

double AuItDesign::GroundHeightAboveCG() const
{
	return m_Parts->GroundHeightAboveCG(m_GroundOrient);
}

unsigned int AuItDesign::CompressedSizeLimit() const
{
	return AuItReport::CompressedSizeLimit();
}

quint64 AuItDesign::MemoryUsage() const
{
	quint64 mem = AuDesign::MemoryUsage();
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_GroundOrient);
	mem += m_Parts->MemoryUsage();
	return mem;
}

//static
double AuItDesign::ModelSizeLimit()
{
	return FeetPerMeter();
}

QString AuItDesign::StructureName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Item");
	else
		return g_Au.CommodityName(m_CommodityId);
}

