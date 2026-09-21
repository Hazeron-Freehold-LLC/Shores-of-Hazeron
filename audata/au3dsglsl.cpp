//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: au3dsglsl.cpp,v 1.41 2022/04/08 14:35:55 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#if defined(Q_OS_WIN)
#undef AuGLDLL		// So gl extension function prototypes work in Windows.
#endif
#include "au3dsglsl.h"
//
#include "aubox.h"
#include "augleffects.h"
#include "auline.h"
#include "aumatrix3x3.h"
#include "auplane.h"
#include <QFile>
#include <QThreadPool>

// Note: I tried to use a namespace here instead of a class but could not
// eliminate compiler warnings when s3ds.h was included from sgleffects.h.
//using namespace Au3dsGLSL;

static bool s_ShowErrors = false;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Load the subchunks of the EDIT_MATERIAL chunk.
bool Au3dsGLSLMaterial::LoadChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType)
{
	//qDebug() << "Au3dsGLSLMaterial::LoadChunk" << chunkSize << modelType;
	if(!chunkSize)
		return IsValid();

	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLMaterial::LoadChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in chunk",
					subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case MAT_NAME01:
			{
				// Name data is an "ASCIIZ" string.
				m_Name = Au3dsGLSL::LoadString(in);
				//qDebug() << "MAT_NAME01" << m_Name;
			}
			break;
		case MAT_TEXTURE1_MAP:
			{
				// Image file name data is an "ASCIIZ" string.
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_TEXTURE1_MAP subchunk of %d bytes", subChunkSize);
				LoadTextureChunk(in, subChunkSize - chunkHeaderSize);
			}
			break;
		case MAT_BUMP_MAP:
			//qDebug("Au3dsGLSLMaterial::LoadChunk skipping MAT_BUMP_MAP subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		case MAT_AMBIENTCOLOR:
			{
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_AMBIENTCOLOR subchunk of %d bytes", subChunkSize);
				LoadColorChunk(in, subChunkSize - chunkHeaderSize, m_AmbientColor);
			}
			break;
		case MAT_DIFFUSECOLOR:
			{
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_DIFFUSECOLOR subchunk of %d bytes", subChunkSize);
				LoadColorChunk(in, subChunkSize - chunkHeaderSize, m_DiffuseColor);
			}
			break;
		case MAT_SPECULARCOLOR:
			{
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_SPECULARCOLOR subchunk of %d bytes", subChunkSize);
				LoadColorChunk(in, subChunkSize - chunkHeaderSize, m_SpecularColor);
			}
			break;
		case MAT_SHININESS:
			{
				m_Shininess = LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType);
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_SHININESS subchunk of %d bytes, val %f",
						//subChunkSize, m_Shininess);
			}
			break;
		case MAT_SHINSTRENGTH:
			{
				m_ShineStrength = (qint16)LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType);
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_SHINSTRENGTH subchunk of %d bytes, val %d",
						//subChunkSize, m_ShineStrength);
			}
			break;
		case MAT_TRANSPARENCY:
			{
				m_Transparency = LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType);
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_TRANSPARENCY subchunk of %d bytes, val %f", subChunkSize,
						//m_Transparency);
			}
			break;
		case MAT_TRANSFALLOFF:
			{
				LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType);
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_TRANSFALLOFF subchunk of %d bytes", subChunkSize);
			}
			break;
		case MAT_TRANS_FALLOF_IN:
			{
				//qDebug("Au3dsGLSLMaterial::LoadChunk skipping MAT_TRANS_FALLOF_IN subchunk of %d bytes", subChunkSize);
				in.skipRawData(subChunkSize - chunkHeaderSize);
			}
			break;
		case MAT_REFLECTBLUR:
			{
				m_ReflectBlur = (qint16)LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType);
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_REFLECTBLUR subchunk of %d bytes, val %d",
						//subChunkSize, m_ReflectBlur);
			}
			break;
		case MAT_SELFILLUM:
			{
				if(IsSymmetricaModel(modelType))
					m_SelfIllumination = LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType);
				else
					m_SelfIllumination = LoadAmountChunk(in, subChunkSize - chunkHeaderSize, modelType)*100;
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_SELFILLUM subchunk of %d bytes, val %f", subChunkSize,
						//m_SelfIllumination);
			}
			break;
		case MAT_SELFILLUM_BOOL:
			{
				m_SelfIllumination = 100;
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_SELFILLUM_BOOL subchunk of %d bytes, val %f",
						//subChunkSize, m_SelfIllumination);
			}
			break;
		case MAT_TYPE:
			{
				m_Type = Au3dsGLSL::LoadUint16(in);
				//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_TYPE subchunk of %d bytes, val %d", subChunkSize, m_Type);
			}
			break;
		case MAT_WIRETHICKNESS:
			//qDebug("Au3dsGLSLMaterial::LoadChunk skipping MAT_THICKNESS subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		case MAT_TWO_SIDED:
			//qDebug("Au3dsGLSLMaterial::LoadChunk loading MAT_TWO_SIDED subchunk of %d bytes", subChunkSize);
			m_TwoSided = true;
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		default:
			//qWarning("Au3dsGLSLMaterial::LoadChunk skipping %04x subchunk of %d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);

	//qDebug() << "Au3dsGLSLMaterial::LoadChunk" << m_Name;
	if(m_Name.left(5) == "Light" && m_SelfIllumination == 0.)
	{
		//DHH Force self illumination based on material name since I can't seem to get material emission property to export from
		//DHH Blender. Probably my mistake.
		//qDebug() << "Au3dsGLSLMaterial::LoadChunk material" << m_Name << "should be self illuminated. Making it so.";
		m_SelfIllumination = 100.;
	}

	if(m_Name.contains("Ta:"))
	{
		//DHH Force transparency based on material name since I can't seem to get material transparency alpha to export from
		//DHH Blender. Probably my mistake.
		int t = m_Name.indexOf("Ta:");
		QString astr = m_Name.mid(t+3);
		m_Transparency = astr.toFloat();
		if(m_Transparency == 0.f)
			if(s_ShowErrors)
			qDebug() << "Au3dsGLSLMaterial::LoadChunk error: material" << m_Name << "should be transparent. Transparency"
				<< m_Transparency;
	}

	return IsValid();
}

float Au3dsGLSLMaterial::LoadAmountChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType)
{
	// Load the subchunks.
	float amount = 0;
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLMaterial::LoadAmountChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in "
					"chunk", subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case MAT_AMOUNT:
			{
				if(IsSymmetricaModel(modelType) || (subChunkSize - chunkHeaderSize) == 2)
					amount = Au3dsGLSL::LoadUint16(in);
				else
					amount = Au3dsGLSL::LoadFloat(in);
				//qDebug("Au3dsGLSLMaterial::LoadAmountChunk loaded MAT_AMOUNT subchunk of %d bytes, value %f",
						//subChunkSize, amount);
			}
			break;
		default:
			if(s_ShowErrors)
			qWarning("Au3dsGLSL::LoadAmountChunk skipping %04x subchunk of %d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);
	return amount;
}

bool Au3dsGLSLMaterial::LoadColorChunk(QDataStream& in, int chunkSize, QColor color[3])
{
	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLMaterial::LoadColorChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in "
					"chunk", subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case COL_RGBF:
			{
				float r = Au3dsGLSL::LoadFloat(in);
				float g = Au3dsGLSL::LoadFloat(in);
				float b = Au3dsGLSL::LoadFloat(in);
				//qDebug("Au3dsGLSLMaterial::LoadColorChunk loaded COL_RGBF "
				//"subchunk of %d bytes, RGB %g,%g,%g", subChunkSize,
				//r,g,b);
				color[0].setRgbF(r,g,b);
			}
			break;
		case COL_RGB1:
			{
				quint8 r,g,b;
				in >> r;
				in >> g;
				in >> b;
				//qDebug("Au3dsGLSLMaterial::LoadColorChunk loaded COL_RGB1 "
				//"subchunk of %d bytes, RGB %d,%d,%d", subChunkSize,
				//r,g,b);
				color[1].setRgb(r,g,b);
			}
			break;
		case COL_RGB2:
			{
				quint8 r,g,b;
				in >> r;
				in >> g;
				in >> b;
				//qDebug("Au3dsGLSLMaterial::LoadColorChunk loaded COL_RGB2 "
				//"subchunk of %d bytes, RGB %d,%d,%d", subChunkSize,
				//r,g,b);
				color[2].setRgb(r,g,b);
			}
			break;
		default:
			if(s_ShowErrors)
			qWarning("Au3dsGLSL::LoadColorChunk skipping %04x subchunk of %d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);
	return true;
}

// Load the subchunks of the MAT_TEXTURE1_MAP chunk.
bool Au3dsGLSLMaterial::LoadTextureChunk(QDataStream& in, int chunkSize)
{
	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLMaterial::LoadTextureChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in "
					"chunk", subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case TEX_FILENAME:
			m_TextureFile = Au3dsGLSL::LoadString(in);
			// A lot of the 3ds files exported from blender for the character
			// models are using a .tga file because the .png files would not load
			// properly in blender. Change the name to the actual file the character
			// pieces will be using.
			if(m_TextureFile == QString("armUvTest.tga"))
				m_TextureFile = QString("txBody.png");
			break;
		case TEX_AMOUNT:
			m_TextureAmount = (qint16)Au3dsGLSL::LoadUint16(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_AMOUNT "
			//"subchunk of %d bytes, value %d", subChunkSize,
			//m_TextureAmount);
			break;
		case TEX_MAP_OPTION:
			m_TexMapOption = (qint16)Au3dsGLSL::LoadUint16(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_OPTION "
			//"subchunk of %d bytes, value %d", subChunkSize,
			//m_TexMapOption);
			break;
		case TEX_MAP_FILTBLUR:
			m_TexMapFilterBlur = Au3dsGLSL::LoadFloat(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_FILTBLUR "
			//"subchunk of %d bytes, value %g", subChunkSize,
			//m_TexMapFilterBlur);
			break;
		case TEX_MAP_USCALE:
			m_TexMapScaleU = Au3dsGLSL::LoadFloat(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_USCALE "
			//"subchunk of %d bytes, value %g", subChunkSize,
			//m_TexMapScaleU);
			break;
		case TEX_MAP_VSCALE:
			m_TexMapScaleV = Au3dsGLSL::LoadFloat(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_VSCALE "
			//"subchunk of %d bytes, value %g", subChunkSize,
			//m_TexMapScaleV);
			break;
		case TEX_MAP_UOFFSET:
			m_TexMapOffsetU = Au3dsGLSL::LoadFloat(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_UOFFSET "
			//"subchunk of %d bytes, value %g", subChunkSize,
			//m_TexMapOffsetU);
			break;
		case TEX_MAP_VOFFSET:
			m_TexMapOffsetV = Au3dsGLSL::LoadFloat(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_VOFFSET "
			//"subchunk of %d bytes, value %g", subChunkSize,
			//m_TexMapOffsetV);
			break;
		case TEX_MAP_ROTATION:
			m_TexMapRotation = Au3dsGLSL::LoadFloat(in);
			//qDebug("Au3dsGLSLMaterial::LoadTextureChunk loaded TEX_MAP_ROTATION "
			//"subchunk of %d bytes, value %g", subChunkSize,
			//m_TexMapRotation);
			break;
		case TEX_MAP_LUMTINT1:
		case TEX_MAP_LUMTINT2:
		case TEX_MAP_RGBTINTR:
		case TEX_MAP_RGBTINTG:
		case TEX_MAP_RGBTINTB:
			{
				quint8 r,g,b;
				in >> r;
				in >> g;
				in >> b;
				//qDebug("Au3dsGLSLMaterial::LoadColorChunk loaded TEX_MAP color "
				//"subchunk of %d bytes, RGB %d,%d,%d", subChunkSize,
				//r,g,b);
				//color.setRgb(r,g,b);
			}
			break;
		default:
			if(s_ShowErrors)
			qWarning("Au3dsGLSLMaterial::LoadTextureChunk skipping %04x subchunk of %d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);
	return true;
}

void Au3dsGLSLMaterial::Dump() const
{
	if(m_TextureFile.isEmpty())
		qDebug() << m_Name;
	else
		qDebug() << m_Name << "file" << m_TextureFile;
}

GLuint Au3dsGLSLMaterial::CreateDeviceObject(const QString& filePath, AuGLEffects& re)
{
	// Texture names incorporate the path of the 3ds file so they are unique
	// between models. Otherwise two different models might use the same file
	// name and there would be a problem.
	Q_ASSERT(!filePath.isEmpty());
	//qDebug() << "Au3dsGLSLMaterial::CreateDeviceObject" << filePath << m_TextureFile;
	if(!m_TextureFile.isEmpty())
	{
		QString name = filePath + "/" + m_TextureFile;
		return re.CreateTextureFromBitmap(STATIC_MODEL_ID, name, name);
	}
	return 0;
}

quint64 Au3dsGLSLMaterial::MemoryUsage() const
{
	quint64 mem = sizeof(Au3dsGLSLMaterial);
	mem += StringMemoryUsage(m_Name);
	mem += StringMemoryUsage(m_TextureFile);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

GLuint Au3dsGLSLMaterialArray::CreateDeviceObjects(const QString& filePath, AuGLEffects& re)
{
	GLuint texId = 0;
	for(iterator it = begin(); it != end(); it++)
	{
		// We currently require that each 3ds object only reference a single
		// texture for efficiency.
		if(!texId)
			texId = (*it).CreateDeviceObject(filePath, re);
		else
			(*it).CreateDeviceObject(filePath, re);
	}

	// Prevent the texture from ever getting deleted by claiming it with a fictitious user id.
	if(texId)
		re.UseTexture(STATIC_MODEL_ID, texId);

	return texId;
}

int Au3dsGLSLMaterialArray::IndexOf(const QString& name) const
{
	int index = 0;
	for(const_iterator it = constBegin(); it != constEnd(); it++, index++)
		if((*it).Name() == name)
			return index;
	return -1;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

// Load the subchunks of the EDIT_OBJECT chunk.
bool Au3dsGLSLObject::LoadChunk(QDataStream& in, int chunkSize, AuPoint& min, AuPoint& max, MODEL_TYPE modelType)
{
	// First item is an "ASCIIZ" string.
	m_Name = Au3dsGLSL::LoadString(in);
	chunkSize -= m_Name.length() + 1;
	if(m_Name.contains("Light"))
		m_bLight = true;

	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLObject::LoadChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in chunk",
					subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case OBJ_TRIMESH:
			//qDebug("Au3dsGLSLObject::LoadChunk loading OBJ_TRIMESH subchunk of "
			//"%d bytes", subChunkSize);
			LoadMeshChunk(in, subChunkSize - chunkHeaderSize, min, max, modelType);
			break;
		default:
			if(s_ShowErrors)
			qWarning("Au3dsGLSLObject::LoadChunk skipping %04x subchunk of %d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);
	return IsValid(modelType);
}

// Load the subchunks of the OBJ_TRIMESH chunk.
bool Au3dsGLSLObject::LoadMeshChunk(QDataStream& in, int chunkSize, AuPoint& min, AuPoint& max, MODEL_TYPE modelType)
{
	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		if(!subChunkSize)
		{
			if(s_ShowErrors)
			qWarning() << "Au3dsGLSLObject::LoadMeshChunk name" << m_Name << "chunkSize" << chunkSize
				<< "subChunkName" << subChunkName << "subChunkSize ZERO!";
			return false;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case TRI_VERTEXL:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk loading TRI_VERTEXL subchunk "
			//"of %d bytes", subChunkSize);
			LoadVertexListChunk(in, subChunkSize - chunkHeaderSize);
			break;
		case TRI_VERTEXOPTIONS:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk skipping TRI_VERTEXOPTIONS "
			//"subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		case TRI_VERTEXGROUPS:
			{
				Au3dsGLSLVGroup vertexGroup;
				if(vertexGroup.LoadChunk(in, subChunkSize - chunkHeaderSize))
				{
					// Note: Any vertex not in BottomSkin group is considered TopSkin
					if(vertexGroup.m_Type == VERT_ConnectPt)
						m_ConnectPt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_HandWingPt)
						m_HandWingPt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_EyePt)
						m_EyePt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_TopScalePt)
						m_TopScalePt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_MidScalePt)
						m_MidScalePt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_BottomScalePt)
						m_BottomScalePt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_WeaponUpDownPt)
						m_WeaponUpDownPt = vertexGroup.m_ScalePt;
					else if(vertexGroup.m_Type == VERT_WeaponLeftRightPt)
						m_WeaponLeftRightPt = vertexGroup.m_ScalePt;
					else
					{
						// Everything else is added to our list of vertex groups
						if(vertexGroup.m_Type == VERT_MidPt)
							m_MidPt = vertexGroup.m_ScalePt;
						if(vertexGroup.m_Type == VERT_Weight)
							m_bHasBones = true;
						m_VertexGroups.append(vertexGroup);
					}
				}
			}
			break;
		case TRI_FACELIST:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk loading TRI_FACELIST subchunk "
			//"of %d bytes", subChunkSize);
			LoadFaceListChunk(in, subChunkSize - chunkHeaderSize, modelType);
			break;
		case TRI_MAPPINGCOORDS:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk loading TRI_MAPPINGCOORDS "
			//"subchunk of %d bytes", subChunkSize);
			if(LoadMappingCoordsChunk(in, subChunkSize - chunkHeaderSize) && !IsSymmetricaModel(modelType))
				SetMappingCoords();
			break;
		case TRI_MATRIX:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk loading TRI_MATRIX subchunk of "
			//"%d bytes", subChunkSize);
			LoadMatrixChunk(in, subChunkSize - chunkHeaderSize, modelType);
			break;
		case TRI_VISIBLE:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk skipping TRI_VISIBLE subchunk "
			//"of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		case TRI_STANDARDMAPPING:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk skipping TRI_STANDARDMAPPING "
			//"subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		default:
			//qDebug("Au3dsGLSLObject::LoadMeshChunk skipping %04x subchunk of "
			//		"%d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);

	// Calculate min/max 3d extents of vertices.
	bool extentsValid = false;
	for(Au3dsGLSLVertexArray::const_iterator it = m_Vertices.constBegin();
		it != m_Vertices.constEnd(); it++)
	{
		AuPoint vertex((*it).X(), (*it).Y(), (*it).Z());
		vertex = m_Matrix.Project(vertex);
		if(!extentsValid)
		{
			// First vertex.
			min = vertex;
			max = vertex;
			extentsValid = true;
		}
		else
		{
			// Expand extents using the vertex.
			min = min.Min(vertex);
			max = max.Max(vertex);
		}
	}

	return true;
}

bool Au3dsGLSLVGroup::LoadChunk(QDataStream& in, int chunkSize)
{
	//qDebug("Au3dsGLSLVGroup::LoadChunk started");
	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLVGroup::LoadChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in chunk",
					subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case VERT_GROUPNAME:
			m_Name = Au3dsGLSL::LoadString(in);
			// Note: Any vertex not in BottomSkin group is considered TopSkin
			if(m_Name.contains("ScalePt"))
				m_Type = VERT_ScalePt;
			else if(m_Name.contains("ConnectPt"))
				m_Type = VERT_ConnectPt;
			else if(m_Name.contains("MidPt"))
				m_Type = VERT_MidPt;
			else if(m_Name == "Top")
				m_Type = VERT_Top;
			else if(m_Name == "Bottom")
				m_Type = VERT_Bottom;
			else if(m_Name.contains("BottomSkin"))
				m_Type = VERT_BottomSkin;
			else if(m_Name == "TopConnect")
				m_Type = VERT_TopConnect;
			else if(m_Name == "BottomConnect")
				m_Type = VERT_BottomConnect;
			else if(m_Name == "HandWingPt")
				m_Type = VERT_HandWingPt;
			else if(m_Name == "EyePosition")
				m_Type = VERT_EyePt;
			else if(m_Name == "TopScalePt")
				m_Type = VERT_TopScalePt;
			else if(m_Name == "MidScalePt")
				m_Type = VERT_MidScalePt;
			else if(m_Name == "BottomScalePt")
				m_Type = VERT_BottomScalePt;
			else if(m_Name.contains("Torso"))
				m_bTorsoSegment = true;
			else if(m_Name.contains("Hand"))
				m_bHand = true;
			else if(m_Name.contains("Foot"))
				m_bFoot = true;
			else if(m_Name.contains("Neck"))
				m_bNeck = true;
			else if(m_Name.contains("Tail"))
				m_bTail = true;
			else if(m_Name.contains("Head"))
				m_bHead = true;
			else if(m_Name == "SeatLoc")
				m_Type = VERT_SeatLoc;
			else if(m_Name == "OffLoad" || m_Name == "Offload")
				m_Type = VERT_OffLoad;
			else if(m_Name == "WeaponLoc")
				m_Type = VERT_WeaponLoc;
			else if(m_Name == "DirLoc")
				m_Type = VERT_DirLoc;
			else if(m_Name == "WeaponLRPt")
				m_Type = VERT_WeaponLeftRightPt;
			else if(m_Name == "WeaponUDPt")
				m_Type = VERT_WeaponUpDownPt;
			else if(m_Name == "Screen1")
				m_Type = VERT_Screen1;
			else if(m_Name == "Screen2")
				m_Type = VERT_Screen2;

			// Defense Bases
			else if(m_Name == "FlagDown" || m_Name == "Flag Down")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgFlagBottom %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgFlagBottom;
			}
			else if(m_Name == "FlagUp" || m_Name == "Flag Up")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgFlagTop %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgFlagTop;
			}
			else if(m_Name == "Missile Control" || m_Name == "MissileControl" || m_Name == "Missle Control")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgMissileControl %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgMissileControl;
			}
			else if(m_Name == "Missile Deploy" || m_Name == "MissleDeploy" || m_Name == "MissileDeploy")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgMissileDeploy %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgMissileDeploy;
			}
			else if(m_Name == "Seat1")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgSeat1 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgSeat1;
			}
			else if(m_Name == "Seat2")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgSeat2 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgSeat2;
			}
			else if(m_Name == "Seat3")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgSeat3 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgSeat3;
			}
			else if(m_Name == "Stand1")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgStand1 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgStand1;
			}
			else if(m_Name == "Stand2")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgStand2 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgStand2;
			}
			else if(m_Name == "Stand3")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgStand3 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgStand3;
			}
			else if(m_Name == "Stand4")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgStand4 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgStand4;
			}
			else if(m_Name == "Turret 1" || m_Name == "Turret1")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgTurret1 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgTurret1;
			}
			else if(m_Name == "Turret 2" || m_Name == "Turret2")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgTurret2 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgTurret2;
			}
			else if(m_Name == "Turret 3" || m_Name == "Turret3")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgTurret3 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgTurret3;
			}
			else if(m_Name == "Turret 4" || m_Name == "Turret4")
			{
				//qDebug("Au3dsGLSLVGroup::LoadChunk VERT_BldgTurret4 %s", (const char*)m_Name.toAscii());
				m_Type = VERT_BldgTurret4;
			}
			//else if(!m_Name.isEmpty())
				//qDebug("Au3dsGLSLVGroup::LoadChunk unhandled vertex group name %s", (const char*)m_Name.toAscii());
			break;
		case VERT_LIST:
			LoadVertexGroupChunk(in, subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while (chunkSize > 0);

	//qDebug("Au3dsGLSLVGroup::LoadChunk done");
	return true;
}

bool Au3dsGLSLVGroup::LoadVertexGroupChunk(QDataStream& in, int chunkSize)
{
	quint16 vertexCount = Au3dsGLSL::LoadUint16(in);
	chunkSize -= sizeof(quint16);

	// Last coordinates are the scale/connect location if this is a scaling/connecting
	// vertex group.  Some groups only export the location.
	if(vertexCount == 1)
	{
		float x = Au3dsGLSL::LoadFloat(in);
		float y = Au3dsGLSL::LoadFloat(in);
		float z = Au3dsGLSL::LoadFloat(in);
		Au3dsGLSL::LoadFloat(in);			// Must read in weight even if we don't use it
		chunkSize -= sizeof(float) * 4;
		if(m_Type == VERT_ConnectPt || m_Type == VERT_HandWingPt || m_Type == VERT_EyePt ||
				m_Type == VERT_TopScalePt || m_Type == VERT_MidScalePt || m_Type == VERT_BottomScalePt ||
				m_Type == VERT_SeatLoc || m_Type == VERT_OffLoad || m_Type == VERT_WeaponLoc || m_Type == VERT_DirLoc ||
				m_Type == VERT_BldgFlagBottom || m_Type == VERT_BldgFlagTop ||
				m_Type == VERT_BldgSeat1 || m_Type == VERT_BldgSeat2 || m_Type == VERT_BldgSeat3 ||
				m_Type == VERT_BldgStand1 || m_Type == VERT_BldgStand2 || m_Type == VERT_BldgStand3 || m_Type == VERT_BldgStand4)
			m_ScalePt = Au3dsGLSLVertex(x,y,z);
		else
			if(s_ShowErrors)
			qWarning("warning: Unexpected single-vertex vertex group type=%d", m_Type);
	}
	else
	{
		/*
		if(m_Type == VERT_SeatLoc || m_Type == VERT_OffLoad || m_Type == VERT_WeaponLoc || m_Type == VERT_DirLoc ||
				m_Type == VERT_BldgFlagBottom || m_Type == VERT_BldgFlagTop ||
				m_Type == VERT_BldgSeat1 || m_Type == VERT_BldgSeat2 || m_Type == VERT_BldgSeat3 ||
				m_Type == VERT_BldgStand1 || m_Type == VERT_BldgStand2 || m_Type == VERT_BldgStand3 || m_Type == VERT_BldgStand4)
			qWarning("warning: Vertex group type=%d was expected to have only 1 vertex; has %d vertices", m_Type, vertexCount);
			*/
		for(;vertexCount;vertexCount--)
		{
			float x = Au3dsGLSL::LoadFloat(in);
			float y = Au3dsGLSL::LoadFloat(in);
			float z = Au3dsGLSL::LoadFloat(in);
			float w = Au3dsGLSL::LoadFloat(in);
			chunkSize -= sizeof(float) * 4;
			if(vertexCount == 1 && (HasScalePt(m_Type) || m_bTorsoSegment || m_bHand || m_bFoot || m_bNeck || m_bTail || m_bHead))
				m_ScalePt = Au3dsGLSLVertex(x,y,z);
			else
				m_VertexList.insert(Au3dsGLSLVertex(x,y,z), w);		// coords, weight
		}
	}

	if(chunkSize)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLVGroup::LoadVertexGroupChunk skipping remaining %d "
			"bytes", chunkSize);
		in.skipRawData(chunkSize);
	}

	return true;
}

