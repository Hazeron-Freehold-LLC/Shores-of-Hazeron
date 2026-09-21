//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglsl.h,v 1.180 2022/02/19 18:29:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// OpenGL Shader Language programs.

#ifndef AuGLSL_H
#define AuGLSL_H

#include "augl.h"
#include "aumatrix4x4.h"
#include "auscface.h"
#include "autriangle.h"

#define MAX_POINT_LIGHTS 80				// Max point/spot light sources supported by this code. Shaders can be configured for less.
										// On Romulus, 93 caused a shader compile error; 92 was ok. 80 allows for a margin.
#define MAX_SUNS 3						// Max sun light sources supported by shaders.
#define MAX_SHIELD_HIT_EFFECTS 20		// Same number as in shield shaders.
#define MAX_BONES 61
#define MAX_ANIMAL_BONES 50				// Default human uses 24.
#define MAX_SPACECRAFT_OFFSETS 150
#define MAX_CLOTHING_TEXTURES 15		// Mac texture limit is 16. This allows 15 clothing textures plus one decal texture.

#define d_FogColorMax					(1.f)					// Maximum R G or B value of a fog color.
#define d_FogDensityFactor				(.8f)					// 1 for 1/exp(d) fog. About .5 for 1/exp(d^2) fog.

class AuGLEffects;
class AuGLSLScBase;
class AuScTriRenderList;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Texture palette supported by Sc shaders.
// The Sc fragment shaders support 16 concurrent textures. This is how we use them. Most of them are arbitrarily selected by the
// user through the use of a custom palette of other textures.

enum SC_TEXTURE_INDEX
{
	SCTEXI_User0,					// Imported 3ds model texture lands here.
	SCTEXI_Material,				// Changes based on construction material.
	SCTEXI_User1,
	SCTEXI_User2,
	SCTEXI_User3,
	SCTEXI_User4,
	SCTEXI_User5,
	SCTEXI_User6,
	SCTEXI_User7,
	SCTEXI_User8,
	SCTEXI_User9,
	SCTEXI_User10,
	SCTEXI_User11,
	SCTEXI_User12,
	SCTEXI_User13,
	SCTEXI_Count					// LAST ITEM. Concurrent texture limit on Mac is 16. We'll stop there.
};

class AuTexturePalette
{
public:
	GLuint								m_Tex[SCTEXI_Count];	// Texture palette.

public:	// Constructors
	inline AuTexturePalette()
	{
		for(int t=0; t<SCTEXI_Count; t++)
			m_Tex[t] = 0;
	}

	inline AuTexturePalette(const AuTexturePalette& o)
	{
		for(int t=0; t<SCTEXI_Count; t++)
			m_Tex[t] = o.m_Tex[t];
	}

public:	// Operators
	inline const AuTexturePalette& operator=(const AuTexturePalette& o)
	{
		for(int t=0; t<SCTEXI_Count; t++)
			m_Tex[t] = o.m_Tex[t];
		return *this;
	}

	inline GLuint& operator[](unsigned int index)
	{
		Q_ASSERT(index < SCTEXI_Count);
		return m_Tex[index];
	}

	inline GLuint operator[](unsigned int index) const
	{
		Q_ASSERT(index < SCTEXI_Count);
		return m_Tex[index];
	}

public:	// Methods
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// OpenGL Version
#if defined(Q_OS_WIN)
// OpenGL 3.2
#include <QOpenGLFunctions_3_2_Core>
#define GLFuncName						"OpenGL 3.2"
#define GLMajorVersion					3
#define GLMinorVersion					2

class AUGL_DLL AuOpenGLFunctions : public QOpenGLFunctions_3_2_Core
{
};

#else
// AuGLVer.h file is created by make. See project Makefile.rel or Makefile.
#if defined(QT_NO_DEBUG)
#include ".r/AuGLVer.h"
#else
#include ".d/AuGLVer.h"
#endif

#if defined(AUGL41)
// OpenGL 4.1
#include <QOpenGLFunctions_4_1_Core>
#define GLFuncName						"OpenGL 4.1"
#define GLMajorVersion					4
#define GLMinorVersion					1

class AUGL_DLL AuOpenGLFunctions : public QOpenGLFunctions_4_1_Core
{
};

#elif defined(AUGL33)
// OpenGL 3.3
#include <QOpenGLFunctions_3_3_Core>
#define GLFuncName						"OpenGL 3.3"
#define GLMajorVersion					3
#define GLMinorVersion					3

class AUGL_DLL AuOpenGLFunctions : public QOpenGLFunctions_3_3_Core
{
};

#elif defined(AUGL32)
// OpenGL 3.2
#include <QOpenGLFunctions_3_2_Core>
#define GLFuncName						"OpenGL 3.2"
#define GLMajorVersion					3
#define GLMinorVersion					2

class AUGL_DLL AuOpenGLFunctions : public QOpenGLFunctions_3_2_Core
{
};

#endif	// AUGLnn

#endif	// Q_OS_WIN

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// NOTE: OpenGL context must be current when calling all NnnGL() functions.

class AUGL_DLL AuGLVAO
{
private:
	GLuint								m_BufferCount;			// Count of buffer ids in m_Buffers.
	GLuint*								m_Buffers;				// Array of GLuints.
	AuOpenGLFunctions*					m_GLf;
	GLuint								m_VAO;					// Vertex Array Object identifier assigned by OpenGL
	GLuint								m_Vertices;				// Number of vertices in this vertex array object.

public:
	inline AuGLVAO()
	{
		m_BufferCount = 0;
		m_Buffers = 0;
		m_GLf = 0;
		m_VAO = 0;
		m_Vertices = 0;
	}

	inline AuGLVAO(const AuGLVAO& c)
	{
		m_BufferCount = c.m_BufferCount;
		if(m_BufferCount && c.m_Buffers)
		{
			m_Buffers = new GLuint[m_BufferCount];
			memcpy(m_Buffers, c.m_Buffers, m_BufferCount * sizeof(GLuint));
		}
		else
			m_Buffers = 0;
		m_GLf = c.m_GLf;
		m_VAO = c.m_VAO;
		m_Vertices = c.m_Vertices;
	}

	inline ~AuGLVAO()
	{
		if(m_Buffers)
		{
			delete [] m_Buffers;
			m_Buffers = 0;
			m_BufferCount = 0;
		}
	}

	inline GLuint Buffer(GLuint bufferIndex) const
	{
		//Q_ASSERT(bufferIndex < m_BufferCount);
		return m_Buffers[bufferIndex];
	}

	// Creates this vertex array object from a list of triangles.
	// The target shaders for this VAO are 'Body', 'BodyFog', 'BodyFogRingworld' and 'BodyRingworld'.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'tris' is the list of triangle data needed to construct the VAO.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// DOES NOTHING if this VAO has already been created or if 'tris' is empty.
	void CreateForBody(AuOpenGLFunctions& GLf, const QList<AuTriBody>& tris, GLenum usage);

	// Creates this vertex array object from a list of points.
	// The target shader for this VAO is 'Line'.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'verts' is the list of vertex data needed to construct the VAO.
	// 'closeBoundary' is true to insure the last vertex matches the first vertex by adding one if necessary.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// DOES NOTHING if this VAO has already been created or if 'verts' is empty.
	void CreateForLines(AuOpenGLFunctions& GLf, const AuPointArray& verts, bool closeBoundary, GLenum usage);

	// Creates this vertex array object from a list of generic triangles.
	// The target shader for this VAO is 'Model'.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'tris' is the list of triangle data needed to construct the VAO.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// DOES NOTHING if this VAO has already been created or if 'tris' is empty.
	void CreateForModel(AuOpenGLFunctions& GLf, const QList<AuTriTexNS>& tris, GLenum usage);

	// Creates this vertex array object from a list of generic triangles.
	// The target shaders for this VAO are 'ModelTex', 'ModelTexColor', 'ModelTexFog', 'ModelTexFogGlobe'.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'tris' is the list of triangle data needed to construct the VAO.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// DOES NOTHING if this VAO has already been created or if 'tris' is empty.
	void CreateForModelTexFog(AuOpenGLFunctions& GLf, const QList<AuTriTexNS>& tris, GLenum usage);

	// Creates this vertex array object from a list of triangles.
	// The target shaders for this VAO are 'Sc' and 'ScFog'.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'tris' is the list of triangle data needed to construct the VAO.
	// 'themeColors' are used on painted surfaces that indicate such.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// 'effect' is an AuScFace::EFFECT. Only triangles with the effect are used to build the VAO. If none, resulting VAO will be 0.
	// DOES NOTHING if this VAO has already been created or if 'tris' is empty.
	void CreateForSc(AuOpenGLFunctions& GLf, const AuScTriRenderList& tris, GLenum usage, quint8 effect);

	// Creates this vertex array object from a list of points.
	// The target shader for this VAO is 'SpriteSize'.
	// This works for the 'SpriteActualSize' shader but the alpha value is treated as a size.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'verts' is the list of vertex data needed to construct the VAO.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// DOES NOTHING if this VAO has already been created or if 'verts' is empty.
	void CreateForSprites(AuOpenGLFunctions& GLf, const QList<AuPointAlpha>& verts, GLenum usage);

	// This works for the 'SpriteSizeColor' shader.
	// vert location is the sprite location.
	// vert.Glow() is the sprite color, including alpha.
	// vert.EffectIntensity() is the sprite size.
	void CreateForSpriteSizeColor(AuOpenGLFunctions& GLf, const QList<AuScPoint>& verts, GLenum usage);

	// Creates this vertex array object from a list of generic triangles.
	// The target shader for this VAO is 'Texture'.
	// An OpenGL context must be current when this function is called.
	// Calls CreateGL, SelectGL and DeselectGL.
	// 'tris' is the list of triangle data needed to construct the VAO.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// DOES NOTHING if this VAO has already been created or if 'tris' is empty.
	void CreateForTexture(AuOpenGLFunctions& GLf, const QList<AuTriTex>& tris, GLenum usage);

	// Creates the OpenGL VAO. Be sure to call DeleteGL some time.
	inline void CreateGL(AuOpenGLFunctions& GLf, GLuint vertices, GLuint buffers)
	{
		m_GLf = &GLf;

		//Q_ASSERT(vertices);
		//Q_ASSERT(!m_VAO);
		m_GLf->glGenVertexArrays(1, &m_VAO);
		m_Vertices = vertices;

		//Q_ASSERT(buffers);
		//Q_ASSERT(!m_Buffers);
		m_BufferCount = buffers;
		m_Buffers = new GLuint[m_BufferCount];
		m_GLf->glGenBuffers(m_BufferCount, m_Buffers);
	}

	// Deletes this VAO.
	inline void DeleteGL()
	{
		//Q_ASSERT(m_GLf);
		//Q_ASSERT(m_Buffers);
		m_GLf->glDeleteBuffers(m_BufferCount, m_Buffers);
		m_BufferCount = 0;
		delete [] m_Buffers;
		m_Buffers = 0;

		//Q_ASSERT(m_VAO);
		m_GLf->glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}

	// Selects no vertex array as the current OpenGL vertex array.
	inline void DeselectGL() const
	{
		m_GLf->glBindVertexArray(0);
	}

	// Draws the current OpenGL vertex array, telling it my number of vertices. It's a good idea to call SelectGL before calling
	// this function. However, after calling SelectGL, you probably need to enable one or more vertex attribute array by calling
	// glEnableVertexAttribArray.
	inline void DrawGL() const
	{
		for(GLuint buffer=0; buffer<m_BufferCount; buffer++)
			m_GLf->glEnableVertexAttribArray(buffer);

		m_GLf->glDrawArrays(GL_TRIANGLES, 0, m_Vertices);

		for(GLuint buffer=0; buffer<m_BufferCount; buffer++)
			m_GLf->glDisableVertexAttribArray(buffer);
	}

	inline void DrawGL(GLenum mode) const
	{
		for(GLuint buffer=0; buffer<m_BufferCount; buffer++)
			m_GLf->glEnableVertexAttribArray(buffer);

		m_GLf->glDrawArrays(mode, 0, m_Vertices);

		for(GLuint buffer=0; buffer<m_BufferCount; buffer++)
			m_GLf->glDisableVertexAttribArray(buffer);
	}

	inline const AuGLVAO& operator=(const AuGLVAO& c)
	{
		m_BufferCount = c.m_BufferCount;
		if(m_Buffers)
			delete [] m_Buffers;
		if(m_BufferCount && c.m_Buffers)
		{
			m_Buffers = new GLuint[m_BufferCount];
			memcpy(m_Buffers, c.m_Buffers, m_BufferCount * sizeof(GLuint));
		}
		else
			m_Buffers = 0;
		m_GLf = c.m_GLf;
		m_VAO = c.m_VAO;
		m_Vertices = c.m_Vertices;

		return *this;
	}

	// Selects this vertex array as the current OpenGL vertex array.
	inline void SelectGL() const
	{
		m_GLf->glBindVertexArray(m_VAO);
	}

