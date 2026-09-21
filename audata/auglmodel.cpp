//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglmodel.cpp,v 1.3 2022/02/19 18:20:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auglmodel.h"
//
#include "au3dsglsl.h"
#include "audef.h"
#include "audesign.h"
#include "auscparts.h"
#include "aufilever.h"

//static
AuGLModelInstance* AuGLModelInstance::Create(const QString& fileName)
{
	if(fileName.right(4).compare(".SoH", Qt::CaseInsensitive) == 0)
		return new AuSoHGLModelInstance(fileName);
	else
		return new Au3dsGLSLModelInstance(fileName);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSoHGLModelInstance::AuSoHGLModelInstance(const QString& fileName)
{
	m_Design = 0;
	m_FileName = fileName;
	m_GeometryScale = 1;
}

AuSoHGLModelInstance::~AuSoHGLModelInstance()
{
	if(m_Design)
		delete m_Design;
}

bool AuSoHGLModelInstance::CreateDeviceObjects(AuGLEffects& re, bool lightOn, double scale)
{
	if(!CreateGeometry(scale))
		return false;
	if(!GLModel(re))
		return false;
	return true;
}

bool AuSoHGLModelInstance::CreateGeometry(double scale)
{
	m_GeometryScale = scale;
	if(m_Design)
		delete m_Design;
	m_Design = AuDesign::LoadFromFile(m_FileName, SOH_FILE_TYPE);
	if(m_Design && scale != 1)
	{
		AuMatrix4x4 t;
		t.Scale(scale, scale, scale);
		m_Design->Parts().Transform(t);
	}
	return m_Design ? true : false;
}

void AuSoHGLModelInstance::DeleteGL()
{
	if(m_OpaqueVAO.VAO())
		m_OpaqueVAO.DeleteGL();
	if(m_TransparentVAO.VAO())
		m_TransparentVAO.DeleteGL();
}

const QString& AuSoHGLModelInstance::FileName() const
{
	return m_FileName;
}

void AuSoHGLModelInstance::GetExtents(AuPoint& min, AuPoint& max) const
{
	if(m_Design)
		m_Design->Parts().ModelExtents(min, max);
}

bool AuSoHGLModelInstance::GLModel(AuGLEffects& re)
{
	if(!m_Design)
		return false;

	m_Design->ShaderTexturePalette(CID_Lumber, re, m_Textures);

	AuScFaceList faces;
	m_Design->Parts().VisibleFaces(faces);
	if(faces.empty())
		return false;

	AuScTriRenderList opaqueTris, transparentTris;
	faces.RenderTriangles(opaqueTris, transparentTris);
	m_OpaqueVAO.CreateForSc(re, opaqueTris, GL_STATIC_DRAW, EFF_None);
	m_TransparentVAO.CreateForSc(re, transparentTris, GL_STATIC_DRAW, EFF_None);
	return true;
}

bool AuSoHGLModelInstance::HasGLModel() const
{
	return m_OpaqueVAO.VAO() || m_TransparentVAO.VAO();
}

void AuSoHGLModelInstance::QueueLoadModel(bool lightOn, double scale)
{
	CreateGeometry(scale);
}

void AuSoHGLModelInstance::LoadModel(bool lightOn, double scale)
{
	CreateGeometry(scale);
}

bool AuSoHGLModelInstance::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
		float alpha, int pass) const
{
	re.GLSL().ScShader().Select(projection, re);
	bool hasTransparency = RenderShader(&re.GLSL().ScShader(), model, re, ambientLightR,ambientLightG,ambientLightB,
			ambientLightUp, lightOn, alpha, pass);
	re.GLSL().ScShader().Deselect();
	return hasTransparency;
}

