//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameselect.cpp,v 1.2 2022/02/19 18:10:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augameselect.h"
//
#include <augame.h>
#include <auglobal.h>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QVBoxLayout>

AuGameSelect::AuGameSelect(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Game.png"));
	setAuDlgTitle("<b>Choose a Game to Play</b>");
	m_GameType = AuGame::GameType();	// This is an invalid game type for a real game to play.

	m_GamesList = new QListWidget(this);
	m_GamesList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_GamesList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_GamesList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_GamesList->setSortingEnabled(true);
	connect(m_GamesList, SIGNAL(itemSelectionChanged()), SLOT(GameSelected()));
	connect(m_GamesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(GameDoubleClicked(QListWidgetItem*)));

	// List the games available.
	QList<quint8> gameTypes;
	AuGame::ListGameTypes(gameTypes);
	for(QList<quint8>::const_iterator gti = gameTypes.constBegin(); gti != gameTypes.constEnd(); gti++)
	{
		AuGame* game = AuGame::CreateGame(*gti);
		Q_ASSERT(game);
		QListWidgetItem* lw = new QListWidgetItem(game->Icon(), game->Name(), m_GamesList);
		lw->setData(Qt::UserRole, game->Type());
		delete game;
	}

	m_DescBrowser = new QTextBrowser;
	m_DescBrowser->setOpenExternalLinks(true);
	m_DescBrowser->setReadOnly(true);

	// Assemble the widgets into a splitter.
	QSplitter* split = new QSplitter;
	split->setOrientation(Qt::Vertical);
	split->addWidget(m_GamesList);
	split->addWidget(m_DescBrowser);

	m_Ok = new QPushButton(tr("Choose Game"), this);
	m_Ok->setToolTip(tr("<html>Chooses the game that is selected.<br>"
				"%1 Everyone seated at the table is considered to be a player in the new game.<br>"
				"%1 A new game cannot be started while another game is in progress at the table.<br>"
				"%1 The person who starts a game is the <i>dealer</i>. "
				"The dealer configures the game settings and starts the game when everyone is ready."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_Ok->setEnabled(false);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(split);
	lay->addWidget(m_Ok);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameSelect::~AuGameSelect()
{
}

void AuGameSelect::setVisible(bool visible)
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

void AuGameSelect::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameSelect/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameSelect/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GameSelect/SizeX", szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GameSelect/SizeY", szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameSelect::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameSelect/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameSelect/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GameSelect/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GameSelect/SizeY",sz.height());
}

void AuGameSelect::GameSelected()
{
	QItemSelectionModel* ism = m_GamesList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* item = (QListWidgetItem*)idxsel.internalPointer();
				GameSelected(item);
			}
		}
	}
}

void AuGameSelect::GameSelected(QListWidgetItem* item)
{
	Q_ASSERT(item);
	AuGame* game = AuGame::CreateGame(item->data(Qt::UserRole).toInt());
	Q_ASSERT(game);

	// Put the descriptive text and instruction into the text browser control.
	m_GameType = game->Type();
	m_DescBrowser->setText(game->Description());
	m_Ok->setEnabled(true);

	delete game;
}

void AuGameSelect::GameDoubleClicked(QListWidgetItem* item)
{
	Q_ASSERT(item);
	GameSelected(item);
	OkPressed();
}

void AuGameSelect::OkPressed()
{
	accept();
}
