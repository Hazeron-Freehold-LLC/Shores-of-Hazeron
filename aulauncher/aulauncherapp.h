//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulauncherapp.h,v 1.7 2022/02/19 18:34:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLauncherApp_H
#define AuLauncherApp_H

#include <QApplication>

class AuLauncherApp : public QApplication
{
	Q_OBJECT;
public:
	AuLauncherApp(int& argc, char** argv);
	~AuLauncherApp();
};

#define auLauncherApp	((AuLauncherApp*)qApp)

#endif // AuLauncherApp_H
