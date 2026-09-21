//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglmodel.h,v 1.3 2022/02/19 18:19:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGLModel_H
#define AuGLModel_H

#include "audata.h"
#include "audef.h"
#include "auglsl.h"
#include "aupoint.h"

class Au3dsGLSLModelDefinition;
class Au3dsGLSLModelInstance;
class AuDesign;
class AuGLEffects;
class AuOpenGLFunctions;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGLModelInstance
{
public:
	virtual ~AuGLModelInstance()
	{
	}

public:
	// Returns the axis down which the item is aimed. 0=positive X, 1=positive Y, -1=unknown
	virtual int AimAxis() const = 0;

	// Constructs an appropriate model instance, based on the file name.
	// .SoH file returns an AuSoHGLModelInstance.
	// All other files return an Au3dsGLSLModelInstance.
	static AuGLModelInstance* Create(const QString& fileName);

	// OpenGL context required.
	virtual bool CreateDeviceObjects(AuGLEffects& re, bool lightOn, double scale) = 0;

	// Creates only the geometry of this object. Does not require an OpenGL context.
	virtual bool CreateGeometry(double scale=1.) = 0;

	// OpenGL context required.
	virtual void DeleteGL() = 0;

	virtual const QString& FileName() const = 0;

	virtual void GetExtents(AuPoint& min, AuPoint& max) const = 0;

	// Returns true if the OpenGL model has been created for this object.
	// Creates the OpenGL model from geometry data if the data is ready.
	// OpenGL context required.
	virtual bool GLModel(AuGLEffects& re) = 0;

	// Returns GORI_Count if the model does not know.
	virtual GROUND_ORIENT GroundOrientation() const = 0;

	// Returns true if the model geometry exists. Not a test for OpenGL objects. Does not require an OpenGL context.
	virtual bool HasGLModel() const = 0;

	// Loads the intermediate model data, without using a thread.
	virtual void LoadModel(bool lightOn, double scale) = 0;

	virtual quint64 MemoryUsage() const = 0;

	// Starts a worker thread to load the intermediate model data.
	virtual void QueueLoadModel(bool lightOn, double scale) = 0;

