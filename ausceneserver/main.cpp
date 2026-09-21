//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.40 2022/02/19 18:40:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "auatomres.h"
#include "aubodypartsres.h"
#include "auloginclient.h"
#include "ausceneserverapp.h"
#include "aumaillib.h"
#include "aumodelres.h"
#include "daatom.h"
#include <iostream>
#include <signal.h>

void SignalHandler(int value)
{
	auSceneServerApp->HandleSignal(value);
}

int main(int argc, char** argv)
{
	int returnValue = 0;
	{
		// Install signal handler(s).
		signal(SIGINT, SignalHandler);	// Termination waits for db save to finish.
		signal(SIGTERM, SignalHandler);	// Termination waits for db save to finish.
#if !defined(Q_OS_WIN)
		signal(SIGPIPE, SignalHandler);	// Broken pipe gets ignored.
		signal(SIGHUP, SignalHandler);	// Hangup gets ignored.
		signal(SIGQUIT, SignalHandler);	// Termination waits for db save to finish.
#endif

		// Create an object from each library, so their constructors can call the appropriate Q_INIT_RESOURCE macros.
		// The objects for shared libraries must persist through the life of the program. Otherwise, they get thrashed loading and
		// unloading every time a resource is needed on Windows, which makes this program load very slowly.
		AuAtom auAtom;
		AuAtomRes auAtomRes;
		AuBodyPartsRes auBodyPartsRes;
		AuData auData;
		AuGL auGL;
		AuGlobalLib auGlobal;
		AuLoginClient auLoginClient;
		AuMailLib auMailDll;
		AuModelRes auModelRes;
		AuUtil auUtil;

		bool vonly = false;
		AuSceneServerApp a(argc, argv);
		if(argc > 1)
		{
			QStringList args = a.arguments();
			if(args.contains("-version") || args.contains("-v"))
			{
				// Display version information and then exit.
				cout << "AuSceneServer " << (const char*)AuUtil::VersionFull().toLatin1() << endl;
				vonly = true;
			}
		}
		if(!vonly)
			returnValue = a.exec();
	}

	cout << "Finished with result " << returnValue << endl;
#if defined(MEM_TRACE_ATOM)
	{
		// Report atom pointers that leaked.
		if(DaAtom::AllocatedCount() != DaAtom::FreedCount())
			cout << "Atoms: " << DaAtom::AllocatedCount() << " allocated, "
				<< DaAtom::FreedCount() << " freed "
				<< ((qint64)DaAtom::AllocatedCount() - (qint64)DaAtom::FreedCount()) << " leaked." << endl;
		else
			cout << "Atoms: " << DaAtom::AllocatedCount() << " allocated and freed." << endl;
	}
#endif
#if defined(MEM_TRACE_GLOBAL)
	{
		// Report global pointers that leaked.
		// If this is the only block of code in main(), 11 pointers leak.
		// If the nnnRes objects are loaded, 34 pointers leak, more if I use Q_CLEANUP_RESOURCE in their destructors.
		// Constructing the app object, without StartThreads and with DtDb disabled, 136 pointers leak.
		// Constructing the app object, with DtDb disabled, 151 pointers leak.
		// Worker server startup and shutdown without any systems assigned, 585 pointers leak.
		// Worker server startup, assignment of Hazeron system, and shutdown, 228 pointers leak.
		if(AuSceneServerApp::AllocatedCount() != AuSceneServerApp::FreedCount())
			cout << "Global: " << AuSceneServerApp::AllocatedCount() << " allocated, "
				<< AuSceneServerApp::FreedCount() << " freed "
				<< ((qint64)AuSceneServerApp::AllocatedCount() - (qint64)AuSceneServerApp::FreedCount()) << " leaked." << endl;
		else
			cout << "Global: " << AuSceneServerApp::AllocatedCount() << " allocated and freed." << endl;
	}
#endif
	//AuGlobal::MemoryInfo();
	return returnValue;
}
