//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augleffects.cpp,v 1.165 2022/04/15 20:34:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augleffects.h"
//
#include "autexturelib.h"
#include "autime.h"
#include <QDataStream>
#include <QMessageBox>
#include <QOpenGLFunctions_1_0>
#include <QOpenGLFunctions_1_1>
#include <QOpenGLFunctions_1_2>
#include <QOpenGLFunctions_1_3>
#include <QOpenGLFunctions_1_4>
#include <QOpenGLFunctions_1_5>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_2_1>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_3_1>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_3_Compatibility>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLFunctions_4_1_Compatibility>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLFunctions_4_2_Compatibility>
#include <QOpenGLFunctions_4_2_Core>
#include <QOpenGLFunctions_4_3_Compatibility>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFunctions_4_4_Compatibility>
#include <QOpenGLFunctions_4_4_Core>
#include <QOpenGLFunctions_4_5_Compatibility>
#include <QOpenGLFunctions_4_5_Core>
//#include <QOpenGLFunctions_ES2>

static bool								s_Initializing = false;	// True during InitializeGL

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Texture Library

AuTextureLibItem s_TextureLibrary[TEXLIB_Count] =
{
	AuTextureLibItem(TEXLIB_ConstructionZone,
			QObject::tr("Construction Zone"),
			":data/ConstructionZone.png"),
	AuTextureLibItem(TEXLIB_ConcreteHeavy,
			QObject::tr("Concrete Heavy"),
			":data/ConcreteHeavy.png"),
	AuTextureLibItem(TEXLIB_Tarmac,
			QObject::tr("Tarmac"),
			":data/Tarmac.png"),
	AuTextureLibItem(TEXLIB_DirtSmooth,
			QObject::tr("Dirt Smooth"),
			":data/DirtSmooth.png"),
	AuTextureLibItem(TEXLIB_CobblestoneLight,
			QObject::tr("Cobblestone Light"),
			":data/CobblestoneLight.png"),
	AuTextureLibItem(TEXLIB_DirtCrackedSmooth,
			QObject::tr("Dirt Cracked Smooth"),
			":data/DirtCrackedSmooth.png"),
	AuTextureLibItem(TEXLIB_RockTan,
			QObject::tr("Rock Tan"),
			":data/RockTan.png"),
	AuTextureLibItem(TEXLIB_SnowModerate,
			QObject::tr("Snow"),
			":data/SnowModerate.png"),
	AuTextureLibItem(TEXLIB_IceModerate,
			QObject::tr("Ice Moderate"),
			":data/IceModerate.png"),
	AuTextureLibItem(TEXLIB_ConcreteDarkModerate,
			QObject::tr("Concrete Dark Moderate"),
			":data/ConcreteDarkModerate.png"),
	AuTextureLibItem(TEXLIB_DirtCoarse,
			QObject::tr("Dirt Coarse"),
			":data/DirtCoarse.png"),
	AuTextureLibItem(TEXLIB_DirtGray,
			QObject::tr("Dirt Gray"),
			":data/DirtGray.png"),
	AuTextureLibItem(TEXLIB_DirtHeavy,
			QObject::tr("Dirt Heavy"),
			":data/DirtHeavy.png"),
	AuTextureLibItem(TEXLIB_DirtModerate,
			QObject::tr("Dirt Moderate"),
			":data/DirtModerate.png"),
	AuTextureLibItem(TEXLIB_Pebbles,
			QObject::tr("Pebbles"),
			":data/Pebbles.png"),
	AuTextureLibItem(TEXLIB_RockBrown,
			QObject::tr("Rock Brown"),
			":data/RockBrown.png"),
	AuTextureLibItem(TEXLIB_RockBrownModerate,
			QObject::tr("Rock Brown Moderate"),
			":data/RockBrownModerate.png"),
	AuTextureLibItem(TEXLIB_RockGrayCoarse,
			QObject::tr("Rock Gray Coarse"),
			":data/RockGrayCoarse.png"),
	AuTextureLibItem(TEXLIB_RockGrayHeavy,
			QObject::tr("Rock Gray Heavy"),
			":data/RockGrayHeavy.png"),
	AuTextureLibItem(TEXLIB_RockGrayModerate,
			QObject::tr("Rock Gray Moderate"),
			":data/RockGrayModerate.png"),
	AuTextureLibItem(TEXLIB_RockGraySmooth,
			QObject::tr("Rock Gray Smooth"),
			":data/RockGraySmooth.png"),
	AuTextureLibItem(TEXLIB_RockRedHeavy,
			QObject::tr("Rock Red Heavy"),
			":data/RockRedHeavy.png"),
	AuTextureLibItem(TEXLIB_RockRedModerate,
			QObject::tr("Rock Red Moderate"),
			":data/RockRedModerate.png"),
	AuTextureLibItem(TEXLIB_RockRedSmooth,
			QObject::tr("Rock Red Smooth"),
			":data/RockRedSmooth.png"),
	AuTextureLibItem(TEXLIB_SandSmooth,
			QObject::tr("Sand"),
			":data/SandSmooth.png"),
	AuTextureLibItem(TEXLIB_Magma,
			QObject::tr("Lava"),
			":data/Magma.png"),
	AuTextureLibItem(TEXLIB_SeaWater,
			QObject::tr("Sea Water"),
			":data/SeaWater.png"),
	AuTextureLibItem(TEXLIB_SupercooledLiquid,
			QObject::tr("Supercooled Liquid"),
			":data/SupercooledLiquidMedium.png"),
	AuTextureLibItem(TEXLIB_AlabasterTile1,
			QObject::tr("Tile White Diagonal"),
			":data/AlabasterTile1.png"),
	AuTextureLibItem(TEXLIB_AlabasterTile2,
			QObject::tr("Tile White Square"),
			":data/AlabasterTile2.png"),
	AuTextureLibItem(TEXLIB_BlackRoundDots,
			QObject::tr("Black Round Dots"),
			":data/BlackRoundDots.png"),
	AuTextureLibItem(TEXLIB_BlackRoundDotsBig,
			QObject::tr("Black Round Dots Big"),
			":data/BlackRoundDotsBig.png"),
	AuTextureLibItem(TEXLIB_BlackTechnoGothicTile,
			QObject::tr("Black Techno Gothic Tile"),
			":data/BlackTechnoGothicTile.png"),
	AuTextureLibItem(TEXLIB_BlueWhiteWallPanel,
			QObject::tr("Wall Blue White"),
			":data/BlueWhiteWallPanel.png"),
	AuTextureLibItem(TEXLIB_BrickLight,
			QObject::tr("Brick Light"),
			":data/BrickLight.png"),
	AuTextureLibItem(TEXLIB_BrickWall,
			QObject::tr("Brick Wall"),
			":data/BrickWall.png"),
	AuTextureLibItem(TEXLIB_BronzeMetal,
			QObject::tr("Gold"),
			":data/BronzeMetal.png"),
	AuTextureLibItem(TEXLIB_CeilingMetal,
			QObject::tr("Ceiling Metal"),
			":data/CeilingMetal.png"),
	AuTextureLibItem(TEXLIB_Concrete,
			QObject::tr("Concrete"),
			":data/Concrete.png"),
	AuTextureLibItem(TEXLIB_ConcretePrestressed,
			QObject::tr("Wall Concrete"),
			":data/ConcretePrestressed.png"),
	AuTextureLibItem(TEXLIB_CopperTubes,
			QObject::tr("Copper Tubes"),
			":data/CopperTubes.png"),
	AuTextureLibItem(TEXLIB_DirtCrackedModerate,
			QObject::tr("Dirt Cracked Moderate"),
			":data/DirtCrackedModerate.png"),
	AuTextureLibItem(TEXLIB_DirtRocky,
			QObject::tr("Dirt Rocky"),
			":data/DirtRocky.png"),
	AuTextureLibItem(TEXLIB_DryWall,
			QObject::tr("Wall Gypsum"),
			":data/DryWall.png"),
	AuTextureLibItem(TEXLIB_FloorMetal,
			QObject::tr("Tile Metal"),
			":data/FloorMetal.png"),
	AuTextureLibItem(TEXLIB_Grass,
			QObject::tr("Grass"),
			":data/Grass.png"),
	AuTextureLibItem(TEXLIB_IceSmooth,
			QObject::tr("Ice Smooth"),
			":data/IceSmooth.png"),
	AuTextureLibItem(TEXLIB_Logs,
			QObject::tr("Logs"),
			":data/Logs.png"),
	AuTextureLibItem(TEXLIB_LogsRough,
			QObject::tr("Logs Rough"),
			":data/LogsRough.png"),
	AuTextureLibItem(TEXLIB_MarbleGreenGold,
			QObject::tr("Marble Green Gold"),
			":data/MarbleGreenGold.png"),
	AuTextureLibItem(TEXLIB_Marble,
			QObject::tr("Marble"),
			":data/Marble.png"),
	AuTextureLibItem(TEXLIB_MarbleTileWhiteBlackSquares,
			QObject::tr("Tile White Black"),
			":data/MarbleTileWhiteBlackSquares.png"),
	AuTextureLibItem(TEXLIB_MetalBeaten,
			QObject::tr("Magmium"),
			":data/MetalBeaten.png"),
	AuTextureLibItem(TEXLIB_MetalCeilingGold,
			QObject::tr("Ceiling Gold"),
			":data/MetalCeilingGold.png"),
	AuTextureLibItem(TEXLIB_MetalCeilingSilver,
			QObject::tr("Ceiling Silver"),
			":data/MetalCeilingSilver.png"),
	AuTextureLibItem(TEXLIB_MetalGreenShiny,
			QObject::tr("Vulcium"),
			":data/MetalGreenShiny.png"),
	AuTextureLibItem(TEXLIB_MetalGrillHoles,
			QObject::tr("Grill Metal Holes"),
			":data/MetalGrillHoles.png"),
	AuTextureLibItem(TEXLIB_MetalPlateBox,
			QObject::tr("Plate Metal Box"),
			":data/MetalPlateBox.png"),
	AuTextureLibItem(TEXLIB_MetalPlateSquares,
			QObject::tr("Plate Metal Squares"),
			":data/MetalPlateSquares.png"),
	AuTextureLibItem(TEXLIB_MetalPlateX,
			QObject::tr("Plate Metal X"),
			":data/MetalPlateX.png"),
	AuTextureLibItem(TEXLIB_MetalScratchBumps,
			QObject::tr("Metal Scratch Bumps"),
			":data/MetalScratchBumps.png"),
	AuTextureLibItem(TEXLIB_MetalScratchRust,
			QObject::tr("Metal Scratch Rust"),
			":data/MetalScratchRust.png"),
	AuTextureLibItem(TEXLIB_MetalTread,
			QObject::tr("Metal Tread"),
			":data/MetalTread.png"),
	AuTextureLibItem(TEXLIB_Obsidian,
			QObject::tr("Marble Obsidian"),
			":data/Obsidian.png"),
	AuTextureLibItem(TEXLIB_OSB,
			QObject::tr("OSB"),
			":data/OSB.png"),
	AuTextureLibItem(TEXLIB_PopcornCeilingHeavy,
			QObject::tr("Popcorn Ceiling Heavy"),
			":data/PopcornCeilingHeavy.png"),
	AuTextureLibItem(TEXLIB_PopcornCeilingLight,
			QObject::tr("Popcorn Ceiling Light"),
			":data/PopcornCeilingLight.png"),
	AuTextureLibItem(TEXLIB_RedWhiteWallPanel,
			QObject::tr("Wall Red White"),
			":data/RedWhiteWallPanel.png"),
	AuTextureLibItem(TEXLIB_SeaDark,
			QObject::tr("Sea Dark"),
			":data/SeaDark.png"),
	AuTextureLibItem(TEXLIB_SeaLight,
			QObject::tr("Sea Light"),
			":data/SeaLight.png"),
	AuTextureLibItem(TEXLIB_SeaMedium,
			QObject::tr("Sea Medium"),
			":data/SeaMedium.png"),
	AuTextureLibItem(TEXLIB_ShakeShingles,
			QObject::tr("Shingles Shake"),
			":data/ShakeShingles.png"),
	AuTextureLibItem(TEXLIB_ShinglesAsphalt,
			QObject::tr("Shingles Asphalt"),
			":data/ShinglesAsphalt.png"),
	AuTextureLibItem(TEXLIB_StainedGlassAbstract,
			QObject::tr("Stained Glass Abstract"),
			":data/StainedGlassAbstract.png"),
	AuTextureLibItem(TEXLIB_StainedGlassAngels,
			QObject::tr("Stained Glass Angels"),
			":data/StainedGlassAngels.png"),
	AuTextureLibItem(TEXLIB_StainedGlassAnimals,
			QObject::tr("Stained Glass Animals"),
			":data/StainedGlassAnimals.png"),
	AuTextureLibItem(TEXLIB_StainedGlassDeity,
			QObject::tr("Stained Glass Deity"),
			":data/StainedGlassDeity.png"),
	AuTextureLibItem(TEXLIB_StainedGlassLaborers,
			QObject::tr("Stained Glass Laborers"),
			":data/StainedGlassLaborers.png"),
	AuTextureLibItem(TEXLIB_StainedGlassSunMoon,
			QObject::tr("Stained Glass Sun Moon"),
			":data/StainedGlassSunMoon.png"),
	AuTextureLibItem(TEXLIB_StoneFloorTileSquares,
			QObject::tr("Tile Stone"),
			":data/StoneFloorTileSquares.png"),
	AuTextureLibItem(TEXLIB_StoneTileGreenTanTriangles,
			QObject::tr("Tile Green Tan Triangles"),
			":data/StoneTileGreenTanTriangles.png"),
	AuTextureLibItem(TEXLIB_StuccoPaintPlaster,
			QObject::tr("Stucco Light"),
			":data/StuccoPaintPlaster.png"),
	AuTextureLibItem(TEXLIB_StuccoWallPlaster,
			QObject::tr("Stucco Heavy"),
			":data/StuccoWallPlaster.png"),
	AuTextureLibItem(TEXLIB_TanMarble,
			QObject::tr("Marble Tan"),
			":data/TanMarble.png"),
	AuTextureLibItem(TEXLIB_TerraCottaRoof,
			QObject::tr("Shingles Terra Cotta"),
			":data/TerraCottaRoof.png"),
	AuTextureLibItem(TEXLIB_ThomasCole1Savage,
			QObject::tr("Artwork Thomas Cole 1.Savage"),
			":data/ThomasCole.1.Savage.png"),
	AuTextureLibItem(TEXLIB_ThomasCole2Pastoral,
			QObject::tr("Artwork Thomas Cole 2.Pastoral"),
			":data/ThomasCole.2.Pastoral.png"),
	AuTextureLibItem(TEXLIB_ThomasCole3Consummation,
			QObject::tr("Artwork Thomas Cole 3.Consummation"),
			":data/ThomasCole.3.Consummation.png"),
	AuTextureLibItem(TEXLIB_ThomasCole4Destruction,
			QObject::tr("Artwork Thomas Cole 4.Destruction"),
			":data/ThomasCole.4.Destruction.png"),
	AuTextureLibItem(TEXLIB_ThomasCole5Desolation,
			QObject::tr("Artwork Thomas Cole 5.Desolation"),
			":data/ThomasCole.5.Desolation.png"),
	AuTextureLibItem(TEXLIB_TileJade,
			QObject::tr("Tile Jade"),
			":data/TileJade.png"),
	AuTextureLibItem(TEXLIB_VentBlack,
			QObject::tr("Grill Black"),
			":data/VentBlack.png"),
	AuTextureLibItem(TEXLIB_VentCopper,
			QObject::tr("Grill Copper"),
			":data/VentCopper.png"),
	AuTextureLibItem(TEXLIB_VentWhite,
			QObject::tr("Grill White"),
			":data/VentWhite.png"),
	AuTextureLibItem(TEXLIB_WallCargo,
			QObject::tr("Wall Cargo"),
			":data/WallCargo.png"),
	AuTextureLibItem(TEXLIB_WallKnockdown,
			QObject::tr("Knockdown"),
			":data/WallKnockdown.png"),
	AuTextureLibItem(TEXLIB_WallMetal,
			QObject::tr("Wall Metal"),
			":data/WallMetal.png"),
	AuTextureLibItem(TEXLIB_WallPipes,
			QObject::tr("Wall Pipes"),
			":data/WallPipes.png"),
	AuTextureLibItem(TEXLIB_WoodBare,
			QObject::tr("Wood Bare"),
			":data/WoodBare.png"),
	AuTextureLibItem(TEXLIB_WoodFloorTile,
			QObject::tr("Tile Wood"),
			":data/WoodFloorTile.png"),
	AuTextureLibItem(TEXLIB_WoodLattice,
			QObject::tr("Wood Lattice"),
			":data/WoodLattice.png"),
	AuTextureLibItem(TEXLIB_WoodPlanksBeams,
			QObject::tr("Wood Planks Beams"),
			":data/WoodPlanksBeams.png"),
	AuTextureLibItem(TEXLIB_WoodSidingPainted,
			QObject::tr("Wood Siding Painted"),
			":data/WoodSidingPainted.png"),
	AuTextureLibItem(TEXLIB_WoodSiding,
			QObject::tr("Wood Siding"),
			":data/WoodSiding.png"),
	AuTextureLibItem(TEXLIB_WoodTileRustic,
			QObject::tr("Tile Wood Rustic"),
			":data/WoodTileRustic.png"),
	AuTextureLibItem(TEXLIB_WoodWallStuds,
			QObject::tr("Wall Wood Studs"),
			":data/WoodWallStuds.png"),
	AuTextureLibItem(TEXLIB_MetalBrushed,
			QObject::tr("Metal Brushed"),
			":data/MetalBrushed.png"),
	AuTextureLibItem(TEXLIB_GrillBlackOblongHoles,
			QObject::tr("Grill Black Oblong Holes"),
			":data/GrillBlackOblongHoles.png"),
	AuTextureLibItem(TEXLIB_MagmiumPlateBox,
			QObject::tr("Plate Magmium Box"),
			":data/MagmiumPlateBox.png"),
	AuTextureLibItem(TEXLIB_MagmiumPlateSquares,
			QObject::tr("Plate Magmium Squares"),
			":data/MagmiumPlateSquares.png"),
	AuTextureLibItem(TEXLIB_MagmiumPlateX,
			QObject::tr("Plate Magmium X"),
			":data/MagmiumPlateX.png"),
	AuTextureLibItem(TEXLIB_VulciumPlateBox,
			QObject::tr("Plate Vulcium Box"),
			":data/VulciumPlateBox.png"),
	AuTextureLibItem(TEXLIB_VulciumPlateSquares,
			QObject::tr("Plate Vulcium Squares"),
			":data/VulciumPlateSquares.png"),
	AuTextureLibItem(TEXLIB_VulciumPlateX,
			QObject::tr("Plate Vulcium X"),
			":data/VulciumPlateX.png"),
	AuTextureLibItem(TEXLIB_Decals,
			QObject::tr("Decals and Signs"),
			":data/Decals.png"),
	AuTextureLibItem(TEXLIB_Fabric,
			QObject::tr("Fabric"),
			":data/Fabric.png"),
	AuTextureLibItem(TEXLIB_StoneWall,
			QObject::tr("Stone"),
			":data/StoneWall.png"),
	AuTextureLibItem(TEXLIB_AdamantinePlateBox,
			QObject::tr("Plate Adamantine Box"),
			":data/AdamantinePlateBox.png"),
	AuTextureLibItem(TEXLIB_AdamantinePlateSquares,
			QObject::tr("Plate Adamantine Squares"),
			":data/AdamantinePlateSquares.png"),
	AuTextureLibItem(TEXLIB_AdamantinePlateX,
			QObject::tr("Plate Adamantine X"),
			":data/AdamantinePlateX.png"),
	AuTextureLibItem(TEXLIB_ChainLink,
			QObject::tr("Chain Link Fence"),
			":data/ChainLinkFence.png"),
	AuTextureLibItem(TEXLIB_SandBags,
			QObject::tr("Sand Bags"),
			":data/SandBagWall.png"),
};