	// 'ambientLightUp' is in eye space.
	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass.
	// 'pass' is 2 to render final pass.
	virtual bool Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float alpha=1.f, int pass=0) const = 0;

	// 'globeP' is used for sun over the horizon effect.
	// 'globeP' and 'ambientLightUp' are in eye space.
	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass.
	// 'pass' is 2 to render final pass.
	virtual bool RenderFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& globeP,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f, int pass=0) const = 0;

	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass.
	// 'pass' is 2 to render final pass.
	virtual bool RenderFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			float shadowPanelSunAngles[MAX_SUNS],
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f, int pass=0) const = 0;

	virtual void ResourceReport(QDataStream& out) const = 0;

	virtual double RoughRadius() const = 0;

	// Scale applied to model when loaded, for differentiating the same model loaded at different scales.
	virtual double Scale() const = 0;

	// Sets the texture id when this model is used as the base for a special effect.
	virtual void SetTexture(GLuint texture) = 0;

	// Returns the texture id when this model is used as the base for a special effect.
	virtual GLuint Texture() const = 0;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSoHGLModelInstance : public AuGLModelInstance
{
	AuDesign*							m_Design;				// Loaded from m_FileName.
	QString								m_FileName;
	double								m_GeometryScale;		// Scaling that was applied in CreateGeometry.

	AuGLVAO								m_OpaqueVAO;
	AuTexturePalette					m_Textures;
	AuGLVAO								m_TransparentVAO;

public:
	AuSoHGLModelInstance(const QString& fileName);

	virtual ~AuSoHGLModelInstance();

public:
	virtual int AimAxis() const;

	virtual bool CreateDeviceObjects(AuGLEffects& re, bool lightOn, double scale);

	virtual bool CreateGeometry(double scale=1.);

	virtual void DeleteGL();

	virtual const QString& FileName() const;

	virtual void GetExtents(AuPoint& min, AuPoint& max) const;

	virtual bool GLModel(AuGLEffects& re);

	virtual GROUND_ORIENT GroundOrientation() const;

	virtual bool HasGLModel() const;

	virtual void LoadModel(bool lightOn, double scale);

	virtual quint64 MemoryUsage() const;

	virtual void QueueLoadModel(bool lightOn, double scale);

	virtual bool Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float alpha=1.f, int pass=0) const;

	virtual bool RenderFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& globeP,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f, int pass=0) const;

	virtual bool RenderFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			float shadowPanelSunAngles[MAX_SUNS],
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f, int pass=0) const;

	bool RenderShader(AuGLSLScBase* shader, const AuMatrix4x4& model, AuGLEffects& re, float ambientLightR, float ambientLightG,
			float ambientLightB, const AuPoint& ambientLightUp, bool lightOn, float alpha, int pass) const;

	virtual void ResourceReport(QDataStream& out) const;

	virtual double RoughRadius() const;

	virtual double Scale() const
	{
		return m_GeometryScale;
	}

	// Does nothing in this class.
	virtual void SetTexture(GLuint texture)
	{
		Q_UNUSED(texture);
	}

	// Does nothing in this class.
	virtual GLuint Texture() const
	{
		return 0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A cache for efficiently using 3ds models by multiple users in a scene.
// Similar to AuGLTexture

class AuGLModelCache : public QMultiHash<QString, AuGLModelInstance*>			// Key is file name
{
public:
	inline ~AuGLModelCache()
	{
		DeleteModels();
	}

	// Creates only the geometry of this objects.
	// Similar to CreateModel
	// Used on the server to cache basic geometry without GL or any sort of rendering context.
	// 'scale' is an optional scaling factor applied to the model geometry when it is loaded from the file.
	inline AuGLModelInstance* CreateGeometry(const QString& fileName, double scale=1.)
	{
		AuGLModelInstance* pm = FindModel(fileName, scale);
		if(!pm)
		{
			pm = AuGLModelInstance::Create(fileName);
			if(pm)
			{
				if(pm->CreateGeometry(scale))
				{
					//pm->GetExtents(min, max);
					//pm->GetUpVector(upvector);
					insert(fileName, pm);
				}
				else
				{
					qWarning() << "AuGLModelCache::CreateGeometry AuGLModelInstance::CreateGeometry FAILED." << fileName;
					delete pm;
					pm = 0;
				}
			}
			else
			{
				qWarning() << "AuGLModelCache::CreateGeometry 'new AuGLModelInstance' FAILED." << fileName;
			}
		}
		return pm;
	}

	// OpenGL context MUST be current to call this function.
	AuGLModelInstance* CreateModel(const QString& fileName, AuGLEffects& re, bool lightOn, double scale);

	// OpenGL context does not have to be current to call this function.
	AuGLModelInstance* LoadModelNow(const QString& fileName, bool lightOn, double scale);		// Loads the model now.
	AuGLModelInstance* LoadModelThreaded(const QString& fileName, bool lightOn, double scale);	// Loads the model using a thread.

	// 'min' and 'max' are output parameters set to the 3d extents of the vertices of the 3ds model.
	Au3dsGLSLModelInstance* CreateBodyModel(AuOpenGLFunctions& GLf, const QString& fileName, AuPoint& min, AuPoint& max,
			bool lightOn, double iScale, double overallScale, int pb, int tb, int nb, const AuPoint& pOffset, double uL,
			double uT, double lL, double lT, double hwL, QColor tC=QColor(255,255,255,0), float tS=0,
			QColor bC=QColor(255,255,255,0), float bS=0, float glow=0, float glowdim=0, bool bCreate=false, bool bSkip=false,
			bool bArrangement=false, GLuint destBlendFunc=GL_ONE_MINUS_SRC_ALPHA);

	// 'min' and 'max' are output parameters set to the 3d extents of the
	// vertices of the 3ds model.
	Au3dsGLSLModelInstance* CreateBodyModel(AuOpenGLFunctions& GLf, const QString& fileName, AuPoint& min, AuPoint& max,
			bool lightOn, Au3dsGLSLModelDefinition modelDef);

	void CreatePatchedModel(AuOpenGLFunctions& GLf, Au3dsGLSLModelInstance* pm);

	inline void DeleteModel(AuGLModelInstance* pm)
	{
		//DHH what about texture reference?
		if(pm)
		{
			int iPos = 0;
			for(iterator it = begin(); it != end(); it++)
			{
				if(*it == pm)
				{
					erase(it);
					break;
				}
				iPos++;
			}
			pm->DeleteGL();
			delete pm;
		}
	}

	inline void DeleteModels()
	{
		iterator it = begin();
		while(it != end())
		{
			delete *it;
			it = erase(it);
		}
	}

	inline bool IsLoaded(const QString& fileName, double scale)
	{
		for(iterator it = find(fileName); it != end() && it.key() == fileName; it++)
			if((*it)->Scale() == scale)
				return true;
		return false;
	}

	inline AuGLModelInstance* FindModel(const QString &fileName, double scale)
	{
		for(iterator it = find(fileName); it != end() && it.key() == fileName; it++)
			if((*it)->Scale() == scale)
				return *it;
		return 0;
	}

	Au3dsGLSLModelInstance* FindBodyModel(const QString &fileName,
		double OverallScale=0, double iScale=0, //double uTop=0, double vTop=0,
		/*double uBottom=0, double vBottom=0,*/ double uThick=0,
		double uLength=0, double lThick=0, double lLength=0, double hwLength=0,
		const AuPoint& loc=AuPoint(0,0,0), int pb=0, int tb=0, int nb=0,
		QColor tC=QColor(255,255,255,0), float tS=0, QColor bC=QColor(255,255,255,0), float bS=0, float glow=0, float glowdim=0);

	void ResourceReport(QDataStream& out) const;

	quint64 MemoryUsage() const;
};

#endif // AuGLModel_H
