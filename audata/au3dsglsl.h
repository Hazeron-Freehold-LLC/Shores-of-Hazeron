//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: au3dsglsl.h,v 1.25 2022/04/08 14:39:14 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Coded using the following "specifications".
// "3D-Studio File Format (.3ds)"
//   Autodesk Ltd., Document Revision 0.91. June 1996
//   Original by Jim Pitts. Edited by Paul Bourke.
//   http://local.wasp.uwa.edu.au/~pbourke/dataformats/3ds/
// "3D-Studio Material-Library File Format (.mli)"
//   Autodesk Ltd., Document Revision 0.25. May 1996, December 1996
//   Robin Fercoq, Martin van Velsen, James Fleming
//   http://www.martinreddy.net/gfx/3d/MLI.spec
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef Au3dsGLSL_H
#define Au3dsGLSL_H

#include "auglmodel.h"

class Au3dsGLSLModelLoader;
class AuGLEffects;
class QFile;

enum MODEL_TYPE
{
	MODEL_3ds,
	MODEL_3dsN,
	MODEL_3dsNT,
	MODEL_3dsX
};

inline bool IsSymmetricaModel(MODEL_TYPE modelType)
{
	return modelType == MODEL_3dsX;
}

inline bool HasPrecalculatedNormals(MODEL_TYPE modelType)
{
	return modelType == MODEL_3dsN || modelType == MODEL_3dsNT;
}

inline bool HasPrecalculatedTangents(MODEL_TYPE modelType)
{
	return modelType == MODEL_3dsNT;
}

#define STATIC_MODEL_ID	1	// Common id for 3ds models that must remain resident until the application exits.

//>------ Primary chunk

 #define MAIN3DS       0x4D4D

 //>------ Main Chunks

 #define EDIT3DS       0x3D3D  // this is the start of the editor config
 #define KEYF3DS       0xB000  // this is the start of the keyframer config

 //>------ sub defines of EDIT3DS

 #define EDIT_MATERIAL 0xAFFF
 #define EDIT_CONFIG1  0x0100
 #define EDIT_CONFIG2  0x3E3D
 #define EDIT_VIEW_P1  0x7012
 #define EDIT_VIEW_P2  0x7011
 #define EDIT_VIEW_P3  0x7020
 #define EDIT_VIEW1    0x7001
 #define EDIT_BACKGR   0x1200
 #define EDIT_AMBIENT  0x2100
 #define EDIT_OBJECT   0x4000

 #define EDIT_UNKNW01  0x1100
 #define EDIT_UNKNW02  0x1201
 #define EDIT_UNKNW03  0x1300
 #define EDIT_UNKNW04  0x1400
 #define EDIT_UNKNW05  0x1420
 #define EDIT_UNKNW06  0x1450
 #define EDIT_UNKNW07  0x1500
 #define EDIT_UNKNW08  0x2200
 #define EDIT_UNKNW09  0x2201
 #define EDIT_UNKNW10  0x2210
 #define EDIT_UNKNW11  0x2300
 #define EDIT_UNKNW12  0x2302 
 #define EDIT_UNKNW13  0x3000
 #define EDIT_UNKNW14  0xAFFF

 //>------ sub defines of EDIT_MATERIAL
 #define MAT_AMOUNT          0x0030	// 'amount of' subchunk (short int)
 #define MAT_NAME01          0xa000	// material name (asciiz)
 #define MAT_AMBIENTCOLOR    0xa010	// RGB1 and RGB2 (subchunks)
 #define MAT_DIFFUSECOLOR    0xa020	// RGB1 and RGB2 (subchunks)
 #define MAT_SPECULARCOLOR   0xa030	// RGB1 and RGB2 (subchunks)
 #define MAT_SHININESS       0xa040	// amount of (subchunks)
 #define MAT_SHINSTRENGTH    0xa041	// amount of (subchunks)
 #define MAT_TRANSPARENCY    0xa050	// amount of (subchunks)
 #define MAT_TRANSFALLOFF    0xa052	// amount of (subchunks)
 #define MAT_REFLECTBLUR     0xa053	// amount of (subchunks)
 #define MAT_SELFILLUM_BOOL  0xa080	// mere presense indicates self illum true
 #define MAT_TWO_SIDED       0xa081
 #define MAT_TRANS_ADD       0xa083
 #define MAT_SELFILLUM       0xa084	// amount of (subchunks)
 #define MAT_WIRE_ON         0xa085
 #define MAT_WIRETHICKNESS   0xa087
 #define MAT_FACE_MAP        0xa088
 #define MAT_TRANS_FALLOF_IN 0xa08a
 #define MAT_SOFTEN          0xa08c
 #define MAT_3DWIRETHICKNESS 0xa08e
 #define MAT_TYPE            0xa100	// 1=flat, 2=gour, 3=phong 4=metal (sh int)
 #define MAT_TEXTURE1_MAP    0xa200
 #define MAT_SPECULAR_MAP    0xa204
 #define MAT_OPACITY_MAP     0xa210
 #define MAT_REFLECTION_MAP  0xa220
 #define MAT_BUMP_MAP        0xa230
 #define MAT_TRANS_FALLOFF   0xa240
 #define MAT_REFLECT_BLUR    0xa250
 #define MAT_TEXTURE2_MAP    0xa33a
 #define MAT_SHININESS_MAP   0xa33c
 #define MAT_SELFILLUM_MAP   0xa33d
 #define MAT_TEXTURE1_MASK   0xa33e
 #define MAT_TEXTURE2_MASK   0xa340
 #define MAT_OPACITY_MASK    0xa342
 #define MAT_BUMP_MASK       0xa344
 #define MAT_SHININESS_MASK  0xa346
 #define MAT_SPECULAR_MASK   0xa348
 #define MAT_SELFILLUM_MASK  0xa34a
 #define MAT_REFLECTION_MASK 0xa34c

 //>------ sub defines of MAT_TEXTURE
 #define TEX_FILENAME        0xa300
 #define TEX_AMOUNT          0x0030	// 'amount of' subchunk (short int)
 #define TEX_MAP_OPTION      0xa351
 #define TEX_MAP_FILTBLUR    0xa353
 #define TEX_MAP_USCALE      0xa354
 #define TEX_MAP_VSCALE      0xa356
 #define TEX_MAP_UOFFSET     0xa358
 #define TEX_MAP_VOFFSET     0xa35a
 #define TEX_MAP_ROTATION    0xa35c
 #define TEX_MAP_LUMTINT1    0xa360
 #define TEX_MAP_LUMTINT2    0xa362
 #define TEX_MAP_RGBTINTR    0xa364
 #define TEX_MAP_RGBTINTG    0xa366
 #define TEX_MAP_RGBTINTB    0xa368

 //>------ sub defines of EDIT_OBJECT
 #define OBJ_TRIMESH   0x4100
 #define OBJ_LIGHT     0x4600
 #define OBJ_CAMERA    0x4700

 #define OBJ_UNKNWN01  0x4010
 #define OBJ_UNKNWN02  0x4012 //>>---- Could be shadow

 //>------ sub defines of OBJ_CAMERA
 #define CAM_UNKNWN01  0x4710 
 #define CAM_UNKNWN02  0x4720 

 //>------ sub defines of OBJ_LIGHT
 #define LIT_OFF       0x4620
 #define LIT_SPOT      0x4610
 #define LIT_UNKNWN01  0x465A

 //>------ sub defines of OBJ_TRIMESH
 #define TRI_VERTEXL			0x4110
 #define TRI_VERTEXOPTIONS		0x4111
	//>------ Special vertex list options for Hazeron models
	#define TRI_VERTEXGROUPS	0x4112	// Two kinds - Regular and 'ScalePt'
		//>------ Sub defines of TRI_VERTEXGROUPS
		#define VERT_GROUPNAME		0x4113	// vertex group name
		#define VERT_LIST			0x4114	// vertex list with weights
	//>------ End Special vertex list options
 #define TRI_FACELIST			0x4120
 #define TRI_MAPPINGCOORDS		0x4140
 #define TRI_MATRIX				0x4160
 #define TRI_VISIBLE			0x4165
 #define TRI_STANDARDMAPPING	0x4170

 //>------ sub defines of TRI_FACELIST
 #define FACE_MATERIAL  0x4130
 #define FACE_SMOOTH    0x4150

 //>>------ sub defs of KEYF3DS

 #define KEYF_UNKNWN01 0xB009
 #define KEYF_UNKNWN02 0xB00A
 #define KEYF_FRAMES   0xB008
 #define KEYF_OBJDES   0xB002

 //>>------  these define the different color chunk types
 #define COL_RGBF      0x0010
 #define COL_RGB1      0x0011
 #define COL_RGB2      0x0012
 #define COL_UNKNOWN   0x0013

 //>>------ defines for viewport chunks

 #define TOP           0x0001
 #define BOTTOM        0x0002
 #define LEFT          0x0003
 #define RIGHT         0x0004
 #define FRONT         0x0005
 #define BACK          0x0006
 #define USER          0x0007
 #define CAMERA        0x0008 // 0xFFFF is the actual code read from file
 #define LIGHT         0x0009
 #define DISABLED      0x0010
 #define BOGUS         0x0011

enum VERT_GROUP
{
	VERT_Invalid,
	VERT_Weight,
	VERT_ScalePt,
	VERT_ConnectPt,
	VERT_MidPt,
	VERT_Top,
	VERT_Bottom,
	VERT_BottomSkin,
	VERT_TopConnect,
	VERT_BottomConnect,
	VERT_HandWingPt,
	VERT_EyePt,
	VERT_TopScalePt,
	VERT_MidScalePt,
	VERT_BottomScalePt,
	VERT_SeatLoc,
	VERT_OffLoad,
	VERT_WeaponLoc,
	VERT_DirLoc,
	VERT_WeaponUpDownPt,
	VERT_WeaponLeftRightPt,
	VERT_Screen1,				// Large primary display screen on control consoles.
	VERT_Screen2,				// Small secondary display screen on control consoles.
	VERT_BldgFlagBottom,		// Point at bottom of flag track.
	VERT_BldgFlagTop,			// Point at top of flag track.
	VERT_BldgMissileControl,	// Quad to mark location of fire control console.
	VERT_BldgMissileDeploy,		// Quad to mark point of fire for weapon.
	VERT_BldgSeat1,				// Seat point for turret 1.
	VERT_BldgSeat2,				// Seat point for turret 2.
	VERT_BldgSeat3,				// Seat point for turret 3.
	VERT_BldgStand1,			// Stand point when exiting turret 1.
	VERT_BldgStand2,			// Stand point when exiting turret 2.
	VERT_BldgStand3,			// Stand point when exiting turret 3.
	VERT_BldgStand4,			// Stand point when operating turret 4.
	VERT_BldgTurret1,			// Quad on wall. Normal points out. Gun pivots at center of quad.
	VERT_BldgTurret2,			// Quad on wall. Normal points out. Gun pivots at center of quad.
	VERT_BldgTurret3,			// Quad on wall. Normal points out. Gun pivots at center of quad.
	VERT_BldgTurret4,			// Quad on wall. Normal points out. Gun pivots at center of quad.
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class Au3dsGLSLVAO : public AuGLVAO
{
public:
	GLuint								m_DestBlendFunc;		// Destination blend function used when transparent.
	QColor								m_GlowColor;			// Alpha is transparency alpha for this object.

public:
	Au3dsGLSLVAO(GLuint destBlendFunc, const QColor& glowColorAlpha)
	{
		m_DestBlendFunc = destBlendFunc;
		m_GlowColor = glowColorAlpha;
	}

	Au3dsGLSLVAO()
	{
		m_DestBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
		m_GlowColor = QColor(0,0,0,255);
	}

	inline void DrawGLBlend(AuOpenGLFunctions& GLf) const
	{
		if(m_GlowColor.alphaF() < 1.f)
		{
			GLf.glEnable(GL_BLEND);
			GLf.glBlendFunc(GL_SRC_ALPHA, m_DestBlendFunc);
			DrawGL();
			GLf.glDisable(GL_BLEND);
		}
		else
			DrawGL();
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLVAO);
	}

