//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitreport.h,v 1.2 2022/02/19 18:19:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItReport_H
#define AuItReport_H

#include "audata.h"
#include "auscreporthelpers.h"
#include "demanufacturingprocess.h"

class AuKnownPatents;
class AuItDesign;
class AuScParts;
class DbCommodity;

class AUDATA_DLL AuItReport
{
friend class AuItDesign;
	// Design
	AuId								m_ArchitectId;			// The architect of the design.
	QString								m_ArchitectName;
	QString								m_DesignDesc;
	quint32								m_DesignId;
	QString								m_DesignName;
	AuPartArchitectHash					m_PartArchitects;		// Architects of parts that differ from the architect of the design.

	// Structure
	COMMODITYID							m_CommodityId;			// Commodity intended for this design. Not a restriction.
	GROUND_ORIENT						m_GroundOrient;			// Preferred orientation when lying on the ground.

	// Volumes, in cubic meters.
	AuPoint								m_HullMax;
	AuPoint								m_HullMin;

	// Analysis
	quint32								m_FaceCount;
	QString								m_PartsList;			// Only on finalized assemblies.
	quint32								m_TriangleCount;

public:	// Constructors
	AuItReport();
	virtual ~AuItReport();
	AuItReport(const AuItReport& c);

public:	// Operators
	const AuItReport& operator=(const AuItReport& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const AuItReport& c) const;

	inline bool operator!=(const AuItReport& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline const AuId& ArchitectId() const
	{
		return m_ArchitectId;
	}

	inline const QString& ArchitectName() const
	{
		return m_ArchitectName;
	}

	// Returns 0 if this item design can be used for a specific commodity 'dbc'.
	// Returns non-zero failure code if this item design cannot be sued for a specific industry 'dbc'.
	// See .cpp file for list of failure codes.
	int CommodityCanUse(const DbCommodity& dbc) const;

	// Returns a string version of the result code returned by CommodityCanUse.
	static QString CommodityCanUseMessage(int resultCode);

	inline COMMODITYID CommodityId() const
	{
		return m_CommodityId;
	}

	static unsigned int CompressedSizeLimit();

	static unsigned int CompressedSizeLimitPerc();

	// Returns a brief descriptive line of text.
	QString DescLine() const;

	// This is the design description stored on the design by the architect.
	inline const QString& DesignDescription() const
	{
		return m_DesignDesc;
	}

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline const QString& DesignName() const
	{
		return m_DesignName;
	}

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		max = m_HullMax;
		min = m_HullMin;
	}

	inline quint32 FaceCount() const
	{
		return m_FaceCount;
	}

	inline GROUND_ORIENT GroundOrientation() const
	{
		return m_GroundOrient;
	}

	quint64 MemoryUsage() const;

	// Returns true if the design is valid and ready to be used as an assembly. The only real requirement is a name and a part.
	// 'msgs' is filled with warnings and errors reported during the check, with html coloring.
	bool OkForAssembly(QStringList& msgs, const AuItDesign& design) const;

	// Returns true if the design is valid and can be manufactured as a game object.
	// 'msgs' is filled with warnings and errors reported during the check, with html coloring.
	// 'design' is needed for a true analysis. The ReportAnalysis function calls this with a null design, for warnings and errors.
	bool OkToManufacture(QStringList& msgs, const AuItDesign* design) const;

	// Returns the full detailed design analysis report with html formatting.
	// 'designState' is the AuDesign::STATE of the design being reported.
	QString ReportAnalysis(quint8 designState) const;

	// Returns a minimal description for use on tool tips.
	QString ReportToolTip() const;

	// Returns a radius that encompasses all of my points, measured from my own origin.
	double RoughHullRadius() const;

	static unsigned int SizeLimit();

	static unsigned int SizeLimitPerc();

	inline quint32 TriangleCount() const
	{
		return m_TriangleCount;
	}

	// Updates stored values that are calculated based on system volumes.
	void UpdateCalculatedValues();
};

inline QDataStream& operator<<(QDataStream& out, const AuItReport& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItReport& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuItReport)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuItReport_H