AuTextureLibItem s_EffectLibrary[AuTextureLibItem::EI_Count] =
{
	AuTextureLibItem(AuTextureLibItem::EI_None,
			QObject::tr("No Effect"), ":data/EffBlank.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Capacitor1,
			QObject::tr("Capacitor Effect"), ":data/EffCapacitor1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Capacitor2,
			QObject::tr("Capacitor Effect"), ":data/EffCapacitor2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Capacitor3,
			QObject::tr("Capacitor Effect"), ":data/EffCapacitor3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Capacitor4,
			QObject::tr("Capacitor Effect"), ":data/EffCapacitor4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Capacitor5,
			QObject::tr("Capacitor Effect"), ":data/EffCapacitor5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Capacitor6,
			QObject::tr("Capacitor Effect"), ":data/EffCapacitor6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDrive1,
			QObject::tr("FTL Drive Effect"), ":data/EffFTL1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDrive2,
			QObject::tr("FTL Drive Effect"), ":data/EffFTL2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDrive3,
			QObject::tr("FTL Drive Effect"), ":data/EffFTL3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDrive4,
			QObject::tr("FTL Drive Effect"), ":data/EffFTL4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDrive5,
			QObject::tr("FTL Drive Effect"), ":data/EffFTL5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDrive6,
			QObject::tr("FTL Drive Effect"), ":data/EffFTL6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDriveLow1,
			QObject::tr("FTL Drive Low Effect"), ":data/EffFTLLow1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDriveLow2,
			QObject::tr("FTL Drive Low Effect"), ":data/EffFTLLow2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDriveLow3,
			QObject::tr("FTL Drive Low Effect"), ":data/EffFTLLow3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDriveLow4,
			QObject::tr("FTL Drive Low Effect"), ":data/EffFTLLow4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDriveLow5,
			QObject::tr("FTL Drive Low Effect"), ":data/EffFTLLow5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_FTLDriveLow6,
			QObject::tr("FTL Drive Low Effect"), ":data/EffFTLLow6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWarp1,
			QObject::tr("Hull Warp Effect"), ":data/EffWarp1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWarp2,
			QObject::tr("Hull Warp Effect"), ":data/EffWarp2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWarp3,
			QObject::tr("Hull Warp Effect"), ":data/EffWarp3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWarp4,
			QObject::tr("Hull Warp Effect"), ":data/EffWarp4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWarp5,
			QObject::tr("Hull Warp Effect"), ":data/EffWarp5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWarp6,
			QObject::tr("Hull Warp Effect"), ":data/EffWarp6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormPos1,
			QObject::tr("Hull Positive Wormhole Effect"), ":data/EffWormPos1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormPos2,
			QObject::tr("Hull Positive Wormhole Effect"), ":data/EffWormPos2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormPos3,
			QObject::tr("Hull Positive Wormhole Effect"), ":data/EffWormPos3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormPos4,
			QObject::tr("Hull Positive Wormhole Effect"), ":data/EffWormPos4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormPos5,
			QObject::tr("Hull Positive Wormhole Effect"), ":data/EffWormPos5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormPos6,
			QObject::tr("Hull Positive Wormhole Effect"), ":data/EffWormPos6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_LifeSupport1,
			QObject::tr("Life Support Effect"), ":data/EffLifeSupport1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_LifeSupport2,
			QObject::tr("Life Support Effect"), ":data/EffLifeSupport2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_LifeSupport3,
			QObject::tr("Life Support Effect"), ":data/EffLifeSupport3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_LifeSupport4,
			QObject::tr("Life Support Effect"), ":data/EffLifeSupport4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_LifeSupport5,
			QObject::tr("Life Support Effect"), ":data/EffLifeSupport5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_LifeSupport6,
			QObject::tr("Life Support Effect"), ":data/EffLifeSupport6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDrive1,
			QObject::tr("Maneuver Drive Effect"), ":data/EffManeuver1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDrive2,
			QObject::tr("Maneuver Drive Effect"), ":data/EffManeuver2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDrive3,
			QObject::tr("Maneuver Drive Effect"), ":data/EffManeuver3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDrive4,
			QObject::tr("Maneuver Drive Effect"), ":data/EffManeuver4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDrive5,
			QObject::tr("Maneuver Drive Effect"), ":data/EffManeuver5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDrive6,
			QObject::tr("Maneuver Drive Effect"), ":data/EffManeuver6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDriveLow1,
			QObject::tr("Maneuver Drive Low Effect"), ":data/EffManeuverLow1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDriveLow2,
			QObject::tr("Maneuver Drive Low Effect"), ":data/EffManeuverLow2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDriveLow3,
			QObject::tr("Maneuver Drive Low Effect"), ":data/EffManeuverLow3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDriveLow4,
			QObject::tr("Maneuver Drive Low Effect"), ":data/EffManeuverLow4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDriveLow5,
			QObject::tr("Maneuver Drive Low Effect"), ":data/EffManeuverLow5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_ManeuverDriveLow6,
			QObject::tr("Maneuver Drive Low Effect"), ":data/EffManeuverLow6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_PowerPlant1,
			QObject::tr("Power Plant Effect"), ":data/EffPowerPlant1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_PowerPlant2,
			QObject::tr("Power Plant Effect"), ":data/EffPowerPlant2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_PowerPlant3,
			QObject::tr("Power Plant Effect"), ":data/EffPowerPlant3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_PowerPlant4,
			QObject::tr("Power Plant Effect"), ":data/EffPowerPlant4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_PowerPlant5,
			QObject::tr("Power Plant Effect"), ":data/EffPowerPlant5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_PowerPlant6,
			QObject::tr("Power Plant Effect"), ":data/EffPowerPlant6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Sensor1,
			QObject::tr("Sensor Effect"), ":data/EffSensor1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Sensor2,
			QObject::tr("Sensor Effect"), ":data/EffSensor2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Sensor3,
			QObject::tr("Sensor Effect"), ":data/EffSensor3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Sensor4,
			QObject::tr("Sensor Effect"), ":data/EffSensor4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Sensor5,
			QObject::tr("Sensor Effect"), ":data/EffSensor5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Sensor6,
			QObject::tr("Sensor Effect"), ":data/EffSensor6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Shield1,
			QObject::tr("Shield Effect"), ":data/EffShield1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Shield2,
			QObject::tr("Shield Effect"), ":data/EffShield2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Shield3,
			QObject::tr("Shield Effect"), ":data/EffShield3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Shield4,
			QObject::tr("Shield Effect"), ":data/EffShield4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Shield5,
			QObject::tr("Shield Effect"), ":data/EffShield5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Shield6,
			QObject::tr("Shield Effect"), ":data/EffShield6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Transporter1,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporter1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Transporter2,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporter2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Transporter3,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporter3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Transporter4,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporter4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Transporter5,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporter5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Transporter6,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporter6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_TransporterLow1,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporterLow1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_TransporterLow2,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporterLow2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_TransporterLow3,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporterLow3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_TransporterLow4,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporterLow4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_TransporterLow5,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporterLow5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_TransporterLow6,
			QObject::tr("Transporter Pad Effect"), ":data/EffTransporterLow6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Weapon1,
			QObject::tr("Weapon Effect"), ":data/EffWeapon1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Weapon2,
			QObject::tr("Weapon Effect"), ":data/EffWeapon2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Weapon3,
			QObject::tr("Weapon Effect"), ":data/EffWeapon3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Weapon4,
			QObject::tr("Weapon Effect"), ":data/EffWeapon4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Weapon5,
			QObject::tr("Weapon Effect"), ":data/EffWeapon5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Weapon6,
			QObject::tr("Weapon Effect"), ":data/EffWeapon6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeg1,
			QObject::tr("Hull Negative Wormhole Effect"), ":data/EffWormNeg1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeg2,
			QObject::tr("Hull Negative Wormhole Effect"), ":data/EffWormNeg2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeg3,
			QObject::tr("Hull Negative Wormhole Effect"), ":data/EffWormNeg3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeg4,
			QObject::tr("Hull Negative Wormhole Effect"), ":data/EffWormNeg4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeg5,
			QObject::tr("Hull Negative Wormhole Effect"), ":data/EffWormNeg5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeg6,
			QObject::tr("Hull Negative Wormhole Effect"), ":data/EffWormNeg6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeu1,
			QObject::tr("Hull Neutral Wormhole Effect"), ":data/EffWormNeu1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeu2,
			QObject::tr("Hull Neutral Wormhole Effect"), ":data/EffWormNeu2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeu3,
			QObject::tr("Hull Neutral Wormhole Effect"), ":data/EffWormNeu3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeu4,
			QObject::tr("Hull Neutral Wormhole Effect"), ":data/EffWormNeu4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeu5,
			QObject::tr("Hull Neutral Wormhole Effect"), ":data/EffWormNeu5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_HullWormNeu6,
			QObject::tr("Hull Neutral Wormhole Effect"), ":data/EffWormNeu6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage1,
			QObject::tr("Hull Damage 1"), ":data/EffDamage1.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage2,
			QObject::tr("Hull Damage 2"), ":data/EffDamage2.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage3,
			QObject::tr("Hull Damage 3"), ":data/EffDamage3.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage4,
			QObject::tr("Hull Damage 4"), ":data/EffDamage4.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage5,
			QObject::tr("Hull Damage 5"), ":data/EffDamage5.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage6,
			QObject::tr("Hull Damage 6"), ":data/EffDamage6.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage7,
			QObject::tr("Hull Damage 7"), ":data/EffDamage7.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage8,
			QObject::tr("Hull Damage 8"), ":data/EffDamage8.png"),
	AuTextureLibItem(AuTextureLibItem::EI_Damage9,
			QObject::tr("Hull Damage 9"), ":data/EffDamage9.png"),
};

// Gets/loads a texture.
// OpenGL context must be current when calling this function.
//static
GLuint AuTextureLibItem::LibraryTexture(AuGLEffects& re, quint16 libTexId)
{
	QString sid = QString("LTx%1").arg(libTexId);
	GLuint custom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, LibraryTextureFile(libTexId), GL_LINEAR, GL_LINEAR, true);
	return custom;
}

GLuint AuTextureLibItem::EffectTexture(AuGLEffects& re, SURFACE_EFFECT effect, int frame)
{
	AuTextureLibItem& ei = EffectTextureItem(effect, frame);
	return EffectTexture(re, (EFFECT_INDEX)ei.Id());
}

GLuint AuTextureLibItem::EffectTexture(AuGLEffects& re, EFFECT_INDEX effIdx)
{
	QString sid = QString("ETx%1").arg(effIdx);
	GLuint custom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, EffectTextureFile(effIdx), GL_LINEAR, GL_LINEAR,
			true);
	return custom;
}

// Returns the number of textures available in the library.
//static
quint16 AuTextureLibItem::LibraryTextureCount()
{
	return TEXLIB_Count;
}

//static
AuTextureLibItem& AuTextureLibItem::EffectTextureItem(EFFECT_INDEX effIdx)
{
#if !defined(QT_NO_DEBUG)
	// This will prevent me from letting these get out of sync.
	int texLibCount = sizeof(s_EffectLibrary) / sizeof(AuTextureLibItem);
	Q_ASSERT(texLibCount == EI_Count);
#endif
	if(effIdx >= EI_Count)
		effIdx = EI_None;
	return s_EffectLibrary[effIdx];
}

AuTextureLibItem& AuTextureLibItem::EffectTextureItem(SURFACE_EFFECT effect)
{
	EFFECT_INDEX effi[EFF_Count] =
	{
		EI_None,						// EFF_None
		EI_FTLDrive1,					// EFF_FTLDrive 1
		EI_FTLDrive1,					// EFF_FTLDrive 2
		EI_FTLDrive1,					// EFF_FTLDrive 3
		EI_FTLDrive1,					// EFF_FTLDrive 4
		EI_LifeSupport1,				// EFF_LifeSupport
		EI_ManeuverDrive1,				// EFF_ManeuverDrive
		EI_PowerPlant1,					// EFF_PowerPlant
		EI_Sensor1,						// EFF_Sensor
		EI_Shield1,						// EFF_Shield
		EI_Transporter1,				// EFF_TransporterPad 1
		EI_Transporter1,				// EFF_TransporterPad 2
		EI_Transporter1,				// EFF_TransporterPad 3
		EI_Transporter1,				// EFF_TransporterPad 4
		EI_Transporter1,				// EFF_TransporterPad 5
		EI_Transporter1,				// EFF_TransporterPad 6
		EI_Transporter1,				// EFF_TransporterPad 7
		EI_Transporter1,				// EFF_TransporterPad 8
		EI_Transporter1,				// EFF_TransporterPad 9
		EI_Transporter1,				// EFF_TransporterPad 10
		EI_Weapon1,						// EFF_Weapon 1
		EI_Weapon1,						// EFF_Weapon 2
		EI_Weapon1,						// EFF_Weapon 3
		EI_Weapon1,						// EFF_Weapon 4
		EI_Weapon1,						// EFF_Weapon 5
		EI_Weapon1,						// EFF_Weapon 6
		EI_Weapon1,						// EFF_Weapon 7
		EI_Weapon1,						// EFF_Weapon 8
		EI_Weapon1,						// EFF_Weapon 9
		EI_Weapon1,						// EFF_Weapon 10
		EI_Capacitor1,					// EFF_Capacitor
		EI_HullWormPos1,				// EFF_Hull
	};
	return EffectTextureItem(effi[effect]);
}

AuTextureLibItem& AuTextureLibItem::EffectTextureItem(SURFACE_EFFECT effect, int frame)
{
	EFFECT_INDEX effi[EFF_Count] =
	{
		EI_None,						// EFF_None
		EI_FTLDrive1,					// EFF_FTLDrive 1
		EI_FTLDrive1,					// EFF_FTLDrive 2
		EI_FTLDrive1,					// EFF_FTLDrive 3
		EI_FTLDrive1,					// EFF_FTLDrive 4
		EI_LifeSupport1,				// EFF_LifeSupport
		EI_ManeuverDrive1,				// EFF_ManeuverDrive
		EI_PowerPlant1,					// EFF_PowerPlant
		EI_Sensor1,						// EFF_Sensor
		EI_Shield1,						// EFF_Shield
		EI_Transporter1,				// EFF_TransporterPad 1
		EI_Transporter1,				// EFF_TransporterPad 2
		EI_Transporter1,				// EFF_TransporterPad 3
		EI_Transporter1,				// EFF_TransporterPad 4
		EI_Transporter1,				// EFF_TransporterPad 5
		EI_Transporter1,				// EFF_TransporterPad 6
		EI_Transporter1,				// EFF_TransporterPad 7
		EI_Transporter1,				// EFF_TransporterPad 8
		EI_Transporter1,				// EFF_TransporterPad 9
		EI_Transporter1,				// EFF_TransporterPad 10
		EI_Weapon1,						// EFF_Weapon 1
		EI_Weapon1,						// EFF_Weapon 2
		EI_Weapon1,						// EFF_Weapon 3
		EI_Weapon1,						// EFF_Weapon 4
		EI_Weapon1,						// EFF_Weapon 5
		EI_Weapon1,						// EFF_Weapon 6
		EI_Weapon1,						// EFF_Weapon 7
		EI_Weapon1,						// EFF_Weapon 8
		EI_Weapon1,						// EFF_Weapon 9
		EI_Weapon1,						// EFF_Weapon 10
		EI_Capacitor1,					// EFF_Capacitor
		EI_HullWormPos1,				// EFF_Hull
	};

	int effectIndex = effi[effect];
	if(effect != EFF_None)
		effectIndex += frame;
	Q_ASSERT(effectIndex >= EI_None && effectIndex < EI_Count);
	return EffectTextureItem((EFFECT_INDEX)effectIndex);
}

