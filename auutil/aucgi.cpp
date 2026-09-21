//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucgi.cpp,v 1.10 2022/02/28 16:39:20 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucgi.h"
//

#define g_AppName						"Shores of Hazeron"

static const char* s_htmlFile = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
static const char* s_htmlHeader = "<html>\n";
static const char* s_htmlStyle = "<style>"
	"a"
	"{"
		"text-decoration: none;"
	"}"

	".button"
	"{"
		"border-style: solid;"
		"border-color: #000000;"
		"border-radius: 8px;"
		"background-color: #000000;"
		"background-image: url(https://hazeron.com/images/hazBlackbutton.png);"
		"background-size: cover;"
		"padding: 2px 4px;"
		"text-align: center;"
		"text-decoration: none;"
		"display: inline-block;"
		"font-size: 20px;"
	"}"

	".button:hover"
	"{"
		"-webkit-transition-duration: .4s;"
		"transition-duration: .4s;"
		"border-style: solid;"
		"border-color: #fffcb9 #a3772f #7a4f0c #fedc69;"
	"}"

	".exbutton"
	"{"
		"border-style: solid;"
		"border-color: #000000;"
		"border-radius: 8px;"
		"background-color: #000000;"
		"background-image: url(https://hazeron.com/images/hazBlackbutton.png);"
		"background-size: cover;"
		"padding: 2px 4px;"
		"text-align: left;"
		"text-decoration: none;"
		"display: inline-block;"
		"font-size: 20px;"
		"width: 960px;"
	"}"

	".exbutton:hover"
	"{"
		"-webkit-transition-duration: .4s;"
		"transition-duration: .4s;"
		"border-style: solid;"
		"border-color: #fffcb9 #a3772f #7a4f0c #fedc69;"
	"}"

	".title"
	"{"
		"color: #ffffff;"
		"font-size: 20px;"
		"font-weight: bold;"
		"text-align: center;"
	"}"

	".subtitle"
	"{"
		"color: #a0a0a0;"
		"font-size: 11px;"
		"font-weight: normal;"
		"text-align: center;"
	"}"

	".rightsub"
	"{"
		"font-size: 12px;"
		"font-weight: normal;"
		"text-align: right;"
	"}"

	".righttitle"
	"{"
		"color: #ffffff;"
		"font-size: 20px;"
		"font-weight: bold;"
		"text-align: right;"
	"}"

	".form"
	"{"
		"color: #a0a0a0;"
		"font-size: 20px;"
		"margin-left: 20px;"
	"}"

	".tip"
	"{"
		"font-size: 10px;"
		"font-style: italic;"
	"}"
"</style>\n";

static const char* s_bodyStart = "<body style=\"color: rgb(112, 127, 255); background-color: rgb(0, 0, 0); "
	"font-family: Helvetica, Arial, sans-serif;\" alink=\"#ffffff\" lang=\"EN-US\" link=\"#a0b3f9\" vlink=\"#a0a0a0\">\n"
	"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
	"<tbody>"
	"<tr>"
	"<td>"
	"<a href=\"https://Hazeron.com\" target=\"_top\">"
		"<img align=\"left\" title=\"Shores of Hazeron Home\" alt=\"Shores of Hazeron\" "
		"src=\"https://hazeron.com/images/HSSHazeronHeader.png\" style=\"border: 0px solid; width: 968px;\">"
	"</a>"
	"</td>"
	"<tr>"
	"</tbody>"
	"</table>"
	"<br>\n";

static const char* s_bodyStartPhp = "<body style=\"color: rgb(112, 127, 255); background-color: rgb(0, 0, 0); "
	"font-family: Helvetica, Arial, sans-serif;\" alink=\"#ffffff\" lang=\"EN-US\" link=\"#a0b3f9\" vlink=\"#a0a0a0\">\n"
	"<?php include(\"/home/webmaster/hazeron/sohpageheader.html\"); ?><br>\n";

static const char* s_htmlFooter = "</body></html>\n";

