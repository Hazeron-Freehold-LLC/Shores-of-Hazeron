//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.12 2024/03/25 18:58:54 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "hashpack.h"
#include <math.h>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QTextStream>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void CompressCgFiles(const QString& outputFileName);
void CompressGLSLFiles(const QString& outputFileName);
bool CompressFiles(const QStringList& nameFilters, const QString& outputFileName);
void PrintUsage();
void ViewContents(const QString& outputFileName);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int main(int argc, char** argv)
{
	int returnCode = 0;	// Good return;

	bool didSomething = false;
	QCoreApplication a(argc, argv);
	QStringList args = a.arguments();

	// Test for the command to create a hash archive out of the .cg files.
	int farg = args.indexOf("-cg");
	if(farg >= 0)
	{
		// We are commanded to compress .cg files into a hash archive.
		QString fileName("cg.har");		// Default hash archive name.
		if(farg+1 < args.count() && args[farg+1][0] != '-')
			fileName = args[farg+1];		// An alternate archive file name was provided.
		CompressCgFiles(fileName);
		didSomething = true;
	}

	if(!didSomething)
	{
		// Test for the command to create a hash archive out of the .glsl files.
		farg = args.indexOf("-glsl");
		if(farg >= 0)
		{
			// We are commanded to compress .cg files into a hash archive.
			QString fileName("glsl.har");		// Default hash archive name.
			if(farg+1 < args.count() && args[farg+1][0] != '-')
				fileName = args[farg+1];		// An alternate archive file name was provided.
			CompressGLSLFiles(fileName);
			didSomething = true;
		}
	}

	if(!didSomething && args.size() >= 4 && args[1] == "-c")
	{
		// Create hash archive.
		QString fileName = args[2];

		QStringList nameFilters;
		for(int ai=3; ai<args.size(); ai++)
		{
			QString fileToPack = args[ai];
			nameFilters << fileToPack;
		}

		if(!CompressFiles(nameFilters, fileName))
			returnCode = 1;	// error
		didSomething = true;
	}

	if(!didSomething && args.size() >= 3 && args[1] == "-v")
	{
		// View contents of hash archive.
		QString fileName = args[2];
		ViewContents(fileName);
		didSomething = true;
	}

	if(!didSomething)
	{
		farg = args.indexOf("-?");
		if(farg >= 0)
		{
			PrintUsage();
			didSomething = true;
		}
	}

	if(!didSomething)
		PrintUsage();

	return returnCode;
}

void PrintUsage()
{
	QTextStream out(stdout);
	out << "Qt Packer\n";
	out << " Usage: QtPacker [-cg [file]] [-glsl [file]] [-?] [-c file filters...] [-v file] [-x file]\n";
	out << "  -cg   Compresses all .cg files in the current folder.\n";
	out << "  -glsl Compresses all .v, .f, and .h files in the current folder.\n";
	out << "  -c    Creates 'file' by compressing files that match 'filters' .\n";
	out << "  -v    Views contents of 'file'.\n";
	out << "  -x    Extracts contents of 'file'.\n";
	out << "  file  Hash archive file name. Default is 'cg.har' or 'glsl.har' if unspecified.\n";
	out << "  filters File filters to compress into hash archive 'file', separated by spaces.\n";
	out << "  -?    Outputs this program usage information.\n";
}

void CompressCgFiles(const QString& outputFileName)
{
	QStringList nameFilters;
	nameFilters << "*.cg";
	CompressFiles(nameFilters, outputFileName);
}

void CompressGLSLFiles(const QString& outputFileName)
{
	QStringList nameFilters;
	nameFilters << "*.f";
	nameFilters << "*.h";
	nameFilters << "*.v";
	CompressFiles(nameFilters, outputFileName);
}

