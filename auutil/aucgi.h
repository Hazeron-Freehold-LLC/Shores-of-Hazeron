//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucgi.h,v 1.2 2022/02/19 18:42:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCGI_H
#define AuCGI_H

#include "auutil.h"
#include <QString>

class AUUTIL_DLL AuCGI
{
public:
	// fails if unable to create the file, something like '/home/webmaster/Blueprint.Exchange/Sc/Haxus#BasicDesign.SoH.php'.
	static bool CreateExchangeRecordFile(const QDir& updir, const QString& archName, const QString& stype,
			const QString& postFileName, const QString& designName, const QString& thumbFileName, const QString& reportFileName,
			const QString& notes);

	static QString ExtractFieldValue(const QByteArray& raw, const char* field);

	static QString ExtractFileData(const QByteArray& raw, QByteArray& fileData);

	// '<body...' and SoH page header
	static const char* HtmlBodyStart();

	// '<body...' and SoH page header php include
	static const char* HtmlBodyStartPhp();

	// '<!DOCTYPE HTML...'
	static const char* HtmlFile();

	// '</body></html>'
	static const char* HtmlFooter();

	// '<html>'
	static const char* HtmlHeader();

	// '<style>...</style>'
	static const char* HtmlStyle();

	// 'pageTitle' can be blank
	static void Reply(const QString& pageTitle, const QString& pageText);

	static void FailUpdateExchange(const QString& blueprintTypeName);

	static QByteArray ReadRawInput();

	// 'updir' is something like '/home/webmaster/Blueprint.Exchange/Sc'.
	// 'blueprintName' is something like 'Spacecraft'.
	static bool UpdateExchangeByName(const QDir& updir, const QString& blueprintTypeName);
};

#endif // AuCGI_H