bool AuCGI::UpdateExchangeByName(const QDir& updir, const QString& blueprintTypeName)
{
	QFile exchFile(updir.filePath("Exchange.php"));
	if(!exchFile.open(QIODevice::WriteOnly))
		return false;

	QString titleBar = QString("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>"
			g_AppName " - %1 Blueprint Exchange</title></head>\n<link rel=\"shortcut icon\" "
			"href=\"https://hazeron.com/favicon.png\" type=\"image/png\">\n").arg(blueprintTypeName);
	QString pageTitle = QString("<div class=\"title\">%1 Blueprint Exchange</div><br>\n").arg(blueprintTypeName);

	QStringList nameFilters;
	nameFilters << "*#*.SoH.php";
	QStringList rowFiles = updir.entryList(nameFilters);
	QMultiMap<QString, QString> sortedFiles;
	for(QStringList::const_iterator ri = rowFiles.cbegin(); ri != rowFiles.cend(); ri++)
	{
		int ip = (*ri).indexOf('#');
		if(ip < 0)
			continue;
		QString name = (*ri).mid(ip+1).toUpper();
		sortedFiles.insert(name, *ri);
	}

	QTextStream so(&exchFile);
	so.setCodec("UTF-8");
	so << HtmlFile();
	so << HtmlHeader();
	so << HtmlStyle();
	so << titleBar;
	so << HtmlBodyStartPhp();
	so << pageTitle;

	so << "<table><tbody>\n";
	for(QMultiMap<QString, QString>::const_iterator si = sortedFiles.cbegin(); si != sortedFiles.cend(); si++)
		so << QString("<?php include(\"%1\"); ?>").arg(*si);
	so << "</tbody></table>\n";

	so << s_htmlFooter << "\n";
	exchFile.close();
	return true;
}

void AuCGI::FailUpdateExchange(const QString& blueprintTypeName)
{
	// This HTML header is required to be the first output to stdout from our
	// cgi program. The HTML header(s) must terminate with a blank line.
	printf("Content-type: text/html\n\n");

	// The following code simply constructs a web page.
	const char* titleBar = "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>" g_AppName 
		" - Exchange Update Failure</title></head>\n"
		"<link rel=\"shortcut icon\" href=\"https://hazeron.com/favicon.png\" type=\"image/png\">\n";

	const char* pageTitle = "<div class=\"title\">Exchange Update Failure</div><br>\n";

	puts(HtmlHeader());
	puts(HtmlStyle());
	puts(titleBar);
	puts(HtmlBodyStart());
	puts(pageTitle);

	QString msg = QString("<p>The server failed to update the <i>%1 Blueprint Exchange</i> page.<p>\n"
			"<p>Please try again later.<p>\n").arg(blueprintTypeName);
	puts(msg.toUtf8());

	puts(s_htmlFooter);
}

QString AuCGI::ExtractFieldValue(const QByteArray& raw, const char* field)
{
	// Extract field value.
	int fieldbegin = raw.indexOf(field);
	fieldbegin += (int)strlen(field) + 4;
	int fieldend = raw.indexOf("\r", fieldbegin);
	return QString(raw.mid(fieldbegin, fieldend-fieldbegin));
}

QString AuCGI::ExtractFileData(const QByteArray& raw, QByteArray& fileData)
{
	// Extract the name of the posted file.
	int ifilename = raw.indexOf("filename=\"");
	ifilename += (int)strlen("filename=\"");
	int ifileend = raw.indexOf("\"", ifilename);
	QString fileName = raw.mid(ifilename, ifileend-ifilename);

	// Locate the beginning of the file data.
	int idatastart = raw.indexOf("\r", ifileend+2) + 4;	// Position after the \r\n\r\n

	// Locate the end of the file data.
	int idataend = raw.size() - 1;
	bool foundEnd = false;
	for(; idataend > idatastart; idataend--)
	{
		if(raw[idataend] == '\r')
		{
			if(foundEnd)
				break;
			else
				foundEnd = true;	// The very end. We really want the second to last.
		}
	}

	// Extract the file data.
	fileData = raw.mid(idatastart, idataend-idatastart);
	return fileName;	// Return the original name of the posted file.
}

QByteArray AuCGI::ReadRawInput()
{
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	int contentLen = env.value("CONTENT_LENGTH", "").toInt();

	QFile sin;
	if(!sin.open(stdin, QIODevice::ReadOnly))
		return QByteArray();	// Can't open stdin!?

	// Read the blob from stdin.
	QDataStream ds(&sin);
	int lenvar = contentLen;
	QByteArray raw(lenvar, 0);
	char* readTarget = raw.data();
	while(lenvar)
	{
		int read = ds.readRawData(readTarget, lenvar);
		if(read > 0)
		{
			readTarget += read;
			lenvar -= read;
		}
		else if(read < 0)
			break;	// Can't read the stream?
	}
	return raw;
}