quint64 Au3dsGLSLVGroup::MemoryUsage() const
{
	quint64 mem = sizeof(Au3dsGLSLVGroup);
	mem += StringMemoryUsage(m_Name);
	mem += (sizeof(Au3dsGLSLVertex) + sizeof(double)) * m_VertexList.size();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool Au3dsGLSLObject::LoadVertexListChunk(QDataStream& in, int chunkSize)
{
	quint16 vertexCount = Au3dsGLSL::LoadUint16(in);
	chunkSize -= sizeof(quint16);
	//qDebug("Au3dsGLSLObject::LoadVertexListChunk %d vertices", vertexCount);
	for(;vertexCount;vertexCount--)
	{
		float x = Au3dsGLSL::LoadFloat(in);
		float y = Au3dsGLSL::LoadFloat(in);
		float z = Au3dsGLSL::LoadFloat(in);
		chunkSize -= sizeof(float) * 3;
		//qDebug("Au3dsGLSLObject::LoadVertexListChunk  (%g,%g,%g)", x,y,z);
		Au3dsGLSLVertex vertex = Au3dsGLSLVertex(x,y,z);
		m_Vertices.append(vertex);
	}

	// Project the vertices through the inverse matrix. They will get projected
	// back at render time. The difference is the location of the pivot point
	// if we rotate a piece. e.g. a tank turret.
	if(!m_Matrix.IsIdentity())
		m_Vertices.Project(m_Matrix.Inverse());

	if(chunkSize)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLObject::LoadVertexListChunk skipping remaining %d bytes",
			chunkSize);
		in.skipRawData(chunkSize);
	}

	return true;
}

bool Au3dsGLSLObject::LoadFaceListChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType)
{
	quint16 faces = Au3dsGLSL::LoadUint16(in);
	chunkSize -= sizeof(quint16);
	//qDebug("Au3dsGLSLObject::LoadFaceListChunk %d faces", faces);
	for(quint16 face=0; face<faces; face++)
	{
		quint16 a = Au3dsGLSL::LoadUint16(in);
		quint16 b = Au3dsGLSL::LoadUint16(in);
		quint16 c = Au3dsGLSL::LoadUint16(in);
		quint16 info = Au3dsGLSL::LoadUint16(in);
		chunkSize -= sizeof(quint16) * 4;
		AuPoint vn0, vn1, vn2;
		AuPoint vt0, vt1, vt2;
		float w = 1;
		if(HasPrecalculatedNormals(modelType))
		{
			// Below is not a standard face chunk.  It was modified in blender so that
			// faces would pre-calculate there face normals, tangents, and UV handedness
			// to save on load time.
			// Get the 3 normals
			float x = Au3dsGLSL::LoadFloat(in);
			float y = Au3dsGLSL::LoadFloat(in);
			float z = Au3dsGLSL::LoadFloat(in);
			chunkSize -= sizeof(float) * 3;
			vn0 = AuPoint(x,y,z);
			x = Au3dsGLSL::LoadFloat(in);
			y = Au3dsGLSL::LoadFloat(in);
			z = Au3dsGLSL::LoadFloat(in);
			chunkSize -= sizeof(float) * 3;
			vn1 = AuPoint(x,y,z);
			x = Au3dsGLSL::LoadFloat(in);
			y = Au3dsGLSL::LoadFloat(in);
			z = Au3dsGLSL::LoadFloat(in);
			chunkSize -= sizeof(float) * 3;
			vn2 = AuPoint(x,y,z);

			if(HasPrecalculatedTangents(modelType))
			{
				// Get the 3 tangents
				x = Au3dsGLSL::LoadFloat(in);
				y = Au3dsGLSL::LoadFloat(in);
				z = Au3dsGLSL::LoadFloat(in);
				chunkSize -= sizeof(float) * 3;
				vt0 = AuPoint(x,y,z);
				x = Au3dsGLSL::LoadFloat(in);
				y = Au3dsGLSL::LoadFloat(in);
				z = Au3dsGLSL::LoadFloat(in);
				chunkSize -= sizeof(float) * 3;
				vt1 = AuPoint(x,y,z);
				x = Au3dsGLSL::LoadFloat(in);
				y = Au3dsGLSL::LoadFloat(in);
				z = Au3dsGLSL::LoadFloat(in);
				chunkSize -= sizeof(float) * 3;
				vt2 = AuPoint(x,y,z);

				w = Au3dsGLSL::LoadFloat(in);
				chunkSize -= sizeof(float);
			}
		}
		//qDebug("Au3dsGLSLObject::LoadFaceListChunk  %d %d %d %02x", a,b,c,info);
		m_Faces.append(Au3dsGLSLFace(a,b,c,info,vn0,vn1,vn2,vt0,vt1,vt2,w));
	}

	// Load the subchunks (only if there is some)
	// Ok, here's what happened.  The blender files were exported with the wrong 3dsFace size.
	// This caused the chunkSize value to become negative when reading all the information above.
	// Most of the blender files worked because they all had either a face material chunk or a
	// face smoothing chunk but not both.  However, seat position objects don't have either.  Also, 
	// Face smoothing chunk no longer gets added to the .3dsN files because we are now pre-calculating
	// the face normals.  So instead of always trying to load the subchunks we only need to if there is some.
	// Seat position objects contain '@' in there name so we have to look for that.  The old .3dsN
	// files will happily work just fine without needing to re-export them but they should eventually
	// be re-exported.  When they are then this check could be if(chunkSize>0) instead of looking for seat
	// position objects.  We can't do if(chunkSize>0) now because the old .3dsN files will
	// have a negative chunkSize but yet still need to load the subchunks.
	// Also ignore DataOnly and CollisionFloorN blocks.
	//if((newModel && !(m_Name.left(4) == "Data" || m_Name.left(9) == "Collision" ||
					//m_Name.contains("@") || m_Name.contains("!"))) || !newModel)
	//if((newModel && chunkSize) || !newModel)
	if((HasPrecalculatedNormals(modelType) && chunkSize && !(m_Name.contains("@") || m_Name.contains("!"))) ||
			(!HasPrecalculatedNormals(modelType) && chunkSize))
	{
		const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
		do
		{
			quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
			quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
			if(!subChunkSize)
			{
				if(s_ShowErrors)
				qWarning("Au3dsGLSLObject::LoadFaceListChunk subChunkSize is ZERO!");
			}
			else if(subChunkSize > (quint32)chunkSize)
			{
				if(s_ShowErrors)
				qWarning("error: Au3dsGLSLObject::LoadFaceListChunk bad chunk size %d for subchunk %04x exceeds remaining %d "
						"bytes in chunk",
						subChunkSize, subChunkName, chunkSize);
				subChunkSize = chunkSize;
			}
			chunkSize -= subChunkSize;
			switch(subChunkName)
			{
			case FACE_MATERIAL:
				//qDebug("Au3dsGLSLObject::LoadFaceListChunk loading FACE_MATERIAL "
				//"subchunk of %d bytes", subChunkSize);
				LoadFaceMaterialChunk(in, subChunkSize - chunkHeaderSize);
				break;
			case FACE_SMOOTH:
				//qDebug("Au3dsGLSLObject::LoadFaceListChunk loading FACE_SMOOTH "
				//"subchunk of %d bytes", subChunkSize);
				LoadFaceSmoothingChunk(in, subChunkSize - chunkHeaderSize, faces);
				break;
			default:
				if(s_ShowErrors)
				qWarning("Au3dsGLSLObject::LoadFaceListChunk skipping %04x subchunk of %d bytes", subChunkName, subChunkSize);
				in.skipRawData(subChunkSize - chunkHeaderSize);
				break;
			}
		}
		while(chunkSize > 0);
	}

	return true;
}

bool Au3dsGLSLObject::LoadFaceMaterialChunk(QDataStream& in, int chunkSize)
{
	// First item is an "ASCIIZ" string.
	QString name = Au3dsGLSL::LoadString(in);
	chunkSize -= name.length() + 1;

	// Second item is number of faces.
	quint16 faces = Au3dsGLSL::LoadUint16(in);
	chunkSize -= sizeof(quint16);

	//qDebug("Au3dsGLSLObject::LoadFaceMaterialChunk name '%s' faces %d",
	//(const char*)name.toAscii(), faces);

	QVector<quint16> faceMaterials;
	for(;faces;faces--)
	{
		quint16 material = Au3dsGLSL::LoadUint16(in);
		chunkSize -= sizeof(quint16);
		//qDebug("Au3dsGLSLObject::LoadFaceMaterialChunk  %d", material);
		faceMaterials.append(material);
	}
	if(!faceMaterials.isEmpty())
		m_Materials.insert(name, faceMaterials);

	if(chunkSize)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLObject::LoadFaceMaterialChunk skipping remaining %d "
			"bytes", chunkSize);
		in.skipRawData(chunkSize);
	}

	return true;
}

bool Au3dsGLSLObject::LoadFaceSmoothingChunk(QDataStream& in, int chunkSize,
										quint16 faces)
{
	//qDebug("Au3dsGLSLObject::LoadFaceSmoothingChunk faces %d", faces);
	for(;faces;faces--)
	{
		quint32 smoothing = Au3dsGLSL::LoadUint32(in);
		chunkSize -= sizeof(quint32);
		//qDebug("Au3dsGLSLObject::LoadFaceSmoothingChunk %d", smoothing);
		m_FaceSmoothing.append(smoothing);
	}

	if(chunkSize)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLObject::LoadFaceSmoothingChunk skipping remaining %d "
			"bytes", chunkSize);
		in.skipRawData(chunkSize);
	}

	return true;
}

bool Au3dsGLSLObject::LoadMappingCoordsChunk(QDataStream& in, int chunkSize)
{
	quint16 vertexCount = Au3dsGLSL::LoadUint16(in);
	chunkSize -= sizeof(quint16);
	//qDebug("Au3dsGLSLObject::LoadMappingCoordsChunk %d vertices", vertexCount);
	for(;vertexCount;vertexCount--)
	{
		float s = Au3dsGLSL::LoadFloat(in);
		float t = Au3dsGLSL::LoadFloat(in);
		chunkSize -= sizeof(float) * 2;
		//qDebug("Au3dsGLSLObject::LoadMappingCoordsChunk  %g,%g", s,t);
		m_MappingCoords.append(Au3dsGLSLMappingCoord(s,t));
	}

	if(chunkSize)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLObject::LoadMappingCoordsChunk skipping remaining %d "
			"bytes", chunkSize);
		in.skipRawData(chunkSize);
	}

	return true;
}

bool Au3dsGLSLObject::LoadMatrixChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType)
{
	// Scale Symmetrica vertices by 1/12 because my models are all created in Inch units but the game expects everything in Feet.
	double unitScaling;
	if(IsSymmetricaModel(modelType))
	{
		unitScaling = 1./12.;
		m_Vertices.ScaleBy(unitScaling);
	}
	else
		unitScaling = 1;

	// Load the matrix.
	float mat[4][4];
	mat[0][0] = Au3dsGLSL::LoadFloat(in);
	mat[0][1] = Au3dsGLSL::LoadFloat(in);
	mat[0][2] = Au3dsGLSL::LoadFloat(in);
	mat[0][3] = 0;
	mat[1][0] = Au3dsGLSL::LoadFloat(in);
	mat[1][1] = Au3dsGLSL::LoadFloat(in);
	mat[1][2] = Au3dsGLSL::LoadFloat(in);
	mat[1][3] = 0;
	mat[2][0] = Au3dsGLSL::LoadFloat(in);
	mat[2][1] = Au3dsGLSL::LoadFloat(in);
	mat[2][2] = Au3dsGLSL::LoadFloat(in);
	mat[2][3] = 0;
	mat[3][0] = Au3dsGLSL::LoadFloat(in);
	mat[3][1] = Au3dsGLSL::LoadFloat(in);
	mat[3][2] = Au3dsGLSL::LoadFloat(in);
	mat[3][3] = 1;
	chunkSize -= sizeof(float) * 12;
	m_Matrix = AuMatrix4x4(mat);

	if(unitScaling != 1)
	{
		// Scale the translation factors in the matrix.
		m_Matrix[12] *= unitScaling;
		m_Matrix[13] *= unitScaling;
		m_Matrix[14] *= unitScaling;
	}

	// Project the vertices through the inverse matrix. They will get projected
	// back at render time. The difference is the location of the pivot point
	// if we rotate a piece. e.g. a tank turret.
	if(!m_Matrix.IsIdentity())
		m_Vertices.Project(m_Matrix.Inverse());

	if(chunkSize)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLObject::LoadMatrixChunk skipping remaining %d bytes",
			chunkSize);
		in.skipRawData(chunkSize);
	}

	return true;
}

void Au3dsGLSLObject::Dump() const
{
	qDebug() << m_Name << "vertices" << m_Vertices.count() << "faces" << m_Faces.count();
}

// Not sure if this is right but I need this value outside the object.
float Au3dsGLSLObject::Transparency(const Au3dsGLSLMaterialArray& materials, MODEL_TYPE modelType) const
{
	int faceIndex = 0;
	for(Au3dsGLSLFaceArray::const_iterator it = m_Faces.constBegin();
		it != m_Faces.constEnd(); it++, faceIndex++)
	{
		if(m_Materials.count())
		{
			// Materials are applied to this object.
			// Check this face.
			for(QMap<QString, QVector<quint16> >::const_iterator mit =
				m_Materials.constBegin(); mit != m_Materials.constEnd(); mit++)
			{
				const QVector<quint16>& faceMaterials = *mit;
				if(faceMaterials.contains(faceIndex))
				{
					// The material applies to the face.
					//DHH This material overrides any previous material on this
					//DHH face. It probably needs to be blended in some sensible
					//DHH way.
					int materialIndex = materials.IndexOf(mit.key());
					if(materialIndex >= 0)
					{
						// Found the material...woo hoo!
						const Au3dsGLSLMaterial& material = materials[materialIndex];
						return material.Transparency();
					}
				}
			}
		}
	}
	// Transparency for most 3d software has 0 being fully transparent and 1 as fully opaque.
	// Blender follows this as well so default new models exported from blender to return 1, 
	// i.e. fully opaque.
	if(!IsSymmetricaModel(modelType))
		return 1;
	return 0;
}

void Au3dsGLSLObject::CreateDeviceObject(AuOpenGLFunctions& GLf, Au3dsGLSLVAO& model,
		const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc)
{
	// Calculate normals to each face so we can do smoothing calculations as
	// we go.  Only needed for the old models exported from Symmetrica
	AuPointArray faceNormals;
	if(!HasPrecalculatedNormals(modelType))
	{
		for(Au3dsGLSLFaceArray::const_iterator itConst = m_Faces.constBegin(); itConst != m_Faces.constEnd(); itConst++)
		{
			const Au3dsGLSLVertex& v0 = (*itConst).Vertex(0, m_Vertices);
			const Au3dsGLSLVertex& v1 = (*itConst).Vertex(1, m_Vertices);
			const Au3dsGLSLVertex& v2 = (*itConst).Vertex(2, m_Vertices);

			// Calculate the normal to the unprojected triangle.
			AuPoint vx;
			vx.x = v1.X() - v0.X();
			vx.y = v1.Y() - v0.Y();
			vx.z = v1.Z() - v0.Z();
			if(vx.IsZero())
				vx = AuPoint(1,0,0);
			vx.Unit();

			AuPoint vy;
			vy.x = v2.X() - v0.X();
			vy.y = v2.Y() - v0.Y();
			vy.z = v2.Z() - v0.Z();
			if(vy.IsZero())
				vy = AuPoint(0,1,0);
			vy.Unit();

			// The raw normal needs to be projected through the object's matrix
			// because the vertices were projected through the inverse matrix when
			// loaded.
			AuPoint vn = vx.Cross(vy);
			AuPoint pvn = m_Matrix.ProjectVector(vn);	// Project the normal.
			pvn.Unit();

			faceNormals.append(pvn);
		}
	}

	Au3dsGLSLTriTexList smdl;
	LoadModel(smdl, materials, lightsOn, modelType, destBlendFunc);
	model.CreateForModelTexFog(GLf, smdl, GL_STATIC_DRAW);
	model.m_DestBlendFunc = smdl.m_DestBlendFunc;
	model.m_GlowColor = smdl.m_GlowColor;
}

void Au3dsGLSLObject::CreateBodyDeviceObject(AuOpenGLFunctions& GLf, Au3dsGLSLVAO& model,
		const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc)
{
	// Calculate normals to each face so we can do smoothing calculations as
	// we go.  Only needed for the old models exported from Symmetrica
	AuPointArray faceNormals;
	if(!HasPrecalculatedNormals(modelType))
	{
		for(Au3dsGLSLFaceArray::const_iterator itConst = m_Faces.constBegin(); itConst != m_Faces.constEnd(); itConst++)
		{
			const Au3dsGLSLVertex& v0 = (*itConst).Vertex(0, m_Vertices);
			const Au3dsGLSLVertex& v1 = (*itConst).Vertex(1, m_Vertices);
			const Au3dsGLSLVertex& v2 = (*itConst).Vertex(2, m_Vertices);

			// Calculate the normal to the unprojected triangle.
			AuPoint vx;
			vx.x = v1.X() - v0.X();
			vx.y = v1.Y() - v0.Y();
			vx.z = v1.Z() - v0.Z();
			if(vx.IsZero())
				vx = AuPoint(1,0,0);
			vx.Unit();

			AuPoint vy;
			vy.x = v2.X() - v0.X();
			vy.y = v2.Y() - v0.Y();
			vy.z = v2.Z() - v0.Z();
			if(vy.IsZero())
				vy = AuPoint(0,1,0);
			vy.Unit();

			// The raw normal needs to be projected through the object's matrix
			// because the vertices were projected through the inverse matrix when
			// loaded.
			AuPoint vn = vx.Cross(vy);
			AuPoint pvn = m_Matrix.ProjectVector(vn);	// Project the normal.
			pvn.Unit();

			faceNormals.append(pvn);
		}
	}

	Au3dsGLSLTriBodyList smdl;
	LoadModel(smdl, materials, lightsOn, modelType, destBlendFunc);
	model.CreateForBody(GLf, smdl, GL_STATIC_DRAW);
	model.m_DestBlendFunc = smdl.m_DestBlendFunc;
	model.m_GlowColor = smdl.m_GlowColor;
}

