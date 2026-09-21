//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitreport.cpp,v 1.2 2022/02/19 18:20:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitreport.h"
//
#include "aucombat.h"
#include "auglobal.h"
#include "auitem.h"
#include "auitdesign.h"
#include "auscparts.h"
#include "dgdata.h"

#define REPORT_SERIALIZE_VER			0x00

AuItReport::AuItReport()
{
	// Design
	//m_ArchitectId;
	//m_ArchitectName;
	//m_DesignDesc;
	m_DesignId = 0;
	//m_DesignName;
	//m_PartArchitects;

	// Structure
	m_CommodityId = CID_Invalid;
	m_GroundOrient = GORI_ZPosUp;

	// Volumes, in cubic meters.
	//m_HullMax;
	//m_HullMin;

	// Analysis
	m_FaceCount = 0;
	m_TriangleCount = 0;
}

AuItReport::~AuItReport()
{
}

AuItReport::AuItReport(const AuItReport& c) : m_PartArchitects(c.m_PartArchitects)
{
	// Design
	m_ArchitectId = c.m_ArchitectId;
	m_ArchitectName = c.m_ArchitectName;
	m_DesignDesc = c.m_DesignDesc;
	m_DesignId = c.m_DesignId;
	m_DesignName = c.m_DesignName;

	// Structure
	m_CommodityId = c.m_CommodityId;
	m_GroundOrient = c.m_GroundOrient;

	// Volumes, in cubic meters.
	m_HullMax = c.m_HullMax;
	m_HullMin = c.m_HullMin;

	// Analysis
	m_FaceCount = c.m_FaceCount;
	m_PartsList = c.m_PartsList;
	m_TriangleCount = c.m_TriangleCount;
}

const AuItReport& AuItReport::operator=(const AuItReport& c)
{
	// Design
	m_ArchitectId = c.m_ArchitectId;
	m_ArchitectName = c.m_ArchitectName;
	m_DesignDesc = c.m_DesignDesc;
	m_DesignId = c.m_DesignId;
	m_DesignName = c.m_DesignName;
	m_PartArchitects = c.m_PartArchitects;

	// Structure
	m_CommodityId = c.m_CommodityId;
	m_GroundOrient = c.m_GroundOrient;

	// Volumes, in cubic meters.
	m_HullMax = c.m_HullMax;
	m_HullMin = c.m_HullMin;

	// Analysis
	m_FaceCount = c.m_FaceCount;
	m_PartsList = c.m_PartsList;
	m_TriangleCount = c.m_TriangleCount;
	return *this;
}

//static
unsigned int AuItReport::CompressedSizeLimit()
{
	return 100000;
}

//static
unsigned int AuItReport::CompressedSizeLimitPerc()
{
	return CompressedSizeLimit()/100;
}

//static
unsigned int AuItReport::SizeLimit()
{
	return 1000000;
}

//static
unsigned int AuItReport::SizeLimitPerc()
{
	return SizeLimit()/100;
}

bool AuItReport::OkForAssembly(QStringList& msgs, const AuItDesign& design) const
{
	QString error("<span style=\"color: rgb(255, 32, 32);\">Error: %1</span>");
	QString note("<span style=\"color: rgb(32, 255, 32);\">%1</span>");
	QString warning("<span style=\"color: rgb(255, 128, 32);\">Warning: %1</span>");

	bool isOk = true;

	if(m_DesignName.isEmpty())
	{
		msgs.append(error.arg(QObject::tr("Design name is blank.")));
		isOk = false;
	}
	else if(!IsNameValid(m_DesignName))
	{
		msgs.append(error.arg(QObject::tr("Design name <b>%1</b> is invalid.").arg(m_DesignName)));
		isOk = false;
	}

	if(design.Parts().empty())
	{
		msgs.append(error.arg(QObject::tr("No parts. Add at least one part.")));
		isOk = false;
	}

	// If the uncompressed data gets too large, the servers run out of memory.
	// If the compressed data gets too large, the SQL transaction fails.
	{
		quint64 memSize = design.MemoryUsage();
		int perc = memSize / SizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("%L1 bytes blueprint data exceeds the size limit of %L2 bytes. "
							"Reduce the number of faces and custom textures. "))
					.arg(memSize)
					.arg(SizeLimit()));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes. "
							"Conserve the number of faces and custom textures.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));

		QByteArray bac(qCompress(design.Pack()));
		perc = bac.size() / CompressedSizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("When compressed, %L1 bytes blueprint data exceeds the compressed size limit "
							"of %L2 bytes. Reduce the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes. Conserve the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
	}

	if(isOk)
		msgs.append(note.arg(QObject::tr("Success! The item blueprint can be used as an assembly.")));

	return isOk;
}