void AuCGI::Reply(const QString& pageTitle, const QString& pageText)
{
	// This HTML header is required to be the first output to stdout from our
	// cgi program. The HTML header(s) must terminate with a blank line.
	printf("Content-type: text/html\n\n");

	// The following code simply constructs a web page.
	QString titleBar, titleHead;
	if(pageTitle.isEmpty())
	{
		titleBar = QString("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>" g_AppName 
				"</title></head>\n<link rel=\"shortcut icon\" href=\"https://hazeron.com/favicon.png\" type=\"image/png\">\n");
	}
	else
	{
		titleBar = QString("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>" g_AppName 
				" - %1</title></head>\n<link rel=\"shortcut icon\" href=\"https://hazeron.com/favicon.png\" type=\"image/png\">\n")
			.arg(pageTitle);

		titleHead = QString("<div class=\"title\">%1</div><br>\n").arg(pageTitle);
	}

	//QString exchangeButton = QString("<a class=\"button\" href=\"https://hazeron.com/Exchange.php\">"
			//"<img src=\"https://hazeron.com/images/Exchange.png\" height=\"32px\"> Return to Exchange</a>");
	QString backButton = QString("<form><input type=\"button\" value=\"Ok\" onclick=\"history.back()\"></form>\n");

	puts(HtmlHeader());
	puts(HtmlStyle());
	puts(titleBar.toUtf8());
	puts(HtmlBodyStart());
	if(!titleHead.isEmpty())
		puts(titleHead.toUtf8());
	puts(pageText.toUtf8());
	puts(backButton.toUtf8());
	puts(s_htmlFooter);
}

const char* AuCGI::HtmlFile()
{
	return s_htmlFile;
}

const char* AuCGI::HtmlHeader()
{
	return s_htmlHeader;
}

const char* AuCGI::HtmlStyle()
{
	return s_htmlStyle;
}

const char* AuCGI::HtmlBodyStart()
{
	return s_bodyStart;
}

const char* AuCGI::HtmlBodyStartPhp()
{
	return s_bodyStartPhp;
}

const char* AuCGI::HtmlFooter()
{
	return s_htmlFooter;
}

bool AuCGI::CreateExchangeRecordFile(const QDir& updir, const QString& archName, const QString& stype, const QString& postFileName,
		const QString& designName, const QString& thumbFileName, const QString& reportFileName, const QString& notes)
{
	QString recFileName = QString("%1#%2.php").arg(archName).arg(postFileName);
	QFile recFile(updir.filePath(recFileName));
	if(!recFile.open(QIODevice::WriteOnly))
		return false;

	QTextStream so(&recFile);
	so.setCodec("UTF-8");
	so << "<tr>\n";
	so << "<td class=\"exbutton\">\n";

	so << QString("<a href=\"https://hazeron.com/Blueprint.Exchange/%1/%2\">").arg(stype).arg(reportFileName);
		so << "<table><tbody style=\"vertical-align: top;\">\n";
			so << "<tr>\n";

			so << "<td>\n";
			if(thumbFileName.isEmpty())
				so << QString("<img src=\"https://hazeron.com/images/BdNoImage.png\" width: 128px; style=\"float: left;\">");
			else
				so << QString("<img src=\"https://hazeron.com/Blueprint.Exchange/%1/%2\" width: 128px; style=\"float: left;\">")
					.arg(stype).arg(thumbFileName);
			so << "</td>\n";

			QDateTime now(QDateTime::currentDateTime());
			so << "<td  style=\"width: 100%\">\n";
			so << QString(
					"<div class=\"rightsub\"><small>%3</small></div>"
					"<big style=\"color: #ffffff\">%1</big>"
					"<br>by %2 on %5 at %6"
					"<br>"
					"<br>"
					"%4\n")
				.arg(designName).arg(archName).arg(postFileName).arg(notes)
				.arg(now.toString("ddd MMM d yyyy"))
				.arg(now.toString("h:mma t"));
			so << "</td>\n";

			so << "</tr>\n";
		so << "</tbody></table>\n";
	so << "</a>\n";

	so << "</td>\n";
	so << "</tr>\n";
	recFile.close();
	return true;
}
