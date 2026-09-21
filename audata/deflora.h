//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deflora.h,v 1.82 2022/02/19 18:19:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// DePlanetFlora - Describes the various plants found on a habitable planet.
// DeTerrainFlora - Describes the various plants found in a terrain type.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#ifndef DeFlora_H
#define DeFlora_H

#include "audata.h"
#include "aufloradna.h"
#include "auglsl.h"
#include "aupoint.h"
#include "dbcommodity.h"

class AuDice;
class AuId;
class AuGLEffects;
class AuGLVAO;
class AuTime;

// Plant textures are combined into a palette that is FT_PaletteHeight
// tiles tall and FT_PaletteWidth tiles wide. Plant and bark textures are
// indexed top to bottom as follows in the palette; index FT_PaletteHeight is
// the first tile in the second column.
//
// Plants              Sub Texture Index
// ------------------- -----------------
// 11 Polar Shrubs     0-10
//  6 Polar Trees      11-16
//  1 Polar Grass      17
// 11 Temperate Shrubs 18-28
//  6 Temperate Trees  29-34
//  1 Temperate Grass  35
// 11 Tropical Shrubs  36-46
//  6 Tropical Trees   47-52
//  1 Tropical Grass   53
//  8 Bark Textures    54-61
#define ZONE_SHRUBS		11	// Number of species per zone.
#define ZONE_TREES		6	// Number of species per zone.
enum FLORA_TEXTURE
{
	FT_PolarShrubs = 0,
	FT_PolarTrees = 11,
	FT_PolarGrass = 17,
	FT_TemperateShrubs = 18,
	FT_TemperateTrees = 29,
	FT_TemperateGrass = 35,
	FT_TropicalShrubs = 36,
	FT_TropicalTrees = 47,
	FT_TropicalGrass = 53,
	FT_BarkStart = 64 - 8,	// Last column in our 8x8 palette.
	FT_PaletteHeight = 8,	// vertical tiles in texture palette
	FT_PaletteWidth = 8,	// horizontal tiles in texture palette
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Flora Vertex Array Object vertex.

class AuFloraVAOVert
{
public:
	float								m_MoveFactor;			// Movement factor multipled by wind vector.
	float								m_NX;					// Normal X.
	float								m_NY;					// Normal Y.
	float								m_NZ;					// Normal Z.
	float								m_TexX;					// Texture coordinate X.
	float								m_TexY;					// Texture coordinate Y.
	float								m_X;					// Vertex location X.
	float								m_Y;					// Vertex location Y.
	float								m_Z;					// Vertex location Z.

public:
	AuFloraVAOVert(float x, float y, float z, float nx, float ny, float nz, float texX, float texY, float moveFactor)
	{
		m_MoveFactor = moveFactor;
		m_NX = nx;
		m_NY = ny;
		m_NZ = nz;
		m_TexX = texX;
		m_TexY = texY;
		m_X = x;
		m_Y = y;
		m_Z = z;
	}

