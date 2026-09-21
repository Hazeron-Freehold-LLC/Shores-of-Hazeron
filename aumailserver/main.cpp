//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.13 2022/02/19 18:37:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include <auglobal.h>
#include "aumailserverapp.h"
#include <iostream>
#include <signal.h>

void SignalHandler(int value)
{
	auMailServerApp->HandleSignal(value);
}

int main(int argc, char** argv)
{
	// Install signal handler(s).
	signal(SIGINT, SignalHandler);	// Termination waits for db save to finish.
	signal(SIGTERM, SignalHandler);	// Termination waits for db save to finish.
#if !defined(Q_OS_WIN)
	signal(SIGHUP, SignalHandler);	// Hangup gets ignored.
	signal(SIGQUIT, SignalHandler);	// Termination waits for db save to finish.
#endif

	AuMailServerApp a(argc, argv);
	if(argc > 1)
	{
		QStringList args = a.arguments();
		if(args.contains("-version") || args.contains("-v"))
		{
			// Display version information and then exit.
			std::cout << "AuMailServer " << AuUtil::VersionFull().toLatin1().data() << '\n';
			exit(0);
		}
	}
	return a.exec();
}
