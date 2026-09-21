//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsolegame.cpp,v 1.22 2022/02/19 17:44:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpconsolegame.h"
//

DpConsoleGame::DpConsoleGame(AuStation::TYPE stationType, quint16 whichTable) : DpStationGame(stationType, whichTable)
{
}

DpConsoleGame::~DpConsoleGame()
{
}

DpConsoleGame::DpConsoleGame(const DpConsoleGame& c) : DpStationGame(c), DpConsole(c)
{
}

DpStation* DpConsoleGame::Clone() const
{
	return new DpConsoleGame(*this);
}
