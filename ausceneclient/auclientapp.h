//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientapp.h,v 1.8 2022/02/19 18:39:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientApp_H
#define AuClientApp_H

#include "audesignerapp.h"

class AuClientApp : public AuDesignerApp
{
	Q_OBJECT;
public:	// Construction
	AuClientApp(int& argc, char** argv);
	virtual ~AuClientApp();

	virtual QString Icon128() const;

	virtual QString Icon32() const;

	virtual bool IsHazeronStarship() const
	{
		return false;
	}

	virtual QString Slogan() const;
};

#define auClientApp	((AuClientApp*)qApp)

#endif // AuClientApp_H
