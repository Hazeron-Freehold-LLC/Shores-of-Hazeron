//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationgame.cpp,v 1.7 2022/02/19 17:46:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationgame.h"
//
#include "augleffects.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include "dicontrolinput.h"

// Table of control inputs.
DiControlInputArray s_GameConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_GameConsoleDefaults[] =
{
	DiControlInput(QObject::tr("Select A New Game"), DpStationGame::CI_SelectGame,
				Qt::Key_N, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumGameConsoleDefaults = sizeof(s_GameConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationGame::DpStationGame(AuStation::TYPE stationType, quint16 whichTable) : DpStation()
{
	m_StationType = stationType;
	m_WhichTable = whichTable;
}

DpStationGame::~DpStationGame()
{
}

DpStationGame::DpStationGame(const DpStationGame& c) : DpStation(c)
{
	m_StationType = c.m_StationType;
	m_WhichTable = c.m_WhichTable;
}

void DpStationGame::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.constBegin(); ci != aAllControls.constEnd(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_GameConsoleControls.begin(); myci != s_GameConsoleControls.end(); myci++)
		{
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the nu data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void DpStationGame::RestoreDefaultControls()
{
	s_GameConsoleControls.clear();
	for(int ic=0; ic<s_NumGameConsoleDefaults; ic++)
		s_GameConsoleControls.append(s_GameConsoleDefaults[ic]);
	SaveControls();
}

void DpStationGame::LoadControls() const
{
	// Init with my default controls.
	s_GameConsoleControls.clear();
	for(int ic=0; ic<s_NumGameConsoleDefaults; ic++)
		s_GameConsoleControls.append(s_GameConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_GameConsoleControls.LoadSettings("Controls/Game Table");
}

void DpStationGame::SaveControls() const
{
	// Store my controls in the settings file.
	s_GameConsoleControls.SaveSettings("Controls/Game Table");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationGame::EnumControlInputs(DiControlInputArray &aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_GameConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_GameConsoleControls.constBegin(); ci != s_GameConsoleControls.constEnd(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be // passed.
// Only control inputs that have been activated will be passed.
void DpStationGame::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* /*pSceneAtom*/, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	Q_UNUSED(cc);
	// None of the actions of this console can be performed on a "nu" ship.
	if(ship.IsNew())
		return;

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_SelectGame:
			// Game selection is performed on the client by watching for this active control as a special case in AuView.
			// It gets ignored here, if it ever even makes it this far.
			break;
		}
	}
}

void DpStationGame::Tx(QDataStream& out) const
{
	DpStation::Tx(out);
	quint8 hasWhat = 0;
	if(m_StationType != AuStation::Table_Game_0)
		hasWhat |= 0x01;
	if(m_WhichTable)
		hasWhat |= 0x02;
	out << hasWhat;
	if(hasWhat & 0x01)
		out << m_StationType;
	if(hasWhat & 0x02)
		out << m_WhichTable;
}

void DpStationGame::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);
	quint8 hasWhat;
	in >> hasWhat;
	if(hasWhat & 0x01)
		in >> m_StationType;
	else
		m_StationType = AuStation::Table_Game_0;
	if(hasWhat & 0x02)
		in >> m_WhichTable;
	else
		m_WhichTable = 0;
}

// Refresh data from counterpart.
void DpStationGame::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.
	const DpStationGame* pA = (const DpStationGame*)pCon;
	m_StationType = pA->m_StationType;
	m_WhichTable = pA->m_WhichTable;
}

quint64 DpStationGame::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_StationType);
	mem += sizeof(m_WhichTable);
	return mem;
}
