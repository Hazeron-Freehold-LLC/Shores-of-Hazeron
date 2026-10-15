//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.41 2022/02/19 18:57:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include <auglobal.h>
#include <austory.h>
#include <dbcommodity.h>
#include <dbindustry.h>
#include <dgdata.h>
#include <getopt.h>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QMatrix4x4>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextStream>
#include <QUrl>

// This AuId object needs to be instantiated to clear up a linker ambiguity.
//AuId g_id;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#if !defined(QT_NO_DEBUG)
void DoIntrinsics();
void DoMyTest();
void DoTranslate();
#endif

int CompileScript(const QString& dataPath, const QString& scriptFileName, const QString& scriptId);
void CreateDataFile(const QString& dataPath);
void PrintBuildings(const QString& dataPath);
void PrintCommodities(const QString& dataPath);
void PrintEmpires(const QString& dataPath);
void PrintFunctions();
void PrintFunctionsWiki();
void PrintIdString(quint32 idVal);
void PrintIdValue(const QString& stringId);
void PrintParameterTypes();
void PrintParameterTypesWiki();
void PrintRanges();
void PrintStandardChannels();
void PrintUsage();
void PrintVersion();
bool ReadDataFile(const QString& dataPath, DbCommodityArray& commodities, DbIndustryArray& industries, DgEmpireHash& empires);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int main(int argc, char** argv)
{
	int retval = 0;
	bool didSomething = false;
	QCoreApplication a(argc, argv);
	QStringList args = a.arguments();

	// Extract my path from arg 0.
	QFileInfo fileInfo(args.first());
	QString dataPath = fileInfo.path();

	// Recreate data file needed by the compiler because the Universe database will not be accessible to everyone.
	// Do this first because some other command line options depend on the presense of the data file.
	if(args.contains("-d"))
	{
		CreateDataFile(dataPath);
		didSomething = true;
	}

	if(args.contains("-e"))
	{
		PrintEmpires(dataPath);
		didSomething = true;
	}

	if(args.contains("-f"))
	{
		PrintFunctions();
		didSomething = true;
	}

	if(args.contains("-fw"))
	{
		PrintFunctionsWiki();
		didSomething = true;
	}

	if(args.contains("-b"))
	{
		PrintBuildings(dataPath);
		didSomething = true;
	}

	if(args.contains("-p"))
	{
		PrintParameterTypes();
		didSomething = true;
	}

	if(args.contains("-pw"))
	{
		PrintParameterTypesWiki();
		didSomething = true;
	}

	if(args.contains("-l"))
	{
		PrintStandardChannels();
		didSomething = true;
	}

	if(args.contains("-m"))
	{
		PrintCommodities(dataPath);
		didSomething = true;
	}

	if(args.contains("-r"))
	{
		PrintRanges();
		didSomething = true;
	}

	if(args.contains("-v"))
	{
		PrintVersion();
		didSomething = true;
	}

	// Undocumented options.
	if(args.contains("-id"))
	{
		int argidx = args.indexOf("-id");
		if(argidx+1 < args.count())
		{
			QString sid = args[argidx+1];
			PrintIdValue(sid);
			didSomething = true;
		}
	}

	if(args.contains("-stringid"))
	{
		int argidx = args.indexOf("-stringid");
		if(argidx+1 < args.count())
		{
			QString sid = args[argidx+1];
			PrintIdString(QVariant(sid).toUInt());
			didSomething = true;
		}
	}

#if !defined(QT_NO_DEBUG)
	if(args.contains("-test"))
	{
		//DoIntrinsics();
		DoMyTest();
		//DoTranslate();
		didSomething = true;
	}
#endif

	// Test for the command to compile a script.
	int farg = args.indexOf("-c");
	if(farg >= 0)
	{
		// We are commanded to compile a script.
		QString scriptName("script.sh");	// Default script name.
		QString scriptId;
		if(farg+1 < args.count() && args[farg+1][0] != '-')
			scriptId = args[farg+1];		// A script id was provided.
		retval = CompileScript(dataPath, scriptName, scriptId);
		didSomething = true;
	}

	if(!didSomething && args.size() > 1)
	{
		if(args[1][0] != '-')
		{
			// Take the next argument as a file name and try to compile it.
			QString scriptName(args[1]);
			QString scriptId;
			retval = CompileScript(dataPath, scriptName, scriptId);
			didSomething = true;
		}
	}

	if(!didSomething)
	{
		PrintUsage();
		retval = 1;
	}

	return retval;
}