	AuFloraVAOVert()
	{
		m_MoveFactor = 0;
		m_NX = 0;
		m_NY = 0;
		m_NZ = 0;
		m_TexX = 0;
		m_TexY = 0;
		m_X = 0;
		m_Y = 0;
		m_Z = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Flora Vertex Array Object triangle.

class AuFloraVAOTri
{
public:
	AuFloraVAOVert						m_V0;
	AuFloraVAOVert						m_V1;
	AuFloraVAOVert						m_V2;

public:
	AuFloraVAOTri(const AuFloraVAOVert& v0, const AuFloraVAOVert& v1, const AuFloraVAOVert& v2)
	{
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A node of a branch of a plant.

class DhPlantNode
{
public:
	AuPoint			m_Loc;		// 2D, z is not used.
	float			m_Dia;		// Diameter of branch at node.
	unsigned int	m_Branches;	// Branches emanating from this node, including trunk.

public:
	DhPlantNode()
	{
		m_Dia = 0.f;
		m_Branches = 0;
	}
	DhPlantNode(const AuPoint& loc, float dia, unsigned int branches)
	{
		m_Loc = loc;
		m_Dia = dia;
		m_Branches = branches;
	}
	void Transform(const AuMatrix4x4& t);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A branch of a plant.

class DhPlantBranch : public QVector<DhPlantNode>
{
public:
	void AddNode(const AuPoint& loc, float dia, unsigned int branches);
	void Extents(AuPoint& min, AuPoint& max) const;
	void Transform(const AuMatrix4x4& t);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// All the branches of a plant.

class DhPlantBranches : public QVector<DhPlantBranch>
{
public:
	void Extents(AuPoint& min, AuPoint& max) const;
	void Transform(const AuMatrix4x4& t);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AUDATA_DLL DeFlora
{
public:
	enum TYPE
	{
		GroundCover,
		Shrub,
		Tree
	};

private:
	AuFloraDNA							m_DNA;					// The DNA of the plant.
	float								m_TextureSize;			// World edge size of texture after painting

public:
	DeFlora();
	~DeFlora();

	inline bool operator==(const DeFlora& c) const
	{
		return m_DNA == c.m_DNA;
	}

	inline bool operator!=(const DeFlora& c) const
	{
		return !operator==(c);
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

public:
	void CalculateTextureSizes(const AuDice& dice);

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	// 'maxRadius' is used by potted plants. It will be 0 for wild plants.
	void CreateVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int textureIndex, TYPE type, const AuDice& dice,
			int lod, float maxRadius) const;

	const AuFloraDNA& DNA() const
	{
		return m_DNA;
	}

	void Init(AuFloraDNA::ZONE zone, const AuDice& dice, int sizemod);

	void LoadTextureSize(QDataStream& in);

	// Sets m_TextureSize
	void PaintBranch(QImage& im, AuFloraDNA::ZONE zone, const AuDice& dice, int leafBrushImage, int petalBrushImage);

	void PaintGroundCover(QImage& im, quint32 genSeed, AuFloraDNA::ZONE zone, const AuDice& dice, const AuTime& floraChange);

	void RxUpdate(QDataStream& fromServer);

	inline void SetFloraDNA(const AuFloraDNA& dna)
	{
		m_DNA = dna;
	}

	void StoreTextureSize(QDataStream& out) const;

	void TxUpdate(QDataStream& toClient) const;

protected:
	float CreateBranches(DhPlantBranches& branches, float dia, int maxsegs, float mindia, const AuDice& dice) const;

	// 'lod' is a level-of-detail factor, currently 1 or 2.
	void CreateBranchVAOTriangles(QList<AuFloraVAOTri>& tris, const AuPoint& loc0, float dia0, const AuPoint& loc1, float dia1,
			float bs, float bt, float bu, float bv, int lod) const;

	// 'lod' is a level-of-detail factor, currently 1 or 2.
	void CreateBranchVAOTriangles(QList<AuFloraVAOTri>& tris, const AuPoint& loc, float dia, AuPoint& dir,
			float s, float t, float u, float v, int lod) const;

	// 'lod' is a level-of-detail factor, currently 1 or 2.
	// 'maxRadius' is used by potted plants. It will be 0 for wild plants.
	void CreateVAOTriangles(QList<AuFloraVAOTri>& tris, unsigned int textureIndex, TYPE type, const AuDice& dice, int lod,
			float maxRadius) const;

	// 'lod' is a level-of-detail factor, currently 1 or 2.
	void DrawBranchVAOTriangles(QList<AuFloraVAOTri>& tris, float s, float t, float u, float v, const AuMatrix4x4& m,
			int lod) const;

	void MakeBranch(DhPlantBranch& branch, const AuPoint& baseLoc, float dia, const AuPoint& startVec, int maxangle, int maxsegs,
			float mindia, const AuDice& dice) const;

	void MakeBranchingBranch(DhPlantBranches& branches, DhPlantNode& node,
			float dia, const AuPoint& startVec, int maxangle, int maxsegs,
			float mindia, const AuDice& dice) const;

	void PaintFlower(QPainter& p, const QPointF& baseLoc, float petalInc, const AuDice& dice, QBrush brPetal[3], int brPetalSize,
			float unitsPerPixel) const;

	void PaintOneFlower(QPainter& p, const QPointF& baseLoc, float petalInc, const AuDice& dice, QBrush brPetal[3],
			int brPetalSize, float unitsPerPixel) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Flora in a particular latitude region.

class AUDATA_DLL DeTerrainFlora
{
private:
	DeFlora								m_GroundCover;
	DeFlora								m_Shrubs[ZONE_SHRUBS];
	DeFlora								m_Trees[ZONE_TREES];

public:
	DeTerrainFlora();
	DeTerrainFlora(const DeTerrainFlora& c)
	{
		m_GroundCover = c.m_GroundCover;

		for(int shrub=0; shrub<ZONE_SHRUBS; shrub++)
			m_Shrubs[shrub] = c.m_Shrubs[shrub];

		for(int tree=0; tree<ZONE_TREES; tree++)
			m_Trees[tree] = c.m_Trees[tree];
	}

	~DeTerrainFlora();

	const DeTerrainFlora& operator=(const DeTerrainFlora& c)
	{
		m_GroundCover = c.m_GroundCover;

		for(int shrub=0; shrub<ZONE_SHRUBS; shrub++)
			m_Shrubs[shrub] = c.m_Shrubs[shrub];

		for(int tree=0; tree<ZONE_TREES; tree++)
			m_Trees[tree] = c.m_Trees[tree];

		return *this;
	}

	inline bool operator==(const DeTerrainFlora& c) const
	{
		if(m_GroundCover != c.m_GroundCover)
			return false;

		for(int shrub=0; shrub<ZONE_SHRUBS; shrub++)
			if(m_Shrubs[shrub] != c.m_Shrubs[shrub])
				return false;

		for(int tree=0; tree<ZONE_TREES; tree++)
			if(m_Trees[tree] != c.m_Trees[tree])
				return false;

		return true;
	}

	inline bool operator!=(const DeTerrainFlora& c) const
	{
		return !operator==(c);
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

public:
	void CalculateTextureSizes(quint32 genSeed, int resourceZone);

	bool ContainsDNA(int type, const AuFloraDNA& dna) const;

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	void CreateGroundCoverVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int texBase, const AuDice& dice,
			int lod, float maxRadius) const;

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	// 'maxRadius' is used by potted plants. It will be 0 for wild plants.
	void CreateShrubVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int texBase, COMMODITYID idCommodity,
			const AuDice& dice, int lod, float maxRadius) const;

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	// 'maxRadius' is used by potted plants. It will be 0 for wild plants.
	void CreateTreeVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int texBase, COMMODITYID idCommodity,
			const AuDice& dice, int lod, float maxRadius) const;

	const DeFlora& GroundCover() const
	{
		return m_GroundCover;
	}

	void Init(AuFloraDNA::ZONE zone, const AuDice& dice, int sizemod);

	// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
	void Init(int plantType, AuFloraDNA::ZONE zone, const AuDice& dice, int sizemod);

	void LoadTextureSizes(QDataStream& in);

	void PaintBranches(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, const AuTime& floraChange);

	// Used when painting a single plant, for a flower pot.
	void PaintPottedBranches(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, int plantType,
			COMMODITYID idCommodity);

	void PaintGroundCover(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, const AuTime& floraChange);

	void RxUpdate(QDataStream& fromServer);

	inline void SetFloraDNA(int plantType, COMMODITYID idCommodity, const AuFloraDNA& dna)
	{
		switch(plantType)
		{
		case 0:
			m_GroundCover.SetFloraDNA(dna);
			break;
		case 1:
			m_Shrubs[ShrubIndex(idCommodity)].SetFloraDNA(dna);
			break;
		case 2:
			m_Trees[TreeIndex(idCommodity)].SetFloraDNA(dna);
			break;
		}
	}

	const DeFlora& Shrub(COMMODITYID idCommodity) const;

	inline const DeFlora& ShrubAt(unsigned int index) const
	{
		Q_ASSERT(index < ZONE_SHRUBS);
		return m_Shrubs[index];
	}

	static unsigned int ShrubIndex(COMMODITYID idCommodity)
	{
		switch(idCommodity)
		{
		case CID_Beans:
			return 0;
		case CID_Fiber:
			return 1;
		case CID_Fruit:
			return 2;
		case CID_Grain:
			return 3;
		case CID_Grapes:
			return 4;
		case CID_Herbs:
			return 5;
		case CID_Hops:
			return 6;
		case CID_Nuts:
			return 7;
		case CID_Spices:
			return 8;
		case CID_Vegetable:
			return 9;
		case CID_Hay:
		default:
			return 10;
		}
	}

	void StoreTextureSizes(QDataStream& out) const;

	const DeFlora& Tree(COMMODITYID idCommodity) const;

	inline const DeFlora& TreeAt(unsigned int index) const
	{
		Q_ASSERT(index < ZONE_TREES);
		return m_Trees[index];
	}

	static COMMODITYID TreeCommodity(unsigned int index)
	{
		Q_ASSERT(index < ZONE_TREES);
		COMMODITYID treeCIDs[] =
		{
			CID_Fiber,
			CID_Fruit,
			CID_Herbs,
			CID_Nuts,
			CID_Spices,
			CID_Wood,
		};
		return treeCIDs[index];
	}

	static unsigned int TreeIndex(COMMODITYID idCommodity)
	{
		switch(idCommodity)
		{
		case CID_Fiber:
			return 0;
		case CID_Fruit:
			return 1;
		case CID_Herbs:
			return 2;
		case CID_Nuts:
			return 3;
		case CID_Spices:
			return 4;
		case CID_Wood:
		default:
			return 5;
		}
	}

	void TxUpdate(QDataStream& toClient) const;
};

inline QDataStream& operator<<(QDataStream& out, const DeTerrainFlora& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeTerrainFlora& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL DePlanetFlora
{
private:
	DeTerrainFlora						m_Flora[AuFloraDNA::Zones];
	unsigned int						m_glFloraTex;							// Texture palette

public:
	DePlanetFlora();
	~DePlanetFlora();

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const DePlanetFlora& c) const
	{
		for(int zone=0; zone<AuFloraDNA::Zones; zone++)
			if(m_Flora[zone] != c.m_Flora[zone])
				return false;
		return true;
	}

	inline bool operator!=(const DePlanetFlora& c) const
	{
		return !operator==(c);
	}

public:
	void CalculateTextureSizes(quint32 genSeed, int resourceZone);

	// Flora change is needed to dirty the flora DNA cache if the plant flora DNA changes.
	void CreateDeviceObjects(AuGLEffects& re, const AuId& planetId, quint32 genSeed, int resourceZone, const AuTime& floraChange);

	void CreateDeviceObjects(AuGLEffects& re, const AuId& flowerPotId, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone,
			int plantType, COMMODITYID idCommodity);

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	void CreateGroundCoverVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, AuFloraDNA::ZONE latRegion, const AuDice& dice,
			int lod, float maxRadius=0.f) const;

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	// 'maxRadius' is used by potted plants. It will be 0 for wild plants.
	void CreateShrubVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, AuFloraDNA::ZONE latRegion, COMMODITYID idCommodity,
			const AuDice& dice, int lod, float maxRadius=0.f) const;

	// NOTE: OpenGL context must be current when this function is called.
	// 'lod' is a level-of-detail factor, currently 1 or 2.
	// 'maxRadius' is used by potted plants. It will be 0 for wild plants.
	void CreateTreeVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, AuFloraDNA::ZONE latRegion, COMMODITYID idCommodity,
			const AuDice& dice, int lod, float maxRadius=0.f) const;

	inline const unsigned char* Data() const
	{
		return (const unsigned char*)m_Flora;
	}

	void DeleteDeviceObjects(AuGLEffects& re);

	const DeFlora& GroundCover(AuFloraDNA::ZONE latRegion) const;

	// 'radius100km' is radius in 100's of kilometers. Acceptable range is 10-129.
	void Init(int radius100km, const AuDice& dice);

	// 'radius100km' is radius in 100's of kilometers. Acceptable range is 10-129.
	// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
	void Init(int radius100km, int plantType, AuFloraDNA::ZONE zone, const AuDice& dice);

	void InitRingworld(const AuDice& dice);

	// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
	void InitRingworld(int plantType, AuFloraDNA::ZONE zone, const AuDice& dice);

	void LoadTextureSizes(QDataStream& in);

	void PaintBranches(QImage& im, quint32 genSeed, int resourceZone, const AuTime& floraChange);

	// Used when painting a single plant, for a flower pot.
	void PaintPottedBranches(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, int plantType,
			COMMODITYID idCommodity);

	// This one does it all, including loading/storing result from/to cache.
	void PaintFlora(QImage& im, quint32 genSeed, int resourceZone, const AuTime& floraChange);

	void PaintPottedFlora(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, int plantType,
			COMMODITYID idCommodity);

	// Creates an image of the grass texture that matches the grass texture created by CreateDeviceObjects for the same genSeed.
	void PaintGroundCover(QImage& grassPolar, QImage& grassTemperate, QImage& grassTropical, quint32 genSeed, int resourceZone,
			const AuTime& floraChange);

	void RxUpdate(QDataStream& fromServer);

	void SetDNA(AuFloraDNA::ZONE latRegion, const DeTerrainFlora& dna);

	// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
	inline void SetFloraDNA(AuFloraDNA::ZONE latRegion, int plantType, COMMODITYID idCommodity, const AuFloraDNA& dna)
	{
		m_Flora[latRegion].SetFloraDNA(plantType, idCommodity, dna);
	}

	const DeFlora& Shrub(AuFloraDNA::ZONE latRegion, COMMODITYID idCommodity) const;

	void StoreTextureSizes(QDataStream& out) const;

	static void Texture(unsigned int tileIndex, float& s, float& t, float& u, float& v);

	inline unsigned int Texture() const
	{
		return m_glFloraTex;
	}

	const DeFlora& Tree(AuFloraDNA::ZONE latRegion, COMMODITYID idCommodity) const;

	void TxUpdate(QDataStream& toClient) const;
};

inline QDataStream& operator<<(QDataStream& out, const DePlanetFlora& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DePlanetFlora& o)
{
	o << in;
	return in;
}

#endif // DeFlora_H