	inline GLuint VAO() const
	{
		return m_VAO;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGLVAOAlpha : public AuGLVAO
{
private:
	float								m_Alpha;

public:	// Constructors
	inline AuGLVAOAlpha()
	{
		m_Alpha = 1;
	}

	inline AuGLVAOAlpha(float alpha)
	{
		m_Alpha = alpha;
	}

	inline AuGLVAOAlpha(const AuGLVAOAlpha& c) : AuGLVAO(c)
	{
		m_Alpha = c.m_Alpha;
	}

public:	// Operators

public:	// Methods
	inline float Alpha() const
	{
		return m_Alpha;
	}

	inline void SetAlpha(float a)
	{
		m_Alpha = a;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuScVAO
{
private:
	QHash<quint8, AuGLVAO>				m_VAOs;					// Key is AuScFace::EFFECT.

public:
	inline AuScVAO()
	{
	}

	void Create(AuOpenGLFunctions& GLf, const AuScTriRenderList& tris, GLenum usage);

	// 'hullEffAppl' says how to apply the EFF_Hull effect: 0=not at all, 1=lit by environment, 2=glow
	void DrawSc(const AuGLSLScBase& shader, const GLuint effectTex[EFF_Count], int hullEffAppl);

	void DeleteGL();

	// Returns true if any VAOs have been made.
	inline bool VAO() const
	{
		return !m_VAOs.empty();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLShader is a helper class for creating shader programs.

class AUGL_DLL AuGLSLShader
{
protected:
	GLuint								m_Shader;				// Shader identifier assigned by OpenGL glCreateShader.

public:
	// Constructor loads and compiles the shader source code into an OpenGL shader object.
	// OpenGL context must be current when this function is called.
	// 'maxLights' can be 0 when compiling a fragment shader because it does not apply to them.
	AuGLSLShader(AuOpenGLFunctions& GLf, GLenum shaderType, const QString& shaderName, unsigned int maxLights=0);

	// Returns the numeric identifier assigned by OpenGL glCreateShader when this shader was constructed.
	inline GLuint Id() const
	{
		return m_Shader;
	}

protected:
	void DumpInfoLog(AuOpenGLFunctions& GLf);

	QString GetInfoLog(AuOpenGLFunctions& GLf);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLProgramOpt
{
private:
	unsigned int						m_MaxLights;

public:
	inline AuGLSLProgramOpt(unsigned int maxLights)
	{
		m_MaxLights = maxLights;
	}

	inline unsigned int MaxLights() const
	{
		return m_MaxLights;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLProgram is the base class for all GLSL shader program classes.

class AUGL_DLL AuGLSLProgram
{
protected:
	GLuint								m_Program;				// Program identifier assigned by OpenGL glCreateProgram.
	AuOpenGLFunctions*					m_GLf;					// Captured when Build function called.

public:
	AuGLSLProgram();
	virtual ~AuGLSLProgram();

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt)
	{
		Q_UNUSED(opt);
		m_GLf = &GLf;
	}

	// Deselects shaders from the GL context to restore default OpenGL behavior.
	// OpenGL context must be current when this function is called.
	virtual void Deselect();

	virtual QString Name() const = 0;

	inline void OutputGLError(const QString& hint) const
	{
		Q_UNUSED(hint);
#if !defined(QT_NO_DEBUG)
		GLenum errorno = glGetError();
		if(errorno != GL_NO_ERROR)
		{
			if(hint.isEmpty())
				qWarning() << "error: AuGLSLProgram::OutputGLError" << Name() << "errorno" << errorno;
			else
				qWarning() << "error: AuGLSLProgram::OutputGLError" << Name() << "errorno" << errorno << "hint" << hint;
		}
#endif
	}

	// Selects this GLSL program into the current context. Call Deselect when finished.
	// Initializes projection, suns and lights, if they are not valid.
	void Select();

protected:
	void DumpProgramInfoLog() const;

	// 'attrib' is the name of an attrib variable.
	GLint GetAttribLoc(const QString& attrib) const;

	// 'uniform' is the name of a uniform variable.
	GLint GetUniformLoc(const QString& uniform) const;

	QString GetProgramInfoLog() const;

	inline static void SetUniformMatrix(AuOpenGLFunctions* glf, GLint uniform, const AuMatrix4x4& m)
	{
		// Must create a matrix of float values.
		GLfloat mf[16];
		const double* C = m.GLMatrix();
		for(int i=0; i<16; i++)
			mf[i] = C[i];

		// Pass the matrix of floats to the shader.
		glf->glUniformMatrix4fv(uniform, 1, GL_FALSE, mf);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLColor is a simple shader program that renders each vertex using an unmodulated color. This was the prototype for working
// out the GLSL interfaces. This shader provides a concise example of GLSL. SGLTest uses this shader to render a simple object.

class AUGL_DLL AuGLSLColor : public AuGLSLProgram
{
private:
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

public:
	AuGLSLColor()
	{
		m_pmv = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Color");
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLLine shader colors every fragment the same uniform color.

class AUGL_DLL AuGLSLLine : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

public:
	AuGLSLLine()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_pmv = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Line");
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLLineFog shader colors every fragment the same uniform color.

class AUGL_DLL AuGLSLLineFog : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogParm;				// Location of 'fogParm' uniform variable.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

public:
	AuGLSLLineFog()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_fogColor = -1;
		m_fogParm = -1;
		m_pmv = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Line Fog");
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	inline void SetFog(float startDepth, float endDepth, float r, float g, float b, float a=1) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform4f(m_fogColor, r,g,b,a);
		m_GLf->glUniform2f(m_fogParm, startDepth, endDepth-startDepth);
	}

	inline void SetFogColor(float r, float g, float b, float a=1) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform4f(m_fogColor, r,g,b,a);
	}

	inline void SetFogDepth(float startDepth, float endDepth) const
	{
		m_GLf->glUniform2f(m_fogParm, startDepth, endDepth-startDepth);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearStar shader is used for rendering nearby stars.

class AUGL_DLL AuGLSLNearStar : public AuGLSLProgram
{
private:
	GLint								m_opacity;				// Opacity of the star.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLNearStar()
	{
		m_OldTexture0 = 0;
		m_opacity = -1;
		m_pmv = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Near Star");
	}

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetOpacity(float opacity) const
	{
		m_GLf->glUniform1f(m_opacity, opacity);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLFarStar shader is used for rendering nearby stars.

class AUGL_DLL AuGLSLFarStar : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_size;					// Location of 'size' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLFarStar()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_size = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Star");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetSize(float size) const
	{
		m_GLf->glUniform1f(m_size, size);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLGalaxyStar shader is used for rendering nearby stars.

class AUGL_DLL AuGLSLGalaxyStar : public AuGLSLProgram
{
private:
	GLint								m_intenDist;			// Location of 'intenDist' uniform variable.
	GLint								m_intenRanges;			// Location of 'intenRanges' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_size;					// Location of 'size' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLGalaxyStar()
	{
		m_intenDist = -1;
		m_intenRanges = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_size = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Galaxy Star");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// 'minIntenDist' is the distance at which the intensity is weakest. Should be farther than maxIntenDist.
	// 'maxIntenDist' is the distance at which the intensity is strongest. Should be less than minIntenDist.
	inline void SetIntensityDistance(float minIntenDist, float maxIntenDist) const
	{
		m_GLf->glUniform2f(m_intenDist, minIntenDist, maxIntenDist);
	}

	// Set range of particle intensity across the range established in SetIntensityDistance.
	inline void SetIntensityRanges(float minInten, float maxInten, float minTrans, float maxTrans) const
	{
		m_GLf->glUniform4f(m_intenRanges, minInten, maxInten, minTrans, maxTrans);
	}

	inline void SetSize(float size) const
	{
		m_GLf->glUniform1f(m_size, size);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection model view matrix.
	inline void SetProjection(const AuMatrix4x4& p) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLGalaxyCloud is used to render the soft blue patches around the galaxy stars in the sky box.

class AUGL_DLL AuGLSLGalaxyCloud : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLGalaxyCloud()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Galaxy Cloud");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF());
	}

	inline void SetColor(float r, float g, float b)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b)
			return;
		m_GLf->glUniform3f(m_color, r,g,b);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection model view matrix.
	inline void SetProjection(const AuMatrix4x4& p) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNebula is used to render the nebula in the sky box.

class AUGL_DLL AuGLSLNebula : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLNebula()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Nebula");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLSkyBox : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLSkyBox()
	{
		m_mv = -1;
		m_OldTexture0 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_CUBE_MAP, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Sky Box");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Opacity value causes sky box to fade to black as opacity approaches 0.
	inline void SetOpacity(float opacity)
	{
		m_GLf->glUniform1f(m_opacity, opacity);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_CUBE_MAP, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearGlobe is used to render a sun.

class AUGL_DLL AuGLSLNearGlobe : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearGlobe()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Globe");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearRingworld is used to render a sun.

class AUGL_DLL AuGLSLNearRingworld : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearRingworld()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Ringworld");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearGlobe is used to render a sun.

class AUGL_DLL AuGLSLNearGlobeFog : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearGlobeFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Globe Fog");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearGlobe is used to render a sun.

class AUGL_DLL AuGLSLNearRingworldFog : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearRingworldFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Ringworld Fog");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearGlobeLava is used to render a sun.

class AUGL_DLL AuGLSLNearGlobeLava : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearGlobeLava()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Globe Lava");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearGlobeLavaFog is used to render a sun.

class AUGL_DLL AuGLSLNearGlobeLavaFog : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearGlobeLavaFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Globe Lava Fog");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearRingworldGlow is used to render a sun.

class AUGL_DLL AuGLSLNearRingworldGlow : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearRingworldGlow()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Ringworld Glow");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};


//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLNearRingworldGlowFog is used to render a sun.

class AUGL_DLL AuGLSLNearRingworldGlowFog : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLNearRingworldGlowFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Near Ringworld Glow Fog");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLGlobe : public AuGLSLProgram
{
private:
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

public:
	AuGLSLGlobe()
	{
		m_OldLookup = 0;
		m_OldSnow = 0;
		m_pmv = -1;
		m_texLookup = -1;
		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Globe");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
			texUnit++;
		}
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 1);								// Configure uniform 'texSnow' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);								// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 2;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);							// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);			// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLGlobeFlora : public AuGLSLProgram
{
private:
	GLint								m_OldGrass;				// Previous texture binding for restore upon deselect.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texGrass;				// Location of 'texGrass' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

public:
	AuGLSLGlobeFlora()
	{
		m_OldLookup = 0;
		m_OldSnow = 0;
		m_pmv = -1;

		m_texGrass = -1;
		m_OldGrass = -1;

		m_texLookup = -1;
		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Globe Flora");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
			texUnit++;
		}
	}

	inline void SetGrassTexture(GLuint texName) const
	{
		GLuint texUnit = 2;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texGrass, texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 1);						// Configure uniform 'texSnow' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);						// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);		// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLRingworldFlora : public AuGLSLProgram
{
private:
	GLint								m_OldGrass;				// Previous texture binding for restore upon deselect.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texGrass;				// Location of 'texGrass' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

public:
	AuGLSLRingworldFlora()
	{
		m_OldLookup = 0;
		m_OldSnow = 0;
		m_pmv = -1;

		m_texGrass = -1;
		m_OldGrass = -1;

		m_texLookup = -1;
		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Flora");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
			texUnit++;
		}
	}

	inline void SetGrassTexture(GLuint texName) const
	{
		GLuint texUnit = 2;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texGrass, texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 1);						// Configure uniform 'texSnow' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);						// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);		// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLGlobeLit : public AuGLSLProgram
{
private:
	GLint								m_altNoiseFactor;		// Multiplier for altitude noise.
	GLint								m_clipCos;				// Location of 'clipCos' uniform variable.
	GLint								m_clipUp;				// Location of 'clipUp' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldNoise;				// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texNoise;				// Location of 'texNoise' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLGlobeLit()
	{
		m_altNoiseFactor = -1;
		m_clipCos = -1;
		m_clipUp = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldLookup = -1;
		m_OldNoise = -1;
		m_OldSnow = -1;
		m_pmv = -1;
		m_texLookup = -1;
		m_texNoise = -1;
		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Globe Lit");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
			texUnit++;
		}
	}

	inline void SetAltitudeNoiseFactor(float altNoise) const
	{
		m_GLf->glUniform1f(m_altNoiseFactor, altNoise);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipCos, float clipUpX, float clipUpY, float clipUpZ)
	{
		m_GLf->glUniform1f(m_clipCos, clipCos);
		m_GLf->glUniform3f(m_clipUp, clipUpX, clipUpY, clipUpZ);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	inline void SetNoiseTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texNoise, 1);								// Configure uniform 'texNoise' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 2);								// Configure uniform 'texSnow' for texture unit 2.
		m_GLf->glActiveTexture(GL_TEXTURE2);							// Select texture unit 2 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);								// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLGlobeLitFlora : public AuGLSLProgram
{
private:
	GLint								m_altNoiseFactor;		// Multiplier for altitude noise.
	GLint								m_clipCos;				// Location of 'clipCos' uniform variable.
	GLint								m_clipUp;				// Location of 'clipUp' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldGrass;				// Previous texture binding for restore upon deselect.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldNoise;				// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texGrass;				// Location of 'texGrass' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texNoise;				// Location of 'texNoise' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLGlobeLitFlora()
	{
		m_altNoiseFactor = -1;
		m_clipCos = -1;
		m_clipUp = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldLookup = -1;
		m_OldNoise = -1;
		m_OldSnow = -1;
		m_pmv = -1;

		m_texGrass = -1;
		m_OldGrass = -1;

		m_texLookup = -1;
		m_texNoise = -1;
		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Globe Lit Flora");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
			texUnit++;
		}
	}

	inline void SetAltitudeNoiseFactor(float altNoise) const
	{
		m_GLf->glUniform1f(m_altNoiseFactor, altNoise);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipCos, float clipUpX, float clipUpY, float clipUpZ)
	{
		m_GLf->glUniform1f(m_clipCos, clipCos);
		m_GLf->glUniform3f(m_clipUp, clipUpX, clipUpY, clipUpZ);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	inline void SetGrassTexture(GLuint texName) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texGrass, texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetNoiseTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texNoise, 1);								// Configure uniform 'texNoise' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 2);								// Configure uniform 'texSnow' for texture unit 2.
		m_GLf->glActiveTexture(GL_TEXTURE2);							// Select texture unit 2 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);						// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 4;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);		// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLRingworldLitFlora : public AuGLSLProgram
{
private:
	GLint								m_altNoiseFactor;		// Multiplier for altitude noise.
	GLint								m_clipCenter;			// Location of 'clipCenter' uniform variable.
	GLint								m_clipDist;				// Location of 'clipDist' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldGrass;				// Previous texture binding for restore upon deselect.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldNoise;				// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texGrass;				// Location of 'texGrass' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texNoise;				// Location of 'texNoise' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLRingworldLitFlora()
	{
		m_altNoiseFactor = -1;
		m_clipCenter = -1;
		m_clipDist = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldLookup = -1;
		m_OldNoise = -1;
		m_OldSnow = -1;
		m_pmv = -1;

		m_texGrass = -1;
		m_OldGrass = -1;

		m_texLookup = -1;
		m_texNoise = -1;
		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Lit Flora");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTerrain[ter]);
			texUnit++;
		}
	}

	inline void SetAltitudeNoiseFactor(float altNoise) const
	{
		m_GLf->glUniform1f(m_altNoiseFactor, altNoise);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipDist, float clipCenterX, float clipCenterY, float clipCenterZ)
	{
		m_GLf->glUniform1f(m_clipDist, clipDist);
		m_GLf->glUniform3f(m_clipCenter, clipCenterX, clipCenterY, clipCenterZ);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	inline void SetGrassTexture(GLuint texName) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texGrass, texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetNoiseTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texNoise, 1);								// Configure uniform 'texNoise' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 2);								// Configure uniform 'texSnow' for texture unit 2.
		m_GLf->glActiveTexture(GL_TEXTURE2);							// Select texture unit 2 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);						// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 4;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);		// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLTerrainLit : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_altNoiseFactor;		// Multiplier for altitude noise.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldNoise;				// Previous texture binding for restore upon deselect.
	GLint								m_OldPavement[7];		// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texNoise;				// Location of 'texNoise' uniform variable.
	GLint								m_texPavement[7];		// Location of 'texPavement' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_cityAmbient;			// Location of 'cityAmbient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLTerrainLit()
	{
		m_altNoiseFactor = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldLookup = -1;
		m_OldNoise = -1;
		m_OldSnow = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_texLookup = -1;
		m_texNoise = -1;

		for(int pave=0; pave<7; pave++)
		{
			m_texPavement[pave] = -1;
			m_OldPavement[pave] = -1;
		}

		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		for(int pave=0; pave<7; pave++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldPavement[pave]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Terrain Lit");
	}

	virtual void Select(const AuMatrix4x4& projection);

	inline void SetAltitudeNoiseFactor(float altNoise) const
	{
		m_GLf->glUniform1f(m_altNoiseFactor, altNoise);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetCityAmbientLight(float r, float g, float b)
	{
		m_GLf->glUniform3f(m_cityAmbient, r, g, b);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	inline void SetNoiseTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texNoise, 1);								// Configure uniform 'texNoise' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 2);								// Configure uniform 'texSnow' for texture unit 2.
		m_GLf->glActiveTexture(GL_TEXTURE2);							// Select texture unit 2 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);								// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetPavementPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 8;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int pave=0; pave<7; pave++)
		{
			m_GLf->glUniform1i(m_texPavement[pave], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[pave]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLTerrainLitFlora : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_altNoiseFactor;		// Multiplier for altitude noise.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldGrass;				// Previous texture binding for restore upon deselect.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldNoise;				// Previous texture binding for restore upon deselect.
	GLint								m_OldPavement[7];		// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texGrass;				// Location of 'texGrass' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texNoise;				// Location of 'texNoise' uniform variable.
	GLint								m_texPavement[7];		// Location of 'texPavement' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_cityAmbient;			// Location of 'cityAmbient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLTerrainLitFlora()
	{
		m_altNoiseFactor = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldLookup = -1;
		m_OldNoise = -1;
		m_OldSnow = -1;
		m_p = -1;
		m_ProjectionValid = false;

		m_texGrass = -1;
		m_OldGrass = -1;

		m_texLookup = -1;
		m_texNoise = -1;

		for(int pave=0; pave<7; pave++)
		{
			m_texPavement[pave] = -1;
			m_OldPavement[pave] = -1;
		}

		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		for(int pave=0; pave<7; pave++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldPavement[pave]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Terrain Lit Flora");
	}

	virtual void Select(const AuMatrix4x4& projection);

	inline void SetAltitudeNoiseFactor(float altNoise) const
	{
		m_GLf->glUniform1f(m_altNoiseFactor, altNoise);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetCityAmbientLight(float r, float g, float b)
	{
		m_GLf->glUniform3f(m_cityAmbient, r, g, b);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	inline void SetGrassTexture(GLuint texName) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texGrass, texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	inline void SetNoiseTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texNoise, 1);								// Configure uniform 'texNoise' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 2);								// Configure uniform 'texSnow' for texture unit 2.
		m_GLf->glActiveTexture(GL_TEXTURE2);							// Select texture unit 2 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);						// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetPavementPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 9;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int pave=0; pave<7; pave++)
		{
			m_GLf->glUniform1i(m_texPavement[pave], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[pave]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 4;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLRingworldTerrainLitFlora : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_altNoiseFactor;		// Multiplier for altitude noise.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldGrass;				// Previous texture binding for restore upon deselect.
	GLint								m_OldLookup;			// Previous texture binding for restore upon deselect.
	GLint								m_OldNoise;				// Previous texture binding for restore upon deselect.
	GLint								m_OldPavement[7];		// Previous texture binding for restore upon deselect.
	GLint								m_OldSnow;				// Previous texture binding for restore upon deselect.
	GLint								m_OldTerrain[5];		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texGrass;				// Location of 'texGrass' uniform variable.
	GLint								m_texLookup;			// Location of 'texLookup' uniform variable.
	GLint								m_texNoise;				// Location of 'texNoise' uniform variable.
	GLint								m_texPavement[7];		// Location of 'texPavement' uniform variable.
	GLint								m_texSnow;				// Location of 'texSnow' uniform variable.
	GLint								m_texTerrain[5];		// Location of 'texTerrain' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_cityAmbient;			// Location of 'cityAmbient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRingworldTerrainLitFlora()
	{
		m_altNoiseFactor = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldLookup = -1;
		m_OldNoise = -1;
		m_OldSnow = -1;
		m_p = -1;
		m_ProjectionValid = false;

		m_texGrass = -1;
		m_OldGrass = -1;

		m_texLookup = -1;
		m_texNoise = -1;

		for(int pave=0; pave<7; pave++)
		{
			m_texPavement[pave] = -1;
			m_OldPavement[pave] = -1;
		}

		m_texSnow = -1;

		for(int ter=0; ter<5; ter++)
		{
			m_texTerrain[ter] = -1;
			m_OldTerrain[ter] = -1;
		}

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		GLuint texUnit = GL_TEXTURE0;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldLookup);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldNoise);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSnow);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldGrass);
		texUnit++;

		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTerrain[ter]);
			texUnit++;
		}

		for(int pave=0; pave<7; pave++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldPavement[pave]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Terrain Lit Flora");
	}

	virtual void Select(const AuMatrix4x4& projection);

	inline void SetAltitudeNoiseFactor(float altNoise) const
	{
		m_GLf->glUniform1f(m_altNoiseFactor, altNoise);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetCityAmbientLight(float r, float g, float b)
	{
		m_GLf->glUniform3f(m_cityAmbient, r, g, b);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	inline void SetGrassTexture(GLuint texName) const
	{
		GLuint texUnit = 3;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texGrass, texUnit);				// Configure uniform 'texTerrainHigh' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	inline void SetNoiseTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texNoise, 1);								// Configure uniform 'texNoise' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	inline void SetSnowTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSnow, 2);								// Configure uniform 'texSnow' for texture unit 2.
		m_GLf->glActiveTexture(GL_TEXTURE2);							// Select texture unit 2 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTerrainLookup(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texLookup, 0);						// Configure uniform 'texLookup' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetPavementPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 9;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int pave=0; pave<7; pave++)
		{
			m_GLf->glUniform1i(m_texPavement[pave], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[pave]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}

	inline void SetTerrainPalette(GLuint texName[7]) const
	{
		GLuint texUnit = 4;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		for(int ter=0; ter<5; ter++)
		{
			m_GLf->glUniform1i(m_texTerrain[ter], texUnit);	// Configure uniform 'texTerrainHigh' for texture unit.
			m_GLf->glActiveTexture(texBase);					// Select texture unit 0 as active.
			m_GLf->glBindTexture(GL_TEXTURE_2D, texName[ter]);	// Bind the texture to the current texture unit.
			texBase++;
			texUnit++;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLLava shader is used for rendering lava surface

class AUGL_DLL AuGLSLLava : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texBump;				// Location of 'texBump' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLLava()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texBump = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Lava");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetBumpMap(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texBump, 1);								// Configure uniform 'texBump' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLAtmosphere : public AuGLSLProgram
{
private:
	GLint								m_atmAlpha;				// Location of 'atmAlpha' uniform variable.
	GLint								m_atmColor;				// Location of 'atmColor' uniform variable.
	GLint								m_atmRadius;			// Location of 'atmRadius' uniform variable.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

	//--------------------
	// Lighting parameters
	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLAtmosphere()
	{
		m_atmAlpha = -1;
		m_atmColor = -1;
		m_atmRadius = -1;
		m_pmv = -1;

		// Lighting parameters
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Atmosphere");
	}

	inline void SetAtmosphereAlpha(float a) const
	{
		m_GLf->glUniform1f(m_atmAlpha, a);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmRadius, radius);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLAtmosphereRingworld : public AuGLSLProgram
{
private:
	GLint								m_atmAlpha;				// Location of 'atmAlpha' uniform variable.
	GLint								m_atmAltMSL;			// Location of 'atmAltMSL' uniform variable.
	GLint								m_atmColor;				// Location of 'atmColor' uniform variable.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

	//--------------------
	// Lighting parameters
	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLAtmosphereRingworld()
	{
		m_atmAlpha = -1;
		m_atmAltMSL = -1;
		m_atmColor = -1;
		m_pmv = -1;

		// Lighting parameters
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Atmosphere Ringworld");
	}

	inline void SetAtmosphereAlpha(float a) const
	{
		m_GLf->glUniform1f(m_atmAlpha, a);
	}

	inline void SetAtmosphereAltitudeMSL(float atmAltMSL) const
	{
		m_GLf->glUniform1f(m_atmAltMSL, atmAltMSL);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLCloud : public AuGLSLProgram
{
private:
	GLint								m_cloudColor;			// Location of 'cloudColor' uniform variable.
	GLint								m_cloudLayerRadius;		// Location of 'cloudLayerRadius' uniform variable.
	GLint								m_cloudLiftMax;			// Location of 'cloudLiftMax' uniform variable.
	GLint								m_cloudSunset;			// Location of 'cloudSunset uniform variable.
	GLint								m_maxWx;				// Location of 'maxWx' uniform variable.
	GLint								m_minWx;				// Location of 'minWx' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texWx;				// Location of 'texWx' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunSunsetColor[MAX_SUNS];	// Location of 'sun.sunsetColor' lighting uniform variable.
	//--------------------

public:
	AuGLSLCloud()
	{
		m_cloudColor = -1;
		m_cloudLayerRadius = -1;
		m_cloudLiftMax = -1;
		m_cloudSunset = -1;
		m_maxWx = -1;
		m_minWx = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_texWx = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunSunsetColor[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Cloud");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetCloudColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_cloudColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetCloudLayerRadius(float radius) const
	{
		m_GLf->glUniform1f(m_cloudLayerRadius, radius);
	}

	// Cloud layer vertices are elevated by this much at maximum cloud density.
	inline void SetCloudLiftMax(float liftMax) const
	{
		m_GLf->glUniform1f(m_cloudLiftMax, liftMax);
	}

	// 'sunsetFactor' is the amount of sunset lighting to apply, 0 = none to 1 = max.
	inline void SetCloudSunset(float sunsetFactor) const
	{
		m_GLf->glUniform1f(m_cloudSunset, sunsetFactor);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float sunsetColorR, float sunsetColorG, float sunsetColorB)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform3f(m_sunSunsetColor[sun], sunsetColorR, sunsetColorG, sunsetColorB);
	}

	// Set saturation pressure and vapor pressure maximums.
	inline void SetWxMaximums(float spMax, float vpMax) const
	{
		m_GLf->glUniform2f(m_maxWx, spMax, vpMax);
	}

	// Set saturation pressure and vapor pressure minimums.
	inline void SetWxMinimums(float spMin, float vpMin) const
	{
		m_GLf->glUniform2f(m_minWx, spMin, vpMin);
	}

	inline void SetWxOffset(float spOffset, float vpOffset) const
	{
		m_GLf->glUniform2f(m_offWx, spOffset, vpOffset);
	}

	inline void SetWxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texWx, 0);								// Configure uniform 'texWx' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLCloudRingworld : public AuGLSLProgram
{
private:
	GLint								m_cloudColor;			// Location of 'cloudColor' uniform variable.
	GLint								m_cloudLayerRadius;		// Location of 'cloudLayerRadius' uniform variable.
	GLint								m_cloudLiftMax;			// Location of 'cloudLiftMax' uniform variable.
	//GLint								m_cloudSunset;			// Location of 'cloudSunset uniform variable.
	GLint								m_maxWx;				// Location of 'maxWx' uniform variable.
	GLint								m_minWx;				// Location of 'minWx' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texWx;				// Location of 'texWx' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLCloudRingworld()
	{
		m_cloudColor = -1;
		m_cloudLayerRadius = -1;
		m_cloudLiftMax = -1;
		//m_cloudSunset = -1;
		m_maxWx = -1;
		m_minWx = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_texWx = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Cloud Ringworld");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetCloudColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_cloudColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetCloudLayerRadius(float radius) const
	{
		m_GLf->glUniform1f(m_cloudLayerRadius, radius);
	}

	// Cloud layer vertices are elevated by this much at maximum cloud density.
	inline void SetCloudLiftMax(float liftMax) const
	{
		m_GLf->glUniform1f(m_cloudLiftMax, liftMax);
	}

	// 'sunsetFactor' is the amount of sunset lighting to apply, 0 = none to 1 = max.
	//inline void SetCloudSunset(float sunsetFactor) const
	//{
		//m_GLf->glUniform1f(m_cloudSunset, sunsetFactor);
	//}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	// Set saturation pressure and vapor pressure maximums.
	inline void SetWxMaximums(float spMax, float vpMax) const
	{
		m_GLf->glUniform2f(m_maxWx, spMax, vpMax);
	}

	// Set saturation pressure and vapor pressure minimums.
	inline void SetWxMinimums(float spMin, float vpMin) const
	{
		m_GLf->glUniform2f(m_minWx, spMin, vpMin);
	}

	inline void SetWxOffset(float spOffset, float vpOffset) const
	{
		m_GLf->glUniform2f(m_offWx, spOffset, vpOffset);
	}

	inline void SetWxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texWx, 0);								// Configure uniform 'texWx' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarGlobe : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_atmosphereRadius;		// Location of 'atmosphereRadius' uniform variable.
	GLint								m_lowMaxWx;				// Location of 'lowMaxWx' uniform variable.
	GLint								m_lowMinWx;				// Location of 'lowMinWx' uniform variable.
	GLint								m_lowTexWx;				// Location of 'lowTexWx' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture2;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

public:
	AuGLSLFarGlobe()
	{
		m_atmosphereColor = -1;
		m_atmosphereRadius = -1;
		m_lowMaxWx = -1;
		m_lowMinWx = -1;
		m_lowTexWx = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_OldTexture2 = 0;
		m_pmv = -1;
		m_texPlanet = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE2);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture2);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Globe");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE2);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture2);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmosphereRadius, radius);
	}

	// Set saturation pressure and vapor pressure maximums.
	inline void SetLowWxMaximums(float spMax, float vpMax) const
	{
		m_GLf->glUniform2f(m_lowMaxWx, spMax, vpMax);
	}

	// Set saturation pressure and vapor pressure minimums.
	inline void SetLowWxMinimums(float spMin, float vpMin) const
	{
		m_GLf->glUniform2f(m_lowMinWx, spMin, vpMin);
	}

	inline void SetLowWxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_lowTexWx, 1);								// Configure uniform 'lowTexWx' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetWxOffset(float spOffset, float vpOffset) const
	{
		m_GLf->glUniform2f(m_offWx, spOffset, vpOffset);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarGlobeLit : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_atmosphereRadius;		// Location of 'atmosphereRadius' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLFarGlobeLit()
	{
		m_atmosphereColor = -1;
		m_atmosphereRadius = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_texPlanet = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Globe Lit");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmosphereRadius, radius);
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarGlobeLitClouds : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_atmosphereRadius;		// Location of 'atmosphereRadius' uniform variable.
	GLint								m_lowMaxWx;				// Location of 'lowMaxWx' uniform variable.
	GLint								m_lowMinWx;				// Location of 'lowMinWx' uniform variable.
	GLint								m_lowTexWx;				// Location of 'lowTexWx' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLFarGlobeLitClouds()
	{
		m_atmosphereColor = -1;
		m_atmosphereRadius = -1;
		m_lowMaxWx = -1;
		m_lowMinWx = -1;
		m_lowTexWx = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_pmv = -1;
		m_texPlanet = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Globe Lit Clouds");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmosphereRadius, radius);
	}

	// Set saturation pressure and vapor pressure maximums.
	inline void SetLowWxMaximums(float spMax, float vpMax) const
	{
		m_GLf->glUniform2f(m_lowMaxWx, spMax, vpMax);
	}

	// Set saturation pressure and vapor pressure minimums.
	inline void SetLowWxMinimums(float spMin, float vpMin) const
	{
		m_GLf->glUniform2f(m_lowMinWx, spMin, vpMin);
	}

	inline void SetLowWxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_lowTexWx, 1);								// Configure uniform 'lowTexWx' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetWxOffset(float spOffset, float vpOffset) const
	{
		m_GLf->glUniform2f(m_offWx, spOffset, vpOffset);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarGlobeLitLava : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_atmosphereRadius;		// Location of 'atmosphereRadius' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLFarGlobeLitLava()
	{
		m_atmosphereColor = -1;
		m_atmosphereRadius = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_texPlanet = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Globe Lit Lava");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmosphereRadius, radius);
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarGlobeLitLavaClouds : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_atmosphereRadius;		// Location of 'atmosphereRadius' uniform variable.
	GLint								m_lowMaxWx;				// Location of 'lowMaxWx' uniform variable.
	GLint								m_lowMinWx;				// Location of 'lowMinWx' uniform variable.
	GLint								m_lowTexWx;				// Location of 'lowTexWx' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLFarGlobeLitLavaClouds()
	{
		m_atmosphereColor = -1;
		m_atmosphereRadius = -1;
		m_lowMaxWx = -1;
		m_lowMinWx = -1;
		m_lowTexWx = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_pmv = -1;
		m_texPlanet = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Globe Lit Lava Clouds");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmosphereRadius, radius);
	}

	// Set saturation pressure and vapor pressure maximums.
	inline void SetLowWxMaximums(float spMax, float vpMax) const
	{
		m_GLf->glUniform2f(m_lowMaxWx, spMax, vpMax);
	}

	// Set saturation pressure and vapor pressure minimums.
	inline void SetLowWxMinimums(float spMin, float vpMin) const
	{
		m_GLf->glUniform2f(m_lowMinWx, spMin, vpMin);
	}

	inline void SetLowWxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_lowTexWx, 1);								// Configure uniform 'lowTexWx' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetWxOffset(float spOffset, float vpOffset) const
	{
		m_GLf->glUniform2f(m_offWx, spOffset, vpOffset);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarRingworldLit : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLFarRingworldLit()
	{
		m_atmosphereColor = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_texPlanet = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Ringworld Lit");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFarRingworldLitClouds : public AuGLSLProgram
{
private:
	GLint								m_atmosphereColor;		// Location of 'atmosphereColor' uniform variable.
	GLint								m_lowMaxWx;				// Location of 'lowMaxWx' uniform variable.
	GLint								m_lowMinWx;				// Location of 'lowMinWx' uniform variable.
	GLint								m_lowTexWx;				// Location of 'lowTexWx' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_texPlanet;			// Location of 'texPlanet' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLFarRingworldLitClouds()
	{
		m_atmosphereColor = -1;
		m_lowMaxWx = -1;
		m_lowMinWx = -1;
		m_lowTexWx = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_pmv = -1;
		m_texPlanet = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Far Ringworld Lit Clouds");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
	}

	// Set ambient starlight level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Alpha value of this color establishes the transparency of the atmosphere.
	inline void SetAtmosphereColor(const QColor& color) const
	{
		m_GLf->glUniform4f(m_atmosphereColor, color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	// Set saturation pressure and vapor pressure maximums.
	inline void SetLowWxMaximums(float spMax, float vpMax) const
	{
		m_GLf->glUniform2f(m_lowMaxWx, spMax, vpMax);
	}

	// Set saturation pressure and vapor pressure minimums.
	inline void SetLowWxMinimums(float spMin, float vpMin) const
	{
		m_GLf->glUniform2f(m_lowMinWx, spMin, vpMin);
	}

	inline void SetLowWxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_lowTexWx, 1);								// Configure uniform 'lowTexWx' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetPlanetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texPlanet, 0);							// Configure uniform 'texPlanet' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetWxOffset(float spOffset, float vpOffset) const
	{
		m_GLf->glUniform2f(m_offWx, spOffset, vpOffset);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLFlora shader is used for rendering plants.

class AUGL_DLL AuGLSLFlora : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_plantM;				// Location of 'plantM' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_vForce;				// Location of 'vForce' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLFlora()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_plantM = -1;
		m_tex = -1;
		m_vForce = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Flora");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Force vector applied to flora vertices.
	// This vector is in eye space.
	inline void SetForceVector(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_vForce, x, y, z);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity.
	// 'alpha' is overall alpha for entire plant.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'plantM' is plant placement matrix on the globe.
	inline void SetPlantMatrix(const AuMatrix4x4& plantM) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_plantM, plantM);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLFloraRingworld shader is used for rendering plants.

class AUGL_DLL AuGLSLFloraRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_plantM;				// Location of 'plantM' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_vForce;				// Location of 'vForce' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLFloraRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_plantM = -1;
		m_tex = -1;
		m_vForce = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Flora Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Force vector applied to flora vertices.
	// This vector is in eye space.
	inline void SetForceVector(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_vForce, x, y, z);
	}

	// Set ambient light color/intensity.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'plantM' is plant placement matrix on the globe.
	inline void SetPlantMatrix(const AuMatrix4x4& plantM) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_plantM, plantM);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAquaticFlora shader is used for rendering plants.

class AUGL_DLL AuGLSLAquaticFlora : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_plantM;				// Location of 'plantM' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_vForce;				// Location of 'vForce' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAquaticFlora()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_plantM = -1;
		m_tex = -1;
		m_vForce = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Flora");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Force vector applied to flora vertices.
	// This vector is in eye space.
	inline void SetForceVector(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_vForce, x, y, z);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity.
	// 'alpha' is overall alpha for entire plant.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'plantM' is plant placement matrix on the globe.
	inline void SetPlantMatrix(const AuMatrix4x4& plantM) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_plantM, plantM);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAquaticFloraRingworld shader is used for rendering plants.

class AUGL_DLL AuGLSLAquaticFloraRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_plantM;				// Location of 'plantM' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_vForce;				// Location of 'vForce' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAquaticFloraRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_plantM = -1;
		m_tex = -1;
		m_vForce = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Flora Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Force vector applied to flora vertices.
	// This vector is in eye space.
	inline void SetForceVector(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_vForce, x, y, z);
	}

	// Set ambient light color/intensity.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'plantM' is plant placement matrix on the globe.
	inline void SetPlantMatrix(const AuMatrix4x4& plantM) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_plantM, plantM);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLRock shader is used for rendering rocks.

class AUGL_DLL AuGLSLRock : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_rockM;				// Location of 'rockM' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRock()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_rockM = -1;
		m_tex = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Rock");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set ambient light color/intensity.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'rockM' is rock placement matrix on the globe.
	inline void SetRockMatrix(const AuMatrix4x4& rockM) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_rockM, rockM);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLRockRingworld shader is used for rendering rocks.

class AUGL_DLL AuGLSLRockRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_rockM;				// Location of 'rockM' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRockRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_rockM = -1;
		m_tex = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Rock Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set ambient light color/intensity.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'rockM' is rock placement matrix on the globe.
	inline void SetRockMatrix(const AuMatrix4x4& rockM) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_rockM, rockM);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLSpriteActualSize : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLSpriteActualSize()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Sprite Actual Size");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLSpriteSize : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_size;					// Location of 'size' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLSpriteSize()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_size = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Sprite Size");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSize(float size) const
	{
		m_GLf->glUniform1f(m_size, size);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLSpriteSizeColor : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLSpriteSizeColor()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Sprite Size Color");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLFlag : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texFlag;				// Location of 'texFlag' uniform variable.
	GLint								m_wind;					// Location of 'wind' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLFlag()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texFlag = -1;
		m_wind = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Flag");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	inline void SetFlagTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texFlag, 0);								// Configure uniform 'texFlag' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetWind(float windForce, float flagPeriod)
	{
		m_GLf->glUniform2f(m_wind, windForce, flagPeriod);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLRingworldFlag : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texFlag;				// Location of 'texFlag' uniform variable.
	GLint								m_wind;					// Location of 'wind' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRingworldFlag()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texFlag = -1;
		m_wind = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Ringworld Flag");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	inline void SetFlagTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texFlag, 0);								// Configure uniform 'texFlag' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetWind(float windForce, float flagPeriod)
	{
		m_GLf->glUniform2f(m_wind, windForce, flagPeriod);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLShadowSquare is used to render a sun.

class AUGL_DLL AuGLSLShadowSquare : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	//--------------------

public:
	AuGLSLShadowSquare()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Shadow Square");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLRingworldHull is used to render a sun.

class AUGL_DLL AuGLSLRingworldHull : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLRingworldHull()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Hull");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLRingworldHullFog

class AUGL_DLL AuGLSLRingworldHullFog : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.
	//--------------------

public:
	AuGLSLRingworldHullFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Hull Fog");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLRingworldStatue

class AUGL_DLL AuGLSLRingworldStatue : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRingworldStatue()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Statue");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLRingworldTerrainStatue

class AUGL_DLL AuGLSLRingworldTerrainStatue : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRingworldTerrainStatue()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Ringworld Terrain Statue");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLRain : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRain()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_pmv = -1;

		// Lighting parameters
		m_ambient = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Rain");
	}

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLRainRingworld : public AuGLSLProgram
{
private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLRainRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_pmv = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Rain Ringworld");
	}

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLSnow : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_offset;				// Location of 'offset' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_size;					// Location of 'size' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLSnow()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_offset = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_size = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Snow");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Offset position for snow flake sprite. This is the center of the snow flake in object space.
	// This location is in object space.
	inline void SetFlakePosition(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_offset, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in object space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	inline void SetSize(float size) const
	{
		m_GLf->glUniform1f(m_size, size);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLSnowRingworld : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_offset;				// Location of 'offset' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_size;					// Location of 'size' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLSnowRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_offset = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_size = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Snow Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Offset position for snow flake sprite. This is the center of the snow flake in object space.
	// This location is in object space.
	inline void SetFlakePosition(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_offset, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	inline void SetSize(float size) const
	{
		m_GLf->glUniform1f(m_size, size);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBlackHole is used to render a black hole.

class AUGL_DLL AuGLSLBlackHole : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_rotSky;				// Location of 'rotSky' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texSky;				// Location of 'texSky' uniform variable.

public:
	AuGLSLBlackHole()
	{
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_pmv = -1;
		m_rotSky = -1;
		m_tex = -1;
		m_texSky = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_CUBE_MAP, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Black Hole");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &m_OldTexture1);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSkyBoxRotation(const AuMatrix4x4& mref) const
	{
		SetUniformMatrix(m_GLf, m_rotSky, mref);
	}

	inline void SetSkyBoxTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSky, 1);								// Configure uniform 'texSky' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_CUBE_MAP, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBlackHoleSphere is used to render a black hole sphere.

class AUGL_DLL AuGLSLBlackHoleSphere : public AuGLSLProgram
{
private:
	GLint								m_pmv;					// Location of 'pmv' uniform variable.

public:
	AuGLSLBlackHoleSphere()
	{
		m_pmv = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual QString Name() const
	{
		return QString("Black Hole Sphere");
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLPhotonSphere is used to render a photon sphere around a black hole.

class AUGL_DLL AuGLSLPhotonSphere : public AuGLSLProgram
{
private:
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLPhotonSphere()
	{
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Photon Sphere");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLModelTexFog shader is used for rendering models.

class AUGL_DLL AuGLSLModelTexFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLModelTexFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Model Tex Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLModelTexFogGlobe shader is used for rendering models.

class AUGL_DLL AuGLSLModelTexFogGlobe : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLModelTexFogGlobe()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Model Tex Fog Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLModelTexFogRingworld shader is used for rendering models.

class AUGL_DLL AuGLSLModelTexFogRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLModelTexFogRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Model Tex Fog Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLModelTex shader is used for rendering models.

class AUGL_DLL AuGLSLModelTex : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLModelTex()
	{
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Model Tex");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLModelTexColor shader is used for rendering models in a design studio. Overall color is modulated by a color.

class AUGL_DLL AuGLSLModelTexColor : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_glow;					// Location of 'glow' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texPaletteHigh;		// Location of 'texPaletteHigh' uniform variable.
	GLint								m_texPaletteLow;		// Location of 'texPaletteLow' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLModelTexColor()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_glow = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texPaletteHigh = -1;
		m_texPaletteLow = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Model Tex Color");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// Set ambient light color/intensity.
	inline void SetGlow(float glowR, float glowG, float glowB) const
	{
		m_GLf->glUniform3f(m_glow, glowR, glowG, glowB);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Sets the high and low resolution texture palettes.
	inline void SetTexture(GLuint texPaletteHigh, GLuint texPaletteLow) const
	{
		m_GLf->glUniform1i(m_texPaletteHigh, 1);						// Configure uniform 'texPaletteHigh' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texPaletteHigh);			// Bind the texture to the current texture unit.

		m_GLf->glUniform1i(m_texPaletteLow, 0);						// Configure uniform 'texPaletteLow' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texPaletteLow);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLModel shader is used for rendering models in a design studio with no texture, only a color.

class AUGL_DLL AuGLSLModel : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_glow;					// Location of 'glow' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLModel()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_glow = -1;
		m_mv = -1;
		m_p = -1;
		m_ProjectionValid = false;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Model");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// Set ambient light color/intensity.
	inline void SetGlow(float glowR, float glowG, float glowB) const
	{
		m_GLf->glUniform3f(m_glow, glowR, glowG, glowB);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLTexture shader is used for rendering textured polygons with no light modulation.

class AUGL_DLL AuGLSLTexture : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLTexture()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Texture");
	}

	virtual void Select();

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLTexBar shader renders textured polygons with no light modulation. Left side is one color and right side is another color.

class AUGL_DLL AuGLSLTexBar : public AuGLSLProgram
{
private:
	GLint								m_colorLeft;			// Location of 'colorLeft' uniform variable.
	GLint								m_colorRight;			// Location of 'colorRight' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.
	GLint								m_threshold;			// Location of 'threshold' uniform variable.

public:
	AuGLSLTexBar()
	{
		m_colorLeft = -1;
		m_colorRight = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
		m_texOffset = -1;
		m_threshold = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Tex Bar");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColorLeft(float r, float g, float b, float a=1) const
	{
		m_GLf->glUniform4f(m_colorLeft, r,g,b,a);
	}

	inline void SetColorRight(float r, float g, float b, float a=1) const
	{
		m_GLf->glUniform4f(m_colorRight, r,g,b,a);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTextureOffset(float vs, float vt) const
	{
		m_GLf->glUniform2f(m_texOffset, vs, vt);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetThreshold(float t) const
	{
		m_GLf->glUniform1f(m_threshold, t);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLTexOffset shader is used for rendering textured polygons with no light modulation. A 2d offset is applied to texture
// coords.

class AUGL_DLL AuGLSLTexOffset : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_discardAlpha;			// Location of 'discardAlpha' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

public:
	AuGLSLTexOffset()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_discardAlpha = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;
		m_texOffset = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Tex Offset");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	inline void SetDiscardAlpha(float discardAlpha) const
	{
		m_GLf->glUniform1f(m_discardAlpha, discardAlpha);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetTextureOffset(float vs, float vt) const
	{
		m_GLf->glUniform2f(m_texOffset, vs, vt);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Shield bubble with no current hits on it.

class AUGL_DLL AuGLSLShield : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_btr;					// Location of 'btr' uniform variable.
	GLint								m_btrScram;				// Location of 'btrScram' uniform variable.
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_fbl;					// Location of 'fbl' uniform variable.
	GLint								m_fblScram;				// Location of 'fblScram' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_scale;				// Location of 'scale' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.
	GLint								m_texScram;				// Location of 'texScram' uniform variable.

public:
	AuGLSLShield()
	{
		m_btr = -1;
		m_btrScram = -1;
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_fbl = -1;
		m_fblScram = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_pmv = -1;
		m_scale = -1;
		m_tex = -1;
		m_texOffset = -1;
		m_texScram = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Shield");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture1);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetScale(float sx, float sy, float sz) const
	{
		m_GLf->glUniform3f(m_scale, sx,sy,sz);
	}

	inline void SetSideIntensities(float front, float bottom, float left, float back, float top, float right)
	{
		m_GLf->glUniform3f(m_fbl, front, bottom, left);
		m_GLf->glUniform3f(m_btr, back, top, right);
	}

	inline void SetSideScram(bool front, bool bottom, bool left, bool back, bool top, bool right)
	{
		m_GLf->glUniform3f(m_fblScram, front, bottom, left);
		m_GLf->glUniform3f(m_btrScram, back, top, right);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetTextureOffset(float s, float t) const
	{
		m_GLf->glUniform2f(m_texOffset, s,t);
	}

	inline void SetTextureScram(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texScram, 1);								// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Shield bubble with current hits on it.

class AUGL_DLL AuGLSLShieldHits : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_btr;					// Location of 'btr' uniform variable.
	GLint								m_btrScram;				// Location of 'btrScram' uniform variable.
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_fbl;					// Location of 'fbl' uniform variable.
	GLint								m_fblScram;				// Location of 'fblScram' uniform variable.
	GLint								m_hitDur[MAX_SHIELD_HIT_EFFECTS];	// Location of 'hitDur' uniform variable array.
	GLint								m_hitPos[MAX_SHIELD_HIT_EFFECTS];	// Location of 'hitPos' uniform variable array.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_scale;				// Location of 'scale' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.
	GLint								m_texScram;				// Location of 'texScram' uniform variable.

public:
	AuGLSLShieldHits()
	{
		m_btr = -1;
		m_btrScram = -1;
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_fbl = -1;
		m_fblScram = -1;
		for(int s=0; s<MAX_SHIELD_HIT_EFFECTS; s++)
		{
			m_hitDur[s] = -1;
			m_hitPos[s] = -1;
		}
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_scale = -1;
		m_tex = -1;
		m_texOffset = -1;
		m_texScram = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Shield Hits");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	inline void SetHits(float hitPos[MAX_SHIELD_HIT_EFFECTS*4], float hitDur[MAX_SHIELD_HIT_EFFECTS])
	{
		float* hp = hitPos;
		float* hd = hitDur;
		for(int s=0; s<MAX_SHIELD_HIT_EFFECTS; s++)
		{
			m_GLf->glUniform4f(m_hitPos[s], *hp, *(hp+1), *(hp+2), *(hp+3));
			m_GLf->glUniform1f(m_hitDur[s], *hd);
			hp += 4;
			hd++;
		}
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetScale(float sx, float sy, float sz) const
	{
		m_GLf->glUniform3f(m_scale, sx,sy,sz);
	}

	inline void SetSideIntensities(float front, float bottom, float left, float back, float top, float right)
	{
		m_GLf->glUniform3f(m_fbl, front, bottom, left);
		m_GLf->glUniform3f(m_btr, back, top, right);
	}

	inline void SetSideScram(bool front, bool bottom, bool left, bool back, bool top, bool right)
	{
		m_GLf->glUniform3f(m_fblScram, front, bottom, left);
		m_GLf->glUniform3f(m_btrScram, back, top, right);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetTextureOffset(float s, float t) const
	{
		m_GLf->glUniform2f(m_texOffset, s,t);
	}

	inline void SetTextureScram(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texScram, 1);								// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLGasGiant is used to render the globe of a gas giant.

class AUGL_DLL AuGLSLGasGiant : public AuGLSLProgram
{
private:
	GLint								m_atmColor;				// Location of 'atmColor' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLGasGiant()
	{
		m_atmColor = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Gas Giant");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetAtmosphereColor(float r, float g, float b, float a=1.f) const
	{
		m_GLf->glUniform4f(m_atmColor, r,g,b,a);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLGasGiantLayer is used to render the globe of a gas giant.

class AUGL_DLL AuGLSLGasGiantLayer : public AuGLSLProgram
{
private:
	GLint								m_atmColor;				// Location of 'atmColor' uniform variable.
	GLint								m_atmRadius;			// Location of 'atmRadius' uniform variable.
	GLint								m_offWx;				// Location of 'offWx' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLGasGiantLayer()
	{
		m_atmColor = -1;
		m_atmRadius = -1;
		m_offWx = -1;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_tex = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Gas Giant Layer");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetAtmosphereColor(float r, float g, float b, float a=1.f) const
	{
		m_GLf->glUniform4f(m_atmColor, r,g,b,a);
	}

	// Incoming vertices are multiplied by this radius.
	// The atmosphere globe VAO is created with radius 1 so it can be used for cloud layers in addition to the outer atmosphere
	// layer, which this shader draws.
	inline void SetAtmosphereRadius(float radius) const
	{
		m_GLf->glUniform1f(m_atmRadius, radius);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// Set eye location relative to the object rendered.
	inline void SetWeatherOffset(float offWx) const
	{
		m_GLf->glUniform1f(m_offWx, offWx);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSurfaceTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLWarpTunnel : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_rotationAngle;		// Location of 'rotationAngle' uniform variable.
	GLint								m_texSky;				// Location of 'texSky' uniform variable.

public:
	AuGLSLWarpTunnel()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_rotationAngle = -1;
		m_texSky = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_CUBE_MAP, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Warp Tunnel");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetRotationAngle(float angle) const
	{
		m_GLf->glUniform1f(m_rotationAngle, angle);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSky, 0);								// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_CUBE_MAP, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLSplat shader is used for rendering textured polygons with no light modulation.

class AUGL_DLL AuGLSLSplat : public AuGLSLProgram
{
	bool								m_ColorValid;
	float								m_ColR, m_ColG, m_ColB, m_ColA;

private:
	GLint								m_color;				// Location of 'color' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_pmv;					// Location of 'pmv' uniform variable.
	GLint								m_splatScale;			// Location of 'splatScale' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.

public:
	AuGLSLSplat()
	{
		m_color = -1;
		m_ColorValid = false;
		m_ColR = 0;
		m_ColG = 0;
		m_ColB = 0;
		m_ColA = 0;
		m_OldTexture0 = 0;
		m_pmv = -1;
		m_splatScale = -1;
		m_tex = -1;
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Splat");
	}

	virtual void Select()
	{
		AuGLSLProgram::Select();

		// Remember previous texture bindings.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_OldTexture0);
	}

	inline void SetColor(const QColor& color)
	{
		SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetColor(float r, float g, float b, float a=1)
	{
		if(m_ColorValid && m_ColR == r && m_ColG == g && m_ColB == b && m_ColA == a)
			return;
		m_GLf->glUniform4f(m_color, r,g,b,a);
		m_ColR = r;
		m_ColG = g;
		m_ColB = b;
		m_ColA = a;
		m_ColorValid = true;
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'pmv' is projection model view matrix.
	inline void SetProjectionModelView(const AuMatrix4x4& pmv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_pmv, pmv);
	}

	inline void SetSplatScale(float splatScale) const
	{
		m_GLf->glUniform1f(m_splatScale, splatScale);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimal shader is used for rendering animals.

class AUGL_DLL AuGLSLAnimal : public AuGLSLProgram
{
	bool								m_BottomColorValid;
	float								m_BottomColR, m_BottomColG, m_BottomColB, m_BottomColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.
	bool								m_TopColorValid;
	float								m_TopColR, m_TopColG, m_TopColB, m_TopColA;

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_bottomSkinColor;		// Location of 'bottomSkinColor' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldSkinBottom;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinDetail;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinTop;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texSkinBottom;		// Location of 'texSkinBottom' uniform variable.
	GLint								m_texSkinDetail;		// Location of 'texSkinDetail' uniform variable.
	GLint								m_texSkinTop;			// Location of 'texSkinTop' uniform variable.
	GLint								m_topSkinColor;			// Location of 'topSkinColor' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimal()
	{
		m_bottomSkinColor = -1;
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldSkinBottom = 0;
		m_OldSkinDetail = 0;
		m_OldSkinTop = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texSkinBottom = -1;
		m_texSkinDetail = -1;
		m_texSkinTop = -1;
		m_topSkinColor = -1;

		m_BottomColorValid = false;
		m_BottomColR = 0;
		m_BottomColG = 0;
		m_BottomColB = 0;
		m_BottomColA = 0;
		m_LightsValid = false;
		m_SunsValid = false;
		m_TopColorValid = false;
		m_TopColR = 0;
		m_TopColG = 0;
		m_TopColB = 0;
		m_TopColA = 0;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinBottom);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinDetail);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinTop);
		texUnit++;

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSkinBottomColor(const QColor& color)
	{
		SetSkinBottomColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinBottomColor(float r, float g, float b, float a=1)
	{
		if(m_BottomColorValid && m_BottomColR == r && m_BottomColG == g && m_BottomColB == b && m_BottomColA == a)
			return;
		m_GLf->glUniform4f(m_bottomSkinColor, r,g,b,a);
		m_BottomColR = r;
		m_BottomColG = g;
		m_BottomColB = b;
		m_BottomColA = a;
		m_BottomColorValid = true;
	}

	inline void SetSkinBottomTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinBottom, 0);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinDetailTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinDetail, 1);					// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinTopColor(const QColor& color)
	{
		SetSkinTopColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinTopColor(float r, float g, float b, float a=1)
	{
		if(m_TopColorValid && m_TopColR == r && m_TopColG == g && m_TopColB == b && m_TopColA == a)
			return;
		m_GLf->glUniform4f(m_topSkinColor, r,g,b,a);
		m_TopColR = r;
		m_TopColG = g;
		m_TopColB = b;
		m_TopColA = a;
		m_TopColorValid = true;
	}

	inline void SetSkinTopTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinTop, 2);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE2);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalClothing shader is used for rendering clothing on animals.

class AUGL_DLL AuGLSLAnimalClothing : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_clothColor[MAX_CLOTHING_TEXTURES];// Location of 'clothColor' lighting uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldClothing[MAX_CLOTHING_TEXTURES];// Previous texture binding for restore upon deselect.
	GLint								m_OldClothingDecal;		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothing[MAX_CLOTHING_TEXTURES];	// Location of 'texClothing' uniform variable.
	GLint								m_texClothingDecal;		// Location of 'texSkinDetail' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalClothing()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_clothColor[c] = -1;
			m_OldClothing[c] = 0;
			m_texClothing[c] = 0;
		}
		m_OldClothingDecal = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothingDecal = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothingDecal);
		texUnit++;

		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothing[c]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Clothing");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, const QColor& color, GLuint texName) const
	{
		SetClothingTexture(texIndex, color.redF(), color.greenF(), color.blueF(), texName);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, float clothR, float clothG, float clothB, GLuint texName) const
	{
		m_GLf->glUniform3f(m_clothColor[texIndex], clothR, clothG, clothB);

		GLuint texUnit = 1 + texIndex;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texClothing[texIndex], texUnit);	// Configure uniform 'tex' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothingDecalTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothingDecal, 0);				// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalHair shader is used for rendering hair on animals.

class AUGL_DLL AuGLSLAnimalHair : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldHair;				// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texHair;				// Location of 'texHair' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalHair()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldHair = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texHair = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldHair);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Hair");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetHairTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texHair, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalFog shader is used for rendering animals.

class AUGL_DLL AuGLSLAnimalFog : public AuGLSLProgram
{
	bool								m_BottomColorValid;
	float								m_BottomColR, m_BottomColG, m_BottomColB, m_BottomColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.
	bool								m_TopColorValid;
	float								m_TopColR, m_TopColG, m_TopColB, m_TopColA;

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_bottomSkinColor;		// Location of 'bottomSkinColor' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldSkinBottom;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinDetail;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinTop;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texSkinBottom;		// Location of 'texSkinBottom' uniform variable.
	GLint								m_texSkinDetail;		// Location of 'texSkinDetail' uniform variable.
	GLint								m_texSkinTop;			// Location of 'texSkinTop' uniform variable.
	GLint								m_topSkinColor;			// Location of 'topSkinColor' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalFog()
	{
		m_bottomSkinColor = -1;
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldSkinBottom = 0;
		m_OldSkinDetail = 0;
		m_OldSkinTop = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texSkinBottom = -1;
		m_texSkinDetail = -1;
		m_texSkinTop = -1;
		m_topSkinColor = -1;

		m_BottomColorValid = false;
		m_BottomColR = 0;
		m_BottomColG = 0;
		m_BottomColB = 0;
		m_BottomColA = 0;
		m_LightsValid = false;
		m_SunsValid = false;
		m_TopColorValid = false;
		m_TopColR = 0;
		m_TopColG = 0;
		m_TopColB = 0;
		m_TopColA = 0;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinBottom);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinDetail);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinTop);
		texUnit++;

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSkinBottomColor(const QColor& color)
	{
		SetSkinBottomColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinBottomColor(float r, float g, float b, float a=1)
	{
		if(m_BottomColorValid && m_BottomColR == r && m_BottomColG == g && m_BottomColB == b && m_BottomColA == a)
			return;
		m_GLf->glUniform4f(m_bottomSkinColor, r,g,b,a);
		m_BottomColR = r;
		m_BottomColG = g;
		m_BottomColB = b;
		m_BottomColA = a;
		m_BottomColorValid = true;
	}

	inline void SetSkinBottomTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinBottom, 0);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinDetailTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinDetail, 1);					// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinTopColor(const QColor& color)
	{
		SetSkinTopColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinTopColor(float r, float g, float b, float a=1)
	{
		if(m_TopColorValid && m_TopColR == r && m_TopColG == g && m_TopColB == b && m_TopColA == a)
			return;
		m_GLf->glUniform4f(m_topSkinColor, r,g,b,a);
		m_TopColR = r;
		m_TopColG = g;
		m_TopColB = b;
		m_TopColA = a;
		m_TopColorValid = true;
	}

	inline void SetSkinTopTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinTop, 2);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE2);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalClothingFog shader is used for rendering clothing on animals.

class AUGL_DLL AuGLSLAnimalClothingFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_clothColor[MAX_CLOTHING_TEXTURES];// Location of 'clothColor' lighting uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldClothing[MAX_CLOTHING_TEXTURES];// Previous texture binding for restore upon deselect.
	GLint								m_OldClothingDecal;		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothing[MAX_CLOTHING_TEXTURES];	// Location of 'texClothing' uniform variable.
	GLint								m_texClothingDecal;		// Location of 'texSkinDetail' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalClothingFog()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_clothColor[c] = -1;
			m_OldClothing[c] = 0;
			m_texClothing[c] = 0;
		}
		m_OldClothingDecal = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothingDecal = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothingDecal);
		texUnit++;

		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothing[c]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Clothing Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, const QColor& color, GLuint texName) const
	{
		SetClothingTexture(texIndex, color.redF(), color.greenF(), color.blueF(), texName);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, float clothR, float clothG, float clothB, GLuint texName) const
	{
		m_GLf->glUniform3f(m_clothColor[texIndex], clothR, clothG, clothB);

		GLuint texUnit = 1 + texIndex;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texClothing[texIndex], texUnit);	// Configure uniform 'tex' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothingDecalTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothingDecal, 0);				// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalHairFog shader is used for rendering hair on animals.

class AUGL_DLL AuGLSLAnimalHairFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldHair;				// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texHair;				// Location of 'texHair' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalHairFog()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldHair = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texHair = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldHair);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Hair Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetHairTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texHair, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimal shader is used for rendering animals.

class AUGL_DLL AuGLSLAnimalGlobe : public AuGLSLProgram
{
	bool								m_BottomColorValid;
	float								m_BottomColR, m_BottomColG, m_BottomColB, m_BottomColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.
	bool								m_TopColorValid;
	float								m_TopColR, m_TopColG, m_TopColB, m_TopColA;

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_bottomSkinColor;		// Location of 'bottomSkinColor' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldSkinBottom;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinDetail;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinTop;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texSkinBottom;		// Location of 'texSkinBottom' uniform variable.
	GLint								m_texSkinDetail;		// Location of 'texSkinDetail' uniform variable.
	GLint								m_texSkinTop;			// Location of 'texSkinTop' uniform variable.
	GLint								m_topSkinColor;			// Location of 'topSkinColor' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalGlobe()
	{
		m_bottomSkinColor = -1;
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldSkinBottom = 0;
		m_OldSkinDetail = 0;
		m_OldSkinTop = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texSkinBottom = -1;
		m_texSkinDetail = -1;
		m_texSkinTop = -1;
		m_topSkinColor = -1;

		m_BottomColorValid = false;
		m_BottomColR = 0;
		m_BottomColG = 0;
		m_BottomColB = 0;
		m_BottomColA = 0;
		m_LightsValid = false;
		m_SunsValid = false;
		m_TopColorValid = false;
		m_TopColR = 0;
		m_TopColG = 0;
		m_TopColB = 0;
		m_TopColA = 0;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinBottom);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinDetail);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinTop);
		texUnit++;

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSkinBottomColor(const QColor& color)
	{
		SetSkinBottomColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinBottomColor(float r, float g, float b, float a=1)
	{
		if(m_BottomColorValid && m_BottomColR == r && m_BottomColG == g && m_BottomColB == b && m_BottomColA == a)
			return;
		m_GLf->glUniform4f(m_bottomSkinColor, r,g,b,a);
		m_BottomColR = r;
		m_BottomColG = g;
		m_BottomColB = b;
		m_BottomColA = a;
		m_BottomColorValid = true;
	}

	inline void SetSkinBottomTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinBottom, 0);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinDetailTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinDetail, 1);					// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinTopColor(const QColor& color)
	{
		SetSkinTopColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinTopColor(float r, float g, float b, float a=1)
	{
		if(m_TopColorValid && m_TopColR == r && m_TopColG == g && m_TopColB == b && m_TopColA == a)
			return;
		m_GLf->glUniform4f(m_topSkinColor, r,g,b,a);
		m_TopColR = r;
		m_TopColG = g;
		m_TopColB = b;
		m_TopColA = a;
		m_TopColorValid = true;
	}

	inline void SetSkinTopTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinTop, 2);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE2);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalClothingGlobe shader is used for rendering clothing on animals.

class AUGL_DLL AuGLSLAnimalClothingGlobe : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_clothColor[MAX_CLOTHING_TEXTURES];// Location of 'clothColor' lighting uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldClothing[MAX_CLOTHING_TEXTURES];// Previous texture binding for restore upon deselect.
	GLint								m_OldClothingDecal;		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothing[MAX_CLOTHING_TEXTURES];	// Location of 'texClothing' uniform variable.
	GLint								m_texClothingDecal;		// Location of 'texSkinDetail' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalClothingGlobe()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_clothColor[c] = -1;
			m_OldClothing[c] = 0;
			m_texClothing[c] = 0;
		}
		m_OldClothingDecal = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothingDecal = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothingDecal);
		texUnit++;

		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothing[c]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Clothing Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, const QColor& color, GLuint texName) const
	{
		SetClothingTexture(texIndex, color.redF(), color.greenF(), color.blueF(), texName);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, float clothR, float clothG, float clothB, GLuint texName) const
	{
		m_GLf->glUniform3f(m_clothColor[texIndex], clothR, clothG, clothB);

		GLuint texUnit = 1 + texIndex;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texClothing[texIndex], texUnit);	// Configure uniform 'tex' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothingDecalTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothingDecal, 0);				// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalHair shader is used for rendering hair on animals.

class AUGL_DLL AuGLSLAnimalHairGlobe : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldHair;				// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texHair;				// Location of 'texHair' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalHairGlobe()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldHair = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texHair = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldHair);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Hair Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetHairTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texHair, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalFog shader is used for rendering animals.

class AUGL_DLL AuGLSLAnimalGlobeFog : public AuGLSLProgram
{
	bool								m_BottomColorValid;
	float								m_BottomColR, m_BottomColG, m_BottomColB, m_BottomColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.
	bool								m_TopColorValid;
	float								m_TopColR, m_TopColG, m_TopColB, m_TopColA;

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_bottomSkinColor;		// Location of 'bottomSkinColor' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldSkinBottom;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinDetail;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinTop;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texSkinBottom;		// Location of 'texSkinBottom' uniform variable.
	GLint								m_texSkinDetail;		// Location of 'texSkinDetail' uniform variable.
	GLint								m_texSkinTop;			// Location of 'texSkinTop' uniform variable.
	GLint								m_topSkinColor;			// Location of 'topSkinColor' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalGlobeFog()
	{
		m_bottomSkinColor = -1;
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldSkinBottom = 0;
		m_OldSkinDetail = 0;
		m_OldSkinTop = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texSkinBottom = -1;
		m_texSkinDetail = -1;
		m_texSkinTop = -1;
		m_topSkinColor = -1;

		m_BottomColorValid = false;
		m_BottomColR = 0;
		m_BottomColG = 0;
		m_BottomColB = 0;
		m_BottomColA = 0;
		m_LightsValid = false;
		m_SunsValid = false;
		m_TopColorValid = false;
		m_TopColR = 0;
		m_TopColG = 0;
		m_TopColB = 0;
		m_TopColA = 0;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinBottom);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinDetail);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinTop);
		texUnit++;

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Globe Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetSkinBottomColor(const QColor& color)
	{
		SetSkinBottomColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinBottomColor(float r, float g, float b, float a=1)
	{
		if(m_BottomColorValid && m_BottomColR == r && m_BottomColG == g && m_BottomColB == b && m_BottomColA == a)
			return;
		m_GLf->glUniform4f(m_bottomSkinColor, r,g,b,a);
		m_BottomColR = r;
		m_BottomColG = g;
		m_BottomColB = b;
		m_BottomColA = a;
		m_BottomColorValid = true;
	}

	inline void SetSkinBottomTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinBottom, 0);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinDetailTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinDetail, 1);					// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinTopColor(const QColor& color)
	{
		SetSkinTopColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinTopColor(float r, float g, float b, float a=1)
	{
		if(m_TopColorValid && m_TopColR == r && m_TopColG == g && m_TopColB == b && m_TopColA == a)
			return;
		m_GLf->glUniform4f(m_topSkinColor, r,g,b,a);
		m_TopColR = r;
		m_TopColG = g;
		m_TopColB = b;
		m_TopColA = a;
		m_TopColorValid = true;
	}

	inline void SetSkinTopTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinTop, 2);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE2);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalClothingFog shader is used for rendering clothing on animals.

class AUGL_DLL AuGLSLAnimalClothingGlobeFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_clothColor[MAX_CLOTHING_TEXTURES];// Location of 'clothColor' lighting uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldClothing[MAX_CLOTHING_TEXTURES];// Previous texture binding for restore upon deselect.
	GLint								m_OldClothingDecal;		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothing[MAX_CLOTHING_TEXTURES];	// Location of 'texClothing' uniform variable.
	GLint								m_texClothingDecal;		// Location of 'texSkinDetail' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalClothingGlobeFog()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_clothColor[c] = -1;
			m_OldClothing[c] = 0;
			m_texClothing[c] = 0;
		}
		m_OldClothingDecal = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothingDecal = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothingDecal);
		texUnit++;

		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothing[c]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Clothing Globe Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, const QColor& color, GLuint texName) const
	{
		SetClothingTexture(texIndex, color.redF(), color.greenF(), color.blueF(), texName);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, float clothR, float clothG, float clothB, GLuint texName) const
	{
		m_GLf->glUniform3f(m_clothColor[texIndex], clothR, clothG, clothB);

		GLuint texUnit = 1 + texIndex;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texClothing[texIndex], texUnit);	// Configure uniform 'tex' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothingDecalTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothingDecal, 0);				// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalHairFog shader is used for rendering hair on animals.

class AUGL_DLL AuGLSLAnimalHairGlobeFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldHair;				// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texHair;				// Location of 'texHair' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalHairGlobeFog()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldHair = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texHair = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldHair);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Hair Globe Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetHairTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texHair, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimal shader is used for rendering animals.

class AUGL_DLL AuGLSLAnimalRingworld : public AuGLSLProgram
{
	bool								m_BottomColorValid;
	float								m_BottomColR, m_BottomColG, m_BottomColB, m_BottomColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.
	bool								m_TopColorValid;
	float								m_TopColR, m_TopColG, m_TopColB, m_TopColA;

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_bottomSkinColor;		// Location of 'bottomSkinColor' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldSkinBottom;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinDetail;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinTop;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texSkinBottom;		// Location of 'texSkinBottom' uniform variable.
	GLint								m_texSkinDetail;		// Location of 'texSkinDetail' uniform variable.
	GLint								m_texSkinTop;			// Location of 'texSkinTop' uniform variable.
	GLint								m_topSkinColor;			// Location of 'topSkinColor' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalRingworld()
	{
		m_bottomSkinColor = -1;
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldSkinBottom = 0;
		m_OldSkinDetail = 0;
		m_OldSkinTop = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texSkinBottom = -1;
		m_texSkinDetail = -1;
		m_texSkinTop = -1;
		m_topSkinColor = -1;

		m_BottomColorValid = false;
		m_BottomColR = 0;
		m_BottomColG = 0;
		m_BottomColB = 0;
		m_BottomColA = 0;
		m_LightsValid = false;
		m_SunsValid = false;
		m_TopColorValid = false;
		m_TopColR = 0;
		m_TopColG = 0;
		m_TopColB = 0;
		m_TopColA = 0;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinBottom);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinDetail);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinTop);
		texUnit++;

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetSkinBottomColor(const QColor& color)
	{
		SetSkinBottomColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinBottomColor(float r, float g, float b, float a=1)
	{
		if(m_BottomColorValid && m_BottomColR == r && m_BottomColG == g && m_BottomColB == b && m_BottomColA == a)
			return;
		m_GLf->glUniform4f(m_bottomSkinColor, r,g,b,a);
		m_BottomColR = r;
		m_BottomColG = g;
		m_BottomColB = b;
		m_BottomColA = a;
		m_BottomColorValid = true;
	}

	inline void SetSkinBottomTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinBottom, 0);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinDetailTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinDetail, 1);					// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinTopColor(const QColor& color)
	{
		SetSkinTopColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinTopColor(float r, float g, float b, float a=1)
	{
		if(m_TopColorValid && m_TopColR == r && m_TopColG == g && m_TopColB == b && m_TopColA == a)
			return;
		m_GLf->glUniform4f(m_topSkinColor, r,g,b,a);
		m_TopColR = r;
		m_TopColG = g;
		m_TopColB = b;
		m_TopColA = a;
		m_TopColorValid = true;
	}

	inline void SetSkinTopTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinTop, 2);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE2);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalClothingRingworld shader is used for rendering clothing on animals.

class AUGL_DLL AuGLSLAnimalClothingRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_clothColor[MAX_CLOTHING_TEXTURES];// Location of 'clothColor' lighting uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldClothing[MAX_CLOTHING_TEXTURES];// Previous texture binding for restore upon deselect.
	GLint								m_OldClothingDecal;		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothing[MAX_CLOTHING_TEXTURES];	// Location of 'texClothing' uniform variable.
	GLint								m_texClothingDecal;		// Location of 'texSkinDetail' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalClothingRingworld()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_clothColor[c] = -1;
			m_OldClothing[c] = 0;
			m_texClothing[c] = 0;
		}
		m_OldClothingDecal = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothingDecal = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothingDecal);
		texUnit++;

		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothing[c]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Clothing Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, const QColor& color, GLuint texName) const
	{
		SetClothingTexture(texIndex, color.redF(), color.greenF(), color.blueF(), texName);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, float clothR, float clothG, float clothB, GLuint texName) const
	{
		m_GLf->glUniform3f(m_clothColor[texIndex], clothR, clothG, clothB);

		GLuint texUnit = 1 + texIndex;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texClothing[texIndex], texUnit);	// Configure uniform 'tex' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetClothingDecalTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothingDecal, 0);				// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalHair shader is used for rendering hair on animals.

class AUGL_DLL AuGLSLAnimalHairRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldHair;				// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texHair;				// Location of 'texHair' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalHairRingworld()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldHair = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texHair = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldHair);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Hair Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetHairTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texHair, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalFog shader is used for rendering animals.

class AUGL_DLL AuGLSLAnimalRingworldFog : public AuGLSLProgram
{
	bool								m_BottomColorValid;
	float								m_BottomColR, m_BottomColG, m_BottomColB, m_BottomColA;
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.
	bool								m_TopColorValid;
	float								m_TopColR, m_TopColG, m_TopColB, m_TopColA;

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_bottomSkinColor;		// Location of 'bottomSkinColor' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldSkinBottom;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinDetail;		// Previous texture binding for restore upon deselect.
	GLint								m_OldSkinTop;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texSkinBottom;		// Location of 'texSkinBottom' uniform variable.
	GLint								m_texSkinDetail;		// Location of 'texSkinDetail' uniform variable.
	GLint								m_texSkinTop;			// Location of 'texSkinTop' uniform variable.
	GLint								m_topSkinColor;			// Location of 'topSkinColor' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalRingworldFog()
	{
		m_bottomSkinColor = -1;
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldSkinBottom = 0;
		m_OldSkinDetail = 0;
		m_OldSkinTop = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texSkinBottom = -1;
		m_texSkinDetail = -1;
		m_texSkinTop = -1;
		m_topSkinColor = -1;

		m_BottomColorValid = false;
		m_BottomColR = 0;
		m_BottomColG = 0;
		m_BottomColB = 0;
		m_BottomColA = 0;
		m_LightsValid = false;
		m_SunsValid = false;
		m_TopColorValid = false;
		m_TopColR = 0;
		m_TopColG = 0;
		m_TopColB = 0;
		m_TopColA = 0;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinBottom);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinDetail);
		texUnit++;

		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldSkinTop);
		texUnit++;

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Ringworld Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetSkinBottomColor(const QColor& color)
	{
		SetSkinBottomColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinBottomColor(float r, float g, float b, float a=1)
	{
		if(m_BottomColorValid && m_BottomColR == r && m_BottomColG == g && m_BottomColB == b && m_BottomColA == a)
			return;
		m_GLf->glUniform4f(m_bottomSkinColor, r,g,b,a);
		m_BottomColR = r;
		m_BottomColG = g;
		m_BottomColB = b;
		m_BottomColA = a;
		m_BottomColorValid = true;
	}

	inline void SetSkinBottomTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinBottom, 0);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinDetailTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinDetail, 1);					// Configure uniform 'tex' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	inline void SetSkinTopColor(const QColor& color)
	{
		SetSkinTopColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}

	inline void SetSkinTopColor(float r, float g, float b, float a=1)
	{
		if(m_TopColorValid && m_TopColR == r && m_TopColG == g && m_TopColB == b && m_TopColA == a)
			return;
		m_GLf->glUniform4f(m_topSkinColor, r,g,b,a);
		m_TopColR = r;
		m_TopColG = g;
		m_TopColB = b;
		m_TopColA = a;
		m_TopColorValid = true;
	}

	inline void SetSkinTopTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkinTop, 2);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE2);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalClothingFog shader is used for rendering clothing on animals.

class AUGL_DLL AuGLSLAnimalClothingRingworldFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_clothColor[MAX_CLOTHING_TEXTURES];// Location of 'clothColor' lighting uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldClothing[MAX_CLOTHING_TEXTURES];// Previous texture binding for restore upon deselect.
	GLint								m_OldClothingDecal;		// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothing[MAX_CLOTHING_TEXTURES];	// Location of 'texClothing' uniform variable.
	GLint								m_texClothingDecal;		// Location of 'texSkinDetail' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalClothingRingworldFog()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_clothColor[c] = -1;
			m_OldClothing[c] = 0;
			m_texClothing[c] = 0;
		}
		m_OldClothingDecal = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothingDecal = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		GLuint texUnit = GL_TEXTURE0;

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(texUnit);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothingDecal);
		texUnit++;

		for(int c=0; c<MAX_CLOTHING_TEXTURES; c++)
		{
			m_GLf->glActiveTexture(texUnit);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldClothing[c]);
			texUnit++;
		}

		m_GLf->glActiveTexture(GL_TEXTURE0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Clothing Ringworld Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, const QColor& color, GLuint texName) const
	{
		SetClothingTexture(texIndex, color.redF(), color.greenF(), color.blueF(), texName);
	}

	// Set clothing texture and the color that is used to modulate it.
	inline void SetClothingTexture(int texIndex, float clothR, float clothG, float clothB, GLuint texName) const
	{
		m_GLf->glUniform3f(m_clothColor[texIndex], clothR, clothG, clothB);

		GLuint texUnit = 1 + texIndex;
		GLuint texBase = GL_TEXTURE0 + texUnit;
		m_GLf->glUniform1i(m_texClothing[texIndex], texUnit);	// Configure uniform 'tex' for texture unit.
		m_GLf->glActiveTexture(texBase);						// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetClothingDecalTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothingDecal, 0);				// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLAnimalHairFog shader is used for rendering hair on animals.

class AUGL_DLL AuGLSLAnimalHairRingworldFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_ANIMAL_BONES];		// Bone matrices
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldHair;				// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texHair;				// Location of 'texHair' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLAnimalHairRingworldFog()
	{
		for(int bone=0; bone<MAX_ANIMAL_BONES; bone++)
			m_bM[bone] = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldHair = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texHair = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldHair);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Animal Hair Ringworld Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetHairTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texHair, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBody shader is used for rendering models.

class AUGL_DLL AuGLSLBody : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_BONES];		// Bone matrices
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothes;			// Location of 'texClothes' uniform variable.
	GLint								m_texClothesOffsetWear;	// Location of 'texClothesOffsetWear' uniform variable.
	GLint								m_texSkin;				// Location of 'texSkin' uniform variable.
	GLint								m_texSkinOffset;		// Location of 'texSkinOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLBody()
	{
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothes = -1;
		m_texClothesOffsetWear = -1;
		m_texSkin = -1;
		m_texSkinOffset = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Bone matrices.
		for(int bone=0; bone<MAX_BONES; bone++)
			m_bM[bone] = -1;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Body");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// Set clothes texture offset and wear value.
	// 'wear' is 0 for skin only (naked), 1 for clothes only (env suit), 2 to blend skin and clothes (clothes).
	inline void SetClothesTextureOffsetWear(float u, float v, float wear) const
	{
		m_GLf->glUniform3f(m_texClothesOffsetWear, u, v, wear);
	}

	// Set skin texture offset.
	inline void SetSkinTextureOffset(float uT, float vT, float uB, float vB) const
	{
		m_GLf->glUniform4f(m_texSkinOffset, uT, vT, uB, vB);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothesTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothes, 1);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetSkinTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkin, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBodyFog shader is used for rendering models.

class AUGL_DLL AuGLSLBodyFog : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_BONES];		// Bone matrices
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothes;			// Location of 'texClothes' uniform variable.
	GLint								m_texClothesOffsetWear;	// Location of 'texClothesOffsetWear' uniform variable.
	GLint								m_texSkin;				// Location of 'texSkin' uniform variable.
	GLint								m_texSkinOffset;		// Location of 'texSkinOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLBodyFog()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothes = -1;
		m_texClothesOffsetWear = -1;
		m_texSkin = -1;
		m_texSkinOffset = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Bone matrices.
		for(int bone=0; bone<MAX_BONES; bone++)
			m_bM[bone] = -1;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Body Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// Set clothes texture offset and wear value.
	// 'wear' is 0 for skin only (naked), 1 for clothes only (env suit), 2 to blend skin and clothes (clothes).
	inline void SetClothesTextureOffsetWear(float u, float v, float wear) const
	{
		m_GLf->glUniform3f(m_texClothesOffsetWear, u, v, wear);
	}

	// Set skin texture offset.
	inline void SetSkinTextureOffset(float uT, float vT, float uB, float vB) const
	{
		m_GLf->glUniform4f(m_texSkinOffset, uT, vT, uB, vB);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothesTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothes, 1);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetSkinTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkin, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBodyGlobe shader is used for rendering models.

class AUGL_DLL AuGLSLBodyGlobe : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_BONES];		// Bone matrices
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothes;			// Location of 'texClothes' uniform variable.
	GLint								m_texClothesOffsetWear;	// Location of 'texClothesOffsetWear' uniform variable.
	GLint								m_texSkin;				// Location of 'texSkin' uniform variable.
	GLint								m_texSkinOffset;		// Location of 'texSkinOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLBodyGlobe()
	{
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothes = -1;
		m_texClothesOffsetWear = -1;
		m_texSkin = -1;
		m_texSkinOffset = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Bone matrices.
		for(int bone=0; bone<MAX_BONES; bone++)
			m_bM[bone] = -1;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Body Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// Set clothes texture offset and wear value.
	// 'wear' is 0 for skin only (naked), 1 for clothes only (env suit), 2 to blend skin and clothes (clothes).
	inline void SetClothesTextureOffsetWear(float u, float v, float wear) const
	{
		m_GLf->glUniform3f(m_texClothesOffsetWear, u, v, wear);
	}

	// Set skin texture offset.
	inline void SetSkinTextureOffset(float uT, float vT, float uB, float vB) const
	{
		m_GLf->glUniform4f(m_texSkinOffset, uT, vT, uB, vB);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothesTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothes, 1);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetSkinTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkin, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBodyFogGlobe shader is used for rendering models.

class AUGL_DLL AuGLSLBodyFogGlobe : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_BONES];		// Bone matrices
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothes;			// Location of 'texClothes' uniform variable.
	GLint								m_texClothesOffsetWear;	// Location of 'texClothesOffsetWear' uniform variable.
	GLint								m_texSkin;				// Location of 'texSkin' uniform variable.
	GLint								m_texSkinOffset;		// Location of 'texSkinOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLBodyFogGlobe()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_globeP = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothes = -1;
		m_texClothesOffsetWear = -1;
		m_texSkin = -1;
		m_texSkinOffset = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Bone matrices.
		for(int bone=0; bone<MAX_BONES; bone++)
			m_bM[bone] = -1;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Body Fog Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(float x, float y, float z)
	{
		m_GLf->glUniform3f(m_globeP, x, y, z);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// Set clothes texture offset and wear value.
	// 'wear' is 0 for skin only (naked), 1 for clothes only (env suit), 2 to blend skin and clothes (clothes).
	inline void SetClothesTextureOffsetWear(float u, float v, float wear) const
	{
		m_GLf->glUniform3f(m_texClothesOffsetWear, u, v, wear);
	}

	// Set skin texture offset.
	inline void SetSkinTextureOffset(float uT, float vT, float uB, float vB) const
	{
		m_GLf->glUniform4f(m_texSkinOffset, uT, vT, uB, vB);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	inline void SetClothesTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothes, 1);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetSkinTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkin, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBodyRingworld shader is used for rendering models.

class AUGL_DLL AuGLSLBodyRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_BONES];		// Bone matrices
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothes;			// Location of 'texClothes' uniform variable.
	GLint								m_texClothesOffsetWear;	// Location of 'texClothesOffsetWear' uniform variable.
	GLint								m_texSkin;				// Location of 'texSkin' uniform variable.
	GLint								m_texSkinOffset;		// Location of 'texSkinOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLBodyRingworld()
	{
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothes = -1;
		m_texClothesOffsetWear = -1;
		m_texSkin = -1;
		m_texSkinOffset = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Bone matrices.
		for(int bone=0; bone<MAX_BONES; bone++)
			m_bM[bone] = -1;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Body Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// Set clothes texture offset and wear value.
	// 'wear' is 0 for skin only (naked), 1 for clothes only (env suit), 2 to blend skin and clothes (clothes).
	inline void SetClothesTextureOffsetWear(float u, float v, float wear) const
	{
		m_GLf->glUniform3f(m_texClothesOffsetWear, u, v, wear);
	}

	// Set skin texture offset.
	inline void SetSkinTextureOffset(float uT, float vT, float uB, float vB) const
	{
		m_GLf->glUniform4f(m_texSkinOffset, uT, vT, uB, vB);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetClothesTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothes, 1);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetSkinTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkin, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLBodyFogRingworld shader is used for rendering models.

class AUGL_DLL AuGLSLBodyFogRingworld : public AuGLSLProgram
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_bM[MAX_BONES];		// Bone matrices
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowAlpha;			// Location of 'glowAlpha' lighting uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texClothes;			// Location of 'texClothes' uniform variable.
	GLint								m_texClothesOffsetWear;	// Location of 'texClothesOffsetWear' uniform variable.
	GLint								m_texSkin;				// Location of 'texSkin' uniform variable.
	GLint								m_texSkinOffset;		// Location of 'texSkinOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLBodyFogRingworld()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowAlpha = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_texClothes = -1;
		m_texClothesOffsetWear = -1;
		m_texSkin = -1;
		m_texSkinOffset = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Bone matrices.
		for(int bone=0; bone<MAX_BONES; bone++)
			m_bM[bone] = -1;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Body Fog Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	inline void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	inline void SetAmbientLightUp(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_ambientUp, x, y, z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Sets the values into a bone matrix. 'bone' is a bone index.
	inline void SetBone(int bone, const AuMatrix4x4& m) const
	{
		SetUniformMatrix(m_GLf, m_bM[bone], m);
	}

	// Set glow color/intensity and also the vertex transparency alpha.
	inline void SetGlowAlpha(float glowR, float glowG, float glowB, float alpha) const
	{
		m_GLf->glUniform4f(m_glowAlpha, glowR, glowG, glowB, alpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// Set clothes texture offset and wear value.
	// 'wear' is 0 for skin only (naked), 1 for clothes only (env suit), 2 to blend skin and clothes (clothes).
	inline void SetClothesTextureOffsetWear(float u, float v, float wear) const
	{
		m_GLf->glUniform3f(m_texClothesOffsetWear, u, v, wear);
	}

	// Set skin texture offset.
	inline void SetSkinTextureOffset(float uT, float vT, float uB, float vB) const
	{
		m_GLf->glUniform4f(m_texSkinOffset, uT, vT, uB, vB);
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	inline void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	inline void SetClothesTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texClothes, 1);					// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE1);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetSkinTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSkin, 0);						// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSLScBase : public AuGLSLProgram
{
public:
	AuGLSLScBase()
	{
	}

public:
	virtual QString Name() const
	{
		return QString("ScBase");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re)
	{
		Q_UNUSED(projection);
		Q_UNUSED(re);
		AuGLSLProgram::Select();
	}

	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const = 0;

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const = 0;

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const = 0;

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const = 0;

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const = 0;

	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const = 0;

	virtual void SetLightEnabled(int light, bool enabled) const = 0;

	virtual void SetModelView(const AuMatrix4x4& p) const = 0;

	virtual void SetProjection(const AuMatrix4x4& p) = 0;

	// Setup sun for non-ringworld solar lighting shaders.
	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const = 0;

	// Setup sun for ringworld solar lighting shaders.
	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in the object space of this sun
	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const = 0;

	virtual void SetSunEnabled(int sun, bool enabled) const = 0;

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const = 0;

	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const = 0;

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const = 0;

	virtual void SetVertAlpha(float vertAlpha) const = 0;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Renders Sc models without sun over the horizon or ringworld shadow panel effects, for scene not at a world.

class AUGL_DLL AuGLSLSc : public AuGLSLScBase
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_effectApply;			// Location of 'effectApply' uniform variable.
	GLint								m_glowDiscard;			// Location of 'glowDiscard' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture[SCTEXI_Count];	// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texEffect;			// Location of 'texEffect' uniform variable.
	GLint								m_texSurface[SCTEXI_Count];	// Location of 'texSurface' uniform variable.
	GLint								m_vertAlpha;			// Location of 'vertAlpha' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLSc()
	{
		m_effectApply = -1;
		m_glowDiscard = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_OldTexture[t] = 0;
			m_texSurface[t] = -1;
		}
		m_p = -1;
		m_ProjectionValid = false;
		m_texEffect = -1;
		m_vertAlpha = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLScBase::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_GLf->glActiveTexture(GL_TEXTURE1 + t);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture[t]);
		}
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Sc");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const
	{
		m_GLf->glUniform3f(m_ambientUp, upVec.x,upVec.y,upVec.z);
	}

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const
	{
		m_GLf->glUniform1i(m_texEffect, 0);						// Configure uniform 'texEffect' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
		m_GLf->glUniform1f(m_effectApply, (float)appl);
	}

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glowR, glowG, glowB, discardAlpha);
	}

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glow.redF(), glow.greenF(), glow.blueF(), discardAlpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	virtual void SetLightEnabled(int light, bool enabled) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	virtual void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	virtual void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const
	{
		Q_ASSERT(false);	// Oops, called the wrong SetSun variation.
		Q_UNUSED(sun);
		Q_UNUSED(enabled);
		Q_UNUSED(attenuation);
		Q_UNUSED(colorR);
		Q_UNUSED(colorG);
		Q_UNUSED(colorB);
		Q_UNUSED(locX);
		Q_UNUSED(locY);
		Q_UNUSED(locZ);
		Q_UNUSED(diskRadius);
		Q_UNUSED(shadowAngle);
		Q_UNUSED(shadowCount);
		Q_UNUSED(shadowXVec);
		Q_UNUSED(shadowYVec);
	}

	virtual void SetSunEnabled(int sun, bool enabled) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
	}

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		Q_UNUSED(sun);
		Q_UNUSED(shadowAngle);
	}

	// 'texIndex' should be in the range 0..(SCTEXI_Count-1).
	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSurface[texIndex], 1 + texIndex);				// Configure uniform 'texSurface' for texture unit.
		m_GLf->glActiveTexture(GL_TEXTURE1 + texIndex);							// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);							// Bind the texture to the current texture unit.
	}

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const
	{
		for(int t=0; t<SCTEXI_Count; t++)
			SetSurfaceTexture(t, tex[t]);
	}

	virtual void SetVertAlpha(float vertAlpha) const
	{
		m_GLf->glUniform1f(m_vertAlpha, vertAlpha);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Renders Sc models with fog, without sun over the horizon or ringworld shadow panel effects, for scene not at a world.

class AUGL_DLL AuGLSLScFog : public AuGLSLScBase
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_effectApply;			// Location of 'effectApply' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowDiscard;			// Location of 'glowDiscard' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture[SCTEXI_Count];	// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texEffect;			// Location of 'texEffect' uniform variable.
	GLint								m_texSurface[SCTEXI_Count];	// Location of 'texSurface' uniform variable.
	GLint								m_vertAlpha;			// Location of 'vertAlpha' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLScFog()
	{
		m_effectApply = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowDiscard = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_OldTexture[t] = 0;
			m_texSurface[t] = -1;
		}
		m_p = -1;
		m_ProjectionValid = false;
		m_texEffect = -1;
		m_vertAlpha = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLScBase::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_GLf->glActiveTexture(GL_TEXTURE1 + t);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture[t]);
		}
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Sc Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const
	{
		m_GLf->glUniform3f(m_ambientUp, upVec.x,upVec.y,upVec.z);
	}

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const
	{
		m_GLf->glUniform1i(m_texEffect, 0);						// Configure uniform 'texEffect' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
		m_GLf->glUniform1f(m_effectApply, (float)appl);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glowR, glowG, glowB, discardAlpha);
	}

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glow.redF(), glow.greenF(), glow.blueF(), discardAlpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	virtual void SetLightEnabled(int light, bool enabled) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	virtual void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	virtual void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const
	{
		Q_ASSERT(false);	// Oops, called the wrong SetSun variation.
		Q_UNUSED(sun);
		Q_UNUSED(enabled);
		Q_UNUSED(attenuation);
		Q_UNUSED(colorR);
		Q_UNUSED(colorG);
		Q_UNUSED(colorB);
		Q_UNUSED(locX);
		Q_UNUSED(locY);
		Q_UNUSED(locZ);
		Q_UNUSED(diskRadius);
		Q_UNUSED(shadowAngle);
		Q_UNUSED(shadowCount);
		Q_UNUSED(shadowXVec);
		Q_UNUSED(shadowYVec);
	}

	virtual void SetSunEnabled(int sun, bool enabled) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
	}

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		Q_UNUSED(sun);
		Q_UNUSED(shadowAngle);
	}

	// 'texIndex' should be in the range 0..(SCTEXI_Count-1).
	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSurface[texIndex], 1 + texIndex);				// Configure uniform 'texSurface' for texture unit.
		m_GLf->glActiveTexture(GL_TEXTURE1 + texIndex);							// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);							// Bind the texture to the current texture unit.
	}

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const
	{
		for(int t=0; t<SCTEXI_Count; t++)
			SetSurfaceTexture(t, tex[t]);
	}

	virtual void SetVertAlpha(float vertAlpha) const
	{
		m_GLf->glUniform1f(m_vertAlpha, vertAlpha);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Renders Sc models without sun over the horizon or ringworld shadow panel effects, for scene not at a world.

class AUGL_DLL AuGLSLScGlobe : public AuGLSLScBase
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_effectApply;			// Location of 'effectApply' uniform variable.
	GLint								m_glowDiscard;			// Location of 'glowDiscard' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture[SCTEXI_Count];	// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texEffect;			// Location of 'texEffect' uniform variable.
	GLint								m_texSurface[SCTEXI_Count];	// Location of 'texSurface' uniform variable.
	GLint								m_vertAlpha;			// Location of 'vertAlpha' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLScGlobe()
	{
		m_effectApply = -1;
		m_glowDiscard = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_OldTexture[t] = 0;
			m_texSurface[t] = -1;
		}
		m_p = -1;
		m_ProjectionValid = false;
		m_texEffect = -1;
		m_vertAlpha = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLScBase::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_GLf->glActiveTexture(GL_TEXTURE1 + t);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture[t]);
		}
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Sc Globe");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const
	{
		m_GLf->glUniform3f(m_ambientUp, upVec.x,upVec.y,upVec.z);
	}

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const
	{
		m_GLf->glUniform1i(m_texEffect, 0);						// Configure uniform 'texEffect' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
		m_GLf->glUniform1f(m_effectApply, (float)appl);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(const AuPoint& globeLoc) const
	{
		m_GLf->glUniform3f(m_globeP, globeLoc.x,globeLoc.y,globeLoc.z);
	}

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glowR, glowG, glowB, discardAlpha);
	}

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glow.redF(), glow.greenF(), glow.blueF(), discardAlpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	virtual void SetLightEnabled(int light, bool enabled) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	virtual void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	virtual void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const
	{
		Q_ASSERT(false);	// Oops, called the wrong SetSun variation.
		Q_UNUSED(sun);
		Q_UNUSED(enabled);
		Q_UNUSED(attenuation);
		Q_UNUSED(colorR);
		Q_UNUSED(colorG);
		Q_UNUSED(colorB);
		Q_UNUSED(locX);
		Q_UNUSED(locY);
		Q_UNUSED(locZ);
		Q_UNUSED(diskRadius);
		Q_UNUSED(shadowAngle);
		Q_UNUSED(shadowCount);
		Q_UNUSED(shadowXVec);
		Q_UNUSED(shadowYVec);
	}

	virtual void SetSunEnabled(int sun, bool enabled) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
	}

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		Q_UNUSED(sun);
		Q_UNUSED(shadowAngle);
	}

	// 'texIndex' should be in the range 0..(SCTEXI_Count-1).
	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSurface[texIndex], 1 + texIndex);				// Configure uniform 'texSurface' for texture unit.
		m_GLf->glActiveTexture(GL_TEXTURE1 + texIndex);							// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);							// Bind the texture to the current texture unit.
	}

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const
	{
		for(int t=0; t<SCTEXI_Count; t++)
			SetSurfaceTexture(t, tex[t]);
	}

	virtual void SetVertAlpha(float vertAlpha) const
	{
		m_GLf->glUniform1f(m_vertAlpha, vertAlpha);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Renders Sc models with fog, without sun over the horizon or ringworld shadow panel effects, for scene not at a world.

class AUGL_DLL AuGLSLScGlobeFog : public AuGLSLScBase
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_effectApply;			// Location of 'effectApply' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowDiscard;			// Location of 'glowDiscard' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture[SCTEXI_Count];	// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texEffect;			// Location of 'texEffect' uniform variable.
	GLint								m_texSurface[SCTEXI_Count];	// Location of 'texSurface' uniform variable.
	GLint								m_vertAlpha;			// Location of 'vertAlpha' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_globeP;				// Location of 'globeP' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLScGlobeFog()
	{
		m_effectApply = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowDiscard = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_OldTexture[t] = 0;
			m_texSurface[t] = -1;
		}
		m_p = -1;
		m_ProjectionValid = false;
		m_texEffect = -1;
		m_vertAlpha = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_globeP = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLScBase::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_GLf->glActiveTexture(GL_TEXTURE1 + t);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture[t]);
		}
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Sc Globe Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const
	{
		m_GLf->glUniform3f(m_ambientUp, upVec.x,upVec.y,upVec.z);
	}

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const
	{
		m_GLf->glUniform1i(m_texEffect, 0);						// Configure uniform 'texEffect' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
		m_GLf->glUniform1f(m_effectApply, (float)appl);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// Globe position for sun horizon calculation.
	// This location is in eye space.
	inline void SetGlobeLocation(const AuPoint& globeLoc) const
	{
		m_GLf->glUniform3f(m_globeP, globeLoc.x,globeLoc.y,globeLoc.z);
	}

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glowR, glowG, glowB, discardAlpha);
	}

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glow.redF(), glow.greenF(), glow.blueF(), discardAlpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	virtual void SetLightEnabled(int light, bool enabled) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	virtual void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	virtual void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'sun' is an index from 0 to MAX_SUNS-1.
	// 'loc' is in eye space
	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const
	{
		Q_ASSERT(false);	// Oops, called the wrong SetSun variation.
		Q_UNUSED(sun);
		Q_UNUSED(enabled);
		Q_UNUSED(attenuation);
		Q_UNUSED(colorR);
		Q_UNUSED(colorG);
		Q_UNUSED(colorB);
		Q_UNUSED(locX);
		Q_UNUSED(locY);
		Q_UNUSED(locZ);
		Q_UNUSED(diskRadius);
		Q_UNUSED(shadowAngle);
		Q_UNUSED(shadowCount);
		Q_UNUSED(shadowXVec);
		Q_UNUSED(shadowYVec);
	}

	virtual void SetSunEnabled(int sun, bool enabled) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
	}

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		Q_UNUSED(sun);
		Q_UNUSED(shadowAngle);
	}

	// 'texIndex' should be in the range 0..(SCTEXI_Count-1).
	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSurface[texIndex], 1 + texIndex);				// Configure uniform 'texSurface' for texture unit.
		m_GLf->glActiveTexture(GL_TEXTURE1 + texIndex);							// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);							// Bind the texture to the current texture unit.
	}

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const
	{
		for(int t=0; t<SCTEXI_Count; t++)
			SetSurfaceTexture(t, tex[t]);
	}

	virtual void SetVertAlpha(float vertAlpha) const
	{
		m_GLf->glUniform1f(m_vertAlpha, vertAlpha);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Renders Sc models with ringworld shadow panel effects, for scene at a ringworld.

class AUGL_DLL AuGLSLScRingworld : public AuGLSLScBase
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_effectApply;			// Location of 'effectApply' uniform variable.
	GLint								m_glowDiscard;			// Location of 'glowDiscard' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture[SCTEXI_Count];	// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texEffect;			// Location of 'texEffect' uniform variable.
	GLint								m_texSurface[SCTEXI_Count];	// Location of 'texSurface' uniform variable.
	GLint								m_vertAlpha;			// Location of 'vertAlpha' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLScRingworld()
	{
		m_effectApply = -1;
		m_glowDiscard = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_OldTexture[t] = 0;
			m_texSurface[t] = -1;
		}
		m_p = -1;
		m_ProjectionValid = false;
		m_texEffect = -1;
		m_vertAlpha = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLScBase::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_GLf->glActiveTexture(GL_TEXTURE1 + t);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture[t]);
		}
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Sc Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const
	{
		m_GLf->glUniform3f(m_ambientUp, upVec.x,upVec.y,upVec.z);
	}

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const
	{
		m_GLf->glUniform1i(m_texEffect, 0);						// Configure uniform 'texEffect' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
		m_GLf->glUniform1f(m_effectApply, (float)appl);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(const AuPoint& eyeLoc) const
	{
		m_GLf->glUniform3f(m_eyeLocation, eyeLoc.x,eyeLoc.y,eyeLoc.z);
	}

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glowR, glowG, glowB, discardAlpha);
	}

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glow.redF(), glow.greenF(), glow.blueF(), discardAlpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	virtual void SetLightEnabled(int light, bool enabled) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	virtual void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	virtual void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const
	{
		Q_ASSERT(false);	// Oops, called the wrong SetSun variation.
		Q_UNUSED(sun);
		Q_UNUSED(enabled);
		Q_UNUSED(attenuation);
		Q_UNUSED(colorR);
		Q_UNUSED(colorG);
		Q_UNUSED(colorB);
		Q_UNUSED(locX);
		Q_UNUSED(locY);
		Q_UNUSED(locZ);
		Q_UNUSED(diskRadius);
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVec.x, shadowXVec.y, shadowXVec.z);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVec.x, shadowYVec.y, shadowYVec.z);
	}

	virtual void SetSunEnabled(int sun, bool enabled) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
	}

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	// 'texIndex' should be in the range 0..(SCTEXI_Count-1).
	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSurface[texIndex], 1 + texIndex);				// Configure uniform 'texSurface' for texture unit.
		m_GLf->glActiveTexture(GL_TEXTURE1 + texIndex);							// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);							// Bind the texture to the current texture unit.
	}

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const
	{
		for(int t=0; t<SCTEXI_Count; t++)
			SetSurfaceTexture(t, tex[t]);
	}

	virtual void SetVertAlpha(float vertAlpha) const
	{
		m_GLf->glUniform1f(m_vertAlpha, vertAlpha);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Renders Sc models with fog and ringworld shadow panel effects, for scene at a ringworld.

class AUGL_DLL AuGLSLScRingworldFog : public AuGLSLScBase
{
	bool								m_LightsValid;			// True while this shader's lights are in sync with AuGLEffects.
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.
	bool								m_SunsValid;			// True while this shader's suns are in sync with AuGLEffects.

private:
	GLint								m_effectApply;			// Location of 'effectApply' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_glowDiscard;			// Location of 'glowDiscard' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture[SCTEXI_Count];	// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_texEffect;			// Location of 'texEffect' uniform variable.
	GLint								m_texSurface[SCTEXI_Count];	// Location of 'texSurface' uniform variable.
	GLint								m_vertAlpha;			// Location of 'vertAlpha' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_ambientUp;			// Location of 'ambientUp' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLScRingworldFog()
	{
		m_effectApply = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_glowDiscard = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_OldTexture[t] = 0;
			m_texSurface[t] = -1;
		}
		m_p = -1;
		m_ProjectionValid = false;
		m_texEffect = -1;
		m_vertAlpha = -1;

		m_LightsValid = false;
		m_SunsValid = false;

		// Lighting parameters
		m_ambient = -1;
		m_ambientUp = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLScBase::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		for(int t=0; t<SCTEXI_Count; t++)
		{
			m_GLf->glActiveTexture(GL_TEXTURE1 + t);
			m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture[t]);
		}
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	// Call when lights in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateLights()
	{
		m_LightsValid = false;
	}

	// Call when suns in AuGLEffects change during a render frame. Next Select will reconfigure lights.
	inline void InvalidateSuns()
	{
		m_SunsValid = false;
	}

	virtual QString Name() const
	{
		return QString("Sc Ringworld Fog");
	}

	virtual void Select(const AuMatrix4x4& projection, const AuGLEffects& re);

	// Set ambient light color/intensity.
	virtual void SetAmbientLight(float ambientR, float ambientG, float ambientB) const
	{
		m_GLf->glUniform3f(m_ambient, ambientR, ambientG, ambientB);
	}

	// Set ambient light up vector, for spherical ambient light calc.
	// This vector is in eye space.
	virtual void SetAmbientLightUp(const AuPoint& upVec) const
	{
		m_GLf->glUniform3f(m_ambientUp, upVec.x,upVec.y,upVec.z);
	}

	// 'texName' is the OpenGL texture id.
	// 'appl' is 0 to disable surface effect, 1 to light the effect with environment lighting, 2 for glowing effect.
	virtual void SetEffectTexture(GLuint texName, int appl) const
	{
		m_GLf->glUniform1i(m_texEffect, 0);						// Configure uniform 'texEffect' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);					// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);			// Bind the texture to the current texture unit.
		m_GLf->glUniform1f(m_effectApply, (float)appl);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(const AuPoint& eyeLoc) const
	{
		m_GLf->glUniform3f(m_eyeLocation, eyeLoc.x,eyeLoc.y,eyeLoc.z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	virtual void SetGlowDiscard(float glowR, float glowG, float glowB, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glowR, glowG, glowB, discardAlpha);
	}

	virtual void SetGlowDiscard(const QColor& glow, float discardAlpha) const
	{
		m_GLf->glUniform4f(m_glowDiscard, glow.redF(), glow.greenF(), glow.blueF(), discardAlpha);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in eye space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit eye-space vector when defining a spot light.
	virtual void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY,
			float spotDirZ) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	virtual void SetLightEnabled(int light, bool enabled) const
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	virtual void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	virtual void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
			float locX, float locY, float locZ, float diskRadius) const
	{
		Q_ASSERT(false);	// Oops, called the wrong SetSun variation.
		Q_UNUSED(sun);
		Q_UNUSED(enabled);
		Q_UNUSED(attenuation);
		Q_UNUSED(colorR);
		Q_UNUSED(colorG);
		Q_UNUSED(colorB);
		Q_UNUSED(locX);
		Q_UNUSED(locY);
		Q_UNUSED(locZ);
		Q_UNUSED(diskRadius);
	}

	virtual void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB, float locX, float locY,
			float locZ, float diskRadius, float shadowAngle, float shadowCount, const AuPoint& shadowXVec,
			const AuPoint& shadowYVec) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVec.x, shadowXVec.y, shadowXVec.z);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVec.x, shadowYVec.y, shadowYVec.z);
	}

	virtual void SetSunEnabled(int sun, bool enabled) const
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
	}

	virtual void SetSunShadowAngle(int sun, float shadowAngle) const
	{
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
	}

	// 'texIndex' should be in the range 0..(SCTEXI_Count-1).
	virtual void SetSurfaceTexture(int texIndex, GLuint texName) const
	{
		m_GLf->glUniform1i(m_texSurface[texIndex], 1 + texIndex);				// Configure uniform 'texSurface' for texture unit.
		m_GLf->glActiveTexture(GL_TEXTURE1 + texIndex);							// Select texture unit as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);							// Bind the texture to the current texture unit.
	}

	virtual void SetSurfaceTextures(const AuTexturePalette& tex) const
	{
		for(int t=0; t<SCTEXI_Count; t++)
			SetSurfaceTexture(t, tex[t]);
	}

	virtual void SetVertAlpha(float vertAlpha) const
	{
		m_GLf->glUniform1f(m_vertAlpha, vertAlpha);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWater shader is used for rendering water surface

class AUGL_DLL AuGLSLWater : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCos;				// Location of 'clipCos' uniform variable.
	GLint								m_clipUp;				// Location of 'clipUp' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texBump;				// Location of 'texBump' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWater()
	{
		m_clipCos = -1;
		m_clipUp = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texBump = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetBumpMap(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texBump, 1);								// Configure uniform 'texBump' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetClip(float clipCos, float clipUpX, float clipUpY, float clipUpZ)
	{
		m_GLf->glUniform1f(m_clipCos, clipCos);
		m_GLf->glUniform3f(m_clipUp, clipUpX, clipUpY, clipUpZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterRingworld shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterRingworld : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCenter;			// Location of 'clipCenter' uniform variable.
	GLint								m_clipDist;				// Location of 'clipDist' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texBump;				// Location of 'texBump' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterRingworld()
	{
		m_clipCenter = -1;
		m_clipDist = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texBump = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water Ringworld");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetBumpMap(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texBump, 1);								// Configure uniform 'texBump' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetClip(float clipDist, float clipCenterX, float clipCenterY, float clipCenterZ)
	{
		m_GLf->glUniform1f(m_clipDist, clipDist);
		m_GLf->glUniform3f(m_clipCenter, clipCenterX, clipCenterY, clipCenterZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterNoBump shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterNoBump : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCos;				// Location of 'clipCos' uniform variable.
	GLint								m_clipUp;				// Location of 'clipUp' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterNoBump()
	{
		m_clipCos = -1;
		m_clipUp = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water No Bump");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipCos, float clipUpX, float clipUpY, float clipUpZ)
	{
		m_GLf->glUniform1f(m_clipCos, clipCos);
		m_GLf->glUniform3f(m_clipUp, clipUpX, clipUpY, clipUpZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterRingworldNoBump shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterRingworldNoBump : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCenter;			// Location of 'clipCenter' uniform variable.
	GLint								m_clipDist;				// Location of 'clipDist' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterRingworldNoBump()
	{
		m_clipCenter = -1;
		m_clipDist = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water Ringworld No Bump");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipDist, float clipCenterX, float clipCenterY, float clipCenterZ)
	{
		m_GLf->glUniform1f(m_clipDist, clipDist);
		m_GLf->glUniform3f(m_clipCenter, clipCenterX, clipCenterY, clipCenterZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterNoHole shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterNoHole : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCos;				// Location of 'clipCos' uniform variable.
	GLint								m_clipUp;				// Location of 'clipUp' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texBump;				// Location of 'texBump' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterNoHole()
	{
		m_clipCos = -1;
		m_clipUp = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texBump = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water No Hole");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetBumpMap(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texBump, 1);								// Configure uniform 'texBump' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetClip(float clipCos, float clipUpX, float clipUpY, float clipUpZ)
	{
		m_GLf->glUniform1f(m_clipCos, clipCos);
		m_GLf->glUniform3f(m_clipUp, clipUpX, clipUpY, clipUpZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterRingworldNoHole shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterRingworldNoHole : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCenter;			// Location of 'clipCenter' uniform variable.
	GLint								m_clipDist;				// Location of 'clipDist' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_OldTexture1;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texBump;				// Location of 'texBump' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterRingworldNoHole()
	{
		m_clipCenter = -1;
		m_clipDist = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_OldTexture1 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texBump = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE1);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture1);
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water Ringworld No Hole");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetBumpMap(GLuint texName) const
	{
		m_GLf->glUniform1i(m_texBump, 1);								// Configure uniform 'texBump' for texture unit 1.
		m_GLf->glActiveTexture(GL_TEXTURE1);							// Select texture unit 1 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}

	inline void SetClip(float clipDist, float clipCenterX, float clipCenterY, float clipCenterZ)
	{
		m_GLf->glUniform1f(m_clipDist, clipDist);
		m_GLf->glUniform3f(m_clipCenter, clipCenterX, clipCenterY, clipCenterZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b) const
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterNoBumpNoHole shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterNoBumpNoHole : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCos;				// Location of 'clipCos' uniform variable.
	GLint								m_clipUp;				// Location of 'clipUp' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterNoBumpNoHole()
	{
		m_clipCos = -1;
		m_clipUp = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water No Bump No Hole");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipCos, float clipUpX, float clipUpY, float clipUpZ)
	{
		m_GLf->glUniform1f(m_clipCos, clipCos);
		m_GLf->glUniform3f(m_clipUp, clipUpX, clipUpY, clipUpZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLWaterRingworldNoBumpNoHole shader is used for rendering water surface

class AUGL_DLL AuGLSLWaterRingworldNoBumpNoHole : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_clipCenter;			// Location of 'clipCenter' uniform variable.
	GLint								m_clipDist;				// Location of 'clipDist' uniform variable.
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_opacity;				// Location of 'opacity' uniform variable.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.
	GLint								m_sunShadowAngle[MAX_SUNS];	// Location of 'sun.shadowAngle' lighting uniform variable.
	GLint								m_sunShadowCount[MAX_SUNS];	// Location of 'sun.shadowCount' lighting uniform variable.
	GLint								m_sunShadowXVec[MAX_SUNS];	// Location of 'sun.shadowXVec' lighting uniform variable.
	GLint								m_sunShadowYVec[MAX_SUNS];	// Location of 'sun.shadowYVec' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLWaterRingworldNoBumpNoHole()
	{
		m_clipCenter = -1;
		m_clipDist = -1;
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_opacity = -1;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
			m_sunShadowAngle[sun] = -1;
			m_sunShadowCount[sun] = -1;
			m_sunShadowXVec[sun] = -1;
			m_sunShadowYVec[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Water Ringworld No Bump No Hole");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	inline void SetClip(float clipDist, float clipCenterX, float clipCenterY, float clipCenterZ)
	{
		m_GLf->glUniform1f(m_clipDist, clipDist);
		m_GLf->glUniform3f(m_clipCenter, clipCenterX, clipCenterY, clipCenterZ);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// Opacity controls the alpha value of the final fragment.
	inline void SetOpacity(float o)
	{
		m_GLf->glUniform1f(m_opacity, o);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius, float shadowAngle, float shadowCount,
		float shadowXVecX, float shadowXVecY, float shadowXVecZ, float shadowYVecX, float shadowYVecY, float shadowYVecZ)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
		m_GLf->glUniform1f(m_sunShadowAngle[sun], shadowAngle);
		m_GLf->glUniform1f(m_sunShadowCount[sun], shadowCount);
		m_GLf->glUniform3f(m_sunShadowXVec[sun], shadowXVecX, shadowXVecY, shadowXVecZ);
		m_GLf->glUniform3f(m_sunShadowYVec[sun], shadowYVecX, shadowYVecY, shadowYVecZ);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSLLavaNoBump shader is used for rendering lava surface

class AUGL_DLL AuGLSLLavaNoBump : public AuGLSLProgram
{
	AuMatrix4x4							m_Projection;			// Current projection matrix set.
	bool								m_ProjectionValid;		// True if projection has been set.

private:
	GLint								m_fogColor;				// Location of 'fogColor' uniform variable.
	GLint								m_fogDensity;			// Location of 'fogDensity' uniform variable.
	GLint								m_mv;					// Location of 'mv' uniform variable.
	GLint								m_OldTexture0;			// Previous texture binding for restore upon deselect.
	GLint								m_p;					// Location of 'p' uniform variable.
	GLint								m_tex;					// Location of 'tex' uniform variable.
	GLint								m_texOffset;			// Location of 'texOffset' uniform variable.

	//--------------------
	// Lighting parameters
	GLint								m_ambient;				// Location of 'ambient' lighting uniform variable.
	GLint								m_eyeLocation;			// Location of 'eyeLocation' lighting uniform variable.

	// Suns
	GLint								m_sunEnabled[MAX_SUNS];	// Location of 'sun.enabled' lighting uniform variable.
	GLint								m_sunAttenuation[MAX_SUNS];	// Location of 'sun.atten' lighting uniform variable.
	GLint								m_sunColor[MAX_SUNS];	// Location of 'sun.color' lighting uniform variable.
	GLint								m_sunPosition[MAX_SUNS];	// Location of 'sun.pos' lighting uniform variable.
	GLint								m_sunRadius[MAX_SUNS];	// Location of 'sun.radius' lighting uniform variable.

	// Point lights
	GLint								m_lightEnabled[MAX_POINT_LIGHTS];
	GLint								m_lightKL[MAX_POINT_LIGHTS];
	GLint								m_lightKQ[MAX_POINT_LIGHTS];
	GLint								m_lightColor[MAX_POINT_LIGHTS];
	GLint								m_lightPosition[MAX_POINT_LIGHTS];
	GLint								m_lightCosOuterCone[MAX_POINT_LIGHTS];
	GLint								m_lightCosInnerCone[MAX_POINT_LIGHTS];
	GLint								m_lightDirection[MAX_POINT_LIGHTS];
	//--------------------

public:
	AuGLSLLavaNoBump()
	{
		m_fogColor = -1;
		m_fogDensity = -1;
		m_mv = -1;
		m_OldTexture0 = 0;
		m_p = -1;
		m_ProjectionValid = false;
		m_tex = -1;
		m_texOffset = -1;

		// Lighting parameters
		m_ambient = -1;
		m_eyeLocation = -1;
		for(int sun=0; sun<MAX_SUNS; sun++)
		{
			m_sunEnabled[sun] = -1;
			m_sunAttenuation[sun] = -1;
			m_sunColor[sun] = -1;
			m_sunPosition[sun] = -1;
			m_sunRadius[sun] = -1;
		}
		for(int light=0; light<MAX_POINT_LIGHTS; light++)
		{
			m_lightEnabled[light] = -1;
			m_lightKL[light] = -1;
			m_lightKQ[light] = -1;
			m_lightColor[light] = -1;
			m_lightPosition[light] = -1;
			m_lightCosOuterCone[light] = -1;
			m_lightCosInnerCone[light] = -1;
			m_lightDirection[light] = -1;
		}
	}

	// Loads and compiles all shaders. Links the shader program. Resolves attribute locations.
	virtual void Build(AuOpenGLFunctions& GLf, const AuGLSLProgramOpt& opt);

	virtual void Deselect()
	{
		AuGLSLProgram::Deselect();

		// Restore previous texture bindings, leaving texture unit 0 as the active unit.
		m_GLf->glActiveTexture(GL_TEXTURE0);
		m_GLf->glBindTexture(GL_TEXTURE_2D, m_OldTexture0);
	}

	virtual QString Name() const
	{
		return QString("Lava No Bump");
	}

	virtual void Select(const AuMatrix4x4& projection);

	// Set ambient light level.
	inline void SetAmbientLight(float ambient) const
	{
		m_GLf->glUniform1f(m_ambient, ambient);
	}

	// Set eye location relative to the object rendered.
	inline void SetEyeLocation(float x, float y, float z) const
	{
		m_GLf->glUniform3f(m_eyeLocation, x,y,z);
	}

	// 'density' is fog density for GL_EXP fog computation.
	//   fog = exp(-density * Zdepth)
	inline void SetFog(float density, float r, float g, float b)
	{
		if(r < 0)
			r = 0;
		else if(r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if(g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if(b > d_FogColorMax)
			b = d_FogColorMax;
		m_GLf->glUniform3f(m_fogColor, r, g, b);
		m_GLf->glUniform1f(m_fogDensity, density * d_FogDensityFactor);
	}

	// 'light' is an index from 0 to MAX_POINT_LIGHTS-1.
	// 'enabled' enables or disables the light.
	//
	// Light:
	//   'kL' is the linear light attenuation value.
	//   'kQ' is the quadratic light attenuation value.
	//   'color' defines the color and intensity of the light.
	//   'loc' is the position of the light in object space
	//
	// Spot light:
	//   'spotCosOuterCone' > -1 and/or 'spotCosInnerCone' > -1 defines a conically constrained spot light. These are the cosine
	//     values for the cone angle off the direction vector. Light is full intensity inside the inner cone, dropping off to 0
	//     intensity from the inner cone to the outer cone.
	//   'spotDir' must be a unit object-space vector when defining a spot light.
	inline void SetLight(int light, bool enabled, float kL, float kQ, float colorR, float colorG, float colorB, float locX,
			float locY, float locZ, float spotCosOuterCone, float spotCosInnerCone, float spotDirX, float spotDirY, float spotDirZ)
	{
		m_GLf->glUniform1i(m_lightEnabled[light], enabled);
		m_GLf->glUniform1f(m_lightKL[light], kL);
		m_GLf->glUniform1f(m_lightKQ[light], kQ);
		m_GLf->glUniform3f(m_lightColor[light], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_lightPosition[light], locX, locY, locZ);
		m_GLf->glUniform1f(m_lightCosOuterCone[light], spotCosOuterCone);
		m_GLf->glUniform1f(m_lightCosInnerCone[light], spotCosInnerCone);
		m_GLf->glUniform3f(m_lightDirection[light], spotDirX, spotDirY, spotDirZ);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'mv' is model view matrix of the globe.
	inline void SetModelView(const AuMatrix4x4& mv) const
	{
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_mv, mv);
	}

	// This shader program must be selected into the current OpenGL context before calling this function.
	// 'p' is projection matrix.
	inline void SetProjection(const AuMatrix4x4& p)
	{
		if(m_ProjectionValid && m_Projection == p)
			return;
		// Set uniform variables.
		SetUniformMatrix(m_GLf, m_p, p);
		m_Projection = p;
		m_ProjectionValid = true;
	}

	// 'loc' is in the object space of this sun
	inline void SetSun(int sun, bool enabled, float attenuation, float colorR, float colorG, float colorB,
		float locX, float locY, float locZ, float diskRadius)
	{
		m_GLf->glUniform1i(m_sunEnabled[sun], enabled);
		m_GLf->glUniform1f(m_sunAttenuation[sun], attenuation);
		m_GLf->glUniform3f(m_sunColor[sun], colorR, colorG, colorB);
		m_GLf->glUniform3f(m_sunPosition[sun], locX, locY, locZ);
		m_GLf->glUniform1f(m_sunRadius[sun], diskRadius);
	}

	// Texture offset causes surface texture to move across the model.
	inline void SetTexOffset(float x, float y)
	{
		m_GLf->glUniform2f(m_texOffset, x, y);
	}

	inline void SetTexture(GLuint texName) const
	{
		m_GLf->glUniform1i(m_tex, 0);									// Configure uniform 'tex' for texture unit 0.
		m_GLf->glActiveTexture(GL_TEXTURE0);							// Select texture unit 0 as active.
		m_GLf->glBindTexture(GL_TEXTURE_2D, texName);					// Bind the texture to the current texture unit.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Add new shader classes above this line.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLSL manages all GLSL shader programs.

class AUGL_DLL AuGLSL
{
private:
	AuOpenGLFunctions*					m_GLf;
	unsigned int						m_MaxLights;

	// Storing only pointers here allows the size of shaders to change without changing the size of this object. That saves some
	// considerable rebuilding while developing new shaders. Only the code that references the specific shader needs to be rebuilt,
	// instead of all objects that reference any shaders at all.
	AuGLSLAnimal*						m_Animal;
	AuGLSLAnimalFog*					m_AnimalFog;
	AuGLSLAnimalGlobe*					m_AnimalGlobe;
	AuGLSLAnimalGlobeFog*				m_AnimalGlobeFog;
	AuGLSLAnimalRingworld*				m_AnimalRingworld;
	AuGLSLAnimalRingworldFog*			m_AnimalRingworldFog;
	AuGLSLAnimalClothing*				m_AnimalClothing;
	AuGLSLAnimalClothingFog*			m_AnimalClothingFog;
	AuGLSLAnimalClothingGlobe*			m_AnimalClothingGlobe;
	AuGLSLAnimalClothingGlobeFog*		m_AnimalClothingGlobeFog;
	AuGLSLAnimalClothingRingworld*		m_AnimalClothingRingworld;
	AuGLSLAnimalClothingRingworldFog*	m_AnimalClothingRingworldFog;
	AuGLSLAnimalHair*					m_AnimalHair;
	AuGLSLAnimalHairFog*				m_AnimalHairFog;
	AuGLSLAnimalHairGlobe*				m_AnimalHairGlobe;
	AuGLSLAnimalHairGlobeFog*			m_AnimalHairGlobeFog;
	AuGLSLAnimalHairRingworld*			m_AnimalHairRingworld;
	AuGLSLAnimalHairRingworldFog*		m_AnimalHairRingworldFog;
	AuGLSLAquaticFlora*					m_AquaticFlora;
	AuGLSLAquaticFloraRingworld*		m_AquaticFloraRingworld;
	AuGLSLAtmosphere*					m_Atmosphere;
	AuGLSLAtmosphereRingworld*			m_AtmosphereRingworld;
	AuGLSLBlackHole*					m_BlackHole;
	AuGLSLBlackHoleSphere*				m_BlackHoleSphere;
	AuGLSLBody*							m_Body;
	AuGLSLBodyFog*						m_BodyFog;
	AuGLSLBodyFogGlobe*					m_BodyFogGlobe;
	AuGLSLBodyGlobe*					m_BodyGlobe;
	AuGLSLBodyRingworld*				m_BodyRingworld;
	AuGLSLBodyFogRingworld*				m_BodyFogRingworld;
	AuGLSLCloud*						m_Cloud;
	AuGLSLCloudRingworld*				m_CloudRingworld;
	AuGLSLColor*						m_Color;
	AuGLSLFarGlobe*						m_FarGlobe;
	AuGLSLFarGlobeLit*					m_FarGlobeLit;
	AuGLSLFarGlobeLitClouds*			m_FarGlobeLitClouds;
	AuGLSLFarGlobeLitLava*				m_FarGlobeLitLava;
	AuGLSLFarGlobeLitLavaClouds*		m_FarGlobeLitLavaClouds;
	AuGLSLFarRingworldLit*				m_FarRingworldLit;
	AuGLSLFarRingworldLitClouds*		m_FarRingworldLitClouds;
	AuGLSLFarStar*						m_FarStar;
	AuGLSLFlag*							m_Flag;
	AuGLSLFlora*						m_Flora;
	AuGLSLFloraRingworld*				m_FloraRingworld;
	AuGLSLGalaxyCloud*					m_GalaxyCloud;
	AuGLSLGalaxyStar*					m_GalaxyStar;
	AuGLSLGasGiant*						m_GasGiant;
	AuGLSLGasGiantLayer*				m_GasGiantLayer;
	AuGLSLGlobe*						m_Globe;
	AuGLSLGlobeFlora*					m_GlobeFlora;
	AuGLSLGlobeLit*						m_GlobeLit;
	AuGLSLGlobeLitFlora*				m_GlobeLitFlora;
	AuGLSLLava*							m_Lava;
	AuGLSLLavaNoBump*					m_LavaNoBump;
	AuGLSLLine*							m_Line;
	AuGLSLLineFog*						m_LineFog;
	AuGLSLModel*						m_Model;
	AuGLSLModelTex*						m_ModelTex;
	AuGLSLModelTexColor*				m_ModelTexColor;
	AuGLSLModelTexFog*					m_ModelTexFog;
	AuGLSLModelTexFogGlobe*				m_ModelTexFogGlobe;
	AuGLSLModelTexFogRingworld*			m_ModelTexFogRingworld;
	AuGLSLNearGlobe*					m_NearGlobe;
	AuGLSLNearGlobeFog*					m_NearGlobeFog;
	AuGLSLNearGlobeLava*				m_NearGlobeLava;
	AuGLSLNearGlobeLavaFog*				m_NearGlobeLavaFog;
	AuGLSLNearRingworld*				m_NearRingworld;
	AuGLSLNearRingworldFog*				m_NearRingworldFog;
	AuGLSLNearRingworldGlow*			m_NearRingworldGlow;
	AuGLSLNearRingworldGlowFog*			m_NearRingworldGlowFog;
	AuGLSLNearStar*						m_NearStar;
	AuGLSLNebula*						m_Nebula;
	AuGLSLPhotonSphere*					m_PhotonSphere;
	AuGLSLRain*							m_Rain;
	AuGLSLRainRingworld*				m_RainRingworld;
	AuGLSLRingworldFlag*				m_RingworldFlag;
	AuGLSLRingworldFlora*				m_RingworldFlora;
	AuGLSLRingworldHull*				m_RingworldHull;
	AuGLSLRingworldHullFog*				m_RingworldHullFog;
	AuGLSLRingworldLitFlora*			m_RingworldLitFlora;
	AuGLSLRingworldStatue*				m_RingworldStatue;
	AuGLSLRingworldTerrainLitFlora*		m_RingworldTerrainLitFlora;
	AuGLSLRingworldTerrainStatue*		m_RingworldTerrainStatue;
	AuGLSLRock*							m_Rock;
	AuGLSLRockRingworld*				m_RockRingworld;
	AuGLSLSc*							m_Sc;
	AuGLSLScFog*						m_ScFog;
	AuGLSLScGlobe*						m_ScGlobe;
	AuGLSLScGlobeFog*					m_ScGlobeFog;
	AuGLSLScRingworld*					m_ScRingworld;
	AuGLSLScRingworldFog*				m_ScRingworldFog;
	AuGLSLShadowSquare*					m_ShadowSquare;
	AuGLSLShield*						m_Shield;
	AuGLSLShieldHits*					m_ShieldHits;
	AuGLSLSkyBox*						m_SkyBox;
	AuGLSLSnow*							m_Snow;
	AuGLSLSnowRingworld*				m_SnowRingworld;
	AuGLSLSplat*						m_Splat;
	AuGLSLSpriteActualSize*				m_SpriteActualSize;
	AuGLSLSpriteSize*					m_SpriteSize;
	AuGLSLSpriteSizeColor*				m_SpriteSizeColor;
	AuGLSLTerrainLit*					m_TerrainLit;
	AuGLSLTerrainLitFlora*				m_TerrainLitFlora;
	AuGLSLTexBar*						m_TexBar;
	AuGLSLTexOffset*					m_TexOffset;
	AuGLSLTexture*						m_Texture;
	AuGLSLWarpTunnel*					m_WarpTunnel;
	AuGLSLWater*						m_Water;
	AuGLSLWaterNoBump*					m_WaterNoBump;
	AuGLSLWaterNoBumpNoHole*			m_WaterNoBumpNoHole;
	AuGLSLWaterNoHole*					m_WaterNoHole;
	AuGLSLWaterRingworld*				m_WaterRingworld;
	AuGLSLWaterRingworldNoBump*			m_WaterRingworldNoBump;
	AuGLSLWaterRingworldNoBumpNoHole*	m_WaterRingworldNoBumpNoHole;
	AuGLSLWaterRingworldNoHole*			m_WaterRingworldNoHole;

public:
	AuGLSL();
	virtual ~AuGLSL();

	inline AuGLSLAnimal& AnimalShader()
	{
		return *m_Animal;
	}

	inline AuGLSLAnimalFog& AnimalFogShader()
	{
		return *m_AnimalFog;
	}

	inline AuGLSLAnimalGlobe& AnimalGlobeShader()
	{
		return *m_AnimalGlobe;
	}

	inline AuGLSLAnimalGlobeFog& AnimalGlobeFogShader()
	{
		return *m_AnimalGlobeFog;
	}

	inline AuGLSLAnimalRingworld& AnimalRingworldShader()
	{
		return *m_AnimalRingworld;
	}

	inline AuGLSLAnimalRingworldFog& AnimalRingworldFogShader()
	{
		return *m_AnimalRingworldFog;
	}

	inline AuGLSLAnimalClothing& AnimalClothingShader()
	{
		return *m_AnimalClothing;
	}

	inline AuGLSLAnimalClothingFog& AnimalClothingFogShader()
	{
		return *m_AnimalClothingFog;
	}

	inline AuGLSLAnimalClothingGlobe& AnimalClothingGlobeShader()
	{
		return *m_AnimalClothingGlobe;
	}

	inline AuGLSLAnimalClothingGlobeFog& AnimalClothingGlobeFogShader()
	{
		return *m_AnimalClothingGlobeFog;
	}

	inline AuGLSLAnimalClothingRingworld& AnimalClothingRingworldShader()
	{
		return *m_AnimalClothingRingworld;
	}

	inline AuGLSLAnimalClothingRingworldFog& AnimalClothingRingworldFogShader()
	{
		return *m_AnimalClothingRingworldFog;
	}

	inline AuGLSLAnimalHair& AnimalHairShader()
	{
		return *m_AnimalHair;
	}

	inline AuGLSLAnimalHairFog& AnimalHairFogShader()
	{
		return *m_AnimalHairFog;
	}

	inline AuGLSLAnimalHairGlobe& AnimalHairGlobeShader()
	{
		return *m_AnimalHairGlobe;
	}

	inline AuGLSLAnimalHairGlobeFog& AnimalHairGlobeFogShader()
	{
		return *m_AnimalHairGlobeFog;
	}

	inline AuGLSLAnimalHairRingworld& AnimalHairRingworldShader()
	{
		return *m_AnimalHairRingworld;
	}

	inline AuGLSLAnimalHairRingworldFog& AnimalHairRingworldFogShader()
	{
		return *m_AnimalHairRingworldFog;
	}

	inline AuGLSLAquaticFlora& AquaticFloraShader()
	{
		return *m_AquaticFlora;
	}

	inline AuGLSLAquaticFloraRingworld& AquaticFloraRingworldShader()
	{
		return *m_AquaticFloraRingworld;
	}

	inline AuGLSLAtmosphere& AtmosphereShader()
	{
		return *m_Atmosphere;
	}

	inline AuGLSLAtmosphereRingworld& AtmosphereRingworldShader()
	{
		return *m_AtmosphereRingworld;
	}

	inline AuGLSLBlackHole& BlackHoleShader()
	{
		return *m_BlackHole;
	}

	inline AuGLSLBlackHoleSphere& BlackHoleSphereShader()
	{
		return *m_BlackHoleSphere;
	}

	inline AuGLSLBody& BodyShader()
	{
		return *m_Body;
	}

	inline AuGLSLBodyFog& BodyFogShader()
	{
		return *m_BodyFog;
	}

	inline AuGLSLBodyFogGlobe& BodyFogGlobeShader()
	{
		return *m_BodyFogGlobe;
	}

	inline AuGLSLBodyGlobe& BodyGlobeShader()
	{
		return *m_BodyGlobe;
	}

	inline AuGLSLBodyRingworld& BodyRingworldShader()
	{
		return *m_BodyRingworld;
	}

	inline AuGLSLBodyFogRingworld& BodyFogRingworldShader()
	{
		return *m_BodyFogRingworld;
	}

	inline AuGLSLCloud& CloudShader()
	{
		return *m_Cloud;
	}

	inline AuGLSLCloudRingworld& CloudRingworldShader()
	{
		return *m_CloudRingworld;
	}

	inline AuGLSLColor& ColorShader()
	{
		return *m_Color;
	}

	// Deselects shaders from the GL context to restore default OpenGL behavior.
	// OpenGL context must be current when this function is called.
	// NOTE: It is preferable to deselect the individual program using its AuGLSLProgram::Deselect function.
	inline void DeselectProgram()
	{
		m_GLf->glUseProgram(0);
	}

	inline AuGLSLFarGlobe& FarGlobeShader()
	{
		return *m_FarGlobe;
	}

	inline AuGLSLFarGlobeLit& FarGlobeLitShader()
	{
		return *m_FarGlobeLit;
	}

	inline AuGLSLFarGlobeLitClouds& FarGlobeLitCloudsShader()
	{
		return *m_FarGlobeLitClouds;
	}

	inline AuGLSLFarGlobeLitLava& FarGlobeLitLavaShader()
	{
		return *m_FarGlobeLitLava;
	}

	inline AuGLSLFarGlobeLitLavaClouds& FarGlobeLitLavaCloudsShader()
	{
		return *m_FarGlobeLitLavaClouds;
	}

	inline AuGLSLFarRingworldLit& FarRingworldLitShader()
	{
		return *m_FarRingworldLit;
	}

	inline AuGLSLFarRingworldLitClouds& FarRingworldLitCloudsShader()
	{
		return *m_FarRingworldLitClouds;
	}

	inline AuGLSLFarStar& FarStarShader()
	{
		return *m_FarStar;
	}

	inline AuGLSLFlag& FlagShader()
	{
		return *m_Flag;
	}

	inline AuGLSLFlora& FloraShader()
	{
		return *m_Flora;
	}

	inline AuGLSLFloraRingworld& FloraRingworldShader()
	{
		return *m_FloraRingworld;
	}

	inline AuGLSLGalaxyCloud& GalaxyCloudShader()
	{
		return *m_GalaxyCloud;
	}

	inline AuGLSLGalaxyStar& GalaxyStarShader()
	{
		return *m_GalaxyStar;
	}

	inline AuGLSLGasGiant& GasGiantShader()
	{
		return *m_GasGiant;
	}

	inline AuGLSLGasGiantLayer& GasGiantLayerShader()
	{
		return *m_GasGiantLayer;
	}

	inline AuGLSLGlobeFlora& GlobeFloraShader()
	{
		return *m_GlobeFlora;
	}

	inline AuGLSLGlobe& GlobeShader()
	{
		return *m_Globe;
	}

	inline AuGLSLGlobeLit& GlobeLitShader()
	{
		return *m_GlobeLit;
	}

	inline AuGLSLGlobeLitFlora& GlobeLitFloraShader()
	{
		return *m_GlobeLitFlora;
	}

	// Builds all known GLSL shader programs.
	// OpenGL context must be current when this function is called.
	void Initialize(AuOpenGLFunctions& GLf, unsigned int maxLights);

	// Calls InvalidateLights on all shaders with object space lights.
	void InvalidateLights();

	// Calls InvalidateSuns on all shaders with object space suns.
	void InvalidateSuns();

	inline AuGLSLLava& LavaShader()
	{
		return *m_Lava;
	}

	inline AuGLSLLavaNoBump& LavaNoBumpShader()
	{
		return *m_LavaNoBump;
	}

	inline AuGLSLLine& LineShader()
	{
		return *m_Line;
	}

	inline AuGLSLLineFog& LineFogShader()
	{
		return *m_LineFog;
	}

	inline unsigned int MaxLights() const
	{
		return m_MaxLights;
	}

	inline AuGLSLModel& ModelShader()
	{
		return *m_Model;
	}

	inline AuGLSLModelTex& ModelTexShader()
	{
		return *m_ModelTex;
	}

	inline AuGLSLModelTexColor& ModelTexColorShader()
	{
		return *m_ModelTexColor;
	}

	inline AuGLSLModelTexFog& ModelTexFogShader()
	{
		return *m_ModelTexFog;
	}

	inline AuGLSLModelTexFogGlobe& ModelTexFogGlobeShader()
	{
		return *m_ModelTexFogGlobe;
	}

	inline AuGLSLModelTexFogRingworld& ModelTexFogRingworldShader()
	{
		return *m_ModelTexFogRingworld;
	}

	inline AuGLSLNearGlobe& NearGlobeShader()
	{
		return *m_NearGlobe;
	}

	inline AuGLSLNearGlobeFog& NearGlobeFogShader()
	{
		return *m_NearGlobeFog;
	}

	inline AuGLSLNearGlobeLava& NearGlobeLavaShader()
	{
		return *m_NearGlobeLava;
	}

	inline AuGLSLNearGlobeLavaFog& NearGlobeLavaFogShader()
	{
		return *m_NearGlobeLavaFog;
	}

	inline AuGLSLNearRingworldFog& NearRingworldFogShader()
	{
		return *m_NearRingworldFog;
	}

	inline AuGLSLNearRingworldGlowFog& NearRingworldGlowFogShader()
	{
		return *m_NearRingworldGlowFog;
	}

	inline AuGLSLNearRingworldGlow& NearRingworldGlowShader()
	{
		return *m_NearRingworldGlow;
	}

	inline AuGLSLNearRingworld& NearRingworldShader()
	{
		return *m_NearRingworld;
	}

	inline AuGLSLNearStar& NearStarShader()
	{
		return *m_NearStar;
	}

	inline AuGLSLNebula& NebulaShader()
	{
		return *m_Nebula;
	}

	inline AuGLSLPhotonSphere& PhotonSphereShader()
	{
		return *m_PhotonSphere;
	}

	inline AuGLSLRain& RainShader()
	{
		return *m_Rain;
	}

	inline AuGLSLRainRingworld& RainRingworldShader()
	{
		return *m_RainRingworld;
	}

	inline AuGLSLRingworldFlag& RingworldFlagShader()
	{
		return *m_RingworldFlag;
	}

	inline AuGLSLRingworldFlora& RingworldFloraShader()
	{
		return *m_RingworldFlora;
	}

	inline AuGLSLRingworldHull& RingworldHullShader()
	{
		return *m_RingworldHull;
	}

	inline AuGLSLRingworldHullFog& RingworldHullFogShader()
	{
		return *m_RingworldHullFog;
	}

	inline AuGLSLRingworldLitFlora& RingworldLitFloraShader()
	{
		return *m_RingworldLitFlora;
	}

	inline AuGLSLRingworldStatue& RingworldStatueShader()
	{
		return *m_RingworldStatue;
	}

	inline AuGLSLRingworldTerrainLitFlora& RingworldTerrainLitFloraShader()
	{
		return *m_RingworldTerrainLitFlora;
	}

	inline AuGLSLScFog& ScFogShader()
	{
		return *m_ScFog;
	}

	inline AuGLSLSc& ScShader()
	{
		return *m_Sc;
	}

	inline AuGLSLScGlobeFog& ScGlobeFogShader()
	{
		return *m_ScGlobeFog;
	}

	inline AuGLSLScGlobe& ScGlobeShader()
	{
		return *m_ScGlobe;
	}

	inline AuGLSLScRingworldFog& ScRingworldFogShader()
	{
		return *m_ScRingworldFog;
	}

	inline AuGLSLScRingworld& ScRingworldShader()
	{
		return *m_ScRingworld;
	}

	inline AuGLSLSplat& SplatShader()
	{
		return *m_Splat;
	}

	inline AuGLSLTexBar& TexBarShader()
	{
		return *m_TexBar;
	}

	inline AuGLSLTexOffset& TexOffsetShader()
	{
		return *m_TexOffset;
	}

	inline AuGLSLTexture& TextureShader()
	{
		return *m_Texture;
	}

	inline AuGLSLRingworldTerrainStatue& RingworldTerrainStatueShader()
	{
		return *m_RingworldTerrainStatue;
	}

	inline AuGLSLRock& RockShader()
	{
		return *m_Rock;
	}

	inline AuGLSLRockRingworld& RockRingworldShader()
	{
		return *m_RockRingworld;
	}

	inline AuGLSLShadowSquare& ShadowSquareShader()
	{
		return *m_ShadowSquare;
	}

	inline AuGLSLShield& ShieldShader()
	{
		return *m_Shield;
	}

	inline AuGLSLShieldHits& ShieldHitsShader()
	{
		return *m_ShieldHits;
	}

	inline AuGLSLSkyBox& SkyBoxShader()
	{
		return *m_SkyBox;
	}

	inline AuGLSLSnow& SnowShader()
	{
		return *m_Snow;
	}

	inline AuGLSLSnowRingworld& SnowRingworldShader()
	{
		return *m_SnowRingworld;
	}

	inline AuGLSLSpriteActualSize& SpriteActualSizeShader()
	{
		return *m_SpriteActualSize;
	}

	inline AuGLSLSpriteSize& SpriteSizeShader()
	{
		return *m_SpriteSize;
	}

	inline AuGLSLSpriteSizeColor& SpriteSizeColorShader()
	{
		return *m_SpriteSizeColor;
	}

	inline AuGLSLTerrainLit& TerrainLitShader()
	{
		return *m_TerrainLit;
	}

	inline AuGLSLTerrainLitFlora& TerrainLitFloraShader()
	{
		return *m_TerrainLitFlora;
	}

	inline AuGLSLWarpTunnel& WarpTunnelShader()
	{
		return *m_WarpTunnel;
	}

	inline AuGLSLWater& WaterShader()
	{
		return *m_Water;
	}

	inline AuGLSLWaterNoBump& WaterNoBumpShader()
	{
		return *m_WaterNoBump;
	}

	inline AuGLSLWaterRingworld& WaterRingworldShader()
	{
		return *m_WaterRingworld;
	}

	inline AuGLSLWaterRingworldNoBump& WaterRingworldNoBumpShader()
	{
		return *m_WaterRingworldNoBump;
	}

	inline AuGLSLWaterNoHole& WaterNoHoleShader()
	{
		return *m_WaterNoHole;
	}

	inline AuGLSLWaterNoBumpNoHole& WaterNoBumpNoHoleShader()
	{
		return *m_WaterNoBumpNoHole;
	}

	inline AuGLSLWaterRingworldNoHole& WaterRingworldNoHoleShader()
	{
		return *m_WaterRingworldNoHole;
	}

	inline AuGLSLWaterRingworldNoBumpNoHole& WaterRingworldNoBumpNoHoleShader()
	{
		return *m_WaterRingworldNoBumpNoHole;
	}
};

#endif // AuGLSL_H
