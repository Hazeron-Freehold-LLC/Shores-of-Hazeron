//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufilever.h,v 1.9 2022/02/19 18:42:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// File types and current versions created by Shores of Hazeron
#define SHIP_FILE_TYPE					0x10					// .ship spacecraft design files
#define SHIP_FILE_VER					0x01					// .ship spacecraft design files

#define CHR_FILE_TYPE					0x11					// .chr avatar files
#define CHR_FILE_VER					0x01					// .chr avatar files

#define SHIP_MODEL_CACHE				0x12					// S12345 spacecraft model files in temporary cache
#define SHIP_MODEL_VER					0x12					// S12345 spacecraft model files in temporary cache

#define SOHCRIT_FILE_TYPE				0x13					// .SoHCrit system search criteria files
#define SOHCRIT_FILE_VER				0x01					// .SoHCrit system search criteria files

#define SHIP_PATH_CACHE					0x14					// SP12345 spacecraft path files in temporary cache
#define SHIP_PATH_VER					0x11					// SP12345 spacecraft path files in temporary cache

#define SHIP_DESIGN_CACHE				0x15					// D12345 spacecraft design files in temporary cache
#define SHIP_DESIGN_VER					0x11					// D12345 spacecraft design files in temporary cache

#define FLORA_TEXTURE_CACHE				0x16					// F12345Z0 plant texture files in temporary cache
#define FLORA_TEXTURE_VER				0x21					// F12345Z0 plant texture files in temporary cache

#define NEARGLOBE_MODEL_CACHE			0x17					// WGN12345 globe model files in temporary cache
//auglmodelmaker.cpp #define NEARGLOBE_MODEL_VER				0x11					// WGN12345 globe models in temporary cache

#define GAME_DATA_CACHE					0x18					// GD file as distributed and in temporary cache
//dgdata.cpp #define GAME_DATA_VER					0x2d					// GD file as distributed and in temporary cache

#define ATGLOBE_MODEL_CACHE				0x19					// WGA12345 globe model files in temporary cache
//auglmodelmaker.cpp #define ATGLOBE_MODEL_VER				0x11					// WGA12345 globe model files in temporary cache

#define BUILDING_FILE_TYPE				0x1a					// .building building design files
#define BUILDING_FILE_VER				0x01					// .building building design files

#define BUILDING_DESIGN_CACHE			0x1b					// B12345 building design files in temporary cache
#define BUILDING_DESIGN_VER				0x11					// B12345 building design files in temporary cache

#define BUILDING_PATH_CACHE				0x1c					// BP12345 building path files in temporary cache
#define BUILDING_PATH_VER				0x11					// BP12345 building path files in temporary cache

#define BUILDING_EXTERIOR_MODEL_CACHE	0x1d					// BE12345.1.2 building exterior model files in temporary cache
#define BUILDING_EXTERIOR_MODEL_VER		0x12					// BE12345.1.2 building exterior model files in temporary cache

#define FLATGLOBE_MODEL_CACHE			0x1e					// WGF12345 flat globe model files in temporary cache
//auglmodelmaker.cpp #define FLATGLOBE_MODEL_VER				0x11					// WGF12345 flat globe models in temp cache

#define BUILDING_INTERIOR_MODEL_CACHE	0x1f					// BI12345.1.2 building interior model files in temporary cache
#define BUILDING_INTERIOR_MODEL_VER		0x12					// BI12345.1.2 building interior model files in temporary cache

#define WORLD_TEXTURE_FAR_CACHE			0x20					// WTF12345 far world texture files in temporary cache
//daworld.h #define WORLD_TEXTURE_FAR_VER			0x12					// WTF12345 far world texture files in temporary cache

#define MAIL_FILE_TYPE					0x21					// .m files in mail message folder
#define MAIL_FILE_VER					0x10					// .m files in mail message folder

#define GRASS_TEXTURE_CACHE				0x22					// FG12345Z0 plant texture files in temporary cache
//deflora.cpp #define GRASS_TEXTURE_VER				0x22					// FG12345Z0 plant texture files in temporary cache

#define MALL_MODEL_CACHE				0x23					// M12345 harsh environment enclosure model files in temporary cache
#define MALL_MODEL_VER					0x10					// M12345 harsh environment enclosure model files in temporary cache

#define SOH_FILE_TYPE					0x24					// .SoH files store AuScDesign spacecraft designs.
//auscdesign.cpp #define SOH_FILE_VER

#define DESIGNER_MIME_PART_TYPE			0x25					// Designer parts on the clip board.
//auscparts.cpp #define DESIGNER_MIME_PART_VER

#define DESIGNER_MIME_PART_OBJECT_TYPE	0x26					// Designer part objects (vertices, edges, faces) on the clip board.
//auscparts.cpp #define DESIGNER_MIME_PART_OBJECT_VER

#define SYSTEM_SCAN_CACHE				0x27					// SS solar system scan file in temporary cache
#define SYSTEM_SCAN_VER					0x01					// SS solar system scan file in temporary cache

#define DNA_FILE_TYPE					0x28					// Animalia DNA file
//audna.cpp #define DNA_FILE_VER

#define DNA_SPECIES_TYPE				0x29					// AuDNA species data type
//audna.cpp #define DNA_SPECIES_VER

#define BD_EXCHANGE_CACHE				0x2a					// Building exchange rec file in temporary cache
#define BD_EXCHANGE_VER					0x01					// Building exchange rec file in temporary cache

#define SC_EXCHANGE_CACHE				0x2b					// Spacecraft exchange rec file in temporary cache
#define SC_EXCHANGE_VER					0x01					// Spacecraft exchange rec file in temporary cache

#define SOLO_AVATAR_CACHE				0x2c					// Av file containing solitaire avatar save.
//auapp.cpp #define SOLO_AVATAR_VER				0x01

#define SOLO_GAME_CACHE					0x2d					// Db file containing solitaire game save.
//auapp.cpp #define SOLO_GAME_VER					0x01

#define SOLO_INIT_TYPE					0x2e					// Solitaire game DgData init.
//dgdata.cpp #define SOLO_INIT_VER					0x01