bool AuSoHGLModelInstance::RenderFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& globeP,
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
		float fogDensity, float fogR, float fogG, float fogB, float alpha, int pass) const
{
	re.GLSL().ScGlobeFogShader().Select(projection, re);
    re.GLSL().ScGlobeFogShader().SetFog(fogDensity, fogR, fogG, fogB);
    re.GLSL().ScGlobeFogShader().SetGlobeLocation(globeP);
	bool hasTransparency = RenderShader(&re.GLSL().ScGlobeFogShader(), model, re, ambientLightR,ambientLightG,ambientLightB,
			ambientLightUp, lightOn, alpha, pass);
	re.GLSL().ScGlobeFogShader().Deselect();
	return hasTransparency;
}

bool AuSoHGLModelInstance::RenderFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		float shadowPanelSunAngles[MAX_SUNS],
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
		float fogDensity, float fogR, float fogG, float fogB, float alpha, int pass) const
{
	re.GLSL().ScRingworldFogShader().Select(projection, re);
	re.GLSL().ScRingworldFogShader().SetEyeLocation(model.UnProject(re.EyeLocation()));
    re.GLSL().ScRingworldFogShader().SetFog(fogDensity, fogR, fogG, fogB);
    for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().ScRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);
	bool hasTransparency = RenderShader(&re.GLSL().ScRingworldFogShader(), model, re, ambientLightR,ambientLightG,ambientLightB,
			ambientLightUp, lightOn, alpha, pass);
	re.GLSL().ScRingworldFogShader().Deselect();
	return hasTransparency;
}

bool AuSoHGLModelInstance::RenderShader(AuGLSLScBase* shader, const AuMatrix4x4& model, AuGLEffects& re, float ambientLightR,
		float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn, float alpha, int pass) const
{
    shader->SetModelView(model);
    shader->SetSurfaceTextures(m_Textures);
    shader->SetGlowDiscard(0,0,0, .01f);
    shader->SetVertAlpha(alpha);
    shader->SetAmbientLight(ambientLightR, ambientLightG, ambientLightB);
    shader->SetAmbientLightUp(ambientLightUp);
	if(pass < 2)
	{
		// pass is 0=everything or 1=solid
		if(m_OpaqueVAO.VAO())
		{
			m_OpaqueVAO.SelectGL();
			m_OpaqueVAO.DrawGL();
			m_OpaqueVAO.DeselectGL();
		}
	}
	if(pass != 1)
	{
		// pass is 0=everything or 2=blend
		if(m_TransparentVAO.VAO())
		{
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_TransparentVAO.SelectGL();
			m_TransparentVAO.DrawGL();
			m_TransparentVAO.DeselectGL();
			re.glDisable(GL_BLEND);
		}
	}
	return m_TransparentVAO.VAO() ? true : false;	// Return true if I have any transparent parts to render.
}

double AuSoHGLModelInstance::RoughRadius() const
{
	AuPoint min,max;
	GetExtents(min, max);
	double rmax = max.Length2();
	double rmin = min.Length2();
	if(rmax > rmin)
		return sqrt(rmax);
	return sqrt(rmin);
}

void AuSoHGLModelInstance::ResourceReport(QDataStream& out) const
{
	out << m_FileName;
	out << (quint16)1;	// 1=AuSoHGLModelInstance
}

quint64 AuSoHGLModelInstance::MemoryUsage() const
{
	quint64 mem = sizeof(AuSoHGLModelInstance);
	mem += StringMemoryUsage(m_FileName);
	return mem;
}

int AuSoHGLModelInstance::AimAxis() const
{
	return 1;	// .SoH models are created aiming down the +Y axis.
}

