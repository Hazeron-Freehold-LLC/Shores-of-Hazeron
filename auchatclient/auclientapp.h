//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientapp.h,v 1.6 2022/02/19 17:47:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientApp_H
#define AuClientApp_H

#include "auapp.h"

class AuClientFrame;

class AuClientApp : public AuApp
{
	Q_OBJECT;
public:	// Construction
	AuClientApp(int& argc, char** argv);
	virtual ~AuClientApp();

public:	// Methods
	inline AuClientFrame* ClientFrame()
	{
		return (AuClientFrame*)Frame();
	}

	virtual QString Icon128() const;

	virtual QString Icon32() const;

	virtual bool IsHazeronCommander() const
	{
		return true;
	}

	virtual bool IsHazeronStarship() const
	{
		return false;
	}

	virtual void SceneLogin();	// Send login packet to scene server.

	virtual QString Slogan() const;
};

#define auClientApp	((AuClientApp*)qApp)

#endif // AuClientApp_H
