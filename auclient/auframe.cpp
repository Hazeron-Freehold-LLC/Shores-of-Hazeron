//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auframe.cpp,v 1.13 2022/02/19 18:10:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auframe.h"
//
#include "aublueprintratingdlg.h"
#include "audialog.h"
#include "auglobal.h"
#include "austeamworksui.h"
#include "auview.h"
#include <QDesktopServices>
#include <QMessageBox>

// Restores the last known visibility state from settings.
void AuFrameDock::RestoreVisibilityState()
{
	m_WasVisible = g_Au.Settings().readBoolEntry(QString("/Widget/MainFrame/%1Visible").arg(m_DockBar->objectName()), false);
}

// Saves the last known visibility state in settings.
void AuFrameDock::SaveVisibilityState() const
{
	g_Au.Settings().setValue(QString("/Widget/MainFrame/%1Visible").arg(m_DockBar->objectName()), IsDockBarVisible());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuFrame::AuFrame(QWidget *parent) : QMainWindow(parent)
{
}

AuFrame::~AuFrame()
{
}

int AuFrame::ExecDialog(AuDialog& dlg)
{
	// Release keyboard and mouse grab.
	AuView* view = View();
	if(view)
		view->SetModalDlgUp(true);

	// Show the dialog modally using its exec() function.
	int result = dlg.exec();

	// Reacquire keyboard and mouse grab.
	if(view)
		view->SetModalDlgUp(false);

	// Return the exec() result.
	return result;
}

int AuFrame::ExecMessageBox(QMessageBox& mb)
{
	// Release keyboard and mouse grab.
	AuView* view = View();
	if(view)
		view->SetModalDlgUp(true);

	// Show the dialog modally using its exec() function.
	int result = mb.exec();

	// Reacquire keyboard and mouse grab.
	if(view)
		view->SetModalDlgUp(false);

	// Return the exec() result.
	return result;
}

void AuFrame::DesignerExited()
{
}

void AuFrame::GameOver(quint8 gameType, QByteArray gameResult)
{
	Q_UNUSED(gameType);
	Q_UNUSED(gameResult);
}

void AuFrame::GameSelect(quint8 gameType, bool gameSelected)
{
	Q_UNUSED(gameType);
	Q_UNUSED(gameSelected);
}

void AuFrame::GameTurn(AuId personsTurn)
{
	Q_UNUSED(personsTurn);
}

void AuFrame::BlueprintRating(quint32 designId, QString designName, AuExchangeBlueprintRating rating)
{
	// Show the reviews on a modeless dialog.
	AuBlueprintRatingDlg* dlg = new AuBlueprintRatingDlg(designId, designName, rating, this);
	dlg->show();
}

void AuFrame::ShowHomePage()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com");
	QDesktopServices::openUrl(url);
}

void AuFrame::ShowWiki()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/wiki");
	QDesktopServices::openUrl(url);
}

void AuFrame::ShowWikiTopic(const QString& topic)
{
	QUrl url(QString("https://www.hazeron.com/wiki/index.php/%1").arg(topic));
    QDesktopServices::openUrl(url);
}

bool AuFrame::IsCommChannelOpen(const QString& channelName) const
{
	QStringList channels;
	ListOpenChannels(channels);
	return channels.contains(channelName);
}

void AuFrame::Steamworks()
{
	AuSteamworksUI dlg;
	ExecDialog(dlg);
}

