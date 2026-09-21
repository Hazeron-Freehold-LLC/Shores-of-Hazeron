//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomrenderinfo.h,v 1.6 2022/02/19 17:30:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomRenderInfo_H
#define AuAtomRenderInfo_H

#include "auatom.h"
#include "augleffects.h"
#include "auscface.h"
#include "dbcommodity.h"
#include <QOpenGLWidget>

class Au3dsGLSLModelDefinition;
class Au3dsGLSLModelInstance;
class AuGLModelInstance;
class AuGLModelMaker;
class AuGLModelCache;
class AuSound;

class AUATOM_DLL AuAtomRenderInfo
{
public:
	// View info accompanies view mode to specify what information is overlayed.
	enum TOP_VIEW_INFO
	{
		VI_None,						// no info overlayed in top down view
		VI_RocksAll,					// rocks
		VI_RocksSome,					// rocks without stone or eludium
		VI_Plants,						// plants
		VI_Buildings,					// developments except roads
		VI_Roads,						// roads
		VI_Flags,						// flags
		VI_CityNames,					// city names
	};

	enum VIEW_MODE
	{
		VM_FirstPerson,					// Camera is positioned at the eye location of the character's body.
		VM_ThirdPerson,					// Camera is positioned behind and slightly above the character's body.
		VM_TopDown,						// Camera is positioned directly above the character's body, pointing down.
	};

public:
	AuSound*							al;						// May be 0 for no sounds.
	float								fade;					// Overall modulator applied to all sounds. 0=silent, 1=loud
	const QFont&						font;
	QOpenGLWidget*						glw;
	AuGLModelMaker&						mm;
	AuGLEffects*						re;						// Created/deleted by this object.
	float								textBrightness;			// 0..1
	int									viewDeck;
	bool								m_ShowHUD;
	bool								m_ShowPaths;

private:
	AuGLModelCache&						m_ModelCache;			// Cache of 3d models.

	// Designer
private:
	quint16								m_AnchorPart;			// Anchor part for DaDesigner selection logic. 0=no anchor part
	bool								m_DesignerObjectIsolate;	// True when object mode is isolated. Applies in object mode.
	bool								m_DesignerObjectMode;	// True when object mode is active.
	AuScSelection						m_DesignerSelection;	// Selected parts in designer.
	mutable AuScSelection				m_DesignerSelectionParts;	// Top-level selected parts in designer.
public:
	bool								m_DesignerShowBacks;
	bool								m_DesignerShowEdges;
	bool								m_DesignerShowFaces;
	bool								m_DesignerShowGround;
	bool								m_DesignerShowSkyBox;
private:
	bool								m_Drag;					// True while dragging highlighted elements.
	quint16								m_DragPart;				// Drag part for DaDesigner selection logic. 0=no drag part
	AuMatrix4x4							m_DragPlane;			// Design plane for drag operation.
public:
	bool								m_DragCopy;				// True if drag transformation is copying, false when moving.
	AuMatrix4x4							m_DragT;				// Drag transformation when moving/copying selected elements.

public:
	// Camera
	float								m_ViewAlt;				// camera altitude in top down view mode.
	float								m_ViewAngle1st;			// camera angle in first person view mode, in radians.
	float								m_ViewAngle3rd;			// camera angle in third person view mode, in radians.
	float								m_ViewDist1st;			// camera distance behind eye in first person view mode, in feet.
	float								m_ViewDist3rd;			// camera distance behind eye in third person view mode, in feet,

public:
	// Textures
	mutable QHash<COMMODITYID, GLuint>	m_CommodityTextures;
	mutable QHash<COMMODITYID, GLuint>	m_CommodityTexturesN;

private:
	AuId								m_SelectedBd;			// Bd selected on capitol/hq ui.
	TOP_VIEW_INFO						m_TopViewInfo;
	VIEW_MODE							m_ViewMode;

public:
	// 'pal' may be 0.
	AuAtomRenderInfo(QOpenGLWidget* pglw, AuSound* pal, AuGLModelMaker& pmm, AuGLModelCache& mc, const QFont& atomFont);
	~AuAtomRenderInfo();

public:
	inline quint16 AnchorPart() const
	{
		return m_AnchorPart;
	}

