//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auterraininfo.cpp,v 1.14 2022/02/19 18:33:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auterraininfo.h"
//

QString AuTerrainInfo::TerrainTypeName() const
{
	QString typeName;
	if(IsGasGiant())
	{
		switch(TerrainType())
		{
		case HighlandForest:
			typeName += QObject::tr("Crust");
			break;
		case HighlandSavanna:
			typeName += QObject::tr("Crust");
			break;
		case HighlandDesert:
			typeName += QObject::tr("Crust");
			break;
		case LowlandForest:
			typeName += QObject::tr("Lithosphere");
			break;
		case LowlandSavanna:
			typeName += QObject::tr("Lithosphere");
			break;
		case LowlandDesert:
			typeName += QObject::tr("Lithosphere");
			break;
		case SeaLevelForest:
			typeName += QObject::tr("Mantle");
			break;
		case SeaLevelSavanna:
			typeName += QObject::tr("Mantle");
			break;
		case SeaLevelDesert:
			typeName += QObject::tr("Mantle");
			break;
		case Underwater:
			typeName += QObject::tr("Core");
			break;
		case TerrainTypes:
			typeName += "WTF";
			break;
		}
	}
	else
	{
		switch(TerrainType())
		{
		case HighlandForest:
			typeName += QObject::tr("Highland Forest");
			break;
		case HighlandSavanna:
			typeName += QObject::tr("Highland Savanna");
			break;
		case HighlandDesert:
			typeName += QObject::tr("Highland Desert");
			break;
		case LowlandForest:
			typeName += QObject::tr("Lowland Forest");
			break;
		case LowlandSavanna:
			typeName += QObject::tr("Lowland Savanna");
			break;
		case LowlandDesert:
			typeName += QObject::tr("Lowland Desert");
			break;
		case SeaLevelForest:
			typeName += QObject::tr("Sea Level Forest");
			break;
		case SeaLevelSavanna:
			typeName += QObject::tr("Sea Level Savanna");
			break;
		case SeaLevelDesert:
			typeName += QObject::tr("Sea Level Desert");
			break;
		case Underwater:
			typeName += QObject::tr("Underwater");
			break;
		case TerrainTypes:
			typeName += "WTF";
			break;
		}
	}
	return typeName;
}

