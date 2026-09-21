//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationgame.h,v 1.6 2022/02/19 17:37:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationGame_H
#define DpStationGame_H

#include "auatom.h"
#include "dpstation.h"

class AUATOM_DLL DpStationGame : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_SelectGame,
	};

private:
	quint8			m_StationType;
	quint16			m_WhichTable;	// Index of game table in ship design analysis.

public:
	DpStationGame(AuStation::TYPE stationType, quint16 whichTable);
	virtual ~DpStationGame();
	DpStationGame(const DpStationGame& c);

public:
	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void EnumControlInputs(DiControlInputArray& /*aAllControls*/) const;

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	virtual void ProcessControlInput(const DiControlInputArray& /*aActiveControls*/, DaPerson* /*pSceneAtom*/,
			DaVessel& /*ship*/, CONTROL_CONTEXT /*cc*/, const AuPoint& /*location*/);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);	// Refresh data from counterpart.

	void RestoreDefaultControls();

	virtual void Rx(QDataStream& in, DaCapsule& capsule);

	void SaveControls() const;

	virtual quint16 SystemWhich() const
	{
		return m_WhichTable;
	}

	virtual void Tx(QDataStream& out) const;

	virtual AuStation::TYPE Type() const
	{
		return (AuStation::TYPE)m_StationType;
	}
};

#endif // DpStationGame_H