	inline bool AnchorPartSelected() const
	{
		return m_AnchorPart && m_DesignerSelection.contains(m_AnchorPart);
	}

	inline void ClearAnchorPart()
	{
		m_AnchorPart = 0;
	}

	inline void ClearDragPart()
	{
		m_DragPart = 0;
	}

	// The texture returned is flipped in Y, for sprites. It is upside down.
	GLuint CommodityTextureF(COMMODITYID cid) const;

	// The texture returned is NOT flipped in Y. It is right side up.
	GLuint CommodityTextureN(COMMODITYID cid) const;

	// 'min' and 'max' are output parameters set to the 3d extents of the
	// vertices of the 3ds model.
	Au3dsGLSLModelInstance* CreateBodyModelGLSL(const QString& fileName, AuPoint& min, AuPoint& max, bool bLightsOn,
			const Au3dsGLSLModelDefinition& modelDef);

	AuGLModelInstance* CreateModelGLSL(const QString& fileName, bool bLightsOn=false, double scale=1.);

	void CreatePatchedModelGLSL(Au3dsGLSLModelInstance* pm);

	void DeleteModelGLSL(AuGLModelInstance* pm);

	void DeleteSound(unsigned int audioBuffer);

	inline void DesignerClearPartSelection()
	{
		m_DesignerSelection.clear();
	}

	inline void DesignerClearPartObjectSelection()
	{
		m_DesignerSelection.ClearObjectSelections();
	}

	inline bool DesignerHasPartSelection() const
	{
		return !m_DesignerSelection.empty();
	}

	inline bool DesignerHasPartObjectSelection() const
	{
		return m_DesignerSelection.HasObjectSelection();
	}

	inline bool DesignerObjectIsolate() const
	{
		return m_DesignerObjectIsolate;
	}

	inline bool DesignerObjectMode() const
	{
		return m_DesignerObjectMode;
	}

	// Returns part id that is the target of a paste operation when in object mode. More than one part can be operated on at once
	// in object mode. Returns 0 if not in object mode.
	inline quint16 DesignerObjectModeTargetPart() const
	{
		if(!DesignerObjectMode())
			return 0;
		if(m_DragPart && m_DesignerSelection.contains(m_DragPart))
			return m_DragPart;
		else if(!m_DesignerSelection.empty())
			return m_DesignerSelection.begin().key();
		return 0;
	}

	// Returns the current top-level selection. i.e. No vertices, edges and faces.
	inline const AuScSelection& DesignerPartSelection() const
	{
		m_DesignerSelectionParts = m_DesignerSelection;
		m_DesignerSelectionParts.ClearObjectSelections();
		return m_DesignerSelectionParts;
	}

	// Returns the full selection with sub object selections. i.e. vertices, edges and faces.
	inline const AuScSelection& DesignerPartObjectSelection() const
	{
		return m_DesignerSelection;
	}

	inline AuScSelection& DesignerSelection()
	{
		return m_DesignerSelection;
	}

	void DragEnd();

	inline quint16 DragPart() const
	{
		return m_DragPart;
	}

	inline const AuMatrix4x4& DragPlane() const
	{
		return m_DragPlane;
	}

	inline bool DragPartSelected() const
	{
		return m_DragPart && m_DesignerSelection.contains(m_DragPart);
	}

	// 'dragRef' is the drag reference location, ideally the point that will be used for snapping the drag selection, such as the
	// grip at a vertex.
	// 'designPlane' is the design plane when the drag started.
	void DragStart(const AuPoint& dragRef, const AuMatrix4x4& designPlane);

	inline bool HasAnchorPart() const
	{
		return m_AnchorPart;
	}

	inline bool HasDragPart() const
	{
		return m_DragPart;
	}

	void InitializeGL();

	// Returns true during a drag operation.
	inline bool IsDragging() const
	{
		return m_Drag;
		//if(DesignerObjectMode())
			//return DragPartSelected();
		//else
			//return AnchorPartSelected();
	}