//static
AuTextureLibItem& AuTextureLibItem::LibraryTextureItem(quint16 libTexId)
{
#if !defined(QT_NO_DEBUG)
	// This will prevent me from letting these get out of sync.
	int texLibCount = sizeof(s_TextureLibrary) / sizeof(AuTextureLibItem);
	Q_ASSERT(texLibCount == TEXLIB_Count);
#endif
	if(libTexId >= TEXLIB_Count)
		libTexId = 0;
	return s_TextureLibrary[libTexId];
}

//static
quint16 AuTextureLibItem::LibraryTextureHullMetal()
{
	return TEXLIB_MetalPlateBox;
}

//static
quint16 AuTextureLibItem::LibraryTextureHullMagmium()
{
	return TEXLIB_MagmiumPlateBox;
}

//static
quint16 AuTextureLibItem::LibraryTextureHullWood()
{
	return TEXLIB_LogsRough;
}

//static
quint16 AuTextureLibItem::LibraryTextureHullLumber()
{
	return TEXLIB_WoodSiding;
}

//static
quint16 AuTextureLibItem::LibraryTextureHullStone()
{
	return TEXLIB_StoneWall;
}

//static
quint16 AuTextureLibItem::LibraryTextureHullAdamantine()
{
	return TEXLIB_AdamantinePlateBox;
}

//static
quint16 AuTextureLibItem::LibraryTextureHullVulcium()
{
	return TEXLIB_VulciumPlateBox;
}

// Returns the library texture id for the background of the person's body stats.
//static
quint16 AuTextureLibItem::LibraryTextureStatsBody()
{
	return TEXLIB_BlackTechnoGothicTile;
	return TEXLIB_MetalBrushed;
}

// Returns the library texture id for the background of the person's vehicle stats.
//static
quint16 AuTextureLibItem::LibraryTextureStatsVehicle()
{
	return TEXLIB_MetalGrillHoles;
}

//static
const QString& AuTextureLibItem::LibraryTextureFile(quint16 libTexId)
{
	return LibraryTextureItem(libTexId).FileName();
}

//static
const QString& AuTextureLibItem::LibraryTextureName(quint16 libTexId)
{
	return LibraryTextureItem(libTexId).Name();
}

//static
const QString& AuTextureLibItem::EffectTextureFile(SURFACE_EFFECT effect, int frame)
{
	return EffectTextureItem(effect, frame).FileName();
}

