//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommlog.cpp,v 1.6 2022/04/08 16:36:55 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommlog.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "autoolbutton.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QScrollBar>
#include <QTextEdit>

AuCommLog::AuCommLog(const QString& channel, const QString& channelIcon, const QString& logText, QWidget* parent) : AuDialog(parent)
{
	m_ChannelName = channel;

	setAuDlgTitle(tr("%1 Channel Transcript").arg(channel));
	setAuDlgIcon(QIcon(channelIcon));

	QLabel* lbico = new QLabel();
	lbico->setPixmap(QPixmap(channelIcon).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	AuIconButton* clearLog = new AuIconButton(":data/DeleteMail.png", this);
	clearLog->setToolTip(tr("<html><b>Clear Transcript...</b><br>"
				"Erases the transcript that has accumulated on this channel. "
				"Closes this window.</html>"));
	connect(clearLog, SIGNAL(clicked()), SLOT(ClearLog()));

	AuIconButton* home = new AuIconButton(":data/hazBlackSliderArrow.png", this);
	home->setToolTip(tr("Scroll to Beginning"));
	connect(home, SIGNAL(clicked()), SLOT(ScrollHome()));

	QPixmap pm(":data/hazBlackSliderArrow.png");
	QTransform pmrot;
	pmrot.rotate(180);
	pm = pm.transformed(pmrot, Qt::FastTransformation);
	AuIconButton* end = new AuIconButton(QIcon(pm), this);
	end->setToolTip(tr("Scroll to End"));
	connect(end, SIGNAL(clicked()), SLOT(ScrollEnd()));

	m_Log = new QTextEdit;
	m_Log->setReadOnly(true);
	m_Log->setText(logText);

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addWidget(lbico);
	vlay->addStretch();
	vlay->addWidget(home);
	vlay->addWidget(end);
	vlay->addStretch();
	vlay->addWidget(clearLog);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(vlay);
	lay->addWidget(m_Log);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCommLog::~AuCommLog()
{
}

void AuCommLog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuCommLog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CommLog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CommLog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CommLog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CommLog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCommLog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CommLog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CommLog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CommLog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CommLog/SizeY",sz.height());
}

void AuCommLog::ClearLog()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	QDir logDir(g_Au.CacheGame());
	QString logFileName = logDir.absoluteFilePath(QString("Transcript.%1.txt").arg(m_ChannelName));

	QString smsg = tr("<html>"
			"Confirm removal of transcript file <b>%1</b>. "
			"</html>").arg(logFileName);
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = pFrame->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	QFile::remove(logFileName);
	accept();
}

void AuCommLog::ScrollEnd()
{
	QScrollBar* vs = m_Log->verticalScrollBar();
	if(!vs)
		return;

	vs->setSliderPosition(vs->maximum());
}

void AuCommLog::ScrollHome()
{
	QScrollBar* vs = m_Log->verticalScrollBar();
	if(!vs)
		return;

	vs->setSliderPosition(vs->minimum());
}
