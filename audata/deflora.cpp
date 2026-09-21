//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deflora.cpp,v 1.140 2022/04/08 14:49:07 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "deflora.h"
//
#include "audice.h"
#include "aufilever.h"
#include "aufloradna.h"
#include "augleffects.h"
#include "auglobal.h"
#include "aumatrix4x4.h"
#include "hashpack.h"
#include <QDataStream>
#include <QDir>
#include <QPainter>
#include <QVariant>

#define GRASS_TEXTURE_VER				0x24					// FG12345Z0 plant texture files in temporary cache

#define d_CacheDebugs					false					// True to log cache hits/misses.
static int s_LeafBrushImages = 51;								// Count of leaf brush image files, named Leaf1.png, Leaf2.png, etc.
static int s_PetalBrushImages = 30;								// Count of flower petal brush image files, named Petal1.png, etc.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A node of a plant branch.

void DhPlantNode::Transform(const AuMatrix4x4& t)
{
	m_Loc = t.Project(m_Loc);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A branch of a plant.

void DhPlantBranch::AddNode(const AuPoint& loc, float dia, unsigned int branches)
{
	DhPlantNode node(loc, dia, branches);
	append(node);
}

void DhPlantBranch::Extents(AuPoint& min, AuPoint& max) const
{
	// Expand extents using nodes.
	for(const_iterator bit = constBegin(); bit != constEnd(); bit++)
	{
		min = min.Min((*bit).m_Loc);
		max = max.Max((*bit).m_Loc);
	}
}

void DhPlantBranch::Transform(const AuMatrix4x4& t)
{
	for(iterator bit = begin(); bit != end(); bit++)
		(*bit).Transform(t);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// All the branches of a plant.

void DhPlantBranches::Extents(AuPoint& min, AuPoint& max) const
{
	// Init our extents to the branch origin. It must be included no matter what.
	min = AuPoint(0,0);
	max = AuPoint(0,0);

	// Expand extents using branches.
	for(const_iterator pit = constBegin(); pit != constEnd(); pit++)
		(*pit).Extents(min, max);
}

void DhPlantBranches::Transform(const AuMatrix4x4& t)
{
	for(iterator pit = begin(); pit != end(); pit++)
		(*pit).Transform(t);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DeFlora::DeFlora()
{
	m_TextureSize = 0.f;
}

DeFlora::~DeFlora()
{
}

void DeFlora::Init(AuFloraDNA::ZONE zone, const AuDice& dice, int sizemod)
{
	m_DNA.Init(zone, dice, sizemod);
}

void DeFlora::operator<<(QDataStream& in)
{
	m_DNA << in;
}

void DeFlora::operator>>(QDataStream& out) const
{
	m_DNA >> out;
}

void DeFlora::TxUpdate(QDataStream& toClient) const
{
	m_DNA.TxUpdate(toClient);
}

void DeFlora::RxUpdate(QDataStream& fromServer)
{
	m_DNA.RxUpdate(fromServer);
}

void DeFlora::PaintGroundCover(QImage& im, quint32 genSeed, AuFloraDNA::ZONE zone, const AuDice& Cdice, const AuTime& floraChange)
{
	AuDice dice(Cdice);
	//qDebug() << "DeFlora::PaintGroundCover" << genSeed << zone;
	// Try loading the ground cover image from a cache file.
	bool imloaded = false;
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName;
	if(floraChange.IsNull())
		cacheFileName = cacheDir.absoluteFilePath(QString("FG%1Z%2")
				.arg(genSeed)
				.arg(zone));
	else
		cacheFileName = cacheDir.absoluteFilePath(QString("FG%1Z%2C%3")
				.arg(genSeed)
				.arg(zone)
				.arg(floraChange.MSecsSinceEpoch()));
	QFile cacheFile(cacheFileName);
	if(cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;								// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == GRASS_TEXTURE_CACHE && fileVersion == GRASS_TEXTURE_VER)
		{
			// Cache file looks good.
			// Read the compressed byte array from the file.
			QByteArray bcomp;
			dsfile >> bcomp;
			dsfile >> m_TextureSize;

			if(!bcomp.isEmpty())
			{
				// Uncompress the byte array.
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,cacheFileName);

				// Read the model cache from the uncompressed byte array.
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> im;
				if(!im.isNull())
					imloaded = true;
			}
		}

		if(d_CacheDebugs && imloaded)
			qDebug() << "DeFlora::PaintGroundCover im loaded size" << im.width() << im.height();;

		// Close the file.
		cacheFile.close();
		if(imloaded && im.width() == g_Au.PlantTextureSize() && im.height() == g_Au.PlantTextureSize())
		{
			if(d_CacheDebugs)
				qDebug() << "DeFlora::PaintGroundCover" << genSeed << zone << "loaded from cache" << cacheFileName;
			return;
		}
	}

	//float saveTextureSize = m_TextureSize;

	// Erase the image to transparent black.
	im = QImage(g_Au.PlantTextureSize(), g_Au.PlantTextureSize(), QImage::Format_ARGB32);
	QPainter p(&im);
	p.setCompositionMode(QPainter::CompositionMode_Clear);		// clear mode
	QSize sz = im.size();
	p.fillRect(0,0,sz.width(),sz.height(), QBrush(Qt::transparent));
	p.setCompositionMode(QPainter::CompositionMode_SourceOver);	// default mode

	// Make four small images of the plant and tile them into the big image.
	sz /= 2;
	QImage implant(sz, QImage::Format_ARGB32);

	const int leafBrushImage = dice.Roll(1,s_LeafBrushImages);
	const int petalBrushImage = dice.Roll(1,s_PetalBrushImages);

	// Paint a branch in the top left quadrant.
	PaintBranch(implant, zone, dice, leafBrushImage, petalBrushImage);
	dice.Random();
	p.drawImage(0,0, implant);

	// Paint a branch in the top right quadrant.
	PaintBranch(implant, zone, dice, leafBrushImage, petalBrushImage);
	dice.Random();
	p.drawImage(sz.width(),0, implant);

	// Paint a branch in the bottom left quadrant.
	PaintBranch(implant, zone, dice, leafBrushImage, petalBrushImage);
	dice.Random();
	p.drawImage(0,sz.height(), implant);

	// Paint a branch in the bottom right quadrant.
	PaintBranch(implant, zone, dice, leafBrushImage, petalBrushImage);
	dice.Random();
	p.drawImage(sz.width(),sz.height(), implant);

	p.end();

	//m_TextureSize = saveTextureSize;
	if(d_CacheDebugs)
		qDebug() << "DeFlora::PaintGroundCover" << genSeed << zone << "created" << m_TextureSize;

	// Store the result in a cache file.
	// Insure the cache folder exists.
	if(g_Au.CreateCacheGame())
	{
		// The cache folder exists.
		if(cacheFile.open(QFile::WriteOnly))
		{
			// Prepare to write to the file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Write a header to the file that includes the date/time the cache file was created.
			const quint8 fileType = GRASS_TEXTURE_CACHE;
			const quint8 fileVersion = GRASS_TEXTURE_VER;
			dsfile << fileType;
			dsfile << fileVersion;
			dsfile << floraChange.UTC();

			// Store path cache into a byte array so it can be compressed.
			QByteArray ba;
			QDataStream ds(&ba, QIODevice::WriteOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			ds << im;

			// Compress the byte array.
			QByteArray bcomp = qCompress(ba);

			// Write the compressed byte array to the file.
			dsfile << bcomp;
			dsfile << m_TextureSize;

			// Close the file.
			cacheFile.close();
			if(d_CacheDebugs)
				qDebug() << "DeFlora::PaintGroundCover" << genSeed << zone << "cached" << cacheFileName;
		}
		else
			qWarning() << QString("DeFlora::PaintGroundCover FAILED TO CREATE FILE %1").arg(cacheFileName);
	}
}

// Paint a typical branch of this plant into a pixel image.
// The branch is depicted as if viewed from above, dropping from the center
// pixel along the top edge of the image.
void DeFlora::CalculateTextureSizes(const AuDice& dice)
{
	// Calculate a typical branch structure using my DNA.
	float tdia = m_DNA.TrunkBaseDiameter();
	//qDebug("DeTerrainFlora::PaintBranch %s base dia %g",
			//(const char*)m_DNA.Description(), tdia);
	
	// Stiffness determines branch taper, 15-50%
	float taperFactor = m_DNA.BranchTaperFactor();

	// The first node is the point the main branch starts.
	// Calculate trunk diameter at the second node. This will be the diameter of
	// the typical branch we are painting.
	float basedia = tdia * taperFactor;		// Reduce diameter by taper factor.

	// Create a data structure to accumulate all branches of this branch.
	// There is always a main branch consisting of at least one segment.
	// Main branch segment always starts at the center of the bottom edge and
	// goes straight upward in y. There is always at least one main branch
	// segment.
	// A leaf cluster can exist at every node.
	// The last/tip loc of every branch may potentially sprout a flower,
	// depending upon DNA probability.
	//
	// NOTE: Comments describe the branch as constructed "up" a positive Y axis,
	//       like in geometry. When painted, the Y axis goes down the image so
	//       the branch ends up hanging downward from the top center pixel of
	//       the image.
	DhPlantBranches branches;
	float tipdia = CreateBranches(branches, basedia, g_Au.PlantBranchMaxSegs(), m_DNA.MinBranchDiameter(), dice);
	//if(branches.count() > 100)
		//qDebug("DeFlora::PaintBranch %s zone %d base dia %g branches %d", (const char*)m_DNA.Description().toAscii(), zone, tdia,
				//branches.count());

	// Scale the branches to fit within the image size, allowing for a leaf/petal length margin at the left, right, and bottom.
	float margdia = tipdia;
	float tipLen = m_DNA.BranchLength(margdia);
	float leafVeinWidth = tipLen * ((float)m_DNA.LeafVeinWidth)/8.f;
	float stemlen = leafVeinWidth * ((float)m_DNA.LeafStemLength+1.f) / 4.f;
	float tipLeafLen = m_DNA.LeafLength(tipLen);
	float leafSpace = tipLeafLen + leafVeinWidth + stemlen;

	float petalInc = m_DNA.PetalSizeIncrement(margdia, 0);
	float petalWidth = m_DNA.PetalWidthCalc(petalInc);
	float petalLen;
	if(m_DNA.PetalLength > m_DNA.PetalTaper)
		petalLen = petalInc * (float)m_DNA.PetalLength;
	else
		petalLen = petalInc * (float)m_DNA.PetalTaper;
	float petalSpace = petalLen + petalWidth;
	if(m_DNA.IsUmbel())
		petalSpace /= 3.f;

	float margin;
	if(petalSpace > leafSpace)
		margin = petalSpace;
	else
		margin = leafSpace;
	AuPoint bmin,bmax;
	branches.Extents(bmin,bmax);
	bmin.x -= margin;
	bmax.x += margin;
	bmax.y += margin;

	// Calculate scale value as branch units per pixel.
	// Calculate a scale that allows one extra pixel at the left, right, and bottom.
	const int tile = g_Au.PlantTextureSize();	// Image is expected to be square.
	float xScaleL;				// units per pixel scale needed for part of branch left of center.
	if(bmin.x >= 0.)
		xScaleL = 0.;			// No branches to the left of the center...too bad.
	else
		xScaleL = -bmin.x/(tile/2-1);	// Scale to fit left side into left half.
	float xScaleR;				// units per pixel scale needed for part of branch right of center.
	if(bmax.x <= 0.)
		xScaleR = 0.;			// No branches to the right of the center...too bad.
	else
		xScaleR = bmax.x/(tile/2-1);	// Scale to fit right side into right half.
	float yScale;		// units/pixel scale needed to fit branch in Y direction.
	if(bmax.y <= 0.)
		yScale = 0.;			// No branches above the branch point isn't possible.
	else
		yScale = bmax.y/(tile-1);		// Scale to fit vertically.

	// We have three scales to compare; keep the largest one.
	float unitsPerPixel;
	if(xScaleL > xScaleR && xScaleL > yScale)
		unitsPerPixel = xScaleL;
	else if(xScaleR > yScale)
		unitsPerPixel = xScaleR;
	else
		unitsPerPixel = yScale;

	m_TextureSize = unitsPerPixel * tile;
}

// Paint a typical branch of this plant into a pixel image.
// The branch is depicted as if viewed from above, dropping from the center
// pixel along the top edge of the image.
void DeFlora::PaintBranch(QImage& im, AuFloraDNA::ZONE /*zone*/, const AuDice& Cdice, int leafBrushImage, int petalBrushImage)
{
	AuDice dice(Cdice);
	/*
	// Planet provides foliage color based on zone.
	bool isPolar, isTropical;
	switch(zone)
	{
	case AuFloraDNA::Polar:
		isPolar = true;
		isTropical = false;
		break;
	case AuFloraDNA::Temperate:
		isPolar = false;
		isTropical = false;
		break;
	case AuFloraDNA::Tropical:
		isPolar = false;
		isTropical = true;
		break;
	case AuFloraDNA::Zones:
	default:
		return;
	}
	*/

	// Calculate a typical branch structure using my DNA.
	float tdia = m_DNA.TrunkBaseDiameter();
	//qDebug("DeTerrainFlora::PaintBranch %s base dia %g",
			//(const char*)m_DNA.Description(), tdia);
	
	// Stiffness determines branch taper, 15-50%
	float taperFactor = m_DNA.BranchTaperFactor();

	// The first node is the point the main branch starts.
	// Calculate trunk diameter at the second node. This will be the diameter of
	// the typical branch we are painting.
	float basedia = tdia * taperFactor;		// Reduce diameter by taper factor.

	// Create a data structure to accumulate all branches of this branch.
	// There is always a main branch consisting of at least one segment.
	// Main branch segment always starts at the center of the bottom edge and
	// goes straight upward in y. There is always at least one main branch
	// segment.
	// A leaf cluster can exist at every node.
	// The last/tip loc of every branch may potentially sprout a flower,
	// depending upon DNA probability.
	//
	// NOTE: Comments describe the branch as constructed "up" a positive Y axis,
	//       like in geometry. When painted, the Y axis goes down the image so
	//       the branch ends up hanging downward from the top center pixel of
	//       the image.
	DhPlantBranches branches;
	float tipdia = CreateBranches(branches, basedia, g_Au.PlantBranchMaxSegs(), m_DNA.MinBranchDiameter(), dice);
	dice.Random();
	//if(branches.count() > 100)
		//qDebug("DeFlora::PaintBranch %s zone %d base dia %g branches %d", (const char*)m_DNA.Description().toAscii(), zone, tdia,
				//branches.count());

	// Scale the branches to fit within the image size, allowing for a leaf/petal length margin at the left, right, and bottom.
	float margdia = tipdia;
	float tipLen = m_DNA.BranchLength(margdia);
	float leafVeinWidth = tipLen * ((float)m_DNA.LeafVeinWidth)/8.f;
	float stemlen = leafVeinWidth * ((float)m_DNA.LeafStemLength+1.f) / 4.f;
	float tipLeafLen = m_DNA.LeafLength(tipLen);
	float leafSpace = tipLeafLen + leafVeinWidth + stemlen;

	float petalInc = m_DNA.PetalSizeIncrement(margdia, 0);
	float petalWidth = m_DNA.PetalWidthCalc(petalInc);
	float petalLen;
	if(m_DNA.PetalLength > m_DNA.PetalTaper)
		petalLen = petalInc * (float)m_DNA.PetalLength;
	else
		petalLen = petalInc * (float)m_DNA.PetalTaper;
	float petalSpace = petalLen + petalWidth;
	if(m_DNA.IsUmbel())
		petalSpace /= 3.f;

	float margin;
	if(petalSpace > leafSpace)
		margin = petalSpace;
	else
		margin = leafSpace;
	AuPoint bmin,bmax;
	branches.Extents(bmin,bmax);
	bmin.x -= margin;
	bmax.x += margin;
	bmax.y += margin;

	// Calculate scale value as branch units per pixel.
	// Calculate a scale that allows one extra pixel at the left, right, and bottom.
	const int tile = im.height();	// Image is expected to be square.
	float xScaleL;				// units per pixel scale needed for part of branch left of center.
	if(bmin.x >= 0.)
		xScaleL = 0.;			// No branches to the left of the center...too bad.
	else
		xScaleL = -bmin.x/(tile/2-1);	// Scale to fit left side into left half.
	float xScaleR;				// units per pixel scale needed for part of branch right of center.
	if(bmax.x <= 0.)
		xScaleR = 0.;			// No branches to the right of the center...too bad.
	else
		xScaleR = bmax.x/(tile/2-1);	// Scale to fit right side into right half.
	float yScale;		// units/pixel scale needed to fit branch in Y direction.
	if(bmax.y <= 0.)
		yScale = 0.;			// No branches above the branch point isn't possible.
	else
		yScale = bmax.y/(tile-1);		// Scale to fit vertically.

	// We have three scales to compare; keep the largest one.
	float unitsPerPixel;
	if(xScaleL > xScaleR && xScaleL > yScale)
		unitsPerPixel = xScaleL;
	else if(xScaleR > yScale)
		unitsPerPixel = xScaleR;
	else
		unitsPerPixel = yScale;

	float pixelsPerUnit;		// QPainter::scale wants pixels per unit.
	if(unitsPerPixel != 0.)
		pixelsPerUnit = 1./unitsPerPixel;
	else
		pixelsPerUnit = 1.;

	// Make brushes for painting the leaves.
	QBrush brLeaf[3];	// Brushes from dark to light.
	int brLeafSize;
	{
		// The brush images are expected to be square.
		QImage imleaf(QString(":data/Leaf%1.png").arg(leafBrushImage));
		brLeafSize = imleaf.width();
		QPixmap myleaf(brLeafSize, brLeafSize);
		QPainter pleaf(&myleaf);

		pleaf.drawImage(0,0, imleaf);
		brLeaf[2].setTexture(myleaf);

		// Make two more that are progressively darker.
		pleaf.fillRect(0,0, brLeafSize,brLeafSize, QColor(0,0,0,30));
		brLeaf[1].setTexture(myleaf);

		pleaf.fillRect(0,0, brLeafSize,brLeafSize, QColor(0,0,0,50));
		brLeaf[0].setTexture(myleaf);
	}

	// Make brushs from the bark texture for painting the branches and thorns.
	QBrush brBark[4];
	{
		QImage imbark(":data/txBark.png");
		QPixmap mybark(imbark.width(), imbark.width());
		QTransform tbark;
		tbark.scale(unitsPerPixel, unitsPerPixel);

		QPainter pbark(&mybark);
		pbark.drawImage(0,0, imbark, 0,DNA().BarkTexture*imbark.width(), imbark.width(),imbark.width());
		brBark[3].setTexture(mybark);
		brBark[3].setTransform(tbark);

		// Make three more that are progressively darker.
		pbark.fillRect(0,0, imbark.width(),imbark.width(), QColor(0,0,0,30));
		brBark[2].setTexture(mybark);
		brBark[2].setTransform(tbark);

		pbark.fillRect(0,0, imbark.width(),imbark.width(), QColor(0,0,0,30));
		brBark[1].setTexture(mybark);
		brBark[1].setTransform(tbark);

		pbark.fillRect(0,0, imbark.width(),imbark.width(), QColor(0,0,0,30));
		brBark[0].setTexture(mybark);
		brBark[0].setTransform(tbark);
	}

	// Erase the image to transparent black.
	QPainter p(&im);
	p.setCompositionMode(QPainter::CompositionMode_Clear);		// clear mode
	p.fillRect(0,0,tile,tile, QBrush(Qt::transparent));
	p.setCompositionMode(QPainter::CompositionMode_SourceOver);	// default mode

	// Paint the branches.
	float diamod[4] =
	{
		1,			// 90 degrees
		//.9239f,		// 67.5 degrees
		.7071f,		// 45 degrees
		.3827f,		// 22.5 degrees
		.1951f,		// 11.25 degrees
	};
	p.translate(tile/2,0);
	p.scale(pixelsPerUnit, pixelsPerUnit);
	p.setRenderHint(QPainter::Antialiasing);
	QPointF prevLoc;
	float prevDia = 0;
	for(DhPlantBranches::const_iterator pit = branches.constBegin(); pit != branches.constEnd(); pit++)
	{
		// Each branch is painted multiple times with reducing brush sizes and
		// lightening colors so it has some depth.
		for(int pass=0; pass<4; pass++)
		{
			// Use a different color on each pass.
			for(DhPlantBranch::const_iterator bit = (*pit).constBegin(); bit != (*pit).constEnd(); bit++)
			{
				QPointF loc((*bit).m_Loc.x, (*bit).m_Loc.y);
				float dia = (*bit).m_Dia * diamod[pass];
				if(bit != (*pit).constBegin())
				{
					// Paint the branch segment at its ending thickness.
					// Each pass draws the branch thinner and lighter in color.
					float avgdia = (prevDia + dia)/2.f;
					// Stop drawing this color band on this branch when it gets too small. We need to draw all of pass zero though.
					if(pass && avgdia < m_DNA.MinBranchDiameter())
						break;
					p.setPen(QPen(brBark[pass], avgdia, Qt::SolidLine, Qt::RoundCap));
					p.drawLine(prevLoc, loc);
				}
				prevLoc = loc;
				prevDia = dia;
			}
		}
	}

	// Paint the thorns.
	if(m_DNA.ThornSize > 0)
	{
		// This plant has thorns.
		// Create a pen for drawing the thorns.
		for(DhPlantBranches::const_iterator pit = branches.constBegin(); pit != branches.constEnd(); pit++)
		{
			// The branches are iterated from tip to base so bigger low thorns overlap smaller tip thorns.
			QPointF ploc;
			for(int ib = (*pit).count() - 1; ib >= 0; ib--)
			{
				QPointF bloc((*pit)[ib].m_Loc.x, (*pit)[ib].m_Loc.y);
				if(ib != (*pit).count() - 1)
				{
					// Draw thorns along the branch segment from ploc to bloc.
					float thornLen = (float)m_DNA.ThornSize * (*pit)[ib].m_Dia;
					float thornDia = (*pit)[ib].m_Dia / 6.;
					QPointF bvec = bloc - ploc;
					float blen = AuPoint(bloc.x(), bloc.y()).Distance_2D(AuPoint(ploc.x(), ploc.y()));
					if(blen == 0)
						continue;
					bvec /= blen;	// Make branch vec into a unit vector.
					QPointF bperp(-bvec.y(), bvec.x());	// Perp vector for thorns.
					QPointF tip = bperp * thornLen;		// Thorn tip vector
					QPointF base = bperp * (*pit)[ib].m_Dia/2.;	// Thorn base vector
					int thorns = (int)(blen * ((int)m_DNA.ThornFreq + 1)) + 2;
					bvec *= blen/(float)(thorns+1);	// vec to each thorn
					for(int it=0; it<thorns; it++)
					{
						ploc += bvec;
						int lenroll = dice.Roll(1,50,50);	// percent of length
						float lenfact = (float)lenroll/100.;
						if(it&1)
						{
							p.setPen(QPen(brBark[0], thornDia, Qt::SolidLine, Qt::RoundCap));
							p.drawLine(ploc + base*lenfact, ploc + tip*(lenfact/2.));
							p.setPen(QPen(brBark[3], thornDia/2., Qt::SolidLine, Qt::RoundCap));
							p.drawLine(ploc + base*lenfact, ploc + tip*lenfact);
						}
						else
						{
							p.setPen(QPen(brBark[0], thornDia, Qt::SolidLine, Qt::RoundCap));
							p.drawLine(ploc - base*lenfact, ploc - tip*(lenfact/2.));
							p.setPen(QPen(brBark[3], thornDia/2., Qt::SolidLine, Qt::RoundCap));
							p.drawLine(ploc - base*lenfact, ploc - tip*lenfact);
						}
					}
				}
				ploc = bloc;
			}
		}
	}

	// Paint the leaves.
	bool mustHaveLeaf = false;
	if(m_DNA.FlowerProbability == 0)
		mustHaveLeaf = true;	// No flowers so we will guarantee at least on leaf.
	bool hasLeaf = false;
	for(DhPlantBranches::const_iterator pit = branches.constBegin(); pit != branches.constEnd(); pit++)
	{
		// The branches are iterated from tip to base so bigger low leaves overlap smaller tip leaves.
		for(int ib = (*pit).count() - 1; ib > 0; ib--)
		{
			// Test for leaf cluster presense.
			if(!mustHaveLeaf && dice.Roll(1,4,-1) > (int)m_DNA.LeafProbability)
				continue;
			mustHaveLeaf = false;	// We've got a leaf now.
			hasLeaf = true;

			QPointF bloc((*pit)[ib].m_Loc.x, (*pit)[ib].m_Loc.y);
			for(unsigned int leaf=0; leaf<=m_DNA.Multiples; leaf++)
			{
				int leafAngle = dice.Roll(1,360,-1);	// Random angle for each.
				float leafLen = m_DNA.LeafLength(m_DNA.BranchLength((*pit)[ib].m_Dia));
				if(leafLen <= 0.)
					continue;
				float leafVeinWidth = leafLen * (float)m_DNA.LeafVeinWidth / 8.f;
				AuPoint leafTipVec(leafLen, 0.);
				leafTipVec = leafTipVec.RotateZ((float)leafAngle * DEG2RADf);

				// Move off the connect point so multiples look nicer.
				float stemlen = leafVeinWidth * ((float)m_DNA.LeafStemLength+1.f) / 4.f;
				QPointF loc(bloc);
				if(stemlen > 0.)
					loc += QPointF(leafTipVec.x,leafTipVec.y) * stemlen / leafLen;

				// Calculate a scale for the brush to fit onto the leaf so we keep the most detail.
				float maxLeafLength = leafLen + leafVeinWidth;
				float leadOff = leafVeinWidth/2;
				if(m_DNA.LeafVeinCount)
				{
					switch(m_DNA.LeafType)
					{
					case AuFloraDNA::LEAF_ParallelLonger:
					case AuFloraDNA::LEAF_ParallelShorter:
						maxLeafLength += leafLen * 1.2 * (m_DNA.LeafVeinCount+1);
						break;
					case AuFloraDNA::LEAF_ParallelSame:
						maxLeafLength += leafLen;
						break;
					case AuFloraDNA::LEAF_RadialSame:
						maxLeafLength = leafLen * 2 + leafVeinWidth;
						leadOff = maxLeafLength / 2;
						break;
					case AuFloraDNA::LEAF_RadialLonger:
					case AuFloraDNA::LEAF_RadialShorter:
						maxLeafLength = leafLen * 1.3 * (m_DNA.LeafVeinCount+1) * 2 + leafVeinWidth;
						leadOff = maxLeafLength / 2;
						break;
					case AuFloraDNA::LEAF_BowEnds:
					case AuFloraDNA::LEAF_BowStem:
						break;
					}
				}
				float brushScale = maxLeafLength * pixelsPerUnit / brLeafSize;

				// Configure the brush to paint the leaf at the proper location and angle.
				QTransform tleaf;
				tleaf.translate(loc.x(), loc.y());
				tleaf.scale(brushScale, brushScale);
				tleaf.rotate(leafAngle);
				tleaf.translate(-leadOff / brushScale, brLeafSize/2 * unitsPerPixel);
				tleaf.scale(unitsPerPixel, unitsPerPixel);

				// Draw the leaf flesh from dark to light.
				for(int flesh=0; flesh<3; flesh++)
				{
					brLeaf[flesh].setTransform(tleaf);
					p.setPen(QPen(brLeaf[flesh], leafVeinWidth, Qt::SolidLine, Qt::RoundCap));
					p.drawLine(loc, loc + QPointF(leafTipVec.x, leafTipVec.y));
					if(m_DNA.LeafVeinCount)
					{
						switch(m_DNA.LeafType)
						{
						case AuFloraDNA::LEAF_ParallelLonger:
						case AuFloraDNA::LEAF_ParallelSame:
						case AuFloraDNA::LEAF_ParallelShorter:
							{
								AuPoint vbvec(leafTipVec / ((float)m_DNA.LeafVeinCount+1));
								for(unsigned int iv=0; iv<=m_DNA.LeafVeinCount; iv++)
								{
									float angle = (42.f + 6.f * (float)m_DNA.BranchAngle)*DEG2RADf;
									QPointF vb = loc + QPointF(vbvec.x*iv, vbvec.y*iv);
									AuPoint vevec = vbvec.RotateZ(angle);
									if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelLonger)
										vevec *= (1.2 * (iv+1));
									else if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelShorter)
										vevec *= (1.2 * ((float)m_DNA.LeafVeinCount-iv+1));
									QPointF ve = vb + QPointF(vevec.x, vevec.y);
									p.drawLine(vb, ve);

									vevec = vbvec.RotateZ(-angle);
									if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelLonger)
										vevec *= (1.2 * (iv+1));
									else if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelShorter)
										vevec *= (1.2 * ((float)m_DNA.LeafVeinCount-iv+1));
									ve = vb + QPointF(vevec.x, vevec.y);
									p.drawLine(vb, ve);
								}
							}
							break;
						case AuFloraDNA::LEAF_RadialLonger:
						case AuFloraDNA::LEAF_RadialSame:
						case AuFloraDNA::LEAF_RadialShorter:
							{
								AuPoint vbvec(leafTipVec);
								for(unsigned int iv=0; iv<=m_DNA.LeafVeinCount; iv++)
								{
									float angle = (40.f + 3.f * (float)m_DNA.BranchAngle)*iv*DEG2RADf;
									AuPoint vevec = vbvec.RotateZ(angle);
									if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialLonger)
										vevec *= (1.3 * (iv+1));
									else if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialShorter)
										vevec *= (1.3 * ((float)m_DNA.LeafVeinCount-iv+1));
									QPointF ve = loc + QPointF(vevec.x, vevec.y);
									p.drawLine(loc, ve);

									vevec = vbvec.RotateZ(-angle);
									if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialLonger)
										vevec *= (1.3 * (iv+1));
									else if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialShorter)
										vevec *= (1.3 * ((float)m_DNA.LeafVeinCount-iv+1));
									ve = loc + QPointF(vevec.x, vevec.y);
									p.drawLine(loc, ve);
								}
							}
							break;
						case AuFloraDNA::LEAF_BowEnds:
						case AuFloraDNA::LEAF_BowStem:
							{
								AuPoint vbvec(leafTipVec);

								// Calculate useful vectors.
								AuPoint hvec(vbvec/2.);
								AuPoint qvec(hvec/2.);
								AuPoint perp = qvec.PerpendicularVector_2D();
								perp.Unit();
								QPolygonF poly;
								for(unsigned int iv=0; iv<=m_DNA.LeafVeinCount; iv++)
								{
									// Calculate width arc line must fill at center
									// of branch segment.
									float cenwid = leafVeinWidth/2.f*iv;
									float qwid = cenwid * .7071f;

									poly.clear();

									// Left side.
									poly.append(loc);
									poly.append(loc + QPointF(qvec.x,qvec.y) + QPointF(perp.x, perp.y)*qwid);
									poly.append(loc + QPointF(hvec.x,hvec.y) + QPointF(perp.x, perp.y)*cenwid);
									poly.append(loc + QPointF(qvec.x*3.,qvec.y*3.) + QPointF(perp.x, perp.y)*qwid);
									poly.append(loc + QPointF(vbvec.x,vbvec.y));

									// Right side.
									poly.append(loc + QPointF(qvec.x*3.,qvec.y*3.) - QPointF(perp.x, perp.y)*qwid);
									poly.append(loc + QPointF(hvec.x,hvec.y) - QPointF(perp.x, perp.y)*cenwid);
									poly.append(loc + QPointF(qvec.x,qvec.y) - QPointF(perp.x, perp.y)*qwid);
									poly.append(loc);

									p.drawPolyline(poly);
								}
							}
							break;
						}
					}

					// Shrink a bit for the next pass, so we leave a dark edge.
					leafVeinWidth *= .9f;
				}

				// Highlight the leaf with lighter veins.
				// If the flesh was drawn small, don't bother drawing veins. The result looks too washed out.
				//if(leafVeinWidth * pixelsPerUnit < 2.5f)
					//continue;	// Skip veins.
				if(bloc != loc)
				{
					//p.setPen(QPen(QBrush(QColor(70,85,70)), 0., Qt::SolidLine, Qt::RoundCap));
					p.setPen(QPen(QBrush(brBark[3]), 0., Qt::SolidLine, Qt::RoundCap));
					// The center vein goes all the way to the branch connect point to form the stem.
					p.drawLine(bloc, loc);
				}
				/*
				if(m_DNA.LeafVeinCount)
				{
					switch(m_DNA.LeafType)
					{
					case AuFloraDNA::LEAF_ParallelLonger:
					case AuFloraDNA::LEAF_ParallelSame:
					case AuFloraDNA::LEAF_ParallelShorter:
						{
							AuPoint vbvec(leafTipVec / ((float)m_DNA.LeafVeinCount+1));
							for(unsigned int iv=0; iv<=m_DNA.LeafVeinCount; iv++)
							{
								float angle = (42.f + 6.f * (float)m_DNA.BranchAngle)*DEG2RADf;
								QPointF vb = loc + QPointF(vbvec.x*iv, vbvec.y*iv);
								AuPoint vevec = vbvec.RotateZ(angle);
								if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelLonger)
									vevec *= (1.2 * (iv+1));
								else if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelShorter)
									vevec *= (1.2 * ((float)m_DNA.LeafVeinCount-iv+1));
								QPointF ve = vb + QPointF(vevec.x, vevec.y);
								p.drawLine(vb, ve);

								vevec = vbvec.RotateZ(-angle);
								if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelLonger)
									vevec *= (1.2 * (iv+1));
								else if(m_DNA.LeafType == AuFloraDNA::LEAF_ParallelShorter)
									vevec *= (1.2 * ((float)m_DNA.LeafVeinCount-iv+1));
								ve = vb + QPointF(vevec.x, vevec.y);
								p.drawLine(vb, ve);
							}
						}
						break;
					case AuFloraDNA::LEAF_RadialLonger:
					case AuFloraDNA::LEAF_RadialSame:
					case AuFloraDNA::LEAF_RadialShorter:
						{
							AuPoint vbvec(leafTipVec);
							for(unsigned int iv=0; iv<=m_DNA.LeafVeinCount; iv++)
							{
								float angle = (40.f + 3.f * (float)m_DNA.BranchAngle)*iv*DEG2RADf;
								AuPoint vevec = vbvec.RotateZ(angle);
								if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialLonger)
									vevec *= (1.3 * (iv+1));
								else if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialShorter)
									vevec *= (1.3 * ((float)m_DNA.LeafVeinCount-iv+1));
								QPointF ve = loc + QPointF(vevec.x, vevec.y);
								p.drawLine(loc, ve);

								vevec = vbvec.RotateZ(-angle);
								if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialLonger)
									vevec *= (1.3 * (iv+1));
								else if(m_DNA.LeafType == AuFloraDNA::LEAF_RadialShorter)
									vevec *= (1.3 * ((float)m_DNA.LeafVeinCount-iv+1));
								ve = loc + QPointF(vevec.x, vevec.y);
								p.drawLine(loc, ve);
							}
						}
						break;
					case AuFloraDNA::LEAF_BowEnds:
					case AuFloraDNA::LEAF_BowStem:
						// This leaf only gets the single vein down the center.
						break;
					}
				}
				*/
			}
		}
	}

	// Paint the flowers.
	if(m_DNA.FlowerProbability > 0)
	{
		// Make brushes for painting the flowers.
		QBrush brPetal[3];	// Brushes from dark to light.
		int brPetalSize;
		{
			// The brush images are expected to be square.
			QImage impetal(QString(":data/Petal%1.png").arg(petalBrushImage));
			brPetalSize = impetal.width();
			QImage mypetal(brPetalSize, brPetalSize, QImage::Format_ARGB32);
			QPainter ppetal(&mypetal);

			// The petal image is grey scale. We will paint over it in a mode that changes its color to the DNA petal color.
			ppetal.drawImage(0,0, impetal);
			ppetal.setCompositionMode(QPainter::CompositionMode_Multiply);
			ppetal.fillRect(0,0, brPetalSize,brPetalSize, m_DNA.PetalColor());
			ppetal.setCompositionMode(QPainter::CompositionMode_SourceOver);	// default mode
			brPetal[2].setTexture(QPixmap::fromImage(mypetal));

			// Make two more that are progressively darker.
			ppetal.fillRect(0,0, brPetalSize,brPetalSize, QColor(0,0,0,30));
			brPetal[1].setTexture(QPixmap::fromImage(mypetal));

			ppetal.fillRect(0,0, brPetalSize,brPetalSize, QColor(0,0,0,60));
			brPetal[0].setTexture(QPixmap::fromImage(mypetal));
		}

		bool mustHaveFlower = !hasLeaf;	// No leaves means we must have a flower.
		for(DhPlantBranches::const_iterator pit = branches.constBegin(); pit != branches.constEnd(); pit++)
		{
			// Flowers appear at the tips, except spike.
			if(m_DNA.Inflorescence == AuFloraDNA::INFL_Spike)
			{
				// There will be only one branch with Spike, the trunk.
				// Flowers may sprout at each node, except the base of course.
				for(int ib = (*pit).count() - 1; ib > 0; ib--)
				{
					// Test for flower presense at the node.
					if(!mustHaveFlower && dice.Roll(1,3) > (int)m_DNA.FlowerProbability)
						continue;
					mustHaveFlower = false;

					// Calculate petal base location.
					QPointF baseLoc((*pit)[ib].m_Loc.x, (*pit)[ib].m_Loc.y);

					// Calculate petal size increment from tip diameter.
					float nodeDia = (*pit)[ib].m_Dia;
					float petalInc = m_DNA.PetalSizeIncrement(nodeDia, 0);

					PaintFlower(p, baseLoc, petalInc, dice, brPetal, brPetalSize, unitsPerPixel);
					dice.Random();
				}
			}
			else
			{
				// The tip of the branch is a potential flower location.
				// Test for flower presense at the tip.
				if(!mustHaveFlower && dice.Roll(1,3) > (int)m_DNA.FlowerProbability)
					continue;
				mustHaveFlower = false;

				// Calculate petal base location.
				int nodes = (*pit).count();
				QPointF baseLoc((*pit)[nodes-1].m_Loc.x, (*pit)[nodes-1].m_Loc.y);

				// Calculate petal size increment from tip diameter.
				float tipDia = (*pit)[nodes-1].m_Dia;
				float petalInc;
				if(pit == branches.constBegin())
					petalInc = m_DNA.PetalSizeIncrement(tipDia, 0);
				else
					petalInc = m_DNA.PetalSizeIncrement(tipDia, nodes-1);

				PaintFlower(p, baseLoc, petalInc, dice, brPetal, brPetalSize, unitsPerPixel);
				dice.Random();
			}
		}
	}

	// Erase the edges of the image to transparent black to avoid bleedover
	// between adjacent textures.
	p.setCompositionMode(QPainter::CompositionMode_Clear);	// clear mode
	p.resetTransform();
	p.setRenderHint(QPainter::Antialiasing, false);		// need a crisp erase
	p.setPen(QPen(QColor(0,0,0), 0.f));
	p.drawLine(QPoint(0,0), QPoint(0,tile-1));
	p.drawLine(QPoint(0,tile-1), QPoint(tile-1,tile-1));
	p.drawLine(QPoint(tile-1,tile-1), QPoint(tile-1,0));

	// Finish painting in the tile.
	p.end();

	m_TextureSize = unitsPerPixel * tile;
}