	// Returns true during a drag operation that will result in a copy. e.g. Ctrl key held down during mouse drag.
	inline bool IsDragCopying() const
	{
		return IsDragging() && m_DragCopy;
	}

	// Returns true during a drag operation that will result in a move. e.g. Ctrl key NOT held down during mouse drag.
	inline bool IsDragMoving() const
	{
		return IsDragging() && !m_DragCopy;
	}

	inline bool IsViewFirstPerson() const
	{
		return m_ViewMode == VM_FirstPerson;
	}

	inline bool IsViewThirdPerson() const
	{
		return m_ViewMode == VM_ThirdPerson;
	}

	inline bool IsViewTopDown() const
	{
		return m_ViewMode == VM_TopDown;
	}

	// Creates a model instance and triggers a thread to begin loading the model.
	// The model's GLModel() function will respond with a nonzero value when it is ready for use.
	AuGLModelInstance* LoadModelThreaded(const QString& fileName, bool bLightsOn=false, double scale=1.);

	inline AuGLModelCache& Models()
	{
		return m_ModelCache;
	}

	// 'curSound' and 'sound' are SOUND_EFFECT values
	void PlaySound(unsigned int& audioSource, int& curSound, int sound, float posX, float posY, float posZ, float gain,
			float pitch, bool loopSound);

	// 'curSound' and 'sound' are SOUND_EFFECT values
	void PlaySound(unsigned int& audioSource, int& curSound, int sound, float posX, float posY, float posZ, float gain,
			float pitch, bool loopSound, float referenceDistance, float rolloffFactor, float secOffset=0);

	void PlaySound(unsigned int& audioSource, unsigned int audioBuffer, float posX, float posY, float posZ, float gain,
			float pitch, float referenceDistance, float rolloffFactor);

	// Must have certain rendering states already set up before calling these hud rendering functions.
	// Used by aircraft and spacecraft helm.
	inline void RenderAircraftHud(float xpos, float ypos, float alt, float pitch, float roll, float heading, float speed,
			float vspeed, float lspeed, float longitude, float latitude, int hudIntensity, float hullStress, int throttle,
			float etesecs, bool inEnemyRange, bool isHolding) const
	{
		re->RenderAircraftHud(glw->rect(), xpos, ypos, alt/FeetPerMeter(), pitch, roll, heading, speed/FeetPerMeter(),
				vspeed/FeetPerMeter(), lspeed/FeetPerMeter(), longitude, latitude, hudIntensity, hullStress, throttle, etesecs,
				inEnemyRange, isHolding);
	}

	inline void RenderAPCHud(float xpos, float ypos, float heading, float longitude, float latitude, int hudIntensity,
			bool inEnemyRange) const
	{
		re->RenderAPCHud(glw->rect(), xpos, ypos, heading, longitude, latitude, hudIntensity, inEnemyRange);
	}

	// Renders a horizontal status bar from the HudItems.png texture.
	// This function sets up its own shaders.
	// 'value' is clamped to the range 0 to 1.
	// 'whichBar' 0=vehicle health/wrench, 1=fuel/gas pump, 2=body health/heart, 3=stamina/lungs, 4=hunger/stomach
	inline void RenderBar(float left, float bottom, float barwid, float barht, float value, int whichBar,
			float rleft, float gleft, float bleft, float rright, float gright, float bright, float alpha=1.f) const
	{
		re->RenderBar(glw->rect(), left, bottom, barwid, barht, value, whichBar, rleft,gleft,bleft, rright,gright,bright, alpha);
	}

	// 'targetLevel' is clamped to the range 0 to 1.
	inline void RenderCrosshair(float xpos, float ypos, float targetLevel,
			float r, float g, float b, float a=1.f) const
	{
		re->RenderCrosshair(glw->rect(), xpos, ypos, targetLevel, r,g,b,a);
	}