void Au3dsGLSLObject::LoadModel(Au3dsGLSLTriTexList& smdl, const Au3dsGLSLMaterialArray& materials, bool lightsOn,
		MODEL_TYPE modelType, GLuint destBlendFunc)
{
	//qDebug() << "Au3dsGLSLObject::LoadModel" << m_Name << m_Faces.size() << HasPrecalculatedNormals(modelType);
	// Calculate normals to each face so we can do smoothing calculations as
	// we go.  Only needed for the old models exported from Symmetrica
	AuPointArray faceNormals;
	if(!HasPrecalculatedNormals(modelType))
	{
		for(Au3dsGLSLFaceArray::const_iterator itConst = m_Faces.constBegin(); itConst != m_Faces.constEnd(); itConst++)
		{
			const Au3dsGLSLVertex& v0 = (*itConst).Vertex(0, m_Vertices);
			const Au3dsGLSLVertex& v1 = (*itConst).Vertex(1, m_Vertices);
			const Au3dsGLSLVertex& v2 = (*itConst).Vertex(2, m_Vertices);

			// Calculate the normal to the unprojected triangle.
			AuPoint vx;
			vx.x = v1.X() - v0.X();
			vx.y = v1.Y() - v0.Y();
			vx.z = v1.Z() - v0.Z();
			if(vx.IsZero())
				vx = AuPoint(1,0,0);
			vx.Unit();

			AuPoint vy;
			vy.x = v2.X() - v0.X();
			vy.y = v2.Y() - v0.Y();
			vy.z = v2.Z() - v0.Z();
			if(vy.IsZero())
				vy = AuPoint(0,1,0);
			vy.Unit();

			// The raw normal needs to be projected through the object's matrix
			// because the vertices were projected through the inverse matrix when
			// loaded.
			AuPoint vn = vx.Cross(vy);
			AuPoint pvn = m_Matrix.ProjectVector(vn);	// Project the normal.
			pvn.Unit();

			faceNormals.append(pvn);
		}
	}

	// Alpha blending must be enabled outside the calls to glBegin/glEnd.
	// Calculate alpha value; 1 is opaque.
	float transAlpha = 1.0f;
	if(IsSymmetricaModel(modelType))
		transAlpha = transAlpha - Transparency(materials, modelType)/100.0f;
	else
		transAlpha = Transparency(materials, modelType);
	smdl.m_DestBlendFunc = destBlendFunc;
	smdl.SetTransAlpha(transAlpha);

	int faceIndex = 0;
	for(Au3dsGLSLFaceArray::iterator it = m_Faces.begin(); it != m_Faces.end(); it++, faceIndex++)
	{
		// The raw vertices need to be projected through the object's matrix
		// because they were projected through the inverse matrix when loaded.
		quint16 m0 = (*it).VertexIndex(0);
		quint16 m1 = (*it).VertexIndex(1);
		quint16 m2 = (*it).VertexIndex(2);
		const Au3dsGLSLVertex& v0 = (*it).Vertex(0, m_Vertices);
		const Au3dsGLSLVertex& v1 = (*it).Vertex(1, m_Vertices);
		const Au3dsGLSLVertex& v2 = (*it).Vertex(2, m_Vertices);
		AuPoint t0 = m_Matrix.Project(v0.X(), v0.Y(), v0.Z());
		AuPoint t1 = m_Matrix.Project(v1.X(), v1.Y(), v1.Z());
		AuPoint t2 = m_Matrix.Project(v2.X(), v2.Y(), v2.Z());

		// If TRUE then this file was created with Symmetrica.  The texture coordinates
		// were done wrong on the 3ds export.  New files are being created with Blender
		// now which does texture coordinates properly.  Until the 3ds files that were
		// made with Symmetrica are fixed we must keep the old code in here.
		int tc0, tc1, tc2;
		if(IsSymmetricaModel(modelType))
		{
			// Texture coords are in an array parallel to the faces array, with
			// three entries per face.
			tc0 = faceIndex * 3;
			tc1 = tc0 + 1;
			tc2 = tc0 + 2;
		}
		else
		{
			tc0 = 0;
			tc1 = 0;
			tc2 = 0;
		}
		AuPoint vn0, vn1, vn2;
		AuPoint vt0, vt1, vt2;
		//float w = 1;
		if(HasPrecalculatedNormals(modelType))		// 1 & 2 have normals pre-calculated
		{
			vn0 = (*it).m_normals[0];
			vn1 = (*it).m_normals[1];
			vn2 = (*it).m_normals[2];
			if(HasPrecalculatedTangents(modelType))	// 2 has tangents pre-calculated
			{
				vt0 = (*it).m_Tangents[0];
				vt1 = (*it).m_Tangents[1];
				vt2 = (*it).m_Tangents[2];
				//w = (*it).m_UVhandedness;
			}
		}
		else
		{
			vn0 = faceNormals[faceIndex];
			vn1 = vn0;
			vn2 = vn0;
			(*it).m_normals[0] += vn0;
			(*it).m_normals[1] += vn1;
			(*it).m_normals[2] += vn2;
			if(m_FaceSmoothing.count() > faceIndex && m_FaceSmoothing[faceIndex])
			{
				// This face is part of a smoothing group.
				// Locate other faces in the same smoothing group that share each
				// vertex and average their normals.

				// Start the loop right after this face because we already did the
				// work for it up to this point.
				int smoothFaceIdx = faceIndex+1;
				for(int i=smoothFaceIdx; i<m_Faces.count(); i++)
				{
					if(m_FaceSmoothing[faceIndex] & m_FaceSmoothing[i])
					{
						// The faces are in the same smoothing group.
						AuPoint fn = faceNormals[i];
						AuPoint sfn = faceNormals[faceIndex];
						//if(LessThanOrFuzzyEquals(faceNormals[faceIndex].Dot(fn), 0))
						//	continue;	// Too tight, 90 degrees or less.
						int iVertIndex = -1;
						iVertIndex = m_Faces[i].UsesVertex(m_bHasDuplicates, m0, v0, m_HashVertices);
						if(iVertIndex != -1)
						{
							// The face is smoothed with vertex 0.
							(*it).m_normals[0] += fn;
							(*it).m_sharedFaces[0] += 1;
							// Now do the same for the other face
							m_Faces[i].m_normals[iVertIndex] += sfn;
							m_Faces[i].m_sharedFaces[iVertIndex] += 1;
						}
						iVertIndex = m_Faces[i].UsesVertex(m_bHasDuplicates, m1, v1, m_HashVertices);
						if(iVertIndex != -1)
						{
							// The face is smoothed with vertex 1.
							(*it).m_normals[1] += fn;
							(*it).m_sharedFaces[1] += 1;
							// Now do the same for the other face
							m_Faces[i].m_normals[iVertIndex] += sfn;
							m_Faces[i].m_sharedFaces[iVertIndex] += 1;
						}
						iVertIndex = m_Faces[i].UsesVertex(m_bHasDuplicates, m2, v2, m_HashVertices);
						if(iVertIndex != -1)
						{
							// The face is smoothed with vertex 2.
							(*it).m_normals[2] += fn;
							(*it).m_sharedFaces[2] += 1;
							// Now do the same for the other face
							m_Faces[i].m_normals[iVertIndex] += sfn;
							m_Faces[i].m_sharedFaces[iVertIndex] += 1;
						}
					}
					// 					else
					// 						qWarning("Face smoothing count: %d, Index: %d, SmoothIndex", m_FaceSmoothing.count(), faceIndex, smoothFaceIdx);
				}
				vn0 = (*it).m_normals[0];
				vn1 = (*it).m_normals[1];
				vn2 = (*it).m_normals[2];
				if((*it).m_sharedFaces[0] > 1)
					vn0 /= (*it).m_sharedFaces[0];
				if((*it).m_sharedFaces[1] > 1)
					vn1 /= (*it).m_sharedFaces[1];
				if((*it).m_sharedFaces[2] > 1)
					vn2 /= (*it).m_sharedFaces[2];
				// Reset this face
				//(*it).m_normals[0] = AuPoint(0,0,0);
				//(*it).m_normals[1] = AuPoint(0,0,0);
				//(*it).m_normals[2] = AuPoint(0,0,0);
				(*it).m_sharedFaces[0] = 0;
				(*it).m_sharedFaces[1] = 0;
				(*it).m_sharedFaces[2] = 0;
			}
			// 			else
			// 				qWarning("Face smoothing count: %d, Index: %d", m_FaceSmoothing.count(), faceIndex);
		}

		// Configure material parameters.
		QColor diffuse(255,255,255,255);
		QColor ambient(255,255,255,0);
		QColor specular(0,0,0,0);
		QColor diffuseBack(255,255,255,255);
		QColor ambientBack(255,255,255,0);
		QColor specularBack(0,0,0,0);
		if(HasPrecalculatedNormals(modelType))
		{
			ambient = m_tColor;
			diffuse = m_tColor;
		}
		float selfIllum = 0;
		float shininess = 0;
		//bool twoSided = false;
		if(materials.count())
		{
			// Materials are applied to this object.
			// Check this face.
			for(QMap<QString, QVector<quint16> >::const_iterator mit =
				m_Materials.constBegin(); mit != m_Materials.constEnd(); mit++)
			{
				const QVector<quint16>& faceMaterials = *mit;
				if(faceMaterials.contains(faceIndex))
				{
					// The material applies to the face.
					//DHH This material overrides any previous material on this
					//DHH face. It probably needs to be blended in some sensible
					//DHH way.
					int materialIndex = materials.IndexOf(mit.key());
					if(materialIndex >= 0)
					{
						// Found the material...woo hoo!
						const Au3dsGLSLMaterial& material = materials[materialIndex];
						ambient = material.AmbientColor(1);
						diffuse = material.DiffuseColor(1);
						specular = material.SpecularColor(1);
						if(lightsOn)
							selfIllum = (float)material.SelfIllumination()/100;
						else
							selfIllum = 0;
						//if(material.SelfIllumination())
						//qDebug("mat sI %d selfIllum %g",
						//material.SelfIllumination(), selfIllum);
						shininess = material.Shininess();
						//twoSided = material.IsTwoSided();
						if(m_Name.contains("TwoSided"))
						{
							ambientBack = material.AmbientColor(2);
							diffuseBack = material.DiffuseColor(2);
							specularBack = material.SpecularColor(2);
						}
					}
				}
			}
		}

		// When the two-sided flag is on, we draw the back side first.
		//const float diffIllum = 1 - selfIllum;
		double v0uOffset = 0;
		double v0vOffset = 0;
		double v1uOffset = 0;
		double v1vOffset = 0;
		double v2uOffset = 0;
		double v2vOffset = 0;
		//bool bTopFace = false;
		if(!IsSymmetricaModel(modelType))
		{
// 			if(m_UTOffset == 0 && m_UBOffset == 0 && m_VTOffset == 0 && m_VBOffset == 0)
// 			{
// 				v0uOffset = v0.U();
// 				v1uOffset = v1.U();
// 				v2uOffset = v2.U();
// 				v0vOffset = v0.V();
// 				v1vOffset = v1.V();
// 				v2vOffset = v2.V();
// 			}
// 			else
// 			{
				// Apply Texture offset for this face
				if(m_MappingCoords.count() > 0)
					//bTopFace = ApplyTextureOffset(v0, v1, v2, v0uOffset, v0vOffset, v1uOffset, v1vOffset, v2uOffset, v2vOffset);
					ApplyTextureOffset(v0, v1, v2, v0uOffset, v0vOffset, v1uOffset, v1vOffset, v2uOffset, v2vOffset);
			//}
		}
		//if(selfIllum != 0)
		//qDebug("selfIllum %g diffIllum %g", selfIllum, diffIllum);
		/*
		if(m_Name.contains("TwoSided"))
		{
			// Draw the back side.
			if(m_bCreate && !IsSymmetricaModel(modelType))
			{
				if(bTopFace)
				{
					smdl.glColor4f(m_tColor.redF()*diffIllum, m_tColor.greenF()*diffIllum, m_tColor.blueF()*diffIllum, 0);			// diffuse color
					smdl.glMultiTexCoord4f(GL_TEXTURE1, m_tColor.redF(), m_tColor.greenF(), m_tColor.blueF(), m_tColor.alphaF());	// ambient
					if(shininess == 0)
						shininess = m_tShininess;
				}
				else
				{
					smdl.glColor4f(m_bColor.redF(), m_bColor.greenF(), m_bColor.blueF(), m_bColor.alphaF());							// diffuse color
					smdl.glMultiTexCoord4f(GL_TEXTURE1, m_bColor.redF(), m_bColor.greenF(), m_bColor.blueF(), m_bColor.alphaF());	// ambient
					if(shininess == 0)
						shininess = m_bShininess;
				}
				smdl.glMultiTexCoord4f(GL_TEXTURE2, m_Glowdim,m_Glow,m_Glowdim,transAlpha);	// emission color
				smdl.glMultiTexCoord4f(GL_TEXTURE3, 1, 1, 1,0);								// specular color
			}
			else
			{
				smdl.glColor4f(1 * diffIllum,
					1 * diffIllum,
					1 * diffIllum,
					0);									// diffuse color
				smdl.glMultiTexCoord4f(GL_TEXTURE1,
					ambient.redF(),
					ambient.greenF(),
					ambient.blueF(),
					0);									// ambient color
				smdl.glMultiTexCoord4f(GL_TEXTURE2,
					diffuse.redF() * selfIllum,
					diffuse.greenF() * selfIllum,
					diffuse.blueF() * selfIllum,
					transAlpha);						// emission color (glow)
				smdl.glMultiTexCoord4f(GL_TEXTURE3,
					specular.redF(),
					specular.greenF(),
					specular.blueF(),
					0);									// specular color
				smdl.glMultiTexCoord2f(GL_TEXTURE4, shininess, shininess);
			}

			// A front-facing triangle is clockwise to us.
			// We cull triangles facing counter-clockwise.
			smdl.glNormal3d(-vn0.x, -vn0.y, -vn0.z);
			AuPoint xt0, xt1, xt2;
			xt0 = t0;
			xt1 = t1;
			xt2 = t2;
			if(IsSymmetricaModel(modelType))
			{
				if(tc0 < m_MappingCoords.count())
					smdl.glTexCoord2f(m_MappingCoords[tc0].s(), m_MappingCoords[tc0].t());
			}
			else
			{
				if(m_MappingCoords.count() > 0)
					smdl.glTexCoord2f(v0uOffset, v0vOffset);
				if(!m_bScaled)
					xt0 = ApplyScaleFactor(xt0);
			}
			if(m_bHasBones)
			{
				smdl.glMultiTexCoord4fv(GL_TEXTURE5, v0.m_wt);		// bone weights
				smdl.glMultiTexCoord4iv(GL_TEXTURE6, v0.m_idx);		// bone indices
			}
			if(HasPrecalculatedTangents(modelType))
				smdl.glMultiTexCoord4f(GL_TEXTURE7, vt0.x, vt0.y, vt0.z, w);	// Tangent vector - w is handedness
			smdl.glMultiTexCoord3f(GL_TEXTURE4, shininess, shininess, GetTextureWeight(v0));				// shininess
			smdl.glVertex3d(xt0.x, xt0.y, xt0.z);
			if(vn1 != vn0)
				smdl.glNormal3d(-vn1.x, -vn1.y, -vn1.z);
			if(IsSymmetricaModel(modelType))
			{
				if(tc1 < m_MappingCoords.count())
					smdl.glTexCoord2f(m_MappingCoords[tc1].s(), m_MappingCoords[tc1].t());
			}
			else
			{
				if(m_MappingCoords.count() > 0)
					smdl.glTexCoord2f(v1uOffset, v1vOffset);
				if(!m_bScaled)
					xt1 = ApplyScaleFactor(xt1);
			}
			if(m_bHasBones)
			{
				smdl.glMultiTexCoord4fv(GL_TEXTURE5, v1.m_wt);		// bone weights
				smdl.glMultiTexCoord4iv(GL_TEXTURE6, v1.m_idx);		// bone indices
			}
			if(HasPrecalculatedTangents(modelType))
				smdl.glMultiTexCoord4f(GL_TEXTURE7, vt1.x, vt1.y, vt1.z, w);	// Tangent vector - w is handedness
			smdl.glMultiTexCoord3f(GL_TEXTURE4, shininess, shininess, GetTextureWeight(v1));				// shininess
			smdl.glVertex3d(xt1.x, xt1.y, xt1.z);
			if(vn2 != vn0)
				smdl.glNormal3d(-vn2.x, -vn2.y, -vn2.z);
			if(IsSymmetricaModel(modelType))
			{
				if(tc2 < m_MappingCoords.count())
					smdl.glTexCoord2f(m_MappingCoords[tc2].s(), m_MappingCoords[tc2].t());
			}
			else
			{
				if(m_MappingCoords.count() > 0)
					smdl.glTexCoord2f(v2uOffset, v2vOffset);
				if(!m_bScaled)
					xt2 = ApplyScaleFactor(xt2);
			}
			if(m_bHasBones)
			{
				smdl.glMultiTexCoord4fv(GL_TEXTURE5, v2.m_wt);		// bone weights
				smdl.glMultiTexCoord4iv(GL_TEXTURE6, v2.m_idx);		// bone indices
			}
			if(HasPrecalculatedTangents(modelType))
				smdl.glMultiTexCoord4f(GL_TEXTURE7, vt2.x, vt2.y, vt2.z, w);	// Tangent vector - w is handedness
			smdl.glMultiTexCoord3f(GL_TEXTURE4, shininess, shininess, GetTextureWeight(v2));				// shininess
			smdl.glVertex3d(xt2.x, xt2.y, xt2.z);
		}
		*/

		// Side one is the counter-clockwise side in 3ds' sequence.
		if(m_bCreate && !IsSymmetricaModel(modelType))
		{
			if(shininess == 0)
				shininess = m_tShininess;
			smdl.SetGlowColor(m_Glowdim, m_Glow, m_Glowdim);
		}
		else
		{
			if(selfIllum)
				smdl.SetGlowColor(diffuse.redF() * selfIllum, diffuse.greenF() * selfIllum, diffuse.blueF() * selfIllum);
		}

		// A front-facing triangle is clockwise to us.
		//qDebug("Front face tex coords %g,%g %g,%g %g,%g",
		//m_MappingCoords[tc0].s(), m_MappingCoords[tc0].t(),
		//m_MappingCoords[tc1].s(), m_MappingCoords[tc1].t(),
		//m_MappingCoords[tc2].s(), m_MappingCoords[tc2].t());
		// We cull triangles facing counter-clockwise.
		AuPointTexNS tv0;
		tv0.m_N = vn0;
		if(IsSymmetricaModel(modelType))
		{
			if(tc0 < m_MappingCoords.count())
				tv0.SetTexCoord(m_MappingCoords[tc0].s(), m_MappingCoords[tc0].t());
		}
		else
		{
			if(m_MappingCoords.count() > 0)
				tv0.SetTexCoord(v0uOffset, v0vOffset);
			if(!m_bScaled)
				t0 = ApplyScaleFactor(t0);
		}
		/*
		if(m_bHasBones)
		{
			smdl.glMultiTexCoord4fv(GL_TEXTURE5, v0.m_wt);		// bone weights
			smdl.glMultiTexCoord4iv(GL_TEXTURE6, v0.m_idx);		// bone indices
		}
		if(HasPrecalculatedTangents(modelType))
			smdl.glMultiTexCoord4f(GL_TEXTURE7, vt0.x, vt0.y, vt0.z, w);	// Tangent vector - w is handedness
		smdl.glMultiTexCoord3f(GL_TEXTURE4, shininess, shininess, GetTextureWeight(v0));				// shininess
		smdl.glVertex3d(t0.x, t0.y, t0.z);
		*/
		tv0.m_Shine = shininess;
		tv0.SetCoord(t0);

		AuPointTexNS tv1;
		tv1.m_N = vn2;
		if(IsSymmetricaModel(modelType))
		{
			if(tc1 < m_MappingCoords.count())
				tv1.SetTexCoord(m_MappingCoords[tc1].s(), m_MappingCoords[tc1].t());
		}
		else
		{
			if(m_MappingCoords.count() > 0)
				tv1.SetTexCoord(v2uOffset, v2vOffset);
			if(!m_bScaled)
				t2 = ApplyScaleFactor(t2);
		}
		/*
		if(m_bHasBones)
		{
			smdl.glMultiTexCoord4fv(GL_TEXTURE5, v2.m_wt);		// bone weights
			smdl.glMultiTexCoord4iv(GL_TEXTURE6, v2.m_idx);		// bone indices
		}
		if(HasPrecalculatedTangents(modelType))
			smdl.glMultiTexCoord4f(GL_TEXTURE7, vt2.x, vt2.y, vt2.z, w);	// Tangent vector - w is handedness
		smdl.glMultiTexCoord3f(GL_TEXTURE4, shininess, shininess, GetTextureWeight(v2));				// shininess
		smdl.glVertex3d(t2.x, t2.y, t2.z);
		*/
		tv1.m_Shine = shininess;
		tv1.SetCoord(t2);

		AuPointTexNS tv2;
		tv2.m_N = vn1;
		if(IsSymmetricaModel(modelType))
		{
			if(tc2 < m_MappingCoords.count())
				tv2.SetTexCoord(m_MappingCoords[tc2].s(), m_MappingCoords[tc2].t());
		}
		else
		{
			if(m_MappingCoords.count() > 0)
				tv2.SetTexCoord(v1uOffset, v1vOffset);
			if(!m_bScaled)
				t1 = ApplyScaleFactor(t1);
		}
		/*
		if(m_bHasBones)
		{
			smdl.glMultiTexCoord4fv(GL_TEXTURE5, v1.m_wt);		// bone weights
			smdl.glMultiTexCoord4iv(GL_TEXTURE6, v1.m_idx);		// bone indices
		}
		if(HasPrecalculatedTangents(modelType))
			smdl.glMultiTexCoord4f(GL_TEXTURE7, vt1.x, vt1.y, vt1.z, w);	// Tangent vector - w is handedness
		smdl.glMultiTexCoord3f(GL_TEXTURE4, shininess, shininess, GetTextureWeight(v1));				// shininess
		smdl.glVertex3d(t1.x, t1.y, t1.z);
		*/
		tv2.m_Shine = shininess;
		tv2.SetCoord(t1);

		// Build a triangle.
		AuTriTexNS tri(tv0, tv1, tv2);
		smdl.append(tri);
	}

	// 	// Display the normals
	// 	for(Au3dsGLSLFaceArray::iterator it = m_Faces.begin(); it != m_Faces.end(); it++, faceIndex++)
	// 	{
	// 		AuPoint vn0 = (*it).m_normals[0];
	// 		AuPoint vn1 = (*it).m_normals[1];
	// 		AuPoint vn2 = (*it).m_normals[2];
	// 		const Au3dsGLSLVertex& v0 = (*it).Vertex(0, m_Vertices);
	// 		const Au3dsGLSLVertex& v1 = (*it).Vertex(1, m_Vertices);
	// 		const Au3dsGLSLVertex& v2 = (*it).Vertex(2, m_Vertices);
	// 		smdl.glColor3f(1, 0, 0);
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v0.X(), v0.Y(), v0.Z());
	// 			vn0.Unit(.2);
	// 			smdl.glVertex3d(v0.X()+vn0.x, v0.Y()+vn0.y, v0.Z()+vn0.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v1.X(), v1.Y(), v1.Z());
	// 			vn1.Unit(.2);
	// 			smdl.glVertex3d(v1.X()+vn1.x, v1.Y()+vn1.y, v1.Z()+vn1.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v2.X(), v2.Y(), v2.Z());
	// 			vn2.Unit(.2);
	// 			smdl.glVertex3d(v2.X()+vn2.x, v2.Y()+vn2.y, v2.Z()+vn2.z);
	// 		smdl.glEnd();
	// 
	// 		smdl.glColor3f(0, 1, 0);
	// 		AuPoint vt0 = (*it).m_Tangents[0];
	// 		AuPoint vt1 = (*it).m_Tangents[1];
	// 		AuPoint vt2 = (*it).m_Tangents[2];
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v0.X(), v0.Y(), v0.Z());
	// 			vt0.Unit(.2);
	// 			smdl.glVertex3d(v0.X()+vt0.x, v0.Y()+vt0.y, v0.Z()+vt0.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v1.X(), v1.Y(), v1.Z());
	// 			vt1.Unit(.2);
	// 			smdl.glVertex3d(v1.X()+vt1.x, v1.Y()+vt1.y, v1.Z()+vt1.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v2.X(), v2.Y(), v2.Z());
	// 			vt2.Unit(.2);
	// 			smdl.glVertex3d(v2.X()+vt2.x, v2.Y()+vt2.y, v2.Z()+vt2.z);
	// 		smdl.glEnd();
	// 	}
}