// Will expand umbels.
void DeFlora::PaintFlower(QPainter& p, const QPointF& baseLoc, float petalInc, const AuDice& Cdice, QBrush brPetal[3],
		int brPetalSize, float unitsPerPixel) const
{
	AuDice dice(Cdice);
	if(m_DNA.IsUmbel())
	{
		// An umbel places a multitude of smaller flowers in
		// concentric rings. This takes a long time to draw so we draw a
		// single ring around a center flower.

		// umbel flower size
		float sizeInc = petalInc / 3.f;

		// Rings are spaced so flowers overlap. We'll draw outer to inner.
		float ringSpacing;
		if(m_DNA.PetalLength > m_DNA.PetalTaper && m_DNA.PetalLength > m_DNA.PetalWidth)
			ringSpacing = sizeInc * (float)m_DNA.PetalLength;
		else if(m_DNA.PetalWidth > m_DNA.PetalTaper)
			ringSpacing = m_DNA.PetalWidthCalc(sizeInc);
		else
			ringSpacing = sizeInc * (float)m_DNA.PetalTaper;
		ringSpacing *= 1.5;	// Let the centers peek out.

		// Draw flowers in ring, minus center.
		if(m_DNA.Multiples)
		{
			float angInc = PI*2. / (float)m_DNA.Multiples;
			if(m_DNA.Multiples&1)
				angInc *= 2.f;	// Odd number allows skip by 2.
			float flowerAng = (float)dice.Roll(1,360,-1) * DEG2RADf;
			for(unsigned int flower=0; flower<m_DNA.Multiples; flower++)
			{
				AuPoint flowerVec = AuPoint(ringSpacing, 0.).RotateZ(flowerAng);
				QPointF loc(baseLoc + QPointF(flowerVec.x, flowerVec.y));
				PaintOneFlower(p, loc, sizeInc, dice, brPetal, brPetalSize, unitsPerPixel);	// Paint a tiny flower.
				dice.Random();
				flowerAng += angInc;				// Rotate to next position.
			}
		}

		// Place the center flower last so it's on top.
		PaintOneFlower(p, baseLoc, sizeInc, dice, brPetal, brPetalSize, unitsPerPixel);
		dice.Random();
	}
	else
	{
		// Not an umbel so place a single flower.
		PaintOneFlower(p, baseLoc, petalInc, dice, brPetal, brPetalSize, unitsPerPixel);
		dice.Random();
	}
}

