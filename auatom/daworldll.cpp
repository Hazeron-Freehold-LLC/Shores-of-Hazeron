//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldll.cpp,v 1.728 2022/05/17 18:46:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daworldll.h"
//
#include "auatomrenderinfo.h"
#include "aubddesign.h"
#include "aubox.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "aufireparticles.h"
#include "auglmodelmaker.h"
#include "auroadmap.h"
#include "auscparts.h"
#include "ausmokeparticles.h"
#include "auterraininfo.h"
#include "auvillage.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesignbase.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldringsection.h"
#include "deconstructionprocess.h"
#include "defauna.h"
#include "deflora.h"
#include "dgdata.h"
#include "dprock.h"

#define ROAD_MAX_ALT_DEVIATION			(15*FeetPerMeter())		// Max difference between road level and actual ground level.
#define d_SiteEdgeStep					(50*FeetPerMeter())
#define d_MinVillageRange				(75*FeetPerMeter())
#define d_MaxVillageRange				(175*FeetPerMeter())

static GLuint							s_FlagEmpireTex = 0;	// Texture id of flag icon for top-down view.
static GLuint							s_FlagEnemyTex = 0;		// Texture id of flag icon for top-down view.
static GLuint							s_FlagFriendTex = 0;	// Texture id of flag icon for top-down view.
static GLuint							s_FlagNeutralTex = 0;	// Texture id of flag icon for top-down view.
static GLuint							s_TexConstruction = 0;
static GLuint							s_TexFire = 0;
static GLuint							s_TexSmoke = 0;

enum CONSTRUCTION_TERRAIN
{
	CONST_Coast,						// Construction requires the sea, but not lava.
	CONST_CoastLava,					// Construction requires the sea, may include lava.
	CONST_DryGround,					// Construction requires dry ground.
	CONST_Ground,						// Construction requires ground, water may be present.
	CONST_Rocks,						// Construction requires rocks. Must not overlap an existing mine.
	CONST_SeaSurface,					// Construction is on the sea surface.
	CONST_Trees,						// Construction requires trees.
	CONST_Well,							// Must not overlap an existing well.
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuWorldEnclosure::AuWorldEnclosure(const AuId& bdId, const AuScParts& envVoids, bool underSea)
{
	m_BdId = bdId;
	m_EnvVoids = new AuScParts(envVoids);
	m_UnderSea = underSea;
}

AuWorldEnclosure::AuWorldEnclosure(const AuWorldEnclosure& c)
{
	m_BdId = c.m_BdId;
	m_EnvVoids = new AuScParts(*c.m_EnvVoids);
	m_UnderSea = c.m_UnderSea;
}

AuWorldEnclosure::AuWorldEnclosure()
{
	m_EnvVoids = new AuScParts();
	m_UnderSea = false;
}

AuWorldEnclosure::~AuWorldEnclosure()
{
	delete m_EnvVoids;
}

// 'worldLoc' is in the space of the host world.
bool AuWorldEnclosure::Contains(const AuPoint& worldLoc) const
{
	return m_EnvVoids->VoidContainingLoc(worldLoc, AuDoorStateHash(), AuLandingGearStateHash(), AuLiftHash(), 0);
}

void AuWorldEnclosure::operator>>(QDataStream& out) const
{
	out << m_BdId;
	out << *m_EnvVoids;
	out << m_UnderSea;
}

void AuWorldEnclosure::operator<<(QDataStream& in)
{
	in >> m_BdId;
	in >> *m_EnvVoids;
	in >> m_UnderSea;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuWorldEnclosureCache::AnyNotReady() const
{
	for(const_iterator di = cbegin(); di != cend(); di++)
		if((*di).EnvironmentVoids()->empty())
			return true;
	return false;
}

// Return: 0=no, 1=yes land enclosure, 2=yes under sea enclosure, -1=maybe because a enclosure is here but its design is not ready
int AuWorldEnclosureCache::Contains(const AuPoint& worldLoc) const
{
	int insideEnclosure = 0;
	bool anyNotReady = false;
	for(const_iterator di = cbegin(); di != cend(); di++)
	{
		if((*di).EnvironmentVoids()->empty())
			anyNotReady = true;
		else if((*di).Contains(worldLoc))
		{
			insideEnclosure = (*di).IsUnderSea() ? 2 : 1;
			break;
		}
	}
	if(insideEnclosure)
		return insideEnclosure;
	else if(anyNotReady)
		return -1;
	return 0;
}

int AuWorldEnclosureCache::ContainsExcept(const AuPoint& worldLoc, const AuId& exceptId, AuId& enclosureId) const
{
	int insideEnclosure = 0;
	bool anyNotReady = false;
	for(const_iterator di = cbegin(); di != cend(); di++)
	{
		if((*di).BdId() == exceptId)
			continue;
		if((*di).EnvironmentVoids()->empty())
		{
			// An enclosure with no voids is added specifically to indicate when a potential enclosure's design is not ready.
			enclosureId = (*di).BdId();
			anyNotReady = true;
		}
		else if((*di).Contains(worldLoc))
		{
			insideEnclosure = (*di).IsUnderSea() ? 2 : 1;
			enclosureId = (*di).BdId();
			break;
		}
	}
	if(insideEnclosure)
		return insideEnclosure;
	else if(anyNotReady)
		return -1;
	return 0;
}

AuId AuWorldEnclosureCache::ContainsId(const AuPoint& worldLoc) const
{
	for(const_iterator di = cbegin(); di != cend(); di++)
	{
		if((*di).EnvironmentVoids()->empty())
			continue;
		if((*di).Contains(worldLoc))
			return (*di).BdId();
	}
	return AuId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGlobeRock::CreateVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao) const
{
	AuGlobeTriTexNShine tri;
	QList<AuGlobeTriTexNShine> rockTris;
	switch(CommodityId())
	{
	default:
	case CID_Stone:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Stone, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Coal:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Coal, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			float shine = .5f;

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);
		}
		break;
	case CID_Crystals:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Crystals, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Vertices
			AuPoint av[] = 
			{
				AuPoint(-.5,-.25,0),
				AuPoint(0,.5,0),
				AuPoint(.5,-.25,0),
				AuPoint(0,0,.75),
				AuPoint(-1,.75,1),
				AuPoint(1,.75,1),
				AuPoint(0,-1,1),
				AuPoint(0,0,-1),
			};

			// Triangles of av[] vertex indices.
			int at[][3] =
			{
				{4,0,1},
				{4,1,3},
				{4,3,0},
				{5,1,2},
				{5,2,3},
				{5,3,1},
				{6,0,3},
				{6,2,0},
				{6,3,2},
				{7,0,2},
				{7,1,0},
				{7,2,1},
			};

			float shine = .8f;

			for(int it=0; it<12; it++)
			{
				// The AuTriangle constructor calculates the normal.
				AuTriangle triN(av[at[it][0]], av[at[it][1]], av[at[it][2]]);

				tri.m_Vert0 = AuGlobeVertTexNShine(triN.m_fV[0], QPointF(s5,v), triN.m_vN, shine);
				tri.m_Vert1 = AuGlobeVertTexNShine(triN.m_fV[1], QPointF(u,t), triN.m_vN, shine);
				tri.m_Vert2 = AuGlobeVertTexNShine(triN.m_fV[2], QPointF(s,t), triN.m_vN, shine);
				rockTris.append(tri);
			}
		}
		break;
	case CID_Eludium:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Eludium, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Gems:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Gems, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Vertices
			AuPoint av[] = 
			{
				AuPoint(-.875,0,0),
				AuPoint(-.375,.75,0),
				AuPoint(.375,.75,0),
				AuPoint(.875,0,0),
				AuPoint(.375,-.75,0),
				AuPoint(-.375,-.75,0),
				AuPoint(0,0,.75),
				AuPoint(0,0,-1.25),
			};

			// Triangles of av[] vertex indices.
			int at[][3] =
			{
				{6,0,1},
				{6,1,2},
				{6,2,3},
				{6,3,4},
				{6,4,5},
				{6,5,0},
				{7,0,5},
				{7,1,0},
				{7,2,1},
				{7,3,2},
				{7,4,3},
				{7,5,4},
			};

			float shine = 75.f;

			for(int it=0; it<12; it++)
			{
				// The AuTriangle constructor calculates the normal.
				AuTriangle triN(av[at[it][0]], av[at[it][1]], av[at[it][2]]);

				tri.m_Vert0 = AuGlobeVertTexNShine(triN.m_fV[0], QPointF(s5,v), triN.m_vN, shine);
				tri.m_Vert1 = AuGlobeVertTexNShine(triN.m_fV[1], QPointF(u,t), triN.m_vN, shine);
				tri.m_Vert2 = AuGlobeVertTexNShine(triN.m_fV[2], QPointF(s,t), triN.m_vN, shine);
				rockTris.append(tri);
			}
		}
		break;
	case CID_Ice:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Ice, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			float shine = 100.f;

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);
		}
		break;
	case CID_Minerals:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Minerals, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			float shine = .1f;

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);
		}
		break;
	case CID_Ore:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Ore, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			float shine = .1f;

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);
		}
		break;
	case CID_GoldOre:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_GoldOre, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			float shine = .1f;

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);
		}
		break;
	case CID_Radioactives:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Radioactives, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			float shine = .2f;

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], shine);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], shine);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], shine);
			rockTris.append(tri);
		}
		break;
	case CID_Lumenite:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Lumenite, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Vertices
			AuPoint av[] = 
			{
				AuPoint(-.5,-.25,0),
				AuPoint(0,.5,0),
				AuPoint(.5,-.25,0),
				AuPoint(0,0,.75),
				AuPoint(-1,.75,1),
				AuPoint(1,.75,1),
				AuPoint(0,-1,1),
				AuPoint(0,0,-1),
			};

			// Triangles of av[] vertex indices.
			int at[][3] =
			{
				{4,0,1},
				{4,1,3},
				{4,3,0},
				{5,1,2},
				{5,2,3},
				{5,3,1},
				{6,0,3},
				{6,2,0},
				{6,3,2},
				{7,0,2},
				{7,1,0},
				{7,2,1},
			};

			float shine = .8f;

			for(int it=0; it<12; it++)
			{
				// The AuTriangle constructor calculates the normal.
				AuTriangle triN(av[at[it][0]], av[at[it][1]], av[at[it][2]]);

				tri.m_Vert0 = AuGlobeVertTexNShine(triN.m_fV[0], QPointF(s5,v), triN.m_vN, shine);
				tri.m_Vert1 = AuGlobeVertTexNShine(triN.m_fV[1], QPointF(u,t), triN.m_vN, shine);
				tri.m_Vert2 = AuGlobeVertTexNShine(triN.m_fV[2], QPointF(s,t), triN.m_vN, shine);
				rockTris.append(tri);
			}
		}
		break;
	case CID_CrudeOil:	// Oil shale
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_CrudeOil, s,t,u,v);
			float tt = t + (v - t)*.75;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.82, -0.42, 0.05),
				AuPoint(-0.29, 0.55, 0.06),
				AuPoint(0.73, 0.52, 0.05),
				AuPoint(0.73, -0.7, 0.03),
				AuPoint(-0.77, -0.17, -.05),
				AuPoint(-0.43, 0.82, -0.04),
				AuPoint(0.93, 0.62, -0.05),
				AuPoint(0.23, -0.52, -0.04),
			};
			AuPoint an[] = 
			{
				AuPoint(.01, -.01, .99),
				AuPoint(.01, -.02, .98),
				AuPoint(-.01, .01, -.99),
				AuPoint(.01, -.01, -.98),
				AuPoint(-.873,.436,.218),
				AuPoint(.071,.705,.705),
				AuPoint(.743,-.371,-.557),
				AuPoint(-.236,-.943,-.236),
			};

			// top
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[0], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[1], 0);
			rockTris.append(tri);

			// bottom
			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,v), an[2], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[7], QPointF(u,v), an[3], 0);
			rockTris.append(tri);

			// left
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			// back
			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[5], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			// right
			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			// front
			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[7], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Phlogiston:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Phlogiston, s,t,u,v);
			float tt = t + (v - t)*.75;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.82, -0.42, 0.05),
				AuPoint(-0.29, 0.55, 0.06),
				AuPoint(0.73, 0.52, 0.05),
				AuPoint(0.73, -0.7, 0.03),
				AuPoint(-0.77, -0.17, -.05),
				AuPoint(-0.43, 0.82, -0.04),
				AuPoint(0.93, 0.62, -0.05),
				AuPoint(0.23, -0.52, -0.04),
			};
			AuPoint an[] = 
			{
				AuPoint(.01, -.01, .99),
				AuPoint(.01, -.02, .98),
				AuPoint(-.01, .01, -.99),
				AuPoint(.01, -.01, -.98),
				AuPoint(-.873,.436,.218),
				AuPoint(.071,.705,.705),
				AuPoint(.743,-.371,-.557),
				AuPoint(-.236,-.943,-.236),
			};

			// top
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[0], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[1], 0);
			rockTris.append(tri);

			// bottom
			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,v), an[2], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[7], QPointF(u,v), an[3], 0);
			rockTris.append(tri);

			// left
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			// back
			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[5], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			// right
			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			// front
			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[7], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Polytaride:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Polytaride, s,t,u,v);
			float tt = t + (v - t)*.75;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.82, -0.42, 0.05),
				AuPoint(-0.29, 0.55, 0.06),
				AuPoint(0.73, 0.52, 0.05),
				AuPoint(0.73, -0.7, 0.03),
				AuPoint(-0.77, -0.17, -.05),
				AuPoint(-0.43, 0.82, -0.04),
				AuPoint(0.93, 0.62, -0.05),
				AuPoint(0.23, -0.52, -0.04),
			};
			AuPoint an[] = 
			{
				AuPoint(.01, -.01, .99),
				AuPoint(.01, -.02, .98),
				AuPoint(-.01, .01, -.99),
				AuPoint(.01, -.01, -.98),
				AuPoint(-.873,.436,.218),
				AuPoint(.071,.705,.705),
				AuPoint(.743,-.371,-.557),
				AuPoint(-.236,-.943,-.236),
			};

			// top
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[0], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[1], 0);
			rockTris.append(tri);

			// bottom
			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,v), an[2], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[7], QPointF(u,v), an[3], 0);
			rockTris.append(tri);

			// left
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			// back
			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[5], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			// right
			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			// front
			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[7], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Bolite:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Bolite, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Vulcanite:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Vulcanite, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.72, -0.22, -0.05),		//
				AuPoint(-0.69, 0.65, 0.56),			//
				AuPoint(0.73, 0.22, 0.05),			//
				AuPoint(0.13, -0.6, -0.05),			//
				AuPoint(-0.07, 0.17, 1.1),			//
				AuPoint(-0.13, 0.22, -0.75),		//
			};
			AuPoint an[] = 
			{
				AuPoint(-0.889, -0.299, -0.348),	//
				AuPoint(-0.633, 0.759, 0.152),		//
				AuPoint(0.96, 0.28, 0.0),			//
				AuPoint(0.368, -0.93, 0.0),			//
				AuPoint(0.0, 0.0, 1.0),				//
				AuPoint(0.0, 0.0, -1.0),			//
			};

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[4], QPointF(u,t), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s5,v), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[0], QPointF(s,t), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s5,v), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,t), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s5,v), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[2], QPointF(s,t), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s5,v), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,t), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,t), an[5], 0);
			rockTris.append(tri);
		}
		break;
	case CID_Adamantite:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Adamantite, s,t,u,v);
			float s5 = s + (u - s)*.5;

			// Vertices
			AuPoint av[] = 
			{
				AuPoint(-.5,-.25,0),
				AuPoint(0,.5,0),
				AuPoint(.5,-.25,0),
				AuPoint(0,0,.75),
				AuPoint(-1,.75,1),
				AuPoint(1,.75,1),
				AuPoint(0,-1,1),
				AuPoint(0,0,-1),
			};

			// Triangles of av[] vertex indices.
			int at[][3] =
			{
				{4,0,1},
				{4,1,3},
				{4,3,0},
				{5,1,2},
				{5,2,3},
				{5,3,1},
				{6,0,3},
				{6,2,0},
				{6,3,2},
				{7,0,2},
				{7,1,0},
				{7,2,1},
			};

			float shine = .8f;

			for(int it=0; it<12; it++)
			{
				// The AuTriangle constructor calculates the normal.
				AuTriangle triN(av[at[it][0]], av[at[it][1]], av[at[it][2]]);

				tri.m_Vert0 = AuGlobeVertTexNShine(triN.m_fV[0], QPointF(s5,v), triN.m_vN, shine);
				tri.m_Vert1 = AuGlobeVertTexNShine(triN.m_fV[1], QPointF(u,t), triN.m_vN, shine);
				tri.m_Vert2 = AuGlobeVertTexNShine(triN.m_fV[2], QPointF(s,t), triN.m_vN, shine);
				rockTris.append(tri);
			}
		}
		break;
	case CID_Viathol:
		{
			// Texture parameters.
			float s,t,u,v;
			DpRock::TextureCoords(CID_Viathol, s,t,u,v);
			float tt = t + (v - t)*.75;

			// Normals and corresponding vertices.
			AuPoint av[] = 
			{
				AuPoint(-0.82, -0.42, 0.05),
				AuPoint(-0.29, 0.55, 0.06),
				AuPoint(0.73, 0.52, 0.05),
				AuPoint(0.73, -0.7, 0.03),
				AuPoint(-0.77, -0.17, -.05),
				AuPoint(-0.43, 0.82, -0.04),
				AuPoint(0.93, 0.62, -0.05),
				AuPoint(0.23, -0.52, -0.04),
			};
			AuPoint an[] = 
			{
				AuPoint(.01, -.01, .99),
				AuPoint(.01, -.02, .98),
				AuPoint(-.01, .01, -.99),
				AuPoint(.01, -.01, -.98),
				AuPoint(-.873,.436,.218),
				AuPoint(.071,.705,.705),
				AuPoint(.743,-.371,-.557),
				AuPoint(-.236,-.943,-.236),
			};

			// top
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[0], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[1], QPointF(s,v), an[0], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[0], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[1], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[3], QPointF(s,v), an[1], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[1], 0);
			rockTris.append(tri);

			// bottom
			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[2], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[2], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[5], QPointF(u,v), an[2], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[3], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[3], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[7], QPointF(u,v), an[3], 0);
			rockTris.append(tri);

			// left
			tri.m_Vert0 = AuGlobeVertTexNShine(av[0], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[4], QPointF(s,tt), an[4], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[4], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[1], QPointF(u,v), an[4], 0);
			rockTris.append(tri);

			// back
			tri.m_Vert0 = AuGlobeVertTexNShine(av[1], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[5], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[5], QPointF(s,tt), an[5], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[5], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[2], QPointF(u,v), an[5], 0);
			rockTris.append(tri);

			// right
			tri.m_Vert0 = AuGlobeVertTexNShine(av[2], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[6], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[6], QPointF(s,tt), an[6], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[6], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[3], QPointF(u,v), an[6], 0);
			rockTris.append(tri);

			// front
			tri.m_Vert0 = AuGlobeVertTexNShine(av[3], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[7], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);

			tri.m_Vert0 = AuGlobeVertTexNShine(av[7], QPointF(s,tt), an[7], 0);
			tri.m_Vert1 = AuGlobeVertTexNShine(av[4], QPointF(s,v), an[7], 0);
			tri.m_Vert2 = AuGlobeVertTexNShine(av[0], QPointF(u,v), an[7], 0);
			rockTris.append(tri);
		}
		break;
	}

	// Store count of triangles in the far globe vertex array object. This value is needed when displaying the VAO.
	GLuint vertices = rockTris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];				// vertices for triangles that make up the model.
	GLfloat* globeN = new GLfloat[vertices*3];					// normals for vertices that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*3];				// texture coords and shininess

	unsigned int globePosIdx = 0;
	unsigned int globeNIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTexNShine>::const_iterator vi = rockTris.cbegin(); vi != rockTris.cend(); vi++)
	{
		// Triangle vertex 0
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_Shininess;

		// Triangle vertex 1
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_Shininess;

		// Triangle vertex 2
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_Shininess;
	}

	// Generate vertex array(s) for this object. We only need one.
	vao.CreateGL(GLf, vertices, 3);

	// Select vertex array to construct.
	vao.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for tex coordinates and shininess and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	vao.DeselectGL();

	delete [] globePos;
	delete [] globeN;
	delete [] globeTex;
	//qDebug() << "DaWorldGlobe::CreateNearGlobeVAO" << Id() << Name() << "created" << tris.count() << "triangles";
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAtGlobeVAO::InvalidateGlobeTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm)
{
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_TriVAOs.find(triIndex);
	if(vi != m_TriVAOs.end())
		(*vi).second.SetValid(false);

	mm.InvalidateGlobeModelTri(worldId, triIndex);
}

void AuAtGlobeVAO::InvalidateRingworldSectionTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm)
{
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_TriVAOs.find(triIndex);
	if(vi != m_TriVAOs.end())
		(*vi).second.SetValid(false);

	// Delete the model from the model maker thread.
	mm.InvalidateRingworldSectionModelTri(worldId, triIndex);
}

void AuAtGlobeVAO::DeleteGlobeTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm)
{
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_TriVAOs.find(triIndex);
	QSet<int>::iterator wi = m_TriWait.find(triIndex);
	if(vi != m_TriVAOs.end() || wi != m_TriWait.end())
	{
		// Delete the model from the model maker thread.
		mm.DeleteGlobeModelTri(worldId, triIndex);

		// Remove the tri VAO from my list.
		if(vi != m_TriVAOs.end())
			m_TriVAOs.erase(vi);

		if(wi != m_TriWait.end())
			m_TriWait.erase(wi);
	}
}

void AuAtGlobeVAO::DeleteRingworldSectionTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm)
{
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_TriVAOs.find(triIndex);
	QSet<int>::iterator wi = m_TriWait.find(triIndex);
	if(vi != m_TriVAOs.end() || wi != m_TriWait.end())
	{
		// Delete the model from the model maker thread.
		mm.DeleteRingworldSectionModelTri(worldId, triIndex);

		// Remove the tri VAO from my list.
		if(vi != m_TriVAOs.end())
			m_TriVAOs.erase(vi);

		if(wi != m_TriWait.end())
			m_TriWait.erase(wi);
	}
}

void AuAtGlobeVAO::DeleteOldGlobeTriVAOs(const AuId& worldId, AuGLModelMaker& mm)
{
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_TriVAOs.begin();
	while(vi != m_TriVAOs.end())
	{
		if((*vi).second.IsOld())
		{
			//qDebug() << "AuAtGlobeVAO::DeleteOldGlobeTriVAOs index" << (*vi).first;
			// Delete the model from the model maker thread.
			mm.DeleteGlobeModelTri(worldId, (*vi).first);

			// Remove the tri VAO from my list.
			vi = m_TriVAOs.erase(vi);
		}
		else
			vi++;
	}
}

void AuAtGlobeVAO::DeleteOldRingworldSectionTriVAOs(const AuId& worldId, AuGLModelMaker& mm)
{
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_TriVAOs.begin();
	while(vi != m_TriVAOs.end())
	{
		if((*vi).second.IsOld())
		{
			//qDebug() << "AuAtGlobeVAO::DeleteOldRingworldSectionTriVAOs index" << (*vi).first;
			// Delete the model from the model maker thread.
			mm.DeleteRingworldSectionModelTri(worldId, (*vi).first);

			// Remove the tri VAO from my list.
			vi = m_TriVAOs.erase(vi);
		}
		else
			vi++;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaWorldLL::DaWorldLL()
{
	m_AsphaltTex = 0;
	m_BdBits = 0x80 | 0x20 | 0x08;		// Unknown if any bds are here.
	m_ClaimEmpire = 0;
	m_ClaimFlagTex = 0;
	m_ClaimFlagTexEmpire = 0;
	m_ClaimPoleTex = 0;
	m_ConcreteTex = 0;
	m_Enclosures = 0;
	m_EnclosuresValid = false;
	m_FindTerrainAtPixelDetailsOk = false;
	m_GeoConstruction = -1;
	m_HengePresence = -1;
	m_HengeFire = 0;
	m_Irrigation = 0;
	m_LandingSites = 0;
	m_RoadMap = new AuRoadMap();
	m_SightDetailsOk = false;
	m_SightIntersect = 0;
	m_Village = 0;
}

DaWorldLL::~DaWorldLL()
{
	m_GeologicalFeatures.Clear();

	if(m_LandingSites)
		delete [] m_LandingSites;

	if(m_Enclosures)
		delete m_Enclosures;

	if(m_HengeFire)
		delete m_HengeFire;

	if(m_Irrigation)
		delete m_Irrigation;

	for(QMap<AuGlobeLLF, AuSmokeParticles*>::iterator si = m_PSSmoke.begin(); si != m_PSSmoke.end(); si++)
		if(*si)
			delete *si;
	m_PSSmoke.clear();

	delete m_RoadMap;

	if(m_Village)
		delete m_Village;
}

DaWorldLL::DaWorldLL(const DaWorldLL& c) : DaWorld(c)
{
	m_BdBits = c.m_BdBits;
	m_GeoConstruction = c.m_GeoConstruction;
	m_HengePresence = c.m_HengePresence;

	c.m_LLLock.lockForRead();
	m_CaptId = c.m_CaptId;
	m_CaptName = c.m_CaptName;
	m_CitizenSuppliers = c.m_CitizenSuppliers;
	m_ClaimAtLL = c.m_ClaimAtLL;
	m_ClaimEmpire = c.m_ClaimEmpire;
	m_ClaimPersonId = c.m_ClaimPersonId;
	m_ClaimPersonName = c.m_ClaimPersonName;
	m_ClaimTime = c.m_ClaimTime;
	for(AuGeologicalFeatures::const_iterator ci = c.m_GeologicalFeatures.cbegin(); ci != c.m_GeologicalFeatures.cend(); ci++)
		m_GeologicalFeatures.append((*ci)->Clone());
	m_RoadMap = new AuRoadMap(*c.m_RoadMap);
	m_tmGeologyChange = c.m_tmGeologyChange;
	m_TroopSuppliers = c.m_TroopSuppliers;
	m_FindTerrainAtPixelDetailsOk = c.m_FindTerrainAtPixelDetailsOk;
	m_FindTerrainAtPixelEye = c.m_FindTerrainAtPixelEye;
	m_FindTerrainAtPixelLook = c.m_FindTerrainAtPixelLook;
	m_FindTerrainAtPixelResult = c.m_FindTerrainAtPixelResult;
	m_SightDetailsOk = c.m_SightDetailsOk;
	m_SightEye = c.m_SightEye;
	m_SightIntersect = c.m_SightIntersect;
	m_SightLook = c.m_SightLook;
	m_SightedLoc = c.m_SightedLoc;
	c.m_LLLock.unlock();

	c.m_BuildingMapLock.lock();
	m_BuildingMap = c.m_BuildingMap;
	c.m_BuildingMapLock.unlock();

	c.m_TroopsAtStationsLock.lockForRead();
	m_TroopsAtStations = c.m_TroopsAtStations;
	c.m_TroopsAtStationsLock.unlock();

	c.m_EnclosureLock.lockForRead();
	if(c.m_Enclosures)
		m_Enclosures = new AuWorldEnclosureCache(*c.m_Enclosures);
	else
		m_Enclosures = 0;
	m_EnclosuresValid = c.m_EnclosuresValid;
	if(c.m_Irrigation)
		m_Irrigation = new QList<AuWorldIrrigation>(*c.m_Irrigation);
	else
		m_Irrigation = 0;
	c.m_EnclosureLock.unlock();

	c.m_VillageLock.lockForRead();
	if(c.m_Village)
		m_Village = new AuVillage(*c.m_Village);
	else
		m_Village = 0;
	c.m_VillageLock.unlock();

	m_AsphaltTex = 0;
	m_ClaimFlagTex = 0;
	m_ClaimFlagTexEmpire = 0;
	m_ClaimPoleTex = 0;
	m_ConcreteTex = 0;
	m_HengeFire = 0;
	m_LandingSites = 0;

	m_OverlayRocks = c.m_OverlayRocks;
	m_OverlayRocksLoc = c.m_OverlayRocksLoc;
	m_OverlayShrubs = c.m_OverlayShrubs;
	m_OverlayShrubsLoc = c.m_OverlayShrubsLoc;
	m_OverlayTrees = c.m_OverlayTrees;
	m_OverlayTreesLoc = c.m_OverlayTreesLoc;
}

// Send a report to chain-of-command of all cities when the local ship's officer is available.
void DaWorldLL::ReportOfficerAvailable()
{
	//Q_ASSERT(m_CaptId.IsNull());
	// Make a list of the empires who have cities on this planet, with a starfleet academy or university.
	QHash<quint32, const DaBd*> empireBds;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ci = subs.cbegin(); ci != subs.cend(); ci++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*ci);
		if(bd)
		{
			if(bd->IndustryId() != IID_BaseAcademy && bd->IndustryId() != IID_School)
				continue;
			quint32 empireId = bd->EmpireId();

			// Pirate cities do not send mail.
			bool isPirateEmpire = DgEmpire::IsPirate(empireId);
			if(isPirateEmpire)
				continue;

			// Check to see if we have already found an academy for the same empire.
			QHash<quint32, const DaBd*>::iterator ai = empireBds.find(empireId);
			if(ai != empireBds.end() && (*ai)->Residents() > bd->Residents())
				continue;									// Skip academy of same empire with lower population.

			// Insert/replace the academy for the empire.
			empireBds.insert(empireId, bd);
		}
	}

	// Make a list of the people in the chain-of-command of every empire that has a city on this planet.
	AuIdList placeChain;
	ChainOfPlaces(placeChain);

	QHash<AuId, const DaBd*> notifyBds;
	for(QHash<quint32, const DaBd*>::const_iterator ei = empireBds.cbegin(); ei != empireBds.cend(); ei++)
	{
		// Get a list of people in the empire who can load officers from here.
		AuIdList recipients;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireDomainOfficerLoaders(ei.key(), placeChain, recipients);
		g_Au.UnlockData();
		for(AuIdList::const_iterator ri = recipients.cbegin(); ri != recipients.cend(); ri++)
			notifyBds.insert(*ri, *ei);
	}

	// Now we have our list of person atoms to notify.
	if(!notifyBds.isEmpty())
	{
		// Tell about the officer.
		m_LLLock.lockForRead();
		if(m_CaptName.isEmpty())
		{
			m_LLLock.unlock();

			m_LLLock.lockForWrite();
			// Make a name for the next captain but don't set an id until he is actually spawned in DaVessel::BoardOfficer.
			m_CaptName = g_Au.Dice().RandomPersonName();
			SetModified(NOTIFY_Database);
		}
		QString captName = m_CaptName;
		m_LLLock.unlock();

		// I need this server's login credentials to send mail.
		const DgData& gd = g_Au.LockDataForRead();
		unsigned int idPlayer = gd.PlayerId();  // The local server must be "logged in" as someone to send mail.
		unsigned int confirmNum = gd.ConfirmationNumber();
		g_Au.UnlockData();

		// I must compose a message to each recipient so the academy city can be listed in the mail header.
		const DaSolarSystem* system = FindParentSolarSystem();
		const DaSector* sector = FindParentSector();
		for(QHash<AuId, const DaBd*>::const_iterator ni = notifyBds.cbegin(); ni != notifyBds.cend(); ni++)
		{
			// Compose a message.
			QString msg;
			if((*ni)->IsMilitary())
			{
				msg = g_Au.EncodeUTCString() + "<br><br>";
				msg += HtmlName(tr("Cadet %1").arg(captName));

				msg += "<div style=\"margin-left: 10px;\">";
				msg += tr("Star Fleet Academy");

				msg += "<br>" + HtmlName((*ni)->CityBuildingName()) + " at " + (*ni)->GlobeLL().ToString();		// Bd

				msg += "<br>" + HtmlName() + ", " + Description();		// Planet

				if(system && !system->IsEmptySystem())
					msg += "<br>" + system->HtmlNameWithSystem() + ' ' + system->Description();

				if(sector)
					msg += "<br>" + sector->HtmlName() + ' ' + sector->Description();   // Sector

				const DaGalaxy* galaxy = FindParentGalaxy();
				if(galaxy)
					msg += "<br>" + DaAtom::HtmlName(galaxy->Name());

				msg += "</div>";

				msg += tr("<br>Re: Assignment Request on %1<br><br>"
					"Commander,<br><br>"
					"I have graduated from spacecraft officer training at the top of my class.<br><br>"
					"Place me in command of a starship. "
					"I won't let you down.<br><br>"
					"Cordially,<br>"
					"<i>%2</i>").arg(HtmlName()).arg(captName);
			}
			else
			{
				msg = g_Au.EncodeUTCString() + "<br><br>";
				msg += HtmlName(tr("Graduate %1").arg(captName));

				msg += "<div style=\"margin-left: 10px;\">";
				msg += tr("University");

				msg += "<br>" + HtmlName((*ni)->CityBuildingName()) + " at " + (*ni)->GlobeLL().ToString();		// Bd

				msg += "<br>" + HtmlName() + ", " + Description();		// Planet

				if(system && !system->IsEmptySystem())
					msg += "<br>" + system->HtmlNameWithSystem() + ' ' + system->Description();

				if(sector)
					msg += "<br>" + sector->HtmlName() + ' ' + sector->Description();   // Sector

				const DaGalaxy* galaxy = FindParentGalaxy();
				if(galaxy)
					msg += "<br>" + DaAtom::HtmlName(galaxy->Name());

				msg += "</div>";

				msg += tr("<br>Re: Assignment Request on %1<br><br>"
					"Sir,<br><br>"
					"I graduated from spacecraft officer training at the top of my class.<br><br>"
					"Place me in command of a starship. "
					"I won't let you down.<br><br>"
					"Cordially,<br>"
					"<i>%2</i>").arg(HtmlName()).arg(captName);
			}

			// Send a mail message to each of them.
			AuPoint galaxyLoc;
			const DaGalaxy* pg = system->FindParentGalaxy();
			if(pg)
			{
				system->GetLocationRelativeToParent(galaxyLoc, pg);
				g_Au.SendMail(idPlayer, confirmNum, Id(), captName, ni.key(), AuMailMsg::MSG_OfficerReady,
						tr("Assignment Request on %1").arg(Name()), msg, system->Id(), system->Name(), Id(), Name(), pg->Galaxy(),
						galaxyLoc);
			}
		}
	}
}

unsigned int DaWorldLL::IsTocker() const
{
	unsigned int ms = DaWorld::IsTocker();
	if(!IsNormal())
		return ms;
	if(ms == 0 || ms > d_MSecPerHr)
		ms = d_MSecPerHr;
	if(ms > 10*d_MSecPerSec)
		if(HasGeoConstruction())
			ms = 10*d_MSecPerSec;
	return ms;
}

void DaWorldLL::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaWorld::Tock(last, now, scripts, tockTrans);

	// Calc bd and city presense. Bd and City presense determined/cached by BdHere().
	if(BdHere() &&
			g_Au.m_EnableReportOfficers &&
			CaptId().IsNull() &&
			IsHabitable(HABITABLE_System) &&
			last.IsNewPeriod(now, d_MSecPerHr))
	{
		ReportOfficerAvailable();	    // Habitable world or ringworld has an officer ready. Send a notice once per hour.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(last.IsNewPeriod(now, 10*d_MSecPerSec, Id().GetID()))
	{
		ConstructGeo();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

bool DaWorldLL::MaybeBaseHere() const
{
	return m_BdBits & 0xc0;
}

bool DaWorldLL::BaseHere() const
{
	if(m_BdBits & 0x80)
		CalcBasePresence();
	return m_BdBits & 0x40;
}

bool DaWorldLL::BdHere() const
{
	if(m_BdBits & 0x08)
		CalcBdPresence();
	return m_BdBits & 0x04;
}

bool DaWorldLL::MaybeHQHere() const
{
	return m_BdBits & 0x30;
}

bool DaWorldLL::HQHere() const
{
	if(m_BdBits & 0x20)
		CalcHQPresence();
	return m_BdBits & 0x10;
}

void DaWorldLL::CalcBdPresence() const
{
	bool isHere = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->IsDying())
			continue;				// Bd is about to decay from existence.
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(bd)
		{
			isHere = true;
			break;
		}
	}
	if(isHere)
		m_BdBits |= 0x04;		// Turn on bd present bit.
	else
		m_BdBits &= ~0x04;		// Turn off bd present bit.
	m_BdBits &= ~0x08;			// Turn off bd unknown bit.
}

void DaWorldLL::CalcHQPresence() const
{
	bool isHere = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->IsDying())
			continue;				// Bd is about to decay from existence.
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(bd && bd->IndustryId() == IID_BaseHeadquarters)
		{
			isHere = true;
			break;
		}
	}
	if(isHere)
		m_BdBits |= 0x10;		// Turn on hq present bit.
	else
		m_BdBits &= ~0x10;		// Turn off hq present bit.
	m_BdBits &= ~0x20;			// Turn off hq unknown bit.
}

void DaWorldLL::CalcBasePresence() const
{
	bool isHere = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->IsDying())
			continue;				// Bd is about to decay from existence.
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(bd && bd->IndustryId() != IID_BaseHeadquarters && bd->IsMilitary())
		{
			isHere = true;
			break;
		}
	}
	if(isHere)
		m_BdBits |= 0x40;		// Turn on base present bit.
	else
		m_BdBits &= ~0x40;		// Turn off base present bit.
	m_BdBits &= ~0x80;			// Turn off base unknown bit.
}

quint32 DaWorldLL::SitePrepLabor(INDUSTRYID iid, const AuPoint& roadStart, const AuPoint& roadEnd, double roadWidth,
		quint32& paveMaterial) const
{
	// Guess at an amount of labor based on the amount of terrain deviation down the center line.
	double rAlt = AltitudeMSL(roadEnd);
	double startAlt = AltitudeMSL(roadStart);
	AuPoint centerVec = roadEnd - roadStart;
	double centerLen = centerVec.Length();
	Q_ASSERT(centerLen != 0);
	centerVec /= centerLen;

	// Calculate the amount of material based on area of road.
	const float areaPerUnit = 15000;
	float area = centerLen*2 * roadWidth;
	paveMaterial = (int)(area / areaPerUnit);
	if(!paveMaterial)
		paveMaterial = 1;									// Minimum 1 unit to pave.

	// Integrate along the length so we conform to the curvature of the world.
	const double seglen = 15;
	int segs = (int)(centerLen / seglen) + 1;				// Do one too many to insure perfect ending.
	bool atEnd = false;
	double altAccum = 0;									// Used to determine steepness of each segment.
	int locsAccum = 0;										// Count of locations accumulated into altAccum.
	//qDebug() << "DaWorldLL::AddRoad" << Id() << Name() << "startAlt" << startAlt << "endAlt" << rAlt << "seglen" << seglen
		//<< "segs" << segs;
	for(int seg=0; seg<=segs && !atEnd; seg++)
	{
		double lenAlongSeg = seglen * seg;
		if(lenAlongSeg >= centerLen)
		{
			// We are at the end. Truncate the last segment for perfect ending.
			//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
			lenAlongSeg = centerLen;				
			atEnd = true;
		}
		double segRatio = lenAlongSeg / centerLen;
		double altAtSegLoc = startAlt + (rAlt - startAlt) * segRatio;		// Linear alt change from one end to the other.

		// Calculate altitude at road center line location.
		AuPoint roadCenter = roadStart + centerVec * lenAlongSeg;
		AuGlobeLL roadCenterLL;
		LLOfLoc(roadCenterLL, roadCenter);

		double terAltMSL = TerrainAltitudeMSL(roadCenterLL);		// Terrain MSL altitude under road center location.

		// Determine the amount of deviation from the underlying terrain.
		double altDev = altAtSegLoc - terAltMSL;	// Deviation from terrain MSL altitude.
		//qDebug() << "DaWorldLL::AddRoad" << Id() << Name() << "lenAlongSeg" << lenAlongSeg << "segRatio" << segRatio
			//<< "altAtSegLoc" << altAtSegLoc << "terAltMSL" << terAltMSL << "altDev" << altDev;
		altAccum += fabs(altDev);
		locsAccum++;
	}

	quint32 labor;
	if(iid == IID_Landscape)
	{
		// Landscaping an area takes labor based on surface area.
		double area = roadWidth * centerLen;	// Close enough wag at the area.
		labor = (quint32)ceil(area/100000);
	}
	else
	{
		// Take a rough calculation at the volume of dirt that has to be moved, up or down.
		double vol = ((altAccum/locsAccum) * roadWidth * centerLen)/2;	// Close enough wag at the volume.
		labor = (quint32)ceil(vol/100000);
	}
	if(!labor)
		labor = 1;
	return labor;
}

int DaWorldLL::InEnclosure(const AuGlobeLL& ll, double altMSL) const
{
	Q_UNUSED(ll);
	Q_UNUSED(altMSL);
	if(!HasEnclosure())
		return 0;	// I have no Enclosures.

	AuPoint worldLoc = LocOfLL(ll, altMSL);
	int insideEnclosure;
	m_EnclosureLock.lockForRead();
	if(m_Enclosures)
		insideEnclosure = m_Enclosures->Contains(worldLoc);
	else
		insideEnclosure = 0;
	m_EnclosureLock.unlock();
	return insideEnclosure;
}

int DaWorldLL::InEnclosureExcept(const AuGlobeLL& ll, double altMSL, const AuId& exceptId, AuId& enclosureId) const
{
	Q_UNUSED(ll);
	Q_UNUSED(altMSL);
	if(!HasEnclosure())
		return 0;	// I have no Enclosures.

	AuPoint worldLoc = LocOfLL(ll, altMSL);
	int insideEnclosure;
	m_EnclosureLock.lockForRead();
	if(m_Enclosures)
		insideEnclosure = m_Enclosures->ContainsExcept(worldLoc, exceptId, enclosureId);
	else
		insideEnclosure = 0;
	m_EnclosureLock.unlock();
	return insideEnclosure;
}

int DaWorldLL::IsIrrigated(const AuGlobeLL& ll) const
{
	if(!m_Irrigation)
	{
		//qDebug() << "DaWorldLL::IsIrrigated" << Id() << Name() << "no irrigation cache";
		return 0;	// I have no irrigation.
	}

	int isIrrigated = 0;
	m_EnclosureLock.lockForRead();
	for(QList<AuWorldIrrigation>::const_iterator di = m_Irrigation->cbegin(); di != m_Irrigation->cend(); di++)
	{
		if(AuRoadSegment::Contains(*this, AuGlobeLLF(ll.Longitude(), ll.Latitude()), (*di).StartLL(), (*di).StartAltitudeMSL(),
					(*di).EndLL(), (*di).EndAltitudeMSL(), (*di).Width2(), 0))
		{
			//qDebug() << "DaWorldLL::IsIrrigated" << Id() << Name() << "irrigated at" << ll;
			isIrrigated = (*di).Type();	// 1=irrigation, 2=defoliant
			break;
		}
	}
	m_EnclosureLock.unlock();
	return isIrrigated;
}

//static
GLuint DaWorldLL::CreateTerrainTexture(AuGLEffects& re, WORLDTEX_TYPE worldTexType, int tileSize)
{
	// Each tile in the terrain palette must be loaded separately due to size limitation of the windows resource compiler.
	// NOTE: These MUST BE in the same order as the WORLDTEX_ enum.
	const char* tileFile[WORLDTEX_Count] =
	{
		// Pavements
		"ConstructionZone.png",									// WORLDTEX_ConstructionZone
		"ConcreteHeavy.png",									// WORLDTEX_ConcreteHeavy
		"Tarmac.png",											// WORLDTEX_Tarmac
		"DirtSmooth.png",										// WORLDTEX_DirtSmooth
		"CobblestoneLight.png",									// WORLDTEX_CobblestoneLight

		// Land at site developments
		"DirtCrackedSmooth.png",								// WORLDTEX_DirtCrackedSmooth
		"RockTan.png",											// WORLDTEX_RockTan

		// Ice and Snow
		"SnowModerate.png",										// WORLDTEX_SnowModerate
		"IceModerate.png",										// WORLDTEX_IceModerate

		// Ground surface
		"ConcreteDarkModerate.png",								// WORLDTEX_ConcreteDarkModerate
		"DirtCoarse.png",										// WORLDTEX_DirtCoarse
		"DirtGray.png",											// WORLDTEX_DirtGray
		"DirtHeavy.png",										// WORLDTEX_DirtHeavy
		"DirtModerate.png",										// WORLDTEX_DirtModerate
		"Pebbles.png",											// WORLDTEX_Pebbles
		"RockBrown.png",										// WORLDTEX_RockBrown
		"RockBrownModerate.png",								// WORLDTEX_RockBrownModerate
		"RockGrayCoarse.png",									// WORLDTEX_RockGrayCoarse
		"RockGrayHeavy.png",									// WORLDTEX_RockGrayHeavy
		"RockGrayModerate.png",									// WORLDTEX_RockGrayModerate
		"RockGraySmooth.png",									// WORLDTEX_RockGraySmooth
		"RockRedHeavy.png",										// WORLDTEX_RockRedHeavy
		"RockRedModerate.png",									// WORLDTEX_RockRedModerate
		"RockRedSmooth.png",									// WORLDTEX_RockRedSmooth
		"SandSmooth.png",										// WORLDTEX_SandSmooth

		// Sea surfaces
		"Magma.png",											// WORLDTEX_Magma
		"SeaWater.png",											// WORLDTEX_SeaDark
		"SupercooledLiquidMedium.png",							// WORLDTEX_SupercooledLiquid
	};

	GLuint texId = 0;
	QString texName = QString("WTX%1.%2").arg(worldTexType).arg(tileSize);
	QImage imgTile(QString(":data/") + tileFile[worldTexType]);
	if(!imgTile.isNull())
	{
		QImage imgSized = imgTile.scaled(tileSize,tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		if(!imgSized.isNull())
		{
			texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, imgSized, GL_LINEAR, GL_LINEAR, true);
			if(!texId)
				qWarning() << "DaWorldLL::CreateTerrainTexture ERROR Failed to create texture" << tileFile[worldTexType];
			//else
				//qDebug() << "DaWorldLL::CreateTerrainTexture Created texture" << texId << "of" << tileFile[worldTexType];
		}
		else
			qWarning() << "DaWorldLL::CreateTerrainTexture ERROR Failed to resize" << tileFile[worldTexType] << tileSize;
	}
	else
		qWarning() << "DaWorldLL::CreateTerrainTexture ERROR Failed to load" << tileFile[worldTexType];
	return texId;
}

float DaWorldLL::BuildingAmbientLight(const AuGlobeLLF& ll) const
{
	// Calculate ambient light level at the rounded location.
	float cityAmbientLight = 0.f;		// This will be our return value.

	/*
	// Check for roads.
	AuGlobeLLF geoLL;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	INDUSTRYID iid = geof.FindGeoIndustryContaining(*this, ll, 5*FeetPerMeter(), geoLL);
	UnlockGeologicalFeatures();
	if(iid == IID_Road)
		cityAmbientLight = .15f;		// Prevents creatures and indigenous from spawning on roads.
		*/

	// Building maps tell something about nearby developments quickly.
	m_BuildingMapLock.lock();
	QMap<AuGlobeLLF, AuIdList>::const_iterator bmi = BuildingMapLocation(ll);
	cityAmbientLight += (*bmi).size() * .03f;
	m_BuildingMapLock.unlock();

	// Check for cities.
	// Cities may light the location with nearby buildings and roads.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaAtom* atom = *it;
		if(!atom)
			continue;
		cityAmbientLight += atom->AmbientLightEmitted(ll);
	}
	if(cityAmbientLight >= 1.f)
		cityAmbientLight = 1.f;
	return cityAmbientLight;
}

float DaWorldLL::CityAmbientLight(const AuGlobeLLF& ll, WORLDTEX_TYPE pavementType) const
{
	// Pavement type can tell us about ambient light in the area.
	if(pavementType <= WORLDTEX_CobblestoneLight)
		return 1.f;	// Hard pavement gets maximum ambient light.

	// Calculate ambient light level at the rounded location.
	float cityAmbientLight = 0.f;		// This will be our return value.

	/*
	// Check for roads.
	AuGlobeLLF geoLL;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	INDUSTRYID iid = geof.FindGeoIndustryContaining(*this, ll, 5*FeetPerMeter(), geoLL);
	UnlockGeologicalFeatures();
	if(iid == IID_Road)
		cityAmbientLight = .15f;		// Prevents creatures and indigenous from spawning on roads.
		*/

	// Building maps tell something about nearby developments quickly.
	m_BuildingMapLock.lock();
	QMap<AuGlobeLLF, AuIdList>::const_iterator bmi = BuildingMapLocation(ll);
	cityAmbientLight += (*bmi).size() * .03f;
	m_BuildingMapLock.unlock();

	// Check for cities.
	// Cities may light the location with nearby buildings and roads.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaAtom* atom = *it;
		if(!atom)
			continue;
		cityAmbientLight += atom->AmbientLightEmitted(ll);
	}
	if(cityAmbientLight >= 1.f)
		cityAmbientLight = 1.f;
	return cityAmbientLight;
}

DaDesignBase* DaWorldLL::DesignEnvironment(const AuGlobeLLF& devLL, int panelIndex)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
	{
		DaDesignBase* studio = dynamic_cast<DaDesignBase*>(*si);
		if(!studio)
			continue;
		if(studio->StudioId() == panelIndex)
		{
			// Found a studio. Is it in the right building?
			for(DaAtomList::const_iterator bi = subs.cbegin(); bi != subs.cend(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*bi);
				if(!bd)
					continue;
				if(bd->IndustryId() != IID_DesignStudio)
					continue;
				if(bd->GlobeLL() != devLL)
					continue;
				return studio;			// The studio is in the appropriate building.
			}
		}
	}
	return 0;
}

DaNpc* DaWorldLL::SpawnCitizenWorker(DaBd* bd, const AuGlobeLLF& spawnLL)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	unsigned int empireId = bd->EmpireId();
	if(!empireId)
		return 0;

	// Create a citizen atom using the worker DNA at the city location.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* worker = new DaNpc(ROLE_Worker, bd->ResidentSpecies(), chroma, hand, g_Au.Dice().RandomPersonName());
	worker->SetHomeWorld(this);
	worker->ConnectSignals();
	worker->Create();

	// Place the nu creature at exactly the right altitude.
	worker->Stand();
	AuPoint spawnAt = GetSurfaceLocation(spawnLL, worker->GroundHeightAboveCG());

	// Position the atom in the environment.
	AuMatrix4x4 t;
	t.Translate(spawnAt);	// Move to location in terrain.
	t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Align with up.

	// Rotate the atom randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	worker->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(worker);

	worker->ResetHitPoints();
	worker->ResetHunger();
	worker->ResetStamina();
	worker->SetEmpireId(empireId);		// Subjugate the worker.
	worker->InitTraining();

	AuItemHash items, itemsTaken;
	bd->CommunityInventoryAvailable(items, empireId);
	worker->InitGear(items, itemsTaken, true);
	worker->GoToWork();
	worker->SetInScene();			// Only spawns when in a scene. Set this so it doesn't suicide immediately.

	// Send them on their way with any souvenirs the city has to offer.
	//DHH Bd city->LockSituationsForRead().InflictCitizen(worker);
	//DHH Bd city->UnlockSituations();

	g_Au.AtomAdded(worker->Id());
	worker->SetNormal();	// New atom is ready for normal thread processing.
	worker->TickTock();
	bd->CommunityRemoveInventory(itemsTaken);
	return worker;
}

bool DaWorldLL::IsSpawnAreaObstructed(const AuPoint& sloc, double radius) const
{
	if(!HasSubatoms())
		return false;

	AuPoint loc(sloc);
	float altMSL = AltitudeMSL(sloc);
	float minConDepth = DeckHeight() * (2 + 1);
	if(Hydrographics() && altMSL > -minConDepth && altMSL < 0)
		ElevateLocMSL(loc, 0);	// Don't go under water except for under sea spawn.

	if(!HasSubatoms())
		return false;

	AuBox locExt;
	bool locExtValid = false;
	double radius2 = radius*radius;	// squared
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		// Units are the only atoms that prevent spawning of anything.
		DaUnit* unit = dynamic_cast<DaUnit*>(*ai);
		if(!unit)
			continue;
		if(!unit->IsNormal())
			continue;
		if(!unit->IsInAtmosphere())
			continue;

		const DaCreature* creature = dynamic_cast<const DaCreature*>(*ai);
		if(creature)
		{
			// Skip underground creatures.
			if(creature->IsUnderGround())
				continue;

			// Skip people who are not on line.
			const DaPerson* person = dynamic_cast<const DaPerson*>(*ai);
			if(person && person->IsInvisibleOffLine())
				continue;

			double dist2 = creature->Location().Distance2(loc);
			if(dist2 > radius2)
				continue;

			// Creature is in the spawn radius.
			return true;
		}

		bool blockedByUnit = false;

		// Test center point location of unit.
		double dist2 = unit->Location().Distance2(loc);
		if(dist2 < radius2)
		{
			// Unit center point location is within the search radius of loc.
			blockedByUnit = true;
		}
		else
		{
			// Units may be big/bulky. Check for intersection of extents boxes.
			AuBox unitExt;
			if(!unit->RoughExtents(unitExt))
				continue;					// Unit has no extents box? Go with the center test result alone.
			unitExt.Transform(unit->Transform());

			if(!locExtValid)
			{
				// Create an extents box for the spawn location.
				AuPoint vup = -GravityDir(loc);
				AuPoint vx;
				vup.ArbitraryAxis(vx);
				AuPoint vy = vup.Cross(vx);
				AuPoint max = vx*radius + vy*radius + vup*radius;
				AuPoint min = -max;
				max += loc;
				min += loc;
				locExt.SetFromMinMax(min, max);
				locExtValid = true;
			}

			if(locExt.DoesIntersect(unitExt))
				blockedByUnit = true;
		}

		if(blockedByUnit)
			return true;
	}

	// Here if we made it through the gauntlet without finding an obstruction.
	return false;
}

bool DaWorldLL::IsSpawnAreaObstructed(const AuPoint& sloc, quint32 designId, quint8 designQ, double roughRadius,
		DaVessel** replaceableShip) const
{
	*replaceableShip = 0;
	if(!HasSubatoms())
		return false;

	AuPoint loc(sloc);
	float altMSL = AltitudeMSL(sloc);
	float minConDepth = DeckHeight() * (2 + 1);
	if(Hydrographics() && altMSL > -minConDepth && altMSL < 0)
		ElevateLocMSL(loc, 0);	// Don't go under water except for under sea spawn.

	AuBox locExt;
	bool locExtValid = false;
	double radius2 = roughRadius*roughRadius;	// squared
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		// Units are the only atoms that prevent spawning of anything.
		DaUnit* unit = dynamic_cast<DaUnit*>(*ai);
		if(!unit)
			continue;
		if(!unit->IsNormal())
			continue;
		if(!unit->IsInAtmosphere())
			continue;
		if(unit->AltitudeAGL() < -unit->ObstacleRadius())
			continue;

		const DaCreature* creature = dynamic_cast<const DaCreature*>(*ai);
		if(creature)
		{
			// Skip people who are not on line.
			const DaPerson* person = dynamic_cast<const DaPerson*>(*ai);
			if(person && person->IsInvisibleOffLine())
				continue;

			double dist2 = creature->Location().Distance2(loc);
			if(dist2 > radius2)
				continue;

			// Creature is in the spawn radius.
			return true;
		}

		bool blockedByUnit = false;

		// Test center point location of unit.
		double dist2 = unit->Location().Distance2(loc);
		if(dist2 < radius2)
		{
			// Unit center point location is within the search radius of loc.
			blockedByUnit = true;
		}
		else
		{
			// Units may be big/bulky. Check for intersection of extents boxes.
			AuBox unitExt;
			if(!unit->RoughExtents(unitExt))
				continue;					// Unit has no extents box? Go with the center test result alone.
			unitExt.Transform(unit->Transform());

			if(!locExtValid)
			{
				// Create an extents box for the spawn location.
				AuPoint vup = -GravityDir(loc);
				AuPoint vx;
				vup.ArbitraryAxis(vx);
				AuPoint vy = vup.Cross(vx);
				AuPoint max = vx*roughRadius + vy*roughRadius + vup*roughRadius;
				AuPoint min = -max;
				max += loc;
				min += loc;
				locExt.SetFromMinMax(min, max);
				locExtValid = true;
			}

			if(locExt.DoesIntersect(unitExt))
				blockedByUnit = true;
		}

		if(blockedByUnit)
		{
			DaVessel* ship = dynamic_cast<DaVessel*>(unit);
			if(ship && ship->IsNew() &&
					((designQ > ship->QualityOfDesign() ||
					  (designQ == ship->QualityOfDesign() &&
					   designId > ship->DesignId())) &&	// Newer design replaces older design of same Q
					 !ship->FindPersonRecursive()))
			{
				// Replaceable ship is not considered an obstruction.
				*replaceableShip = ship;
				return false;
			}

			return true;
		}
	}

	// Here if we made it through the gauntlet without finding an obstruction.
	return false;
}

// 'mode': 0=normal, 1=any animal ignoring max pop, 2=aerial animal ignoring max pop
// return values:
// negative # = count of animals spawned
// 0 = no animals spawned
// 1 = animal encounters disabled
// 2 = no other atoms present, also returned when spawn location is not on this world
// 3 = planet fauna regenerated due to error, try again
// 5 = planet cannot have fauna
// 6 = planet does not have fauna
// 8 = max animal population present
// 9 = development at spawn location
// 10 = mode wants aerial but no aerial creatures are in this area
int DaWorldLL::AnimalEncounter(const AuGlobeLLF& nearLL, int mode)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!g_Au.m_EnableAnimalEncounters)
		return 1;

	// No encounters if no atoms present.
	if(!HasSubatoms())
		return 2;	// Won't spawn when no other atoms are present, namely people.

	// No encounters if no animals on this planet.
	if(!LockFaunaForWrite())
	{
		UnlockFauna();
		return 6;	// Can't spawn on planet with no animals.
	}

	// We deadlock in QueryEnvironment if we keep this locked.
	UnlockFauna();

	// Select a location within this terrain for the creatures to spawn.
	AuGlobeLLF encLL = RandomLandLLNear(g_Au.Dice(), nearLL, -10000, 3*FeetPerMeter(), 100*FeetPerMeter());

	AuTerrainInfo ter;
	QueryTerrain(ter, encLL);

	// Don't bother if there are too many wild animals in this terrain.
	if(mode == 0)
	{
		float cityAmbientLight = CityAmbientLight(encLL, WORLDTEX_Count);
		if(cityAmbientLight > .02f)
			return 9;	// Too much city development in the area to spawn.

		// Count wild animals present.
		int wildAnimals = 0;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
		{
			// Count wild animals.
			DaAnimal* animal = dynamic_cast<DaAnimal*>(*ait);
			if(!animal)
				continue;
			if(animal->CurrentHitPoints() <= 0)
				continue;	// Skip dying animals.
			if(animal->IsWild())
				wildAnimals++;
		}

		if(wildAnimals >= ter.MaxAnimals()*5)
			return 8;	// Won't spawn because too many wild animals are present.
	}

	AuPoint encLoc = GetSurfaceLocation(encLL);
	float sunlight = QuerySunlightAtSeaLevel(encLoc);

	// Time to reacquire the lock on the fauna.
	DePlanetFauna* fauna = LockFaunaForWrite();
	if(!fauna)
	{
		// Where did it go?
		UnlockFauna();
		return 6;	// Can't spawn on planet with no animals.
	}

	// There is no development in the encounter tile so proceed.
	DeFaunaEncounter enc;
	if(mode == 2)
	{
		// Check to see if there are any aerial creatures in the current fauna.
		if(!fauna->HasAerialCreatures(sunlight, ter.TerrainType()))
		{
			UnlockFauna();
			return 10;
		}
		enc = fauna->NewAerialEncounter(sunlight, ter.TerrainType());
	}
	else
		enc = fauna->NewEncounter(sunlight, ter.TerrainType());
	UnlockFauna();

	// Determine possible altitude positions of the animal.
	bool posspos[4];
	posspos[0] = DhDNA(enc.DNA()).CanFly();	// Up in the air
	posspos[1] = enc.DNA().LivesOnLand  ||
		(DhDNA(enc.DNA()).CanFly() && !enc.DNA().LivesInWater) ||
		enc.DNA().LivesUnderGround;			// On surface
	posspos[2] = enc.DNA().LivesInWater;	// Under water
	posspos[3] = enc.DNA().LivesUnderGround; // Under ground

	// Determine if water is a criteria for placement of the creature.
	int inwater = 2;	// 0=no, 1=yes, 2=don't care
	if(posspos[2])
	{
		// Creature can be placed in the water.
		// Some creatures may be placed in the air above, under the ground,
		// or on land instead.
		if(!posspos[0] && !posspos[1] && !posspos[3])
			inwater = 1;	// Creature must be placed where there is water.
	}
	else
	{
		// Creature cannot be placed in the water.
		// Some creatures may be placed in the air above.
		// Subterranean creatures are not placed below water when they cannot
		// live in water.
		if(!posspos[0])
			inwater = 0;	// Creature must not be placed where there is water.
	}

	// Create a random Hunger level for the creature(s)
	// Groups of creatures should have the same or similar levels of hunger.
	int actualSpawn = 0;
	int randomHunger = g_Au.Dice().Roll(1,100);
	AuPoint eloc = encLoc;
	for(int animal=0; animal<enc.GroupSize(); animal++)
	{
		// Wander a little from the starting location.
		eloc.x += g_Au.Dice().Roll(1,31,-16);	// -15 to +15
		eloc.y += g_Au.Dice().Roll(1,31,-16);	// -15 to +15
		eloc.z += g_Au.Dice().Roll(1,31,-16);	// -15 to +15

		AuGlobeLL ell;
		if(!LLOfLoc(ell, eloc))
			continue;

		// Determine altitude of animal.
		AuTerrainInfo ter;
		QueryTerrain(ter, ell);
		AuPoint gravityN = -GravityDir(eloc);

		// If the location is not valid, the creature simply doesn't spawn.
		if(inwater != 2)
		{
			// Water presense or absense is a requirement for
			// placement of this animal.
			if(ter.IsWater())
			{
				// Terrain at eloc is water.
				if(inwater == 0)
					continue;	// Must not be placed in water.
			}
			else
			{
				// Terrain at eloc is land.
				if(inwater == 1)
					continue;	// Must be placed in water.
			}
		}

		// Create the animal so we can get physical dimensions.
		AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
		HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
		GROWTHSTAGE stage = g_Au.Dice().Roll(1,100) < 20 ? Juvenile : Mature;
		DaAnimal* pan = new DaAnimal(DhDNA(enc.DNA()).Species(), chroma, hand, stage);	//DHH animalia
		pan->ConnectSignals();
		pan->Create();
		pan->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(pan);
		
		// All creatures should be spawned at least a little satiated.
		int initHunger = pan->MaxHunger() * randomHunger/100;
		if(!initHunger)
			initHunger = 1;	// Don't spawn starving animals.
		pan->SetHunger(initHunger);
		
		// Decide upon the final vertical placement domain of the creature.
		// Possible domains: 0=in air, 1=on land, 2=under water, 3=under ground
		int possdomains[3];
		int posscount = 0;
		if(posspos[0])
			possdomains[posscount++] = 0;		// Can be placed in air.
		if(ter.IsWater())
		{
			if(posspos[2])
				possdomains[posscount++] = 2;	// Can be placed in water.
			if(posspos[3])
				possdomains[posscount++] = 3;	// Can be placed under ground.
		}
		else
		{
			if(posspos[1])
				possdomains[posscount++] = 1;	// Can be placed on land.
			if(posspos[3])
				possdomains[posscount++] = 3;	// Can be placed under ground.
		}
		int vdomain;
		if(!posscount)
			continue;							// Shouldn't really happen.

		if(posscount == 1)
			vdomain = possdomains[0];			// Only one domain possible.
		else
		{
			// Select a domain at random from those possible.
			int dom = g_Au.Dice().Roll(1,posscount,-1);
			vdomain = possdomains[dom];
		}

		// Stand to establish ground height; clears NeedsInit state.
		AuPose finalPose = AuPose::Stand;
		pan->Stand();
		double alt = pan->GroundHeightAboveCG();

		// Set CG altitude of the creature based on vertical placement domain.
		switch(vdomain)
		{
		case 0:		// Up in the air
			alt += pan->SightRange() * .7071;
			finalPose = AuPose::Fly;
			break;
		case 1:		// On surface
		default:
			break;
		case 2:		// Under water
			{
				// We don't care if this guy sticks through the surface.
				int perc = g_Au.Dice().Roll(2,50);	// % depth
				double depth = ter.m_rSeaLevelRadius - ter.m_rSurfaceRadius;
				depth *= (double)perc/100.0;
				alt -= depth;
				finalPose = AuPose::Swim;
			}
			break;
		case 3:		// Under ground
			// This is a reasonable guess to get the thing
			// below the ground. The camera height is its 
			// currently-standing eye height, which is close to
			// its overall height.
			alt -= pan->HearingRange()/2.;
			finalPose = AuPose::Dig;
			break;
		}

		// Position the animal in the environment.
		AuMatrix4x4 t;
		t.Translate(eloc);
		t *= AuPoint(0,0,1).GetAlignmentTransform(gravityN);	// Align with up.
		t.Translate(0, 0, alt);

		// Rotate the critter randomly on its z axis.
		double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
		t.RotateZ(rot);
		pan->ChangePosAndOrient(AuAtomPos(Id(), t));

		pan->SetGeneralPose(finalPose);
		pan->ResetHitPoints();	// Now that it has a body.
		pan->ResetStamina();
		pan->SetInScene();

		g_Au.AtomAdded(pan->Id());
		pan->SetNormal();	// New atom is ready for normal thread processing.
		pan->TickTock();
		actualSpawn++;

		if((g_Au.m_LogMessages & LOG_AnimalEncounter) || mode)
			qDebug() << "DaWorldLL::AnimalEncounter"
				<< animal+1 << "of" << enc.GroupSize()
				<< pan->Id() << pan->Name()
				<< pan->CurrentHitPoints() << "hp"
		<< ((finalPose == AuPose::Dig)?"digging":((finalPose == AuPose::Swim)?"swimming":((finalPose == AuPose::Fly)?"flying":"standing")))
			<< ((vdomain == 0)?"up in the air":((vdomain == 1)?"on the ground":((vdomain == 2)?"under water":"under the ground")))
				<< "alt" << alt << "ground" << ter.m_rSurfaceRadius;
	}
	return -actualSpawn;
}

AuPoint DaWorldLL::TerrainNormal(const AuPoint& loc) const
{
	// Determine plane vectors at loc.
	AuPoint vup = -GravityDir(loc);
	AuPoint vx;
	vup.ArbitraryAxis(vx);										// Sets vx to a unit vector.
	AuPoint vy = vup.Cross(vx);
	vy.Unit();

	AuPoint p1 = loc + vx;										// Arbitrary location a short distance from loc.
	AuGlobeLL p1ll;												// Longitude and latitude of that arbitrary location.
	if(!LLOfLoc(p1ll, p1))										// Longitude and latitude of that arbitrary location.
		return vup;
	p1 = GetTerrainLocation(p1ll);							// Terrain surface location at that longitude and latitude.
	AuPoint v1 = p1 - loc;										// Vector to terrain location from loc.
	v1.Unit();

	AuPoint p2 = loc + vy;										// Arbitrary location a short distance from loc.
	AuGlobeLL p2ll;
	if(!LLOfLoc(p2ll, p2))										// Longitude and latitude of that arbitrary location.
		return vup;
	p2 = GetTerrainLocation(p2ll);							// Terrain surface location at that longitude and latitude.
	AuPoint v2 = p2 - loc;										// Vector to terrain location from loc.
	v2.Unit();

	AuPoint vN = v1.Cross(v2);
	vN.Unit();
	if(vN.IsNaN())
	{
		AuGlobeLLF ll;
		if(LLOfLoc(ll, loc))
			qDebug() << "DaWorldLL::TerrainNormal NAN" << Id() << Name() << "loc" << loc << "ll" << ll.ToString()
				<< "vup" << vup << "vx" << vx << "vy" << vy << "p1" << p1 << "v1" << v1 << "p2" << p2 << "v2" << v2;
		else
			qDebug() << "DaWorldLL::TerrainNormal NAN" << Id() << Name() << "loc" << loc << "ll INVALID"
				<< "vup" << vup << "vx" << vx << "vy" << vy << "p1" << p1 << "v1" << v1 << "p2" << p2 << "v2" << v2;
		vN = vup;
	}
	return vN;
}

AuPoint DaWorldLL::TerrainNormal(const AuPoint& loc, double radius) const
{
	// Determine plane vectors at loc.
	AuPoint vup = -GravityDir(loc);
	AuPoint vx;
	vup.ArbitraryAxis(vx);										// Sets vx to a unit vector.
	AuPoint vy = vup.Cross(vx);
	vy.Unit();

	AuPoint p1n = loc - vx * radius;										// Arbitrary location a short distance from loc.
	AuGlobeLL p1nll;												// Longitude and latitude of that arbitrary location.
	if(!LLOfLoc(p1nll, p1n))										// Longitude and latitude of that arbitrary location.
		return vup;
	p1n = GetTerrainLocation(p1nll);							// Terrain surface location at that longitude and latitude.

	AuPoint p1p = loc + vx * radius;										// Arbitrary location a short distance from loc.
	AuGlobeLL p1pll;												// Longitude and latitude of that arbitrary location.
	if(!LLOfLoc(p1pll, p1p))										// Longitude and latitude of that arbitrary location.
		return vup;
	p1p = GetTerrainLocation(p1pll);							// Terrain surface location at that longitude and latitude.

	AuPoint v1 = p1p - p1n;										// Vector across terrain location at loc.
	v1.Unit();

	AuPoint p2n = loc - vy * radius;										// Arbitrary location a short distance from loc.
	AuGlobeLL p2nll;
	if(!LLOfLoc(p2nll, p2n))										// Longitude and latitude of that arbitrary location.
		return vup;
	p2n = GetTerrainLocation(p2nll);							// Terrain surface location at that longitude and latitude.

	AuPoint p2p = loc + vy * radius;										// Arbitrary location a short distance from loc.
	AuGlobeLL p2pll;
	if(!LLOfLoc(p2pll, p2p))										// Longitude and latitude of that arbitrary location.
		return vup;
	p2p = GetTerrainLocation(p2pll);							// Terrain surface location at that longitude and latitude.

	AuPoint v2 = p2p - p2n;										// Vector across terrain location at loc.
	v2.Unit();

	AuPoint vN = v1.Cross(v2);
	vN.Unit();
	if(vN.IsNaN())
	{
		AuGlobeLLF ll;
		if(LLOfLoc(ll, loc))
			qDebug() << "DaWorldLL::TerrainNormal NAN" << Id() << Name() << "loc" << loc << "ll" << ll.ToString()
				<< "vup" << vup << "vx" << vx << "vy" << vy << "p1p" << p1p << "v1" << v1 << "p2p" << p2p << "v2" << v2;
		else
			qDebug() << "DaWorldLL::TerrainNormal NAN" << Id() << Name() << "loc" << loc << "ll INVALID"
				<< "vup" << vup << "vx" << vx << "vy" << vy << "p1p" << p1p << "v1" << v1 << "p2p" << p2p << "v2" << v2;
		vN = vup;
	}
	return vN;
}

// 'isDark' is relative to the fetchers location.
bool DaWorldLL::FetchManufacturingMaterials(DeManufacturingProcess &mp, const AuPoint& loc, bool isDark, INDUSTRYID iid,
		bool isUndersea, bool inEnclosure, quint8& elecSubQ, quint8& waterSubQ)
{
	// Check to see if the environment of this planet at the city's longitude can provide any materials.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(loc));

	// Most industries can substitute electricity for sunlight since its mostly used to grow plants; electricity translates to
	// lights.
	// Solar power plant, however, cannot produce more electricity from a unit of electricity.
	switch(iid)
	{
	default:
		elecSubQ = res.m_SunlightQ;						// Can substitute electricity for sunlight.
		waterSubQ = res.m_WaterInEnvironmentQ;			// Can substitute bottle water for water in environment.
		break;
	case IID_SolarPowerPlant:
		elecSubQ = 0;									// Can't substitute electricity for sunlight.
		waterSubQ = res.m_WaterInEnvironmentQ;			// Can substitute bottle water for water in environment.
		break;
	case IID_HydroelectricPowerPlant:
	case IID_TreatmentPlant:
	case IID_DrillingRig:
	case IID_Well:
	case IID_Wharf:
		elecSubQ = res.m_SunlightQ;						// Can substitute electricity for sunlight.
		waterSubQ = 0;									// Can't substitute bottled water for water in environment.
		break;
	}

	if(inEnclosure)
	{
		// Prevent fetch of atmosphere in Enclosures. This stops wind power production plus all other manufacturing that draws from
		// the atmosphere.
		res.m_Atmosphere = 0;

		// Enclosure provides vegetation density.
		if(res.m_Vegetation < 60)
			res.m_Vegetation = 60;
		if(res.m_Atmosphere < 50)
			res.m_Atmosphere = 50;
	}

	if(isUndersea)
	{
		// Prevent fetch of sun and atmosphere in under sea cities. This stops wind and solar power production plus all other
		// manufacturing that draws from the atmosphere.
		res.m_Sunlight = 0;
		res.m_Atmosphere = 0;
	}
	else if(isDark)
	{
		// Prevent fetch of natural sunlight at night.
		res.m_Sunlight = 0;
	}

	// Apply the natural resources to the manufacturing process.
	return mp.ApplyNaturalResources(res);
}

bool DaWorldLL::FetchConstructionMaterials(DeConstructionProcess& cp, const DaBd* forBuilding)
{
	Q_ASSERT(forBuilding);
	// Check to see if the environment of this planet at the building's location can provide any materials.
	// No construction requires sunlight as a component so time of day doesn't matter.
	//DHH Bd what cproc requires a natural resource? AuZoneResource res;
	//DHH Bd what cproc requires a natural resource? QueryNaturalResources(res, ResourceZone(forBuilding->Location()));
	//DHH Bd what cproc requires a natural resource? cp.ApplyNaturalResources(res);

	// Fetch materials lying about and from crates sitting in the construction zone.
	bool anyFetched = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->DoesQueryEnvironment())
			continue;
		if(!forBuilding->Is(pAtom->BdId()))
			continue;	// Atom is not at the building site.
		DaItem* pc = dynamic_cast<DaItem *>(pAtom);
		if(!pc)
			continue;	// Atom is not an item.
		if(pc->FetchConstructionMaterials(cp))
			anyFetched = true;
	}
	return anyFetched;
}

// Locate any land at random.
AuGlobeLLF DaWorldLL::RandomLandLLNotForbidden(const AuDice& Cdice, double minAltMSL, double radius) const
{
	AuDice dice(Cdice);
	// Try to find a spot at least 'minAltMSL' but don't look forever.
	int tries = 1000;
	while(tries--)
	{
		AuGlobeLLF ll = RandomLL(dice);
		dice.Random();

		// Avoid the poles.
		if(fabs(ll.Latitude()) > 89*DEG2RADf)
			continue;

		double altMSL = TerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(IsForbiddenZone(ll, altMSL, radius))
			continue;

		if(altMSL >= minAltMSL)
			return ll;
	}
	return RandomLL(dice);
}

// Locate any land at random.
AuGlobeLLF DaWorldLL::RandomLandLocationNotForbidden(const AuDice& Cdice, double minAltMSL, double radius) const
{
	AuDice dice(Cdice);
	// Try to find a spot at least 'minAltMSL' but don't look forever.
	int tries = 1000;
	while(tries--)
	{
		AuGlobeLLF ll = RandomLL(dice);
		dice.Random();

		// Avoid the poles.
		if(fabs(ll.Latitude()) > 89*DEG2RADf)
			continue;

		double altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(IsForbiddenZone(ll, altMSL, radius))
			continue;

		if(altMSL >= minAltMSL)
			return ll;
	}
	return RandomLL(dice);
}

AuGlobeLLF DaWorldLL::RandomLandLL(const AuDice& Cdice, double minAltMSL) const
{
	AuDice dice(Cdice);
	// Try to find a spot at least 'minAltMSL' but don't look forever.
	int tries = 1000;
	while(tries--)
	{
		AuGlobeLLF ll = RandomLL(dice);
		dice.Random();

		// Avoid the poles.
		if(fabs(ll.Latitude()) > 89*DEG2RADf)
			continue;

		double altMSL = TerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(altMSL >= minAltMSL)
			return ll;
	}
	return RandomLL(dice);
}

// Locate any land at random.
AuGlobeLLF DaWorldLL::RandomLandLocation(const AuDice& Cdice, double minAltMSL) const
{
	AuDice dice(Cdice);
	// Try to find a spot at least 'minAltMSL' but don't look forever.
	int tries = 1000;
	while(tries--)
	{
		AuGlobeLLF ll = RandomLL(dice);
		dice.Random();

		// Avoid the poles.
		if(fabs(ll.Latitude()) > 89*DEG2RADf)
			continue;

		double altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(altMSL >= minAltMSL)
			return ll;
	}
	return RandomLL(dice);
}

// Locate any land at random with preferences.
AuPoint DaWorldLL::RandomAvatarLocation() const
{
	//AuGlobeLL putThemHereLL(PI,PI/6);
	//return LocOfLL(putThemHereLL);
	int polarLat = (int)(PolarLatitude() * RAD2DEGf);			// We try to start them in the temperate or tropical latitudes.
	int startlat = (polarLat - g_Au.Dice().Roll(1, polarLat*2+1, -1));	// Random latitude between -polarLat and polarLat degrees.
	int startlon = (180 - g_Au.Dice().Roll(1, 181, -1));				// Random longitude between -180 and 180 degrees.

	// We will do this in three passes.
	// The first pass looks for two consecutive degrees of dry land in daylight, should be large enough to construct buildings.
	// The second pass looks for dry land in daylight.
	// The third pass accepts any land.
	int countOk = 0;											// First pass counts sequential good spots at each latitude.
	bool foundDayLitLand = false;								// True if we find any day lit land in pass 0.
	AuPoint litLand;											// Location of that day lit land.
	bool foundLand = false;										// True if we find any unlit land in pass 0 or 1.
	AuPoint landLoc;											// Location of that unlit land.

	// Select a light level appropriate to the zone in which we are landing.
	float lightLevel;
	if(OrbitZone() == OZONE_Frigid)
		lightLevel = .2f;
	else if(OrbitZone() == OZONE_Outer)
		lightLevel = .4f;
	else if(OrbitZone() == OZONE_Habitable)
		lightLevel = .65f;
	else if(OrbitZone() == OZONE_Inner)
		lightLevel = .7f;
	else
		lightLevel = .8f;

	// We can save time by skipping the first pass when there is a lot of water.
	int pass;
	if(Hydrographics() >= 90)
		pass = 1;												// Skip consecutive dry ground pass when there is a lot of water.
	else
		pass = 0;												// Perform all passes to find a large well lit landing site.

	for(; pass<3; pass++)
	{
		// Restrict latitudes checked in first pass to avoid polar regions.
		int testzone;
		if(pass <= 1)
			testzone = polarLat;	// Check to polarLat degrees latitude north and south.
		else
			testzone = 90;			// Check to 90 degrees latitude north and south.

		// Check all latitudes.
		for(int lat=0; lat<180; lat++)
		{
			// Calculate the latitude to test this time through. We will wrap around to the south pole when we hit the north pole.
			int testlat = startlat + lat;
			if(testlat > 89)
				testlat -= 179;									// Wrap to the south pole.
			if(abs(testlat) > testzone)
				continue;										// Skip latitudes too far north or south of our test zone.

			float latitude = testlat * DEG2RADf;

			// Search for dry land at the test latitude.
			for(int lon=-180; lon<180; lon++)
			{
				int testlon = startlon + lon;
				if(testlon >= 180)
					testlon -= 360;

				float longitude = testlon * DEG2RADf;

				// Check the terrain altitude at the random test location.
				AuGlobeLL ll(longitude, latitude);
				float terAlt = SurfaceAltitudeMSL(ll);
				if(pass == 0)
				{
					// First pass wants two consecutive degrees of lit dry land.
					if(terAlt > 0)
					{
						// Found terrain above water.
						// Test light level in first pass.
						// Skip dimly lit terrain in first pass.
						AuPoint terLoc = LocOfLL(ll, terAlt);
						float light = QuerySunlight(terLoc);
						if(light >= lightLevel)
						{
							// Found lit terrain above water. Count consecutive degrees of dry lit land.
							countOk++;
							if(countOk > 2)
								return terLoc;					// Found sufficient degrees of adjacent dry lit land.
							else if(!foundDayLitLand)
							{
								// Remember this bit of lit dry land so we can skip the second pass.
								foundDayLitLand = true;
								litLand = terLoc;
							}
						}
						else
						{
							// Terrain is not well lit.
							countOk = 0;
							if(!foundLand)
							{
								// Remember this bit of unlit dry land so we can skip the third pass.
								foundLand = true;
								landLoc = terLoc;
							}
						}
					}
					else
					{
						// Terrain is under water.
						countOk = 0;
					}
				}
				else if(pass == 1)
				{
					// Second pass wants any lit dry land.
					if(terAlt > 0)
					{
						// Found terrain above water.
						// Test light level in first pass.
						// Skip dimly lit terrain in first pass.
						AuPoint terLoc = LocOfLL(ll, terAlt);
						float light = QuerySunlight(terLoc);
						if(light >= lightLevel)
						{
							// Found lit terrain above water.
							return terLoc;						// Found dry lit land.
						}
						else if(!foundLand)
						{
							// Remember this bit of unlit dry land so we can skip the third pass.
							foundLand = true;
							landLoc = terLoc;
						}
					}
				}
				else
				{
					// Third pass we'll take any dry land.
					if(terAlt > 0)
						return LocOfLL(ll, terAlt);
				}
			}
		}

		if(pass == 0)
		{
			// At the end of pass 0, we'll know if we found any land in daylight or in darkness.
			if(foundDayLitLand)
				return litLand;
			else if(foundLand)
				return landLoc;

			// We did not find any land at all, dark or lit, in the latitude range that was checked.
			// Perform the final pass to test the entire latitude range for any land, dark or lit.
			pass = 1;					// This will get bumped to 2 by the 'for' loop controller.
		}
		else if(pass == 1)
		{
			// At the end of pass 1, we'll know if we found any land in darkness.
			if(foundLand)
				return landLoc;

			// We did not find any land at all, dark or lit, in the latitude range that was checked.
			// Perform the final pass to test the entire latitude range for any land, dark or lit.
		}
	}

	AuGlobeLL ll(PI/2, 0);
	float terAlt = SurfaceAltitudeMSL(ll);
	return LocOfLL(ll, terAlt);
}

AuPoint DaWorldLL::AvatarPreviewLocation(quint8 previewType) const
{
	// Start near the center of the map, not at the center so we don't land at the ringworld control center.
	int startlat = 10;
	int startlon = 190;
	int polarLat = (int)(PolarLatitude() * RAD2DEGf);			// We try to start them in the temperate or tropical latitudes.

	if(previewType != 9)
	{
		// Look for a spot on land.
		// We will do this in two passes.
		// The first pass looks for dry land between the polar latitudes.
		// The second pass accepts any land.
		for(int pass=0; pass<2; pass++)
		{
			// Restrict latitudes checked in first pass to avoid polar regions.
			int testzone;
			if(pass < 1)
				testzone = polarLat;	// Check to polarLat degrees latitude north and south.
			else
				testzone = 90;			// Check to 90 degrees latitude north and south.

			// Check all latitudes.
			for(int lat=0; lat<180; lat++)
			{
				// Calculate the latitude to test this time through. We will wrap to the south pole when we hit the north pole.
				int testlat = startlat + lat;
				if(testlat > 89)
					testlat -= 179;								// Wrap to the south pole.
				if(abs(testlat) > testzone)
					continue;									// Skip latitudes too far north or south of our test zone.

				float latitude = testlat * DEG2RADf;

				// Search for dry land at the test latitude.
				for(int lon=-180; lon<180; lon++)
				{
					int testlon = startlon + lon;
					if(testlon >= 180)
						testlon -= 360;

					float longitude = testlon * DEG2RADf;

					// Check the terrain altitude at the random test location.
					AuGlobeLL ll(longitude, latitude);
					float terAlt = SurfaceAltitudeMSL(ll);
					// We'll take any dry land.
					if(terAlt > 0)
						return LocOfLL(ll, terAlt);
				}
			}
		}
	}
	else
	{
		// Look for the deepest spot in the sea.
		// We will do this in two passes.
		// The first pass looks for the deepest land between the polar latitudes.
		// The second pass accepts any undersea land.
		AuGlobeLL deepLL;
		double deepDepth = 0;
		for(int pass=0; pass<2; pass++)
		{
			// Restrict latitudes checked in first pass to avoid polar regions.
			int testzone;
			if(pass < 1)
				testzone = polarLat;	// Check to polarLat degrees latitude north and south.
			else
				testzone = 90;			// Check to 90 degrees latitude north and south.

			// Check all latitudes.
			for(int lat=0; lat<180; lat++)
			{
				// Calculate the latitude to test this time through. We will wrap to the south pole when we hit the north pole.
				int testlat = startlat + lat;
				if(testlat > 89)
					testlat -= 179;								// Wrap to the south pole.
				if(abs(testlat) > testzone)
					continue;									// Skip latitudes too far north or south of our test zone.

				float latitude = testlat * DEG2RADf;

				// Search for undersea land at the test latitude.
				for(int lon=-180; lon<180; lon++)
				{
					int testlon = startlon + lon;
					if(testlon >= 180)
						testlon -= 360;

					float longitude = testlon * DEG2RADf;

					// Check the terrain altitude at the random test location.
					AuGlobeLL ll(longitude, latitude);
					float terAlt = SurfaceAltitudeMSL(ll);
					if(terAlt < deepDepth)
					{
						deepDepth = terAlt;
						deepLL = ll;
					}
				}
			}
			if(deepDepth < 0)
				return LocOfLL(deepLL, deepDepth);
		}
	}

	AuGlobeLL ll(PI/2, 0);
	float terAlt = SurfaceAltitudeMSL(ll);
	return LocOfLL(ll, terAlt);
}

// Locate any land at random with preferences.
AuPoint DaWorldLL::RandomLandingSite(const AuPoint& suggestion) const
{
	m_LandingSitesLock.lockForRead();
	bool hasSites = m_LandingSites;
	m_LandingSitesLock.unlock();

	if(!hasSites)
	{
		// Locate a landing site in each resource zone.
		m_LandingSitesLock.lockForWrite();
		if(!m_LandingSites)
			m_LandingSites = new AuPoint[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_LandingSites[rz] = ResourceZoneLandingSite(rz);
		m_LandingSitesLock.unlock();
	}

	// A suggested location has been provided. AI pilots do this when looking for a place to land after they have entered the
	// atmosphere. It is their current location. This prevents them from changing their mind at this stage of the landing because
	// a different location got brighter.
	//DHH Disabled because the pilot could enter the atmosphere above a different zone than the target zone and suddenly change
	//DHH his mind.
	if(false && !suggestion.IsZero())
	{
		int suggestZone = ResourceZone(suggestion);
		m_LandingSitesLock.lockForRead();
		AuPoint suggestLanding = m_LandingSites[suggestZone];
		m_LandingSitesLock.unlock();
		if(!suggestLanding.IsZero())
			return suggestLanding;
	}

	// Return the landing site with the brightest light level right now. Light levels don't change quickly so this should remain
	// consistent through most landing approaches by AI.
	float bestLight = -1;
	AuPoint bestSite;
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_LandingSitesLock.lockForRead();
		AuPoint zoneLandingSite = m_LandingSites[rz];
		m_LandingSitesLock.unlock();
		if(zoneLandingSite.IsZero())
			continue;	// No landing site in the zone.
		float zoneLight = QuerySunlight(zoneLandingSite);
		if(zoneLight > bestLight)
		{
			bestLight = zoneLight;
			bestSite = zoneLandingSite;
		}
	}

	return bestSite;
}

// Latitudes > PolarLatitude are polar.
// *Antarctica starts at about 68 degrees south.
// Earth surface water is about 71%.
float DaWorldLL::PolarLatitude() const
{
	// No polar latitude in inferno zone.
	if(OrbitZone() == OZONE_Inferno)
		return PIf/2;

	// Calculate polar latitude factor based on water coverage and orbit zone.
	float latFactor = 1.f - ((float)Hydrographics() / 100) * ((float)OrbitZone() / 5);		// 0 in inferno zone
	return PIf/2 * latFactor;
}

// Latitudes <= TropicalLatitude are tropical.
float DaWorldLL::TropicalLatitude() const
{
	// It's all warm and tropical on inferno worlds.
	if(OrbitZone() == OZONE_Inferno)
		return PIf/2;

	// Lets call the tropical level 1/3 of the remaining land between the equator and the polar latitude.
	return PolarLatitude()/3;
}

AuFloraDNA::ZONE DaWorldLL::LatitudeRegion(float latitude) const
{
	// Use greater test so we don't show a polar vertex on worlds where PolarLatitude is 90 degrees.
	if(fabs(latitude) > PolarLatitude())
		return AuFloraDNA::Polar;
	else if(fabs(latitude) < TropicalLatitude())
		return AuFloraDNA::Tropical;
	else
		return AuFloraDNA::Temperate;
}

bool DaWorldLL::IsLandingAreaObstructed(const AuPoint& loc, double radius, const DaAtom* ignoreAtom) const
{
	AuGlobeLL ll;
	if(!LLOfLoc(ll, loc))
		return true;

	// Any spot under water is obstructed.
	if(Hydrographics())
	{
		double terAlt = SurfaceAltitudeMSL(ll);
		if(terAlt < 0.)
			return true;
	}

	// No atoms here to obstruct the landing area.
	if(!HasSubatoms())
		return false;

	AuBox locExt;
	bool locExtValid = false;
	double radius2 = radius*radius;	// squared
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		// Units are the only atoms that prevent landing of anything.
		DaUnit* unit = dynamic_cast<DaUnit*>(*ai);
		if(!unit)
			continue;
		if(unit == ignoreAtom)
			continue;
		if(!unit->IsInAtmosphere())
			continue;
		if(unit->AltitudeAGL() < -unit->ObstacleRadius())
			continue;	// Underground

		// Creatures do not block spacecraft.
		const DaCreature* creature = dynamic_cast<const DaCreature*>(*ai);
		if(creature)
			continue;
		// Ignore buildings. I may be trying to land on it.
		const DaBd* bd = dynamic_cast<const DaBd*>(*ai);
		if(bd)
			continue;
		/*
		{
			// Skip people who are not on line.
			const DaPerson* person = dynamic_cast<const DaPerson*>(*ai);
			if(person && person->IsInvisibleOffLine())
				continue;

			double dist2 = creature->Location().Distance2(loc);
			if(dist2 > radius2)
				continue;

			// Creature is in the spawn radius.
			return true;
		}
		*/

		bool blockedByUnit = false;

		// Test center point location of unit.
		double dist2 = unit->Location().Distance2(loc);
		if(dist2 < radius2)
		{
			// Unit center point location is within the search radius of loc.
			blockedByUnit = true;
		}
		else
		{
			// Units may be big/bulky. Check for intersection of extents boxes.
			AuBox unitExt;
			if(!unit->RoughExtents(unitExt))
				continue;					// Unit has no extents box? Go with the center test result alone.
			unitExt.Transform(unit->Transform());

			if(!locExtValid)
			{
				// Create an extents box for the spawn location.
				AuPoint vup = -GravityDir(loc);
				AuPoint vx;
				vup.ArbitraryAxis(vx);
				AuPoint vy = vup.Cross(vx);
				AuPoint max = vx*radius + vy*radius + vup*radius;
				AuPoint min = -max;
				max += loc;
				min += loc;
				locExt.SetFromMinMax(min, max);
				locExtValid = true;
			}

			if(locExt.DoesIntersect(unitExt))
				blockedByUnit = true;
		}

		if(blockedByUnit)
			return true;
	}

	// Here if we made it through the gauntlet without finding an obstruction.
	return false;
}

TER_ELEV_TYPE DaWorldLL::TerrainElevationType(float altitudeMSL) const
{
	float altInc = (float)AltitudeIncrement();
	if(Hydrographics())
	{
		// World has water. Could be frozen.
		if(altitudeMSL <= 0)
		{
			// Altitude is under water.
			if(altitudeMSL >= -altInc)
				return TER_ContinentalShelf;
			else if(altitudeMSL >= -altInc*2)
				return TER_SeaFloor;
			return TER_SeaTrench;
		}
		else
		{
			// Altitude is above water.
			if(altitudeMSL <= altInc/2)
				return TER_Coastal;
			else if(altitudeMSL <= altInc)
				return TER_Lowlands;

			float treeLine = TreeLineMSL();
			if(altitudeMSL > treeLine)
			{
				// Altitude is above the tree line.
				if(CanHaveSnow())
				{
					if(altitudeMSL > treeLine + altInc*.75f)
						return TER_Peak;
					else if(altitudeMSL > treeLine + altInc*.4f)
						return TER_Alpine;
				}
				return TER_Highlands;
			}

			return TER_Midlands;
		}
	}
	else
	{
		// Waterless world.
		if(altitudeMSL <= altInc/2)
			return TER_Coastal;
		else if(altitudeMSL <= altInc*3)
			return TER_Lowlands;

		float treeLine = TreeLineMSL();
		if(altitudeMSL > treeLine)
		{
			// Altitude is above the tree line.
			if(CanHaveSnow())
			{
				if(altitudeMSL > treeLine + altInc*.75f)
					return TER_Peak;
				else if(altitudeMSL > treeLine + altInc*.4f)
					return TER_Alpine;
			}
			return TER_Highlands;
		}

		if(altitudeMSL > altInc*6)
			return TER_Highlands;

		return TER_Midlands;
	}
}

bool DaWorldLL::IsControl(const DaAtom* sceneAtom) const
{
	// Globe is a control to allow foraging from the environment.
	// Scene atom cannot be aboard a vehicle or spacecraft or riding a mount to forage from the environment.
	if(!Is(sceneAtom->ParentId()))
		return false;
	if(sceneAtom->InDesignEnvironment())
		return false;	// Cannot forage from the design studio preview environment.
	return true;
}

// This is only called for mines.
void DaWorldLL::SelectDefaultMineProcess(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
		DeManufacturingProcess* proc, const AuKnownPatents& patents) const
{
	// Locate the manufacturing process for the special mineral that is present in the tile, if any.
	int mineralMPID = 0;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator it = gd.m_bManufacturingProcessArray.cbegin();
			it != gd.m_bManufacturingProcessArray.cend(); it++)
	{
		if((*it).m_idi == IID_Mine && (*it).m_idc != CID_Stone)
		{
			// Do not list patented processes unless the patent is available.
			if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*it).m_id, patents))
				continue;
			// In order for mines to produce anything but stone, the appropriate natural resource must be present.
			if(TestRockResource(mBld, siteCen, siteBoxRadius, (COMMODITYID)(*it).m_idc))
			{
				mineralMPID = (*it).m_id;
				break;	// Found a mineral process id.
			}
		}
	}
	g_Au.UnlockData();
	if(mineralMPID)
	{
		// Configure the manufacturing process for the tile.
		proc->SetManufacturingProcessId(mineralMPID);
	}
}

// This is only called for wells and drilling rigs.
void DaWorldLL::SelectDefaultWellProcess(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
		DeManufacturingProcess* proc, const AuKnownPatents& patents) const
{
	// Locate the manufacturing processes for the special mineral that is present in the tile, if any.
	int mineralMPID = 0;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator it = gd.m_bManufacturingProcessArray.cbegin();
			it != gd.m_bManufacturingProcessArray.cend(); it++)
	{
		if(((*it).m_idi == IID_Well || (*it).m_idi == IID_DrillingRig) && (*it).m_idc != CID_Water)
		{
			// Do not list patented processes unless the patent is available.
			if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*it).m_id, patents))
				continue;
			// In order for wells to produce anything but water, the appropriate natural resource must be present.
			if(TestRockResource(mBld, siteCen, siteBoxRadius, (COMMODITYID)(*it).m_idc))
			{
				mineralMPID = (*it).m_id;
				break;	// Found a mineral process id.
			}
		}
	}
	g_Au.UnlockData();
	if(mineralMPID)
	{
		// Configure the manufacturing process for the tile.
		proc->SetManufacturingProcessId(mineralMPID);
	}
}

// 'plantType' is 0=ground cover, 1=shrubber, 2=trees
// 'floraZone' is 0=polar, 1=temperate, 2=tropical
void DaWorldLL::RegenPlantDNA(int resourceZone, int plantType, int floraZone)
{
	if(resourceZone < 0 || resourceZone >= ResourceZones())
	{
		qDebug("DaWorldLL::RegenPlantDNA %d %s resourceZone %d is invalid. %d zones on planet.",
				Id().GetID(), (const char*)Name().toLatin1(), resourceZone, ResourceZones());
		return;
	}
	if(plantType < 0 || plantType > 2)
	{
		qDebug("DaWorldLL::RegenPlantDNA %d %s plantType %d is invalid.",
				Id().GetID(), (const char*)Name().toLatin1(), plantType);
		return;
	}
	if(floraZone < 0 || floraZone > 2)
	{
		qDebug("DaWorldLL::RegenPlantDNA %d %s floraZone %d is invalid.",
				Id().GetID(), (const char*)Name().toLatin1(), floraZone);
		return;
	}

	// Note that this flora will revert back if the system contents are regenerated.
	DePlanetFlora* flora = LockFloraForWrite();
	if(!flora)
	{
		qDebug("DaWorldLL::RegenPlantDNA %d %s Planet can have flora but it does not have any!",
				Id().GetID(), (const char*)Name().toLatin1());
		UnlockFlora();
		return;
	}

	flora[resourceZone].InitRingworld(plantType, (AuFloraDNA::ZONE)floraZone, g_Au.Dice());
	UnlockFlora();

	SetModified();
}

// Finds vehicles that are outside on the ground in a city on this planet that can be used for production processes.
// Only one of each vehicle type is wanted/needed.
void DaWorldLL::TallyVehicles(const AuId& cityId, quint32 empireId, QList<COMMODITYID>& vehicles) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(*it);
		if(!vehicle)
			continue;

		if(vehicle->EmpireId() != empireId)
			continue;				// Only vehicles of the city's empire can be used by its citizens.
		if(vehicle->InBdCity() != cityId)
			continue;

		// Found a vehicle in the city.
		// Manufacturing processes only need to know about one of each kind of vehicle available.
		if(!vehicles.contains(vehicle->CommodityId()))
			vehicles.append(vehicle->CommodityId());
	}
}

DaVehicle* DaWorldLL::FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId, const AuId& onSlabInCity)
{
	// List all the cities on this planet that...
	// - are friendly
	// - have an airport or city is the 'onSlabInCity'
	// - have an appropriate vehicle factory
	if(!HasSubatoms())
		return 0;

	QHash<DaBd*, DaVehicle*> bds;
	DaAtomList cities;
	AuIdList cityIds;
	bool inBdCity = false;
	QHash<quint32, bool> exportRestrictions;	// key is empire id, value is restriction
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(bd)
		{
			AuId bdCityId(bd->CapitolId());
			if(bdCityId != onSlabInCity)
			{
				// Requestor is not at the building.
				if(bdCityId.IsNull())
					continue;									// Not part of any city and requestor is not at the building.

				// If the person looking for a vehicle is not in this city, I must be able to export the vehicle to them.
				//DHH Or they must have a vehicle transporter aboard their ship and be in range.
				if(!bd->HasAirportAccess())
					continue;									// No airport to export vehicles and requestor is not in the city.
			}

			DaVehicle* vehicle = bd->FindNewVehicle(vehicleCID);
			if(!vehicle)
				continue;										// Building has no nu vehicles of that type in town.

			// Check vehicle export restrictions of the empire owning the building.
			bool restricted;
			quint32 bdEmpireId = bd->EmpireId();
			QHash<quint32, bool>::iterator ri = exportRestrictions.find(bdEmpireId);	// key is empire id, value is restriction
			if(ri == exportRestrictions.end())
			{
				POLITICAL_STANCE stance = PoliticalStance(bdEmpireId, empireId);
				const DgData& gd = g_Au.LockDataForRead();
				restricted = gd.EmpireCommerceRestricted(bdEmpireId, vehicleCID, stance);
				g_Au.UnlockData();
				exportRestrictions.insert(bdEmpireId, restricted);
			}
			else
				restricted = *ri;
			if(restricted)
				continue;										// Export policy prohibits transfer of vehicle.

			// Found a building that could export the requested vehicle to the requestor.
			bds.insert(bd, vehicle);
			cityIds.AddIfUnique(inBdCity);
			if(bdCityId == onSlabInCity)
				inBdCity = true;
			continue;
		}
	}

	// Fetch a 'nu' vehicle from one of the acceptable cities or bds.
	// Prefer the 'onSlabInCity' city or bd.
	if(inBdCity)
	{
		// Search for a nu vehicle in the 'onSlabInCity' city.
		for(QHash<DaBd*, DaVehicle*>::iterator it = bds.begin(); it != bds.end(); it++)
		{
			if(it.key()->CapitolId() != onSlabInCity)
				continue;	// Bd is in a different city.
			return *it;
		}
		for(QHash<DaBd*, DaVehicle*>::iterator it = bds.begin(); it != bds.end(); it++)
		{
			if(it.key()->CapitolId() == onSlabInCity)
				continue;	// Bd is in the city we already checked.
			return *it;
		}
	}

	// Search for a nu vehicle in any of the cityIds.
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(*it);
		if(!vehicle)
			continue;
		if(vehicle->CommodityId() != vehicleCID)
			continue;	// Wrong type of vehicle.
		if(!vehicle->IsNew())
			continue;	// Only grab 'nu' vehicles.
		if(cityIds.contains(vehicle->InBdCity()))
			return vehicle;
	}

	// No vehicle fetched, try moons.
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorld* moon = dynamic_cast<DaWorld*>(*it);
		if(!moon)
			continue;
		DaVehicle* vehicle = moon->FindNewVehicle(vehicleCID, empireId, onSlabInCity);
		if(vehicle)
			return vehicle;
	}
	return 0;
}

// Used by DaSc to fetch a vehicle.
DaVehicle* DaWorldLL::FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId, const AuId& onSlabInCity)
{
	if(!HasSubatoms())
		return 0;

	QHash<DaBd*, DaVehicle*> bds;
	DaAtomList sysveh;
	AuIdList cityIds;
	bool inBdCity = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(bd)
		{
			AuId bdCityId(bd->CapitolId());
			if(onSlabInCity.IsNull())
			{
				// Requestor is not at any building.
				// If the person looking for a vehicle is not in this city, I must be able to export the vehicle to them.
				//DHH Or they must have a vehicle transporter aboard their ship and be in range.
				if(!bd->HasAirportAccess())
					continue;									// No airport to export vehicles and requestor is not in the city.
			}
			else
			{
				if(bdCityId != onSlabInCity)
				{
					// Requestor is not at the building.
					if(bdCityId.IsNull())
						continue;								// Bd is not part of any city and requestor is not at the building.

					// If the person looking for a vehicle is not in this city, I must be able to export the vehicle to them.
					//DHH Or they must have a vehicle transporter aboard their ship and be in range.
					if(!bd->HasAirportAccess())
						continue;								// No airport to export vehicles and requestor is not in the city.
				}
			}

			DaVehicle* vehicle = bd->FindNewVehicleType(spotType, 0);
			if(!vehicle)
				continue;										// Building has no nu vehicles of that type in town.

			// Check vehicle export restrictions of the empire owning the building.
			quint32 bdEmpireId = bd->EmpireId();
			POLITICAL_STANCE stance = PoliticalStance(bdEmpireId, empireId);
			const DgData& gd = g_Au.LockDataForRead();
			bool restricted = gd.EmpireCommerceRestricted(bdEmpireId, vehicle->CommodityId(), stance);
			g_Au.UnlockData();
			if(restricted)
				continue;										// Export policy prohibits transfer of vehicle.

			// Found a building that could export the requested vehicle to the requestor.
			sysveh.append(vehicle);
			if(bdCityId.IsNull() || onSlabInCity.IsNull())
				continue;

			bds.insert(bd, vehicle);
			cityIds.AddIfUnique(bdCityId);
			if(bdCityId == onSlabInCity)
				inBdCity = true;
		}
	}

	// Fetch a 'nu' vehicle from one of the acceptable bds.
	// Prefer the 'onSlabInCity' bd.
	if(inBdCity)
	{
		// Search for a nu vehicle in the 'onSlabInCity' city.
		for(QHash<DaBd*, DaVehicle*>::iterator it = bds.begin(); it != bds.end(); it++)
		{
			if(it.key()->CapitolId() != onSlabInCity)
				continue;	// Bd is in a different city.
			return *it;
		}
		for(QHash<DaBd*, DaVehicle*>::iterator it = bds.begin(); it != bds.end(); it++)
		{
			if(it.key()->CapitolId() == onSlabInCity)
				continue;	// Bd is in the city we already checked.
			return *it;
		}
	}

	// Search for a nu vehicle in any of the cities or bds.
	for(DaAtomList::const_iterator it = sysveh.cbegin(); it != sysveh.cend(); it++)
	{
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(*it);
		if(!vehicle)
			continue;
		if(!vehicle->CanFetch(spotType))
			continue;
		if(onSlabInCity.IsNull() || cityIds.contains(vehicle->InBdCity()))
			return vehicle;
	}

	// No vehicle fetched, try moons.
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorld* moon = dynamic_cast<DaWorld*>(*it);
		if(!moon)
			continue;
		DaVehicle* vehicle = moon->FindNewVehicleType(spotType, empireId, onSlabInCity);
		if(vehicle)
			return vehicle;
	}
	return 0;
}

// Used by DaBd to fetch a vehicle.
DaVehicle* DaWorldLL::FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher)
{
	if(!HasSubatoms())
		return 0;

	// Search for nu vehicles at this planet.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(*it);
		if(vehicle)
		{
			if(!vehicle->CanBdFetch(spotType, fetcher))
				continue;
			if(fetcher->Is(vehicle->BdId()))
				continue;	// Don't fetch vehicles from my own site.
			if(fetcher->InBdCity() == vehicle->InBdCity())
				return vehicle;
			else
			{
				AuId cityOrBd(vehicle->BdId());
				if(!cityOrBd.IsNull())
				{
					// Vehicle is not in the same city. Is it in a city that is accessible to bdCity?
					if(fetcher->HasTradeConnection(cityOrBd))
					{
						// Check vehicle export restrictions of the empire owning the city.
						quint32 vemp = vehicle->EmpireId();
						POLITICAL_STANCE stance = PoliticalStance(vemp, fetcher->EmpireId());
						const DgData& gd = g_Au.LockDataForRead();
						bool restricted = gd.EmpireCommerceRestricted(vemp, vehicle->CommodityId(), stance);
						g_Au.UnlockData();
						if(restricted)
							continue;											// Export policy prohibits transfer of vehicle.
						return vehicle;
					}
				}
			}
		}
		else
		{
			DaBd* bd = dynamic_cast<DaBd*>(*it);
			if(bd && !bd->Is(fetcher->Id()) && fetcher->HasTradeConnection(bd->Id()))
			{
				vehicle = bd->FindNewVehicleType(spotType, fetcher);
				if(vehicle)
					return vehicle;
			}
		}
	}

	// No vehicle fetched, try moons.
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorld* moon = dynamic_cast<DaWorld*>(*it);
		if(!moon)
			continue;
		DaVehicle* vehicle = moon->FindNewVehicleType(spotType, fetcher);
		if(vehicle)
			return vehicle;
	}
	return 0;
}

void DaWorldLL::QueryTreeTrunkSize(COMMODITYID treeCID, int resourceZone, AuFloraDNA::ZONE latRegion, float& trunkLength,
		float& trunkRadius) const
{
	const DePlanetFlora* flora = LockFlora();
	if(flora)
	{
		const DeFlora& tree = flora[resourceZone].Tree(latRegion, treeCID);
		trunkLength = tree.DNA().TrunkLength + 1;
		trunkRadius = tree.DNA().TrunkBaseDiameter() / 2;
	}
	else
	{
		trunkLength = 0;
		trunkRadius = 0;
	}
	UnlockFlora();
}

void DaWorldLL::ProcessDemandTally(const AuId& idCity, const AuItem& item,
		unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const
{
	if(!idCity.IsNull())
	{
		// When looking for a particular city, it is known to be on this planet.
		const DaBd* city = FindBd(idCity);
		if(city)
		{
			// Collect demand data from cities.
			city->ProcessConstructionDemandTally(item, needCount);
			city->ProcessDemandTally(item, stock, population, needCount, produceCount);
		}
	}
	else if(HasSubatoms())
	{
		// Not looking for a particular city so collect data recursively from all cities.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			const DaAtom* pAtom = *it;
			const DaBd* city = dynamic_cast<const DaBd *>(pAtom);
			if(city)
			{
				// Collect demand data from cities.
				city->ProcessConstructionDemandTally(item, needCount);
				city->ProcessDemandTally(item, stock, population, needCount, produceCount);
			}
			else
			{
				const DaWorld* planet = dynamic_cast<const DaWorld*>(pAtom);
				if(planet)
				{
					// Collect demand data from moons when not looking for a particular city.
					planet->ProcessDemandTally(idCity, item, stock, population, needCount, produceCount);
				}
			}
		}
	}
}

// Spawns a spacecraft on the ground, if possible.
// Returns:
//   0 = ok
//   1 = no slab found
//   2 = no unobstructed slab found
//   3 = spawn location not on the world
int DaWorldLL::SpawnSc(DaBd* bd, const AuScReport& shipDesign, quint8 quality, const AuId& ownerId, const QString& ownerName,
		quint32 ownerEmpire)
{
	AuEther* ether = Ether();
	if(!ether)
		return 3;
	AuMatrix4x4 spawnAt;
	DaVessel* replaceableShip = 0;
	bool foundSpawnLoc = false;
	QList<AuMatrix4x4> bldMs;
	bd->SpacecraftParkingSpots(bldMs);							// Get locations in the space of the building.
	if(bldMs.empty())
		return 1;
	for(QList<AuMatrix4x4>::iterator li = bldMs.begin(); li != bldMs.end(); li++)
	{
		spawnAt = bd->Transform() * *li;
		AuPoint bldLoc = spawnAt.Translation();
		if(!IsSpawnAreaObstructed(bldLoc, shipDesign.DesignId(), shipDesign.Quality(), shipDesign.RoughHullRadius(),
					&replaceableShip))
		{
			// The spawn area is not obstructed.
			foundSpawnLoc = true;
			break;
		}
	}
	if(!foundSpawnLoc)
		return 2;						// Did not find any unobstructed location at which to spawn.

	// Remove the replaceable ship, if we replaced one.
	if(replaceableShip)
		replaceableShip->Kill();		// Kill the ship and all of its subatoms recursively.

	DaSc* ship = new DaSc();
	ship->ConnectSignals();
	ship->Create();

	// Position the atom in the environment.
	AuMatrix4x4 t(spawnAt);
	t.Translate(0, 0, ship->GroundHeightAboveCG());
	ship->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(ship);

	ship->SetDesign(shipDesign);

	if(!ownerId.IsNull())
	{
		// Privately owned. Owner is responsible for their own fuel and repairs but no subsidy tax.
		ship->SetPrivateOwnership(true);
		ship->SetPrivateOwner(ownerId, ownerName);
		ship->SetEmpireId(ownerEmpire);
	}
	else
		ship->SetEmpireId(bd->EmpireId());
	ship->InitSystems(quality);
	ship->SetInMotion();

	// Mark the solar system as explored by the empire owning the ship.
	DaSolarSystem* system = FindParentSolarSystem();
	if(system && !ship->BdId().IsNull())
		system->ExploredBy(ship->EmpireId());

	// Develop a description of where this ship was made.
	// e.g. Imperium city Bay of Magellan on Hazeron Alpha V in Hazeron, Hazeron Sector
	const DaSector* sector = FindParentSector();
	QString empire;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(ship->EmpireId(), empire);
	g_Au.UnlockData();
	QString whereMade;
	if(system && sector)
	{
		QString cityName = bd->CityOrBuildingName();
		whereMade = tr("%1 %2 on %3 in %4, %5").arg(empire).arg(cityName).arg(Name()).arg(system->Name()).arg(sector->Name());
	}
	ship->SetWhereMade(whereMade);

	// Refuel nu spacecraft if the city has hydrogen.
	ship->FillUpFuel(bd);

	// The spacecraft spawned fine. It can stay where it is.
	g_Au.AtomAdded(ship->Id());
	ship->SetNormal();	// New atom is ready for normal thread processing.
	ship->TickTock();

	// Send mail to the new owner.
	if(!ownerId.IsNull())
		bd->NotifyNewSpacecraftOwner(ownerId, ownerName, ownerEmpire, ship);
	return 0;
}

DaNpc* DaWorldLL::FindTroopReadyToLeave(const DaVehicle& toLoadVehicle) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* troop = dynamic_cast<DaNpc*>(*it);
		if(!troop || !troop->IsNpcTroop())
			continue;
		if(troop->CurrentHitPoints() <= 0)
			continue;	// Skip dying troops.
		if(troop->IsTroopStationPose())
			continue;	// Busy
		if(troop->EmpireId() != toLoadVehicle.EmpireId())
			continue;
		if(toLoadVehicle.DistanceToAtom2(troop, this) > 100*100)
			continue;	// Vehicle is too far away.
		return troop;
	}
	return 0;
}

// Determine foliage density attributed to water and atmosphere.
// Returns a number from 0 to 100.
// Earth returns 71 due to 50% atm density and 71% water
unsigned char DaWorldLL::AverageFoliage() const
{
	// CanHaveFlora requires a minimum of 10 percent atm density.
	if(!CanHaveFlora())
		return 0;

	unsigned int lPlantPercent = (unsigned int)((MSLAtmosphereDensity()*100.f * Hydrographics()) * 100.f / 50.f);
	if(OrbitZone() == OZONE_Inner)
		lPlantPercent = lPlantPercent * 3/4;					// Few plants in the inner zone.
	else if(OrbitZone() == OZONE_Outer)
		lPlantPercent = lPlantPercent * 2/3;					// Rare plants in the outer zone.

	if(lPlantPercent > 100)
		lPlantPercent = 100;

	return lPlantPercent;
}

// static
QColor DaWorldLL::AtmosphereColor(unsigned char byAtmType, unsigned char byAtmDensity, quint32 genSeed)
{
	int r,g,b;
	if(byAtmDensity > 0)
	{
		// Purturb the colors a bit just for fun.
		// Seed the random number generator for consistent values for the planet.
		AuDice dice;
		dice.Seed(genSeed);

		switch(byAtmType)
		{
		default:
		case ATMTYPE_Standard:
			r = 255-30;
			g = 255-30;
			b = 255-30;
			r += dice.Roll(1,61,-31);	// +/-30. This gets clamped at the end.
			g += dice.Roll(1,61,-31);	// +/-30. This gets clamped at the end.
			b += dice.Roll(1,61,-31);	// +/-30. This gets clamped at the end.
			break;
		case ATMTYPE_Tainted:
			r = 255-50;
			g = 255-50;
			b = 255-50;
			r += dice.Roll(1,101,-51);	// +/-50. This gets clamped at the end.
			g += dice.Roll(1,101,-51);	// +/-50. This gets clamped at the end.
			b += dice.Roll(1,101,-51);	// +/-50. This gets clamped at the end.
			break;
		case ATMTYPE_Exotic:
			r = 255-70;
			g = 255-70;
			b = 255-70;
			r += dice.Roll(1,141,-71);	// +/-70. This gets clamped at the end.
			g += dice.Roll(1,141,-71);	// +/-70. This gets clamped at the end.
			b += dice.Roll(1,141,-71);	// +/-70. This gets clamped at the end.
			break;
		case ATMTYPE_Corrosive:
			r = 255-90;
			g = 255-90;
			b = 255-90;
			r += dice.Roll(1,181,-91);	// +/-90. This gets clamped at the end.
			g += dice.Roll(1,181,-91);	// +/-90. This gets clamped at the end.
			b += dice.Roll(1,181,-91);	// +/-90. This gets clamped at the end.
			break;
		case ATMTYPE_Insidious:
			r = 255-110;
			g = 255-110;
			b = 255-110;
			r += dice.Roll(1,221,-111);	// +/-110. This gets clamped at the end.
			g += dice.Roll(1,221,-111);	// +/-110. This gets clamped at the end.
			b += dice.Roll(1,221,-111);	// +/-110. This gets clamped at the end.
			break;
		}
	}
	else
	{
		// Zero atmosphere density. What is this color used for?
		r = 160;
		g = 80;
		b = 0;
		//qDebug("DaWorldLL::AtmosphereColor atmType %d atmDensity %d id %d", byAtmType, byAtmDensity, id);
	}

	if(r < 0)
		r = 0;
	else if(r > 255)
		r = 255;

	if(g < 0)
		g = 0;
	else if(g > 255)
		g = 255;

	if(b < 0)
		b = 0;
	else if(b > 255)
		b = 255;

	return QColor(r,g,b);
}

// This was cobbled from a shader.
// 'sunLoc' is the sun location, relative to this globe.
// 'P' is the test location, relative to this globe.
float DaWorldLL::ComputeSunLight(const AuPoint& P, const AuPoint& sunLoc, float sunAttenuation, float sunRadius) const
{
	// Calculate surface normal N at P. We'll use the globe normal vs the actual terrain normal so lighting doesn't change as the
	// person walks around bumpy terrain. This is supposed to be the overall contribution of sunlight at a planetary location.
	AuPoint N = -GravityDir(P);

	// Compute the diffuse lighting from center of sun.
	AuPoint toSun = sunLoc - P;
	double distSun = toSun.Length();
	AuPoint L = toSun/distSun;									// Unit vector from P pointing toward the light.
	float NdotL = (float)N.Dot(L);

	// Compute attenuation.
	float attenuation = 1.f / (sunAttenuation * powf((float)distSun,2));

	// Account for radius of solar disk.
	float diskang = atan2f(sunRadius, (float)distSun);
	float sunang = acosf(NdotL);
	float rd1 = cosf(sunang + diskang);
	float rd2 = cosf(sunang - diskang);
	NdotL = MAX(rd1, NdotL);
	NdotL = MAX(rd2, NdotL);

	// Check for sun over the horizon.
	AuPoint U = N;												// Up vector at vertex location, with respect to the globe.
	float LdotU = (float)L.Dot(U);
	float sunang2 = acosf(LdotU);
	float rd3 = cosf(sunang2 + diskang);
	float rd4 = cosf(sunang2 - diskang);
	LdotU = MAX(rd3, LdotU);
	LdotU = MAX(rd4, LdotU);
	if(LdotU <= 0.f)
		return 0.;												// Sun is over the horizon.

	// Compute final diffuse and specular lighting results.
	float lighting = MAX(0.f, NdotL) * attenuation;
	return lighting;
}

// This was cobbled from a shader.
// 'sunLoc' is the sun location, relative to this globe.
// 'P' is the test location, relative to this globe.
float DaWorldLL::ComputeSunLight(const AuPoint& P, const AuPoint& sunLoc, float sunAttenuation, float sunRadius,
		int shadowCount, float shadowAngle, const AuPoint& shadowXVec, const AuPoint& shadowYVec) const
{
	// Compute final diffuse and specular lighting results.
	float lighting = ComputeSunLight(P, sunLoc, sunAttenuation, sunRadius);
	if(lighting == 0.f)
		return 0.f;

	// Modulate result by shadow square effect.
	AuPoint Ps = P - sunLoc;									// Position relative to the sun.
	float sunx = (float)Ps.Dot(shadowXVec);
	float suny = (float)Ps.Dot(shadowYVec);
	float ang = atan2f(suny, sunx) - shadowAngle;					// Angle between P and shadow square in plane of sun
	float sunlight = sinf(ang * shadowCount) * 2.f;
	if(sunlight < 0.f)
		sunlight = 0.f;
	else if(sunlight > 1.f)
		sunlight = 1.f;
	lighting *= sunlight;

	return lighting;
}

// Refresh data from valid counterpart.
bool DaWorldLL::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaWorld::Refresh(pAtom, ari);

	const DaWorldLL* pA = (const DaWorldLL*)pAtom;

	// Geological Features
	pA->m_LLLock.lockForRead();
	m_LLLock.lockForWrite();
	m_ClaimAtLL = pA->m_ClaimAtLL;
	m_ClaimEmpire = pA->m_ClaimEmpire;
	if(ari)
	{
		// We are at the view stage. Watch for changes to geological features that trigger a change to the terrain.
		AuGeologicalFeatures oldGeo(m_GeologicalFeatures);
		m_GeologicalFeatures.clear();

		// Copy the incoming geological features one-by-one.
		for(AuGeologicalFeatures::const_iterator fi = pA->m_GeologicalFeatures.cbegin(); fi != pA->m_GeologicalFeatures.cend();
				fi++)
		{
			// Look for an exact match to the incoming feature in my list of old features.
			bool foundExactMatch = false;
			for(AuGeologicalFeatures::iterator odi = oldGeo.begin(); odi != oldGeo.end(); odi++)
				if(*(*odi) == *(*fi))
				{
					// Found an old geo feature that exactly matches a nu feature.
					// Move it back to my list of 'keepers'; it has not changed.
					m_GeologicalFeatures.append(*odi);
					oldGeo.erase(odi);
					foundExactMatch = true;
					break;
				}
			if(foundExactMatch)
				continue;				// The incoming feature did not change. A match was found among my old features.

			// The incoming development does not match any in my old list. Add it as a nu one.
			m_GeologicalFeatures.append((*fi)->Clone());
			GeologicalFeatureChanged(*fi, ari->mm);				// Notify the world that an area of the model changed.
		}

		// Any features remaining in oldGeo list must be deleted from my list.
		for(AuGeologicalFeatures::iterator odi = oldGeo.begin(); odi != oldGeo.end(); odi++)
		{
			// Delete the old feature and clean up its OpenGL device objects.
			GeologicalFeatureChanged(*odi, ari->mm);			// Notify the world that an area of the model changed.
			(*odi)->DeleteDeviceObjects(*ari->re);
			delete *odi;
		}
	}
	else
	{
		// Straight copy of geological features.
		m_GeologicalFeatures.Clear();
		for(AuGeologicalFeatures::const_iterator fi = pA->m_GeologicalFeatures.cbegin(); fi != pA->m_GeologicalFeatures.cend();
				fi++)
			m_GeologicalFeatures.append((*fi)->Clone());
	}
	m_LLLock.unlock();
	pA->m_LLLock.unlock();
	m_HengePresence = -1;

	// Enclosures
	pA->m_EnclosureLock.lockForRead();
	m_EnclosureLock.lockForWrite();
	if(m_Enclosures)
		delete m_Enclosures;
	if(pA->m_Enclosures)
		m_Enclosures = new AuWorldEnclosureCache(*pA->m_Enclosures);
	else
		m_Enclosures = 0;
	m_EnclosuresValid = pA->m_EnclosuresValid;
	if(m_Irrigation)
		delete m_Irrigation;
	if(pA->m_Irrigation)
		m_Irrigation = new QList<AuWorldIrrigation>(*pA->m_Irrigation);
	else
		m_Irrigation = 0;
	m_EnclosureLock.unlock();
	pA->m_EnclosureLock.unlock();

	return false;
}

// Called when a piece of food wants to decay that is in someone's scene in a city that has a rat infestation.
// 'loc' is the location of the food.
void DaWorldLL::RatEncounter(const AuPoint& loc, const AuSpecies& ratSpecies)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	if(!g_Au.m_EnableAnimalEncounters)
		return;

	AuGlobeLLF encLL;
	if(!LLOfLoc(encLL, loc))
		return;

	// Must query terrain without my own subatoms locked.
	AuTerrainInfo ter;
	QueryTerrain(ter, encLL);

	// Count wild animals present in this terrain.
	int wildAnimals = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
	{
		DaAnimal* animal = dynamic_cast<DaAnimal*>(*ait);
		if(!animal)
			continue;
		if(animal->CurrentHitPoints() <= 0)
			continue;	// Skip dying animals.
		if(animal->IsWild())
			wildAnimals++;
	}

	// Don't bother if there are too many wild animals in this ship.
	if(wildAnimals >= ter.MaxAnimals()*5)
		return;					// Won't spawn because too many wild animals are already present.

	// There's not too many animals.
	// This function gets called once per minute for each piece of food in a spaceship that is in someone's scene.
	// How about a 10% chance each minute of a rat appearing to claim the juicy morsel.
	if(g_Au.Dice().Roll(1,100) > 50)
		return;						// 50% chance that no rat appears

	// Create the animal so we can get physical dimensions.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaAnimal* rat = new DaAnimal(ratSpecies, chroma, hand, Mature);
	rat->ConnectSignals();
	rat->Create();

	// Position the rat in the environment. It will right itself as soon as it moves.
	AuMatrix4x4 t;
	t.Translate(loc);
	t.RotateZ(g_Au.Dice().Roll(1,360,-1)*DEG2RADf);
	rat->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(rat);

	rat->Stand();
	rat->ResetHitPoints();	// Now that it has a body.
	rat->ResetStamina();

	if(!rat->IsAirBreathable())
	{
		// Don't spawn rats where they will die instantly.
		qWarning() << "DaTerrainPatch::RatEncounter spawned rat in unbreathable location" << Id();
		TrashSubatom(rat->Id());
	}
	else
	{
		// Notify the universe that a nu atom was created/added.
		g_Au.AtomAdded(rat->Id());
		rat->SetNormal();	// New atom is ready for normal thread processing.
		rat->TickTock();
	}
}

// Returns true if 'loc' is blocked by any city shield generator that is currently active.
bool DaWorldLL::SubatomShieldsBlockTransporters(quint32 empireId, const AuPoint& loc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaAtom* atom = *it;
		if(!atom)
			continue;
		if(atom->ShieldsBlockPeerTransport(empireId, loc))
			return true;
	}
	return false;
}

AuFloraDNA DaWorldLL::GroundCoverDNA(const AuGlobeLLF& ll) const
{
	const DePlanetFlora* flora = LockFlora();
	if(!flora)
	{
		UnlockFlora();
		return AuFloraDNA();
	}
	AuFloraDNA::ZONE latRegion = LatitudeRegion(ll.Latitude());
	int resourceZone = ResourceZone(ll.Longitude());
	AuFloraDNA floraDNA(flora[resourceZone].GroundCover(latRegion).DNA());
	UnlockFlora();
	return floraDNA;
}

AuFloraDNA DaWorldLL::ShrubDNA(const AuGlobeLLF& ll, COMMODITYID shrubCID) const
{
	const DePlanetFlora* flora = LockFlora();
	if(!flora)
	{
		UnlockFlora();
		return AuFloraDNA();
	}
	AuFloraDNA::ZONE latRegion = LatitudeRegion(ll.Latitude());
	int resourceZone = ResourceZone(ll.Longitude());
	AuFloraDNA floraDNA(flora[resourceZone].Shrub(latRegion, shrubCID).DNA());
	UnlockFlora();
	return floraDNA;
}

AuFloraDNA DaWorldLL::TreeDNA(const AuGlobeLLF& ll, COMMODITYID treeCID) const
{
	const DePlanetFlora* flora = LockFlora();
	if(!flora)
	{
		UnlockFlora();
		return AuFloraDNA();
	}
	AuFloraDNA::ZONE latRegion = LatitudeRegion(ll.Latitude());
	int resourceZone = ResourceZone(ll.Longitude());
	AuFloraDNA floraDNA(flora[resourceZone].Tree(latRegion, treeCID).DNA());
	UnlockFlora();
	return floraDNA;
}

// Rebuilds the Enclosures data for this world based on current cities.
// Returns true if the number of Enclosures changes.
bool DaWorldLL::UpdateEnclosureCache() const
{
	// Enclosures come from buildings.
	AuWorldEnclosureCache enclosures;
	if(BdHere())
	{
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(*it);
			if(!bd)
				continue;
			if(!bd->HasEnclosedEnvironment())
				continue;
			AuWorldEnclosure enclosure;
			if(bd->Enclosure(*this, enclosure) == 2)	// Returns: 0=success, 1=design is not ready, 2=no environment voids.
				continue;								// The building has no environment voids.
			enclosures.append(enclosure);
		}
	}

	// Update my real cache.
	m_EnclosureLock.lockForWrite();

	// Discard old Enclosures cache of data.
	int oldCount;
	if(m_Enclosures)
	{
		oldCount = m_Enclosures->size();
		delete m_Enclosures;
		m_Enclosures = 0;
	}
	else
		oldCount = 0;

	// If there are no Enclosures, we are done.
	if(enclosures.empty())
	{
		m_EnclosuresValid = true;
		m_EnclosureLock.unlock();
		//qDebug() << "DaWorldLL::UpdateEnclosureCache" << *this << "has no Enclosures";
		return oldCount != 0;	// Return true if the count changed.
	}

	// Cache the Enclosures.
	if(!m_Enclosures)
		m_Enclosures = new AuWorldEnclosureCache();
	while(!enclosures.empty())
		m_Enclosures->append(enclosures.takeFirst());
	int nuCount = m_Enclosures->size();
	m_EnclosuresValid = true;
	m_EnclosureLock.unlock();

	//qDebug() << "DaWorldLL::UpdateEnclosureCache" << *this << "has" << nuCount << "Enclosures";
	return oldCount != nuCount;	// Return true if the count changed.
}

// Rebuilds the Enclosures data for this world based on current cities.
void DaWorldLL::UpdateIrrigationCache()
{
	m_EnclosureLock.lockForWrite();
	//qDebug() << "DaWorldLL::UpdateIrrigationCache" << Id() << Name();

	// Discard old Enclosures cache of data.
	if(m_Irrigation)
	{
		delete m_Irrigation;
		m_Irrigation = 0;
	}

	// Geological features hold new style irrigation and defoliated areas.
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		int type;
		if((*gi)->IsIrrigation())
			type = 1;
		else if((*gi)->IsDefoliant())
			type = 2;
		else
			continue;
		if(!m_Irrigation)
			m_Irrigation = new QList<AuWorldIrrigation>();
		AuWorldIrrigation irrig(type, (*gi)->LL(), (*gi)->LLMSL(), (*gi)->LL2(), (*gi)->LL2MSL(), (*gi)->Radius());
		m_Irrigation->append(irrig);
	}
	UnlockGeologicalFeatures();
	m_EnclosureLock.unlock();
}

int DaWorldLL::ChangeRoadConstruction(const AuGlobeLLF& roadDevLL, CONSTRUCTIONPROCESSID cpid)
{
	if(OrbitZone() == OZONE_Inferno && cpid == CPID_AsphaltRoad)
		return 21;	// Asphalt road cannot be built on inferno world.

	// Changing a geo road on this world.
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	const AuGeologicalFeature* geo = geof.FindReconstructable(roadDevLL);
	if(!geo)
	{
		UnlockGeologicalFeatures();
		return 11;
	}
	if(geo->IsUnderConstruction())
	{
		UnlockGeologicalFeatures();
		return 2;
	}
	quint8 geoType;
	switch(cpid)
	{
	default:
		UnlockGeologicalFeatures();
		return 1;
	case CPID_RoadGrade:
		geoType = AuGeoRoadGrade::GeoType();
		break;
	case CPID_DirtRoad:
		geoType = AuGeoRoadDirt::GeoType();
		break;
	case CPID_AsphaltRoad:
		if(OrbitZone() == OZONE_Inferno && !IsGasGiant())
		{
			UnlockGeologicalFeatures();
			return 21;
		}
		geoType = AuGeoRoadAsphalt::GeoType();
		break;
	case CPID_ConcreteRoad:
		geoType = AuGeoRoadConcrete::GeoType();
		break;
	}
	if(geo->Type() == geoType)
	{
		UnlockGeologicalFeatures();
		return 3;
	}
	// Collect properties needed to duplicate the old road.
	AuGlobeLLF startLL = geo->LL2();
	double startMSL = geo->LL2MSL();
	AuGlobeLLF endLL = geo->LL();
	double endMSL = geo->LLMSL();
	bool isSlab = geo->IsSlab();
	float width = geo->Width();
	UnlockGeologicalFeatures();

	// If we get here, it is ok to replace the old road with a new road.
	{
		// Delete the existing road.
		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		const AuGeologicalFeature* geo = geof.FindDeletable(roadDevLL);
		AuGeologicalFeature* geoClone = 0;
		if(geo)
		{
			geoClone = geo->Clone();					// For atom cleanup afterwards.
			geof.DeleteGeo(geo);
			m_tmGeologyChange.Set();
		}
		UnlockGeologicalFeatures();
		if(geoClone)
		{
			if(geoClone->IsIrrigation() || geoClone->IsDefoliant())
				UpdateIrrigationCache();
			delete geoClone;
		}
	}

	// Add the new road under construction in the same place.
	// We don't have to update connections or the road map.
	{
		AuPoint startLoc = LocOfLL(startLL, startMSL);
		AuPoint endLoc = LocOfLL(endLL, endMSL);
		quint32 paveMaterial;
		quint32 sitePrepLabor = 1;												// Repave does not require cut and fill again.
		SitePrepLabor(IID_Road, startLoc, endLoc, width, paveMaterial);			// Calculate pavement material needed.
		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		if(geoType == AuGeoRoadGrade::GeoType())
		{
			DeConstructionProcess cp(CPID_RoadGrade, sitePrepLabor, paveMaterial);
			AuGeoRoadGrade* geo = new AuGeoRoadGrade(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
			geof.append(geo);
			m_tmGeologyChange.Set();
			m_GeoConstruction = 1;
		}
		else if(geoType == AuGeoRoadDirt::GeoType())
		{
			DeConstructionProcess cp(CPID_DirtRoad, sitePrepLabor, paveMaterial);
			AuGeoRoadDirt* geo = new AuGeoRoadDirt(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
			geof.append(geo);
			m_tmGeologyChange.Set();
			m_GeoConstruction = 1;
		}
		else if(geoType == AuGeoRoadAsphalt::GeoType())
		{
			DeConstructionProcess cp(CPID_AsphaltRoad, sitePrepLabor, paveMaterial);
			AuGeoRoadAsphalt* geo = new AuGeoRoadAsphalt(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
			geof.append(geo);
			m_tmGeologyChange.Set();
			m_GeoConstruction = 1;
		}
		else if(geoType == AuGeoRoadConcrete::GeoType())
		{
			DeConstructionProcess cp(CPID_ConcreteRoad, sitePrepLabor, paveMaterial);
			AuGeoRoadConcrete* geo = new AuGeoRoadConcrete(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
			geof.append(geo);
			m_tmGeologyChange.Set();
			m_GeoConstruction = 1;
		}
		UnlockGeologicalFeatures();
	}
	UpdateRoadMap();
	SetModified();
	return 0;
}

QString DaWorldLL::TargetDesc(const AuPoint& spottedLocation, quint32 spotterEmpire, AuId& targetId,
		POLITICAL_STANCE& stanceToTarget) const
{
	Q_UNUSED(spotterEmpire);
	Q_UNUSED(stanceToTarget);
	QString targetDesc;											// This will be our return value.

	targetId = Id();											// Targetted on dirt.

	AuGlobeLL spotLL;
	if(!LLOfLoc(spotLL, spottedLocation))
		return "";

	// Describe resources present at the target location.
	AuTerrainInfo terQ;
	QueryTerrainAtLoc(terQ, spottedLocation);
	targetDesc += terQ.TerrainTypeName();

	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(spotLL.Longitude()));

	// Check for rocks, shrubs and trees at the spot location.
	quint32 worldGenSeed = GenSeed();
	COMMODITYID cidRock = SpotRockAt(spotLL, worldGenSeed);
	bool aquaticPlant;
	COMMODITYID cidShrub = SpotShrubAt(spotLL, worldGenSeed, aquaticPlant);
	COMMODITYID cidTree = SpotTreeAt(spotLL, worldGenSeed);
	if(cidRock != CID_Invalid)
	{
		int q = res.Quality(cidRock);
		if(cidRock == CID_CrudeOil)
		{
			// There is always natural gas present where there is crude oil.
			int qgas = res.Quality(CID_NaturalGas);
			targetDesc += QString(", %1 %2 Q%3, %4 %5 Q%6")
				.arg(DbCommodity::QualityDesc(q))
				.arg(g_Au.CommodityName(cidRock))
				.arg(q)
				.arg(DbCommodity::QualityDesc(qgas))
				.arg(g_Au.CommodityName(CID_NaturalGas))
				.arg(qgas);
		}
		else
		{
			targetDesc += QString(", %1 %2 Q%3")
				.arg(DbCommodity::QualityDesc(q))
				.arg(g_Au.CommodityName(cidRock))
				.arg(q);
		}
	}
	if(cidShrub != CID_Invalid)
	{
		int q = res.Quality(cidShrub);
		//targetDesc += QString(", %1 shrubbery")
			//.arg(g_Au.CommodityName(cidShrub));
		targetDesc += QString(", %1 %2 shrubbery Q%3")
			.arg(DbCommodity::QualityDesc(q))
			.arg(g_Au.CommodityName(cidShrub))
			.arg(q);
	}
	if(cidTree != CID_Invalid)
	{
		int q = res.Quality(cidTree);
		//targetDesc += QString(", %1 tree")
			//.arg(g_Au.CommodityName(cidTree));
		targetDesc += QString(", %1 %2 tree Q%3")
			.arg(DbCommodity::QualityDesc(q))
			.arg(g_Au.CommodityName(cidTree))
			.arg(q);
	}

	// Include terrain location information.
	if(InDesignerPreview())
	{
		targetDesc += tr(" at %1 %2")
			.arg(spotLL.ToString())
			.arg(Name());
	}
	else
	{
		const DaGalaxy* galaxy = FindParentGalaxy();
		if(galaxy)
		{
			const DaSector* sector = FindParentSector();
			targetDesc += tr(" at %1 %2, %3 %4, %5")
				.arg(spotLL.ToString())
				.arg(Name())
				.arg(sector->Name())
				.arg(sector->Description())
				.arg(galaxy->Name());
		}
	}
	return targetDesc;
}

// A distant world has activated a henge portal to this world. I could be a ringworld.
bool DaWorldLL::HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment)
{
	if((g_Au.m_LogMessages & LOG_Travel) || !fromShadowPanels)
		qDebug() << "DaWorldLL::HengeActivated" << Id() << Name() << "from sys" << fromSysId << "sun" << fromSun
			<< "panels" << fromShadowPanels << "arc" << fromArcSegment;
	bool foundHenge = false;

	// Locate the specific henge to activate.
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();

	// Look for the specific henge. I could already have a henge going somewhere else.
	AuGlobeLLF hengeLL;
	QList<AuGlobeLLF> henges;
	for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
	{
		AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		henges.append(henge->LL());

		if(henge->IsDestination(fromSysId, fromSun, fromArcSegment))
		{
			// Activate my henge portal.
			henge->SetActive(true);
			foundHenge = true;
			hengeLL = henge->LL();
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "DaWorldLL::HengeActivated" << Id() << Name() << "activated henge at" << henge->LL();
		}
	}
	UnlockGeologicalFeatures();

	if(!foundHenge)
	{
		// Create my henge portal geology for the first time.
		AuDice dice;
		dice.Seed(GenSeed() + 1000);
		do
		{
			// At least 2 meters up because we sink it 1m into the ground.
			hengeLL = RandomLandLLNotForbidden(dice, FeetPerMeter()*2, 200*FeetPerMeter());
			dice.Random();
		}
		while(henges.contains(hengeLL));
		float altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(hengeLL.Longitude(), hengeLL.Latitude()));
		if(altMSL < 2*(float)FeetPerMeter())
			altMSL = 2*(float)FeetPerMeter();
		float topMSL = altMSL - (float)FeetPerMeter();	// 1m below terrain level.

		// Create a henge that is active.
		AuGeoHenge* geo = new AuGeoHenge(fromSysId, fromSun, fromShadowPanels, fromArcSegment, topMSL, hengeLL);
		geo->SetActive(true);

		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		geof.append(geo);
		UnlockGeologicalFeatures();

		m_HengePresence = 1;

		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug() << "DaWorldLL::HengeActivated" << Id() << Name() << "henge created at" << hengeLL;
	}

	GeologyChanged();
	SetModified();

	// Check for cities within 200m of the henge.
	bool outsideCity = true;
	/*DHH DaCity
	DaCity* city;
	if(LockDevelopmentContaining(hengeLL, 200*FeetPerMeter(), &city, FIND_All))
		outsideCity = false;	// Area is close to developments.
	UnlockDevelopment(city);
	*/
	const DaSolarSystem* system = FindParentSolarSystem();
	emit HengeDestinationSignal(FindParentSector()->Id(), system->Id(), system->Name(), fromSun, CatalogName(system), Name(),
			AtomType(), hengeLL, HasBreathableAtmosphere(), OrbitZone(), outsideCity);
	return true;
}

// A distant world has created a henge portal to this world. I could be a ringworld.
// I could be in resource scan data, to get this done before my system is actually finalized.
void DaWorldLL::HengeCreated(DaSolarSystem* sys, const AuId& mySectorId, const AuId& fromSysId, quint8 fromSun,
		quint8 fromShadowPanels, quint8 fromArcSegment, quint8 destSun, quint8 destArcSegment)
{
	//if((g_Au.m_LogMessages & LOG_Travel) || !fromShadowPanels)
		//qDebug() << "DaWorldLL::HengeCreated" << Id() << Name() << "from sys" << fromSysId << "sun" << fromSun
			//<< "panels" << fromShadowPanels << "arc" << fromArcSegment;
	bool foundHenge = false;

	// Locate the specific henge to activate.
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();

	// Look for the specific henge. I could already have a henge going somewhere else.
	AuGlobeLLF hengeLL;
	QList<AuGlobeLLF> henges;
	for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
	{
		AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		henges.append(henge->LL());

		if(henge->IsDestination(fromSysId, fromSun, fromArcSegment))
		{
			foundHenge = true;
			hengeLL = henge->LL();
			//if(g_Au.m_LogMessages & LOG_Travel)
				//qDebug() << "DaWorldLL::HengeCreated" << Id() << Name() << "found henge at" << henge->LL();
		}
	}
	UnlockGeologicalFeatures();

	if(!foundHenge)
	{
		// Create my henge portal geology for the first time.
		AuDice dice;
		dice.Seed(GenSeed() + 1000);
		do
		{
			// At least 2 meters up because we sink it 1m into the ground.
			hengeLL = RandomLandLLNotForbidden(dice, FeetPerMeter()*2, 200*FeetPerMeter());
			dice.Random();
		}
		while(henges.contains(hengeLL));
		float altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(hengeLL.Longitude(), hengeLL.Latitude()));
		if(altMSL < 2*(float)FeetPerMeter())
			altMSL = 2*(float)FeetPerMeter();
		float topMSL = altMSL - (float)FeetPerMeter();	// 1m below terrain level.

		// Create a henge that is not active.
		AuGeoHenge* geo = new AuGeoHenge(fromSysId, fromSun, fromShadowPanels, fromArcSegment, topMSL, hengeLL);

		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		geof.append(geo);
		UnlockGeologicalFeatures();

		m_HengePresence = 1;

		//if(g_Au.m_LogMessages & LOG_Travel)
			//qDebug() << "DaWorldLL::HengeCreated" << Id() << Name() << "henge created at" << hengeLL;
	}

	if(!foundHenge)
	{
		// We created a henge.
		GeologyChanged();
		SetModified();
	}

	// Check for cities within 200m of the henge.
	bool outsideCity = true;
	/*DHH DaCity
	DaCity* city;
	if(LockDevelopmentContaining(hengeLL, 200*FeetPerMeter(), &city, FIND_All))
		outsideCity = false;	// Area is close to developments.
	UnlockDevelopment(city);
	*/

	QString catName(CatalogName(sys));
	QString myName(Name());

	// Can't use parent sector here because I may be in resource scan data.
	// Cache the result on the real system atom. This world could be in resource scan info because real system is not final.
	sys->RememberHengeDestination(mySectorId, fromSysId, fromSun, fromShadowPanels, fromArcSegment, AtomType(),
			catName, myName, hengeLL, HasBreathableAtmosphere(), OrbitZone(), outsideCity, destSun, destArcSegment);

	emit HengeDestinationSignal(mySectorId, sys->Id(), sys->Name(), fromSun, catName, myName, AtomType(), hengeLL,
			HasBreathableAtmosphere(), OrbitZone(), outsideCity);
}

// A distant world has deactivated a henge portal to this world. I could be a ringworld.
void DaWorldLL::HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment)
{
	if((g_Au.m_LogMessages & LOG_Travel) || !fromShadowPanels)
		qDebug() << "DaWorldLL::HengeDeactivated" << Id() << Name() << "from sys" << fromSysId << "sun" << fromSun
			<< "panels" << fromShadowPanels << "arc" << fromArcSegment;
	bool foundHenge = false;

	// Locate the specific henge to activate.
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();

	// Look for the specific henge. I could already have a henge going somewhere else.
	AuGlobeLLF hengeLL;
	QList<AuGlobeLLF> henges;
	for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
	{
		AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		henges.append(henge->LL());

		if(henge->IsDestination(fromSysId, fromSun, fromArcSegment))
		{
			// Deactivate my henge portal.
			henge->SetActive(false);
			foundHenge = true;
			hengeLL = henge->LL();
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "DaWorldLL::HengeDeactivated" << Id() << Name() << "deactivated henge at" << henge->LL();
		}
	}
	UnlockGeologicalFeatures();

	if(!foundHenge)
	{
		// Create my henge portal geology for the first time.
		AuDice dice;
		dice.Seed(GenSeed() + 1000);
		do
		{
			// At least 2 meters up because we sink it 1m into the ground.
			hengeLL = RandomLandLLNotForbidden(dice, FeetPerMeter()*2, 200*FeetPerMeter());
			dice.Random();
		}
		while(henges.contains(hengeLL));
		float altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(hengeLL.Longitude(), hengeLL.Latitude()));
		if(altMSL < 2*(float)FeetPerMeter())
			altMSL = 2*(float)FeetPerMeter();
		float topMSL = altMSL - (float)FeetPerMeter();	// 1m below terrain level.

		// Create a henge that is not active.
		AuGeoHenge* geo = new AuGeoHenge(fromSysId, fromSun, fromShadowPanels, fromArcSegment, topMSL, hengeLL);

		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		geof.append(geo);
		UnlockGeologicalFeatures();

		m_HengePresence = 1;

		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug() << "DaWorldLL::HengeDeactivated" << Id() << Name() << "henge created at" << hengeLL;
	}

	GeologyChanged();
	SetModified();

	// Check for cities within 200m of the henge.
	bool outsideCity = true;
	/*DHH DaCity
	DaCity* city;
	if(LockDevelopmentContaining(hengeLL, 200*FeetPerMeter(), &city, FIND_All))
		outsideCity = false;	// Area is close to developments.
	UnlockDevelopment(city);
	*/
	const DaSolarSystem* system = FindParentSolarSystem();
	emit HengeDestinationSignal(FindParentSector()->Id(), system->Id(), system->Name(), fromSun, CatalogName(system), Name(),
			AtomType(), hengeLL, HasBreathableAtmosphere(), OrbitZone(), outsideCity);
}

bool DaWorldLL::IsHengeActivationLocation(const AuPoint& loc) const
{
	if(!HasHenge())
		return false;

	const double firePitRadius = 5;
	bool inHenge = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		AuPoint hengeLoc = LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL());
		double dist = hengeLoc.Distance(loc);
		if(dist > firePitRadius && dist < henge->CentralSlabRadius())
		{
			inHenge = true;
			break;
		}
	}
	UnlockGeologicalFeatures();
	return inHenge;
}

// Returns true if 'loc' is in the teleport area of an active henge.
bool DaWorldLL::IsHengeTeleportLocation(const AuPoint& loc, AuId& fromSysId, quint8& fromSun, quint8& fromArc, quint8& shadowPanels,
		AuId& destSysId, quint8& destSun, quint8& destArc) const
{
	Q_UNUSED(fromSysId);
	Q_UNUSED(fromSun);
	Q_UNUSED(fromArc);
	if(!HasHenge())
		return false;

	bool inHenge = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;
		if(!henge->IsActive())
			continue;

		// Henge fire pit inside radius is 4'.
		AuPoint hengeLoc = LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL() + 3);
		if(hengeLoc.Distance(loc) < 4)
		{
			destSysId = henge->DestSysId();
			destSun = henge->DestSun();
			destArc = henge->DestArcSegment();
			shadowPanels = henge->DestShadowPanels();
			inHenge = true;
			break;
		}
	}
	UnlockGeologicalFeatures();
	return inHenge;
}

void DaWorldLL::RenderHengeFires(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	// Check to see if we have any henges before doing this work.
	if(!HasHenge())
		return;

	if(!s_TexFire)
		s_TexFire = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	if(!m_HengeFire)
	{
		const float size = 4;	// Radius of fire.

		m_HengeFire = new AuFireParticles(size);
		m_HengeFire->SetMaxParticles(1000);
		m_HengeFire->SetLifeCycle(1.5f);
		m_HengeFire->SetVelocityVar(size * .67f);
		m_HengeFire->SetNumToRelease(20);
		m_HengeFire->SetReleaseInterval(0.05f);
		QColor colFire;
		colFire.setRgbF(1, .25, .15);
		m_HengeFire->SetColor(colFire);
		m_HengeFire->SetSize(size);
		m_HengeFire->SetTextureID(s_TexFire);
		m_HengeFire->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_HengeFire->Init(re);
	}

	if(!m_HengeFire->Update())
		return;

	// Only indigenous people keep the fire burning.
	bool fireCanBurn = (HasBreathableAtmosphere() || AtmosphereType() == ATMTYPE_Exotic) && HasWorkerSpecies();
	if(!fireCanBurn)
		return;

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetTexture(s_TexFire);
	re.GLSL().SpriteSizeShader().SetSize(m_HengeFire->GetSize()*100);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		AuPoint hengeCenterLoc = LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL());
		AuPoint hpx = model.Project(hengeCenterLoc);	// Henge location in eye space.
		if(hpx.Length2() > 1000*FeetPerMeter() * 1000*FeetPerMeter())
			continue;					// We don't need to see henges too far away.

		if(henge->IsActive())
			re.GLSL().SpriteSizeShader().SetColor(.15f, .25f, 1);	// Blue fire.
		else
			re.GLSL().SpriteSizeShader().SetColor(1, .25f, .15f);	// Regular fire only if atmosphere is breathable.

		AuPoint hengeUpVec = -GravityDir(hengeCenterLoc);
		AuMatrix4x4 hengeM(model);
		hengeM.Translate(hengeCenterLoc);
		hengeM *= AuPoint(0,0,1).GetAlignmentTransform(hengeUpVec);
		hengeM.Translate(0,0,2);
		re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * hengeM);

		// Draw the fire at the center.
		m_HengeFire->Render(projection, hengeM, re);
	}
	UnlockGeologicalFeatures();

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

// Render henge fires.
// The scene atom is at this ring section, either inside or outside the atmosphere.
// The scene atom may also be at an adjacent ring section.
void DaWorldLL::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	DaWorld::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);
	if(sceneAtom->IsInAtmosphere())
		RenderHengeFires(projection, rmod.Matrix(Id()), *ari.re);
}

bool DaWorldLL::HasGeoConstruction() const
{
	if(m_GeoConstruction < 0)
	{
		m_GeoConstruction = 0;
		const AuGeologicalFeatures& geof = LockGeologicalFeatures();
		for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
		{
			if(!(*gi)->IsUnderConstruction())
				continue;
			m_GeoConstruction = 1;
			break;
		}
		UnlockGeologicalFeatures();
	}
	return m_GeoConstruction;
}

bool DaWorldLL::HasHenge() const
{
	if(m_HengePresence < 0)
	{
		m_HengePresence = 0;
		const AuGeologicalFeatures& geof = LockGeologicalFeatures();
		for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
		{
			const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
			if(!henge)
				continue;
			m_HengePresence = 1;
			break;
		}
		UnlockGeologicalFeatures();
	}
	return m_HengePresence;
}

void DaWorldLL::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(HasHenge())
	{
		// Only indigenous people keep the fire burning.
		bool fireCanBurn = (HasBreathableAtmosphere() || AtmosphereType() == ATMTYPE_Exotic) && HasWorkerSpecies();
		if(fireCanBurn)
		{
			bool foundHenge = false;			// True if any henge exists.
			AuPoint closestHenge;				// Eye space location of closest henge fire.
			double closestDist2 = 0;			// Distance to closest henge squared.
			bool closestActive = false;			// True if the closest henge is active.
			double closestFireLight = 0;		// Fire light attenuation radius.
			const AuGeologicalFeatures& geof = LockGeologicalFeatures();
			for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
			{
				const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
				if(!henge)
					continue;

				// Skip inactive henges.
				if(!henge->IsActive())
					continue;

				// Lift the light above the ground level.
				AuPoint hengeCenterLoc = model.Project(LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL() + 3));
				double dist2 = hengeCenterLoc.Length2();
				if(!foundHenge || dist2 < closestDist2)
				{
					closestHenge = hengeCenterLoc;
					closestDist2 = dist2;
					closestActive = henge->IsActive();
					closestFireLight = henge->OuterRingRadius();	// Fire light attenuates to the columns.
				}
				foundHenge = true;
			}
			UnlockGeologicalFeatures();

			if(foundHenge)
			{
				// Henge fire is on.
				QColor colLight;
				if(closestActive)
					colLight.setRgbF(.15f, .25f, 1);				// Blue fire light
				else
					colLight = AuGLLight::ColorTorch();		// Regular fire light.

				float kL, kQ;
				float lightRadius = closestFireLight * AuItem::LightFlicker(CID_StoneFirePit, Id());
				kQ = 1.f/(lightRadius * lightRadius);
				kL = .005f;
				lights.append(AuGLLight(Id(), AuGLLight::HengeFire, kL,kQ, colLight, closestHenge));
			}
		}
	}

	// Make lights for subordinate atoms, particularly the scene atom and potentially subordinate vessels that he might be aboard.
	DaWorld::GetRenderLightsRecursive(person, model, suns, lights);
}

void DaWorldLL::HengeTeleportArrived(DaAtom* arrived, const AuId& fromSysId, quint8 fromSun, quint8 fromArc)
{
	// The henge portal has already been created. I just have to find it.
	AuGlobeLLF portalLL;
	bool destFound = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;
		if(!henge->IsDestination(fromSysId, fromSun, fromArc))
			continue;

		destFound = true;
		portalLL = henge->LL();
		break;
	}
	UnlockGeologicalFeatures();

	// It's too late to not find the henge portal at this point. Put them somewhere.
	if(!destFound)
	{
		portalLL = RandomLandLLNotForbidden(g_Au.Dice(), 1, 200*FeetPerMeter());
		qWarning() << "DaWorldLL::HengeTeleportArrived" << Id() << Name() << "arrived" << arrived->Id() << arrived->Name()
			<< "from sys" << fromSysId << "sun" << fromSun << "arc" << fromArc
			<< "dest portal not found";
	}

	// Create transformation matrix.
	AuPoint portalLoc = LocOfLL(portalLL, SurfaceAltitudeMSL(portalLL));
	AuMatrix4x4 portalM;
	AuPoint vUp = -GravityDir(portalLoc);
	portalM.Translate(portalLoc);
	portalM *= AuPoint(0,0,1).GetAlignmentTransform(vUp);
	portalM.RotateZ(g_Au.Dice().Roll(1,3600)/10. * DEG2RAD);	// Rotate to a random facing to move away from the portal.
	portalM.Translate(10,0,arrived->GroundHeightAboveCG());		// Get out of the teleporter or we'll be going back.

	// Move the arriving creature to this world.
	arrived->ForcePosStop(AuAtomPos(Id(), portalM));			// Position and stop moving.
	arrived->SetModified();
}

void DaWorldLL::SetVillage(const AuVillage& village)
{
	m_VillageLock.lockForWrite();
	if(!m_Village)
		m_Village = new AuVillage(village);
	else
		*m_Village = village;
	m_VillageLock.unlock();
}

// Creates an indigenous village at 'atLL'. Forces movement of current village if necessary.
// 'dna' is used only if current indigenous DNA is not established.
void DaWorldLL::CreateIndigenousVillage(const AuGlobeLLF& atLL, const AuSpecies& species)
{
	//qDebug() << "DaWorldLL::CreateIndigenousVillage" << Id() << Name() << "at" << atLL << "dna" << dna.SpotDescription()
		//<< "isHab" << IsHabitable(HABITABLE_System);
	if(!IsHabitable(HABITABLE_System))
		return;						// This world is not habitable.

	if(!HasWorkerSpecies())
		SetWorkerSpecies(species);	// This is the first encounter in the zone. The DNA gets used.

	// Indigenous encounters always occur near a village of at least 5 people: 2 adults, elder, healer, and stargazer.
	AuPoint vloc = LocOfLL(atLL, TerrainAltitudeMSL(atLL));
	quint8 population;
	int daysSinceContact;
	m_VillageLock.lockForWrite();
	if(!m_Village)
	{
		// Create a new village with at least 4 people.
		daysSinceContact = 0;
		population = g_Au.Dice().Roll(2,3,3);		// 5 to 9
		m_Village = new AuVillage(WorkerSpecies(), population, atLL);
	}
	else
	{
		// Use the existing village.
		// Move the existing village.
		m_Village->MoveTo(atLL);

		// Insure the village has at least 3 people. Grow the population if it is less than the max of 13.
		daysSinceContact = m_Village->LastContacted().ElapsedDay();
		int newPop = (int)m_Village->Population() + daysSinceContact;		// Increase population of the village.
		while(newPop > 13)
			newPop /= 2;													// Village split up. Somebody moved away.
		if(newPop < 5)
			newPop = 5;														// Restart the village with a minimum of 5 people.
		population = newPop;
		m_Village->SetPopulation(population);
	}
	Q_ASSERT(m_Village);

	// Build the village.

	// Check for existing fire pits, tents and villagers.
	bool hasFirePit = false;
	int tentsPresent = 0;
	DaAtomList existingInd;	// Existing indians.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaItem* item = dynamic_cast<const DaItem*>(*it);
		if(item)
		{
			if(item->CommodityId() == CID_StoneFirePit)
			{
				if(item->Location().Distance(vloc) < 6)
					hasFirePit = true;	// Looks like our fire pit. It is placed at the center.
			}
			else if(item->CommodityId() == CID_Tepee)
			{
				if(item->Location().Distance(vloc) < 20*FeetPerMeter())
					tentsPresent++;		// Looks like our tent. We place them 9m to 14m from the center.
			}
		}
		else
		{
			DaNpc* ind = dynamic_cast<DaNpc*>(*it);
			if(ind && ind->IsNpcIndigenous() && !ind->IsBodyDyingOrDead())
				existingInd.append(ind);	// Found a villager we can reuse.
		}
	}

	// Make world transform for center of village. It will be the location of the fire pit and our reference for placing people.
	AuPoint vup = -GravityDir(vloc);
	AuMatrix4x4 upm = AuPoint(0,0,1).GetAlignmentTransform(vup);
	AuMatrix4x4 vm;			// Village matrix to center of fire pit.
	vm.Translate(vloc);
	vm *= upm;

	// Local natural resources determine the quality of their equipment.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(atLL.Longitude()));

	// Set up the encampment.
	if(!hasFirePit)
	{
		// Put a fire pit at the center.
		AuItem* fire = AuItem::Create(CID_StoneFirePit, res.m_StoneQ, 0);
		if(fire)
		{
			fire->SetState(1);		// Light it.
			CreateItem(*fire, vm);	// Create a fire at the center of the camp.
			delete fire;
		}
	}

	double tdistance = 9*FeetPerMeter();	// Distance from center of camp/fire to center of closest tent.
	if(!tentsPresent)
	{
		// No tents are already present.
		// How many tents do we want? Population could be 3 to 13.
		int tents = population / 2;

		// Put them in a circle around the camp.
		AuItem* tent = AuItem::Create(CID_Tepee, (res.m_AnimalsQ + res.m_WoodQ + 1)/2, 0);
		if(tent)
		{
			double tang = PI*2/tents;
			for(int t=0; t<tents; t++)
			{
				// Calculate a tent location in a rough circle relative to the center of the camp.
				double ang = tang*t;
				AuPoint tentLoc;
				if(t == 0)
					tentLoc = AuPoint(tdistance,0,0);
				else
				{
					// Rotate around the circle of tents.
					tentLoc = AuPoint(tdistance + (g_Au.Dice().Roll(1,5)*FeetPerMeter()),0,0).RotateZ(ang);	// Moe away from fire.
				}

				// Convert tent location to a longitude latitude.
				tentLoc = vm.Project(tentLoc);
				AuGlobeLLF tll;
				if(!LLOfLoc(tll, tentLoc))
					continue;

				// Determind MSL altitude of terrain there.
				double altMSL = SurfaceAltitudeMSL(tll);
				if(altMSL < .5)
					continue;	// Don't put tents in the water.

				// Raise/lower tent location to the ground.
				ElevateLocMSL(tentLoc, altMSL);

				// Create a matrix to position the tent.
				AuMatrix4x4 tm;
				tm.Translate(tentLoc);
				tm *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(tentLoc));
				tm.RotateZ(ang + PI/2);

				CreateItem(*tent, tm);
			}

			delete tent;
		}
	}

	AuSpecies vspecies = m_Village->Species();

	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);

	// Create some indigenous people.
	{
		// First create two indigenous for the encounter. We have already ensured the village has a population of at least 5.
		// Choose a location relative to atLL.
		AuGlobeLLF eLL = ChooseEncounterLocation(atLL);
		AuPoint eloc = LocOfLL(eLL, TerrainAltitudeMSL(eLL));
		AuMatrix4x4 em;
		em.Translate(eloc);
		em *= upm;
		for(int ii=0; ii<2; ii++)
		{
			// Spawn indian at em.
			DaNpc* ind = CreateOrMoveIndigenous(ROLE_IndigenousPerson, Mature, vspecies, em, res, existingInd);
			if(!ind)
				break;	// wut?

			// Move a bit for the next dude to spawn.
			em.Translate(4,0,0);

			// Adjust to the terrain altitude at that location.
			eloc = em.Translation();
			if(!LLOfLoc(eLL, eloc))
				break;
			ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
			em.LoadIdentity();
			em.Translate(eloc);
			em *= upm;	// Close enough to the proper up.
		}

		// We know population >= 5.
		int villagers = population - 2;
		for(int ii=0; ii<villagers; ii++)
		{
			AuMatrix4x4 em(vm);
			if(ii == 0)
			{
				// The first one is the village elder.
				// Put the village elder where the first tent was placed (it could be gone).
				em.Translate(tdistance + FeetPerMeter(),0,0);	// Move away from the fire, just past center of 1st tent location.
				em.RotateZ(PI/2);								// Rotate to face the door.

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				CreateOrMoveIndigenous(ROLE_IndigenousElder, Mature, vspecies, em, res, existingInd);
			}
			else if(ii == 1)
			{
				// The second one is the village doctor.
				// Put the village doctor near the fire.
				double dist = g_Au.Dice().Roll(1,2,6);
				double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;
				em.RotateZ(angle);
				em.Translate(dist,0,0);

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				CreateOrMoveIndigenous(ROLE_IndigenousHealer, Mature, vspecies, em, res, existingInd);
			}
			else if(ii == 2)
			{
				// The third one is the village stargazer.
				// Put the village stargazer far from the fire.
				double dist = tdistance*2 + g_Au.Dice().Roll(1,10,10);
				double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;
				em.RotateZ(angle);
				em.Translate(dist,0,0);

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				CreateOrMoveIndigenous(ROLE_IndigenousStargazer, Mature, vspecies, em, res, existingInd);
			}
			else
			{
				// Put remaining villagers randomly around the circle between the fire and the tents.
				double mindist = 7;								// Farther than the ring of stones around the fire pit.
				double maxdist = tdistance - 10;				// Closer than the tents.
				double dist = g_Au.Dice().Roll(1,maxdist-mindist,mindist);
				double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;
				em.RotateZ(angle);
				em.Translate(dist,0,0);

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				if(g_Au.Dice().Roll(1,3) == 1)
					CreateOrMoveIndigenous(ROLE_IndigenousPerson, Juvenile, vspecies, em, res, existingInd);
				else
					CreateOrMoveIndigenous(ROLE_IndigenousPerson, Mature, vspecies, em, res, existingInd);
			}
		}
	}
}

void DaWorldLL::CitizenEncounter(const DaPerson& person)
{
	if(g_Au.m_LogMessages & LOG_AnimalEncounter)
		qDebug() << "DaWorldLL::CitizenEncounter" << *this << "person" << person.Id() << person.Name();

	if(!g_Au.m_EnableIndigenousEncounters)
		return;						// Indigenous encounters disabled.

	// Person must be in a city.
	AuId bdId(person.BdId());
	if(bdId.IsNull())
		return;

	// Don't encounter citizens on roads.
	if(person.IsOnRoad())
		return;

	// Don't encounter citizens in the water. IsInWater is not true while dry inside an undersea city.
	if(person.IsInWater())
		return;

	// We must find the bd.
	DaBd* bd = FindBd(bdId);
	if(!bd)
		return;
	POLITICAL_STANCE stance = bd->PoliticalStanceMakeContact(&person);
	if(stance == POL_Enemy)
		return;
	AuId bdCapitolId(bd->CapitolId());

	// Locate existing workers and count nearby workers.
	DaAtomList distantWorkers;				// Existing workers far enough away to be reused.
	DaAtomList existingWorkers;				// Existing workers.
	int nearbyPopulation = 0;
	int workerPopulation = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* worker = dynamic_cast<DaNpc*>(*it);
		if(!worker)
			continue;
		if(!worker->IsNpcWorker())
			continue;
		if(worker->InIndustry() == IID_Road ||
				worker->BdId() == bdId ||
				(!bdCapitolId.IsNull() && worker->InBdCity() == bdCapitolId))
		{
			existingWorkers.append(worker);
			workerPopulation++;
			if(worker->Location().Distance(person.Location()) < 50*FeetPerMeter())
				nearbyPopulation++;				// Found a worker nearby.
			else if(!worker->IsBodyDyingOrDead())
				distantWorkers.append(worker);	// Found a live worker that has gotten far enough away to reuse.
		}
	}
	if(person.Parent() == bd && bd->CitizenEncounter(person, existingWorkers))
		return;	// An encounter happened inside the building. It could have taken one of the existing workers.

	// Insure city population has not been reached.
	int citizensAvailable = bd->CommunityPopulation() - workerPopulation;
	if(citizensAvailable <= 0)
		return;

	// Insure too many people do not spawn in the same area.
	// Limit maximum encounter size to the city density.
	float cityAmbientLight = CityAmbientLight(person.GlobeLL(), WORLDTEX_Count);
	int maxNearbyPeople = (int)ceilf(cityAmbientLight * 10.f);
	if(g_Au.m_LogMessages & LOG_AnimalEncounter)
		qDebug() << "DaWorldLL::CitizenEncounter" << Id() << Name() << "ambience" << cityAmbientLight << "maxPeopleNearby"
			<< maxNearbyPeople << "people" << nearbyPopulation;
	if(nearbyPopulation > maxNearbyPeople)
		return;

	// Spawn a worker at an unoccupied post in the current building.
	if(bd->SpawnWorker(this, distantWorkers) == 0)
		return;	// Worker spawned in the building.

	// Choose a location for the encounter within earshot of the person.
	AuGlobeLLF encLL = RandomCitizenLLNear(g_Au.Dice(), person.GlobeLL(), 5*FeetPerMeter(), 20*FeetPerMeter());
	if(encLL == AuGlobeLLF())
		return;

	// Encounter one to three citizens.
	int citizensToSpawn = g_Au.Dice().Roll(1,3);
	if(citizensToSpawn > citizensAvailable)
		citizensToSpawn = citizensAvailable;

	// Spawn citizens near the person.
	for(int c=0; c<citizensToSpawn; c++)
	{
		// Each citizen appears near the encounter location.
		AuGlobeLLF citizenLL = RandomCitizenLLNear(g_Au.Dice(), encLL, 1, 2*FeetPerMeter());
		if(citizenLL == AuGlobeLLF())
			continue;

		// Can we reuse an existing citizen?
		DaNpc* worker;
		if(distantWorkers.isEmpty())
		{
			// Create a new citizen worker.
			worker = SpawnCitizenWorker(bd, citizenLL);
		}
		else
		{
			// Reuse an existing worker.
			worker = (DaNpc*)distantWorkers.takeFirst();

			// Place the nu creature at exactly the right altitude.
			AuPoint spawnAt = GetSurfaceLocation(citizenLL, worker->GroundHeightAboveCG());

			// Position the citizen in the environment.
			AuMatrix4x4 t;
			t.Translate(spawnAt);												// Move to location in terrain.
			t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Align with up.

			// Rotate the citizen randomly on its z axis.
			double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
			t.RotateZ(rot);
			worker->ChangePosAndOrient(AuAtomPos(Id(), t));
			worker->InvalidateEnvironment();
			worker->GoToWork();
			worker->SetInScene();			// Only spawns when in a scene. Set this so it doesn't suicide immediately.
		}

		// Once in a while a citizen will approach the avatar. This is not done when they spawn inside buildings because their
		// pathing logic is too bad to bother trying to move.
		if(worker && !person.IsInsideBd() && g_Au.Dice().Roll(1,10000) <= 1667)
			worker->SetOrder(ORDER_GoToTarget, person.Id(), AuPoint(0,0));
	}
}

// Test for encounter with indigenous natives.
// 'mode': 0=normal, 1=force encounter, 2=force village to move and force encounter
// Returns: 0=ok
//  1=indigenous encounters disabled
//  2=uninhabitable world
//  5=nobody present
//  6=no place to spawn a village near 'nearLL'
//  7=village is too far away
//  8=person is in the village already
//  9=village had an encounter within the last hour
int DaWorldLL::IndigenousEncounter(const DaPerson& person, const AuGlobeLLF& nearLL, int mode)
{
	if(g_Au.m_LogMessages & LOG_AnimalEncounter)
		qDebug() << "DaWorldLL::IndigenousEncounter" << Id() << Name() << "nearLL" << nearLL << "mode" << mode;

	if(!g_Au.m_EnableIndigenousEncounters)
		return 1;						// Indigenous encounters disabled.

	if(!IsHabitable(HABITABLE_System))
		return 2;						// This world is not habitable.

	// No encounters if no atoms present.
	if(!HasSubatoms())
		return 5;						// Won't spawn when no other atoms are present, namely people.

	AuGlobeLLF vLL = ChooseVillageLocation(nearLL);
	if(vLL == AuGlobeLLF())
		return 6;						// No suitable spot found for a village.

	if(!HasWorkerSpecies())
	{
		// This is the first encounter.
		// It is possible (20%?) pirates in the region visited this planet once.
		if(g_Au.Dice().Roll(1,5) == 1)
		{
			GALAXY galaxy = Galaxy();
			const DgData& gd = g_Au.LockDataForRead();
			quint32 empireId = gd.RandomPirateEmpire(galaxy);
			AuSpecies species = gd.EmpireCapitolSpecies(empireId);
			g_Au.UnlockData();
			SetWorkerSpecies(species);
		}
		else
		{
			// The person's species gets used.
			SetWorkerSpecies(person.Species());
		}
	}

	// Indigenous encounters always occur near a village.
	AuPoint vloc;
	quint8 population;
	int daysSinceContact;
	m_VillageLock.lockForWrite();
	if(!m_Village)
	{
		// Create a new village with at least 4 people.
		daysSinceContact = 0;
		population = g_Au.Dice().Roll(2,3,3);		// 5 to 9
		m_Village = new AuVillage(WorkerSpecies(), population, vLL);
		vloc = LocOfLL(vLL, TerrainAltitudeMSL(vLL));
	}
	else
	{
		// Use the existing village.
		// If the village was contacted recently, then somebody might know where it is, or be in the village. In that case, they
		// will have to make do with whatever parts of the village are still there. We don't want to keep replacing the fire pit
		// and tents and villagers every time they get destroyed.
		if(m_Village->ContactedRecently() && mode == 0)
		{
			// The village was contacted recently.
			m_VillageLock.unlock();
			return 9;
		}

		// Check location of existing village.
		vloc = LocOfLL(m_Village->LL(), TerrainAltitudeMSL(m_Village->LL()));
		AuPoint nloc = LocOfLL(nearLL, TerrainAltitudeMSL(nearLL));
		double distToVillage = nloc.Distance(vloc);
		if(distToVillage > d_MaxVillageRange)
		{
			// The village is too far away. Villages can move after being in one spot for an hour.
			if(m_Village->MovedRecently() && mode != 2)
			{
				// Village is too far away and it can't be moved yet.
				m_VillageLock.unlock();
				return 7;
			}

			// Move the existing village.
			if(g_Au.m_LogMessages & LOG_AnimalEncounter)
				qDebug() << "DaWorldLL::IndigenousEncounter" << Id() << Name() << "village moved to" << vLL;
			m_Village->MoveTo(vLL);

			// Update local variables with new village location.
			vloc = LocOfLL(vLL, TerrainAltitudeMSL(vLL));
		}
		else if(distToVillage < d_MinVillageRange && mode != 2)
		{
			// The village is too close. The person is already in the village. It's weird to have the encounter at this range.
			// It's not needed if people and tents are still standing.
			m_VillageLock.unlock();
			return 8;
		}
		else
		{
			// Village is close enough to leave it right where it is.
			vLL = m_Village->LL();
		}

		// Insure the village has at least 3 people. Grow the population if it is less than the max of 13.
		daysSinceContact = m_Village->LastContacted().ElapsedDay();
		int newPop = (int)m_Village->Population() + daysSinceContact;		// Increase population of the village.
		while(newPop > 13)
			newPop /= 2;													// Village split up. Somebody moved away.
		if(newPop < 5)
			newPop = 5;														// Restart the village with a minimum of 5 people.
		population = newPop;
		m_Village->SetPopulation(population);
	}
	Q_ASSERT(m_Village);

	// Build the village for the encounter.

	// Make world transform for center of village. It will be the location of the fire pit and our reference for placing people.
	AuPoint vup = -GravityDir(vloc);
	AuMatrix4x4 upm = AuPoint(0,0,1).GetAlignmentTransform(vup);
	AuMatrix4x4 vm;			// Village matrix to center of fire pit.
	vm.Translate(vloc);
	vm *= upm;

	// Check for existing fire pits, tents and villagers.
	bool hasFirePit = false;
	int tentsPresent = 0;
	DaAtomList existingInd;	// Existing indians.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaItem* item = dynamic_cast<const DaItem*>(*it);
		if(item)
		{
			if(item->CommodityId() == CID_StoneFirePit)
			{
				if(item->Location().Distance(vloc) < 6)
					hasFirePit = true;	// Looks like our fire pit. It is placed at the center.
			}
			else if(item->CommodityId() == CID_Tepee)
			{
				if(item->Location().Distance(vloc) < 20*FeetPerMeter())
					tentsPresent++;		// Looks like our tent. We place them 9m to 14m from the center.
			}
		}
		else
		{
			DaCitizen* ind = dynamic_cast<DaCitizen*>(*it);
			if(ind && ind->IsNpcIndigenous() && !ind->IsBodyDyingOrDead())
				existingInd.append(ind);	// Found a villager we can reuse.
		}
	}

	if(g_Au.m_LogMessages & LOG_AnimalEncounter)
		qDebug() << "DaWorldLL::IndigenousEncounter" << Id() << Name() << "village at" << vLL << "population" << population
			<< "hasFire" << hasFirePit << "tents" << tentsPresent << "existingInd" << existingInd.count();

	// Local natural resources determine the quality of their equipment.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(vLL.Longitude()));

	// Set up the encampment.
	if(!hasFirePit)
	{
		// Put a fire pit at the center.
		AuItem* fire = AuItem::Create(CID_StoneFirePit, res.m_StoneQ, 0);
		if(fire)
		{
			fire->SetState(1);		// Light it.
			CreateItem(*fire, vm);	// Create a fire at the center of the camp.
			if(g_Au.m_LogMessages & LOG_AnimalEncounter)
				qDebug() << "DaWorldLL::IndigenousEncounter" << Id() << Name() << "fire pit created at" << vLL;
			delete fire;
		}
	}

	double tdistance = 9*FeetPerMeter();	// Distance from center of camp/fire to center of closest tent.
	if(!tentsPresent)
	{
		// No tents are already present.
		// How many tents do we want? Population could be 3 to 13.
		int tents = population / 2;

		// Put them in a circle around the camp.
		AuItem* tent = AuItem::Create(CID_Tepee, (res.m_AnimalsQ + res.m_WoodQ + 1)/2, 0);
		if(tent)
		{
			double tang = PI*2/tents;
			for(int t=0; t<tents; t++)
			{
				// Calculate a tent location in a rough circle relative to the center of the camp.
				double ang = tang*t;
				AuPoint tentLoc;
				if(t == 0)
					tentLoc = AuPoint(tdistance,0,0);
				else
				{
					// Rotate around the circle of tents.
					tentLoc = AuPoint(tdistance + (g_Au.Dice().Roll(1,5)*FeetPerMeter()),0,0).RotateZ(ang);	// Moe away from fire.
				}

				// Convert tent location to a longitude latitude.
				tentLoc = vm.Project(tentLoc);
				AuGlobeLLF tll;
				if(!LLOfLoc(tll, tentLoc))
					continue;

				// Determind MSL altitude of terrain there.
				double altMSL = SurfaceAltitudeMSL(tll);
				if(altMSL < .5)
					continue;	// Don't put tents in the water.

				// Raise/lower tent location to the ground.
				ElevateLocMSL(tentLoc, altMSL);

				// Create a matrix to position the tent.
				AuMatrix4x4 tm;
				tm.Translate(tentLoc);
				tm *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(tentLoc));
				tm.RotateZ(ang + PI/2);

				CreateItem(*tent, tm);
			}

			delete tent;
		}
	}

	// What is the person's current reputation in the village?
	const AuSpecies& vspecies = m_Village->Species();
	bool hasMet = m_Village->HasMet(person.Id());

	// Reduce the village's negative reputation of people over time. They keep positive memories of people.
	if(daysSinceContact > 0)
		m_Village->SoftenReputations(daysSinceContact);

	// What is the reaction of the village to the person?
	qint8 react = 0;
	if(!hasMet)
	{
		// Reaction is based on DNA.
		AuSpecies pspecies(person.Species());
		if(pspecies == vspecies)
			react = 10;
		else if(pspecies.IsSexuallyCompatible(vspecies))
			react = 5;
		else
			react = 1;
	}
	else
	{
		// Reaction is based on their reputation.
		react = m_Village->Reputation(person.Id());
	}

	if(g_Au.m_LogMessages & LOG_AnimalEncounter)
		qDebug() << "DaWorldLL::IndigenousEncounter" << Id() << Name() << "village contact reaction" << react;
	m_Village->SetContacted(person.Id(), react);

	// Build the encounter with the indigenous natives.
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);

	// Create some indigenous people.
	{
		// First create two indigenous for the encounter. We have already ensured the village has a population of at least 3.
		// Choose a location relative to nearLL.
		AuGlobeLLF eLL = ChooseEncounterLocation(nearLL);
		AuPoint eloc = LocOfLL(eLL, TerrainAltitudeMSL(eLL));
		AuMatrix4x4 em;
		em.Translate(eloc);
		em *= upm;

		// Decide what the greeters/guards will do.
		CITIZEN_ORDER order;
		if(react <= -10)
			order = ORDER_AttackTarget;
		else
			order = ORDER_GoToTarget;

		for(int ii=0; ii<2; ii++)
		{
			// Spawn indian at em.
			DaNpc* ind = CreateOrMoveIndigenous(ROLE_IndigenousPerson, Mature, vspecies, em, res, existingInd);
			if(!ind)
				break;	// wut?

			ind->SetOrder(order, person.Id(), AuPoint(0,0));

			// Decide what to say.
			QString sayWhat;
			if(!hasMet)
			{
				// Person has never been met.
				// The reaction of our indigenous to strangers is always positive.
				if(ii == 0)
				{
					// First indigenous in the group.
					if(react >= 10)
						sayWhat = tr("Greetings %1. Our humble village is a short way %2 of here.")
							.arg(person.MaamSir(false))
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
					else if(react >= 5)
						sayWhat = tr("Hello traveller. Welcome to %1. You will find our village a short walk to the %2.")
							.arg(Name())
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
					else
						sayWhat = tr("Hail alien being. Go in peace to our village, %1 a short distance.")
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
				}
				else
				{
					// Second indigenous in the group.
					if(react >= 10)
						sayWhat = tr("%1 is a fine looking one.")
							.arg(person.SheHe(true));
					else if(react >= 5)
						sayWhat = tr("Who or what is that? %1 is kind of cute.")
							.arg(person.SheHe(true));
					else
						sayWhat = tr("Is that thing dangerous?");
				}
			}
			else
			{
				// Person has been met and has a reputation with the village.
				if(ii == 0)
				{
					// First indigenous in the group.
					if(react >= 10)
						sayWhat = tr("Welcome back %1. The village is %2. Everyone will want to hear about your travels.")
							.arg(person.Name())
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
					else if(react >= 5)
						sayWhat = tr("Hello %1. It is good to see you again. Our village is %2 of here.")
							.arg(person.Name())
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
					else if(react > 0)
						sayWhat = tr("Hail %1. You will find our village a short distance to the %2.")
							.arg(person.Name())
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
					else if(react == 0)
						sayWhat = tr("%1, attend to your business and go. Our village is %2; do not stray.")
							.arg(person.Name())
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
					else if(react < -10)
						sayWhat = tr("%1 is back! Get %2! Bring me %3 head.")
							.arg(person.Name())
							.arg(person.HerHim(false))
							.arg(person.HersHis(false));
					else if(react < -5)
						sayWhat = tr("Look, it is %1. Drive %2 away. %3 is to be shunned.")
							.arg(person.Name())
							.arg(person.HerHim(false))
							.arg(person.SheHe(true));
					else
						sayWhat = tr("You are not welcome here %1. I will be watching you. "
								"Our village is %2; you should go the other way.")
							.arg(person.Name())
							.arg(person.GlobeLL().DirectionTo(vLL, LLShape()));
				}
				else
				{
					// Second indigenous in the group.
					if(react >= 10)
						sayWhat = tr("I will tell everyone %1 is here.").arg(person.SheHe(false));
					else if(react >= 5)
						sayWhat = tr("%1 is back.").arg(person.Name());
					else if(react > 0)
						sayWhat = tr("I have heard of %1.").arg(person.HerHim(false));
					else if(react == 0)
						sayWhat = tr("If %1 tries something, should I kill %2?")
							.arg(person.SheHe(false))
							.arg(person.HerHim(false));
					else if(react < -10)
						sayWhat = tr("You can have %1 head. I will take a different piece of %2.")
							.arg(person.HersHis(false))
							.arg(person.HerHim(false));
					else if(react < -5)
						sayWhat = tr("Foul creature! Let me kill it now.");
					else
						sayWhat = tr("Should I whack %1 when %2 turns around?")
							.arg(person.HerHim(false))
							.arg(person.SheHe(false));
				}
			}
			AuVisage visage;
			if(react <= -10)
				visage = AuVisage::Sad;
			else if(react <= -5)
				visage = AuVisage::Frown;
			else if(react == 0)
				visage = AuVisage::Closed;
			else if(react >= 10)
				visage = AuVisage::Grin;
			else
				visage = AuVisage::Smile;

			if(!sayWhat.isEmpty())
			{
				AuCommChat msg(sayWhat, AuCommStdChannel::VoiceChannelName(), ind->Id(), ind->Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				ind->Say(msg, visage, person.Id());	// This causes me to turn to face the person I am talking to.
			}

			// Move a bit for the next dude to spawn.
			em.Translate(4,0,0);

			// Adjust to the terrain altitude at that location.
			eloc = em.Translation();
			if(!LLOfLoc(eLL, eloc))
				break;
			ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
			em.LoadIdentity();
			em.Translate(eloc);
			em *= upm;	// Close enough to the proper up.
		}

		// We know population >= 5.
		int villagers = population - 2;
		for(int ii=0; ii<villagers; ii++)
		{
			AuMatrix4x4 em(vm);
			if(ii == 0)
			{
				// The first one is the village elder.
				// Put the village elder where the first tent was placed (it could be gone).
				em.Translate(tdistance + FeetPerMeter(),0,0);	// Move away from the fire, just past center of 1st tent location.
				em.RotateZ(PI/2);								// Rotate to face the door.

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				CreateOrMoveIndigenous(ROLE_IndigenousElder, Mature, vspecies, em, res, existingInd);
			}
			else if(ii == 1)
			{
				// The second one is the village doctor.
				// Put the village doctor near the fire.
				double dist = g_Au.Dice().Roll(1,2,6);
				double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;
				em.RotateZ(angle);
				em.Translate(dist,0,0);

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				CreateOrMoveIndigenous(ROLE_IndigenousHealer, Mature, vspecies, em, res, existingInd);
			}
			else if(ii == 2)
			{
				// The third one is the village stargazer.
				// Put the village stargazer far from the fire.
				double dist = tdistance*2 + g_Au.Dice().Roll(1,10,10);
				double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;
				em.RotateZ(angle);
				em.Translate(dist,0,0);

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				CreateOrMoveIndigenous(ROLE_IndigenousStargazer, Mature, vspecies, em, res, existingInd);
			}
			else
			{
				// Put remaining villagers randomly around the circle between the fire and the tents.
				double mindist = 7;								// Farther than the ring of stones around the fire pit.
				double maxdist = tdistance - 10;				// Closer than the tents.
				double dist = g_Au.Dice().Roll(1,maxdist-mindist,mindist);
				double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;
				em.RotateZ(angle);
				em.Translate(dist,0,0);

				// Adjust to the terrain altitude at that location.
				eloc = em.Translation();
				if(!LLOfLoc(eLL, eloc))
					continue;
				ElevateLocMSL(eloc, TerrainAltitudeMSL(eLL));
				em.LoadIdentity();
				em.Translate(eloc);
				em *= upm;	// Close enough to the proper up.

				if(g_Au.Dice().Roll(1,3) == 1)
					CreateOrMoveIndigenous(ROLE_IndigenousPerson, Juvenile, vspecies, em, res, existingInd);
				else
				{
					DaNpc* ind = CreateOrMoveIndigenous(ROLE_IndigenousPerson, Mature, vspecies, em, res, existingInd);

					// Adults may join in the attack.
					if(order == ORDER_AttackTarget)
						ind->SetOrder(order, person.Id(), AuPoint(0,0));
				}
			}
		}
	}
	return 0;
}

DaItem* DaWorldLL::CreateItem(const AuGlobeLLF& ll, const AuItem& item)
{
	AuPoint loc = LocOfLL(ll, TerrainAltitudeMSL(ll));

	AuPoint vUp = -GravityDir(loc);

	AuMatrix4x4 m;
	m.Translate(loc);
	m *= AuPoint(0,0,1).GetAlignmentTransform(vUp);

	return CreateItem(item, m);
}

DaItem* DaWorldLL::CreateItem(const AuItem& item, const AuMatrix4x4& m)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!item.IsValid())
	{
		qWarning("DaWorldLL::CreateItem item cid %d is not valid", item.CommodityId());
		return 0;
	}

	// Create a commodity atom to encapsulate the inventory item.
	// The item is initially 'in motion' so it will fall/rise from wherever we place it to its natural resting point.
	DaItem *pc = new DaItem(item);
	pc->ConnectSignals();
	pc->Create();
	pc->SetPosAndOrient(AuAtomPos(Id(), m));
	ether->AddAtom(pc);

	pc->SetInMotion();					// So it will adjust to terrain.
	pc->SetInScene();					// Don't decay too fast right away.

	// Notify the world of the nu atom.
	g_Au.AtomAdded(pc->Id());
	pc->SetNormal();					// New atom is ready for normal thread processing.
	pc->TickTock();
	return pc;
}

bool DaWorldLL::HasVillage() const
{
	m_VillageLock.lockForRead();
	bool v = m_Village && m_Village->Population();
	m_VillageLock.unlock();
	return v;
}

AuGlobeLLF DaWorldLL::ChooseEncounterLocation(const AuGlobeLLF& nearLL) const
{
	// Try 10 random locations near 'nearLL'.
	AuPoint nloc = LocOfLL(nearLL);
	AuPoint vup = -GravityDir(nloc);
	AuPoint vx;
	vup.ArbitraryAxis(vx);
	AuPoint vy = vup.Cross(vx);
	for(int itry=0; itry<10; itry++)
	{
		double minrange = 10*FeetPerMeter();
		double maxrange = 20*FeetPerMeter();	// Much farther than this and they will be out of hearing range.
		double distance = g_Au.Dice().Roll(1,maxrange-minrange,minrange);
		double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;						// Random direction.

		double ca = cos(angle) * distance;
		double sa = sin(angle) * distance;

		AuPoint tryloc = nloc + vx * ca + vy * sa;
		AuGlobeLLF tryll;
		if(!LLOfLoc(tryll, tryloc))
			continue;

		// Is the location above water.
		double terAltMSL = TerrainAltitudeMSL(tryll);
		if(terAltMSL < 1)
			continue;	// Too deep of water.

		// Check for developments near the location.
		float cityAmbientLight = CityAmbientLight(tryll, WORLDTEX_Count);
		if(cityAmbientLight > 0.01f)
			continue;	// Too much city ambient light there.

		// Check for units that are not wild animals.
		bool clearOfUnits = true;
		if(HasSubatoms())
		{
			// Adjust tryloc to terrain altitude.
			ElevateLocMSL(tryloc, terAltMSL);

			// Make a list of the empires who have cities on this planet.
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				const DaUnit* unit = dynamic_cast<const DaUnit*>(*it);
				if(!unit)
					continue;	// Atom is not a unit.
				const DaAnimal* animal = dynamic_cast<const DaAnimal*>(unit);
				if(animal && animal->IsWild())
					continue;	// Ignore wild animals.
				if(tryloc.Distance2(unit->Location()) >= minrange*minrange)
					continue;	// Unit is too far away to care.
				clearOfUnits = false;
				break;
			}
		}
		if(!clearOfUnits)
			continue;	// A unit was found too close.

		// Any other criteria to prevent a villager encounter at tryll?

		return tryll;
	}
	return AuGlobeLLF();
}

AuGlobeLLF DaWorldLL::ChooseVillageLocation(const AuGlobeLLF& nearLL) const
{
	// Try 10 random locations near 'nearLL'.
	AuPoint nloc = LocOfLL(nearLL);
	AuPoint vup = -GravityDir(nloc);
	AuPoint vx;
	vup.ArbitraryAxis(vx);
	AuPoint vy = vup.Cross(vx);
	for(int itry=0; itry<10; itry++)
	{
		// Sight range for tepees and fire pits is 200m. Farthest tepee could be 15m farther than the fire pit.
		double distance = g_Au.Dice().Roll(1,d_MaxVillageRange-d_MinVillageRange,d_MinVillageRange);
		double angle = g_Au.Dice().Roll(1,360) * DEG2RAD;						// Random direction.

		double ca = cos(angle) * distance;
		double sa = sin(angle) * distance;

		AuPoint tryloc = nloc + vx * ca + vy * sa;
		AuGlobeLLF tryll;
		if(!LLOfLoc(tryll, tryloc))
			continue;

		// Is the location above water.
		double terAltMSL = TerrainAltitudeMSL(tryll);
		if(terAltMSL < 1)
			continue;	// Too close to water level.

		// Is the location in a forbidden zone.
		if(IsForbiddenZone(tryll, terAltMSL, 100*FeetPerMeter()))
			continue;	// Too close to a forbidden zone.

		// Check for developments near the location.
		float cityAmbientLight = CityAmbientLight(tryll, WORLDTEX_Count);
		if(cityAmbientLight > 0.01f)
			continue;	// Too much city ambient light there.

		/*DHH DaCity
		DaCity* city;
		const DpDevelopment* dev = LockDevelopmentContaining(tryll, 200*FeetPerMeter(), &city, FIND_All);
		if(dev)
		{
			// Area is close to developments.
			UnlockDevelopment(city);
			continue;
		}
		UnlockDevelopment(city);
		*/

		// No developments within 200 meters.
		// Check for units that are not wild animals.
		bool clearOfUnits = true;
		if(HasSubatoms())
		{
			// Adjust tryloc to terrain altitude.
			ElevateLocMSL(tryloc, terAltMSL);

			// Make a list of the empires who have cities on this planet.
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				const DaUnit* unit = dynamic_cast<const DaUnit*>(*it);
				if(!unit)
					continue;	// Atom is not a unit.
				const DaAnimal* animal = dynamic_cast<const DaAnimal*>(unit);
				if(animal && animal->IsWild())
					continue;	// Ignore wild animals.
				if(tryloc.Distance2(unit->Location()) >= d_MinVillageRange*d_MinVillageRange)
					continue;	// Unit is too far away to care.
				clearOfUnits = false;
				break;
			}
		}
		if(!clearOfUnits)
			continue;	// A unit was found too close.

		// Any other criteria to prevent a village moving to tryll?

		return tryll;
	}
	return AuGlobeLLF();
}

AuGlobeLLF DaWorldLL::VillageLL() const
{
	m_VillageLock.lockForRead();
	AuGlobeLLF vll;
	if(m_Village)
		vll = m_Village->LL();
	m_VillageLock.unlock();
	return vll;
}

qint8 DaWorldLL::VillageReputation(const AuId& personId) const
{
	qint8 rep;
	m_VillageLock.lockForRead();
	if(m_Village)
		rep = m_Village->Reputation(personId);
	else
		rep = 0;
	m_VillageLock.unlock();
	return rep;
}

void DaWorldLL::VillageResetReputation(const AuId& personId)
{
	m_VillageLock.lockForRead();
	if(m_Village)
		m_Village->ResetReputation(personId);
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);

	// Stop indigenous from attacking the person.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* ind = dynamic_cast<DaNpc*>(*it);
		if(!ind || !ind->IsNpcIndigenous())
			continue;
		if(ind->OrderTarget() == personId)
			ind->RequestOrder();
	}
}

void DaWorldLL::VillageSetContacted(const AuId& personId, qint8 initialReputation)
{
	m_VillageLock.lockForRead();
	if(m_Village)
		m_Village->SetContacted(personId, initialReputation);
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);
}

// 'existingInd' contains existing indigenous that are not dying or dead.
DaNpc* DaWorldLL::CreateOrMoveIndigenous(NPC_ROLE role, GROWTHSTAGE stage, const AuSpecies& species, const AuMatrix4x4& m,
		const AuZoneResource& res, DaAtomList& existingInd)
{
	// See if we can move an existing indigenous with the desired role.
	for(DaAtomList::iterator vi = existingInd.begin(); vi != existingInd.end(); vi++)
	{
		DaNpc* ind = dynamic_cast<DaNpc*>(*vi);
		if(!ind)
			continue;	// wut?
		if(ind->Role() != role)
			continue;
		if(ind->GrowthStage() != stage)
			continue;

		// Take the villager from the list of those available.
		existingInd.erase(vi);

		// Move the villager to exactly the right altitude.
		AuMatrix4x4 im(m);
		im.Translate(0,0, ind->GroundHeightAboveCG());
		ind->ChangePosAndOrient(AuAtomPos(Id(), im));
		ind->InvalidateEnvironment();
		ind->GoToWork();
		ind->SetInScene();
		ind->SetModified();

		// Insure they have a good set of equipment. They might have been swindled out of their old stuff.
		ind->InitIndigenousGear(res);

		ind->SetOrder(ORDER_Stay, AuId(0), AuPoint());

		return ind;
	}

	// An existing villager was not moved. Create a new one.
	return CreateIndigenous(role, stage, species, m, res);
}

DaNpc* DaWorldLL::CreateIndigenous(NPC_ROLE role, GROWTHSTAGE stage, const AuSpecies& species, const AuMatrix4x4& m,
		const AuZoneResource& res)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Create an indigenous native citizen.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* ind = new DaNpc(role, species, chroma, hand, g_Au.Dice().RandomPersonName());
	ind->SetHomeWorld(this);
	if(stage == Juvenile)
		ind->InitTimeToFullGrown();
	ind->ConnectSignals();
	ind->Create();

	// Place the nu creature at exactly the right altitude.
	if(g_Au.Dice().Roll(1,10) <= 3)
		ind->SetGeneralPose(AuPose::SitOnGround);
	else
		ind->Stand();
	AuMatrix4x4 im(m);
	im.Translate(0,0, ind->GroundHeightAboveCG());
	ind->SetPosAndOrient(AuAtomPos(Id(), im));
	ether->AddAtom(ind);

	ind->ResetHitPoints();
	ind->ResetHunger();
	ind->ResetStamina();

	ind->InitIndigenousGear(res);
	ind->InitTraining();
	ind->GoToWork();
	ind->SetInScene();

	g_Au.AtomAdded(ind->Id());
	ind->SetNormal();					// New atom is ready for normal thread processing.
	ind->TickTock();

	if(g_Au.m_LogMessages & (LOG_AtomCreate|LOG_AnimalEncounter))
		qDebug() << "DaWorldLL::CreateIndigenous" << Id() << Name() << "dna" << species.Description()
			<< "indigenous" << ind->Id() << ind->Name() << "role" << role;
	return ind;
}

QString DaWorldLL::IndigenousSayGeologicalFeature(const AuGlobeLLF& speakerLL, bool onlyHoly) const
{
	double salt = TerrainAltitudeMSL(speakerLL);
	AuPoint sloc = LocOfLL(speakerLL, salt);

	m_LLLock.lockForRead();

	if(m_GeologicalFeatures.isEmpty())
	{
		m_LLLock.unlock();
		if(onlyHoly)
			return "";
		else
			return tr("There are many beautiful hills and shores in the world.");
	}

	HYDROGRAPHIC_LIQUID hliq = HydrographicLiquid();
	QString featName;
	AuGlobeLLF featLL;
	if(onlyHoly)
	{
		// Look for a holy feature.
		for(AuGeologicalFeatures::const_iterator ci = m_GeologicalFeatures.cbegin(); ci != m_GeologicalFeatures.cend(); ci++)
			if((*ci)->IsHoly())
			{
				featName = (*ci)->IndigenousName(hliq, GenSeed());
				featLL = (*ci)->LL();
				break;
			}
		if(featName.isEmpty())
		{
			m_LLLock.unlock();
			return "";	// No official holy place.
		}
	}
	else
	{
		// Choose a feature at random.
		int feat = g_Au.Dice().Roll(1,m_GeologicalFeatures.count(),-1);
		featName = m_GeologicalFeatures[feat]->IndigenousName(hliq, GenSeed());
		featLL = m_GeologicalFeatures[feat]->LL();
	}
	
	m_LLLock.unlock();

	double falt = TerrainAltitudeMSL(featLL);
	AuPoint floc = LocOfLL(featLL, falt);

	double dist = sloc.Distance(floc);
	double distInKm = dist / FeetPerMeter() / 1000.;
	QString featDir;
	if(distInKm <= 1.)
		featDir = tr("Near here is %1. It can be found a short distance %2 of here.")
			.arg(featName)
			.arg(speakerLL.DirectionTo(featLL, LLShape()));
	else if(distInKm < 10)
		featDir = tr("About %3 kilometers %2 of here is %1.")
			.arg(featName)
			.arg(speakerLL.DirectionTo(featLL, LLShape()))
			.arg(round(distInKm));
	else if(distInKm < 100.)
		featDir = tr("You should see %1. It is about %L2 kilometers %3 of here.")
			.arg(featName)
			.arg(FROUND(distInKm, 10))	// Rounded to nearest 10km
			.arg(speakerLL.DirectionTo(featLL, LLShape()));
	else
		featDir = tr("Visitors from afar tell of %1. It is a great distance %2 of here.")
			.arg(featName)
			.arg(speakerLL.DirectionTo(featLL, LLShape()));

	return featDir;
}

void DaWorldLL::IndigenousAttackedBy(const AuId& attackerId)
{
	m_VillageLock.lockForWrite();
	if(m_Village)
		m_Village->AttackedBy(attackerId);
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaWorldLL::IndigenousHelpedBy(const AuId& personId, bool isContact)
{
	m_VillageLock.lockForWrite();
	if(m_Village)
		m_Village->HelpedBy(personId, isContact);
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaWorldLL::IndigenousInjuredBy(const AuId& attackerId)
{
	m_VillageLock.lockForWrite();
	if(m_Village)
		m_Village->InjuredBy(attackerId);
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaWorldLL::IndigenousKilledBy(const AuId& attackerId)
{
	m_VillageLock.lockForWrite();
	if(m_Village)
	{
		m_Village->KilledBy(attackerId);
		quint8 newpop = m_Village->Population();
		if(newpop)
			newpop--;
		m_Village->SetPopulation(newpop);
	}
	m_VillageLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaWorldLL::DumpGeo() const
{
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	qDebug() << "DaWorldLL::DumpGeo" << Id() << Name() << "has" << geof.count() << "geological features";
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(henge)
			qDebug() << " " << henge->Name() << (henge->IsActive()?"active":"inactive") << "panels" << henge->CentralSlabCount()
				<< "destSys" << henge->DestSysId() << "destSun" << henge->DestSun() << "destArc" << henge->DestArcSegment();
		else
			qDebug() << " " << (*gi)->Name();
	}
	UnlockGeologicalFeatures();
}

const DaBd* DaWorldLL::FindClosestCity(const AuGlobeLLF& searchLL) const
{
	double salt = TerrainAltitudeMSL(searchLL);
	AuPoint sloc = LocOfLL(searchLL, salt);

	const DaBd* bestcity = 0;
	double bestdist2 = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* city = dynamic_cast<const DaBd*>(*it);
		if(!city)
			continue;
		if(city->IndustryId() != IID_Capitol && city->IndustryId() != IID_BaseHeadquarters)
			continue;

		double dist2 = sloc.Distance2(city->Location());
		if(!bestcity || dist2 < bestdist2)
		{
			bestcity = city;
			bestdist2 = dist2;
		}
	}
	return bestcity;
}

QString DaWorldLL::IndigenousSayClosestCity(const AuGlobeLLF& speakerLL) const
{
	// Find a city.
	const DaBd* city = FindClosestCity(speakerLL);
	if(!city)
		return "";

	double salt = TerrainAltitudeMSL(speakerLL);
	AuPoint sloc = LocOfLL(speakerLL, salt);

	AuPoint cloc = city->Location();
	AuGlobeLLF cll = city->GlobeLL();

	double dist = sloc.Distance(cloc);
	double distInKm = dist / FeetPerMeter() / 1000.;

	// Start out with directions to the city.
	QString say;
	if(distInKm <= 1.)
		say = tr("The closest city is %1. It is less than a km %2 of here.")
			.arg(city->Name())
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 7.5)
		say = tr("The closest city is %1. It is %2 km %3 of here.")
			.arg(city->Name())
			.arg(FROUND(distInKm, 1))	// Rounded to nearest km.
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 100.)
		say = tr("The closest city is %1. It is about %2 km %3 of here.")
			.arg(city->Name())
			.arg(FROUND(distInKm, 10))	// Rounded to nearest 10km
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else
		say = tr("The closest city is %1. It is a great distance %2 of here.")
			.arg(city->Name())
			.arg(speakerLL.DirectionTo(cll, LLShape()));

	// Add a brief description of the city.
	say += city->IndigenousDescription();

	return say;
}

const DaBd* DaWorldLL::FindLargestCity() const
{
	const DaBd* bestcity = 0;
	quint32 bestpop = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* city = dynamic_cast<const DaBd*>(*it);
		if(!city)
			continue;
		if(city->IndustryId() != IID_Capitol && city->IndustryId() != IID_BaseHeadquarters)
			continue;

		if(!bestcity || city->CommunityPopulation() > bestpop)
		{
			bestcity = city;
			bestpop = city->CommunityPopulation();
		}
	}
	return bestcity;
}

QString DaWorldLL::IndigenousSayLargestCity(const AuGlobeLLF& speakerLL) const
{
	// Find a city.
	const DaBd* city = FindLargestCity();
	if(!city)
		return "";

	double salt = TerrainAltitudeMSL(speakerLL);
	AuPoint sloc = LocOfLL(speakerLL, salt);

	AuPoint cloc = city->Location();
	AuGlobeLLF cll = city->GlobeLL();

	double dist = sloc.Distance(cloc);
	double distInKm = dist / FeetPerMeter() / 1000.;

	// Start out with directions to the city.
	QString say;
	if(distInKm <= 1.)
		say = tr("The largest city is %1. It is less than a km %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 7.5)
		say = tr("The largest city is %1. It is %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 1))	// Rounded to nearest km.
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 100.)
		say = tr("The largest city is %1. It is about %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 10))	// Rounded to nearest 10km
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else
		say = tr("The largest city is %1. It is a great distance %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));

	// Add a brief description of the city.
	say += city->IndigenousDescription();

	return say;
}

const DaBd* DaWorldLL::FindSmallestCity() const
{
	const DaBd* bestcity = 0;
	quint32 bestpop = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* city = dynamic_cast<const DaBd*>(*it);
		if(!city)
			continue;
		if(city->IndustryId() != IID_Capitol && city->IndustryId() != IID_BaseHeadquarters)
			continue;

		if(!bestcity || city->CommunityPopulation() < bestpop)
		{
			bestcity = city;
			bestpop = city->CommunityPopulation();
		}
	}
	return bestcity;
}

QString DaWorldLL::IndigenousSaySmallestCity(const AuGlobeLLF& speakerLL) const
{
	// Find a city.
	const DaBd* city = FindSmallestCity();
	if(!city)
		return "";

	double salt = TerrainAltitudeMSL(speakerLL);
	AuPoint sloc = LocOfLL(speakerLL, salt);

	AuPoint cloc = city->Location();
	AuGlobeLLF cll = city->GlobeLL();

	double dist = sloc.Distance(cloc);
	double distInKm = dist / FeetPerMeter() / 1000.;

	// Start out with directions to the city.
	QString say;
	if(distInKm <= 1.)
		say = tr("The smallest city is %1. It is less than a km %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 7.5)
		say = tr("The smallest city is %1. It is %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 1))	// Rounded to nearest km.
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 100.)
		say = tr("The smallest city is %1. It is about %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 10))	// Rounded to nearest 10km
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else
		say = tr("The smallest city is %1. It is a great distance %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));

	// Add a brief description of the city.
	say += city->IndigenousDescription();
	return say;
}

const DaBd* DaWorldLL::FindNewestCity() const
{
	const DaBd* bestcity = 0;
	AuTime besttime;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* city = dynamic_cast<const DaBd*>(*it);
		if(!city)
			continue;
		if(city->IndustryId() != IID_Capitol && city->IndustryId() != IID_BaseHeadquarters)
			continue;

		if(!bestcity || city->CreatedTime() > besttime)
		{
			bestcity = city;
			besttime = city->CreatedTime();
		}
	}
	return bestcity;
}

QString DaWorldLL::IndigenousSayNewestCity(const AuGlobeLLF& speakerLL) const
{
	// Find a city.
	const DaBd* city = FindNewestCity();
	if(!city)
		return "";

	double salt = TerrainAltitudeMSL(speakerLL);
	AuPoint sloc = LocOfLL(speakerLL, salt);

	AuPoint cloc = city->Location();
	AuGlobeLLF cll = city->GlobeLL();

	double dist = sloc.Distance(cloc);
	double distInKm = dist / FeetPerMeter() / 1000.;

	// Start out with directions to the city.
	QString say;
	if(distInKm <= 1.)
		say = tr("The newest city is %1. It is less than a km %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 7.5)
		say = tr("The newest city is %1. It is %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 1))	// Rounded to nearest km.
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 100.)
		say = tr("The newest city is %1. It is about %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 10))	// Rounded to nearest 10km
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else
		say = tr("The newest city is %1. It is a great distance %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));

	// Add a brief description of the city.
	say += city->IndigenousDescription();

	return say;
}

const DaBd* DaWorldLL::FindOldestCity() const
{
	const DaBd* bestcity = 0;
	AuTime besttime;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* city = dynamic_cast<const DaBd*>(*it);
		if(!city)
			continue;
		if(!bestcity || city->CreatedTime() < besttime)
		{
			bestcity = city;
			besttime = city->CreatedTime();
		}
	}
	return bestcity;
}

QString DaWorldLL::IndigenousSayOldestCity(const AuGlobeLLF& speakerLL) const
{
	// Find a city.
	const DaBd* city = FindOldestCity();
	if(!city)
		return "";

	double salt = TerrainAltitudeMSL(speakerLL);
	AuPoint sloc = LocOfLL(speakerLL, salt);

	AuPoint cloc = city->Location();
	AuGlobeLLF cll = city->GlobeLL();

	double dist = sloc.Distance(cloc);
	double distInKm = dist / FeetPerMeter() / 1000.;

	// Start out with directions to the city.
	QString say;
	if(distInKm <= 1.)
		say = tr("The oldest city is %1. It is less than a km %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 7.5)
		say = tr("The oldest city is %1. It is %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 1))	// Rounded to nearest km.
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else if(distInKm < 100.)
		say = tr("The oldest city is %1. It is about %2 km %3 of here.")
			.arg(city->CityName())
			.arg(FROUND(distInKm, 10))	// Rounded to nearest 10km
			.arg(speakerLL.DirectionTo(cll, LLShape()));
	else
		say = tr("The oldest city is %1. It is a great distance %2 of here.")
			.arg(city->CityName())
			.arg(speakerLL.DirectionTo(cll, LLShape()));

	// Add a brief description of the city.
	say += " " + city->IndigenousDescription();

	return say;
}

bool DaWorldLL::InsidePressureTent(const AuPoint& loc) const
{
	// Make a list of the empires who have cities on this planet.
	bool inTent = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaItem* item = dynamic_cast<const DaItem*>(*it);
		if(!item)
			continue;
		if(item->CommodityId() != CID_PressureTent)
			continue;

		double dist2 = item->Location().Distance2(loc);
		if(dist2 > 24*24)
			continue;

		inTent = true;
		break;
	}
	return inTent;
}

DaVehicle* DaWorldLL::CreateVehicle(const AuGlobeLLF& ll, COMMODITYID cid, quint8 quality, quint32 empire)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Don't spawn the vehicle if the location is blocked.
	AuPoint spawnAt = GetSurfaceLocation(ll);
	if(IsSpawnAreaObstructed(spawnAt, 5*FeetPerMeter()))
		return 0;

	// The spawn area is not obstructed. Spawn the vehicle.
	// Create the nu vehicle.
	DaVehicle* vehicle = new DaVehicle(cid, quality);
	vehicle->ConnectSignals();
	vehicle->Create();

	// Position the atom in the environment.
	AuMatrix4x4 t;
	t.Translate(spawnAt);
	t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Align with up.
	t.Translate(0, 0, vehicle->GroundHeightAboveCG());	// Elevate so vehicle sits on ground.

	// Rotate the vehicle randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	vehicle->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(vehicle);

	vehicle->ResetHitPoints();
	vehicle->SetEmpireId(empire);
	vehicle->SetInMotion();
	vehicle->SetUsed();

	g_Au.AtomAdded(vehicle->Id());
	vehicle->SetNormal();	// New atom is ready for normal thread processing.
	vehicle->TickTock();
	return vehicle;
}

DaVessel* DaWorldLL::CreateSpacecraft(const AuGlobeLLF& ll, double altAGL, quint32 designId, quint8 quality, quint32 empireId,
		const QString& name)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	AuPoint spawnAt = GetSurfaceLocation(ll, altAGL);
	// Don't spawn the ship on the ground if the location is blocked.
	if(altAGL < 500*FeetPerMeter() && IsSpawnAreaObstructed(spawnAt, 50*FeetPerMeter()))
		return 0;

	const DgData& sdd = g_Au.LockScDesignsForRead();
	AuScReport r;
	bool loaded = sdd.ScDesignReport(designId, r);
	g_Au.UnlockScDesigns();
	if(!loaded)
	{
		qDebug() << "error: DaWorldLL::CreateSpacecraft blueprint" << designId << "not loaded";
		return 0;
	}
	if(quality < r.Quality())
		quality = r.Quality();

	// Create the nu ship.
	DaSc* ship = new DaSc();
	ship->ConnectSignals();
	ship->Create();

	// Position the ship in the environment.
	AuMatrix4x4 t;
	if(altAGL < 500*FeetPerMeter())
	{
		// Determine altitude above terrain at the spawn location.
		double cght = ship->GroundHeightAboveCG();
		t.Translate(spawnAt);
		t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Align with up.
		t.Translate(0, 0, cght);
	}
	else
	{
		// Put the ship in orbit.
		ElevateLocMSL(spawnAt, AtmosphereHeightMSL() + altAGL);
		t.Translate(spawnAt);
		t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Align with up.
	}

	// Rotate the ship randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	ship->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(ship);

	ship->SetDesign(r);
	ship->InitSystems(quality);

	ship->SetEmpireId(empireId);
	if(name.isEmpty())
		ship->SetName(g_Au.Dice().RandomShipName());
	else
		ship->SetName(name);
	ship->Unmoor();						// Unmoor requires environment to be queried first.
	ship->SetInMotion();				// Let gravity adjust this thing to the terrain.

	g_Au.AtomAdded(ship->Id());
	ship->SetNormal();	// New atom is ready for normal thread processing.
	ship->TickTock();
	return ship;
}

AuGlobeLLF DaWorldLL::HengeLL() const
{
	AuGlobeLLF hengeLL;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;
		hengeLL = henge->LL();
		break;
	}
	UnlockGeologicalFeatures();
	return hengeLL;
}

AuGlobeLLF DaWorldLL::RandomCitizenLLNear(const AuDice& Cdice, const AuGlobeLLF& nearLL, double minDist, double maxDist) const
{
	AuDice dice(Cdice);
	AuPoint loc = LocOfLL(nearLL);
	AuPoint vUp = -GravityDir(loc);

	AuMatrix4x4 t;
	t.Translate(loc);
	t *= AuPoint(0,0,1).GetAlignmentTransform(vUp);

	int tries = 100;
	while(tries--)
	{
		double angle = (double)dice.Roll(1, 3600, -1)/10. * DEG2RADf;
		double dist = dice.Roll(1, maxDist-minDist, minDist);

		AuMatrix4x4 r(t);
		r.RotateZ(angle);
		r.Translate(dist,0,0);

		AuPoint rloc = r.Translation();
		AuGlobeLLF ll;
		if(!LLOfLoc(ll, rloc))
			continue;

		// Avoid the poles.
		if(fabs(ll.Latitude()) > 89*DEG2RADf)
			continue;

		// Avoid bad locations in inhospitable environments.
		AuGlobeLL lld(ll.Longitude(), ll.Latitude());
		double altMSL = TerrainAltitudeMSL(lld);
		if(altMSL < 0 && Hydrographics())
		{
			// Altitude is below sea level.
			if(OrbitZone() == OZONE_Inferno)
				continue;	// Can't be in an under sea city in inferno zone.

			int inbio = InEnclosure(ll, altMSL);
			if(inbio != 2)
				continue;	// Don't put them under the sea unless it's an under sea enclosure.
		}
		else if(!HasBreathableAtmosphere())
		{
			// Not below the sea so ll is exposed to the atmosphere.
			int inbio = InEnclosure(ll, altMSL);
			if(inbio <= 0)
				continue;	// Don't put them in harsh environment.
		}

		// Check for obstructs.
		if(IsSpawnAreaObstructed(rloc, 4))
			continue;

		return ll;
	}
	return AuGlobeLLF();
}

AuGlobeLLF DaWorldLL::RandomLandLLNear(const AuDice& Cdice, const AuGlobeLLF& nearLL, double minAltMSL, double minDist,
		double maxDist) const
{
	AuDice dice(Cdice);
	AuPoint loc = LocOfLL(nearLL);
	AuPoint vUp = -GravityDir(loc);

	AuMatrix4x4 t;
	t.Translate(loc);
	t *= AuPoint(0,0,1).GetAlignmentTransform(vUp);

	int tries = 1000;
	while(tries--)
	{
		double angle = (double)dice.Roll(1, 3600, -1)/10. * DEG2RADf;
		double dist = dice.Roll(1, maxDist-minDist, minDist);

		AuMatrix4x4 r(t);
		r.RotateZ(angle);
		r.Translate(dist,0,0);

		AuPoint rloc = r.Translation();
		AuGlobeLLF ll;
		if(!LLOfLoc(ll, rloc))
			continue;

		// Avoid the poles.
		if(fabs(ll.Latitude()) > 89*DEG2RADf)
			continue;

		double altMSL = TerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(altMSL >= minAltMSL)
			return ll;
	}

	return nearLL;
}

bool DaWorldLL::IsHengeActive() const
{
	if(!HasHenge())
		return false;

	bool hengeActive = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;
		if(!henge->IsActive())
			continue;

		hengeActive = true;
		break;
	}
	UnlockGeologicalFeatures();
	return hengeActive;
}

bool DaWorldLL::IsHengeActive(const AuGlobeLLF& ll) const
{
	if(!HasHenge())
		return false;

	bool hengeActive = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;
		if(henge->LL() != ll)
			continue;
		if(!henge->IsActive())
			continue;

		hengeActive = true;
		break;
	}
	UnlockGeologicalFeatures();
	return hengeActive;
}

bool DaWorldLL::IsHengeNear(const AuPoint& loc, AuGlobeLLF& ll) const
{
	if(!HasHenge())
		return false;

	bool hengeFound = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		float altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(henge->LL().Longitude(), henge->LL().Latitude()));
		AuPoint hloc = LocOfLL(henge->LL(), altMSL);
		if(loc.Distance2(hloc) > 100*FeetPerMeter()*100*FeetPerMeter())
			continue;

		ll = henge->LL();
		hengeFound = true;
		break;
	}
	UnlockGeologicalFeatures();
	return hengeFound;
}

void DaWorldLL::IndigenousHengeCeremony(const AuGlobeLLF& hengeLL, int state)
{
	if(state == 1)
	{
		// Start the indigenous henge ceremony at 'hengeLL'.
		// Get indigenous DNA.
		AuSpecies species;
		m_VillageLock.lockForRead();
		if(m_Village)
			species = m_Village->Species();
		m_VillageLock.unlock();
		if(species.IsNull())
			return;

		AuPoint loc = LocOfLL(hengeLL, GeologicalTerrainAltitudeMSL(AuGlobeLL(hengeLL.Longitude(), hengeLL.Latitude())));

		// All existing indigenous do the dance.
		int existing = 0;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			if(!(*it)->IsNormal())
				continue;

			DaCitizen* ind = dynamic_cast<DaCitizen*>(*it);
			if(ind && ind->IsNpcIndigenous() && !ind->IsBodyDyingOrDead())
			{
				ind->SetOrder(ORDER_HengeCeremony, Id(), loc);
				existing++;
			}
		}

		if(existing >= 9)
			return;

		// Add enough indigenous to have a decent ceremony.
		double minDist = 100*FeetPerMeter();
		double maxDist = 200*FeetPerMeter();

		// Local natural resources determine the quality of their equipment.
		AuZoneResource res;
		QueryNaturalResources(res, ResourceZone(hengeLL.Longitude()));

		AuPoint vUp = -GravityDir(loc);
		AuMatrix4x4 t;
		t.Translate(loc);
		t *= AuPoint(0,0,1).GetAlignmentTransform(vUp);
		for(; existing<9; existing++)
		{
			double angle = (double)g_Au.Dice().Roll(1, 3600, -1)/10. * DEG2RADf;
			double dist = g_Au.Dice().Roll(1, maxDist-minDist, minDist);

			AuMatrix4x4 r(t);
			r.RotateZ(angle);
			r.Translate(dist,0,0);

			DaCitizen* ind;
			if(g_Au.Dice().Roll(1,4) == 1)
				ind = CreateIndigenous(ROLE_IndigenousPerson, Juvenile, species, r, res);
			else
				ind = CreateIndigenous(ROLE_IndigenousPerson, Mature, species, r, res);

			if(ind)
				ind->SetOrder(ORDER_HengeCeremony, Id(), loc);
		}
	}
	else
	{
		AuPoint loc = LocOfLL(hengeLL, GeologicalTerrainAltitudeMSL(AuGlobeLL(hengeLL.Longitude(), hengeLL.Latitude())));

		// Stop the indigenous henge ceremony at 'hengeLL'.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaCitizen* ind = dynamic_cast<DaCitizen*>(*it);
			if(ind && ind->IsNpcIndigenous() && !ind->IsBodyDyingOrDead())
			{
				if(ind->Order() == ORDER_HengeCeremony && ind->OrderLoc().FuzzyEquals(loc))
					ind->SetOrder(ORDER_Stay, Id(), ind->Location());
			}
		}
	}
}

bool DaWorldLL::HengeRadii(const AuGlobeLLF& hengeLL, double& centerRadius, double& columnRadius) const
{
	if(!HasHenge())
		return false;

	bool hengeFound = false;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;
		if(henge->LL() != hengeLL)
			continue;

		centerRadius = henge->CentralSlabRadius();
		columnRadius = henge->OuterRingRadius();
		hengeFound = true;
		break;
	}
	UnlockGeologicalFeatures();
	return hengeFound;
}

void DaWorldLL::RenderTerrainInfoSprite(AuGLEffects& re, const AuPoint& txpt, GLuint texture1, int texSize1,
		GLuint texture2, int texSize2, GLuint texQ, double unitsPerPixel, bool sprite2Flip)
{
	// Draw the icon on the screen, not at Z-depth.
	if(texture1)
	{
		// Draw primary sprite.
		if(!m_SpriteVAO[texSize1].VAO())
		{
			// Make a VAO of the size of sprite.
			QList<AuPointAlpha> verts;
			verts << AuPointAlpha(0, 0, 0, texSize1);
			m_SpriteVAO[texSize1].CreateForSprites(re, verts, GL_STATIC_DRAW);
		}

		re.GLSL().SpriteActualSizeShader().SetTexture(texture1);

		// Make a matrix to position the sprite.
		AuMatrix4x4 spriteM;
		spriteM.Translate(txpt.x, txpt.y, -unitsPerPixel*2);
		re.GLSL().SpriteActualSizeShader().SetProjectionModelView(spriteM);

		// Display the VAO once and clean it up.
		QHash<int, AuGLVAO>::iterator vi = m_SpriteVAO.find(texSize1);
		Q_ASSERT((*vi).VAO());
		(*vi).SelectGL();
		(*vi).DrawGL(GL_POINTS);
		(*vi).DeselectGL();

		// Draw construction sprite.
		if(texQ)
		{
			AuPoint tlpt = txpt + AuPoint(texSize1/2, -texSize1/2)*unitsPerPixel;

			re.glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
			re.GLSL().SpriteActualSizeShader().SetTexture(texQ);

			// Make a matrix to position the sprite.
			AuMatrix4x4 spriteM;
			spriteM.Translate(tlpt.x, tlpt.y, -unitsPerPixel*2);
			re.GLSL().SpriteActualSizeShader().SetProjectionModelView(spriteM);

			// Compile a list of vertex locations.
			QList<AuPointAlpha> verts;
			verts << AuPointAlpha(0, 0, 0, texSize1/2);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();

			re.glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
		}
	}

	if(texture2)
	{
		// Draw secondary sprite.
		if(!m_SpriteVAO[texSize2].VAO())
		{
			// Make a VAO of the size of sprite.
			QList<AuPointAlpha> verts;
			verts << AuPointAlpha(0, 0, 0, texSize2);
			m_SpriteVAO[texSize2].CreateForSprites(re, verts, GL_STATIC_DRAW);
		}

		AuPoint txpt2(txpt);
		if(texture1)
			txpt2.x += unitsPerPixel*(texSize1/2 + texSize2/2);

		if(sprite2Flip)
			re.glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

		re.GLSL().SpriteActualSizeShader().SetTexture(texture2);

		// Make a matrix to position the sprite.
		AuMatrix4x4 spriteM;
		spriteM.Translate(txpt2.x, txpt2.y, -unitsPerPixel*2);
		re.GLSL().SpriteActualSizeShader().SetProjectionModelView(spriteM);

		// Display the VAO once and clean it up.
		QHash<int, AuGLVAO>::iterator vi = m_SpriteVAO.find(texSize2);
		Q_ASSERT((*vi).VAO());
		(*vi).SelectGL();
		(*vi).DrawGL(GL_POINTS);
		(*vi).DeselectGL();

		if(sprite2Flip)
			re.glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
	}
}

void DaWorldLL::RenderTerrainInfo(const AuMatrix4x4& model, AuAtomRenderInfo& ari, const DaPerson* scenePerson)
{
	if(ari.TopViewInfo() == AuAtomRenderInfo::VI_None)
		return;
	if(InDesignerPreview())
		return;

	AuPoint worldLoc;
	if(!scenePerson->GetLocationRelativeToParent(worldLoc, this))
		return;	// wut?

	if(!s_FlagEmpireTex)
	{
		s_FlagEmpireTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPEmpire", ":data/FlagEmpire.png");
		s_FlagEnemyTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPEnemy", ":data/FlagEnemy.png");
		s_FlagFriendTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPFriend", ":data/FlagFriend.png");
		s_FlagNeutralTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPNeutral", ":data/FlagNeutral.png");
	}

	// We must calculate the pixel location to draw sprites on the screen, rather than at 3D Z-depth.
	// Calculate view aspect ratio.
	double windowAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
	double sceneAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
	double fovy = AuGLEffects::WindowFieldOfViewY(sceneAspect, g_Au.FieldOfViewY());
	double unitsPerPixel = 0;
	if(ari.glw->width() > 0)
		unitsPerPixel = 2./ari.glw->width();

	// Create perspective camera transformations the way AuView does it.
	AuMatrix4x4 tP;
	tP.Perspective(fovy, sceneAspect, 3.0, 1000000.0);

	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(ari.TopViewInfo() == AuAtomRenderInfo::VI_CityNames)
	{
		// City Names
		UpdateBuildingOverlaySprites(worldLoc, scenePerson->EmpireId());

		for(QMap<AuPoint, AuBldSprite>::const_iterator bi = m_OverlayBuildings.cbegin(); bi != m_OverlayBuildings.cend(); bi++)
		{
			if((*bi).m_CityName.isEmpty())
				continue;
			// Calculate eye-space location of the object.
			AuPoint txpt = model.Project(bi.key());
			if(txpt.z > 0.)
				continue;				// Skip those behind the viewer.
			if(txpt.Length2() > 2000*FeetPerMeter() * 2000*FeetPerMeter())
				continue;				// We don't need to see buildings on the other side of the planet.
			txpt = tP.Project(txpt);	// Calculate screen space location of object.
			txpt.x *= sceneAspect / windowAspect;

			int x = (int)((txpt.x + 1.)/2. * ari.glw->width());
			int y = ari.glw->height() - (int)((txpt.y + 1.)/2. * ari.glw->height());
			ari.RenderText(x, y, (*bi).m_CityName, (*bi).m_StanceColor, Qt::AlignCenter);
		}
	}
	else
	{
		QMap<AuPoint, int> buildingLevels;			// Locations and levels completed for buildings with more than one level.
		QMap<AuPoint, AuBldHealth> buildingHealthBars;	// Locations and health (0 to 1) for buildings with hit points.
		QMap<AuPoint, QString> buildingLevelsUC;	// Locations and "levels of levelsTotal" string for buildings under construction

		ari.re->GLSL().SpriteActualSizeShader().Select();
		ari.re->GLSL().SpriteActualSizeShader().SetProjectionModelView(AuMatrix4x4());
		ari.re->GLSL().SpriteActualSizeShader().SetColor(QColor(255,255,255,255));

		ari.re->glDisable(GL_DEPTH_TEST);
		ari.re->glEnable(GL_PROGRAM_POINT_SIZE); 

		const int pct = g_Au.ResourceIconSize();
		switch(ari.TopViewInfo())
		{
		case AuAtomRenderInfo::VI_None: //   0 = no terrain info
			break;
		case AuAtomRenderInfo::VI_RocksAll: //   1 = rocks
		case AuAtomRenderInfo::VI_RocksSome: //   2 = rocks without stone or eludium
			{
				const int spacing2 = 150*150;						// Squared distance for declutter.
				if(m_OverlayRocks.empty() || worldLoc.Distance(m_OverlayRocksLoc) > 50*FeetPerMeter())
				{
					m_OverlayRocksLoc = worldLoc;
					m_OverlayRocks.clear();
					for(QMap<AuResKey, AuGlobeRock>::const_iterator ri = m_Rocks.cbegin(); ri != m_Rocks.cend(); ri++)
					{
						// Do not show too many of the same resource in the same area.
						bool skipIt = false;
						for(QMultiHash<COMMODITYID, AuPoint>::const_iterator asi =
								m_OverlayRocks.constFind((COMMODITYID)(*ri).m_CID);
								asi != m_OverlayRocks.cend() && asi.key() == (*ri).m_CID; asi++)
						{
							if((*ri).Location().Distance2(*asi) < spacing2)
							{
								// We have already shown this type of icon nearby.
								skipIt = true;
								break;
							}
						}
						if(skipIt)
							continue;
						m_OverlayRocks.insert((COMMODITYID)(*ri).m_CID, (*ri).Location());
					}
				}

				for(QMultiHash<COMMODITYID, AuPoint>::const_iterator asi = m_OverlayRocks.cbegin();
						asi != m_OverlayRocks.cend(); asi++)
				{
					if(ari.TopViewInfo() == AuAtomRenderInfo::VI_RocksSome &&
							(asi.key() == CID_Stone || asi.key() == CID_Eludium || asi.key() == CID_Ice))
						continue;

					// Calculate eye-space location of the object.
					AuPoint txpt = model.Project(*asi);
					if(txpt.z > 0.)
						continue;				// Skip those behind the viewer.
					txpt = tP.Project(txpt);	// Calculate screen space location of object.

					GLuint texture1 = ari.CommodityTextureF(asi.key());

					GLuint texture2 = 0;
					if(asi.key() == CID_CrudeOil)
						texture2 = ari.CommodityTextureF(CID_NaturalGas);

					int texSize = 30 * pct / 100;

					RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, texture2, texSize, 0, unitsPerPixel);
				}
			}
			break;
		case AuAtomRenderInfo::VI_Plants: //   3 = plants
			{
				const int spacing2 = 150*150;						// Squared distance for declutter.
				// Shrubs
				{
					if(m_OverlayShrubs.empty() || worldLoc.Distance(m_OverlayShrubsLoc) > 50*FeetPerMeter())
					{
						m_OverlayShrubsLoc = worldLoc;
						m_OverlayShrubs.clear();
						for(QMap<AuResKey, AuGlobeFlora>::const_iterator si = m_Shrubs.cbegin(); si != m_Shrubs.cend(); si++)
						{
							// Do not show too many of the same resource in the same area.
							bool skipIt = false;
							for(QMultiHash<COMMODITYID, AuPoint>::const_iterator asi =
									m_OverlayShrubs.constFind((COMMODITYID)(*si).m_CID);
									asi != m_OverlayShrubs.cend() && asi.key() == (*si).m_CID; asi++)
							{
								if((*si).Location().Distance2(*asi) < spacing2)
								{
									// We have already shown this type of icon nearby.
									skipIt = true;
									break;
								}
							}
							if(skipIt)
								continue;
							m_OverlayShrubs.insert((COMMODITYID)(*si).m_CID, (*si).Location());
						}

						// Aquatic shrubs do not produce anything.
					}

					for(QMultiHash<COMMODITYID, AuPoint>::const_iterator asi = m_OverlayShrubs.cbegin();
							asi != m_OverlayShrubs.cend(); asi++)
					{
						// Calculate eye-space location of the object.
						AuPoint txpt = model.Project(*asi);
						if(txpt.z > 0.)
							continue;				// Skip those behind the viewer.
						txpt = tP.Project(txpt);	// Calculate screen space location of object.

						GLuint texture1 = ari.CommodityTextureF(asi.key());

						int texSize = 20 * pct / 100;

						RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, 0, 0, 0, unitsPerPixel);
					}
				}
				// Trees
				{
					if(m_OverlayTrees.empty() || worldLoc.Distance(m_OverlayTreesLoc) > 50*FeetPerMeter())
					{
						m_OverlayTreesLoc = worldLoc;
						m_OverlayTrees.clear();
						for(QMap<AuResKey, AuGlobeTree>::const_iterator ti = m_Trees.cbegin(); ti != m_Trees.cend(); ti++)
						{
							// Do not show too many of the same resource in the same area.
							bool skipIt = false;
							for(QMultiHash<COMMODITYID, AuPoint>::const_iterator asi =
									m_OverlayTrees.constFind((COMMODITYID)(*ti).m_CID);
									asi != m_OverlayTrees.cend() && asi.key() == (*ti).m_CID; asi++)
							{
								if((*ti).Location().Distance2(*asi) < spacing2)
								{
									// We have already shown this type of icon nearby.
									skipIt = true;
									break;
								}
							}
							if(skipIt)
								continue;
							m_OverlayTrees.insert((COMMODITYID)(*ti).m_CID, (*ti).Location());
						}
					}

					for(QMultiHash<COMMODITYID, AuPoint>::const_iterator asi = m_OverlayTrees.cbegin();
							asi != m_OverlayTrees.cend(); asi++)
					{
						// Calculate eye-space location of the object.
						AuPoint txpt = model.Project(*asi);
						if(txpt.z > 0.)
							continue;				// Skip those behind the viewer.
						txpt = tP.Project(txpt);	// Calculate screen space location of object.

						GLuint texture1 = ari.CommodityTextureF(asi.key());

						int texSize = 40 * pct / 100;

						RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, 0, 0, 0, unitsPerPixel);
					}
				}
			}
			break;
		case AuAtomRenderInfo::VI_Buildings: //   4 = developments except roads and street lights
			UpdateBuildingOverlaySprites(worldLoc, scenePerson->EmpireId());
			for(QMap<AuPoint, AuBldSprite>::const_iterator bi = m_OverlayBuildings.cbegin(); bi != m_OverlayBuildings.cend();
					bi++)
			{
				if((*bi).m_IID == IID_StreetLight || (*bi).m_IID == IID_Bridge || (*bi).m_IID == IID_Fence)
					continue;
				AuPoint txpt = model.Project(bi.key());
				if(txpt.z > 0.)
					continue;				// Skip those behind the viewer.
				if(txpt.Length2() > 2000*FeetPerMeter() * 2000*FeetPerMeter())
					continue;				// We don't need to see buildings on the other side of the planet.
				txpt = tP.Project(txpt);	// Calculate screen space location of object.

				QString sname = QString("IID%1").arg((*bi).m_IID);
				GLuint texture1 = ari.re->UseTexture(STATIC_TEXTURE_ID, sname);

				GLuint texQ;
				if((*bi).IsBdUnderConstruction())
					texQ = s_TexConstruction;
				else
					texQ = 0;

				int texSize = 40 * pct / 100;

				RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, 0, 0, texQ, unitsPerPixel);

				// Track building levels so we can show them after the sprites.
				if(!(*bi).m_IsBd)
				{
					// Old style buildings can be multiple stories and some of them can be under construction.
					if((*bi).m_LevelsUnderConstruction)
					{
						QString levels = QString("%1/%2")
							.arg((*bi).m_CompletedLevels)
							.arg((*bi).m_CompletedLevels + (*bi).m_LevelsUnderConstruction);
						buildingLevelsUC.insert(txpt, levels);
					}
					else if((*bi).m_CompletedLevels > 1)
						buildingLevels.insert(txpt, (*bi).m_CompletedLevels);
				}

				// Health bar.
				if((*bi).m_MaxHits && (*bi).m_CurHits < (*bi).m_MaxHits)
					buildingHealthBars.insert(txpt, AuBldHealth((*bi).m_CurHits, (*bi).m_MaxHits, (*bi).m_IsBurning));
			}
			break;
		case AuAtomRenderInfo::VI_Roads: //   5 = roads, landscaping and street lights
			UpdateRoadOverlaySprites(worldLoc);
			for(QMap<AuPoint, AuRoadSprite>::const_iterator ri = m_OverlayRoads.cbegin(); ri != m_OverlayRoads.cend(); ri++)
			{
				// Calculate eye-space location of the object.
				AuPoint txpt = model.Project(ri.key());
				if(txpt.z > 0.)
					continue;				// Skip those behind the viewer.
				if(txpt.Length2() > 2000*FeetPerMeter() * 2000*FeetPerMeter())
					continue;				// We don't need to see buildings on the other side of the planet.
				txpt = tP.Project(txpt);	// Calculate screen space location of object.

				GLuint texture1;
				if((*ri).m_IsLandscaper)
				{
					// Offset by 23 into industry textures table. See DbIndustry::BitmapName for an explanation.
					QString sname = QString("IID%1").arg((*ri).m_Material-ROADMAT_GradedArea+23);
					texture1 = ari.re->UseTexture(STATIC_TEXTURE_ID, sname);
				}
				else if((*ri).m_IsSlab)
				{
					// Offset by 14 into industry textures table. See DbIndustry::BitmapName for an explanation.
					QString sname = QString("IID%1").arg((*ri).m_Material+14);
					texture1 = ari.re->UseTexture(STATIC_TEXTURE_ID, sname);
				}
				else
				{
					// Offset by 6 into industry textures table. See DbIndustry::BitmapName for an explanation.
					QString sname = QString("IID%1").arg((*ri).m_Material+6);
					texture1 = ari.re->UseTexture(STATIC_TEXTURE_ID, sname);
				}

				GLuint texQ;
				if((*ri).m_IsUnderConstruction)
					texQ = s_TexConstruction;
				else
					texQ = 0;

				int texSize = 40 * pct / 100;

				RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, 0, 0, texQ, unitsPerPixel);
			}
			UpdateBuildingOverlaySprites(worldLoc, scenePerson->EmpireId());
			for(QMap<AuPoint, AuBldSprite>::const_iterator bi = m_OverlayBuildings.cbegin(); bi != m_OverlayBuildings.cend();
					bi++)
			{
				if((*bi).m_IID != IID_StreetLight && (*bi).m_IID != IID_Bridge && (*bi).m_IID != IID_Fence)
					continue;
				AuPoint txpt = model.Project(bi.key());
				if(txpt.z > 0.)
					continue;				// Skip those behind the viewer.
				if(txpt.Length2() > 2000*FeetPerMeter() * 2000*FeetPerMeter())
					continue;				// We don't need to see buildings on the other side of the planet.
				txpt = tP.Project(txpt);	// Calculate screen space location of object.

				QString sname = QString("IID%1").arg((*bi).m_IID);
				GLuint texture1 = ari.re->UseTexture(STATIC_TEXTURE_ID, sname);

				GLuint texQ;
				if((*bi).IsBdUnderConstruction())
					texQ = s_TexConstruction;
				else
					texQ = 0;

				int texSize = 40 * pct / 100;
				if((*bi).m_IID == IID_StreetLight || (*bi).m_IID == IID_Fence)
					texSize /= 2;	// smaller than roads and bridges

				RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, 0, 0, texQ, unitsPerPixel);

				// Health bar.
				if((*bi).m_MaxHits && (*bi).m_CurHits < (*bi).m_MaxHits)
					buildingHealthBars.insert(txpt, AuBldHealth((*bi).m_CurHits, (*bi).m_MaxHits, (*bi).m_IsBurning));
			}
			break;
		case AuAtomRenderInfo::VI_Flags: //   6 = flags
			if(ClaimedByEmpire())
			{
				// Calculate eye-space location of the object.
				AuPoint buildLoc = GetTerrainLocation(ClaimedAtLL());
				AuPoint txpt = model.Project(buildLoc);
				if(txpt.z <= 0. && txpt.Length2() < 2000*FeetPerMeter()*2000*FeetPerMeter())
				{
					txpt = tP.Project(txpt);	// Calculate screen space location of object.
					quint32 claimEmpire = ClaimedByEmpire();

					GLuint texture1;
					POLITICAL_STANCE stance = PoliticalStance(scenePerson->EmpireId(), claimEmpire);
					switch(stance)
					{
					case POL_Empire:
					case POL_Vassal:
						texture1 = s_FlagEmpireTex;
						break;
					case POL_Enemy:
						texture1 = s_FlagEnemyTex;
						break;
					case POL_Friend:
						texture1 = s_FlagFriendTex;
						break;
					case POL_Neutral:
						texture1 = s_FlagNeutralTex;
						break;
					default:
						texture1 = 0;
						break;
					}

					// Locate the texture of the actual flag.
					// The real flag texture doesn't work because the sprite inverts the texture. :(
					QString stid = QString("Flag:%1").arg(claimEmpire);
					int texture2 = ari.re->UseTexture(STATIC_TEXTURE_ID, stid);

					int texSize = 64 * pct / 100;

					RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, texture2, texSize*3/2, 0, unitsPerPixel, true);
				}
			}
			if(HasSubatoms())
			{
				DaAtomList subs;
				ListNormalSubatoms(subs);
				for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				{
					const DaBd* bd = dynamic_cast<const DaBd*>(*it);
					if(bd)
					{
						if(bd->IndustryId() == IID_Capitol || bd->IndustryId() == IID_BaseHeadquarters)
						{
							// Calculate eye-space location of the object.
							AuPoint buildLoc = bd->Transform().Project(bd->SiteCenter());
							AuPoint txpt = model.Project(buildLoc);
							if(txpt.z <= 0. && txpt.Length2() < 2000*FeetPerMeter() * 2000*FeetPerMeter())
							{
								txpt = tP.Project(txpt);	// Calculate screen space location of object.

								quint32 bdempire = bd->EmpireId();
								GLuint texture1;
								POLITICAL_STANCE stance = PoliticalStance(scenePerson->EmpireId(), bdempire);
								switch(stance)
								{
								case POL_Empire:
								case POL_Vassal:
									texture1 = s_FlagEmpireTex;
									break;
								case POL_Enemy:
									texture1 = s_FlagEnemyTex;
									break;
								case POL_Friend:
									texture1 = s_FlagFriendTex;
									break;
								case POL_Neutral:
									texture1 = s_FlagNeutralTex;
									break;
								default:
									texture1 = 0;
									break;
								}

								// Locate the texture of the actual flag.
								// The real flag texture doesn't work because the sprite inverts the texture. :(
								QString stid = QString("Flag:%1").arg(bdempire);
								int texture2 = ari.re->UseTexture(STATIC_TEXTURE_ID, stid);

								int texSize = 64 * pct / 100;

								RenderTerrainInfoSprite(*ari.re, txpt, texture1, texSize, texture2, texSize*3/2, 0, unitsPerPixel,
										true);
							}
						}
						continue;
					}
				}
			}
			break;
		case AuAtomRenderInfo::VI_CityNames: //   7 = city names
			// Handled above.
			break;
		}

		ari.re->GLSL().SpriteActualSizeShader().Deselect();

		ari.re->glDisable(GL_PROGRAM_POINT_SIZE); 
		ari.re->glEnable(GL_DEPTH_TEST);

		if(!buildingLevels.isEmpty() || !buildingLevelsUC.isEmpty())
		{
			// Calculate icon radius for placement at edge.
			int texSize = 40 * pct / 100;
			texSize /= 2;

			// Draw drop shadows.
			for(QMap<AuPoint, int>::const_iterator li = buildingLevels.cbegin(); li != buildingLevels.cend(); li++)
			{
				int x = (int)((li.key().x + 1.)/2. * ari.glw->width());
				int y = ari.glw->height() - (int)((li.key().y + 1.)/2. * ari.glw->height());
				x += texSize+3;
				y += texSize+1 + 4;
				QString text = QVariant(*li).toString();
				ari.RenderText(x, y, text, QColor(0,0,0), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine);
			}
			for(QMap<AuPoint, QString>::const_iterator li = buildingLevelsUC.cbegin(); li != buildingLevelsUC.cend(); li++)
			{
				int x = (int)((li.key().x + 1.)/2. * ari.glw->width());
				int y = ari.glw->height() - (int)((li.key().y + 1.)/2. * ari.glw->height());
				x += texSize+3;
				y += texSize+1 + 4;
				ari.RenderText(x, y, *li, QColor(0,0,0), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine);
			}

			// Draw text after drop shadows.
			QColor tcolor(214, 0, 255);
			for(QMap<AuPoint, int>::const_iterator li = buildingLevels.cbegin(); li != buildingLevels.cend(); li++)
			{
				int x = (int)((li.key().x + 1.)/2. * ari.glw->width());
				int y = ari.glw->height() - (int)((li.key().y + 1.)/2. * ari.glw->height());
				x += texSize+2;
				y += texSize + 4;
				QString text = QVariant(*li).toString();
				ari.RenderText(x, y, text, tcolor, Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine);
			}
			tcolor.setRgb(214, 148, 0);
			for(QMap<AuPoint, QString>::const_iterator li = buildingLevelsUC.cbegin(); li != buildingLevelsUC.cend(); li++)
			{
				int x = (int)((li.key().x + 1.)/2. * ari.glw->width());
				int y = ari.glw->height() - (int)((li.key().y + 1.)/2. * ari.glw->height());
				x += texSize+2;
				y += texSize + 4;
				ari.RenderText(x, y, *li, tcolor, Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine);
			}
		}

		if(!buildingHealthBars.isEmpty())
		{
			// Calculate icon radius for placement at edge.
			int texSize = 40 * pct / 100;
			texSize /= 2;
			for(QMap<AuPoint, AuBldHealth>::const_iterator li = buildingHealthBars.cbegin(); li != buildingHealthBars.cend(); li++)
			{
				int x = (int)((li.key().x + 1.)/2. * ari.glw->width());
				int y = ari.glw->height() - (int)((li.key().y + 1.)/2. * ari.glw->height());
				x += texSize + 2;
				y += texSize + 4;

				float hitsFrac = (float)(*li).m_CurHits / (float)(*li).m_MaxHits;

				float rl;
				float gl;
				float bl;
				if((*li).m_IsBurning)
				{
					rl = 1.f;
					gl = .2f;
					bl = .2f;
				}
				else if(hitsFrac < .25f)
				{
					rl = 1.f;
					gl = .7f;
					bl = .2f;
				}
				else
				{
					rl = .2f;
					gl = 1.f;
					bl = .2f;
				}

				QString hp;
				if((*li).m_CurHits != (*li).m_MaxHits)
					hp = QString("%L1/%L2hp").arg((*li).m_CurHits).arg((*li).m_MaxHits);
				else
					hp = QString("%L1hp").arg((*li).m_CurHits);

				QColor col;
				col.setRedF(rl);
				col.setGreenF(gl);
				col.setBlueF(bl);
				ari.RenderText(x, y, hp, col, Qt::AlignLeft);
			}
		}
	}

	ari.re->glDisable(GL_BLEND);
}

void DaWorldLL::UpdateBuildingOverlaySprites(const AuPoint& worldLoc, quint32 sceneEmpireId)
{
	if(!m_OverlayBuildings.empty() && worldLoc.Distance(m_OverlayBuildingsLoc) < 50*FeetPerMeter())
		return;

	m_OverlayBuildingsLoc = worldLoc;
	m_OverlayBuildings.clear();

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(bd)
		{
			AuPoint buildLoc = bd->Transform().Project(bd->SiteCenter());
			QString cityName(bd->CityName());
			QColor color = PoliticalStanceColor(sceneEmpireId, bd->EmpireId(), bd->IsCivilian());
			quint32 hitsTot = bd->DesignHitsStructural();
			quint32 hitsCur = hitsTot - bd->HitsDamageStructural();
			m_OverlayBuildings.insert(buildLoc, AuBldSprite(cityName, color, bd->IndustryId(), bd->IsUnderConstruction(),
						hitsCur, hitsTot, bd->IsBurning()));
			continue;
		}
	}
}

void DaWorldLL::UpdateRoadOverlaySprites(const AuPoint& worldLoc)
{
	if(!m_OverlayRoads.empty() && worldLoc.Distance(m_OverlayRoadsLoc) < 50*FeetPerMeter())
		return;

	m_OverlayRoadsLoc = worldLoc;
	m_OverlayRoads.clear();

	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		AuPoint roadLoc1 = LocOfLL((*gi)->LL(), (*gi)->LLMSL());
		AuPoint roadLoc2 = LocOfLL((*gi)->LL2(), (*gi)->LL2MSL());
		AuPoint roadCen = (roadLoc1 + roadLoc2)/2;
		const AuGeoRoadGrade* rg = dynamic_cast<const AuGeoRoadGrade*>(*gi);
		if(rg)
		{
			bool isLandscaper;
			ROADMAT material;
			if(rg->Type() == AuGeoRoadDirt::GeoType())
			{
				material = ROADMAT_Dirt;
				isLandscaper = false;
			}
			else if(rg->Type() == AuGeoRoadAsphalt::GeoType())
			{
				material = ROADMAT_Asphalt;
				isLandscaper = false;
			}
			else if(rg->Type() == AuGeoRoadConcrete::GeoType())
			{
				material = ROADMAT_Concrete;
				isLandscaper = false;
			}
			else
			{
				material = ROADMAT_GradedArea;
				isLandscaper = true;
			}
			m_OverlayRoads.insert(roadCen, AuRoadSprite(isLandscaper, rg->IsSlab(), material, rg->IsUnderConstruction()));
			continue;
		}
		const AuGeoLandscape* ls = dynamic_cast<const AuGeoLandscape*>(*gi);
		if(ls)
		{
			ROADMAT material;
			if(ls->IsDefoliant())
				material = ROADMAT_Defoliant;
			else if(ls->IsIrrigation())
				material = ROADMAT_Irrigation;
			else
				material = ROADMAT_Clearing;
			m_OverlayRoads.insert(roadCen, AuRoadSprite(true, false, material, ls->IsUnderConstruction()));
			continue;
		}
	}
	UnlockGeologicalFeatures();
}

void DaWorldLL::InvalidateTerrainInfoSprites() const
{
	m_OverlayBuildings.clear();
	m_OverlayRoads.clear();
	m_OverlayRocks.clear();
	m_OverlayShrubs.clear();
	m_OverlayTrees.clear();
}

double DaWorldLL::TerrainAltitudeMSL(const AuGlobeLL& ll) const
{
	// Determine the altitude of the virgin terrain at ll.
	WORLDTEX_TYPE pavementType = WORLDTEX_Count;
	AuTime newest;
	double terAltMSL = VirginTerrainAltitudeMSL(ll, pavementType, newest);
	if(!BdHere() || pavementType != WORLDTEX_Count)
	{
		// Landscapers do not change the terrain texture but they do want to decide about the altitude of the terrain.
		return terAltMSL;
	}
	AuEther* ether = Ether();
	if(!ether)
		return terAltMSL;

	DaAtomList subadj;
	if(BdHere())
	{
		// Allow bd buildings to adjust the final height based on their developments.
		// Get the list of bd ids at ll.
		AuIdList bds = BuildingsAt(ll.ToLLF());
		if(!bds.empty())
		{
			// Use the ether's hash table.
			for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
			{
				DaBd* bd = (DaBd*)ether->FindNormalAtom(*bi);
				if(!bd || bd->CreatedTime() < newest)
					continue;
				// Bridges are last so they will remove trees and shrubs created by other buildings at the same location.
				INDUSTRYID iid = bd->IndustryId();
				if(iid == IID_Bridge)
					subadj.append(bd);
				else
					subadj.prepend(bd);
			}
		}
	}
	if(subadj.empty())
		return terAltMSL;

	for(DaAtomList::const_iterator ci = subadj.begin(); ci != subadj.end(); ci++)
		terAltMSL = (*ci)->AdjustAltMSLForDevelopments(*this, terAltMSL, ll);
	return terAltMSL;
}

double DaWorldLL::TerrainAltitudeMSLPave(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, double& farmShrubMSL, COMMODITYID& orchardTree,
		double& orchardTreeMSL) const
{
	// Determine the altitude of the virgin terrain at ll.
	farmShrub = CID_Invalid;
	farmShrubMSL = 0;			// 0 indicates no influence because it isn't really a valid altitude for a plaform build zone.
	orchardTree = CID_Invalid;
	orchardTreeMSL = 0;			// 0 indicates no influence because it isn't really a valid altitude for a plaform build zone.
	AuTime newest;
	double terAltMSL = VirginTerrainAltitudeMSL(ll, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, newest);
	if(!BdHere() || pavementType != WORLDTEX_Count)
	{
		// Landscapers do not change the terrain texture but they do want to decide about the altitude of the terrain.
		return terAltMSL;
	}
	AuEther* ether = Ether();
	if(!ether)
		return terAltMSL;

	DaAtomList subadj;
	if(BdHere())
	{
		// Allow bd buildings to adjust the final height based on their developments.
		// Get the list of bd ids at ll.
		AuIdList bds = BuildingsAt(ll.ToLLF());
		if(!bds.empty())
		{
			for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(ether->FindNormalAtom(*bi));
				if(!bd || bd->CreatedTime() < newest)
					continue;
				// Bridges are last so they will remove trees and shrubs created by other buildings at the same location.
				INDUSTRYID iid = bd->IndustryId();
				if(iid == IID_Bridge)
					subadj.append(bd);
				else
					subadj.prepend(bd);
			}
		}
	}
	if(subadj.empty())
		return terAltMSL;

	bool hideRocksBd = false;
	int hideGrassBd = false;
	bool hideShrubsBd = false;
	bool hideTreesBd = false;
	WORLDTEX_TYPE pavementTypeBd = WORLDTEX_Count;
	for(DaAtomList::const_iterator ci = subadj.begin(); ci != subadj.end(); ci++)
		terAltMSL = (*ci)->AdjustAltMSLForDevelopments(*this, terAltMSL, ll, hideRocksBd, hideGrassBd, hideShrubsBd,
				hideTreesBd, pavementTypeBd, farmShrub, farmShrubMSL, orchardTree, orchardTreeMSL);
	// Don't make things visible that are supposed to be hidden.
	if(!hideRocks)
		hideRocks = hideRocksBd;
	if(hideGrass != 1)
		hideGrass = hideGrassBd;
	if(!hideShrubs)
		hideShrubs = hideShrubsBd;
	if(!hideTrees)
		hideTrees = hideTreesBd;
	if(pavementType == WORLDTEX_Count)
		pavementType = pavementTypeBd;
	else
	{
		farmShrub = CID_Invalid;	// So we don't put farm shrubs on roads.
		orchardTree = CID_Invalid;	// So we don't put orchard trees on roads.
	}
	return terAltMSL;
}

void DaWorldLL::BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm)
{
	Q_UNUSED(bldM);
	Q_UNUSED(radius);
	Q_UNUSED(mm);
	ClearMaps();
}

void DaWorldLL::ClearMaps()
{
	// Clear building map.
	m_BuildingMapLock.lock();
	m_BuildingMap.clear();
	m_BuildingMapLock.unlock();
	m_BdBits |= 0xa8;			// Turn on base unknown, bd unknown and hq unknown bits.
	//qDebug() << "DaWorldLL::ClearMaps" << *this;
}

void DaWorldLL::FindBdsContaining(const AuPoint& loc, FINDDEV filter, DaAtomList& bds) const
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	if(!BdHere())
		return;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return;
	AuIdList bdsNear = BuildingsAt(ll);
	if(!bdsNear.empty())
	{
		// Use the ether's hash table.
		//qDebug() << "DaWorldLL::FindBdsContaining" << *this << "bdsNear" << bdsNear.size();
		bool hasDome = false;
		bool hasPlat = false;
		for(AuIdList::const_iterator bi = bdsNear.cbegin(); bi != bdsNear.cend(); bi++)
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
			if(!bd)
				continue;
			if(bd->TestFindFilter(filter))
				continue;
			if(bd->IndustryId() == IID_Biodome)
			{
				hasDome = true;
				continue;				// Skip biodomes in the first pass, we add them to the list later.
			}
			else if(DbIndustry::IsPlatform(bd->IndustryId()))
			{
				hasPlat = true;
				continue;				// Skip platforms in the first pass, we add them to the list later.
			}
			if(bd->SiteContains(loc))
				bds.append((DaBd*)bd);	// Cast away constness.
		}

		if(hasDome)
		{
			for(AuIdList::const_iterator bi = bdsNear.cbegin(); bi != bdsNear.cend(); bi++)
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
				if(!bd)
					continue;
				if(bd->TestFindFilter(filter))
					continue;
				if(bd->IndustryId() != IID_Biodome)
					continue;
				if(bd->SiteContains(loc))
					bds.append((DaBd*)bd);	// Cast away constness.
			}
		}

		if(hasPlat)
		{
			for(AuIdList::const_iterator bi = bdsNear.cbegin(); bi != bdsNear.cend(); bi++)
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
				if(!bd)
					continue;
				if(bd->TestFindFilter(filter))
					continue;
				if(!DbIndustry::IsPlatform(bd->IndustryId()))
					continue;
				if(bd->SiteContains(loc))
					bds.append((DaBd*)bd);	// Cast away constness.
			}
		}
	}
}

DaBd* DaWorldLL::FindBdContaining(const AuPoint& loc, FINDDEV filter) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	DaAtomList biodomes;
	DaAtomList platforms;
	AuIdList bds = BuildingsAt(ll);
	if(bds.empty())
		return 0;
	// Use the ether's hash table.
	DaBd* bdFound = 0;
	for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
		if(!bd)
			continue;
		INDUSTRYID iid = bd->IndustryId();
		if(bdFound && iid != IID_Bridge)
			continue;	// We only care about bridges after we find a bd site that contains 'loc'.
		if(bd->TestFindFilter(filter))
			continue;
		if(DbIndustry::IsPlatform(iid))
		{
			platforms.append((DaBd*)bd);	// Cast away constness.
			continue;
		}
		else if(iid == IID_Biodome)
		{
			biodomes.append((DaBd*)bd);	// Cast away constness.
			continue;
		}
		if(bd->SiteContains(loc))
		{
			bdFound = (DaBd*)bd;	// Cast away constness.
			if(iid == IID_Bridge)
				return bdFound;
		}
	}
	// If we found a site, return it.
	if(bdFound)
		return bdFound;

	// Check biodomes next so we find them on platforms.
	for(DaAtomList::const_iterator bi = biodomes.cbegin(); bi != biodomes.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->SiteContains(loc))
			return bd;
	}
	// Check platforms last so we find the stuff on them first.
	for(DaAtomList::const_iterator bi = platforms.cbegin(); bi != platforms.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->SiteContains(loc))
			return bd;
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdContaining(const AuPoint& loc, FINDDEV filter, const DaBd& onPlatform) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	DaAtomList biodomes;
	DaAtomList platforms;
	AuIdList bds = BuildingsAt(ll);
	if(!bds.empty())
	{
		// Use the ether's hash table.
		DaBd* bdFound = 0;
		for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
			if(!bd)
				continue;
			if(!onPlatform.Is(bd->PlatformId()))
				continue;	// Bd is not on the platform.
			INDUSTRYID iid = bd->IndustryId();
			if(bdFound && iid != IID_Bridge)
				continue;	// We only care about bridges after we find a bd site that contains 'loc'.
			if(bd->TestFindFilter(filter))
				continue;
			if(DbIndustry::IsPlatform(iid))
			{
				platforms.append((DaBd*)bd);	// Cast away constness.
				continue;
			}
			else if(iid == IID_Biodome)
			{
				biodomes.append((DaBd*)bd);	// Cast away constness.
				continue;
			}
			if(bd->SiteContains(loc))
			{
				bdFound = (DaBd*)bd;	// Cast away constness.
				if(iid == IID_Bridge)
					return bdFound;
			}
		}
		// If we found a site, return it.
		if(bdFound)
			return bdFound;
	}

	// Check biodomes next so we find them on platforms.
	for(DaAtomList::const_iterator bi = biodomes.cbegin(); bi != biodomes.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->SiteContains(loc))
			return bd;
	}
	// Check platforms last so we find the stuff on them first.
	for(DaAtomList::const_iterator bi = platforms.cbegin(); bi != platforms.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->SiteContains(loc))
			return bd;
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdContaining(INDUSTRYID iid, const AuPoint& loc, FINDDEV filter) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	AuIdList bds = BuildingsAt(ll);
	if(bds.empty())
		return 0;
	// Use the ether's hash table.
	DaBd* bdFound = 0;
	for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
		if(!bd)
			continue;
		if(bd->IndustryId() != iid)
			continue;
		if(bd->TestFindFilter(filter))
			continue;
		if(bd->SiteContains(loc))
		{
			bdFound = (DaBd*)bd;	// Cast away constness.
			return bdFound;
		}
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdContaining(INDUSTRYID iid, const AuPoint& loc, FINDDEV filter, const DaBd& onPlatform) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	AuIdList bds = BuildingsAt(ll);
	if(!bds.empty())
	{
		// Use the ether's hash table.
		DaBd* bdFound = 0;
		for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
			if(!bd)
				continue;
			if(!onPlatform.Is(bd->PlatformId()))
				continue;	// Bd is not on the platform.
			if(bd->IndustryId() != iid)
				continue;
			if(bd->TestFindFilter(filter))
				continue;
			if(bd->SiteContains(loc))
			{
				bdFound = (DaBd*)bd;	// Cast away constness.
				return bdFound;
			}
		}
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindMilitaryContaining(const AuPoint& loc, FINDDEV filter) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	AuIdList bds = BuildingsAt(ll);
	if(bds.empty())
		return 0;
	// Use the ether's hash table.
	DaBd* bdFound = 0;
	for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
		if(!bd)
			continue;
		if(!bd->IsMilitary())
			continue;
		if(bd->TestFindFilter(filter))
			continue;
		if(bd->SiteContains(loc))
		{
			bdFound = (DaBd*)bd;	// Cast away constness.
			return bdFound;
		}
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindMilitaryContaining(const AuPoint& loc, FINDDEV filter, const DaBd& onPlatform) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	AuIdList bds = BuildingsAt(ll);
	if(!bds.empty())
	{
		// Use the ether's hash table.
		DaBd* bdFound = 0;
		for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
			if(!bd)
				continue;
			if(!onPlatform.Is(bd->PlatformId()))
				continue;	// Bd is not on the platform.
			if(!bd->IsMilitary())
				continue;
			if(bd->TestFindFilter(filter))
				continue;
			if(bd->SiteContains(loc))
			{
				bdFound = (DaBd*)bd;	// Cast away constness.
				return bdFound;
			}
		}
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdContaining(const AuPoint& loc, FINDDEV filter, ON_GROUND& siteGround, int& hideGrass) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!BdHere())
		return 0;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return 0;
	DaAtomList biodomes;
	DaAtomList platforms;
	AuIdList bds = BuildingsAt(ll);
	if(!bds.empty())
	{
		// Use the ether's hash table.
		DaBd* bdFound = 0;
		for(AuIdList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(ether->FindNormalAtom(*bi));
			if(!bd)
				continue;
			INDUSTRYID iid = bd->IndustryId();
			if(bdFound && iid != IID_Bridge)
				continue;	// We only care about bridges after we find a bd site that contains 'loc'.
			if(bd->TestFindFilter(filter))
				continue;
			if(DbIndustry::IsPlatform(iid))
			{
				platforms.append((DaBd*)bd);	// Cast away constness.
				continue;
			}
			else if(iid == IID_Biodome)
			{
				biodomes.append((DaBd*)bd);	// Cast away constness.
				continue;
			}
			if(bd->SiteContains(loc, siteGround, hideGrass))
			{
				bdFound = (DaBd*)bd;	// Cast away constness.
				if(iid == IID_Bridge)
					return bdFound;
			}
		}
		// If we found a site, return it.
		if(bdFound)
			return bdFound;
	}

	// Check biodomes next so we find them on platforms.
	for(DaAtomList::const_iterator bi = biodomes.cbegin(); bi != biodomes.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->SiteContains(loc, siteGround, hideGrass))
			return bd;
	}
	// Check platforms last so we find the stuff on them first.
	for(DaAtomList::const_iterator bi = platforms.cbegin(); bi != platforms.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->SiteContains(loc, siteGround, hideGrass))
			return bd;
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdContaining(const AuPoint& loc, double radius, FINDDEV filter) const
{
	if(!BdHere())
		return 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->TestFindFilter(filter))
			continue;
		if(!bd->SiteExtentsContain(loc, radius))
			continue;
		return bd;
	}
	return 0;
}

DaBd* DaWorldLL::FindBdContaining(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter) const
{
	if(!BdHere())
		return 0;

	// Check the origin location of the building to see if it falls within the site of any existing building.
	DaBd* bd = FindBdContaining(mBld.Translation(), filter);
	if(bd)
		return bd;
	// Check the site center location of the building to see if it falls within the site of any existing building.
	AuPoint smax = report.SiteMax();
	AuPoint smin = report.SiteMin();
	AuPoint scen = (smin + smax)/2;
	bd = FindBdContaining(mBld.Project(scen), filter);
	if(bd)
		return bd;

	// Check to see if the origin or site center of any existing building falls within the site of the proposed building.
	filter = (FINDDEV)(filter|FIND_SkipBiodomes|FIND_SkipPlatforms);
	DaBd bdnu;
	bdnu.SetPosAndOrient(AuAtomPos(Id(), mBld));
	bdnu.SetDesign(report);
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->TestFindFilter(filter))
			continue;
		if(!bdnu.SiteContains(bd->Location()) && !bdnu.SiteContains(bd->Transform().Project(bd->SiteCenter())))
			continue;
		return bd;
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdContaining(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter, const DaBd& onPlatform) const
{
	if(!BdHere())
		return 0;

	// Check the origin location of the building to see if it falls within the site of any existing building.
	DaBd* bd = FindBdContaining(mBld.Translation(), filter, onPlatform);
	if(bd)
		return bd;
	// Check the site center location of the building to see if it falls within the site of any existing building.
	AuPoint smax = report.SiteMax();
	AuPoint smin = report.SiteMin();
	AuPoint scen = (smin + smax)/2;
	bd = FindBdContaining(mBld.Project(scen), filter, onPlatform);
	if(bd)
		return bd;

	// Check to see if the origin or site center of any existing building falls within the site of the proposed building.
	filter = (FINDDEV)(filter|FIND_SkipBiodomes|FIND_SkipPlatforms);
	DaBd bdnu;
	bdnu.SetPosAndOrient(AuAtomPos(Id(), mBld));
	bdnu.SetDesign(report);
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(!onPlatform.Is(bd->PlatformId()))
			continue;
		if(bd->TestFindFilter(filter))
			continue;
		if(!bdnu.SiteContains(bd->Location()) && !bdnu.SiteContains(bd->Transform().Project(bd->SiteCenter())))
			continue;
		return bd;
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdSiteOverlap(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter) const
{
	if(!BdHere())
		return 0;
	DaBd bdnu;
	bdnu.SetPosAndOrient(AuAtomPos(Id(), mBld));
	bdnu.SetDesign(report);
	AuPoint nuSiteCen = mBld.Project(bdnu.SiteCenter());	// in world space
	double nuSiteRad = bdnu.RoughSiteRadius();
	AuPointArray nuSiteLocs;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->TestFindFilter(filter))
			continue;
		// Skip certain kinds of buildings.
		switch(bd->IndustryId())
		{
		default:
			break;
		case IID_AerialPlatform:
		case IID_GroundPlatform:
		case IID_Biodome:
		case IID_Bridge:
		case IID_Fence:
		case IID_Landscape:
		case IID_Road:
		case IID_SeaPlatform:
		case IID_StreetLight:
		case IID_Tunnel:
			continue;	// These don't need extra site overlap checking.
		}

		// Are they too far apart to do the hard work.
		AuPoint bdSiteCen(bd->Transform().Project(bd->SiteCenter()));	// in world space
		double bdSiteRad = bd->RoughSiteRadius();
		double dist2 = nuSiteRad + bdSiteRad;
		dist2 *= dist2;	// squared
		if(nuSiteCen.Distance2(bdSiteCen) > dist2)
			continue;

		// First test the insertion point and site center point.
		if(!bdnu.SiteContains(bd->Location()) && !bdnu.SiteContains(bdSiteCen))
		{
			// Next test the site locations of the proposed bd against the site of the existing bd.
			if(nuSiteLocs.empty())
			{
				bdnu.BuildZoneCheckLocs(nuSiteLocs);
				nuSiteLocs.Transform(mBld);						// Into world space of bdnu.
			}
			bool anyInSite = bd->SiteContains(nuSiteLocs);
			if(!anyInSite)
			{
				// Reverse the check. Test the site locations of the existing bd against the site of the proposed bd.
				AuPointArray siteLocs;
				bd->BuildZoneCheckLocs(siteLocs);
				siteLocs.Transform(bd->Transform());			// Into world space of bd.
				anyInSite = bdnu.SiteContains(siteLocs);
				if(!anyInSite)
					continue;
			}
		}
		return bd;
	}
	return 0;	// No building was found there.
}

DaBd* DaWorldLL::FindBdSiteOverlap(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter, const DaBd& onPlatform) const
{
	if(!BdHere())
		return 0;
	DaBd bdnu;
	bdnu.SetPosAndOrient(AuAtomPos(Id(), mBld));
	bdnu.SetDesign(report);
	AuPoint nuSiteCen = mBld.Project(bdnu.SiteCenter());	// in world space
	double nuSiteRad = bdnu.RoughSiteRadius();
	AuPointArray nuSiteLocs;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(!onPlatform.Is(bd->PlatformId()))
			continue;
		if(bd->TestFindFilter(filter))
			continue;
		// Skip certain kinds of buildings.
		switch(bd->IndustryId())
		{
		default:
			break;
		case IID_AerialPlatform:
		case IID_GroundPlatform:
		case IID_Biodome:
		case IID_Bridge:
		case IID_Fence:
		case IID_Landscape:
		case IID_Road:
		case IID_SeaPlatform:
		case IID_StreetLight:
		case IID_Tunnel:
			continue;	// These don't need extra site overlap checking.
		}

		// Are they too far apart to do the hard work.
		AuPoint bdSiteCen(bd->Transform().Project(bd->SiteCenter()));	// in world space
		double bdSiteRad = bd->RoughSiteRadius();
		double dist2 = nuSiteRad + bdSiteRad;
		dist2 *= dist2;	// squared
		if(nuSiteCen.Distance2(bdSiteCen) > dist2)
			continue;

		// First test the insertion point and site center point.
		if(!bdnu.SiteContains(bd->Location()) && !bdnu.SiteContains(bdSiteCen))
		{
			// Next test the site locations of the proposed bd against the site of the existing bd.
			if(nuSiteLocs.empty())
			{
				bdnu.BuildZoneCheckLocs(nuSiteLocs);
				nuSiteLocs.Transform(mBld);						// Into world space of bdnu.
			}
			bool anyInSite = bd->SiteContains(nuSiteLocs);
			if(!anyInSite)
			{
				// Reverse the check. Test the site locations of the existing bd against the site of the proposed bd.
				AuPointArray siteLocs;
				bd->BuildZoneCheckLocs(siteLocs);
				siteLocs.Transform(bd->Transform());			// Into world space of bd.
				anyInSite = bdnu.SiteContains(siteLocs);
				if(!anyInSite)
					continue;
			}
		}
		return bd;
	}
	return 0;	// No building was found there.
}

//  0 = ok
//  1 = build location not on this world
//	2 = Alien military HQ present
//	3 = Alien jurisdiction present
//  4 = Empire military HQ needed.
//  5 = Jurisdiction present
//  6 = Empire jurisdiction or empire capitol needed.
//  7 = Empire jurisdiction needed.
//  8 = Too hot.
//  9 = too cold
// 10 = Building would stick out of atmosphere.
// 11 = forbidden zone
// 12 = blocked by existing construction
// 13 = water is not deep enough
// 14 = Insufficient framework for gas giant construction.
// 15 = Insufficient framework for under sea construction.
// 16 = life support inadequate for gas giant.
// 17 = no trees
// 18 = no rocks
// 19 = too wet
// 20 = too dry
// 21 = drilling rig must be within 1m of sea surface
// 22 = Alien military building present
// 23 = life support inadequate for under sea
// 24 = life support inadequate for inferno atmosphere
// 25 = life support inadequate for frigid atmosphere
// 26 = life support inadequate for unbreathable atmosphere
// 27 = Insufficient framework for inferno land construction.
// 28 = Insufficient framework for frigid land construction.
// 29 = drilling rig must touch the bottom
// 30 = Insufficient framework for frigid under sea construction.
// 31 = life support inadequate for frigid sea buttom
// 32 = world not claimed by any empire
// 33 = world claimed by a different empire
// 34 = intersects sea surface. Associated with 2-point constructions, e.g. road from land to sea or biodome too tall
// 35 = road segment too short/biodome too small
// 36 = biodome blocks placement inside
// 37 = road too steep
// 38 = Enemy unit present in construction area.
// 39 = Would create overlapping jurisdictions.
// 40 = Too high above virgin terrain.
// 41 = Too low below virgin terrain.
// 42 = Cannot be built on a platform.
// 43 = Cannot be built on aerial platform.
// 44 = Cannot be built on ground platform.
// 45 = Cannot be built on sea platform.
// 46 = Invalid Industry
// 47 = Building site falls off platform.
// 48 = Fence too long.
// 49 = Empire military HQ already present.
int DaWorldLL::TestBdLocation(INDUSTRYID lIID, quint32 empireOfBuilder, const AuBdReport* bld, COMMODITYID buildMaterial,
		const AuPoint& buildLoc, double roadWid2, float buildRotZ, const DaBd* onPlatform, DaBd*& exDev,
		const AuPoint* startLoc, QString& cityName) const
{
	exDev = 0;
	AuGlobeLLF buildLL;
	if(!LLOfLoc(buildLL, buildLoc))
	{
		qDebug() << "DaWorldLL::TestBdLocation" << Id() << Name() << "Not in terrain";
		return 1;
	}

	// Check build rule for industry.
	const DgData& gd = g_Au.LockDataForRead();
	int buildRule = gd.IndustryBuildRule(lIID);
	g_Au.UnlockData();
	AuId jurisId;
	quint32 jurisEmp = 0;
	switch(buildRule)
	{
	case 0:	// Civilian
		if(ClaimedByEmpire() && ClaimedByEmpire() != empireOfBuilder)
			return 33;	// Claimed by an empire that is not the builder's empire.
		if(InAlienJurisdiction(buildLoc, empireOfBuilder))
			return 3;	// Jurisdiction of alien city present.
		jurisId = JurisdictionAt(buildLoc, jurisEmp);
		break;
	case 1:	// Military HQ
		if(!ClaimedByEmpire())
			return 32;	// Not claimed by any empire, particularly the builder's empire.
		else if(ClaimedByEmpire() != empireOfBuilder)
			return 33;	// Claimed by an empire that is not the builder's empire.
		if(HasEmpireMilitaryHQ(empireOfBuilder, true))
			return 49;	// Already has a military HQ on this world; it could be under construction.
		break;
	case 2:	// Military
		if(!HasEmpireMilitaryHQ(empireOfBuilder, false))
			return 4;	// No empire military hq on the planet that is not under construction.
		break;
	case 3:	// Capitol
		if(ClaimedByEmpire() && ClaimedByEmpire() != empireOfBuilder)
			return 33;	// Claimed by an empire that is not the builder's empire.
		if(InAnyJurisdiction(buildLoc))
			return 5;	// Location is in an existing city's jurisdiction. This notices other capitols because they all have the
						// same jurisdiction radius.
		if(WouldCreateOverlappingJurisdictions(buildLoc, bld->CapitolJurisdictionRadius(), true))
			return 39;
		break;
	case 4:	// Police
		jurisId = CapitolInProposedJurisdiction(buildLoc, bld->PoliceJurisdictionRadius(), empireOfBuilder);
		if(jurisId.IsNull() && !InEmpireJurisdiction(buildLoc, empireOfBuilder))
			return 6;	// Empire jurisdiction or capitol required.
		if(WouldCreateOverlappingJurisdictions(buildLoc, bld->PoliceJurisdictionRadius(), false))
			return 39;
		if(jurisId.IsNull())
			jurisId = JurisdictionAt(buildLoc, jurisEmp);
		break;
	case 5:	// City
		if(!InEmpireJurisdiction(buildLoc, empireOfBuilder))
			return 7;	// Empire jurisdiction required.
		jurisId = JurisdictionAt(buildLoc, jurisEmp);
		break;
	}

	if(!jurisId.IsNull())
	{
		const DaBd* jurisBd = dynamic_cast<const DaBd*>(FindIdRecursive(jurisId));
		if(jurisBd)
			cityName = jurisBd->CityName();
	}

	// Determine if this is a valid build location for the industry. Get build rules.
	double rAlt = AltitudeMSL(buildLoc);
	CONSTRUCTION_TERRAIN cTer = CONST_DryGround;	// Construction terrain rules required by the industry.
	switch(lIID)
	{
	case IID_BaseFlag:
	case IID_Count:
	case IID_None:
	case IID_TownSquare:
		return 46;	// Invalid industry

	case IID_AerialPlatform:
	case IID_GroundPlatform:
		if(onPlatform)
			return 42;	// Cannot be built on a platform.
		cTer = CONST_Ground;
		break;

	case IID_BaseDock:
	case IID_ShipYard:
	case IID_Wharf:
		// Requires the sea.
		cTer = CONST_Coast;
		if(onPlatform && onPlatform->Structure() == BD_AerialPlatform)
			return 43;	// Cannot be built on aerial platform.
		break;

	case IID_Biodome:
	case IID_Bridge:
		cTer = CONST_Ground;
		break;

	case IID_DrillingRig:
	case IID_SeaPlatform:
		if(onPlatform)
			return 42;	// Cannot be built on a platform.
		cTer = CONST_SeaSurface;
		break;

	case IID_Fence:
		cTer = CONST_Ground;
		break;

	case IID_HydroelectricPowerPlant:
		// Requires the sea but not polar.
		if(onPlatform && onPlatform->Structure() == BD_AerialPlatform)
			return 43;	// Cannot be built on aerial platform.
		if(rAlt >= 0)
		{
			// Prevent hydroelectric power plants in polar region/frigid zones.
			if(fabs(buildLL.Latitude()) > PolarLatitude())
				return 9;	// too cold
			if(OrbitZone() == OZONE_Frigid)
				return 9;	// too cold
		}
		cTer = CONST_Coast;
		break;

	case IID_Landscape:
		if(onPlatform)
			return 42;	// Cannot be built on a platform.
		cTer = CONST_Ground;
		break;

	case IID_LoggingCamp:
		if(onPlatform)
			return 42;	// Cannot be built on a platform.
		cTer = CONST_Trees;
		break;

	case IID_Mine:
		// Requires the ground.
		cTer = CONST_Rocks;
		if(onPlatform && onPlatform->Structure() == BD_AerialPlatform)
			return 43;	// Cannot be built on aerial platform.
		else if(onPlatform && onPlatform->Structure() == BD_SeaPlatform)
			return 45;	// Cannot be built on sea platform.
		break;

	case IID_Road:
		if(onPlatform)
			return 42;	// Cannot be built on a platform.
		cTer = CONST_Ground;
		break;

	case IID_StreetLight:
		cTer = CONST_Ground;
		break;

	case IID_TreatmentPlant:
		cTer = CONST_CoastLava;
		break;

	case IID_Tunnel:
		if(onPlatform)
			return 42;	// Cannot be built on a platform.
		break;

	case IID_Well:
		// Requires the ground and can reach it through water.
		cTer = CONST_Well;
		if(onPlatform && onPlatform->Structure() == BD_AerialPlatform)
			return 43;	// Cannot be built on aerial platform.
		break;

	case IID_AircraftFactory:
	case IID_AirportRadar:
	case IID_AirportRepairShop:
	case IID_AirportTerminal:
	case IID_Apartment:
	case IID_Arena:
	case IID_Armorer:
	case IID_Bakery:
	case IID_Bank:
	case IID_BaseAcademy:
	case IID_BaseAirTerminal:
	case IID_BaseBarracks:
	case IID_BaseGuardPost:
	case IID_BaseGuardTower:
	case IID_BaseHeadquarters:
	case IID_BaseMarineBarracks:
	case IID_BaseMotorPool:
	case IID_BaseRadar:
	case IID_BaseShield:
	case IID_BaseWeapon:
	case IID_Brewery:
	case IID_Broker:
	case IID_Cantina:
	case IID_Capitol:
	case IID_Carpenter:
	case IID_Casino:
	case IID_Church:
	case IID_ClothingFactory:
	case IID_CoalPowerPlant:
	case IID_ComputerFactory:
	case IID_Dairy:
	case IID_DesignStudio:
	case IID_Distillery:
	case IID_ElectronicsFactory:
	case IID_ExplosivesFactory:
	case IID_Farm:
	case IID_FireDepartment:
	case IID_GasolinePowerPlant:
	case IID_GristMill:
	case IID_Grocery:
	case IID_GunSmith:
	case IID_Hangar:
	case IID_Hospital:
	case IID_Hotel:
	case IID_House:
	case IID_HydrogenPowerPlant:
	case IID_Jeweler:
	case IID_LumberMill:
	case IID_MachineShop:
	case IID_MassMedia:
	case IID_MeatProcessingPlant:
	case IID_MunitionsFactory:
	case IID_NaturalGasPowerPlant:
	case IID_NuclearFuelPlant:
	case IID_NuclearPowerPlant:
	case IID_Observatory:
	case IID_Orchard:
	case IID_PaperMill:
	case IID_Park:
	case IID_PharmaceuticalLab:
	case IID_PlasticsFactory:
	case IID_PoliceAcademy:
	case IID_PoliceDepartment:
	case IID_Refinery:
	case IID_RetailStore:
	case IID_RocketMotorFactory:
	case IID_School:
	case IID_Shed:
	case IID_Smelter:
	case IID_SolarPowerPlant:
	case IID_SpacecraftFactory:
	case IID_StorageTank:
	case IID_SugarMill:
	case IID_TextileMill:
	case IID_VehicleFactory:
	case IID_Warehouse:
	case IID_WindPowerPlant:
	case IID_Winery:
	case IID_Zoo:
		// No special requirements. Can be built on a platform.
		break;
	}

	// All construction must occur within the atmosphere radius, even if there is no atmosphere.
	if(bld && (rAlt + bld->HullMax().z + 100*FeetPerMeter() > AtmosphereHeightMSL()))
		return 10;	// The terrain location is outside the atmosphere.

	// Calculate a matrix for orientation of the building.
	AuMatrix4x4 mBld;	// ONLY used when bld is non-zero.
	if(bld)
	{
		BuildingConstructionTransform(buildLoc, buildRotZ, onPlatform, bld->SiteCenter(), mBld);
		if(onPlatform)
		{
			// Insure all site locations are in a build zone of the platform.
			AuPointArray siteLocs;
			const DgData& bdd = g_Au.LockBdDesignsForRead();
			bool hasDesign = bdd.BdDesignBuildZoneCheckLocs(bld->DesignId(), siteLocs);
			g_Au.UnlockBdDesigns();
			if(!hasDesign)
				bld->BuildZoneCheckLocs(siteLocs);					// The site extents will have to work.
			siteLocs.Transform(mBld);								// Locs in the space of the world.
			siteLocs.Transform(onPlatform->Transform().Inverse());	// Locs in the space of the platform.

			// Elevate test locations slightly above the floor, to avoid fuzzy collisions.
			AuMatrix4x4 liftM;
			liftM.Translate(0,0,.1);
			siteLocs.Transform(liftM);

			if(!onPlatform->TestInBuildZone(siteLocs))
				return 47;	// Site fell off the platform.
		}
	}
	else if(startLoc && onPlatform)
	{
		// Insure both ends of the segment are in a build zone of the platform.
		AuPointArray siteLocs;
		siteLocs.append(buildLoc);
		siteLocs.append(*startLoc);
		siteLocs.Transform(onPlatform->Transform().Inverse());	// Locs in the space of the platform.

		// Elevate test locations slightly above the floor, to avoid fuzzy collisions.
		AuMatrix4x4 liftM;
		liftM.Translate(0,0,.1);
		siteLocs.Transform(liftM);

		if(!onPlatform->TestInBuildZone(siteLocs))
			return 47;	// Site fell off the platform.
	}

	// Test at maximum possible radius.
	AuGlobeLLF cenLL;		// For forbidden zone test.
	double maxRadius;		// For forbidden zone test.
	AuPoint siteBoxRadius;
	if(bld)
	{
		// Calculate center of build site.
		AuPoint siteMin(bld->SiteMin());
		AuPoint siteMax(bld->SiteMax());
		if(siteMin == siteMax)
		{
			siteMin = bld->HullMin();
			siteMax = bld->HullMax();
		}
		AuPoint siteCen = mBld.Project((siteMin + siteMax)/2);
		LLOfLoc(cenLL, siteCen);
		siteBoxRadius = (siteMax - siteMin)/2;
		maxRadius = siteBoxRadius.Length_2D();
	}
	else if(startLoc)
	{
		if(lIID == IID_Fence && startLoc->Distance(buildLoc) > 1000 * FeetPerMeter())
			return 48;
		AuPoint cen((*startLoc + buildLoc)/2);
		if(!LLOfLoc(cenLL, cen))
			return 1;
		maxRadius = startLoc->Distance(buildLoc) / 2;
		siteBoxRadius = AuPoint(maxRadius, maxRadius, 3*FeetPerMeter());	// Road doesn't need more than 3m for roof test.
	}
	else
	{
		cenLL = buildLL;
		maxRadius = roadWid2 + 10*FeetPerMeter();
		siteBoxRadius = AuPoint(maxRadius, maxRadius, 3*FeetPerMeter());	// Road doesn't need more than 3m for roof test.
	}

	// Any enemy unit on the ground in the area blocks construction.
	if(bld && EnemyUnitBlocksConstruction(mBld, siteBoxRadius, empireOfBuilder))
		return 38;

	// Test for zones that are off limits to constructions. See ringworld sections for examples of this, at ends and control center.
	if(IsForbiddenZone(cenLL, rAlt, maxRadius))
		return 11;

	// Check for existing development at build location.
	BUILDING_STRUCTURE bdStruct;
	if(bld)
	{
		bdStruct = bld->Structure();
		switch(bdStruct)
		{
		case BD_AerialPlatform:
			//DHH Ignore existing developments until we get more picky about existing aerial developments.
			break;
		case BD_SeaPlatform:
		case BD_SeaSurfaceBuilding:
			//DHH Ignore existing developments until we get more picky about existing sea surface developments.
			break;
		case BD_Fence:			// Fences ignore existing developments.
		case BD_Invalid:
		case BD_Count:
			break;
		case BD_Biodome:
		case BD_Bridge:
		case BD_GroundPlatform:
		case BD_RoadEnclosure:
		case BD_SpaceElevator:
		case BD_Statue:
		case BD_Tunnel:
		case BD_StreetLight:
			if(onPlatform)
				exDev = FindBdContaining(*bld, mBld, FIND_SkipLandscapers, *onPlatform);
			else
				exDev = FindBdContaining(*bld, mBld, FIND_SkipLandscapers);
			break;
		case BD_Building:
			if(onPlatform)
			{
				exDev = FindBdContaining(*bld, mBld, FIND_SkipLandscapers, *onPlatform);
				if(!exDev)
					exDev = FindBdSiteOverlap(*bld, mBld, (FINDDEV)(FIND_SkipLandscapers|FIND_SkipBiodomes|FIND_SkipPlatforms),
							*onPlatform);
			}
			else
			{
				exDev = FindBdContaining(*bld, mBld, FIND_SkipLandscapers);
				if(!exDev)
					exDev = FindBdSiteOverlap(*bld, mBld, (FINDDEV)(FIND_SkipLandscapers|FIND_SkipBiodomes|FIND_SkipPlatforms));
			}
			break;
		}
	}
	else
	{
		// No building report. Must be a road or fence.
		if(lIID == IID_Fence)
		{
			bdStruct = BD_Fence;
		}
		else
		{
			bdStruct = BD_Invalid;
			exDev = FindBdContaining(buildLoc, FIND_SkipLandscapers);
		}
	}
	if(exDev)
	{
		bool errorOnOverlap;
		if(!bld)
			errorOnOverlap = false;	// Ok to overlap anything with non buildings.
		else if(lIID == IID_StreetLight)
			errorOnOverlap = exDev->IndustryId() == IID_StreetLight;	// Street lights overlap anything except street lights.
		else if(exDev->IndustryId() == IID_StreetLight)
			errorOnOverlap = false;	// Ok to overlap existing street lights with anything, except street lights.
		else
			errorOnOverlap = true;	// Overlap is not allowed.
		if(errorOnOverlap)
		{
			// Enclosure allows things to be built inside it, as long as they are completely inside.
			// Airport terminal cannot be built inside enclosure.
			if(lIID == IID_AirportTerminal ||
					lIID == IID_BaseAirTerminal ||
					!exDev->EnclosureContainsCompletely(*this, mBld, siteBoxRadius))
				return 12;			// Cannot construct on this existing development. Must clear the land first.
		}
	}

	if(lIID == IID_Road)
	{
		// Prevent roads from raising|lowering terrain too much to prevent islands on gas giants and other mischief.
		if(startLoc)
		{
			AuGlobeLL startLL;
			if(!LLOfLoc(startLL, *startLoc))
				return 1;
			double virginStartAlt = UndevelopedVirginTerrainAltitudeMSL(startLL);
			double startAlt = AltitudeMSL(*startLoc);
			if(startAlt > virginStartAlt)
			{
				// Check too high above virgin terrain.
				double virginStartChange = startAlt - virginStartAlt;
				if(virginStartChange > AuBdDesign::SiteSizeLimitHeight(BD_Building) * 2)
					return 40;
			}
			else if(startAlt < virginStartAlt)
			{
				// Check too low below virgin terrain.
				double virginStartChange = virginStartAlt - startAlt;
				if(virginStartChange > AuBdDesign::SiteSizeLimitDepth(BD_Building) * 2)
					return 41;
			}
		}

		AuGlobeLL endLL;
		if(!LLOfLoc(endLL, buildLoc))
			return 1;
		double virginEndAlt = UndevelopedVirginTerrainAltitudeMSL(endLL);
		if(rAlt > virginEndAlt)
		{
			// Check too high above virgin terrain.
			double virginEndChange = rAlt - virginEndAlt;
			if(virginEndChange > AuBdDesign::SiteSizeLimitHeight(BD_Building) * 2)
				return 40;
		}
		else if(rAlt < virginEndAlt)
		{
			// Check too low below virgin terrain.
			double virginEndChange = virginEndAlt - rAlt;
			if(virginEndChange > AuBdDesign::SiteSizeLimitDepth(BD_Building) * 2)
				return 41;
		}
	}

	// Get information about the terrain and resources at the construction site.
	AuTerrainInfo ter;
	QueryTerrain(ter, buildLL);
	ORBITZONE orbitZone = OrbitZone();
	if(!ter.m_InEnclosure)
		ter.m_InEnclosure = InEnclosure(buildLL, rAlt);	// Test for aerial enclosure.

	// Test the planet location for terrain requirements of the particular construction process.
	if(ter.m_InEnclosure && bdStruct != BD_AerialPlatform)
	{
		// We are building inside an enclosure, either a biodome or an enclosed platform.
		// If not, then the location is known to be inside an enclosure due to QueryTerrain. That is possible when an enclosure has
		// no site mesh in its environment area, which is optional.
		// The location is inside an enclosure. Does the building fit?
		if(bld)
		{
			AuId bioId = EnclosureContaining(buildLoc);
			if(!bioId.IsNull())
			{
				const DaBd* enc = dynamic_cast<const DaBd*>(FindSubatomId(bioId));
				if(enc && !enc->EnclosureContainsCompletely(*this, mBld, siteBoxRadius))
					return 36;			// Biodome blocks placement inside.
			}
		}
		switch(cTer)
		{
		case CONST_Coast:
		case CONST_CoastLava:
		case CONST_DryGround:
		case CONST_Ground:
		case CONST_Well:
			break;

		case CONST_Rocks:
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(rocks < 1)
					return 18;	// The terrain has no rocks.
			}
			break;

		case CONST_SeaSurface:
			return 20;	// too dry

		case CONST_Trees:
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(trees < 1)
					return 17;	// The terrain has no trees.
			}
			break;

		default:
			Q_ASSERT(false);	// Unrecognized construction terrain type id.
			break;
		}
	}
	else if(ter.IsWater() && bdStruct != BD_AerialPlatform && cTer != CONST_SeaSurface &&
			(!onPlatform || onPlatform->Structure() == BD_GroundPlatform))
	{
		// We are building under the sea.
		// Terrain is liquid and we're not building an aerial platform or sea surface structure. Those buildings are treated as if
		// exposed to atmosphere, not undersea environment.
		// We're not on a platform or the platform is on the ground, at the bottom of the sea. It can't stick out of the surface.
		// Prevent too tall buildings from sticking out of water.
		if(bld && (ter.WaterDepth() < bld->HullMax().z + 10*FeetPerMeter()))
			return 13;	// Water or supercooled liquid is not deep enough to sit on the bottom.

		// Armor and life support needed for underwater construction of everything except fences and walls.
		if(bld)
		{
			if(ter.IsGasGiant())
			{
				if(bdStruct != BD_Fence)
				{
					if(!bld->SkeletonOkForGasGiantCore() && !bld->ArmorOkForGasGiantCore())
						return 14;
					if(!bld->LifeSupportOkForGasGiantCore())
						return 16;
				}
			}
			else
			{
				if(orbitZone == OZONE_Inferno)
					return 8;	// Too hot. Cannot build submerged in lava.
				if(orbitZone == OZONE_Frigid)
				{
					if(bdStruct != BD_Fence)
					{
						if(!bld->SkeletonOkForFrigidSeaBottom() && !bld->ArmorOkForFrigidSeaBottom())
							return 30;
						if(!bld->LifeSupportOkForFrigidSeaBottom())
							return 31;
					}
				}
				else
				{
					if(bdStruct != BD_Fence)
					{
						if(!bld->SkeletonOkForSeaBottom() && !bld->ArmorOkForSeaBottom())
							return 15;
						if(!bld->LifeSupportOkForSeaBottom())
							return 23;
					}
				}
			}
		}

		// Location clicked is in deep enough sea to enable construction.
		switch(cTer)
		{
		case CONST_Coast:
		case CONST_CoastLava:
			return 19;	// Terrain is under water.

		case CONST_DryGround:
		case CONST_Ground:
		case CONST_Well:
			break;

		case CONST_Rocks:
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(rocks < 1)
					return 18;	// The terrain has no rocks.
			}
			break;

		case CONST_SeaSurface:
			return 19;	// Too wet.

		case CONST_Trees:
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(trees < 1)
					return 17;	// The terrain has no trees.
			}
			break;
		}
	}
	else
	{
		// We are building on land, on the sea surface, or on a platform that is not submerged.
		// Check material can tolerate build environment.
		if(bld)
		{
			if(ter.IsGasGiant())
			{
				if(bdStruct != BD_Fence)
				{
					// Building an aerial or sea platform on a gas giant or a building on an aerial or sea platform on a gas giant.
					if(!bld->LifeSupportOkForFrigidLand())
						return 25;
					if(!bld->SkeletonOkForFrigidLand() && !bld->ArmorOkForFrigidLand())
						return 28;
				}
			}
			else if(orbitZone == OZONE_Inferno)
			{
				if(bdStruct != BD_Fence)
				{
					if(!bld->LifeSupportOkForInfernoLand())
						return 24;
					if(!bld->SkeletonOkForInfernoLand() && !bld->ArmorOkForInfernoLand())
						return 27;
				}
				if(buildMaterial == CID_Wood || buildMaterial == CID_Lumber || buildMaterial == CID_CrudeOil ||
						buildMaterial == CID_Textiles)
					return 8;
			}
			else if(orbitZone == OZONE_Frigid)
			{
				if(bdStruct != BD_Fence)
				{
					if(!bld->LifeSupportOkForFrigidLand())
						return 25;
					if(!bld->SkeletonOkForFrigidLand() && !bld->ArmorOkForFrigidLand())
						return 28;
				}
			}
			else if(!HasBreathableAtmosphere())
			{
				if(bdStruct != BD_Fence)
				{
					if(!bld->LifeSupportOkForVacuumLand())
						return 26;
				}
			}
		}

		switch(cTer)
		{
		case CONST_Coast:
			if(onPlatform)
				break;
			if(orbitZone == OZONE_Inferno)
				return 8;	// too hot
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 0);
				if(waterCoverage < .3f)
					return 20;	// too dry
			}
			break;

		case CONST_DryGround:
			if(!onPlatform)
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 0);
				if(waterCoverage > .1f)
				{
					//qDebug("DaWorldLL::TestBdLocation - Land: Under water.");
					return 19;	// Terrain is under water, perhaps only partially.
				}
			}
			break;

		case CONST_Well:
			if(!onPlatform)
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 0);
				if(waterCoverage > .1f)
				{
					//qDebug("DaWorldLL::TestBdLocation - Land: Under water.");
					return 19;	// Terrain is under water, perhaps only partially.
				}
			}
			break;

		case CONST_CoastLava:
		case CONST_Ground:
			break;

		case CONST_Rocks:
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(rocks < 1)
					return 18;	// The terrain has no rocks.
				if(waterCoverage > .25f)
					return 19;	// too wet
			}
			break;

		case CONST_SeaSurface:
			if(fabs(rAlt) > 1)
				return 21;	// Not +/- a foot of the surface.
			if(!ter.IsGasGiant() && orbitZone == OZONE_Inferno &&
					buildMaterial != CID_Vulcium && buildMaterial != CID_Adamantine)
				return 8;	// Lava is too hot for all but vulcium and adamantine.
			if(!ter.IsWater())
				return 20;	// too dry
			/* this is too slow
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(waterCoverage < .9f)
					return 20;	// too dry
			}
			*/
			break;

		case CONST_Trees:
			{
				unsigned int rocks, shrubs, trees;
				float waterCoverage;
				ResourceDistributionAt(mBld, LocOfLL(cenLL), siteBoxRadius, rocks, shrubs, trees, waterCoverage, 1);
				if(trees < 1)
					return 17;	// The terrain has no trees.
				if(waterCoverage > .25f)
					return 19;	// too wet
			}
			break;
		}
	}
	return 0;
}

int DaWorldLL::TestBdPreviewLocation(const AuBdReport* bld, const AuPoint& buildLoc, float buildRotZ, DaBd*& exDev) const
{
	exDev = 0;
	AuGlobeLLF buildLL;
	if(!LLOfLoc(buildLL, buildLoc))
	{
		qDebug() << "DaWorldLL::TestBdPreviewLocation" << Id() << Name() << "Not in terrain";
		return 1;
	}

	INDUSTRYID lIID;
	if(bld)
		lIID = bld->IndustryId();
	else
		lIID = IID_None;

	// Determine if this is a valid build location for the industry. Get build rules.
	// This switch was stripped down from the corresponding code in TestBdLocation.
	double rAlt = AltitudeMSL(buildLoc);
	CONSTRUCTION_TERRAIN cTer = CONST_DryGround;	// Construction terrain rules required by the industry.
	switch(lIID)
	{
	case IID_BaseFlag:
	case IID_Count:
	case IID_None:
	case IID_TownSquare:
		break;	//return 46;	// Invalid industry

	case IID_AerialPlatform:
	case IID_GroundPlatform:
		cTer = CONST_Ground;
		break;

	case IID_BaseDock:
	case IID_ShipYard:
	case IID_Wharf:
		cTer = CONST_Coast;
		break;

	case IID_Biodome:
	case IID_Bridge:
		cTer = CONST_Ground;
		break;

	case IID_DrillingRig:
	case IID_SeaPlatform:
		cTer = CONST_SeaSurface;
		break;

	case IID_Fence:
		cTer = CONST_Ground;
		break;

	case IID_HydroelectricPowerPlant:
		cTer = CONST_Coast;
		break;

	case IID_Landscape:
		cTer = CONST_Ground;
		break;

	case IID_LoggingCamp:
		cTer = CONST_Trees;
		break;

	case IID_Mine:
		cTer = CONST_Rocks;
		break;

	case IID_Road:
		cTer = CONST_Ground;
		break;

	case IID_StreetLight:
		cTer = CONST_Ground;
		break;

	case IID_TreatmentPlant:
		cTer = CONST_CoastLava;
		break;

	case IID_Tunnel:
		break;

	case IID_Well:
		break;

	case IID_AircraftFactory:
	case IID_AirportRadar:
	case IID_AirportRepairShop:
	case IID_AirportTerminal:
	case IID_Apartment:
	case IID_Arena:
	case IID_Armorer:
	case IID_Bakery:
	case IID_Bank:
	case IID_BaseAcademy:
	case IID_BaseAirTerminal:
	case IID_BaseBarracks:
	case IID_BaseGuardPost:
	case IID_BaseGuardTower:
	case IID_BaseHeadquarters:
	case IID_BaseMarineBarracks:
	case IID_BaseMotorPool:
	case IID_BaseRadar:
	case IID_BaseShield:
	case IID_BaseWeapon:
	case IID_Brewery:
	case IID_Broker:
	case IID_Cantina:
	case IID_Capitol:
	case IID_Carpenter:
	case IID_Casino:
	case IID_Church:
	case IID_ClothingFactory:
	case IID_CoalPowerPlant:
	case IID_ComputerFactory:
	case IID_Dairy:
	case IID_DesignStudio:
	case IID_Distillery:
	case IID_ElectronicsFactory:
	case IID_ExplosivesFactory:
	case IID_Farm:
	case IID_FireDepartment:
	case IID_GasolinePowerPlant:
	case IID_GristMill:
	case IID_Grocery:
	case IID_GunSmith:
	case IID_Hangar:
	case IID_Hospital:
	case IID_Hotel:
	case IID_House:
	case IID_HydrogenPowerPlant:
	case IID_Jeweler:
	case IID_LumberMill:
	case IID_MachineShop:
	case IID_MassMedia:
	case IID_MeatProcessingPlant:
	case IID_MunitionsFactory:
	case IID_NaturalGasPowerPlant:
	case IID_NuclearFuelPlant:
	case IID_NuclearPowerPlant:
	case IID_Observatory:
	case IID_Orchard:
	case IID_PaperMill:
	case IID_Park:
	case IID_PharmaceuticalLab:
	case IID_PlasticsFactory:
	case IID_PoliceAcademy:
	case IID_PoliceDepartment:
	case IID_Refinery:
	case IID_RetailStore:
	case IID_RocketMotorFactory:
	case IID_School:
	case IID_Shed:
	case IID_Smelter:
	case IID_SolarPowerPlant:
	case IID_SpacecraftFactory:
	case IID_StorageTank:
	case IID_SugarMill:
	case IID_TextileMill:
	case IID_VehicleFactory:
	case IID_Warehouse:
	case IID_WindPowerPlant:
	case IID_Winery:
	case IID_Zoo:
		// No special requirements. Can be built on a platform.
		break;
	}

	// All construction must occur within the atmosphere radius, even if there is no atmosphere.
	if(bld && (rAlt + bld->HullMax().z + 100*FeetPerMeter() > AtmosphereHeightMSL()))
		return 10;	// The terrain location is outside the atmosphere.

	// Calculate a matrix for orientation of the building.
	AuMatrix4x4 mBld;
	if(bld)
		BuildingConstructionTransform(buildLoc, buildRotZ, 0, bld->SiteCenter(), mBld);

	// Test at maximum possible radius.
	AuGlobeLLF cenLL;		// For forbidden zone test.
	double maxRadius;		// For forbidden zone test.
	AuPoint siteBoxRadius;
	if(bld)
	{
		// Calculate center of build site.
		AuPoint siteMin(bld->SiteMin());
		AuPoint siteMax(bld->SiteMax());
		if(siteMin == siteMax)
		{
			siteMin = bld->HullMin();
			siteMax = bld->HullMax();
		}
		AuPoint siteCen = mBld.Project((siteMin + siteMax)/2);
		LLOfLoc(cenLL, siteCen);
		siteBoxRadius = (siteMax - siteMin)/2;
		maxRadius = siteBoxRadius.Length_2D();
	}
	else
	{
		cenLL = buildLL;
		maxRadius = 10*FeetPerMeter();
		siteBoxRadius = AuPoint(maxRadius, maxRadius, 3*FeetPerMeter());	// Road doesn't need more than 3m for roof test.
	}

	// Test for zones that are off limits to constructions. See ringworld sections for examples of this, at ends and control center.
	if(IsForbiddenZone(cenLL, rAlt, maxRadius))
		return 11;

	// Check for existing development at build location.
	BUILDING_STRUCTURE bdStruct;
	if(bld)
	{
		bdStruct = bld->Structure();
		switch(bdStruct)
		{
		case BD_AerialPlatform:
			//DHH Ignore existing developments until we get more picky about existing aerial developments.
			break;
		case BD_SeaPlatform:
		case BD_SeaSurfaceBuilding:
			//DHH Ignore existing developments until we get more picky about existing sea surface developments.
			break;
		case BD_Fence:			// Fences ignore existing developments.
		case BD_Invalid:
		case BD_Count:
			break;
		case BD_Biodome:
		case BD_Bridge:
		case BD_Building:
		case BD_GroundPlatform:
		case BD_RoadEnclosure:
		case BD_SpaceElevator:
		case BD_Statue:
		case BD_Tunnel:
		case BD_StreetLight:
			exDev = FindBdContaining(*bld, mBld, FIND_SkipLandscapers);
			break;
		}
	}
	else
	{
		bdStruct = BD_Invalid;	// Not a building. Must be a road.
		exDev = FindBdContaining(buildLoc, FIND_SkipLandscapers);
	}

	// Get information about the terrain and resources at the construction site.
	AuTerrainInfo ter;
	QueryTerrain(ter, buildLL);
	ORBITZONE orbitZone = OrbitZone();

	// Test the planet location for terrain requirements of the particular construction process.
	if(ter.IsWater() && bdStruct != BD_AerialPlatform && cTer != CONST_SeaSurface)
	{
		// We are building under the sea.
		// Terrain is liquid and we're not building an aerial platform or sea surface structure. Those buildings are treated as if
		// exposed to atmosphere, not undersea environment.
		// We're not on a platform or the platform is on the ground, at the bottom of the sea. It can't stick out of the surface.
		// Prevent too tall buildings from sticking out of water.
		if(bld && (ter.WaterDepth() < bld->HullMax().z + 10*FeetPerMeter()))
			return 13;	// Water or supercooled liquid is not deep enough to sit on the bottom.

		// Armor and life support needed for underwater construction.
		if(ter.IsGasGiant())
		{
			if(bld && !bld->SkeletonOkForGasGiantCore() && !bld->ArmorOkForGasGiantCore())
				return 14;
			if(bld && !bld->LifeSupportOkForGasGiantCore())
				return 16;
		}
		else
		{
			if(orbitZone == OZONE_Inferno)
				return 8;	// Too hot. Cannot build submerged in lava.
			if(orbitZone == OZONE_Frigid)
			{
				if(bld && !bld->SkeletonOkForFrigidSeaBottom() && !bld->ArmorOkForFrigidSeaBottom())
					return 30;
				if(bld && !bld->LifeSupportOkForFrigidSeaBottom())
					return 31;
			}
			else
			{
				if(bld && !bld->SkeletonOkForSeaBottom() && !bld->ArmorOkForSeaBottom())
					return 15;
				if(bld && !bld->LifeSupportOkForSeaBottom())
					return 23;
			}
		}
	}
	else
	{
		// We are building on land, on the sea surface, or on a platform that is not submerged.
		// Check material can tolerate build environment.
		if(ter.IsGasGiant())
		{
			// Building an aerial or sea platform on a gas giant or a building on an aerial or sea platform on a gas giant.
			if(bld && !bld->LifeSupportOkForFrigidLand())
				return 25;
			if(bld && !bld->SkeletonOkForFrigidLand() && !bld->ArmorOkForFrigidLand())
				return 28;
		}
		else if(orbitZone == OZONE_Inferno)
		{
			if(bld && !bld->LifeSupportOkForInfernoLand())
				return 24;
			if(bld && !bld->SkeletonOkForInfernoLand() && !bld->ArmorOkForInfernoLand())
				return 27;
		}
		else if(orbitZone == OZONE_Frigid)
		{
			if(bld && !bld->LifeSupportOkForFrigidLand())
				return 25;
			if(bld && !bld->SkeletonOkForFrigidLand() && !bld->ArmorOkForFrigidLand())
				return 28;
		}
		else if(!HasBreathableAtmosphere())
		{
			if(bld && !bld->LifeSupportOkForVacuumLand())
				return 26;
		}
	}
	return 0;
}

//static
QString DaWorldLL::TestBdResultPixmap(int testBdResult)
{
	switch(testBdResult)
	{
	case 0:
		return QString("");
	case 1:		//  1 = build location not on this world
		return QString(":data/bldNotTerrain.png");
	case 2:		//  2 = Alien military HQ present
		return QString(":data/bldOccupiedHQ.png");
	case 3:		//  3 = Alien city jurisdiction present
		return QString(":data/bldForeignCity.png");
	case 4:		//  4 = Empire military HQ needed.
		return QString(":data/bldHQNeeded.png");
	case 5:		//  5 = Jurisdiction present
		return QString(":data/bldExistingCity.png");
	case 6:		//  6 = Empire city jurisdiction or empire capitol needed.
		return QString(":data/bldPoliceOrCapitolNeeded.png");
	case 7:		//  7 = Empire city jurisdiction needed.
		return QString(":data/bldPoliceNeeded.png");
	case 8:		//  8 = Too hot.
		return QString(":data/bldTooHot.png");
	case 9:		//  9 = too cold
		return QString(":data/bldTooCold.png");
	case 10:	// 10 = Building would stick out of atmosphere.
		return QString(":data/bldTooTall.png");
	case 11:	// 11 = forbidden zone
		return QString(":data/bldForbiddenZone.png");
	case 12:	// 12 = blocked by existing construction
		return QString(":data/bldBlocked.png");
	case 13:	// 13 = water is not deep enough
		return QString(":data/bldTooShallow.png");
	case 14:	// 14 = Insufficient armor for gas giant construction.
		return QString(":data/bldArmorGasGiant.png");
	case 15:	// 15 = Insufficient armor for under sea construction.
		return QString(":data/bldArmorSea.png");
	case 16:	// 16 = Building life support needed.
		return QString(":data/bldLifeSupport.png");
	case 17:	// 17 = no trees
		return QString(":data/bldNoTrees.png");
	case 18:	// 18 = no rocks
		return QString(":data/bldNoRocks.png");
	case 19:	// 19 = too wet
		return QString(":data/bldTooWet.png");
	case 20:	// 20 = too dry
		return QString(":data/bldTooDry.png");
	case 21:	// 21 = drilling rig must be within 1m of sea surface
		return QString(":data/bldSeaSurface.png");
	case 22:	// 22 = Alien military building present
		return QString(":data/bldOccupied.png");
	case 23:
		return QString(":data/bldLifeSupport.png");
	case 24:
		return QString(":data/bldLifeSupport.png");
	case 25:
		return QString(":data/bldLifeSupport.png");
	case 26:
		return QString(":data/bldLifeSupport.png");
	case 27:
		return QString(":data/bldArmorInferno.png");
	case 28:
		return QString(":data/bldArmorFrigid.png");
	case 29:
		return QString(":data/bldTooDeep.png");
	case 30:
		return QString(":data/bldArmorFrigid.png");
	case 31:
		return QString(":data/bldLifeSupport.png");
	case 32:
		return QString(":data/bldClaimNeeded.png");
	case 33:
		return QString(":data/bldForeignCity.png");
	case 34:
		return QString(":data/bldTooShallow.png");
	case 35:
		return QString(":data/bldEdgeOfTerrain.png");
	case 36:
		return QString(":data/bldTooSmall.png");
	case 37:
		return QString(":data/bldEdgeOfTerrain.png");
	case 38:
		return QString(":data/bldUnderAttack.png");
	case 39:							// 39 = Would create overlapping jurisdictions.
		return QString(":data/bldExistingCity.png");
	case 40:	// Too high above virgin terrain.
	case 41:	// Too low below virgin terrain.
	case 46:	// Invalid Industry	is truly unlikey to ever occur.
		return QString(":data/bldNotTerrain.png");
	case 42:	// Cannot be built on a platform.
		return QString(":data/bldNotOnPlatform.png");
	case 43:	// Cannot be built on aerial platform.
		return QString(":data/bldNotOnAerialPlatform.png");
	case 44:	// Cannot be built on ground platform.
		return QString(":data/bldNotOnGroundPlatform.png");
	case 45:	// Cannot be built on sea platform.
		return QString(":data/bldNotOnSeaPlatform.png");
	case 47:	// Building site falls off platform.
		return QString(":data/bldEdgeOfTerrain.png");
	case 48:	// Fence too long
		return QString(":data/bldFenceTooLong.png");
	case 49:	// 49 = Empire military HQ already present.
		return QString(":data/bldHQNeeded.png");
	default:
		return QString("");
	}
}

//static
QString DaWorldLL::TestBdResultText(int testBdResult)
{
	switch(testBdResult)
	{
	case 0:
		return tr("Location Ok");
	case 1:		//  1 = build location not on this world
		return tr("Location Not In Terrain Or Too Far Away");
	case 2:		//  2 = Alien military HQ present
		return tr("Foreign Military HQ");
	case 3:		//  3 = Alien jurisdiction present
		return tr("Foreign City Jurisdiction");
	case 4:		//  4 = Empire military HQ needed.
		return tr("Military HQ Needed");
	case 5:		//  5 = Jurisdiction present
		return tr("Existing City Jurisdiction");
	case 6:		//  6 = Empire police or empire capitol needed.
		return tr("Empire Police or Capitol Needed");
	case 7:		//  7 = Empire jurisdiction needed.
		return tr("Empire City Jurisdiction Needed");
	case 8:		//  8 = Too hot.
		return tr("Location Too Hot");
	case 9:		//  9 = too cold
		return tr("Location Too Cold");
	case 10:	// 10 = Building would stick out of atmosphere.
		return tr("Building Too Tall for Atmosphere");
	case 11:	// 11 = forbidden zone
		return tr("Blocked by Forbidden Construction Zone");
	case 12:	// 12 = blocked by existing construction
		return tr("Blocked By Development at Location");
	case 13:	// 13 = water is not deep enough
		return tr("Too Shallow for Undersea Construction");
	case 14:	// 14 = Insufficient framework for gas giant construction.
		return tr("Framework Inadequate for Gas Giant Sea Bottom");
	case 15:	// 15 = Insufficient framework for under sea construction.
		return tr("Framework Inadequate for Sea Bottom");
	case 16:	// 16 = Building life support needed.
		return tr("Life Support Inadequate for Gas Giant Sea Bottom");
	case 17:	// 17 = no trees
		return tr("Trees Needed at Location");
	case 18:	// 18 = no rocks
		return tr("Rocks Needed at Location");
	case 19:	// 19 = too wet
		return tr("Location Under Water");
	case 20:	// 20 = too dry
		return tr("Water Needed at Location");
	case 21:	// 21 = drilling rig must be within 1m of sea surface
		return tr("Must be Constructed at Sea Surface");
	case 22:	// 22 = Alien military building present
		return tr("Foreign Military Building Present");
	case 23:
		return tr("Life Support Inadequate for Sea Bottom");
	case 24:
		return tr("Life Support Inadequate for Inferno Conditions");
	case 25:
		return tr("Life Support Inadequate for Frigid Conditions");
	case 26:
		return tr("Life Support Inadequate for Airless Conditions");
	case 27:
		return tr("Framework Inadequate for Inferno Land");
	case 28:
		return tr("Framework Inadequate for Frigid Land");
	case 29:
		return tr("Too Deep for Drilling Rig to Touch Bottom");
	case 30:
		return tr("Framework Inadequate for Frigid Sea Bottom");
	case 31:
		return tr("Life Support Inadequate for Frigid Sea Bottom");
	case 32:
		return tr("World Has Not Been Claimed for Your Empire");
	case 33:
		return tr("World Claimed by Foreign Empire");
	case 34:
		return tr("Intersects sea surface.");
	case 35:
		return tr("Too Short");
	case 36:
		return tr("Building Not Completely Inside Enclosure");
	case 37:
		return tr("Too Steep");
	case 38:
		return tr("Enemy Unit in Construction Zone");
	case 39:
		return tr("Creates Overlapping City Jurisdictions");
	case 40:
		return tr("Too High Above Virgin Terrain");
	case 41:
		return tr("Too Low Below Virgin Terrain");
	case 42:
		return tr("Cannot be Built on Platform");
	case 43:
		return tr("Cannot be Built on Aerial Platform");
	case 44:
		return tr("Cannot be Built on Ground Platform");
	case 45:
		return tr("Cannot be Built on Sea Platform");
	case 46:
		return tr("Invalid Industry");
	case 47:
		return tr("Building Site Falls Off Platform");
	case 48:
		return tr("Fence/Wall Too Long");
	case 49:	// 49 = Empire military HQ already present.
		return tr("Military HQ Already Present");
	default:
		return tr("Unknown Reason %1").arg(testBdResult);
	}
}

void DaWorldLL::ResetWeaponRadarRanges()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		bd->ResetWeaponRadarRange();
	}
}

void DaWorldLL::BdAdded()
{
	m_BdBits |= 0x04;			// We know there is a bd here now.
	m_BdBits &= ~0x08;			// We know there is a bd here now.
	if(!(m_BdBits & 0x10))
		m_BdBits |= 0x20;		// There was no hq here so now it is unknown if an hq is here.
	if(!(m_BdBits & 0x40))
		m_BdBits |= 0x80;		// There was no base here so now it is unknown if a base is here.

	UpdateRoadMap();					// Recreate the road map when a building is added.
	ResetWeaponRadarRanges();
	FenceAdjust();						// Adjust fences to new terrain.
}

void DaWorldLL::BdRemoved(const AuId& bdId)
{
	CitizenNotAvailable(bdId);
	TroopNotAvailable(bdId);
	WorkersReleased(bdId, 0xffff);		// Release all employees.

	m_BdBits |= 0xa8;			// Turn on base unknown, bd unknown and hq unknown bits.

	UpdateRoadMap();					// Recreate the road map when a building is deleted.
	ResetWeaponRadarRanges();
	FenceAdjust();						// Adjust fences to new terrain.
	JurisdictionUpdate();				// Update all city jurisdictions.

	DaAtomList people;
	FindPeopleRecursive(people);
	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		DaPerson* person = (DaPerson*)*pi;
		if(person->SpawnCity() == bdId)
			person->ClearSpawnCity();
	}
}

void DaWorldLL::CreateClaimVAOs(AuGLEffects& re)
{
	quint32 claimedBy = ClaimedByEmpire();
	if(!claimedBy)
		return;
	if(m_ClaimPoleVAO.VAO() && m_ClaimFlagTexEmpire == claimedBy)
		return;

	// Get/create the texture image so we can calculate its aspect ratio.
	float imageAspect = 1.0;
	QImage imtex;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireFlag(claimedBy, imtex);
	g_Au.UnlockData();
	if(!imtex.isNull())
	{
		QString stid = QString("Flag:%1").arg(claimedBy);
		if(imtex.width() != 0 && imtex.height() != 0)
			imageAspect = (float)imtex.height()/(float)imtex.width();
		m_ClaimFlagTex = re.CreateTextureFromBitmap(Id().GetID(), stid, imtex, GL_LINEAR, GL_LINEAR, false, 1);
		m_ClaimFlagTexEmpire = claimedBy;
	}
	else
	{
		m_ClaimFlagTex = 0;
		m_ClaimFlagTexEmpire = 0;
		return;
	}

	// The pole is a simple spindle. This is drawn lying down so it will spin around the X axis.
	if(m_ClaimPoleVAO.VAO())
		m_ClaimPoleVAO.DeleteGL();
	const double poleRad = 2. * .083;
	AuPointArray profile;
	profile.append(AuPoint(-4, -poleRad));	// Start down in the ground.
	profile.append(AuPoint(0, -poleRad));
	profile.append(AuPoint(5, -poleRad));
	profile.append(AuPoint(10, -poleRad));
	profile.append(AuPoint(15, -poleRad));
	profile.append(AuPoint(20, -poleRad));
	profile.append(AuPoint(25, -poleRad));
	profile.append(AuPoint(25 + poleRad*1/2, -poleRad/2));
	profile.append(AuPoint(25 + poleRad*2/2, -poleRad/2));
	profile.append(AuPoint(25 + poleRad*3/2, -poleRad));
	profile.append(AuPoint(25 + poleRad*4/2, -poleRad));
	profile.append(AuPoint(25 + poleRad*5/2, -poleRad/2));
	profile.append(AuPoint(25 + poleRad*5/2, 0));
	AuScFaceList poleFaces;
	poleFaces.MakeSpindle(profile, 12, AuMatrix4x4());
	AuScSelectInfo sel;
	poleFaces.FaceIds(sel.m_Faces);
	poleFaces.VertexIds(sel.m_Vertices);
	AuMatrix4x4 texM;
	//texM.RotateZ(PI/2);
	poleFaces.TextureCylindrical(sel, 2, 1, 32, texM);	// Texture index 2 is flag pole.
	AuMatrix4x4 poleM;
	poleM.RotateY(-PI/2);
	poleFaces.Transform(poleM);

	// Flag dimensions and positions.
	double flagWidth;
	double flagHeight;
	if(imageAspect > 1)
	{
		flagHeight = 8;
		flagWidth = flagHeight / imageAspect;
	}
	else
	{
		flagWidth = 8;
		flagHeight = flagWidth * imageAspect;
	}
	double flagTop = 25 - poleRad;
	double flagBottom = flagTop - flagHeight;
	double flagPole = poleRad;
	double flagEdge = flagPole + flagWidth;
	AuScFaceList flagFaces;
	// Up facing side.
	AuScPoint ull(flagBottom,-flagPole,0, 0,0, AuPoint(0,0,1));
	AuScPoint uul(flagTop,-flagPole,0, 0,1, AuPoint(0,0,1));
	AuScPoint uur(flagTop,-flagEdge,0, 1,1, AuPoint(0,0,1));
	AuScPoint ulr(flagBottom,-flagEdge,0, 1,0, AuPoint(0,0,1));
	AuScFaceQuad ufq(ull, uul, uur, ulr);
	ufq.SetTextureIndex(3);
	flagFaces.AddFace(ufq);
	// Down facing side.
	AuScPoint dll(flagBottom,-flagPole,0, 0,0, AuPoint(0,0,-1));
	AuScPoint dul(flagTop,-flagPole,0, 0,1, AuPoint(0,0,-1));
	AuScPoint dur(flagTop,-flagEdge,0, 1,1, AuPoint(0,0,-1));
	AuScPoint dlr(flagBottom,-flagEdge,0, 1,0, AuPoint(0,0,-1));
	AuScFaceQuad dfq(dul, dll, dlr, dur);
	dfq.SetTextureIndex(3);
	flagFaces.AddFace(dfq);
	flagFaces.Transform(poleM);

	AuScTriRenderList opaqueTris, transparentTris;
	poleFaces.RenderTriangles(opaqueTris, transparentTris);
	flagFaces.RenderTriangles(opaqueTris, transparentTris);
	m_ClaimPoleVAO.CreateForSc(re, opaqueTris, GL_STATIC_DRAW, 0);
	if(!m_ClaimPoleTex)
		m_ClaimPoleTex = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClPole", ":data/MetalBrushed.png");
}

bool DaWorldLL::SetClaimedBy(const DaPerson& person, const AuGlobeLLF& atLL)
{
	bool personHasIndigenousDNA = false;
	if(!HasWorkerSpecies())
	{
		personHasIndigenousDNA = true;
		SetWorkerSpecies(person.Species());
	}
	else
	{
		if(WorkerSpecies() == person.Species())
			personHasIndigenousDNA = true;
	}
	m_LLLock.lockForWrite();
	m_ClaimAtLL = atLL;
	m_ClaimEmpire = person.EmpireId();
	m_ClaimPersonId = person.Id();
	m_ClaimPersonName = person.Name();
	m_ClaimTime.Set();
	m_LLLock.unlock();
	Capture(person.EmpireId(), person.Id(), person.Name());		// Capture all existing structures.
	SetModified();
	return personHasIndigenousDNA;
}

void DaWorldLL::WorkersReleased(const AuId& employerBdId, quint16 count)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		bd->UnemployResidents(employerBdId, count);
		if(!count)
			break;
	}
}

void DaWorldLL::CitizenAvailable(const AuId& bdId, quint32 empireId)
{
	m_LLLock.lockForWrite();
	m_CitizenSuppliers.AddSupplier(bdId, empireId);
	m_LLLock.unlock();
}

void DaWorldLL::CitizenNotAvailable(const AuId& bdId)
{
	m_LLLock.lockForWrite();
	m_CitizenSuppliers.RemoveSupplier(bdId);
	m_LLLock.unlock();
}

void DaWorldLL::CitizenSuppliers(quint32 importEmpireId, AuIdList& suppliers)
{
	m_LLLock.lockForRead();
	m_CitizenSuppliers.ListSuppliers(CID_Invalid, importEmpireId, suppliers);	// CID_Invalid = citizens
	m_LLLock.unlock();
}

void DaWorldLL::CitizenSupplierUpdate()
{
	// We will use a list of all bds so there are no locking issues.
	DaAtomList bds;
	FindBds(bds);
	m_LLLock.lockForWrite();
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->IsMilitary())
			continue;
		if(!bd->ResidentsUnemployed())
			continue;
		m_CitizenSuppliers.AddSupplier(bd->Id(), bd->EmpireId());
	}
	m_LLLock.unlock();
}

bool DaWorldLL::CitizenTakeNonessential(const AuId& toWorkAtBdId, QHash<AuId, int>& tradeConnections)
{
	if(tradeConnections.empty())
		return false;
	bool hasBreathableAtmo = HasBreathableAtmosphere();
	// Find a few buildings from which to take workers. We'll stop if we find an office worker.
	AuId takeOfficeWorkerFromBd;
	AuId takeFactoryWorkerFromBd;
	AuId takeWorkerFromBd;
	AuId takeOnlyWorkerFromBd;
	DaAtomList bds;
	FindBds(bds);
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->IsMilitary())
			continue;
		if(!tradeConnections.contains(bd->Id()))
			continue;	// Skip building that has no trade connection.
		if(bd->IsEssential(hasBreathableAtmo))
			continue;	// Skip essential buildings.
		if(bd->Is(toWorkAtBdId))
			continue;	// Skip the building that is looking.
		quint16 workers = bd->Workers();
		if(!workers)
			continue;	// No workers to take from there.
		quint32 manufacturingJobs = bd->ManufacturingJobs();
		if(workers > manufacturingJobs)
		{
			// We can take a person from there without impacting their operation.
			takeOfficeWorkerFromBd = bd->Id();
			break;	// We don't need to keep looking. This is what we really want.
		}
		else if(workers == manufacturingJobs && manufacturingJobs > 1)
		{
			// This building is full of workers and it has more than one manufacturing job.
			takeFactoryWorkerFromBd = bd->Id();
		}
		else if(workers > 1)
		{
			// This building has more than one worker.
			takeWorkerFromBd = bd->Id();
		}
		else
		{
			// This building has only one worker.
			takeOnlyWorkerFromBd = bd->Id();
		}
	}

	AuId takeFromBdId;
	if(!takeOfficeWorkerFromBd.IsNull())
		takeFromBdId = takeOfficeWorkerFromBd;
	else if(!takeFactoryWorkerFromBd.IsNull())
		takeFromBdId = takeFactoryWorkerFromBd;
	else if(!takeWorkerFromBd.IsNull())
		takeFromBdId = takeWorkerFromBd;
	else if(!takeOnlyWorkerFromBd.IsNull())
		takeFromBdId = takeOnlyWorkerFromBd;
	if(takeFromBdId.IsNull())
		return false;

	// We found a building from which to steal a worker.
	DaBd* bd = (DaBd*)FindIdRecursive(takeFromBdId);
	if(!bd)
		return false;
	if(!bd->TakeWorker())
		return false;	// Its worker was gone? Maybe somebody else grabbed it.

	// A worker was taken from bd. It's home must be notified.
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->ResidentChangedJobs(takeFromBdId, toWorkAtBdId))
			break;
	}
	return true;
}

bool DaWorldLL::TroopTakeNonessential(const AuId& toWorkAtBdId, QHash<AuId, int>& tradeConnections)
{
	if(tradeConnections.empty())
		return false;
	bool hasBreathableAtmo = HasBreathableAtmosphere();
	// Find a few buildings from which to take workers. We'll stop if we find an office worker.
	AuId takeOfficeWorkerFromBd;
	AuId takeFactoryWorkerFromBd;
	AuId takeWorkerFromBd;
	AuId takeOnlyWorkerFromBd;
	DaAtomList bds;
	FindBds(bds);
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(!bd->IsMilitary())
			continue;
		if(!tradeConnections.contains(bd->Id()))
			continue;	// Skip building that has no trade connection.
		if(bd->IsEssential(hasBreathableAtmo))
			continue;	// Skip essential buildings.
		if(bd->Is(toWorkAtBdId))
			continue;	// Skip the building that is looking.
		quint16 workers = bd->Workers();
		if(!workers)
			continue;	// No workers to take from there.
		quint32 jobs = bd->ServiceJobs();
		if(workers > jobs)
		{
			// We can take a person from there without impacting their operation.
			takeOfficeWorkerFromBd = bd->Id();
			break;	// We don't need to keep looking. This is what we really want.
		}
		else if(workers == jobs && jobs > 1)
		{
			// This building is full of workers and it has more than one manufacturing job.
			takeFactoryWorkerFromBd = bd->Id();
		}
		else if(workers > 1)
		{
			// This building has more than one worker.
			takeWorkerFromBd = bd->Id();
		}
		else
		{
			// This building has only one worker.
			takeOnlyWorkerFromBd = bd->Id();
		}
	}

	AuId takeFromBdId;
	if(!takeOfficeWorkerFromBd.IsNull())
		takeFromBdId = takeOfficeWorkerFromBd;
	else if(!takeFactoryWorkerFromBd.IsNull())
		takeFromBdId = takeFactoryWorkerFromBd;
	else if(!takeWorkerFromBd.IsNull())
		takeFromBdId = takeWorkerFromBd;
	else if(!takeOnlyWorkerFromBd.IsNull())
		takeFromBdId = takeOnlyWorkerFromBd;
	if(takeFromBdId.IsNull())
		return false;

	// We found a building from which to steal a worker.
	DaBd* bd = (DaBd*)FindIdRecursive(takeFromBdId);
	if(!bd)
		return false;
	if(!bd->TakeWorker())
		return false;	// Its worker was gone? Maybe somebody else grabbed it.

	// A worker was taken from bd. It's home must be notified.
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(bd->ResidentChangedJobs(takeFromBdId, toWorkAtBdId))
			break;
	}
	return true;
}

void DaWorldLL::TroopAvailable(const AuId& bdId, quint32 empireId)
{
	m_LLLock.lockForWrite();
	m_TroopSuppliers.AddSupplier(bdId, empireId);	// Troops are exported only to their own empire.
	m_LLLock.unlock();
}

void DaWorldLL::TroopNotAvailable(const AuId& bdId)
{
	m_LLLock.lockForWrite();
	m_TroopSuppliers.RemoveSupplier(bdId);
	m_LLLock.unlock();
}

void DaWorldLL::TroopSuppliers(quint32 importEmpireId, AuIdList& suppliers)
{
	m_LLLock.lockForRead();
	m_TroopSuppliers.ListSuppliersEmpire(importEmpireId, suppliers);	// Empire only.
	m_LLLock.unlock();
}

void DaWorldLL::TroopSupplierUpdate()
{
	// We will use a list of all bds so there are no locking issues.
	DaAtomList bds;
	FindBds(bds);
	m_LLLock.lockForWrite();
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		if(!bd->IsMilitary())
			continue;

		// Buildings with unemployed residents can supply them as troops.
		if(bd->ResidentsUnemployed())
			m_TroopSuppliers.AddSupplier(bd->Id(), bd->EmpireId());

		// Count troops assigned to the building. This is its initial worker count.
		DaAtomList soldiers;
		FindTroopsRecursive(bd->Id(), soldiers);
		bd->SetTroopsPresent((int)soldiers.size());
	}
	m_LLLock.unlock();
}

void DaWorldLL::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& nearLoc,
		const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor)
{
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	geof.RenderCursor(*this, projection, model, re, nearLoc, hitRoad, hitColor, roadColor);
	UnlockGeologicalFeatures();
}

void DaWorldLL::AddRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float width, quint8 geoType,
		bool completeConstruction)
{
	if(startLoc.FuzzyEquals(endLoc))
		return;
	AuGlobeLLF startLL;
	if(!LLOfLoc(startLL, startLoc))
		return;
	AuGlobeLLF endLL;
	if(!LLOfLoc(endLL, endLoc))
		return;
	double startMSL = AltitudeMSL(startLoc);
	double endMSL = AltitudeMSL(endLoc);
	/* If this test is needed, it should be made in TestBdLocation, which is called before calling this function.
	if(Hydrographics() > 0 &&
			((startMSL < 0 && endMSL > 0) ||
			 (startMSL > 0 && endMSL < 0)))
		return;
		*/
	quint32 paveMaterial;
	quint32 sitePrepLabor = SitePrepLabor(IID_Road, startLoc, endLoc, width, paveMaterial);
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	if(geoType == AuGeoRoadGrade::GeoType())
	{
		AuGeoRoadGrade* geo;
		if(completeConstruction)
			geo = new AuGeoRoadGrade(startLL, startMSL, endLL, endMSL, width, isSlab, 0);
		else
		{
			DeConstructionProcess cp(CPID_RoadGrade, sitePrepLabor, paveMaterial);
			geo = new AuGeoRoadGrade(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
		}
		geof.append(geo);
		m_tmGeologyChange.Set();
		m_GeoConstruction = 1;
	}
	else if(geoType == AuGeoRoadDirt::GeoType())
	{
		AuGeoRoadDirt* geo;
		if(completeConstruction)
			geo = new AuGeoRoadDirt(startLL, startMSL, endLL, endMSL, width, isSlab, 0);
		else
		{
			DeConstructionProcess cp(CPID_DirtRoad, sitePrepLabor, paveMaterial);
			geo = new AuGeoRoadDirt(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
		}
		geof.append(geo);
		m_tmGeologyChange.Set();
		m_GeoConstruction = 1;
		geo->CreateRoadMap(*this, *m_RoadMap);	// Add the road segment to the world's road map.
	}
	else if(geoType == AuGeoRoadAsphalt::GeoType())
	{
		AuGeoRoadAsphalt* geo;
		if(completeConstruction)
			geo = new AuGeoRoadAsphalt(startLL, startMSL, endLL, endMSL, width, isSlab, 0);
		else
		{
			DeConstructionProcess cp(CPID_AsphaltRoad, sitePrepLabor, paveMaterial);
			geo = new AuGeoRoadAsphalt(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
		}
		geof.append(geo);
		m_tmGeologyChange.Set();
		m_GeoConstruction = 1;
		geo->CreateRoadMap(*this, *m_RoadMap);	// Add the road segment to the world's road map.
	}
	else if(geoType == AuGeoRoadConcrete::GeoType())
	{
		AuGeoRoadConcrete* geo;
		if(completeConstruction)
			geo = new AuGeoRoadConcrete(startLL, startMSL, endLL, endMSL, width, isSlab, 0);
		else
		{
			DeConstructionProcess cp(CPID_ConcreteRoad, sitePrepLabor, paveMaterial);
			geo = new AuGeoRoadConcrete(startLL, startMSL, endLL, endMSL, width, isSlab, &cp);
		}
		geof.append(geo);
		m_tmGeologyChange.Set();
		m_GeoConstruction = 1;
		geo->CreateRoadMap(*this, *m_RoadMap);	// Add the road segment to the world's road map.
	}
	UnlockGeologicalFeatures();
	FenceAdjust();
	SetModified();
}

void DaWorldLL::AddLandscape(const AuPoint& startLoc, const AuPoint& endLoc, float width, bool hideRocks, int hideGrass,
		bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement,
		bool completeConstruction)
{
	if(startLoc.FuzzyEquals(endLoc))
		return;
	AuGlobeLLF startLL;
	if(!LLOfLoc(startLL, startLoc))
		return;
	AuGlobeLLF endLL;
	if(!LLOfLoc(endLL, endLoc))
		return;
	double startMSL = AltitudeMSL(startLoc);
	double endMSL = AltitudeMSL(endLoc);
	/* If this test is needed, it should be made in TestBdLocation, which is called before calling this function.
	if(Hydrographics() > 0 &&
			((startMSL < 0 && endMSL > 0) ||
			 (startMSL > 0 && endMSL < 0)))
		return;
		*/
	quint32 paveMaterial;
	quint32 sitePrepLabor = SitePrepLabor(IID_Landscape, startLoc, endLoc, width, paveMaterial);
	CONSTRUCTIONPROCESSID cpid;
	if(hideGrass == 1)
		cpid = CPID_Defoliant;
	else if(hideGrass == 2)
		cpid = CPID_Clearing;
	else if(hideGrass == 3)
		cpid = CPID_Irrigation;
	else
		cpid = CPID_Clearing;
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	AuGeoLandscape* geo;
	if(completeConstruction)
		geo = new AuGeoLandscape(startLL, startMSL, endLL, endMSL, width, hideRocks, hideGrass, hideShrubs, hideTrees, pavement, 0);
	else
	{
		DeConstructionProcess cp(cpid, sitePrepLabor, paveMaterial);
		geo = new AuGeoLandscape(startLL, startMSL, endLL, endMSL, width, hideRocks, hideGrass, hideShrubs, hideTrees, pavement,
				&cp);
	}
	geof.append(geo);
	m_tmGeologyChange.Set();
	m_GeoConstruction = 1;
	UnlockGeologicalFeatures();
	SetModified();
}

void DaWorldLL::AddBombCrater(const AuPoint& explodeLoc, float bottomMSL, float craterRadius, float debrisHeight)
{
	if(craterRadius < 2*FeetPerMeter())
		return;
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, explodeLoc))
		return;
	DeleteExcessBombCraters(49);
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	geof.AddBombCrater(*this, craterRadius, debrisHeight, bottomMSL, ll);
	m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();
	SetModified();
}

void DaWorldLL::DeleteExcessBombCraters(unsigned int maxCraters)
{
	// Check for excess craters.
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	unsigned int craters = geof.CountBombCraters();
	if(craters <= maxCraters)
	{
		UnlockGeologicalFeatures();
		return;
	}
	// Move excess craters into the deleted list so we can clean up subatoms afterwards.
	AuGeologicalFeatures deletedCraters;
	AuGeologicalFeatures::iterator gi = geof.begin();
	while(gi != geof.end())
	{
		if((*gi)->IsBombCrater())
		{
			// Move the crater to the deleted list.
			deletedCraters.append(*gi);
			gi = geof.erase(gi);
			craters--;
			if(craters <= maxCraters)
				break;
		}
		else
			gi++;
	}
	m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();
	SetModified();

	// Put subatoms in motion so they'll adjust to changes in ground elevation.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
	{
		if(!(*ait)->DoesQueryEnvironment())
			continue;
		if(!(*ait)->IsInAtmosphere())
			continue;
		if(!deletedCraters.FindDeletableContaining(*this, (*ait)->GlobeLL(), 10*FeetPerMeter()))
			continue;	// Object is outside the deleted features, by a 10m margin.
		(*ait)->SetInMotion();
		(*ait)->TickTock();
	}
	deletedCraters.Clear();
}

bool DaWorldLL::DeleteGeologicalFeature(const AuGlobeLLF& geoLL)
{
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	//AuGeologicalFeature* geo = geof.FindDeletableContaining(*this, ll, 0);
	const AuGeologicalFeature* geo = geof.FindDeletable(geoLL);
	AuGeologicalFeature* geoClone = 0;
	if(geo)
	{
		geoClone = geo->Clone();				// For atom cleanup afterwards.
		geof.DeleteGeo(geo);
		m_tmGeologyChange.Set();
	}
	UnlockGeologicalFeatures();
	if(!geoClone)
		return false;
	if(geoClone->IsUnderConstruction())
		InvalidateGeoConstruction();
	if(geoClone->IsIrrigation() || geoClone->IsDefoliant())
		UpdateIrrigationCache();
	if(geoClone->IsRoad())
	{
		UpdateRoadMap();	// Recreate the road map when a road is deleted.
		FenceAdjust();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
		if(system)
			system->UpdateTradeConnections();				// So people disconnect from me.
	}
	SetModified();

	// Put subatoms in motion so they'll adjust to changes in ground elevation when we delete the feature.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
	{
		if(!(*ait)->DoesQueryEnvironment())
			continue;
		if(!(*ait)->IsInAtmosphere())
			continue;
		if(!geoClone->IsDeletableContaining(*this, (*ait)->GlobeLL(), 10*FeetPerMeter()))
			continue;	// Object is outside the feature and the terrain transition zone.
		(*ait)->SetInMotion();
		(*ait)->TickTock();
	}
	delete geoClone;
	return true;
}

void DaWorldLL::RenderSc_Shader(AuGLSLScBase* shader, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, const DaPerson* sceneAtom)
{
	Q_UNUSED(shader);
	Q_UNUSED(sceneAtom);
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	geof.Render_Shader(shader, *this, projection, model, *ari.re);
	UnlockGeologicalFeatures();
}

void DaWorldLL::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaWorld::CreateDeviceObjects(ari);
	if(!m_AsphaltTex)
		m_AsphaltTex = CreateTerrainTexture(*ari.re, WORLDTEX_Tarmac, g_Au.GroundTextureSize());
	if(!m_ConcreteTex)
		m_ConcreteTex = CreateTerrainTexture(*ari.re, WORLDTEX_ConcreteHeavy, g_Au.GroundTextureSize());
	if(!s_TexConstruction)
		s_TexConstruction = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "CSign", ":data/ConstructionSign.png");
}

bool DaWorldLL::EnemyUnitBlocksConstruction(const AuMatrix4x4& mBld, const AuPoint& siteBoxRadius, quint32 empireId) const
{
	bool foundEnemy = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaUnit* unit = dynamic_cast<const DaUnit*>(*it);
		if(!unit)
			continue;
		if(!unit->IsInAtmosphere())
			continue;
		if(!unit->IsOnGround())
			continue;
		if(unit->EmpireId() == 0)
			continue;
		// Only certain types of units block construction.
		if( ! (dynamic_cast<const DaSentient*>(unit) ||
					dynamic_cast<const DaVehicle*>(unit) ||
					dynamic_cast<const DaVessel*>(unit)))
			continue;	// Indigenous and story characters do not block construction.
		if(PoliticalStance(unit->EmpireId(), empireId) != POL_Enemy && !unit->ContainsEnemyUnit(empireId))
			continue;
		if(unit->IsInvisibleOffLine())
			continue;
		if(unit->HasAgentPower())
			continue;
		// Check location of enemy unit.
		AuPoint eloc = mBld.UnProject(unit->Location());	// In the space of the building.
		if(eloc.x > fabs(siteBoxRadius.x) ||
				eloc.y > fabs(siteBoxRadius.y) ||
				eloc.z > fabs(siteBoxRadius.z))
			continue;	// Unit location is outside the site box extents. Check Z to skip units on the other side of the world.
		foundEnemy = true;	// Enemy unit is on the ground in the atmosphere.
		break;
	}
	return foundEnemy;
}

bool DaWorldLL::EnemyUnitBlocksDestruction(quint32 empireId) const
{
	bool foundEnemy = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaUnit* unit = dynamic_cast<const DaUnit*>(*it);
		if(!unit)
			continue;
		if(!unit->IsInAtmosphere())
			continue;
		if(!unit->IsOnGround())
			continue;
		if(unit->EmpireId() == 0)
			continue;
		// Only certain types of units block destruction.
		if( ! (dynamic_cast<const DaSentient*>(unit) ||
					dynamic_cast<const DaVehicle*>(unit) ||
					dynamic_cast<const DaVessel*>(unit)))
			continue;	// Indigenous and story characters do not block construction.
		if(PoliticalStance(unit->EmpireId(), empireId) != POL_Enemy && !unit->ContainsEnemyUnit(empireId))
			continue;
		if(unit->IsInvisibleOffLine())
			continue;
		if(unit->HasAgentPower())
			continue;
		foundEnemy = true;	// Enemy unit is on the ground in the atmosphere.
		break;
	}
	return foundEnemy;
}

INDUSTRYID DaWorldLL::FindGeoIndustryContaining(const AuPoint& loc, AuGlobeLLF& geoLL) const
{
	AuGlobeLLF ll;
	if(!LLOfLoc(ll, loc))
		return IID_None;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	INDUSTRYID geoiid = geof.FindGeoIndustryContaining(*this, ll, 0, geoLL);
	UnlockGeologicalFeatures();
	return geoiid;
}

bool DaWorldLL::GeoUnderConstruction(const AuGlobeLLF& geoLL, DeConstructionProcess& cp) const
{
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	bool foundGeo = geof.FindGeoUnderConstruction(geoLL, cp);
	UnlockGeologicalFeatures();
	return foundGeo;
}

void DaWorldLL::GeoConstructionEnable(const AuGlobeLLF& geoLL, bool enable)
{
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	bool didIt = geof.ConstructionEnable(geoLL, enable);
	if(didIt)
		m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();
	if(didIt)
		SetModified();
}

void DaWorldLL::GeoConstructionFetch(const AuGlobeLLF& geoLL)
{
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	bool didIt = geof.ConstructionFetch(geoLL, *this);
	if(didIt)
		m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();
	if(didIt)
		SetModified();
}

bool DaWorldLL::GeoConstructionGo(const AuGlobeLLF& geoLL, AuGear& gear)
{
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	int bitResult = geof.ConstructionGo(geoLL, gear);
	if(bitResult)
		m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();
	if(bitResult)
		SetModified();
	if(bitResult & 0x02)
	{
		// Construction completed.
		UpdateIrrigationCache();
		InvalidateGeoConstruction();
	}
	return bitResult & 0x04;	// b2=gear changed
}

void DaWorldLL::RoadConnections(const AuId& bdId, QSet<AuId>& connectedBdsAndCities) const
{
	LockGeologicalFeatures();
	m_RoadMap->Connections(bdId, connectedBdsAndCities);
	UnlockGeologicalFeatures();
}

void DaWorldLL::UpdateRoadMap()
{
	// Clear building map. It gets recreated on demand.
	m_BuildingMapLock.lock();
	m_BuildingMap.clear();
	m_BuildingMapLock.unlock();

	// Get road map information from geo roads.
	AuRoadMap roadMap;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	geof.CreateRoadMap(*this, roadMap);
	UnlockGeologicalFeatures();

	// Get road map information from DaBd atoms.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->CreateRoadMap(*this, roadMap);

	// Update my road map.
	LockGeologicalFeaturesForWrite();
	*m_RoadMap = roadMap;
	UnlockGeologicalFeatures();
	//qDebug() << "DaWorldLL::UpdateRoadMap" << Id() << Name();
	//roadMap.Dump();
}

void DaWorldLL::ConstructGeo()
{
	if(!HasGeoConstruction())
		return;
	if(WorldPopulation() == 0)
		return;
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	geof.ConstructionFetch(*this);
	m_GeoConstruction = geof.ConstructionGo() & 0x01 ? 1 : 0;	// b0=construction remains.
	m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();
	SetModified();
}

void DaWorldLL::JurisdictionPoliceJoin(DaBd* nuPolice)
{
	AuPoint loc = nuPolice->Location();
	DaAtomList subs;
	ListNormalSubatoms(subs);
	if(!subs.empty())
	{
		// Determine the jurisdiction of the nu police and all buildings that fall within its jurisdiction.
		double jr = nuPolice->JurisdictionRadius();
		DaAtomList bds;	// Buildings that are in the jurisdiction of the nu police.
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaBd* bd = dynamic_cast<DaBd*>(*it);
			if(!bd)
				continue;
			if(nuPolice == bd)
				continue;
			// This test matches the one in DaWorld::WouldCreateOverlappingJurisdictions.
			if(bd->IsCapitolInJurisdictionAt(*this, loc, jr))
				nuPolice->SetCapitol(bd->CapitolId(), bd->EmpireId());	// Found a capitol that is in my nu jurisdiction.
			else if(bd->HasJurisdictionAt(*this, loc))
				nuPolice->SetCapitol(bd->CapitolId(), bd->EmpireId());	// Found a bd whose jurisdiction contains the nuPolice.
			else if(bd->InProposedJurisdiction(*this, loc, jr))
				bds.append(bd);											// Found a civilian building that is in my nu jurisdiction.
		}

		// Add buildings in the new police jurisdiction to the city.
		AuId policeCity(nuPolice->CapitolId());
		if(!policeCity.IsNull())
		{
			for(DaAtomList::const_iterator ai = bds.begin(); ai != bds.end(); ai++)
			{
				DaBd* bd = (DaBd*)*ai;
				bd->SetCapitol(policeCity, nuPolice->EmpireId());
			}
		}
		else
			qWarning() << "error: DaWorldLL::JurisdictionPoliceJoin" << Id() << Name() << "nuPolice not in any city jurisdiction";
	}
}

void DaWorldLL::JurisdictionJoin(DaBd* nuBld)
{
	// Military buildings do not join cities. Power plants are considered military if they don't join any cities.
	if(!DbIndustry::IsPowerPlant(nuBld->IndustryId()) && nuBld->IsMilitary())
		return;
	AuPoint loc = nuBld->Location();
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(nuBld == bd)
			continue;
		if(bd->EmpireId() != nuBld->EmpireId())
			continue;
		if(bd->CapitolId().IsNull())
			continue;	// Police that is not part of a city does not assert a jurisdiction.
		if(bd->HasJurisdictionAt(*this, loc))
		{
			// Found a police station or capitol whose jurisdiction contains the nu building.
			nuBld->SetCapitol(bd->CapitolId(), nuBld->EmpireId());
			break;
		}
	}
}

void DaWorldLL::JurisdictionUpdate()
{
	DaAtomList capitols;
	DaAtomList police;
	DaAtomList bds;						// Civilian buildings.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(bd->IndustryId() == IID_Capitol)
			capitols.append(bd);	// City capitol.
		else if(bd->IndustryId() == IID_PoliceDepartment)
			police.append(bd);		// Police station.
		else if(DbIndustry::IsPowerPlant(bd->IndustryId()) || !bd->IsMilitary())
			bds.append(bd);			// Civilian building. Power plants are not considered military if they join any cities.
	}

	// Update jurisdictions of police that contains capitols.
	QHash<AuId, AuId> policeCaps;	// key is police, value is capitol
	QHash<AuId, quint32> policeEmps;	// key is police, value is empire
	for(DaAtomList::const_iterator ci = capitols.begin(); ci != capitols.end(); ci++)
	{
		DaBd* cap = (DaBd*)*ci;
		for(DaAtomList::const_iterator pi = police.begin(); pi != police.end(); pi++)
		{
			DaBd* pol = (DaBd*)*pi;
			if(cap->IsCapitolInJurisdictionAt(*this, pol->Location(), pol->JurisdictionRadius()))
			{
				// Found a police department whose jurisdiction includes the capitol.
				policeCaps.insert(pol->Id(), cap->Id());
				policeEmps.insert(pol->Id(), pol->EmpireId());
			}
		}
	}

	// Update jurisdictions of police that contain unassigned police.
	bool anyChange;
	do
	{
		anyChange = false;
		for(DaAtomList::const_iterator pi = police.begin(); pi != police.end(); pi++)
		{
			DaBd* pol = (DaBd*)*pi;
			if(!policeCaps.contains(pol->Id()))
				continue;	// The capitol of the police station is unknown.
			// Find police in the juris.
			for(DaAtomList::const_iterator spi = police.begin(); spi != police.end(); spi++)
			{
				DaBd* subpol = (DaBd*)*spi;
				if(subpol == pol)
					continue;
				if(policeCaps.contains(subpol->Id()))
					continue;	// The capitol of the police station is known.
				if(pol->HasJurisdictionAt(*this, subpol->Location()))
				{
					// Found a jurisdiction that contains the other police.
					policeCaps.insert(subpol->Id(), pol->CapitolId());
					policeEmps.insert(subpol->Id(), subpol->EmpireId());
					anyChange = true;
				}
			}
		}
	}
	while(anyChange);

	// Update city id of police stations.
	for(DaAtomList::const_iterator pi = police.begin(); pi != police.end(); pi++)
	{
		DaBd* pol = (DaBd*)*pi;
		QHash<AuId, AuId>::const_iterator ci = policeCaps.find(pol->Id());
		if(ci != policeCaps.cend())
			pol->SetCapitol(*ci, policeEmps[pol->Id()]);		// The capitol of the police station was found.
		else
		{
			quint32 empireId = ClaimedByEmpire();	// Default empire for buildings that are not in any jurisdiction.
			if(!empireId)
				empireId = pol->EmpireId();		// Unclaimed world so default empire is the current empire who owns the police.
			pol->SetCapitol(AuId(0), empireId);	// No capitol was found for the police station.
		}
	}

	// Update jurisdictions of civilian buildings, based on the capitol and police jurisdictions that contains them, if any.
	for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		AuId capitolId;
		quint32 empireId = ClaimedByEmpire();	// Default empire for buildings that are not in any jurisdiction.
		if(!empireId)
			empireId = bd->EmpireId();			// Unclaimed world so default empire is the current empire who owns the building.
		// Capitol buildings have a small jurisdiction, to enable creation of Mellette. i.e. no cops.
		for(DaAtomList::const_iterator pi = capitols.begin(); pi != capitols.end(); pi++)
		{
			DaBd* cap = (DaBd*)*pi;
			if(cap->HasJurisdictionAt(*this, bd->Location()))
			{
				// Found a capitol jurisdiction that contains the building.
				capitolId = cap->CapitolId();
				empireId = cap->EmpireId();
				break;
			}
		}
		if(capitolId.IsNull())
		{
			// Bd is not in a capitol jurisdiction. Check police.
			for(DaAtomList::const_iterator pi = police.begin(); pi != police.end(); pi++)
			{
				DaBd* pol = (DaBd*)*pi;
				if(pol->CapitolId().IsNull())
					continue;	// Police with no capitol do not assert a jurisdiction over buildings.
				if(pol->HasJurisdictionAt(*this, bd->Location()))
				{
					// Found a jurisdiction that contains the building.
					capitolId = pol->CapitolId();
					empireId = pol->EmpireId();
					break;
				}
			}
		}
		bd->SetCapitol(capitolId, empireId);	// capitolId will be null if I am in no jurisdiction.
	}
}

void DaWorldLL::SendBdFacilities()
{
	// Trigger all city capitol and base hq buildings to notify the master of their current facilities.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		bd->SendCityFacilities();
	}
}

AuIdList DaWorldLL::BuildingsAt(const AuGlobeLLF& ll) const
{
	m_BuildingMapLock.lock();
	AuIdList bds = *BuildingMapLocation(ll);
	m_BuildingMapLock.unlock();
	return bds;
}

QMap<AuGlobeLLF, AuIdList>::const_iterator DaWorldLL::BuildingMapLocation(const AuGlobeLLF& ll) const
{
	// Round the search location to a map location.
	// This rounding granularity gives us locations that are about 1000' apart.
	int eqdivs = GroundLevelEquatorDivisions();
	AuGlobeLLF mapLL = RoundedGlobeLoc(ll, eqdivs);

	// Find the list of buildings that share that map location.
	QMap<AuGlobeLLF, AuIdList>::const_iterator bmi = m_BuildingMap.constFind(mapLL);
	if(bmi == m_BuildingMap.cend())
	{
		// Create a building map entry at the location.
		// Make a list of all buildings that share that map location. This should be all buildings where any point in the site of
		// the building rounds to the rounded map location.
		m_BuildingMapLock.unlock();
		AuIdList buildingsAtMapLL;
		if(BdHere())
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(*it);
				if(!bd)
					continue;

				// Check insertion point of building, to see if it rounds to the mapLL.
				AuGlobeLLF bdll;
				if(LLOfLoc(bdll, bd->Location()))
				{
					if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
					{
						buildingsAtMapLL.append(bd->Id());
						continue;
					}
				}

				// Check site center point of building, to see if it rounds to the mapLL.
				AuMatrix4x4 bt(bd->Transform());
				AuPoint sc = bd->SiteCenter();
				if(LLOfLoc(bdll, bt.Project(sc)))
				{
					if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
					{
						buildingsAtMapLL.append(bd->Id());
						continue;
					}
				}

				// Check eight perimeter points of the site, to see if any of them rounds to the mapLL.
				AuPoint sr(bd->SiteHullBoxRadius());
				{
					// Check site corners.
					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(sr.x, sr.y))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(-sr.x, sr.y))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(-sr.x, -sr.y))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(sr.x, -sr.y))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					// Check site edge center points.
					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(sr.x, 0))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(0, sr.y))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(-sr.x, 0))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}

					if(LLOfLoc(bdll, bt.Project(sc + AuPoint(0, -sr.y))))
					{
						if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
						{
							buildingsAtMapLL.append(bd->Id());
							continue;
						}
					}
				}

				// Check grid of points over the site, to see if any of them rounds to the mapLL.
				int xsteps = (int)ceil(sr.x / d_SiteEdgeStep);
				int ysteps = (int)ceil(sr.y / d_SiteEdgeStep);
				if(xsteps && ysteps)
				{
					// We will iterate on one quadrant of the site extents, mirroring the point to the other three
					// quadrants as we go, so we check the whole thing out to the edges in all directions.
					bool onSite = false;
					double distPerXStep = sr.x / xsteps;
					double distPerYStep = sr.y / ysteps;
					for(int xstep=1; xstep<=xsteps && !onSite; xstep++)
					{
						double stepX = distPerXStep * xstep;
						for(int ystep=1; ystep<=ysteps; ystep++)
						{
							double stepY = distPerYStep * ystep;
							if(LLOfLoc(bdll, bt.Project(sc + AuPoint(stepX, stepY))))
							{
								if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
								{
									buildingsAtMapLL.append(bd->Id());
									onSite = true;
									break;
								}
							}
							if(LLOfLoc(bdll, bt.Project(sc + AuPoint(-stepX, stepY))))
							{
								if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
								{
									buildingsAtMapLL.append(bd->Id());
									onSite = true;
									break;
								}
							}
							if(LLOfLoc(bdll, bt.Project(sc + AuPoint(-stepX, -stepY))))
							{
								if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
								{
									buildingsAtMapLL.append(bd->Id());
									onSite = true;
									break;
								}
							}
							if(LLOfLoc(bdll, bt.Project(sc + AuPoint(stepX, -stepY))))
							{
								if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
								{
									buildingsAtMapLL.append(bd->Id());
									onSite = true;
									break;
								}
							}
						}
					}
					if(onSite)
						continue;
				}

				// Check fence segments.
				if(bd->IndustryId() == IID_Fence)
				{
					// Get fence segments in the space of the building. We don't care about style.
					AuSegmentArray fenceSegs;
					if(bd->FenceSegments(fenceSegs) && !fenceSegs.empty())
					{
						bool fenceFound = false;
						for(AuSegmentArray::const_iterator si = fenceSegs.cbegin(); si != fenceSegs.cend(); si++)
						{
							if(LLOfLoc(bdll, bt.Project((*si).m_fLoc1)))
							{
								if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
								{
									buildingsAtMapLL.append(bd->Id());
									fenceFound = true;
									break;
								}
							}
							if(LLOfLoc(bdll, bt.Project((*si).m_fLoc2)))
							{
								if(RoundedGlobeLoc(bdll, eqdivs) == mapLL)
								{
									buildingsAtMapLL.append(bd->Id());
									fenceFound = true;
									break;
								}
							}
						}
						if(fenceFound)
							continue;
					}
				}
			}
		}
		m_BuildingMapLock.lock();
		bmi = m_BuildingMap.insert(mapLL, buildingsAtMapLL);
	}
	return bmi;
}

quint64 DaWorldLL::MemoryUsage() const
{
	quint64 mem = DaWorld::MemoryUsage();
	mem += sizeof(DaWorldLL) - sizeof(DaWorld);
	mem += StringMemoryUsage(m_CaptName);
	mem += m_CitizenSuppliers.MemoryUsage();
	mem += StringMemoryUsage(m_ClaimPersonName);
	mem += m_GeologicalFeatures.MemoryUsage();
	if(m_RoadMap)
		mem += m_RoadMap->MemoryUsage();
	mem += m_TroopSuppliers.MemoryUsage();

	for(QMap<AuGlobeLLF, AuIdList>::const_iterator bi = m_BuildingMap.cbegin(); bi != m_BuildingMap.cend(); bi++)
		mem += sizeof(AuGlobeLLF) + sizeof(AuId) * (*bi).size();

	if(m_Enclosures)
		mem += sizeof(AuWorldEnclosure) * m_Enclosures->size();
	if(m_Irrigation)
		mem += sizeof(AuWorldIrrigation) * m_Irrigation->size();

	if(m_LandingSites)
		mem += sizeof(AuPoint) * ResourceZones();

	mem += (sizeof(AuId) + sizeof(int)) * m_TroopSuppliers.MemoryUsage();

	if(m_Village)
		mem += sizeof(AuVillage);

	mem += (sizeof(int) + sizeof(AuGLVAO)) * m_SpriteVAO.capacity();
	mem += (sizeof(AuPoint) + sizeof(AuBldSprite)) * m_OverlayBuildings.size();
	mem += (sizeof(AuPoint) + sizeof(AuRoadSprite)) * m_OverlayRoads.size();
	mem += (sizeof(COMMODITYID) + sizeof(AuPoint)) * m_OverlayRocks.capacity();
	mem += (sizeof(COMMODITYID) + sizeof(AuPoint)) * m_OverlayShrubs.capacity();
	mem += (sizeof(COMMODITYID) + sizeof(AuPoint)) * m_OverlayTrees.capacity();
	mem += (sizeof(AuResKey) + sizeof(AuGlobeRock)) * m_Rocks.size();
	mem += (sizeof(AuResKey) + sizeof(AuGlobeFlora)) * m_Shrubs.size();
	mem += (sizeof(AuResKey) + sizeof(AuGlobeFlora)) * m_AquaticShrubs.size();
	mem += (sizeof(AuResKey) + sizeof(AuGlobeTree)) * m_Trees.size();
	return mem;
}

bool DaWorldLL::FindSeaLevelLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp,
		const AuMatrix4x4& tDeviceOrtho, const AuMatrix4x4& tModel, AuPoint& locTerrain) const
{
	// Create two parallel planes in scene atom space.
	// Project ground location and plane vectors into scene atom space.
	AuPoint planeOrg0 = tDeviceOrtho.UnProject(AuPoint(0,0,-1000));				// Plane origin, in scene atom space.
	AuPoint planeOrg1 = tDeviceOrtho.UnProject(AuPoint(0,0,-100000));			// Another plane origin, in scene atom space.
	AuPoint vxS = tDeviceOrtho.UnProject(AuPoint(100000,0,-1000)) - planeOrg0;
	vxS.Unit();
	AuPoint vyS = tDeviceOrtho.UnProject(AuPoint(0,100000,-1000)) - planeOrg0;
	vyS.Unit();
	AuPoint planeRot = AuPoint::CalculateRotation(vxS, vyS);

	AuMatrix4x4 tPlane0;
	tPlane0.Translate(planeOrg0);
	tPlane0.Rotate(planeRot);

	AuMatrix4x4 tPlane1;
	tPlane1.Translate(planeOrg1);
	tPlane1.Rotate(planeRot);

	// Determine scene atom space location of fpxFind, where it intersects each plane.
	// These two points will be used as a sighting segment for SightTargetDetails.
	AuPoint fpxFind(pxFind);
	AuPoint seg0;
	tDevicePersp.UnProjectIntoPlane(seg0, fpxFind, tPlane0);
	AuPoint seg1;
	tDevicePersp.UnProjectIntoPlane(seg1, fpxFind, tPlane1);

	AuPoint sightVec = seg1 - seg0;
	seg1 = seg0 + sightVec;

	AuPoint sightedLoc = seg1;
	if(!SightSeaSurface(seg0, tModel, sightedLoc))
		return false;

	AuPoint fpxLoc = tDeviceOrtho.Project(sightedLoc);
	if(fpxLoc.z < 0)
	{
		// The scene atom-space plane intersect point is in front of the scene atom.
		locTerrain = tModel.UnProject(sightedLoc);			// Convert scene atom-space location to be relative to this world.
		return true;
	}
	return false;
}

// Search for a terrain location that contains pxFind.
// 'tModel' already includes 'this' atom's transform.
bool DaWorldLL::FindTerrainLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp,
		const AuMatrix4x4& tDeviceOrtho, const AuMatrix4x4& tModel, AuPoint& locTerrain) const
{
	// Create two parallel planes in scene atom space.
	// Project ground location and plane vectors into scene atom space.
	AuPoint planeOrg0 = tDeviceOrtho.UnProject(AuPoint(0,0,-1000));				// Plane origin, in scene atom space.
	AuPoint planeOrg1 = tDeviceOrtho.UnProject(AuPoint(0,0,-100000));			// Another plane origin, in scene atom space.
	AuPoint vxS = tDeviceOrtho.UnProject(AuPoint(100000,0,-1000)) - planeOrg0;
	vxS.Unit();
	AuPoint vyS = tDeviceOrtho.UnProject(AuPoint(0,100000,-1000)) - planeOrg0;
	vyS.Unit();
	AuPoint planeRot = AuPoint::CalculateRotation(vxS, vyS);

	AuMatrix4x4 tPlane0;
	tPlane0.Translate(planeOrg0);
	tPlane0.Rotate(planeRot);

	AuMatrix4x4 tPlane1;
	tPlane1.Translate(planeOrg1);
	tPlane1.Rotate(planeRot);

	// Determine scene atom space location of fpxFind, where it intersects each plane.
	// These two points will be used as a sighting segment for SightTargetDetails.
	AuPoint fpxFind(pxFind);
	AuPoint seg0;
	tDevicePersp.UnProjectIntoPlane(seg0, fpxFind, tPlane0);
	AuPoint seg1;
	tDevicePersp.UnProjectIntoPlane(seg1, fpxFind, tPlane1);

	AuPoint sightVec = seg1 - seg0;
	//sightVec.Unit(g_Au.DetailedTerrainRangeFt());
	sightVec.Unit(1000*FeetPerMeter() + g_Au.DetailedTerrainRangeFt());
	seg1 = seg0 + sightVec;

	// Now I can use sighting logic to spot the exact point that is hit on my terrain.
	AuPoint sightedLoc;
	if(IsCachedFindTerrainAtPixel(seg0, seg1))
	{
		if(!CachedFindTerrainAtPixel(sightedLoc))
			return false;
	}
	else
	{
		unsigned int sightedPart = 0;
		bool sightedShield = false;
		sightedLoc = seg1;
		if(!SightTargetDetails(seg0, tModel, sightedLoc, sightedPart, sightedShield))
		{
			SetCachedFindTerrainAtPixel(seg0, seg1, false, sightedLoc);
			return false;
		}
		SetCachedFindTerrainAtPixel(seg0, seg1, true, sightedLoc);
	}

	AuPoint fpxLoc = tDeviceOrtho.Project(sightedLoc);
	if(fpxLoc.z < 0)
	{
		// The scene atom-space plane intersect point is in front of the scene atom.
		locTerrain = tModel.UnProject(sightedLoc);			// Convert scene atom-space location to be relative to this world.
		return true;
	}
	return false;
}

bool DaWorldLL::IsCachedFindTerrainAtPixel(const AuPoint& eyeLoc, const AuPoint& sightedLoc) const
{
	if(!g_Au.IsSceneApp())
		return false;	// Sight target cache is only useful on the client.
	m_LLLock.lockForRead();
	bool v = eyeLoc.FuzzyEquals(m_FindTerrainAtPixelEye) && sightedLoc.FuzzyEquals(m_FindTerrainAtPixelLook);
	m_LLLock.unlock();
	return v;
}

bool DaWorldLL::CachedFindTerrainAtPixel(AuPoint& sightedLoc) const
{
	m_LLLock.lockForRead();
	bool detailsOk = m_FindTerrainAtPixelDetailsOk;
	sightedLoc = m_FindTerrainAtPixelResult;
	m_LLLock.unlock();
	return detailsOk;
}

void DaWorldLL::SetCachedFindTerrainAtPixel(const AuPoint& eyeLoc, const AuPoint& sightLook, bool detailsOk,
		const AuPoint& sightedLoc) const
{
	if(!g_Au.IsSceneApp())
		return;	// Sight target cache is only useful on the client.
	m_LLLock.lockForWrite();
	m_FindTerrainAtPixelDetailsOk = detailsOk;
	m_FindTerrainAtPixelResult = sightedLoc;
	m_FindTerrainAtPixelEye = eyeLoc;
	m_FindTerrainAtPixelLook = sightLook;
	m_LLLock.unlock();
}

bool DaWorldLL::IsCachedSightTarget(const AuPoint& eyeLoc, const AuPoint& sightedLoc) const
{
	if(!g_Au.IsSceneApp())
		return false;	// Sight target cache is only useful on the client.
	m_LLLock.lockForRead();
	bool v = eyeLoc.FuzzyEquals(m_SightEye) && sightedLoc.FuzzyEquals(m_SightLook);
	m_LLLock.unlock();
	return v;
}

int DaWorldLL::CachedSightTarget(bool& detailsOk, AuPoint& sightedLoc) const
{
	m_LLLock.lockForRead();
	int v = m_SightIntersect;
	detailsOk = m_SightDetailsOk;
	sightedLoc = m_SightedLoc;
	m_LLLock.unlock();
	return v;
}

void DaWorldLL::SetCachedSightTarget(const AuPoint& eyeLoc, const AuPoint& sightLook, int isect, bool detailsOk,
		const AuPoint& sightedLoc) const
{
	if(!g_Au.IsSceneApp())
		return;	// Sight target cache is only useful on the client.
	m_LLLock.lockForWrite();
	m_SightDetailsOk = detailsOk;
	m_SightedLoc = sightedLoc;
	m_SightEye = eyeLoc;
	m_SightIntersect = isect;
	m_SightLook = sightLook;
	m_LLLock.unlock();
}

void DaWorldLL::SurrenderClaim(quint32 empireId)
{
	// Get information about the emperor of 'empireId'.
	AuId emperorId;
	QString emperorName;
	const DgData& gd = g_Au.LockDataForRead();
	if(!gd.PirateEmperor(empireId, emperorId, emperorName))
	{
		// Empire is not a pirate empire.
		emperorId = gd.EmpireEmperor(empireId);
		emperorName = gd.EmpireCitizenNameWithTitle(empireId, emperorId);
	}
	g_Au.UnlockData();

	m_LLLock.lockForWrite();
	m_ClaimEmpire = empireId;
	m_ClaimPersonId = emperorId;
	m_ClaimPersonName = emperorName;
	m_ClaimTime.Set();
	m_LLLock.unlock();
	SetModified();
}

void DaWorldLL::Capture(quint32 empireId, const AuId& capturedById, const QString& capturedByName)
{
	// Capture the claim. In practice, I must already be claimed by 'empireId' before they can build a military base here.
	if(EmpireId() != empireId)
	{
		m_LLLock.lockForWrite();
		m_ClaimEmpire = empireId;
		m_ClaimPersonId = capturedById;
		m_ClaimPersonName = capturedByName;
		m_ClaimTime.Set();
		m_LLLock.unlock();
		SetModified();
	}

	// Change empire id of all buildings, nu vessels, and nu vehicles.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->WorldCaptured(empireId);
}

void DaWorldLL::EmpireDeleted(quint32 empireId)
{
	if(ClaimedByEmpire() != empireId)
		return;
	// Military buildings, capitols and police will destroy themselves so it is ok to release the claim.
	m_LLLock.lockForWrite();
	m_ClaimAtLL = AuGlobeLLF();
	m_ClaimEmpire = 0;
	m_ClaimPersonId = AuId(0);
	m_ClaimPersonName = "";
	m_ClaimTime.Set();
	m_LLLock.unlock();
	SetModified();
}

DaWorldLL* DaWorldLL::FindParentWorldInEnv()
{
	return this;
}

const DaWorldLL* DaWorldLL::FindParentWorldInEnv() const
{
	return this;
}

bool DaWorldLL::IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho)
{
	// World does not block path if both locations are outside the atmosphere.
	double nalt = InAtmosphereCGAltitude(nloc);
	if(nalt == -10000 && InAtmosphereCGAltitude(oloc) == -10000)
		return false;

	const DaMissile* missile = dynamic_cast<const DaMissile*>(toWho);
	if(BdHere())
	{
		// Large missiles and rockets do not hit buildings in the city that launches them.
		DaAtomList bds;
		AuPoint bhitloc, bhitnorm;
		FindBdsContaining(oloc, FIND_BuildingsOnly, bds);
		for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
		{
			if(missile && missile->IsArmedBy((*bi)->Id()))
				continue;
			AuPoint boloc = (*bi)->Transform().UnProject(oloc);	// Into the space of the building.
			AuPoint bnloc = (*bi)->Transform().UnProject(nloc);	// Into the space of the building.
			if((*bi)->IsPathBlocked(boloc, bnloc, bhitloc, bhitnorm, toWho))
			{
				hitLoc = (*bi)->Transform().Project(bhitloc);
				hitNormal = (*bi)->Transform().ProjectVector(bhitnorm);
				return true;
			}
		}

		bds.clear();
		FindBdsContaining(nloc, FIND_BuildingsOnly, bds);
		for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
		{
			if(missile && missile->IsArmedBy((*bi)->Id()))
				continue;
			AuPoint boloc = (*bi)->Transform().UnProject(oloc);	// Into the space of the building.
			AuPoint bnloc = (*bi)->Transform().UnProject(nloc);	// Into the space of the building.
			if((*bi)->IsPathBlocked(boloc, bnloc, bhitloc, bhitnorm, toWho))
			{
				hitLoc = (*bi)->Transform().Project(bhitloc);
				hitNormal = (*bi)->Transform().ProjectVector(bhitnorm);
				return true;
			}
		}
	}

	//DHH DaWorld::IsPathBlocked Check collision with trees.
	return false;
}

ON_GROUND DaWorldLL::GeoGround(const AuGlobeLLF& geoLL) const
{
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	ON_GROUND ground = geof.Ground(geoLL);
	UnlockGeologicalFeatures();
	return ground;
}

void DaWorldLL::RenderSmoke(const AuGlobeLLF& smokeLL, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& planetLoc, float size, const QPointF& wind, float ambientR, float ambientG, float ambientB)
{
	if(!s_TexSmoke)
		s_TexSmoke = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SmokeT", ":data/smoketransparent.png");

	AuSmokeParticles* smoke = m_PSSmoke[smokeLL];
	if(!smoke)
	{
		smoke = new AuSmokeParticles();
		smoke->SetMaxParticles(1000);
		smoke->SetLifeCycle(10);
		smoke->SetVelocityVar(3);	// about 2mph (1.4667 fps = 1mph)
		smoke->SetNumToRelease(1);
		smoke->SetReleaseInterval(0.25f);
		smoke->SetSize(size*100);
		smoke->SetTextureID(s_TexSmoke);
		smoke->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		smoke->Init(re);
		m_PSSmoke.insert(smokeLL, smoke);
	}

	AuPoint w(wind.x(), wind.y());
	float windDir = w.ArcTangent_2D();		// Wind direction in radians
	float windForce = w.Length();			// Miles per hour
	windForce *= 5280.f;					// Convert to feet per hour
	windForce /= (60.f * 60.f);				// Convert to feet per sec

	AuPoint vnorth(0,0,1);
	AuPoint vup(-GravityDir(planetLoc));
	AuPoint veast = vnorth.Cross(vup);
	veast.Unit(windForce);					// Wind vec pointing east.
	float f = size/2;
	veast += vup * f;						// Give it some lift.
	AuMatrix4x4 tWind;
	if(windDir != 0)
		tWind.RotateAxis(windDir, vup);
	AuPoint vwind = tWind.Project(veast);
	smoke->SetVelocity(vwind);

	QColor colSmoke;
	colSmoke.setRgbF(ambientR, ambientG, ambientB);
	smoke->SetColor(colSmoke);

	AuMatrix4x4 smokeM(model);
	smokeM.Translate(planetLoc);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * smokeM);
	re.GLSL().SpriteSizeShader().SetTexture(s_TexSmoke);
	re.GLSL().SpriteSizeShader().SetColor(smoke->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(smoke->GetSize()*10);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if(smoke->Update())
		smoke->Render(projection, smokeM, re);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void DaWorldLL::PirateBaseEncounter()
{
	// Double check some things to be sure we don't put two bases on the same world.
	if(BdHere() ||
			BaseHere() ||
			HQHere())
		return;	// Skip world with any cities, bases or buildings.
	if(FindVessel())
		return;	// Skip world with any spacecraft in the atmosphere or in orbit.
	if(FindPersonOnLine())
		return;	// Skip world with any person who is on line.
	DaSolarSystem* system = FindParentSolarSystemInEnv();
	if(!system)
		return;
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;
	const DaGalaxy* galaxy = sector->FindParentGalaxy();
	if(!galaxy)
		return;

	AuId emperorId;
	QString emperorName;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 empireId = gd.RandomPirateEmpire(Galaxy());
	AuSpecies species = gd.EmpireCapitolSpecies(empireId);
	AuBdPref prefhq = gd.EmpirePreferredBlueprints(empireId, IID_BaseHeadquarters);
	gd.PirateEmperor(empireId, emperorId, emperorName);
	int rank = gd.PirateEmpireRank(empireId);
	g_Au.UnlockData();
	if(!empireId)
	{
		qWarning() << "warning: DaWorldLL::PirateBaseEncounter" << *this << "encounter aborted, pirate empire" << empireId;
		return;
	}

	// Claim the world.
	AuGlobeLLF claimLL = RandomLandLLNotForbidden(g_Au.Dice(), FeetPerMeter(), 1000*FeetPerMeter());
	if(claimLL == AuGlobeLLF())
	{
		qWarning() << "warning: DaWorldLL::PirateBaseEncounter" << *this << "RandomLandLLNotForbidden returned 0,0";
		return;
	}
	SetClaim(empireId, claimLL, emperorId, emperorName, AuTime::Now().UTC());

	// Cleanup old craters, roads, landscaping, etc.
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	if(geof.DeleteDeletableFeatures())
		m_tmGeologyChange.Set();
	UnlockGeologicalFeatures();

	// Build some basic roads centered on the claim flag, distinctive of each empire.
	if(PirateBaseRoads(empireId, rank, claimLL))
		system->UpdateTradeConnections();						// So we can import people and supplies.

	// Base Headquarters
	AuGlobeLLF baseLL = RandomLandLLNear(g_Au.Dice(), claimLL, FeetPerMeter(), 3*FeetPerMeter(), 10*FeetPerMeter());
	COMMODITYID material = CID_Stone;
	float zRotation = g_Au.Dice().Roll(1,360,-1) * DEG2RADf;
	DaBd* bd = PirateBaseBuilding(IID_BaseHeadquarters, material, baseLL, zRotation, empireId, species);
	if(!bd)
		return;
	bd->SetFounder(emperorId, emperorName);
	Capture(empireId, emperorId, emperorName);	// Military headquarters captures all cities on the world.

	QString cityName(g_Au.Dice().RandomCityName());
	bd->SetCityName(cityName);

	// Set worker species.
	if(!HasWorkerSpecies())
		SetWorkerSpecies(species);

	SetModified();
	system->UpdateTradeConnections();	// So we can import construction materials.

	// Solar system is considered to be explored by the builder of a city. I guess their populace looks up once in a while.
	system->ExploredBy(empireId);

	QString systemName;
	if(system->HasName())
		systemName = system->Name();	// System already has a name so don't change it.
	else
	{
		// New city in unnamed system sets the name of the system.
		system->SetNameIncludePlanets(cityName, true);
		systemName = cityName;
	}

	// Collect information for the notification to the master.
	AuPoint systemLoc;
	system->GetLocationRelativeToParent(systemLoc, galaxy);
	QString sectorName;
	if(sector->HasName())
		sectorName = sector->Name();	// Sector already has a name so don't change it.
	else
		sectorName = cityName;			// New city in unnamed sector sets the name of the sector.

	// Notify the master server to add a city to my empire record.
	emit CityCreated(empireId, CITY_MilitaryBase, bd->Id(), cityName, AuId(0), QString(), Id(), Name(), system->Id(), systemName,
			systemLoc, sector->Id(), sectorName, galaxy->Id(), galaxy->Galaxy());
}

DaBd* DaWorldLL::PirateBaseBuilding(INDUSTRYID industryId, COMMODITYID material, const AuGlobeLLF& atLL, float zRotation,
		quint32 empireId, const AuSpecies& residentSpecies)
{
	const DgData& gd = g_Au.LockDataForRead();
	AuBdPref prefbd = gd.EmpirePreferredBlueprints(empireId, industryId);
	int rank = gd.PirateEmpireRank(empireId);
	g_Au.UnlockData();
	if(prefbd.Blueprints().empty())
		return 0;	// wut?
	quint8 quality = rank * 16;
	quint32 designId = prefbd.Blueprints().first().BlueprintId();	//DHH Only one pirate building per industry for all environs.
	DaBd* building = CreateBuilding(designId, quality, industryId, material, atLL, zRotation, empireId, residentSpecies);
	if(!building)
		return 0;

	building->SetupPirate(empireId, quality, this);
	//qDebug() << "DaWorldLL::PirateBaseBuilding" << *this << "constructed building" << *building;
	if(industryId != IID_BaseHeadquarters && building->IsIsolated())
	{
		// Connect the building by road to the claim point.
		AuPoint buildLoc = building->Location();
		double rwid = 10*FeetPerMeter();				// Road width.
		double wdist = rwid / 2;						// 1/2 width of road.
		AuGlobeLLF claimLL = ClaimedAtLL();
		AuPoint claimLoc = LocOfLL(claimLL);
		AuPoint roadVec = claimLoc - buildLoc;
		double dist = roadVec.Length();
		roadVec.Unit(dist - wdist);						// Shorten the road so it doesn't terminate at the claim point.
		AuPoint startLoc = GroundLoc(buildLoc + roadVec);
		if(!PirateBaseRoad(empireId, startLoc, buildLoc, false, rwid))
		{
			qDebug() << "DaWorldLL::PirateBaseBuilding" << *this << "destroying" << *building
				<< "won't interconnect and can't build a road";
			building->Kill();
			return 0;
		}
	}

	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
	if(system)
		system->UpdateTradeConnections();				// So we can import people and supplies.
	return building;
}

bool DaWorldLL::PirateBaseRoads(quint32 empireId, int rank, const AuGlobeLLF& baseLL)
{
	AuPoint baseLoc = LocOfLL(baseLL);
	double terAltMSL = TerrainAltitudeMSL(baseLL);
	ElevateLocMSL(baseLoc, terAltMSL);
	AuPoint vN, vE, vUp;
	if(!CompassVectors(baseLoc, vN, vE, vUp))
		return false;

	switch(rank)
	{
	case 1:	// One big slab across the claim flag.
		{
			double rwid = 20*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);
		}
		break;

	case 2:	// Y of roads.
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.
			double ca = cos(30*DEG2RAD);
			double sa = sin(30*DEG2RAD);

			AuPoint startLoc = baseLoc;
			AuPoint endLoc = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			AuGlobeLLF eLL;
			endLoc = GroundLoc(baseLoc + vE * (rdist * ca) - vN * (rdist * sa));
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			endLoc = GroundLoc(baseLoc - vE * (rdist * ca) - vN * (rdist * sa));
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);
		}
		break;

	case 3:	// Cross of roads.
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			AuGlobeLLF eLL;
			startLoc = baseLoc + vE * wdist;
			endLoc = GroundLoc(baseLoc + vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc - vE * wdist;
			endLoc = GroundLoc(baseLoc - vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc + vN * wdist;
			endLoc = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc - vN * wdist;
			endLoc = GroundLoc(baseLoc - vN * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);
		}
		break;

	case 4:	// Grid of roads
	case 10:
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			AuGlobeLLF eLL;
			startLoc = baseLoc + vE * wdist;
			AuPoint eloc = GroundLoc(baseLoc + vE * rdist);
			PirateBaseRoad(empireId, startLoc, eloc, false, rwid);

			startLoc = baseLoc - vE * wdist;
			AuPoint wloc = GroundLoc(baseLoc - vE * rdist);
			PirateBaseRoad(empireId, startLoc, wloc, false, rwid);

			startLoc = baseLoc + vN * wdist;
			AuPoint nloc = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, nloc, false, rwid);

			startLoc = baseLoc - vN * wdist;
			AuPoint sloc = GroundLoc(baseLoc - vN * rdist);
			PirateBaseRoad(empireId, startLoc, sloc, false, rwid);

			AuPoint neloc = GroundLoc(eloc + vN * wdist);
			PirateBaseRoad(empireId, neloc, nloc, false, rwid);
			PirateBaseRoad(empireId, neloc, eloc, false, rwid);

			AuPoint seloc = GroundLoc(eloc - vN * wdist);
			PirateBaseRoad(empireId, seloc, sloc, false, rwid);
			PirateBaseRoad(empireId, seloc, eloc, false, rwid);

			AuPoint nwloc = GroundLoc(wloc + vN * wdist);
			PirateBaseRoad(empireId, nwloc, nloc, false, rwid);
			PirateBaseRoad(empireId, nwloc, wloc, false, rwid);

			AuPoint swloc = GroundLoc(wloc - vN * wdist);
			PirateBaseRoad(empireId, swloc, sloc, false, rwid);
			PirateBaseRoad(empireId, swloc, wloc, false, rwid);
		}
		break;

	case 5:	// Cross of roads with Y branches.
	case 11:
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.
			double ydist = 10*FeetPerMeter() * (rank/5);
			double yrdist = rdist + ydist;

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			startLoc = baseLoc + vE * wdist;
			AuPoint eloc = GroundLoc(baseLoc + vE * rdist);
			PirateBaseRoad(empireId, startLoc, eloc, false, rwid);

			startLoc = baseLoc - vE * wdist;
			AuPoint wloc = GroundLoc(baseLoc - vE * rdist);
			PirateBaseRoad(empireId, startLoc, wloc, false, rwid);

			startLoc = baseLoc + vN * wdist;
			AuPoint nloc = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, nloc, false, rwid);

			startLoc = baseLoc - vN * wdist;
			AuPoint sloc = GroundLoc(baseLoc - vN * rdist);
			PirateBaseRoad(empireId, startLoc, sloc, false, rwid);

			// East Y
			endLoc = GroundLoc(baseLoc + vE * yrdist + vN * ydist);
			PirateBaseRoad(empireId, eloc, endLoc, false, rwid);
			endLoc = GroundLoc(baseLoc + vE * yrdist - vN * ydist);
			PirateBaseRoad(empireId, eloc, endLoc, false, rwid);

			// West Y
			endLoc = GroundLoc(baseLoc - vE * yrdist + vN * ydist);
			PirateBaseRoad(empireId, wloc, endLoc, false, rwid);
			endLoc = GroundLoc(baseLoc - vE * yrdist - vN * ydist);
			PirateBaseRoad(empireId, wloc, endLoc, false, rwid);

			// North Y
			endLoc = GroundLoc(baseLoc + vN * yrdist + vE * ydist);
			PirateBaseRoad(empireId, nloc, endLoc, false, rwid);
			endLoc = GroundLoc(baseLoc + vN * yrdist - vE * ydist);
			PirateBaseRoad(empireId, nloc, endLoc, false, rwid);

			// South Y
			endLoc = GroundLoc(baseLoc - vN * yrdist + vE * ydist);
			PirateBaseRoad(empireId, sloc, endLoc, false, rwid);
			endLoc = GroundLoc(baseLoc - vN * yrdist - vE * ydist);
			PirateBaseRoad(empireId, sloc, endLoc, false, rwid);
		}
		break;

	case 6:	// 8 spoke radial.
	case 12:
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			AuGlobeLLF eLL;
			startLoc = baseLoc + vE * wdist;
			endLoc = GroundLoc(baseLoc + vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc - vE * wdist;
			endLoc = GroundLoc(baseLoc - vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc + vN * wdist;
			endLoc = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc - vN * wdist;
			endLoc = GroundLoc(baseLoc - vN * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc - vN * 5*FeetPerMeter() + vE * 5*FeetPerMeter();
			endLoc = GroundLoc(baseLoc - vN * rdist + vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc - vN * 5*FeetPerMeter() - vE * 5*FeetPerMeter();
			endLoc = GroundLoc(baseLoc - vN * rdist - vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc + vN * 5*FeetPerMeter() + vE * 5*FeetPerMeter();
			endLoc = GroundLoc(baseLoc + vN * rdist + vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);

			startLoc = baseLoc + vN * 5*FeetPerMeter() - vE * 5*FeetPerMeter();
			endLoc = GroundLoc(baseLoc + vN * rdist - vE * rdist);
			PirateBaseRoad(empireId, startLoc, endLoc, false, rwid);
		}
		break;

	case 7:	// Y of roads, with interconnects.
	case 14:
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.
			double idist = rdist / 2;						// Interconnect distance.
			double ca = cos(30*DEG2RAD);
			double sa = sin(30*DEG2RAD);

			AuPoint startLoc = baseLoc;
			AuPoint iloc1 = GroundLoc(baseLoc + vN * idist);
			AuPoint rloc1 = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, iloc1, true, rwid);
			PirateBaseRoad(empireId, iloc1, rloc1, true, rwid);

			AuGlobeLLF eLL;
			AuPoint iloc2 = GroundLoc(baseLoc + vE * (idist * ca) - vN * (idist * sa));
			AuPoint rloc2 = GroundLoc(baseLoc + vE * (rdist * ca) - vN * (rdist * sa));
			PirateBaseRoad(empireId, startLoc, iloc2, false, rwid);
			PirateBaseRoad(empireId, iloc2, rloc2, false, rwid);

			AuPoint iloc3 = GroundLoc(baseLoc - vE * (idist * ca) - vN * (idist * sa));
			AuPoint rloc3 = GroundLoc(baseLoc - vE * (rdist * ca) - vN * (rdist * sa));
			PirateBaseRoad(empireId, startLoc, iloc3, false, rwid);
			PirateBaseRoad(empireId, iloc3, rloc3, false, rwid);

			PirateBaseRoad(empireId, iloc1, iloc2, false, rwid);
			PirateBaseRoad(empireId, iloc2, iloc3, false, rwid);
			PirateBaseRoad(empireId, iloc3, iloc1, false, rwid);
		}
		break;

	case 8:	// Grid of roads with outriggers.
	case 13:
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.
			double odist = 50*FeetPerMeter() * rank;		// Outrigger distance.
			double rdist = odist * .75;						// Grid of roads distance.

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			AuGlobeLLF eLL;
			startLoc = baseLoc + vE * wdist;
			AuPoint eloc = GroundLoc(baseLoc + vE * rdist);
			AuPoint eoloc = GroundLoc(baseLoc + vE * odist);
			PirateBaseRoad(empireId, startLoc, eloc, false, rwid);

			startLoc = baseLoc - vE * wdist;
			AuPoint wloc = GroundLoc(baseLoc - vE * rdist);
			AuPoint woloc = GroundLoc(baseLoc - vE * odist);
			PirateBaseRoad(empireId, startLoc, wloc, false, rwid);

			startLoc = baseLoc + vN * wdist;
			AuPoint nloc = GroundLoc(baseLoc + vN * rdist);
			AuPoint noloc = GroundLoc(baseLoc + vN * odist);
			PirateBaseRoad(empireId, startLoc, nloc, false, rwid);

			startLoc = baseLoc - vN * wdist;
			AuPoint sloc = GroundLoc(baseLoc - vN * rdist);
			AuPoint soloc = GroundLoc(baseLoc - vN * odist);
			PirateBaseRoad(empireId, startLoc, sloc, false, rwid);

			AuPoint neloc = GroundLoc(eloc + vN * wdist);
			PirateBaseRoad(empireId, neloc, nloc, false, rwid);
			PirateBaseRoad(empireId, neloc, eloc, false, rwid);

			AuPoint seloc = GroundLoc(eloc - vN * wdist);
			PirateBaseRoad(empireId, seloc, sloc, false, rwid);
			PirateBaseRoad(empireId, seloc, eloc, false, rwid);

			AuPoint nwloc = GroundLoc(wloc + vN * wdist);
			PirateBaseRoad(empireId, nwloc, nloc, false, rwid);
			PirateBaseRoad(empireId, nwloc, wloc, false, rwid);

			AuPoint swloc = GroundLoc(wloc - vN * wdist);
			PirateBaseRoad(empireId, swloc, sloc, false, rwid);
			PirateBaseRoad(empireId, swloc, wloc, false, rwid);

			PirateBaseRoad(empireId, eloc, eoloc, false, rwid);
			PirateBaseRoad(empireId, wloc, woloc, false, rwid);
			PirateBaseRoad(empireId, nloc, noloc, false, rwid);
			PirateBaseRoad(empireId, sloc, soloc, false, rwid);
		}
		break;

	case 9:	// 8 spoke radial, with interconnects.
	case 15:
		{
			double rwid = 10*FeetPerMeter();				// Road width.
			double wdist = rwid / 2;						// 1/2 width of road.
			double rdist = 50*FeetPerMeter() * rank;		// Rank distance.
			double idist = rdist / 2;						// Interconnect distance.

			AuPoint startLoc = baseLoc + vE * wdist;
			AuPoint endLoc = baseLoc - vE * wdist;
			PirateBaseRoad(empireId, startLoc, endLoc, true, rwid);

			AuGlobeLLF eLL;
			startLoc = baseLoc + vE * wdist;
			AuPoint eloc = GroundLoc(baseLoc + vE * idist);
			AuPoint reloc = GroundLoc(baseLoc + vE * rdist);
			PirateBaseRoad(empireId, startLoc, eloc, false, rwid);
			PirateBaseRoad(empireId, eloc, reloc, false, rwid);

			startLoc = baseLoc - vE * wdist;
			AuPoint wloc = GroundLoc(baseLoc - vE * idist);
			AuPoint rwloc = GroundLoc(baseLoc - vE * rdist);
			PirateBaseRoad(empireId, startLoc, wloc, false, rwid);
			PirateBaseRoad(empireId, wloc, rwloc, false, rwid);

			startLoc = baseLoc + vN * wdist;
			AuPoint nloc = GroundLoc(baseLoc + vN * idist);
			AuPoint rnloc = GroundLoc(baseLoc + vN * rdist);
			PirateBaseRoad(empireId, startLoc, nloc, false, rwid);
			PirateBaseRoad(empireId, nloc, rnloc, false, rwid);

			startLoc = baseLoc - vN * wdist;
			AuPoint sloc = GroundLoc(baseLoc - vN * idist);
			AuPoint rsloc = GroundLoc(baseLoc - vN * rdist);
			PirateBaseRoad(empireId, startLoc, sloc, false, rwid);
			PirateBaseRoad(empireId, sloc, rsloc, false, rwid);

			startLoc = baseLoc - vN * 5*FeetPerMeter() + vE * 5*FeetPerMeter();
			AuPoint seloc = GroundLoc(baseLoc - vN * idist + vE * idist);
			AuPoint rseloc = GroundLoc(baseLoc - vN * rdist + vE * rdist);
			PirateBaseRoad(empireId, startLoc, seloc, false, rwid);
			PirateBaseRoad(empireId, seloc, rseloc, false, rwid);

			startLoc = baseLoc - vN * 5*FeetPerMeter() - vE * 5*FeetPerMeter();
			AuPoint swloc = GroundLoc(baseLoc - vN * idist - vE * idist);
			AuPoint rswloc = GroundLoc(baseLoc - vN * rdist - vE * rdist);
			PirateBaseRoad(empireId, startLoc, swloc, false, rwid);
			PirateBaseRoad(empireId, swloc, rswloc, false, rwid);

			startLoc = baseLoc + vN * 5*FeetPerMeter() + vE * 5*FeetPerMeter();
			AuPoint neloc = GroundLoc(baseLoc + vN * idist + vE * idist);
			AuPoint rneloc = GroundLoc(baseLoc + vN * rdist + vE * rdist);
			PirateBaseRoad(empireId, startLoc, neloc, false, rwid);
			PirateBaseRoad(empireId, neloc, rneloc, false, rwid);

			startLoc = baseLoc + vN * 5*FeetPerMeter() - vE * 5*FeetPerMeter();
			AuPoint nwloc = GroundLoc(baseLoc + vN * idist - vE * idist);
			AuPoint rnwloc = GroundLoc(baseLoc + vN * rdist - vE * rdist);
			PirateBaseRoad(empireId, startLoc, nwloc, false, rwid);
			PirateBaseRoad(empireId, nwloc, rnwloc, false, rwid);

			// Interconnects
			PirateBaseRoad(empireId, eloc, neloc, false, rwid);
			PirateBaseRoad(empireId, neloc, nloc, false, rwid);
			PirateBaseRoad(empireId, nloc, nwloc, false, rwid);
			PirateBaseRoad(empireId, nwloc, wloc, false, rwid);
			PirateBaseRoad(empireId, wloc, swloc, false, rwid);
			PirateBaseRoad(empireId, swloc, sloc, false, rwid);
			PirateBaseRoad(empireId, sloc, seloc, false, rwid);
			PirateBaseRoad(empireId, seloc, eloc, false, rwid);
		}
		break;
	}
	return true;
}

bool DaWorldLL::PirateBaseRoad(quint32 empireId, const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, double width)
{
	COMMODITYID material = CID_Stone;
	DaBd* exDev;
	QString cityName;
	int testResult = TestBdLocation(IID_Road, empireId, 0, material, startLoc, width, 0, 0, exDev, &endLoc, cityName);
	if(testResult != 0 && width > 2*FeetPerMeter())
	{
		// Full width road segment failed. Try again, much narrower.
		width = 2*FeetPerMeter();
		testResult = TestBdLocation(IID_Road, empireId, 0, material, startLoc, width, 0, 0, exDev, &endLoc, cityName);
	}
	if(testResult != 0)
		return false;
	AddRoad(startLoc, endLoc, isSlab, width, AuGeoRoadConcrete::GeoType(), true);
	return true;
}

AuPoint DaWorldLL::GroundLoc(const AuPoint& loc) const
{
	AuGlobeLLF ll;
	LLOfLoc(ll, loc);
	double terMSL = TerrainAltitudeMSL(ll);
	AuPoint gloc(loc);
	ElevateLocMSL(gloc, terMSL);
	return gloc;
}

DaBd* DaWorldLL::CreateBuilding(quint32 designId, quint8 quality, INDUSTRYID industryId, COMMODITYID material,
		const AuGlobeLLF& atLL, float zRotation, quint32 empireId, const AuSpecies& residentSpecies)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	AuPoint buildLoc = LocOfLL(atLL);
	double terAltMSL = TerrainAltitudeMSL(atLL);
	ElevateLocMSL(buildLoc, terAltMSL);

	AuBdDesign design;
	const DgData& bdd = g_Au.LockBdDesignsForRead();
	bool hasDesign = bdd.BdDesign(designId, design);
	g_Au.UnlockBdDesigns();
	if(!hasDesign)
	{
		// This is an error. We are supposed to insure the pirate's preferred designs are already loaded.
		qWarning() << "warning: DaWorldLL::CreateBuilding" << *this << "designId" << designId << "bd design not loaded"
			<< designId;
		return 0;												// Server was triggered to load the design.
	}

	AuBdReport report;
	design.ReportDesign(report);
	report.SetConstructionMaterial(material);
	report.SetQuality(quality);
	DaBd* exDev;
	QString cityName;
	int testResult = TestBdLocation(industryId, empireId, &report, material, buildLoc, 0, zRotation, 0, exDev, 0, cityName);
	if(testResult != 0)
	{
		//qDebug() << "  DaWorldLL::CreateBuilding" << *this << "FAIL" << TestBdResultText(testResult);
		return 0;												// Build location bad or blocked.
	}

	AuMatrix4x4 mBld;
	BuildingConstructionTransform(buildLoc, zRotation, 0, report.SiteCenter(), mBld);

	DaBd* building = new DaBd;
	building->ConnectSignals();
	building->Create();
	building->SetPosAndOrient(AuAtomPos(Id(), mBld));
	ether->AddAtom(building);

	building->InitPreview(report, report.DesignName(), empireId, residentSpecies);
	building->SetIndustry(industryId);
	g_Au.AtomAdded(building->Id());
	building->SetNormal();										// New atom is ready for normal thread processing.

	BdAdded();													// Update city presence.
	if(industryId == IID_PoliceDepartment)
		JurisdictionPoliceJoin(building);						// This could change city jurisdictions and claim buildings.
	else
		JurisdictionJoin(building);								// Join a city if the nu building falls in its jurisdiction.
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
	if(system)
		system->UpdateTradeConnections();						// So we can import people and supplies.

	building->TickTock();
	return building;
}

// Adds hangared spacecraft to the list that are subordinate to this world. Not recursive.
void DaWorldLL::FindHangaredSpaceships(DaAtomList& spaceships) const
{
	DaAtomList bds;
	FindBds(bds);
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*bi);
		bd->FindHangaredSpaceships(spaceships);
	}
}

// Uses enclosure cache to return the enclosure that contains 'loc'.
AuId DaWorldLL::EnclosureContaining(const AuPoint& worldLoc) const
{
	if(!HasEnclosure())
		return AuId();	// I have no Enclosures.

	AuId insideEnclosure;
	m_EnclosureLock.lockForRead();
	if(m_Enclosures)
		insideEnclosure = m_Enclosures->ContainsId(worldLoc);
	m_EnclosureLock.unlock();
	return insideEnclosure;
}

void DaWorldLL::BuildingConstructionTransform(const AuPoint& buildLoc, double zRotation, const DaBd* onPlatform,
		const AuPoint& siteCen, AuMatrix4x4& mBld) const
{
	if(onPlatform)
	{
		// Initial placement matrix. Building site is perpendicular to the gravity normal at platform loc.
		AuPoint vUp;
		vUp = -GravityDir(onPlatform->Location());
		AuMatrix4x4 mAlign = AuPoint(0,0,1).GetAlignmentTransform(vUp);
		mBld.Translate(buildLoc);
		mBld *= mAlign;
		mBld.RotateZ(zRotation);
	}
	else
	{
		// Initial placement matrix. Building site is perpendicular to the gravity normal at build loc.
		AuPoint vUp;
		vUp = -GravityDir(buildLoc);
		AuMatrix4x4 mAlign = AuPoint(0,0,1).GetAlignmentTransform(vUp);
		mBld.Translate(buildLoc);
		mBld *= mAlign;
		mBld.RotateZ(zRotation);
		if(siteCen.x != 0 || siteCen.y != 0)
		{
			// Site center is not above/below the origin.
			// Calculate matrix for building so the gravity normal is up at the center of the site.
			AuPoint b = mBld.Project(siteCen.x * 2, siteCen.y * 2, 0);
			AuPoint bUp = -GravityDir(b);
			bUp.Unit();
			double ca = vUp.AngleTo(bUp);
			double ba = PI/2 - ca;					// Reduced from 'double ba = PI - PI/2 - ca;'
			double ia = (PI/2 + ba)/2;				// Isoscelese triangle base angles.
			double ta = PI/2 - ia;					// Tilt angle needed to align site center with the gravity normal.
			//qDebug() << "DaWorldLL::BuildingConstructionTransform" << *this << "tilt" << (ta*RAD2DEG) << "degrees";

			// Calculate tilt axis in the space of the building design.
			AuPoint ty(siteCen.x, siteCen.y);		// 2D vector to site center from origin.
			ty.Unit();
			AuPoint tx = ty.RotateZ(-PI/2);			// Axis on which to tilt.
			mBld.LoadIdentity();
			mBld.Translate(buildLoc);
			mBld *= mAlign;
			mBld.RotateZ(zRotation);
			mBld.RotateAxis(-ta, tx.x, tx.y, tx.z);

			// On a spherical world, the center of the site is now at a lower MSL altitude than buildLoc. At near sea level
			// altitudes, large sites like platforms clip off the ball, leaving a large roundish lake where the building belongs.
			// The opposite might be needed on ringworlds if the site clips into the sea to the east or west.
			double altMSL = AltitudeMSL(buildLoc);
			AuPoint sc = mBld.Project(siteCen.x, siteCen.y, 0);	// Current site center, at the site's z=0, relative to the world.
			double smsl = AltitudeMSL(sc);
			if(smsl < altMSL)
				mBld.Translate(0,0,altMSL - smsl);				// Lift site so center is at placement msl.
		}
	}
}

double DaWorldLL::BuildingConstructionNorthAngle(const AuPoint& loc, const DaBd* onPlatform) const
{
	// Calculate north angle at build location.
	AuPoint vUp;
	if(onPlatform)
		vUp = -GravityDir(onPlatform->Location());
	else
		vUp = -GravityDir(loc);
	AuMatrix4x4 mAlign = AuPoint(0,0,1).GetAlignmentTransform(vUp);

	// Calculate north angle at build location.
	AuPoint rvec = mAlign.ProjectVector(1,0,0);	// Vector pointing to right. (0,1,0) is the front.
	AuPoint vX;
	vUp.ArbitraryAxis(vX);						// Returns a unit vector.
	AuPoint vY = vUp.Cross(vX);
	vY.Unit();
	double x = rvec.Dot(vX);
	double y = rvec.Dot(vY);
	return atan2(y,x);							// Angle in the plane of vUp to face the building to the north.
}

// Search for a terrain location that contains pxFind.
// 'tModel' already includes 'this' atom's transform.
DaBd* DaWorldLL::FindPlatformLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp,
		const AuMatrix4x4& tDeviceOrtho, const AuMatrix4x4& tModel, AuPoint& platformLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		AuPoint bloc;
		if(!bd->FindPlatformLocationAtPixel(pxFind, tDevicePersp, tDeviceOrtho, tModel * bd->Transform(), bloc))
			continue;
		platformLoc = bd->Transform().Project(bloc);
		return bd;
	}
	return 0;
}

void DaWorldLL::FindBdsOnPlatform(const AuId& platformId, DaAtomList& bds) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(bd->PlatformId() != platformId)
			continue;
		if(bd->Is(platformId))
			continue;	// should not happen
		bds.append(bd);
	}
}

void DaWorldLL::FindBdsInsideEnclosure(const AuId& enclosureId, DaAtomList& bds) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(!bd->IsInsideEnclosure())
			continue;
		if(bd->EnclosureId() != enclosureId)
			continue;
		if(bd->IsPlatform())
			continue;	// Ignore platform that thinks it is inside an enclosure.
		if(bd->Is(enclosureId))
			continue;	// should not happen
		bds.append(bd);
	}
}

bool DaWorldLL::HasMilitaryOnPlatform(const AuId& platformId) const
{
	if(!MaybeHQHere() && !MaybeBaseHere())	// Don't trigger call to CalcBasePresence or CalcHQPresence due to recursion.
		return false;						// No military buildings on this world at all.

	// Let's just check for military industries, so we can return true on the first one we find. This will ignore power plants,
	// platforms, and enclosures, but the caller doesn't really care about them.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(!DbIndustry::IsMilitary(bd->IndustryId()))
			continue;
		if(bd->PlatformId() != platformId)
			continue;
		return true;
	}
	return false;
}

bool DaWorldLL::HasMilitaryInEnclosure(const AuId& enclosureId) const
{
	if(!MaybeHQHere() && !MaybeBaseHere())	// Don't trigger call to CalcBasePresence or CalcHQPresence due to recursion.
		return false;						// No military buildings on this world at all.

	// Let's just check for military industries, so we can return true on the first one we find. This will ignore power plants,
	// platforms, and enclosures, but the caller doesn't really care about them.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(!DbIndustry::IsMilitary(bd->IndustryId()))
			continue;
		if(bd->EnclosureId() != enclosureId)
			continue;
		return true;
	}
	return false;
}

void DaWorldLL::InvalidateEnclosureCache()
{
	m_EnclosureLock.lockForWrite();
	m_EnclosuresValid = false;
	m_EnclosureLock.unlock();
}

bool DaWorldLL::HasEnclosure() const
{
	m_EnclosureLock.lockForRead();
	if(!m_EnclosuresValid)
	{
		m_EnclosureLock.unlock();
		UpdateEnclosureCache();
		m_EnclosureLock.lockForRead();
	}
	bool v = m_Enclosures;
	m_EnclosureLock.unlock();
	return v;
}

bool DaWorldLL::HasEnclosureNotReady() const
{
	m_EnclosureLock.lockForRead();
	bool v = m_Enclosures && (m_Enclosures->empty() || m_Enclosures->AnyNotReady());
	m_EnclosureLock.unlock();
	return v;
}

void DaWorldLL::SetEnclosureNotReady()
{
	m_EnclosureLock.lockForWrite();
	if(m_Enclosures)
		m_Enclosures->clear();
	else
		m_Enclosures = new AuWorldEnclosureCache();
	m_EnclosuresValid = true;
	m_EnclosureLock.unlock();
}

bool DaWorldLL::FindClosestFencePost(AuPoint& postLoc, const AuPoint& searchLoc, const AuId& onPlatformId)
{
	bool foundPost = false;
	double foundDist2 = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;
		if(bd->PlatformId() != onPlatformId)
			continue;
		AuSegmentArray fenceSegs;
		bd->FenceSegments(fenceSegs);
		fenceSegs.Transform(bd->Transform());	// Into the space of the world.
		for(AuSegmentArray::const_iterator si = fenceSegs.cbegin(); si != fenceSegs.cend(); si++)
		{
			double dist1 = searchLoc.Distance2((*si).m_fLoc1);	// squared distance
			double dist2 = searchLoc.Distance2((*si).m_fLoc2);	// squared distance
			if(dist1 < dist2)
			{
				if(!foundPost || dist1 < foundDist2)
				{
					foundPost = true;
					foundDist2 = dist1;
					postLoc = (*si).m_fLoc1;
				}
			}
			else
			{
				if(!foundPost || dist2 < foundDist2)
				{
					foundPost = true;
					foundDist2 = dist2;
					postLoc = (*si).m_fLoc2;
				}
			}
		}
	}
	return foundPost;
}

void DaWorldLL::FenceAdjust()
{
 	DaAtomList bds;
	FindBds(bds);
 	for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
 	{
 		DaBd* bd = (DaBd*)*bi;
		bd->FenceAdjust();
 	}
}

void DaWorldLL::operator>>(QDataStream& out) const
{
	DaWorld::operator>>(out);
	out << m_BdBits;
	out << m_GeoConstruction;
	out << m_HengePresence;

	m_LLLock.lockForRead();
	out << m_CaptId;
	out << m_CaptName;
	out << m_CitizenSuppliers;
	out << m_ClaimAtLL;
	out << m_ClaimEmpire;
	out << m_ClaimPersonId;
	out << m_ClaimPersonName;
	out << m_ClaimTime;
	out << m_GeologicalFeatures;
	out << *m_RoadMap;
	out << AuTime();	//m_tmGeologyChange;
	out << m_TroopSuppliers;

	out << m_FindTerrainAtPixelDetailsOk;
	out << m_FindTerrainAtPixelEye;
	out << m_FindTerrainAtPixelLook;
	out << m_FindTerrainAtPixelResult;

	out << m_SightDetailsOk;
	out << m_SightedLoc;
	out << m_SightEye;
	out << m_SightIntersect;
	out << m_SightLook;
	m_LLLock.unlock();

	m_BuildingMapLock.lock();
	out << m_BuildingMap;
	m_BuildingMapLock.unlock();

	m_EnclosureLock.lockForRead();
	if(m_Enclosures)
	{
		out << (quint8)1;
		out << *m_Enclosures;
	}
	else
		out << (quint8)0;
	out << m_EnclosuresValid;
	if(m_Irrigation)
	{
		out << (quint8)1;
		out << *m_Irrigation;
	}
	else
		out << (quint8)0;
	m_EnclosureLock.unlock();

	m_LandingSitesLock.lockForRead();
	if(m_LandingSites)
	{
		out << (quint8)1;
		out << (quint8)ResourceZones();
		for(int rz=0; rz<ResourceZones(); rz++)
			out << m_LandingSites[rz];
	}
	else
		out << (quint8)0;
	m_LandingSitesLock.unlock();

	m_TroopsAtStationsLock.lockForRead();
	out << (quint16)m_TroopsAtStations.size();
	for(unordered_map<AuId, int, AuIdHash>::const_iterator ti = m_TroopsAtStations.cbegin(); ti != m_TroopsAtStations.cend(); ti++)
	{
		out << (*ti).first;
		out << (*ti).second;
	}
	m_TroopsAtStationsLock.unlock();

	m_VillageLock.lockForRead();
	if(m_Village)
	{
		out << (quint8)1;
		out << *m_Village;
	}
	else
		out << (quint8)0;
	m_VillageLock.unlock();
}

void DaWorldLL::operator<<(QDataStream& in)
{
	DaWorld::operator<<(in);
	in >> m_BdBits;
	in >> m_GeoConstruction;
	in >> m_HengePresence;

	m_LLLock.lockForWrite();
	in >> m_CaptId;
	in >> m_CaptName;
	in >> m_CitizenSuppliers;
	in >> m_ClaimAtLL;
	in >> m_ClaimEmpire;
	in >> m_ClaimPersonId;
	in >> m_ClaimPersonName;
	in >> m_ClaimTime;
	in >> m_GeologicalFeatures;
	in >> *m_RoadMap;
	AuTime _tmGeologyChange;
	in >> _tmGeologyChange;
	in >> m_TroopSuppliers;

	in >> m_FindTerrainAtPixelDetailsOk;
	in >> m_FindTerrainAtPixelEye;
	in >> m_FindTerrainAtPixelLook;
	in >> m_FindTerrainAtPixelResult;

	in >> m_SightDetailsOk;
	in >> m_SightedLoc;
	in >> m_SightEye;
	in >> m_SightIntersect;
	in >> m_SightLook;
	m_LLLock.unlock();

	m_BuildingMapLock.lock();
	in >> m_BuildingMap;
	m_BuildingMapLock.unlock();

	m_EnclosureLock.lockForWrite();
	if(m_Enclosures)
		delete m_Enclosures;
	quint8 ui8;
	in >> ui8;
	if(ui8)
	{
		m_Enclosures = new AuWorldEnclosureCache;
		in >> *m_Enclosures;
	}
	else
		m_Enclosures = 0;
	in >> m_EnclosuresValid;
	if(m_Irrigation)
		delete m_Irrigation;
	in >> ui8;
	if(ui8)
	{
		m_Irrigation = new QList<AuWorldIrrigation>();
		in >> *m_Irrigation;
	}
	else
		m_Irrigation = 0;
	m_EnclosureLock.unlock();

	m_LandingSitesLock.lockForWrite();
	if(m_LandingSites)
		delete [] m_LandingSites;
	in >> ui8;
	if(ui8)
	{
		quint8 zones;
		in >> zones;
		Q_ASSERT(zones == ResourceZones());
		m_LandingSites = new AuPoint[zones];
		for(int rz=0; rz<zones; rz++)
			in >> m_LandingSites[rz];
	}
	else
		m_LandingSites = 0;
	m_LandingSitesLock.unlock();

	m_TroopsAtStationsLock.lockForWrite();
	m_TroopsAtStations.clear();
	quint16 cnt;
	in >> cnt;
	while(cnt--)
	{
		AuId id;
		int t;
		in >> id;
		in >> t;
		m_TroopsAtStations.insert(pair<AuId, int>(id, t));
	}
	m_TroopsAtStationsLock.unlock();

	m_VillageLock.lockForWrite();
	if(m_Village)
		delete m_Village;
	in >> ui8;
	if(ui8)
	{
		m_Village = new AuVillage;
		in >> *m_Village;
	}
	else
		m_Village = 0;
	m_VillageLock.unlock();
}

void DaWorldLL::DestroyCities()
{
 	DaAtomList bds;
	FindBds(bds);
	QString reason(tr("world destroyed by asteroid"));
	for(DaAtomList::const_iterator ai = bds.begin(); ai != bds.end(); ai++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*ai);
		if(!bd || !bd->IsNormal())
			continue;
		bd->DamageAndDestroy(reason);
	}
	m_BdBits = 0;
}

void DaWorldLL::SceneRenderParameters(const DaAtom* sceneAtom, float& ambientR, float& ambientG, float& ambientB, float& fogDensity,
		float& fogR, float& fogG, float& fogB, bool viewIsTopDown, bool& sunsBlocked) const
{
	AuPoint planetLoc;
	if(!sceneAtom || !sceneAtom->GetLocationRelativeToParent(planetLoc, this))
	{
		DaWorld::SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, viewIsTopDown,
				sunsBlocked);
		return;
	}
	AuGlobeLLF planetLL;
	LLOfLoc(planetLL, planetLoc);

	// My ambient lighting does not depend on the scene atom.
	// Ambient light starts with starlight.
	float starLight;
	const DaSolarSystem* pss = FindParentSolarSystemInEnv();
	if(pss)
		starLight = pss->StarLight();							// Actual starlight level based on local star density.
	else
		starLight = 0.2f;	// Designer preview?
	float sunLight = QuerySunlight(planetLoc);
	float fillum = starLight + sunLight;
	ambientR = fillum;
	ambientG = fillum;
	ambientB = fillum;

	float cityAmbientLight = CityAmbientLight(planetLL, WORLDTEX_Count);
	if(cityAmbientLight > 0.f)
	{
		const QColor& c = AuGLLight::ColorSodiumVapor();
		ambientR += cityAmbientLight * c.redF() * AuGLLight::AmbientLitCity();
		ambientG += cityAmbientLight * c.greenF() * AuGLLight::AmbientLitCity();
		ambientB += cityAmbientLight * c.blueF() * AuGLLight::AmbientLitCity();
	}

	if(ambientR < 0)
		ambientR = 0;
	else if(ambientR > 1)
		ambientR = 1;

	if(ambientG < 0)
		ambientG = 0;
	else if(ambientG > 1)
		ambientG = 1;

	if(ambientB < 0)
		ambientB = 0;
	else if(ambientB > 1)
		ambientB = 1;

	sunsBlocked = false;

	// We do not apply fog when in top down view.
	if(viewIsTopDown)
	{
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
		return;
	}

	if(Hydrographics())
	{
		// This world has seas. Are we separated by water?
		bool seaFogMe;
		if(!sceneAtom->IsInAtmosphere())
		{
			// Scene atom is outside the atmosphere. Fog makes me turn white.
			fogDensity = 0;
			fogR = 0;
			fogG = 0;
			fogB = 0;
			return;
		}
		else if(sceneAtom->IsEyeUnderWater() && !sceneAtom->IsInsideBd() && !sceneAtom->IsInsideEnclosure())
		{
			// Viewer is below the sea surface and they are not inside a building or enclosure. They are viewing their surroundings
			// through water.
			seaFogMe = true;			// Viewer sees me through water, unless we are both in the same undersea city.
		}
		else
		{
			// Viewer is above the sea surface or they are inside a building or enclosure.
			seaFogMe = false;			// Viewer sees me clearly.
		}

		if(seaFogMe)
		{
			// Setup atmospheric fog to present underwater environment.
			QColor colSea = SeaColor();
			fogDensity = SeaFogDensity();
			fogR = colSea.redF() * fillum;
			fogG = colSea.greenF() * fillum;
			fogB = colSea.blueF() * fillum;
			return;
		}
	}

	// I am not separated by sea water.
	// Calculate atmospheric fog.
	float atmDensity = sceneAtom->AtmosphereDensity();
	if(atmDensity > 0 && sceneAtom->IsInAtmosphere())
	{
		// Setup atmospheric fog based on atmosphere density.
		QColor colAtm = DaWorld::AtmosphereColor();
		if(IsGasGiant() && sceneAtom->IsEyeUnderWater())
			fogDensity = SeaFogDensity();
		else
			fogDensity = atmDensity/1000.f;
		fogR = colAtm.redF() * fillum;
		fogG = colAtm.greenF() * fillum;
		fogB = colAtm.blueF() * fillum;
	}
	else
	{
		// I am inside atmosphere range but there is no atmosphere density.
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
	}
}

bool DaWorldLL::IsSuitableRespawnPlanet(quint32 empireId) const
{
	if(!IsSuitableHomePlanet())
		return false;		// Environment is not suitable.

	quint32 claimEmpire = ClaimedByEmpire();
	if(claimEmpire && claimEmpire != empireId)
		return false;		// World is claimed by someone else.

	if(!BdHere())
		return true;		// No buildings or cities at all. This world is ok.

	// Check for any buildings that belong to an empire other than 0 or 'empireId'.
	DaAtomList bds;
	FindBds(bds);
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		const DaBd* bd = (const DaBd*)*bi;
		if(bd->EmpireId() != 0 && bd->EmpireId() != empireId)
			return false;	// Building belongs to an empire other than 0 or empireId.
	}

	// Did not find any buildings that belong to an empire other than 0 or empireId.
	return true;
}

