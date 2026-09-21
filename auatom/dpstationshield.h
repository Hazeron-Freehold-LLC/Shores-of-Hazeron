//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationshield.h,v 1.13 2022/04/14 16:42:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationShield_H
#define DpStationShield_H

#include "auatom.h"
#include "dpstation.h"

class AUATOM_DLL DpStationShield : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_PowerOnOff,			// Shield system power toggle.

		CI_Reset,				// Sets shield power allocation to be the same for all sides.

		CI_FrontAll,			// Allocate all shield power to the front shield.
		CI_FrontDec,			// Decrement shield power allocated to the front shield.
		CI_FrontInc,			// Increment shield power allocated to the front shield.
		CI_FrontNone,			// Allocate no shield power to the front shield.

		CI_BackAll,
		CI_BackDec,
		CI_BackInc,
		CI_BackNone,

		CI_LeftAll,
		CI_LeftDec,
		CI_LeftInc,
		CI_LeftNone,

		CI_RightAll,
		CI_RightDec,
		CI_RightInc,
		CI_RightNone,

		CI_TopAll,
		CI_TopDec,
		CI_TopInc,
		CI_TopNone,

		CI_BottomAll,
		CI_BottomDec,
		CI_BottomInc,
		CI_BottomNone,
	};

private:
	// Shield Values monitored on the server that are not available in the ship's data on the client.
	float				m_Shields[6];		// Percentage of shield energy at the front, back, left, right, top, and bottom.
	double				m_ShieldUnits;		// Number of shield units actually functioning.

	// Values used for device animation...NOT serialized.
	int					m_LastShieldsPower;	// Shield power on/off setting at last update.
	double				m_LastShieldsUnits;	// Number of functioning shield units on board at last update.
	float				m_LastShields[6];	// Shield power percentage at last update.

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationShield();
	virtual ~DpStationShield();
	DpStationShield(const DpStationShield& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);    // Refresh data from counterpart.

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);
	virtual void Tx(QDataStream& out) const;

protected:
	static GLuint CreateBuildingDisplayTexture(AuGLEffects& re, const AuId& parentId);

	static GLuint CreateSpacecraftDisplayTexture(AuGLEffects& re, const AuId& parentId);

	virtual void PanelDimensions(double& width, double& height) const = 0;
};

#endif // DpStationShield_H