void PrintUsage()
{
	QTextStream out(stdout);
	out << "Hazeron Story Script Compiler\n";
	out << " Usage: AuScript [-b] [-c [id]] [-d] [-e] [-f] [-l] [-m] [-p] [-r] [-v] [-?]\n";
	out << "  -b    Outputs the list of building names.\n";
	out << "  -c    Compile script.sh file.\n";
	out << "  id    Script id cannot have spaces. Appears in output. Has no effect on the compilation.\n";
	out << "  -d    Creates 'AuScript.bin' file. Access to Universe database server needed.\n";
	out << "  -e    Outputs the list of empire names.\n";
	out << "  -f    Outputs the list of functions and their parameters.\n";
	out << "  -fw   Outputs the list of functions and their parameters in wiki format.\n";
	out << "  -l    Outputs the list of standard communication channel names.\n";
	out << "  -m    Outputs the list of commodity names.\n";
	out << "  -p    Outputs a description of the function parameter types.\n";
	out << "  -pw   Outputs a description of the function parameter types in wiki format.\n";
	out << "  -r    Outputs the list of communication ranges.\n";
	out << "  -v    Outputs the program version.\n";
	out << "  -?    Outputs this program usage information.\n";
}

void PrintVersion()
{
	QTextStream out(stdout);
	out << "Hazeron Story Script Compiler " << AuUtil::VersionFull() << '\n';
}

int CompileScript(const QString& dataPath, const QString& scriptFileName, const QString& scriptId)
{
	DgData* data = new DgData;
	gp_AuGlobal = new AuGlobal("Hazeron Story Script Compiler", "Hazeron Freehold", AuGlobal::WebCGI, data, 0);

	// Read the data file.
	if(!ReadDataFile(dataPath, gp_AuGlobal->m_CommodityArray, data->m_bIndustryArray, data->m_gEmpires))
		return 1;

	int retval = 0;
	AuScript script(1);
	int result = script.Compile(scriptFileName);
	if(result == 0)
	{
		QTextStream out(stdout);
		//out << "Story script compiled successfully.\n";
		if(!scriptId.isEmpty())
			out << "   Script: " << scriptId << "\n";
		out << "    Title: " << script.Title() << "\n";
		out << "   Author: " << script.Author() << "\n";
		out << "    Brief: " << script.Brief() << "\n";
		out << "   Blocks: " << script.BlockCount() << "\n";
		if(script.AutoStart())
			out << "     Auto: Start\n";
		else if(script.AutoRepeat())
			out << "     Auto: Repeat\n";
		else if(script.AutoRepeatUntilComplete())
			out << "     Auto: Repeat Until Complete\n";
		if(script.Interval())
			out << " Interval: " << script.Interval() << " minutes\n";
	}
	else if(result == 2)
	{
		QTextStream out(stdout);
		out << "  Terminated: #nocompile\n";
	}
	else
	{
		QTextStream out(stderr);
		for(QStringList::const_iterator si = script.CompilerOutput().cbegin(); si != script.CompilerOutput().cend(); si++)
			out << (*si) << '\n';
		retval = 2;
	}

	delete data;

	return retval;
}

// Create a data file from information in the Universe database.
void CreateDataFile(const QString& dataPath)
{
	QSqlDatabase dbUniverse = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
	if(!dbUniverse.isValid())
	{
		qWarning("Failed to add the Universe database object.");
		return;
	}

	// auUniverse database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	dbUniverse.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
	dbUniverse.setHostName("UniverseDb.Hazeron.net");
	if(!dbUniverse.open("audog","Gold"))
	{
		qWarning("Failed to connect to the Universe database.");
		return;
	}

	// Commodities
	DbCommodityArray commodities;
	commodities.Load();

	// Industries
	DbIndustryArray industries;
	industries.Load();

	// Empires
	DgEmpireHash empires;
	empires.Load();
	QHash<quint32, QString> empireNames;	// key is empire id.
	for(DgEmpireHash::const_iterator ei = empires.cbegin(); ei != empires.cend(); ei++)
	{
		if(!(*ei).second.IsPirate() &&
				!(*ei).second.IsArenaEmpire() &&
				!(*ei).second.IsAgentEmpire() &&
				(*ei).second.Name() != "Imperium")
			continue;
		empireNames.insert((*ei).first, (*ei).second.Name());
	}

	// We're done with the databases now.
	dbUniverse.close();

	// Create the data file.
	QFile dataFile(dataPath + "/AuScript.bin");
	if(dataFile.open(QIODevice::WriteOnly))
	{
		QDataStream out(&dataFile);
		out.setVersion(QDataStream::Qt_4_1);
		commodities >> out;
		industries >> out;
		out << empireNames;
		dataFile.close();
	}
}