	inline void SetGlowColor(float r, float g, float b)
	{
		m_GlowColor.setRedF(r);
		m_GlowColor.setGreenF(g);
		m_GlowColor.setBlueF(b);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class Au3dsGLSLTriTexList : public QList<AuTriTexNS>
{
public:
	GLuint								m_DestBlendFunc;		// Destination blend function used when transparent.
	QColor								m_GlowColor;			// Alpha is transparency alpha for this object.

public:
	inline Au3dsGLSLTriTexList()
	{
		m_GlowColor = QColor(0,0,0,255);
		m_DestBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLTriTexList) + sizeof(AuTriTexNS) * size();
	}

	inline void SetGlowColor(float r, float g, float b)
	{
		m_GlowColor.setRedF(r);
		m_GlowColor.setGreenF(g);
		m_GlowColor.setBlueF(b);
	}

	inline void SetTransAlpha(float a)
	{
		m_GlowColor.setAlphaF(a);
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class Au3dsGLSLTriBodyList : public QList<AuTriBody>
{
public:
	GLuint								m_DestBlendFunc;		// Destination blend function used when transparent.
	QColor								m_GlowColor;			// Alpha is transparency alpha for this object.

public:
	inline Au3dsGLSLTriBodyList()
	{
		m_GlowColor = QColor(0,0,0,255);
		m_DestBlendFunc = GL_ONE_MINUS_SRC_ALPHA;
	}

	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLTriBodyList);
		mem += sizeof(AuTriBody) * size();
		return mem;
	}

	inline void SetGlowColor(float r, float g, float b)
	{
		m_GlowColor.setRedF(r);
		m_GlowColor.setGreenF(g);
		m_GlowColor.setBlueF(b);
	}

	inline void SetTransAlpha(float a)
	{
		m_GlowColor.setAlphaF(a);
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Commonly used 3ds functions.

// This union is used by LoadFloat.
union U_f32
{
	quint32		ui32;
	float		f;
};

// Note: I tried to use a namespace here instead of a class but could not
// eliminate compiler warnings when s3ds.h was included from sgleffects.h.
// namespace Au3dsGLSL
class Au3dsGLSL
{
public:
	static inline float LoadFloat(QDataStream& in)
	{
		quint8 b0, b1, b2, b3;
		in >> b0;
		in >> b1;
		in >> b2;
		in >> b3;
		// This code generates a compiler warning in the release build.
		//quint32 ui32 = (b3<<24)|(b2<<16)|(b1<<8)|b0;
		//float f = *((float*)&ui32);
		//return f;
		// This code does not generate a compiler warning and should produce the same result.
		U_f32 uf;
		uf.ui32 = (b3<<24)|(b2<<16)|(b1<<8)|b0;
		return uf.f;
	}

	static inline QString LoadString(QDataStream& in)
	{
		QString str;
		quint8 ch;
		do
		{
			in >> ch;
			if(ch)
				str.append(QChar(ch));
		}
		while(ch);
		return str;
	}

	static inline quint16 LoadUint16(QDataStream& in)
	{
		quint8 lsb, msb;
		in >> lsb;
		in >> msb;
		quint16 ui16 = (msb<<8)|lsb;
		return ui16;
	}

	static inline quint32 LoadUint32(QDataStream& in)
	{
		quint8 lsb, msb;
		in >> lsb;
		in >> msb;
		quint16 lsw = (msb<<8)|lsb;
		in >> lsb;
		in >> msb;
		quint16 msw = (msb<<8)|lsb;
		quint32 ui32 = (msw<<16)|lsw;
		return ui32;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Texture mapping coordinate.

class Au3dsGLSLMappingCoord
{
public:
	inline Au3dsGLSLMappingCoord(float s, float t)
	{
		m_Coordinate[0] = s;
		m_Coordinate[1] = t;
	}
	inline Au3dsGLSLMappingCoord()
	{
		m_Coordinate[0] = 0;
		m_Coordinate[1] = 0;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLMappingCoord);
	}

	inline float s() const
	{
		return m_Coordinate[0];
	}

	inline float t() const
	{
		return m_Coordinate[1];
	}

private:
	float		m_Coordinate[2];
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Texture mapping coordinates array.

class Au3dsGLSLMappingCoordArray : public QVector<Au3dsGLSLMappingCoord>
{
public:
	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLMappingCoordArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// 3d vertex

class Au3dsGLSLVertex
{
public:
	inline Au3dsGLSLVertex(float x, float y, float z)
	{
		m_Coordinate[0] = x;
		m_Coordinate[1] = y;
		m_Coordinate[2] = z;
		m_MapCoordinates[0] = 0;
		m_MapCoordinates[1] = 0;
		for(int i=0; i<4; i++)
		{
			m_wt[i] = 0;
			m_idx[i] = 0;
		}
	}

	inline Au3dsGLSLVertex()
	{
		m_Coordinate[0] = 0;
		m_Coordinate[1] = 0;
		m_Coordinate[2] = 0;
		m_MapCoordinates[0] = 0;
		m_MapCoordinates[1] = 0;
		for(int i=0; i<4; i++)
		{
			m_wt[i] = 0;
			m_idx[i] = 0;
		}
	}

	inline Au3dsGLSLVertex(const Au3dsGLSLVertex& c)
	{
		for(int i=0; i<4; i++)
		{
			m_idx[i] = c.m_idx[i];
			m_wt[i] = c.m_wt[i];
		}

		for(int i=0; i<2; i++)
			m_MapCoordinates[i] = c.m_MapCoordinates[i];

		for(int i=0; i<3; i++)
			m_Coordinate[i] = c.m_Coordinate[i];
	}

	inline const Au3dsGLSLVertex& operator=(const Au3dsGLSLVertex& c)
	{
		for(int i=0; i<4; i++)
		{
			m_idx[i] = c.m_idx[i];
			m_wt[i] = c.m_wt[i];
		}

		for(int i=0; i<2; i++)
			m_MapCoordinates[i] = c.m_MapCoordinates[i];

		for(int i=0; i<3; i++)
			m_Coordinate[i] = c.m_Coordinate[i];

		return *this;
	}

	inline AuPoint Coord() const
	{
		return AuPoint(m_Coordinate[0], m_Coordinate[1], m_Coordinate[2]);
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLVertex);
	}

	inline float X() const
	{
		return m_Coordinate[0];
	}

	inline float Y() const
	{
		return m_Coordinate[1];
	}

	inline float Z() const
	{
		return m_Coordinate[2];
	}

	inline bool operator<(const Au3dsGLSLVertex &p) const
	{
		if(X() != p.X())
		{
			if(X() < p.X())
				return true;
			else if(X() > p.X())
				return false;
		}
		else if(Y() != p.Y())
		{
			if(Y() < p.Y())
				return true;
			else if(Y() > p.Y())
				return false;
		}
		else if(Z() != p.Z())
		{
			if(Z() < p.Z())
				return true;
			else if(Z() > p.Z())
				return false;
		}
		return false;
	}

	inline bool operator==(const Au3dsGLSLVertex &p) const
		{return ((X() == p.X()) && (Y() == p.Y()) && (Z() == p.Z()));}

	inline bool operator!=(const Au3dsGLSLVertex &p) const
		{return ((X() != p.X()) || (Y()  != p.Y() ) || (Z() != p.Z()));}

	void Project(const AuMatrix4x4& m)
	{
		AuPoint coord = m.Project(X(), Y(), Z());
		SetVertexCoords(coord);
	}

	inline void SetMapCoordinates(float u, float v)
	{
		m_MapCoordinates[0] = u;
		m_MapCoordinates[1] = v;
	}

	inline void ScaleBy(double scale)
	{
		m_Coordinate[0] *= scale;
		m_Coordinate[1] *= scale;
		m_Coordinate[2] *= scale;
	}

	inline void SetVertexCoords(const AuPoint& pLoc)
	{
		m_Coordinate[0] = pLoc.x;
		m_Coordinate[1] = pLoc.y;
		m_Coordinate[2] = pLoc.z;
	}

	inline float U() const
	{
		return m_MapCoordinates[0];
	}

	inline float V() const
	{
		return m_MapCoordinates[1];
	}

public:
	GLint		m_idx[4];				// indices of bones 0=prev, 1=this, 2=next
	GLfloat		m_wt[4];				// weight of bones affecting this vertex, must add up to 1.
	float		m_MapCoordinates[2];

private:
	float		m_Coordinate[3];
};

inline uint qHash(const Au3dsGLSLVertex& key)
{
	return uint(key.X()+key.Y()+key.Z()) ^ uint(key.m_MapCoordinates[0]+key.m_MapCoordinates[1]);
}

class Au3dsGLSLVertex_Hash
{
public:
	inline size_t operator()(const Au3dsGLSLVertex& o) const
	{
		return hash<unsigned int>()(qHash(o));
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// 3d vertex array

class Au3dsGLSLVertexArray : public QVector<Au3dsGLSLVertex>
{
public:
	inline Au3dsGLSLVertexArray()
	{
	}

	inline Au3dsGLSLVertexArray(const Au3dsGLSLVertexArray& c) : QVector<Au3dsGLSLVertex>(c)
	{
	}

public:	// Operators
	inline const Au3dsGLSLVertexArray& operator=(const Au3dsGLSLVertexArray& c)
	{
		QVector<Au3dsGLSLVertex>::operator=(c);
		return *this;
	}

public:	// Methods
	inline void Project(const AuMatrix4x4& m)
	{
		for(iterator it = begin(); it != end(); it++)
			(*it).Project(m);
	}

	int AddIfUnique(Au3dsGLSLVertex pVertex, double fuzz, bool bCheckTexture);

	inline void ScaleBy(double scale)
	{
		for(iterator it = begin(); it != end(); it++)
			(*it).ScaleBy(scale);
	}

	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLVertexArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A 3d face is a set of 3 indices into a vertices array.

class Au3dsGLSLFace
{
public:
	inline Au3dsGLSLFace(quint16 indexA, quint16 indexB, quint16 indexC,
			quint16 info, const AuPoint& v0, const AuPoint& v1, const AuPoint& v2,
			const AuPoint& vt0, const AuPoint& vt1, const AuPoint& vt2, float UVhandedness)
	{
		m_Index[0] = indexA;
		m_Index[1] = indexB;
		m_Index[2] = indexC;
		m_InfoBits = info;
		m_normals[0] = v0;
		m_normals[1] = v1;
		m_normals[2] = v2;
		m_Tangents[0] = vt0;
		m_Tangents[1] = vt1;
		m_Tangents[2] = vt2;
		m_sharedFaces[0] = 0;
		m_sharedFaces[1] = 0;
		m_sharedFaces[2] = 0;
		m_UVhandedness = UVhandedness;
	}
	inline Au3dsGLSLFace()
	{
		m_Index[0] = 0;
		m_Index[1] = 0;
		m_Index[2] = 0;
		m_InfoBits = 0;
		m_normals[0] = AuPoint(0,0,0);
		m_normals[1] = AuPoint(0,0,0);
		m_normals[2] = AuPoint(0,0,0);
		m_sharedFaces[0] = 0;
		m_sharedFaces[1] = 0;
		m_sharedFaces[2] = 0;
		m_Tangents[0] = AuPoint(0,0,0);
		m_Tangents[1] = AuPoint(0,0,0);
		m_Tangents[2] = AuPoint(0,0,0);
		m_UVhandedness = 1;
	}

	inline int UsesVertex(bool bUseHash, const quint16 index, const Au3dsGLSLVertex &vertex,
			const unordered_multimap<Au3dsGLSLVertex, int, Au3dsGLSLVertex_Hash> &hash) const
	{
		if(bUseHash)
		{
			unordered_multimap<Au3dsGLSLVertex, int, Au3dsGLSLVertex_Hash>::const_iterator it = hash.find(vertex);
			while(it != hash.cend() && (*it).first == vertex)
			{
				if((*it).second == m_Index[0])
					return 0;
				if((*it).second == m_Index[1])
					return 1;
				if((*it).second == m_Index[2])
					return 2;
				it++;
			}
		}
		else
		{
			if(m_Index[0] == index)
				return 0;
			if(m_Index[1] == index)
				return 1;
			if(m_Index[2] == index)
				return 2;
		}
		return -1;
	}

	inline const Au3dsGLSLVertex& Vertex(unsigned int index,
			const Au3dsGLSLVertexArray& vertices) const
	{
		Q_ASSERT(index <= 2);
		return vertices[m_Index[index]];
	}

	inline quint16 VertexIndex(unsigned int index) const
	{
		Q_ASSERT(index <= 2);
		return m_Index[index];
	}

	inline void SetVertexIndex(unsigned int index, unsigned int vertexIndex)
	{
		Q_ASSERT(index <= 2);
		m_Index[index] = vertexIndex;
	}

	inline void SetNormal(unsigned int index, const AuPoint& pt)
	{
		Q_ASSERT(index <= 2);
		m_normals[index] = pt;
	}

	inline const AuPoint& GetNormal(unsigned int index) const
	{
		Q_ASSERT(index <= 2);
		return m_normals[index];
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLFace);
	}

private:
	quint16		m_Index[3];		// The index of the vertex in the m_Vertices array
	quint16		m_InfoBits;		// edge-line visibility bits 0-2
public:
	// Pre-calculated values stored in the .3dsN file from the modified blender export
	AuPoint		m_Tangents[3];		// Array of tangent vectors in same order as m_Index
	AuPoint		m_normals[3];		// Array of vertex normals in same order as m_Index
	float		m_UVhandedness;		// UV handedness of face

	// Calculated value
	int			m_sharedFaces[3];	// Parallel to the normals array
	
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// 3d face array.

class Au3dsGLSLFaceArray : public QVector<Au3dsGLSLFace>
{
public:
	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLFaceArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// VertexGroup definition.

class Au3dsGLSLVGroup
{
public:
	inline Au3dsGLSLVGroup()
	{
		m_Type = VERT_Weight;
		m_Name = "";
		m_ScalePt = Au3dsGLSLVertex(0,0,0);
		m_scaleFactor = 1;
		m_bTorsoSegment = false;
		m_bHand = false;
		m_bFoot = false;
		m_bNeck = false;
		m_bTail = false;
		m_bHead = false;
		m_eyePos = Au3dsGLSLVertex(0, 0, 0);
		m_VertexList.clear();
	}

	inline Au3dsGLSLVGroup(const Au3dsGLSLVGroup& c) : m_VertexList(c.m_VertexList)
	{
		m_ScalePt = c.m_ScalePt;
		m_Type = c.m_Type;
		m_Name = c.m_Name;
		m_scaleFactor = c.m_scaleFactor;
		m_bTorsoSegment = c.m_bTorsoSegment;
		m_bHand = c.m_bHand;
		m_bFoot = c.m_bFoot;
		m_bNeck = c.m_bNeck;
		m_bTail = c.m_bTail;
		m_bHead = c.m_bHead;
		m_eyePos = c.m_eyePos;
	}

	inline const Au3dsGLSLVGroup& operator=(const Au3dsGLSLVGroup& c)
	{
		m_ScalePt = c.m_ScalePt;
		m_Type = c.m_Type;
		m_Name = c.m_Name;
		m_VertexList = c.m_VertexList;
		m_scaleFactor = c.m_scaleFactor;
		m_bTorsoSegment = c.m_bTorsoSegment;
		m_bHand = c.m_bHand;
		m_bFoot = c.m_bFoot;
		m_bNeck = c.m_bNeck;
		m_bTail = c.m_bTail;
		m_bHead = c.m_bHead;
		m_eyePos = c.m_eyePos;
		return *this;
	}

	bool LoadChunk(QDataStream& in, int chunkSize);
	bool LoadVertexGroupChunk(QDataStream& in, int chunkSize);
	inline bool HasScalePt(VERT_GROUP type)
	{
		if(type == VERT_ScalePt || type == VERT_ConnectPt || type == VERT_MidPt ||
			type == VERT_Top || type == VERT_Bottom || type == VERT_HandWingPt ||
			type == VERT_EyePt)
			return true;
		return false;
	}

	quint64 MemoryUsage() const;

public:
	Au3dsGLSLVertex					m_ScalePt;	// Scaling point for body parts. Used for other purposes if HasScalePt returns false.
	// Group types	0-Weight,	1-ScalePt,		2-ConnectPt,	3-MidPt,		4-Top,
	//				5-Bottom,	6-BottomSkin    7-TopConnect,	8-BottomConnect,9-HandPt
	// Note: Any vertex not in BottomSkin group is considered TopSkin
	VERT_GROUP					m_Type;
	QString						m_Name;			// Could be a bone or 'ScalePt'
	QMap<Au3dsGLSLVertex, double>	m_VertexList;	// Vertex Coordinates, weight
	double						m_scaleFactor;
	bool						m_bTorsoSegment;
	bool						m_bHand;
	bool						m_bFoot;
	bool						m_bNeck;
	bool						m_bTail;
	bool						m_bHead;
	Au3dsGLSLVertex					m_eyePos;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Au3dsGLSLVGroupArray definitions array.

class Au3dsGLSLVGroupArray : public QVector<Au3dsGLSLVGroup>
{
public:
	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLVGroupArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Material definition.

class Au3dsGLSLMaterial
{
public:
	inline Au3dsGLSLMaterial()
	{
		m_ReflectBlur = 0;
		m_SelfIllumination = 0;
		m_ShineStrength = 0;
		m_Shininess = 0;
		m_TexMapFilterBlur = 0;
		m_TexMapOption = 0;
		m_TexMapOffsetU = 0;
		m_TexMapOffsetV = 0;
		m_TexMapRotation = 0;
		m_TexMapScaleU = 1;
		m_TexMapScaleV = 1;
		m_TextureAmount = 0;
		m_Transparency = 1;
		m_TwoSided = false;
		m_Type = 0; 
		m_Name = "";
		m_TextureFile = "";
		for(int ic=0; ic<3; ic++)
		{
			m_AmbientColor[ic] = QColor(255,255,255);
			m_DiffuseColor[ic] = QColor(255,255,255);
			m_SpecularColor[ic] = QColor(255,255,255);
		}
	}

	inline Au3dsGLSLMaterial(const Au3dsGLSLMaterial& c)
	{
		m_ReflectBlur = c.m_ReflectBlur;
		m_AmbientColor[0] = c.m_AmbientColor[0];
		m_AmbientColor[1] = c.m_AmbientColor[1];
		m_AmbientColor[2] = c.m_AmbientColor[2];
		m_DiffuseColor[0] = c.m_DiffuseColor[0];
		m_DiffuseColor[1] = c.m_DiffuseColor[1];
		m_DiffuseColor[2] = c.m_DiffuseColor[2];
		m_Name = c.m_Name;
		m_SelfIllumination = c.m_SelfIllumination;
		m_ShineStrength = c.m_ShineStrength;
		m_Shininess = c.m_Shininess;
		m_SpecularColor[0] = c.m_SpecularColor[0];
		m_SpecularColor[1] = c.m_SpecularColor[1];
		m_SpecularColor[2] = c.m_SpecularColor[2];
		m_TexMapFilterBlur = c.m_TexMapFilterBlur;
		m_TexMapOption = c.m_TexMapOption;
		m_TexMapOffsetU = c.m_TexMapOffsetU;
		m_TexMapOffsetV = c.m_TexMapOffsetV;
		m_TexMapRotation = c.m_TexMapRotation;
		m_TexMapScaleU = c.m_TexMapScaleU;		// 1/U scale
		m_TexMapScaleV = c.m_TexMapScaleV;		// 1/V scale
		m_TextureAmount = c.m_TextureAmount;
		m_TextureFile = c.m_TextureFile;
		m_Transparency = c.m_Transparency;
		m_TwoSided = c.m_TwoSided;
		m_Type = c.m_Type;				// 1=flat, 2=gour, 3=phong, 4=metal
	}

public:		// Operators
	inline const Au3dsGLSLMaterial& operator=(const Au3dsGLSLMaterial& c)
	{
		m_ReflectBlur = c.m_ReflectBlur;
		m_AmbientColor[0] = c.m_AmbientColor[0];
		m_AmbientColor[1] = c.m_AmbientColor[1];
		m_AmbientColor[2] = c.m_AmbientColor[2];
		m_DiffuseColor[0] = c.m_DiffuseColor[0];
		m_DiffuseColor[1] = c.m_DiffuseColor[1];
		m_DiffuseColor[2] = c.m_DiffuseColor[2];
		m_Name = c.m_Name;
		m_SelfIllumination = c.m_SelfIllumination;
		m_ShineStrength = c.m_ShineStrength;
		m_Shininess = c.m_Shininess;
		m_SpecularColor[0] = c.m_SpecularColor[0];
		m_SpecularColor[1] = c.m_SpecularColor[1];
		m_SpecularColor[2] = c.m_SpecularColor[2];
		m_TexMapFilterBlur = c.m_TexMapFilterBlur;
		m_TexMapOption = c.m_TexMapOption;
		m_TexMapOffsetU = c.m_TexMapOffsetU;
		m_TexMapOffsetV = c.m_TexMapOffsetV;
		m_TexMapRotation = c.m_TexMapRotation;
		m_TexMapScaleU = c.m_TexMapScaleU;		// 1/U scale
		m_TexMapScaleV = c.m_TexMapScaleV;		// 1/V scale
		m_TextureAmount = c.m_TextureAmount;
		m_TextureFile = c.m_TextureFile;
		m_Transparency = c.m_Transparency;
		m_TwoSided = c.m_TwoSided;
		m_Type = c.m_Type;				// 1=flat, 2=gour, 3=phong, 4=metal
		return *this;
	}

	inline const QColor& AmbientColor(int which) const
	{
		return m_AmbientColor[which];
	}

	GLuint CreateDeviceObject(const QString& filePath, AuGLEffects& re);	// returns texture id or 0 if none

	const QColor& DiffuseColor(int which) const
	{
		return m_AmbientColor[which];
	}

	void Dump() const;

	inline bool IsTwoSided() const
	{
		return m_TwoSided;
	}

	inline bool IsValid() const
	{
		return !m_Name.isEmpty();
	}

	bool LoadChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType);

	quint64 MemoryUsage() const;

	const QString& Name() const
	{
		return m_Name;
	}

	float SelfIllumination() const
	{
		return m_SelfIllumination;
	}

	float Shininess() const
	{
		return m_Shininess;
	}

	const QColor& SpecularColor(int which) const
	{
		return m_AmbientColor[which];
	}

	inline float Transparency() const
	{
		return m_Transparency;
	}

private:
	float LoadAmountChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType);

	bool LoadColorChunk(QDataStream& in, int chunkSize, QColor color[3]);

	bool LoadTextureChunk(QDataStream& in, int chunkSize);

private:
	qint16		m_ReflectBlur;
	QColor		m_AmbientColor[3];
	QColor		m_DiffuseColor[3];
	QString		m_Name;
	float		m_SelfIllumination;
	qint16		m_ShineStrength;
	float		m_Shininess;
	QColor		m_SpecularColor[3];
	float		m_TexMapFilterBlur;
	qint16		m_TexMapOption;
	float		m_TexMapOffsetU;
	float		m_TexMapOffsetV;
	float		m_TexMapRotation;
	float		m_TexMapScaleU;		// 1/U scale
	float		m_TexMapScaleV;		// 1/V scale
	qint16		m_TextureAmount;
	QString		m_TextureFile;
	float		m_Transparency;
	bool		m_TwoSided;
	qint16		m_Type;				// 1=flat, 2=gour, 3=phong, 4=metal
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Material definitions array.

class Au3dsGLSLMaterialArray : public QVector<Au3dsGLSLMaterial>
{
public:
	GLuint CreateDeviceObjects(const QString& filePath, AuGLEffects& re);	// returns the texture id or 0

	int IndexOf(const QString& name) const;

	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLMaterialArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A control console display screen

class Au3dsGLSLScreen
{
public:
	Au3dsGLSLScreen()
	{
		m_VertGroup = VERT_Invalid;
	}

	inline const AuPoint& LocLL() const
	{
		return m_LocLL;
	}

	inline const AuPoint& LocUL() const
	{
		return m_LocUL;
	}

	inline const AuPoint& LocUR() const
	{
		return m_LocUR;
	}

	inline const AuPoint& LocLR() const
	{
		return m_LocLR;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLScreen);
	}

	inline void SetData(VERT_GROUP vertGroup, const AuPoint& vert0, const AuPoint& vert1, const AuPoint& vert2, const AuPoint& vert3)
	{
		m_VertGroup = vertGroup;

		if(vertGroup == VERT_Screen1)
		{
			m_LocLR = vert0;
			m_LocLL = vert1;
			m_LocUR = vert2;
			m_LocUL = vert3;
		}
		else
		{
			m_LocLR = vert0;
			m_LocUR = vert1;
			m_LocLL = vert2;
			m_LocUL = vert3;
		}
	}

	inline VERT_GROUP VertexGroup() const
	{
		return m_VertGroup;
	}

private:
	VERT_GROUP		m_VertGroup;	// Vertex group id. e.g. VERT_Screen1 or VERT_Screen2
	AuPoint			m_LocLL;		// Location of lower left corner.
	AuPoint			m_LocUL;		// Location of upper left corner.
	AuPoint			m_LocUR;		// Location of upper right corner.
	AuPoint			m_LocLR;		// Location of lower right corner.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class Au3dsGLSLScreenArray : public QVector<Au3dsGLSLScreen>
{
public:
	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLScreenArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A weapon firing location.

class Au3dsGLSLWeapon
{
public:
	Au3dsGLSLWeapon()
	{
		m_Id = 0;
	}

	void Dump() const
	{
		qDebug(" %d loc %g,%g,%g dir %g,%g,%g",
				m_Id, m_Loc.x, m_Loc.y, m_Loc.z,
				m_Dir.x, m_Dir.y, m_Dir.z);
	}

	quint8 Id() const
	{
		return m_Id;
	}

	const AuPoint& Loc() const
	{
		return m_Loc;
	}

	// Firing direction unit vector.
	const AuPoint& Dir() const
	{
		return m_Dir;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLWeapon);
	}

	void SetData(quint8 id, const AuPoint& loc, const AuPoint& dir)
	{
		m_Id = id;
		m_Loc = loc;
		m_Dir = dir;
	}

private:
	quint8			m_Id;		// Unique identifier for this station.
	AuPoint			m_Loc;		// location of firing point in model
	AuPoint			m_Dir;		// unit vector of firing direction
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class Au3dsGLSLWeaponArray : public QVector<Au3dsGLSLWeapon>
{
public:
	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLWeaponArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A station at which a person may be positioned.

class Au3dsGLSLStation
{
public:
	enum POSE
	{
		// These enum values are used as a bitmask.
		Invalid		= 0,
		Stand		= 0x01,
		Crouch		= 0x02,
		Prone		= 0x04,
		Sit			= 0x08,	// sitting on the ground
		Any			= 0x0f,	// NOTE: Except 'Seated' position, i.e. no chair.
		Seated		= 0x10,	// sitting at a console or in a chair
		Straddle	= 0x20,	// e.g. motorcycle
		EyesOnly	= 0x40,	// e.g. space fighter cockpit
		All			= 0x7f
	};

public:
	Au3dsGLSLStation()
	{
		m_Id = 0;
		m_Pose = Any;
	}

	POSE DefaultPose() const
	{
		if(m_Pose & EyesOnly)
			return EyesOnly;
		else if(m_Pose & Stand)
			return Stand;
		else if(m_Pose & Seated)
			return Seated;
		else if(m_Pose & Straddle)
			return Straddle;
		else if(m_Pose & Sit)
			return Sit;
		else if(m_Pose & Crouch)
			return Crouch;
		else if(m_Pose & Prone)
			return Prone;
		qWarning("Au3dsGLSLStation::DefaultPose pose %x invalid", m_Pose);
		return Stand;
	}

	void Dump() const
	{
		qDebug(" %d poseMask %02x loc %g,%g,%g offloadLoc %g,%g,%g",
				m_Id, m_Pose, m_Loc.x, m_Loc.y, m_Loc.z,
				m_LocOff.x, m_LocOff.y, m_LocOff.z);
	}

	quint8 Id() const
	{
		return m_Id;
	}

	const AuPoint& Loc() const
	{
		return m_Loc;
	}

	// Returns offload location for this station, which is where the occupant
	// exits to when departing. This is also the item drop location when they
	// drop an item and there are no more empty stations inside the vehicle at
	// which to put the item.
	const AuPoint& LocOff() const
	{
		return m_LocOff;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLStation);
	}

	quint16 Pose() const
	{
		return m_Pose;
	}

	void SetData(quint8 id, const AuPoint& loc, const AuPoint& off, quint16 pose)
	{
		m_Id = id;
		m_Loc = loc;
		m_LocOff = off;
		m_Pose = pose;
	}

private:
	quint8			m_Id;		// Unique identifier for this station.
	AuPoint			m_Loc;		// location of station in model
	AuPoint			m_LocOff;	// location off model when departing station
	quint8			m_Pose;		// bitmask of POSE values defining allowed poses
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class Au3dsGLSLStationArray : public QVector<Au3dsGLSLStation>
{
public:
	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLStationArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A 3d object
// This is the 3d face and vertex data that defines one object of a 3ds model.

class Au3dsGLSLObject
{
public:
	inline Au3dsGLSLObject()
	{
		m_bColor = QColor(255,255,255);
		m_bCreate = false;
		m_bHasDuplicates = false;
		m_BottomScalePt = Au3dsGLSLVertex(0, 0, 0);
		m_bScaled = false;
		m_bShininess = 0;
		m_ConnectPt = Au3dsGLSLVertex(0, 0, 0);
		m_EyePt = Au3dsGLSLVertex(0, 0, 0);
		m_Faces.clear();
		m_FaceSmoothing.clear();
		m_Glow = 0;
		m_Glowdim = 0;
		m_HandWingLength = 1;
		m_HandWingPt = Au3dsGLSLVertex(0, 0, 0);
		m_HashVertices.clear();
		m_IScale = 1;
		m_LowerLength = 1;
		m_LowerThickness = 1;
		m_MappingCoords.clear();
		m_Materials.clear();
		m_MidPt = Au3dsGLSLVertex(0, 0, 0);
		m_MidScalePt = Au3dsGLSLVertex(0, 0, 0);
		m_Offset = Au3dsGLSLVertex(0, 0, 0);
		m_OverallScale = 1;
		m_tColor = QColor(255,255,255);
		m_TopScalePt = Au3dsGLSLVertex(0, 0, 0);
		m_tShininess = 0;
		m_UBOffset = 0;
		m_UpperLength = 1;
		m_UpperThickness = 1;
		m_UTOffset = 0;
		m_VBOffset = 0;
		m_Vertices.clear();
		m_VertexGroups.clear();
		m_VTOffset = 0;
		m_bLight = false;
		m_bHasBones = false;
		m_WeaponUpDownPt = Au3dsGLSLVertex(0, 0, 0);
		m_WeaponLeftRightPt = Au3dsGLSLVertex(0, 0, 0);
	}

	inline Au3dsGLSLObject(const Au3dsGLSLObject& c) : m_Materials(c.m_Materials), m_MappingCoords(c.m_MappingCoords),
		m_FaceSmoothing(c.m_FaceSmoothing), m_Faces(c.m_Faces), m_Vertices(c.m_Vertices), m_VertexGroups(c.m_VertexGroups),
		m_HashVertices(c.m_HashVertices)
	{
		m_Matrix = c.m_Matrix;
		m_Name = c.m_Name;
		m_OverallScale = c.m_OverallScale;		// Overall scaling
		m_IScale = c.m_IScale;			// Independent scaling
		m_UTOffset = c.m_UTOffset;			// The TopSkin Offset we want to apply to this object
		m_VTOffset = c.m_VTOffset;			// The TopSkin Offset we want to apply to this object
		m_UBOffset = c.m_UBOffset;			// The TopSkin Offset we want to apply to this object
		m_VBOffset = c.m_VBOffset;			// The TopSkin Offset we want to apply to this object
		m_UpperThickness = c.m_UpperThickness;
		m_UpperLength = c.m_UpperLength;
		m_LowerThickness = c.m_LowerThickness;
		m_LowerLength = c.m_LowerLength;
		m_HandWingLength = c.m_HandWingLength;
		m_Offset = c.m_Offset;
		m_ConnectPt = c.m_ConnectPt;
		m_MidPt = c.m_MidPt;
		m_HandWingPt = c.m_HandWingPt;
		m_TopScalePt = c.m_TopScalePt;
		m_MidScalePt = c.m_MidScalePt;
		m_BottomScalePt = c.m_BottomScalePt;
		m_EyePt = c.m_EyePt;
		m_tColor = c.m_tColor;
		m_tShininess = c.m_tShininess;
		m_bColor = c.m_bColor;
		m_bShininess = c.m_bShininess;
		m_Glow = c.m_Glow;
		m_Glowdim = c.m_Glowdim;
		m_bCreate = c.m_bCreate;
		m_bScaled = c.m_bScaled;			// True if object has previously has been scaled
		m_bHasDuplicates = c.m_bHasDuplicates;
		m_bHasBones = c.m_bHasBones;
		m_bLight = c.m_bLight;
		m_WeaponUpDownPt = c.m_WeaponUpDownPt;
		m_WeaponLeftRightPt = c.m_WeaponLeftRightPt;
	}

public:
	inline const Au3dsGLSLObject& operator=(const Au3dsGLSLObject& c)
	{
		m_Materials = c.m_Materials;
		m_MappingCoords = c.m_MappingCoords;
		m_FaceSmoothing = c.m_FaceSmoothing;
		m_Faces = c.m_Faces;
		m_Vertices = c.m_Vertices;
		m_VertexGroups = c.m_VertexGroups;
		m_HashVertices = c.m_HashVertices;

		m_Matrix = c.m_Matrix;
		m_Name = c.m_Name;
		m_OverallScale = c.m_OverallScale;		// Overall scaling
		m_IScale = c.m_IScale;			// Independent scaling
		m_UTOffset = c.m_UTOffset;			// The TopSkin Offset we want to apply to this object
		m_VTOffset = c.m_VTOffset;			// The TopSkin Offset we want to apply to this object
		m_UBOffset = c.m_UBOffset;			// The TopSkin Offset we want to apply to this object
		m_VBOffset = c.m_VBOffset;			// The TopSkin Offset we want to apply to this object
		m_UpperThickness = c.m_UpperThickness;
		m_UpperLength = c.m_UpperLength;
		m_LowerThickness = c.m_LowerThickness;
		m_LowerLength = c.m_LowerLength;
		m_HandWingLength = c.m_HandWingLength;
		m_Offset = c.m_Offset;
		m_ConnectPt = c.m_ConnectPt;
		m_MidPt = c.m_MidPt;
		m_HandWingPt = c.m_HandWingPt;
		m_TopScalePt = c.m_TopScalePt;
		m_MidScalePt = c.m_MidScalePt;
		m_BottomScalePt = c.m_BottomScalePt;
		m_EyePt = c.m_EyePt;
		m_tColor = c.m_tColor;
		m_tShininess = c.m_tShininess;
		m_bColor = c.m_bColor;
		m_bShininess = c.m_bShininess;
		m_Glow = c.m_Glow;
		m_Glowdim = c.m_Glowdim;
		m_bCreate = c.m_bCreate;
		m_bScaled = c.m_bScaled;			// True if object has previously has been scaled
		m_bHasDuplicates = c.m_bHasDuplicates;
		m_bHasBones = c.m_bHasBones;
		m_bLight = c.m_bLight;
		m_WeaponUpDownPt = c.m_WeaponUpDownPt;
		m_WeaponLeftRightPt = c.m_WeaponLeftRightPt;
		return *this;
	}
	// Windows seems to want this. :/
	inline bool operator==(const Au3dsGLSLObject& c) const
	{
		if(m_Name != c.m_Name)
			return false;
		return true;
	}

public:
	void CreateDeviceObject(AuOpenGLFunctions& GLf, Au3dsGLSLVAO& model, const Au3dsGLSLMaterialArray& materials,
			bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc);

	void CreateBodyDeviceObject(AuOpenGLFunctions& GLf, Au3dsGLSLVAO& model,
			const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc);

	// Loads a regular model.
	void LoadModel(Au3dsGLSLTriTexList& smdl, const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType,
			GLuint destBlendFunc);

	// Loads a body model, with bone weights and stuff.
	void LoadModel(Au3dsGLSLTriBodyList& smdl, const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType,
			GLuint destBlendFunc);

	void Dump() const;

	inline bool IsValid(MODEL_TYPE modelType) const
	{
		if(HasPrecalculatedNormals(modelType))
		{
			if(!m_Faces.count() && m_Name[0] != '@' && m_Name[0] != '!' && m_Name != "DataOnly" && m_Name != "UpVector")
				return false;
		}
		else
		{
			if(!m_Faces.count())
				return false;
		}
		if(!m_Vertices.count())
			return false;
		return true;
	}

	bool LoadChunk(QDataStream& in, int chunkSize, AuPoint& min, AuPoint& max, MODEL_TYPE modelType);

	inline const QString& Name() const
	{
		return m_Name;
	}

	bool TestForExtents(MODEL_TYPE modelType) const;								// based on name

	bool TestForScreens(Au3dsGLSLScreen& screen1, Au3dsGLSLScreen& screen2) const;	// 'screen' is output if return is true

	bool TestForStation(Au3dsGLSLStation& station, MODEL_TYPE modelType) const;			// 'station' is output if return is true

	bool TestForWeapon(Au3dsGLSLWeapon& weapon, MODEL_TYPE modelType) const;				// 'weapon' is output if return is true

	bool TestForScalePt();

	bool TestForUpVector(AuPoint &UpVector);	// up vector when object is on the ground.

	AuPoint ApplyScaleFactor(AuPoint &vPoint) const;

	bool ApplyTextureOffset(const Au3dsGLSLVertex& v0, const Au3dsGLSLVertex& v1, const Au3dsGLSLVertex& v2, 
		double &v0uOffset, double &v0vOffset, 
		double &v1uOffset, double &v1vOffset,
		double &v2uOffset, double &v2vOffset) const;

	double GetTextureWeight(Au3dsGLSLVertex vert);

	void AverageWeights(int pb, int tb, int nb, QString sFileName);

	bool IsBone(QString string, int bone) const;	// True if string name is part of bone group

	int FindTailBone(QString string) const;

	int FindNeckBone(QString string) const;

	quint64 MemoryUsage() const;

	// Multiplies all geometry/vertices by 'scale' for uniform scaling.
	inline void ScaleBy(double scale);

	float Transparency(const Au3dsGLSLMaterialArray& materials, MODEL_TYPE modelType) const;

private:
	bool LoadFaceListChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType);
	bool LoadFaceMaterialChunk(QDataStream& in, int chunkSize);
	bool LoadFaceSmoothingChunk(QDataStream& in, int chunkSize, quint16 faces);
	bool LoadMeshChunk(QDataStream& in, int chunkSize, AuPoint& min, AuPoint& max, MODEL_TYPE modelType);
	bool LoadMappingCoordsChunk(QDataStream& in, int chunkSize);
	bool LoadMatrixChunk(QDataStream& in, int chunkSize, MODEL_TYPE modelType);
	bool LoadVertexListChunk(QDataStream& in, int chunkSize);

	void SetMappingCoords();

private:
	AuMatrix4x4				m_Matrix;
	QString					m_Name;
public:
	QMap<QString, QVector<quint16> >	m_Materials;	// key is material name
	Au3dsGLSLMappingCoordArray	m_MappingCoords;
	QVector<quint32>				m_FaceSmoothing;
	Au3dsGLSLFaceArray					m_Faces;
	Au3dsGLSLVertexArray					m_Vertices;
	Au3dsGLSLVGroupArray					m_VertexGroups;		// Vertex group definition
	double							m_OverallScale;		// Overall scaling
	double							m_IScale;			// Independent scaling
	double							m_UTOffset;			// The TopSkin Offset we want to apply to this object
	double							m_VTOffset;			// The TopSkin Offset we want to apply to this object
	double							m_UBOffset;			// The TopSkin Offset we want to apply to this object
	double							m_VBOffset;			// The TopSkin Offset we want to apply to this object
	double							m_UpperThickness;
	double							m_UpperLength;
	double							m_LowerThickness;
	double							m_LowerLength;
	double							m_HandWingLength;
	Au3dsGLSLVertex						m_Offset;
	Au3dsGLSLVertex						m_ConnectPt;
	Au3dsGLSLVertex						m_MidPt;
	Au3dsGLSLVertex						m_HandWingPt;
	Au3dsGLSLVertex						m_TopScalePt;
	Au3dsGLSLVertex						m_MidScalePt;
	Au3dsGLSLVertex						m_BottomScalePt;
	Au3dsGLSLVertex						m_EyePt;
	QColor							m_tColor;
	float							m_tShininess;
	QColor							m_bColor;
	float							m_bShininess;
	float							m_Glow;
	float							m_Glowdim;
	bool							m_bCreate;
	bool							m_bScaled;			// True if object has previously has been scaled
	unordered_multimap<Au3dsGLSLVertex, int, Au3dsGLSLVertex_Hash>	m_HashVertices;
	bool							m_bHasDuplicates;
	bool							m_bHasBones;
	bool							m_bLight;
	Au3dsGLSLVertex						m_WeaponUpDownPt;
	Au3dsGLSLVertex						m_WeaponLeftRightPt;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// 3d objects array.

class Au3dsGLSLObjectArray : public QVector<Au3dsGLSLObject>
{
public:
	void CreateDeviceObjects(AuOpenGLFunctions& GLf, QList<Au3dsGLSLVAO>& models,
			const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc);

	void CreateBodyDeviceObjects(AuOpenGLFunctions& GLf, QList<Au3dsGLSLVAO>& models,
			const Au3dsGLSLMaterialArray& materials, bool lightsOn, MODEL_TYPE modelType, GLuint destBlendFunc);

	// Loads regular models.
	void LoadModels(QList<Au3dsGLSLTriTexList>& models, const Au3dsGLSLMaterialArray& materials, bool lightsOn,
			MODEL_TYPE modelType, GLuint destBlendFunc);

	// Loads models for creature body, with bone weights and stuff.
	void LoadModels(QList<Au3dsGLSLTriBodyList>& models, const Au3dsGLSLMaterialArray& materials, MODEL_TYPE modelType,
			GLuint destBlendFunc);

	inline quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(Au3dsGLSLObjectArray);
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			mem += (*ii).MemoryUsage();
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A 3ds model.
// A 3ds model is made up of one or more objects and may include material
// definitions.

class Au3dsGLSLModel
{
public:
	Au3dsGLSLModel();
	virtual ~Au3dsGLSLModel();

	inline void GetExtents(AuPoint& min, AuPoint& max) const
	{
		min = m_Min;
		max = m_Max;
	}

	inline void GetUpVector(AuPoint& upvector) const
	{
		upvector = m_UpVector;
	}

	bool Load(QFile& file3ds, double scale=1.);

	bool Load(const QString& file3dsName, double scale=1.);

	inline bool HasPrecalculatedNormals() const
	{
		return ::HasPrecalculatedNormals(m_ModelType);
	}

	virtual quint64 MemoryUsage() const;

	inline double RoughRadius() const
	{
		return m_RoughRadius;
	}

private:
	bool LoadEditorChunk(QDataStream& in, int chunkSize);
	bool LoadMainChunk(QDataStream& in, int chunkSize);

public:
	double									m_GeometryScale;		// Scaling that was applied in CreateGeometry.
	Au3dsGLSLMaterialArray					m_Materials;			// Material definitions
	Au3dsGLSLObjectArray					m_Objects;				// 3d object definitions
	Au3dsGLSLVGroupArray					m_VertexGroups;			// Vertex group definitions
	AuPoint									m_Min, m_Max;			// 3d extents of vertices
	MODEL_TYPE								m_ModelType;
	double									m_RoughRadius;
	Au3dsGLSLScreenArray*					m_Screens;				// screens on control consoles, can be null
	Au3dsGLSLStationArray*					m_Stations;				// stations in vehicles, can be null
	AuPoint									m_UpVector;				// unit vector should point up when on the ground
	Au3dsGLSLWeaponArray*					m_Weapons;				// weapons in vehicles, can be null
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A 3ds model instance in the cache.

class Au3dsGLSLModelInstance : private Au3dsGLSLModel, public AuGLModelInstance
{
public:
	/*
	inline Au3dsGLSLModelInstance()
	{
		m_bColor = QColor(255,255,255);
		m_bShininess = 0;
		m_EyePt = AuPoint(0, 0, 0);
		m_FileName = "";
		m_Glow = 0;
		m_Glowdim = 0;
		m_HandWingLength = 1;
		m_IScale =1;
		m_LoaderJob = 0;
		m_LowerLength = 1;
		m_LowerThickness = 1;
		m_nBone = 0;
		m_Offset = AuPoint(0, 0, 0);
		m_OverallScale = 1;
		m_pBone = 0;
		m_tBone = 0;
		m_tColor = QColor(255,255,255);
		m_Texture = 0;
		m_tShininess = 0;
		m_UBOffset = 0;
		m_UpperLength = 1;
		m_UpperThickness = 1;
		m_UTOffset = 0;
		m_VBOffset = 0;
		m_VTOffset = 0;
	}
	*/
	inline Au3dsGLSLModelInstance(const QString& fileName)
	{
		m_bColor = QColor(255,255,255);
		m_bShininess = 0;
		m_EyePt = AuPoint(0, 0, 0);
		m_FileName = fileName;
		m_Glow = 0;
		m_Glowdim = 0;
		m_HandWingLength = 1;
		m_IScale =1;
		m_LoaderJob = 0;
		m_LowerLength = 1;
		m_LowerThickness = 1;
		m_nBone = 0;
		m_Offset = AuPoint(0, 0, 0);
		m_OverallScale = 1;
		m_pBone = 0;
		m_tBone = 0;
		m_tColor = QColor(255,255,255);
		m_Texture = 0;
		m_tShininess = 0;
		m_UBOffset = 0;
		m_UpperLength = 1;
		m_UpperThickness = 1;
		m_UTOffset = 0;
		m_VBOffset = 0;
		m_VTOffset = 0;
	}

	// 3ds model does not know its aim axis. It comes from the DbCommodity record.
	virtual int AimAxis() const
	{
		return -1;
	}

	virtual bool CreateDeviceObjects(AuGLEffects& re, bool bLightsOn, double scale);

	// Returns true if the open gl model has been created for this object.
	// Creates the open gl model from intermediate data if the data is ready.
	// OpenGL context must be current when calling this guy.
	virtual bool GLModel(AuGLEffects& re);

	virtual GROUND_ORIENT GroundOrientation() const
	{
		return GORI_Count;	// 3ds models do not know their ground orientation. It comes from the DbCommodity record.
	}

	virtual bool HasGLModel() const
	{
		return !m_Models.isEmpty();
	}

	virtual void LoadModel(bool lightOn, double scale)
	{
		LoadModels(lightOn, scale);
	}

	// Starts a worker thread to load the intermediate model data.
	virtual void QueueLoadModel(bool lightsOn, double scale);

	bool LoadModels(bool bLightsOn, double scale);

	inline bool CreateBodyDeviceObjects(AuOpenGLFunctions& GLf, const QString& fileName, bool bLightsOn,
			double iScale1, double overallScale,
			//double uTOffset, double vTOffset, double uBOffset, double vBOffset, 
			int pb, int tb, int nb, const AuPoint& pOffset,
			double uL, double uT, double lL, double lT, double hwL, QColor tC, float tS, 
			QColor bC, float bS, float glow, float glowdim, bool bCreate, bool bSkip, GLuint destBlendFunc)
	{
		// Delete this model if it was already created.
		DeleteGL();

		// Load the 3ds model.
		if(!Load(fileName))
			return false;

		// Load succeeded. Did we get any model data?
		int modelCount = m_Objects.count();
		if(!modelCount)
			return false;

		// Strip the file path from the file name to locate associated
		// map files.
		QFileInfo fileInfo(fileName);
		QString filePath = fileInfo.absolutePath();

		// Create texture(s) outside of the model display list(s).
		// Textures are not created for bodies. They are designed to be used with a specific texture palette, not the texture
		// referenced by any of the model pieces.
		//m_Texture = m_Materials.CreateDeviceObjects(STATIC_MODEL_ID, filePath, re);

		// Gather model manipulation data
		m_Offset = pOffset;
		m_OverallScale = overallScale;
		m_IScale = iScale1;
// 		m_UTOffset = uTOffset;
// 		m_VTOffset = vTOffset;
// 		m_UBOffset = uBOffset;
// 		m_VBOffset = vBOffset;
		m_UpperLength = uL;
		m_UpperThickness = uT;
		m_LowerLength = lL;
		m_LowerThickness = lT;
		m_HandWingLength = hwL;
		m_pBone = pb;
		m_tBone = tb;
		m_nBone = nb;
		m_tColor = tC;
		m_tShininess = tS;
		m_bColor = bC;
		m_bShininess = bS;
		m_Glow = glow;
		m_Glowdim = glowdim;
		m_LitModelCount = 0;

		for(int i=0; i<modelCount; i++)
		{
			if(m_Objects[i].m_bLight)
				m_LitModelCount++;
			m_Objects[i].m_Offset = Au3dsGLSLVertex(pOffset.x, pOffset.y, pOffset.z);
			m_Objects[i].m_OverallScale = overallScale;
			m_Objects[i].m_IScale = iScale1;
// 			m_Objects[i].m_UTOffset = uTOffset;
// 			m_Objects[i].m_VTOffset = vTOffset;
// 			m_Objects[i].m_UBOffset = uBOffset;
// 			m_Objects[i].m_VBOffset = vBOffset;
			m_Objects[i].m_UpperLength = uL;
			m_Objects[i].m_UpperThickness = uT;
			m_Objects[i].m_LowerLength = lL;
			m_Objects[i].m_LowerThickness = lT;
			m_Objects[i].m_HandWingLength = hwL;
			m_Objects[i].m_tColor = tC;
			m_Objects[i].m_tShininess = tS;
			m_Objects[i].m_bColor = bC;
			m_Objects[i].m_bShininess = bS;
			m_Objects[i].m_Glow = glow;
			m_Objects[i].m_Glowdim = glowdim;
			m_Objects[i].m_bCreate = bCreate;
			QVector<Au3dsGLSLVGroup>::iterator itVGroup;
			AuPoint headScalePt;
			for (itVGroup = m_Objects[i].m_VertexGroups.begin(); itVGroup != m_Objects[i].m_VertexGroups.end(); itVGroup++)
			{
				(*itVGroup).m_scaleFactor = iScale1;
				if((*itVGroup).m_bHead)
				{
					headScalePt = AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
					break;
				}
			}
			m_Objects[i].AverageWeights(pb, tb, nb, fileName);

			if(fileName.contains("Skull"))
			{
				AuPoint vConnect(m_Objects[i].m_ConnectPt.X(), m_Objects[i].m_ConnectPt.Y(), m_Objects[i].m_ConnectPt.Z());
				AuPoint vEyePt(m_Objects[i].m_EyePt.X(), m_Objects[i].m_EyePt.Y(), m_Objects[i].m_EyePt.Z());

				// Apply scaling so that eye position is correct
				vEyePt *= m_Objects[i].m_OverallScale;
				vConnect *= m_Objects[i].m_OverallScale;
				if(m_Objects[i].m_LowerThickness != 1)		// This is torso thickness for head
				{
					vEyePt = vEyePt-headScalePt;	// Move to scale location
					vEyePt = AuPoint(vEyePt.x*m_LowerThickness, vEyePt.y*m_LowerThickness, vEyePt.z*m_LowerThickness) + headScalePt;

					vConnect = vConnect-headScalePt;	// Move to scale location
					vConnect = AuPoint(vConnect.x*m_LowerThickness, vConnect.y*m_LowerThickness, vConnect.z*m_LowerThickness) + headScalePt;
				}
				m_EyePt = vEyePt - vConnect;
			}
			//m_EyePt = AuPoint(m_Objects[i].m_EyePt.X(), m_Objects[i].m_EyePt.Y(), m_Objects[i].m_EyePt.Z());
			//m_EyePt = m_EyePt - AuPoint(m_Objects[i].m_ConnectPt.X(), m_Objects[i].m_ConnectPt.Y(), m_Objects[i].m_ConnectPt.Z());
		}

		// If we are skipping the creation of the device objects then this must be a torso or face
		// As each torso segment instance is loaded their data gets merged together to form one mesh
		if(!bSkip)
		{
// 			if(!HasPrecalculatedNormals())
// 			{
				// We need a Gl display list for each component object of the model.
				m_Objects.CreateBodyDeviceObjects(GLf, m_Models, m_Materials, false, m_ModelType, destBlendFunc);
				if(bLightsOn)
				{
					// Create Gl models of the component objects.
					m_Objects.CreateBodyDeviceObjects(GLf, m_ModelsLit, m_Materials, true, m_ModelType, destBlendFunc);
				}
// 			}
// 			else
// 			{
// 				if(bLightsOn && m_LitModelCount)
// 				{
// 					m_Objects.CreateBodyDeviceObjects(m_ModelsLit, m_Materials, true, m_ModelType, destBlendFunc, false);
// 				}
// 				modelCount = modelCount - m_LitModelCount;
// 				if(modelCount)
// 				{
// 					m_Objects.CreateBodyDeviceObjects(m_Models, m_Materials, false, m_ModelType, destBlendFunc, false);
// 				}
// 			}
		}

		m_FileName = fileName;
		return true;
	}

	inline void CreatePatchedModel(AuOpenGLFunctions& GLf)
	{
		int modelCount = m_Objects.count();
		if(modelCount < 0)
			return;

		// Create unlit and lit versions of our model
		m_Objects.CreateBodyDeviceObjects(GLf, m_Models, m_Materials, false, m_ModelType, GL_ONE_MINUS_SRC_ALPHA);
	}

	/*
	inline bool GenerateDeviceObjects(Au3dsGLSLModelInstance* pModel, unsigned int uiUserId,
		const QString& fileName, AuGLEffects& re, bool bLightsOn)
	{
		// Generate device objects for the model instance passed in.
		// Can only be used for generic model types

		// First copy 'this' generic model instance
		pModel->CopyInstance(this);
		if(!pModel->m_Objects.count())
			return false;

		// Strip the file path from the file name to locate associated
		// map files.
		QFileInfo fileInfo(fileName);
		QString filePath = fileInfo.absolutePath();

		// Create texture(s) outside of the model display list(s).
		pModel->m_Texture = pModel->m_Materials.CreateDeviceObjects(uiUserId, filePath, re);

		pModel->m_Objects.CreateDeviceObjects(pModel->m_Models, pModel->m_Materials, false, pModel->m_ModelType, GL_ONE_MINUS_SRC_ALPHA, false);
		if(bLightsOn)
		{
			// Create Gl models of the component objects.
			pModel->m_Objects.CreateDeviceObjects(pModel->m_ModelsLit, pModel->m_Materials, true, pModel->m_ModelType, GL_ONE_MINUS_SRC_ALPHA, false);
		}

		pModel->m_FileName = fileName;
		return true;
	}
	*/

	inline bool GenerateBodyDeviceObjects(AuOpenGLFunctions& GLf, Au3dsGLSLModelInstance* pModel,
			const QString& fileName, bool bLightsOn, double iScale1, double overallScale,
			//double uTOffset, double vTOffset, double uBOffset, double vBOffset,
			int pb, int tb, int nb, const AuPoint& pOffset,
			double uL, double uT, double lL, double lT, double hwL,
			QColor tC, float tS, QColor bC, float bS, float glow, float glowdim, bool bCreate, bool bSkip, GLuint destBlendFunc)
	{
		// Generate device objects for the model instance passed in.
		// Can only be used for generic model types

		// First copy 'this' generic model instance
		pModel->CopyInstance(this);
		if(!pModel->m_Objects.count())
			return false;

		// Strip the file path from the file name to locate associated
		// map files.
		QFileInfo fileInfo(fileName);
		QString filePath = fileInfo.absolutePath();

		// Create texture(s) outside of the model display list(s).
		// Textures are not created for bodies. They are designed to be used with a specific texture palette, not the texture
		// referenced by any of the model pieces.
		//pModel->m_Texture = pModel->m_Materials.CreateDeviceObjects(STATIC_MODEL_ID, filePath, re);

		// Gather model manipulation data
		pModel->m_Offset = pOffset;
		pModel->m_OverallScale = overallScale;
		pModel->m_IScale = iScale1;
// 		pModel->m_UTOffset = uTOffset;
// 		pModel->m_VTOffset = vTOffset;
// 		pModel->m_UBOffset = uBOffset;
// 		pModel->m_VBOffset = vBOffset;
		pModel->m_UpperLength = uL;
		pModel->m_UpperThickness = uT;
		pModel->m_LowerLength = lL;
		pModel->m_LowerThickness = lT;
		pModel->m_HandWingLength = hwL;
		pModel->m_pBone = pb;
		pModel->m_tBone = tb;
		pModel->m_nBone = nb;
		pModel->m_tColor = tC;
		pModel->m_tShininess = tS;
		pModel->m_bColor = bC;
		pModel->m_bShininess = bS;
		pModel->m_Glow = glow;
		pModel->m_Glowdim = glowdim;

		// Gather object manipulation data
		for(int i=0; i<m_Objects.count(); i++)
		{
			pModel->m_Objects[i].m_Offset = Au3dsGLSLVertex(pOffset.x, pOffset.y, pOffset.z);
			pModel->m_Objects[i].m_OverallScale = overallScale;
			pModel->m_Objects[i].m_IScale = iScale1;
// 			pModel->m_Objects[i].m_UTOffset = uTOffset;
// 			pModel->m_Objects[i].m_VTOffset = vTOffset;
// 			pModel->m_Objects[i].m_UBOffset = uBOffset;
// 			pModel->m_Objects[i].m_VBOffset = vBOffset;
			pModel->m_Objects[i].m_UpperLength = uL;
			pModel->m_Objects[i].m_UpperThickness = uT;
			pModel->m_Objects[i].m_LowerLength = lL;
			pModel->m_Objects[i].m_LowerThickness = lT;
			pModel->m_Objects[i].m_HandWingLength = hwL;
			pModel->m_Objects[i].m_tColor = tC;
			pModel->m_Objects[i].m_tShininess = tS;
			pModel->m_Objects[i].m_bColor = bC;
			pModel->m_Objects[i].m_bShininess = bS;
			pModel->m_Objects[i].m_Glow = glow;
			pModel->m_Objects[i].m_Glowdim = glowdim;
			pModel->m_Objects[i].m_bCreate = bCreate;
			QVector<Au3dsGLSLVGroup>::iterator itVGroup;
			for (itVGroup = pModel->m_Objects[i].m_VertexGroups.begin(); itVGroup != pModel->m_Objects[i].m_VertexGroups.end(); itVGroup++)
			{
				(*itVGroup).m_scaleFactor = iScale1;
			}
			pModel->m_Objects[i].AverageWeights(pb, tb, nb, fileName);
		}

		if(!bSkip)
		{
// 			if(!pModel->HasPrecalculatedNormals())
// 			{
				// We need a Gl display list for each component object of the model.
				pModel->m_Objects.CreateBodyDeviceObjects(GLf, pModel->m_Models, pModel->m_Materials, false, pModel->m_ModelType, destBlendFunc);
				if(bLightsOn)
				{
					// Create Gl models of the component objects.
					pModel->m_Objects.CreateBodyDeviceObjects(GLf, pModel->m_ModelsLit, pModel->m_Materials, true, pModel->m_ModelType, destBlendFunc);
				}
// 			}
// 			else
// 			{
// 				if(bLightsOn && pModel->m_LitModelCount)
// 				{
// 					pModel->m_Objects.CreateBodyDeviceObjects(pModel->m_ModelsLit, pModel->m_Materials, true, pModel->m_ModelType, destBlendFunc, false);
// 				}
// 				pModel->m_ModelCount = pModel->m_ModelCount - pModel->m_LitModelCount;
// 				if(pModel->m_Objects.count())
// 				{
// 					pModel->m_Objects.CreateBodyDeviceObjects(pModel->m_Models, pModel->m_Materials, false, pModel->m_ModelType, destBlendFunc, false);
// 				}
// 			}
		}

		pModel->m_FileName = fileName;
		return true;
	}

	// Creates only the geometry of this objects.
	// Similar to CreateDeviceObjects.
	// Used on the server to determine basic geometry without GL or any sort
	// of rendering context.
	// 'scale' is an optional scaling factor applied to the model geometry when it is loaded from the file.
	virtual bool CreateGeometry(double scale=1.)
	{
		Q_ASSERT(!m_FileName.isEmpty());
		// Load the 3ds model.
		if(!Load(m_FileName, scale))
			return false;

		// Load succeeded. Did we get any model data?
		if(!m_Objects.count())
			return false;

		return true;
	}

	// OpenGL context must be current when this is called.
	virtual void DeleteGL()
	{
		for(QList<Au3dsGLSLVAO>::iterator vi = m_Models.begin(); vi != m_Models.end(); vi++)
			if((*vi).VAO())
				(*vi).DeleteGL();
		m_Models.clear();

		for(QList<Au3dsGLSLVAO>::iterator vi = m_ModelsLit.begin(); vi != m_ModelsLit.end(); vi++)
			if((*vi).VAO())
				(*vi).DeleteGL();
		m_ModelsLit.clear();
	}

	virtual const QString& FileName() const
	{
		return m_FileName;
	}

	virtual void GetExtents(AuPoint& min, AuPoint& max) const
	{
		Au3dsGLSLModel::GetExtents(min, max);
	}

	inline void GetUpVector(AuPoint& upvector) const
	{
		Au3dsGLSLModel::GetUpVector(upvector);
	}

	inline void CopyInstance(const Au3dsGLSLModelInstance* pModel)
	{
		// Copy Object array
		m_Objects = pModel->m_Objects;

		// Intermediate models.
		m_IntModels = pModel->m_IntModels;;
		m_IntModelsLit = pModel->m_IntModelsLit;;

		// Copy Screens array
		if(pModel->m_Screens)
		{
			m_Screens = new Au3dsGLSLScreenArray(*pModel->m_Screens);
		}
		else if(m_Screens)
		{
			delete m_Screens;
			m_Screens = 0;
		}

		// Copy Station array
		if(pModel->m_Stations)
		{
			m_Stations = new Au3dsGLSLStationArray(*pModel->m_Stations);
		}
		else if(m_Stations)
		{
			delete m_Stations;
			m_Stations = 0;
		}

		// Copy Weapons array
		if(pModel->m_Weapons)
		{
			m_Weapons = new Au3dsGLSLWeaponArray(*pModel->m_Weapons);
		}
		else if(m_Weapons)
		{
			delete m_Weapons;
			m_Weapons = 0;
		}

		// Copy VertexGroup array
		m_VertexGroups = pModel->m_VertexGroups;

		// Copy Materials array
		m_Materials = pModel->m_Materials;

		// Manipulating fields
		m_OverallScale = pModel->m_OverallScale;
		m_IScale = pModel->m_IScale;
// 		m_UTOffset = pModel->m_UTOffset;
// 		m_VTOffset = pModel->m_VTOffset;
// 		m_UBOffset = pModel->m_UBOffset;
// 		m_VBOffset = pModel->m_VBOffset;
		m_UpperThickness = pModel->m_UpperThickness;
		m_UpperLength = pModel->m_UpperLength;
		m_LowerThickness = pModel->m_LowerThickness;
		m_LowerLength = pModel->m_LowerLength;
		m_HandWingLength = pModel->m_HandWingLength;
		m_Offset = pModel->m_Offset;
		m_Glowdim = pModel->m_Glowdim;
		m_Glow = pModel->m_Glow;
		m_ModelType = pModel->m_ModelType;
		m_tColor = pModel->m_tColor;
		m_bColor = pModel->m_bColor;
		m_pBone = pModel->m_pBone;
		m_tBone = pModel->m_tBone;
		m_nBone = pModel->m_nBone;
		m_tShininess = pModel->m_tShininess;
		m_bShininess = pModel->m_bShininess;
		m_EyePt = pModel->m_EyePt;
		m_Min = pModel->m_Min;
		m_Max = pModel->m_Max;
		m_RoughRadius = pModel->m_RoughRadius;
		m_UpVector = pModel->m_UpVector;

		m_FileName = pModel->m_FileName;

		//m_LitModelCount;// Number of Gl model ids with lights on
		//m_Texture;		// Gl texture
	}

	inline bool CheckFields(double OverallScale, double iScale, //double uTop, double vTop,
							/*double uBottom, double vBottom,*/ double uThick,
							double uLength, double lThick, double lLength, double hwLength,
							const AuPoint& loc, int pb, int tb, int nb, QColor tC, float tS,
							QColor bC, float bS,float glow, float glowdim) const
	{
		if(m_OverallScale != OverallScale)
			return false;
		if(m_IScale != iScale)
			return false;
// 		if(m_UTOffset != uTop)
// 			return false;
// 		if(m_VTOffset != vTop)
// 			return false;
// 		if(m_UBOffset != uBottom)
// 			return false;
// 		if(m_VBOffset != vBottom)
// 			return false;
		if(uThick != m_UpperThickness)
			return false;
		if(uLength != m_UpperLength)
			return false;
		if(lThick != m_LowerThickness)
			return false;
		if(lLength != m_LowerLength)
			return false;
		if(loc != m_Offset)
			return false;
		if(pb != m_pBone)
			return false;
		if(tb != m_tBone)
			return false;
		if(nb != m_nBone)
			return false;
		if(tC != m_tColor)
			return false;
		if(tS != m_tShininess)
			return false;
		if(bC != m_bColor)
			return false;
		if(bS != m_bShininess)
			return false;
		if(glow != m_Glow)
			return false;
		if(glowdim != m_Glowdim)
			return false;
		if(hwLength != m_HandWingLength)
			return false;
		return true;
	}

	// Returns the four corners of a console display screen.
	// Nothing happens if this model does not have screens info.
	// 'index' is 0 for the large primary display screen
	// 'index' is 1 for the small secondary display screen
	void ScreenVertices(int index, AuPoint& ll, AuPoint& ul, AuPoint& ur, AuPoint& lr) const
	{
		Q_ASSERT(m_Screens);	// Do not call this function unless you know the model has display screens.
		Q_ASSERT(index >= 0 && index < m_Screens->count());
		Au3dsGLSLScreenArray::const_iterator si = m_Screens->constBegin() + index;
		ll = (*si).LocLL();
		ul = (*si).LocUL();
		ur = (*si).LocUR();
		lr = (*si).LocLR();
	}

	bool MergeHeadWithInstance(Au3dsGLSLModelInstance *&pDest, double fuzz);

	bool MergeTorsoWithInstance(Au3dsGLSLModelInstance *&pDest, double fuzz, bool bConnectOnly = false);

	AuPoint OffloadLoc(int index) const	// Location to offload from a seat.
	{
		if(!m_Stations)
			return AuPoint(0,0);
		if(index < 0 || index >= m_Stations->count())
			return AuPoint(0,0);
		return (*m_Stations)[index].LocOff();
	}

	inline void Render(AuOpenGLFunctions& GLf, bool lightsOn) const
	{
		if(lightsOn)
		{
			if(!m_ModelsLit.isEmpty())
			{
				for(QList<Au3dsGLSLVAO>::const_iterator vi = m_ModelsLit.constBegin(); vi != m_ModelsLit.constEnd(); vi++)
					if((*vi).VAO())
					{
						(*vi).SelectGL();
						(*vi).DrawGLBlend(GLf);
						(*vi).DeselectGL();
					}
			}
			else
				qWarning() << "Expected lit version of model" << m_FileName;
		}
		else
		{
			for(QList<Au3dsGLSLVAO>::const_iterator vi = m_Models.constBegin(); vi != m_Models.constEnd(); vi++)
				if((*vi).VAO())
				{
					(*vi).SelectGL();
					(*vi).DrawGLBlend(GLf);
					(*vi).DeselectGL();
				}
		}
	}

	virtual bool Render(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float alpha=1.f, int pass=0) const;

	// 'globeP' is used for sun over the horizon effect.
	// 'globeP' and 'ambientLightUp' are in eye space.
	virtual bool RenderFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuPoint& globeP,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f, int pass=0) const;

	virtual bool RenderFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			float shadowPanelSunAngles[MAX_SUNS], float ambientLightR, float ambientLightG, float ambientLightB,
			const AuPoint& ambientLightUp, bool lightOn, float fogDensity, float fogR, float fogG, float fogB,
			float alpha=1.f, int pass=0) const;

	// Renders this model using the GLSL shader ModelTex or ModelTexFog, as appropriate based on current render states set in re.
	// 'ambientLightUp' is in eye space.
	void RenderGLSLModel(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuPoint& ambientLightUp, bool lightOn, float alpha=1.f) const;

	// Renders this model using the GLSL shader ModelTex. Gets current ambient light from re.
	// 'ambientLightUp' is in eye space.
	void RenderGLSLModelTex(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuPoint& ambientLightUp, bool lightOn, float alpha=1.f) const;

	// Renders this model using the GLSL shader ModelTex.
	// 'ambientLightUp' is in eye space.
	void RenderGLSLModelTex(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float alpha=1.f, bool useModelBlending=true) const;

	// ModelTexShader must already be selected and configured for this one.
	void RenderGLSLModelTexShader(const AuMatrix4x4& mv, AuGLEffects& re, const AuPoint& ambientLightUp, bool lightOn,
			float alpha=1.f, bool useModelBlending=true) const;

	// Renders this model using the GLSL shader ModelTexFog. Gets current ambient light and fog from re.
	// 'ambientLightUp' is in eye space.
	inline void RenderGLSLModelTexFog(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuPoint& ambientLightUp, bool lightOn, float alpha=1.f) const;

	// Renders this model using the GLSL shader ModelTexFog.
	// 'ambientLightUp' is in eye space.
	void RenderGLSLModelTexFog(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f, bool useModelBlending=true) const;

	// Renders this model using the GLSL shader ModelTexFogGlobe, using current render states in re.
	// 'globeP' is used for sun over the horizon effect.
	// 'globeP' and 'ambientLightUp' are in eye space.
	void RenderGLSLModelTexFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuPoint& globeP, const AuPoint& ambientLightUp, bool lightOn, float alpha=1.f) const;

	// Renders this model using the GLSL shader ModelTexFogGlobe.
	// 'globeP' is used for sun over the horizon effect.
	// 'globeP' and 'ambientLightUp' are in eye space.
	void RenderGLSLModelTexFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re, const AuPoint& globeP,
			float ambientLightR, float ambientLightG, float ambientLightB, const AuPoint& ambientLightUp, bool lightOn,
			float fogDensity, float fogR, float fogG, float fogB, float alpha=1.f) const;

	// Renders this model using the GLSL shader ModelTexFogRingworld, using current render states in re.
	void RenderGLSLModelTexFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			float shadowPanelSunAngles[MAX_SUNS], const AuPoint& ambientLightUp, bool lightOn, float alpha=1.f) const;

	// Renders this model using the GLSL shader ModelTexFogRingworld.
	void RenderGLSLModelTexFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			float shadowPanelSunAngles[MAX_SUNS], float ambientLightR, float ambientLightG, float ambientLightB,
			const AuPoint& ambientLightUp, bool lightOn, float fogDensity, float fogR, float fogG, float fogB,
			float alpha=1.f) const;

	virtual double RoughRadius() const
	{
		return Au3dsGLSLModel::RoughRadius();
	}

	virtual double Scale() const
	{
		return m_GeometryScale;
	}

	unsigned int SeatCount() const
	{
		if(!m_Stations)
			return 0;
		return m_Stations->count();
	}

	AuPoint SeatLoc(int index) const	// Location of a seat.
	{
		if(!m_Stations)
			return AuPoint(0,0);
		if(index < 0 || index >= m_Stations->count())
			return AuPoint(0,0);
		return (*m_Stations)[index].Loc();
	}

	Au3dsGLSLStation::POSE SeatPose(int index) const	// Default pose.
	{
		if(!m_Stations)
			return Au3dsGLSLStation::Invalid;
		if(index < 0 || index >= m_Stations->count())
			return Au3dsGLSLStation::Invalid;
		return (*m_Stations)[index].DefaultPose();
	}

	virtual void SetTexture(GLuint texture)
	{
		m_Texture = texture;
	}

	virtual GLuint Texture() const
	{
		return m_Texture;
	}

	int WeaponCount() const
	{
		if(m_Weapons)
			return m_Weapons->count();
		return 0;
	}

	AuPoint WeaponDirection(int index) const
	{
		if(!m_Weapons)
			return AuPoint(0,0);
		if(index < 0 || index >= m_Weapons->count())
			return AuPoint(0,0);
		return (*m_Weapons)[index].Dir();
	}

	AuPoint WeaponLocation(int index) const
	{
		if(!m_Weapons)
			return AuPoint(0,0);
		if(index < 0 || index >= m_Weapons->count())
			return AuPoint(0,0);
		return (*m_Weapons)[index].Loc();
	}

	void SetFileName(const QString& sFileName)
	{
		m_FileName = sFileName;
	}

	const AuPoint& GetEyePos() const
	{
		return m_EyePt;
	}

	AuPoint GetConnectPos() const
	{
		if(m_Objects.count())
			return AuPoint(m_Objects[0].m_ConnectPt.X(), m_Objects[0].m_ConnectPt.Y(), m_Objects[0].m_ConnectPt.Z());
		return AuPoint(0,0);
	}

	AuPoint GetFootScalePos() const
	{
		if(m_Objects.count())
		{
			QVector<Au3dsGLSLVGroup>::const_iterator itVGroup;
			for(itVGroup = m_Objects[0].m_VertexGroups.constBegin(); itVGroup != m_Objects[0].m_VertexGroups.constEnd(); itVGroup++)
			{
				if((*itVGroup).m_bFoot)
					return AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
		}
		return AuPoint(0,0);
	}

	AuPoint GetHandScalePos() const
	{
		if(m_Objects.count())
		{
			QVector<Au3dsGLSLVGroup>::const_iterator itVGroup;
			for(itVGroup = m_Objects[0].m_VertexGroups.constBegin(); itVGroup != m_Objects[0].m_VertexGroups.constEnd(); itVGroup++)
			{
				if((*itVGroup).m_bHand)
					return AuPoint((*itVGroup).m_ScalePt.X(), (*itVGroup).m_ScalePt.Y(), (*itVGroup).m_ScalePt.Z());
			}
		}
		return AuPoint(0,0);
	}

	double GetOverallScale() const
	{
		return m_OverallScale;
	}

	double GetLowerThickness() const
	{
		return m_LowerThickness;
	}

	double GetLowerLength() const
	{
		return m_LowerLength;
	}

	virtual quint64 MemoryUsage() const;

	virtual void ResourceReport(QDataStream& out) const;

private:
	QString					m_FileName;		// A unique id for this instance.
	// Keep display lists seperate
	int						m_LitModelCount;// Number of Gl model ids with lights on
	QList<Au3dsGLSLVAO>			m_Models;		// Gl models
	QList<Au3dsGLSLVAO>			m_ModelsLit;	// Gl models with lights on
	GLuint					m_Texture;		// Gl texture
	double					m_OverallScale;
	double					m_IScale;			// Independent scaling
	double					m_UTOffset;			// U TopSkin Offset
	double					m_VTOffset;			// V TopSkin Offset
	double					m_UBOffset;			// U BottomSkin Offset
	double					m_VBOffset;			// V BottomSkin Offset
	double					m_UpperThickness;	// Scaling thickness of upper piece
	double					m_UpperLength;		// Scaling length of upper piece
	double					m_LowerThickness;	// Scaling thickness of lower piece
	double					m_LowerLength;		// Scaling length of lower piece
	double					m_HandWingLength;	// Hand wing length
	AuPoint					m_Offset;			// Connection point Offset
	int						m_pBone;			// Previous Bone
	int						m_tBone;			// This Bone
	int						m_nBone;			// Next Bone
	QColor					m_tColor;			// Top Color
	float					m_tShininess;		// Top Shininess
	QColor					m_bColor;			// Bottom Color
	float					m_bShininess;		// Bottom Shininess
	float					m_Glow;				// Glow
	float					m_Glowdim;			// Glow dim
	AuPoint					m_EyePt;

	QList<Au3dsGLSLTriTexList>		m_IntModels;
	QList<Au3dsGLSLTriTexList>		m_IntModelsLit;
	Au3dsGLSLModelLoader*		m_LoaderJob;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// A class for efficiently storing the variables used on a 3ds model.

class Au3dsGLSLModelDefinition
{
public:

	Au3dsGLSLModelDefinition(double iScale=1, double overallScale=1, double uTOffset=0, double vTOffset=0, double uBOffset=0,
			double vBOffset=0, int pb=0, int tb=0, int nb=0, const AuPoint& pOffset = AuPoint(0,0,0), double uL=0, double uT=0,
			double lL=0, double lT=0, double hwL=0, QColor tC=QColor(255,255,255,0), float tS=0, QColor bC=QColor(255,255,255,0),
			float bS=0, float glow=0, float glowdim=0, bool bCreate=false, bool bSkip=false, bool bArrangement=false,
			GLuint destBlendFunc=GL_ONE_MINUS_SRC_ALPHA);
	~Au3dsGLSLModelDefinition();

	void CopyDefinition(Au3dsGLSLModelDefinition* pModelDef);

	quint64 MemoryUsage() const;

	inline void SetPrevBone(int iPrevBone)
	{
		m_iPrevBone = iPrevBone;
	}

	inline void SetThisBone(int iThisBone)
	{
		m_iThisBone = iThisBone;
	}

	inline void SetNextBone(int iNextBone)
	{
		m_iNextBone = iNextBone;
	}

	inline void SetOffset(const AuPoint& pOffset)
	{
		m_pOffset = pOffset;
	}

	inline void SetLowerLength(double lLength)
	{
		m_rlLength = lLength;
	}

	inline void Skip(bool bSkip)
	{
		m_bSkip = bSkip;
	}

public:
	double			m_rScale;			// Individual scale
	double			m_rOverallScale;	// Overall scale
	double			m_ruTOffset;		// Top skin U Texture coordinate offset
	double			m_rvTOffset;		// Top skin V Texture coordinate offset
	double			m_ruBOffset;		// Bottom skin U Texture coordinate offset
	double			m_rvBOffset;		// Bottom skin V Texture coordinate offset
	int				m_iPrevBone;		// ID of previous bone
	int				m_iThisBone;		// ID of this bone
	int				m_iNextBone;		// ID of next bone
	AuPoint			m_pOffset;			// Location offset to place model at
	double			m_ruLength;			// Upper body piece length
	double			m_ruThickness;		// Upper body piece thickness
	double			m_rlLength;			// Lower body piece length
	double			m_rlThickness;		// Lower body piece thickness
	double			m_rhwLength;			// Hand wing length
	bool			m_bSkip;			// True if skipping the creation of a model that has a parent object
	GLuint			m_idestBlendFunc;	// Type of blend function to use
	// Variables used for texture appearance
	QColor			m_fTColor;			// Top color
	QColor			m_fBColor;			// Bottom color
	float			m_fTShininess;		// Top shininess
	float			m_fBShininess;		// Bottom shininess
	float			m_fGlow;			// Glow amount
	float			m_fGlowdim;			// Glow dimness
	bool			m_bCreate;			// True if we are to use the values listed for texture appearance
	bool			m_bArrangement;		// True if we are searching for a predefined arrangement
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class Au3dsGLSLModelLoader : public QRunnable
{
private:
	bool					m_IsDone;			// true when this runnable is done executing
	bool					m_LightsOn;
	Au3dsGLSLModelInstance		m_Model;			// The model that is loaded.
	double					m_Scale;

public:
	Au3dsGLSLModelLoader(const QString& fileName, bool lightsOn, double scale);

	inline const QString& FileName() const
	{
		return m_Model.FileName();
	}

	inline bool IsDone() const
	{
		return m_IsDone;
	}

	inline const Au3dsGLSLModelInstance* Model() const
	{
		return &m_Model;
	}

	virtual void run();
};

#endif // Au3dsGLSL_H
