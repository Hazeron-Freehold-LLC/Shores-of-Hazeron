//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.9 2022/04/15 22:49:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "auapp.h"
#include "auatomres.h"
#include "aubodypartsres.h"
#include "auclientres.h"
#include "auclientmailres.h"
#include "auframe.h"
#include "auglsl.h"
#include "aumodelres.h"
#include "autexture.h"
#include "auutil.h"
#include <iostream>
#include <QDir>
#include <QMessageBox>
#include <signal.h>

void SignalHandler(int value)
{
	auApp->HandleSignal(value);
}

int main(int argc, char** argv)
{
	int returnValue;
	{
		// Install signal handler(s).
		signal(SIGINT, SignalHandler);
		signal(SIGTERM, SignalHandler);
#if !defined(Q_OS_WIN)
		signal(SIGPIPE, SignalHandler);	// Broken pipe gets ignored.
		signal(SIGHUP, SignalHandler);
		signal(SIGQUIT, SignalHandler);
#endif

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
		/*
		qDebug() << "Library Paths";
		for(QStringList::const_iterator pi = libPaths.cbegin(); pi != libPaths.cend(); pi++)
			qDebug() << *pi;
		*/

		QCoreApplication::setLibraryPaths(libPaths);

		// Must set default OpenGL version before app constructor, particularly for Mac.
		QSurfaceFormat glformat;
		glformat.setProfile(QSurfaceFormat::CoreProfile);
		glformat.setVersion(GLMajorVersion, GLMinorVersion);
		QSurfaceFormat::setDefaultFormat(glformat);

		// Create an object from each library, so their constructors can call the appropriate Q_INIT_RESOURCE macros.
		// The objects for shared libraries must persist through the life of the program. Otherwise, they get thrashed loading and
		// unloading every time a resource is needed on Windows, which makes this program load very slowly.
		AuAtomRes auAtomRes;
		AuBodyPartsRes auBodyPartsRes;
		AuClientRes auClientRes;
		AuClientMailRes auClientMailRes;
		AuGL auGL;
		AuModelRes auModelRes;
		AuTexture auTexture;
		AuUtil auUtil;

		AuApp a(argc, argv);

		// Create the main frame window.
		AuFrame cw;
		a.SetFrame(&cw);
		cw.showNormal();

		// Start chuggin'.
		returnValue = a.exec();
	}

	return returnValue;
}