bool ReadDataFile(const QString& dataPath, DbCommodityArray& commodities, DbIndustryArray& industries, DgEmpireHash& empires)
{
	QHash<quint32, QString> empireNames;	// key is empire id.

	// Read the data file.
	QFile dataFile(dataPath + "/AuScript.bin");
	if(dataFile.open(QIODevice::ReadOnly))
	{
		QDataStream in(&dataFile);
		in.setVersion(QDataStream::Qt_4_1);
		commodities << in;
		industries << in;
		in >> empireNames;
		dataFile.close();

		empires.clear();
		for(QHash<quint32, QString>::const_iterator ei = empireNames.cbegin(); ei != empireNames.cend(); ei++)
		{
			DgEmpire empire;
			empire.m_id = ei.key();
			empire.ChangeEmpireName("", *ei);
			empires.insert(pair<quint32, DgEmpire>(ei.key(), empire));
		}
		return true;
	}
	else
	{
		qWarning("AuScript ERROR: Unable to open data file. Use the -d option to create the data file.");
		return false;
	}
}

void PrintBuildings(const QString& dataPath)
{
	// Read the data file.
	DbCommodityArray commodities;
	DbIndustryArray industries;
	DgEmpireHash empires;
	if(!ReadDataFile(dataPath, commodities, industries, empires))
		return;

	// Alphabetize the building names.
	QMap<QString,bool> bldnames;
	for(DbIndustryArray::const_iterator ii = industries.constBegin(); ii != industries.constEnd(); ii++)
		if(!(*ii).m_name.isEmpty())
			bldnames.insert((*ii).m_name, true);

	// Print out the alphabetized lists of names.
	QTextStream out(stdout);
	//out << "Building Names\n";
	//out << "--------------\n";
	while(!bldnames.isEmpty())
	{
		out << bldnames.begin().key() << "\n";
		bldnames.erase(bldnames.begin());
	}
}

void PrintCommodities(const QString& dataPath)
{
	// Read the data file.
	DbCommodityArray commodities;
	DbIndustryArray industries;
	DgEmpireHash empires;
	if(!ReadDataFile(dataPath, commodities, industries, empires))
		return;

	// Alphabetize the commodity names.
	QMap<QString,bool> commodnames;
	for(DbCommodityArray::const_iterator ci = commodities.constBegin(); ci != commodities.constEnd(); ci++)
		if(!(*ci).m_name.isEmpty())
			commodnames.insert((*ci).m_name, true);

	// Print out the alphabetized lists of names.
	QTextStream out(stdout);
	//out << "Commodity Names\n";
	//out << "---------------\n";
	while(!commodnames.isEmpty())
	{
		out << commodnames.begin().key() << "\n";
		commodnames.erase(commodnames.begin());
	}
}

void PrintEmpires(const QString& dataPath)
{
	// Read the data file.
	DbCommodityArray commodities;
	DbIndustryArray industries;
	DgEmpireHash empires;
	if(!ReadDataFile(dataPath, commodities, industries, empires))
		return;

	// Alphabetize the empire names.
	QMap<QString,bool> empirenames;
	for(DgEmpireHash::const_iterator ei = empires.begin(); ei != empires.end(); ei++)
		if(!(*ei).second.Name().isEmpty())
			empirenames.insert((*ei).second.Name(), true);

	// Print out the alphabetized lists of names.
	QTextStream out(stdout);
	while(!empirenames.isEmpty())
	{
		out << empirenames.begin().key() << "\n";
		empirenames.erase(empirenames.begin());
	}
}

void PrintIdString(quint32 idVal)
{
	AuId id(idVal);
	QTextStream out(stdout);
	out << id.StringId() << "\n";
}

