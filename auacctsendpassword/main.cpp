//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.18 2022/04/08 15:16:42 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "auutil.h"
#include "aulogincheck.h"
#include <iostream>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QVariant>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Commonly used HTML fragments.
static const char* s_htmlHeader = "<html>";
static const char* s_bodyStart = "<body style=\"color: rgb(112, 127, 255); background-color: rgb(0, 0, 0); font-family: Helvetica, Arial, sans-serif;\" alink=\"#ffffff\" lang=\"EN-US\" link=\"#a0b3f9\" vlink=\"#a0a0a0\">"

// This part is the contents of sohpageheader.html. The php include did not work here. Hm.
"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
"<tbody>"
"<tr>"
"<td>"
"<a href=\"https://Hazeron.com\" target=\"_top\"><img align=\"left\" title=\"Shores of Hazeron Home\" alt=\"Shores of Hazeron\" src=\"https://hazeron.com/images/HSSPageHeader.png\" style=\"border: 0px solid; width: 1000px;\"></a>"
"</td>"
"<tr>"
"</tbody>"
"</table><br>";

static const char* s_htmlFooter = "</body></html>\n";

// Function Prototypes
void GoodReply(const QString& acctname);
void SendPassword(const QString& acctname);

int main(int argc, char** argv)
{
	QCoreApplication a(argc, argv);
	if(argc > 1)
	{
		QStringList args = a.arguments();
		if(args.contains("-version") || args.contains("-v"))
		{
			// Display version information and then exit.
			std::cout << "AuAcctSendPassword " << (const char*)AuUtil::VersionFull().toLatin1() << '\n';
			exit(0);
		}
	}

	// Read the POST data from standard input.
	QTextStream stream(stdin);
	QString line;
	line = stream.readLine();
	if(!line.isEmpty())
	{
		// Parse the line into "a=b" fields. They are delimited with ampersands.
		// e.g. "a=b&name=Bob%20Jones&state=CO"
		QString acctname;
		QStringList fields = line.split('&');
		for(QStringList::const_iterator fit = fields.constBegin();
				fit != fields.constEnd(); fit++)
		{
			// Parse the "a=b" field into two values.
			QStringList fieldVals = (*fit).split('=');
			if(fieldVals.count() == 2)
			{
				QString name = fieldVals[0];	// field name
				QString val = fieldVals[1];		// field value
				if(!val.isEmpty())
				{
					QString parsval = val.replace("+", " ");
					QRegExp rexp("[%%][0-9a-fA-F][0-9a-fA-F]");
					val = "";
					do
					{
						int idx = parsval.indexOf(rexp);
						if(idx >= 0)
						{
							// An encoded char was found in the parse string.
							// Grab the field leading up to the encoded char.
							if(idx)
								val += parsval.left(idx);

							// Decode the hex encoded character.
							int dig1val;
							char dig = parsval[idx+1].toLatin1();
							if(dig >= 'a')
							{
								dig1val = dig - 'a' + 10;
							}
							else if(dig >= 'A')
							{
								dig1val = dig - 'A' + 10;
							}
							else
							{
								dig1val = dig - '0';
							}

							int dig0val;
							dig = parsval[idx+2].toLatin1();
							if(dig >= 'a')
							{
								dig0val = dig - 'a' + 10;
							}
							else if(dig >= 'A')
							{
								dig0val = dig - 'A' + 10;
							}
							else
							{
								dig0val = dig - '0';
							}

							// Append the decoded character.
							dig = dig1val * 16 + dig0val;
							val += QChar(dig);			// Decoded character
							
							// Remove the encoded char from the parse string.
							parsval = parsval.mid(idx+3);	// Drop encoded char
						}
						else
						{
							// No encoded chars were found in the remainder of
							// the parse string.
							val += parsval;			// Remainder of the field
							parsval = "";
						}
					}
					while(!parsval.isEmpty());
				}

				// Collect field values.
				if(name == "acctname")
					acctname = val.trimmed();
			}
		}

		// This guy will validate the field contents and spit out the
		// appropriate response.
		SendPassword(acctname);
	}

	return 0;
}

void SendPassword(const QString& acctname)
{
	// Login to the login server.
	QString loginName = "WebMaster.Hazeron.com";

	// Create a SHA-1 hash key from the webmaster's password.
	QByteArray loginPassCrypt;
	loginPassCrypt.append(QString("web*master").toUtf8());
	QCryptographicHash loginPassCryptHash(QCryptographicHash::Sha1);
	loginPassCryptHash.addData(loginPassCrypt);
	loginPassCrypt = loginPassCryptHash.result();

	AuLoginCheck login;
	int result = login.LoginTestCredentials(loginName, loginPassCrypt);
	if(!result)
	{
		// Tell the server to reset the password.
		login.ResetPassword(acctname);

		// Send a good reply whether we actually mailed a password or
		// not so this tool can't be used to probe the existence of
		// account names.
		GoodReply(acctname);
	}
	else
	{
		qWarning("SendPassword failed to login to Login Server.");
	}
}

void GoodReply(const QString& acctname)
{
	// This HTML header is required to be the first output to stdout from our
	// cgi program. The HTML header(s) must terminate with a blank line.
	printf("Content-type: text/html\n\n");

	// The following code simply constructs a web page.
	const char* titleBar = "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>Shores of Hazeron - Password Reset</title><meta content=\"Haxus\" name=\"author\"></head>\n<link rel=\"shortcut icon\" href=\"https://hazeron.com/favicon.png\" type=\"image/png\">\n";
	const char* pageTitle = "<div style=\"text-align: center;\"><big style=\"font-weight: bold;\">Password Reset</big></div><br>\n";

	//printf(s_htmlHeader);
	//printf(titleBar);
	//printf(s_bodyStart);
	//printf(pageTitle);
	puts(s_htmlHeader);
	puts(titleBar);
	puts(s_bodyStart);
	puts(pageTitle);

	printf("The password has been reset. The new password was sent to the e-mail address on record for the login account.<p>\n");
	printf("<div style=\"margin-left: 40px;\">Account Name: <b>%s</b><br>",
			(const char*)acctname.toLatin1());

	//printf(s_htmlFooter);
	puts(s_htmlFooter);
}
