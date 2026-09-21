//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.29 2022/02/19 18:37:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "auclientmailres.h"
#include "auglobal.h"
#include "aumail.h"
#include "aumailclientapp.h"
#include "aumaillogin.h"
#include "auutil.h"
#include "dgdata.h"
#include <QDir>
#include <QFileInfo>
#include <iostream>

int main(int argc, char** argv)
{
	// Determine the path to my files.
	QFileInfo exe(QString::fromLocal8Bit(argv[0]));
	QDir dir(exe.absoluteDir());

	// Change my current directory.
	QDir::setCurrent(dir.absolutePath());

	QStringList libPaths;
#if defined(Q_OS_MACOS)
	libPaths << dir.absolutePath();
	dir.cdUp();
	dir.cd("PlugIns");
	libPaths << dir.absolutePath();
#elif defined(Q_OS_WIN)
	libPaths << dir.absolutePath();
	dir.cd("platforms");
	libPaths << dir.absolutePath();
#endif
	libPaths << QCoreApplication::libraryPaths();

	QCoreApplication::setLibraryPaths(libPaths);

	// Create an object from each library, so their constructors can call the appropriate Q_INIT_RESOURCE macros.
	// The objects for shared libraries must persist through the life of the program. Otherwise, they get thrashed loading and
	// unloading every time a resource is needed on Windows, which makes this program load very slowly.
	AuClientMailRes auClientMailRes;
	AuUtil auUtil;

	AuMailClientApp a(argc, argv);
	if(argc > 1)
	{
		QStringList args = a.arguments();
		if(args.contains("-version") || args.contains("-v"))
		{
			// Display version information and then exit.
			std::cout << "AuMailClient " << AuUtil::VersionFull().toLatin1().data() << '\n';
			exit(0);
		}
	}

	AuMailLogin login;
	if(login.exec() == QDialog::Accepted)
	{
		// We store the name and password locally for use in db lookups (contacts)
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetAcctName(login.GetAcctName());
		gd.SetAcctPassCrypt(login.GetAcctPassCrypt());
		g_Au.UnlockData();

		AuMail cw;
		cw.show();
		return a.exec();
	}
	return 0;
}