bool AuItReport::OkToManufacture(QStringList& msgs, const AuItDesign* design) const
{
	QString error("<span style=\"color: rgb(255, 32, 32);\">Error: %1</span>");
	QString note("<span style=\"color: rgb(32, 255, 32);\">%1</span>");
	QString warning("<span style=\"color: rgb(255, 128, 32);\">Warning: %1</span>");

	bool isOk = true;

	if(m_DesignName.isEmpty())
	{
		msgs.append(error.arg(QObject::tr("Design name is blank.")));
		isOk = false;
	}
	else if(!IsNameValid(m_DesignName))
	{
		msgs.append(error.arg(QObject::tr("Design name <b>%1</b> is invalid.").arg(m_DesignName)));
		isOk = false;
	}

	// If the uncompressed data gets too large, the servers run out of memory.
	// If the compressed data gets too large, the SQL transaction fails.
	if(design)
	{
		quint64 memSize = design->MemoryUsage();
		int perc = memSize / SizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("%L1 bytes blueprint data exceeds the size limit of %L2 bytes. "
							"Reduce the number of faces and custom textures. "))
					.arg(memSize)
					.arg(SizeLimit()));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes. "
							"Conserve the number of faces and custom textures.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));

		QByteArray bac(qCompress(design->Pack()));
		perc = bac.size() / CompressedSizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("When compressed, %L1 bytes blueprint data exceeds the compressed size limit "
							"of %L2 bytes. Reduce the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes. Conserve the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
	}

	// Overall model bounds.
	if(design)
	{
		AuPoint modelMin, modelMax;
		design->Parts().ModelExtents(modelMin, modelMax);
		if(modelMin.x < -AuItDesign::ModelSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds to the left. "
							"Move the left side of the model to the right or move the whole model right.")));
			isOk = false;
		}

		if(modelMax.x > AuItDesign::ModelSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds to the right. "
							"Move the right side of the model to the left or move the whole model left.")));
			isOk = false;
		}

		if(modelMin.y < -AuItDesign::ModelSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds in back. "
							"Move the back side of the model forward or move the whole model forward.")));
			isOk = false;
		}

		if(modelMax.y > AuItDesign::ModelSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds in front. "
							"Move the front side of the model back or move the whole model back.")));
			isOk = false;
		}

		if(modelMin.z < -AuItDesign::ModelSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model out of bounds below. "
							"Move the bottom of the model up or move the whole model up.")));
			isOk = false;
		}

		if(modelMax.z > AuItDesign::ModelSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model out of bounds above. "
							"Move the top of the model down or move the whole model down.")));
			isOk = false;
		}
	}

	// Check proper parts in design.
	if(design && isOk)
	{
		QSet<quint16> dups;					// Ids of parts that have exact duplicates, so we don't report them twice.
		for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend() && isOk; pi++)
		{
			if((*pi)->IsOmitted())
				continue;

			if(!dynamic_cast<AuScMesh*>(*pi))
			{
				msgs.append(error.arg(QObject::tr("%1 %2 is not permitted in an item design. Try converting it to a mesh.")
							.arg((*pi)->PartName())
							.arg(pi.key())));
				isOk = false;
				break;
			}

			// Check for duplicates.
			for(AuScParts::const_iterator di = design->Parts().cbegin(); di != design->Parts().cend(); di++)
			{
				if(pi.key() == di.key())
					continue;	// Don't check a part against itself.
				if(dups.contains(di.key()))
					continue;	// Already reported that part.
				if(*(*pi) != *(*di))
					continue;
				dups.insert(pi.key());
				dups.insert(di.key());
				msgs.append(error.arg(QObject::tr("Part %1 %2 is an exact duplicate of part %3 %4. Delete one of them.")
							.arg(pi.key())
							.arg((*pi)->PartName())
							.arg(di.key())
							.arg((*di)->PartName())));
				isOk = false;
				break;
			}

			if(!isOk)
				msgs.append(warning.arg(QObject::tr("Skipping remainder of detailed analysis due to error, to save time.")));
		}
	}
	else if(design)
		msgs.append(warning.arg(QObject::tr("Skipping detailed analysis due to error, to save time.")));

	if(isOk && design)
		msgs.append(note.arg(QObject::tr("Success! An item can be manufactured using the blueprint.")));

	return isOk;
}

