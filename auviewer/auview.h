//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auview.h,v 1.35 2022/02/19 18:42:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuView_H
#define AuView_H

#include "auanimalia.h"
#include "auatomrenderinfo.h"
#include "auglmodelmaker.h"
#include <QOpenGLWidget>

class Au3dsGLSLModelInstance;
class AuDNA;
class AuItem;
class AuMatrix4x4;
class DhDNA;
class QWheelEvent;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuView : public QOpenGLWidget
{
	Q_OBJECT;
	enum MODE
	{
		None,
		Animalia,
		Body,
		Commodity,
		DNAPair,
		Item,
		MeatProfile,
		ModelFile,
	};

	enum INPUT
	{
		InputNothing,
		InputFrontProfile,
		InputFrontJoints,
		InputSideProfile,
		InputSideJoints,
	};

private:
	AuPoint								m_AmbientUp;
	QFont								m_Font;
    AuMatrix4x4							m_Model;				// Model matrix.
	Qt::MouseButtons					m_MouseButtons;			// Buttons during last mouse press event.
	QPoint								m_MouseLast;
	AuGLModelMaker						m_MM;					// Model maker.
	AuGLModelCache						m_ModelCache;
	AuAtomRenderInfo					m_RenderInfo;			// Values needed by atoms when rendering them.
	MODE								m_ViewMode;

	// Meat profile data
	AuPointArray						m_MeatJointsFront;
	AuPointArray						m_MeatJointsSide;
	AuPointArray						m_MeatProfileFront;
	QString								m_MeatProfileImage;
	INPUT								m_MeatProfileInput;		// Set while entering points for the meat profile.
	AuPointArray						m_MeatProfileSide;

	// Things to render.
	Au3dsGLSLModelInstance*				m_3dsModel;
	COMMODITYID							m_3dsModelCID;
	QString								m_3dsModelFile;
	bool								m_3dsModelState;
	AuAnimalia*							m_AnimalFemale;
	COMMODITYID							m_AnimalFemaleArmor;
	COMMODITYID							m_AnimalFemaleClothes;
	AuAnimalia*							m_AnimalFemaleDelete;
	AuHairStyle::STYLE					m_AnimalFemaleHair;
	COMMODITYID							m_AnimalFemaleHat;
	AuPose								m_AnimalFemaleMouth;
	AuPose								m_AnimalFemalePose;
	AuHairStyle::SHAVE					m_AnimalFemaleShave;
	AuAnimalia*							m_AnimalMale;
	COMMODITYID							m_AnimalMaleArmor;
	COMMODITYID							m_AnimalMaleClothes;
	AuAnimalia*							m_AnimalMaleDelete;
	AuHairStyle::STYLE					m_AnimalMaleHair;
	COMMODITYID							m_AnimalMaleHat;
	AuPose								m_AnimalMaleMouth;
	AuPose								m_AnimalMalePose;
	AuHairStyle::SHAVE					m_AnimalMaleShave;
	DpBody*								m_Body;
	GLuint								m_ConcreteTex;
	GLuint								m_DirtTex;
	AuDNA								m_DNA;
	GLuint								m_GrassTex;
	GLuint								m_GridTex;
	AuGLVAO								m_Ground;
	AuItem*								m_Item;
	QSize								m_MeatImageSize;
	GLuint								m_MeatImageTex;
	AuGLVAO								m_MeatImageVAO;
	AuItem*								m_MeatJointFront;
	AuItem*								m_MeatJointSide;
	AuItem*								m_MeatOriginFront;
	AuItem*								m_MeatOriginSide;
	AuGLVAO								m_MeatProfileFrontVAO;
	bool								m_MeatProfileFrontVAOValid;	// true if the VAO is valid for the meat profile points.
	AuGLVAO								m_MeatProfileSideVAO;
	bool								m_MeatProfileSideVAOValid;	// true if the VAO is valid for the meat profile points.
	int									m_ShowGround;			// 0=no, 1=dirt, 2=grass, 3=concrete, 4=tarmac, 5=grid, 6=Z grid,
																// 7=both grid
	int									m_ShowMode;				// 0=lines, 1=solid, 2=solid and mesh, 3=solid, mesh, and lines
	int									m_ShowMeatImage;
	GLuint								m_TarmacTex;

public:	// Constructors
	AuView(QWidget *parent=0);

	~AuView();

public:
	virtual void initializeGL();

	virtual void paintGL();

	void LoadMeatLine(const QString& fileName);

	void LoadMeatProfile(const QString& fileName);

	AuPoint PointAtPixel(const QPoint& pixel) const;

	void SaveMeatLine(const QString& fileName);

	void SaveMeatProfile(const QString& fileName);

	void SetDNAPair(const AuDNA& dna);

	// 'modelchg' 1=alpha, 2=beta, 3=both models
	void SetDNAPair(const AuDNA& dna, int modelchg);

	void SetFemaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes);

	void SetFemaleMouthPose(const AuPose& mouth);

	void SetFemaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave);

	void SetFemalePose(const AuPose& pose);

	void SetMaleClothing(COMMODITYID hat, COMMODITYID armor, COMMODITYID clothes);

	void SetMaleMouthPose(const AuPose& mouth);

	void SetMaleHair(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave);

	void SetMalePose(const AuPose& pose);

	void SetupLights(const AuMatrix4x4& model);

	void ViewAnimal(const AuDNA& dna, GENDER gender, HANDEDNESS handedness);

	void ViewBody(int gender, bool isLefty, const DhDNA& dna);

	void ViewItem(COMMODITYID cid);

	void ViewModel(const QString& fileName);

public slots:
	void BodyRun();

	void BodyStand();

	void BodyWalk();

	void ConcreteToggle();

	void DirtToggle();

	void GrassToggle();

	void ItemToggleState();

	void MeatImageToggle();

	void MeatProfileBackspace();

	void MeatProfileChooseImage();

	void MeatProfileClear();

	void MeatProfileEnd();

	void MeatProfileInputFrontJoints();

	void MeatProfileInputFrontProfile();

	void MeatProfileInputSideJoints();

	void MeatProfileInputSideProfile();

	void MeatProfileScale();

	void MeatProfileStopInput();

	void ModelToggleState();

	void NewMeatProfile();

	void ResetView();

	// 'showWhat' is 0=none, 1=dirt, 2=grass, 3=concrete, 4=tarmac, 5=grid, 6=z grid
	void SetShowGround(int showWhat);

	void TarmacToggle();

	void ViewLines();

	void ViewLinesAndSolid();

	void ViewRestore();

	void ViewSave();

	void ViewSolid();

	void ViewSolidMeshAndIntersections();

protected:
	void DeleteViewObjects();

	virtual void mouseDoubleClickEvent(QMouseEvent* e);

	virtual void mouseMoveEvent(QMouseEvent* e);

	virtual void mousePressEvent(QMouseEvent* e);

	virtual void wheelEvent(QWheelEvent* e);
};

#endif // AuView_H