bool CompressFiles(const QStringList& nameFilters, const QString& outputFileName)
{
	bool returnCode = true;	// good return
	// Get a list of the files in the current folder.
	QDir curDir = QDir::current();
	curDir.setFilter(QDir::Files|QDir::Readable);
	curDir.setNameFilters(nameFilters);
	QStringList fileNames = curDir.entryList();

	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	// Load all of the matching files in the current folder into a hash table of compressed byte arrays.
	QHash<QString, QByteArray>	files;	// Key is file name with no path. Data is compressed file data.
	QHash<QString, unsigned int>	fileSizes;	// Key is file name with no path. Data is uncompressed file size.
	for(QStringList::const_iterator fni = fileNames.constBegin(); fni != fileNames.constEnd(); fni++)
	{
		QFile fp(*fni);
		if(fp.open(QIODevice::ReadOnly))
		{
			// Read the contents of the file.
			QByteArray data = fp.readAll();
			fp.close();

			if(!data.isEmpty())
			{
				QByteArray compdata = qCompress(data);
				files.insert(*fni, compdata);
				fileSizes.insert(*fni, data.size());
				so << QString("%1 %L2 bytes compressed to %L3 bytes. %4% compression.")
					.arg(*fni)
					.arg(data.size())
					.arg(compdata.size())
					.arg((1. - (double)compdata.size() / (double)data.size()) * 100, 0,'f',0) << '\n';
				so.flush();
			}
			else
				qWarning() << QString("warning: Empty file skipped.") << *fni;
		}
	}

	if(!files.isEmpty())
	{
		// Build a table of contents.
		QHash<QString, HashpackTocEntry> toc;	// Key is file name

		// Create the hash archive file.
		QFile harFile(curDir.filePath(outputFileName));
		if(harFile.open(QIODevice::WriteOnly))
		{
			QDataStream out(&harFile);
			out.setVersion(QDataStream::Qt_4_1);

			// Reserve space for the offset to the toc, once we know where it is.
			unsigned int tocOffset = 0;
			out << tocOffset;

			// Store each file's byte array and make toc entries as we go.
			for(QHash<QString, QByteArray>::const_iterator fi = files.cbegin(); fi != files.cend(); fi++)
			{
				QHash<QString, unsigned int>::const_iterator si = fileSizes.find(fi.key());
				toc.insert(fi.key(), HashpackTocEntry(out.device()->pos(), *si, (*fi).size()));	// Position in the file.
				out << *fi;
			}

			// Remember the toc position.
			tocOffset = out.device()->pos();

			// Store the toc into a byte array so we can compress it.
			QByteArray batoc;
			QDataStream stoc(&batoc, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			stoc << toc;

			// Compress the toc.
			QByteArray comptoc = qCompress(batoc);
			out << comptoc;

			// Store the final toc offset at the beginning of the file.
			out.device()->seek(0);
			out << tocOffset;

			harFile.close();
			so << QString("%1 created with %2 files.").arg(curDir.filePath(outputFileName)).arg(files.count()) << '\n';
		}
		else
		{
			returnCode = false;	// bad return
			qWarning() << "error; Could not open output file for writing" << harFile.fileName();
		}
	}
	else
		qWarning("warning: No files found in current folder.");
	return returnCode;
}

void ViewContents(const QString& outputFileName)
{
	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	QDir curDir = QDir::current();
	QFile harFile(curDir.filePath(outputFileName));
	if(harFile.open(QIODevice::ReadOnly))
	{
		QDataStream in(&harFile);
		in.setVersion(QDataStream::Qt_4_1);

		// Read the offset to the table of contents.
		unsigned int tocOffset = 0;
		in >> tocOffset;
		in.device()->seek(tocOffset);

		// Read the compressed toc.
		QByteArray comptoc;
		in >> comptoc;
		QByteArray batoc = qUncompress(comptoc);

		// Read the toc.
		QHash<QString, HashpackTocEntry> toc;	// Key is file name, value is offset in file.
		QDataStream stoc(batoc);
		stoc.setVersion(QDataStream::Qt_4_1);
		stoc >> toc;

		// Determine field widths.
		int namewid = 0;
		int szwid = 1;
		int cszwid = 1;
		QMap<QString, QString> sorted;
		for(QHash<QString, HashpackTocEntry>::const_iterator ti = toc.cbegin(); ti != toc.cend(); ti++)
		{
			sorted.insert(ti.key().toUpper(), ti.key());

			if(ti.key().length() > namewid)
				namewid = ti.key().length();

			int wid = (int)log10((*ti).UncompressedFileSize()) + 1;
			if(wid > szwid)
				szwid = wid;

			wid = (int)log10((*ti).CompressedFileSize()) + 1;
			if(wid > cszwid)
				cszwid = wid;
		}

		namewid += 2;				// Allow for a margin.
		szwid += (szwid-1)/3;		// Allow for commas.
		cszwid += (cszwid-1)/3 + 1;	// Allow for commas and a margin.

		for(QMap<QString, QString>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
		{
			QHash<QString, HashpackTocEntry>::const_iterator ti = toc.find(*si);
			if(ti == toc.cend())
				continue;
			double perc = (1. - (double)(*ti).CompressedFileSize()/(double)(*ti).UncompressedFileSize()) * 100;
			so << QString("%1 %L2 %L3 %4%\n")
				.arg(ti.key(), -namewid)
				.arg((*ti).UncompressedFileSize(), szwid)
				.arg((*ti).CompressedFileSize(), cszwid)
				.arg(perc, 0,'f',0);
		}
	}
}

