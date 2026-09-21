//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerapp.h,v 1.9 2022/02/19 18:21:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerApp_H
#define AuDesignerApp_H

#include "auapp.h"
#include "audesigner.h"

class AuDesignerFrame;
class AuTime;
class DaAtom;
class DtTrash;

class AUDESIGNER_DLL AuDesignerApp : public AuApp
{
	Q_OBJECT;
	DtTrash*        					m_tTrash;       // Deletes atoms from memory.

public:	// Construction
	AuDesignerApp(int& argc, char** argv, const QString& appName, const QString& settingsAppName, bool isSolo);
	virtual ~AuDesignerApp();

public:	// Methods
	inline AuDesignerFrame* DesignerFrame()
	{
		return (AuDesignerFrame*)Frame();
	}

	virtual bool IsHazeronCommander() const
	{
		return false;
	}

	virtual void SceneLogin();	// Send login packet to scene server.
};

#define auDesignerApp	((AuDesignerApp*)qApp)

#endif // AuDesignerApp_H
