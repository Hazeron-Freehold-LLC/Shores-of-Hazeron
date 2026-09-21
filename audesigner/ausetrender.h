//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetrender.h,v 1.8 2022/05/19 15:27:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetRender_H
#define AuSetRender_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;

class AuSetRender : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QSlider*							buildingViewRangeSlider;
	QComboBox*							cbdrawme;
	QCheckBox*							ckbumpmapseas;
	QCheckBox*							ckdrawframedpics;
	QCheckBox*							ckdrawgun;
	QCheckBox*							ckdrawhelmethud;
	QCheckBox*							ckdrawneb;
	QCheckBox*							ckdrawprecip;
	QCheckBox*							ckdrawspecks;
	QCheckBox*							ckdrawsky;
	QSlider*							cloudsSlider;
	QSlider*							framedPictureSizeSlider;
	QSlider*							groundTextureSizeSlider;
	QSlider*							plantTextureSizeSlider;
	QSlider*							mapQualSlider;
	QSlider*							maxLightsSlider;
	QSlider*							detailedTerrainRangeSlider;
	QSlider*							rockViewRangeSlider;
	QSlider*							grassSlider;
	QSlider*							grassViewRangeSlider;
	QSlider*							shrubViewRangeSlider;
	QSlider*							treeViewRangeSlider;
	QLabel*								m_BuildingViewRange;
	QLabel*								m_Clouds;
	QLabel*								m_DetailedTerrainRange;
	QLabel*								m_FoVLabel;
	QSlider*							m_FoVSlider;
	QLabel*								m_FramedPictureSize;
	QLabel*								m_GrassViewRange;
	QLabel*								m_GroundCoverQuantity;
	QLabel*								m_GroundTextureSize;
	QLabel*								m_MaxLightsLabel;
	QLabel*								m_OpenGLInfo;
	QLabel*								m_PlantTextureSize;
	QComboBox*							m_RatingCombo;
	QLabel*								m_RockViewRange;
	QLabel*								m_ShrubViewRange;
	QCheckBox*							m_SkyBoxMipmap;
	QLabel*								m_SkyBoxSizeLabel;
	QSlider*							m_SkyBoxSizeSlider;
	QLabel*								m_TreeViewRange;
	QLabel*								m_WorldMapQuality;

public:
	AuSetRender(bool glContextOk, QWidget *parent=0);
	~AuSetRender();

	void SetOpenGLInfo(int textureUnits, int uniformLocs);

protected slots:
	void FieldOfViewChanged(int value);
	void FramedPictureSizeChanged(int value);
	void GroundTextureSizeChanged(int value);
	void LowMemoryDefault();
	void MaxGraphicsDefault();
	void PlantBranchMaxSegsChanged(int value);
	void PlantTextureSizeChanged(int value);
	void QueryOpenGLVersions();
	void ResetDefault();
	void SetBuildingViewRange(int value);
	void SetBumpMapSeas(int value);
	void SetClouds(int value);
	void SetDetailedTerrainRange(int value);
	void SetDivTexels(int value);
	void SetDrawBody(int value);
	void SetDrawFramedPictures(int value);
	void SetDrawGun(int value);
	void SetDrawHelmetHUDAlways(int value);
	void SetDrawNebulaCloud(int value);
	void SetDrawSpecks(int value);
	void SetDrawSkyBox(int value);
	void SetDrawWeatherEffects(int value);
	void SetGrassViewRange(int value);
	void SetGroundCoverQty(int value);
	void SetMaxLights(int value);
	void SetRating(int value);
	void SetRockViewRange(int value);
	void SetShrubViewRange(int value);
	void SetSkyBoxMipmap(int value);
	void SetSkyBoxSize(int value);
	void SetTreeViewRange(int value);
	void TreeTrunkMinDiaChanged(const QString& value);
	void TreeTrunkFacesChanged(int value);
	void TreeTrunkMaxSegsChanged(int value);
};

#endif // AuSetRender_H
