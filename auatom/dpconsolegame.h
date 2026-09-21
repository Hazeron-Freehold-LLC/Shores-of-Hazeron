//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsolegame.h,v 1.23 2022/02/19 17:36:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpConsoleGame_H
#define DpConsoleGame_H

#include "auatom.h"
#include "dpconsole.h"
#include "dpstationgame.h"

class AUATOM_DLL DpConsoleGame : public DpStationGame, public DpConsole
{
public:
	DpConsoleGame(AuStation::TYPE stationType, quint16 whichTable);
	virtual ~DpConsoleGame();
	DpConsoleGame(const DpConsoleGame& c);

public:
	virtual DpStation* Clone() const;
};

#endif // DpConsoleGame_H
