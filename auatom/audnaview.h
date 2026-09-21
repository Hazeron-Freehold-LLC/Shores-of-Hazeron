//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnaview.h,v 1.8 2022/02/19 17:30:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDNAView_H
#define AuDNAView_H

#include "auanimalia.h"
#include "aupose.h"
#include <QOpenGLWidget>

class AuAtomRenderInfo;
class AuGLModelCache;
class AuGLModelMaker;
class AuMatrix4x4;
class QWheelEvent;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAView : public QOpenGLWidget
{
	Q_OBJECT;
private:
	AuPoint								m_AmbientUp;
	QFont								m_Font;
    AuMatrix4x4							m_Model;				// Model matrix.
	AuGLModelCache*						m_ModelCache;
	Qt::MouseButtons					m_MouseButtons;			// Buttons during last mouse press event.
	QPoint								m_MouseLast;
	AuGLModelMaker*						m_MM;					// Model maker.
	AuAtomRenderInfo*					m_RenderInfo;			// Values needed by atoms when rendering them.

	// Things to render.
	AuAnimalia*							m_Female;
	bool								m_FemaleArmBent;
	COMMODITYID							m_FemaleArmor;
	COMMODITYID							m_FemaleClothes;
	AuAnimalia*							m_FemaleDelete;
	AuHairStyle::STYLE					m_FemaleHair;
	COMMODITYID							m_FemaleHat;
	AuPose								m_FemaleMouth;
	AuPose								m_FemalePose;
	AuHairStyle::SHAVE					m_FemaleShave;
	AuAnimalia*							m_Male;
	bool								m_MaleArmBent;
	COMMODITYID							m_MaleArmor;
	COMMODITYID							m_MaleClothes;
	AuAnimalia*							m_MaleDelete;
	AuHairStyle::STYLE					m_MaleHair;
	COMMODITYID							m_MaleHat;
	AuPose								m_MaleMouth;
	AuPose								m_MalePose;
	AuHairStyle::SHAVE					m_MaleShave;
	GLuint								m_ConcreteTex;
	GLuint								m_DirtTex;
	AuDNA								m_DNA;
	GLuint								m_GrassTex;
	GLuint								m_GridTex;
	AuGLVAO								m_Ground;
	QSize								m_MeatImageSize;
	GLuint								m_MeatImageTex;
	AuGLVAO								m_MeatImageVAO;
	int									m_ShowGround;			// 0=no, 1=dirt, 2=grass, 3=concrete, 4=tarmac, 5=grid, 6=Z grid,
																// 7=both grids
	GLuint								m_TarmacTex;

public:	// Constructors
	AuDNAView(QWidget *parent=0);

	~AuDNAView();

public:
	virtual void initializeGL();

	int AbilityPointsUsedFemale();
	int AbilityPointsUsedMale();

	// Returns the result of AuAnimalia::AvatarCheck.
	// Bit 0x8000 indicates the animalia pointer was null; no DNA set?
	int AvatarCheckFemale();
	int AvatarCheckMale();

	float CameraHeightFemale();
	float CameraHeightMale();

	float CgHeightFemale();
	float CgHeightMale();

	float HeadHeightFemale();
	float HeadHeightMale();

	// Returns the max hit points of the bodies.
	int HitPointsFemale();
	int HitPointsMale();

	virtual void paintGL();

	void SetDNAPair(const AuDNA& dna);

	// 'modelchg' 1=alpha, 2=beta, 3=both models
	void SetDNAPair(const AuDNA& dna, int modelchg);

	void SetFemaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes);

	void SetFemaleMouthPose(const AuPose& pose);

	void SetFemaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave);

	void SetFemalePose(const AuPose& pose, bool bendDominantArm);

	void SetMaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes);

	void SetMaleMouthPose(const AuPose& pose);

	void SetMaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave);

	void SetMalePose(const AuPose& pose, bool bendDominantArm);

	void SetupLights(const AuMatrix4x4& model);

public slots:
	void ResetView();

	// 'showWhat' is 0=none, 1=dirt, 2=grass, 3=concrete, 4=tarmac, 5=grid, 6=z grid
	void SetShowGround(int showWhat);

	void ViewRestore();

	void ViewSave();

protected:
	void DeleteViewObjects();

	virtual void mouseMoveEvent(QMouseEvent* e);

	virtual void mousePressEvent(QMouseEvent* e);

	virtual void wheelEvent(QWheelEvent* e);
};

#endif // AuDNAView_H
