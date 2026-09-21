//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.15 2022/02/19 18:34:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "aulauncherapp.h"
#include "aulauncherdialog.h"
#include "auutil.h"
#include <QDir>

int main(int argc, char *argv[])
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
	AuUtil auUtil;

	AuLauncherApp app(argc, argv);

	AuLauncherDialog frame;
	frame.show();
	return app.exec();
}
