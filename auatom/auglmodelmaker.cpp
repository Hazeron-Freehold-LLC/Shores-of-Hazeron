//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglmodelmaker.cpp,v 1.162 2022/05/09 16:24:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auglmodelmaker.h"
//
#include "aubddesign.h"
#include "aufilever.h"
#include "auglobal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "dastar.h"
#include "daworldglobe.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "hashpack.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QThreadPool>

#define ATGLOBE_MODEL_VER				0x15					// WGA12345 globe model files in temporary cache
#define FLATGLOBE_MODEL_VER				0x15					// WGF12345 flat globe model files in temporary cache
#define NEARGLOBE_MODEL_VER				0x15					// WGN12345 globe model files in temporary cache

#define c_CacheDebugs					false					// True to log cache hits/misses.

bool AuRunnable::RunStart()
{
	// When the program is killed, the global data pointer gets set to null.
	//m_RunStateLock.lock();
	if(!gp_AuGlobal || m_Killed)
	{
		RunFinish();
		return false;
	}
	m_RunState = 1;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGLModelMaker::AuGLModelMaker(bool cacheEnabled)
{
	m_CacheEnabled = cacheEnabled;
	//qDebug() << "AuGLModelMaker::AuGLModelMaker cacheEnabled" << cacheEnabled;
}

AuGLModelMaker::~AuGLModelMaker()
{
	// Deletion of these objects causes the thread pool to crash if they are still being processed.
	// It's better to just let the memory leak on the ones that are still running, to be cleaned up by the OS.
#if !defined(QT_NO_DEBUG)
	int animaliaDeleted = 0;
	int animaliaRunning = 0;
	int animaliaDeletedDel = 0;
	int animaliaRunningDel = 0;
#endif
	{
		unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator ami = m_Animalia.begin();
		while(ami != m_Animalia.end())
		{
			if((*ami).second->IsDoneRunning())
			{
				delete (*ami).second;
				ami = m_Animalia.erase(ami);
#if !defined(QT_NO_DEBUG)
				animaliaDeleted++;
#endif
			}
			else
			{
				ami++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
				animaliaRunning++;
#endif
			}
		}

		QList<AuAnimaliaModel*>::iterator adi = m_AnimaliaToDelete.begin();
		while(adi != m_AnimaliaToDelete.end())
		{
			if((*adi)->IsDoneRunning())
			{
				delete *adi;
				adi = m_AnimaliaToDelete.erase(adi);
#if !defined(QT_NO_DEBUG)
				animaliaDeletedDel++;
#endif
			}
			else
			{
				adi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
				animaliaRunningDel++;
#endif
			}
		}
	}

#if !defined(QT_NO_DEBUG)
	int bdDeleted = 0;
	int bdRunning = 0;
	int bdDeletedDel = 0;
	int bdRunningDel = 0;
#endif
	{
		unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator ami = m_Animalia.begin();
		while(ami != m_Animalia.end())
		{
			if((*ami).second->IsDoneRunning())
			{
				delete (*ami).second;
				ami = m_Animalia.erase(ami);
#if !defined(QT_NO_DEBUG)
				bdDeleted++;
#endif
			}
			else
			{
				ami++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
				bdRunning++;
#endif
			}
		}

		QList<AuAnimaliaModel*>::iterator adi = m_AnimaliaToDelete.begin();
		while(adi != m_AnimaliaToDelete.end())
		{
			if((*adi)->IsDoneRunning())
			{
				delete *adi;
				adi = m_AnimaliaToDelete.erase(adi);
#if !defined(QT_NO_DEBUG)
				bdDeletedDel++;
#endif
			}
			else
			{
				adi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
				bdRunningDel++;
#endif
			}
		}
	}

#if !defined(QT_NO_DEBUG)
	int flatGlobesDeleted = 0;
	int flatGlobesRunning = 0;
#endif
	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::iterator gmi = m_FlatGlobes.begin();
	while(gmi != m_FlatGlobes.end())
	{
		if((*gmi).second->IsDoneRunning())
		{
			delete (*gmi).second;
			gmi = m_FlatGlobes.erase(gmi);
#if !defined(QT_NO_DEBUG)
			flatGlobesDeleted++;
#endif
		}
		else
		{
			gmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			flatGlobesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delFlatGlobesDeleted = 0;
	int delFlatGlobesRunning = 0;
#endif
	QList<AuFlatGlobeModel*>::iterator gdi = m_FlatGlobesToDelete.begin();
	while(gdi != m_FlatGlobesToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			delete *gdi;
			gdi = m_FlatGlobesToDelete.erase(gdi);
#if !defined(QT_NO_DEBUG)
			delFlatGlobesDeleted++;
#endif
		}
		else
		{
			gdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delFlatGlobesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int globesDeleted = 0;
	int globesRunning = 0;
#endif
	unordered_map<AuId, AuGlobeModel*, AuIdHash>::iterator fgmi = m_Globes.begin();
	while(fgmi != m_Globes.end())
	{
		if((*fgmi).second->IsDoneRunning())
		{
			delete (*fgmi).second;
			fgmi = m_Globes.erase(fgmi);
#if !defined(QT_NO_DEBUG)
			globesDeleted++;
#endif
		}
		else
		{
			fgmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			globesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delGlobesDeleted = 0;
	int delGlobesRunning = 0;
#endif
	QList<AuGlobeModel*>::iterator fgdi = m_GlobesToDelete.begin();
	while(fgdi != m_GlobesToDelete.end())
	{
		if((*fgdi)->IsDoneRunning())
		{
			delete *fgdi;
			fgdi = m_GlobesToDelete.erase(fgdi);
#if !defined(QT_NO_DEBUG)
			delGlobesDeleted++;
#endif
		}
		else
		{
			fgdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delGlobesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int globeTrisDeleted = 0;
	int globeTrisRunning = 0;
#endif
	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gtmi = m_GlobeTris.begin();
	while(gtmi != m_GlobeTris.end())
	{
		if((*gtmi).second->IsDoneRunning())
		{
			delete (*gtmi).second;
			gtmi = m_GlobeTris.erase(gtmi);
#if !defined(QT_NO_DEBUG)
			globeTrisDeleted++;
#endif
		}
		else
		{
			gtmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			globeTrisRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delGlobeTrisDeleted = 0;
	int delGlobeTrisRunning = 0;
#endif
	QList<AuGlobeModelTri*>::iterator gtdi = m_GlobeTrisToDelete.begin();
	while(gtdi != m_GlobeTrisToDelete.end())
	{
		if((*gtdi)->IsDoneRunning())
		{
			delete *gtdi;
			gtdi = m_GlobeTrisToDelete.erase(gtdi);
#if !defined(QT_NO_DEBUG)
			delGlobeTrisDeleted++;
#endif
		}
		else
		{
			gtdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delGlobeTrisRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int nearGlobesDeleted = 0;
	int nearGlobesRunning = 0;
#endif
	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::iterator ngmi = m_NearGlobes.begin();
	while(ngmi != m_NearGlobes.end())
	{
		if((*ngmi).second->IsDoneRunning())
		{
			delete (*ngmi).second;
			ngmi = m_NearGlobes.erase(ngmi);
#if !defined(QT_NO_DEBUG)
			nearGlobesDeleted++;
#endif
		}
		else
		{
			ngmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			nearGlobesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delNearGlobesDeleted = 0;
	int delNearGlobesRunning = 0;
#endif
	QList<AuNearGlobeModel*>::iterator ngdi = m_NearGlobesToDelete.begin();
	while(ngdi != m_NearGlobesToDelete.end())
	{
		if((*ngdi)->IsDoneRunning())
		{
			delete *ngdi;
			ngdi = m_NearGlobesToDelete.erase(ngdi);
#if !defined(QT_NO_DEBUG)
			delNearGlobesDeleted++;
#endif
		}
		else
		{
			ngdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delNearGlobesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int flatRingworldSectionsDeleted = 0;
	int flatRingworldSectionsRunning = 0;
#endif
	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::iterator frsmi = m_FlatRingworldSections.begin();
	while(frsmi != m_FlatRingworldSections.end())
	{
		if((*frsmi).second->IsDoneRunning())
		{
			delete (*frsmi).second;
			frsmi = m_FlatRingworldSections.erase(frsmi);
#if !defined(QT_NO_DEBUG)
			flatRingworldSectionsDeleted++;
#endif
		}
		else
		{
			frsmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			flatRingworldSectionsRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delFlatRingworldSectionsDeleted = 0;
	int delFlatRingworldSectionsRunning = 0;
#endif
	QList<AuFlatGlobeModel*>::iterator frsdi = m_FlatRingworldSectionsToDelete.begin();
	while(frsdi != m_FlatRingworldSectionsToDelete.end())
	{
		if((*frsdi)->IsDoneRunning())
		{
			delete *frsdi;
			frsdi = m_FlatRingworldSectionsToDelete.erase(frsdi);
#if !defined(QT_NO_DEBUG)
			delFlatRingworldSectionsDeleted++;
#endif
		}
		else
		{
			frsdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delFlatRingworldSectionsRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int ringworldSectionsDeleted = 0;
	int ringworldSectionsRunning = 0;
#endif
	unordered_map<AuId, AuGlobeModel*, AuIdHash>::iterator rsmi = m_RingworldSections.begin();
	while(rsmi != m_RingworldSections.end())
	{
		if((*rsmi).second->IsDoneRunning())
		{
			delete (*rsmi).second;
			rsmi = m_RingworldSections.erase(rsmi);
#if !defined(QT_NO_DEBUG)
			ringworldSectionsDeleted++;
#endif
		}
		else
		{
			rsmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			ringworldSectionsRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delRingworldSectionsDeleted = 0;
	int delRingworldSectionsRunning = 0;
#endif
	QList<AuGlobeModel*>::iterator rsdi = m_RingworldSectionsToDelete.begin();
	while(rsdi != m_RingworldSectionsToDelete.end())
	{
		if((*rsdi)->IsDoneRunning())
		{
			delete *rsdi;
			rsdi = m_RingworldSectionsToDelete.erase(rsdi);
#if !defined(QT_NO_DEBUG)
			delRingworldSectionsDeleted++;
#endif
		}
		else
		{
			rsdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delRingworldSectionsRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int ringworldSectionTrisDeleted = 0;
	int ringworldSectionTrisRunning = 0;
#endif
	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator rstmi = m_RingworldSectionTris.begin();
	while(rstmi != m_RingworldSectionTris.end())
	{
		if((*rstmi).second->IsDoneRunning())
		{
			delete (*rstmi).second;
			rstmi = m_RingworldSectionTris.erase(rstmi);
#if !defined(QT_NO_DEBUG)
			ringworldSectionTrisDeleted++;
#endif
		}
		else
		{
			rstmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			ringworldSectionTrisRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delRingworldSectionTrisDeleted = 0;
	int delRingworldSectionTrisRunning = 0;
#endif
	QList<AuGlobeModelTri*>::iterator rstdi = m_RingworldSectionTrisToDelete.begin();
	while(rstdi != m_RingworldSectionTrisToDelete.end())
	{
		if((*rstdi)->IsDoneRunning())
		{
			delete *rstdi;
			rstdi = m_RingworldSectionTrisToDelete.erase(rstdi);
#if !defined(QT_NO_DEBUG)
			delRingworldSectionTrisDeleted++;
#endif
		}
		else
		{
			rstdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delRingworldSectionTrisRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int nearRingworldSectionsDeleted = 0;
	int nearRingworldSectionsRunning = 0;
#endif
	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::iterator nrsmi = m_NearRingworldSections.begin();
	while(nrsmi != m_NearRingworldSections.end())
	{
		if((*nrsmi).second->IsDoneRunning())
		{
			delete (*nrsmi).second;
			nrsmi = m_NearRingworldSections.erase(nrsmi);
#if !defined(QT_NO_DEBUG)
			nearRingworldSectionsDeleted++;
#endif
		}
		else
		{
			nrsmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			nearRingworldSectionsRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delNearRingworldSectionsDeleted = 0;
	int delNearRingworldSectionsRunning = 0;
#endif
	QList<AuNearGlobeModel*>::iterator nrsdi = m_NearRingworldSectionsToDelete.begin();
	while(nrsdi != m_NearRingworldSectionsToDelete.end())
	{
		if((*nrsdi)->IsDoneRunning())
		{
			delete *nrsdi;
			nrsdi = m_NearRingworldSectionsToDelete.erase(nrsdi);
#if !defined(QT_NO_DEBUG)
			delNearRingworldSectionsDeleted++;
#endif
		}
		else
		{
			nrsdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delNearRingworldSectionsRunning++;
#endif
		}
	}

	// Flora Images
#if !defined(QT_NO_DEBUG)
	int floraImagesDeleted = 0;
	int floraImagesRunning = 0;
#endif
	unordered_map<AuId, AuFloraImage*, AuIdHash>::iterator fii = m_FloraImages.begin();
	while(fii != m_FloraImages.end())
	{
		if((*fii).second->IsDoneRunning())
		{
			delete (*fii).second;
			fii = m_FloraImages.erase(fii);
#if !defined(QT_NO_DEBUG)
			floraImagesDeleted++;
#endif
		}
		else
		{
			fii++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			floraImagesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delFloraImagesDeleted = 0;
	int delFloraImagesRunning = 0;
#endif
	QList<AuFloraImage*>::iterator fidi = m_FloraImagesToDelete.begin();
	while(fidi != m_FloraImagesToDelete.end())
	{
		if((*fidi)->IsDoneRunning())
		{
			delete *fidi;
			fidi = m_FloraImagesToDelete.erase(fidi);
#if !defined(QT_NO_DEBUG)
			delFloraImagesDeleted++;
#endif
		}
		else
		{
			fidi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delFloraImagesRunning++;
#endif
		}
	}

	// Flora
#if !defined(QT_NO_DEBUG)
	int floraModelsDeleted = 0;
	int floraModelsRunning = 0;
#endif
	unordered_map<AuId, AuFloraModel*, AuIdHash>::iterator fmi = m_Flora.begin();
	while(fmi != m_Flora.end())
	{
		if((*fmi).second->IsDoneRunning())
		{
			delete (*fmi).second;
			fmi = m_Flora.erase(fmi);
#if !defined(QT_NO_DEBUG)
			floraModelsDeleted++;
#endif
		}
		else
		{
			fmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			floraModelsRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delFloraModelsDeleted = 0;
	int delFloraModelsRunning = 0;
#endif
	QList<AuFloraModel*>::iterator fdi = m_FloraToDelete.begin();
	while(fdi != m_FloraToDelete.end())
	{
		if((*fdi)->IsDoneRunning())
		{
			delete *fdi;
			fdi = m_FloraToDelete.erase(fdi);
#if !defined(QT_NO_DEBUG)
			delFloraModelsDeleted++;
#endif
		}
		else
		{
			fdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delFloraModelsRunning++;
#endif
		}
	}

	// Grass Images
#if !defined(QT_NO_DEBUG)
	int grassImagesDeleted = 0;
	int grassImagesRunning = 0;
#endif
	unordered_multimap<AuId, AuGrassImages*, AuIdHash>::iterator gii = m_GrassImages.begin();
	while(gii != m_GrassImages.end())
	{
		if((*gii).second->IsDoneRunning())
		{
			delete (*gii).second;
			gii = m_GrassImages.erase(gii);
#if !defined(QT_NO_DEBUG)
			grassImagesDeleted++;
#endif
		}
		else
		{
			gii++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			grassImagesRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delGrassImagesDeleted = 0;
	int delGrassImagesRunning = 0;
#endif
	QList<AuGrassImages*>::iterator gidi = m_GrassImagesToDelete.begin();
	while(gidi != m_GrassImagesToDelete.end())
	{
		if((*gidi)->IsDoneRunning())
		{
			delete *gidi;
			gidi = m_GrassImagesToDelete.erase(gidi);
#if !defined(QT_NO_DEBUG)
			delGrassImagesDeleted++;
#endif
		}
		else
		{
			gidi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delGrassImagesRunning++;
#endif
		}
	}

	// Rocks
#if !defined(QT_NO_DEBUG)
	int rocksDeleted = 0;
	int rocksRunning = 0;
#endif
	unordered_map<AuId, AuRockModel*, AuIdHash>::iterator rmi = m_Rock.begin();
	while(rmi != m_Rock.end())
	{
		if((*rmi).second->IsDoneRunning())
		{
			delete (*rmi).second;
			rmi = m_Rock.erase(rmi);
#if !defined(QT_NO_DEBUG)
			rocksDeleted++;
#endif
		}
		else
		{
			rmi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			rocksRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delRocksDeleted = 0;
	int delRocksRunning = 0;
#endif
	QList<AuRockModel*>::iterator rdi = m_RockToDelete.begin();
	while(rdi != m_RockToDelete.end())
	{
		if((*rdi)->IsDoneRunning())
		{
			delete *rdi;
			rdi = m_RockToDelete.erase(rdi);
#if !defined(QT_NO_DEBUG)
			delRocksDeleted++;
#endif
		}
		else
		{
			rdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delRocksRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int wxDeleted = 0;
	int wxRunning = 0;
#endif
	unordered_map<AuId, AuWeatherData*, AuIdHash>::iterator wi = m_Wx.begin();
	while(wi != m_Wx.end())
	{
		if((*wi).second->IsDoneRunning())
		{
			delete (*wi).second;
			wi = m_Wx.erase(wi);
#if !defined(QT_NO_DEBUG)
			wxDeleted++;
#endif
		}
		else
		{
			wi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			wxRunning++;
#endif
		}
	}

#if !defined(QT_NO_DEBUG)
	int delWxDeleted = 0;
	int delWxRunning = 0;
#endif
	QList<AuWeatherData*>::iterator wdi = m_WxToDelete.begin();
	while(wdi != m_WxToDelete.end())
	{
		if((*wdi)->IsDoneRunning())
		{
			delete *wdi;
			wdi = m_WxToDelete.erase(wdi);
#if !defined(QT_NO_DEBUG)
			delWxDeleted++;
#endif
		}
		else
		{
			wdi++;	// Let this one leak. Don't kill it while it's still running.
#if !defined(QT_NO_DEBUG)
			delWxRunning++;
#endif
		}
	}

	/*
#if !defined(QT_NO_DEBUG)
	qDebug() << "AuGLModelMaker cleanup";
	if(animaliaDeleted)
		qDebug() << animaliaDeleted << "animalia deleted";
	if(animaliaRunning)
		qDebug() << animaliaRunning << "animalia running...leaked";
	if(animaliaDeletedDel)
		qDebug() << animaliaDeletedDel << "deleted animalia deleted";
	if(animaliaRunningDel)
		qDebug() << animaliaRunningDel << "deleted animalia running...leaked";
	if(bdDeleted)
		qDebug() << bdDeleted << "bd deleted";
	if(bdRunning)
		qDebug() << bdRunning << "bd running...leaked";
	if(bdDeletedDel)
		qDebug() << bdDeletedDel << "deleted bd deleted";
	if(bdRunningDel)
		qDebug() << bdRunningDel << "deleted bd running...leaked";

	if(flatGlobesDeleted)
		qDebug() << flatGlobesDeleted << "flat globes deleted";
	if(flatGlobesRunning)
		qDebug() << flatGlobesRunning << "flat globes running...leaked";
	if(delFlatGlobesDeleted)
		qDebug() << delFlatGlobesDeleted << "deleted flat globes deleted";
	if(delFlatGlobesRunning)
		qDebug() << delFlatGlobesRunning << "deleted flat globes running...leaked";
	if(globesDeleted)
		qDebug() << globesDeleted << "globes deleted";
	if(globesRunning)
		qDebug() << globesRunning << "globes running...leaked";
	if(delGlobesDeleted)
		qDebug() << delGlobesDeleted << "deleted globes deleted";
	if(delGlobesRunning)
		qDebug() << delGlobesRunning << "deleted globes running...leaked";
	if(globeTrisDeleted)
		qDebug() << globeTrisDeleted << "globe tris deleted";
	if(globeTrisRunning)
		qDebug() << globeTrisRunning << "globe tris running...leaked";
	if(delGlobeTrisDeleted)
		qDebug() << delGlobeTrisDeleted << "deleted globe tris deleted";
	if(delGlobeTrisRunning)
		qDebug() << delGlobeTrisRunning << "deleted globe tris running...leaked";
	if(nearGlobesDeleted)
		qDebug() << nearGlobesDeleted << "near globes deleted";
	if(nearGlobesRunning)
		qDebug() << nearGlobesRunning << "near globes running...leaked";
	if(delNearGlobesDeleted)
		qDebug() << delNearGlobesDeleted << "deleted near globes deleted";
	if(delNearGlobesRunning)
		qDebug() << delNearGlobesRunning << "deleted near globes running...leaked";
	if(flatRingworldSectionsDeleted)
		qDebug() << flatRingworldSectionsDeleted << "flat ringworld sections deleted";
	if(flatRingworldSectionsRunning)
		qDebug() << flatRingworldSectionsRunning << "flat ringworld sections running...leaked";
	if(delFlatRingworldSectionsDeleted)
		qDebug() << delFlatRingworldSectionsDeleted << "deleted flat ringworld sections deleted";
	if(delFlatRingworldSectionsRunning)
		qDebug() << delFlatRingworldSectionsRunning << "deleted flat ringworld sections running...leaked";
	if(ringworldSectionsDeleted)
		qDebug() << ringworldSectionsDeleted << "ringworld sections deleted";
	if(ringworldSectionsRunning)
		qDebug() << ringworldSectionsRunning << "ringworld sections running...leaked";
	if(delRingworldSectionsDeleted)
		qDebug() << delRingworldSectionsDeleted << "deleted ringworld sections deleted";
	if(delRingworldSectionsRunning)
		qDebug() << delRingworldSectionsRunning << "deleted ringworld sections running...leaked";
	if(ringworldSectionTrisDeleted)
		qDebug() << ringworldSectionTrisDeleted << "ringworld section tris deleted";
	if(ringworldSectionTrisRunning)
		qDebug() << ringworldSectionTrisRunning << "ringworld section tris running...leaked";
	if(delRingworldSectionTrisDeleted)
		qDebug() << delRingworldSectionTrisDeleted << "deleted ringworld section tris deleted";
	if(delRingworldSectionTrisRunning)
		qDebug() << delRingworldSectionTrisRunning << "deleted ringworld section tris running...leaked";
	if(nearRingworldSectionsDeleted)
		qDebug() << nearRingworldSectionsDeleted << "near ringworld sections deleted";
	if(nearRingworldSectionsRunning)
		qDebug() << nearRingworldSectionsRunning << "near ringworld sections running...leaked";
	if(delNearRingworldSectionsDeleted)
		qDebug() << delNearRingworldSectionsDeleted << "deleted near ringworld sections deleted";
	if(delNearRingworldSectionsRunning)
		qDebug() << delNearRingworldSectionsRunning << "deleted near ringworld sections running...leaked";
	if(floraImagesDeleted)
		qDebug() << floraImagesDeleted << "flora images deleted";
	if(floraImagesRunning)
		qDebug() << floraImagesRunning << "flora images running...leaked";
	if(delFloraImagesDeleted)
		qDebug() << delFloraImagesDeleted << "deleted flora images deleted";
	if(delFloraImagesRunning)
		qDebug() << delFloraImagesRunning << "deleted flora images running...leaked";
	if(floraModelsDeleted)
		qDebug() << floraModelsDeleted << "flora models deleted";
	if(floraModelsRunning)
		qDebug() << floraModelsRunning << "flora models running...leaked";
	if(delFloraModelsDeleted)
		qDebug() << delFloraModelsDeleted << "deleted flora models deleted";
	if(delFloraModelsRunning)
		qDebug() << delFloraModelsRunning << "deleted flora models running...leaked";

	if(grassImagesDeleted)
		qDebug() << grassImagesDeleted << "grass images deleted";
	if(grassImagesRunning)
		qDebug() << grassImagesRunning << "grass images running...leaked";
	if(delGrassImagesDeleted)
		qDebug() << delGrassImagesDeleted << "deleted grass images deleted";
	if(delGrassImagesRunning)
		qDebug() << delGrassImagesRunning << "deleted grass images running...leaked";

	if(rocksDeleted)
		qDebug() << rocksDeleted << "rocks deleted";
	if(rocksRunning)
		qDebug() << rocksRunning << "rocks running...leaked";
	if(delRocksDeleted)
		qDebug() << delRocksDeleted << "deleted rocks deleted";
	if(delRocksRunning)
		qDebug() << delRocksRunning << "deleted rocks running...leaked";

	if(wxDeleted)
		qDebug() << wxDeleted << "weather data deleted";
	if(wxRunning)
		qDebug() << wxRunning << "weather data running...leaked";
	if(delWxDeleted)
		qDebug() << delWxDeleted << "deleted weather data deleted";
	if(delWxRunning)
		qDebug() << delWxRunning << "deleted weather data running...leaked";
#endif
	*/
}

bool AuGLModelMaker::IsBusy() const
{
	{
		for(unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::const_iterator tmi = m_Animalia.begin(); tmi != m_Animalia.end();
				tmi++)
			if(!(*tmi).second->IsDoneRunning())
				return true;										// Found a busy runnable.

		for(QList<AuAnimaliaModel*>::const_iterator tdi = m_AnimaliaToDelete.cbegin(); tdi != m_AnimaliaToDelete.cend(); tdi++)
			if(!(*tdi)->IsDoneRunning())
				return true;										// Found a busy runnable.
	}

	{
		for(unordered_multimap<quint32, AuBdModel*, AuDesignIdHash>::const_iterator tmi = m_Bd.begin(); tmi != m_Bd.end(); tmi++)
			if(!(*tmi).second->IsDoneRunning())
				return true;										// Found a busy runnable.

		for(QList<AuBdModel*>::const_iterator tdi = m_BdToDelete.cbegin(); tdi != m_BdToDelete.cend(); tdi++)
			if(!(*tdi)->IsDoneRunning())
				return true;										// Found a busy runnable.
	}

	{
		for(unordered_multimap<quint32, AuBdFrame*, AuDesignIdHash>::const_iterator tmi = m_BdFrame.begin(); tmi != m_BdFrame.end();
				tmi++)
			if(!(*tmi).second->IsDoneRunning())
				return true;										// Found a busy runnable.

		for(QList<AuBdFrame*>::const_iterator tdi = m_BdFrameToDelete.cbegin(); tdi != m_BdFrameToDelete.cend(); tdi++)
			if(!(*tdi)->IsDoneRunning())
				return true;										// Found a busy runnable.
	}

	for(unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::const_iterator tmi = m_FlatGlobes.begin(); tmi != m_FlatGlobes.end();
			tmi++)
		if(!(*tmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuFlatGlobeModel*>::const_iterator tdi = m_FlatGlobesToDelete.cbegin(); tdi != m_FlatGlobesToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(unordered_map<AuId, AuGlobeModel*, AuIdHash>::const_iterator tmi = m_Globes.begin(); tmi != m_Globes.end(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuGlobeModel*>::const_iterator tdi = m_GlobesToDelete.cbegin(); tdi != m_GlobesToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::const_iterator gtmi = m_GlobeTris.begin(); gtmi != m_GlobeTris.end();
			gtmi++)
		if(!(*gtmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuGlobeModelTri*>::const_iterator gtdi = m_GlobeTrisToDelete.cbegin(); gtdi != m_GlobeTrisToDelete.cend();
			gtdi++)
		if(!(*gtdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::const_iterator tmi = m_NearGlobes.begin(); tmi != m_NearGlobes.end();
			tmi++)
		if(!(*tmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuNearGlobeModel*>::const_iterator tdi = m_NearGlobesToDelete.cbegin(); tdi != m_NearGlobesToDelete.cend();
			tdi++)
		if(!(*tdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(unordered_map<AuId, AuGlobeModel*, AuIdHash>::const_iterator tmi = m_RingworldSections.begin();
			tmi != m_RingworldSections.end(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuGlobeModel*>::const_iterator tdi = m_RingworldSectionsToDelete.cbegin();
			tdi != m_RingworldSectionsToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::const_iterator gtmi = m_RingworldSectionTris.begin();
			gtmi != m_RingworldSectionTris.end(); gtmi++)
		if(!(*gtmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuGlobeModelTri*>::const_iterator gtdi = m_RingworldSectionTrisToDelete.cbegin();
			gtdi != m_RingworldSectionTrisToDelete.cend(); gtdi++)
		if(!(*gtdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::const_iterator tmi = m_NearRingworldSections.begin();
			tmi != m_NearRingworldSections.end(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuNearGlobeModel*>::const_iterator tdi = m_NearRingworldSectionsToDelete.cbegin();
			tdi != m_NearRingworldSectionsToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	// Flora Images
	for(unordered_map<AuId, AuFloraImage*, AuIdHash>::const_iterator fii = m_FloraImages.begin(); fii != m_FloraImages.end(); fii++)
		if(!(*fii).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuFloraImage*>::const_iterator fidi = m_FloraImagesToDelete.cbegin(); fidi != m_FloraImagesToDelete.cend();
			fidi++)
		if(!(*fidi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	// Flora
	for(unordered_map<AuId, AuFloraModel*, AuIdHash>::const_iterator fmi = m_Flora.begin(); fmi != m_Flora.end(); fmi++)
		if(!(*fmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuFloraModel*>::const_iterator fdi = m_FloraToDelete.cbegin(); fdi != m_FloraToDelete.cend(); fdi++)
		if(!(*fdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	// Grass Images
	for(unordered_multimap<AuId, AuGrassImages*, AuIdHash>::const_iterator gii = m_GrassImages.begin(); gii != m_GrassImages.end();
			gii++)
		if(!(*gii).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuGrassImages*>::const_iterator gidi = m_GrassImagesToDelete.cbegin(); gidi != m_GrassImagesToDelete.cend();
			gidi++)
		if(!(*gidi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	// Rock
	for(unordered_map<AuId, AuRockModel*, AuIdHash>::const_iterator rmi = m_Rock.begin(); rmi != m_Rock.end(); rmi++)
		if(!(*rmi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuRockModel*>::const_iterator rdi = m_RockToDelete.cbegin(); rdi != m_RockToDelete.cend(); rdi++)
		if(!(*rdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	// Weather
	for(unordered_map<AuId, AuWeatherData*, AuIdHash>::const_iterator wi = m_Wx.begin(); wi != m_Wx.end(); wi++)
		if(!(*wi).second->IsDoneRunning())
			return true;										// Found a busy runnable.

	for(QList<AuWeatherData*>::const_iterator wdi = m_WxToDelete.cbegin(); wdi != m_WxToDelete.cend(); wdi++)
		if(!(*wdi)->IsDoneRunning())
			return true;										// Found a busy runnable.

	// We get here if there are no runnables that are still running.
	return false;
}

int AuGLModelMaker::BusyThreadCount() const
{
	int busyThreads = 0;

	for(unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::const_iterator tmi = m_Animalia.cbegin(); tmi != m_Animalia.cend();
			tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuAnimaliaModel*>::const_iterator tdi = m_AnimaliaToDelete.cbegin(); tdi != m_AnimaliaToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_multimap<quint32, AuBdModel*, AuDesignIdHash>::const_iterator tmi = m_Bd.cbegin(); tmi != m_Bd.cend(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuBdModel*>::const_iterator tdi = m_BdToDelete.cbegin(); tdi != m_BdToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_multimap<quint32, AuBdFrame*, AuDesignIdHash>::const_iterator tmi = m_BdFrame.cbegin(); tmi != m_BdFrame.cend();
			tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuBdFrame*>::const_iterator tdi = m_BdFrameToDelete.cbegin(); tdi != m_BdFrameToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::const_iterator tmi = m_FlatGlobes.begin(); tmi != m_FlatGlobes.end();
			tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuFlatGlobeModel*>::const_iterator tdi = m_FlatGlobesToDelete.cbegin(); tdi != m_FlatGlobesToDelete.cend();
			tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_map<AuId, AuGlobeModel*, AuIdHash>::const_iterator tmi = m_Globes.begin(); tmi != m_Globes.end(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuGlobeModel*>::const_iterator tdi = m_GlobesToDelete.cbegin(); tdi != m_GlobesToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::const_iterator gtmi = m_GlobeTris.begin(); gtmi != m_GlobeTris.end(); gtmi++)
		if(!(*gtmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuGlobeModelTri*>::const_iterator gtdi = m_GlobeTrisToDelete.cbegin(); gtdi != m_GlobeTrisToDelete.cend();
			gtdi++)
		if(!(*gtdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::const_iterator tmi = m_NearGlobes.begin(); tmi != m_NearGlobes.end(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuNearGlobeModel*>::const_iterator tdi = m_NearGlobesToDelete.cbegin(); tdi != m_NearGlobesToDelete.cend();
			tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_map<AuId, AuGlobeModel*, AuIdHash>::const_iterator tmi = m_RingworldSections.begin(); tmi != m_RingworldSections.end();
			tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuGlobeModel*>::const_iterator tdi = m_RingworldSectionsToDelete.cbegin();
			tdi != m_RingworldSectionsToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::const_iterator gtmi = m_RingworldSectionTris.begin();
			gtmi != m_RingworldSectionTris.end(); gtmi++)
		if(!(*gtmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuGlobeModelTri*>::const_iterator gtdi = m_RingworldSectionTrisToDelete.cbegin();
			gtdi != m_RingworldSectionTrisToDelete.cend(); gtdi++)
		if(!(*gtdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::const_iterator tmi = m_NearRingworldSections.begin();
			tmi != m_NearRingworldSections.end(); tmi++)
		if(!(*tmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuNearGlobeModel*>::const_iterator tdi = m_NearRingworldSectionsToDelete.cbegin();
			tdi != m_NearRingworldSectionsToDelete.cend(); tdi++)
		if(!(*tdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	// Flora Images
	for(unordered_map<AuId, AuFloraImage*, AuIdHash>::const_iterator fii = m_FloraImages.begin(); fii != m_FloraImages.end(); fii++)
		if(!(*fii).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuFloraImage*>::const_iterator fidi = m_FloraImagesToDelete.cbegin(); fidi != m_FloraImagesToDelete.cend();
			fidi++)
		if(!(*fidi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	// Flora
	for(unordered_map<AuId, AuFloraModel*, AuIdHash>::const_iterator fmi = m_Flora.begin(); fmi != m_Flora.end(); fmi++)
		if(!(*fmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuFloraModel*>::const_iterator fdi = m_FloraToDelete.cbegin(); fdi != m_FloraToDelete.cend(); fdi++)
		if(!(*fdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	// Grass Images
	for(unordered_multimap<AuId, AuGrassImages*, AuIdHash>::const_iterator gii = m_GrassImages.begin(); gii != m_GrassImages.end(); gii++)
		if(!(*gii).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuGrassImages*>::const_iterator gidi = m_GrassImagesToDelete.cbegin(); gidi != m_GrassImagesToDelete.cend();
			gidi++)
		if(!(*gidi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	// Rock
	for(unordered_map<AuId, AuRockModel*, AuIdHash>::const_iterator rmi = m_Rock.begin(); rmi != m_Rock.end(); rmi++)
		if(!(*rmi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuRockModel*>::const_iterator rdi = m_RockToDelete.cbegin(); rdi != m_RockToDelete.cend(); rdi++)
		if(!(*rdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	// Weather
	for(unordered_map<AuId, AuWeatherData*, AuIdHash>::const_iterator wi = m_Wx.begin(); wi != m_Wx.end(); wi++)
		if(!(*wi).second->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	for(QList<AuWeatherData*>::const_iterator wdi = m_WxToDelete.cbegin(); wdi != m_WxToDelete.cend(); wdi++)
		if(!(*wdi)->IsDoneRunning())
			busyThreads++;										// Found a busy runnable.

	// Return count of runnables that are still running.
	return busyThreads;
}

bool AuGLModelMaker::Weather(AuWeather& wx, const DaWorldLL* world)
{
	//qDebug() << "AuGLModelMaker::Weather" << world->Id() << world->Name();
	unordered_map<AuId, AuWeatherData*, AuIdHash>::iterator wi = m_Wx.find(world->Id());
	if(wi != m_Wx.end())
		return (*wi).second->Weather(wx);						// Create the weather data when ready; returns true when done.
	else
	{
		// Insert a nu entry and give it to the thread pool to do the work.
		wi = m_Wx.insert(pair<AuId, AuWeatherData*>(world->Id(), new AuWeatherData(world->Id(), wx))).first;
		QThreadPool::globalInstance()->start((*wi).second, 0);			// Lower priority than terrain so terrain gets made first.
	}
	return false;
}

bool AuGLModelMaker::GrassImages(QImage& grass0, QImage& grass1, QImage& grass2, DePlanetFlora& flora, const AuId& planetId,
		quint32 genSeed, int resourceZone)
{
	for(unordered_multimap<AuId, AuGrassImages*, AuIdHash>::iterator gi = m_GrassImages.find(planetId);
			gi != m_GrassImages.end() && (*gi).first == planetId; gi++)
		if((*gi).second->ResourceZone() == resourceZone)
			return (*gi).second->GrassImages(grass0, grass1, grass2);

	// Did not find the grass images for the specific resource zone.
	// Insert a nu entry and give it to the thread pool to do the work.
	AuGrassImages* grassi = new AuGrassImages(flora, genSeed, resourceZone);
	m_GrassImages.insert(pair<AuId, AuGrassImages*>(planetId, grassi));
	QThreadPool::globalInstance()->start(grassi, 0);			// Lower priority than terrain so terrain gets made first.
	return false;
}

void AuGLModelMaker::FloraImage(QImage& imtex, DePlanetFlora& flora, const AuId& planetId, quint32 genSeed, int resourceZone)
{
	for(unordered_multimap<AuId, AuFloraImage*, AuIdHash>::iterator gi = m_FloraImages.find(planetId);
			gi != m_FloraImages.end() && (*gi).first == planetId; gi++)
	{
		if((*gi).second->ResourceZone() == resourceZone)
		{
			(*gi).second->FloraImage(flora, imtex);
			return;
		}
	}

	// Did not find the specific model. Schedule it for creation.
	// Insert a nu entry and give it to the thread pool to create the model.
	AuFloraImage* gm = new AuFloraImage(flora, genSeed, resourceZone);
	m_FloraImages.insert(pair<AuId, AuFloraImage*>(planetId, gm));
	QThreadPool::globalInstance()->start(gm, 0);			// Lowest priority so terrain gets made first.
}

bool AuGLModelMaker::CreateFlora(const DaWorldLL* world, AuPoint& groundLoc, QMap<AuResKey, AuGlobeFlora>& grass,
		QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeTree>& trees,
		QMap<AuResKey, AuGlobeFlora>& aquaticgrass, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs)
{
	unordered_map<AuId, AuFloraModel*, AuIdHash>::iterator fmi = m_Flora.find(world->Id());
	if(fmi != m_Flora.end())
	{
		// Returns true when the result is valid, even if it's empty.
		if((*fmi).second->Flora(grass, shrubs, trees, aquaticgrass, aquaticshrubs))
		{
			groundLoc = (*fmi).second->GroundLoc();
			return true;
		}
	}
	else
	{
		// Insert a nu entry and give it to the thread pool to create the model.
		fmi = m_Flora.insert(pair<AuId, AuFloraModel*>(world->Id(), new AuFloraModel(world->Id(), groundLoc))).first;
		QThreadPool::globalInstance()->start((*fmi).second, 1);			// Higher priority than ships so grass gets made first.
	}
	return false;
}

bool AuGLModelMaker::CreateRocks(const DaWorldLL* world, AuPoint& groundLoc, QMap<AuResKey, AuGlobeRock>& rocks)
{
	unordered_map<AuId, AuRockModel*, AuIdHash>::iterator rmi = m_Rock.find(world->Id());
	if(rmi != m_Rock.end())
	{
		if((*rmi).second->Rock(rocks))								// Returns true when the result is valid, even if it's empty.
		{
			groundLoc = (*rmi).second->GroundLoc();
			return true;
		}
	}
	else
	{
		// Insert a nu entry and give it to the thread pool to create the model.
		rmi = m_Rock.insert(pair<AuId, AuRockModel*>(world->Id(), new AuRockModel(world->Id(), groundLoc))).first;
		QThreadPool::globalInstance()->start((*rmi).second, 1);			// Higher priority than ships so grass gets made first.
	}
	return false;
}

// Determine the VAO id of the globe atom.
AuGLVAO AuGLModelMaker::NearGlobeModel(AuOpenGLFunctions& GLf, const DaWorldSphere* world)
{
	// Search hash for the specific globe model.
	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::iterator gi = m_NearGlobes.find(world->Id());
	if(gi != m_NearGlobes.end())
		return (*gi).second->VAO(GLf);									// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuNearGlobeModel* gm = new AuNearGlobeModel(world->Id(), world->ModifyTime(), CacheEnabled());
		m_NearGlobes.insert(pair<AuId, AuNearGlobeModel*>(world->Id(), gm));
		QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
		return AuGLVAO();
	}
}

void AuGLModelMaker::Animalia(AuOpenGLFunctions& GLf, const AuId& id, AuAnimalia& animalia)
{
	QMultiMap<int, unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator> scored;	// <100% matches sorted by score.
	QList<unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator> tooOld;
	bool making = false;	// True if we are already making an animalia for id; it is probably the one we are waiting for.
	for(unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator gi = m_Animalia.find(id);
			gi != m_Animalia.end() && (*gi).first == id; gi++)
	{
		int matchScore = (*gi).second->VisualCompare(animalia);
		if(matchScore == 100)
		{
			// Found the animalia model with 100% matching hair and clothing.
			//if(animalia.HasVAOs())
				//return;								// No need to update animalia if it already has its VAOs; they don't change.
			(*gi).second->VAO(GLf, animalia);			// Creates the model when ready; configures 'animalia' when done.
			return;
		}
		if(matchScore != 0)
			scored.insert(matchScore, gi); 				// Only finished threads return a non-zero score.
		else
			making = true;
	}

	// Did not find the exact 100% model and we are not making a model for that id. Provide the best looking alternative.
	if(!making)
	{
		// Schedule the 100% matching model for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuAnimaliaModel* gm = new AuAnimaliaModel(id, animalia);

		// The map of alternate models is sorted by score, lowest to highest.
		// Must grab this after creating the thread or the hair and clothing is wrong.
		if(!scored.empty())
		{
			(*scored.last()).second->VAO(GLf, animalia);	// Creates the model when ready; configures 'animalia' when done.
			if(scored.size() > 1)
			{
				// Throw away the oldest one.
				unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator oldest = scored.first();
				for(QMultiMap<int, unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator>::iterator si = scored.begin()+1;
						si != scored.end(); si++)
				{
					unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator ssi = *si;
					if((*ssi).second->LastUsed() < (*oldest).second->LastUsed())
						oldest = ssi;
				}

				m_AnimaliaToDelete.append((*oldest).second);
				m_Animalia.erase(oldest);
			}
		}

		m_Animalia.insert(pair<AuId, AuAnimaliaModel*>(id, gm));
		QThreadPool::globalInstance()->start(gm, 10);			// Higher priority than everything so my body is first.
	}
	else
	{
		// The map of alternate models is sorted by score, lowest to highest.
		if(!scored.empty())
		{
			(*scored.last()).second->VAO(GLf, animalia);	// Creates the model when ready; configures 'animalia' when done.
			if(scored.size() > 1)
			{
				// Throw away the oldest one.
				unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator oldest = scored.first();
				for(QMultiMap<int, unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator>::iterator si = scored.begin()+1;
						si != scored.end(); si++)
				{
					unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator ssi = *si;
					if((*ssi).second->LastUsed() < (*oldest).second->LastUsed())
						oldest = ssi;
				}

				m_AnimaliaToDelete.append((*oldest).second);
				m_Animalia.erase(oldest);
			}
		}
	}
}

bool AuGLModelMaker::Bd(AuOpenGLFunctions& GLf, quint32 designId, quint32 empireId, AuBdRender& bdRender)
{
	// Find the design/empire combination.
	unordered_multimap<quint32, AuBdModel*, AuDesignIdHash>::iterator gi = m_Bd.find(designId);
	while(gi != m_Bd.cend() && (*gi).first == designId && (*gi).second->EmpireId() != empireId)
		gi++;
	if(gi != m_Bd.end() && (*gi).first == designId)
		return (*gi).second->VAO(GLf, bdRender);				// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuBdModel* gm = new AuBdModel(designId, empireId);
		m_Bd.insert(pair<quint32, AuBdModel*>(designId, gm));
		QThreadPool::globalInstance()->start(gm, 0);			// Lower priority than terrain so terrain gets made first.
		return false;
	}
}

bool AuGLModelMaker::BdFrame(AuOpenGLFunctions& GLf, quint32 designId, COMMODITYID hullMaterial, int percentComplete,
		AuBdRender& bdRender)
{
	// Find the design/empire combination.
	unordered_multimap<quint32, AuBdFrame*, AuDesignIdHash>::iterator gi = m_BdFrame.find(designId);
	while(gi != m_BdFrame.cend() && (*gi).first == designId &&
			((*gi).second->HullMaterial() != hullMaterial ||
			 (*gi).second->PercentComplete() != percentComplete))
		gi++;
	if(gi != m_BdFrame.end() && (*gi).first == designId)
		return (*gi).second->VAO(GLf, bdRender);				// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuBdFrame* gm = new AuBdFrame(designId, hullMaterial, percentComplete);
		m_BdFrame.insert(pair<quint32, AuBdFrame*>(designId, gm));
		QThreadPool::globalInstance()->start(gm, 0);			// Lower priority than terrain so terrain gets made first.
		return false;
	}
}

// Determine the VAO id of the globe atom.
AuGLVAO AuGLModelMaker::FlatGlobeModel(AuOpenGLFunctions& GLf, const DaWorldSphere* world, float& minAltMSL, float& maxAltMSL)
{
	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::iterator gi = m_FlatGlobes.find(world->Id());
	if(gi != m_FlatGlobes.end())
		return (*gi).second->VAO(GLf, minAltMSL, maxAltMSL);				// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuFlatGlobeModel* gm = new AuFlatGlobeModel(world->Id(), world->ModifyTime(), world->CanHaveFlora(), CacheEnabled());
		m_FlatGlobes.insert(pair<AuId, AuFlatGlobeModel*>(world->Id(), gm));
		QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
		return AuGLVAO();
	}
}

// Determine the VAO id of the globe atom.
void AuGLModelMaker::GlobeModel(AuOpenGLFunctions& GLf, AuAtGlobeVAO& vao, const DaWorldSphere* world)
{
	unordered_map<AuId, AuGlobeModel*, AuIdHash>::iterator gi = m_Globes.find(world->Id());
	if(gi != m_Globes.end())
		(*gi).second->VAOs(GLf, vao);										// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuGlobeModel* gm = new AuGlobeModel(world->Id(), world->ModifyTime(), world->SeaLevelRadius(), world->CanHaveFlora(),
				CacheEnabled());
		m_Globes.insert(pair<AuId, AuGlobeModel*>(world->Id(), gm));
		QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
	}
}

// Determine the VAO id of the globe atom.
void AuGLModelMaker::GlobeModelTri(AuOpenGLFunctions& GLf, AuGLTriVAO& vao, const DaWorldSphere* world, int triIndex,
		const AuGlobeLLTri& tri, float maxAltMSL, float minAltMSL)
{
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gi = m_GlobeTris.find(world->Id());
			gi != m_GlobeTris.end() && (*gi).first == world->Id(); gi++)
	{
		if((*gi).second->TriIndex() == triIndex)
		{
			// Grab the VAO. It could be invalid. We'll return the invalid one until we find a valid one. That way terrain updates
			// don't leave a hole in the ground until the nu patch is ready.
			vao = (*gi).second->VAO(GLf);
			if(vao.IsValid())
				return;
		}
	}

	// Did not find the specific model or the one we have is invalid. Schedule it for creation.
	// Insert a nu entry and give it to the thread pool to create the model.
	AuGlobeModelTri* gm = new AuGlobeModelTri(world->Id(), triIndex, tri, world->SeaLevelRadius(),
			world->CanHaveFlora() || world->BdHere(), maxAltMSL, minAltMSL);
	m_GlobeTris.insert(pair<AuId, AuGlobeModelTri*>(world->Id(), gm));
	QThreadPool::globalInstance()->start(gm, 1);				// Higher priority than ships so globe gets made first.
}

void AuGLModelMaker::InvalidateGlobeModelTri(const AuId& worldId, int triIndex)
{
	// Check for a valid one that is complete and an invalid one for the same index.
	// In that case we must remove the invalid one or we end up with two usable invalid ones, which is ambiguous.
	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator ii = m_GlobeTris.end();
	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator vi = m_GlobeTris.end();
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gi = m_GlobeTris.find(worldId);
			gi != m_GlobeTris.end() && (*gi).first == worldId; gi++)
		if((*gi).second->TriIndex() == triIndex)
		{
			// Stop if we find one that is not done.
			if(!(*gi).second->IsDoneRunning())
				break;

			if((*gi).second->IsValid())
				vi = gi;
			else
				ii = gi;
		}
	if(vi != m_GlobeTris.end() && ii != m_GlobeTris.end())
	{
		// We have two models that are usable, one is valid the other is invalid.
		// Delete the invalid one.
		(*ii).second->DeleteGL();
		delete (*ii).second;
		m_GlobeTris.erase(ii);
	}

	// Invalidate remaining ones even if they are still running.
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gi = m_GlobeTris.find(worldId);
			gi != m_GlobeTris.end() && (*gi).first == worldId; gi++)
		if((*gi).second->TriIndex() == triIndex)
			(*gi).second->SetValid(false);
}

// Determine the VAO id of the globe atom.
AuGLVAO AuGLModelMaker::FlatRingworldSectionModel(AuOpenGLFunctions& GLf, const DaWorldRingSection* world, float& minAltMSL,
		float& maxAltMSL)
{
	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::iterator gi = m_FlatRingworldSections.find(world->Id());
	if(gi != m_FlatRingworldSections.end())
		return (*gi).second->VAO(GLf, minAltMSL, maxAltMSL);				// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuFlatGlobeModel* gm = new AuFlatGlobeModel(world->Id(), world->ModifyTime(), world->CanHaveFlora(), CacheEnabled());
		m_FlatRingworldSections.insert(pair<AuId, AuFlatGlobeModel*>(world->Id(), gm));
		QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
		return AuGLVAO();
	}
}

// Determine the VAO id of the globe atom.
AuGLVAO AuGLModelMaker::NearRingworldSectionModel(AuOpenGLFunctions& GLf, const DaWorldRingSection* world)
{
	// Search multi hash for the specific globe model.
	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::iterator gi = m_NearRingworldSections.find(world->Id());
	if(gi != m_NearRingworldSections.end())
		return (*gi).second->VAO(GLf);									// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuNearGlobeModel* gm = new AuNearGlobeModel(world->Id(), world->ModifyTime(), CacheEnabled());
		m_NearRingworldSections.insert(pair<AuId, AuNearGlobeModel*>(world->Id(), gm));
		QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
		return AuGLVAO();
	}
}

// Determine the VAO id of the globe atom.
void AuGLModelMaker::RingworldSectionModel(AuOpenGLFunctions& GLf, AuAtGlobeVAO& vao, const DaWorldRingSection* world)
{
	unordered_map<AuId, AuGlobeModel*, AuIdHash>::iterator gi = m_RingworldSections.find(world->Id());
	if(gi != m_RingworldSections.end())
		(*gi).second->VAOs(GLf, vao);										// Creates the model when ready; returns id when done.
	else
	{
		// Did not find the specific model. Schedule it for creation.
		// Insert a nu entry and give it to the thread pool to create the model.
		AuGlobeModel* gm = new AuGlobeModel(world->Id(), world->ModifyTime(), 0, world->CanHaveFlora(), CacheEnabled());
		m_RingworldSections.insert(pair<AuId, AuGlobeModel*>(world->Id(), gm));
		QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
	}
}

// Determine the VAO id of the globe atom.
void AuGLModelMaker::RingworldSectionModelTri(AuOpenGLFunctions& GLf, AuGLTriVAO& vao, const DaWorldRingSection* world,
		int triIndex, const AuGlobeLLTri& tri, float maxAltMSL, float minAltMSL)
{
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gi = m_RingworldSectionTris.find(world->Id());
			gi != m_RingworldSectionTris.end() && (*gi).first == world->Id(); gi++)
	{
		if((*gi).second->TriIndex() == triIndex)
		{
			// Grab the VAO. It could be invalid. We'll return the invalid one until we find a valid one. That way terrain updates
			// don't leave a hole in the ground until the nu patch is ready.
			vao = (*gi).second->VAO(GLf);
			if(vao.IsValid())
				return;
		}
	}

	// Did not find the specific model or the one we have is invalid. Schedule it for creation.
	// Insert a nu entry and give it to the thread pool to create the model.
	AuGlobeModelTri* gm = new AuGlobeModelTri(world->Id(), triIndex, tri, 0,
			world->CanHaveFlora() || world->BdHere(), maxAltMSL, minAltMSL);
	m_RingworldSectionTris.insert(pair<AuId, AuGlobeModelTri*>(world->Id(), gm));
	QThreadPool::globalInstance()->start(gm, 1);			// Higher priority than ships so globe gets made first.
}

void AuGLModelMaker::InvalidateRingworldSectionModelTri(const AuId& worldId, int triIndex)
{
	// Check for a valid one that is complete and an invalid one for the same index.
	// In that case we must remove the invalid one or we end up with two usable invalid ones, which is ambiguous.
	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator ii = m_RingworldSectionTris.end();
	unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator vi = m_RingworldSectionTris.end();
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gi = m_RingworldSectionTris.find(worldId);
			gi != m_RingworldSectionTris.end() && (*gi).first == worldId; gi++)
		if((*gi).second->TriIndex() == triIndex)
		{
			// Stop if we find one that is not done.
			if(!(*gi).second->IsDoneRunning())
				break;

			if((*gi).second->IsValid())
				vi = gi;
			else
				ii = gi;
		}
	if(vi != m_RingworldSectionTris.end() && ii != m_RingworldSectionTris.end())
	{
		// We have two models that are usable, one is valid the other is invalid.
		// Delete the invalid one.
		(*ii).second->DeleteGL();
		delete (*ii).second;
		m_RingworldSectionTris.erase(ii);
	}

	// Invalidate remaining ones even if they are still running.
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gi = m_RingworldSectionTris.find(worldId);
			gi != m_RingworldSectionTris.end() && (*gi).first == worldId; gi++)
		if((*gi).second->TriIndex() == triIndex)
			(*gi).second->SetValid(false);
}

void AuGLModelMaker::DeleteFloraImage(const AuId& worldId, int resourceZone)
{
	//qDebug() << "AuGLModelMaker::DeleteFloraImage" << worldId;
	// Delete flora images for all resource zones.
	for(unordered_multimap<AuId, AuFloraImage*, AuIdHash>::iterator fii = m_FloraImages.find(worldId);
			fii != m_FloraImages.end() && (*fii).first == worldId; fii++)
	{
		// The flora image is in the multi hash.
		// We must be careful with it because another thread may be running it.
		if((*fii).second->ResourceZone() == resourceZone)
		{
			m_FloraImagesToDelete.append((*fii).second);
			m_FloraImages.erase(fii);
			break;
		}
	}

	// Delete all flora images from the delete queue of runnables that are done making their models.
	QList<AuFloraImage*>::iterator fdi = m_FloraImagesToDelete.begin();
	while(fdi != m_FloraImagesToDelete.end())
	{
		if((*fdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteFloraImage" << worldId << "done running";
			// Delete the grass object. It is just data. There is no GL object associated with it.
			delete *fdi;
			fdi = m_FloraImagesToDelete.erase(fdi);
			//qDebug() << "AuGLModelMaker::DeleteFloraImage" << worldId << "terrain model deleted";
		}
		else
		{
			fdi++;
			//qDebug() << "AuGLModelMaker::DeleteFloraImage" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteFloraImage" << worldId << "done";
}

void AuGLModelMaker::DeleteFloraImages(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteFloraImages" << worldId;
	// Delete flora images for all resource zones.
	for(unordered_multimap<AuId, AuFloraImage*, AuIdHash>::iterator fii = m_FloraImages.find(worldId);
			fii != m_FloraImages.end() && (*fii).first == worldId; )
	{
		// The flora image is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_FloraImagesToDelete.append((*fii).second);
		fii = m_FloraImages.erase(fii);
	}

	// Delete all flora images from the delete queue of runnables that are done making their models.
	QList<AuFloraImage*>::iterator fdi = m_FloraImagesToDelete.begin();
	while(fdi != m_FloraImagesToDelete.end())
	{
		if((*fdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteFloraImages" << worldId << "done running";
			// Delete the grass object. It is just data. There is no GL object associated with it.
			delete *fdi;
			fdi = m_FloraImagesToDelete.erase(fdi);
			//qDebug() << "AuGLModelMaker::DeleteFloraImages" << worldId << "terrain model deleted";
		}
		else
		{
			fdi++;
			//qDebug() << "AuGLModelMaker::DeleteFloraImages" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteFloraImages" << worldId << "done";
}

void AuGLModelMaker::DeleteGrassImages(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteGrassImages" << worldId;
	// Delete grass images for all resource zones.
	for(unordered_multimap<AuId, AuGrassImages*, AuIdHash>::iterator gii = m_GrassImages.find(worldId);
			gii != m_GrassImages.end() && (*gii).first == worldId; )
	{
		// The flora image is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_GrassImagesToDelete.append((*gii).second);
		gii = m_GrassImages.erase(gii);
	}

	// Delete all flora images from the delete queue of runnables that are done making their models.
	QList<AuGrassImages*>::iterator gdi = m_GrassImagesToDelete.begin();
	while(gdi != m_GrassImagesToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteGrassImages" << worldId << "done running";
			// Delete the grass object. It is just data. There is no GL object associated with it.
			delete *gdi;
			gdi = m_GrassImagesToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteGrassImages" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteGrassImages" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteGrassImages" << worldId << "done";
}

void AuGLModelMaker::DeleteWeather(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteWeather" << worldId;
	// Delete the list object.
	unordered_map<AuId, AuWeatherData*, AuIdHash>::iterator wi = m_Wx.find(worldId);
	if(wi != m_Wx.end())
	{
		// We must be careful with it because another thread may be running it.
		m_WxToDelete.append((*wi).second);
		m_Wx.erase(wi);
	}

	// Delete all weather from the delete queue of runnables that are done making their models.
	QList<AuWeatherData*>::iterator wdi = m_WxToDelete.begin();
	while(wdi != m_WxToDelete.end())
	{
		if((*wdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteWeather" << worldId << "done running";
			// Delete the grass object. It is just data. There is no GL object associated with it.
			delete *wdi;
			wdi = m_WxToDelete.erase(wdi);
			//qDebug() << "AuGLModelMaker::DeleteWeather" << worldId << "terrain model deleted";
		}
		else
		{
			wdi++;
			//qDebug() << "AuGLModelMaker::DeleteWeather" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteWeather" << worldId << "done";
}

void AuGLModelMaker::DeleteFloraModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteFloraModel" << worldId;
	// Delete the display list object.
	unordered_map<AuId, AuFloraModel*, AuIdHash>::iterator fmi = m_Flora.find(worldId);
	if(fmi != m_Flora.end())
	{
		// We must be careful with it because another thread may be running it.
		m_FloraToDelete.append((*fmi).second);
		m_Flora.erase(fmi);
	}

	// Delete all grass models from the delete queue of runnables that are done making their models.
	QList<AuFloraModel*>::iterator fdi = m_FloraToDelete.begin();
	while(fdi != m_FloraToDelete.end())
	{
		if((*fdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteFloraModel" << worldId << "done running";
			// Delete the grass object. It is just data. There is no GL object associated with it.
			delete *fdi;
			fdi = m_FloraToDelete.erase(fdi);
			//qDebug() << "AuGLModelMaker::DeleteFloraModel" << worldId << "terrain model deleted";
		}
		else
		{
			fdi++;
			//qDebug() << "AuGLModelMaker::DeleteFloraModel" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteFloraModel" << worldId << "done";
}

void AuGLModelMaker::DeleteRockModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteRockModel" << worldId;
	// Delete the display list object.
	unordered_map<AuId, AuRockModel*, AuIdHash>::iterator rmi = m_Rock.find(worldId);
	if(rmi != m_Rock.end())
	{
		// We must be careful with it because another thread may be running it.
		m_RockToDelete.append((*rmi).second);
		m_Rock.erase(rmi);
	}

	// Delete all grass models from the delete queue of runnables that are done making their models.
	QList<AuRockModel*>::iterator fdi = m_RockToDelete.begin();
	while(fdi != m_RockToDelete.end())
	{
		if((*fdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteRockModel" << worldId << "done running";
			// Delete the grass object. It is just data. There is no GL object associated with it.
			delete *fdi;
			fdi = m_RockToDelete.erase(fdi);
			//qDebug() << "AuGLModelMaker::DeleteRockModel" << worldId << "terrain model deleted";
		}
		else
		{
			fdi++;
			//qDebug() << "AuGLModelMaker::DeleteRockModel" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteRockModel" << worldId << "done";
}

void AuGLModelMaker::DeleteNearGlobeModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteNearGlobeModel" << worldId;
	// Delete the display list object.
	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::iterator gi = m_NearGlobes.find(worldId);
	if(gi != m_NearGlobes.end())
	{
		// The globe model is in the hash.
		// We must be careful with it because another thread may be running it.
		m_NearGlobesToDelete.append((*gi).second);
		m_NearGlobes.erase(gi);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuNearGlobeModel*>::iterator gdi = m_NearGlobesToDelete.begin();
	while(gdi != m_NearGlobesToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteNearGlobeModel" << worldId << "done running";
			// Delete the gl model while the context is current.
			if((*gdi)->VAOId())
				(*gdi)->VAOEx().DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteNearGlobeModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_NearGlobesToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteNearGlobeModel" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteNearGlobeModel" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteNearGlobeModel" << worldId << "done";
}

void AuGLModelMaker::DeleteAnimalia(const AuId& id)
{
	//qDebug() << "AuGLModelMaker::DeleteAnimalia" << id;
	// Delete the display list object.
	unordered_multimap<AuId, AuAnimaliaModel*, AuIdHash>::iterator gi = m_Animalia.begin();
	while(gi != m_Animalia.end())
	{
		if((*gi).first == id)
		{
			// Delete all of the ones with that id.
			m_AnimaliaToDelete.append((*gi).second);
			gi = m_Animalia.erase(gi);
		}
		else
			gi++;
	}

	// Delete all animalia from the delete queue of runnables that are done making their models.
	QList<AuAnimaliaModel*>::iterator gdi = m_AnimaliaToDelete.begin();
	while(gdi != m_AnimaliaToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteAnimalia" << id << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteAnimalia" << id << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_AnimaliaToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteAnimalia" << id << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteAnimalia" << id << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteAnimalia" << id << "done";
}

void AuGLModelMaker::DeleteBd(quint32 designId)
{
	//qDebug() << "AuGLModelMaker::DeleteBd" << designId;
	// Delete the display list object.
	unordered_multimap<quint32, AuBdModel*, AuDesignIdHash>::iterator gi = m_Bd.begin();
	while(gi != m_Bd.end())
	{
		if((*gi).first == designId)
		{
			// Delete all of the ones with that designId.
			m_BdToDelete.append((*gi).second);
			gi = m_Bd.erase(gi);
		}
		else
			gi++;
	}

	// Delete all models from the delete queue of runnables that are done making their models.
	QList<AuBdModel*>::iterator gdi = m_BdToDelete.begin();
	while(gdi != m_BdToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteBd" << designId << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteBd" << designId << "gl vao deleted";

			// Delete the model object.
			delete *gdi;
			gdi = m_BdToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteBd" << designId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteBd" << designId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteBd" << designId << "done";
}

void AuGLModelMaker::DeleteBdFrame(quint32 designId)
{
	//qDebug() << "AuGLModelMaker::DeleteBdFrame" << designId;
	// Delete the display list object.
	unordered_multimap<quint32, AuBdFrame*, AuDesignIdHash>::iterator gi = m_BdFrame.begin();
	while(gi != m_BdFrame.end())
	{
		if((*gi).first == designId)
		{
			// Delete all of the ones with that designId.
			m_BdFrameToDelete.append((*gi).second);
			gi = m_BdFrame.erase(gi);
		}
		else
			gi++;
	}

	// Delete all models from the delete queue of runnables that are done making their models.
	QList<AuBdFrame*>::iterator gdi = m_BdFrameToDelete.begin();
	while(gdi != m_BdFrameToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteBdFrame" << designId << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteBdFrame" << designId << "gl vao deleted";

			// Delete the model object.
			delete *gdi;
			gdi = m_BdFrameToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteBdFrame" << designId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteBdFrame" << designId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteBdFrame" << designId << "done";
}

void AuGLModelMaker::DeleteFlatGlobeModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteFlatGlobeModel" << worldId << "deleteCache" << deleteCache;
	// Delete the display list object.
	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::iterator gi = m_FlatGlobes.find(worldId);
	if(gi != m_FlatGlobes.end())
	{
		// The globe model is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_FlatGlobesToDelete.append((*gi).second);
		m_FlatGlobes.erase(gi);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuFlatGlobeModel*>::iterator gdi = m_FlatGlobesToDelete.begin();
	while(gdi != m_FlatGlobesToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteFlatGlobeModel" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteFlatGlobeModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_FlatGlobesToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteFlatGlobeModel" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteFlatGlobeModel" << worldId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteFlatGlobeModel" << worldId << "done";
}

void AuGLModelMaker::DeleteFlatRingworldSectionModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteFlatRingworldSectionModel" << worldId << "deleteCache" << deleteCache;
	// Delete the display list object.
	unordered_map<AuId, AuFlatGlobeModel*, AuIdHash>::iterator gi = m_FlatRingworldSections.find(worldId);
	if(gi != m_FlatRingworldSections.end())
	{
		// The globe model is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_FlatRingworldSectionsToDelete.append((*gi).second);
		m_FlatRingworldSections.erase(gi);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuFlatGlobeModel*>::iterator gdi = m_FlatRingworldSectionsToDelete.begin();
	while(gdi != m_FlatRingworldSectionsToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteFlatRingworldSectionModel" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteFlatRingworldSectionModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_FlatRingworldSectionsToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteFlatRingworldSectionModel" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteFlatRingworldSectionModel" << worldId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteFlatRingworldSectionModel" << worldId << "done";
}

void AuGLModelMaker::DeleteGlobeModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "deleteCache" << deleteCache;
	// Delete the display list object.
	unordered_map<AuId, AuGlobeModel*, AuIdHash>::iterator gi = m_Globes.find(worldId);
	if(gi != m_Globes.end())
	{
		// The globe model is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_GlobesToDelete.append((*gi).second);
		m_Globes.erase(gi);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuGlobeModel*>::iterator gdi = m_GlobesToDelete.begin();
	while(gdi != m_GlobesToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_GlobesToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "STILL running";
		}
	}

	// Delete individual high-tessellation triangles.
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gti = m_GlobeTris.find(worldId);
			gti != m_GlobeTris.end() && (*gti).first == worldId;)
	{
		// The globe model is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_GlobeTrisToDelete.append((*gti).second);
		gti = m_GlobeTris.erase(gti);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuGlobeModelTri*>::iterator gtdi = m_GlobeTrisToDelete.begin();
	while(gtdi != m_GlobeTrisToDelete.end())
	{
		if((*gtdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gtdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gtdi;
			gtdi = m_GlobeTrisToDelete.erase(gtdi);
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "terrain model deleted";
		}
		else
		{
			(*gtdi)->StopRunning();
			gtdi++;
			//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteGlobeModel" << worldId << "done";
}

void AuGLModelMaker::DeleteGlobeModelTri(const AuId& worldId, int triIndex)
{
	//qDebug() << "AuGLModelMaker::DeleteGlobeModelTri" << worldId << "triIndex" << triIndex;
	// Delete individual high-tessellation triangles.
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gti = m_GlobeTris.find(worldId);
			gti != m_GlobeTris.end() && (*gti).first == worldId;)
	{
		if((*gti).second->TriIndex() == triIndex)
		{
			// The globe tri model is in the multi hash.
			// We must be careful with it because another thread may be running it.
			m_GlobeTrisToDelete.append((*gti).second);
			gti = m_GlobeTris.erase(gti);
		}
		else
			gti++;
	}

	// Delete all globe tri models from the delete queue of runnables that are done making their models.
	QList<AuGlobeModelTri*>::iterator gtdi = m_GlobeTrisToDelete.begin();
	while(gtdi != m_GlobeTrisToDelete.end())
	{
		if((*gtdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteGlobeModelTri" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gtdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteGlobeModelTri" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gtdi;
			gtdi = m_GlobeTrisToDelete.erase(gtdi);
			//qDebug() << "AuGLModelMaker::DeleteGlobeModelTri" << worldId << "terrain model deleted";
		}
		else
		{
			(*gtdi)->StopRunning();
			gtdi++;
			//qDebug() << "AuGLModelMaker::DeleteGlobeModelTri" << worldId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteGlobeModelTri" << worldId << "done";
}

void AuGLModelMaker::DeleteRingworldSectionModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "deleteCache" << deleteCache;
	// Delete the display list object.
	unordered_map<AuId, AuGlobeModel*, AuIdHash>::iterator gi = m_RingworldSections.find(worldId);
	if(gi != m_RingworldSections.end())
	{
		// The globe model is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_RingworldSectionsToDelete.append((*gi).second);
		m_RingworldSections.erase(gi);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuGlobeModel*>::iterator gdi = m_RingworldSectionsToDelete.begin();
	while(gdi != m_RingworldSectionsToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_RingworldSectionsToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "STILL running";
		}
	}

	// Delete individual high-tessellation triangles.
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gti = m_RingworldSectionTris.find(worldId);
			gti != m_RingworldSectionTris.end() && (*gti).first == worldId;)
	{
		// The globe model is in the multi hash.
		// We must be careful with it because another thread may be running it.
		m_RingworldSectionTrisToDelete.append((*gti).second);
		gti = m_RingworldSectionTris.erase(gti);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuGlobeModelTri*>::iterator gtdi = m_RingworldSectionTrisToDelete.begin();
	while(gtdi != m_RingworldSectionTrisToDelete.end())
	{
		if((*gtdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gtdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gtdi;
			gtdi = m_RingworldSectionTrisToDelete.erase(gtdi);
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "terrain model deleted";
		}
		else
		{
			(*gtdi)->StopRunning();
			gtdi++;
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModel" << worldId << "done";
}

void AuGLModelMaker::DeleteRingworldSectionModelTri(const AuId& worldId, int triIndex)
{
	//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModelTri" << worldId << "triIndex" << triIndex;
	// Delete individual high-tessellation triangles.
	for(unordered_multimap<AuId, AuGlobeModelTri*, AuIdHash>::iterator gti = m_RingworldSectionTris.find(worldId);
			gti != m_RingworldSectionTris.end() && (*gti).first == worldId;)
	{
		if((*gti).second->TriIndex() == triIndex)
		{
			// The globe tri model is in the multi hash.
			// We must be careful with it because another thread may be running it.
			m_RingworldSectionTrisToDelete.append((*gti).second);
			gti = m_RingworldSectionTris.erase(gti);
		}
		else
			gti++;
	}

	// Delete all globe tri models from the delete queue of runnables that are done making their models.
	QList<AuGlobeModelTri*>::iterator gtdi = m_RingworldSectionTrisToDelete.begin();
	while(gtdi != m_RingworldSectionTrisToDelete.end())
	{
		if((*gtdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModelTri" << worldId << "done running";
			// Delete the gl models while the context is current.
			(*gtdi)->DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModelTri" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gtdi;
			gtdi = m_RingworldSectionTrisToDelete.erase(gtdi);
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModelTri" << worldId << "terrain model deleted";
		}
		else
		{
			(*gtdi)->StopRunning();
			gtdi++;
			//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModelTri" << worldId << "STILL running";
		}
	}

	//qDebug() << "AuGLModelMaker::DeleteRingworldSectionModelTri" << worldId << "done";
}

void AuGLModelMaker::DeleteNearRingworldSectionModel(const AuId& worldId)
{
	//qDebug() << "AuGLModelMaker::DeleteNearRingworldSectionModel" << worldId;
	// Delete the display list object.
	unordered_map<AuId, AuNearGlobeModel*, AuIdHash>::iterator gi = m_NearRingworldSections.find(worldId);
	if(gi != m_NearRingworldSections.end())
	{
		// The globe model is in the hash.
		// We must be careful with it because another thread may be running it.
		m_NearRingworldSectionsToDelete.append((*gi).second);
		m_NearRingworldSections.erase(gi);
	}

	// Delete all globe models from the delete queue of runnables that are done making their models.
	QList<AuNearGlobeModel*>::iterator gdi = m_NearRingworldSectionsToDelete.begin();
	while(gdi != m_NearRingworldSectionsToDelete.end())
	{
		if((*gdi)->IsDoneRunning())
		{
			//qDebug() << "AuGLModelMaker::DeleteNearRingworldSectionModel" << worldId << "done running";
			// Delete the gl model while the context is current.
			if((*gdi)->VAOId())
				(*gdi)->VAOEx().DeleteGL();
			//qDebug() << "AuGLModelMaker::DeleteNearRingworldSectionModel" << worldId << "gl vao deleted";

			// Delete the terrain model object.
			delete *gdi;
			gdi = m_NearRingworldSectionsToDelete.erase(gdi);
			//qDebug() << "AuGLModelMaker::DeleteNearRingworldSectionModel" << worldId << "terrain model deleted";
		}
		else
		{
			gdi++;
			//qDebug() << "AuGLModelMaker::DeleteNearRingworldSectionModel" << worldId << "STILL running";
		}
	}
	//qDebug() << "AuGLModelMaker::DeleteNearRingworldSectionModel" << worldId << "done";
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAnimaliaModel::AuAnimaliaModel(const AuId& animalId, const AuAnimalia& animalia) : m_Animalia(animalia)
{
	m_AnimalId = animalId;
	m_Animalia.ClearVAOs();
	m_LastUsed.Set();
	/*
	qDebug() << "AuAnimaliaModel id" << animalId
		<< "chromatid" << animalia.Chromatid()
		<< "hat" << g_Au.CommodityName(m_Animalia.WornHat())
		<< "armor" << g_Au.CommodityName(m_Animalia.WornArmor())
		<< "clothes" << g_Au.CommodityName(m_Animalia.WornClothing());
		*/
}

AuAnimaliaModel::~AuAnimaliaModel()
{
}

void AuAnimaliaModel::DeleteGL()
{
	m_Animalia.DeleteGL();
}

void AuAnimaliaModel::VAO(AuOpenGLFunctions& GLf, AuAnimalia& animalia)
{
	if(!IsDoneRunning())
		return;
	if(!m_Animalia.HasVAOs())
		m_Animalia.CreateVAOs(GLf, m_SkinMesh, m_ClothingMesh);
	if(m_Animalia.HasVAOs())
		animalia.CopyVAOs(m_Animalia);
	m_LastUsed.Set();
}

void AuAnimaliaModel::run()
{
	if(!RunStart())
		return;
	m_Animalia.CreateMeshes(m_SkinMesh, m_ClothingMesh, m_AnimalId.GetID());
	RunFinish();
}

int AuAnimaliaModel::VisualCompare(const AuAnimalia& animalia) const
{
	if(!IsDoneRunning())
		return 0;						// Return 0 when the thread is not finished.
	int c = m_Animalia.VisualCompare(animalia);
	if(!c)
		return 1;						// Only return 0 if the thread is not finished.
	return c;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdModel::AuBdModel(quint32 designId, quint32 empireId)
{
	m_DesignId = designId;
	m_EmpireId = empireId;
}

void AuBdModel::DeleteGL()
{
	m_BdRender.DeleteGL();
}

bool AuBdModel::VAO(AuOpenGLFunctions& GLf, AuBdRender& bdRender)
{
	if(!IsDoneRunning())
		return false;
	if(!m_BdRender.m_VAOsCreated)
		m_BdRender.CreateVAOs(GLf);
	bdRender.CopyVAOs(m_BdRender);
	return true;
}

void AuBdModel::run()
{
	if(!RunStart())
		return;
	//qDebug() << "AuBdModel::run design" << m_DesignId << "empire" << m_EmpireId;

	// Get empire theme colors, or default theme colors if the empire doesn't exist.
	QColor themeColors[AuScFace::THEME_Count];
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireThemeColors(EmpireId(), themeColors);
	g_Au.UnlockData();

	const DgData& sdd = g_Au.LockBdDesignsForRead();
	AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find(m_DesignId);
	if(di != sdd.m_BdDesigns->cend())
	{
		QSet<quint32> bdDesigns;
		for(AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->cbegin(); di != sdd.m_BdDesigns->cend(); di++)
			bdDesigns.insert((*di).first);
		m_BdRender.PrepareForRendering((*di).second.Parts(), bdDesigns, themeColors);	// Must call before CreatePartTextures
	}
	g_Au.UnlockBdDesigns();
	RunFinish();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdFrame::AuBdFrame(quint32 designId, COMMODITYID hullMaterial, int percentComplete)
{
	//qDebug() << "AuBdFrame::AuBdFrame design" << designId << "matl" << g_Au.CommodityName(hullMaterial) << percentComplete << "%";
	m_DesignId = designId;
	m_HullMaterial = hullMaterial;
	m_PercentComplete = percentComplete;
}

void AuBdFrame::DeleteGL()
{
	m_BdRender.DeleteGL();
}

bool AuBdFrame::VAO(AuOpenGLFunctions& GLf, AuBdRender& bdRender)
{
	if(!IsDoneRunning())
		return false;
	if(!m_BdRender.m_VAOsCreated)
		m_BdRender.CreateVAOs(GLf);
	bdRender.CopyVAOs(m_BdRender);
	return true;
}

void AuBdFrame::run()
{
	if(!RunStart())
		return;
	const DgData& sdd = g_Au.LockBdDesignsForRead();
	AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find(m_DesignId);
	if(di != sdd.m_BdDesigns->cend())
	{
		QSet<quint32> bdDesigns;
		for(AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->cbegin(); di != sdd.m_BdDesigns->cend(); di++)
			bdDesigns.insert((*di).first);
		m_BdRender.PrepareForRenderingFrame((*di).second.Parts(), HullMaterial(), PercentComplete());
	}
	g_Au.UnlockBdDesigns();
	RunFinish();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuFlatGlobeModel::AuFlatGlobeModel(const AuId& worldId, const AuTime& worldModTime, bool worldCanHaveFlora, bool cacheEnabled)
{
	m_CacheEnabled = cacheEnabled;
	m_MaxAltMSL = 0;
	m_MinAltMSL = 0;
	m_WorldCanHaveFlora = worldCanHaveFlora;
	m_WorldId = worldId;
	m_WorldModifyTime = worldModTime;
}

// Returns the VAO of this globe.
// Creates the model from the cache data and sets m_VAO to the result.
// Returns m_VAO, which may be 0 if it's not ready yet.
// NOTE: A gl context must be current when this function is called.
AuGLVAO AuFlatGlobeModel::VAO(AuOpenGLFunctions& GLf, float& minAltMSL, float& maxAltMSL)
{
	if(!IsDoneRunning())
		return AuGLVAO();

	if(!m_VAO.VAO() && !m_GlobeTris.isEmpty())
	{
		// Store count of triangles in the vertex array object.
		int vertices = m_GlobeTris.count()*3;

		float altRange = m_MaxAltMSL - m_MinAltMSL;			// Altitude range of all land, for altitude factor calc.
		if(m_WorldCanHaveFlora || true)
		{
			// Copy surface triangles into gl array buffers.
			GLfloat* globePos = new GLfloat[vertices*4];		// vertices for triangles that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*4];		// texture coords, altitude, veg density for each vertex

			unsigned int globePosIdx = 0;
			unsigned int globeTexIdx = 0;
			for(QList<AuGlobeTriAltVeg>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
			{
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Snow;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert0.m_Altitude - m_MinAltMSL) / altRange;	// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_Vegetation/100.f;

				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Snow;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert1.m_Altitude - m_MinAltMSL) / altRange;	// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_Vegetation/100.f;

				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Snow;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert2.m_Altitude - m_MinAltMSL) / altRange;	// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_Vegetation/100.f;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_VAO.CreateGL(GLf, vertices, 2);

			m_VAO.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(0));
			GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(1));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_VAO.DeselectGL();

			delete [] globePos;
			delete [] globeTex;
		}
		else
		{
			// Copy surface triangles into gl array buffers.
			GLfloat* globePos = new GLfloat[vertices*4];		// vertices for triangles that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*3];		// texture coords, altitude for each of the vertices

			unsigned int globePosIdx = 0;
			unsigned int globeTexIdx = 0;
			for(QList<AuGlobeTriAltVeg>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
			{
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Snow;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert0.m_Altitude - m_MinAltMSL) / altRange;	// Altitude in range of 0..1.

				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Snow;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert1.m_Altitude - m_MinAltMSL) / altRange;	// Altitude in range of 0..1.

				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Snow;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert2.m_Altitude - m_MinAltMSL) / altRange;	// Altitude in range of 0..1.
			}

			// Generate vertex array(s) for this object. We only need one.
			m_VAO.CreateGL(GLf, vertices, 2);

			m_VAO.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(0));
			GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(1));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_VAO.DeselectGL();

			delete [] globePos;
			delete [] globeTex;
		}

		// These triangles are no longer needed after the VAO has been created.
		m_GlobeTris.clear();
	}
	minAltMSL = m_MinAltMSL;
	maxAltMSL = m_MaxAltMSL;
	return m_VAO;
}

// This guy does the grunt work to create the model of the globe.
void AuFlatGlobeModel::run()
{
	//qDebug() << "AuFlatGlobeModel::run" << m_WorldId;
	if(!RunStart())
		return;

	Q_ASSERT(m_GlobeTris.isEmpty());

	// Try loading from cache.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("WGF%1").arg(m_WorldId.GetID()));
	QFile cacheFile(cacheFileName);
	if(CacheEnabled() && cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;		// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == FLATGLOBE_MODEL_CACHE && fileVersion == FLATGLOBE_MODEL_VER && storeTime >= m_WorldModifyTime.UTC())
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
				ds >> m_GlobeTris;
				ds >> m_MinAltMSL;
				ds >> m_MaxAltMSL;
				if(c_CacheDebugs)
					qDebug() << "AuFlatGlobeModel::run loaded" << m_GlobeTris.count() << m_MinAltMSL << m_MaxAltMSL;
			}
		}

		// Close the file.
		cacheFile.close();
	}

	if(m_GlobeTris.isEmpty())
	{
		// Create the models since they were not loaded from the cache.
		if(c_CacheDebugs)
			qDebug() << "AuFlatGlobeModel::run creating model";
		DaWorldLL* clonedWorld = 0;
		const AuEther& etherium = g_Au.LockEtheriumDelete();
		const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
		if(world)
			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
		g_Au.UnlockEtheriumDelete();
		if(clonedWorld)
		{
			DaWorldSphere* glworld = dynamic_cast<DaWorldSphere*>(clonedWorld);
			if(glworld)
			{
				// Tessellate surface for flat model.
				AuGlobeLLTriList globeTris;
				glworld->CreateSurfaceTrianglesFlat(globeTris, glworld->GroundLevelEquatorDivisions(), m_MaxAltMSL, m_MinAltMSL);

				// Create the triangles that make up the model.
				QList<AuGlobeTriAltVeg> tris;			// Triangles that make up this model.
				for(AuGlobeLLTriList::const_iterator ti = globeTris.cbegin(); ti != globeTris.cend(); ti++)
					glworld->CreateFlatGlobeVAOTriangle(m_GlobeTris, (*ti).m_LL0, (*ti).m_LL1, (*ti).m_LL2);
			}
			else
			{
				DaWorldRingSection* rsworld = dynamic_cast<DaWorldRingSection*>(clonedWorld);
				if(rsworld)
				{
					// Tessellate surface for flat model.
					AuGlobeLLTriList globeTris;
					rsworld->CreateSurfaceTriangles(globeTris, rsworld->GroundLevelEquatorDivisions(), m_MaxAltMSL, m_MinAltMSL);

					// Create the triangles that make up the model.
					QList<AuGlobeTriAltVeg> tris;			// Triangles that make up this model.
					for(AuGlobeLLTriList::const_iterator ti = globeTris.cbegin(); ti != globeTris.cend(); ti++)
						rsworld->CreateFlatGlobeVAOTriangle(m_GlobeTris, (*ti).m_LL0, (*ti).m_LL1, (*ti).m_LL2);
				}
			}

			m_WorldModifyTime = clonedWorld->ModifyTime();

			/* This takes way too long.
			// The normal atom destructor would destroy its subatoms and remove their indexes from the root atom. We don't want
			// that to happen to our cloned cities. We must delete the cloned cities in a way that they do not climb up the parent
			// atom chain and mess with root atom indexes. That means simply deleting them ourself here, instead of letting the
			// clonedWorld destructor do it.
			DaAtomList* subatoms = clonedWorld->LockSubatomsForWrite();
			if(subatoms)
				while(!subatoms->isEmpty())
					delete subatoms->takeFirst();
			clonedWorld->UnlockSubatoms();
			*/
			delete clonedWorld;
		}

		// Store result in cache.
		if(CacheEnabled() && !m_GlobeTris.isEmpty())
		{
			// Insure the cache folder exists.
			if(g_Au.CreateCacheGame())
			{
				// The cache folder exists.
				if(cacheFile.open(QFile::WriteOnly))
				{
					// Prepare to write to the file.
					QDataStream dsfile(&cacheFile);
					dsfile.setVersion(QDataStream::Qt_4_1);

					// Write a header to the file.
					const quint8 fileType = FLATGLOBE_MODEL_CACHE;
					const quint8 fileVersion = FLATGLOBE_MODEL_VER;
					dsfile << fileType;
					dsfile << fileVersion;
					dsfile << m_WorldModifyTime.UTC();

					// Store model cache into a byte array so it can be compressed.
					QByteArray ba;
					QDataStream ds(&ba, QIODevice::WriteOnly);
					ds.setVersion(QDataStream::Qt_4_1);
					ds << m_GlobeTris;
					ds << m_MinAltMSL;
					ds << m_MaxAltMSL;

					// Compress the byte array.
					QByteArray bcomp = qCompress(ba);

					// Write the compressed byte array to the file.
					dsfile << bcomp;

					// Close the file.
					cacheFile.close();
				}
			}
		}
	}
	RunFinish();
	//qDebug() << "AuFlatGlobeModel::run" << m_WorldId << "done.";
}

void AuFlatGlobeModel::DeleteGL()
{
	if(m_VAO.VAO())
		m_VAO.DeleteGL();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuNearGlobeModel::AuNearGlobeModel(const AuId& worldId, const AuTime& worldModTime, bool cacheEnabled)
{
	m_CacheEnabled = cacheEnabled;
	m_WorldId = worldId;
	m_WorldModifyTime = worldModTime;
}

// Returns the VAO of this globe.
// Creates the model from the cache data and sets m_VAO to the result.
// Returns m_VAO, which may be 0 if it's not ready yet.
// NOTE: A gl context must be current when this function is called.
AuGLVAO AuNearGlobeModel::VAO(AuOpenGLFunctions& GLf)
{
	if(!IsDoneRunning())
		return AuGLVAO();

	if(!m_VAO.VAO() && !m_GlobeTris.isEmpty())
	{
		// Store count of triangles in the globe vertex array object.
		GLuint vertices = m_GlobeTris.count()*3;

		// Copy surface triangles into gl array buffers.
		GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
		GLfloat* globeN = new GLfloat[vertices*3];				// normals for vertices that make up the model.
		GLfloat* globeTex = new GLfloat[vertices*3];			// texture coords and shininess

		unsigned int globePosIdx = 0;
		unsigned int globeNIdx = 0;
		unsigned int globeTexIdx = 0;
		for(QList<AuGlobeTriTexNShineSnow>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
		{
			globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
			globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
			globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

			globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
			globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
			globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

			globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
			globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
			globeTex[globeTexIdx++] = (*vi).m_Vert0.m_Shininess;

			globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
			globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
			globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

			globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
			globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
			globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

			globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
			globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
			globeTex[globeTexIdx++] = (*vi).m_Vert1.m_Shininess;

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
		m_VAO.CreateGL(GLf, vertices, 3);

		// Select vertex array to construct.
		m_VAO.SelectGL();

		// bind buffer for vertices and copy data into buffer.
		GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(0));
		GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
		GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		// bind buffer for normals and copy data into buffer.
		GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(1));
		GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
		GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		// bind buffer for colors and copy data into buffer.
		GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(2));
		GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
		GLf.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Disable the vertex array object.
		m_VAO.DeselectGL();

		delete [] globePos;
		delete [] globeN;
		delete [] globeTex;

		// These triangles are no longer needed after the VAO has been created.
		m_GlobeTris.clear();
	}
	return m_VAO;
}

// This guy does the grunt work to create the model of the globe.
void AuNearGlobeModel::run()
{
	//qDebug() << "AuNearGlobeModel::run" << m_WorldId;
	if(!RunStart())
		return;
	Q_ASSERT(m_GlobeTris.isEmpty());

	// Try loading from cache.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("WGN%1").arg(m_WorldId.GetID()));
	QFile cacheFile(cacheFileName);
	if(CacheEnabled() && cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;		// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == NEARGLOBE_MODEL_CACHE && fileVersion == NEARGLOBE_MODEL_VER && storeTime >= m_WorldModifyTime.UTC())
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
				ds >> m_GlobeTris;
				if(c_CacheDebugs)
					qDebug() << "AuNearGlobeModel::run loaded" << m_GlobeTris.count();
			}
		}

		// Close the file.
		cacheFile.close();
	}

	if(m_GlobeTris.isEmpty())
	{
		// Create the models since they were not loaded from the cache.
		if(c_CacheDebugs)
			qDebug() << "AuNearGlobeModel::run creating";
		DaWorldLL* clonedWorld = 0;
		const AuEther& etherium = g_Au.LockEtheriumDelete();
		const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
		if(world)
			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
		g_Au.UnlockEtheriumDelete();
		if(clonedWorld)
		{
			clonedWorld->CreateNearGlobeSurfaceTriangles(m_GlobeTris);

			m_WorldModifyTime = clonedWorld->ModifyTime();

			delete clonedWorld;
		}

		// Store result in cache.
		if(CacheEnabled() && !m_GlobeTris.isEmpty())
		{
			// Insure the cache folder exists.
			if(g_Au.CreateCacheGame())
			{
				// The cache folder exists.
				if(cacheFile.open(QFile::WriteOnly))
				{
					// Prepare to write to the file.
					QDataStream dsfile(&cacheFile);
					dsfile.setVersion(QDataStream::Qt_4_1);

					// Write a header to the file.
					const quint8 fileType = NEARGLOBE_MODEL_CACHE;
					const quint8 fileVersion = NEARGLOBE_MODEL_VER;
					dsfile << fileType;
					dsfile << fileVersion;
					dsfile << m_WorldModifyTime.UTC();

					// Store model cache into a byte array so it can be compressed.
					QByteArray ba;
					QDataStream ds(&ba, QIODevice::WriteOnly);
					ds.setVersion(QDataStream::Qt_4_1);
					ds << m_GlobeTris;

					// Compress the byte array.
					QByteArray bcomp = qCompress(ba);

					// Write the compressed byte array to the file.
					dsfile << bcomp;

					// Close the file.
					cacheFile.close();
				}
			}
		}
	}
	RunFinish();
	//qDebug() << "AuNearGlobeModel::run" << m_WorldId << "done.";
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGlobeModel::AuGlobeModel(const AuId& worldId, const AuTime& worldModTime, float worldRadius, bool worldCanHaveFlora,
		bool cacheEnabled)
{
	m_CacheEnabled = cacheEnabled;
	m_WorldCanHaveFlora = worldCanHaveFlora;
	m_WorldId = worldId;
	m_WorldModifyTime = worldModTime;
	m_WorldRadius = worldRadius;
}

// Returns the VAO of this globe.
// Creates the model from the cache data and sets m_VAO to the result.
// Returns m_VAO, which may be 0 if it's not ready yet.
// NOTE: A gl context must be current when this function is called.
void AuGlobeModel::VAOs(AuOpenGLFunctions& GLf, AuAtGlobeVAO& vaos)
{
	if(!IsDoneRunning())
		return;

	if(!m_VAO.m_Low.VAO() && !m_GlobeTris.isEmpty())
	{
		const int vertices = m_GlobeTris.count()*3;				// Each triangle has 3 vertices.

		// Copy surface triangles into gl array buffers.
		if(m_WorldCanHaveFlora || true)
		{
			GLfloat* globePos = new GLfloat[vertices*4];		// vertices for triangles that make up the model.
			GLfloat* globeN = new GLfloat[vertices*3];			// normals for vertices that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*4];		// texture coords, altitude, veg density for each vertex
			float fMinHeight = m_WorldRadius + m_VAO.m_MinAltMSL;
			float altRange = m_VAO.m_MaxAltMSL - m_VAO.m_MinAltMSL;	// Altitude range of all land, for altitude factor calc.
			//qDebug() << "AuGlobeModel::VAOs" << m_VAO.m_MinAltMSL << m_VAO.m_MaxAltMSL << "rad" << m_WorldRadius;

			unsigned int globePosIdx = 0;
			unsigned int globeNIdx = 0;
			unsigned int globeTexIdx = 0;
			for(QList<AuGlobeTriAltVegN>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
			{
				// Vertex 0
				//if((*vi).m_Vert0.m_Altitude < fMinHeight)
					//qDebug() << "vert0" << (*vi).m_Vert0.m_Loc << (*vi).m_Vert0.m_N << (*vi).m_Vert0.m_Snow
						//<< (*vi).m_Vert0.m_TexCoord << (*vi).m_Vert0.m_Altitude << (*vi).m_Vert0.m_Vegetation << globePosIdx;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
				//Q_ASSERT((*vi).m_Vert0.m_Altitude >= fMinHeight);
				//Q_ASSERT((*vi).m_Vert0.m_Altitude <= m_WorldRadius + m_VAO.m_MaxAltMSL);
				globeTex[globeTexIdx++] = ((*vi).m_Vert0.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_Vegetation/100.f;

				// Vertex 1
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
				//Q_ASSERT((*vi).m_Vert1.m_Altitude >= fMinHeight);
				//Q_ASSERT((*vi).m_Vert1.m_Altitude <= m_WorldRadius + m_VAO.m_MaxAltMSL);
				globeTex[globeTexIdx++] = ((*vi).m_Vert1.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_Vegetation/100.f;

				// Vertex 2
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
				//Q_ASSERT((*vi).m_Vert2.m_Altitude >= fMinHeight);
				//Q_ASSERT((*vi).m_Vert2.m_Altitude <= m_WorldRadius + m_VAO.m_MaxAltMSL);
				globeTex[globeTexIdx++] = ((*vi).m_Vert2.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_Vegetation/100.f;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_VAO.m_Low.CreateGL(GLf, vertices, 3);

			// Select vertex array to construct.
			m_VAO.m_Low.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.m_Low.Buffer(0));
			GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for normals and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.m_Low.Buffer(1));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for texture coordinates and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.m_Low.Buffer(2));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_VAO.m_Low.DeselectGL();

			delete [] globePos;
			delete [] globeN;
			delete [] globeTex;
		}
		else
		{
			GLfloat* globePos = new GLfloat[vertices*4];		// vertices for triangles that make up the model.
			GLfloat* globeN = new GLfloat[vertices*3];			// normals for vertices that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*3];		// texture coords, altitude for each of the vertices
			float fMinHeight = m_WorldRadius + m_VAO.m_MinAltMSL;
			float altRange = m_VAO.m_MaxAltMSL - m_VAO.m_MinAltMSL;	// Altitude range of all land, for altitude factor calc.

			unsigned int globePosIdx = 0;
			unsigned int globeNIdx = 0;
			unsigned int globeTexIdx = 0;
			for(QList<AuGlobeTriAltVegN>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
			{
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert0.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.

				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert1.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.

				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert2.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
			}

			// Generate vertex array(s) for this object. We only need one.
			m_VAO.m_Low.CreateGL(GLf, vertices, 3);

			// Select vertex array to construct.
			m_VAO.m_Low.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.m_Low.Buffer(0));
			GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for normals and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.m_Low.Buffer(1));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for texture coordinates and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.m_Low.Buffer(2));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_VAO.m_Low.DeselectGL();

			delete [] globePos;
			delete [] globeN;
			delete [] globeTex;
		}

		// These triangles are no longer needed after the VAO has been created.
		m_GlobeTris.clear();
	}

	// Return the VAOs and the large terrain triangles for the globe. Each large terrain triangle is the basis for the high
	// tessellation patches of the globe, which are only created as needed.
	vaos.m_Low = m_VAO.m_Low;
	vaos.m_MaxAltMSL = m_VAO.m_MaxAltMSL;
	vaos.m_MinAltMSL = m_VAO.m_MinAltMSL;
	vaos.m_TerrainTris = m_VAO.m_TerrainTris;
}

// This guy does the grunt work to create the model of the globe.
void AuGlobeModel::run()
{
	//qDebug() << "AuGlobeModel::run" << m_WorldId;
	if(!RunStart())
		return;
	Q_ASSERT(m_GlobeTris.isEmpty());

	// Try loading from cache.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("WGA%1").arg(m_WorldId.GetID()));
	QFile cacheFile(cacheFileName);
	if(CacheEnabled() && cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;		// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == ATGLOBE_MODEL_CACHE && fileVersion == ATGLOBE_MODEL_VER && storeTime >= m_WorldModifyTime.UTC())
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
				ds >> m_GlobeTris;
				ds >> m_WorldCanHaveFlora;
				ds >> m_WorldRadius;
				ds >> m_VAO.m_TerrainTris;
				ds >> m_VAO.m_MinAltMSL;
				ds >> m_VAO.m_MaxAltMSL;
				if(c_CacheDebugs)
					qDebug() << "AuGlobeModel::run loaded" << m_VAO.m_MinAltMSL << m_VAO.m_MaxAltMSL
						<< "globeTris" << m_GlobeTris.count() << "rad" << m_WorldRadius;
				/*
				int videx = 0;
				for(QList<AuGlobeTriAltVegN>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++, videx++)
				{
					if((*vi).m_Vert0.m_Altitude < m_VAO.m_MinAltMSL+m_WorldRadius)
						qDebug() << "vert0" << videx << "alt" << (*vi).m_Vert0.m_Altitude << "TOO LOW";
					else if((*vi).m_Vert0.m_Altitude > m_VAO.m_MaxAltMSL+m_WorldRadius)
						qDebug() << "vert0" << videx << "alt" << (*vi).m_Vert0.m_Altitude << "TOO HIGH"
							<< ((*vi).m_Vert0.m_Altitude - (m_VAO.m_MaxAltMSL + m_WorldRadius));

					if((*vi).m_Vert1.m_Altitude < m_VAO.m_MinAltMSL+m_WorldRadius)
						qDebug() << "vert1" << videx << "alt" << (*vi).m_Vert1.m_Altitude << "TOO LOW";
					else if((*vi).m_Vert1.m_Altitude > m_VAO.m_MaxAltMSL+m_WorldRadius)
						qDebug() << "vert1" << videx << "alt" << (*vi).m_Vert1.m_Altitude << "TOO HIGH"
							<< ((*vi).m_Vert1.m_Altitude - (m_VAO.m_MaxAltMSL + m_WorldRadius));

					if((*vi).m_Vert2.m_Altitude < m_VAO.m_MinAltMSL+m_WorldRadius)
						qDebug() << "vert2" << videx << "alt" << (*vi).m_Vert2.m_Altitude << "TOO LOW";
					else if((*vi).m_Vert2.m_Altitude > m_VAO.m_MaxAltMSL+m_WorldRadius)
						qDebug() << "vert2" << videx << "alt" << (*vi).m_Vert2.m_Altitude << "TOO HIGH"
							<< ((*vi).m_Vert2.m_Altitude - (m_VAO.m_MaxAltMSL + m_WorldRadius));
				}
				*/
			}
		}

		// Close the file.
		cacheFile.close();
	}

	if(m_GlobeTris.isEmpty())
	{
		// Create the models since they were not loaded from the cache.
		if(c_CacheDebugs)
			qDebug() << "AuGlobeModel::run creating";
		DaWorldLL* clonedWorld = 0;
		const AuEther& etherium = g_Au.LockEtheriumDelete();
		const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
		if(world)
			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
		g_Au.UnlockEtheriumDelete();
		if(clonedWorld)
		{
			// Create terrain triangles array that will be used when making high tessellation patches on the globe.
			clonedWorld->CreateAtSurfaceTriangles(m_VAO.m_TerrainTris);

			// Create the low tessellation of the at-globe model.
			clonedWorld->CreateAtGlobeSurfaceTriangles(m_GlobeTris, m_VAO.m_MaxAltMSL, m_VAO.m_MinAltMSL);

			//qDebug() << "AuGlobeModel::run" << m_VAO.m_MaxAltMSL << m_VAO.m_MinAltMSL;
			m_WorldModifyTime = clonedWorld->ModifyTime();

			delete clonedWorld;
		}

		// Store result in cache.
		if(CacheEnabled() && !m_GlobeTris.isEmpty())
		{
			// Insure the cache folder exists.
			if(g_Au.CreateCacheGame())
			{
				// The cache folder exists.
				if(cacheFile.open(QFile::WriteOnly))
				{
					// Prepare to write to the file.
					QDataStream dsfile(&cacheFile);
					dsfile.setVersion(QDataStream::Qt_4_1);

					// Write a header to the file.
					const quint8 fileType = ATGLOBE_MODEL_CACHE;
					const quint8 fileVersion = ATGLOBE_MODEL_VER;
					dsfile << fileType;
					dsfile << fileVersion;
					dsfile << m_WorldModifyTime.UTC();

					// Store model cache into a byte array so it can be compressed.
					QByteArray ba;
					QDataStream ds(&ba, QIODevice::WriteOnly);
					ds.setVersion(QDataStream::Qt_4_1);
					ds << m_GlobeTris;
					ds << m_WorldCanHaveFlora;
					ds << m_WorldRadius;
					ds << m_VAO.m_TerrainTris;
					ds << m_VAO.m_MinAltMSL;
					ds << m_VAO.m_MaxAltMSL;

					// Compress the byte array.
					QByteArray bcomp = qCompress(ba);

					// Write the compressed byte array to the file.
					dsfile << bcomp;

					// Close the file.
					cacheFile.close();
				}
			}
		}
	}
	RunFinish();
	//qDebug() << "AuGlobeModel::run" << m_WorldId << "done.";
}

void AuGlobeModel::DeleteGL()
{
	// Delete the low tessellation model of the globe.
	if(m_VAO.m_Low.VAO())
		m_VAO.m_Low.DeleteGL();

	// Delete the high tessellation model patches of the globe.
	if(!m_VAO.m_TriVAOs.empty())
	{
		qDebug() << "AuGlobeModel::DeleteGL triVAOs" << m_VAO.m_TriVAOs.size() << "EXPECTING NONE";
		for(unordered_map<int, AuGLTriVAO, AuTriHash>::iterator tvi = m_VAO.m_TriVAOs.begin(); tvi != m_VAO.m_TriVAOs.end(); tvi++)
			if((*tvi).second.VAO())
				(*tvi).second.DeleteGL();
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGlobeModelTri::AuGlobeModelTri(const AuId& worldId, int triIndex, const AuGlobeLLTri& tri, float worldRadius,
		bool worldCanHaveFlora, float maxAltMSL, float minAltMSL)
{
	m_MaxAltMSL = maxAltMSL;
	m_MinAltMSL = minAltMSL;
	m_TerrainTri = tri;
	m_TriIndex = triIndex;
	m_WorldCanHaveFlora = worldCanHaveFlora;
	m_WorldId = worldId;
	m_WorldRadius = worldRadius;
}

// Returns the VAO of this globe.
// Creates the model from the cache data and sets m_VAO to the result.
// Returns m_VAO, which may be 0 if it's not ready yet.
// NOTE: A gl context must be current when this function is called.
AuGLTriVAO AuGlobeModelTri::VAO(AuOpenGLFunctions& GLf)
{
	if(!IsDoneRunning())
		return AuGLTriVAO();

	if(!m_VAO.VAO() && !m_GlobeTris.isEmpty())
	{
		const int vertices = m_GlobeTris.count()*3;				// Each triangle has 3 vertices.

		// Copy surface triangles into gl array buffers.
		if(m_WorldCanHaveFlora || true)
		{
			GLfloat* globePos = new GLfloat[vertices*4];		// vertices for triangles that make up the model.
			GLfloat* globeN = new GLfloat[vertices*4];			// normals for vertices that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*4];		// texture coords, altitude, veg density for each vertex
			GLfloat* paveCity = new GLfloat[vertices*2];		// pavement coverage(x), city presence(y)
			float fMinHeight = m_WorldRadius + m_MinAltMSL;
			float altRange = m_MaxAltMSL - m_MinAltMSL;			// Altitude range of all land, for altitude factor calc.
			//qDebug() << "worldRad" << m_WorldRadius << "minAlt" << m_MinAltMSL << "maxAlt" << m_MaxAltMSL;

			unsigned int globePosIdx = 0;
			unsigned int globeNIdx = 0;
			unsigned int globeTexIdx = 0;
			unsigned int paveCityIdx = 0;
			for(QList<AuGlobeTriAltVegNPave>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
			{
				// Vertex 0
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;
				globeN[globeNIdx++] = (*vi).m_Vert0.m_PavementType;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert0.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_Vegetation/100.f;

				paveCity[paveCityIdx++] = (*vi).m_Vert0.m_PavementCover;
				paveCity[paveCityIdx++] = (*vi).m_Vert0.m_CityPresence;

				// Vertex 1
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;
				globeN[globeNIdx++] = (*vi).m_Vert1.m_PavementType;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert1.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_Vegetation/100.f;

				paveCity[paveCityIdx++] = (*vi).m_Vert1.m_PavementCover;
				paveCity[paveCityIdx++] = (*vi).m_Vert1.m_CityPresence;

				// Vertex 2
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;
				globeN[globeNIdx++] = (*vi).m_Vert2.m_PavementType;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert2.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_Vegetation/100.f;

				paveCity[paveCityIdx++] = (*vi).m_Vert2.m_PavementCover;
				paveCity[paveCityIdx++] = (*vi).m_Vert2.m_CityPresence;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_VAO.CreateGL(GLf, vertices, 4);

			// Select vertex array to construct.
			m_VAO.SelectGLNoTime();

			// bind buffer for vertices and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(0));
			GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for normals and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(1));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for texture coordinates and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(2));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for texture coordinates and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(3));
			GLf.glBufferData(GL_ARRAY_BUFFER, paveCityIdx*sizeof(GLfloat), paveCity, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_VAO.DeselectGL();

			delete [] globePos;
			delete [] globeN;
			delete [] globeTex;
			delete [] paveCity;
		}
		else
		{
			GLfloat* globePos = new GLfloat[vertices*4];		// vertices for triangles that make up the model.
			GLfloat* globeN = new GLfloat[vertices*4];			// normals for vertices that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*4];		// texture coords, altitude for each of the vertices
			GLfloat* city = new GLfloat[vertices];				// city presence(y)
			float fMinHeight = m_WorldRadius + m_MinAltMSL;
			float altRange = m_MaxAltMSL - m_MinAltMSL;			// Altitude range of all land, for altitude factor calc.

			unsigned int globePosIdx = 0;
			unsigned int globeNIdx = 0;
			unsigned int globeTexIdx = 0;
			unsigned int cityIdx = 0;
			for(QList<AuGlobeTriAltVegNPave>::const_iterator vi = m_GlobeTris.cbegin(); vi != m_GlobeTris.cend(); vi++)
			{
				// Vertex 0
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;
				globeN[globeNIdx++] = (*vi).m_Vert0.m_PavementType;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert0.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_PavementCover;

				city[cityIdx++] = (*vi).m_Vert0.m_CityPresence;

				// Vertex 1
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;
				globeN[globeNIdx++] = (*vi).m_Vert1.m_PavementType;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert1.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_PavementCover;

				city[cityIdx++] = (*vi).m_Vert1.m_CityPresence;

				// Vertex 2
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Snow;

				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;
				globeN[globeNIdx++] = (*vi).m_Vert2.m_PavementType;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
				globeTex[globeTexIdx++] = ((*vi).m_Vert2.m_Altitude - fMinHeight) / altRange;		// Altitude in range of 0..1.
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_PavementCover;

				city[cityIdx++] = (*vi).m_Vert2.m_CityPresence;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_VAO.CreateGL(GLf, vertices, 4);

			// Select vertex array to construct.
			m_VAO.SelectGLNoTime();

			// bind buffer for vertices and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(0));
			GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for normals and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(1));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for texture coordinates and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(2));
			GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for city presence and copy data into buffer.
			GLf.glBindBuffer(GL_ARRAY_BUFFER, m_VAO.Buffer(3));
			GLf.glBufferData(GL_ARRAY_BUFFER, cityIdx*sizeof(GLfloat), city, GL_STATIC_DRAW);
			GLf.glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_VAO.DeselectGL();

			delete [] globePos;
			delete [] globeN;
			delete [] globeTex;
			delete [] city;
		}

		// These triangles are no longer needed after the VAO has been created.
		m_GlobeTris.clear();
	}

	return m_VAO;
}

// This guy does the grunt work to create the model of the globe.
void AuGlobeModelTri::run()
{
	//qDebug() << "AuGlobeModelTri::run" << m_WorldId << m_TriIndex
		//<< "maxThreads" << QThreadPool::globalInstance()->maxThreadCount();
	if(!RunStart())
	{
		//qDebug() << "AuGlobeModelTri::run" << m_WorldId << m_TriIndex << m_Killed << "never got started";
		return;
	}
	Q_ASSERT(m_GlobeTris.isEmpty());

	/*
	// Try loading from cache.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("G%1").arg(m_WorldId.GetID()));
	QFile cacheFile(cacheFileName);
	if(CacheEnabled() && cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;		// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == GLOBE_MODEL_CACHE && fileVersion == GLOBE_MODEL_VER && storeTime >= m_WorldModifyTime)
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
				ds >> m_GlobeTris;
				ds >> m_MaxAltMSL;
				ds >> m_MinAltMSL;
			}
		}

		// Close the file.
		cacheFile.close();
	}

	if(m_GlobeTris.isEmpty())
	*/
	{
		// Create the models since they were not loaded from the cache.
		AuEther tempEther("AuGlobeModelTri::run");
		DaWorldLL* clonedWorld = 0;
		const AuEther& etherium = g_Au.LockEtheriumDelete();
		const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
		if(world)
		{
			// Buildings in a design environment preview need the design environment parent.
			const DaDesigner* pdes = world->FindParentDesignerInEnv();
			if(pdes)
			{
				DaAtom* clone = pdes->Clone();
				tempEther.AddAtom(clone);

				clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
				tempEther.AddAtom(clonedWorld);
				clonedWorld->ChangePos(AuAtomPos(pdes->Id(), AuPoint(), AuPoint()));
			}
			else
			{
				clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
				tempEther.AddAtom(clonedWorld);
			}

			// Must also clone the bds so terrain can conform to site meshes.
			bool bds = false;
			DaAtomList subs;
			world->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
			{
				if(!dynamic_cast<const DaBd*>(*ai))
					continue;

				// Add the clone.
				DaAtom* clone = (*ai)->Clone();
				tempEther.AddAtom(clone);
				bds = true;
			}
			if(bds)
				clonedWorld->BdAdded();
		}
		g_Au.UnlockEtheriumDelete();
		if(clonedWorld)
			clonedWorld->CreateAtTerrainTriangles(m_GlobeTris, m_TerrainTri, m_Killed);

		// Store result in cache.
		/*
		if(CacheEnabled() && !m_GlobeTris.isEmpty())
		{
			// Insure the cache folder exists.
			if(g_Au.CreateCacheGame())
			{
				// The cache folder exists.
				if(cacheFile.open(QFile::WriteOnly))
				{
					// Prepare to write to the file.
					QDataStream dsfile(&cacheFile);
					dsfile.setVersion(QDataStream::Qt_4_1);

					// Write a header to the file.
					const quint8 fileType = GLOBE_MODEL_CACHE;
					const quint8 fileVersion = GLOBE_MODEL_VER;
					dsfile << fileType;
					dsfile << fileVersion;
					dsfile << m_WorldModifyTime;

					// Store model cache into a byte array so it can be compressed.
					QByteArray ba;
					QDataStream ds(&ba, QIODevice::WriteOnly);
					ds.setVersion(QDataStream::Qt_4_1);
					ds << m_GlobeTris;
					ds << m_MaxAltMSL;
					ds << m_MinAltMSL;

					// Compress the byte array.
					QByteArray bcomp = qCompress(ba);

					// Write the compressed byte array to the file.
					dsfile << bcomp;

					// Close the file.
					cacheFile.close();
				}
			}
		}
		*/
	}
	RunFinish();
	//qDebug() << "AuGlobeModelTri::run" << m_WorldId << m_TriIndex << m_Killed;
		//<< "maxThreads" << QThreadPool::globalInstance()->maxThreadCount()
		//<< "done";
}

void AuGlobeModelTri::DeleteGL()
{
	if(m_VAO.VAO())
		m_VAO.DeleteGL();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGrassImages::AuGrassImages(const DePlanetFlora& flora, quint32 genSeed, int resourceZone)
{
	m_Flora = flora;
	m_GenSeed = genSeed;
	m_ResourceZone = resourceZone;
}

bool AuGrassImages::GrassImages(QImage& grass0, QImage& grass1, QImage& grass2)
{
	if(!IsDoneRunning())
		return false;
	grass0 = m_Grass0;
	grass1 = m_Grass1;
	grass2 = m_Grass2;
	return true;
}

// This function does the grunt work.
void AuGrassImages::run()
{
	//qDebug() << "AuGrassImages::run" << m_WorldId;
	if(!RunStart())
		return;
	//DHH flora change time needed m_Flora.PaintGroundCover(m_Grass0, m_Grass1, m_Grass2, m_GenSeed, m_ResourceZone);
	RunFinish();
	//qDebug() << "AuGrassImages::run" << m_WorldId << "done.";
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuFloraImage::AuFloraImage(const DePlanetFlora& flora, quint32 genSeed, int resourceZone)
{
	m_Flora = flora;
	m_GenSeed = genSeed;
	m_ResourceZone = resourceZone;
}

void AuFloraImage::FloraImage(DePlanetFlora& flora, QImage& imgFlora)
{
	if(!IsDoneRunning())
		return;
	flora = m_Flora;
	imgFlora = m_FloraImage;
}

// This function does the grunt work.
void AuFloraImage::run()
{
	//qDebug() << "AuFloraImage::run" << m_WorldId;
	if(!RunStart())
		return;
	//DHH flora change time needed m_Flora.PaintFlora(m_FloraImage, m_GenSeed, m_ResourceZone);
	RunFinish();
	//qDebug() << "AuFloraImage::run" << m_WorldId << "done.";
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuFloraModel::AuFloraModel(const AuId& worldId, const AuPoint& groundLoc)
{
	m_GroundLoc = groundLoc;
	m_WorldId = worldId;
}

// Returns the VAO of this globe.
// Creates the model from the cache data and sets m_VAO to the result.
// Returns m_VAO, which may be 0 if it's not ready yet.
// NOTE: A gl context must be current when this function is called.
bool AuFloraModel::Flora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
		QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs)
{
	if(!IsDoneRunning())
		return false;

	grass = m_Grass;
	aquaticgrass = m_AquaticGrass;

	QMap<AuResKey, AuGlobeFlora> shrubFade(shrubs);
	shrubs = m_Shrubs;
	for(QMap<AuResKey, AuGlobeFlora>::iterator ti = shrubs.begin(); ti != shrubs.end(); ti++)
	{
		QMap<AuResKey, AuGlobeFlora>::const_iterator fi = shrubFade.constFind(ti.key());
		if(fi != shrubFade.cend())
			(*ti).m_FadeAlpha = (*fi).m_FadeAlpha;
	}

	QMap<AuResKey, AuGlobeFlora> aquaticshrubFade(aquaticshrubs);
	aquaticshrubs = m_AquaticShrubs;
	for(QMap<AuResKey, AuGlobeFlora>::iterator ti = aquaticshrubs.begin(); ti != aquaticshrubs.end(); ti++)
	{
		QMap<AuResKey, AuGlobeFlora>::const_iterator fi = aquaticshrubFade.constFind(ti.key());
		if(fi != aquaticshrubFade.cend())
			(*ti).m_FadeAlpha = (*fi).m_FadeAlpha;
	}

	QMap<AuResKey, AuGlobeTree> treeFade(trees);
	trees = m_Trees;
	for(QMap<AuResKey, AuGlobeTree>::iterator ti = trees.begin(); ti != trees.end(); ti++)
	{
		QMap<AuResKey, AuGlobeTree>::const_iterator fi = treeFade.constFind(ti.key());
		if(fi != treeFade.cend())
			(*ti).m_FadeAlpha = (*fi).m_FadeAlpha;
	}

	return true;
}

// This guy does the grunt work to create the model of the globe.
void AuFloraModel::run()
{
	//qDebug() << "AuFloraModel::run" << m_WorldId;
	if(!RunStart())
		return;
	Q_ASSERT(m_AquaticGrass.isEmpty());
	Q_ASSERT(m_AquaticShrubs.isEmpty());
	Q_ASSERT(m_Grass.isEmpty());
	Q_ASSERT(m_Shrubs.isEmpty());
	Q_ASSERT(m_Trees.isEmpty());

	// Create the models since they were not loaded from the cache.
	AuEther tempEther("AuFloraModel::run");
	DaWorldLL* clonedWorld = 0;
	quint32 genSeed = 0;
	int floraGlow = 0;
	const AuEther& etherium = g_Au.LockEtheriumDelete();
	const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
	if(world)
	{
		// Must be sure natural resources data is up-to-date on the world before we clone it.
		for(int rz=0; rz<world->ResourceZones(); rz++)
		{
			AuZoneResource res;
			world->QueryNaturalResources(res, rz);
		}

		// Buildings in a design environment preview need the design environment parent.
		const DaDesigner* pdes = world->FindParentDesignerInEnv();
		if(pdes)
		{
			DaAtom* clone = pdes->Clone();
			tempEther.AddAtom(clone);

			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
			tempEther.AddAtom(clonedWorld);
			clonedWorld->ChangePos(AuAtomPos(pdes->Id(), AuPoint(), AuPoint()));
		}
		else
		{
			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
			tempEther.AddAtom(clonedWorld);
		}

		genSeed = world->GenSeed();
		floraGlow = world->FloraGlow();

		// Must also clone the bds so terrain can conform to site meshes.
		bool bds = false;
		DaAtomList subs;
		world->ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
		{
			if(!dynamic_cast<const DaBd*>(*ai))
				continue;

			// Add the clones.
			DaAtom* clone = (*ai)->Clone();
			tempEther.AddAtom(clone);
			bds = true;
		}
		if(bds)
			clonedWorld->BdAdded();
	}
	g_Au.UnlockEtheriumDelete();
	if(clonedWorld && gp_AuGlobal)
		clonedWorld->CreateFlora(m_Grass, m_Shrubs, m_Trees, m_AquaticGrass, m_AquaticShrubs, m_GroundLoc, genSeed, floraGlow);

	RunFinish();
	//qDebug() << "AuFloraModel::run" << m_WorldId << "done.";
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuWeatherData::AuWeatherData(const AuId& worldId, const AuWeather& wx)
{
	m_WorldId = worldId;
	m_Wx = wx;
}

bool AuWeatherData::Weather(AuWeather& wx)
{
	if(!IsDoneRunning())
		return false;

	wx = m_Wx;
	return true;
}

// This guy does the grunt work to create the model of the globe.
void AuWeatherData::run()
{
	//qDebug() << "AuWeatherData::run" << m_WorldId;
	if(!RunStart())
		return;

	// Create the models since they were not loaded from the cache.
	DaWorldLL* clonedWorld = 0;
	quint32 genSeed = 0;
	float solarOrbitRadius = 1;
	float sunLightAtten = 1;
	const AuEther& etherium = g_Au.LockEtheriumDelete();
	const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
	if(world)
	{
		clonedWorld = (DaWorldLL*)world->Clone();			// Clone the world so it can make grass without locking etherium.
		genSeed = world->GenSeed();

		const DaStar* star = world->FindParentStarInEnv();
		if(star)
		{
			if(world->IsSatellite())
				solarOrbitRadius = ((const DaWorld*)world->Parent())->OrbitRadius();
			else
				solarOrbitRadius = world->OrbitRadius();
			sunLightAtten = star->LightAttenuation(0);
		}
	}
	g_Au.UnlockEtheriumDelete();
	if(clonedWorld)
	{
		AuDice dice;
		dice.Seed(genSeed);
		DaWorldSphere* glclone = dynamic_cast<DaWorldSphere*>(clonedWorld);
		if(glclone)
			m_Wx.InitSphere(*glclone, glclone->Radius100km(), dice, solarOrbitRadius, sunLightAtten);
		else
		{
			DaWorldRingSection* rsclone = dynamic_cast<DaWorldRingSection*>(clonedWorld);
			if(rsclone)
				m_Wx.InitRingSection(*rsclone, dice, solarOrbitRadius, sunLightAtten);
		}
		delete clonedWorld;
	}
	RunFinish();
	//qDebug() << "AuWeatherData::run" << m_WorldId << "done.";
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuRockModel::AuRockModel(const AuId& worldId, const AuPoint& groundLoc)
{
	m_GroundLoc = groundLoc;
	m_WorldId = worldId;
}

// Returns the VAO of this globe.
// Creates the model from the cache data and sets m_VAO to the result.
// Returns m_VAO, which may be 0 if it's not ready yet.
// NOTE: A gl context must be current when this function is called.
bool AuRockModel::Rock(QMap<AuResKey, AuGlobeRock>& rocks)
{
	if(!IsDoneRunning())
		return false;

	//rocks = m_Rock;

	QMap<AuResKey, AuGlobeRock> rockFade(rocks);
	rocks = m_Rock;
	for(QMap<AuResKey, AuGlobeRock>::iterator ri = rocks.begin(); ri != rocks.end(); ri++)
	{
		QMap<AuResKey, AuGlobeRock>::const_iterator fi = rockFade.constFind(ri.key());
		if(fi != rockFade.cend())
			(*ri).m_FadeAlpha = (*fi).m_FadeAlpha;
	}

	return true;
}

// This guy does the grunt work to create the model of the globe.
void AuRockModel::run()
{
	//qDebug() << "AuRockModel::run" << m_WorldId;
	if(!RunStart())
		return;
	Q_ASSERT(m_Rock.isEmpty());

	// Create the models since they were not loaded from the cache.
	AuEther tempEther("AuRockModel::run");
	DaWorldLL* clonedWorld = 0;
	quint32 genSeed = 0;
	const AuEther& etherium = g_Au.LockEtheriumDelete();
	const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(etherium.FindNormalAtom(m_WorldId));
	if(world)
	{
		// Must be sure natural resources data is up-to-date on the world before we clone it.
		for(int rz=0; rz<world->ResourceZones(); rz++)
		{
			AuZoneResource res;
			world->QueryNaturalResources(res, rz);
		}

		// Buildings in a design environment preview need the design environment parent.
		const DaDesigner* pdes = world->FindParentDesignerInEnv();
		if(pdes)
		{
			DaAtom* clone = pdes->Clone();
			tempEther.AddAtom(clone);

			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
			tempEther.AddAtom(clonedWorld);
			clonedWorld->ChangePos(AuAtomPos(pdes->Id(), AuPoint(), AuPoint()));
		}
		else
		{
			clonedWorld = (DaWorldLL*)world->Clone();	// Clone the world so it can make triangles without locking etherium.
			tempEther.AddAtom(clonedWorld);
		}

		genSeed = world->GenSeed();

		// Must also clone the bds so terrain can conform to site meshes.
		bool bds = false;
		DaAtomList subs;
		world->ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
		{
			if(!dynamic_cast<const DaBd*>(*ai))
				continue;

			// Add the clones.
			DaAtom* clone = (*ai)->Clone();
			tempEther.AddAtom(clone);
			bds = true;
		}
		if(bds)
			clonedWorld->BdAdded();
	}
	g_Au.UnlockEtheriumDelete();
	if(clonedWorld)
		clonedWorld->CreateRocks(m_Rock, m_GroundLoc, genSeed, g_Au.RockViewRangeFt() + 100*FeetPerMeter(), true);

	RunFinish();
	//qDebug() << "AuRockModel::run" << m_WorldId << "done.";
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