void DeFlora::PaintOneFlower(QPainter& p, const QPointF& baseLoc, float petalInc, const AuDice& Cdice, QBrush brPetal[3],
		int brPetalSize, float unitsPerPixel) const
{
	AuDice dice(Cdice);
	float petalLen = petalInc * (float)m_DNA.PetalLength;
	float petalTap = petalInc * (float)m_DNA.PetalTaper;
	float petalWid = m_DNA.PetalWidthCalc(petalInc);
	int ovaRed = (int)m_DNA.FlowerOvaryRed << 5;
	int ovaGreen = (int)m_DNA.FlowerOvaryGreen << 5;
	int ovaBlue = (int)m_DNA.FlowerOvaryBlue << 5;

	// Ovary values.
	float ovarySize;
	if(petalLen > petalWid && petalLen > petalTap)
		ovarySize = petalLen / (2.f + (float)m_DNA.FlowerOvarySize);
	else if(petalWid > petalTap)
		ovarySize = petalWid / (2.f + (float)m_DNA.FlowerOvarySize);
	else
		ovarySize = petalTap / (2.f + (float)m_DNA.FlowerOvarySize);
	QRectF ova(baseLoc.x() - ovarySize, baseLoc.y() - ovarySize, ovarySize * 2.f, ovarySize * 2.f);

	// Inferior ovary.
	if(m_DNA.FlowerOvary == AuFloraDNA::OVARY_Inferior)
	{
		// Inferior ovary is drawn below the petals.
		p.setPen(QPen(QColor(ovaRed/2, ovaGreen/2, ovaBlue/2), 0.f));
		p.setBrush(QColor(ovaRed*2/3, ovaGreen*2/3, ovaBlue*2/3));
		p.drawEllipse(ova);
	}

	// Draw petals.
	unsigned int petals = ((unsigned int)m_DNA.Multiples + 1);
	unsigned int layers = ((unsigned int)m_DNA.PetalMultiples + 2);
	float angInc = (PI*2.f) / (float)petals;					// per petal
	float lyrInc = angInc / (2.f + (float)m_DNA.PetalMultiples);		// per layer
	if(petals&1)
		angInc *= 2.f;	// Odd number allows skip by 2.
	else if(petals == 8)
		angInc *= 3.f;	// 8 allows skip by 3.
	float petalAng = (float)dice.Roll(1,360,-1) * DEG2RADf;	// initial
	for(unsigned int layer=0; layer<layers; layer++)
	{
		petalAng += lyrInc;
		for(unsigned int petal=0; petal<petals; petal++)
		{
			// Calculate angle of petal in radians.
			petalAng += angInc;
			if(petalAng >= PIf*2.f)
				petalAng -= PIf*2.f;

			// Calculate useful petal vectors.
			AuPoint petalDir = AuPoint(1., 0.).RotateZ(petalAng);
			AuPoint petalTipVec = petalDir * petalLen;
			AuPoint petalLVec = AuPoint(petalTap, petalWid/2.).RotateZ(petalAng);
			AuPoint petalRVec = AuPoint(petalTap, -petalWid/2.).RotateZ(petalAng);

			// Move off the connect point so multiples look nicer.
			QPointF loc(baseLoc);
			if(petalWid > 0.)
				loc += QPointF(petalDir.x,petalDir.y) * (petalWid/2.f);

			// Draw the petal with a dark edge.
			QPointF locL = loc + QPointF(petalLVec.x, petalLVec.y);
			QPointF locTip = loc + QPointF(petalTipVec.x, petalTipVec.y);
			QPointF locR = loc + QPointF(petalRVec.x, petalRVec.y);

			float maxPetalSize;
			if(petalWid > petalLen)
				maxPetalSize = petalWid + petalWid;
			else
				maxPetalSize = petalLen + petalWid;
			float brushScale = (maxPetalSize / unitsPerPixel) / brPetalSize;

			// Configure the brush to paint the leaf at the proper location and angle.
			QTransform tpetal;
			tpetal.translate(loc.x(), loc.y());
			tpetal.scale(brushScale, brushScale);
			tpetal.rotate(petalAng * RAD2DEGf);
			tpetal.translate(-(petalWid/2) / brushScale, brPetalSize/2 * unitsPerPixel);
			tpetal.scale(unitsPerPixel, unitsPerPixel);

			// Draw the petal from dark to light.
			float dtlpw = petalWid;
			for(int dtl=0; dtl<3; dtl++)
			{
				brPetal[dtl].setTransform(tpetal);
				p.setPen(QPen(brPetal[dtl], dtlpw, Qt::SolidLine, Qt::RoundCap));
				p.drawLine(loc, locL);
				p.drawLine(locL, locTip);
				p.drawLine(locTip, locR);
				p.drawLine(locR, loc);
				dtlpw *= .95f;
			}
		}
	}

	// Superior ovary.
	if(m_DNA.FlowerOvary == AuFloraDNA::OVARY_Superior)
	{
		// Superior ovary is drawn above the petals.
		p.setPen(QPen(QColor(ovaRed/2, ovaGreen/2, ovaBlue/2), 0.f));
		p.setBrush(QColor(ovaRed*2/3, ovaGreen*2/3, ovaBlue*2/3));
		p.drawEllipse(ova);
	}

	// Draw the pistil.
	QRectF pistil(baseLoc.x() - ovarySize/2., baseLoc.y() - ovarySize/2., ovarySize, ovarySize);
	p.setPen(QPen(QColor(ovaRed/2, ovaGreen/2, ovaBlue/2), 0.f));
	p.setBrush(QColor(ovaRed, ovaGreen, ovaBlue));
	p.drawEllipse(pistil);
	QRectF pistilInner(baseLoc.x() - ovarySize/4., baseLoc.y() - ovarySize/4., ovarySize/2., ovarySize/2.);
	p.setPen(QPen(QColor(ovaRed/2, ovaGreen/2, ovaBlue/2), 0.f));
	p.setBrush(QColor(ovaRed/4, ovaGreen/4, ovaBlue/4));
	p.drawEllipse(pistilInner);

	// Draw the stamens.
	int stamRed = 255 - ovaRed;
	int stamGreen = 255 - ovaGreen;
	int stamBlue = 255 - ovaBlue;
	unsigned int stamens = layers * petals;
	angInc = PI*2. / (float)stamens;
	float stamAng = (float)dice.Roll(1,360,-1) * DEG2RADf;	// initial
	p.setPen(QPen(QColor(stamRed,stamGreen,stamBlue), 0.f));
	for(unsigned int stamen=0; stamen<stamens; stamen++)
	{
		// Calculate angle of petal in radians.
		stamAng += angInc;
		if(stamAng >= PIf*2.f)
			stamAng -= PIf*2.f;

		// Calculate useful petal vectors.
		AuPoint stamStVec = AuPoint(ovarySize/2., 0.).RotateZ(stamAng);
		AuPoint stamEndVec = AuPoint(ovarySize*2./3., 0.).RotateZ(stamAng);
		QPointF locSt = baseLoc + QPointF(stamStVec.x, stamStVec.y);
		QPointF locEnd = baseLoc + QPointF(stamEndVec.x, stamEndVec.y);

		p.drawLine(locSt, locEnd);
	}
}