void PrintIdValue(const QString& stringId)
{
	AuId id(stringId);
	QTextStream out(stdout);
	out << id.GetID() << "\n";
}

void PrintRanges()
{
	QTextStream out(stdout);
	out << "Close\n";
	out << "Hail\n";
	out << "System\n";
	out << "Sector\n";
	out << "Galaxy\n";
	out << "Universe\n";
}

void PrintStandardChannels()
{
	QStringList channelNames;
	AuCommStdChannel::ListStandardChannels(channelNames, false);

	QTextStream out(stdout);
	for(QStringList::const_iterator ni = channelNames.constBegin(); ni != channelNames.constEnd(); ni++)
		out << (*ni) << " - " << AuCommStdChannel::ChannelDescription(*ni, AuCommStdChannel::Scope(*ni)) << "\n";
}

void PrintParameterTypes()
{
	QStringList parms = AuScript::ListParameterTypes();

	QTextStream out(stdout);
	for(QStringList::const_iterator pi = parms.constBegin(); pi != parms.constEnd(); pi++)
		out << (*pi) << "\n";
}

void PrintParameterTypesWiki()
{
	QStringList parms = AuScript::ListParameterTypesWiki();

	QTextStream out(stdout);
	for(QStringList::const_iterator pi = parms.constBegin(); pi != parms.constEnd(); pi++)
		out << (*pi) << "\n";
}

void PrintFunctions()
{
	QStringList protos = AuScript::ListFunctionPrototypes();

	QTextStream out(stdout);
	for(QStringList::const_iterator pi = protos.constBegin(); pi != protos.constEnd(); pi++)
		out << (*pi) << "\n\n";
}

void PrintFunctionsWiki()
{
	QStringList protos = AuScript::ListFunctionPrototypesWiki();

	QTextStream out(stdout);
	for(QStringList::const_iterator pi = protos.constBegin(); pi != protos.constEnd(); pi++)
		out << (*pi) << "\n\n";
}

#if !defined(QT_NO_DEBUG)
void DoMyTest()
{
	QElapsedTimer timer;

	/*
	double M[16] = {	1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1};
	M[0] = 1;	// Eliminates compiler warning.
	*/


	timer.start();
	AuMatrix4x4 myT;
	AuMatrix4x4 myoT;
	myoT.Translate(123,456,789);
	myoT.Rotate(AuPoint(12*DEG2RAD, 34*DEG2RAD, 56*DEG2RAD));
	myoT.Dump("myoT");
	AuMatrix4x4 myiT = myoT.Inverse();
	myiT.Dump("inverted");

	AuPoint myvec(123,456,789);
	AuPoint myvec2 = myoT.Project(myvec);
	myvec2.Dump("myvec2");
	AuPoint myvec3 = myoT.ProjectVector(myvec);
	myvec3.Dump("myvec3");
	AuPoint myvec4 = myoT.UnProjectVector(myvec);
	myvec4.Dump("myvec4");
	QVector3D myvec5;
	QVector3D myvec6(myvec.x, myvec.y, myvec.z);
	//int i;
	for(int it=0; it<10000000; it++)
	{
		//myoT.LoadIdentity();
		//myoT.LoadArray(M);
		//if(myoT == myT)
			//i = it;
		//myoT = myT.Inverse();
		//myoT.Translate(123,456,789);
		//myoT.Rotate(AuPoint(12*DEG2RAD, 34*DEG2RAD, 56*DEG2RAD));
		myoT = myT * myoT;

		//myoT = myT * myoT.Inverse();
		//myvec2 = myT.Project(myvec.x, myvec.y, myvec.z);
		//myvec2 = myoT.Project(myvec);
		//myvec5 = myT.map(myvec6);
	}
	int myElapsed = timer.elapsed();

	timer.start();
	QMatrix4x4 qT;
	QMatrix4x4 qoT;
	qoT.translate(123,456,789);
	qoT.rotate(12, 1,0,0);
	qoT.rotate(34, 0,1,0);
	qoT.rotate(56, 0,0,1);
	//qoT.rotate(QQuaternion(12, QVector3D(1,0,0)));
	//qoT.rotate(QQuaternion(34, QVector3D(0,1,0)));
	//qoT.rotate(QQuaternion(56, QVector3D(0,0,1)));
	qDebug() << qoT;
	bool invertible;
	QMatrix4x4 qiT = qoT.inverted(&invertible);
	qDebug() << "invertible" << invertible;
	qDebug() << "inverted" << qiT;

	QVector3D ovec(123,456,789);
	QVector3D ovec2 = qoT.map(ovec);
	qDebug() << "ovec2" << ovec2;
	QVector3D ovec3 = qoT.mapVector(ovec);
	qDebug() << "ovec3" << ovec3;
	QVector3D ovec4 = qiT.mapVector(ovec);
	qDebug() << "ovec4" << ovec4;
	for(int it=0; it<10000000; it++)
	{
		//qoT.setToIdentity();
		//qoT = qT.inverted();
		//qoT.translate(123,456,789);
		//qoT.rotate(12, 1,0,0);
		//qoT.rotate(34, 0,1,0);
		//qoT.rotate(56, 0,0,1);
		qoT = qT * qoT;
		//qoT = qT * qoT.inverted();
		//ovec2 = qT.map(ovec);
		//ovec2 = qoT.map(ovec);
	}
	int qElapsed = timer.elapsed();

	qDebug() << QString("myElapsed %1ms qElapsed %2ms").arg(myElapsed).arg(qElapsed);
}

