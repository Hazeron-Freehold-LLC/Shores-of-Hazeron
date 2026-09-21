//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubodyrenderwidget.h,v 1.22 2022/05/16 23:16:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatom.h"
#include "audnabase.h"
//#include "augleffects.h"
#include "auhair.h"
#include "aupose.h"
#include "dbcommodity.h"
#include <QOpenGLWidget>

class AuAnimalia;
class AuDNA;
class AuGLEffects;
class AuGLModelCache;
class DhDNA;
class DpBody;

class AUATOM_DLL AuBodyRenderWidget : public QOpenGLWidget
{
	Q_OBJECT;      												// must include this to use Qt signals/slots
	// Scavenged from DaSpaceShip for the deck scene.
	enum SHIP_TEXTURE
	{
		ST_Berth,
		ST_BerthBottom,
		ST_LargeDoorClosed,
		ST_LargeDoorHalfOpen,
		ST_LargeDoorOpen,
		ST_LargeCrewDoorClosed,
		ST_LargeCrewDoorHalfOpen,
		ST_LargeCrewDoorOpen,
		ST_LargeHullDoorClosed,
		ST_LargeHullDoorHalfOpen,
		ST_LargeHullDoorOpen,
		ST_CrewDoorClosed,
		ST_CrewDoorHalfOpen,
		ST_CrewDoorOpen,
		ST_CrewHatchClosed,
		ST_CrewHatchHalfOpen,
		ST_CrewHatchOpen,
		ST_GravityDrive,
		ST_HullBottom,
		ST_HullDoorClosed,
		ST_HullDoorHalfOpen,
		ST_HullDoorOpen,
		ST_HullHatchClosed,
		ST_HullHatchHalfOpen,
		ST_HullHatchOpen,
		ST_HullTop,
		ST_HullTopBottomWindow,
		ST_HullWall,
		ST_HullWallWindow,
		ST_IntCeiling,
		ST_IntDoorClosed,
		ST_IntDoorHalfOpen,
		ST_IntDoorOpen,
		ST_IntFloor,
		ST_IntHatchClosed,
		ST_IntHatchHalfOpen,
		ST_IntHatchOpen,
		ST_IntWallEndCap,
		ST_IntWallFinished,
		ST_IntWallFill,
		ST_IntWallCargo,
		ST_IntWallFillCargo,
		ST_IntWallEquipment,
		ST_IntWallFillEquipment,
		ST_Ladder,
		ST_LadderPole,
		ST_LifeSupport,
		ST_PowerPlant,
		ST_Railing,
		ST_RailingPost,
		ST_Seat,
		ST_Sensor,
		ST_SensorTop,
		ST_Shield,
		ST_Surgery,
		ST_TransporterPad,
		ST_Turret,
		ST_WeaponBay,
		ST_FTLDrive,
		ST_StairCasing,
		ST_StairRiser,
		ST_StairTread,
	};

	AuAnimalia*							m_Animalia;
	AuAnimalia*							m_AnimaliaDel;
	bool								m_Animate;				// True to call update() at the end of each paint. Default=true.
	COMMODITYID							m_Armor;				// Commodity id of visible armor. CID_Invalid for none.
	bool								m_BendArm;
	bool								m_bShowScene;			// Set true to render a spacecraft deck scene around the body.
	COMMODITYID							m_Clothes;				// Commodity id of visible clothing. CID_Invalid for none.
	AuPoint								m_fRotation;			// Set to the 3D rotation of the body.
	AuPoint								m_fTranslation;			// Set to the 3D offset of the body.
	AuHairStyle::STYLE					m_Haircut;
	COMMODITYID							m_Helmet;				// Commodity id of visible helmet. CID_Invalid for none.
	quint8								m_HelmetVisor;
	AuId								m_Id;
	DpBody*								m_pBody;
	DpBody*								m_pBodyDel;
	AuHairStyle::SHAVE					m_Shave;

private:
	AuGLVAO								m_BackdropVAO;
	AuGLVAO								m_ShipDeckVAO;
	AuGLEffects*						m_pRE;
	AuGLModelCache*						m_ModelCache;
	bool								m_bLeftMousePushed;
	bool								m_bRightMousePushed;
	bool								m_bMiddleMousePushed;
	bool								m_bDeleteDeviceContext;
	float								m_fRotAngle;
	GLuint								m_texBackdrop;
	GLuint								m_texSpacecraft;
	QPoint								posClicked;
	QString								m_sFileName;
	QString								m_sPath;

public:
	AuBodyRenderWidget(QWidget *parent);

	~AuBodyRenderWidget();

public:	// Methods
	int AbilityPointsUsed() const;

	void Animate();

	int AvatarCheck() const;

	inline DpBody* Body()
	{
		return m_pBody;
	}

	float CGHeight() const;

	AuPose CurrentGeneralPose() const;

	const DhDNA& DNA3() const;

	const AuDNA& DNA4() const;

	inline int DNAVersion() const
	{
		if(m_Animalia)
			return 4;
		else if(m_pBody)
			return 3;
		else
			return 0;
	}

	float EyeHeight() const;

	float HeadHeight() const;

	bool IsChange(const AuDNA& dna, AuDNABase::CHROMATID chroma, COMMODITYID clothing, COMMODITYID armor, COMMODITYID helmet,
			AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave) const;

	int MaxHitPoints() const;

	void ResetView();

	void SetAnimalia(const AuDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, COMMODITYID clothing, COMMODITYID armor,
			COMMODITYID hat, quint8 visorState, AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave);

	void SetBody(const DhDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, COMMODITYID clothing, COMMODITYID helmet,
			quint8 visorState);

	void SetHandedness(HANDEDNESS hand);

	void SetId(const AuId& id);

	void SetPose(const AuPose& pose, bool bendDominantArm, const AuPose& mouth);

protected:
	AuGLVAO CreateDeckScene();

	// Scavenged from dnspaceship.h.
	inline static double DeckGridSpacing()
	{
		// Deck grid squares are 32 square feet.
		// We allow diagonal lines to the center.
		// This results in 4' as the shortest diagonal wall, half the length
		// of the diagonal of a deck grid square.
		// NOTE: There are two grid coordinate increments per DeckGridSpacing.
		return 5.656854249;	// sqrt(32.0)
	}

	// Returns 1/2 DeckGridSpacing().
	inline static double DeckGridSpacingHalf()
	{
		return 5.656854249/2.0;
	}

	void initializeGL();

	void mouseMoveEvent(QMouseEvent *mouseEvent);

	void mousePressEvent(QMouseEvent *mouseEvent);

	void mouseReleaseEvent(QMouseEvent *mouseEvent);

	void paintGL();

	void RenderDeckScene(const AuMatrix4x4& projection, const AuMatrix4x4& model);

	void resizeGL(int w, int h);

	void ShipTexture(SHIP_TEXTURE tx, float& s, float& t, float& u, float& v);

	void wheelEvent(QWheelEvent *wheelEvent);

public slots:
	virtual void setVisible(bool visible);
	void SetShowScene(bool show);
}; 

