//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucompileroutput.cpp,v 1.2 2022/02/19 18:10:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucompileroutput.h"
//
#include "auglobal.h"
#include "austory.h"
#include <QIcon>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

AuCompilerOutput::AuCompilerOutput(const QString& file, const AuScript& script, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Script Compiler Output"));
	setAuDlgIcon(QIcon(":data/StoryOpen.png"));

	QString text = "<html>";
	text += tr("<b>%1</b><br>").arg(file);
	if(script.CompileResult() == 0)
	{
		text += tr("Story script compiled successfully.<br>");

		// Report warnings and other compiler messages.
		if(!script.CompilerOutput().isEmpty())
		{
			int fl = file.length();
			text += "<code><br>";
			for(QStringList::const_iterator li = script.CompilerOutput().cbegin(); li != script.CompilerOutput().cend(); li++)
			{
				QString l(*li);
				if(l.left(fl) == file)
					l = l.mid(fl + 1);	// Drop the file name and the colon.
				text += l + "<br>";
			}
			text += "</code>";
		}

		text += "<table><tbody>";

		if(script.Id())
			text += tr("<tr><td>Script</td><td><b>%1</b></td></tr>").arg(script.Id());

		text += tr("<tr><td>Title</td><td><b>%1</b></td></tr>").arg(script.Title());
		text += tr("<tr><td>Author</td><td><b>%1</b></td></tr>").arg(script.Author());
		text += tr("<tr><td>Brief</td><td><b>%1</b></td></tr>").arg(script.Brief());
		text += tr("<tr><td>Blocks</td><td><b>%1</b></td></tr>").arg(script.BlockCount());
		if(script.AutoStart())
			text += tr("<tr><td>Auto</td><td><b>Start</b></td></tr>");
		else if(script.AutoRepeat())
			text += tr("<tr><td>Auto</td><td><b>Repeat</b></td></tr>");
		else if(script.AutoRepeatUntilComplete())
			text += tr("<tr><td>Auto</td><td><b>Repeat Until Complete</b></td></tr>");
		if(script.Interval())
			text += tr("<tr><td>Restart Interval</td><td><b>%1 minutes</b></td></tr>").arg(script.Interval());

		text += "</tbody></table>";
	}
	else if(script.CompileResult() == 2)
	{
		text += QString("Compilation is disabled by #nocompile directive in the script.");
	}
	else
	{
		if(!script.CompilerOutput().isEmpty())
		{
			text += tr("Errors were reported.<br>");

			int fl = file.length();
			text += "<code><br>";
			for(QStringList::const_iterator li = script.CompilerOutput().cbegin(); li != script.CompilerOutput().cend(); li++)
			{
				QString l(*li);
				if(l.left(fl) == file)
					l = l.mid(fl + 1);	// Drop the file name and the colon.
				text += l + "<br>";
			}
			text += "</code>";
		}
	}
	text += "</html>";

	QTextBrowser* tb = new QTextBrowser;
	tb->setHtml(text);

	QPushButton* pbok = new QPushButton(tr("Attach Script to Avatar"), this);
	pbok->setDefault(true);
	pbok->setAutoDefault(true);
	pbok->setEnabled(script.CompileResult() == 0);
	pbok->setToolTip(tr("<hmtl>"
				"The story will start when any of its catalyst conditions are met. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(tb);
	lay->addLayout(blay);
	lay->setSpacing(2);
	lay->setMargin(4);
	setAuDlgLayout(lay);
}

AuCompilerOutput::~AuCompilerOutput()
{
}

void AuCompilerOutput::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScriptCompiler/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScriptCompiler/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScriptCompiler/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScriptCompiler/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCompilerOutput::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScriptCompiler/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScriptCompiler/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScriptCompiler/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScriptCompiler/SizeY",sz.height());
}

void AuCompilerOutput::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