void DoIntrinsics()
{
	QElapsedTimer timer;

	timer.start();
	AuMatrix4x4 myT;
	AuMatrix4x4 myoT;
	myoT.Translate(123,456,789);
	myoT.Rotate(AuPoint(12*DEG2RAD, 34*DEG2RAD, 56*DEG2RAD));
	myoT.Dump("myoT");
	AuPoint myvec(123,456,789);
	AuPoint myvec2 = myoT.Project(myvec);
	myvec2.Dump("myvec2");
	AuPoint myvec3 = myoT.ProjectVector(myvec);
	myvec3.Dump("myvec3");
	AuPoint myvec4 = myoT.UnProjectVector(myvec);
	myvec4.Dump("myvec4");
	QVector3D myvec5;
	QVector3D myvec6(myvec.x, myvec.y, myvec.z);
	for(int it=0; it<1000000; it++)
	{
		//myoT = myT.Inverse();
		myoT = myT * myoT;
		//myoT = myT * myoT.Inverse();
		//myvec2 = myT.Project(myvec.x, myvec.y, myvec.z);
		//myvec2 = myoT.Project(myvec);
		//myvec5 = myT.map(myvec6);
	}
	int myElapsed = timer.elapsed();

	timer.start();
	AuMatrix4x4 sseT;
	AuMatrix4x4 sseoT;
	sseoT.Translate(123,456,789);
	sseoT.Rotate(AuPoint(12*DEG2RAD, 34*DEG2RAD, 56*DEG2RAD));
	sseoT.Dump("sseoT");
	AuPoint ssevec(123,456,789);
	AuPoint ssevec2 = sseoT.Project(ssevec);
	ssevec2.Dump("ssevec2");
	AuPoint ssevec3 = sseoT.ProjectVector(ssevec);
	ssevec3.Dump("ssevec3");
	AuPoint ssevec4 = sseoT.UnProjectVector(ssevec);
	ssevec4.Dump("ssevec4");
	QVector3D ssevec5;
	QVector3D ssevec6(ssevec.x, ssevec.y, ssevec.z);
	for(int it=0; it<1000000; it++)
	{
		//sseoT = sseT.Inverse();
		sseoT = sseT * sseoT;
		//sseoT = sseT * sseoT.Inverse();
		//ssevec2 = sseT.Project(ssevec.x, ssevec.y, ssevec.z);
		//ssevec2 = sseoT.Project(ssevec);
		//ssevec5 = sseT.map(ssevec6);
	}
	int sseElapsed = timer.elapsed();

	qDebug() << QString("myElapsed %1ms sseElapsed %2ms").arg(myElapsed).arg(sseElapsed);
}

void DoTranslate()
{
	QMatrix4x4 t;
	t.translate(1,2,3);
	t.rotate(12, 1,0,0);
	t.rotate(34, 0,1,0);
	t.rotate(56, 0,0,1);
	t.translate(1,2,3);
	qDebug() << t;
}
#endif	// !QT_NO_DEBUG