// Construct a branch for this plant.
void DeFlora::MakeBranch(DhPlantBranch& branch, const AuPoint& baseLoc,
		float dia, const AuPoint& startVec, int maxangle, int maxsegs,
		float mindia, const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	// Add the branch start node.
	AuPoint brloc = baseLoc;
	float brdia = dia;							// Diameter at start node.
	float brlen = m_DNA.BranchLength(brdia);	// Length of base segment.
	branch.AddNode(brloc, brdia, 1);			// We know there will be a
												// branch to the next node.

	// Calculate the direction to the second node point.
	bool isMain;
	AuPoint brvec;
	if(maxangle)
	{
		// This is a side branch, not the trunk.
		isMain = false;

		// Deflect branch vector within initial max branch angle.
		int brang = dice.Roll(1, 2*maxangle+1, -(maxangle+1));
		if(m_DNA.Inflorescence == AuFloraDNA::INFL_ScorpioidCyme)
		{
			// Always bend the same way on scorpioid branch.
			// It is helpful to bend the main branch the opposite direction of the
			// side branches for obvious reasons.
			if(brang > 0)
				brang = -brang;
		}
		brvec = startVec.RotateZ((float)brang * DEG2RADf);
	}
	else
	{
		// This is the main branch.
		isMain = true;
		brvec = startVec;
	}

	// Max angle of subsequent bends is determined by DNA.
	maxangle = 7 + (int)m_DNA.BranchAngle * 6;

	// Stiffness determines branch taper, 15-50%
	float taperFactor = m_DNA.BranchTaperFactor();

	// Add at least one node to the branch.
	do
	{
		// Calculate end node of branch segment.
		brloc += brvec * brlen;
		brdia -= brdia * taperFactor;				// Diameter at end node of branch.
		branch.AddNode(brloc, brdia, 1);

		// Prepare for next node.
		brlen = m_DNA.BranchLength(brdia);			// Length of next branch segment.

		// Deflect branch vector for next segment within allowed max bend angle.
		int brang = dice.Roll(1, 2*maxangle+1, -(maxangle+1));
		if(m_DNA.Inflorescence == AuFloraDNA::INFL_ScorpioidCyme)
		{
			// Always bend the same way on scorpioid branch.
			// It is helpful to bend the main branch the opposite direction of the
			// side branches for obvious reasons.
			if(isMain)
			{
				if(brang < 0)
					brang = -brang;
			}
			else
			{
				if(brang > 0)
					brang = -brang;
			}
		}
		brvec = brvec.RotateZ((float)brang * DEG2RADf);

		// Observe max segment limit.
		maxsegs--;
	}
	while(brdia >= mindia && maxsegs > 0);

	// There aren't any branches emanating from the last node...yet.
	branch.last().m_Branches = 0;
}

// Construct a branching branch for this plant.
void DeFlora::MakeBranchingBranch(DhPlantBranches& branches, DhPlantNode& node,
		float dia, const AuPoint& startVec, int maxangle, int maxsegs,
		float mindia, const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	if(maxsegs <= 0)
		return;

	DhPlantBranch branch;
	MakeBranch(branch, node.m_Loc, dia, startVec, maxangle, maxsegs, mindia, dice);
	dice.Random();
	if(branch.count() < 2)
		return;
	branches.append(branch);
	node.m_Branches++;

	// Add branches at each node that are multiple segments in length, except at
	// the base node. These branches do not branch further.
	AuPoint prevLoc;
	float prevLen = 1;
	for(DhPlantBranch::iterator it = branch.begin(); it != branch.end(); it++)
	{
		AuPoint brloc = (*it).m_Loc;
		float brdia = (*it).m_Dia;

		if(it != branch.begin())
		{
			// Get the unit vector from last node to this node for branch dir.
			AuPoint brvec = brloc - prevLoc;
			if(prevLen != 0)
				brvec /= prevLen;			// Make a unit vector.

			// Assuming a multiple of branches originally sprouted, determine
			// how many of them actually developed into a branch.
			// Note that we've already handled one of them; it is the next
			// segment of the main branch, unless we're at the end.
			unsigned int brs = m_DNA.Multiples;
			if(it == branch.end() - 1)
				brs++;
			for(unsigned int ib=0; ib<brs; ib++)
			{
				// Each multiple branch has a chance of existing.
				if(dice.Roll(1,4,-1) > (int)m_DNA.BranchProbability)
					continue;

				// A branch sprouted.
				int maxangle = 7 + (int)m_DNA.BranchAngle * 6;
				maxangle *= (ib + 1);
				float szb = (float)dice.Roll(1,5,1) * .25f + .5f;
				szb = brdia / szb;
				if(szb < mindia)
					continue;	// skip tiny branches
				// Reduce the number of branches aggressively or else we get plants with 20k branches.
				if(dice.Roll(1,4) == 1)
					MakeBranchingBranch(branches, *it, szb, brvec, maxangle, maxsegs--, mindia, dice);
				else
					MakeBranchingBranch(branches, *it, szb, brvec, maxangle, maxsegs-=2, mindia, dice);
				dice.Random();
			}
		}

		prevLoc = brloc;
		prevLen = m_DNA.BranchLength(brdia);
	}
}

void DeFlora::CreateVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int textureIndex, TYPE type, const AuDice& dice,
		int lod, float maxRadius) const
{
	// Create surface triangles of the VAO. This portion does not need the OpenGL context so it could be done by a different thread.
	QList<AuFloraVAOTri> tris;
	CreateVAOTriangles(tris, textureIndex, type, dice, lod, maxRadius);
	Q_ASSERT(!tris.isEmpty());
	if(tris.isEmpty())
		return;

	// Create the OpenGL VAO from the surface triangles.
	unsigned int vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* vaoPos = new GLfloat[vertices*3];					// vertices for triangles that make up the model.
	GLfloat* vaoN = new GLfloat[vertices*3];					// normals for triangles that make up the model.
	GLfloat* vaoTex = new GLfloat[vertices*3];					// texture coords, move factor

	unsigned int vaoPosIdx = 0;
	unsigned int vaoNIdx = 0;
	unsigned int vaoTexIdx = 0;
	for(QList<AuFloraVAOTri>::const_iterator ti = tris.constBegin(); ti != tris.constEnd(); ti++)
	{
		vaoPos[vaoPosIdx++] = (*ti).m_V0.m_X;
		vaoPos[vaoPosIdx++] = (*ti).m_V0.m_Y;
		vaoPos[vaoPosIdx++] = (*ti).m_V0.m_Z;

		vaoN[vaoNIdx++] = (*ti).m_V0.m_NX;
		vaoN[vaoNIdx++] = (*ti).m_V0.m_NY;
		vaoN[vaoNIdx++] = (*ti).m_V0.m_NZ;

		vaoTex[vaoTexIdx++] = (*ti).m_V0.m_TexX;
		vaoTex[vaoTexIdx++] = (*ti).m_V0.m_TexY;
		vaoTex[vaoTexIdx++] = (*ti).m_V0.m_MoveFactor;

		vaoPos[vaoPosIdx++] = (*ti).m_V1.m_X;
		vaoPos[vaoPosIdx++] = (*ti).m_V1.m_Y;
		vaoPos[vaoPosIdx++] = (*ti).m_V1.m_Z;

		vaoN[vaoNIdx++] = (*ti).m_V1.m_NX;
		vaoN[vaoNIdx++] = (*ti).m_V1.m_NY;
		vaoN[vaoNIdx++] = (*ti).m_V1.m_NZ;

		vaoTex[vaoTexIdx++] = (*ti).m_V1.m_TexX;
		vaoTex[vaoTexIdx++] = (*ti).m_V1.m_TexY;
		vaoTex[vaoTexIdx++] = (*ti).m_V1.m_MoveFactor;

		vaoPos[vaoPosIdx++] = (*ti).m_V2.m_X;
		vaoPos[vaoPosIdx++] = (*ti).m_V2.m_Y;
		vaoPos[vaoPosIdx++] = (*ti).m_V2.m_Z;

		vaoN[vaoNIdx++] = (*ti).m_V2.m_NX;
		vaoN[vaoNIdx++] = (*ti).m_V2.m_NY;
		vaoN[vaoNIdx++] = (*ti).m_V2.m_NZ;

		vaoTex[vaoTexIdx++] = (*ti).m_V2.m_TexX;
		vaoTex[vaoTexIdx++] = (*ti).m_V2.m_TexY;
		vaoTex[vaoTexIdx++] = (*ti).m_V2.m_MoveFactor;
	}

	// Generate vertex array(s) for this object. We only need one.
	vao.CreateGL(GLf, vertices, 3);

	// Select vertex array to construct.
	vao.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, vaoPosIdx*sizeof(GLfloat), vaoPos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for normals and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, vaoNIdx*sizeof(GLfloat), vaoN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, vaoTexIdx*sizeof(GLfloat), vaoTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	vao.DeselectGL();

	delete [] vaoPos;
	delete [] vaoN;
	delete [] vaoTex;
}

