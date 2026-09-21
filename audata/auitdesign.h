//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitdesign.h,v 1.8 2022/02/19 18:19:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItDesign_H
#define AuItDesign_H

#include "audesign.h"
#include "auhash.h"
#include "dbcommodity.h"

class AuGLEffects;
class AuMatrix4x4;
class AuItReport;
class DeProcessComponentArray;
class QProgressDialog;
class QSqlDatabase;

class AUDATA_DLL AuItDesign : public AuDesign
{
friend class AuItDesignHash;
	// Design Properties
	COMMODITYID							m_CommodityId;			// Commodity intended for this design. Not a restriction.
	GROUND_ORIENT						m_GroundOrient;			// Preferred orientation when lying on the ground.
	AuScParts*							m_Parts;

public:	// Constructors
	AuItDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect);
	AuItDesign();
	virtual ~AuItDesign();
	AuItDesign(const AuItDesign &c);

public:	// Operators
	virtual AuItDesign& operator=(const AuItDesign &c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	virtual AuDesign* Clone() const;

	virtual void ClearParts();

	virtual unsigned int CompressedSizeLimit() const;

	// Copies only the fields the server wants from a design record loaded from a file is sent to the server.
	virtual void DesignLoaded(const AuDesign& design);

	// Returns the description as a single line.
	// Format example: Streamlined Starship 32m^3 (using cubed symbol instead of ^3).
	QString DescriptionOneLine() const;

	virtual bool FinalizeAssembly(QStringList& msgs);

	virtual bool FinalizeDesign(QStringList& msgs, QProgressDialog& progress);

	virtual void FinalizePreview();

	double GroundHeightAboveCG() const;

	inline COMMODITYID CommodityId() const
	{
		return m_CommodityId;
	}

	virtual GROUND_ORIENT GroundOrientation() const
	{
		return m_GroundOrient;
	}

	inline QString GroundOrientationName() const
	{
		return ::GroundOrientationName(GroundOrientation());
	}

	virtual void ListComponents(DeProcessComponentArray& pc) const;

	void Load(quint32 idDesign);

	virtual quint64 MemoryUsage() const;

	// This is the box radius of the largest possible item model.
	static double ModelSizeLimit();

	virtual const AuScParts& Parts() const;

	virtual AuScParts& Parts();

	virtual void RefreshObstructionsCache();
	virtual void RefreshObstructionsCache(QProgressDialog& progress);
	virtual bool RefreshObstructionsCacheIfNeeded();

	virtual QString ReportAnalysis() const;

	// Builds the data in 'report' based on this design as equipped.
	void ReportDesign(AuItReport& report) const;

	void SetCommodityId(COMMODITYID cid);

	void SetGroundOrientation(GROUND_ORIENT gori);

	virtual void SetProperties(const AuDesign& design);

	// Fills 'textures' with the OpenGL texture ids expected by the Sc shaders.
	// Loads the textures if needed.
	// An OpenGL context must be current when this function is called.
	// Note the 'textures' array is larger than the m_TexturePalette. Palette textures are looked up/loaded to fill most of the
	// entries in the 'textures' array.
	virtual void ShaderTexturePalette(COMMODITYID hullModule, AuGLEffects& re, AuTexturePalette& textures) const;

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// Returns true if the sight segment hits the design. 'sightPart' is the specific part that is hit.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit, and 'sightSurfaceNormal' is set.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, AuPoint& sightSurfaceNormal) const;

	virtual QString StructureName() const;

	void Insert(QSqlDatabase& db) const;
	void Update(QSqlDatabase& db) const;

	virtual void Rx(QDataStream& in);
	virtual void Tx(QDataStream& out) const;

	virtual BLUEPRINT_TYPE Type() const
	{
		return TypeStatic();
	}

	static BLUEPRINT_TYPE TypeStatic()
	{
		return BLUEPRINT_Item;
	}

	// Modifies values of this record as necessary to make it valid.
	// Returns true if the record was already valid and no changes are required.
	// Returns false if this record was invalid and changes were made to make it valid.
	virtual bool Validate();
};

inline QDataStream& operator<<(QDataStream& out, const AuItDesign& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItDesign& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuItDesign)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Key is design id, value is the design.
class AUDATA_DLL AuItDesignHash : public unordered_map<quint32, AuItDesign, AuDesignIdHash>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint32 HighestId() const;

	void Load(quint32 idDesign);

	// Loads all designs by 'idArchitect'.
	void LoadArchitect(const AuId& idArchitect);

	// Loads all designs that will be stored in the solo init GS file.
	void LoadSoloInit();

	quint64 MemoryUsage() const;

	void Merge(const AuItDesignHash& mdh);

	quint32 SoloInitNextId() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuItDesignHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItDesignHash& o)
{
	o << in;
	return in;
}

#endif // AuItDesign_H