void Au3dsGLSLObject::LoadModel(Au3dsGLSLTriBodyList& smdl, const Au3dsGLSLMaterialArray& materials, bool lightsOn,
		MODEL_TYPE modelType, GLuint destBlendFunc)
{
	// Calculate normals to each face so we can do smoothing calculations as
	// we go.  Only needed for the old models exported from Symmetrica
	AuPointArray faceNormals;
	if(!HasPrecalculatedNormals(modelType))
	{
		for(Au3dsGLSLFaceArray::const_iterator itConst = m_Faces.constBegin(); itConst != m_Faces.constEnd(); itConst++)
		{
			const Au3dsGLSLVertex& v0 = (*itConst).Vertex(0, m_Vertices);
			const Au3dsGLSLVertex& v1 = (*itConst).Vertex(1, m_Vertices);
			const Au3dsGLSLVertex& v2 = (*itConst).Vertex(2, m_Vertices);

			// Calculate the normal to the unprojected triangle.
			AuPoint vx;
			vx.x = v1.X() - v0.X();
			vx.y = v1.Y() - v0.Y();
			vx.z = v1.Z() - v0.Z();
			if(vx.IsZero())
				vx = AuPoint(1,0,0);
			vx.Unit();

			AuPoint vy;
			vy.x = v2.X() - v0.X();
			vy.y = v2.Y() - v0.Y();
			vy.z = v2.Z() - v0.Z();
			if(vy.IsZero())
				vy = AuPoint(0,1,0);
			vy.Unit();

			// The raw normal needs to be projected through the object's matrix
			// because the vertices were projected through the inverse matrix when
			// loaded.
			AuPoint vn = vx.Cross(vy);
			AuPoint pvn = m_Matrix.ProjectVector(vn);	// Project the normal.
			pvn.Unit();

			faceNormals.append(pvn);
		}
	}

	// Alpha blending must be enabled outside the calls to glBegin/glEnd.
	// Calculate alpha value; 1 is opaque.
	float transAlpha = 1.0f;
	if(IsSymmetricaModel(modelType))
		transAlpha = transAlpha - Transparency(materials, modelType)/100.0f;
	else
		transAlpha = Transparency(materials, modelType);
	smdl.m_DestBlendFunc = destBlendFunc;
	smdl.SetTransAlpha(transAlpha);

	int faceIndex = 0;
	for(Au3dsGLSLFaceArray::iterator it = m_Faces.begin(); it != m_Faces.end(); it++, faceIndex++)
	{
		// The raw vertices need to be projected through the object's matrix
		// because they were projected through the inverse matrix when loaded.
		quint16 m0 = (*it).VertexIndex(0);
		quint16 m1 = (*it).VertexIndex(1);
		quint16 m2 = (*it).VertexIndex(2);
		const Au3dsGLSLVertex& v0 = (*it).Vertex(0, m_Vertices);
		const Au3dsGLSLVertex& v1 = (*it).Vertex(1, m_Vertices);
		const Au3dsGLSLVertex& v2 = (*it).Vertex(2, m_Vertices);
		AuPoint t0 = m_Matrix.Project(v0.X(), v0.Y(), v0.Z());
		AuPoint t1 = m_Matrix.Project(v1.X(), v1.Y(), v1.Z());
		AuPoint t2 = m_Matrix.Project(v2.X(), v2.Y(), v2.Z());

		// If TRUE then this file was created with Symmetrica.  The texture coordinates
		// were done wrong on the 3ds export.  New files are being created with Blender
		// now which does texture coordinates properly.  Until the 3ds files that were
		// made with Symmetrica are fixed we must keep the old code in here.
		int tc0, tc1, tc2;
		if(IsSymmetricaModel(modelType))
		{
			// Texture coords are in an array parallel to the faces array, with
			// three entries per face.
			tc0 = faceIndex * 3;
			tc1 = tc0 + 1;
			tc2 = tc0 + 2;
		}
		else
		{
			tc0 = 0;
			tc1 = 0;
			tc2 = 0;
		}
		AuPoint vn0, vn1, vn2;
		AuPoint vt0, vt1, vt2;
		//float w = 1;
		if(HasPrecalculatedNormals(modelType))		// 1 & 2 have normals pre-calculated
		{
			vn0 = (*it).m_normals[0];
			vn1 = (*it).m_normals[1];
			vn2 = (*it).m_normals[2];
			if(HasPrecalculatedTangents(modelType))	// 2 has tangents pre-calculated
			{
				vt0 = (*it).m_Tangents[0];
				vt1 = (*it).m_Tangents[1];
				vt2 = (*it).m_Tangents[2];
				//w = (*it).m_UVhandedness;
			}
		}
		else
		{
			vn0 = faceNormals[faceIndex];
			vn1 = vn0;
			vn2 = vn0;
			(*it).m_normals[0] += vn0;
			(*it).m_normals[1] += vn1;
			(*it).m_normals[2] += vn2;
			if(m_FaceSmoothing.count() > faceIndex && m_FaceSmoothing[faceIndex])
			{
				// This face is part of a smoothing group.
				// Locate other faces in the same smoothing group that share each
				// vertex and average their normals.

				// Start the loop right after this face because we already did the
				// work for it up to this point.
				int smoothFaceIdx = faceIndex+1;
				for(int i=smoothFaceIdx; i<m_Faces.count(); i++)
				{
					if(m_FaceSmoothing[faceIndex] & m_FaceSmoothing[i])
					{
						// The faces are in the same smoothing group.
						AuPoint fn = faceNormals[i];
						AuPoint sfn = faceNormals[faceIndex];
						//if(LessThanOrFuzzyEquals(faceNormals[faceIndex].Dot(fn), 0))
						//	continue;	// Too tight, 90 degrees or less.
						int iVertIndex = -1;
						iVertIndex = m_Faces[i].UsesVertex(m_bHasDuplicates, m0, v0, m_HashVertices);
						if(iVertIndex != -1)
						{
							// The face is smoothed with vertex 0.
							(*it).m_normals[0] += fn;
							(*it).m_sharedFaces[0] += 1;
							// Now do the same for the other face
							m_Faces[i].m_normals[iVertIndex] += sfn;
							m_Faces[i].m_sharedFaces[iVertIndex] += 1;
						}
						iVertIndex = m_Faces[i].UsesVertex(m_bHasDuplicates, m1, v1, m_HashVertices);
						if(iVertIndex != -1)
						{
							// The face is smoothed with vertex 1.
							(*it).m_normals[1] += fn;
							(*it).m_sharedFaces[1] += 1;
							// Now do the same for the other face
							m_Faces[i].m_normals[iVertIndex] += sfn;
							m_Faces[i].m_sharedFaces[iVertIndex] += 1;
						}
						iVertIndex = m_Faces[i].UsesVertex(m_bHasDuplicates, m2, v2, m_HashVertices);
						if(iVertIndex != -1)
						{
							// The face is smoothed with vertex 2.
							(*it).m_normals[2] += fn;
							(*it).m_sharedFaces[2] += 1;
							// Now do the same for the other face
							m_Faces[i].m_normals[iVertIndex] += sfn;
							m_Faces[i].m_sharedFaces[iVertIndex] += 1;
						}
					}
					// 					else
					// 						qWarning("Face smoothing count: %d, Index: %d, SmoothIndex", m_FaceSmoothing.count(), faceIndex, smoothFaceIdx);
				}
				vn0 = (*it).m_normals[0];
				vn1 = (*it).m_normals[1];
				vn2 = (*it).m_normals[2];
				if((*it).m_sharedFaces[0] > 1)
					vn0 /= (*it).m_sharedFaces[0];
				if((*it).m_sharedFaces[1] > 1)
					vn1 /= (*it).m_sharedFaces[1];
				if((*it).m_sharedFaces[2] > 1)
					vn2 /= (*it).m_sharedFaces[2];
				// Reset this face
				//(*it).m_normals[0] = AuPoint(0,0,0);
				//(*it).m_normals[1] = AuPoint(0,0,0);
				//(*it).m_normals[2] = AuPoint(0,0,0);
				(*it).m_sharedFaces[0] = 0;
				(*it).m_sharedFaces[1] = 0;
				(*it).m_sharedFaces[2] = 0;
			}
			// 			else
			// 				qWarning("Face smoothing count: %d, Index: %d", m_FaceSmoothing.count(), faceIndex);
		}

		// Configure material parameters.
		QColor diffuse(255,255,255,255);
		QColor ambient(255,255,255,0);
		QColor specular(0,0,0,0);
		QColor diffuseBack(255,255,255,255);
		QColor ambientBack(255,255,255,0);
		QColor specularBack(0,0,0,0);
		if(HasPrecalculatedNormals(modelType))
		{
			ambient = m_tColor;
			diffuse = m_tColor;
		}
		float selfIllum = 0;
		float shininess = 0;
		//bool twoSided = false;
		if(materials.count())
		{
			// Materials are applied to this object.
			// Check this face.
			for(QMap<QString, QVector<quint16> >::const_iterator mit =
				m_Materials.constBegin(); mit != m_Materials.constEnd(); mit++)
			{
				const QVector<quint16>& faceMaterials = *mit;
				if(faceMaterials.contains(faceIndex))
				{
					// The material applies to the face.
					//DHH This material overrides any previous material on this
					//DHH face. It probably needs to be blended in some sensible
					//DHH way.
					int materialIndex = materials.IndexOf(mit.key());
					if(materialIndex >= 0)
					{
						// Found the material...woo hoo!
						const Au3dsGLSLMaterial& material = materials[materialIndex];
						ambient = material.AmbientColor(1);
						diffuse = material.DiffuseColor(1);
						specular = material.SpecularColor(1);
						if(lightsOn)
							selfIllum = (float)material.SelfIllumination()/100;
						else
							selfIllum = 0;
						//if(material.SelfIllumination())
						//qDebug("mat sI %d selfIllum %g",
						//material.SelfIllumination(), selfIllum);
						shininess = material.Shininess();
						//twoSided = material.IsTwoSided();
						if(m_Name.contains("TwoSided"))
						{
							ambientBack = material.AmbientColor(2);
							diffuseBack = material.DiffuseColor(2);
							specularBack = material.SpecularColor(2);
						}
					}
				}
			}
		}

		// When the two-sided flag is on, we draw the back side first.
		//const float diffIllum = 1 - selfIllum;
		double v0uOffset = 0;
		double v0vOffset = 0;
		double v1uOffset = 0;
		double v1vOffset = 0;
		double v2uOffset = 0;
		double v2vOffset = 0;
		//bool bTopFace = false;
		if(!IsSymmetricaModel(modelType))
		{
// 			if(m_UTOffset == 0 && m_UBOffset == 0 && m_VTOffset == 0 && m_VBOffset == 0)
// 			{
// 				v0uOffset = v0.U();
// 				v1uOffset = v1.U();
// 				v2uOffset = v2.U();
// 				v0vOffset = v0.V();
// 				v1vOffset = v1.V();
// 				v2vOffset = v2.V();
// 			}
// 			else
// 			{
				// Apply Texture offset for this face
				if(m_MappingCoords.count() > 0)
					//bTopFace = ApplyTextureOffset(v0, v1, v2, v0uOffset, v0vOffset, v1uOffset, v1vOffset, v2uOffset, v2vOffset);
					ApplyTextureOffset(v0, v1, v2, v0uOffset, v0vOffset, v1uOffset, v1vOffset, v2uOffset, v2vOffset);
			//}
		}

		//if(selfIllum != 0)
			//qDebug("selfIllum %g diffIllum %g", selfIllum, diffIllum);

		if(m_bCreate && !IsSymmetricaModel(modelType))
		{
			if(shininess == 0)
				shininess = m_tShininess;
			smdl.SetGlowColor(m_Glowdim, m_Glow, m_Glowdim);
		}
		else
		{
			if(selfIllum)
				smdl.SetGlowColor(diffuse.redF() * selfIllum, diffuse.greenF() * selfIllum, diffuse.blueF() * selfIllum);
		}

		if(m_Name.contains("TwoSided"))
		{
			// Draw the back side.
			// A front-facing triangle is clockwise to us.
			// We cull triangles facing counter-clockwise.
			AuPointBody tv0;
			tv0.m_N = -vn0;
			AuPoint xt0, xt1, xt2;
			xt0 = t0;
			xt1 = t1;
			xt2 = t2;
			if(IsSymmetricaModel(modelType))
			{
				if(tc0 < m_MappingCoords.count())
					tv0.SetTexCoord(m_MappingCoords[tc0].s(), m_MappingCoords[tc0].t());
			}
			else
			{
				if(m_MappingCoords.count() > 0)
					tv0.SetTexCoord(v0uOffset, v0vOffset);
				if(!m_bScaled)
					xt0 = ApplyScaleFactor(xt0);
			}

			if(m_bHasBones)
			{
				for(int ib=0; ib<4; ib++)
				{
					tv0.m_BoneIndex[ib] = v0.m_idx[ib];
					tv0.m_BoneWeight[ib] = v0.m_wt[ib];
				}
			}
			/*
			if(HasPrecalculatedTangents(modelType))
				smdl.glMultiTexCoord4f(GL_TEXTURE7, vt0.x, vt0.y, vt0.z, w);	// Tangent vector - w is handedness
			smdl.glVertex3d(xt0.x, xt0.y, xt0.z);
			*/
			tv0.m_Shine = shininess;
			tv0.m_TopFace = GetTextureWeight(v0);
			tv0.SetCoord(xt0);

			AuPointBody tv1;
			tv1.m_N = -vn1;
			if(IsSymmetricaModel(modelType))
			{
				if(tc1 < m_MappingCoords.count())
					tv1.SetTexCoord(m_MappingCoords[tc1].s(), m_MappingCoords[tc1].t());
			}
			else
			{
				if(m_MappingCoords.count() > 0)
					tv1.SetTexCoord(v1uOffset, v1vOffset);
				if(!m_bScaled)
					xt1 = ApplyScaleFactor(xt1);
			}

			if(m_bHasBones)
			{
				for(int ib=0; ib<4; ib++)
				{
					tv1.m_BoneIndex[ib] = v1.m_idx[ib];
					tv1.m_BoneWeight[ib] = v1.m_wt[ib];
				}
			}
			/*
			if(HasPrecalculatedTangents(modelType))
				smdl.glMultiTexCoord4f(GL_TEXTURE7, vt1.x, vt1.y, vt1.z, w);	// Tangent vector - w is handedness
			*/
			tv1.m_Shine = shininess;
			tv1.m_TopFace = GetTextureWeight(v1);
			tv1.SetCoord(xt1);

			AuPointBody tv2;
			tv2.m_N = -vn2;
			if(IsSymmetricaModel(modelType))
			{
				if(tc2 < m_MappingCoords.count())
					tv2.SetTexCoord(m_MappingCoords[tc2].s(), m_MappingCoords[tc2].t());
			}
			else
			{
				if(m_MappingCoords.count() > 0)
					tv2.SetTexCoord(v2uOffset, v2vOffset);
				if(!m_bScaled)
					xt2 = ApplyScaleFactor(xt2);
			}
			if(m_bHasBones)
			{
				for(int ib=0; ib<4; ib++)
				{
					tv2.m_BoneIndex[ib] = v2.m_idx[ib];
					tv2.m_BoneWeight[ib] = v2.m_wt[ib];
				}
			}
			/*
			if(HasPrecalculatedTangents(modelType))
				smdl.glMultiTexCoord4f(GL_TEXTURE7, vt2.x, vt2.y, vt2.z, w);	// Tangent vector - w is handedness
			smdl.glVertex3d(xt2.x, xt2.y, xt2.z);
			*/
			tv2.m_Shine = shininess;
			tv2.m_TopFace = GetTextureWeight(v2);
			tv2.SetCoord(xt2);

			// Build a triangle.
			AuTriBody tri(tv0, tv1, tv2);
			smdl.append(tri);
		}

		// Side one is the counter-clockwise side in 3ds' sequence.

		// A front-facing triangle is clockwise to us.
		//qDebug("Front face tex coords %g,%g %g,%g %g,%g",
		//m_MappingCoords[tc0].s(), m_MappingCoords[tc0].t(),
		//m_MappingCoords[tc1].s(), m_MappingCoords[tc1].t(),
		//m_MappingCoords[tc2].s(), m_MappingCoords[tc2].t());
		// We cull triangles facing counter-clockwise.
		AuPointBody tv0;
		tv0.m_N = vn0;
		if(IsSymmetricaModel(modelType))
		{
			if(tc0 < m_MappingCoords.count())
				tv0.SetTexCoord(m_MappingCoords[tc0].s(), m_MappingCoords[tc0].t());
		}
		else
		{
			if(m_MappingCoords.count() > 0)
				tv0.SetTexCoord(v0uOffset, v0vOffset);
			if(!m_bScaled)
				t0 = ApplyScaleFactor(t0);
		}

		if(m_bHasBones)
		{
			for(int ib=0; ib<4; ib++)
			{
				tv0.m_BoneIndex[ib] = v0.m_idx[ib];
				tv0.m_BoneWeight[ib] = v0.m_wt[ib];
			}
		}
		/*
		if(HasPrecalculatedTangents(modelType))
			smdl.glMultiTexCoord4f(GL_TEXTURE7, vt0.x, vt0.y, vt0.z, w);	// Tangent vector - w is handedness
		smdl.glVertex3d(t0.x, t0.y, t0.z);
		*/
		tv0.m_Shine = shininess;
		tv0.m_TopFace = GetTextureWeight(v0);
		tv0.SetCoord(t0);

		AuPointBody tv1;
		tv1.m_N = vn2;
		if(IsSymmetricaModel(modelType))
		{
			if(tc1 < m_MappingCoords.count())
				tv1.SetTexCoord(m_MappingCoords[tc1].s(), m_MappingCoords[tc1].t());
		}
		else
		{
			if(m_MappingCoords.count() > 0)
				tv1.SetTexCoord(v2uOffset, v2vOffset);
			if(!m_bScaled)
				t2 = ApplyScaleFactor(t2);
		}

		if(m_bHasBones)
		{
			for(int ib=0; ib<4; ib++)
			{
				tv1.m_BoneIndex[ib] = v2.m_idx[ib];
				tv1.m_BoneWeight[ib] = v2.m_wt[ib];
			}
		}
		/*
		if(HasPrecalculatedTangents(modelType))
			smdl.glMultiTexCoord4f(GL_TEXTURE7, vt2.x, vt2.y, vt2.z, w);	// Tangent vector - w is handedness
		smdl.glVertex3d(t2.x, t2.y, t2.z);
		*/

		tv1.m_Shine = shininess;
		tv1.m_TopFace = GetTextureWeight(v2);
		tv1.SetCoord(t2);

		AuPointBody tv2;
		tv2.m_N = vn1;
		if(IsSymmetricaModel(modelType))
		{
			if(tc2 < m_MappingCoords.count())
				tv2.SetTexCoord(m_MappingCoords[tc2].s(), m_MappingCoords[tc2].t());
		}
		else
		{
			if(m_MappingCoords.count() > 0)
				tv2.SetTexCoord(v1uOffset, v1vOffset);
			if(!m_bScaled)
				t1 = ApplyScaleFactor(t1);
		}

		if(m_bHasBones)
		{
			for(int ib=0; ib<4; ib++)
			{
				tv2.m_BoneIndex[ib] = v1.m_idx[ib];
				tv2.m_BoneWeight[ib] = v1.m_wt[ib];
			}
		}
		/*
		if(HasPrecalculatedTangents(modelType))
			smdl.glMultiTexCoord4f(GL_TEXTURE7, vt1.x, vt1.y, vt1.z, w);	// Tangent vector - w is handedness
		smdl.glVertex3d(t1.x, t1.y, t1.z);
		*/

		tv2.m_Shine = shininess;
		tv2.m_TopFace = GetTextureWeight(v1);
		tv2.SetCoord(t1);

		// Build a triangle.
		AuTriBody tri(tv0, tv1, tv2);
		smdl.append(tri);
	}

	// 	// Display the normals
	// 	for(Au3dsGLSLFaceArray::iterator it = m_Faces.begin(); it != m_Faces.end(); it++, faceIndex++)
	// 	{
	// 		AuPoint vn0 = (*it).m_normals[0];
	// 		AuPoint vn1 = (*it).m_normals[1];
	// 		AuPoint vn2 = (*it).m_normals[2];
	// 		const Au3dsGLSLVertex& v0 = (*it).Vertex(0, m_Vertices);
	// 		const Au3dsGLSLVertex& v1 = (*it).Vertex(1, m_Vertices);
	// 		const Au3dsGLSLVertex& v2 = (*it).Vertex(2, m_Vertices);
	// 		smdl.glColor3f(1, 0, 0);
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v0.X(), v0.Y(), v0.Z());
	// 			vn0.Unit(.2);
	// 			smdl.glVertex3d(v0.X()+vn0.x, v0.Y()+vn0.y, v0.Z()+vn0.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v1.X(), v1.Y(), v1.Z());
	// 			vn1.Unit(.2);
	// 			smdl.glVertex3d(v1.X()+vn1.x, v1.Y()+vn1.y, v1.Z()+vn1.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v2.X(), v2.Y(), v2.Z());
	// 			vn2.Unit(.2);
	// 			smdl.glVertex3d(v2.X()+vn2.x, v2.Y()+vn2.y, v2.Z()+vn2.z);
	// 		smdl.glEnd();
	// 
	// 		smdl.glColor3f(0, 1, 0);
	// 		AuPoint vt0 = (*it).m_Tangents[0];
	// 		AuPoint vt1 = (*it).m_Tangents[1];
	// 		AuPoint vt2 = (*it).m_Tangents[2];
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v0.X(), v0.Y(), v0.Z());
	// 			vt0.Unit(.2);
	// 			smdl.glVertex3d(v0.X()+vt0.x, v0.Y()+vt0.y, v0.Z()+vt0.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v1.X(), v1.Y(), v1.Z());
	// 			vt1.Unit(.2);
	// 			smdl.glVertex3d(v1.X()+vt1.x, v1.Y()+vt1.y, v1.Z()+vt1.z);
	// 		smdl.glEnd();
	// 		smdl.glBegin(GL_LINES);
	// 			smdl.glVertex3d(v2.X(), v2.Y(), v2.Z());
	// 			vt2.Unit(.2);
	// 			smdl.glVertex3d(v2.X()+vt2.x, v2.Y()+vt2.y, v2.Z()+vt2.z);
	// 		smdl.glEnd();
	// 	}
}

void Au3dsGLSLObject::SetMappingCoords()
{
	// Mapping coords array should be the same as the vertex count, something went wrong
	if(m_MappingCoords.count() != m_Vertices.count())
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLObject::SetMappingCoords skipping UV Mapping of vertices's");
		return;
	}
	int i=0;
	Au3dsGLSLVertex vertex;
	for(Au3dsGLSLVertexArray::iterator it = m_Vertices.begin(); it != m_Vertices.end(); it++)
	{
		if(vertex == (*it))
			m_bHasDuplicates = true;
		(*it).SetMapCoordinates(m_MappingCoords[i].s(), m_MappingCoords[i].t());
		m_HashVertices.insert(pair<Au3dsGLSLVertex, int>((*it), i));
		vertex = (*it);
		i++;
	}
}

double Au3dsGLSLObject::GetTextureWeight(Au3dsGLSLVertex vert)
{
	QVector<Au3dsGLSLVGroup>::const_iterator itVGroup;
	for (itVGroup = m_VertexGroups.constBegin(); itVGroup != m_VertexGroups.constEnd(); itVGroup++)
	{
		if((*itVGroup).m_Type != VERT_BottomSkin)
			continue;
		if((*itVGroup).m_VertexList.find(vert) != (*itVGroup).m_VertexList.constEnd())
			return 0.0;
	}
	return 1.0;
}

void Au3dsGLSLObject::ScaleBy(double scale)
{
	m_Matrix[12] *= scale;
	m_Matrix[13] *= scale;
	m_Matrix[14] *= scale;
	m_Vertices.ScaleBy(scale);
	m_Offset.ScaleBy(scale);
	m_ConnectPt.ScaleBy(scale);
	m_MidPt.ScaleBy(scale);
	m_HandWingPt.ScaleBy(scale);
	m_TopScalePt.ScaleBy(scale);
	m_MidScalePt.ScaleBy(scale);
	m_BottomScalePt.ScaleBy(scale);
	m_EyePt.ScaleBy(scale);
	m_WeaponUpDownPt.ScaleBy(scale);
	m_WeaponLeftRightPt.ScaleBy(scale);

	unordered_multimap<Au3dsGLSLVertex, int, Au3dsGLSLVertex_Hash> oldhash(m_HashVertices);
	m_HashVertices.clear();
	for(unordered_multimap<Au3dsGLSLVertex, int, Au3dsGLSLVertex_Hash>::iterator vi = oldhash.begin(); vi != oldhash.end(); vi++)
	{
		Au3dsGLSLVertex newkey((*vi).first);
		newkey.ScaleBy(scale);
		m_HashVertices.insert(pair<Au3dsGLSLVertex, int>(newkey, (*vi).second));
	}
}

bool Au3dsGLSLObject::ApplyTextureOffset(const Au3dsGLSLVertex& v0, const Au3dsGLSLVertex& v1, const Au3dsGLSLVertex& v2, 
									double &v0uOffset, double &v0vOffset, 
									double &v1uOffset, double &v1vOffset,
									double &v2uOffset, double &v2vOffset) const
{
	bool bvPointBottom = false;
	bool v1Bottom = false;
	bool v2Bottom = false;
	QVector<Au3dsGLSLVGroup>::const_iterator itVGroup;
	for (itVGroup = m_VertexGroups.constBegin(); itVGroup != m_VertexGroups.constEnd(); itVGroup++)
	{
		if((*itVGroup).m_Type != VERT_BottomSkin)
			continue;
		if((*itVGroup).m_VertexList.find(v0) != (*itVGroup).m_VertexList.constEnd())
			bvPointBottom = true;
		else
			break;	// break out since we know this is a TopSkin face
		if((*itVGroup).m_VertexList.find(v1) != (*itVGroup).m_VertexList.constEnd())
			v1Bottom = true;
		else
			break;	// break out since we know this is a TopSkin face
		if((*itVGroup).m_VertexList.find(v2) != (*itVGroup).m_VertexList.constEnd())
			v2Bottom = true;
		else
			break;	// break out since we know this is a TopSkin face
	}

	v0uOffset = v0.U();
	v1uOffset = v1.U();
	v2uOffset = v2.U();
	v0vOffset = v0.V();
	v1vOffset = v1.V();
	v2vOffset = v2.V();
	// 	// Pixels left of 1792 get offset the rest stay put
	// 	float fCheck = 1792.0/2048.0;
	// Vertex offset for this face is top offset if
	// one of the vertice's is not bottom
	if(!bvPointBottom || !v1Bottom || !v2Bottom)
	{
		// Check Horizontal position
		// 		if(v0.U() < fCheck)
		// 		{
		// 			v0vOffset += m_VTOffset;
		// 			v0uOffset += m_UTOffset;
		// // 		}
		// // 		if(v1.U() < fCheck)
		// // 		{
		// 			v1vOffset += m_VTOffset;
		// 			v1uOffset += m_UTOffset;
		// // 		}
		// // 		if(v2.U() < fCheck)
		// // 		{
		// 			v2vOffset += m_VTOffset;
		// 			v2uOffset += m_UTOffset;
		/*		}*/
		return true;
	}

	// Bottom skin offset because all three vertices's are bottom ones
	// Check Horizontal position
	// 	if(v0.U() < fCheck)
	// 	{
	// 		v0vOffset += m_VBOffset;
	// 		v0uOffset += m_UBOffset;
	// // 	}
	// // 	if(v1.U() < fCheck)
	// // 	{
	// 		v1vOffset += m_VBOffset;
	// 		v1uOffset += m_UBOffset;
	// // 	}
	// // 	if(v2.U() < fCheck)
	// // 	{
	// 		v2vOffset += m_VBOffset;
	// 		v2uOffset += m_UBOffset;
	/*	}*/
	return false;
}

