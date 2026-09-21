//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.12 2022/03/10 22:35:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientmailres.h"
#include "auservermonapp.h"
#include "auservermonframe.h"
#include "auutil.h"
#include <QDir>

int main(int argc, char *argv[])
{
	// Determine the path to my files.
	QFileInfo exe(QString::fromLocal8Bit(argv[0]));
	QDir dir(exe.absoluteDir());

	// Change my current directory.
	QDir::setCurrent(dir.absolutePath());

#if defined(QT_NO_DEBUG)
	QStringList libPaths;
#else
	// In debug mode, we need the default library paths.
	QStringList libPaths = QCoreApplication::libraryPaths();
#endif

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
	QCoreApplication::setLibraryPaths(libPaths);

	// Create an object from each library, so their constructors can call the appropriate Q_INIT_RESOURCE macros.
	// The objects for shared libraries must persist through the life of the program. Otherwise, they get thrashed loading and
	// unloading every time a resource is needed on Windows, which makes this program load very slowly.
	AuClientMailRes auClientMailRes;
	AuUtil auUtil;

	AuServerMonApp a(argc, argv);
	AuServerMonFrame w;
	a.SetMainWindow(&w);
	w.show();
	return a.exec();
}

