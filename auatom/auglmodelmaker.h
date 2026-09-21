//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglmodelmaker.h,v 1.71 2022/02/19 17:31:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGLModelMaker_H
#define AuGLModelMaker_H

#include "auatom.h"
#include "auanimalia.h"
#include "auweather.h"
#include "dabd.h"
#include "daworldll.h"
#include <QRunnable>

class AuId;
class Au3dsModelInstance;
class DaWorldSphere;
class DaWorldRingSection;
class DePlanetFlora;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuRunnable : public QRunnable
{
protected:
	bool								m_Killed;				// Used in StopRunning logic.

private:
	//mutable QMutex						m_RunStateLock;			// Guards m_RunState. Locked while running.
	quint8								m_RunState;				// 0=hasn't run, 1=running, 2=done running.

public:
	inline AuRunnable()
	{
		m_Killed = false;
		m_RunState = 0;
		setAutoDelete(false);
	}

	virtual ~AuRunnable()
	{
	}

	// Returns true if this runnable has been run at all.
	inline bool HasRun() const
	{
		return m_RunState != 0;
	}

	inline bool IsDoneRunning() const
	{
		//if(m_RunStateLock.tryLock())
		{
			bool isDoneRunning = m_RunState == 2;
			//m_RunStateLock.unlock();
			return isDoneRunning;
		}
		//return false;
	}

	inline void StopRunning()
	{
		m_Killed = true;
	}

protected:
	bool RunStart();