	// Renders a horizontal status bar from the HudItems.png texture.
	// This function sets up its own shaders.
	// 'value' is clamped to the range 0 to 1.
	// 'whichBar' 0=vehicle health/wrench, 1=fuel/gas pump, 2=body health/heart, 3=stamina/lungs, 4=hunger/stomach
	inline void RenderItemConditionBar(float left, float bottom, float barwid, float barht, float value,
			float rbottom, float gbottom, float bbottom, float rtop, float gtop, float btop, float alpha=1.f) const
	{
		re->RenderItemConditionBar(glw->rect(), left, bottom, barwid, barht, value, rbottom,gbottom,bbottom, rtop,gtop,btop, alpha);
	}

	inline void RenderItemEffect(ITEMEFFECT effect, float left, float bottom, float size,
			float r, float g, float b, float a=1.f) const
	{
		re->RenderItemEffect(glw->rect(), effect, left, bottom, size, r,g,b,a);
	}

	// 'value' is clamped to the range 0 to 1.
	inline void RenderJumpSpring(float left, float bottom, float barwid, float barht, float value,
			float r, float g, float b, float a) const
	{
		re->RenderJumpSpring(glw->rect(), left, bottom, barwid, barht, value, r, g, b, a);
	}

	// 'value' is clamped to the range 0 to 1.
	inline void RenderMovePower(float left, float bottom, float barwid, float barht, float value,
			float r, float g, float b, float a) const
	{
		re->RenderMovePower(glw->rect(), left, bottom, barwid, barht, value, r, g, b, a);
	}

	// Renders pulsating circle at screen position.
	inline void RenderLocation(float xpos, float ypos, float size, float r, float g, float b, float a=1.f) const
	{
		re->RenderLocation(glw->rect(), xpos, ypos, size, r,g,b,a);
	}

	// This version of RenderNumber sets up its own shaders.
	// 'justify' 0=left, 1=center, 2=right
	// 'sign' is 0=minus when negative, 1=plus and minus always, 2=NS for latitude, 3=EW for longitude
	inline float RenderNumber(int value, float left, float base, float ht, int justify, int sign,
			float r, float g, float b, float a) const
	{
		return re->RenderNumber(glw->rect(), value, left, base, ht, justify, sign, r,g,b,a);
	}

	inline void RenderOrientationIndicator(float xpos, float ypos, float zRot, float size,
			float r, float g, float b, float a=1.f) const
	{
		re->RenderOrientationIndicator(glw->rect(), xpos, ypos, zRot, size, r,g,b,a);
	}

	inline void RenderSail(float xpos, float ypos, float sizeWid, float r=1.f, float g=1.f, float b=1.f, float a=1.f) const
	{
		re->RenderSail(glw->rect(), xpos, ypos, sizeWid, r,g,b,a);
	}

	inline void RenderSpeedometer(float xpos, float ypos, float size, float speedFps, float r=1.f, float g=1.f, float b=1.f,
			float a=1.f) const
	{
		// Convert feet per second to statute miles per hour.
		// 5280 feet = 1 statute mile
		float speedMph = fabs(speedFps) * 3600.f / 5280.f;
		re->RenderSpeedometer(glw->rect(), xpos, ypos, size, speedMph, r,g,b,a);
	}

	// 'sizeX' can be negative to flip in X
	// 'sizeY' can be negative to flip in Y
	void RenderSprite(float xpos, float ypos, float sizeX, float sizeY, GLuint texture, float r, float g, float b,
			float a=1.f) const
	{
		re->RenderSprite(glw->rect(), xpos, ypos, sizeX, sizeY, texture, r,g,b,a);
	}

	// 'system' matches the enum SYSTEM.
	inline void RenderSystemIcon(int system, float left, float bottom, float size, float r, float g, float b, float a=1.f) const
	{
		re->RenderSystemIcon(glw->rect(), system, left, bottom, size, r,g,b,a);
	}