AuPoint Au3dsGLSLObject::ApplyScaleFactor(AuPoint& vPoint) const
{
	bool bUpperScaleFound = false;
	bool bLowerScaleFound = false;
	AuPoint offset, connectPt, midPt, handWingPt, upperScalePt, lowerScalePt;
	offset = AuPoint(m_Offset.X(), m_Offset.Y(), m_Offset.Z());
	connectPt = AuPoint(m_ConnectPt.X(), m_ConnectPt.Y(), m_ConnectPt.Z());
	midPt = AuPoint(m_MidPt.X(), m_MidPt.Y(), m_MidPt.Z());
	if(m_MidScalePt != Au3dsGLSLVertex(0, 0, 0))
		midPt = AuPoint(m_MidScalePt.X(), m_MidScalePt.Y(), m_MidScalePt.Z());
	if(m_TopScalePt != Au3dsGLSLVertex(0, 0, 0))
	{
		bUpperScaleFound = true;
		upperScalePt = AuPoint(m_TopScalePt.X(), m_TopScalePt.Y(), m_TopScalePt.Z());
	}
	if(m_BottomScalePt != Au3dsGLSLVertex(0, 0, 0))
	{
		bLowerScaleFound = true;
		lowerScalePt = AuPoint(m_BottomScalePt.X(), m_BottomScalePt.Y(), m_BottomScalePt.Z());
	}
	handWingPt = AuPoint(m_HandWingPt.X(), m_HandWingPt.Y(), m_HandWingPt.Z());
	QVector<Au3dsGLSLVGroup>::const_iterator itVGroup;
	AuPoint scalePt, torsoScalePt, handScalePt, footScalePt, neckScalePt, tailScalePt, headScalePt;
	//double scaleFactor = 0;
	bool bUpper = false;
	bool bLower = false;
	bool bFound = false;
	bool bMidPt = false;
	bool bTorso = false;
	bool bHand = false;
	bool bFoot = false;
	bool bNeck = false;
	bool bTail = false;
	bool bHead = false;
	bool bWing = false;
	bool bHandWing = false;
	for (itVGroup = m_VertexGroups.constBegin(); itVGroup != m_VertexGroups.constEnd(); itVGroup++)
	{
		if((*itVGroup).m_VertexList.find(Au3dsGLSLVertex(vPoint.x, vPoint.y, vPoint.z)) != (*itVGroup).m_VertexList.constEnd())
		{
			if((*itVGroup).m_Name.contains("Wing"))
				bWing = true;
			bFound = true;
			// Note: Any vertex not in BottomSkin group is considered TopSkin
			if((*itVGroup).m_Type == VERT_ScalePt)			// ScalePt
			{
				//scaleFactor = (*itVGroup).m_scaleFactor;
				scalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_Type == VERT_MidPt)	// MidPt
			{
				bMidPt = true;
			}
			else if((*itVGroup).m_Type == VERT_Top)	// Top
			{
				bUpper = true;
				if(!bUpperScaleFound)
					upperScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_Type == VERT_Bottom)	// Bottom
			{
				bLower = true;
				if(!bLowerScaleFound)
					lowerScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_bTorsoSegment)	// Torso segment piece
			{
				bTorso = true;
				torsoScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_bHand)	// Hand piece
			{
				if((*itVGroup).m_Name.contains("Wing"))
					bHandWing = true;
				bHand = true;
				handScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_bFoot)	// Foot piece
			{
				bFoot = true;
				footScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_bNeck)	// Neck piece
			{
				bNeck = true;
				neckScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_bTail)	// Tail piece
			{
				bTail = true;
				tailScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
			else if((*itVGroup).m_bHead)	// Head piece
			{
				bHead = true;
				headScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
		}
	}
	if(!bFound)
		return (vPoint*m_OverallScale)+(offset-connectPt*m_OverallScale);

	// Apply Overall Scale
	vPoint =  vPoint*m_OverallScale;
	connectPt = connectPt*m_OverallScale;
	midPt = midPt*m_OverallScale;
	scalePt = scalePt*m_OverallScale;
	handWingPt = handWingPt*m_OverallScale;

	if(bTail)
	{
		// X,Z - scaling
		if(m_LowerThickness != 1)		// This is torso thickness
		{
			vPoint = vPoint-tailScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y, vPoint.z*m_LowerThickness) + tailScalePt;

			connectPt = connectPt-tailScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x*m_LowerThickness, connectPt.y, connectPt.z*m_LowerThickness) + tailScalePt;
		}

		// Y - scaling
		if(m_LowerLength != 1)			// This is hand length
		{
			vPoint = vPoint-tailScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*m_LowerLength, vPoint.z) + tailScalePt;

			connectPt = connectPt-tailScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x, connectPt.y*m_LowerLength, connectPt.z) + tailScalePt;
		}
	}
	else if(bHead)
	{
		// Overall scaling
		if(m_LowerThickness != 1)		// This is torso thickness
		{
			vPoint = vPoint-headScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y*m_LowerThickness, vPoint.z*m_LowerThickness) + headScalePt;

			connectPt = connectPt-headScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x*m_LowerThickness, connectPt.y*m_LowerThickness, connectPt.z*m_LowerThickness) + headScalePt;
		}
	}
	else if(bNeck)
	{
		// X,Z - scaling
		if(m_LowerThickness != 1)		// This is torso thickness
		{
			vPoint = vPoint-neckScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y, vPoint.z*m_LowerThickness) + neckScalePt;

			connectPt = connectPt-neckScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x*m_LowerThickness, connectPt.y, connectPt.z*m_LowerThickness) + neckScalePt;
		}

		// Y - scaling
		if(m_LowerLength != 1)			// This is hand length
		{
			vPoint = vPoint-neckScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*m_LowerLength, vPoint.z) + neckScalePt;

			connectPt = connectPt-neckScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x, connectPt.y*m_LowerLength, connectPt.z) + neckScalePt;
		}
	}
	else if(bHand)
	{
		if(bHandWing)
		{
			// Apply movement from top length by changing offset of where lower meets upper
			AuPoint org = midPt;
			if(m_UpperLength != 1)
			{
				midPt = midPt-upperScalePt;		// Move to scale location
				midPt = AuPoint(midPt.x, midPt.y*m_UpperLength, midPt.z) + upperScalePt;			// Change midPt
				AuPoint handOffset = midPt-org;
				vPoint = vPoint+handOffset;
				org = midPt;				// Change original location
			}

			// Overall scaling
			if(m_LowerThickness != 1)		// This is forearm thickness
			{
				vPoint = vPoint-handScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y, vPoint.z*m_LowerThickness) + handScalePt;

				connectPt = connectPt-handScalePt;	// Move to scale location
				connectPt = AuPoint(connectPt.x*m_LowerThickness, connectPt.y, connectPt.z*m_LowerThickness) + handScalePt;
			}

			// Apply Bottom length - y scaling
			if(m_LowerLength != 1)
			{
				vPoint = vPoint-lowerScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x, vPoint.y*m_LowerLength, vPoint.z) + lowerScalePt;

				// Apply movement from bottom length
				midPt = midPt-lowerScalePt;		// Move to scale location
				midPt = AuPoint(midPt.x, midPt.y*m_LowerLength, midPt.z) + lowerScalePt;			// Change midPt
				AuPoint handOffset = org - midPt;
				vPoint = vPoint+handOffset;
			}
		}
		else
		{
			// Overall scaling
			if(m_LowerThickness != 1)		// This is forearm thickness
			{
				vPoint = vPoint-handScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y*m_LowerThickness, vPoint.z*m_LowerThickness) + handScalePt;

				connectPt = connectPt-handScalePt;	// Move to scale location
				connectPt = AuPoint(connectPt.x*m_LowerThickness, connectPt.y*m_LowerThickness, connectPt.z*m_LowerThickness) + handScalePt;
			}
		}

		double scaleLength = m_LowerLength;
		if(bHandWing)
			scaleLength = m_HandWingLength;

		// Y - scaling
		if(scaleLength != 1)			// This is hand length
		{
			vPoint = vPoint-handScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*scaleLength, vPoint.z) + handScalePt;

			// Position hand back to correct location
			if(bHandWing)
			{
				AuPoint org = handWingPt;
				handWingPt = handWingPt-handScalePt;		// Move to scale location
				handWingPt = AuPoint(handWingPt.x, handWingPt.y*scaleLength, handWingPt.z) + handScalePt;			// Change handWingPt
				AuPoint handOffset = org - handWingPt;
				vPoint = vPoint+handOffset;
			}
			else
			{
				connectPt = connectPt-handScalePt;	// Move to scale location
				connectPt = AuPoint(connectPt.x, connectPt.y*scaleLength, connectPt.z) + handScalePt;
			}
		}
	}
	else if(bFoot)
	{
		// Overall scaling
		if(m_LowerThickness != 1)		// This is calf thickness
		{
			vPoint = vPoint-footScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y*m_LowerThickness, vPoint.z*m_LowerThickness) + footScalePt;

			connectPt = connectPt-footScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x*m_LowerThickness, connectPt.y*m_LowerThickness, connectPt.z*m_LowerThickness) + footScalePt;
		}

		// Y - scaling
		if(m_LowerLength != 1)			// This is foot length
		{
			vPoint = vPoint-footScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*m_LowerLength, vPoint.z) + footScalePt;

			connectPt = connectPt-footScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x, connectPt.y*m_LowerLength, connectPt.z) + footScalePt;
		}
	}
	else if(bTorso)
	{
		if(m_UpperThickness != 1)
		{
			vPoint = vPoint-torsoScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x*m_UpperThickness, vPoint.y, vPoint.z*m_UpperThickness) + torsoScalePt;

			connectPt = connectPt-torsoScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x*m_UpperThickness, connectPt.y, connectPt.z*m_UpperThickness) + torsoScalePt;
		}
		if(m_UpperLength != 1)
		{
			vPoint = vPoint-torsoScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*m_UpperLength, vPoint.z) + torsoScalePt;

			connectPt = connectPt-torsoScalePt;	// Move to scale location
			connectPt = AuPoint(connectPt.x, connectPt.y*m_UpperLength, connectPt.z) + torsoScalePt;
		}
	}
	else if(bUpper)		// Upper Arm and Leg Scaling
	{
		// Apply Top Thickness - x,z scaling
		if(!bMidPt)
		{
			if(m_LowerThickness > m_UpperThickness && !bWing)
			{
				double dScale = (m_UpperThickness+m_LowerThickness)/2;
				vPoint = vPoint-upperScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*dScale, vPoint.y, vPoint.z*dScale) + upperScalePt;
			}
			else if(m_UpperThickness != 1)
			{
				vPoint = vPoint-upperScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*m_UpperThickness, vPoint.y, vPoint.z*m_UpperThickness) + upperScalePt;
			}
		}
		else if(bMidPt) // Middle points between Upper and Lower piece
		{
			// Apply Middle Thickness - x,z scaling
			double dScale = (m_UpperThickness+m_LowerThickness)/2;
			if(dScale != 1)
			{
				vPoint = vPoint-midPt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*dScale, vPoint.y, vPoint.z*dScale) + midPt;
			}
		}

		// Apply Top length - y scaling
		if(m_UpperLength != 1)
		{
			vPoint = vPoint-upperScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*m_UpperLength, vPoint.z) + upperScalePt;
		}
	}
	else if(bLower)		// Lower Arm and Leg Scaling
	{
		// Apply movement from top length by changing offset of where lower meets upper
		AuPoint org = midPt;
		if(m_UpperLength != 1 && !bWing)
		{
			midPt = midPt-upperScalePt;		// Move to scale location
			midPt = AuPoint(midPt.x, midPt.y*m_UpperLength, midPt.z) + upperScalePt;			// Change midPt
			AuPoint lowerOffset = midPt-org;
			vPoint = vPoint+lowerOffset;
			org = midPt;				// Change original location
		}

		// Apply Bottom Thickness - x,z scaling
		if(!bMidPt)
		{
			if(m_UpperThickness > m_LowerThickness && !bWing)
			{
				double dScale = (m_UpperThickness+m_LowerThickness)/2;
				vPoint = vPoint-lowerScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*dScale, vPoint.y, vPoint.z*dScale) + lowerScalePt;
			}
			else if(m_LowerThickness != 1)
			{
				vPoint = vPoint-lowerScalePt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*m_LowerThickness, vPoint.y, vPoint.z*m_LowerThickness) + lowerScalePt;
			}
		}
		else if(bMidPt) // Middle points between Upper and Lower piece
		{
			// Apply Middle Thickness - x,z scaling
			double dScale = (m_UpperThickness+m_LowerThickness)/2;
			if(dScale != 1)
			{
				vPoint = vPoint-midPt;	// Move to scale location
				vPoint = AuPoint(vPoint.x*dScale, vPoint.y, vPoint.z*dScale) + midPt;
			}
		}

		// Apply Bottom length - y scaling
		if(m_LowerLength != 1)
		{
			vPoint = vPoint-lowerScalePt;	// Move to scale location
			vPoint = AuPoint(vPoint.x, vPoint.y*m_LowerLength, vPoint.z) + lowerScalePt;

			// Apply movement from bottom length
			midPt = midPt-lowerScalePt;		// Move to scale location
			midPt = AuPoint(midPt.x, midPt.y*m_LowerLength, midPt.z) + lowerScalePt;			// Change midPt
			AuPoint lowerOffset = org - midPt;
			vPoint = vPoint+lowerOffset;
		}
	}

	// Apply movement from top length to connectPt
	if(m_UpperLength != 1 && ((!bTorso && !bHand && !bFoot && !bNeck && !bTail && !bHead && !bWing) || (bWing && bUpper)))
	{
		connectPt = (connectPt-upperScalePt);	// Move to scale location
		connectPt = AuPoint(connectPt.x, connectPt.y*m_UpperLength, connectPt.z) + upperScalePt;
	}

	// Connect to body part
	vPoint = vPoint + (offset-connectPt);

	//// Start independent scaling
	//if(scaleFactor != 0)
	//{
	//		scalePt = scalePt-headScalePt;
	//		scalePt = AuPoint(scalePt.x*m_LowerThickness, scalePt.y*m_LowerThickness, scalePt.z*m_LowerThickness) + headScalePt;
	//	scalePt = scalePt + (offset-connectPt);
	//	vPoint = vPoint-scalePt;
	//	vPoint = AuPoint(vPoint.x*scaleFactor, vPoint.y*scaleFactor, vPoint.z*scaleFactor) + scalePt;
	//}
	return vPoint;
}

void Au3dsGLSLObject::AverageWeights(int pb, int tb, int nb, QString sFilename)
{	
	if(m_bHasBones)
	{
		QVector<Au3dsGLSLVGroup>::const_iterator itVGroup;
		for(Au3dsGLSLVertexArray::iterator it = m_Vertices.begin(); it != m_Vertices.end(); it++)
		{
			int iBoneCount = 0;
			int iBone = -1;
			double vertWeight = 0;
			bool bTop = false;
			bool bBottom = false;
			// First try and find this vertex in the Top/Bottom Connect groups
			for (itVGroup = m_VertexGroups.constBegin(); itVGroup != m_VertexGroups.constEnd(); itVGroup++)
			{
				if((*itVGroup).m_Type == VERT_TopConnect)
				{
					if((*itVGroup).m_VertexList.find((*it)) != (*itVGroup).m_VertexList.end())
						bTop = true;
				}
				if((*itVGroup).m_Type == VERT_BottomConnect)
				{
					if((*itVGroup).m_VertexList.find((*it)) != (*itVGroup).m_VertexList.end())
						bBottom = true;
				}
			}

			// Now look for Weight group types this vertex belongs to and add them together
			for (itVGroup = m_VertexGroups.constBegin(); itVGroup != m_VertexGroups.constEnd(); itVGroup++)
			{
				if(iBoneCount > 4)
				{
					if(s_ShowErrors)
					qWarning("Au3dsGLSLObject::AverageWeights too many bone groups assigned to vertex");
					continue;
				}
				// Only allow Weight group types, i.e. ones with bone names
				if((*itVGroup).m_Type != VERT_Weight)
					continue;
				QMap<Au3dsGLSLVertex, double>::const_iterator itVert = (*itVGroup).m_VertexList.find((*it));
				if(itVert == (*itVGroup).m_VertexList.end())
					continue;		// Vertex not found in this vertex group

				// The vertex belongs to this vertex group, perform the average weights calculation
				int ibidx = 0;
				if((*itVGroup).m_bTorsoSegment)
				{
					if(IsBone((*itVGroup).m_Name, tb))
					{
						iBone = 1;
						ibidx = tb;
					}
					else
					{
						if(s_ShowErrors)
						qWarning() << "Au3dsGLSLObject::AverageWeights vertex group" << m_Name << "skipped in file" << sFilename
							<< "Was this the intent?";
						continue;
					}
				}

				// Get bone index and bone weight position
				else if((*itVGroup).m_bTail)
				{
					// Tail models have too many bones to pass into.  Instead lets just find
					// the bone appropriate for the vertex group
					ibidx = FindTailBone((*itVGroup).m_Name);
					iBone = iBoneCount;
				}
				else if((*itVGroup).m_bNeck)
				{
					// Neck models have too many bones to pass into.  Instead lets just find
					// the bone appropriate for the vertex group
					ibidx = FindNeckBone((*itVGroup).m_Name);
					iBone = iBoneCount;
				}
				else
				{
					if(IsBone((*itVGroup).m_Name, pb))
					{
						iBone = 0;
						ibidx = pb;
					}
					else if(IsBone((*itVGroup).m_Name, tb))
					{
						iBone = 1;
						ibidx = tb;
					}
					else if(IsBone((*itVGroup).m_Name, nb))
					{
						iBone = 2;
						ibidx = nb;
					}
					else
					{
						if(s_ShowErrors)
						qWarning() << "Au3dsGLSLObject::AverageWeights vertex group" << m_Name << "skipped in file" << sFilename
							<< "Was this the intent?";
						continue;
					}
				}

				// Add to vertWeight
				if((*itVGroup).m_bTail || (*itVGroup).m_bNeck)
				{
					if(iBone == 1 || iBone == 2)
					{
						// Change order if necessary
						(*it).m_wt[iBone] = itVert.value();
						(*it).m_idx[iBone] = ibidx;
						if((*it).m_idx[iBone] < (*it).m_idx[iBone-1])
						{
							int boneValue = (*it).m_idx[iBone-1];
							GLfloat weightValue = (*it).m_wt[iBone-1];
							(*it).m_wt[iBone-1] = itVert.value();
							(*it).m_idx[iBone-1] = ibidx;
							(*it).m_wt[iBone] = weightValue;
							(*it).m_idx[iBone] = boneValue;
						}
					}
					else
					{
						(*it).m_wt[iBone] = itVert.value();
						(*it).m_idx[iBone] = ibidx;
					}
					iBoneCount++;
					vertWeight = vertWeight+(*it).m_wt[iBone];
				}
				else
				{
					if(bTop || bBottom)
					{
						for(int i=0; i<3; i++)
						{
							(*it).m_wt[i] = itVert.value();
							// This is working for the bending of torso pieces
							// with one bend.  Not sure on the results of two bends
							// because we don't do that yet.
							if(i == 0 || i == 1)
							{
								if(bTop)
									(*it).m_idx[i] = pb;
								else
									(*it).m_idx[i] = tb;
							}
							//else if(i == 1)
							//{
							//	if(bTop)
							//		(*it).m_idx[i] = pb;
							//	else
							//		(*it).m_idx[i] = tb;
							//}
							else
							{
								if(bTop)
									(*it).m_idx[i] = tb;
								else
									(*it).m_idx[i] = nb;
							}
							vertWeight = vertWeight+(*it).m_wt[i];
						}
					}
					else
					{
						(*it).m_wt[iBone] = itVert.value();
						(*it).m_idx[iBone] = ibidx;
						iBoneCount++;
						vertWeight = vertWeight+(*it).m_wt[iBone];
					}
				}
			}

			// Average the weight
			for(int i=0; i<4; i++)
				(*it).m_wt[i] = (*it).m_wt[i]/vertWeight;
		}
	}
}

bool Au3dsGLSLObject::IsBone(QString string, int bone) const
{
	if(string.contains("Torso"))
	{
		if(bone == 0 || bone == 1 || bone == 2 ||
			bone == 3 || bone == 4 || bone == 5 || bone == 6)
			return true;
		return false;
	}
	else if(string.contains("Head"))
	{
		if(bone == 7 || bone == 12)
			return true;
		return false;
	}
	else if(string.contains("Tail"))
	{
		if(bone == 17 || bone == 18 || bone == 19 || 
			bone == 20 || bone == 21 || bone == 22)
			return true;
		return false;
	}
	else if(string.contains("Hand"))
	{
		if(string.contains("Left"))
		{
			if(bone == 23 || bone == 29 || bone == 35)
				return true;
			return false;
		}
		if(bone == 26 || bone == 32 || bone == 38)
			return true;
		return false;
	}
	else if(string.contains("ForeArm"))
	{
		if(string.contains("Left"))
		{
			if(bone == 24 || bone == 30 || bone == 36)
				return true;
			return false;
		}
		if(bone == 27 || bone == 33 || bone == 39)
			return true;
		return false;
	}
	else if(string.contains("UpperArm"))
	{
		if(string.contains("Left"))
		{
			if(bone == 25 || bone == 31 || bone == 37)
				return true;
			return false;
		}
		if(bone == 28 || bone == 34 || bone == 40)
			return true;
		return false;
	}
	else if(string.contains("Foot"))
	{
		if(string.contains("Left"))
		{
			if(bone == 41 || bone == 47 || bone == 53)
				return true;
			return false;
		}
		if(bone == 44 || bone == 50 || bone == 56)
			return true;
		return false;
	}
	else if(string.contains("Calf"))
	{
		if(string.contains("Left"))
		{
			if(bone == 42 || bone == 48 || bone == 54)
				return true;
			return false;
		}
		if(bone == 45 || bone == 51 || bone == 57)
			return true;
		return false;
	}
	else if(string.contains("Thigh"))
	{
		if(string.contains("Left"))
		{
			if(bone == 43 || bone == 49 || bone == 55)
				return true;
			return false;
		}
		if(bone == 46 || bone == 52 || bone == 58)
			return true;
		return false;
	}
	else if(string.contains("Jaw"))
	{
		if(bone == 59 || bone == 60)
			return true;
		return false;
	}

	return false;
}

int Au3dsGLSLObject::FindTailBone(QString string) const
{
	if(string == "Tail0")
		return 17;
	else if(string == "Tail1")
		return 18;
	else if(string == "Tail2")
		return 19;
	else if(string == "Tail3")
		return 20;
	else if(string == "Tail4")
		return 21;
	else if(string == "Tail5")
		return 22;

	return 17;		// Return a valid bone
}

int Au3dsGLSLObject::FindNeckBone(QString string) const
{
	if(string == "Neck00")
		return 8;
	else if(string == "Neck10")
		return 13;
	else if(string == "Neck01")
		return 9;
	else if(string == "Neck11")
		return 14;
	else if(string == "Neck02")
		return 10;
	else if(string == "Neck12")
		return 15;
	else if(string == "Neck03")
		return 11;
	else if(string == "Neck13")
		return 16;
	return false;
}

// returns true if this object identifies a weapon firing position.
// This is typically done using a single triangle. The first vertex in the
// vertices array becomes the base location of the weapon fire effect.
// The center point between the other two vertices is used to calculate a unit
// vector that points in the direction the weapon fires.
// 'weapon' is output if the return value is true
// A naming convention determines whether this object is actually a weapon
// firing point definition.
// First character is '!'
// Second character is subtracted from ASCII '0' to get an id value. This id
// probably should be unique within an object but it's really up to each object
// to decide what it means. e.g. A tank might decide weapon 0 is the main gun
// and weapon 1 is an auxiliary weapon.
// Other characters may follow the second character. This is the symbol element
// id in Symmetrica; we don't need it for anything.
bool Au3dsGLSLObject::TestForWeapon(Au3dsGLSLWeapon& weapon, MODEL_TYPE modelType) const
{
	if(m_Vertices.isEmpty())
		return false;
	if(m_Name.length() < 2)
		return false;
	if(m_Name[0] != '!')
		return false;
	quint16 id = QString(m_Name[1]).toInt();
	if(id > 26)
		return false;

	AuPoint loc, dir;
	if(HasPrecalculatedNormals(modelType))
	{
		QVector<Au3dsGLSLVGroup>::const_iterator it;
		for(it = m_VertexGroups.constBegin(); it != m_VertexGroups.constEnd(); it++)
		{
			if((*it).m_Type == VERT_WeaponLoc)
			{
				Au3dsGLSLVertex vertex = (*it).m_ScalePt;
				loc = AuPoint(vertex.X(), vertex.Y(), vertex.Z());
			}
			else if((*it).m_Type == VERT_DirLoc)
			{
				Au3dsGLSLVertex vertex = (*it).m_ScalePt;
				dir = AuPoint(vertex.X(), vertex.Y(), vertex.Z());
			}
		}
		dir = dir-loc;
	}
	else
	{
		loc = AuPoint(m_Vertices[0].X(), m_Vertices[0].Y(), m_Vertices[0].Z());
		// Firing direction is the unit vector pointing to the center between the
		// other two triangle points so we don't have to worry about the vertex
		// order.
		AuPoint end0(m_Vertices[1].X(), m_Vertices[1].Y(), m_Vertices[1].Z());
		AuPoint end1(m_Vertices[2].X(), m_Vertices[2].Y(), m_Vertices[2].Z());
		AuPoint end = (end0 + end1)/2.;
		dir = end - loc;
	}
	dir.Unit();
	weapon.SetData(id, loc, dir);
	return true;
}

// returns true if this object identifies a station position that can be
// occupied by a person. This is typically done using a single triangle. The
// first vertex in the vertices array becomes the location of the station.
// 'station' is output if the return value is true
// A naming convention determines whether this object is actually a station
// for a person to occupy.
// First character is '@'
// Second character is subtracted from ASCII '0' to get an id value. This id
// probably should be unique within an object but it's really up to each object
// to decide what it means. e.g. A boat might use station 0 to be the helm.
// The next two characters are taken as a hexadecimal number, which is a
// bitwise combination of Au3dsGLSLStation::POSE values.
// Other characters may follow the fourth character. This is the symbol element
// id in Symmetrica; we don't need it for anything.
bool Au3dsGLSLObject::TestForStation(Au3dsGLSLStation& station, MODEL_TYPE modelType) const
{
	if(m_Vertices.isEmpty())
		return false;
	if(m_Name.length() < 4)
		return false;
	if(m_Name[0] != '@')
		return false;
	quint16 id = QString(m_Name[1]).toInt();
	if(id > 26)
		return false;
	QString poses = m_Name.mid(2,2);
	bool convOk = false;
	quint8 pose = poses.toUShort(&convOk, 16);	// Convert base 16 ASCII number
	if(!convOk)
		return false;

	//qDebug("Au3dsGLSLObject::TestForStation name %s poses '%s' pose %d",
	//(const char*)m_Name.toAscii(), (const char*)poses.toAscii(),
	//pose);
	if(pose == (quint8)Au3dsGLSLStation::Invalid)// || pose > (quint8)Au3dsGLSLStation::All)
		return false;

	AuPoint loc, off;
	if(HasPrecalculatedNormals(modelType))
	{
		QVector<Au3dsGLSLVGroup>::const_iterator it;
		for(it = m_VertexGroups.constBegin(); it != m_VertexGroups.constEnd(); it++)
		{
			if((*it).m_Type == VERT_SeatLoc)
			{
				Au3dsGLSLVertex vertex = (*it).m_ScalePt;
				loc = AuPoint(vertex.X(), vertex.Y(), vertex.Z());
			}
			else if((*it).m_Type == VERT_OffLoad)
			{
				Au3dsGLSLVertex vertex = (*it).m_ScalePt;
				off = AuPoint(vertex.X(), vertex.Y(), vertex.Z());
			}
		}
	}
	else
	{
		loc = AuPoint(m_Vertices[0].X(), m_Vertices[0].Y(), m_Vertices[0].Z());
		// Offload location is the center between the other two triangle points
		// so we don't have to worry about the order.
		AuPoint off0(m_Vertices[1].X(), m_Vertices[1].Y(), m_Vertices[1].Z());
		AuPoint off1(m_Vertices[2].X(), m_Vertices[2].Y(), m_Vertices[2].Z());
		off = (off0 + off1)/2.;
	}
	station.SetData(id, loc, off, pose);
	return true;
}

bool Au3dsGLSLObject::TestForScreens(Au3dsGLSLScreen& screen1, Au3dsGLSLScreen& screen2) const
{
	//if(m_Vertices.isEmpty())
		//return false;

	QVector<Au3dsGLSLVGroup>::const_iterator it;
	for(it = m_VertexGroups.constBegin(); it != m_VertexGroups.constEnd(); it++)
	{
		if((*it).m_Type == VERT_Screen1 || (*it).m_Type == VERT_Screen2)
		{
			// The vertex group is a display screen.
			if((*it).m_VertexList.count() < 4)
				return false;

			// Grab the first four vertices from the vertex group.
			AuPoint loc0, loc1, loc2, loc3;
			QMap<Au3dsGLSLVertex, double>::const_iterator si = (*it).m_VertexList.constBegin();
			loc0 = AuPoint(si.key().X(), si.key().Y(), si.key().Z());
			si++;
			loc1 = AuPoint(si.key().X(), si.key().Y(), si.key().Z());
			si++;
			loc2 = AuPoint(si.key().X(), si.key().Y(), si.key().Z());
			si++;
			loc3 = AuPoint(si.key().X(), si.key().Y(), si.key().Z());

			// We can stop searching. We can only handle one screen per object.
			if((*it).m_Type == VERT_Screen1)
				screen1.SetData((*it).m_Type, loc0, loc1, loc2, loc3);
			else
				screen2.SetData((*it).m_Type, loc0, loc1, loc2, loc3);
		}
	}

	if(screen1.VertexGroup() != VERT_Invalid && screen2.VertexGroup() != VERT_Invalid)
		return true;
	return false;
}

bool Au3dsGLSLObject::TestForScalePt()
{
	if(m_Vertices.isEmpty())
		return false;
	if(m_Name.length() < 4)
		return false;
	if(!m_Name.contains("SPt"))
		return false;

	return true;
}

bool Au3dsGLSLObject::TestForUpVector(AuPoint &UpVector)
{
	if(m_Vertices.isEmpty())
		return false;
	if(m_Name.length() < 4)
		return false;
	if(m_Name.contains("UpVector"))
	{
		// This guy is the up vector object.
		// one point of this edge is at 0,0,0. The other one is the vector from there that is up.
		// Grab the first two vertices...should be the ONLY vertices...
		AuPoint end0(m_Vertices[0].X(), m_Vertices[0].Y(), m_Vertices[0].Z());
		AuPoint end1(m_Vertices[1].X(), m_Vertices[1].Y(), m_Vertices[1].Z());
		UpVector = end0 + end1;	// not sure which it is. add them is faster than comparing? One is zero so adding them is safe for now.
		UpVector.Unit();

		return true;
	}

	return false;
}

