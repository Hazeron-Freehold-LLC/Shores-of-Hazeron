//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentplayers.cpp,v 1.14 2022/05/04 19:45:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentplayers.h"
//
#include "auagentteleport.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auplayerstable.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTcpSocket>

AuAgentPlayers::AuAgentPlayers(QWidget* parent) : QWidget(parent)
{
	// Players online list.
	m_PlayersLabel = new QLabel(tr("Players Online"));

	m_Players = new AuPlayersTable(true, this);
	m_Players->setToolTip(tr("List of players who are online."));
	connect(m_Players, SIGNAL(EmpireRequested(quint32)), auApp, SLOT(EmpireRequested(quint32)));
	connect(m_Players, SIGNAL(itemSelectionChanged()), SLOT(PlayerSelectionChanged()));
	connect(m_Players, SIGNAL(ItemRightClicked(QTableWidgetItem*)), SLOT(PlayerRightClicked(QTableWidgetItem*)));
	connect(m_Players->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), SLOT(SaveSettings()));

	QPushButton* pbrefreshplayer = new QPushButton("Refresh Players Online List");
	connect(pbrefreshplayer, SIGNAL(clicked()), SLOT(RefreshPlayersOnline()));
	pbrefreshplayer->setToolTip("<html>Requests the server to send you the current list of avatars who are on line. "
			"Avatars who are in a wormhole or respawning may not show up on the list.</html>");

	// Construct final overall layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(pbrefreshplayer, row,1, 1,3);
	row++;
	lay->addWidget(m_PlayersLabel, row,0, 1,4);
	row++;
	lay->addWidget(m_Players, row,0, 1,4);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	RestoreSettings();
}

AuAgentPlayers::~AuAgentPlayers()
{
}

void AuAgentPlayers::RefreshPlayersOnline()
{
	// Values we will get from the master server on successful players query.
	AuPlayerOnlineStatusVector players;
	auApp->QueryPlayersOnline(players);

	const DgData& gd = g_Au.LockDataForRead();
	bool isArchitect = gd.ClientPlayerIsArchitect();
	g_Au.UnlockData();

	// Refresh the contents of the table widget.
	m_Players->Update(players, isArchitect);
	int playersOnCount = m_Players->AvatarsOnline();
	QString playtext = QString("%1 Avatars Online").arg(playersOnCount);
	m_PlayersLabel->setText(playtext);
}

void AuAgentPlayers::PlayerSelectionChanged()
{
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(curItem)
	{
		QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
		AuId gotoId = AuId(item->data(Qt::UserRole).toInt());
		QString gotoName = item->text();
		emit AvatarSelected(gotoId, gotoName);
	}
}

void AuAgentPlayers::PlayerRightClicked(QTableWidgetItem* item)
{
	if(!item)
		return;

	// Our player's id and name are in column 0.
	QTableWidgetItem* item0 = m_Players->item(item->row(), 0);
	AuId gotoId = AuId(item0->data(Qt::UserRole).toInt());
	QString gotoName = item0->text();

	QMenu menu;

	QAction* openWebPage = menu.addAction(QIcon(":data/c_VacSuitHelmet.png"), tr("View Web Page for %1...").arg(gotoName));

	QAction* goToAvatar = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport to %1...").arg(gotoName));
	connect(goToAvatar, SIGNAL(triggered()), SLOT(GoToAvatar()));

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;	// Menu canceled.

	if(action == openWebPage)
	{
		QUrl url(QString("https://www.hazeron.com/EmpireStandings/p%1.php").arg(gotoId.StringId()));
		QDesktopServices::openUrl(url);
	}
}

void AuAgentPlayers::GoToAvatar()
{
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(curItem)
	{
		QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
		AuId gotoId = AuId(item->data(Qt::UserRole).toInt());
		if(gotoId.IsNull())
			return;
		QMessageBox mb(g_Au.ApplicationName(),
				tr("This action will teleport you to the destination atom.\n\n"
					"After 3 minutes, if the destination atom is not found, the agent teleport is redirected to the capital "
					"city of a randomly selected empire."),
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
		if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
			return;

		AuAgentTeleport::GoToDestination(gotoId, false, false);
		emit AgentActionComplete();
	}
}

void AuAgentPlayers::RestoreSettings()
{
	// Restore the sizes of the table widget's columns 1 through 4, not 0 which is the automatically sized column.
	m_Players->horizontalHeader()->blockSignals(true);
	for(int col=0; col<m_Players->columnCount(); col++)
	{
		if(col == 0)
			continue;	// skip the automatically sized column
		int colwid = g_Au.Settings().readNumEntry(QString("Widget/AgentPlayers/ColumnWid%1").arg(col),-123);
		if(colwid != -123)
			m_Players->setColumnWidth(col, colwid);
	}
	m_Players->horizontalHeader()->blockSignals(false);
}

void AuAgentPlayers::SaveSettings()
{
	// Save the sizes of the table widget's columns.
	for(int col=0; col<m_Players->columnCount(); col++)
		g_Au.Settings().writeEntry(QString("Widget/AgentPlayers/ColumnWid%1").arg(col), m_Players->columnWidth(col));
}