QString AuItReport::ReportAnalysis(quint8 designState) const
{
	if(designState == AuDesign::Deleted)
		return QObject::tr("Deleted Design");
	else if(designState == AuDesign::Edit && m_HullMin == m_HullMax)
		return "";		// Return an empty string. That cues the caller to provide helpful design tips in place of this report.

	// Design name
	// This name includes the architect name. e.g. "Scout Ship by Bob"
	// The working design will be blank.
	QString stat;	// This will be our return value.
	if(!m_DesignName.isEmpty() && !m_ArchitectName.isEmpty())
		stat = QString("<b>%1 by %2</b><br>").arg(m_DesignName).arg(m_ArchitectName);
	else if(!m_DesignName.isEmpty())
		stat = QString("<b>%1</b><br>").arg(m_DesignName);
	else if(!m_ArchitectName.isEmpty())
		stat = QString("<b>by %1</b><br>").arg(m_ArchitectName);

	if(!m_PartArchitects.Empty())
	{
		// Sort collaborating architects alphabetically.
		QMultiMap<QString, AuId> sortedNames;
		for(QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Cbegin(); ai != m_PartArchitects.Cend(); ai++)
			sortedNames.insert((*ai).toUpper(), ai.key());

		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Additional Architects");
		stat += "<div style=\"margin-left: 10px;\">";
		for(QMultiMap<QString, AuId>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
		{
			QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Find(*si);
			if(ai != m_PartArchitects.Cend())
				stat += QString("%1 <i>%2</i><br>").arg(*ai).arg(ai.key().StringId());
		}
		//stat = stat.left(stat.count() - 4);	// Drop the trailing '<br>'.
		stat += "</div>";
		stat += "</div>";
	}

	// Description.
	if(!m_DesignDesc.isEmpty())
	{
		stat += QObject::tr("<b>DESCRIPTION</b>");
		stat += "<div style=\"margin-left: 10px;\">";
		stat += m_DesignDesc;
		stat += "</div>";
		stat += "<hr>";
	}

	// Watch for empty design.
	if(m_HullMin != m_HullMax)
	{
		// Summary
		{
			stat += "<b>";
			stat += QObject::tr("ITEM");
			stat += "</b>";

			stat += "<div style=\"margin-left: 10px;\">";
			if(CommodityId() != CID_Invalid)
				stat += QObject::tr("Intended Use <b>%1</b><br>").arg(g_Au.CommodityName(CommodityId()));

			stat += QObject::tr("Ground Orientation: %1")
				.arg(::GroundOrientationName(GroundOrientation()));

			stat += "</div>";

			if(designState == AuDesign::Edit)
			{
				// Provide the warning and error messages that the finalizer shows.
				QStringList msgs;
				OkToManufacture(msgs, 0);
				if(!msgs.empty())
				{
					stat += "<div style=\"margin-left: 10px;\">";
					stat += QObject::tr("Notes");
					stat += "<div style=\"margin-left: 10px;\">";
					for(QStringList::const_iterator ci = msgs.cbegin(); ci != msgs.cend(); ci++)
					{
						if(ci != msgs.cbegin())
							stat += "<br>";
						stat += *ci;
					}
					stat += "</div>";
					stat += "</div>";
				}
			}
		}
	}

	// Append parts list, if there is one. It is already formatted the way we like.
	stat += m_PartsList;

	return stat;
}

// This function is used by AuItemScDesign::HtmlToolTip.
QString AuItReport::ReportToolTip() const
{
	QString stat;	// This will be our return value.

	// Design name
	// This name includes the architect name. e.g. "Scout Ship by Bob"
	// The working design will be blank.
	if(!m_DesignName.isEmpty() && !m_ArchitectName.isEmpty())
		stat = QString("<b>%1 by %2</b><br>").arg(m_DesignName).arg(m_ArchitectName);
	else if(!m_DesignName.isEmpty())
		stat = QString("<b>%1</b><br>").arg(m_DesignName);
	else if(!m_ArchitectName.isEmpty())
		stat = QString("<b>by %1</b><br>").arg(m_ArchitectName);

	if(!m_PartArchitects.Empty())
	{
		// Sort collaborating architects alphabetically.
		QMultiMap<QString, AuId> sortedNames;
		for(QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Cbegin(); ai != m_PartArchitects.Cend(); ai++)
			sortedNames.insert((*ai).toUpper(), ai.key());

		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Additional Architects");
		stat += "<div style=\"margin-left: 10px;\">";
		for(QMultiMap<QString, AuId>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
		{
			QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Find(*si);
			if(ai != m_PartArchitects.Cend())
				stat += QString("%1 <i>%2</i><br>").arg(*ai).arg(ai.key().StringId());
		}
		//stat = stat.left(stat.count() - 4);	// Drop the trailing '<br>'.
		stat += "</div>";
		stat += "</div>";
	}

	// Description.
	if(!m_DesignDesc.isEmpty())
	{
		stat += QObject::tr("<b>DESCRIPTION</b>");
		stat += "<div style=\"margin-left: 10px;\">";
		stat += m_DesignDesc;
		stat += "</div>";
		stat += "<hr>";
	}

	// Summary
	{
		stat += "<b>";
		stat += QObject::tr("ITEM");
		stat += "</b>";

		stat += "<div style=\"margin-left: 10px;\">";
		if(CommodityId() != CID_Invalid)
			stat += QObject::tr("Intended Use <b>%1</b><br>").arg(g_Au.CommodityName(CommodityId()));

		stat += QObject::tr("Ground Orientation: %1")
			.arg(::GroundOrientationName(GroundOrientation()));

		stat += "</div>";
	}
	return stat;
}

void AuItReport::operator>>(QDataStream& out) const
{
	out << (quint8)REPORT_SERIALIZE_VER;

	// Design
	out << m_ArchitectId;
	out << m_ArchitectName;
	out << m_DesignId;
	out << m_DesignName;
	out << m_PartArchitects;
	out << m_DesignDesc;

	// Structure
	out << (quint16)m_CommodityId;
	out << (quint8)m_GroundOrient;

	// Volumes, in cubic meters.
	out << m_HullMax;
	out << m_HullMin;

	// Analysis
	out << m_FaceCount;
	out << m_PartsList;
	out << m_TriangleCount;
}

void AuItReport::operator<<(QDataStream& in)
{
	quint8 ver;
	in >> ver;

	// Design
	in >> m_ArchitectId;
	in >> m_ArchitectName;
	in >> m_DesignId;
	in >> m_DesignName;
	in >> m_PartArchitects;
	in >> m_DesignDesc;

	// Structure
	quint16 cid;
	in >> cid;
	m_CommodityId = (COMMODITYID)cid;
	quint8 gori;
	in >> gori;
	m_GroundOrient = (GROUND_ORIENT)gori;

	// Volumes, in cubic meters.
	in >> m_HullMax;
	in >> m_HullMin;

	// Analysis
	in >> m_FaceCount;
	in >> m_PartsList;
	in >> m_TriangleCount;
}

bool AuItReport::operator==(const AuItReport& c) const
{
	// Design
	if(m_ArchitectId != c.m_ArchitectId)
		return false;
	if(m_ArchitectName != c.m_ArchitectName)
		return false;
	if(m_DesignDesc != c.m_DesignDesc)
		return false;
	if(m_DesignId != c.m_DesignId)
		return false;
	if(m_DesignName != c.m_DesignName)
		return false;
	if(m_PartArchitects != c.m_PartArchitects)
		return false;

	// Structure
	if(m_CommodityId != c.m_CommodityId)
		return false;
	if(m_GroundOrient != c.m_GroundOrient)
		return false;

	// Volumes, in cubic meters.
	if(m_HullMax != c.m_HullMax)
		return false;
	if(m_HullMin != c.m_HullMin)
		return false;

	// Analysis
	if(m_FaceCount != c.m_FaceCount)
		return false;
	if(m_PartsList != c.m_PartsList)
		return false;
	if(m_TriangleCount != c.m_TriangleCount)
		return false;

	return true;
}

QString AuItReport::DescLine() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Item");
	else
		return g_Au.CommodityName(m_CommodityId);
}

void AuItReport::UpdateCalculatedValues()
{
}

double AuItReport::RoughHullRadius() const
{
	double max2 = m_HullMax.Length2();
	double min2 = m_HullMin.Length2();
	if(max2 > min2)
		return sqrt(max2);
	else
		return sqrt(min2);
}

quint64 AuItReport::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ArchitectId);
	mem += StringMemoryUsage(m_ArchitectName);
	mem += StringMemoryUsage(m_DesignDesc);
	mem += sizeof(m_DesignId);
	mem += StringMemoryUsage(m_DesignName);
	mem += m_PartArchitects.MemoryUsage();

	mem += sizeof(m_CommodityId);
	mem += sizeof(m_GroundOrient);

	mem += sizeof(m_HullMax);
	mem += sizeof(m_HullMin);

	mem += sizeof(m_FaceCount);
	mem += StringMemoryUsage(m_PartsList);
	mem += sizeof(m_TriangleCount);
	return mem;
}

int AuItReport::CommodityCanUse(const DbCommodity& dbc) const
{
	Q_UNUSED(dbc);
	return 0;
}

//static
QString AuItReport::CommodityCanUseMessage(int resultCode)
{
	QString resultMsg[] =
	{
		QObject::tr("Ok"),
	};
	int rc = sizeof(resultMsg) / sizeof(QString);
	if(resultCode >= rc)
		return QObject::tr("Failed for unknown reason %1.").arg(resultCode);
	return resultMsg[resultCode];
}

