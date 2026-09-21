//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationpower.h,v 1.12 2022/04/14 16:42:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationPower_H
#define DpStationPower_H

#include "auatom.h"
#include "davessel.h"
#include "dpstation.h"

#define MAX_POWER_PLANT_SHUNTS 8

class AUATOM_DLL DpStationPower : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_AlterLevel1Down = 0,
		CI_AlterLevel2Down,
		CI_AlterLevel3Down,
		CI_AlterLevel4Down,
		CI_AlterLevel5Down,
		CI_AlterLevel6Down,
		CI_AlterLevel7Down,
		CI_AlterLevel8Down,
		CI_AlterLevel1Up,
		CI_AlterLevel2Up,
		CI_AlterLevel3Up,
		CI_AlterLevel4Up,
		CI_AlterLevel5Up,
		CI_AlterLevel6Up,
		CI_AlterLevel7Up,
		CI_AlterLevel8Up,
		CI_Connection1Consumer,
		CI_Connection2Consumer,
		CI_Connection3Consumer,
		CI_Connection4Consumer,
		CI_Connection5Consumer,
		CI_Connection6Consumer,
		CI_Connection7Consumer,
		CI_Connection8Consumer,
		CI_Connection1Supplier,
		CI_Connection2Supplier,
		CI_Connection3Supplier,
		CI_Connection4Supplier,
		CI_Connection5Supplier,
		CI_Connection6Supplier,
		CI_Connection7Supplier,
		CI_Connection8Supplier,
		CI_Consumer1Down,
		CI_Consumer2Down,
		CI_Consumer3Down,
		CI_Consumer4Down,
		CI_Consumer5Down,
		CI_Consumer6Down,
		CI_Consumer7Down,
		CI_Consumer8Down,
		CI_Consumer1Up,
		CI_Consumer2Up,
		CI_Consumer3Up,
		CI_Consumer4Up,
		CI_Consumer5Up,
		CI_Consumer6Up,
		CI_Consumer7Up,
		CI_Consumer8Up,
		CI_PowerShunt1,
		CI_PowerShunt2,
		CI_PowerShunt3,
		CI_PowerShunt4,
		CI_PowerShunt5,
		CI_PowerShunt6,
		CI_PowerShunt7,
		CI_PowerShunt8,
		CI_Supplier1Down,
		CI_Supplier2Down,
		CI_Supplier3Down,
		CI_Supplier4Down,
		CI_Supplier5Down,
		CI_Supplier6Down,
		CI_Supplier7Down,
		CI_Supplier8Down,
		CI_Supplier1Up,
		CI_Supplier2Up,
		CI_Supplier3Up,
		CI_Supplier4Up,
		CI_Supplier5Up,
		CI_Supplier6Up,
		CI_Supplier7Up,
		CI_Supplier8Up,

		CI_Login,
	};

private:
	SYSTEM								m_ConsumerType[MAX_DISPLAYED_SYSTEMS];

	quint8								m_SupplierDamage[MAX_DISPLAYED_SYSTEMS];		// max 15
	SYSTEM								m_SupplierType[MAX_DISPLAYED_SYSTEMS];
	
	QHash<SYSTEM, QImage>				m_SystemImages;			// 16x16 images of systems, on solid black background.

	qint32		m_iSystemCount;				// Total Number of systems on ship

	quint8		m_PowerPlantShunts;			// Each bit represents a power plant shunt that is on.

	qint16		m_LastPowerPlantHealth;		// Last displayed health of power plant, in the range 0-255. -1 is the undrawn state.
	qint16		m_LastPowerPlantShunts;		// Last displayed state of shunts. -1 is the undrawn state.
	
	// Power Suppliers / Consumers variables
	qint32		m_iSupplierIndex[MAX_DISPLAYED_SYSTEMS];
	qint32		m_iConsumerIndex[MAX_DISPLAYED_SYSTEMS];

	// Connection variables
	int			m_iSelectedSupplier;		// System Index of chosen Supplier, for connection to the next selected consumer.

	// HIGH 3 = Overclocked (true/false)
	// LOW 5 = Power in pixels (max 31)
	quint8 m_q8SupplierPower[MAX_DISPLAYED_SYSTEMS];
	
	// HIGH 4 = Connection (off, Connect, D/C)
	// LOW 4 = Consumer (MAX_DISPLAYED_SYSTEMS or higher means offscreen)
	quint8 m_q8SupplierConnectionConsumer[MAX_DISPLAYED_SYSTEMS];
	
	// HIGH 3 = Connection (off, Connect, D/C)
	// LOW 5 = Power in pixels (max 31)
	quint8 m_q8ConsumerConnectionAndPower[MAX_DISPLAYED_SYSTEMS];
	
protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationPower();
	virtual ~DpStationPower();
	DpStationPower(const DpStationPower& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	// Power Suppliers / Consumers functions
	void FindPreviousSupplier(int iIndex);
	void FindNextSupplier(int iIndex);
	void FindPreviousConsumer(int iIndex);
	void FindNextConsumer(int iIndex);

	virtual quint64 MemoryUsage() const;

	void SystemImage(SYSTEM system, QImage& image);

	// Overclock functions
	void ToggleOverclock(int iIndex, DaVessel& ship);

	// Connection functions
	void SupplierSelection(int iIndex, DaVessel& ship);
	void ConsumerSelection(int iIndex, DaVessel& ship);

	// Power Plant Shunt System functions
	void TogglePowerPlantShunt(int iIndex, DaVessel& ship);

	void LoadControls() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);    // Refresh data from counterpart.
	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
		DiControlInputArray &aActiveControls, bool bContinuous);
	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);
	virtual void Tx(QDataStream& out) const;

protected:
	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);

	virtual void PanelDimensions(double& width, double& height) const = 0;
};

#endif // DpStationPower_H