void DeFlora::CreateVAOTriangles(QList<AuFloraVAOTri>& tris, unsigned int textureIndex, TYPE type, const AuDice& Cdice, int lod,
		float maxRadius) const
{
	AuDice dice(Cdice);
	// Get information about the texture map for the leaves.
	float s, t, u, v;
	DePlanetFlora::Texture(textureIndex, s, t, u, v);

	switch(type)
	{
	case Tree:
		{
			// Make a tree model.
			float bs, bt, bu, bv;
			DePlanetFlora::Texture(FT_BarkStart + (int)m_DNA.BarkTexture, bs, bt, bu, bv);

			// Calculate a typical branch structure using my DNA.
			// Minimum diameter allows for taper to minimum branch size.
			float tdia = m_DNA.TrunkBaseDiameter();
			float mindia = m_DNA.MinBaseDiameter();
			if(maxRadius > 0)
			{
				float maxdia = maxRadius*2;
				if(tdia > maxdia)
					tdia = maxdia;
				if(mindia > maxdia)
					mindia = maxdia;
			}
			DhPlantBranches branches;
			CreateBranches(branches, tdia, g_Au.TreeTrunkMaxSegs(), mindia, dice);
			dice.Random();

			// Branches are all lying flat.
			// Flip them upright.
			AuMatrix4x4 tstand;
			tstand.RotateX(90. * DEG2RADf);
			branches.Transform(tstand);

			//qDebug("DeFlora::CreateModel tree %d", glModels);
			// Create the model of the woody branches.
			AuPoint prevloc;
			float prevdia = 0;
			for(DhPlantBranches::const_iterator pit = branches.constBegin(); pit != branches.constEnd(); pit++)
			{
				for(DhPlantBranch::const_iterator bit = (*pit).constBegin(); bit != (*pit).constEnd(); bit++)
				{
					AuPoint curloc = (*bit).m_Loc;
					float curdia = (*bit).m_Dia;
					if(bit != (*pit).constBegin())
					{
						// This is not the first node. Draw a log.
						CreateBranchVAOTriangles(tris, prevloc, prevdia, curloc, curdia, bs,bt,bu,bv, lod);
					}
					prevloc = curloc;
					prevdia = curdia;
				}
			}

			// Add the leaves.
			bool anyleaves = false;
			for(DhPlantBranches::const_iterator pit = branches.constBegin(); pit != branches.constEnd(); pit++)
			{
				for(DhPlantBranch::const_iterator bit = (*pit).constBegin(); bit != (*pit).constEnd(); bit++)
				{
					AuPoint curloc = (*bit).m_Loc;
					float curdia = (*bit).m_Dia;
					if(bit != (*pit).constBegin())
					{
						// Not the first node. Check for leaves.
						int leaves = ((int)m_DNA.Multiples + 1) - (*bit).m_Branches;
						for(int leaf=0; leaf<leaves; leaf++)
						{
							// Each multiple branch has a chance of existing.
							// Compound umbel always has all of them.
							if(m_DNA.Inflorescence != AuFloraDNA::INFL_UmbelCompound &&
									dice.Roll(1,4,-1) > (int)m_DNA.BranchProbability)
							{
								// When we reach the tip of the trunk and we still
								// don't have any leaves, we do not 'continue'.
								// Otherwise we sometimes get a little stick
								// sticking out of the ground with no foliage.
								if(anyleaves ||
										pit != branches.constBegin() ||
										bit != (*pit).constEnd() - 1)
									continue;
							}

							// A leafy branch sprouted.
							AuPoint dir(curloc - prevloc);
							dir.Unit();
							int maxangle = 7 + (int)m_DNA.BranchAngle * 6;
							maxangle *= ((*bit).m_Branches + leaf + 1);
							float brang = dice.Roll(1, 2*maxangle+1, -(maxangle+1));
							AuPoint brvec = dir.RotateY(brang * DEG2RADf);
							float axang = (float)dice.Roll(1, 360, -1);
							AuMatrix4x4 tb;
							tb.RotateAxis(-axang * DEG2RADf, dir);
							brvec = tb.Project(brvec);

							float szb = (float)dice.Roll(1,5,1) * .25f + .5f;
							szb = curdia / szb;
							if(szb < m_DNA.MinBranchDiameter()/2)
								continue;
							CreateBranchVAOTriangles(tris, curloc, szb, brvec, s,t,u,v, lod);
							anyleaves = true;	// we've added some foliage
						}
					}
					prevloc = curloc;
					prevdia = curdia;
				}
			}
		}
		break;
	case Shrub:
		{
			// Make a shrub model.
			//qDebug("DeFlora::CreateModel shrub %d", glModels);
			int sprouts = (int)m_DNA.Multiples * (int)m_DNA.SproutMultiples;
			if(!sprouts)
				sprouts = 1;	// At least one sprout.
			float sproutRad = m_DNA.TrunkBaseDiameter() * ((float)sprouts/4.f);
			if(maxRadius > 0 && sproutRad > maxRadius)
				sproutRad = maxRadius;
			for(int sprout=0; sprout<sprouts; sprout++)
			{
				int radroll = dice.Roll(1,100);		// percent of radius away
				float scl = (float)radroll / 100.f;
				float sproutAt = sproutRad * scl;

				int angroll = dice.Roll(1,360,-1);	// degree angle
				float ang = (float)angroll * DEG2RADf;
				AuPoint vec = AuPoint(sproutAt,0.).RotateZ(ang);

				AuMatrix4x4 bm;
				bm.Translate(vec);
				bm.RotateZ(ang);
				bm.Scale(scl,1.,1.-scl/2.);
				DrawBranchVAOTriangles(tris, s,t,u,v, bm, lod);
			}
		}
		break;
	case GroundCover:
		{
			// Make a ground cover model.
			//qDebug("DeFlora::CreateModel shrub %d", glModels);

			// Project the normal vector so we can get a sense of the scaling.
			// We need it to calculate the wind movement distance.
			float windMoveDist = 1.;				// vertex move distance at force 1
			windMoveDist /= ((float)m_DNA.Stiffness + 2)/2.;	// move less when stiffer

			const float hmarg = 1.f/(FT_PaletteWidth*g_Au.PlantTextureSize());
			const float vmarg = 1.f/(FT_PaletteHeight*g_Au.PlantTextureSize());
			float sproutRad = 2.f;
			if(maxRadius > 0 && sproutRad > maxRadius*.75f)
				sproutRad = maxRadius*.75f;
			const float sproutSize = sproutRad;
			//qDebug() << "sproutRad" << sproutRad << "sproutSize" << sproutSize << "maxRad" << maxRadius;
			float ss = 0, st = 0, su = 0, sv = 0;
			int subtex = dice.Roll(1,4,-1);
			//const unsigned int smul = m_DNA.SproutMultiples+1;	// 1-8
			//const unsigned int mul = m_DNA.Multiples+1;			// 1-4
			//const unsigned int smul = 8;	// Max for ground cover.
			//const unsigned int mul = 4;		// Max for ground cover.
			const unsigned int smul = m_DNA.SproutMultiples/2+2+(lod-1)*3;	// 5-8 high, 2-5 low
			const unsigned int mul = m_DNA.Multiples/2+1+(lod-1)*2;			// 3-4 high, 1-2 low
			for(unsigned int it=0; it<smul; it++)
			{
				for(unsigned int sprout=0; sprout<mul; sprout++)
				{
					// Calc texture coords for quarter of tile assigned to ground cover.
					switch(subtex%4)
					{
					case 0:
						ss = s;
						st = t;
						su = (s + u)/2. - hmarg;
						sv = (t + v)/2. + vmarg;
						break;
					case 1:
						ss = (s + u)/2. + hmarg;
						st = t;
						su = u;
						sv = (t + v)/2. + vmarg;
						break;
					case 2:
						ss = s;
						st = (t + v)/2.;
						su = (s + u)/2.;
						sv = v;
						break;
					case 3:
						ss = (s + u)/2.;
						st = (t + v)/2.;
						su = u;
						sv = v;
						break;
					}
					subtex++;

					int radroll = dice.Roll(1,100);	// percent of radius away
					float scl = (float)radroll / 100.f;
					float sproutAt = sproutRad * scl;

					int angroll = dice.Roll(1,360,-1);	// degree angle
					float ang = (float)angroll * DEG2RADf;
					AuPoint vec = AuPoint(sproutAt,0.).RotateZ(ang);

					angroll = dice.Roll(1,360,-1);		// degree angle for rot
					ang = (float)angroll * DEG2RADf;
					AuPoint dir = AuPoint(1,0).RotateZ(ang);

					//AuPoint vnf = AuPoint(0, -.7071, .7071).RotateZ(ang);	// Front surface normal pointing up at 45 degree angle
					//AuPoint vnb = AuPoint(0, .7071, .7071).RotateZ(ang);	// Back surface normal pointing up at 45 degree angle
					AuPoint vnf = AuPoint(0, -1, 0).RotateZ(ang);	// Front surface normal.
					AuPoint vnb = AuPoint(0, 1, 0).RotateZ(ang);	// Back surface normal.

					AuPoint ll, lr, ur, ul;
					ll = vec - dir*sproutSize/2.f;
					lr = vec + dir*sproutSize/2.f;
					ul = ll;
					ul.z += sproutSize;
					ur = lr;
					ur.z += sproutSize;

					// Draw triangles on each side of the grass so it is visible from both sides, each with its own surface normal.
					// Front vertices.
					AuFloraVAOVert vllf(ll.x,ll.y,ll.z, vnf.x,vnf.y,vnf.z, ss,st, 0);
					AuFloraVAOVert vlrf(lr.x,lr.y,lr.z, vnf.x,vnf.y,vnf.z, su,st, 0);
					AuFloraVAOVert vulf(ul.x,ul.y,ul.z, vnf.x,vnf.y,vnf.z, ss,sv, windMoveDist);
					AuFloraVAOVert vurf(ur.x,ur.y,ur.z, vnf.x,vnf.y,vnf.z, su,sv, windMoveDist);

					// Front triangles.
					tris.append(AuFloraVAOTri(vllf, vulf, vurf));
					tris.append(AuFloraVAOTri(vurf, vlrf, vllf));

					// Back vertices
					AuFloraVAOVert vllb(lr.x,lr.y,lr.z, vnb.x,vnb.y,vnb.z, su,st, 0);
					AuFloraVAOVert vlrb(ll.x,ll.y,ll.z, vnb.x,vnb.y,vnb.z, ss,st, 0);
					AuFloraVAOVert vulb(ur.x,ur.y,ur.z, vnb.x,vnb.y,vnb.z, su,sv, windMoveDist);
					AuFloraVAOVert vurb(ul.x,ul.y,ul.z, vnb.x,vnb.y,vnb.z, ss,sv, windMoveDist);

					// Back triangles.
					tris.append(AuFloraVAOTri(vllb, vulb, vurb));
					tris.append(AuFloraVAOTri(vurb, vlrb, vllb));
				}
			}
		}
		break;
	}
}

float DeFlora::CreateBranches(DhPlantBranches& branches, float dia, int maxsegs, float mindia, const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	// Make a trunk segment.
	DhPlantBranch trunk;
	MakeBranch(trunk, AuPoint(0,0), dia, AuPoint(0,1), 0, maxsegs, mindia, dice);
	dice.Random();
	if(trunk.count() < 2)
		return 0.f;

	// Add more branches based on inflorescence.
	switch(m_DNA.Inflorescence)
	{
	case AuFloraDNA::INFL_Spike:
		// Spike does not add more side branches. The node points along the main
		// trunk will sprout leaf clusters and/or flowers.
		break;
	case AuFloraDNA::INFL_Raceme:
	case AuFloraDNA::INFL_ScorpioidCyme:
		{
			// Add branches at each node that are one segment in length, except
			// at the base node. These branches do not branch further.
			//qDebug("Raceme or ScorpioidCyme");
			AuPoint prevLoc;
			float prevLen = 1;
			for(DhPlantBranch::iterator it = trunk.begin(); it != trunk.end(); it++)
			{
				AuPoint brloc = (*it).m_Loc;
				float brdia = (*it).m_Dia;

				if(it != trunk.begin())
				{
					// Get unit vector from last node to this node for branch
					// dir.
					AuPoint brvec = brloc - prevLoc;
					if(prevLen != 0)
						brvec /= prevLen;			// Make a unit vector.

					// Assuming a multiple of branches originally sprouted,
					// determine how many of them actually developed into a
					// branch.
					// Note that we've already handled one of them; it is the
					// next segment of the main branch.
					unsigned int brs = m_DNA.Multiples;
					if(it == trunk.end() - 1)
						brs++;
					for(unsigned int ib=0; ib<brs; ib++)
					{
						// Each multiple branch has a chance of existing.
						if(dice.Roll(1,4,-1) > (int)m_DNA.BranchProbability)
							continue;

						// A branch sprouted.
						int maxangle = 7 + (int)m_DNA.BranchAngle * 6;
						maxangle *= (ib + 1);
						float szb = (float)dice.Roll(1,5,1) * .25f + .5f;
						szb = brdia / szb;
						if(szb < mindia)
							continue;	// skip tiny branches
						DhPlantBranch branch;
						MakeBranch(branch, brloc, szb, brvec, maxangle, 1, mindia, dice);
						dice.Random();
						if(branch.count() > 1)
						{
							branches.append(branch);
							(*it).m_Branches++;
						}
					}
				}

				prevLoc = brloc;
				prevLen = m_DNA.BranchLength(brdia);
			}
			//qDebug(" raceme or scorpioid cyme created %d branches", branches.count());
		}
		break;
	case AuFloraDNA::INFL_Corymb:
	case AuFloraDNA::INFL_Cyme:
	case AuFloraDNA::INFL_UmbelCompound:
		{
			// Add branches at each node that are multiple segments in length,
			// except at the base node. These branches do not branch further.
			//qDebug("Corymb, Cyme or UmbelCompound");
			AuPoint prevLoc;
			float prevLen = 1;
			for(DhPlantBranch::iterator it = trunk.begin(); it != trunk.end(); it++)
			{
				AuPoint brloc = (*it).m_Loc;
				float brdia = (*it).m_Dia;

				if(it != trunk.begin())
				{
					// Get unit vector from last node to this node for branch dir.
					AuPoint brvec = brloc - prevLoc;
					if(prevLen != 0)
						brvec /= prevLen;			// Make a unit vector.

					// Assuming a multiple of branches originally sprouted,
					// determine how many of them actually developed into a branch.
					// Note that we've already handled one of them; it is the next
					// segment of the main branch.
					unsigned int brs = m_DNA.Multiples;
					if(it == trunk.end() - 1)
						brs++;
					float szb = 1;
					for(unsigned int ib=0; ib<brs; ib++)
					{
						// Each multiple branch has a chance of existing.
						// Compound Umbel always retains every multiple in my world.
						if(m_DNA.Inflorescence != AuFloraDNA::INFL_UmbelCompound &&
							dice.Roll(1,4,-1) > (int)m_DNA.BranchProbability)
							continue;

						// A branch sprouted.
						int maxangle = 7 + (int)m_DNA.BranchAngle * 6;
						maxangle *= (ib + 1);

						// Keep all compound umbels the same size at the node by
						// only rolling the size the first time through. Other
						// types roll the size for each branch at the node.
						if(m_DNA.Inflorescence != AuFloraDNA::INFL_UmbelCompound || ib == 0)
						{
							szb = (float)dice.Roll(1,5,1) * .25f + .5f;
							szb = brdia / szb;
							if(szb < mindia)
								continue;	// skip tiny branches
						}
						DhPlantBranch branch;
						MakeBranch(branch, brloc, szb, brvec, maxangle, maxsegs--, mindia, dice);
						dice.Random();
						if(branch.count() > 1)
						{
							branches.append(branch);
							(*it).m_Branches++;
						}
					}
				}

				prevLoc = brloc;
				prevLen = m_DNA.BranchLength(brdia);
			}
			//qDebug(" corymb, cyme or compound umbel created %d branches", branches.count());
		}
		break;
	case AuFloraDNA::INFL_Panicle:
	case AuFloraDNA::INFL_UmbelSimple:
		{
			// Add branches at each node that are multiple segments in length,
			// except at the base node. These branches branch further. This is
			// the most complex branching style.
			//qDebug("Panicle or UmbelSimple");
			AuPoint prevLoc;
			float prevLen = 1;
			for(DhPlantBranch::iterator it = trunk.begin(); it != trunk.end(); it++)
			{
				AuPoint brloc = (*it).m_Loc;
				float brdia = (*it).m_Dia;

				if(it != trunk.begin())
				{
					// Get unit vector from last node to this node for branch
					// dir.
					AuPoint brvec = brloc - prevLoc;
					if(prevLen != 0)
						brvec /= prevLen;			// Make a unit vector.

					// Assuming a multiple of branches originally sprouted,
					// determine how many of them actually developed into a
					// branch.
					// Note that we've already handled one of them; it is the
					// next segment of the main branch.
					unsigned int brs = m_DNA.Multiples;
					if(it == trunk.end() - 1)
						brs++;
					for(unsigned int ib=0; ib<brs; ib++)
					{
						// Each multiple branch has a chance of existing.
						if(dice.Roll(1,4,-1) > (int)m_DNA.BranchProbability)
							continue;

						// A branch sprouted.
						int maxangle = 7 + (int)m_DNA.BranchAngle * 6;
						maxangle *= (ib + 1);
						float szb = (float)dice.Roll(1,5,1) * .25f + .5f;
						szb = brdia / szb;
						if(szb < mindia)
							continue;	// skip tiny branches
						MakeBranchingBranch(branches, *it, szb, brvec, maxangle, maxsegs--, mindia, dice);
						dice.Random();
					}
				}

				prevLoc = brloc;
				prevLen = m_DNA.BranchLength(brdia);
			}
			//qDebug(" panicle or simple umbel created %d branches", branches.count());
		}
		break;
	}

	// Append trunk after branch count at nodes has been changed by adding
	// branches.
	branches.append(trunk);

	// Return the diameter of the ending tip of the trunk segment.
	float tipdia = trunk.last().m_Dia;
	return tipdia;
}