	// Render text at a pixel position on the screen using OpenGL.
	// 'x' and 'y' are pixel positions on the screen, with 0,0 at upper left of screen. Coord specifies lower left of text.
	// 'str' is the string to display.
	// 'font' is the font to use.
	// 'viewX' and 'viewY' are the pixel dimensions of the viewport.
	// 'flags' are those documented in QPainter::drawText. 'wrapWidth' in pixels is used when Qt::TextWordWrap is specified; it may
	//   also affect Qt::AlignJustify.
	// NOTE: glDepthMask and GL_DEPTH_TEST are disabled while rendering the text.
	// NOTE: With no blending enabled, the text appears on a solid black background rectangle.
	// NOTE: Qt::TextWordWrap and Qt::AlignLeft together either conflict or invoke logic I don't understand because sometimes the
	//   lines don't wordwrap. Qt::TextWordWrap alone aligns to the left by default and seems to work properly.
	// Returns the pixel height of the text item.
	inline int RenderText(int x, int y, const QString& str, const QColor& color,
			int flags=Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, int wrapWidth=1) const
	{
		return re->RenderText(x,y, str, font, color, glw->width(), glw->height(), flags, wrapWidth);
	}

	// Render text at a model space position on the screen using OpenGL.
	// 'x', 'y' and 'z' are model space positions on the screen. Coord specifies lower left of text.
	// 'str' is the string to display.
	// 'font' is the font to use.
	// 'viewY' is the pixel height of the viewport.
	// 'flags' are those documented in QPainter::drawText. 'wrapWidth' in pixels is used when Qt::TextWordWrap is specified; it may
	//   also affect Qt::AlignJustify.
	// NOTE: With no blending enabled, the text appears on a solid black background rectangle.
	// NOTE: Qt::TextWordWrap and Qt::AlignLeft together either conflict or invoke logic I don't understand because sometimes the
	//   lines don't wordwrap. Qt::TextWordWrap alone aligns to the left by default and seems to work properly.
	inline void RenderText(const AuMatrix4x4& projection, const AuMatrix4x4& model, float x, float y, float z, const QString& str,
			const QColor& color, int flags=Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, int wrapWidth=1) const
	{
		re->RenderText(projection, model, AuPoint(x,y,z), str, font, color, glw->height(), flags, wrapWidth);
	}

	inline void RenderText(const AuMatrix4x4& projection, const AuMatrix4x4& model, const AuPoint& loc, const QString& str,
			const QColor& color, int flags=Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, int wrapWidth=1) const
	{
		re->RenderText(projection, model, loc, str, font, color, glw->height(), flags, wrapWidth);
	}

	inline void RenderWeaponEffect(WEAPONEFFECT effect, float left, float bottom, float size,
			float r, float g, float b, float a=1.f) const
	{
		re->RenderWeaponEffect(glw->rect(), effect, left, bottom, size, r,g,b,a);
	}

	inline void RenderWeaponMode(WEAPONMODE mode, float left, float bottom, float size,
			float r, float g, float b, float a=1.f) const
	{
		re->RenderWeaponMode(glw->rect(), mode, left, bottom, size, r,g,b,a);
	}

	inline void ResetViewport() const
	{
		re->glViewport(0, 0, glw->width(), glw->height());
	}

	inline const AuId& SelectedBd() const
	{
		return m_SelectedBd;
	}

	inline void SetAnchorPart(quint16 partId)
	{
		m_AnchorPart = partId;
	}

	inline void SetDesignerObjectIsolate(bool objectIsolate)
	{
		m_DesignerObjectIsolate = objectIsolate;
	}

	inline void SetDesignerObjectMode(bool objectMode)
	{
		m_DesignerObjectMode = objectMode;
	}

	inline void SetDragPart(quint16 partId)
	{
		m_DragPart = partId;
	}

	inline void SetSelectedBd(const AuId& idBd)
	{
		m_SelectedBd = idBd;
	}

	inline void SetTopViewInfo(TOP_VIEW_INFO topViewInfo)
	{
		m_TopViewInfo = topViewInfo;
	}

	inline void SetViewMode(VIEW_MODE viewMode)
	{
		m_ViewMode = viewMode;
	}

	void StopSound(unsigned int& audioSource);

	inline AuAtomRenderInfo::TOP_VIEW_INFO TopViewInfo() const
	{
		return m_TopViewInfo;
	}

	QString TopViewInfoName() const;

	inline AuAtomRenderInfo::VIEW_MODE ViewMode() const
	{
		return m_ViewMode;
	}
};

#endif // AuAtomRenderInfo_H