// Returns true if this object contributes to the extents of the overall model.
bool Au3dsGLSLObject::TestForExtents(MODEL_TYPE modelType) const
{
	// An object with no vertices cannot contribute to the extents.
	if(m_Vertices.isEmpty())
		return false;

	// Objects with names that begins with dash '-' are not included in extents.
	if(!m_Name.isEmpty() && m_Name[0] == '-')
		return false;

	// Station definition objects are not included in extents.
	Au3dsGLSLStation station;
	if(TestForStation(station, modelType))
		return false;

	// Weapon definition objects are not included in extents.
	Au3dsGLSLWeapon weapon;
	if(TestForWeapon(weapon, modelType))
		return false;

	// This object is included in the extents.
	return true;
}

quint64 Au3dsGLSLObject::MemoryUsage() const
{
	quint64 mem = sizeof(Au3dsGLSLObject);
	mem += StringMemoryUsage(m_Name);
	for(QMap<QString, QVector<quint16> >::const_iterator mit = m_Materials.constBegin(); mit != m_Materials.constEnd(); mit++)
		mem += StringMemoryUsage(mit.key()) + sizeof(quint16) * (*mit).capacity();
	mem += m_MappingCoords.MemoryUsage();
	mem += sizeof(quint32) * m_FaceSmoothing.capacity();
	mem += m_Faces.MemoryUsage();
	mem += m_Vertices.MemoryUsage();
	mem += m_VertexGroups.MemoryUsage();		// Vertex group definition
	mem += (sizeof(Au3dsGLSLVertex) + sizeof(int)) * m_HashVertices.size();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

// Create a gl display list of each component object.
// Objects are created opaque first then transparent.
void Au3dsGLSLObjectArray::CreateDeviceObjects(AuOpenGLFunctions& GLf, QList<Au3dsGLSLVAO>& models,
		const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc)
{
	//qDebug("Au3dsGLSLObjectArray::CreateDeviceObjects gl models %d", models);
	// Create the opaque objects first.
	if(!IsSymmetricaModel(modelType))
	{
		for(iterator it = begin(); it != end(); it++)
		{
			// 			if(IsSymmetricaModel(modelType))
			// 			{
			if((*it).Transparency(materials, modelType) == 1)
			{
				Au3dsGLSLVAO model;
				(*it).CreateDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
			// 			}
			// 			else
			// 			{
			// 				if(lightsOn)
			// 				{
			// 					if((*it).Transparency(materials) == 1 && (*it).m_bLight)
			// 					{
			// 						(*it).CreateDeviceObject(models, materials, lightsOn, modelType, destBlendFunc);
			// 						models++;
			// 					}
			// 				}
			// 			}
		}

		// NOTE: Lights and Weapon objects will never be transparent
		// Create the transparent objects last.
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) < 1)
			{
				Au3dsGLSLVAO model;
				(*it).CreateDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
		}
	}
	else
	{
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) == 0)
			{
				Au3dsGLSLVAO model;
				(*it).CreateDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
		}

		// Create the transparent objects last.
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) > 0)
			{
				Au3dsGLSLVAO model;
				(*it).CreateDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
		}
	}
}

// Create a gl display list of each component object.
// Objects are created opaque first then transparent.
void Au3dsGLSLObjectArray::CreateBodyDeviceObjects(AuOpenGLFunctions& GLf, QList<Au3dsGLSLVAO>& models,
		const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc)
{
	//qDebug("Au3dsGLSLObjectArray::CreateDeviceObjects gl models %d", models);
	// Create the opaque objects first.
	if(!IsSymmetricaModel(modelType))
	{
		for(iterator it = begin(); it != end(); it++)
		{
			// 			if(IsSymmetricaModel(modelType))
			// 			{
			if((*it).Transparency(materials, modelType) == 1)
			{
				Au3dsGLSLVAO model;
				(*it).CreateBodyDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
			// 			}
			// 			else
			// 			{
			// 				if(lightsOn)
			// 				{
			// 					if((*it).Transparency(materials) == 1 && (*it).m_bLight)
			// 					{
			// 						(*it).CreateBodyDeviceObject(models, materials, lightsOn, modelType, destBlendFunc);
			// 						models++;
			// 					}
			// 				}
			// 			}
		}

		// NOTE: Lights and Weapon objects will never be transparent
		// Create the transparent objects last.
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) < 1)
			{
				Au3dsGLSLVAO model;
				(*it).CreateBodyDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
		}
	}
	else
	{
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) == 0)
			{
				Au3dsGLSLVAO model;
				(*it).CreateBodyDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
		}

		// Create the transparent objects last.
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) > 0)
			{
				Au3dsGLSLVAO model;
				(*it).CreateBodyDeviceObject(GLf, model, materials, lightsOn, modelType, destBlendFunc);
				models.append(model);
			}
		}
	}
}

void Au3dsGLSLObjectArray::LoadModels(QList<Au3dsGLSLTriTexList>& models, const Au3dsGLSLMaterialArray& materials, bool lightsOn,
		MODEL_TYPE modelType, GLuint destBlendFunc)
{
	// Create the opaque objects first.
	if(!IsSymmetricaModel(modelType))
	{
		for(iterator it = begin(); it != end(); it++)
		{
			// 			if(IsSymmetricaModel(modelType))
			// 			{
			if((*it).Transparency(materials, modelType) == 1)
			{
				Au3dsGLSLTriTexList smdl;
				(*it).LoadModel(smdl, materials, lightsOn, modelType, destBlendFunc);
				models.append(smdl);
			}
			// 			}
			// 			else
			// 			{
			// 				if(lightsOn)
			// 				{
			// 					if((*it).Transparency(materials) == 1 && (*it).m_bLight)
			// 					{
			// 						(*it).CreateDeviceObject(models, materials, lightsOn, modelType, destBlendFunc);
			// 						models++;
			// 					}
			// 				}
			// 			}
		}

		// NOTE: Lights and Weapon objects will never be transparent
		// Create the transparent objects last.
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) < 1)
			{
				Au3dsGLSLTriTexList smdl;
				(*it).LoadModel(smdl, materials, lightsOn, modelType, destBlendFunc);
				models.append(smdl);
			}
		}
	}
	else
	{
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) == 0)
			{
				Au3dsGLSLTriTexList smdl;
				(*it).LoadModel(smdl, materials, lightsOn, modelType, destBlendFunc);
				models.append(smdl);
			}
		}

		// Create the transparent objects last.
		for(iterator it = begin(); it != end(); it++)
		{
			if((*it).Transparency(materials, modelType) > 0)
			{
				Au3dsGLSLTriTexList smdl;
				(*it).LoadModel(smdl, materials, lightsOn, modelType, destBlendFunc);
				models.append(smdl);
			}
		}
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

Au3dsGLSLModel::Au3dsGLSLModel()
{
	m_GeometryScale = 1;
	m_ModelType = MODEL_3ds;
	m_RoughRadius = 0;
	m_Screens = 0;
	m_Stations = 0;
	m_Weapons = 0;

	m_UpVector = AuPoint(0,0,1);
}

Au3dsGLSLModel::~Au3dsGLSLModel()
{
	if(m_Screens)
	{
		delete m_Screens;
		m_Screens = 0;
	}
	if(m_Stations)
	{
		delete m_Stations;
		m_Stations = 0;
	}
	if(m_Weapons)
	{
		delete m_Weapons;
		m_Weapons = 0;
	}
}

bool Au3dsGLSLModel::Load(const QString& file3dsName, double scale)
{
	m_GeometryScale = scale;
	Q_ASSERT(!file3dsName.isEmpty());
	QFile file3ds(file3dsName);
	if(file3ds.open(QIODevice::ReadOnly))
	{
		//qDebug("Au3dsGLSLModel::Load file '%s'", (const char*)file3dsName.toAscii());
		bool success = Load(file3ds, scale);
		file3ds.close();
		return success;
	}
	else
	{
		if(s_ShowErrors)
		qWarning() << "Au3dsGLSLModel::Load ERROR opening file" << file3dsName;
		return false;
	}
}

bool Au3dsGLSLModel::Load(QFile& file3ds, double scale)
{
	m_GeometryScale = scale;
	QDataStream in(&file3ds);
	if(in.atEnd())
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLModel::Load ERROR empty 3DS file");
		return false;
	}

	if(file3ds.fileName().contains(".3dsX"))
		m_ModelType = MODEL_3dsX;
	else if(file3ds.fileName().contains(".3dsNT"))
		m_ModelType = MODEL_3dsNT;
	else if(file3ds.fileName().contains(".3dsN"))
		m_ModelType = MODEL_3dsN;
	else
		m_ModelType = MODEL_3ds;

	// Clear the model data.
	m_Materials.clear();
	m_Objects.clear();
	if(m_Screens)
	{
		delete m_Screens;
		m_Screens = 0;
	}
	if(m_Stations)
	{
		delete m_Stations;
		m_Stations = 0;
	}
	if(m_Weapons)
	{
		delete m_Weapons;
		m_Weapons = 0;
	}

	// Load the MAIN3DS chunk, which is always first.
	quint16 chunkName = Au3dsGLSL::LoadUint16(in);
	if(chunkName != MAIN3DS)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLModel::Load ERROR MAIN3DS chunk missing");
		return false;
	}
	quint32 chunkSize = Au3dsGLSL::LoadUint32(in);
	if(!chunkSize)
		return true;	// validly stated empty file. Return nicely.

	// Load the main chunk of the file.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	//qDebug("Au3dsGLSLModel::Load loading MAIN3DS chunk of %d bytes", chunkSize);
	bool loadOk = LoadMainChunk(in, chunkSize - chunkHeaderSize);
	if(loadOk)
	{
		// Post process the model according to our specifications.
		// - Apply scaling
		// - Look for screens on control consoles.
		// - Look for stations at which people may be positioned.
		// - Look for weapon firing locations.
		// - Look for building definitions.
		if(m_Objects.count())
		{
			Au3dsGLSLScreen screen1, screen2;
			Au3dsGLSLStation station;
			Au3dsGLSLWeapon weapon;
			Au3dsGLSLObjectArray::iterator it = m_Objects.begin();
			while(it != m_Objects.end())
			{
				if(scale != 1 && scale > 0)
					(*it).ScaleBy(scale);

				//qDebug(" object %s", (const char*)(*it).Name().toAscii());
				if((*it).TestForStation(station, m_ModelType))
				{
					// The stations array is only created when needed.
					if(!m_Stations)
						m_Stations = new Au3dsGLSLStationArray;

					// Stations are sorted into the array in 'id' order.
					bool inserted = false;
					for(Au3dsGLSLStationArray::iterator si = m_Stations->begin();
						si != m_Stations->end(); si++)
					{
						if((*si).Id() > station.Id())
						{
							// Insert the new station before the 'si' entry.
							m_Stations->insert(si, station);
							inserted = true;
							break;
						}
					}
					// If not inserted, the new station belongs at the end.
					if(!inserted)
						m_Stations->append(station);

					// The station definition object is not kept as part of the model.
					it = m_Objects.erase(it);
				}
				else if((*it).TestForWeapon(weapon, m_ModelType))
				{
					// The weapons array is only created when needed.
					if(!m_Weapons)
						m_Weapons = new Au3dsGLSLWeaponArray;

					// Weapons are sorted into the array in 'id' order.
					bool inserted = false;
					for(Au3dsGLSLWeaponArray::iterator si = m_Weapons->begin(); si != m_Weapons->end(); si++)
					{
						if((*si).Id() > weapon.Id())
						{
							// Insert the new weapon before the 'si' entry.
							m_Weapons->insert(si, weapon);
							inserted = true;
							break;
						}
					}
					// If not inserted, the new weapon belongs at the end.
					if(!inserted)
						m_Weapons->append(weapon);

					// The weapon definition object is not kept as part of the model.
					it = m_Objects.erase(it);
				}
				else if((*it).TestForScreens(screen1, screen2))
				{
					// The screens array is only created when needed.
					if(!m_Screens)
						m_Screens = new Au3dsGLSLScreenArray;

					/*
					qDebug("Screen1 Found %d", screen1.VertexGroup());
					screen1.LocLL().Dump(" locLL");
					screen1.LocUL().Dump(" locUL");
					screen1.LocUR().Dump(" locUR");
					screen1.LocLR().Dump(" locLR");

					qDebug("Screen2 Found %d", screen2.VertexGroup());
					screen2.LocLL().Dump(" locLL");
					screen2.LocUL().Dump(" locUL");
					screen2.LocUR().Dump(" locUR");
					screen2.LocLR().Dump(" locLR");
					*/

					// Append screens, 1 then 2, so index 0 is the big screen.
					m_Screens->append(screen1);
					m_Screens->append(screen2);

					// The screen definition object is not kept as part of the model.
					//it = m_Objects.erase(it);
					it++;
				}
				else if((*it).TestForUpVector(m_UpVector))
				{
					it = m_Objects.erase(it);
				}
				else
					it++;
			}
		}

		if(scale != 1 && scale > 0)
		{
			m_Min *= scale;
			m_Max *= scale;
		}

		// Calculate rough radius of this model.
		m_RoughRadius = AuBox(m_Min, m_Max).RoughRadius();

		// Dump the result.
		/*
		qDebug() << "Au3dsGLSLModel::Load" << file3ds.fileName() << "min" << m_Min << "max" << m_Max << "scale" << scale
			<< "roughRad" << m_RoughRadius;
		if(m_Materials.count())
		{
		qDebug("Materials %d", m_Materials.count());
		for(Au3dsGLSLMaterialArray::const_iterator it = m_Materials.constBegin();
		it != m_Materials.constEnd(); it++)
		(*it).Dump();
		}
		else
		qDebug("No materials loaded.");

		if(m_Objects.count())
		{
		qDebug("Objects %d", m_Objects.count());
		for(Au3dsGLSLObjectArray::const_iterator it = m_Objects.constBegin();
		it != m_Objects.constEnd(); it++)
		(*it).Dump();
		}
		else
		qDebug("No objects loaded.");

		if(m_Stations && m_Stations->count())
		{
		qDebug("Stations %d", m_Stations->count());
		for(Au3dsGLSLStationArray::const_iterator it = m_Stations->constBegin();
		it != m_Stations->constEnd(); it++)
		(*it).Dump();
		}

		if(m_Weapons && m_Weapons->count())
		{
		qDebug("Weapons %d", m_Weapons->count());
		for(Au3dsGLSLWeaponArray::const_iterator it = m_Weapons->constBegin();
		it != m_Weapons->constEnd(); it++)
		(*it).Dump();
		}
		*/
	}
	else
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLModel::Load FAILED");
	}
	return loadOk;
}

// Load the subchunks of the MAIN3DS chunk.
bool Au3dsGLSLModel::LoadMainChunk(QDataStream& in, int chunkSize)
{
	// Load the subchunks.
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	bool edit3dsLoaded = false;
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLModel::LoadMainChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in chunk",
					subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case EDIT3DS:
			//qDebug("Au3dsGLSLModel::LoadMainChunk loading EDIT3DS subchunk of "
			//"%d bytes", subChunkSize);
			LoadEditorChunk(in, subChunkSize - chunkHeaderSize);
			edit3dsLoaded = true;	// we tried, that's good enough
			break;
		default:
			//qDebug("Au3dsGLSLModel::LoadMainChunk skipping %04x subchunk of "
			//"%d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(!edit3dsLoaded && !in.atEnd());
	if(!edit3dsLoaded)
	{
		if(s_ShowErrors)
		qWarning("Au3dsGLSLModel::LoadMainChunk ERROR EDIT3DS chunk missing");
		return false;
	}
	//qDebug("Au3dsGLSLModel::LoadMainChunk done");
	return true;
}

// Load the subchunks of the EDIT3DS chunk.
bool Au3dsGLSLModel::LoadEditorChunk(QDataStream& in, int chunkSize)
{
	// Load the subchunks.
	bool extentsValid = false;
	const int chunkHeaderSize = sizeof(quint16) + sizeof(quint32);
	do
	{
		quint16 subChunkName = Au3dsGLSL::LoadUint16(in);
		quint32 subChunkSize = Au3dsGLSL::LoadUint32(in);
		//Q_ASSERT(subChunkSize);
		if(subChunkSize > (quint32)chunkSize)
		{
			if(s_ShowErrors)
			qWarning("error: Au3dsGLSLModel::LoadEditorChunk bad chunk size %d for subchunk %04x exceeds remaining %d bytes in "
					"chunk", subChunkSize, subChunkName, chunkSize);
			subChunkSize = chunkSize;
		}
		chunkSize -= subChunkSize;
		switch(subChunkName)
		{
		case EDIT_MATERIAL:
			{
				//qDebug("Au3dsGLSLModel::LoadEditorChunk loading EDIT_MATERIAL "
				//"subchunk of %d bytes", subChunkSize);
				Au3dsGLSLMaterial material;
				if(material.LoadChunk(in, subChunkSize - chunkHeaderSize, m_ModelType))
					m_Materials.append(material);
			}
			break;
		case EDIT_OBJECT:
			{
				//qDebug("Au3dsGLSLModel::LoadEditorChunk loading EDIT_OBJECT subchunk of %d bytes", subChunkSize);
				AuPoint min,max;	// extents of the object
				Au3dsGLSLObject object;
				if(object.LoadChunk(in, subChunkSize - chunkHeaderSize, min, max, m_ModelType))
				{
					// Some objects are not included in the extents.
					if(object.TestForExtents(m_ModelType))
					{
						// Grow the overall extents of the model using the
						// extents of the object.
						if(extentsValid)
						{
							m_Min = m_Min.Min(min);
							m_Max = m_Max.Max(max);
						}
						else
						{
							m_Min = min;
							m_Max = max;
							extentsValid = true;
						}
						//qDebug() << "Au3dsGLSLModel::LoadEditorChunk loaded EDIT_OBJECT subchunk" << subChunkSize << "bytes"
							//<< "name" << object.Name() << "min" << min << "max" << max << "scale" << scale;
					}
					m_Objects.append(object);
				}
				//else
					//qDebug("Au3dsGLSLModel::LoadEditorChunk EDIT_OBJECT failed to load!");
			}
			break;
		case EDIT_VIEW1:
			//qDebug("Au3dsGLSLModel::LoadEditorChunk skipping EDIT_VIEW1 "
			//"subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		case EDIT_BACKGR:
			//qDebug("Au3dsGLSLModel::LoadEditorChunk skipping EDIT_BACKGR "
			//"subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		case EDIT_AMBIENT:
			//qDebug("Au3dsGLSLModel::LoadEditorChunk skipping EDIT_AMBIENT "
			//"subchunk of %d bytes", subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		default:
			//qDebug("Au3dsGLSLModel::LoadEditorChunk skipping %04x subchunk of "
			//"%d bytes", subChunkName, subChunkSize);
			in.skipRawData(subChunkSize - chunkHeaderSize);
			break;
		}
	}
	while(chunkSize > 0);
	return true;
}