// Draws a branch using 2 quads for the top and 2 quads for the bottom.
void DeFlora::DrawBranchVAOTriangles(QList<AuFloraVAOTri>& tris, float s, float t, float u, float v,
		const AuMatrix4x4& bm, int lod) const
{
	// Create branch display list so it can be used by the main display list.
	// The branch is the same, whether its a tree or a shrub.
	float rht = m_TextureSize;	// Nominal height.
	float rleafrad = rht/2.;

	// Branch is drawn on two quads so it can be bent according to its stiffness.
	float bendang = m_DNA.BranchBendFactor() * (90.f * DEG2RADf) * 2.f;
	AuPoint branch[6];
	AuPoint norm[2];
	AuPoint normal(1,0,0);
	AuPoint upvec(0,0,rht/2.);	// Vec for each section up.
	branch[0] = AuPoint(0,-rleafrad,0);
	branch[1] = AuPoint(0,rleafrad,0);
	normal = normal.RotateY(bendang);
	upvec = upvec.RotateY(bendang);
	norm[0] = normal;
	branch[2] = branch[0] + upvec;
	branch[3] = branch[1] + upvec;
	normal = normal.RotateY(bendang);
	upvec = upvec.RotateY(bendang);
	norm[1] = normal;
	branch[4] = branch[2] + upvec;
	branch[5] = branch[3] + upvec;

	// Calculate the wind movement distance per unit height.
	float windmovedist = .25;	// move distance at force 1
	windmovedist /= ((float)m_DNA.Stiffness + 2)/2.;	// move less when stiff
	float windmove[3];
	for(int iv=0; iv<6; iv+=2)
	{
		AuPoint bv = bm.Project(branch[iv]);
		windmove[iv/2] = bv.z * windmovedist;
	}
	windmove[0] *= .5;	// woody branches end moves less than leaves
	windmove[1] *= .75;	// woody branches end moves less than leaves

	// Project the normals and vertices through the branch matrix.
	for(int in=0; in<2; in++)
		norm[in] = bm.ProjectVector(norm[in]);	// The shader normalizes these.
	for(int iv=0; iv<6; iv++)
		branch[iv] = bm.Project(branch[iv]);

	// Calculate intermediate vertical texture coords.
	float v3 = (v - t)/2.;
	float tv1 = t + v3;

	if(lod == 1)
	{
		// Only one quad.
		// Top vertices.
		AuFloraVAOVert b0t(branch[0].x, branch[0].y, branch[0].z, -norm[0].x, -norm[0].y, -norm[0].z, s,t, windmove[0]);
		AuFloraVAOVert b1t(branch[1].x, branch[1].y, branch[1].z, -norm[0].x, -norm[0].y, -norm[0].z, u,t, windmove[0]);
		AuFloraVAOVert b4t(branch[4].x, branch[4].y, branch[4].z, -norm[1].x, -norm[1].y, -norm[1].z, s,v, windmove[2]);
		AuFloraVAOVert b5t(branch[5].x, branch[5].y, branch[5].z, -norm[1].x, -norm[1].y, -norm[1].z, u,v, windmove[2]);

		// Top triangles.
		tris.append(AuFloraVAOTri(b0t, b1t, b4t));
		tris.append(AuFloraVAOTri(b4t, b1t, b5t));

		// Bottom vertices.
		AuFloraVAOVert b0b(branch[1].x, branch[1].y, branch[1].z, norm[0].x, norm[0].y, norm[0].z, u,t, windmove[0]);
		AuFloraVAOVert b1b(branch[0].x, branch[0].y, branch[0].z, norm[0].x, norm[0].y, norm[0].z, s,t, windmove[0]);
		AuFloraVAOVert b4b(branch[5].x, branch[5].y, branch[5].z, norm[1].x, norm[1].y, norm[1].z, u,v, windmove[2]);
		AuFloraVAOVert b5b(branch[4].x, branch[4].y, branch[4].z, norm[1].x, norm[1].y, norm[1].z, s,v, windmove[2]);

		// Bottom triangles.
		tris.append(AuFloraVAOTri(b0b, b1b, b4b));
		tris.append(AuFloraVAOTri(b4b, b1b, b5b));
	}
	else
	{
		// Top vertices.
		AuFloraVAOVert b0t(branch[0].x, branch[0].y, branch[0].z, -norm[0].x, -norm[0].y, -norm[0].z, s,t, windmove[0]);
		AuFloraVAOVert b1t(branch[1].x, branch[1].y, branch[1].z, -norm[0].x, -norm[0].y, -norm[0].z, u,t, windmove[0]);
		AuFloraVAOVert b2t(branch[2].x, branch[2].y, branch[2].z, -norm[1].x, -norm[1].y, -norm[1].z, s,tv1, windmove[1]);
		AuFloraVAOVert b3t(branch[3].x, branch[3].y, branch[3].z, -norm[1].x, -norm[1].y, -norm[1].z, u,tv1, windmove[1]);
		AuFloraVAOVert b4t(branch[4].x, branch[4].y, branch[4].z, -norm[1].x, -norm[1].y, -norm[1].z, s,v, windmove[2]);
		AuFloraVAOVert b5t(branch[5].x, branch[5].y, branch[5].z, -norm[1].x, -norm[1].y, -norm[1].z, u,v, windmove[2]);

		// Top triangles.
		tris.append(AuFloraVAOTri(b0t, b1t, b2t));
		tris.append(AuFloraVAOTri(b2t, b1t, b3t));
		tris.append(AuFloraVAOTri(b2t, b3t, b4t));
		tris.append(AuFloraVAOTri(b4t, b3t, b5t));

		// Bottom vertices.
		AuFloraVAOVert b0b(branch[1].x, branch[1].y, branch[1].z, norm[0].x, norm[0].y, norm[0].z, u,t, windmove[0]);
		AuFloraVAOVert b1b(branch[0].x, branch[0].y, branch[0].z, norm[0].x, norm[0].y, norm[0].z, s,t, windmove[0]);
		AuFloraVAOVert b2b(branch[3].x, branch[3].y, branch[3].z, norm[1].x, norm[1].y, norm[1].z, u,tv1, windmove[1]);
		AuFloraVAOVert b3b(branch[2].x, branch[2].y, branch[2].z, norm[1].x, norm[1].y, norm[1].z, s,tv1, windmove[1]);
		AuFloraVAOVert b4b(branch[5].x, branch[5].y, branch[5].z, norm[1].x, norm[1].y, norm[1].z, u,v, windmove[2]);
		AuFloraVAOVert b5b(branch[4].x, branch[4].y, branch[4].z, norm[1].x, norm[1].y, norm[1].z, s,v, windmove[2]);

		// Bottom triangles.
		tris.append(AuFloraVAOTri(b0b, b1b, b2b));
		tris.append(AuFloraVAOTri(b2b, b1b, b3b));
		tris.append(AuFloraVAOTri(b2b, b3b, b4b));
		tris.append(AuFloraVAOTri(b4b, b3b, b5b));
	}
}

// Creates a thick 3d tree branch.
void DeFlora::CreateBranchVAOTriangles(QList<AuFloraVAOTri>& tris, const AuPoint& loc0, float dia0, const AuPoint& loc1,
		float dia1, float bs, float bt, float bu, float bv, int lod) const
{
	// Align to the branch.
	AuPoint vb(loc1 - loc0);
	AuPoint vbu(vb);
	vbu.Unit();
	AuMatrix4x4 t = AuPoint(0,0,1).GetAlignmentTransform(vbu);

	// NOTE: These used to be named rad0 and rad1 but that caused build errors in Windows!
	float rada = dia0/2.f;
	float radb = dia1/2.f;
	AuPoint tip0 = loc0 - vbu*(rada/2.);
	AuPoint tip1 = loc1 + vbu*(radb/2.);

	// Wind movement parameters.
	float windmovedist = .125f;	// move distance at force 1, less than leaf move
	windmovedist /= ((float)m_DNA.Stiffness + 2)/2.;	// move less when stiff

	int trunkFaces = g_Au.TreeTrunkFaces();
	if(lod == 1)
	{
		trunkFaces /= 2;
		if(trunkFaces < 3)
			trunkFaces = 3;
	}

	// Create a 3D solid branch.
	AuFloraVAOVert blast;
	AuFloraVAOVert elast;
	for(int face=0; face<=trunkFaces; face++)
	{
		// Calculate the angle to the next vertex.
		float angle;											// Angle to the next vertex.
		float texX;												// Texture X value to wrap the texture tile around the branch.
		if(face == 0)
		{
			// First vertex is easy.
			angle = 0;
			texX = bs;
		}
		else if(face == trunkFaces)
		{
			// For perfect closure at the last vertex.
			angle = 0;	
			texX = bu;
		}
		else
		{
			// Intermediate vertex needs a little math.
			float frac = (float)face / (float)trunkFaces;
			angle = frac * (PIf*2);
			texX = bs + (bu - bs) * frac;
		}

		float c = cosf(angle);
		float s = sinf(angle);

		AuPoint b = tip0 + t.Project(AuPoint(c * rada, s * rada));
		AuPoint e = tip1 + t.Project(AuPoint(c * radb, s * radb));
		AuPoint n = t.Project(AuPoint(c, s));

		// Calculate the wind movement distance per unit height.
		float bmove = b.z * windmovedist;
		float emove = e.z * windmovedist;

		// Trunk vertices
		AuFloraVAOVert bcur(b.x,b.y,b.z, n.x,n.y,n.z, texX,bt, bmove);
		AuFloraVAOVert ecur(e.x,e.y,e.z, n.x,n.y,n.z, texX,bv, emove);
		if(face)
		{
			// Trunk triangles.
			tris.append(AuFloraVAOTri(blast, elast, ecur));
			tris.append(AuFloraVAOTri(ecur, bcur, blast));
		}

		blast = bcur;
		elast = ecur;
	}
}

void DeFlora::CreateBranchVAOTriangles(QList<AuFloraVAOTri>& tris, const AuPoint& loc, float dia, AuPoint& dir,
		float s, float t, float u, float v, int lod) const
{
	// Align to the branch.
	float yrot = PI/2. - dir.PolarIncline();
	float zrot = dir.ArcTangent_2D();

	// Scale to branch node size.
	float tdia = m_DNA.TrunkBaseDiameter() * m_DNA.BranchTaperFactor();
	float scl = dia/tdia;

	AuMatrix4x4 bm;
	bm.Translate(loc);
	bm.RotateZ(zrot);
	bm.RotateY(yrot);
	bm.Scale(scl,scl,scl);
	DrawBranchVAOTriangles(tris, s,t,u,v, bm, lod);
}

void DeFlora::StoreTextureSize(QDataStream& out) const
{
	out << m_TextureSize;
	//qDebug() << "DeFlora::StoreTextureSize" << m_TextureSize;
}

