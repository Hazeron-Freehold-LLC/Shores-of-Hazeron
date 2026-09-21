//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationempty.h,v 1.10 2022/02/19 17:37:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationEmpty_H
#define DpStationEmpty_H

#include "auatom.h"
#include "dpstation.h"

class DaCapsule;

class AUATOM_DLL DpStationEmpty : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_DoorRemote,
		CI_HeadlightOnOff,
		CI_RemoteCameraNext,
		CI_RemoteCameraPrevious,
		CI_RemoteCameraOnOff,
		CI_UNUSED3, // used to be Unlock Crew Doors,
		CI_UNUSED4, // used to be Unlock Hull Doors,
		CI_CloseHullDoors,
	};

private:
	quint16								m_RemoteCameraId;		// Part id of current remote camera.
	bool								m_RemoteCameraSwitch;	// True if remote camera is on.

protected:
	QString								m_ScreenName;			// Name to be used for the texture identifier.
	const char*							m_ScreenFile;			// File name for screen texture.

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationEmpty();
	virtual ~DpStationEmpty();
	DpStationEmpty(const DpStationEmpty& c);

public:
	void ConfigureControls(const DiControlInputArray &aAllControls);

	// 'inBuilding' is true if this console is in a building, false if this console is in a spacecraft.
	virtual void CreateDeviceObjectsEmpty(AuAtomRenderInfo& ari, const DaAtom& parent, const QString& screenName, const char* screenFile,
			bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	virtual void EnumControlInputs(DiControlInputArray& aAllControls) const;

	inline bool HasDeviceObjects() const
	{
		return !m_ScreenName.isEmpty();
	}

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* person, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	// Refresh data from counterpart.
	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);

	virtual quint16 RemoteCameraId() const;

	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	void RestoreDefaultControls();

	void SaveControls() const;

	virtual int SystemNamesVisible() const
	{
		return -1;
	}

private:
	bool RemoteCameraNext(const DaCapsule& cap);
	bool RemoteCameraPrevious(const DaCapsule& cap);
	bool RemoteCameraSwitch(const DaCapsule& cap);
};

#endif // DpStationEmpty_H
