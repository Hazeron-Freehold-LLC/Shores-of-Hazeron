//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationsurgery.h,v 1.16 2022/04/14 16:42:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationSurgery_H
#define DpStationSurgery_H

#include "auatom.h"
#include "aucondition.h"
#include "dpstation.h"

class AuItem;

class AUATOM_DLL DpStationSurgery : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_Antidote,
		CI_Antitoxin,
		CI_Defibrillate,
		CI_Heal,
		CI_PhysicalTherapy,
		CI_Surgery,
		CI_PrevCondition,
		CI_NextCondition,
		CI_Patient,				// Patient image area
		CI_Decontaminate,
	};

private:
	// Surgery Values monitored on the server that are not available in the ship's data on the client.
	qint16				m_CurHits;
	qint8				m_Gender;				// 0=female, 1=male
	qint16				m_MaxHits;
	QString				m_Name;					// Patient's name.
	quint16				m_WhichSurgery;			// 'which' surgery unit identifier from ship.

private:
	mutable QReadWriteLock				m_ConditionLock;
	quint16								m_ConditionIndex;		// Index of currently selected condition. 0xffff is none selected.
	QList<QPoint>						m_ConditionLocs;		// Pixel locs of conditions on body image. 1:1 to m_Conditions.
	AuConditionList						m_Conditions;			// Conditions on patient.

private:
	// Used for client-side animation. Not serialized.
	qint16				m_LastCurHits;
	qint16				m_LastGender;
	quint16				m_LastMaxHits;

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationSurgery(quint16 whichSurgery);
	virtual ~DpStationSurgery();
	DpStationSurgery(const DpStationSurgery& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	static void DoAntidote(DaCreature* doctor, DaCreature* patient, DaCapsule& capsule, const AuCondition* condition,
			const AuItem& antidote);

	static void DoAntitoxin(DaCreature* doctor, DaCreature* patient, DaCapsule& capsule, const AuCondition* condition,
			const AuItem& antitoxin);

	static void DoDecontaminate(DaCreature* doctor, DaCreature* patient, DaCapsule& capsule, const AuCondition* condition,
			const AuItem& water);

	static void DoDefibrillate(DaCreature* doctor, DaCreature* patient, DaCapsule& capsule);

	static void DoExtinguish(DaCreature* doctor, DaCreature* patient, DaCapsule& capsule, const AuCondition* condition,
			const AuItem& water);

	static void DoHeal(DaCreature* doctor, DaCreature* patient, DaCapsule& capsule, const AuItem& medicalKit);

	static void DoPhysicalTherapy(DaCreature* doctor, DaCreature* patient, const AuCondition* condition);

	static void DoSurgery(DaCreature* doctor, DaCreature* patient, const AuCondition* condition);

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual quint16 SystemWhich() const
	{
		return m_WhichSurgery;
	}

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaBd& bd,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);    // Refresh data from counterpart.

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);
	virtual void Tx(QDataStream& out) const;

protected:
	void AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& capsule);

	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);

	void DoAntidote(DaPerson* doctor, DaCapsule& ship);

	void DoAntitoxin(DaPerson* doctor, DaCapsule& ship);

	void DoDecontaminateOrDouse(DaPerson* doctor, DaCapsule& ship);

	void DoDefibrillate(DaPerson* doctor, DaCapsule& ship);

	void DoExtinguish(DaPerson* doctor, DaCapsule& ship);

	void DoHeal(DaPerson* doctor, DaCapsule& ship);

	void DoPhysicalTherapy(DaPerson* doctor, DaCapsule& ship);

	void DoSurgery(DaPerson* doctor, DaCapsule& ship);

	QPoint GenConditionLoc(const AuCondition& condition) const;

	virtual void PanelDimensions(double& width, double& height) const = 0;

	void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaCapsule& capsule,
			CONTROL_CONTEXT cc);

	void ProcessDisplayHit(const QPoint& pxHit);

	bool TestInsidePatientMask(const QPoint& pxTest) const;
};

#endif // DpStationSurgery_H