void DeFlora::LoadTextureSize(QDataStream& in)
{
	in >> m_TextureSize;
	//qDebug() << "DeFlora::LoadTextureSize" << m_TextureSize;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DeTerrainFlora::DeTerrainFlora()
{
}

DeTerrainFlora::~DeTerrainFlora()
{
}

void DeTerrainFlora::Init(AuFloraDNA::ZONE zone, const AuDice& Cdice, int sizemod)
{
	AuDice dice(Cdice);
	// Create a DNA for the ground cover.
	m_GroundCover.Init(zone, dice, sizemod);
	dice.Random();

	// Create a DNA for each of our tree table positions.
	// The positions correspond to the commodity type that can be acquired from the tree, besides wood.
	for(int tpos=0; tpos<ZONE_TREES; tpos++)
	{
		m_Trees[tpos].Init(zone, dice, sizemod);
		dice.Random();
	}

	// Create a DNA for each of our shrub table positions.
	// The positions correspond to the commodity type that can be acquired from the shrub, besides vegetation.
	for(int spos=0; spos<ZONE_SHRUBS; spos++)
	{
		m_Shrubs[spos].Init(zone, dice, sizemod);
		dice.Random();
	}
}

// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
void DeTerrainFlora::Init(int plantType, AuFloraDNA::ZONE zone, const AuDice& Cdice, int sizemod)
{
	AuDice dice(Cdice);
	switch(plantType)
	{
	case 0:	// ground cover
		// Create a DNA for the ground cover.
		m_GroundCover.Init(zone, dice, sizemod);
		break;
	case 1:	// shrubs
		// Create a DNA for each of our shrub table positions.
		// The positions correspond to the commodity type that can be acquired from the shrub, besides vegetation.
		for(int spos=0; spos<ZONE_SHRUBS; spos++)
		{
			m_Shrubs[spos].Init(zone, dice, sizemod);
			dice.Random();
		}
		break;
	case 2:	// trees
		// Create a DNA for each of our tree table positions.
		// The positions correspond to the commodity type that can be acquired from the tree, besides wood.
		for(int tpos=0; tpos<ZONE_TREES; tpos++)
		{
			m_Trees[tpos].Init(zone, dice, sizemod);
			dice.Random();
		}
		break;
	}
}

void DeTerrainFlora::operator<<(QDataStream& in)
{
	m_GroundCover << in;
	for(int tpos=0; tpos<ZONE_TREES; tpos++)
		m_Trees[tpos] << in;
	for(int spos=0; spos<ZONE_SHRUBS; spos++)
		m_Shrubs[spos] << in;
}

void DeTerrainFlora::operator>>(QDataStream& out) const
{
	m_GroundCover >> out;
	for(int tpos=0; tpos<ZONE_TREES; tpos++)
		m_Trees[tpos] >> out;
	for(int spos=0; spos<ZONE_SHRUBS; spos++)
		m_Shrubs[spos] >> out;
}

void DeTerrainFlora::TxUpdate(QDataStream& toClient) const
{
	m_GroundCover.TxUpdate(toClient);
	for(int tpos=0; tpos<ZONE_TREES; tpos++)
		m_Trees[tpos].TxUpdate(toClient);
	for(int spos=0; spos<ZONE_SHRUBS; spos++)
		m_Shrubs[spos].TxUpdate(toClient);
}

void DeTerrainFlora::RxUpdate(QDataStream& fromServer)
{
	m_GroundCover.RxUpdate(fromServer);
	for(int tpos=0; tpos<ZONE_TREES; tpos++)
		m_Trees[tpos].RxUpdate(fromServer);
	for(int spos=0; spos<ZONE_SHRUBS; spos++)
		m_Shrubs[spos].RxUpdate(fromServer);
}

const DeFlora& DeTerrainFlora::Shrub(COMMODITYID idCommodity) const
{
	return m_Shrubs[ShrubIndex(idCommodity)];
}

const DeFlora& DeTerrainFlora::Tree(COMMODITYID idCommodity) const
{
	return m_Trees[TreeIndex(idCommodity)];
}

void DeTerrainFlora::PaintGroundCover(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone,
		const AuTime& floraChange)
{
	// We will be seeding the random number generator before each plant so
	// changes to an individual plant don't bleed over onto other plants.
	int seed = genSeed + resourceZone * 100000;

	// Create an image to use when painting each branch tile.
	im = QImage(g_Au.PlantTextureSize(), g_Au.PlantTextureSize(), QImage::Format_ARGB32);

	// Seed the random number generator for the ground cover.
	// Ground cover is drawn first so the ground cover alone can be regenerated with the same result. This is used by planets to
	// create their surface 'grass' models.
	AuDice dice;
	dice.Seed(seed);
	m_GroundCover.PaintGroundCover(im, genSeed, zone, dice, floraChange);
}

void DeTerrainFlora::CalculateTextureSizes(quint32 genSeed, int resourceZone)
{
	// We will be seeding the random number generator before each plant so
	// changes to an individual plant don't bleed over onto other plants.
	int seed = genSeed + resourceZone * 100000;

	AuDice dice;
	dice.Seed(seed);
	m_GroundCover.CalculateTextureSizes(dice);

	// Shrubs
	for(int spos = 0; spos<ZONE_SHRUBS; spos++)
	{
		// Seed the random number generator for each plant.
		dice.Seed(seed+=54321);

		// Paint the branch into the tile image.
		m_Shrubs[spos].CalculateTextureSizes(dice);
	}

	// Trees
	for(int tpos = 0; tpos<ZONE_TREES; tpos++)
	{
		// Seed the random number generator for each plant.
		dice.Seed(seed+=54321);

		// Paint the branch into the tile image.
		m_Trees[tpos].CalculateTextureSizes(dice);
	}
}

void DeTerrainFlora::PaintBranches(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone,
		const AuTime& floraChange)
{
	// We will be seeding the random number generator before each plant so
	// changes to an individual plant don't bleed over onto other plants.
	int seed = genSeed + resourceZone * 100000;

	// Calculate starting y pixel position for my batch of shrub and tree texture tiles.
	// There are ZONE_TREES trees, ZONE_SHRUBS shrubs, and 1 grass in each zone.
	const int height = FT_PaletteHeight * g_Au.PlantTextureSize();
	const int plantsPerZone = ZONE_TREES + ZONE_SHRUBS + 1;
	const int heightNeeded = plantsPerZone * g_Au.PlantTextureSize();
	int xpos = 0;
	int ypos = zone * heightNeeded;
	while(ypos >= height)
	{
		xpos += g_Au.PlantTextureSize();	// Next column
		ypos -= height;
	}

	// Create an image to use when painting each branch tile.
	QImage imbranch(g_Au.PlantTextureSize(), g_Au.PlantTextureSize(), QImage::Format_ARGB32);

	// Create a painter to use for copying the completed branch tiles to the strip.
	QPainter p(&im);

	// Seed the random number generator for the ground cover.

	// Ground cover is painted first so the ground cover alone can be regenerated with the same result. This is used by planets to
	// create their surface 'grass' models.
	AuDice dice;
	dice.Seed(seed);
	QImage imgrass(g_Au.PlantTextureSize(), g_Au.PlantTextureSize(), QImage::Format_ARGB32);
	m_GroundCover.PaintGroundCover(imgrass, genSeed, zone, dice, floraChange);

	// Shrubs
	for(int spos = 0; spos<ZONE_SHRUBS; spos++)
	{
		// Seed the random number generator for each plant.
		dice.Seed(seed+=54321);

		// Paint the branch into the tile image.
		m_Shrubs[spos].PaintBranch(imbranch, zone, dice, dice.Roll(1,s_LeafBrushImages), dice.Roll(1,s_PetalBrushImages));

		// Copy the tile image into the strip at the proper position.
		p.drawImage(xpos,ypos, imbranch);
		ypos += g_Au.PlantTextureSize();
		if(ypos >= height)
		{
			xpos += g_Au.PlantTextureSize();	// Next column
			ypos = 0;
		}
	}

	// Trees
	for(int tpos = 0; tpos<ZONE_TREES; tpos++)
	{
		// Seed the random number generator for each plant.
		dice.Seed(seed+=54321);

		// Paint the branch into the tile image.
		m_Trees[tpos].PaintBranch(imbranch, zone, dice, dice.Roll(1,s_LeafBrushImages), dice.Roll(1,s_PetalBrushImages));

		// Copy the tile image into the strip at the proper position.
		p.drawImage(xpos,ypos, imbranch);
		ypos += g_Au.PlantTextureSize();
		if(ypos >= height)
		{
			xpos += g_Au.PlantTextureSize();	// Next column
			ypos = 0;
		}
	}

	p.drawImage(xpos,ypos, imgrass);	// Grass is placed at the end.

	// Finish painting in the tile palette.
	p.end();
}

void DeTerrainFlora::PaintPottedBranches(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, int plantType,
		COMMODITYID idCommodity)
{
	// We will be seeding the random number generator before each plant so
	// changes to an individual plant don't bleed over onto other plants.
	int seed = genSeed + resourceZone * 100000;

	// Calculate starting y pixel position for my batch of shrub and tree texture tiles.
	// There are ZONE_TREES trees, ZONE_SHRUBS shrubs, and 1 grass in each zone.
	const int height = FT_PaletteHeight * g_Au.PlantTextureSize();
	const int plantsPerZone = ZONE_TREES + ZONE_SHRUBS + 1;
	const int heightNeeded = plantsPerZone * g_Au.PlantTextureSize();
	int xpos = 0;
	int ypos = zone * heightNeeded;
	while(ypos >= height)
	{
		xpos += g_Au.PlantTextureSize();	// Next column
		ypos -= height;
	}

	// Create an image to use when painting each branch tile.
	QImage imbranch(g_Au.PlantTextureSize(), g_Au.PlantTextureSize(), QImage::Format_ARGB32);

	// Create a painter to use for copying the completed branch tiles to the strip.
	QPainter p(&im);

	switch(plantType)
	{
	case 0:	// Ground Cover
		{
			// Seed the random number generator for the ground cover.
			AuDice dice;
			dice.Seed(seed);
			QImage imgrass(g_Au.PlantTextureSize(), g_Au.PlantTextureSize(), QImage::Format_ARGB32);
			m_GroundCover.PaintGroundCover(imgrass, genSeed, zone, dice, AuTime());

			// Calculate location in strip.
			ypos += (ZONE_TREES + ZONE_SHRUBS) * g_Au.PlantTextureSize();
			if(ypos >= height)
			{
				int cols = ypos/height;
				xpos += g_Au.PlantTextureSize() * cols;
				ypos -= height * cols;
			}

			// Copy the tile image into the strip at the proper position.
			p.drawImage(xpos,ypos, imgrass);	// Grass is placed at the end.
		}
		break;
	case 1: // Shrub
		{
			// Seed the random number generator for each plant.
			int index = ShrubIndex(idCommodity);
			seed += (index+1)*54321;
			AuDice dice;
			dice.Seed(seed);

			// Paint the branch into the tile image.
			m_Shrubs[index].PaintBranch(imbranch, zone, dice, dice.Roll(1,s_LeafBrushImages), dice.Roll(1,s_PetalBrushImages));

			// Calculate location in strip.
			ypos += index * g_Au.PlantTextureSize();
			if(ypos >= height)
			{
				int cols = ypos/height;
				xpos += g_Au.PlantTextureSize() * cols;
				ypos -= height * cols;
			}

			// Copy the tile image into the strip at the proper position.
			p.drawImage(xpos,ypos, imbranch);
		}
		break;
	case 2:	// Tree
		{
			// Seed the random number generator for each plant.
			int index = TreeIndex(idCommodity);
			seed += (index+1 + ZONE_SHRUBS)*54321;
			AuDice dice;
			dice.Seed(seed);

			// Paint the branch into the tile image.
			m_Trees[index].PaintBranch(imbranch, zone, dice, dice.Roll(1,s_LeafBrushImages), dice.Roll(1,s_PetalBrushImages));

			// Calculate location in strip.
			ypos += (index + ZONE_SHRUBS) * g_Au.PlantTextureSize();
			if(ypos >= height)
			{
				int cols = ypos/height;
				xpos += g_Au.PlantTextureSize() * cols;
				ypos -= height * cols;
			}

			// Copy the tile image into the strip at the proper position.
			p.drawImage(xpos,ypos, imbranch);
		}
		break;
	}

	// Finish painting in the tile palette.
	p.end();
}

void DeTerrainFlora::StoreTextureSizes(QDataStream& out) const
{
	// Shrubs
	for(int spos = 0; spos<ZONE_SHRUBS; spos++)
		m_Shrubs[spos].StoreTextureSize(out);

	// Trees
	for(int tpos = 0; tpos<ZONE_TREES; tpos++)
		m_Trees[tpos].StoreTextureSize(out);

	// Ground Cover
	m_GroundCover.StoreTextureSize(out);
}

void DeTerrainFlora::LoadTextureSizes(QDataStream& in)
{
	// Shrubs
	for(int spos = 0; spos<ZONE_SHRUBS; spos++)
		m_Shrubs[spos].LoadTextureSize(in);

	// Trees
	for(int tpos = 0; tpos<ZONE_TREES; tpos++)
		m_Trees[tpos].LoadTextureSize(in);

	// Ground Cover
	m_GroundCover.LoadTextureSize(in);
}

void DeTerrainFlora::CreateGroundCoverVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int texBase, const AuDice& dice,
		int lod, float maxRadius) const
{
	unsigned int textureIndex = texBase + ZONE_TREES + ZONE_SHRUBS;
	m_GroundCover.CreateVAO(GLf, vao, textureIndex, DeFlora::GroundCover, dice, lod, maxRadius);
}

void DeTerrainFlora::CreateShrubVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int texBase,
		COMMODITYID idCommodity, const AuDice& dice, int lod, float maxRadius) const
{
	unsigned int textureIndex = texBase + ShrubIndex(idCommodity);
	m_Shrubs[ShrubIndex(idCommodity)].CreateVAO(GLf, vao, textureIndex, DeFlora::Shrub, dice, lod, maxRadius);
}

void DeTerrainFlora::CreateTreeVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, unsigned int texBase,
		COMMODITYID idCommodity, const AuDice& dice, int lod, float maxRadius) const
{
	unsigned int textureIndex = texBase + ZONE_SHRUBS + TreeIndex(idCommodity);
	m_Trees[TreeIndex(idCommodity)].CreateVAO(GLf, vao, textureIndex, DeFlora::Tree, dice, lod, maxRadius);
}

