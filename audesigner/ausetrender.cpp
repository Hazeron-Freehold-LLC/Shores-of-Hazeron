//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetrender.cpp,v 1.23 2022/05/19 16:33:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetrender.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aujoystick.h"
#include "auframe.h"
#include <daperson.h>
#include <davehicle.h>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

AuSetRender::AuSetRender(bool glContextOk, QWidget *parent) : QWidget(parent)
{
	QPushButton* queryVers = new QPushButton(tr("OpenGL Versions"));
	if(glContextOk)
		queryVers->setToolTip(tr("<html><b>Query OpenGL Versions</b><br>"
					"Attempts to determine what OpenGL versions are supported by the video device, if any. "
					"<b>This program requires %1.</b>"
					"</html>").arg(GLFuncName));
	else
		queryVers->setToolTip(tr("<html><b>Query OpenGL Versions</b><br>"
					"Attempts to determine what OpenGL versions are supported by the video device, if any. "
					"<b>This program requires %1.</b><br>"
					"This button is disabled because OpenGL has not been initialized. "
					"To use this button, display this window after entering the game or designer. "
					"</html>").arg(GLFuncName));
	connect(queryVers, SIGNAL(clicked()), SLOT(QueryOpenGLVersions()));
	queryVers->setEnabled(glContextOk);

	m_OpenGLInfo = new QLabel(tr("Fragment shaders require 16 texture units."));
	m_OpenGLInfo->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QPushButton* resetDefault = new QPushButton(tr("Default"));
	resetDefault->setToolTip(tr("<html><b>Default Configuration</b><br>"
				"Restores render slide controls to default values. "
				"</html>"));
	connect(resetDefault, SIGNAL(clicked()), SLOT(ResetDefault()));

	QPushButton* lowMemoryDefault = new QPushButton(tr("Low Memory"));
	lowMemoryDefault->setToolTip(tr("<html><b>Low Memory Configuration</b><br>"
				"Configures render slide controls to values that may be better for computers with low memory. "
				"</html>"));
	connect(lowMemoryDefault, SIGNAL(clicked()), SLOT(LowMemoryDefault()));

	QPushButton* maxGraphicsDefault = new QPushButton(tr("Max Graphics"));
	maxGraphicsDefault->setToolTip(tr("<html><b>Max Graphics Configuration</b><br>"
				"Configures render slide controls to values that are very demanding on graphics and memory. "
				"</html>"));
	connect(maxGraphicsDefault, SIGNAL(clicked()), SLOT(MaxGraphicsDefault()));

	cbdrawme = new QComboBox(this);
	cbdrawme->addItem(tr("Draw Full Body"), 1);
	cbdrawme->addItem(tr("Draw Headless Body"), 2);
	cbdrawme->addItem(tr("Draw No Body"), 0);
	cbdrawme->setToolTip(tr("<html>Choose how the 3D model of your body is drawn in your scene. "
				"With some DNA, it may be helpful to turn off some or all of your body, to remove parts that block your view. "
				"<br>"
				"This setting does not affect the way your body is drawn in other players' scenes. "
				"</html>"));
	int cur = cbdrawme->findData(g_Au.DrawBodyModel());
	cbdrawme->setCurrentIndex(cur);
	connect(cbdrawme, SIGNAL(currentIndexChanged(int)), SLOT(SetDrawBody(int)));

	ckdrawgun = new QCheckBox(tr("Draw Current Item Model"));
	ckdrawgun->setCheckState(g_Au.DrawGun()?Qt::Checked:Qt::Unchecked);
	ckdrawgun->setToolTip(tr("<html>When checked, the 3D model of your current item is drawn in the scene.<br>"
				"When unchecked, the 3D model of your current item is not drawn in the scene.</html>"));
	connect(ckdrawgun, SIGNAL(stateChanged(int)), SLOT(SetDrawGun(int)));

	ckdrawframedpics = new QCheckBox(tr("Draw Framed Pictures"));
	ckdrawframedpics->setCheckState(g_Au.DrawFramedPictures()?Qt::Checked:Qt::Unchecked);
	ckdrawframedpics->setToolTip(tr("<html>When checked, picture frame contents are downloaded from the Internet and shown "
				"in the frame.<br>"
				"When unchecked, picture frame contents are not download from the Internet; "
				"an internal image selected randomly is shown in the frame.<br>"
				"<b>Changes to this setting only affect pictures that have not yet appeared in your scene during this "
				"session.</b></html>"));
	connect(ckdrawframedpics, SIGNAL(stateChanged(int)), SLOT(SetDrawFramedPictures(int)));

	ckdrawsky = new QCheckBox(tr("Draw Sky Box (Galaxy and Nebula)"));
	ckdrawsky->setCheckState(g_Au.DrawSkyBox()?Qt::Checked:Qt::Unchecked);
	ckdrawsky->setToolTip(tr("<html>"
				"The sky box is generated based on your current sector's location in the universe. "
				"It is regenerated when you move from one sector to another. "
				"<br>"
				"When checked, the sky box of the galaxy is drawn in the scene. "
				"<br>"
				"When unchecked, the sky box of the galaxy is not drawn in the scene.</html>"));
	connect(ckdrawsky, SIGNAL(stateChanged(int)), SLOT(SetDrawSkyBox(int)));

	ckdrawneb = new QCheckBox(tr("Draw Nebula Cloud"));
	ckdrawneb->setCheckState(g_Au.DrawNebulaCloud()?Qt::Checked:Qt::Unchecked);
	ckdrawneb->setToolTip(tr("<html>When checked, a cloudy effect is painted in the scene when you are in a nebula. "
				"The color and density of the nebula cloud is based on your position in the galaxy relative to the nebula.<br>"
				"When unchecked, the nebula cloud is not painted in the scene.</html>"));
	connect(ckdrawneb, SIGNAL(stateChanged(int)), SLOT(SetDrawNebulaCloud(int)));

	ckdrawspecks = new QCheckBox(tr("Draw Small Rocks in Space"));
	ckdrawspecks->setCheckState(g_Au.DrawSpecks()?Qt::Checked:Qt::Unchecked);
	ckdrawspecks->setToolTip(tr("<html>When checked, small rocks are painted in the scene when you are in space. "
				"Small rocks help to provide a sense of motion in otherwise empty space.<br>"
				"When unchecked, the small rocks are not painted in the scene.</html>"));
	connect(ckdrawspecks, SIGNAL(stateChanged(int)), SLOT(SetDrawSpecks(int)));

	ckdrawprecip = new QCheckBox(tr("Draw Rain/Snow Effects"));
	ckdrawprecip->setCheckState(g_Au.DrawWeatherEffects()?Qt::Checked:Qt::Unchecked);
	ckdrawprecip->setToolTip(tr("<html>When checked, the rain/snow effects are drawn in the scene.<br>"
		"When unchecked, the rain/snow effects are not drawn in the scene.</html>"));
	connect(ckdrawprecip, SIGNAL(stateChanged(int)), SLOT(SetDrawWeatherEffects(int)));

	ckdrawhelmethud = new QCheckBox(tr("Draw Helmet HUD When Screen Elements Are Hidden"));
	ckdrawhelmethud->setCheckState(g_Au.DrawHelmetHUDAlways()?Qt::Checked:Qt::Unchecked);
	ckdrawhelmethud->setToolTip(tr("<html>When checked, the helmet HUD is drawn even if screen elements are hidden.<br>"
		"When unchecked, the helmet HUD is not drawn when scene elements are hidden.</html>"));
	connect(ckdrawhelmethud, SIGNAL(stateChanged(int)), SLOT(SetDrawHelmetHUDAlways(int)));

	m_SkyBoxMipmap = new QCheckBox(tr("Mipmap Sky Box Texture"));
	m_SkyBoxMipmap->setCheckState(g_Au.SkyBoxMipMap()?Qt::Checked:Qt::Unchecked);
	m_SkyBoxMipmap->setToolTip(tr("<html>Mipmapping helps eliminate sparkling when a texture image gets reduced in size. "
				"<br>"
				"%1 If the sky box texture size is too high, mipmapping will blend stars away to faint grey dots or nothing at all."
				"<br>"
				"Mipmapping increases the amount of texture memory used by the sky box. "
				"<b>You must move to a different sector or the program must be restarted for this change to take effect.</b>"
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_SkyBoxMipmap, SIGNAL(stateChanged(int)), SLOT(SetSkyBoxMipmap(int)));

	ckbumpmapseas = new QCheckBox(tr("Bump Map Sea Surfaces"));
	ckbumpmapseas->setCheckState(g_Au.BumpMapSeas()?Qt::Checked:Qt::Unchecked);
	ckbumpmapseas->setToolTip(tr("<html><p>Bump mapping makes shiny highlights of suns where they reflect off the sea. "
				"Bump mapping is applied to seas of water, lava and supercooled liquid when the viewer is within atmosphere range "
				"of a globe or ringworld.</p>"
				"<p>When checked, sea surfaces are bump mapped.</p>"
				"<p>When unchecked, sea surfaces are not bump mapped. "
				"Turn off bump mapping to reduce shader load and texture memory usage.</p>"
				"</html>"));
	connect(ckbumpmapseas, SIGNAL(stateChanged(int)), SLOT(SetBumpMapSeas(int)));

	// Max Lights
	QLabel* maxLightsLabel = new QLabel(tr("Light Sources"));

	m_MaxLightsLabel = new QLabel(QString("%1 Lights").arg(g_Au.MaxLights()));

	maxLightsSlider = new QSlider(Qt::Horizontal, this);
	maxLightsSlider->setRange(3, MAX_POINT_LIGHTS);
	maxLightsSlider->setSingleStep(1);
	maxLightsSlider->setPageStep(1);
	maxLightsSlider->setValue(g_Au.MaxLights());
	maxLightsSlider->setToolTip(tr("<html><b>Light Sources</b><br>"
				"Light sources illuminate objects in the scene. "
				"Dynamic lighting calculations consider brightness and distance, among many factors. "
				"A large number of light sources could significantly reduce frame rate, depending on the GPU. "
				"<br>"
				"%1 This setting does not affect light from suns. "
				"<br>"
				"%1 This setting does not affect things that glow. "
				"<br>"
				"<b>The program must be restarted for this change to take effect.</b>"
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(maxLightsSlider, SIGNAL(valueChanged(int)), SLOT(SetMaxLights(int)));

	// Sky box size
	QLabel* skyboxsizelabel = new QLabel(tr("Sky Box Texture Size"));

	m_SkyBoxSizeLabel = new QLabel(tr("%1 pixels").arg(g_Au.SkyBoxPaneSize()));

	m_SkyBoxSizeSlider = new QSlider(Qt::Horizontal, this);
	m_SkyBoxSizeSlider->setRange(256, 4096);
	m_SkyBoxSizeSlider->setSingleStep(1);
	m_SkyBoxSizeSlider->setPageStep(1);
	m_SkyBoxSizeSlider->setValue(g_Au.SkyBoxPaneSize());
	m_SkyBoxSizeSlider->setToolTip(tr("<html><b>Sky Box Texture Size</b><br>"
				"The sky box texture is generated based on your current sector's location in the universe. "
				"It is regenerated when you move from one sector to another. "
				"<br>"
				"%1 1/2 the width of the view window is recommended, for best overall quality. "
				"<br>"
				"%1 As sky box size increases, background stars become tinier and fainter; "
				"the milky way becomes less dense and bright. "
				"<br>"
				"%1 If the texture is too large, stars will appear as tiny sparkles or they will not be visible at all. "
				"<br>"
				"%1 At high resolution, <i>mipmapping</i> may blur the stars away completely. "
				"<br>"
				"%1 The sky box texture is generated regardless of the <i>Draw Sky Box</i> setting. "
				"<br>"
				"Reduce sky box texture size to lower the amount of texture memory used by the sky box. "
				"<b>You must move to a different sector or the program must be restarted for this change to take effect.</b>"
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_SkyBoxSizeSlider, SIGNAL(valueChanged(int)), SLOT(SetSkyBoxSize(int)));

	// Field of View
	QLabel* fovlabel = new QLabel(tr("Field of View Angle"));

	int fovAngle = (int)round(g_Au.FieldOfViewY() * RAD2DEG);
	m_FoVLabel = new QLabel(tr("%1%2").arg(fovAngle).arg(g_Au.DegreeSymbol()));

	m_FoVSlider = new QSlider(Qt::Horizontal, this);
	m_FoVSlider->setRange(40, 90);
	m_FoVSlider->setSingleStep(1);
	m_FoVSlider->setPageStep(1);
	m_FoVSlider->setValue(fovAngle);
	m_FoVSlider->setToolTip(tr("<html><b>Field of View Angle</b><br>"
				"Adjusts the vertical field of view angle of the eye camera. "
				"Horizontal angle is determined by view aspect ratio. "
				"</html>"));
	connect(m_FoVSlider, SIGNAL(valueChanged(int)), SLOT(FieldOfViewChanged(int)));

	// Clouds
	QLabel* cloudsLabel = new QLabel(tr("Atmospheric Clouds"));

	if(g_Au.DrawClouds() == 0)
		m_Clouds = new QLabel(tr("None"));
	else if(g_Au.DrawClouds() == 1)
		m_Clouds = new QLabel(tr("One Layer"));
	else if(g_Au.DrawClouds() == 2)
		m_Clouds = new QLabel(tr("Two Layers"));
	else
		m_Clouds = new QLabel(tr("Many Layers"));

	cloudsSlider = new QSlider(Qt::Horizontal, this);
	cloudsSlider->setRange(0, 3);
	cloudsSlider->setSingleStep(1);
	cloudsSlider->setPageStep(1);
	cloudsSlider->setValue(g_Au.DrawClouds());
	cloudsSlider->setToolTip(tr("<html><b>Atmospheric Clouds</b><br>"
				"Adjusts the quality of the clouds that appear in the atmospheres of worlds.<br>"
				"Gas giants are not affected by this setting."
				"</html>"));
	connect(cloudsSlider, SIGNAL(valueChanged(int)), SLOT(SetClouds(int)));

	// Framed picture size.
	QLabel* framedPictureSizeLabel = new QLabel("Framed Picture Size");
	//framedPictureSizeLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_FramedPictureSize = new QLabel(QString("%1 pixels").arg(g_Au.FramedPictureSize()));

	framedPictureSizeSlider = new QSlider(Qt::Horizontal, this);
	framedPictureSizeSlider->setRange(64, 2048);
	framedPictureSizeSlider->setSingleStep(1);
	framedPictureSizeSlider->setPageStep(64);
	framedPictureSizeSlider->setValue(g_Au.FramedPictureSize());
	framedPictureSizeSlider->setToolTip(tr("<html><b>Framed Picture Size</b><br>"
				"Restricts the largest size of texture image created for each picture frame.<br>"
				"<b>The program must be restarted for this change to take effect.</b></html>"));
	connect(framedPictureSizeSlider, SIGNAL(valueChanged(int)), SLOT(FramedPictureSizeChanged(int)));

	// Ground texture size.
	QLabel* groundTextureSizeLabel = new QLabel("Ground Texture Size");
	//groundTextureSizeLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_GroundTextureSize = new QLabel(QString("%1 pixels").arg(g_Au.GroundTextureSize()));

	groundTextureSizeSlider = new QSlider(Qt::Horizontal, this);
	groundTextureSizeSlider->setRange(64, 256);
	groundTextureSizeSlider->setSingleStep(1);
	groundTextureSizeSlider->setPageStep(16);
	groundTextureSizeSlider->setValue(g_Au.GroundTextureSize());
	groundTextureSizeSlider->setToolTip(tr("<html><b>Ground Texture Size</b><br>"
				"Adjusts the size of texture image created for each soil type. "
				"At the high end, tiny individual pebbles can be seen. "
				"At the low end, the ground appears as diffuse blobs of color. "
				"<b>The program must be restarted for this change to take effect.</b></html>"));
	connect(groundTextureSizeSlider, SIGNAL(valueChanged(int)), SLOT(GroundTextureSizeChanged(int)));

	// Plant texture size.
	QLabel* plantTextureSizeLabel = new QLabel("Plant Texture Size");
	//plantTextureSizeLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_PlantTextureSize = new QLabel(QString("%1 pixels").arg(g_Au.PlantTextureSize()));

	plantTextureSizeSlider = new QSlider(Qt::Horizontal, this);
	plantTextureSizeSlider->setRange(64, 512);
	plantTextureSizeSlider->setSingleStep(1);
	plantTextureSizeSlider->setPageStep(16);
	plantTextureSizeSlider->setValue(g_Au.PlantTextureSize());
	plantTextureSizeSlider->setToolTip(tr("<html><b>Plant Texture Size</b><br>"
				"Adjusts the size of texture image created for each plant specie. "
				"At the high end, tiny individual flowers can be seen. "
				"At the low end, most flowers appear as diffuse blobs of color. "
				"<b>The program must be restarted for this change to take effect.</b></html>"));
	connect(plantTextureSizeSlider, SIGNAL(valueChanged(int)), SLOT(PlantTextureSizeChanged(int)));

	QLabel* mapQualLabel = new QLabel(tr("World Map Quality"));

	if(g_Au.DivTexels() == 1)
		m_WorldMapQuality = new QLabel(tr("Low"));
	else if(g_Au.DivTexels() == 2)
		m_WorldMapQuality = new QLabel(tr("Medium"));
	else
		m_WorldMapQuality = new QLabel(tr("High"));

	mapQualSlider = new QSlider(Qt::Horizontal, this);
	mapQualSlider->setRange(1, 3);
	mapQualSlider->setSingleStep(1);
	mapQualSlider->setPageStep(1);
	mapQualSlider->setValue(g_Au.DivTexels());
	mapQualSlider->setToolTip(tr("<html><b>World Map Quality</b><br>"
				"Adjusts the quality of the world map created for planets, moons and ringworlds. "
				"World maps appear on the Locator window and on transporter stations. "
				"Gas giants are not affected by this setting.<br>"
				"This setting also affects the texture resolution on distant worlds.<br>"
				"<b>The program must be restarted for this change to take effect.</b></html>"));
	connect(mapQualSlider, SIGNAL(valueChanged(int)), SLOT(SetDivTexels(int)));

	QLabel* detailedTerrainRangeLabel = new QLabel(tr("Detailed Terrain Range"));

	m_DetailedTerrainRange = new QLabel(QString("%1m").arg(g_Au.DetailedTerrainRange()));

	detailedTerrainRangeSlider = new QSlider(Qt::Horizontal, this);
	detailedTerrainRangeSlider->setRange(200, 2000);	// meters
	detailedTerrainRangeSlider->setSingleStep(1);
	detailedTerrainRangeSlider->setPageStep(100);
	detailedTerrainRangeSlider->setValue(g_Au.DetailedTerrainRange());
	detailedTerrainRangeSlider->setToolTip(tr("<html><b>Detailed Terrain Range</b><br>"
				"Adjusts the distance to which detailed terrain is tessellated. "
				"Within this range the terrain conforms closely to the surface of the world at the expense of CPU time to "
				"generate the surface model. "
				"Outside this range the terrain is less tessellated, conforming only roughly to the surface of the world. "
				"This does not restrict the range at which plants and buildings appear."
				"</html>"));
	connect(detailedTerrainRangeSlider, SIGNAL(valueChanged(int)), SLOT(SetDetailedTerrainRange(int)));

	QLabel* rockViewRangeLabel = new QLabel(tr("Rock View Range"));

	m_RockViewRange = new QLabel(QString("%1m").arg(g_Au.RockViewRange()));

	rockViewRangeSlider = new QSlider(Qt::Horizontal, this);
	rockViewRangeSlider->setRange(10, 500);	// meters
	rockViewRangeSlider->setSingleStep(1);
	rockViewRangeSlider->setPageStep(10);
	rockViewRangeSlider->setValue(g_Au.RockViewRange());
	rockViewRangeSlider->setToolTip(tr("<html><b>Rock View Range</b><br>"
				"Adjusts the distance at which rocks are visible."
				"</html>"));
	connect(rockViewRangeSlider, SIGNAL(valueChanged(int)), SLOT(SetRockViewRange(int)));

	QLabel* grassLabel = new QLabel(tr("Grass Quantity"));

	m_GroundCoverQuantity = new QLabel(QString("%1%").arg(g_Au.GroundCoverQty()/10));

	grassSlider = new QSlider(Qt::Horizontal, this);
	grassSlider->setRange(0, 1000);
	grassSlider->setSingleStep(1);
	grassSlider->setPageStep(10);
	grassSlider->setValue(g_Au.GroundCoverQty());
	grassSlider->setToolTip(tr("<html><b>Grass Quantity</b><br>"
				"Adjusts the number of grass plants on the terrain.<br>"
				"%1 Large numbers of plants create a noticeable delay when a terrain model is added to the OpenGL scene. "
				"This includes terrain models restored from the model cache.<br>"
				"%1 Large number of plants create a delay when the terrain model changes during city construction "
				"and when terrain appears in the scene for the first time.<br>"
				"%1 <b>The maximum quantity setting for grass is extremely high.</b> "
				"Most computers will achieve a low frame rate with the grass quantity set to the maximum. "
				"This can exceed internal rendering limits on some video interfaces.<br>"
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(grassSlider, SIGNAL(valueChanged(int)), SLOT(SetGroundCoverQty(int)));

	QLabel* grassViewRangeLabel = new QLabel(tr("Grass View Range"));

	m_GrassViewRange = new QLabel(QString("%1m").arg(g_Au.GrassViewRange()));

	grassViewRangeSlider = new QSlider(Qt::Horizontal, this);
	grassViewRangeSlider->setRange(10, 200);	// meters
	grassViewRangeSlider->setSingleStep(1);
	grassViewRangeSlider->setPageStep(10);
	grassViewRangeSlider->setValue(g_Au.GrassViewRange());
	grassViewRangeSlider->setToolTip(tr("<html><b>Grass View Range</b><br>"
				"Adjusts the distance at which grass is visible."
				"</html>"));
	connect(grassViewRangeSlider, SIGNAL(valueChanged(int)), SLOT(SetGrassViewRange(int)));

	QLabel* shrubViewRangeLabel = new QLabel(tr("Shrub View Range"));

	m_ShrubViewRange = new QLabel(QString("%1m").arg(g_Au.ShrubViewRange()));

	shrubViewRangeSlider = new QSlider(Qt::Horizontal, this);
	shrubViewRangeSlider->setRange(50, 500);	// meters
	shrubViewRangeSlider->setSingleStep(1);
	shrubViewRangeSlider->setPageStep(10);
	shrubViewRangeSlider->setValue(g_Au.ShrubViewRange());
	shrubViewRangeSlider->setToolTip(tr("<html><b>Shrub View Range</b><br>"
				"Adjusts the distance at which shrubbery is visible."
				"</html>"));
	connect(shrubViewRangeSlider, SIGNAL(valueChanged(int)), SLOT(SetShrubViewRange(int)));

	QLabel* treeViewRangeLabel = new QLabel(tr("Tree View Range"));

	m_TreeViewRange = new QLabel(QString("%1m").arg(g_Au.TreeViewRange()));

	treeViewRangeSlider = new QSlider(Qt::Horizontal, this);
	treeViewRangeSlider->setRange(100, 1000);	// meters
	treeViewRangeSlider->setSingleStep(1);
	treeViewRangeSlider->setPageStep(10);
	treeViewRangeSlider->setValue(g_Au.TreeViewRange());
	treeViewRangeSlider->setToolTip(tr("<html><b>Tree View Range</b><br>"
				"Adjusts the distance at which trees are visible."
				"</html>"));
	connect(treeViewRangeSlider, SIGNAL(valueChanged(int)), SLOT(SetTreeViewRange(int)));

	QLabel* buildingViewRangeLabel = new QLabel(tr("Building View Range"));

	m_BuildingViewRange = new QLabel(QString("%1m").arg(g_Au.BuildingViewRange()));

	buildingViewRangeSlider = new QSlider(Qt::Horizontal, this);
	buildingViewRangeSlider->setRange(200, 20000);	// meters
	buildingViewRangeSlider->setSingleStep(1);
	buildingViewRangeSlider->setPageStep(100);
	buildingViewRangeSlider->setValue(g_Au.BuildingViewRange());
	buildingViewRangeSlider->setToolTip(tr("<html><b>Building View Range</b><br>"
				"Adjusts the distance at which buildings are visible."
				"</html>"));
	connect(buildingViewRangeSlider, SIGNAL(valueChanged(int)), SLOT(SetBuildingViewRange(int)));

	if(!g_Au.IsSceneSolo())
	{
		m_RatingCombo = new QComboBox(this);
		m_RatingCombo->addItem(AuGlobal::RatingDescription(AuGlobal::Everyone), AuGlobal::Everyone);
		m_RatingCombo->addItem(AuGlobal::RatingDescription(AuGlobal::Teen), AuGlobal::Teen);
		m_RatingCombo->addItem(AuGlobal::RatingDescription(AuGlobal::Mature), AuGlobal::Mature);
		m_RatingCombo->addItem(AuGlobal::RatingDescription(AuGlobal::Adult), AuGlobal::Adult);
		m_RatingCombo->setToolTip(tr("<html>Choose a play rating. "
					"This affects your play experience; it does not affect what other players see or do. "
					"Nudity and mating only apply to sentient beings. "
					"Wild animals are always nude and they do not mate. "
					"</html>"));
		cur = m_RatingCombo->findData(g_Au.Rating());
		m_RatingCombo->setCurrentIndex(cur);
		connect(m_RatingCombo, SIGNAL(currentIndexChanged(int)), SLOT(SetRating(int)));
	}
	else
		m_RatingCombo = 0;

	/*
	// Tree/shrub generation.
	QGroupBox* plantGroup = new QGroupBox(tr("Plant Model Generator"));
	{
		// Max plant branch segments.
		QLabel* plantBranchMaxSegsLabel = new QLabel("Texture Branch Segment Limit");
		plantBranchMaxSegsLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		QSpinBox* plantBranchMaxSegsSpin = new QSpinBox;
		plantBranchMaxSegsSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		plantBranchMaxSegsSpin->setMinimum(1);
		plantBranchMaxSegsSpin->setMaximum(20);
		plantBranchMaxSegsSpin->setValue(g_Au.PlantBranchMaxSegs());
		connect(plantBranchMaxSegsSpin, SIGNAL(valueChanged(int)), SLOT(PlantBranchMaxSegsChanged(int)));

		// Tree trunk min diameter
		QLabel* treeTrunkMinDiaLabel = new QLabel("Solid Branch Smallest Diameter");
		treeTrunkMinDiaLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		QLineEdit* treeTrunkMinDiaEdit = new QLineEdit;
		treeTrunkMinDiaEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		treeTrunkMinDiaEdit->setText(QVariant(g_Au.TreeTrunkMinDia()).toString());
		connect(treeTrunkMinDiaEdit, SIGNAL(textChanged(const QString&)), SLOT(TreeTrunkMinDiaChanged(const QString&)));

		// Tree trunk faces
		QLabel* treeTrunkFacesLabel = new QLabel("Solid Branch Faces");
		treeTrunkFacesLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		QSpinBox* treeTrunkFacesSpin = new QSpinBox;
		treeTrunkFacesSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		treeTrunkFacesSpin->setMinimum(3);
		treeTrunkFacesSpin->setMaximum(32);
		treeTrunkFacesSpin->setValue(g_Au.TreeTrunkFaces());
		connect(treeTrunkFacesSpin, SIGNAL(valueChanged(int)), SLOT(TreeTrunkFacesChanged(int)));

		// Tree trunk max segments
		QLabel* treeTrunkMaxSegsLabel = new QLabel("Solid Branch Segment Limit");
		treeTrunkMaxSegsLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		QSpinBox* treeTrunkMaxSegsSpin = new QSpinBox;
		treeTrunkMaxSegsSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		treeTrunkMaxSegsSpin->setMinimum(1);
		treeTrunkMaxSegsSpin->setMaximum(20);
		treeTrunkMaxSegsSpin->setValue(g_Au.TreeTrunkMaxSegs());
		connect(treeTrunkMaxSegsSpin, SIGNAL(valueChanged(int)), SLOT(TreeTrunkMaxSegsChanged(int)));

		QGridLayout* layplant = new QGridLayout;
		int row=0;
		layplant->addWidget(treeTrunkFacesLabel, row,0);
		layplant->addWidget(treeTrunkFacesSpin, row,1);
		row++;
		layplant->addWidget(treeTrunkMaxSegsLabel, row,0);
		layplant->addWidget(treeTrunkMaxSegsSpin, row,1);
		row++;
		layplant->addWidget(treeTrunkMinDiaLabel, row,0);
		layplant->addWidget(treeTrunkMinDiaEdit, row,1);
		row++;
		layplant->addWidget(plantBranchMaxSegsLabel, row,0);
		layplant->addWidget(plantBranchMaxSegsSpin, row,1);
		row++;
		layplant->addWidget(plantTextureSizeLabel, row,0);
		layplant->addWidget(plantTextureSizeSlider, row,1);
		row++;
		plantGroup->setLayout(layplant);
	}
	*/

	QGridLayout* sliderlay = new QGridLayout;
	{
		int row=0;
		sliderlay->addWidget(skyboxsizelabel, row,0);
		sliderlay->addWidget(m_SkyBoxSizeSlider, row,1);
		sliderlay->addWidget(m_SkyBoxSizeLabel, row,2);
		row++;
		sliderlay->addWidget(maxLightsLabel, row,0);
		sliderlay->addWidget(maxLightsSlider, row,1);
		sliderlay->addWidget(m_MaxLightsLabel, row,2);
		row++;
		sliderlay->addWidget(cloudsLabel, row,0);
		sliderlay->addWidget(cloudsSlider, row,1);
		sliderlay->addWidget(m_Clouds, row,2);
		row++;
		sliderlay->addWidget(framedPictureSizeLabel, row,0);
		sliderlay->addWidget(framedPictureSizeSlider, row,1);
		sliderlay->addWidget(m_FramedPictureSize, row,2);
		row++;
		sliderlay->addWidget(groundTextureSizeLabel, row,0);
		sliderlay->addWidget(groundTextureSizeSlider, row,1);
		sliderlay->addWidget(m_GroundTextureSize, row,2);
		row++;
		sliderlay->addWidget(plantTextureSizeLabel, row,0);
		sliderlay->addWidget(plantTextureSizeSlider, row,1);
		sliderlay->addWidget(m_PlantTextureSize, row,2);
		row++;
		sliderlay->addWidget(mapQualLabel, row,0);
		sliderlay->addWidget(mapQualSlider, row,1);
		sliderlay->addWidget(m_WorldMapQuality, row,2);
		row++;
		sliderlay->addWidget(detailedTerrainRangeLabel, row,0);
		sliderlay->addWidget(detailedTerrainRangeSlider, row,1);
		sliderlay->addWidget(m_DetailedTerrainRange, row,2);
		row++;
		sliderlay->addWidget(rockViewRangeLabel, row,0);
		sliderlay->addWidget(rockViewRangeSlider, row,1);
		sliderlay->addWidget(m_RockViewRange, row,2);
		row++;
		sliderlay->addWidget(grassLabel, row,0);
		sliderlay->addWidget(grassSlider, row,1);
		sliderlay->addWidget(m_GroundCoverQuantity, row,2);
		row++;
		sliderlay->addWidget(grassViewRangeLabel, row,0);
		sliderlay->addWidget(grassViewRangeSlider, row,1);
		sliderlay->addWidget(m_GrassViewRange, row,2);
		row++;
		sliderlay->addWidget(shrubViewRangeLabel, row,0);
		sliderlay->addWidget(shrubViewRangeSlider, row,1);
		sliderlay->addWidget(m_ShrubViewRange, row,2);
		row++;
		sliderlay->addWidget(treeViewRangeLabel, row,0);
		sliderlay->addWidget(treeViewRangeSlider, row,1);
		sliderlay->addWidget(m_TreeViewRange, row,2);
		row++;
		sliderlay->addWidget(buildingViewRangeLabel, row,0);
		sliderlay->addWidget(buildingViewRangeSlider, row,1);
		sliderlay->addWidget(m_BuildingViewRange, row,2);
		row++;
		sliderlay->addWidget(fovlabel, row,0);
		sliderlay->addWidget(m_FoVSlider, row,1);
		sliderlay->addWidget(m_FoVLabel, row,2);
		row++;

		QHBoxLayout* deflay = new QHBoxLayout;
		deflay->addWidget(lowMemoryDefault);
		deflay->addWidget(resetDefault);
		deflay->addWidget(maxGraphicsDefault);

		sliderlay->addLayout(deflay, row,1, 1,2);
	}

	QVBoxLayout* cklay = new QVBoxLayout;
	if(m_RatingCombo)
		cklay->addWidget(m_RatingCombo);
	cklay->addWidget(cbdrawme);
	cklay->addWidget(ckdrawgun);
	cklay->addWidget(ckdrawframedpics);
	cklay->addWidget(ckdrawsky);
	cklay->addWidget(ckdrawneb);
	cklay->addWidget(ckdrawspecks);
	cklay->addWidget(ckdrawprecip);
	cklay->addWidget(ckdrawhelmethud);

	QVBoxLayout* infolay = new QVBoxLayout;
	if(queryVers)
		infolay->addWidget(queryVers);
	infolay->addWidget(m_OpenGLInfo);
	infolay->addWidget(m_SkyBoxMipmap);
	infolay->addWidget(ckbumpmapseas);
	infolay->addStretch();

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(cklay, row,0, 1,1);
	lay->addLayout(infolay, row,1, 1,1);
	row++;
	lay->addLayout(sliderlay, row,0, 1,2);
	row++;
	/*
	lay->setRowStretch(row, 1);
	row++;
	lay->addWidget(plantGroup, row,0, 1,2);
	row++;
	lay->setRowStretch(row, 1);
	row++;
	*/
	lay->setRowStretch(row, 4);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetRender::~AuSetRender()
{
}

void AuSetRender::SetDrawBody(int index)
{
	int drawBody = cbdrawme->itemData(index).toInt();
	g_Au.SetDrawBodyModel(drawBody);
}

void AuSetRender::SetDrawFramedPictures(int value)
{
	g_Au.SetDrawFramedPictures(value ? true : false);
}

void AuSetRender::SetDrawGun(int value)
{
	g_Au.SetDrawGun(value ? true : false);
}

void AuSetRender::SetDrawNebulaCloud(int value)
{
	g_Au.SetDrawNebulaCloud(value ? true : false);
}

void AuSetRender::SetDrawSpecks(int value)
{
	g_Au.SetDrawSpecks(value ? true : false);
}

void AuSetRender::SetDrawSkyBox(int value)
{
	g_Au.SetDrawSkyBox(value ? true : false);
}

void AuSetRender::SetBumpMapSeas(int value)
{
	g_Au.SetBumpMapSeas(value ? true : false);
}

void AuSetRender::SetDrawWeatherEffects(int value)
{
	g_Au.SetDrawWeatherEffects(value ? true : false);
}

void AuSetRender::SetDrawHelmetHUDAlways(int value)
{
	g_Au.SetDrawHelmetHUDAlways(value ? true : false);
}

void AuSetRender::GroundTextureSizeChanged(int value)
{
	g_Au.SetGroundTextureSize(value);
	m_GroundTextureSize->setText(QString("%1 pixels").arg(g_Au.GroundTextureSize()));
}

void AuSetRender::FramedPictureSizeChanged(int value)
{
	g_Au.SetFramedPictureSize(value);
	m_FramedPictureSize->setText(QString("%1 pixels").arg(g_Au.FramedPictureSize()));
}

void AuSetRender::PlantBranchMaxSegsChanged(int value)
{
	g_Au.SetPlantBranchMaxSegs(value);
}

void AuSetRender::PlantTextureSizeChanged(int value)
{
	g_Au.SetPlantTextureSize(value);
	m_PlantTextureSize->setText(QString("%1 pixels").arg(g_Au.PlantTextureSize()));
}

void AuSetRender::TreeTrunkMinDiaChanged(const QString& value)
{
	float minDia = QVariant(value).toFloat();
	g_Au.SetTreeTrunkMinDia(minDia);
}

void AuSetRender::TreeTrunkFacesChanged(int value)
{
	g_Au.SetTreeTrunkFaces(value);
}

void AuSetRender::TreeTrunkMaxSegsChanged(int value)
{
	g_Au.SetTreeTrunkMaxSegs(value);
}

void AuSetRender::SetGroundCoverQty(int value)
{
	g_Au.SetGroundCoverQty(value);
	m_GroundCoverQuantity->setText(QString("%1%").arg(value/10));
}

void AuSetRender::SetDivTexels(int value)
{
	g_Au.SetDivTexels(value);

	if(g_Au.DivTexels() == 1)
		m_WorldMapQuality->setText(tr("Low"));
	else if(g_Au.DivTexels() == 2)
		m_WorldMapQuality->setText(tr("Medium"));
	else
		m_WorldMapQuality->setText(tr("High"));
}

void AuSetRender::SetDetailedTerrainRange(int value)
{
	g_Au.SetDetailedTerrainRange(value);
	m_DetailedTerrainRange->setText(QString("%1m").arg(value));
}

void AuSetRender::SetGrassViewRange(int value)
{
	g_Au.SetGrassViewRange(value);
	m_GrassViewRange->setText(QString("%1m").arg(value));
}

void AuSetRender::SetRockViewRange(int value)
{
	g_Au.SetRockViewRange(value);
	m_RockViewRange->setText(QString("%1m").arg(value));
}

void AuSetRender::SetShrubViewRange(int value)
{
	g_Au.SetShrubViewRange(value);
	m_ShrubViewRange->setText(QString("%1m").arg(value));
}

void AuSetRender::SetBuildingViewRange(int value)
{
	g_Au.SetBuildingViewRange(value);
	m_BuildingViewRange->setText(QString("%1m").arg(value));
}

void AuSetRender::SetTreeViewRange(int value)
{
	g_Au.SetTreeViewRange(value);
	m_TreeViewRange->setText(QString("%1m").arg(value));
}

void AuSetRender::ResetDefault()
{
	//ckdrawgun->setChecked(true);
	//ckdrawminime->setChecked(true);
	//ckdrawframedpics->setChecked(true);
	//ckdrawsky->setChecked(true);
	//ckdrawprecip->setChecked(true);

	maxLightsSlider->setValue(8);
	cloudsSlider->setValue(3);
	framedPictureSizeSlider->setValue(512);
	groundTextureSizeSlider->setValue(128);
	plantTextureSizeSlider->setValue(256);
	mapQualSlider->setValue(2);
	detailedTerrainRangeSlider->setValue(800);
	rockViewRangeSlider->setValue(150);
	grassSlider->setValue(200);
	grassViewRangeSlider->setValue(100);
	shrubViewRangeSlider->setValue(200);
	treeViewRangeSlider->setValue(400);
	buildingViewRangeSlider->setValue(12000);
	m_SkyBoxSizeSlider->setValue(1024);
	m_SkyBoxMipmap->setChecked(true);

	m_FoVSlider->setValue(60);
}

void AuSetRender::LowMemoryDefault()
{
	maxLightsSlider->setValue(4);
	cloudsSlider->setValue(0);
	framedPictureSizeSlider->setValue(64);
	groundTextureSizeSlider->setValue(64);
	plantTextureSizeSlider->setValue(64);
	mapQualSlider->setValue(0);
	detailedTerrainRangeSlider->setValue(500);
	rockViewRangeSlider->setValue(150);
	grassSlider->setValue(100);
	grassViewRangeSlider->setValue(100);
	shrubViewRangeSlider->setValue(150);
	treeViewRangeSlider->setValue(300);
	buildingViewRangeSlider->setValue(6000);
	m_SkyBoxSizeSlider->setValue(512);
	m_SkyBoxMipmap->setChecked(false);
}

void AuSetRender::MaxGraphicsDefault()
{
	maxLightsSlider->setValue(MAX_POINT_LIGHTS);
	cloudsSlider->setValue(3);
	framedPictureSizeSlider->setValue(2048);
	groundTextureSizeSlider->setValue(256);
	plantTextureSizeSlider->setValue(512);
	mapQualSlider->setValue(3);
	detailedTerrainRangeSlider->setValue(1000);
	rockViewRangeSlider->setValue(500);
	grassSlider->setValue(1000);
	grassViewRangeSlider->setValue(200);
	shrubViewRangeSlider->setValue(500);
	treeViewRangeSlider->setValue(1000);
	buildingViewRangeSlider->setValue(20000);
	m_SkyBoxSizeSlider->setValue(1024);
	m_SkyBoxMipmap->setChecked(true);
}

void AuSetRender::SetOpenGLInfo(int textureUnits, int uniformLocs)
{
	if(!m_OpenGLInfo)
		return;

	QString tumsg;
	if(textureUnits < 0)
		tumsg = tr("<html>Fragment shaders require 16 texture units.</html>");
	else if(textureUnits < 16)
		tumsg = tr("<html>Fragment shaders require 16 texture units.<br>"
				"<b>OpenGL driver supports only %1 texture units.<br>"
				"This is insufficient to run <i>%3</i>.</b>"
				"<br>"
				"OpenGL driver supports %L2 uniform locations."
				"</html>").arg(textureUnits).arg(uniformLocs).arg(g_Au.ApplicationName());
	else
		tumsg = tr("<html>Fragment shaders require 16 texture units.<br>"
				"OpenGL driver supports %1 texture units."
				"<br>"
				"OpenGL driver supports %L2 uniform locations."
				"</html>").arg(textureUnits).arg(uniformLocs);

	m_OpenGLInfo->setText(tumsg);
	if(textureUnits < 16)
	{
		QPalette pal;
		pal.setColor(QPalette::WindowText, QColor(255,32,32));
		m_OpenGLInfo->setPalette(pal);
	}
}

void AuSetRender::QueryOpenGLVersions()
{
	QStringList glVers;
	AuGLEffects::OpenGLVersions(glVers);

	QString msg = QObject::tr("<html><p>The current video device says it supports the following OpenGL versions:</p>");

	if(glVers.isEmpty())
		msg += "<b>None</b><br>";
	else
		for(QStringList::const_iterator si = glVers.cbegin(); si != glVers.cend(); si++)
			msg += "<b>" + (*si) + "</b><br>";

	QMessageBox::information(0, "OpenGL Versions", msg, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void AuSetRender::SetClouds(int value)
{
	g_Au.SetDrawClouds(value);

	if(g_Au.DrawClouds() == 0)
		m_Clouds->setText(tr("None"));
	else if(g_Au.DrawClouds() == 1)
		m_Clouds->setText(tr("One Layer"));
	else if(g_Au.DrawClouds() == 2)
		m_Clouds->setText(tr("Two Layers"));
	else
		m_Clouds->setText(tr("Many Layers"));
}

void AuSetRender::SetMaxLights(int value)
{
	g_Au.SetMaxLights(value);
	m_MaxLightsLabel->setText(QString("%1 Lights").arg(g_Au.MaxLights()));
}

void AuSetRender::SetSkyBoxMipmap(int value)
{
	g_Au.SetSkyBoxMipMap(value ? true : false);
}

void AuSetRender::SetSkyBoxSize(int value)
{
	g_Au.SetSkyBoxPaneSize(value);
	m_SkyBoxSizeLabel->setText(QString("%1 pixels").arg(g_Au.SkyBoxPaneSize()));
}

void AuSetRender::FieldOfViewChanged(int value)
{
	double fovy = (double)value * DEG2RAD;
	g_Au.SetFieldOfViewY(fovy);
	m_FoVLabel->setText(QString("%1%2").arg(g_Au.FieldOfViewY()*RAD2DEG, 0,'f',0).arg(g_Au.DegreeSymbol()));
}

void AuSetRender::SetRating(int index)
{
	if(!m_RatingCombo)
		return;
	AuGlobal::RATING rating = (AuGlobal::RATING)m_RatingCombo->itemData(index).toInt();
	g_Au.SetRating(rating);
}