//static
const QString& AuTextureLibItem::EffectTextureFile(EFFECT_INDEX effIdx)
{
	return EffectTextureItem(effIdx).FileName();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QColor AuGLSun::SunsetColor(const QColor& atmColor, float atmDensity) const
{
	QColor sunsetColor;
	if(m_Enabled)
	{
		sunsetColor.setRgbF(m_ColorR, m_ColorG, m_ColorB);

		qreal h,s,v;
		sunsetColor.getHsvF(&h, &s, &v);

		// Combine atmosphere color.
		qreal ah,as,av;
		atmColor.getHsvF(&ah, &as, &av);

		// Interpolate between hues of sun and atmosphere based on density.
		qreal sw = ah - h;				// Calc span from sun hue to atm hue.
		if(sw > .5)
			sw = sw - 1;				// The span is more than 1/2 way; go the shorter way around the hue wheel.
		else if(sw < -.5)
			sw = 1 + sw;				// The span is more than 1/2 way; go the shorter way around the hue wheel.
		h = h + sw * (atmDensity * .5);	// Shift sun hue toward atm hue based on density.
		if(h < 0)
			h += 1;						// Wrap
		else if(h > 1)
			h -= 1;						// Wrap

		// Saturation is increased for richness.
		s *= 5;
		if(s > 1)
			s = 1;

		sunsetColor.setHsvF(h, s, v);
	}
	return sunsetColor;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGLEffects::AuGLEffects()
{
	m_EyePos = AuPoint(0,0);
	m_HUDItemsTexture = 0;
}

AuGLEffects::~AuGLEffects()
{
	// Clean up textures.
	for(QVector<AuGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); it++)
		delete *it;
	m_Textures.clear();
}

// Must be called while GL context is current.
void AuGLEffects::InitializeGL(unsigned int maxLights)
{
	if(s_Initializing)
		return;
	s_Initializing = true;
	if(!initializeOpenGLFunctions())
	{
		// Try to determine what profiles ARE supported.
		QStringList glsupport;
		OpenGLVersions(glsupport);

		QString msg = QObject::tr("<html><p><b>%1</b> functions could not be initialized. "
				"This is usually because the computer's video device does not support the version of OpenGL needed "
				"to run this software.</p>"
#if defined(Q_OS_MACOS)
				"<p>Use Launcher settings to select between <b>OpenGL 3.2</b> and <b>OpenGL 4.1</b> compatible program files.</p>"
#else
				"<p>An updated video driver may fix the problem. "
				"The generic video driver distributed with most operating systems is not usually adequate. "
				"You should visit the web site of the manufacturer of your graphics processor to get the best driver.</p>"
#endif
				"<p>The current video device says it supports the following OpenGL versions:</p>").arg(GLFuncName);

		if(glsupport.isEmpty())
			msg += "<b>None</b><br>";
		else
			for(QStringList::const_iterator si = glsupport.cbegin(); si != glsupport.cend(); si++)
				msg += "<b>" + (*si) + "</b><br>";

		msg += "<p>Without OpenGL, the game scene cannot be painted on the screen. <b>This program must exit.</b></p></html>";

		QMessageBox::critical(0, "OpenGL Compatibility Failure", msg, QMessageBox::Abort, QMessageBox::NoButton,
				QMessageBox::NoButton);
		exit(-1);
	}

	m_GLSL.Initialize(*this, maxLights);

	s_Initializing = false;
}

//static
void AuGLEffects::OpenGLVersions(QStringList& glsupport)
{
	// Try to determine what profiles ARE supported.
	QOpenGLFunctions_1_0 gl10;
	if(gl10.initializeOpenGLFunctions())
		glsupport << "OpenGL 1.0";
	QOpenGLFunctions_1_1 gl11;
	if(gl11.initializeOpenGLFunctions())
		glsupport << "OpenGL 1.1";
	QOpenGLFunctions_1_2 gl12;
	if(gl12.initializeOpenGLFunctions())
		glsupport << "OpenGL 1.2";
	QOpenGLFunctions_1_3 gl13;
	if(gl13.initializeOpenGLFunctions())
		glsupport << "OpenGL 1.3";
	QOpenGLFunctions_1_4 gl14;
	if(gl14.initializeOpenGLFunctions())
		glsupport << "OpenGL 1.4";
	QOpenGLFunctions_1_5 gl15;
	if(gl15.initializeOpenGLFunctions())
		glsupport << "OpenGL 1.5";
	QOpenGLFunctions_2_0 gl20;
	if(gl20.initializeOpenGLFunctions())
		glsupport << "OpenGL 2.0";
	QOpenGLFunctions_2_1 gl21;
	if(gl21.initializeOpenGLFunctions())
		glsupport << "OpenGL 2.1";
	QOpenGLFunctions_3_0 gl30;
	if(gl30.initializeOpenGLFunctions())
		glsupport << "OpenGL 3.0";
	QOpenGLFunctions_3_1 gl31;
	if(gl31.initializeOpenGLFunctions())
		glsupport << "OpenGL 3.1";
	QOpenGLFunctions_3_2_Compatibility gl32c;
	if(gl32c.initializeOpenGLFunctions())
		glsupport << "OpenGL 3.2 Compatiblity";
	QOpenGLFunctions_3_2_Core gl32;
	if(gl32.initializeOpenGLFunctions())
		glsupport << "OpenGL 3.2 Core";
	QOpenGLFunctions_3_3_Compatibility gl33c;
	if(gl33c.initializeOpenGLFunctions())
		glsupport << "OpenGL 3.3 Compatiblity";
	QOpenGLFunctions_3_3_Core gl33;
	if(gl33.initializeOpenGLFunctions())
		glsupport << "OpenGL 3.3 Core";
	QOpenGLFunctions_4_0_Compatibility gl40c;
	if(gl40c.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.0 Compatiblity";
	QOpenGLFunctions_4_0_Core gl40;
	if(gl40.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.0 Core";
	QOpenGLFunctions_4_1_Compatibility gl41c;
	if(gl41c.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.1 Compatiblity";
	QOpenGLFunctions_4_1_Core gl41;
	if(gl41.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.1 Core";
	QOpenGLFunctions_4_2_Compatibility gl42c;
	if(gl42c.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.2 Compatiblity";
	QOpenGLFunctions_4_2_Core gl42;
	if(gl42.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.2 Core";
	QOpenGLFunctions_4_3_Compatibility gl43c;
	if(gl43c.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.3 Compatiblity";
	QOpenGLFunctions_4_3_Core gl43;
	if(gl43.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.3 Core";
	QOpenGLFunctions_4_4_Compatibility gl44c;
	if(gl44c.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.4 Compatiblity";
	QOpenGLFunctions_4_4_Core gl44;
	if(gl44.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.4 Core";
	QOpenGLFunctions_4_5_Compatibility gl45c;
	if(gl45c.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.5 Compatiblity";
	QOpenGLFunctions_4_5_Core gl45;
	if(gl45.initializeOpenGLFunctions())
		glsupport << "OpenGL 4.5 Core";
	//QOpenGLFunctions_ES2 gles2;
	//if(gles2.initializeOpenGLFunctions())
		//glsupport << "OpenGL ES2";
}

//// Eye Space Lighting vs. Object Space Lighting
// If nothing is required for the lighting calculation to be in Eye Space,
// then it is 'cheaper' to use Object Space Lighting for the following reason
// listed in the GPU work done by the shader.
// Eye Space Lighting
// 1)	CPU work:	Pass light pos down in Eye Space, setup is done once
//		GPU work:	Each vertex does the following setup work
//						a) Puts position in Eye Space
//						b) Puts normal in Eye Space
//	 or
// Object Space Lighting
// 2)	CPU work:	Pass light pos down in Object Space, setup is done for every object
//		GPU work:	Each vertex does the following setup work
//						a) No extra work needed
// Notice how there is no additional setup work for the GPU when working in Object
// Space. As there is always more vertices's than objects in a scene the additional
// CPU work is negligible.  Since we don't need anything to be in Eye Space for our
// shader it is for this reason that ALL our lighting calculations should be done in
// Object Space.
//
//// NOTES ON BUMP MAPPING
// For bump mapping to work we need everything to be in Tangent Space
// let's take a look at Eye Space vs. Object Space Lighting again.
// Eye Space Lighting
// 1)	CPU work:	Pass light pos down in Eye Space, setup is done once
//		GPU work:	Each vertex does the following setup work
//						a) Changes light pos to Object Space
//						b) Puts light direction in Tangent Space
//						c) Puts eye direction in Tangent Space
//	 or
// Object Space Lighting
// 2)	CPU work:	Pass light pos down in Object Space, setup is done for every object
//		GPU work:	Each vertex does the following setup work
//						a) Puts light direction in Tangent Space
//						b) Puts eye direction in Tangent Space
// Again we see that there are fewer steps needed per-vertex when working in Object Space
// so this will be our preferred method of lighting.

// Don't actually change the values in m_Sun or m_Lights as those positions are needed
// for other objects in the universe. This function will pass those positions down to
// the shader which is expecting them in Object Space. This requires the caller to setup
// the positions in Object Space and pass them down.
// Ex:	AuMatrix4x4 tThis = object being rendered with shader, includes all transformations
//		AuMatrix4x4 tSun;   sun position
//		tSun.Translate(re.SunLocation(iIndex));		// include transformation of sun
//		AuMatrix4x4 tSun  = tThis.Inverse()*tSun;
//		AuPoint pObjSpaceLoc = tSun.Translation();
//		re.SetSunLocation(iIndex, pObjSpaceLoc, cgp)

// Set the eye position based off of where we are rendering the scene from
void AuGLEffects::SetEyePos(const AuPoint& pEyePos)
{
	m_EyePos = pEyePos;
}

GLuint AuGLEffects::HUDItemsTexture()
{
	if(!m_HUDItemsTexture)
		m_HUDItemsTexture = CreateTextureFromBitmap(STATIC_TEXTURE_ID, "HUDItems", ":data/HudItems.png");
	return m_HUDItemsTexture;
}

GLuint AuGLEffects::CreateTextureFromBitmap(unsigned int uiUserId, const QString &sTextureId, const QImage& bm,
		GLint magFilter, GLint minFilter, bool mipMap, int clampEdges)
{
	if (uiUserId == 0)
	{
		// If no userid or textureid is provided, just create the texture live.
		CreateTextureFromBitmap(bm, mipMap);
		return 0;
	}

	GLuint uiTex = UseTexture(uiUserId, sTextureId);
	if (uiTex)
	{
		//qDebug("AuGLEffects::CreateTextureFromBitmap - uid %d tid %s - Used "
				//"cached texture id %d", uiUserId, (const char*)sTextureId, uiTex);
		return uiTex;
	}

	AuGLTexture *tex = new AuGLTexture(uiUserId, sTextureId);
	if (tex)
	{
		tex->NameAndBind(*this, mipMap, magFilter, minFilter);

		if(clampEdges == 1 || clampEdges == 2)
		{
			// Clamp edges and blend to the final edge texel color.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		/*
		else if(clampEdges == 2)
		{
			// Clamp edges and blend to GL_TEXTURE_BORDER_COLOR, which should be the default 0,0,0,0 since I never change it.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		*/

		bool bTexOk = tex->CreateFromBitmap(*this, bm, mipMap);
		if (bTexOk)
		{
			uiTex = tex->GlName();
			m_muTextures.lock();
			m_Textures.append(tex);
			m_muTextures.unlock();
			//qDebug("AuGLEffects::CreateTextureFromBitmap - uid %d tid %s - Created "
					//"texture id %d", uiUserId, (const char*)sTextureId, uiTex);
		}
		else
		{
			delete tex;
		}
	}
	return uiTex;
}

GLuint AuGLEffects::CreateTextureFromBitmap(unsigned int uiUserId, const QString &sTextureId, const QString &sFileName,
		GLint magFilter, GLint minFilter, bool mipMap, int clampEdges)
{
	if (uiUserId != 0)
	{
		GLuint uiTex = UseTexture(uiUserId, sTextureId);
		if (uiTex)
		{
			//qDebug("AuGLEffects::CreateTextureFromBitmap - uid %d tid %s - Used "
					//"cached texture id %d", uiUserId, (const char*)sTextureId, uiTex);
			return uiTex;
		}
	}

	QImage imtex(sFileName);
	if(!imtex.isNull())
		return CreateTextureFromBitmap(uiUserId, sTextureId, imtex, magFilter, minFilter, mipMap, clampEdges);

	//qWarning("AuGLEffects::CreateTextureFromBitmap - Failed to load %s", (const char*)sFileName.toAscii());
	return 0;
}

// Create a texture map from an array of floating point triplets.
GLuint AuGLEffects::CreateTextureFromFloats(unsigned int uiUserId, const QString &sTextureId, const float* data, unsigned int width,
		unsigned int height, bool mipMap, int clampEdges, GLenum format)
{
	if(uiUserId == 0)
	{
		// If no userid or textureid is provided, just create the texture live.
		CreateTextureFromFloats(data, width, height);
		return 0;
	}

	GLuint uiTex = UseTexture(uiUserId, sTextureId);
	if(uiTex)
	{
		//qDebug("AuGLEffects::CreateTextureFromFloats - uid %d tid %s - Used "
				//"cached texture id %d", uiUserId, (const char*)sTextureId, uiTex);
		return uiTex;
	}

	AuGLTexture *tex = new AuGLTexture(uiUserId, sTextureId);
	if(tex)
	{
		tex->NameAndBind(*this, mipMap);

		if(clampEdges == 1 || clampEdges == 2)
		{
			// Clamp edges and blend to the final edge texel color.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		/*
		else if(clampEdges == 2)
		{
			// Clamp edges and blend to GL_TEXTURE_BORDER_COLOR, which should be the default 0,0,0,0 since I never change it.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		*/

		bool bTexOk = tex->CreateFromFloats(*this, data, width, height, mipMap, format);
		if(bTexOk)
		{
			uiTex = tex->GlName();
			m_muTextures.lock();
			m_Textures.append(tex);
			m_muTextures.unlock();
			//qDebug("AuGLEffects::CreateTextureFromFloats - uid %d tid %s - Created "
					//"texture id %d", uiUserId, (const char*)sTextureId, uiTex);
		}
		else
		{
			delete tex;
		}
	}
	return uiTex;
}

void AuGLEffects::DeleteTexture(unsigned int uiUserId, const QString &sTextureId)
{
	AuGLTexture *pTex = FindTexture(sTextureId);
	if (pTex && pTex->RemoveUser(uiUserId))
	{
		// The last user was removed from the texture object.
		// Delete the unused texture object.
		/*
		POSITION pos = Find(pTex);
		//Q_ASSERT(pos);
		if (pos)
			RemoveAt(pos);
		*/
		m_muTextures.lock();
		for(QVector<AuGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); it++)
		{
			if(*it == pTex)
			{
				m_Textures.erase(it);
				break;
			}
		}
		m_muTextures.unlock();
		delete pTex;
	}
}

// ALL textures used by 'uiUserId'.
void AuGLEffects::DeleteTextures(unsigned int uiUserId)
{
	m_muTextures.lock();
	QVector<AuGLTexture*>::iterator it = m_Textures.begin();
	while(it != m_Textures.end())
	{
		if((*it)->FindUser(uiUserId))
		{
			if((*it)->RemoveUser(uiUserId))
			{
				// The last user was removed from the texture object.
				// Delete the unused texture object.
				delete *it;
				it = m_Textures.erase(it);
			}
			else
				it++;
		}
		else
			it++;
	}
	m_muTextures.unlock();
}

void AuGLEffects::GetRenderTextItem(AuGLTextItem& textItem, const QString& str, const QFont& font, int flags, int wrapWidth)
{
	// See if we can locate the text item already created.
	QString strKey = QString("%1%2%3").arg(str).arg(flags).arg(wrapWidth);
	QMap<QFont, QMap<QString, AuGLTextItem> >::iterator fi = m_StringTextures.find(font);
	if(fi != m_StringTextures.end())
	{
		// Found text items that have already been created for the font.
		// Look for the specific text.
		QMap<QString, AuGLTextItem>::iterator ti = (*fi).find(strKey);
		if(ti != (*fi).end())
		{
			textItem = *ti;
			(*ti).m_LastUsed.restart();
		}
	}

	// Do we need to create the text item.
	if(!textItem.m_Texture)
	{
		// Measure the size of the text.
		QFontMetrics fm(font);
		QRect textrect = fm.boundingRect(0,0,wrapWidth,1, flags, str);
		if(textrect.height() && textrect.width())
		{
			// The text has a non-zero size.
			// Create an image of the text.
			textrect.setWidth(textrect.width()+2);				// Extra space is needed for wordwrap to work correctly. Qt may
			textrect.setHeight(textrect.height()+2);			// be allowing for a 1 pixel rectangle outline in drawText.
			QImage imtext(textrect.width(), textrect.height(), QImage::Format_ARGB32);
			imtext.fill(Qt::transparent);
			QPainter p(&imtext);
			p.setPen(QPen(Qt::white, 0.f));
			p.setFont(font);
			textrect.moveTo(-1,-1);								// Align the rectangle with the image.
			p.drawText(textrect, flags, str);
			p.end();

			// Create a texture of the text image.
			textItem.m_TexName = QString("%1%2%3%4%5%6%7")
				.arg(font.family())
				.arg(font.pointSize())
				.arg(font.bold())
				.arg(font.italic())
				.arg(str)
				.arg(flags)
				.arg(wrapWidth);
			textItem.m_Texture = CreateTextureFromBitmap(STATIC_TEXTURE_ID, textItem.m_TexName, imtext, GL_LINEAR, GL_LINEAR,
					false, 2);
			textItem.m_LastUsed.start();
			textItem.m_Height = textrect.height();
			textItem.m_Width = textrect.width();
			if(fi != m_StringTextures.end())
				(*fi).insert(strKey, textItem);
			else
			{
				QMap<QString, AuGLTextItem> tmap;
				tmap.insert(strKey, textItem);
				m_StringTextures.insert(font, tmap);
			}
		}
	}
}

void AuGLEffects::DecayRenderText()
{
	// While we're here, throw away text objects that have gone unused for too long.
	QMap<QFont, QMap<QString, AuGLTextItem> >::iterator fi = m_StringTextures.begin();
	while(fi != m_StringTextures.end())
	{
		// Delete any text items that have gotten too old.
		QMap<QString, AuGLTextItem>::iterator ti = (*fi).begin();
		while(ti != (*fi).end())
		{
			// Fade time for title messages in AuView is 15 seconds. We must wait at least that long.
			if((*ti).m_LastUsed.elapsed()/d_MSecPerSec > 20)
			{
				// Delete text item that has gone too long without being rendered.
				//qDebug() << "AuGLEffects::DecayRenderText discarding" << (*ti).m_TexName;
				DeleteTexture(STATIC_TEXTURE_ID, (*ti).m_TexName);
				ti = (*fi).erase(ti);
			}
			else
				ti++;
		}

		// Delete the map font entry if it has no more text items.
		if((*fi).isEmpty())
		{
			//qDebug() << "AuGLEffects::DecayRenderText font category decayed" << fi.key().family();
			fi = m_StringTextures.erase(fi);
		}
		else
			fi++;
	}
}

// Render text at a pixel position on the screen using OpenGL.
int AuGLEffects::RenderText(int x, int y, const QString& str, const QFont& font, const QColor& color, int viewX, int viewY,
		int flags, int wrapWidth)
{
	//qDebug() << "AuGLEffects::RenderText" << x << y << str;
	if(viewX <= 0 || viewY <= 0)
		return 0;

	// Get/create the render text item.
	AuGLTextItem textItem;
	GetRenderTextItem(textItem, str, font, flags, wrapWidth);
	if(!textItem.m_Texture)
		return 0;

	// Calculate position that was specified.
	float xpos = ((float)x / (float)viewX - .5f) * 2.f;		// -1 left to 1 right.
	float ypos = (.5f - (float)y / (float)viewY) * 2.f;		// -1 bottom to 1 top
	float xsz = (float)textItem.m_Width / (float)viewX * 2.f;
	float ysz = (float)textItem.m_Height / (float)viewY * 2.f;

	// Adjust the position according to alignment flags.
	// (xpos,ypos) specify the render location of the lower left corner of the text rectangle so they currently represents a
	// bottom-left alignment.
	if(flags&Qt::AlignRight)
		xpos -= xsz;
	else if(flags&Qt::AlignHCenter)
		xpos -= xsz/2;

	if(flags&Qt::AlignTop)
		ypos -= ysz;
	else if(flags&Qt::AlignVCenter)
		ypos -= ysz/2;

	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(xpos, ypos, 0, 0,0),
			AuPointTex(xpos, ypos + ysz, 0, 0,1),
			AuPointTex(xpos + xsz, ypos + ysz, 0, 1,1));
	tris << AuTriTex(AuPointTex(xpos + xsz, ypos + ysz, 0, 1,1),
			AuPointTex(xpos + xsz, ypos, 0, 1,0),
			AuPointTex(xpos, ypos, 0, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*this, tris, GL_STREAM_DRAW);
	if(!vao.VAO())
		return 0;

	// Change rendering states but keep old states so we can restore them on exit.
	GLboolean oldDepthTest;
	glGetBooleanv(GL_DEPTH_TEST, &oldDepthTest);
	glDisable(GL_DEPTH_TEST);								// Ignore current Z buffer.
	GLboolean oldDepthMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);
	glDepthMask(GL_FALSE);									// Do not modify Z buffer.

	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(AuMatrix4x4());
	GLSL().TextureShader().SetTexture(textItem.m_Texture);
	GLSL().TextureShader().SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());

	vao.SelectGL();
	vao.DrawGL();
	vao.DeselectGL();
	vao.DeleteGL();

	GLSL().TextureShader().Deselect();

	// Restore rendering states to the values they held on entry.
	if(oldDepthMask)
		glDepthMask(GL_TRUE);
	if(oldDepthTest)
		glEnable(GL_DEPTH_TEST);

	return textItem.m_Height;
}

void AuGLEffects::RenderText(const AuMatrix4x4& projection, const AuMatrix4x4& model, const AuPoint& ll, const AuPoint& ul,
		const AuPoint& ur, const AuPoint& lr, const QString& str, const QFont& font, const QColor& color)
{
	// Get/create the render text item.
	AuGLTextItem textItem;
	GetRenderTextItem(textItem, str, font, 0, 1);
	if(!textItem.m_Texture)
		return;

	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(ll.x, ll.y, ll.z, 0,0),
			AuPointTex(ul.x, ul.y, ul.z, 0,1),
			AuPointTex(ur.x, ur.y, ur.z, 1,1));
	tris << AuTriTex(AuPointTex(ur.x, ur.y, ur.z, 1,1),
			AuPointTex(lr.x, lr.y, lr.z, 1,0),
			AuPointTex(ll.x, ll.y, ll.z, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*this, tris, GL_STREAM_DRAW);
	if(!vao.VAO())
		return;

	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(projection * model);
	GLSL().TextureShader().SetTexture(textItem.m_Texture);
	GLSL().TextureShader().SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());

	vao.SelectGL();
	vao.DrawGL();
	vao.DeselectGL();
	vao.DeleteGL();

	GLSL().TextureShader().Deselect();
}

// Render text at a model space position on the screen using OpenGL.
void AuGLEffects::RenderText(const AuMatrix4x4& projection, const AuMatrix4x4& model, const AuPoint& loc, const QString& str,
		const QFont& font, const QColor& color, int viewY, int flags, int wrapWidth)
{
	//qDebug() << "AuGLEffects::RenderText" << x << y << z << str << "viewY" << viewY;
	if(viewY <= 0)
		return;

	// Get/create the render text item.
	AuGLTextItem textItem;
	GetRenderTextItem(textItem, str, font, flags, wrapWidth);
	if(!textItem.m_Texture)
		return;

	AuPoint tloc = model.Project(loc);

	// Render the text texture.
	float xsz = (float)textItem.m_Width / (float)viewY * -tloc.z;
	float ysz = (float)textItem.m_Height / (float)viewY * -tloc.z;

	// Adjust the position according to alignment flags.
	// (xpos,ypos) specify the render location of the lower left corner of the text rectangle so they currently represents a
	// bottom-left alignment.
	if(flags&Qt::AlignRight)
		tloc.x -= xsz;
	else if(flags&Qt::AlignHCenter)
		tloc.x -= xsz/2;

	if(flags&Qt::AlignTop)
		tloc.y -= ysz;
	else if(flags&Qt::AlignVCenter)
		tloc.y -= ysz/2;

	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(tloc.x, tloc.y, tloc.z, 0,0),
			AuPointTex(tloc.x, tloc.y + ysz, tloc.z, 0,1),
			AuPointTex(tloc.x + xsz, tloc.y + ysz, tloc.z, 1,1));
	tris << AuTriTex(AuPointTex(tloc.x + xsz, tloc.y + ysz, tloc.z, 1,1),
			AuPointTex(tloc.x + xsz, tloc.y, tloc.z, 1,0),
			AuPointTex(tloc.x, tloc.y, tloc.z, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*this, tris, GL_STREAM_DRAW);
	if(!vao.VAO())
		return;

	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(projection);
	GLSL().TextureShader().SetTexture(textItem.m_Texture);
	GLSL().TextureShader().SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());

	vao.SelectGL();
	vao.DrawGL();
	vao.DeselectGL();
	vao.DeleteGL();

	GLSL().TextureShader().Deselect();
}

// Render text at a model space position on the screen using OpenGL.
void AuGLEffects::RenderText_Shader(const AuMatrix4x4& projection, const AuMatrix4x4& model, const QString& str, const QFont& font,
		const QColor& color, int viewY, int flags, int wrapWidth)
{
	//qDebug() << "AuGLEffects::RenderText" << x << y << z << str << "viewY" << viewY;
	if(viewY <= 0)
		return;

	// Get/create the render text item.
	AuGLTextItem textItem;
	GetRenderTextItem(textItem, str, font, flags, wrapWidth);
	if(!textItem.m_Texture)
		return;

	// Render the text texture.
	float xsz = (float)textItem.m_Width / (float)viewY;
	float ysz = (float)textItem.m_Height / (float)viewY;

	// Adjust the position according to alignment flags.
	// (xpos,ypos) specify the render location of the lower left corner of the text rectangle so they currently represents a
	// bottom-left alignment.
	AuPoint tloc;
	if(flags&Qt::AlignRight)
		tloc.x -= xsz;
	else if(flags&Qt::AlignHCenter)
		tloc.x -= xsz/2;

	if(flags&Qt::AlignTop)
		tloc.y -= ysz;
	else if(flags&Qt::AlignVCenter)
		tloc.y -= ysz/2;

	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(tloc.x, tloc.y, tloc.z, 0,0),
			AuPointTex(tloc.x, tloc.y + ysz, tloc.z, 0,1),
			AuPointTex(tloc.x + xsz, tloc.y + ysz, tloc.z, 1,1));
	tris << AuTriTex(AuPointTex(tloc.x + xsz, tloc.y + ysz, tloc.z, 1,1),
			AuPointTex(tloc.x + xsz, tloc.y, tloc.z, 1,0),
			AuPointTex(tloc.x, tloc.y, tloc.z, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*this, tris, GL_STREAM_DRAW);
	if(!vao.VAO())
		return;

	//GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(projection * model);
	GLSL().TextureShader().SetTexture(textItem.m_Texture);
	GLSL().TextureShader().SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());

	vao.SelectGL();
	vao.DrawGL();
	vao.DeselectGL();
	vao.DeleteGL();

	//GLSL().TextureShader().Deselect();
}

void AuGLEffects::RenderLocation(const QRect& clientRect, float xpos, float ypos, float size, float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Create VAOs.
	if(!m_LocationDot.VAO())
	{
		float s = 0.19921875f;		// = 204.0f / himaxx;
		float t = 0.015625f;		// = 1.0f / himaxy;
		float u = 0.21240234375f;	// = 217.5f / himaxx;
		float v = 0.234375f;		// = 15.0f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-.01,-.01,0, s,t);
		tri0.m_V1 = AuPointTex(-.01,.01,0, s,v);
		tri0.m_V2 = AuPointTex(.01,.01,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(.01,.01,0, u,v);
		tri1.m_V1 = AuPointTex(.01,-.01,0, u,t);
		tri1.m_V2 = AuPointTex(-.01,-.01,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_LocationDot.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_LocationCircle.VAO())
	{
		float s = 0.39013671875f;	// = 399.5f / himaxx;
		float t = 0.0859375f;		// = 5.5f / himaxy;
		float u = 0.42138671875f;	// = 431.5f / himaxx;
		float v = 0.59375f;			// = 38.0f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-1,-1,0, s,t);
		tri0.m_V1 = AuPointTex(-1,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,-1,0, u,t);
		tri1.m_V2 = AuPointTex(-1,-1,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_LocationCircle.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	
	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	AuMatrix4x4 mv;
	mv.Translate(xpos, ypos, 0);

	// Select shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(projection * mv);
	GLSL().TextureShader().SetTexture(texHudItems);
	GLSL().TextureShader().SetColor(r, g, b, a);

	//determine circle size
	int dsteps = 8;
	int dstep = (AuTime::CurrentMSecsSinceEpoch()/100) % dsteps;
	float dfrac = (float)(dstep+1) / (float)dsteps;
	float circleSize = .005f + size*dfrac;

	//draw dot
	m_LocationDot.SelectGL();
	m_LocationDot.DrawGL();
	m_LocationDot.DeselectGL();

	//draw circle
	mv.Scale(circleSize, circleSize, 1);
	GLSL().TextureShader().SetProjectionModelView(projection * mv);

	m_LocationCircle.SelectGL();
	m_LocationCircle.DrawGL();
	m_LocationCircle.DeselectGL();

	GLSL().TextureShader().Deselect();
}

// 'justify' 0=left, 1=center, 2=right
float AuGLEffects::RenderNumber(const QRect& clientRect, int value, float left, float base, float ht, int justify, int sign,
		float r, float g, float b, float a)
{
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return 0;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Configure shader.
	GLSL().TexOffsetShader().Select();
	GLSL().TexOffsetShader().SetTexture(texHudItems);
	GLSL().TexOffsetShader().SetColor(r,g,b,a);
	GLSL().TexOffsetShader().SetDiscardAlpha(0);

	float width = RenderNumber2(projection, value, left, base, ht, justify, sign);

	GLSL().TexOffsetShader().Deselect();

	return width;
}

// NOTE: Rendering states must already be configured. Check actual usage of this function.
// 'justify' 0=left, 1=center, 2=right
float AuGLEffects::RenderNumber2(const AuMatrix4x4& projection, int value, float left, float base, float ht, int justify, int sign)
{
	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;			// width of the texture
	const float himaxy = 64;			// height of the texture

	// Create VAOs.
	if(!m_OffsetVAO.VAO())
	{
		// Shader offsets these texture coordinates using the offset calculated for the effect.
		const float s = .5f / himaxx;
		const float t = .5f / himaxy;
		const float u = (16.f - .5f) / himaxx;
		const float v = (16.f - .5f) / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_OffsetVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_NumberVAO.VAO())
	{
		// Shader offsets these texture coordinates using the offset calculated for the digit.
		// In this texture, each digit is 21 x 32 pixels with a 2 pixel margin between each digit. :|
		const float s = .5f / himaxx;
		const float t = .5f / himaxy;
		const float u = (21.f - .5f) / himaxx;
		const float v = (32.f - .5f) / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_NumberVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_NWVAO.VAO())
	{
		// Shader offsets these texture coordinates using the offset calculated for the effect.
		const float s = .5f / himaxx;
		const float t = 0 / himaxy;
		const float u = (6.f - .5f) / himaxx;	// 6 pixels wide
		const float v = (7.f - .5f) / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_NWVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_SEVAO.VAO())
	{
		// Shader offsets these texture coordinates using the offset calculated for the effect.
		const float s = .5f / himaxx;
		const float t = 0 / himaxy;
		const float u = (5.f - .5f) / himaxx;	// 5 pixels wide
		const float v = (7.f - .5f) / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_SEVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	//qDebug("value %d", value);
	// Altitude value.
	unsigned int avalue = abs(value);
	
	//qDebug("avalue %d", avalue);

	// The number of digits the value can reach.
	unsigned int digits;
	unsigned int magnitude;
	if(value)
	{
		// Determine how many digits the number has.
		digits = 10;
		magnitude = 1000000000;
		while(avalue < magnitude)
		{
			magnitude /= 10;
			digits--;
		}
	}
	else
	{
		// At least show a zero when the value is zero.
		magnitude = 1;
		digits = 1;
	}

	int signVisible = 0;
	if(sign || value < 0)
		signVisible = 1;	// sign

	float digitwid = ht * 21.f / 32.f;	// Based on texture aspect ratio.
	float fullwid = digitwid * (digits + signVisible);
	switch(justify)
	{
	case 0:
	default:
		break;
	case 1:
		left -= fullwid/2.f;
		break;
	case 2:
		left -= fullwid;
		break;
	}

	// Sign +/- precedes the number.
	if(sign <= 1 && signVisible)
	{
		float vs;
		float vt = 0.f;
		if(value < 0)
			vs = 160.f / himaxx;
		else
			vs = 176.f / himaxx;
		GLSL().TexOffsetShader().SetTextureOffset(vs, vt);

		AuMatrix4x4 mv;
		mv.Translate(left, base, 0);
		mv.Scale(digitwid, ht, 1);
		GLSL().TexOffsetShader().SetProjectionModelView(projection * mv);

		m_OffsetVAO.SelectGL();
		m_OffsetVAO.DrawGL();
		m_OffsetVAO.DeselectGL();

		left += digitwid;
	}

	while(magnitude)
	{
		int digval = avalue/magnitude; 
		avalue -= digval*magnitude;
		
		float vs = ((float)digval*23.f) / himaxx;	// Digits are spaced at 23 pixels though they are only 21 pixels wide.
		float vt = 16.f / himaxy;
		GLSL().TexOffsetShader().SetTextureOffset(vs, vt);

		AuMatrix4x4 mv;
		mv.Translate(left, base, 0);
		mv.Scale(digitwid, ht, 1);
		GLSL().TexOffsetShader().SetProjectionModelView(projection * mv);

		m_NumberVAO.SelectGL();
		m_NumberVAO.DrawGL();
		m_NumberVAO.DeselectGL();

		left += digitwid;
		digits--;
		magnitude /= 10;
	}

	// Sign NSEW follows the number.
	if(sign == 2)
	{
		// NS
		float vs;
		float vt = 0.f;
		if(value >= 0)
			vs = 47.f / himaxx;	// N
		else
			vs = 53.f / himaxx;	// S
		GLSL().TexOffsetShader().SetTextureOffset(vs, vt);

		AuMatrix4x4 mv;
		mv.Translate(left, base, 0);
		mv.Scale(digitwid, ht, 1);
		GLSL().TexOffsetShader().SetProjectionModelView(projection * mv);

		if(value >= 0)
		{
			m_NWVAO.SelectGL();
			m_NWVAO.DrawGL();
			m_NWVAO.DeselectGL();
		}
		else
		{
			m_SEVAO.SelectGL();
			m_SEVAO.DrawGL();
			m_SEVAO.DeselectGL();
		}

		left += digitwid;
	}
	else if(sign == 3)
	{
		// EW
		float vs;
		float vt = 0.f;
		if(value >= 0)
			vs = 58.f / himaxx;	// E
		else
			vs = 63.f / himaxx;	// W
		GLSL().TexOffsetShader().SetTextureOffset(vs, vt);

		AuMatrix4x4 mv;
		mv.Translate(left, base, 0);
		mv.Scale(digitwid, ht, 1);
		GLSL().TexOffsetShader().SetProjectionModelView(projection * mv);

		if(value >= 0)
		{
			m_SEVAO.SelectGL();
			m_SEVAO.DrawGL();
			m_SEVAO.DeselectGL();
		}
		else
		{
			m_NWVAO.SelectGL();
			m_NWVAO.DrawGL();
			m_NWVAO.DeselectGL();
		}

		left += digitwid;
	}
	
	return fullwid;
}

// This function sets up it's own shaders.
void AuGLEffects::RenderWeaponEffect(const QRect& clientRect, WEAPONEFFECT effect, float left, float bottom, float size,
		float r, float g, float b, float a)
{
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	GLSL().TexOffsetShader().Select();
	GLSL().TexOffsetShader().SetColor(r,g,b,a);
	GLSL().TexOffsetShader().SetTexture(texHudItems);
	GLSL().TexOffsetShader().SetDiscardAlpha(0);

	RenderWeaponEffect(projection, effect, left, bottom, size);

	GLSL().TexOffsetShader().Deselect();
}

void AuGLEffects::RenderWeaponMode(const QRect& clientRect, WEAPONMODE mode, float left, float bottom, float size,
		float r, float g, float b, float a)
{
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	GLSL().TexOffsetShader().Select();
	GLSL().TexOffsetShader().SetColor(r,g,b,a);
	GLSL().TexOffsetShader().SetTexture(texHudItems);
	GLSL().TexOffsetShader().SetDiscardAlpha(0);

	RenderWeaponMode(projection, mode, left, bottom, size);

	GLSL().TexOffsetShader().Deselect();
}

// This function sets up it's own shaders.
void AuGLEffects::RenderItemEffect(const QRect& clientRect, ITEMEFFECT effect, float left, float bottom, float size,
		float r, float g, float b, float a)
{
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	GLSL().TexOffsetShader().Select();
	GLSL().TexOffsetShader().SetColor(r,g,b,a);
	GLSL().TexOffsetShader().SetTexture(texHudItems);
	GLSL().TexOffsetShader().SetDiscardAlpha(0);

	RenderItemEffect(projection, effect, left, bottom, size);

	GLSL().TexOffsetShader().Deselect();
}

// NOTE: Rendering states must already be configured. Check actual usage of this function.
void AuGLEffects::RenderItemEffect(const AuMatrix4x4& projection, ITEMEFFECT effect, float left, float bottom, float size)
{
	// Texture offsets into HudItems.png for various item effects.
	// Y offset is measured up from the bottom of the texture.
	QPointF texOffsets[] =
	{
		QPointF(0*16, 3*16),			// IE_Acid,
		QPointF(1*16, 3*16),			// IE_Suffocate,
		QPointF(2*16, 3*16),			// IE_Stun,
		QPointF(3*16, 3*16),			// IE_Poison,
		QPointF(4*16, 3*16),			// IE_Radiation,
		QPointF(5*16, 3*16),			// IE_Disease,
		QPointF(6*16, 3*16),			// IE_Electric,
		QPointF(7*16, 3*16),			// IE_Bludgeon,
		QPointF(8*16, 3*16),			// IE_Cold,
		QPointF(9*16, 3*16),			// IE_Crush,
		QPointF(10*16, 3*16),			// IE_Sonic,
		QPointF(11*16, 3*16),			// IE_Heat,
		QPointF(12*16, 3*16),			// IE_Pierce,
		QPointF(13*16, 3*16),			// IE_Implant,
		QPointF(14*16, 3*16),			// IE_Slash,
		QPointF(15*16, 3*16),			// IE_Food,
		QPointF(16*16, 3*16),			// IE_Liquor,
		QPointF(17*16, 3*16),			// IE_Revive,
		QPointF(18*16, 3*16),			// IE_CureDisease,
		QPointF(19*16, 3*16),			// IE_CurePoison,
		QPointF(20*16, 3*16),			// IE_Heal,
		QPointF(26*16, 3*16),			// IE_RepairMetal,
		QPointF(34*16, 1*16),			// IE_RepairMagmium,
		QPointF(34*16, 0*16),			// IE_RepairVulcium,
		QPointF(21*16, 3*16),			// IE_RepairAdamantine,
		QPointF(22*16, 3*16),			// IE_RepairModule,
		QPointF(32*16, 2*16),			// IE_RepairElectronicPart,
		QPointF(32*16, 1*16),			// IE_RepairMechanicalPart,
		QPointF(29*16, 3*16),			// IE_Butcher,
		QPointF(31*16, 3*16),			// IE_Dig,
		QPointF(33*16, 2*16),			// IE_Light,
		QPointF(34*16, 2*16),			// IE_Visor,	// helmet opens and closes
		QPointF(29*16, 2*16),			// IE_UpgradeModule,				// Item upgrades a system of a spacecraft.
		QPointF(29*16, 1*16),			// IE_SampleDNA,
		QPointF(29*16, 0*16),			// IE_PowerPlantOn,
		QPointF(56*16, 1*16),			// IE_ReticleBox,
		QPointF(56*16, 0*16),			// IE_ReticleDiamond,
		QPointF(57*16, 1*16),			// IE_PoliticalStance,
		QPointF(48*16, 1*16),			// IE_System_CommandAndControl
		QPointF(49*16, 1*16),			// IE_System_Fuel
		QPointF(50*16, 1*16),			// IE_System_ManeuverDrive
		QPointF(51*16, 1*16),			// IE_System_Hold
		QPointF(52*16, 1*16),			// IE_System_Hull
		QPointF(53*16, 1*16),			// IE_System_VehicleLaunch
		QPointF(54*16, 1*16),			// IE_System_LifeSupport
		QPointF(55*16, 1*16),			// IE_System_PowerPlant
		QPointF(48*16, 0*16),			// IE_System_Sensor
		QPointF(49*16, 0*16),			// IE_System_Shield
		QPointF(50*16, 0*16),			// IE_System_SurgeryUnit
		QPointF(51*16, 0*16),			// IE_System_Transporter
		QPointF(52*16, 0*16),			// IE_System_Turret
		QPointF(53*16, 0*16),			// IE_System_Weapon
		QPointF(54*16, 0*16),			// IE_System_FTLWormholeDrive
		QPointF(28*16, 0*16),			// IE_System_Capacitor
		QPointF(33*16, 1*16),			// IE_System_VehicleLanding
		QPointF(55*16, 0*16),			// IE_System_Door
		QPointF(28*16, 1*16),			// IE_System_FTLWarpDrive
		QPointF(27*16, 0*16),			// IE_System_Berth
		QPointF(58*16, 0*16),			// IE_SpecWeaponNotReady
		QPointF(61*16, 1*16),			// IE_WM_Area
		QPointF(61*16, 0*16),			// IE_WM_Contact
		QPointF(62*16, 1*16),			// IE_WM_Detonator
		QPointF(63*16, 1*16),			// IE_WM_Fuse
		QPointF(62*16, 0*16),			// IE_WM_Projectile
		QPointF(63*16, 0*16),			// IE_WM_Psionic
		QPointF(57*16, 0*16),			// IE_Decontaminate,
		QPointF(27*16, 1*16),			// IE_MotherShipReticle,
		QPointF(203, 0),				// IE_StallWarning,
		QPointF(23*16, 3*16),			// IE_DissassembleModule,
	};

	// Calculate texture coordinates in HudItems.png.
	const float himaxx = 1024;			// width of the texture
	const float himaxy = 64;			// height of the texture
	QPointF tOff(texOffsets[effect]);
	float vs = tOff.x() / himaxx;
	float vt = tOff.y() / himaxy;
	GLSL().TexOffsetShader().SetTextureOffset(vs, vt);

	// Create VAOs.
	if(!m_OffsetVAO.VAO())
	{
		// Shader offsets these texture coordinates using the offset calculated for the effect.
		const float s = .5f / himaxx;
		const float t = .5f / himaxy;
		const float u = (16.f - .5f) / himaxx;
		const float v = (16.f - .5f) / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_OffsetVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 mve;
	mve.Translate(left, bottom, 0);
	mve.Scale(size, size, 1);
	GLSL().TexOffsetShader().SetProjectionModelView(projection * mve);

	m_OffsetVAO.SelectGL();
	m_OffsetVAO.DrawGL();
	m_OffsetVAO.DeselectGL();
}

// NOTE: Rendering states must already be configured. Check actual usage of this function.
void AuGLEffects::RenderSprite(const QRect& clientRect, float xpos, float ypos, float sizeX, float sizeY, GLuint texture,
		float r, float g, float b, float a)
{
	// Create VAOs.
	if(!m_SpriteVAO.VAO())
	{
		// Shader offsets these texture coordinates using the offset calculated for the effect.
		const float s = 0;
		const float t = 0;
		const float u = 1;
		const float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-.5,-.5,0, s,t);
		tri0.m_V1 = AuPointTex(-.5,.5,0, s,v);
		tri0.m_V2 = AuPointTex(.5,.5,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(.5,.5,0, u,v);
		tri1.m_V1 = AuPointTex(.5,-.5,0, u,t);
		tri1.m_V2 = AuPointTex(-.5,-.5,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_SpriteVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	AuMatrix4x4 mv;
	mv.Translate(xpos, ypos, 0);
	mv.Scale(sizeX, sizeY, 1);

	if(sizeX < 0 || sizeY < 0)
		glDisable(GL_CULL_FACE);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(projection * mv);
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texture);

	m_SpriteVAO.SelectGL();
	m_SpriteVAO.DrawGL();
	m_SpriteVAO.DeselectGL();

	GLSL().TextureShader().Deselect();

	if(sizeX < 0 || sizeY < 0)
		glEnable(GL_CULL_FACE);
}

// xpos, ypos are screen coords in [-1..1] ranges (0,0) = center of screen
void AuGLEffects::RenderAircraftHud(const QRect& clientRect, float xpos, float ypos, float alt, float pitch, float roll,
		float heading, float speed, float vspeed, float lspeed, float longitude, float latitude, int hudIntensity, float hullStress,
		int throttle, int etesecs, bool inEnemyRange, bool isHolding)
{
	// Nothing to do when hud is invisible.
	if(!hudIntensity)
		return;

	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	// Create VAOs.
	if(!m_HeadingVAO.VAO())
	{
		// Texture coordinates of full heading tape in texHudItems.
		const float s = 273.f / himaxx;
		const float t = 38.5f / himaxy;
		const float u = 453. / himaxx;
		const float v = 48.5f / himaxy;

		// Create a cylinder on which to map our heading texture.
		// The height is 1 and the diameter is 1 so it can be scaled easily to the desired size.
		QList<AuTriTex> tris;
		AuTriTex tri0;
		AuTriTex tri1;
		AuPointTex pvb(.5,0,0, s,t);
		AuPointTex pvt(.5,0,1, s,v);
		for(int facet=1; facet<=32; facet++)
		{
			float ns;											// Texture s coordinate at next vertices.
			AuPoint vb, vt;										// Vertex locations at next cylinder facet edge.
			if(facet == 32)
			{
				ns = u;
				vb = AuPoint(.5,0,0);
				vt = AuPoint(.5,0,1);
			}
			else
			{
				float frac = (float)facet / 32.f;					// Fractional distance around cylinder.
				float nextang = frac * (PIf*2.f);					// Angle at next vertices.
				ns = s + frac * (u - s);
				vb = AuPoint(.5,0,0).RotateZ(nextang);
				vt = AuPoint(.5,0,1).RotateZ(nextang);
			}

			// Build the next two vertices in the cylinder.
			AuPointTex nvb(vb, ns,t);
			AuPointTex nvt(vt, ns,v);

			// Add two triangle to make the face of the cylinder, clockwise from the outside facing in.
			tri0.m_V0 = pvb;
			tri0.m_V1 = pvt;
			tri0.m_V2 = nvt;
			tri1.m_V0 = nvt;
			tri1.m_V1 = nvb;
			tri1.m_V2 = pvb;
			tris << tri0 << tri1;

			pvb = nvb;
			pvt = nvt;
		}

		m_HeadingVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	// Hud size
	const float wid = .5f;		// All hud sizes ultimately driven by this value
	const float wid2 = wid / 2.f;
	const float ht = wid;
	const float ht2 = ht / 2.f;
	if(!m_HUDBaseVAO.VAO())
	{
		// Texture coordinates of hud base in texHudItems.
		float s = 576.f / himaxx;
		float t = 4.f / himaxy;
		float u = 640.f / himaxx;
		float v = 63.f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-wid2,-ht2,0, s,t);
		tri0.m_V1 = AuPointTex(-wid2,ht2,0, s,v);
		tri0.m_V2 = AuPointTex(wid2,ht2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(wid2,ht2,0, u,v);
		tri1.m_V1 = AuPointTex(wid2,-ht2,0, u,t);
		tri1.m_V2 = AuPointTex(-wid2,-ht2,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		// Base of hull stress indicator.
		{
			// Texture coordinates of stress base in texHudItems.
			float s = 69.5f / himaxx;
			float t = .5f / himaxy;
			float u = 99.5 / himaxx;
			float v = 6.5f / himaxy;
			float swid = wid2/10;
			float sx = -wid2 - swid*2;
			float sy = swid + .025f;
			float sy2 = ht2 + swid;

			AuTriTex tri0;
			tri0.m_V0 = AuPointTex(sx+swid,sy,0, s,t);
			tri0.m_V1 = AuPointTex(sx,sy,0, s,v);
			tri0.m_V2 = AuPointTex(sx,sy2,0, u,v);
			AuTriTex tri1;
			tri1.m_V0 = AuPointTex(sx,sy2,0, u,v);
			tri1.m_V1 = AuPointTex(sx+swid,sy2,0, u,t);
			tri1.m_V2 = AuPointTex(sx+swid,sy,0, s,t);
			tris << tri0 << tri1;
		}

		m_HUDBaseVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_HUDLadderVAO.VAO())
	{
		// Texture coordinates of hud lines in texHudItems.
		float s = 640.5f / himaxx;
		float t = 4.f / himaxy;
		float u = 703.5 / himaxx;
		float v = 63.f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-wid2,-ht2,0, s,t);
		tri0.m_V1 = AuPointTex(-wid2,ht2,0, s,v);
		tri0.m_V2 = AuPointTex(wid2,ht2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(wid2,ht2,0, u,v);
		tri1.m_V1 = AuPointTex(wid2,-ht2,0, u,t);
		tri1.m_V2 = AuPointTex(-wid2,-ht2,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_HUDLadderVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_HUDStressVAO.VAO())
	{
		// Texture coordinates of indicator in texHudItems.
		float s = .5f / himaxx;
		float t = 2.f / himaxy;
		float u = 47.5 / himaxx;
		float v = 5.f / himaxy;
		float swid = wid2/10;
		float sx = -wid2 - swid*2;
		float sy = swid + .025f;
		float sy2 = ht2 + swid;
		float viewHt = sy2 - sy;	// Total height, including margins.
		float baseMarg = viewHt * 2.f/29.f;	// Graph starts 2 pixels from origin
		float iy = sy + baseMarg;
		float iyb = iy - swid/3;
		float iyt = iy + swid/3;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(sx,iyb,0, s,t);
		tri0.m_V1 = AuPointTex(sx,iyt,0, s,v);
		tri0.m_V2 = AuPointTex(sx+swid,iyt,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(sx+swid,iyt,0, u,v);
		tri1.m_V1 = AuPointTex(sx+swid,iyb,0, u,t);
		tri1.m_V2 = AuPointTex(sx,iyb,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_HUDStressVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_HUDThrottleVAO.VAO())
	{
		// Texture coordinates of throttle setting indicator in texHudItems.
		float s = 771.5f / himaxx;
		float t = 41.f / himaxy;
		float u = s + 22.f / himaxx;
		float v = 64.f / himaxy;

		// This indicator is centered at zero so it can be rotated for negative throttle.
		float tht = wid2*3/4;
		float twid = tht/3;
		tht /= 2;
		twid /= 2;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-twid,-tht,0, s,t);
		tri0.m_V1 = AuPointTex(-twid,tht,0, s,v);
		tri0.m_V2 = AuPointTex(twid,tht,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(twid,tht,0, u,v);
		tri1.m_V1 = AuPointTex(twid,-tht,0, u,t);
		tri1.m_V2 = AuPointTex(-twid,-tht,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_HUDThrottleVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_HUDEnemyVAO.VAO())
	{
		// Texture coords of a circle.
		// For himaxxx = 1024 and himaxy = 64
		float s = 0.19921875;		// = 204.0f / himaxx;
		float t = 0.015625;			// = 1.0f / himaxy;
		float u = 0.21240234375;	// = 217.5f / himaxx;
		float v = 0.234375;			// = 15.0f / himaxy;

		float tht = wid2*3/16;
		float twid = tht;
		float sx = -twid/2;
		float sy = ht2 + tht;
		float sy2 = sy + tht;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(sx,sy,0, s,t);
		tri0.m_V1 = AuPointTex(sx,sy2,0, s,v);
		tri0.m_V2 = AuPointTex(sx+twid,sy2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(sx+twid,sy2,0, u,v);
		tri1.m_V1 = AuPointTex(sx+twid,sy,0, u,t);
		tri1.m_V2 = AuPointTex(sx,sy,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_HUDEnemyVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// HUD colors
	float r = 213.f/255.f;
	float g = 246.f/255.f;
	float b = 213.f/255.f;
	float a = (float)hudIntensity / 10.f;

	// Heading tape
	const float tht = .06f;
	const float ttop = ypos - ht2 - .01f;
	const float tbot = ttop - tht * .3f;	// Bottom not quite dropped by full ht because the heading tape is a tilted cylinder

	// HUD base, ladder, and heading.
	{
		// Configure shader.
		GLSL().TextureShader().Select();
		GLSL().TextureShader().SetColor(r,g,b,a);
		GLSL().TextureShader().SetTexture(texHudItems);

		// HUD Base
		AuMatrix4x4 mvbase;
		mvbase.Translate(xpos, ypos, 0);
		GLSL().TextureShader().SetProjectionModelView(projection * mvbase);

		m_HUDBaseVAO.SelectGL();
		m_HUDBaseVAO.DrawGL();
		m_HUDBaseVAO.DeselectGL();

		if(inEnemyRange)
		{
			float er = 255.f/255.f;
			float eg = 32.f/255.f;
			float eb = 32.f/255.f;
			GLSL().TextureShader().SetColor(er,eg,eb,a);

			m_HUDEnemyVAO.SelectGL();
			m_HUDEnemyVAO.DrawGL();
			m_HUDEnemyVAO.DeselectGL();

			GLSL().TextureShader().SetColor(r,g,b,a);
		}

		// Hold indicator
		if(isHolding)
		{
			if(!m_HUDHoldVAO.VAO())
			{
				// Texture coords of a dot.
				float s = 605.5f / himaxx;
				float v = 3.5f / himaxy;
				float u = 610.5f / himaxx;
				float t = 8.5f / himaxy;

				float twid = wid2*5/16;
				float tht = twid;
				float sx = wid2 - twid*3/4;
				float sy = -tht/2;
				float sx2 = sx + twid;
				float sy2 = sy + tht;

				AuTriTex tri0;
				tri0.m_V0 = AuPointTex(sx,sy,0, s,t);
				tri0.m_V1 = AuPointTex(sx,sy2,0, s,v);
				tri0.m_V2 = AuPointTex(sx2,sy2,0, u,v);
				AuTriTex tri1;
				tri1.m_V0 = AuPointTex(sx2,sy2,0, u,v);
				tri1.m_V1 = AuPointTex(sx2,sy,0, u,t);
				tri1.m_V2 = AuPointTex(sx,sy,0, s,t);
				QList<AuTriTex> tris;
				tris << tri0 << tri1;

				m_HUDHoldVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
			}

			m_HUDHoldVAO.SelectGL();
			m_HUDHoldVAO.DrawGL();
			m_HUDHoldVAO.DeselectGL();
		}

		// HUD Ladder
		{
			AuMatrix4x4 mvladder;
			mvladder.Translate(xpos, ypos, 0);
			mvladder.RotateZ(-roll);
			mvladder.Translate(0, sinf(pitch)*wid2, 0);
			GLSL().TextureShader().SetProjectionModelView(projection * mvladder);

			m_HUDLadderVAO.SelectGL();
			m_HUDLadderVAO.DrawGL();
			m_HUDLadderVAO.DeselectGL();
		}

		// HUD Heading
		{
			AuMatrix4x4 mvheading;
			mvheading.Translate(xpos, tbot, 0);
			mvheading.RotateX(-(80. * DEG2RAD));
			mvheading.RotateZ(-heading-PIf/2.f);
			mvheading.Scale(wid, wid, tht);
			GLSL().TextureShader().SetProjectionModelView(projection * mvheading);

			m_HeadingVAO.SelectGL();
			m_HeadingVAO.DrawGL();
			m_HeadingVAO.DeselectGL();
		}

		// HULL Hull Stress
		if(hullStress >= 0)
		{
			// 29 total pixels in texture
			// 24 pixels center-to-center from 0 to 1.2
			// first graph pixel is 2 pixels from origin
			float swid = wid2/10;
			float sy = swid + .025f;
			float sy2 = ht2 + swid;
			float viewHt = sy2 - sy;	// Total height, including margins.
			float graphHt = viewHt - 5.f/29.f * viewHt;	// Excluding margins.
			if(hullStress > 1.2f)
				hullStress = 1.2f;
			float iy = graphHt * hullStress/1.2f;
			mvbase.Translate(0, iy, 0);
			GLSL().TextureShader().SetProjectionModelView(projection * mvbase);

			m_HUDStressVAO.SelectGL();
			m_HUDStressVAO.DrawGL();
			m_HUDStressVAO.DeselectGL();
		}

		GLSL().TextureShader().Deselect();
	}

	// HUD throttle
	{
		float tht = (wid2*3/4);
		float twid = (tht/3);
		float sx = xpos + wid2 + .02f + twid/2;
		float sy2 = ypos - twid;
		float sy = sy2 - tht/2;

		int absThrottle = abs(throttle);
		float vs = (absThrottle * 23) / himaxx;
		float vt = 0;

		// Configure shader.
		GLSL().TexOffsetShader().Select();
		GLSL().TexOffsetShader().SetColor(r,g,b,a);
		GLSL().TexOffsetShader().SetTexture(texHudItems);
		GLSL().TexOffsetShader().SetDiscardAlpha(0);

		AuMatrix4x4 mvthrottle;
		mvthrottle.Translate(sx, sy, 0);
		if(throttle < 0)
			mvthrottle.RotateZ(PI);
		GLSL().TexOffsetShader().SetProjectionModelView(projection * mvthrottle);

		GLSL().TexOffsetShader().SetTextureOffset(vs, vt);

		m_HUDThrottleVAO.SelectGL();
		m_HUDThrottleVAO.DrawGL();
		m_HUDThrottleVAO.DeselectGL();

		GLSL().TexOffsetShader().Deselect();
	}

	// Overall speed converted to meters per second.
	int ispeed = (int)ceil(speed);
	RenderNumber(clientRect, ispeed, xpos-wid2-.01f,ypos-.015f,.03f, 2, 0, r,g,b,a);

	// Lateral speed below overall speed.
	if(lspeed)
	{
		int ilspeed = (int)ceil(lspeed);
		RenderNumber(clientRect, ilspeed, xpos-wid2-.02f,ypos-.05f,.025f, 2, 1, r,g,b,a);
	}

	// Altimeter in meters
	int ialt = (int)ceil(alt);
	RenderNumber(clientRect, ialt, xpos+wid2+.01f,ypos-.015f,.03f, 0, 0, r,g,b,a);

	// Vertical speed below altimeter.
	if(vspeed)
	{
		int ivspeed = (int)ceil(vspeed);
		RenderNumber(clientRect, ivspeed, xpos+wid2+.02f,ypos-.05f,.025f, 0, 1, r,g,b,a);
	}

	// Longitude and latitude
	const float lht = .025f;
	const float llbot = tbot - lht*3.f - .01f;
	if(longitude || latitude)
	{
		float npos = xpos - lht - .015f;

		// Longitude
		RenderNumber(clientRect, round(longitude*RAD2DEGf), npos,llbot,lht, 2, 3, r,g,b,a);
		npos += lht + .02f;

		// Latitude
		RenderNumber(clientRect, round(latitude*RAD2DEGf), npos,llbot,lht, 0, 2, r,g,b,a);
	}

	// ete
	if(etesecs != 0)
	{
		// show ete above ladder
		const float etebot = ypos + ht2 + .01f;

		// Break down the ete into hours minutes seconds.
		int hours = etesecs/(60*60);
		etesecs -= hours*(60*60);
		int mins = etesecs/60;
		etesecs -= mins*60;
		int secs = etesecs;

		// Hours
		float npos;
		if(hours)
		{
			if(hours >= 100)
				npos = xpos - lht*2.9;
			else if(hours >= 10)
				npos = xpos - lht*2.6;
			else
				npos = xpos - lht*2.3;
			float wid = RenderNumber(clientRect, hours, npos,etebot,lht, 0, false, r,g,b,a);
			npos += wid + lht/2;
		}
		else
			npos = xpos - lht*1.6;

		// Minutes
		if(mins >= 10)
		{
			float wid = RenderNumber(clientRect, mins, npos,etebot,lht, 0, false, r,g,b,a);
			npos += wid + lht/2;
		}
		else
		{
			// Leading zero if there was an hours number.
			if(hours)
			{
				float wid = RenderNumber(clientRect, 0, npos,etebot,lht, 0, false, r,g,b,a);
				npos += wid;
			}
			float wid = RenderNumber(clientRect, mins, npos,etebot,lht, 0, false, r,g,b,a);
			npos += wid + lht/2;
		}

		// Seconds
		if(secs >= 10)
		{
			float wid = RenderNumber(clientRect, secs, npos,etebot,lht, 0, false, r,g,b,a);
			npos += wid + lht/2;
		}
		else
		{
			// Leading zero.
			float wid = RenderNumber(clientRect, 0, npos,etebot,lht, 0, false, r,g,b,a);
			npos += wid;
			wid = RenderNumber(clientRect, secs, npos,etebot,lht, 0, false, r,g,b,a);
			npos += wid + lht/2;
		}
	}
}

// xpos, ypos are screen coords in [-1..1] ranges (0,0) = center of screen
void AuGLEffects::RenderAPCHud(const QRect& clientRect, float xpos, float ypos, float heading, float longitude, float latitude,
		int hudIntensity, bool inEnemyRange)
{
	// Nothing to do when hud is invisible.
	if(!hudIntensity)
		return;

	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	// Create VAOs.
	if(!m_HeadingVAO.VAO())
	{
		// Texture coordinates of full heading tape in texHudItems.
		const float s = 273.f / himaxx;
		const float t = 38.5f / himaxy;
		const float u = 453. / himaxx;
		const float v = 48.5f / himaxy;

		// Create a cylinder on which to map our heading texture.
		// The height is 1 and the diameter is 1 so it can be scaled easily to the desired size.
		QList<AuTriTex> tris;
		AuTriTex tri0;
		AuTriTex tri1;
		AuPointTex pvb(.5,0,0, s,t);
		AuPointTex pvt(.5,0,1, s,v);
		for(int facet=1; facet<=32; facet++)
		{
			float ns;											// Texture s coordinate at next vertices.
			AuPoint vb, vt;										// Vertex locations at next cylinder facet edge.
			if(facet == 32)
			{
				ns = u;
				vb = AuPoint(.5,0,0);
				vt = AuPoint(.5,0,1);
			}
			else
			{
				float frac = (float)facet / 32.f;					// Fractional distance around cylinder.
				float nextang = frac * (PIf*2.f);					// Angle at next vertices.
				ns = s + frac * (u - s);
				vb = AuPoint(.5,0,0).RotateZ(nextang);
				vt = AuPoint(.5,0,1).RotateZ(nextang);
			}

			// Build the next two vertices in the cylinder.
			AuPointTex nvb(vb, ns,t);
			AuPointTex nvt(vt, ns,v);

			// Add two triangle to make the face of the cylinder, clockwise from the outside facing in.
			tri0.m_V0 = pvb;
			tri0.m_V1 = pvt;
			tri0.m_V2 = nvt;
			tri1.m_V0 = nvt;
			tri1.m_V1 = nvb;
			tri1.m_V2 = pvb;
			tris << tri0 << tri1;

			pvb = nvb;
			pvt = nvt;
		}

		m_HeadingVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	// Hud size
	const float wid = .5f;		// All hud sizes ultimately driven by this value
	const float wid2 = wid / 2.f;
	const float ht = wid;
	const float ht2 = ht / 2.f;
	if(!m_HUDGunVAO.VAO())
	{
		// Texture coordinates of hud base in texHudItems.
		// These coordinates exclude the vertical ladder bars at the sides.
		float s = 582.f / himaxx;
		float t = 4.f / himaxy;
		float u = 634.f / himaxx;
		float v = 63.f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-wid2,-ht2,0, s,t);
		tri0.m_V1 = AuPointTex(-wid2,ht2,0, s,v);
		tri0.m_V2 = AuPointTex(wid2,ht2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(wid2,ht2,0, u,v);
		tri1.m_V1 = AuPointTex(wid2,-ht2,0, u,t);
		tri1.m_V2 = AuPointTex(-wid2,-ht2,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_HUDGunVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_HUDEnemyVAO.VAO())
	{
		// Texture coords of a circle.
		// For himaxxx = 1024 and himaxy = 64
		float s = 0.19921875;		// = 204.0f / himaxx;
		float t = 0.015625;			// = 1.0f / himaxy;
		float u = 0.21240234375;	// = 217.5f / himaxx;
		float v = 0.234375;			// = 15.0f / himaxy;

		float tht = wid2*3/16;
		float twid = tht;
		float sx = -twid/2;
		float sy = ht2 + tht;
		float sy2 = sy + tht;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(sx,sy,0, s,t);
		tri0.m_V1 = AuPointTex(sx,sy2,0, s,v);
		tri0.m_V2 = AuPointTex(sx+twid,sy2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(sx+twid,sy2,0, u,v);
		tri1.m_V1 = AuPointTex(sx+twid,sy,0, u,t);
		tri1.m_V2 = AuPointTex(sx,sy,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_HUDEnemyVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// HUD colors
	float r = 213.f/255.f;
	float g = 246.f/255.f;
	float b = 213.f/255.f;
	float a = (float)hudIntensity / 10.f;

	// Heading tape
	const float tht = .06f;
	const float ttop = ypos - ht2 - .01f;
	const float tbot = ttop - tht * .3f;	// Bottom not quite dropped by full ht because the heading tape is a tilted cylinder

	// HUD base, ladder, and heading.
	{
		// Configure shader.
		GLSL().TextureShader().Select();
		GLSL().TextureShader().SetColor(r,g,b,a);
		GLSL().TextureShader().SetTexture(texHudItems);

		// HUD Base
		AuMatrix4x4 mvbase;
		mvbase.Translate(xpos, ypos, 0);
		GLSL().TextureShader().SetProjectionModelView(projection * mvbase);

		m_HUDGunVAO.SelectGL();
		m_HUDGunVAO.DrawGL();
		m_HUDGunVAO.DeselectGL();

		if(inEnemyRange)
		{
			float er = 255.f/255.f;
			float eg = 32.f/255.f;
			float eb = 32.f/255.f;
			GLSL().TextureShader().SetColor(er,eg,eb,a);

			m_HUDEnemyVAO.SelectGL();
			m_HUDEnemyVAO.DrawGL();
			m_HUDEnemyVAO.DeselectGL();

			GLSL().TextureShader().SetColor(r,g,b,a);
		}

		// HUD Heading
		{
			AuMatrix4x4 mvheading;
			mvheading.Translate(xpos, tbot, 0);
			mvheading.RotateX(-(80. * DEG2RAD));
			mvheading.RotateZ(-heading-PIf/2.f);
			mvheading.Scale(wid, wid, tht);
			GLSL().TextureShader().SetProjectionModelView(projection * mvheading);

			m_HeadingVAO.SelectGL();
			m_HeadingVAO.DrawGL();
			m_HeadingVAO.DeselectGL();
		}

		GLSL().TextureShader().Deselect();
	}

	// Longitude and latitude
	const float lht = .025f;
	const float llbot = tbot - lht*3.f - .01f;
	if(longitude || latitude)
	{
		float npos = xpos - lht - .015f;

		// Longitude
		RenderNumber(clientRect, round(longitude*RAD2DEGf), npos,llbot,lht, 2, 3, r,g,b,a);
		npos += lht + .02f;

		// Latitude
		RenderNumber(clientRect, round(latitude*RAD2DEGf), npos,llbot,lht, 0, 2, r,g,b,a);
	}
}

// 'sizeWid' is based on sail setting/throttle, can be negative for left facing sail.
// This function sets up it's own shaders.
void AuGLEffects::RenderSail(const QRect& clientRect, float xpos, float ypos, float sizeWid, float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	if(!m_SailVAO.VAO())
	{
		// Texture coordinates of sail in s_texHudItems.
		float s = 480.5f / himaxx;
		float t = .5f / himaxy;
		float u = 511.5 / himaxx;
		float v = 47.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_SailVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texHudItems);

	// Sail.
	AuMatrix4x4 mv;
	mv.Translate(xpos, ypos, 0);
	mv.Scale(sizeWid, fabs(sizeWid*2), 0);
	GLSL().TextureShader().SetProjectionModelView(projection * mv);

	if(sizeWid < 0)
		glDisable(GL_CULL_FACE);

	m_SailVAO.SelectGL();
	m_SailVAO.DrawGL();
	m_SailVAO.DeselectGL();

	if(sizeWid < 0)
		glEnable(GL_CULL_FACE);

	GLSL().TextureShader().Deselect();
}

void AuGLEffects::RenderSpeedometer(const QRect& clientRect, float xpos, float ypos, float size, float speedMph,
		float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	// Gauge size
	const float wid2 = size / 2.f;
	const float ht = size;

	if(!m_SpeedoGaugeVAO.VAO())
	{
		// Texture coordinates of gauge in s_texHudItems.
		float s = 704.5f / himaxx;
		float t = .5f / himaxy;
		float u = 767.5 / himaxx;
		float v = 63.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-wid2,0,0, s,t);
		tri0.m_V1 = AuPointTex(-wid2,ht,0, s,v);
		tri0.m_V2 = AuPointTex(wid2,ht,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(wid2,ht,0, u,v);
		tri1.m_V1 = AuPointTex(wid2,0,0, u,t);
		tri1.m_V2 = AuPointTex(-wid2,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_SpeedoGaugeVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_SpeedoNeedleVAO.VAO())
	{
		// Texture coordinates of needle in s_texHudItems.
		float s = 128.5f / himaxx;
		float t = 6.5f / himaxy;
		float u = 155.5 / himaxx;
		float v = 15.5f / himaxy;

		// Needle texture is 27 by 9 pixels.
		// Center pivot is at 7,4.5
		float needlewid = size * 27.f / 64.f;
		float needleht = ht * 9.f / 64.f;
		float needleht2 = needleht/2.f;
		float needlecen = needlewid * 7.f/27.f;
		float needlelen = needlewid - needlecen;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-needlecen,-needleht2,0, s,t);
		tri0.m_V1 = AuPointTex(-needlecen,needleht2,0, s,v);
		tri0.m_V2 = AuPointTex(needlelen,needleht2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(needlelen,needleht2,0, u,v);
		tri1.m_V1 = AuPointTex(needlelen,-needleht2,0, u,t);
		tri1.m_V2 = AuPointTex(-needlecen,-needleht2,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_SpeedoNeedleVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texHudItems);

	// Gauge face
	AuMatrix4x4 mvbase;
	mvbase.Translate(xpos, ypos, 0);
	GLSL().TextureShader().SetProjectionModelView(projection * mvbase);

	m_SpeedoGaugeVAO.SelectGL();
	m_SpeedoGaugeVAO.DrawGL();
	m_SpeedoGaugeVAO.DeselectGL();

	// Needle
	// Calculate needle rotation based on speed.
	speedMph = fabs(speedMph);
	if(speedMph < 5)
		speedMph = 5;	// Speedometer bottoms out at 5mph.
	else if(speedMph > 85)
		speedMph = 85;	// Gauge maxes out at 85mph.
	const float degPerMph = 180.f/50.f;
	const float needleZero = 20.f * degPerMph + 180.f;	// 0 position
	const float needleRot = (needleZero - speedMph * degPerMph) * DEG2RADf;

	AuMatrix4x4 mvneedle;
	mvneedle.Translate(xpos, ypos + ht/2.f, 0);
	mvneedle.RotateZ(needleRot);
	GLSL().TextureShader().SetProjectionModelView(projection * mvneedle);

	m_SpeedoNeedleVAO.SelectGL();
	m_SpeedoNeedleVAO.DrawGL();
	m_SpeedoNeedleVAO.DeselectGL();

	GLSL().TextureShader().Deselect();
}

// Show vehicle orientation with respect to rider.
void AuGLEffects::RenderOrientationIndicator(const QRect& clientRect, float xpos, float ypos, float zRot, float size,
		float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in s_texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	if(!m_OrientPointerVAO.VAO())
	{
		// Pointer indicating direction.
		// Texture coordinates of pointer in s_texHudItems.
		// Needle texture is 32 by 48 pixels.
		// Center pivot is at 16,16
		float s = 240.5f / himaxx;
		float t = .5f / himaxy;
		float u = 271.5 / himaxx;
		float v = 47.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-16,-16,0, s,t);
		tri0.m_V1 = AuPointTex(-16,32,0, s,v);
		tri0.m_V2 = AuPointTex(16,32,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(16,32,0, u,v);
		tri1.m_V1 = AuPointTex(16,-16,0, u,t);
		tri1.m_V2 = AuPointTex(-16,-16,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_OrientPointerVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	/*
	if(!m_OrientHeadVAO.VAO())
	{
		// Texture coordinates of head in s_texHudItems.
		// Head texture is 16 by 16 pixels.
		float s = 224.5f / himaxx;
		float t = .5f / himaxy;
		float u = 239.5 / himaxx;
		float v = 15.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-10,-10,0, s,t);
		tri0.m_V1 = AuPointTex(-10,10,0, s,v);
		tri0.m_V2 = AuPointTex(10,10,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(10,10,0, u,v);
		tri1.m_V1 = AuPointTex(10,-10,0, u,t);
		tri1.m_V2 = AuPointTex(-10,-10,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_OrientHeadVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	*/

	float scale = size / 16.f;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Calculate vertices of needle, rotated according to Z rotation.
	AuMatrix4x4 mneedle;
	mneedle.Translate(xpos, ypos, 0);
	mneedle.Scale(scale, scale, 1);
	mneedle.RotateZ(-zRot);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetProjectionModelView(projection * mneedle);
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texHudItems);

	m_OrientPointerVAO.SelectGL();
	m_OrientPointerVAO.DrawGL();
	m_OrientPointerVAO.DeselectGL();

	/*
	AuMatrix4x4 mhead;
	mhead.Translate(xpos, ypos, 0);
	mhead.Scale(scale, scale, 1);

	GLSL().TextureShader().SetModelView(mhead);

	m_OrientHeadVAO.SelectGL();
	m_OrientHeadVAO.DrawGL();
	m_OrientHeadVAO.DeselectGL();
	*/

	GLSL().TextureShader().Deselect();
}

void AuGLEffects::RenderBar(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
		int whichBar, float rleft, float gleft, float bleft, float rright, float gright, float bright, float alpha)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Variables used for calculating texture coordinates in HudItems.png.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	// Create VAOs.
	if(!m_BarVAO.VAO())
	{
		float s = .5f / himaxx;
		float t = .5f / himaxy;
		float u = 128.5f / himaxx;
		float v = 9.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_BarVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	if(value < 0)
		value = 0;
	else if(value > 1)
		value = 1;
	float threshold = (.5f + value * 128.f) / himaxx;

	// Texture offset to appropriate bar texture.
	// 'whichBar' 0=vehicle health/wrench, 1=fuel/gas pump, 2=body health/heart, 3=stamina/lungs, 4=hunger/stomach
	float texoffx, texoffy;
	switch(whichBar)
	{
	default:	// Undefined/Solid bar
		texoffx = 780.f/himaxx;
		texoffy = 31.f/himaxy;
		break;
	case 0:	// Vehicle health/Wrench
		texoffx = 271.f/himaxx;
		texoffy = 20.f/himaxy;
		break;
	case 1:	// Fuel/Gas Pump
		texoffx = 768.f/himaxx;
		texoffy = 31.f/himaxy;
		break;
	case 2:	// Body health/Heart
		texoffx = 0.f/himaxx;
		texoffy = 6.f/himaxy;
		break;
	case 3:	// Stamina/Lungs
		texoffx = 271.f/himaxx;
		texoffy = 29.f/himaxy;
		break;
	case 4:	// Hunger/Stomach
		texoffx = 271.f/himaxx;
		texoffy = 11.f/himaxy;
		break;
	}

	AuMatrix4x4 mv;
	mv.Translate(left, bottom, 0);
	mv.Scale(barwid, barht, 1);

	// Select shader for status bar.
	GLSL().TexBarShader().Select();
	GLSL().TexBarShader().SetProjectionModelView(projection * mv);
	GLSL().TexBarShader().SetTexture(texHudItems);
	GLSL().TexBarShader().SetThreshold(threshold);
	GLSL().TexBarShader().SetTextureOffset(texoffx, texoffy);
	GLSL().TexBarShader().SetColorLeft(rleft, gleft, bleft, alpha);
	GLSL().TexBarShader().SetColorRight(rright, gright, bright, alpha);

	m_BarVAO.SelectGL();
	m_BarVAO.DrawGL();
	m_BarVAO.DeselectGL();

	GLSL().TexBarShader().Deselect();
}

void AuGLEffects::RenderItemConditionBar(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
		float rbottom, float gbottom, float bbottom, float rtop, float gtop, float btop, float alpha)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Variables used for calculating texture coordinates in HudItems.png.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	// Create VAOs.
	if(!m_BarVAO.VAO())
	{
		float s = .5f / himaxx;
		float t = .5f / himaxy;
		float u = 128.5f / himaxx;
		float v = 9.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_BarVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	if(value < 0)
		value = 0;
	else if(value > 1)
		value = 1;
	float threshold = (.5f + value * 128.f) / himaxx;

	// Texture offset to solid bar texture.
	float texoffx = 780.f/himaxx;
	float texoffy = 31.f/himaxy;

	AuMatrix4x4 mv;
	mv.Translate(left + barwid, bottom, 0);	// So rotate to vertical ends up in proper position.
	mv.RotateZ(PI/2);						// Rotate vertical.
	mv.Scale(barht, barwid, 1);

	// Select shader for status bar.
	GLSL().TexBarShader().Select();
	GLSL().TexBarShader().SetProjectionModelView(projection * mv);
	GLSL().TexBarShader().SetTexture(texHudItems);
	GLSL().TexBarShader().SetThreshold(threshold);
	GLSL().TexBarShader().SetTextureOffset(texoffx, texoffy);
	GLSL().TexBarShader().SetColorLeft(rbottom, gbottom, bbottom, alpha);
	GLSL().TexBarShader().SetColorRight(rtop, gtop, btop, alpha);

	m_BarVAO.SelectGL();
	m_BarVAO.DrawGL();
	m_BarVAO.DeselectGL();

	GLSL().TexBarShader().Deselect();
}

void AuGLEffects::RenderMovePower(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
		float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	if(!m_MoveVAO.VAO())
	{
		// Texture coordinates in HudItems.png.
		float s = 908.5f / himaxx;
		float t = 31.5f / himaxy;
		float u = 1023.5 / himaxx;
		float v = 40.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_MoveVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texHudItems);

	// Spring scaled to the right.
	AuMatrix4x4 mv;
	mv.Translate(left, bottom, 0);
	if(value < 0)
		value = 0;
	else if(value > 1)
		value = 1;
	mv.Scale(value * barwid,barht,1);
	GLSL().TextureShader().SetProjectionModelView(projection * mv);

	m_MoveVAO.SelectGL();
	m_MoveVAO.DrawGL();
	m_MoveVAO.DeselectGL();

	GLSL().TextureShader().Deselect();
}

void AuGLEffects::RenderJumpSpring(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
		float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	// Variables used for calculating texture coordinates in texHudItems.
	const float himaxx = 1024;	// width of the texture
	const float himaxy = 64;	// height of the texture

	if(!m_JumpVAO.VAO())
	{
		// Texture coordinates in HudItems.png.
		float s = 271.5f / himaxx;
		float t = 0.5f / himaxy;
		float u = 399.5 / himaxx;
		float v = 10.5f / himaxy;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,t);
		tri0.m_V1 = AuPointTex(0,1,0, s,v);
		tri0.m_V2 = AuPointTex(1,1,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,v);
		tri1.m_V1 = AuPointTex(1,0,0, u,t);
		tri1.m_V2 = AuPointTex(0,0,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_JumpVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texHudItems);

	// Spring scaled to the right.
	AuMatrix4x4 mv;
	mv.Translate(left, bottom, 0);
	if(value < 0)
		value = 0;
	else if(value > 1)
		value = 1;
	mv.Scale(value * barwid,barht,1);
	GLSL().TextureShader().SetProjectionModelView(projection * mv);

	m_JumpVAO.SelectGL();
	m_JumpVAO.DrawGL();
	m_JumpVAO.DeselectGL();

	GLSL().TextureShader().Deselect();
}

void AuGLEffects::RenderCrosshair(const QRect& clientRect, float xpos, float ypos, float targetLevel,
		float r, float g, float b, float a)
{
	// Create texture.
	GLuint texHudItems = HUDItemsTexture();
	if(!texHudItems)
		return;

	if(!m_CrosshairDot.VAO())
	{
		// Texture coordinates in HudItems.png.
		float s = 0.09521484375;	// = 97.5f / himaxx;
		float t = 0.0234375;		// = 1.5f / himaxy;
		float u = 0.09912109375;	// = 101.5f / himaxx;
		float v = 0.0859375;		// = 5.5f / himaxy;

		const float w2 = .005f;	// 1/2 width of crosshair poly
		const float w3 = w2 + .001f;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-w3,-w3,0, s,t);
		tri0.m_V1 = AuPointTex(-w3,w3,0, s,v);
		tri0.m_V2 = AuPointTex(w3,w3,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(w3,w3,0, u,v);
		tri1.m_V1 = AuPointTex(w3,-w3,0, u,t);
		tri1.m_V2 = AuPointTex(-w3,-w3,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_CrosshairDot.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}
	if(!m_CrosshairBar.VAO())
	{
		// Texture coordinates in HudItems.png.
		float s = 0.00048828125;	// = .5f / himaxx;
		float t = 0.0234375;		// = 1.5f / himaxy;
		float u = 0.04638671875;	// = 47.5f / himaxx;
		float v = 0.0859375;		// = 5.5f / himaxy;

		const float w2 = .005f;	// 1/2 width of crosshair poly
		const float cd = .005f;	// center to hair distance
		const float len = .10f;	// length of crosshair poly

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(cd,-w2,0, s,t);
		tri0.m_V1 = AuPointTex(cd,w2,0, s,v);
		tri0.m_V2 = AuPointTex(len,w2,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(len,w2,0, u,v);
		tri1.m_V1 = AuPointTex(len,-w2,0, u,t);
		tri1.m_V2 = AuPointTex(cd,-w2,0, s,t);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_CrosshairBar.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	AuMatrix4x4 projection;
	projection.Scale(WindowAspectRatio(clientRect), 1, 1);

	// Configure shader.
	GLSL().TextureShader().Select();
	GLSL().TextureShader().SetColor(r,g,b,a);
	GLSL().TextureShader().SetTexture(texHudItems);

	// Spring scaled to the right.
	AuMatrix4x4 mv;
	mv.Translate(xpos, ypos, 0);
	GLSL().TextureShader().SetProjectionModelView(projection * mv);

	m_CrosshairDot.SelectGL();
	m_CrosshairDot.DrawGL();
	m_CrosshairDot.DeselectGL();

	// Draw four bars.
	const float mtl = .3f;	// Max dist from center
	const float tl = targetLevel * mtl;	// Dist from center

	m_CrosshairBar.SelectGL();

	AuMatrix4x4 mvbr;
	mvbr.Translate(xpos + tl, ypos, 0);
	GLSL().TextureShader().SetProjectionModelView(projection * mvbr);

	m_CrosshairBar.DrawGL();

	mvbr.LoadIdentity();
	mvbr.Translate(xpos - tl, ypos, 0);
	mvbr.RotateZ(PI);
	GLSL().TextureShader().SetProjectionModelView(projection * mvbr);

	m_CrosshairBar.DrawGL();

	mvbr.LoadIdentity();
	mvbr.Translate(xpos, ypos + tl, 0);
	mvbr.RotateZ(PI/2);
	GLSL().TextureShader().SetProjectionModelView(projection * mvbr);

	m_CrosshairBar.DrawGL();

	mvbr.LoadIdentity();
	mvbr.Translate(xpos, ypos - tl, 0);
	mvbr.RotateZ(-PI/2);
	GLSL().TextureShader().SetProjectionModelView(projection * mvbr);

	m_CrosshairBar.DrawGL();

	m_CrosshairBar.DeselectGL();

	GLSL().TextureShader().Deselect();
}

void AuGLEffects::RenderShieldBubble(const AuMatrix4x4& projection, const AuMatrix4x4& model, float sizeX, float sizeY, float sizeZ,
		float front, bool frontScram, float bottom, bool bottomScram, float left, bool leftScram, float back, bool backScram,
		float top, bool topScram, float right, bool rightScram, float texOffset, float divisor,
		float hitPosArray[MAX_SHIELD_HIT_EFFECTS*4], float hitDurArray[MAX_SHIELD_HIT_EFFECTS], bool hasShieldHits)
{
	GLuint texShield = CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Shield", ":data/TerNoise.png", GL_LINEAR, GL_LINEAR, false, 0);
	if(!texShield)
		return;
	GLuint texShieldScram = CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ShScram", ":data/ShieldScram.png", GL_LINEAR, GL_LINEAR,
			false, 0);
	if(!texShieldScram)
		return;

	// In case this function gets called when a model was already present
	if(!m_ShieldVAO.VAO())
	{
		QList<AuTriTex> tris;
		int i, j;
		int lats = 16;
		int longs = 16;
		for(i = 0; i <= lats; i++)
		{
			if(i == 0 && i != lats)
				continue;
			float lat0 = PIf * (-0.5f + (float) (i - 1) / lats);
			float z0 = sinf(lat0);
			float zr0 = cosf(lat0);

			float lat1 = PIf * (-0.5f + (float) i / lats);
			float z1 = sinf(lat1);
			float zr1 = cosf(lat1);

			// Texture coords for this row.
			float t0 = (float)(i)/(lats+1);
			float t1 = (float)(i+1)/(lats+1);

			AuPoint prev0, prev1;
			float pres = 0;
			for(j = 0; j <= longs; j++)
			{
				float lng = 2 * PIf * (float) (j - 1) / longs;
				float x = cosf(lng);
				float y = sinf(lng);

				AuPoint v0(x * zr0, y * zr0, z0);
				AuPoint v1(x * zr1, y * zr1, z1);
				float s = ((float)j / (float)longs);
				if(j)
				{
					AuTriTex tri0;
					tri0.m_V0 = AuPointTex(prev0, pres, t0);
					tri0.m_V1 = AuPointTex(prev1, pres, t1);
					tri0.m_V2 = AuPointTex(v1, s, t1);
					tris.append(tri0);

					AuTriTex tri1;
					tri1.m_V0 = AuPointTex(v1, s, t1);
					tri1.m_V1 = AuPointTex(v0, s, t0);
					tri1.m_V2 = AuPointTex(prev0, pres, t0);
					tris.append(tri1);
				}
				prev0 = v0;
				prev1 = v1;
				pres = s;
			}
		}
		m_ShieldVAO.CreateForTexture(*this, tris, GL_STATIC_DRAW);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_POLYGON_OFFSET_FILL);

	m_ShieldVAO.SelectGL();

	// Configure shader.
	if(hasShieldHits)
	{
		GLSL().ShieldHitsShader().Select();
		GLSL().ShieldHitsShader().SetProjectionModelView(projection * model);
		GLSL().ShieldHitsShader().SetTexture(texShield);
		GLSL().ShieldHitsShader().SetTextureScram(texShieldScram);
		GLSL().ShieldHitsShader().SetScale(sizeX, sizeY, sizeZ);
		GLSL().ShieldHitsShader().SetSideIntensities(front, bottom, left, back, top, right);
		GLSL().ShieldHitsShader().SetSideScram(frontScram, bottomScram, leftScram, backScram, topScram, rightScram);
		GLSL().ShieldHitsShader().SetColor(.145f/divisor, .145f/divisor, .4325f/divisor);
		GLSL().ShieldHitsShader().SetHits(hitPosArray, hitDurArray);

		for(int i=0; i<4; i++)
		{
			float fUOffset=0;
			float fVOffset=0;
			if(i==1)
			{
				fUOffset = texOffset/20.f;
				fVOffset = 0;
			}
			else if(i==2)
			{
				fUOffset = 0;
				fVOffset = texOffset/23.f;
			}
			else if(i==3)
			{
				fUOffset = -texOffset/40.f;
				fVOffset = -texOffset/23.f;
			}

			// Apply polygon offset since the passes are coplanar with the same polygons.
			glPolygonOffset(-1.f, -i);  // This worked better in gw.

			GLSL().ShieldHitsShader().SetTextureOffset(fUOffset, fVOffset);
			m_ShieldVAO.DrawGL();
		}

		GLSL().ShieldHitsShader().Deselect();
	}
	else
	{
		GLSL().ShieldShader().Select();
		GLSL().ShieldShader().SetProjectionModelView(projection * model);
		GLSL().ShieldShader().SetTexture(texShield);
		GLSL().ShieldShader().SetTextureScram(texShieldScram);
		GLSL().ShieldShader().SetScale(sizeX, sizeY, sizeZ);
		GLSL().ShieldShader().SetSideIntensities(front, bottom, left, back, top, right);
		GLSL().ShieldShader().SetSideScram(frontScram, bottomScram, leftScram, backScram, topScram, rightScram);
		GLSL().ShieldShader().SetColor(.145f/divisor, .145f/divisor, .4325f/divisor);

		for(int i=0; i<4; i++)
		{
			float fUOffset=0;
			float fVOffset=0;
			if(i==1)
			{
				fUOffset = texOffset/20.f;
				fVOffset = 0;
			}
			else if(i==2)
			{
				fUOffset = 0;
				fVOffset = texOffset/23.f;
			}
			else if(i==3)
			{
				fUOffset = -texOffset/40.f;
				fVOffset = -texOffset/23.f;
			}

			// Apply polygon offset since the passes are coplanar with the same polygons.
			glPolygonOffset(-1.f, -i);  // This worked better in gw.

			GLSL().ShieldShader().SetTextureOffset(fUOffset, fVOffset);
			m_ShieldVAO.DrawGL();
		}

		GLSL().ShieldShader().Deselect();
	}

	m_ShieldVAO.DeselectGL();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_BLEND);
}

//static
double AuGLEffects::SceneAspectRatio(const QRect& windowRect)
{
	// Our typical perspective projection has a 90 degree field of view.
	// We account for aspect ratio so the field of view remains constant vertically; horizontal field of view varies with
	// window aspect ratio. We can't allow less than .5 aspect because the resulting horizontal field of view is greater
	// than 180 degrees, which makes things clip too much in tests for on screen, based on Z >= 0. We will stretch the scene
	// using aspect ratio, rather than allow a field of view greater than 180.
	double aspect = WindowAspectRatio(windowRect);
	if(aspect < .5)
		return .5;
	return aspect;
}

//static
double AuGLEffects::WindowFieldOfViewY(double aspect, double desiredFoVY)
{
	if(aspect == 0)
		return desiredFoVY;
	// Don't allow the fovX to exceed 180 degrees.
	double fovx = desiredFoVY / aspect;
	if(fovx <= PI)
		return desiredFoVY;

	// Calculate fovY at fovX = 180 degrees.
	return PI * aspect;
}

//static
QColor AuGLEffects::Blend(const QColor& col1, const QColor& col2, float blend)
{
	if(blend <= 0)
		return col1;
	else if(blend >= 1)
		return col2;
	float b1 = 1 - blend;
	float r = col1.redF() * b1 + col2.redF() * blend;
	float g = col1.greenF() * b1 + col2.greenF() * blend;
	float b = col1.blueF() * b1 + col2.blueF() * blend;
	float a = col1.alphaF() * b1 + col2.alphaF() * blend;
	QColor col;
	col.setRgbF(r,g,b,a);
	return col;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
bool AuGLTexture::CreateFromRGB(AuOpenGLFunctions& GLf, const unsigned char *rgbdata, int xsize, int ysize,
		GLenum format, GLenum dataType, bool mipMap)
{
	// Create and scale the texture.
	int sxsize = xsize;
	int sysize = ysize;

	// Number of color components and data alignment.
	GLint alignment = 1;
	GLint components = format;
	if(format == GL_BGR_EXT)
	{
		components = 3;
		alignment = 4;	// DIB data is always aligned on DWORD boundaries.
	}

	// Establish the desired unpacking alignment before the texture is processed.
	GLint oldalignment;
	GLf.glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldalignment);
	OutputGlError(QString("File: %1 Line: %2 glGetIntegerv").arg(__FILE__).arg(__LINE__));
	GLf.glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	OutputGlError(QString("File: %1 Line: %2 glPixelStorei").arg(__FILE__).arg(__LINE__));

	GLf.glTexImage2D(GL_TEXTURE_2D,		// Reserved 
			0,							// Level
			components,					// Components for modulating and blending
			sxsize, sysize,				// Width and height
			0,							// Border size
			format,						// GL_RGB or GL_RGBA or GL_BGR_EXT
			dataType,					// Data type of pixel values
			rgbdata						// Pixel values
			);
	OutputGlError(QString("File: %1 Line: %2 glTexImage2D").arg(__FILE__).arg(__LINE__));

	if(mipMap)
		GLf.glGenerateMipmap(GL_TEXTURE_2D);

	// Restore the original unpack alignment state.
	GLf.glPixelStorei(GL_UNPACK_ALIGNMENT, oldalignment);
	OutputGlError(QString("File: %1 Line: %2 glPixelStorei").arg(__FILE__).arg(__LINE__));

	return true;
}

// Creates this texture from a QImage.
//static
bool AuGLTexture::CreateFromBitmap(AuOpenGLFunctions& GLf, const QImage &bm, bool mipMap)
{
	// Transfer bitmap to OpenGL texture.
	int pixels = bm.width() * bm.height();
	if(!pixels)
		return false;

	AuGLRGBA* ptex = new AuGLRGBA[pixels];
	if(!ptex)
		return false;

	AuGLRGBA* pDest = ptex;
	for(int iy=bm.height()-1; iy>=0; iy--)
	{
		for(int ix=0; ix<bm.width(); ix++)
		{
			pDest->SetColor(bm.pixel(ix,iy));
			pDest++;
		}
	}

	bool success = CreateFromRGB(GLf, (const unsigned char*)ptex, bm.width(), bm.height(), GL_RGBA, GL_UNSIGNED_BYTE, mipMap);

	delete [] ptex;

	return(success);
}

// expects a compact array of floats in sets of three or four, depending on format
//static
bool AuGLTexture::CreateFromFloats(AuOpenGLFunctions& GLf, const float* data, unsigned int width,
		unsigned int height, bool mipMap, GLenum format)
{
	bool success = false;

	// Transfer bitmap to OpenGL texture.
	int pixels = width * height;
	AuGLRGBAf *ptex = new AuGLRGBAf[pixels];
	if(ptex)
	{
		AuGLRGBAf* pDest = ptex;
		for(int iy=height-1; iy>=0; iy--)
		{
			for(unsigned int ix=0; ix<width; ix++)
			{
				if(format == GL_RGB)
				{
					const float* src = data + iy * (width*3) + ix*3;
					pDest->SetColor(*src, *(src+1), *(src+2), 1.f);
				}
				else if(format == GL_RGBA)
				{
					const float* src = data + iy * (width*4) + ix*4;
					pDest->SetColor(*src, *(src+1), *(src+2), *(src+3));
				}
				else
					Q_ASSERT(false);
				pDest++;
			}
		}
		success = CreateFromRGB(GLf, (const unsigned char*)ptex, width, height, GL_RGBA, GL_FLOAT, mipMap);
		delete [] ptex;
	}

	return(success);
}

void AuGLTexture::Delete(AuOpenGLFunctions& GLf)
{
	if(m_GlName != 0)
	{
#if !defined(QT_NO_DEBUG)
		int users = m_Users.size();
		if(users > 1 || (users == 1 && ((*m_Users.begin()).first) != STATIC_TEXTURE_ID))
			qWarning() << "AuGLTexture deleting" << m_GlName << m_sId << "users" << users;
#endif
		GLf.glDeleteTextures(1, &m_GlName);
		OutputGlError(QString("File: %1 Line: %2 glDeleteTextures %3").arg(__FILE__).arg(__LINE__).arg(m_GlName));
		m_GlName = 0;
	}
}

void AuGLTexture::NameAndBind(AuOpenGLFunctions& GLf, bool mipMap, GLint magFilter, GLint minFilter)
{
	// Delete this texture if it was already created.
	Delete(GLf);

	// Get a new unique texture id from OpenGL.
	GLf.glGenTextures(1, &m_GlName);
	OutputGlError(QString("File: %1 Line: %2 glGenTextures").arg(__FILE__).arg(__LINE__));
	if (m_GlName != 0)
	{
		// Initiate the creation of a texture object by binding to the id
		// for the first time.
		GLf.glBindTexture(GL_TEXTURE_2D, m_GlName);
		OutputGlError(QString("File: %1 Line: %2 glBindTexture %3").arg(__FILE__).arg(__LINE__).arg(m_sId));

		//****************************************
		//     --  O P E N G L   N O T E  --
		//
		// The texture filter options MUST be set
		// now. If not, the texture doesn't work.
		//
		// It's possible that OpenGL requires ANY
		// texture parameter to be set now to
		// somehow "initialize" the texture
		// object. I have not found any doc that
		// establishes this requirement or 
		// describes this behavior.
		//
		// dhh - 12/27/2002
		//****************************************

		// OpenGL Texture Filter Options
		if(mipMap)
		{
			GLf.glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			OutputGlError(QString("File: %1 Line: %2 glTexParameteri").arg(__FILE__).arg(__LINE__));
			GLf.glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			OutputGlError(QString("File: %1 Line: %2 glTexParameteri").arg(__FILE__).arg(__LINE__));
		}
		else
		{
			GLf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
			OutputGlError(QString("File: %1 Line: %2 glTexParameteri").arg(__FILE__).arg(__LINE__));
			GLf.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
			OutputGlError(QString("File: %1 Line: %2 glTexParameteri").arg(__FILE__).arg(__LINE__));
		}
	}
}

void AuGLTexture::ReplaceSubImage(AuOpenGLFunctions& GLf, QImage &bm, unsigned int xpos, unsigned int ypos)
{
	if(!m_GlName || bm.isNull())
		return;

	// Transfer bitmap to OpenGL texture.
	int pixels = bm.width() * bm.height();
	if(!pixels)
		return;

	AuGLRGBA* ptex = new AuGLRGBA[pixels];
	if(!ptex)
		return;

	AuGLRGBA* pDest = ptex;
	for(int iy=bm.height()-1; iy>=0; iy--)
	{
		for(int ix=0; ix<bm.width(); ix++)
		{
			pDest->SetColor(bm.pixel(ix,iy));
			pDest++;
		}
	}

	// Establish the desired unpacking alignment before the texture is processed.
	GLint oldalignment;
	GLf.glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldalignment); 
	OutputGlError(QString("File: %1 Line: %2 glGetIntegerv").arg(__FILE__).arg(__LINE__));
	GLf.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OutputGlError(QString("File: %1 Line: %2 glPixelStorei").arg(__FILE__).arg(__LINE__));

	// Replace the subimage in the Gl texture object.
	GLf.glBindTexture(GL_TEXTURE_2D, m_GlName);
	OutputGlError(QString("File: %1 Line: %2 glBindTexture").arg(__FILE__).arg(__LINE__));
	GLf.glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos, bm.width(), bm.height(), GL_RGBA, GL_UNSIGNED_BYTE, ptex);
	OutputGlError(QString("File: %1 Line: %2 glTexSubImage2D").arg(__FILE__).arg(__LINE__));
	GLf.glBindTexture(GL_TEXTURE_2D, 0);
	OutputGlError(QString("File: %1 Line: %2 glBindTexture").arg(__FILE__).arg(__LINE__));

	// Restore the original unpack alignment state.
	GLf.glPixelStorei(GL_UNPACK_ALIGNMENT, oldalignment);
	OutputGlError(QString("File: %1 Line: %2 glPixelStorei").arg(__FILE__).arg(__LINE__));

	delete [] ptex;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
void AuTextureLibItem::InitDefaultScPalette(quint16 texturePalette[SCTEXI_Count])
{
	int tx=0;
	texturePalette[tx++] = TEXLIB_WallMetal;
	texturePalette[tx++] = TEXLIB_MetalPlateBox;				// SCTEXI_Material
	texturePalette[tx++] = TEXLIB_WallCargo;
	texturePalette[tx++] = TEXLIB_WallPipes;
	texturePalette[tx++] = TEXLIB_CeilingMetal;
	texturePalette[tx++] = TEXLIB_FloorMetal;
	texturePalette[tx++] = TEXLIB_MetalScratchRust;
	texturePalette[tx++] = TEXLIB_WoodPlanksBeams;
	texturePalette[tx++] = TEXLIB_BlackRoundDots;
	texturePalette[tx++] = TEXLIB_GrillBlackOblongHoles;
	texturePalette[tx++] = TEXLIB_VentCopper;
	texturePalette[tx++] = TEXLIB_VentBlack;
	texturePalette[tx++] = TEXLIB_Fabric;
	texturePalette[tx++] = TEXLIB_Grass;
	texturePalette[tx++] = TEXLIB_Decals;
	Q_ASSERT(tx == SCTEXI_Count);
}

//static
void AuTextureLibItem::InitDefaultBdPalette(quint16 texturePalette[SCTEXI_Count])
{
	int tx=0;
	texturePalette[tx++] = TEXLIB_WallKnockdown;
	texturePalette[tx++] = TEXLIB_BrickWall;					// SCTEXI_Material
	texturePalette[tx++] = TEXLIB_BrickLight;
	texturePalette[tx++] = TEXLIB_Concrete;
	texturePalette[tx++] = TEXLIB_OSB;
	texturePalette[tx++] = TEXLIB_StuccoWallPlaster;
	texturePalette[tx++] = TEXLIB_ConcretePrestressed;
	texturePalette[tx++] = TEXLIB_WoodSiding;
	texturePalette[tx++] = TEXLIB_ShinglesAsphalt;
	texturePalette[tx++] = TEXLIB_MetalTread;
	texturePalette[tx++] = TEXLIB_VentWhite;
	texturePalette[tx++] = TEXLIB_SeaWater;
	texturePalette[tx++] = TEXLIB_Fabric;
	texturePalette[tx++] = TEXLIB_Grass;
	texturePalette[tx++] = TEXLIB_Decals;
	Q_ASSERT(tx == SCTEXI_Count);
}