quint64 Au3dsGLSLModel::MemoryUsage() const
{
	quint64 mem = sizeof(Au3dsGLSLModel);
	mem += m_Materials.MemoryUsage();
	mem += m_Objects.MemoryUsage();
	mem += m_VertexGroups.MemoryUsage();
	if(m_Screens)
		mem += m_Screens->MemoryUsage();				// screens on control consoles, can be null
	if(m_Stations)
		mem += m_Stations->MemoryUsage();				// stations in vehicles, can be null
	if(m_Weapons)
		mem += m_Weapons->MemoryUsage();				// weapons in vehicles, can be null
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

bool Au3dsGLSLModelInstance::CreateDeviceObjects(AuGLEffects& re, bool bLightsOn, double scale)
{
	// Delete this model if it was already created.
	DeleteGL();

	// Load the 3ds model.
	if(!Load(m_FileName, scale))
		return false;

	// Load succeeded. Did we get any model data?
	int modelCount = m_Objects.count();
	if(!modelCount)
		return false;

	// Strip the file path from the file name to locate associated map files.
	QFileInfo fileInfo(m_FileName);
	QString filePath = fileInfo.absolutePath();

	// Create texture(s) outside of the model display list(s).
	if(!m_Texture)
	{
		m_Texture = m_Materials.CreateDeviceObjects(filePath, re);
		if(!m_Texture && m_FileName.right(4) == ".3ds")
		{
			QString name(m_FileName);
			name.replace(".3ds",".png");
			//qDebug() << "Au3dsGLSLModelInstance::CreateDeviceObjects object texture not found, trying" << name;
			m_Texture = re.CreateTextureFromBitmap(STATIC_MODEL_ID, name, name);
			if(m_Texture)
				re.UseTexture(STATIC_MODEL_ID, m_Texture);
		}
	}

	m_Objects.CreateDeviceObjects(re, m_Models, m_Materials, false, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);
	if(bLightsOn)
	{
		// Create Gl models of the component objects.
		m_Objects.CreateDeviceObjects(re, m_ModelsLit, m_Materials, true, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);
	}

	return true;
}

void Au3dsGLSLModelInstance::QueueLoadModel(bool lightsOn, double scale)
{
	Q_ASSERT(!m_FileName.isEmpty());
	//qDebug() << "Au3dsGLSLModelInstance::QueueLoadModel" << m_FileName << "lightsOn" << lightsOn;
	m_LoaderJob = new Au3dsGLSLModelLoader(m_FileName, lightsOn, scale);
	QThreadPool::globalInstance()->start(m_LoaderJob);
}

bool Au3dsGLSLModelInstance::LoadModels(bool bLightsOn, double scale)
{
	Q_ASSERT(!m_FileName.isEmpty());
	// Load the 3ds model.
	if(!Load(m_FileName, scale))
	{
		if(s_ShowErrors)
		qWarning() << "LoadModels" << m_FileName << "NOT FOUND";
		return false;
	}

	// Load succeeded. Did we get any model data?
	int modelCount = m_Objects.count();
	if(!modelCount)
	{
		if(s_ShowErrors)
		qWarning() << "LoadModels" << m_FileName << "NO OBJECTS LOADED";
		return false;
	}

	m_Objects.LoadModels(m_IntModels, m_Materials, false, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);
	if(bLightsOn)
		m_Objects.LoadModels(m_IntModelsLit, m_Materials, true, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);

	//qDebug() << "Au3dsGLSLModelInstance::LoadModels" << m_FileName;
	return true;
}

bool Au3dsGLSLModelInstance::GLModel(AuGLEffects& re)
{
	if(!m_Models.isEmpty())
		return true;	// The model has been created.

	// Has the worker thread loaded my model data yet?
	if(m_LoaderJob)
	{
		// The worker thread still exists.
		if(!m_LoaderJob->IsDone())
			return false;	// Worker thread is not done yet.

		// Copy data from the worker thread and destroy it.
		CopyInstance(m_LoaderJob->Model());
		delete m_LoaderJob;
		m_LoaderJob = 0;
	}

	// Has the worker thread loaded my model data yet?
	int modelCount = m_Objects.count();
	if(!modelCount)
		return false;
	if(m_IntModels.isEmpty())
	{
		// This model must have been loaded by calling CreateGeometry only. We need to load the entire model.
		if(s_ShowErrors)
		qWarning() << "warning: Au3dsGLSLModelInstance::GLModel" << m_FileName << "loading models";
		m_Objects.LoadModels(m_IntModels, m_Materials, false, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);
		if(m_IntModels.isEmpty())
			return false;
		m_Objects.LoadModels(m_IntModelsLit, m_Materials, true, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Strip the file path from the file name to locate associated map files.
	QFileInfo fileInfo(m_FileName);
	QString filePath = fileInfo.absolutePath();

	// Create texture(s) outside of the model display list(s).
	if(!m_Texture)
	{
		m_Texture = m_Materials.CreateDeviceObjects(filePath, re);
		if(!m_Texture && m_FileName.right(4) == ".3ds")
		{
			QString name(m_FileName);
			name.replace(".3ds",".png");
			//qDebug() << "Au3dsGLSLModelInstance::GLModel object texture not found, trying" << name;
			m_Texture = re.CreateTextureFromBitmap(STATIC_MODEL_ID, name, name);
			if(m_Texture)
				re.UseTexture(STATIC_MODEL_ID, m_Texture);
		}
	}
	//qDebug() << "Au3dsGLSLModelInstance::GLModel" << m_Texture << m_FileName;

	// Create the primary "unlit" model.
	for(QList<Au3dsGLSLTriTexList>::const_iterator mi = m_IntModels.constBegin(); mi != m_IntModels.constEnd(); mi++)
	{
		Au3dsGLSLVAO model((*mi).m_DestBlendFunc, (*mi).m_GlowColor);
		model.CreateForModelTexFog(re, *mi, GL_STATIC_DRAW);
		m_Models.append(model);
	}
	m_IntModels.clear();	// Intermediate data is no longer needed.

	// Not all models include a lit version, with the appearance of lights on. Vehicles do this.
	if(!m_IntModelsLit.isEmpty())
	{
		// Create the "lit" model, with lights on.
		Q_ASSERT(m_IntModelsLit.count() == modelCount);	// I don't know if this always true. Trap it if it fails.
		for(QList<Au3dsGLSLTriTexList>::const_iterator mi = m_IntModelsLit.constBegin(); mi != m_IntModelsLit.constEnd(); mi++)
		{
			Au3dsGLSLVAO model((*mi).m_DestBlendFunc, (*mi).m_GlowColor);
			model.CreateForModelTexFog(re, *mi, GL_STATIC_DRAW);
			m_ModelsLit.append(model);
		}
		m_IntModelsLit.clear();	// Intermediate data is no longer needed.
	}

	return true;
}

void Au3dsGLSLModelInstance::ResourceReport(QDataStream& out) const
{
	out << m_FileName;
	out << (quint16)0;	// 0=Au3dsGLSLModelInstance
	out << (quint8)(m_Screens ? 1 : 0);
	out << (quint8)(m_Stations ? 1 : 0);
	out << (quint8)(m_Weapons ? 1 : 0);
	out << (quint16)m_Materials.count();
	out << (quint16)m_Objects.count();
	out << (quint16)m_VertexGroups.count();
}

bool Au3dsGLSLModelInstance::MergeTorsoWithInstance(Au3dsGLSLModelInstance *&pDest, double fuzz, bool bConnectOnly)
{
	bool bReturn = true;
	QVector<Au3dsGLSLVGroup>::iterator itDestBottom = 0;
	//QVector<Au3dsGLSLVGroup>::iterator itSourceTop = 0;
	if(!pDest)
	{
		pDest = new Au3dsGLSLModelInstance("");
		pDest->CopyInstance(this);
		return bReturn;
	}
	else
	{
		if(bConnectOnly)
		{
			m_Offset = pDest->m_Offset;
			m_Objects[0].m_ConnectPt = pDest->m_Objects[0].m_ConnectPt;
			for(int i=0; i<m_Objects.count(); i++)
				pDest->m_Objects.append(m_Objects[i]);
			return bReturn;
		}

		// Apply scale factor to destination model if needed
		if(!pDest->m_Objects[0].m_bScaled)
		{
			for(Au3dsGLSLVertexArray::iterator itVert = pDest->m_Objects[0].m_Vertices.begin();
				itVert != pDest->m_Objects[0].m_Vertices.end(); itVert++)
			{
				AuPoint pVert = AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z());
				pVert = pDest->m_Objects[0].ApplyScaleFactor(pVert);
				(*itVert).SetVertexCoords(pVert);
			}
			QVector<Au3dsGLSLVGroup>::iterator itDestGroup;
			for (itDestGroup = pDest->m_Objects[0].m_VertexGroups.begin();
				itDestGroup != pDest->m_Objects[0].m_VertexGroups.end(); itDestGroup++)
			{
				if((*itDestGroup).m_Type == VERT_BottomConnect || (*itDestGroup).m_Type == VERT_BottomSkin)
				{
					QMap<Au3dsGLSLVertex, double> destVertexList;
					for(QMap<Au3dsGLSLVertex, double>::iterator itVert = (*itDestGroup).m_VertexList.begin(); 
						itVert != (*itDestGroup).m_VertexList.end(); itVert++)
					{
						Au3dsGLSLVertex pVertex = itVert.key();
						double weight = itVert.value();
						AuPoint pVert = AuPoint(pVertex.X(), pVertex.Y(), pVertex.Z());
						pVert = pDest->m_Objects[0].ApplyScaleFactor(pVert);
						destVertexList.insert(Au3dsGLSLVertex(pVert.x, pVert.y, pVert.z), weight);
					}
					(*itDestGroup).m_VertexList = destVertexList;
					if((*itDestGroup).m_Type == VERT_BottomConnect)
						itDestBottom = itDestGroup;
				}
			}
			pDest->m_Objects[0].m_bScaled = true;
		}
	}

	Au3dsGLSLModelInstance *pSource = this;

	// Apply scale factor to source model vertex array and add to destination vertex array if unique
	QVector<int> indexArray;
	unordered_map<int, int> indexFaceCheck;
	unordered_map<int, int>	indexDestFace;
	int iVertPos = 0;
	for(Au3dsGLSLVertexArray::iterator itVert = pSource->m_Objects[0].m_Vertices.begin();
		itVert != pSource->m_Objects[0].m_Vertices.end(); itVert++)
	{
		// Scale
		AuPoint pVert = AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z());
		pVert = pSource->m_Objects[0].ApplyScaleFactor(pVert);

		// Determine if top connect piece
		QVector<Au3dsGLSLVGroup>::iterator itSourceGroup;
		bool bTopConnecting = false;
		bool bBottomConnecting = false;
		for(itSourceGroup = pSource->m_Objects[0].m_VertexGroups.begin();
				itSourceGroup != pSource->m_Objects[0].m_VertexGroups.end(); itSourceGroup++)
		{
			if((*itSourceGroup).m_Type == VERT_TopConnect)
			{
				//itSourceTop = itSourceGroup;
				if((*itSourceGroup).m_VertexList.find((*itVert)) != (*itSourceGroup).m_VertexList.constEnd())
				{
					bTopConnecting = true;
					break;
				}
				break;
			}
		}
		(*itVert).SetVertexCoords(pVert);
		int iIndex = 0;
		int iFaceIndex = 0;
		if(bTopConnecting)			// Source vertex was a top connecting piece
		{
			iIndex = pDest->m_Objects[0].m_Vertices.count();
			int i=0;
			for(Au3dsGLSLVertexArray::iterator itDestVert = pDest->m_Objects[0].m_Vertices.begin(); 
				itDestVert != pDest->m_Objects[0].m_Vertices.end(); itDestVert++)
			{
				AuPoint destPoint = AuPoint((*itDestVert).X(), (*itDestVert).Y(), (*itDestVert).Z());
				AuPoint sourcePoint = AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z());
				if(destPoint.Distance(sourcePoint) < fuzz)
					//if(FuzzyEqualsFactor((*itDestVert).X(), (*itVert).X(), fuzz) &&
					//	FuzzyEqualsFactor((*itDestVert).Y(), (*itVert).Y(), fuzz) &&
					//	FuzzyEqualsFactor((*itDestVert).Z(), (*itVert).Z(), fuzz))
				{
					(*itVert).SetVertexCoords(AuPoint((*itDestVert).X(), (*itDestVert).Y(), (*itDestVert).Z()));
					QVector<Au3dsGLSLVGroup>::iterator itDestGroup;
					for(itDestGroup = pDest->m_Objects[0].m_VertexGroups.begin();
							itDestGroup != pDest->m_Objects[0].m_VertexGroups.end(); itDestGroup++)
					{
						if((*itDestGroup).m_Type == VERT_BottomConnect)
						{
							itDestBottom = itDestGroup;
							if((*itDestGroup).m_VertexList.find((*itDestVert)) != (*itDestGroup).m_VertexList.constEnd())
							{
								indexFaceCheck.insert(pair<int, int>(i, i));
								bBottomConnecting = true;
								iFaceIndex = i;
								break;
							}
						}
					}
					break;
				}
				i++;
			}
			pDest->m_Objects[0].m_Vertices.append((*itVert));
			// Added new vertex to end of array so make sure we also add its mapping coordinates
			// to end of array.
			if(iVertPos < pSource->m_Objects[0].m_MappingCoords.count())
				pDest->m_Objects[0].m_MappingCoords.append(Au3dsGLSLMappingCoord((*itVert).m_MapCoordinates[0],
							(*itVert).m_MapCoordinates[1]));
			else
				if(s_ShowErrors)
				qWarning("MergeTorsoWithInstance MappingCoords caught index out of bounds");
		}
		else
		{
			if(itDestBottom != 0 && (*itDestBottom).m_VertexList.find((*itVert)) != (*itDestBottom).m_VertexList.constEnd())
				bBottomConnecting = true;
			int iCount = pDest->m_Objects[0].m_Vertices.count();
			// Not passing down fuzz value here for the Dry Exoskeleton Skin type
			// This will hopefully allow the torso ridges to work properly
			iIndex = pDest->m_Objects[0].m_Vertices.AddIfUnique((*itVert), 0, true);
			if(iIndex == iCount)
			{
				// Added new vertex to end of array so make sure we also add its mapping coordinates
				// to end of array.
				if(iVertPos < pSource->m_Objects[0].m_MappingCoords.count())
					pDest->m_Objects[0].m_MappingCoords.append(Au3dsGLSLMappingCoord((*itVert).m_MapCoordinates[0],
								(*itVert).m_MapCoordinates[1]));
				else
					if(s_ShowErrors)
					qWarning("MergeTorsoWithInstance MappingCoords caught index out of bounds");
			}
		}
		if(bTopConnecting || bBottomConnecting)
		{
			indexFaceCheck.insert(pair<int, int>(iIndex, iIndex));
			indexDestFace.insert(pair<int, int>(iIndex, iFaceIndex));
		}
		indexArray.append(iIndex);
		iVertPos++;
	}

	// Fix the source face indexes and add to the destination model
	//int iDestCount = pDest->m_Objects[0].m_Faces.count();
	unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash> faceNormals;
	for(Au3dsGLSLFaceArray::const_iterator itFace = pSource->m_Objects[0].m_Faces.constBegin();
		itFace != pSource->m_Objects[0].m_Faces.constEnd(); itFace++)
	{
		Au3dsGLSLFace destFace;
		int iIndex0, iIndex1, iIndex2;
		iIndex0 = indexArray.at((*itFace).VertexIndex(0));
		iIndex1 = indexArray.at((*itFace).VertexIndex(1));
		iIndex2 = indexArray.at((*itFace).VertexIndex(2));
		destFace.SetVertexIndex(0, indexArray.at((*itFace).VertexIndex(0)));
		destFace.SetVertexIndex(1, indexArray.at((*itFace).VertexIndex(1)));
		destFace.SetVertexIndex(2, indexArray.at((*itFace).VertexIndex(2)));
		if(HasPrecalculatedNormals())
		{
			int iCount = pDest->m_Objects[0].m_Vertices.count();
			unordered_map<int, int>::iterator itFaceNormal = indexFaceCheck.find(iIndex0);
			unordered_map<int, int>::iterator itDestNormal = indexDestFace.find(iIndex0);
			if(itFaceNormal != indexFaceCheck.end() && (itDestNormal != indexDestFace.end() && (*itDestNormal).second < iCount))
				faceNormals.insert(pair<Au3dsGLSLVertex, AuPoint>(pDest->m_Objects[0].m_Vertices[(*itDestNormal).second], (*itFace).GetNormal(0)));
			else if(itFaceNormal != indexFaceCheck.end())
				bReturn = false;
			itFaceNormal = indexFaceCheck.find(iIndex1);
			itDestNormal = indexDestFace.find(iIndex1);
			if(itFaceNormal != indexFaceCheck.end() && (itDestNormal != indexDestFace.end() && (*itDestNormal).second < iCount))
				faceNormals.insert(pair<Au3dsGLSLVertex, AuPoint>(pDest->m_Objects[0].m_Vertices[(*itDestNormal).second], (*itFace).GetNormal(1)));
			else if(itFaceNormal != indexFaceCheck.end())
				bReturn = false;
			itFaceNormal = indexFaceCheck.find(iIndex2);
			itDestNormal = indexDestFace.find(iIndex2);
			if(itFaceNormal != indexFaceCheck.end() && (itDestNormal != indexDestFace.end() && (*itDestNormal).second < iCount))
				faceNormals.insert(pair<Au3dsGLSLVertex, AuPoint>(pDest->m_Objects[0].m_Vertices[(*itDestNormal).second], (*itFace).GetNormal(2)));
			else if(itFaceNormal != indexFaceCheck.end())
				bReturn = false;
		}
		destFace.SetNormal(0, (*itFace).GetNormal(0));
		destFace.SetNormal(1, (*itFace).GetNormal(1));
		destFace.SetNormal(2, (*itFace).GetNormal(2));
		pDest->m_Objects[0].m_Faces.append(destFace);
	}

	if(HasPrecalculatedNormals())
	{
		unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash> faceDestNormals;
		int i=0;
		for(Au3dsGLSLFaceArray::iterator itFace = pDest->m_Objects[0].m_Faces.begin();
			itFace != pDest->m_Objects[0].m_Faces.end(); itFace++)
		{
			int iIndex0, iIndex1, iIndex2;
			iIndex0 = (*itFace).VertexIndex(0);
			iIndex1 = (*itFace).VertexIndex(1);
			iIndex2 = (*itFace).VertexIndex(2);
			unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash>::iterator itFaceFound;
			unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash>::iterator itDestFaceFound;
			unordered_map<int, int>::iterator itFaceNormal = indexFaceCheck.find(iIndex0);
			if(itFaceNormal != indexFaceCheck.end())
			{
				itFaceFound = faceNormals.find(pDest->m_Objects[0].m_Vertices[iIndex0]);
				if(itFaceFound != faceNormals.end())
				{
					AuPoint vn0;
					//if(i >= iDestCount)
					//{
					//	itDestFaceFound = faceDestNormals.find(pDest->m_Objects[0].m_Vertices[iIndex0]);
					//	if(itDestFaceFound != faceDestNormals.end())
					//		vn0 = itDestFaceFound.value();
					//}
					//else
					//{
					vn0 = (*itFaceFound).second;
					//faceDestNormals.insert(pDest->m_Objects[0].m_Vertices[iIndex0], (*itFace).GetNormal(0));
					//}
					(*itFace).SetNormal(0, vn0);
				}
			}
			itFaceNormal = indexFaceCheck.find(iIndex1);
			if(itFaceNormal != indexFaceCheck.end())
			{
				itFaceFound = faceNormals.find(pDest->m_Objects[0].m_Vertices[iIndex1]);
				if(itFaceFound != faceNormals.end())
				{
					AuPoint vn1 = (*itFaceFound).second;
					// 						if(i >= iDestCount)
					// 						{
					// 							itDestFaceFound = faceDestNormals.find(pDest->m_Objects[0].m_Vertices[iIndex1]);
					// 							if(itDestFaceFound != faceDestNormals.end())
					// 								vn1 = itDestFaceFound.value();
					// 						}
					// 						else
					// 						{
					//vn1 = itFaceFound.value();
					//faceDestNormals.insert(pDest->m_Objects[0].m_Vertices[iIndex1], (*itFace).GetNormal(0));
					//}
					(*itFace).SetNormal(1, vn1);
				}
			}
			itFaceNormal = indexFaceCheck.find(iIndex2);
			if(itFaceNormal != indexFaceCheck.end())
			{
				itFaceFound = faceNormals.find(pDest->m_Objects[0].m_Vertices[iIndex2]);
				if(itFaceFound != faceNormals.end())
				{
					AuPoint vn2 = (*itFaceFound).second;
					// 						if(i >= iDestCount)
					// 						{
					// 							itDestFaceFound = faceDestNormals.find(pDest->m_Objects[0].m_Vertices[iIndex2]);
					// 							if(itDestFaceFound != faceDestNormals.end())
					// 								vn2 = itDestFaceFound.value();
					// 						}
					// 						else
					// 						{
					// 							vn2 = itFaceFound.value();
					//faceDestNormals.insert(pDest->m_Objects[0].m_Vertices[iIndex2], (*itFace).GetNormal(0));
					//}
					(*itFace).SetNormal(2, vn2);
				}
			}
			i++;
		}
	}

	// Scale Source's vertex groups
	QVector<Au3dsGLSLVGroup>::iterator itSourceGroup;
	for (itSourceGroup = pSource->m_Objects[0].m_VertexGroups.begin();
		itSourceGroup != pSource->m_Objects[0].m_VertexGroups.end(); itSourceGroup++)
	{
		if((*itSourceGroup).m_Type == VERT_BottomConnect || (*itSourceGroup).m_Type == VERT_BottomSkin)
		{
			QMap<Au3dsGLSLVertex, double> destVertexList;
			for(QMap<Au3dsGLSLVertex, double>::iterator itVert = (*itSourceGroup).m_VertexList.begin(); 
				itVert != (*itSourceGroup).m_VertexList.end(); itVert++)
			{
				Au3dsGLSLVertex pVertex = itVert.key();
				double weight = itVert.value();
				AuPoint pVert = AuPoint(pVertex.X(), pVertex.Y(), pVertex.Z());
				pVert = pSource->m_Objects[0].ApplyScaleFactor(pVert);
				destVertexList.insert(Au3dsGLSLVertex(pVert.x, pVert.y, pVert.z), weight);
			}
			(*itSourceGroup).m_VertexList = destVertexList;
		}
	}

	// Copy over vertex groups
	for(Au3dsGLSLVGroupArray::iterator itDestVGroup = pDest->m_Objects[0].m_VertexGroups.begin();
		itDestVGroup != pDest->m_Objects[0].m_VertexGroups.end(); itDestVGroup++)
	{
		for(Au3dsGLSLVGroupArray::const_iterator itSourceVGroup = pSource->m_Objects[0].m_VertexGroups.constBegin();
			itSourceVGroup != pSource->m_Objects[0].m_VertexGroups.constEnd(); itSourceVGroup++)
		{
			if((*itDestVGroup).m_Type == (*itSourceVGroup).m_Type)
			{
				if((*itDestVGroup).m_Type == VERT_BottomConnect)
				{
					(*itDestVGroup).m_VertexList = (*itSourceVGroup).m_VertexList;
				}
				else
				{
					//(*itDestVGroup).m_VertexList.unite((*itSourceVGroup).m_VertexList);
					for(QMap<Au3dsGLSLVertex, double>::const_iterator itVert = (*itSourceVGroup).m_VertexList.cbegin();
							itVert != (*itSourceVGroup).m_VertexList.cend(); itVert++)
						(*itDestVGroup).m_VertexList.insert(itVert.key(), itVert.value());
				}
			}
		}
	}

	// Setup face smoothing so that all faces are part of the same smoothing group
	if(!HasPrecalculatedNormals())
	{
		pDest->m_Objects[0].m_FaceSmoothing.clear();
		for(int i=0; i<pDest->m_Objects[0].m_Faces.count(); i++)
			pDest->m_Objects[0].m_FaceSmoothing.append(1);
	}

	return bReturn;
}

bool Au3dsGLSLModelInstance::MergeHeadWithInstance(Au3dsGLSLModelInstance *&pDest, double fuzz)
{
	bool bReturn = true;
	//QVector<Au3dsGLSLVGroup>::const_iterator itDestBottom = 0;
	//QVector<Au3dsGLSLVGroup>::const_iterator itSourceTop = 0;
	if(!pDest)
	{
		pDest = new Au3dsGLSLModelInstance("");
		pDest->CopyInstance(this);
		return bReturn;
	}
	else
	{
		// Apply scale factor to destination model if needed
		if(!pDest->m_Objects[0].m_bScaled)
		{
			for(Au3dsGLSLVertexArray::iterator itVert = pDest->m_Objects[0].m_Vertices.begin();
				itVert != pDest->m_Objects[0].m_Vertices.end(); itVert++)
			{
				AuPoint pVert = AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z());
				pVert = pDest->m_Objects[0].ApplyScaleFactor(pVert);
				(*itVert).SetVertexCoords(pVert);
			}
			QVector<Au3dsGLSLVGroup>::iterator itDestGroup;
			for (itDestGroup = pDest->m_Objects[0].m_VertexGroups.begin();
				itDestGroup != pDest->m_Objects[0].m_VertexGroups.end(); itDestGroup++)
			{
				if((*itDestGroup).m_Type == VERT_BottomConnect)
				{
					//itDestBottom = itDestGroup;
					QMap<Au3dsGLSLVertex, double> destVertexList;
					for(QMap<Au3dsGLSLVertex, double>::iterator itVert = (*itDestGroup).m_VertexList.begin(); 
						itVert != (*itDestGroup).m_VertexList.end(); itVert++)
					{
						Au3dsGLSLVertex pVertex = itVert.key();
						double weight = itVert.value();
						AuPoint pVert = AuPoint(pVertex.X(), pVertex.Y(), pVertex.Z());
						pVert = pDest->m_Objects[0].ApplyScaleFactor(pVert);
						destVertexList.insert(Au3dsGLSLVertex(pVert.x, pVert.y, pVert.z), weight);
					}
					(*itDestGroup).m_VertexList = destVertexList;
					break;
				}
			}
			pDest->m_Objects[0].m_bScaled = true;
		}
	}

	Au3dsGLSLModelInstance *pSource = this;
	pSource->m_Offset = pDest->m_Offset;
	pSource->m_Objects[0].m_ConnectPt = pDest->m_Objects[0].m_ConnectPt;

	// Apply scale factor to source model vertex array and add to destination vertex array if unique
	QVector<int> indexArray;
	unordered_map<int, int> indexFaceCheck;
	unordered_map<int, int>	indexDestFace;
	for(Au3dsGLSLVertexArray::iterator itVert = pSource->m_Objects[0].m_Vertices.begin();
		itVert != pSource->m_Objects[0].m_Vertices.end(); itVert++)
	{
		// Scale
		AuPoint pVert = AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z());
		pVert = pSource->m_Objects[0].ApplyScaleFactor(pVert);

		// Determine if top connect piece
		QVector<Au3dsGLSLVGroup>::iterator itSourceGroup;
		bool bTopConnecting = false;
		bool bBottomConnecting = false;
		for(itSourceGroup = pSource->m_Objects[0].m_VertexGroups.begin();
				itSourceGroup != pSource->m_Objects[0].m_VertexGroups.end(); itSourceGroup++)
		{
			if((*itSourceGroup).m_Type == VERT_TopConnect)
			{
				//itSourceTop = itSourceGroup;
				if((*itSourceGroup).m_VertexList.find((*itVert)) != (*itSourceGroup).m_VertexList.constEnd())
				{
					bTopConnecting = true;
					break;
				}
				break;
			}
		}
		(*itVert).SetVertexCoords(pVert);
		int iIndex = 0;
		int iFaceIndex = 0;
		if(bTopConnecting)
		{
			iIndex = pDest->m_Objects[0].m_Vertices.count();
			iFaceIndex = iIndex;
			int i=0;
			for(Au3dsGLSLVertexArray::iterator itDestVert = pDest->m_Objects[0].m_Vertices.begin(); 
				itDestVert != pDest->m_Objects[0].m_Vertices.end(); itDestVert++)
			{
				if(FuzzyEqualsFactor((*itDestVert).X(), (*itVert).X(), fuzz) &&
					FuzzyEqualsFactor((*itDestVert).Y(), (*itVert).Y(), fuzz) &&
					FuzzyEqualsFactor((*itDestVert).Z(), (*itVert).Z(), fuzz))
				{
					(*itVert).SetVertexCoords(AuPoint((*itDestVert).X(), (*itDestVert).Y(), (*itDestVert).Z()));
					QVector<Au3dsGLSLVGroup>::iterator itDestGroup;
					for(itDestGroup = pDest->m_Objects[0].m_VertexGroups.begin();
							itDestGroup != pDest->m_Objects[0].m_VertexGroups.end(); itDestGroup++)
					{
						if((*itDestGroup).m_Type == VERT_BottomConnect)
						{
							bBottomConnecting = true;
							indexFaceCheck.insert(pair<int, int>(i, i));
							iFaceIndex = i;
							break;
						}
					}
					if(bBottomConnecting)
						break;
				}
				i++;
			}
			pDest->m_Objects[0].m_Vertices.append((*itVert));
		}
		else
		{
			// 			if((*itDestBottom).m_VertexList.find((*itVert)) != (*itDestBottom).m_VertexList.constEnd())
			// 				bBottomConnecting = true;
			iIndex = pDest->m_Objects[0].m_Vertices.AddIfUnique((*itVert), fuzz, true);
		}
		//if((bTopConnecting && !bBottomConnecting) || (!bTopConnecting && bBottomConnecting))
		if(bTopConnecting || bBottomConnecting)
		{
			indexFaceCheck.insert(pair<int, int>(iIndex, iIndex));
			indexDestFace.insert(pair<int, int>(iIndex, iFaceIndex));
		}
		indexArray.append(iIndex);
	}

	// Fix the source face indexes and add to the destination model
	// Also fix face material data
	for(int i=0; i<pSource->m_Materials.count(); i++)
		pDest->m_Materials.append(pSource->m_Materials.at(i));
	//int iDestCount = pDest->m_Objects[0].m_Faces.count();
	int iSourceIndex = 0;
	unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash> faceNormals;
	for(Au3dsGLSLFaceArray::const_iterator itFace = pSource->m_Objects[0].m_Faces.constBegin();
		itFace != pSource->m_Objects[0].m_Faces.constEnd(); itFace++)
	{
		Au3dsGLSLFace destFace;
		int iIndex0, iIndex1, iIndex2;
		iIndex0 = indexArray.at((*itFace).VertexIndex(0));
		iIndex1 = indexArray.at((*itFace).VertexIndex(1));
		iIndex2 = indexArray.at((*itFace).VertexIndex(2));
		destFace.SetVertexIndex(0, iIndex0);
		destFace.SetVertexIndex(1, iIndex1);
		destFace.SetVertexIndex(2, iIndex2);

		// This sets the normals of the source to the skull normals
		if(HasPrecalculatedNormals())
		{
			int iCount = pDest->m_Objects[0].m_Vertices.count();
			unordered_map<int, int>::iterator itFaceNormal = indexFaceCheck.find(iIndex0);
			unordered_map<int, int>::iterator itDestNormal = indexDestFace.find(iIndex0);
			if(itFaceNormal != indexFaceCheck.end() && (itDestNormal != indexDestFace.end() && (*itDestNormal).second < iCount))
				faceNormals.insert(pair<Au3dsGLSLVertex, AuPoint>(pDest->m_Objects[0].m_Vertices[(*itDestNormal).second], (*itFace).GetNormal(0)));
			else if(itFaceNormal != indexFaceCheck.end())
				bReturn = false;
			itFaceNormal = indexFaceCheck.find(iIndex1);
			itDestNormal = indexDestFace.find(iIndex1);
			if(itFaceNormal != indexFaceCheck.end() && (itDestNormal != indexDestFace.end() && (*itDestNormal).second < iCount))
				faceNormals.insert(pair<Au3dsGLSLVertex, AuPoint>(pDest->m_Objects[0].m_Vertices[(*itDestNormal).second], (*itFace).GetNormal(1)));
			else if(itFaceNormal != indexFaceCheck.end())
				bReturn = false;
			itFaceNormal = indexFaceCheck.find(iIndex2);
			itDestNormal = indexDestFace.find(iIndex2);
			if(itFaceNormal != indexFaceCheck.end() && (itDestNormal != indexDestFace.end() && (*itDestNormal).second < iCount))
				faceNormals.insert(pair<Au3dsGLSLVertex, AuPoint>(pDest->m_Objects[0].m_Vertices[(*itDestNormal).second], (*itFace).GetNormal(2)));
			else if(itFaceNormal != indexFaceCheck.end())
				bReturn = false;
		}
		destFace.SetNormal(0, (*itFace).GetNormal(0));
		destFace.SetNormal(1, (*itFace).GetNormal(1));
		destFace.SetNormal(2, (*itFace).GetNormal(2));
		pDest->m_Objects[0].m_Faces.append(destFace);

		// Fix Material
		for(QMap<QString, QVector<quint16> >::const_iterator mit =
			pSource->m_Objects[0].m_Materials.constBegin(); mit != pSource->m_Objects[0].m_Materials.constEnd(); mit++)
		{
			int iNewIndex = pDest->m_Objects[0].m_Faces.count();
			const QVector<quint16>& faceMaterials = *mit;
			if(faceMaterials.contains(iSourceIndex))
			{
				if(pDest->m_Objects[0].m_Materials.find(mit.key()) != pDest->m_Objects[0].m_Materials.end())
				{
					QVector<quint16> newFaceList;
					QVector<quint16> faceList = pDest->m_Objects[0].m_Materials.value(mit.key());
					for(int i=0; i<faceList.count(); i++)
						newFaceList.append(faceList.at(i));
					newFaceList.append(iNewIndex);
					pDest->m_Objects[0].m_Materials.insert(mit.key(), newFaceList);
				}
				else
				{
					QVector<quint16> faceList;
					faceList.append(iNewIndex);
					pDest->m_Objects[0].m_Materials.insert(mit.key(), faceList);
				}
			}
		}
		iSourceIndex++;
	}

	// Correct the face normals for the destination model (skull model)
	if(HasPrecalculatedNormals())
	{
		unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash> faceDestNormals;
		int i=0;
		for(Au3dsGLSLFaceArray::iterator itFace = pDest->m_Objects[0].m_Faces.begin();
			itFace != pDest->m_Objects[0].m_Faces.end(); itFace++)
		{
			int iIndex0, iIndex1, iIndex2;
			iIndex0 = (*itFace).VertexIndex(0);
			iIndex1 = (*itFace).VertexIndex(1);
			iIndex2 = (*itFace).VertexIndex(2);
			unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash>::iterator itFaceFound;
			unordered_map<Au3dsGLSLVertex, AuPoint, Au3dsGLSLVertex_Hash>::iterator itDestFaceFound;
			unordered_map<int, int>::iterator itFaceNormal = indexFaceCheck.find(iIndex0);
			if(itFaceNormal != indexFaceCheck.end())
			{
				itFaceFound = faceNormals.find(pDest->m_Objects[0].m_Vertices[iIndex0]);
				if(itFaceFound != faceNormals.end())
				{
					AuPoint vn0;
					//if(i >= iDestCount)
					//{
					//	itDestFaceFound = faceDestNormals.find(pDest->m_Objects[0].m_Vertices[iIndex0]);
					//	if(itDestFaceFound != faceDestNormals.end())
					//		vn0 = itDestFaceFound.value();
					//}
					//else
					//{
					vn0 = (*itFaceFound).second;
					//faceDestNormals.insert(pDest->m_Objects[0].m_Vertices[iIndex0], (*itFace).GetNormal(0));
					//}
					(*itFace).SetNormal(0, vn0);
				}
			}
			itFaceNormal = indexFaceCheck.find(iIndex1);
			if(itFaceNormal != indexFaceCheck.end())
			{
				itFaceFound = faceNormals.find(pDest->m_Objects[0].m_Vertices[iIndex1]);
				if(itFaceFound != faceNormals.end())
				{
					AuPoint vn1 = (*itFaceFound).second;
					//if(i >= iDestCount)
					//{
					//	itDestFaceFound = faceDestNormals.find(pDest->m_Objects[0].m_Vertices[iIndex1]);
					//	if(itDestFaceFound != faceDestNormals.end())
					//		vn1 = itDestFaceFound.value();
					//}
					//else
					//{
					//	vn1 = itFaceFound.value();
					//	faceDestNormals.insert(pDest->m_Objects[0].m_Vertices[iIndex1], (*itFace).GetNormal(0));
					//}
					(*itFace).SetNormal(1, vn1);
				}
			}
			itFaceNormal = indexFaceCheck.find(iIndex2);
			if(itFaceNormal != indexFaceCheck.end())
			{
				itFaceFound = faceNormals.find(pDest->m_Objects[0].m_Vertices[iIndex2]);
				if(itFaceFound != faceNormals.end())
				{
					AuPoint vn2 = (*itFaceFound).second;
					//if(i >= iDestCount)
					//{
					//	itDestFaceFound = faceDestNormals.find(pDest->m_Objects[0].m_Vertices[iIndex2]);
					//	if(itDestFaceFound != faceDestNormals.end())
					//		vn2 = itDestFaceFound.value();
					//}
					//else
					//{
					//	vn2 = itFaceFound.value();
					//	faceDestNormals.insert(pDest->m_Objects[0].m_Vertices[iIndex2], (*itFace).GetNormal(0));
					//}
					(*itFace).SetNormal(2, vn2);
				}
			}
			i++;
		}
	}

	// Setup face smoothing so that all faces are part of the same smoothing group
	if(!HasPrecalculatedNormals())
	{
		pDest->m_Objects[0].m_FaceSmoothing.clear();
		for(int i=0; i<pDest->m_Objects[0].m_Faces.count(); i++)
			pDest->m_Objects[0].m_FaceSmoothing.append(1);
	}

	return bReturn;
}