	inline void RunFinish()
	{
		m_RunState = 2;
		//m_RunStateLock.unlock();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Model of a spacecraft.

class AuAnimaliaModel : public AuRunnable
{
private:
	AuAnimalia							m_Animalia;
	AuId								m_AnimalId;
	AuClothingMesh						m_ClothingMesh;
	AuTime								m_LastUsed;
	AuMeatMesh							m_SkinMesh;

public:
	AuAnimaliaModel(const AuId& animalId, const AuAnimalia& animalia);
	virtual ~AuAnimaliaModel();

	// Deletes any GL objects that have been created.
	void DeleteGL();

	inline const AuTime& LastUsed() const
	{
		return m_LastUsed;
	}

	// Creates the VAO from the cache data and sets m_Animalia to the result.
	// NOTE: A gl context must be current when this function is called.
	// 'animalia' is an output parameter.
	// Sets LastUsed time.
	void VAO(AuOpenGLFunctions& GLf, AuAnimalia& animalia);

	// Compares 'animalia' to this model. Returns a number from 0 to 100, to indicate how close of a visual match this is.
	// Used when selecting a model to display the person so returns 0 if thread is not done running.
	int VisualCompare(const AuAnimalia& animalia) const;

	// This guy does the grunt work to create the model of the globe.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBdModel : public AuRunnable
{
	AuBdRender							m_BdRender;
	quint32								m_DesignId;
	quint32								m_EmpireId;				// Empire for theme colors.

public:
	AuBdModel(quint32 designId, quint32 empireId);

	// NOTE: A gl context must be current when this function is called.
	void DeleteGL();

	inline quint32 EmpireId() const
	{
		return m_EmpireId;
	}

	// Returns the VAOs of this building.
	// Creates the VAOs from the cache data and preserves the result.
	// Returns false if not ready yet.
	// NOTE: A gl context must be current when this function is called.
	bool VAO(AuOpenGLFunctions& GLf, AuBdRender& bdRender);

	// This guy does the grunt work to create the model for as ship design.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBdFrame : public AuRunnable
{
	AuBdRender							m_BdRender;
	quint32								m_DesignId;
	COMMODITYID							m_HullMaterial;
	int									m_PercentComplete;

public:
	AuBdFrame(quint32 designId, COMMODITYID hullMaterial, int percentComplete);

	// NOTE: A gl context must be current when this function is called.
	void DeleteGL();

	inline COMMODITYID HullMaterial() const
	{
		return m_HullMaterial;
	}

	inline int PercentComplete() const
	{
		return m_PercentComplete;
	}

	// Returns the VAOs of this building.
	// Creates the VAOs from the cache data and preserves the result.
	// Returns false if not ready yet.
	// NOTE: A gl context must be current when this function is called.
	bool VAO(AuOpenGLFunctions& GLf, AuBdRender& bdRender);

	// This guy does the grunt work to create the model for as ship design.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFlatGlobeModel : public AuRunnable
{
private:
	bool								m_CacheEnabled;			// True to load/save files from/to cache dir.
	QList<AuGlobeTriAltVeg>				m_GlobeTris;			// Globe surface triangles.
	float								m_MaxAltMSL;
	float								m_MinAltMSL;
	AuGLVAO								m_VAO;
	bool								m_WorldCanHaveFlora;
	AuId								m_WorldId;
	AuTime								m_WorldModifyTime;

public:
	AuFlatGlobeModel(const AuId& worldId, const AuTime& worldModTime, bool worldCanHaveFlora, bool cacheEnabled);

	// Deletes any GL objects that have been created.
	void DeleteGL();

	// Returns the VAO of this flat world model.
	// Creates the VAO from the cache data and sets m_VAO to the result.
	// NOTE: A gl context must be current when this function is called.
	// 'minAltMSL' and 'maxAltMSL' are output parameters.
	AuGLVAO VAO(AuOpenGLFunctions& GLf, float& minAltMSL, float& maxAltMSL);

	// Returns the current GL VAO id. Does not attempt to create the vertex array object.
	inline GLuint VAOId() const
	{
		return m_VAO.VAO();
	}

	// This guy does the grunt work to create the model of the globe.
	virtual void run();

private:
	inline bool CacheEnabled() const
	{
		return m_CacheEnabled;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeModel : public AuRunnable
{
private:
	bool								m_CacheEnabled;			// True to load/save files from/to cache dir.
	QList<AuGlobeTriAltVegN>			m_GlobeTris;			// Globe surface triangles.
	AuAtGlobeVAO						m_VAO;
	bool								m_WorldCanHaveFlora;
	AuId								m_WorldId;
	AuTime								m_WorldModifyTime;
	float								m_WorldRadius;			// Sea level radius of the world.

public:
	AuGlobeModel(const AuId& worldId, const AuTime& worldModTime, float worldRadius, bool worldCanHaveFlora, bool cacheEnabled);

	// Deletes any GL objects that have been created.
	void DeleteGL();

	// Returns the VAOs of this world.
	// Creates the VAOs from the cache data and sets m_VAO to the result.
	// 'vao' m_TriVAOs will be empty on return if the VAOs are not ready yet.
	// NOTE: A gl context must be current when this function is called.
	void VAOs(AuOpenGLFunctions& GLf, AuAtGlobeVAO& vao);

	// This guy does the grunt work to create the model of the globe.
	virtual void run();

private:
	inline bool CacheEnabled() const
	{
		return m_CacheEnabled;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeModelTri : public AuRunnable
{
private:
	QList<AuGlobeTriAltVegNPave>		m_GlobeTris;			// Globe surface triangles.
	float								m_MaxAltMSL;
	float								m_MinAltMSL;
	AuGlobeLLTri						m_TerrainTri;			// The terrain triangle we are tessellating.
	int									m_TriIndex;
	AuGLTriVAO							m_VAO;
	bool								m_WorldCanHaveFlora;
	AuId								m_WorldId;
	float								m_WorldRadius;			// Sea level radius of the world.

public:
	AuGlobeModelTri(const AuId& worldId, int triIndex, const AuGlobeLLTri& tri, float worldRadius, bool worldCanHaveFlora,
			float maxAltMSL, float minAltMSL);

	// Deletes any GL objects that have been created.
	void DeleteGL();

	// If not valid, the model can still be used until a replacement is ready.
	inline bool IsValid() const
	{
		return m_VAO.IsValid();
	}

	// This guy does the grunt work to create the model of the globe.
	virtual void run();

	inline void SetValid(bool valid)
	{
		m_VAO.SetValid(valid);
	}

	int TriIndex() const
	{
		return m_TriIndex;
	}

	// Returns the VAO of this portion of the world.
	// Creates the VAO from the cache data and sets m_VAO to the result.
	// NOTE: A gl context must be current when this function is called.
	AuGLTriVAO VAO(AuOpenGLFunctions& GLf);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuNearGlobeModel : public AuRunnable
{
private:
	bool								m_CacheEnabled;			// True to load/save files from/to cache dir.
	QList<AuGlobeTriTexNShineSnow>		m_GlobeTris;			// Globe surface triangles.
	AuGLVAO								m_VAO;
	AuId								m_WorldId;
	AuTime								m_WorldModifyTime;

public:
	AuNearGlobeModel(const AuId& worldId, const AuTime& worldModTime, bool cacheEnabled);

	// Returns the VAO of this world.
	// Creates the VAO from the cache data and sets m_VAO to the result.
	// Returns m_VAO, which will be 0 if it's not ready yet.
	// NOTE: A gl context must be current when this function is called.
	AuGLVAO VAO(AuOpenGLFunctions& GLf);

	// Returns the VAO when it is known to exist.
	AuGLVAO VAOEx()
	{
		Q_ASSERT(VAOId());
		return m_VAO;
	}

	// Returns the current GL VAO id. Does not attempt to create the vertex array object.
	inline GLuint VAOId() const
	{
		return m_VAO.VAO();
	}

	// This guy does the grunt work to create the model of the globe.
	virtual void run();

private:
	inline bool CacheEnabled() const
	{
		return m_CacheEnabled;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFloraImage : public AuRunnable
{
private:
	DePlanetFlora						m_Flora;				// Flora, updated with size information when finished.
	QImage								m_FloraImage;			// The resulting image.
	quint32								m_GenSeed;				// Planet genseed
	int									m_ResourceZone;

public:
	AuFloraImage(const DePlanetFlora& flora, quint32 genSeed, int resourceZone);

	// Sets 'imgFlora' to a valid image when ready.
	// Updates 'flora' with my data when done, which includes DeFlora::m_TextureSize gleaned while painting the flora image.
	void FloraImage(DePlanetFlora& flora, QImage& imgFlora);

	int ResourceZone() const
	{
		return m_ResourceZone;
	}

	// This guy does the grunt work to create the grass.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGrassImages : public AuRunnable
{
private:
	DePlanetFlora						m_Flora;				// Flora, updated with size information when finished.
	QImage								m_Grass0;				// The resulting image.
	QImage								m_Grass1;				// The resulting image.
	QImage								m_Grass2;				// The resulting image.
	quint32								m_GenSeed;				// Planet genseed
	int									m_ResourceZone;

public:
	AuGrassImages(const DePlanetFlora& flora, quint32 genSeed, int resourceZone);

	// Returns true when the return images are valid.
	bool GrassImages(QImage& grass0, QImage& grass1, QImage& grass2);

	int ResourceZone() const
	{
		return m_ResourceZone;
	}

	// This guy does the grunt work to create the grass.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFloraModel : public AuRunnable
{
private:
	QMap<AuResKey, AuGlobeFlora>		m_AquaticGrass;			// Aquatic Grass locations.
	QMap<AuResKey, AuGlobeFlora>		m_AquaticShrubs;		// Aquatic Shrub locations.
	QMap<AuResKey, AuGlobeFlora>		m_Grass;				// Grass locations.
	AuPoint								m_GroundLoc;			// Ground location at center of flora distribution.
	QMap<AuResKey, AuGlobeFlora>		m_Shrubs;				// Shrub locations.
	QMap<AuResKey, AuGlobeTree>			m_Trees;				// Tree locations.
	AuId								m_WorldId;

public:
	AuFloraModel(const AuId& worldId, const AuPoint& groundLoc);

	// Returns true when the foliage has been generated in an area.
	// Sets 'grass' to the resulting grass distribution, which may be empty.
	bool Flora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeTree>& trees,
			QMap<AuResKey, AuGlobeFlora>& aquaticgrass, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs);

	const AuPoint& GroundLoc() const
	{
		return m_GroundLoc;
	}

	// This guy does the grunt work to create the grass.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuRockModel : public AuRunnable
{
private:
	AuPoint								m_GroundLoc;			// Ground location at center of flora distribution.
	QMap<AuResKey, AuGlobeRock>			m_Rock;					// Rock locations.
	AuId								m_WorldId;

public:
	AuRockModel(const AuId& worldId, const AuPoint& groundLoc);

	const AuPoint& GroundLoc() const
	{
		return m_GroundLoc;
	}

	// Returns true when the rocks have been generated in an area.
	// Sets 'rocks' to the resulting rock distribution, which may be empty.
	bool Rock(QMap<AuResKey, AuGlobeRock>& rocks);

	// This guy does the grunt work to create the grass.
	virtual void run();
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuWeatherData : public AuRunnable
{
private:
	AuId								m_WorldId;
	AuWeather							m_Wx;

public:
	AuWeatherData(const AuId& worldId, const AuWeather& wx);

	// This guy does the grunt work to create the weather data.
	virtual void run();

	// Sets 'wx' when ready.
	// Returns true if the returned weather is valid.
	bool Weather(AuWeather& wx);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuGLModelMaker
{
public:
	AuGLModelMaker(bool cacheEnabled);
	~AuGLModelMaker();

	void Animalia(AuOpenGLFunctions& GLf, const AuId& id, AuAnimalia& animalia); 

	bool Bd(AuOpenGLFunctions& GLf, quint32 designId, quint32 empireId, AuBdRender& bdRender); 

	bool BdFrame(AuOpenGLFunctions& GLf, quint32 designId, COMMODITYID hullMaterial, int percentComplete, AuBdRender& bdRender); 

	// Query for 3ds model corresponding to a creature body.
	// Causes the model to be created if needed.
	// Returns a body with an empty models array if the model is not created yet.
	// Eventually returns an array of model parts when the model is created.
	//QVector<Au3dsModelInstance*> BodyModel(const DpBody& body, AuPoint& min, AuPoint& max, AuPoint& eyePos);

	// Returns number of our worker threads still running.
	int BusyThreadCount() const;

	// Returns true when the return result is valid, which may mean no plants at all.
	// Does not modify groundLoc until true is returned.
	bool CreateFlora(const DaWorldLL* world, AuPoint& groundLoc, QMap<AuResKey, AuGlobeFlora>& grass,
			QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
			QMap<AuResKey, AuGlobeFlora>& aquaticshrubs);

	// Returns true when the return result is valid, which may mean no rocks at all.
	// Does not modify groundLoc until true is returned.
	bool CreateRocks(const DaWorldLL* world, AuPoint& groundLoc, QMap<AuResKey, AuGlobeRock>& rocks);

	void DeleteAnimalia(const AuId& id);

	void DeleteBd(quint32 designId);

	void DeleteBdFrame(quint32 designId);

	void DeleteFlatGlobeModel(const AuId& worldId);

	void DeleteFlatRingworldSectionModel(const AuId& worldId);

	void DeleteFloraImage(const AuId& worldId, int resourceZone);

	// Deletes all flora images related to 'worldId'.
	void DeleteFloraImages(const AuId& worldId);

	void DeleteFloraModel(const AuId& worldId);

	void DeleteGlobeModel(const AuId& worldId);

	void DeleteGlobeModelTri(const AuId& worldId, int triIndex);

	void DeleteGrassImages(const AuId& worldId);

	void DeleteNearGlobeModel(const AuId& worldId);

	void DeleteNearRingworldSectionModel(const AuId& worldId);

	void DeleteRingworldSectionModel(const AuId& worldId);

	void DeleteRingworldSectionModelTri(const AuId& worldId, int triIndex);

	void DeleteRockModel(const AuId& worldId);

	void DeleteWeather(const AuId& worldId);

	AuGLVAO FlatGlobeModel(AuOpenGLFunctions& GLf, const DaWorldSphere* world, float& minAltMSL, float& maxAltMSL);

	AuGLVAO FlatRingworldSectionModel(AuOpenGLFunctions& GLf, const DaWorldRingSection* world, float& minAltMSL,
			float& maxAltMSL);

	// Paints the image of all the species of flora in a resource zone of a planet. This image is used to create a texture for
	// the plant models themselves.
	// Sets 'imgFlora' to a valid image when ready.
	// Updates 'flora' with my data when done, which includes DeFlora::m_TextureSize gleaned while painting the flora image.
	void FloraImage(QImage& imgFlora, DePlanetFlora& flora, const AuId& planetId, quint32 genSeed, int resourceZone);

	// Query for gl model id corresponding to a globe atom id.
	// Causes the model to be created if needed.
	// Eventually 'vao' return with non-empty m_TriVAOs when the model is created.
	void GlobeModel(AuOpenGLFunctions& GLf, AuAtGlobeVAO& vao, const DaWorldSphere* world);

	void GlobeModelTri(AuOpenGLFunctions& GLf, AuGLTriVAO& vao, const DaWorldSphere* world, int triIndex,
			const AuGlobeLLTri& tri, float maxAltMSL, float minAltMSL);

	void InvalidateGlobeModelTri(const AuId& worldId, int triIndex);

	// Returns true when the images are ready.
	bool GrassImages(QImage& grass0, QImage& grass1, QImage& grass2, DePlanetFlora& flora, const AuId& planetId, quint32 genSeed,
			int resourceZone);

	// Returns true if any of our worker threads are still running.
	bool IsBusy() const;

	// Query for gl model id corresponding to a globe atom id.
	// Causes the model to be created if needed.
	// Returns 0 if the model is not created yet.
	// Eventually returns non-zero when the model is created.
	AuGLVAO NearGlobeModel(AuOpenGLFunctions& GLf, const DaWorldSphere* world);

	// Query for gl model id corresponding to a globe atom id.
	// Causes the model to be created if needed.
	// Returns 0 if the model is not created yet.
	// Eventually returns non-zero when the model is created.
	AuGLVAO NearRingworldSectionModel(AuOpenGLFunctions& GLf, const DaWorldRingSection* world);

	// Query for gl model id corresponding to a globe atom id.
	// Causes the model to be created if needed.
	// Eventually 'vao' return with non-empty m_TriVAOs when the model is created.
	void RingworldSectionModel(AuOpenGLFunctions& GLf, AuAtGlobeVAO& vao, const DaWorldRingSection* world);

	void RingworldSectionModelTri(AuOpenGLFunctions& GLf, AuGLTriVAO& vao, const DaWorldRingSection* world, int triIndex,
			const AuGlobeLLTri& tri, float maxAltMSL, float minAltMSL);

	void InvalidateRingworldSectionModelTri(const AuId& worldId, int triIndex);

	// Returns true if the weather is initialized.
	bool Weather(AuWeather& wx, const DaWorldLL* world);

private:
	inline bool CacheEnabled() const
	{
		return m_CacheEnabled;
	}

private:
	bool								m_CacheEnabled;							// True to load/save files from/to cache dir.
	unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>	m_Animalia;			// Animalia models, key is by animal id.
	QList<AuAnimaliaModel*>				m_AnimaliaToDelete;

	unordered_multimap<quint32, AuBdModel*, AuDesignIdHash>	m_Bd;				// Bd models, key is design id
	QList<AuBdModel*>					m_BdToDelete;

	unordered_multimap<quint32, AuBdFrame*, AuDesignIdHash>	m_BdFrame;			// Bd under construction models, key is design id
	QList<AuBdFrame*>					m_BdFrameToDelete;

	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>	m_FlatGlobes;			// Globe models, key is world id.
	QList<AuFlatGlobeModel*>			m_FlatGlobesToDelete;

	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>	m_FlatRingworldSections;	// Ringworld section models, key is world id.
	QList<AuFlatGlobeModel*>			m_FlatRingworldSectionsToDelete;

	unordered_map<AuId, AuFloraModel*, AuIdHash>	m_Flora;					// Flora models, key is world id.
	QList<AuFloraModel*>				m_FloraToDelete;

	unordered_multimap<AuId, AuFloraImage*, AuIdHash>	m_FloraImages;			// Flora images used for texture on flora models.
	QList<AuFloraImage*>				m_FloraImagesToDelete;

	unordered_map<AuId, AuGlobeModel*, AuIdHash>	m_Globes;					// Globe models, key is world id.
	QList<AuGlobeModel*>				m_GlobesToDelete;

	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>	m_GlobeTris;		// Globe high-tessellation models, key is world id.
	QList<AuGlobeModelTri*>				m_GlobeTrisToDelete;

	unordered_multimap<AuId, AuGrassImages*, AuIdHash>	m_GrassImages;			// Grass images used for texture on planet models.
	QList<AuGrassImages*>				m_GrassImagesToDelete;

	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>	m_NearGlobes;			// Near globe models, key is world id.
	QList<AuNearGlobeModel*>			m_NearGlobesToDelete;

	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>	m_NearRingworldSections;	// Near globe models, key is world id.
	QList<AuNearGlobeModel*>			m_NearRingworldSectionsToDelete;

	unordered_map<AuId, AuGlobeModel*, AuIdHash>	m_RingworldSections;		// Ringworld section models, key is world id.
	QList<AuGlobeModel*>				m_RingworldSectionsToDelete;

	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>	m_RingworldSectionTris;	// Globe high-tess models, key is world id.
	QList<AuGlobeModelTri*>				m_RingworldSectionTrisToDelete;

	unordered_map<AuId, AuRockModel*, AuIdHash>	m_Rock;							// Rock models, key is world id.
	QList<AuRockModel*>					m_RockToDelete;

	unordered_map<AuId, AuWeatherData*, AuIdHash>	m_Wx;						// Weather data models, key is world id.
	QList<AuWeatherData*>				m_WxToDelete;
};

#endif // AuGLModelMaker_H