GROUND_ORIENT AuSoHGLModelInstance::GroundOrientation() const
{
	if(!m_Design)
		return GORI_ZPosUp;
	return m_Design->GroundOrientation();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

Au3dsGLSLModelInstance* AuGLModelCache::FindBodyModel(const QString &fileName, double OverallScale, double iScale, double uThick,
		double uLength, double lThick, double lLength, double hwLength, const AuPoint& loc, int pb, int tb, int nb, QColor tC,
		float tS, QColor bC, float bS, float glow, float glowdim)
{
	for(iterator it = find(fileName); it != end() && it.key() == fileName; it++)
	{
		Au3dsGLSLModelInstance* model3 = dynamic_cast<Au3dsGLSLModelInstance*>(*it);
		if(!model3)
			continue;
		if(OverallScale != 0)
		{
			if(model3->CheckFields(OverallScale, iScale, uThick, uLength, lThick, lLength, hwLength, loc, pb, tb, nb, tC, tS,
						bC, bS, glow, glowdim))
				return model3;
		}
		else
			return model3;
	}
	return 0;
}

Au3dsGLSLModelInstance* AuGLModelCache::CreateBodyModel(AuOpenGLFunctions& GLf, const QString& fileName, AuPoint& min, AuPoint& max,
		bool lightOn, double iScale, double overallScale, int pb, int tb, int nb, const AuPoint& pOffset, double uL, double uT,
		double lL, double lT, double hwL, QColor tC, float tS, QColor bC, float bS, float glow, float glowdim, bool bCreate,
		bool bSkip, bool bArrangement, GLuint destBlendFunc)
{
	// Try and find unique model piece
	Au3dsGLSLModelInstance* pm = FindBodyModel(fileName, overallScale, iScale, uT, uL, lT, lL, hwL, pOffset, pb, tb, nb, tC, tS, bC,
			bS, glow, glowdim);
	if(pm)
		bCreate = false;	// Unique model piece found, no need to create it
	if(!pm)
	{
		// If we were looking for a predefined arrangement of model pieces then they
		// haven't been created yet.  Return without error because this is misleading.
		if(bArrangement)
			return pm;
		pm = new Au3dsGLSLModelInstance(fileName);
		if(pm)
		{
			if(pm->CreateBodyDeviceObjects(GLf, fileName, lightOn, iScale, overallScale,
						pb, tb, nb, pOffset, uL, uT, lL, lT, hwL,
						tC, tS, bC, bS, glow, glowdim, bCreate, bSkip, destBlendFunc))
			{
				pm->GetExtents(min, max);
				insert(fileName, pm);
			}
			else
			{
				qWarning() << "AuGLModelCache::CreateModel AuGLModelInstance::ReCreateDeviceObjects FAILED." << fileName;
				delete pm;
				pm = 0;
			}
		}
		else
		{
			qWarning() << "AuGLModelCache::CreateModel 'new AuGLModelInstance' FAILED." << fileName;
		}
	}
	else
	{
		if(bCreate)
		{
			Au3dsGLSLModelInstance* pModel = new Au3dsGLSLModelInstance(fileName);
			if(pm->GenerateBodyDeviceObjects(GLf, pModel, fileName, lightOn, iScale, overallScale,
						pb, tb, nb, pOffset, uL, uT, lL, lT, hwL,
						tC, tS, bC, bS, glow, glowdim, bCreate, bSkip, destBlendFunc))
			{
				// Get extents
				pModel->GetExtents(min, max);

				// Add unique model to cache
				insert(fileName, pModel);
				return pModel;
			}
			else
			{
				qWarning() << "AuGLModelCache::CreateModel AuGLModelInstance::GenerateBodyDeviceObjects FAILED." << fileName;
				delete pModel;
				pModel = 0;
			}
		}
		pm->GetExtents(min, max);
	}
	return pm;
}

Au3dsGLSLModelInstance* AuGLModelCache::CreateBodyModel(AuOpenGLFunctions& GLf, const QString& fileName, AuPoint& min, AuPoint& max,
		bool lightOn, Au3dsGLSLModelDefinition modelDef)
{
	return CreateBodyModel(GLf, fileName, min, max, lightOn, modelDef.m_rScale, modelDef.m_rOverallScale, modelDef.m_iPrevBone,
			modelDef.m_iThisBone, modelDef.m_iNextBone, modelDef.m_pOffset, modelDef.m_ruLength, modelDef.m_ruThickness,
			modelDef.m_rlLength, modelDef.m_rlThickness, modelDef.m_rhwLength, modelDef.m_fTColor, modelDef.m_fTShininess,
			modelDef.m_fBColor, modelDef.m_fBShininess, modelDef.m_fGlow, modelDef.m_fGlowdim, modelDef.m_bCreate, modelDef.m_bSkip,
			modelDef.m_bArrangement, modelDef.m_idestBlendFunc);
}

void AuGLModelCache::CreatePatchedModel(AuOpenGLFunctions& GLf, Au3dsGLSLModelInstance* pm)
{
	pm->CreatePatchedModel(GLf);
	// Add unique model to cache
	insert("patched", pm);
}

AuGLModelInstance* AuGLModelCache::CreateModel(const QString& fileName, AuGLEffects& re, bool lightOn, double scale)
{
	// Check for model instance already loaded.
	AuGLModelInstance* pm = FindModel(fileName, scale);
	if(pm)
		return pm;

	// Create a new model instance.
	pm = AuGLModelInstance::Create(fileName);
	if(!pm)
	{
		qWarning() << "AuGLModelCache::CreateModel 'AuGLModelInstance::Create' FAILED." << fileName;
		return 0;
	}

	if(pm->CreateDeviceObjects(re, lightOn, scale))
	{
		insert(fileName, pm);
	}
	else
	{
		qWarning() << "AuGLModelCache::CreateModel CreateDeviceObjects FAILED." << fileName;
		delete pm;
		pm = 0;
	}
	return pm;
}

AuGLModelInstance* AuGLModelCache::LoadModelNow(const QString& fileName, bool lightOn, double scale)
{
	Q_ASSERT(!fileName.isEmpty());
	// Check for model instance already loaded.
	AuGLModelInstance* pm = FindModel(fileName, scale);
	if(pm)
	{
		//qDebug() << "AuGLModelCache::LoadModel" << fileName << "Using cached model.";
		return pm;
	}

	// Create a new model instance.
	pm = AuGLModelInstance::Create(fileName);
	if(!pm)
	{
		qWarning() << "AuGLModelCache::LoadModel 'AuGLModelInstance::Create' FAILED." << fileName;
		return 0;
	}

	//qDebug() << "AuGLModelCache::LoadModel" << fileName << "Creating a new model instance.";
	insert(fileName, pm);

	// Start a job in the thread pool to load the model.
	pm->LoadModel(lightOn, scale);
	return pm;
}

AuGLModelInstance* AuGLModelCache::LoadModelThreaded(const QString& fileName, bool lightOn, double scale)
{
	Q_ASSERT(!fileName.isEmpty());
	// Check for model instance already loaded.
	AuGLModelInstance* pm = FindModel(fileName, scale);
	if(pm)
	{
		//qDebug() << "AuGLModelCache::LoadModelThreaded" << fileName << "Using cached model.";
		return pm;
	}

	// Create a new model instance.
	pm = AuGLModelInstance::Create(fileName);
	if(!pm)
	{
		qWarning() << "AuGLModelCache::LoadModelThreaded 'AuGLModelInstance::Create' FAILED." << fileName;
		return 0;
	}

	//qDebug() << "AuGLModelCache::LoadModelThreaded" << fileName << "Creating a new model instance.";
	insert(fileName, pm);

	// Start a job in the thread pool to load the model.
	pm->QueueLoadModel(lightOn, scale);
	return pm;
}

void AuGLModelCache::ResourceReport(QDataStream& out) const
{
	out << (quint32)count();
	for(const_iterator mi = constBegin(); mi != constEnd(); mi++)
		(*mi)->ResourceReport(out);
}

quint64 AuGLModelCache::MemoryUsage() const
{
	quint64 mem = sizeof(AuGLModelCache);
	for(const_iterator mi = constBegin(); mi != constEnd(); mi++)
		mem += (*mi)->MemoryUsage();
	return mem;
}