void Au3dsGLSLModelInstance::RenderGLSLModelTex(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
		float alpha, bool useModelBlending) const
{
	re.GLSL().ModelTexShader().Select(projection, re);
	re.GLSL().ModelTexShader().SetAmbientLight(ambientLightR, ambientLightG, ambientLightB);
	re.GLSL().ModelTexShader().SetTexture(Texture());

	RenderGLSLModelTexShader(mv, re, ambientLightUp, lightOn, alpha, useModelBlending);

	re.GLSL().ModelTexShader().Deselect();
}

void Au3dsGLSLModelInstance::RenderGLSLModelTexShader(const AuMatrix4x4& mv, AuGLEffects& re, const AuPoint& ambientLightUp,
		bool lightOn, float alpha, bool useModelBlending) const
{
	re.GLSL().ModelTexShader().SetModelView(mv);
	re.GLSL().ModelTexShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Render the model.
	if(lightOn)
	{
		if(!m_ModelsLit.isEmpty())
		{
			QColor glowAlpha;
			bool glowAlphaSet = false;
			for(QList<Au3dsGLSLVAO>::const_iterator vi = m_ModelsLit.constBegin(); vi != m_ModelsLit.constEnd(); vi++)
				if((*vi).VAO())
				{
					if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
					{
						glowAlpha = (*vi).m_GlowColor;
						glowAlphaSet = true;
						re.GLSL().ModelTexShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
								glowAlpha.alphaF()*alpha);
					}

					(*vi).SelectGL();
					if(useModelBlending)
						(*vi).DrawGLBlend(re);
					else
						(*vi).DrawGL();
					(*vi).DeselectGL();
				}
		}
		else
			if(s_ShowErrors)
			qWarning() << "Au3dsGLSLModelInstance::RenderGLSL Expected lit version of model" << m_FileName;
	}
	else
	{
		QColor glowAlpha;
		bool glowAlphaSet = false;
		for(QList<Au3dsGLSLVAO>::const_iterator vi = m_Models.constBegin(); vi != m_Models.constEnd(); vi++)
			if((*vi).VAO())
			{
				if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
				{
					glowAlpha = (*vi).m_GlowColor;
					glowAlphaSet = true;
					re.GLSL().ModelTexShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
							glowAlpha.alphaF()*alpha);
				}

				(*vi).SelectGL();
				if(useModelBlending)
					(*vi).DrawGLBlend(re);
				else
					(*vi).DrawGL();
				(*vi).DeselectGL();
			}
	}
}

void Au3dsGLSLModelInstance::RenderGLSLModelTexFog(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp,
		bool lightOn, float fogDensity, float fogR, float fogG, float fogB, float alpha, bool useModelBlending) const
{
	Q_UNUSED(projection);
	re.GLSL().ModelTexFogShader().Select(projection, re);
	re.GLSL().ModelTexFogShader().SetModelView(mv);
	re.GLSL().ModelTexFogShader().SetTexture(Texture());
	re.GLSL().ModelTexFogShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().ModelTexFogShader().SetAmbientLight(ambientLightR, ambientLightG, ambientLightB);
	re.GLSL().ModelTexFogShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Render the model.
	if(lightOn)
	{
		if(!m_ModelsLit.isEmpty())
		{
			QColor glowAlpha;
			bool glowAlphaSet = false;
			for(QList<Au3dsGLSLVAO>::const_iterator vi = m_ModelsLit.constBegin(); vi != m_ModelsLit.constEnd(); vi++)
				if((*vi).VAO())
				{
					if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
					{
						glowAlpha = (*vi).m_GlowColor;
						glowAlphaSet = true;
						re.GLSL().ModelTexFogShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
								glowAlpha.alphaF()*alpha);
					}

					(*vi).SelectGL();
					if(useModelBlending)
						(*vi).DrawGLBlend(re);
					else
						(*vi).DrawGL();
					(*vi).DeselectGL();
				}
		}
		else
			if(s_ShowErrors)
			qWarning() << "Au3dsGLSLModelInstance::RenderGLSLModelTexFog Expected lit version of model" << m_FileName;
	}
	else
	{
		QColor glowAlpha;
		bool glowAlphaSet = false;
		for(QList<Au3dsGLSLVAO>::const_iterator vi = m_Models.constBegin(); vi != m_Models.constEnd(); vi++)
			if((*vi).VAO())
			{
				if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
				{
					glowAlpha = (*vi).m_GlowColor;
					glowAlphaSet = true;
					re.GLSL().ModelTexFogShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
							glowAlpha.alphaF()*alpha);
				}

				(*vi).SelectGL();
				if(useModelBlending)
					(*vi).DrawGLBlend(re);
				else
					(*vi).DrawGL();
				(*vi).DeselectGL();
			}
	}

	re.GLSL().ModelTexFogShader().Deselect();
}

void Au3dsGLSLModelInstance::RenderGLSLModelTexFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuPoint& globeP, float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp,
		bool lightOn, float fogDensity, float fogR, float fogG, float fogB, float alpha) const
{
	Q_UNUSED(projection);
	re.GLSL().ModelTexFogGlobeShader().Select(projection, re);
	re.GLSL().ModelTexFogGlobeShader().SetModelView(mv);
	re.GLSL().ModelTexFogGlobeShader().SetTexture(Texture());
	re.GLSL().ModelTexFogGlobeShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);
	re.GLSL().ModelTexFogGlobeShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().ModelTexFogGlobeShader().SetAmbientLight(ambientLightR, ambientLightG, ambientLightB);
	re.GLSL().ModelTexFogGlobeShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Render the model.
	if(lightOn)
	{
		if(!m_ModelsLit.isEmpty())
		{
			QColor glowAlpha;
			bool glowAlphaSet = false;
			for(QList<Au3dsGLSLVAO>::const_iterator vi = m_ModelsLit.constBegin(); vi != m_ModelsLit.constEnd(); vi++)
				if((*vi).VAO())
				{
					if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
					{
						glowAlpha = (*vi).m_GlowColor;
						glowAlphaSet = true;
						re.GLSL().ModelTexFogGlobeShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
								glowAlpha.alphaF()*alpha);
					}

					(*vi).SelectGL();
					(*vi).DrawGLBlend(re);
					(*vi).DeselectGL();
				}
		}
		else
			if(s_ShowErrors)
			qWarning() << "Au3dsGLSLModelInstance::RenderGLSLModelTexFog Expected lit version of model" << m_FileName;
	}
	else
	{
		QColor glowAlpha;
		bool glowAlphaSet = false;
		for(QList<Au3dsGLSLVAO>::const_iterator vi = m_Models.constBegin(); vi != m_Models.constEnd(); vi++)
			if((*vi).VAO())
			{
				if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
				{
					glowAlpha = (*vi).m_GlowColor;
					glowAlphaSet = true;
					re.GLSL().ModelTexFogGlobeShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
							glowAlpha.alphaF()*alpha);
				}

				(*vi).SelectGL();
				(*vi).DrawGLBlend(re);
				(*vi).DeselectGL();
			}
	}

	re.GLSL().ModelTexFogGlobeShader().Deselect();
}

void Au3dsGLSLModelInstance::RenderGLSLModelTexFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		float shadowPanelSunAngles[MAX_SUNS], float ambientLightR, float ambientLightG, float ambientLightB,
		const AuPoint& ambientLightUp, bool lightOn, float fogDensity, float fogR, float fogG, float fogB, float alpha) const
{
	Q_UNUSED(projection);
	re.GLSL().ModelTexFogRingworldShader().Select(projection, re);
	re.GLSL().ModelTexFogRingworldShader().SetModelView(mv);
	re.GLSL().ModelTexFogRingworldShader().SetTexture(Texture());
	re.GLSL().ModelTexFogRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().ModelTexFogRingworldShader().SetAmbientLight(ambientLightR, ambientLightG, ambientLightB);
	re.GLSL().ModelTexFogRingworldShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().ModelTexFogRingworldShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	if(lightOn)
	{
		if(!m_ModelsLit.isEmpty())
		{
			QColor glowAlpha;
			bool glowAlphaSet = false;
			for(QList<Au3dsGLSLVAO>::const_iterator vi = m_ModelsLit.constBegin(); vi != m_ModelsLit.constEnd(); vi++)
				if((*vi).VAO())
				{
					if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
					{
						glowAlpha = (*vi).m_GlowColor;
						glowAlphaSet = true;
						re.GLSL().ModelTexFogRingworldShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
								glowAlpha.alphaF()*alpha);
					}

					(*vi).SelectGL();
					(*vi).DrawGLBlend(re);
					(*vi).DeselectGL();
				}
		}
		else
			if(s_ShowErrors)
			qWarning() << "Au3dsGLSLModelInstance::RenderGLSLModelTexFogRingworld Expected lit version of model" << m_FileName;
	}
	else
	{
		QColor glowAlpha;
		bool glowAlphaSet = false;
		for(QList<Au3dsGLSLVAO>::const_iterator vi = m_Models.constBegin(); vi != m_Models.constEnd(); vi++)
			if((*vi).VAO())
			{
				if(!glowAlphaSet || glowAlpha != (*vi).m_GlowColor)
				{
					glowAlpha = (*vi).m_GlowColor;
					glowAlphaSet = true;
					re.GLSL().ModelTexFogRingworldShader().SetGlowAlpha(glowAlpha.redF(), glowAlpha.greenF(), glowAlpha.blueF(),
							glowAlpha.alphaF()*alpha);
				}

				(*vi).SelectGL();
				(*vi).DrawGLBlend(re);
				(*vi).DeselectGL();
			}
	}

	re.GLSL().ModelTexFogRingworldShader().Deselect();
}


// Renders this model using the GLSL shader ModelTex or ModelTex fog, as appropriate based on current render states set in re.
// 'ambientLightUp' is in eye space.
void Au3dsGLSLModelInstance::RenderGLSLModel(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuPoint& ambientLightUp, bool lightOn, float alpha) const
{
	if(re.HasFog())
		RenderGLSLModelTexFog(projection, mv, re, ambientLightUp, lightOn, alpha);
	else
		RenderGLSLModelTex(projection, mv, re, ambientLightUp, lightOn, alpha);
}

// Renders this model using the GLSL shader ModelTex. Gets current ambient light from re.
// 'ambientLightUp' is in eye space.
void Au3dsGLSLModelInstance::RenderGLSLModelTex(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuPoint& ambientLightUp, bool lightOn, float alpha) const
{
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);
	RenderGLSLModelTex(projection, mv, re, ar, ag, ab, ambientLightUp, lightOn, alpha);
}

// Renders this model using the GLSL shader ModelTexFog. Gets current ambient light and fog from re.
// 'ambientLightUp' is in eye space.
void Au3dsGLSLModelInstance::RenderGLSLModelTexFog(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuPoint& ambientLightUp, bool lightOn, float alpha) const
{
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	float fr, fg, fb, fdensity;
	re.CurrentFog(fr, fg, fb, fdensity);

	RenderGLSLModelTexFog(projection, mv, re, ar, ag, ab, ambientLightUp, lightOn, fdensity, fr, fg, fb, alpha);
}

// Renders this model using the GLSL shader ModelTexFogGlobe, using current render states in re.
// 'globeP' is used for sun over the horizon effect.
// 'globeP' and 'ambientLightUp' are in eye space.
void Au3dsGLSLModelInstance::RenderGLSLModelTexFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuPoint& globeP, const AuPoint& ambientLightUp, bool lightOn, float alpha) const
{
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	float fr, fg, fb, fdensity;
	re.CurrentFog(fr, fg, fb, fdensity);

	RenderGLSLModelTexFogGlobe(projection, mv, re, globeP, ar, ag, ab, ambientLightUp, lightOn, fdensity, fr, fg, fb, alpha);
}

// Renders this model using the GLSL shader ModelTexFogRingworld, using current render states in re.
void Au3dsGLSLModelInstance::RenderGLSLModelTexFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		float shadowPanelSunAngles[MAX_SUNS], const AuPoint& ambientLightUp, bool lightOn, float alpha) const
{
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	float fr, fg, fb, fdensity;
	re.CurrentFog(fr, fg, fb, fdensity);

	RenderGLSLModelTexFogRingworld(projection, mv, re, shadowPanelSunAngles, ar, ag, ab, ambientLightUp, lightOn, fdensity, fr, fg,
			fb, alpha);
}

quint64 Au3dsGLSLModelInstance::MemoryUsage() const
{
	quint64 mem = Au3dsGLSLModel::MemoryUsage();
	mem += sizeof(Au3dsGLSLModelInstance) - sizeof(Au3dsGLSLModel);
	mem += StringMemoryUsage(m_FileName);
	mem += sizeof(Au3dsGLSLVAO) * m_Models.size();
	mem += sizeof(Au3dsGLSLVAO) * m_ModelsLit.size();
	for(QList<Au3dsGLSLTriTexList>::const_iterator ii = m_IntModels.cbegin(); ii != m_IntModels.cend(); ii++)
		mem += (*ii).MemoryUsage();
	for(QList<Au3dsGLSLTriTexList>::const_iterator ili = m_IntModelsLit.cbegin(); ili != m_IntModelsLit.cend(); ili++)
		mem += (*ili).MemoryUsage();
	return mem;
}

bool Au3dsGLSLModelInstance::Render(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
		float alpha, int pass) const
{
	// 3ds models are not split up for multiple passes
	if(pass == 2)
		return false;
	RenderGLSLModelTex(projection, mv, re, ambientLightR, ambientLightG, ambientLightB, ambientLightUp, lightOn, alpha);
	return false;
}

// 'globeP' is used for sun over the horizon effect.
// 'globeP' and 'ambientLightUp' are in eye space.
bool Au3dsGLSLModelInstance::RenderFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuPoint& globeP,
		float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
		float fogDensity, float fogR, float fogG, float fogB, float alpha, int pass) const
{
	// 3ds models are not split up for multiple passes
	if(pass == 2)
		return false;
	RenderGLSLModelTexFogGlobe(projection, mv, re, globeP, ambientLightR, ambientLightG, ambientLightB, ambientLightUp, lightOn,
			fogDensity, fogR, fogG, fogB, alpha);
	return false;
}

bool Au3dsGLSLModelInstance::RenderFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		float shadowPanelSunAngles[MAX_SUNS], float ambientLightR, float ambientLightG, float ambientLightB,
		const AuPoint& ambientLightUp, bool lightOn, float fogDensity, float fogR, float fogG, float fogB,
		float alpha, int pass) const
{
	// 3ds models are not split up for multiple passes
	if(pass == 2)
		return false;
	RenderGLSLModelTexFogRingworld(projection, mv, re, shadowPanelSunAngles, ambientLightR, ambientLightG, ambientLightB,
			ambientLightUp, lightOn, fogDensity, fogR, fogG, fogB, alpha);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int Au3dsGLSLVertexArray::AddIfUnique(Au3dsGLSLVertex pVertex, double fuzz, bool bCheckTexture)
{
	int index = count();
	int i=0;
	for(Au3dsGLSLVertexArray::const_iterator itVert = begin(); itVert != end(); itVert++)
	{
		AuPoint destPoint = AuPoint(pVertex.X(), pVertex.Y(), pVertex.Z());
		AuPoint sourcePoint = AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z());
		if(destPoint.Distance(sourcePoint) < fuzz)
			//if(FuzzyEqualsFactor((*itVert).X(), pVertex.X(), fuzz) &&
			//	FuzzyEqualsFactor((*itVert).Y(), pVertex.Y(), fuzz) &&
			//	FuzzyEqualsFactor((*itVert).Z(), pVertex.Z(), fuzz))
		{
			// Vertex exists but are the mapping coordinates the same?
			if(bCheckTexture && (*itVert).m_MapCoordinates[0] == pVertex.m_MapCoordinates[0] &&
				(*itVert).m_MapCoordinates[1] == pVertex.m_MapCoordinates[1])
			{
				return i;		// Vertex exists return index position
			}
			else
			{
				pVertex.SetVertexCoords(AuPoint((*itVert).X(), (*itVert).Y(), (*itVert).Z()));
				append(pVertex);
				return index;
			}
		}
		i++;
	}
	append(pVertex);
	return index;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//Au3dsGLSLModelDefinition

Au3dsGLSLModelDefinition::Au3dsGLSLModelDefinition(double iScale/* =1 */, double overallScale/* =1 */, double uTOffset/* =0 */,
										 double vTOffset/* =0 */, double uBOffset/* =0 */, double vBOffset/* =0 */,
										 int pb/* =0 */, int tb/* =0 */, int nb/* =0 */, const AuPoint& pOffset /* = AuPoint */,
										 double uL/* =0 */, double uT/* =0 */, double lL/* =0 */, double lT/* =0 */, double hwL/* =0 */,
										 QColor tC/* =QColor(255,255,255,0) */, float tS/* =0 */, QColor bC/* =QColor(255,255,255,0) */, float bS/* =0 */,
										 float glow/* =0 */, float glowdim/* =0 */, bool bCreate/* =false */, bool bSkip/* =false */, bool bArrangement/* =false */,
										 GLuint destBlendFunc/* =GL_ONE_MINUS_SRC_ALPHA */)
{
	m_rScale = iScale;
	m_rOverallScale = overallScale;
	m_ruTOffset = uTOffset;
	m_rvTOffset = vTOffset;
	m_ruBOffset = uBOffset;
	m_rvBOffset = vBOffset;
	m_iPrevBone = pb;
	m_iThisBone = tb;
	m_iNextBone = nb;
	m_pOffset = pOffset;
	m_ruLength = uL;
	m_ruThickness = uT;
	m_rlLength = lL;
	m_rlThickness = lT;
	m_rhwLength = hwL;
	m_fTColor = tC;
	m_fBColor = bC;
	m_fTShininess = tS;
	m_fBShininess = bS;
	m_fGlow = glow;
	m_fGlowdim = glowdim;
	m_bCreate = bCreate;
	m_bSkip = bSkip;
	m_idestBlendFunc = destBlendFunc;
	m_bArrangement = bArrangement;
}

Au3dsGLSLModelDefinition::~Au3dsGLSLModelDefinition()
{

}

void Au3dsGLSLModelDefinition::CopyDefinition(Au3dsGLSLModelDefinition* pModelDef)
{
	m_rScale = pModelDef->m_rScale;
	m_rOverallScale = pModelDef->m_rOverallScale;
	m_ruTOffset = pModelDef->m_ruTOffset;
	m_rvTOffset = pModelDef->m_rvTOffset;
	m_ruBOffset = pModelDef->m_ruBOffset;
	m_rvBOffset = pModelDef->m_rvBOffset;
	m_iPrevBone = pModelDef->m_iPrevBone;
	m_iThisBone = pModelDef->m_iThisBone;
	m_iNextBone = pModelDef->m_iNextBone;
	m_pOffset = pModelDef->m_pOffset;
	m_ruLength = pModelDef->m_ruLength;
	m_ruThickness = pModelDef->m_ruThickness;
	m_rlLength = pModelDef->m_rlLength;
	m_rlThickness = pModelDef->m_rlThickness;
	m_rhwLength = pModelDef->m_rhwLength;
	m_fTColor = pModelDef->m_fTColor;
	m_fBColor = pModelDef->m_fBColor;
	m_fTShininess = pModelDef->m_fTShininess;
	m_fBShininess = pModelDef->m_fBShininess;
	m_fGlow = pModelDef->m_fGlow;
	m_fGlowdim = pModelDef->m_fGlowdim;
	m_bCreate = pModelDef->m_bCreate;
	m_bSkip = pModelDef->m_bSkip;
	m_idestBlendFunc = pModelDef->m_idestBlendFunc;
	m_bArrangement = pModelDef->m_bArrangement;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

Au3dsGLSLModelLoader::Au3dsGLSLModelLoader(const QString& fileName, bool lightsOn, double scale) : m_Model(fileName)
{
	Q_ASSERT(scale > 0.);
	m_IsDone = false;
	m_LightsOn = lightsOn;
	m_Scale = scale;
	//qDebug() << "Au3dsGLSLModelLoader" << FileName();
	setAutoDelete(false);
}

void Au3dsGLSLModelLoader::run()
{
	//qDebug() << "Au3dsGLSLModelLoader::run" << FileName();
	// Load the 3ds model.
	m_Model.LoadModels(m_LightsOn, m_Scale);
	m_IsDone = true;
	//qDebug() << "Au3dsGLSLModelLoader::run" << FileName() << "done";
}