bool DeTerrainFlora::ContainsDNA(int type, const AuFloraDNA& dna) const
{
	switch(type)
	{
	case 0:
		if(m_GroundCover.DNA() == dna)
			return true;
		break;
	case 1:
		for(int pi = 0; pi<ZONE_SHRUBS; pi++)
			if(m_Shrubs[pi].DNA() == dna)
				return true;
		break;
	case 2:
		for(int pi = 0; pi<ZONE_TREES; pi++)
			if(m_Trees[pi].DNA() == dna)
				return true;
		break;
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DePlanetFlora::DePlanetFlora()
{
	m_glFloraTex = 0;
}

DePlanetFlora::~DePlanetFlora()
{
}

void DePlanetFlora::Init(int radius100km, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	int sizemod = (128 - radius100km) / 10;	// Size increases as planet gets smaller. sizemod range is [0..10].
	m_Flora[AuFloraDNA::Polar].Init(AuFloraDNA::Polar, dice, sizemod);
	dice.Random();
	m_Flora[AuFloraDNA::Temperate].Init(AuFloraDNA::Temperate, dice, sizemod);
	dice.Random();
	m_Flora[AuFloraDNA::Tropical].Init(AuFloraDNA::Tropical, dice, sizemod);
}

// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
void DePlanetFlora::Init(int radius100km, int plantType, AuFloraDNA::ZONE zone, const AuDice& dice)
{
	int sizemod = (128 - radius100km) / 10;	// Size increases as planet gets smaller. sizemod range is [0..10].
	m_Flora[zone].Init(plantType, zone, dice, sizemod);
}

void DePlanetFlora::InitRingworld(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// Ringworlds are considered to have light (size 6) gravity. sizemod is therefore 6.
	m_Flora[AuFloraDNA::Polar].Init(AuFloraDNA::Polar, dice, 6);
	dice.Random();
	m_Flora[AuFloraDNA::Temperate].Init(AuFloraDNA::Temperate, dice, 6);
	dice.Random();
	m_Flora[AuFloraDNA::Tropical].Init(AuFloraDNA::Tropical, dice, 6);
}

// 'plantType' is 0=ground cover, 1=shrubs, 2=trees
void DePlanetFlora::InitRingworld(int plantType, AuFloraDNA::ZONE zone, const AuDice& dice)
{
	// Ringworlds are considered to have light (size 6) gravity. sizemod is therefore 6.
	m_Flora[zone].Init(plantType, zone, dice, 6);
}

void DePlanetFlora::operator<<(QDataStream& in)
{
	m_Flora[AuFloraDNA::Polar] << in;
	m_Flora[AuFloraDNA::Temperate] << in;
	m_Flora[AuFloraDNA::Tropical] << in;
}

void DePlanetFlora::operator>>(QDataStream& out) const
{
	m_Flora[AuFloraDNA::Polar] >> out;
	m_Flora[AuFloraDNA::Temperate] >> out;
	m_Flora[AuFloraDNA::Tropical] >> out;
}

void DePlanetFlora::TxUpdate(QDataStream& toClient) const
{
	m_Flora[AuFloraDNA::Polar].TxUpdate(toClient);
	m_Flora[AuFloraDNA::Temperate].TxUpdate(toClient);
	m_Flora[AuFloraDNA::Tropical].TxUpdate(toClient);
}

void DePlanetFlora::RxUpdate(QDataStream& fromServer)
{
	m_Flora[AuFloraDNA::Polar].RxUpdate(fromServer);
	m_Flora[AuFloraDNA::Temperate].RxUpdate(fromServer);
	m_Flora[AuFloraDNA::Tropical].RxUpdate(fromServer);
}

const DeFlora& DePlanetFlora::GroundCover(AuFloraDNA::ZONE latRegion) const
{
	return m_Flora[latRegion].GroundCover();
}

const DeFlora& DePlanetFlora::Shrub(AuFloraDNA::ZONE latRegion, COMMODITYID idCommodity) const
{
	return m_Flora[latRegion].Shrub(idCommodity);
}

const DeFlora& DePlanetFlora::Tree(AuFloraDNA::ZONE latRegion, COMMODITYID idCommodity) const
{
	return m_Flora[latRegion].Tree(idCommodity);
}

void DePlanetFlora::PaintBranches(QImage& im, quint32 genSeed, int resourceZone, const AuTime& floraChange)
{
	m_Flora[AuFloraDNA::Polar].PaintBranches(im, genSeed, resourceZone, AuFloraDNA::Polar, floraChange);
	m_Flora[AuFloraDNA::Temperate].PaintBranches(im, genSeed, resourceZone, AuFloraDNA::Temperate, floraChange);
	m_Flora[AuFloraDNA::Tropical].PaintBranches(im, genSeed, resourceZone, AuFloraDNA::Tropical, floraChange);
}

// Used when painting a single plant, for a flower pot.
void DePlanetFlora::PaintPottedBranches(QImage& im, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE latRegion, int plantType,
		COMMODITYID idCommodity)
{
	m_Flora[latRegion].PaintPottedBranches(im, genSeed, resourceZone, latRegion, plantType, idCommodity);
}

void DePlanetFlora::PaintGroundCover(QImage& grassPolar, QImage& grassTemperate, QImage& grassTropical, quint32 genSeed,
		int resourceZone, const AuTime& floraChange)
{
	m_Flora[AuFloraDNA::Polar].PaintGroundCover(grassPolar, genSeed, resourceZone, AuFloraDNA::Polar, floraChange);
	m_Flora[AuFloraDNA::Temperate].PaintGroundCover(grassTemperate, genSeed, resourceZone, AuFloraDNA::Temperate, floraChange);
	m_Flora[AuFloraDNA::Tropical].PaintGroundCover(grassTropical, genSeed, resourceZone, AuFloraDNA::Tropical, floraChange);
}

void DePlanetFlora::CreateDeviceObjects(AuGLEffects& re, const AuId& flowerPotId, quint32 genSeed, int resourceZone,
		AuFloraDNA::ZONE zone, int plantType, COMMODITYID idCommodity)
{
	if(!m_glFloraTex)
	{
		QString stid = QString("Pot%1Z%2Z%3T%4C%5")
			.arg(genSeed)
			.arg(resourceZone)
			.arg(zone)
			.arg(plantType)
			.arg(idCommodity);
		m_glFloraTex = re.UseTexture(flowerPotId.GetID(), stid);
		if(m_glFloraTex)
		{
			CalculateTextureSizes(genSeed, resourceZone);
		}
		else
		{
			QImage imtex;
			PaintPottedFlora(imtex, genSeed, resourceZone, zone, plantType, idCommodity);
			if(!imtex.isNull())
			{
				// Create a texture map from the QImage object we've created.
				m_glFloraTex = re.CreateTextureFromBitmap(flowerPotId.GetID(), stid, imtex);
				Q_ASSERT(m_glFloraTex);
			}
		}
	}
}

void DePlanetFlora::CreateDeviceObjects(AuGLEffects& re, const AuId& planetId, quint32 genSeed, int resourceZone,
		const AuTime& floraChange)
{
	if(!m_glFloraTex)
	{
		QString stid = QString("F%1Z%2C%3")
				.arg(genSeed)
				.arg(resourceZone)
				.arg(floraChange.MSecsSinceEpoch());
		m_glFloraTex = re.UseTexture(planetId.GetID(), stid);
		if(m_glFloraTex)
		{
			CalculateTextureSizes(genSeed, resourceZone);
		}
		else
		{
			QImage imtex;
			PaintFlora(imtex, genSeed, resourceZone, floraChange);
			if(!imtex.isNull())
			{
				// Create a texture map from the QImage object we've created.
				m_glFloraTex = re.CreateTextureFromBitmap(planetId.GetID(), stid, imtex);
				Q_ASSERT(m_glFloraTex);
			}
		}
	}
}

void DePlanetFlora::CalculateTextureSizes(quint32 genSeed, int resourceZone)
{
	m_Flora[AuFloraDNA::Polar].CalculateTextureSizes(genSeed, resourceZone);
	m_Flora[AuFloraDNA::Temperate].CalculateTextureSizes(genSeed, resourceZone);
	m_Flora[AuFloraDNA::Tropical].CalculateTextureSizes(genSeed, resourceZone);
}

void DePlanetFlora::PaintFlora(QImage& imtex, quint32 genSeed, int resourceZone, const AuTime& floraChange)
{
	// Try loading the flora texture image from a cache file.
	bool imloaded = false;
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName;
	if(floraChange.IsNull())
		cacheFileName = cacheDir.absoluteFilePath(QString("F%1Z%2")
				.arg(genSeed)
				.arg(resourceZone));
	else
		cacheFileName = cacheDir.absoluteFilePath(QString("F%1Z%2C%3")
				.arg(genSeed)
				.arg(resourceZone)
				.arg(floraChange.MSecsSinceEpoch()));
	QFile cacheFile(cacheFileName);
	if(cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;								// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == FLORA_TEXTURE_CACHE && fileVersion == FLORA_TEXTURE_VER)
		{
			// Cache file looks good.
			// Read the compressed byte array from the file.
			QByteArray bcomp;
			dsfile >> bcomp;

			if(!bcomp.isEmpty())
			{
				// Uncompress the byte array.
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,cacheFileName);

				// Read the model cache from the uncompressed byte array.
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> imtex;
				LoadTextureSizes(ds);
				if(!imtex.isNull())
					imloaded = true;
			}
		}

		// Close the file.
		cacheFile.close();
		if(d_CacheDebugs && imloaded)
			qDebug() << "DeFlora::PaintFlora" << genSeed << resourceZone << "loaded from cache" << cacheFileName
				<< "w" << imtex.width() << "h" << imtex.height() << "tileSz" << g_Au.PlantTextureSize();
	}

	const int height = FT_PaletteHeight * g_Au.PlantTextureSize();
	const int width = FT_PaletteWidth * g_Au.PlantTextureSize();
	if(!imloaded || imtex.height() != height || imtex.width() != width)
	{
		if(d_CacheDebugs)
			qDebug() << "DeFlora::PaintFlora Creating flora texture for genSeed" << genSeed << "resZone" << resourceZone
				<< "w" << width << "h" << height << "tileSz" << g_Au.PlantTextureSize();
		// Flora texture was not loaded from a cache file.
		// Do the work to create it.

		// Create a texture strip to contain a typical branch image of each
		// type of plant on this planet and the standard bark textures. Each
		// branch texture is g_Au.PlantTextureSize() x g_Au.PlantTextureSize() pixels.
		// The textures are combined into a palette that is FT_PaletteHeight
		// tiles tall and FT_PaletteWidth tiles wide. Plant and bark textures
		// are tiled from top to bottom.
		imtex = QImage(width, height, QImage::Format_ARGB32);

		// Clear the image to transparent black.
		QPainter p(&imtex);
		p.setCompositionMode(QPainter::CompositionMode_Clear);	// clear mode
		p.fillRect(0,0,width,height, QBrush(Qt::transparent));
		p.setCompositionMode(QPainter::CompositionMode_Source);

		// Paint the bark image into the texture strip.
		QImage imbark(":data/txBark.png");
		const int barkColumn = FT_BarkStart / FT_PaletteHeight;
		const int barkRow = FT_BarkStart - FT_PaletteHeight*barkColumn;
		QRect rcbark(barkColumn*g_Au.PlantTextureSize(), barkRow*g_Au.PlantTextureSize(), g_Au.PlantTextureSize(),
				8*g_Au.PlantTextureSize());
		p.drawImage(rcbark, imbark);
		p.end();

		// Paint the branch textures.
		PaintBranches(imtex, genSeed, resourceZone, floraChange);

		// Store the result in a cache file.
		// Insure the cache folder exists.
		if(g_Au.CreateCacheGame())
		{
			// The cache folder exists.
			if(cacheFile.open(QFile::WriteOnly))
			{
				// Prepare to write to the file.
				QDataStream dsfile(&cacheFile);
				dsfile.setVersion(QDataStream::Qt_4_1);

				// Write a header to the file that includes the date/time the cache file was created.
				const quint8 fileType = FLORA_TEXTURE_CACHE;
				const quint8 fileVersion = FLORA_TEXTURE_VER;
				dsfile << fileType;
				dsfile << fileVersion;
				dsfile << floraChange.UTC();

				// Store path cache into a byte array so it can be compressed.
				QByteArray ba;
				QDataStream ds(&ba, QIODevice::WriteOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds << imtex;
				StoreTextureSizes(ds);

				// Compress the byte array.
				QByteArray bcomp = qCompress(ba);

				// Write the compressed byte array to the file.
				dsfile << bcomp;

				// Close the file.
				cacheFile.close();
			}
			else
				qWarning() << QString("DePlanetFlora::PaintFlora FAILED TO CREATE FILE %1").arg(cacheFileName);
		}
		else
			qWarning() << QString("DePlanetFlora::PaintFlora FAILED TO CREATE DIR FOR FILE %1").arg(cacheFileName);
	}
	//else
		//qDebug() << "Flora texture loaded from cache for planet zone" << resourceZone;

	/*
#if !defined(QT_NO_DEBUG)
	// Let's see what we got.
	qDebug() << "DePlanetFlora::PaintFlora" << genSeed << resourceZone;
	QString stid = QString("F%1Z%2")
			.arg(genSeed)
			.arg(resourceZone);
	QString file = cacheDir.absoluteFilePath(stid + ".png");
	imtex.save(file);
#endif
	*/
}

void DePlanetFlora::PaintPottedFlora(QImage& imtex, quint32 genSeed, int resourceZone, AuFloraDNA::ZONE zone, int plantType,
		COMMODITYID idCommodity)
{
	// Try loading the flora texture image from a cache file.
	bool imloaded = false;
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName;
	cacheFileName = cacheDir.absoluteFilePath(QString("F%1Z%2Z%3T%4C%5")
			.arg(genSeed)
			.arg(resourceZone)
			.arg(zone)
			.arg(plantType)
			.arg(idCommodity));
	QFile cacheFile(cacheFileName);
	if(cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;								// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == FLORA_TEXTURE_CACHE && fileVersion == FLORA_TEXTURE_VER)
		{
			// Cache file looks good.
			// Read the compressed byte array from the file.
			QByteArray bcomp;
			dsfile >> bcomp;

			if(!bcomp.isEmpty())
			{
				// Uncompress the byte array.
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,cacheFileName);

				// Read the model cache from the uncompressed byte array.
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> imtex;
				LoadTextureSizes(ds);
				if(!imtex.isNull())
					imloaded = true;
			}
		}

		// Close the file.
		cacheFile.close();
		if(d_CacheDebugs && imloaded)
			qDebug() << "DeFlora::PaintPottedFlora" << genSeed << resourceZone << "loaded from cache" << cacheFileName;
	}

	const int height = FT_PaletteHeight * g_Au.PlantTextureSize();
	const int width = FT_PaletteWidth * g_Au.PlantTextureSize();
	if(!imloaded || imtex.height() != height || imtex.width() != width)
	{
		// Create a texture strip to contain a typical branch image of each
		if(d_CacheDebugs)
			qDebug() << "DeFlora::PaintPottedFlora creating" << genSeed << resourceZone << imtex.size() << width << height;
		// type of plant on this planet and the standard bark textures. Each
		// branch texture is g_Au.PlantTextureSize() x g_Au.PlantTextureSize() pixels.
		// The textures are combined into a palette that is FT_PaletteHeight
		// tiles tall and FT_PaletteWidth tiles wide. Plant and bark textures
		// are tiled from top to bottom.
		imtex = QImage(width, height, QImage::Format_ARGB32);

		// Clear the image to transparent black.
		QPainter p(&imtex);
		p.setCompositionMode(QPainter::CompositionMode_Clear);	// clear mode
		p.fillRect(0,0,width,height, QBrush(Qt::transparent));
		p.setCompositionMode(QPainter::CompositionMode_Source);

		// Paint the bark image into the texture strip.
		QImage imbark(":data/txBark.png");
		const int barkColumn = FT_BarkStart / FT_PaletteHeight;
		const int barkRow = FT_BarkStart - FT_PaletteHeight*barkColumn;
		QRect rcbark(barkColumn*g_Au.PlantTextureSize(), barkRow*g_Au.PlantTextureSize(), g_Au.PlantTextureSize(),
				8*g_Au.PlantTextureSize());
		p.drawImage(rcbark, imbark);
		p.end();

		// Paint the branch textures.
		PaintPottedBranches(imtex, genSeed, resourceZone, zone, plantType, idCommodity);

		// Store the result in a cache file.
		// Insure the cache folder exists.
		if(g_Au.CreateCacheGame())
		{
			// The cache folder exists.
			if(cacheFile.open(QFile::WriteOnly))
			{
				// Prepare to write to the file.
				QDataStream dsfile(&cacheFile);
				dsfile.setVersion(QDataStream::Qt_4_1);

				// Write a header to the file that includes the date/time the cache file was created.
				const quint8 fileType = FLORA_TEXTURE_CACHE;
				const quint8 fileVersion = FLORA_TEXTURE_VER;
				dsfile << fileType;
				dsfile << fileVersion;
				dsfile << g_Au.CurrentUTC();

				// Store path cache into a byte array so it can be compressed.
				QByteArray ba;
				QDataStream ds(&ba, QIODevice::WriteOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds << imtex;
				StoreTextureSizes(ds);

				// Compress the byte array.
				QByteArray bcomp = qCompress(ba);

				// Write the compressed byte array to the file.
				dsfile << bcomp;

				// Close the file.
				cacheFile.close();
			}
			else
				qWarning() << QString("DePlanetFlora::PaintPottedFlora FAILED TO CREATE FILE %1").arg(cacheFileName);
		}
		else
			qWarning() << QString("DePlanetFlora::PaintPottedFlora FAILED TO CREATE DIR FOR FILE %1").arg(cacheFileName);
	}

#if !defined(QT_NO_DEBUG)
	// Let's see what we got.
	/*
	qDebug() << "DePlanetFlora::PaintPottedFlora" << g_Au.CommodityName((COMMODITYID)idCommodity);
	QString stid = QString("Pot%1Z%2Z%3T%4C%5")
		.arg(genSeed)
		.arg(resourceZone)
		.arg(zone)
		.arg(plantType)
		.arg(idCommodity);
	QDir cacheDir(g_Au.CacheGame());
	QString file = cacheDir.absoluteFilePath(stid + ".png");
	imtex.save(file);
	*/
#endif
}

void DePlanetFlora::DeleteDeviceObjects(AuGLEffects& /*re*/)
{
	m_glFloraTex = 0;
}

void DePlanetFlora::CreateGroundCoverVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, AuFloraDNA::ZONE latRegion,
		const AuDice& dice, int lod, float maxRadius) const
{
	unsigned int texBase = 0;
	if(latRegion != AuFloraDNA::Polar)
	{
		// There are ZONE_TREES trees, ZONE_SHRUBS shrubs, and 1 grass in each zone.
		const int plantsPerZone = ZONE_TREES + ZONE_SHRUBS + 1;

		// 1 GL display list was used for each plant.
		if(latRegion == AuFloraDNA::Tropical)
		{
			texBase += plantsPerZone * 2;
			//qDebug("tropical shrub %d commodity %d", modelbase, idCommodity);
		}
		else
		{
			texBase += plantsPerZone;
			//qDebug("temperate shrub %d commodity %d", modelbase, idCommodity);
		}
	}

	m_Flora[latRegion].CreateGroundCoverVAO(GLf, vao, texBase, dice, lod, maxRadius);
}

void DePlanetFlora::CreateShrubVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, AuFloraDNA::ZONE latRegion,
		COMMODITYID idCommodity, const AuDice& dice, int lod, float maxRadius) const
{
	unsigned int texBase = 0;
	if(latRegion != AuFloraDNA::Polar)
	{
		// There are ZONE_TREES trees, ZONE_SHRUBS shrubs, and 1 grass in each zone.
		const int plantsPerZone = ZONE_TREES + ZONE_SHRUBS + 1;

		// 1 GL display list was used for each plant.
		if(latRegion == AuFloraDNA::Tropical)
		{
			texBase += plantsPerZone * 2;
			//qDebug("tropical shrub %d commodity %d", modelbase, idCommodity);
		}
		else
		{
			texBase += plantsPerZone;
			//qDebug("temperate shrub %d commodity %d", modelbase, idCommodity);
		}
	}

	m_Flora[latRegion].CreateShrubVAO(GLf, vao, texBase, idCommodity, dice, lod, maxRadius);
}

void DePlanetFlora::CreateTreeVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao, AuFloraDNA::ZONE latRegion,
		COMMODITYID idCommodity, const AuDice& dice, int lod, float maxRadius) const
{
	unsigned int texBase = 0;
	if(latRegion != AuFloraDNA::Polar)
	{
		// There are ZONE_TREES trees, ZONE_SHRUBS shrubs, and 1 grass in each zone.
		const int plantsPerZone = ZONE_TREES + ZONE_SHRUBS + 1;

		// 1 GL display list was used for each plant.
		if(latRegion == AuFloraDNA::Tropical)
		{
			texBase += plantsPerZone * 2;
			//qDebug("tropical shrub %d commodity %d", modelbase, idCommodity);
		}
		else
		{
			texBase += plantsPerZone;
			//qDebug("temperate shrub %d commodity %d", modelbase, idCommodity);
		}
	}

	m_Flora[latRegion].CreateTreeVAO(GLf, vao, texBase, idCommodity, dice, lod, maxRadius);
}

//static
void DePlanetFlora::Texture(unsigned int tileIndex, float& s, float& t, float& u, float& v)
{
	if(tileIndex >= FT_PaletteHeight*FT_PaletteWidth)
	{
		s = 0;
		t = 0;
		u = 0;
		v = 0;
	}
	else
	{
		// Calculate texture coordinates.
		// Y is inverted
		const float maxx = FT_PaletteWidth * g_Au.PlantTextureSize();
		const float maxy = FT_PaletteHeight * g_Au.PlantTextureSize();
		const int column = tileIndex / FT_PaletteHeight;
		const int row = tileIndex - FT_PaletteHeight*column;
		s = (g_Au.PlantTextureSize()*column + 1.f)/maxx;
		t = 1.f - (row * g_Au.PlantTextureSize() + 1.f) / maxy;
		u = (g_Au.PlantTextureSize()*(column+1) - 1.f)/maxx;
		v = 1.f - ((row+1) * g_Au.PlantTextureSize() - 1.f) / maxy;
		//qDebug() << "DePlanetFlora::Texture tile" << tileIndex << "stuv" << s << t << u << v
			//<< "texSize" << g_Au.PlantTextureSize() << "max" << maxx << maxy << "rowCol" << row << column;
	}
}

void DePlanetFlora::StoreTextureSizes(QDataStream& out) const
{
	m_Flora[AuFloraDNA::Polar].StoreTextureSizes(out);
	m_Flora[AuFloraDNA::Temperate].StoreTextureSizes(out);
	m_Flora[AuFloraDNA::Tropical].StoreTextureSizes(out);
}

void DePlanetFlora::LoadTextureSizes(QDataStream& in)
{
	m_Flora[AuFloraDNA::Polar].LoadTextureSizes(in);
	m_Flora[AuFloraDNA::Temperate].LoadTextureSizes(in);
	m_Flora[AuFloraDNA::Tropical].LoadTextureSizes(in);
}

void DePlanetFlora::SetDNA(AuFloraDNA::ZONE latRegion, const DeTerrainFlora& dna)
{
	m_Flora[latRegion] = dna;
}
