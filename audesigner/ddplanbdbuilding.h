//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanbdbuilding.h,v 1.15 2022/04/15 16:42:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPlanBdBuilding_H
#define DdPlanBdBuilding_H

#include "aubdreport.h"
#include "auelapsedtimer.h"
#include "auglobell.h"
#include "ddcommand.h"
#include "dbmanufacturingprocess.h"
#include <QMouseEvent>
#include <QKeyEvent>

class AuGLEffects;
class AuKnownPatents;
class DaBd;
class DaWorldLL;

class DdPlanBdBuilding : public DdCommand  
{
private:
	float								m_AlignRoadRot;			// Radians. 99 = invalid. Used for rotating to existing development.
	float								m_AlignRot;				// Radians. 99 = invalid. Used for rotating to existing development.
	int									m_AlignWhat;			// 0=building, 1=road
	AuGLVAO								m_BdVAO;
	AuGLVAO								m_BdCoverageVAO;
	bool								m_BdVAOValid;
	INDUSTRYID							m_eIndustryID;
	float								m_NorthZ;				// Angle to orient building facing to the north.
	AuBdReport							m_ORpt;					// Original design construction report.
	double								m_PlatformAltMSL;		// MSL altitude for aerial platforms.
	QHash<COMMODITYID, quint32>			m_RocksOnSite;			// Rocks on bd site.
	bool								m_RotAligned;			// True if the rotation was calculated by aligning to existing dev.
	float								m_RotZ;					// Building model rotation on its Z axis.
	AuPoint								m_SiteCenter;
	AuPoint								m_SiteRadius;
	AuBdReport							m_SRpt;					// Changed to selected construction material.
	bool								m_WorkerOk;				// false to disable worker in new manufacturing process

	DbManufacturingProcessArray 		m_aProcesses;
	int									m_iProcessIndex;		// index of current process in m_aProcesses
	QString								m_sProcessName;
	QString								m_sReason;
	AuElapsedTimer						m_LastTime;
	AuGlobeLLF							m_LastBuildLL;
	AuElapsedTimer						m_TimeCursorMoved;

public:
	// 'constructionMaterial' is a COMMODITYID.
	DdPlanBdBuilding(INDUSTRYID industryID, const AuBdReport& report, int constructionMaterial, quint16 mpid, bool enableWorker,
			const AuKnownPatents& patents, AuDesignerView* view);
	virtual ~DdPlanBdBuilding();

	virtual void SetConstructionProcess(const AuBdReport& report);

	inline void SetManufacturingProcess(int mpid)
	{
		// Set process index to point at the mpid that was selected.
		for(m_iProcessIndex=0; m_iProcessIndex<m_aProcesses.count(); m_iProcessIndex++)
			if(m_aProcesses[m_iProcessIndex].m_id == mpid)
				break;
		if(m_iProcessIndex >= m_aProcesses.count())
			m_iProcessIndex = 0;		// Didn't find the mpid so go with index 0.
		//qDebug("SetManufacturingProcess mpid %d index %d of %d processes", mpid, m_iProcessIndex, m_aProcesses.count());
	}

	inline void SetManufacturingWorkerOk(bool enableWorker)
	{
		m_WorkerOk = enableWorker;
	}

	virtual void SetMaterial(int constructionMaterial);

	// Keyboard monitoring functions.
	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);
	virtual void OnMouseMove(QMouseEvent* e);
	virtual void OnMouseWheel(QWheelEvent* e);

	// Cursor rendering functions.
	virtual void CreateDeviceObjects(AuGLEffects& re);
	virtual void DeleteDeviceObjects(AuGLEffects& re);
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& camera, DaPerson* sceneAtom,
			const QPoint& pxCursor, AuAtomRenderInfo& ari, bool snappedToExistingShape);

	// Set appropriate device cursor. Default sets Cross cursor if
	// pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

protected:
	void AlignToExistingDevelopment();
	void AlignToExistingRoad();

	void ConstructBuilding(const QPoint &pxCursor, quint16 mpid);

	int TestBuildLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
			DaBd*& exBd, DaBd** platform, QString& cityName) const;

	inline float ZRotation() const
	{
		if(m_RotAligned)
			return m_RotZ;				// Rotation calculated by alignment with existing development.
		else
			return m_RotZ - m_NorthZ;	// Rotation calculated based on north alignment of building.
	}
};

#endif // DdPlanBdBuilding_H
