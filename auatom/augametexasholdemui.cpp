//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augametexasholdemui.cpp,v 1.25 2022/02/19 17:39:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augametexasholdemui.h"
//
#include "augametexasholdem.h"
#include "auglobal.h"
#include "aupushbutton.h"
#include "daperson.h"
#include <QCheckBox>
#include <QLabel>
#include <QLCDNumber>
#include <QPushButton>
#include <QSpinBox>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>

AuGameTexasHoldemSetup::AuGameTexasHoldemSetup(const AuGameTexasHoldem& game, QWidget* parent) : AuDialog(parent)
{
	// Restore my previous settings.
	quint8 backstyle = g_Au.Settings().readNumEntry("Game Table/Texas Hold Em Poker/BackStyle", (int)game.BackStyle());
	if(backstyle < AuGamePlayingCard::BackStyle0)
		backstyle = AuGamePlayingCard::BackStyle0;
	else if(backstyle > AuGamePlayingCard::BackStyle3)
		backstyle = AuGamePlayingCard::BackStyle3;
	quint32 bigblind = g_Au.Settings().readNumEntry("Game Table/Texas Hold Em Poker/BigBlind", (int)game.BigBlind());
	if(bigblind < 1)
		bigblind = 1;
	else if(bigblind > 50000000)
		bigblind = 50000000;
	quint32 littleblind = g_Au.Settings().readNumEntry("Game Table/Texas Hold Em Poker/LittleBlind", (int)game.LittleBlind());
	if(littleblind < 1)
		littleblind = 1;
	else if(littleblind > 50000000)
		littleblind = 50000000;

	setAuDlgIcon(QIcon(":data/GameTexasHoldem.png"));
	setAuDlgTitle(tr("<b>Texas Hold 'Em Poker Setup</b>"));

	// List the players in the game.
	// A kick button is added for everyone but the dealer.
	QGridLayout* layPlayers = new QGridLayout;
	int row=0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		m_PlayerIds[player] = game.PlayerId(player);
		if(!m_PlayerIds[player].IsNull())
		{
			QLabel* lbhead = new QLabel(this);
			lbhead->setPixmap(QPixmap(":data/Passenger.png"));
			layPlayers->addWidget(lbhead, row,0);

			m_PlayerNames[player] = new QLabel(game.PlayerName(player), this);
			layPlayers->addWidget(m_PlayerNames[player], row,1);

			if(player != 0)
			{
				// Allow dealer to kick any of the players who were automatically enrolled because they are sitting at the table.
				m_PlayerKick[player] = new AuPushButtonValue(player, this);
				m_PlayerKick[player]->setIcon(QIcon(":data/CommKick.png"));
				m_PlayerKick[player]->setToolTip(tr("Kicks the player from the game."));
				connect(m_PlayerKick[player], SIGNAL(ClickedValue(int)), SLOT(KickPlayer(int)));
				layPlayers->addWidget(m_PlayerKick[player], row,2);
			}
			else
				m_PlayerKick[player] = 0;		// No kick button for the dealer.
			layPlayers->setColumnStretch(3,1);	// Scrunch them over to the left.
			row++;
		}
		else
		{
			// No player in that slot.
			m_PlayerNames[player] = 0;
			m_PlayerKick[player] = 0;
		}
	}

	// Add a box to set the big blind.
	QLabel* lbbigblind = new QLabel(QObject::tr("Big Blind %1").arg(g_Au.CurrencySymbol()), this);

	quint32 moneyToPlay = bigblind * 50;						// This can exceed a signed integer.
	m_BigBlindLabel = new QLabel(QObject::tr("<html>Each player must have %L1%2 to play.</html>")
			.arg(moneyToPlay).arg(g_Au.CurrencySymbol()), this);

	m_BigBlind = new QSpinBox(this);
	m_BigBlind->setRange(1, 50000000);
	m_BigBlind->setValue(bigblind);
	m_BigBlind->setToolTip(tr("<html>The <i>big blind</i> bet is paid by the second player to the left of the dealer. "
				"In a two-player hand, the dealer pays the big blind. "
				"The raise increment in each betting round is the big blind amount or 2x the big blind amount, "
				"depending on the betting round. "
				"Players who do not have 50x the big blind in their personal account will be kicked when the hand is "
				"started.</html>"));
	connect(m_BigBlind, SIGNAL(valueChanged(int)), SLOT(BigBlindChanged(int)));

	// Add a box to set the little blind.
	QLabel* lblittleblind = new QLabel(QObject::tr("Little Blind %1").arg(g_Au.CurrencySymbol()), this);

	m_LittleBlind = new QSpinBox(this);
	m_LittleBlind->setRange(1, 50000000);
	m_LittleBlind->setValue(littleblind);
	m_LittleBlind->setToolTip(tr("<html>The amount of the <i>little blind</i> bet is usually 1/2 the amount of the big blind. "
				"The first player to the left of the dealer must pay the little blind.</html>"));

	QPushButton* pbdouble = new QPushButton(tr("Double Blinds"), this);
	connect(pbdouble, SIGNAL(clicked()), SLOT(DoubleBlinds()));
	pbdouble->setToolTip(tr("<html>Doubles the amount of the big blind and the little blind.</html>"));

	// Grab images of the backs out of the card palette.
	QPixmap cards(":data/PlayingCards.png");
	QPixmap card0(QSize(78,122));
	card0.fill(QColor(0,0,0,0));
	QPixmap card1(QSize(78,122));
	card1.fill(QColor(0,0,0,0));
	QPixmap card2(QSize(78,122));
	card2.fill(QColor(0,0,0,0));
	QPixmap card3(QSize(78,122));
	card3.fill(QColor(0,0,0,0));
	QBrush eraser(QColor(0,0,0,0));
	QPainter p(&card0);
	p.drawPixmap(0,0, 78,122, cards, AuGamePlayingCard::BackStyle0*78,0, 78,122);
	p.end();
	p.begin(&card1);
	p.drawPixmap(0,0, 78,122, cards, AuGamePlayingCard::BackStyle1*78,0, 78,122);
	p.end();
	p.begin(&card2);
	p.drawPixmap(0,0, 78,122, cards, AuGamePlayingCard::BackStyle2*78,0, 78,122);
	p.end();
	p.begin(&card3);
	p.drawPixmap(0,0, 78,122, cards, AuGamePlayingCard::BackStyle3*78,0, 78,122);
	p.end();

	// Back style selector.
	// There are only four back styles. Put all of them up there.
	m_BackStyle[0] = new QToolButton(this);
	m_BackStyle[0]->setAutoRaise(true);
	m_BackStyle[0]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
	m_BackStyle[0]->setIcon(QIcon(card0));
	if(backstyle == AuGamePlayingCard::BackStyle0)
		m_BackStyle[0]->setDown(true);
	connect(m_BackStyle[0], SIGNAL(clicked()), SLOT(BackStyleSelected0()));
	m_BackStyle[0]->setToolTip(tr("<html>Selects a style for the card deck that will be used.</html>"));

	m_BackStyle[1] = new QToolButton(this);
	m_BackStyle[1]->setAutoRaise(true);
	m_BackStyle[1]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
	m_BackStyle[1]->setIcon(QIcon(card1));
	if(backstyle == AuGamePlayingCard::BackStyle1)
		m_BackStyle[1]->setDown(true);
	connect(m_BackStyle[1], SIGNAL(clicked()), SLOT(BackStyleSelected1()));
	m_BackStyle[1]->setToolTip(tr("<html>Selects a style for the card deck that will be used.</html>"));

	m_BackStyle[2] = new QToolButton(this);
	m_BackStyle[2]->setAutoRaise(true);
	m_BackStyle[2]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
	m_BackStyle[2]->setIcon(QIcon(card2));
	if(backstyle == AuGamePlayingCard::BackStyle2)
		m_BackStyle[2]->setDown(true);
	connect(m_BackStyle[2], SIGNAL(clicked()), SLOT(BackStyleSelected2()));
	m_BackStyle[2]->setToolTip(tr("<html>Selects a style for the card deck that will be used.</html>"));

	m_BackStyle[3] = new QToolButton(this);
	m_BackStyle[3]->setAutoRaise(true);
	m_BackStyle[3]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
	m_BackStyle[3]->setIcon(QIcon(card3));
	if(backstyle == AuGamePlayingCard::BackStyle3)
		m_BackStyle[3]->setDown(true);
	connect(m_BackStyle[3], SIGNAL(clicked()), SLOT(BackStyleSelected3()));
	m_BackStyle[3]->setToolTip(tr("<html>Selects a style for the card deck that will be used.</html>"));

	m_Ok = new QPushButton(tr("Deal The Cards"), this);
	m_Ok->setToolTip(tr("<html>Starts the hand with the settings that are shown.</html>"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(StartGame()));

	// Create the layout of the settings dialog.
	QGridLayout* lay = new QGridLayout;
	row=0;
	lay->addLayout(layPlayers, row,0, 1,4);
	row++;

	// Add game controls.
	lay->setRowStretch(row++, 1);			// Make a stretchy row to crunch the names to the top when there's extra space.
	lay->addWidget(m_BigBlindLabel, row,0, 1,4);
	row++;
	lay->addWidget(lbbigblind, row,0, 1,2);
	lay->addWidget(m_BigBlind, row,2, 1,2);
	row++;
	lay->addWidget(lblittleblind, row,0, 1,2);
	lay->addWidget(m_LittleBlind, row,2, 1,2);
	row++;
	lay->addWidget(pbdouble, row,2, 1,2);
	row++;
	lay->addWidget(m_BackStyle[0], row,0);
	lay->addWidget(m_BackStyle[1], row,1);
	lay->addWidget(m_BackStyle[2], row,2);
	lay->addWidget(m_BackStyle[3], row,3);
	row++;

	// Add the Start Game button.
	lay->addWidget(m_Ok, row,0, 1,4);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameTexasHoldemSetup::~AuGameTexasHoldemSetup()
{
}

void AuGameTexasHoldemSetup::StartGame()
{
	// Collect and validate my settings from the widgets.
	int players = 0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(!m_PlayerIds[player].IsNull())
			players++;
	if(!players)
		return;		// Can't start a game with no players.

	qint32 bigblind = m_BigBlind->value();
	qint32 littleblind = m_LittleBlind->value();

	// Card back style;
	quint8 backstyle;
	if(m_BackStyle[0]->isDown())
		backstyle = AuGamePlayingCard::BackStyle0;
	else if(m_BackStyle[1]->isDown())
		backstyle = AuGamePlayingCard::BackStyle1;
	else if(m_BackStyle[2]->isDown())
		backstyle = AuGamePlayingCard::BackStyle2;
	else //if(m_BackStyle[3]->isDown())
		backstyle = AuGamePlayingCard::BackStyle3;

	// Pack my settings into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << backstyle;
	out << bigblind;
	out << littleblind;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		out << m_PlayerIds[player];

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameTexasHoldem::GameType(), ba);
	accept();

	// Save these settings for next time.
	g_Au.Settings().writeEntry("Game Table/Texas Hold Em Poker/BackStyle", backstyle);
	g_Au.Settings().writeEntry("Game Table/Texas Hold Em Poker/BigBlind", bigblind);
	g_Au.Settings().writeEntry("Game Table/Texas Hold Em Poker/LittleBlind", littleblind);
}

void AuGameTexasHoldemSetup::KickPlayer(int playerIndex)
{
	Q_ASSERT(playerIndex >= 1 && playerIndex < MAX_GAME_PLAYERS);
	m_PlayerIds[playerIndex] = AuId(0);
	m_PlayerNames[playerIndex]->hide();
	m_PlayerKick[playerIndex]->hide();
}

void AuGameTexasHoldemSetup::setVisible(bool visible)
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

void AuGameTexasHoldemSetup::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemSetup/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemSetup/PosY",-123);
	//QSize szdef = sizeHint();
	//int sizeX = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemSetup/SizeX",szdef.width());
	//int sizeY = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemSetup/SizeY",szdef.height());
	//resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameTexasHoldemSetup::SaveGeometry()
{
	QPoint ps = pos();
	//QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameTexasHoldemSetup/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameTexasHoldemSetup/PosY",ps.y());
	//g_Au.Settings().writeEntry("Widget/GameTexasHoldemSetup/SizeX",sz.width());
	//g_Au.Settings().writeEntry("Widget/GameTexasHoldemSetup/SizeY",sz.height());
}

void AuGameTexasHoldemSetup::BackStyleSelected0()
{
	m_BackStyle[0]->setDown(true);
	m_BackStyle[1]->setDown(false);
	m_BackStyle[2]->setDown(false);
	m_BackStyle[3]->setDown(false);
}

void AuGameTexasHoldemSetup::BackStyleSelected1()
{
	m_BackStyle[0]->setDown(false);
	m_BackStyle[1]->setDown(true);
	m_BackStyle[2]->setDown(false);
	m_BackStyle[3]->setDown(false);
}

void AuGameTexasHoldemSetup::BackStyleSelected2()
{
	m_BackStyle[0]->setDown(false);
	m_BackStyle[1]->setDown(false);
	m_BackStyle[2]->setDown(true);
	m_BackStyle[3]->setDown(false);
}

void AuGameTexasHoldemSetup::BackStyleSelected3()
{
	m_BackStyle[0]->setDown(false);
	m_BackStyle[1]->setDown(false);
	m_BackStyle[2]->setDown(false);
	m_BackStyle[3]->setDown(true);
}

void AuGameTexasHoldemSetup::DoubleBlinds()
{
	qint32 bigblind = m_BigBlind->value();
	bigblind *= 2;
	m_BigBlind->setValue(bigblind);

	qint32 littleblind = m_LittleBlind->value();
	littleblind *= 2;
	m_LittleBlind->setValue(littleblind);
}

void AuGameTexasHoldemSetup::BigBlindChanged(int bigblind)
{
	quint32 moneyToPlay = bigblind * 50;						// This can exceed a signed integer.
	m_BigBlindLabel->setText(QObject::tr("<html>Each player must have %L1%2 to play.</html>")
			.arg(moneyToPlay).arg(g_Au.CurrencySymbol()));
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameTexasHoldemTurn::AuGameTexasHoldemTurn(const AuGameTexasHoldem& game, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/GameTexasHoldem.png"));
	setAuDlgTitle(tr("<b>Texas Hold 'Em Poker Turn</b>"));
	setFocusPolicy(Qt::NoFocus);
	setObjectName(game.TurnUIName());

	int playerIndex = game.TurnIndex();
	QGridLayout* cardsLay = 0;
	QLabel* lbpot = 0;
	QLabel* lbcall = 0;
	QLabel* lbbet = 0;
	QPushButton* pbcall = 0;
	QPushButton* pbcheck = 0;
	QPushButton* pbfold = 0;
	QPushButton* pbraise = 0;
	m_Show = 0;
	QPushButton* pbmuck = 0;
	switch(game.TurnState())
	{
	case AuGameTexasHoldem::LittleBlindAnte:
		pbcall = new QPushButton(tr("Pay Little Blind %1")
				.arg(QString("%L1%2").arg((double)game.LittleBlind(),0,'f',0).arg(g_Au.CurrencySymbol())), this);
		pbcall->setToolTip(tr("<html>Pays the little blind bet. "
					"Money is transferred into the pot from your personal account.</html>"));
		connect(pbcall, SIGNAL(clicked()), SLOT(TurnCall()));
		pbcall->setFocusPolicy(Qt::NoFocus);

		pbfold = new QPushButton(tr("Fold"), this);
		pbfold->setToolTip(tr("<html>Quits the game. No money is paid.</html>"));
		pbfold->setFocusPolicy(Qt::NoFocus);
		connect(pbfold, SIGNAL(clicked()), SLOT(TurnFold()));
		break;

	case AuGameTexasHoldem::BigBlindAnte:
		lbpot = new QLabel(tr("Pot %1").arg(QString("%L1%2").arg((double)game.Pot(),0,'f',0).arg(g_Au.CurrencySymbol())), this);

		pbcall = new QPushButton(tr("Pay Big Blind %1")
				.arg(QString("%L1%2").arg((double)game.BigBlind(),0,'f',0).arg(g_Au.CurrencySymbol())), this);
		pbcall->setToolTip(tr("<html>Pays the big blind bet. "
					"Money is transferred into the pot from your personal account.</html>"));
		pbcall->setFocusPolicy(Qt::NoFocus);
		connect(pbcall, SIGNAL(clicked()), SLOT(TurnCall()));

		pbfold = new QPushButton(tr("Fold"), this);
		pbfold->setToolTip(tr("<html>Quits the game. No money is paid.</html>"));
		pbfold->setFocusPolicy(Qt::NoFocus);
		connect(pbfold, SIGNAL(clicked()), SLOT(TurnFold()));
		break;

	case AuGameTexasHoldem::PreflopBettingRound:
		// No 'Check' in preflop betting round.
		lbpot = new QLabel(tr("Pot %1").arg(QString("%L1%2").arg((double)game.Pot(),0,'f',0).arg(g_Au.CurrencySymbol())), this);

		lbbet = new QLabel(tr("Current Bet %1")
				.arg(QString("%L1%2").arg((double)game.Bet(game.TurnId()),0,'f',0).arg(g_Au.CurrencySymbol())), this);

		if(game.Bet(game.TurnId()) != game.CallBet())
		{
			lbcall = new QLabel(tr("Call Bet %1")
					.arg(QString("%L1%2").arg((double)game.CallBet(),0,'f',0).arg(g_Au.CurrencySymbol())), this);

			pbcall = new QPushButton(tr("Call"), this);
			pbcall->setToolTip(tr("<html>Raises your current bet enough to match the call bet. "
						"Money is transferred into the pot from your personal account.</html>"));
			pbcall->setFocusPolicy(Qt::NoFocus);
			connect(pbcall, SIGNAL(clicked()), SLOT(TurnCall()));

			pbraise = new QPushButton(tr("Raise %1").arg(QString("%L1%2").arg((double)game.BigBlind(),0,'f',0).arg(g_Au.CurrencySymbol())), this);
			pbraise->setToolTip(tr("<html>Calls and then raises your current bet by the amount of the big blind. "
						"Money is transferred into the pot from your personal account.</html>"));
			pbraise->setFocusPolicy(Qt::NoFocus);
			connect(pbraise, SIGNAL(clicked()), SLOT(TurnRaise()));
		}
		else
		{
			pbcheck = new QPushButton(tr("Check"), this);
			pbcheck->setToolTip(tr("<html>Leaves current bet unchanged.</html>"));
			pbcheck->setFocusPolicy(Qt::NoFocus);
			connect(pbcheck, SIGNAL(clicked()), SLOT(TurnCheck()));

			pbraise = new QPushButton(tr("Raise %1").arg(QString("%L1%2").arg((double)game.BigBlind(),0,'f',0).arg(g_Au.CurrencySymbol())), this);
			pbraise->setToolTip(tr("<html>Raises your current bet by the amount of the big blind. "
						"Money is transferred into the pot from your personal account.</html>"));
			pbraise->setFocusPolicy(Qt::NoFocus);
			connect(pbraise, SIGNAL(clicked()), SLOT(TurnRaise()));
		}

		pbfold = new QPushButton(tr("Fold"), this);
		pbfold->setToolTip(tr("<html>Quits the game. Your current bet is forfeit.</html>"));
		pbfold->setFocusPolicy(Qt::NoFocus);
		connect(pbfold, SIGNAL(clicked()), SLOT(TurnFold()));
		break;
	case AuGameTexasHoldem::PostflopBettingRound:
	case AuGameTexasHoldem::TurnBettingRound:
	case AuGameTexasHoldem::FinalBettingRound:
		lbpot = new QLabel(tr("Pot %1").arg(QString("%L1%2").arg((double)game.Pot(),0,'f',0).arg(g_Au.CurrencySymbol())), this);

		lbbet = new QLabel(tr("Current Bet %1")
				.arg(QString("%L1%2").arg((double)game.Bet(game.TurnId()),0,'f',0).arg(g_Au.CurrencySymbol())), this);

		if(game.Bet(game.TurnId()) != game.CallBet())
		{
			lbcall = new QLabel(tr("Call Bet %1")
					.arg(QString("%L1%2").arg((double)game.CallBet(),0,'f',0).arg(g_Au.CurrencySymbol())), this);

			pbcall = new QPushButton(tr("Call"), this);
			pbcall->setToolTip(tr("<html>Raises your current bet enough to match the call bet. "
						"Money is transferred into the pot from your personal account.</html>"));
			pbcall->setFocusPolicy(Qt::NoFocus);
			connect(pbcall, SIGNAL(clicked()), SLOT(TurnCall()));

			// Raise is equal to the big blind in the first two betting rounds.
			// Raise is doubled in the final two betting rounds.
			if(game.TurnState() == AuGameTexasHoldem::PostflopBettingRound)
			{
				pbraise =
					new QPushButton(tr("Raise %1").arg(QString("%L1%2").arg((double)game.BigBlind(),0,'f',0).arg(g_Au.CurrencySymbol())), this);
				pbraise->setToolTip(tr("<html>Calls and then raises your current bet by the amount of the big blind. "
							"Money is transferred into the pot from your personal account.</html>"));
			}
			else
			{
				pbraise =
					new QPushButton(tr("Raise %1").arg(QString("%L1%2").arg((double)game.BigBlind()*2,0,'f',0).arg(g_Au.CurrencySymbol())), this);
				pbraise->setToolTip(tr("<html>Calls and then raises your current bet by twice the amount of the big blind. "
							"Money is transferred into the pot from your personal account.</html>"));
			}
			pbraise->setFocusPolicy(Qt::NoFocus);
			connect(pbraise, SIGNAL(clicked()), SLOT(TurnRaise()));
		}
		else
		{
			pbcheck = new QPushButton(tr("Check"), this);
			pbcheck->setToolTip(tr("<html>Leaves current bet unchanged.</html>"));
			pbcheck->setFocusPolicy(Qt::NoFocus);
			connect(pbcheck, SIGNAL(clicked()), SLOT(TurnCheck()));

			// Raise is equal to the big blind in the first two betting rounds.
			// Raise is doubled in the final two betting rounds.
			if(game.TurnState() == AuGameTexasHoldem::PostflopBettingRound)
			{
				pbraise =
					new QPushButton(tr("Raise %1").arg(QString("%L1%2").arg((double)game.BigBlind(),0,'f',0).arg(g_Au.CurrencySymbol())), this);
				pbraise->setToolTip(tr("<html>Raises your current bet by the amount of the big blind. "
							"Money is transferred into the pot from your personal account.</html>"));
			}
			else
			{
				pbraise =
					new QPushButton(tr("Raise %1").arg(QString("%L1%2").arg((double)game.BigBlind()*2,0,'f',0).arg(g_Au.CurrencySymbol())), this);
				pbraise->setToolTip(tr("<html>Raises your current bet by twice the amount of the big blind. "
							"Money is transferred into the pot from your personal account.</html>"));
			}
			pbraise->setFocusPolicy(Qt::NoFocus);
			connect(pbraise, SIGNAL(clicked()), SLOT(TurnRaise()));
		}

		pbfold = new QPushButton(tr("Fold"), this);
		pbfold->setToolTip(tr("<html>Quits the game. Your current bet is forfeit.</html>"));
		pbfold->setFocusPolicy(Qt::NoFocus);
		connect(pbfold, SIGNAL(clicked()), SLOT(TurnFold()));
		break;

	case AuGameTexasHoldem::Showdown:
		lbpot = new QLabel(tr("Pot %1").arg(QString("%L1%2").arg((double)game.Pot(),0,'f',0).arg(g_Au.CurrencySymbol())), this);

		m_Show = new QPushButton(tr("Show"), this);
		m_Show->setToolTip(tr("<html>Shows the hand you have selected.</html>"));
		m_Show->setFocusPolicy(Qt::NoFocus);
		connect(m_Show, SIGNAL(clicked()), SLOT(Showdown()));

		pbmuck = new QPushButton(tr("Muck"), this);
		pbmuck->setToolTip(tr("<html>Quits the game without revealing your hand. Your current bet is forfeit.</html>"));
		pbmuck->setFocusPolicy(Qt::NoFocus);
		connect(pbmuck, SIGNAL(clicked()), SLOT(TurnFold()));

		cardsLay = new QGridLayout;
		{
			// Build a selection of cards to make a hand out of.
			QPixmap cards(":data/PlayingCards.png");
			QPixmap holecard1(QSize(78,122));
			holecard1.fill(QColor(0,0,0,0));
			QPixmap holecard2(QSize(78,122));
			holecard2.fill(QColor(0,0,0,0));
			QPixmap flopcard1(QSize(78,122));
			flopcard1.fill(QColor(0,0,0,0));
			QPixmap flopcard2(QSize(78,122));
			flopcard2.fill(QColor(0,0,0,0));
			QPixmap flopcard3(QSize(78,122));
			flopcard3.fill(QColor(0,0,0,0));
			QPixmap turncard(QSize(78,122));
			turncard.fill(QColor(0,0,0,0));
			QPixmap rivercard(QSize(78,122));
			rivercard.fill(QColor(0,0,0,0));

			QBrush eraser(QColor(0,0,0,0));
			QPainter p(&holecard1);
			p.drawPixmap(0,0, 78,122, cards, game.HoleCard1(playerIndex).Card()*78,game.HoleCard1(playerIndex).Suit()*122, 78,122);
			p.end();
			p.begin(&holecard2);
			p.drawPixmap(0,0, 78,122, cards, game.HoleCard2(playerIndex).Card()*78,game.HoleCard2(playerIndex).Suit()*122, 78,122);
			p.end();
			p.begin(&flopcard1);
			p.drawPixmap(0,0, 78,122, cards, game.FlopCard1().Card()*78,game.FlopCard1().Suit()*122, 78,122);
			p.end();
			p.begin(&flopcard2);
			p.drawPixmap(0,0, 78,122, cards, game.FlopCard2().Card()*78,game.FlopCard2().Suit()*122, 78,122);
			p.end();
			p.begin(&flopcard3);
			p.drawPixmap(0,0, 78,122, cards, game.FlopCard3().Card()*78,game.FlopCard3().Suit()*122, 78,122);
			p.end();
			p.begin(&turncard);
			p.drawPixmap(0,0, 78,122, cards, game.TurnCard().Card()*78,game.TurnCard().Suit()*122, 78,122);
			p.end();
			p.begin(&rivercard);
			p.drawPixmap(0,0, 78,122, cards, game.RiverCard().Card()*78,game.RiverCard().Suit()*122, 78,122);
			p.end();

			QToolButton*	tbcards[7];
			tbcards[0] = new QToolButton(this);
			tbcards[0]->setAutoRaise(true);
			tbcards[0]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[0]->setIcon(QIcon(holecard1));
			tbcards[0]->setToolTip(tr("<html>Selects the first hole card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[0], SIGNAL(clicked()), SLOT(HoleCard1Selected()));
			m_Cards[0] = new QCheckBox(this);
			connect(m_Cards[0], SIGNAL(clicked()), SLOT(EnableControls()));

			tbcards[1] = new QToolButton(this);
			tbcards[1]->setAutoRaise(true);
			tbcards[1]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[1]->setIcon(QIcon(holecard2));
			tbcards[1]->setToolTip(tr("<html>Selects the second hole card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[1], SIGNAL(clicked()), SLOT(HoleCard2Selected()));
			m_Cards[1] = new QCheckBox(this);
			connect(m_Cards[1], SIGNAL(clicked()), SLOT(EnableControls()));

			tbcards[2] = new QToolButton(this);
			tbcards[2]->setAutoRaise(true);
			tbcards[2]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[2]->setIcon(QIcon(flopcard1));
			tbcards[2]->setToolTip(tr("<html>Selects the first flop card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[2], SIGNAL(clicked()), SLOT(FlopCard1Selected()));
			m_Cards[2] = new QCheckBox(this);
			connect(m_Cards[2], SIGNAL(clicked()), SLOT(EnableControls()));

			tbcards[3] = new QToolButton(this);
			tbcards[3]->setAutoRaise(true);
			tbcards[3]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[3]->setIcon(QIcon(flopcard2));
			tbcards[3]->setToolTip(tr("<html>Selects the second flop card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[3], SIGNAL(clicked()), SLOT(FlopCard2Selected()));
			m_Cards[3] = new QCheckBox(this);
			connect(m_Cards[3], SIGNAL(clicked()), SLOT(EnableControls()));

			tbcards[4] = new QToolButton(this);
			tbcards[4]->setAutoRaise(true);
			tbcards[4]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[4]->setIcon(QIcon(flopcard3));
			tbcards[4]->setToolTip(tr("<html>Selects the third flop card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[4], SIGNAL(clicked()), SLOT(FlopCard3Selected()));
			m_Cards[4] = new QCheckBox(this);
			connect(m_Cards[4], SIGNAL(clicked()), SLOT(EnableControls()));

			tbcards[5] = new QToolButton(this);
			tbcards[5]->setAutoRaise(true);
			tbcards[5]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[5]->setIcon(QIcon(turncard));
			tbcards[5]->setToolTip(tr("<html>Selects the turn card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[5], SIGNAL(clicked()), SLOT(TurnCardSelected()));
			m_Cards[5] = new QCheckBox(this);
			connect(m_Cards[5], SIGNAL(clicked()), SLOT(EnableControls()));

			tbcards[6] = new QToolButton(this);
			tbcards[6]->setAutoRaise(true);
			tbcards[6]->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
			tbcards[6]->setIcon(QIcon(rivercard));
			tbcards[6]->setToolTip(tr("<html>Selects the river card for your final hand. "
						"You must select five cards to form the best poker hand.</html>"));
			connect(tbcards[6], SIGNAL(clicked()), SLOT(RiverCardSelected()));
			m_Cards[6] = new QCheckBox(this);
			connect(m_Cards[6], SIGNAL(clicked()), SLOT(EnableControls()));

			// Add the card buttons to the hbox layout.
			for(int card=0; card<7; card++)
			{
				cardsLay->addWidget(m_Cards[card], 0,card);	// Check boxes at the top left of each card.
				cardsLay->addWidget(tbcards[card], 1,card);	// Cards buttons in a row
			}
		}
		break;

	case AuGameTexasHoldem::DealHoleCard1:
	case AuGameTexasHoldem::DealHoleCard2:
	case AuGameTexasHoldem::BurnFlopCard:
	case AuGameTexasHoldem::DealFlopCard1:
	case AuGameTexasHoldem::DealFlopCard2:
	case AuGameTexasHoldem::DealFlopCard3:
	case AuGameTexasHoldem::BurnTurnCard:
	case AuGameTexasHoldem::DealTurnCard:
	case AuGameTexasHoldem::BurnRiverCard:
	case AuGameTexasHoldem::DealRiverCard:
		Q_ASSERT(false);		// We should not have created a turn ui in this game state.
		break;
	}

	// Create the layout of the settings dialog.
	QVBoxLayout* lay = new QVBoxLayout;

	// Add game controls.
	if(lbpot)
		lay->addWidget(lbpot);
	if(lbcall)
		lay->addWidget(lbcall);
	if(lbbet)
		lay->addWidget(lbbet);
	if(cardsLay)
		lay->addLayout(cardsLay);
	if(pbraise)
		lay->addWidget(pbraise);
	if(pbcall)
		lay->addWidget(pbcall);
	if(pbcheck)
		lay->addWidget(pbcheck);
	if(pbfold)
		lay->addWidget(pbfold);
	if(m_Show)
		lay->addWidget(m_Show);
	if(pbmuck)
		lay->addWidget(pbmuck);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameTexasHoldemTurn::~AuGameTexasHoldemTurn()
{
}

void AuGameTexasHoldemTurn::setVisible(bool visible)
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

void AuGameTexasHoldemTurn::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemTurn/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemTurn/PosY",-123);
	//QSize szdef = sizeHint();
	//int sizeX = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemTurn/SizeX",szdef.width());
	//int sizeY = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemTurn/SizeY",szdef.height());
	//resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameTexasHoldemTurn::SaveGeometry()
{
	QPoint ps = pos();
	//QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameTexasHoldemTurn/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameTexasHoldemTurn/PosY",ps.y());
	//g_Au.Settings().writeEntry("Widget/GameTexasHoldemTurn/SizeX",sz.width());
	//g_Au.Settings().writeEntry("Widget/GameTexasHoldemTurn/SizeY",sz.height());
}

void AuGameTexasHoldemTurn::TurnCall()
{
	// Pack my data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)0;	// Call

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameTexasHoldem::GameType(), ba);
	accept();
}

void AuGameTexasHoldemTurn::TurnCheck()
{
	// Pack my data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)1;	// Check

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameTexasHoldem::GameType(), ba);
	accept();
}

void AuGameTexasHoldemTurn::TurnFold()
{
	// Pack my data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)7;	// Fold value must exceed 0-6 card index range for the final showdown hand

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameTexasHoldem::GameType(), ba);
	accept();
}

void AuGameTexasHoldemTurn::TurnRaise()
{
	// Pack my data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)2;	// Raise

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameTexasHoldem::GameType(), ba);
	accept();
}

void AuGameTexasHoldemTurn::HoleCard1Selected()
{
	m_Cards[0]->setCheckState(m_Cards[0]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(0);
	if(m_Cards[0]->checkState())
		m_FinalHandCards.append(0);
	EnableControls();
}

void AuGameTexasHoldemTurn::HoleCard2Selected()
{
	m_Cards[1]->setCheckState(m_Cards[1]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(1);
	if(m_Cards[1]->checkState())
		m_FinalHandCards.append(1);
	EnableControls();
}

void AuGameTexasHoldemTurn::FlopCard1Selected()
{
	m_Cards[2]->setCheckState(m_Cards[2]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(2);
	if(m_Cards[2]->checkState())
		m_FinalHandCards.append(2);
	EnableControls();
}

void AuGameTexasHoldemTurn::FlopCard2Selected()
{
	m_Cards[3]->setCheckState(m_Cards[3]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(3);
	if(m_Cards[3]->checkState())
		m_FinalHandCards.append(3);
	EnableControls();
}

void AuGameTexasHoldemTurn::FlopCard3Selected()
{
	m_Cards[4]->setCheckState(m_Cards[4]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(4);
	if(m_Cards[4]->checkState())
		m_FinalHandCards.append(4);
	EnableControls();
}

void AuGameTexasHoldemTurn::TurnCardSelected()
{
	m_Cards[5]->setCheckState(m_Cards[5]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(5);
	if(m_Cards[5]->checkState())
		m_FinalHandCards.append(5);
	EnableControls();
}

void AuGameTexasHoldemTurn::RiverCardSelected()
{
	m_Cards[6]->setCheckState(m_Cards[6]->checkState() ? Qt::Unchecked : Qt::Checked);
	m_FinalHandCards.removeAll(6);
	if(m_Cards[6]->checkState())
		m_FinalHandCards.append(6);
	EnableControls();
}

void AuGameTexasHoldemTurn::EnableControls()
{
	if(m_Show)
	{
		int countChecked = 0;
		for(int card=0; card<7; card++)
			if(m_Cards[card]->isChecked())
				countChecked++;
		m_Show->setEnabled(countChecked == 5);
	}
}

void AuGameTexasHoldemTurn::Showdown()
{
	// Make an array of the five cards that were picked.
	quint8 cardpicks[5];
	if(m_FinalHandCards.count() == 5)
	{
		// Send the cards in the order they were selected.
		for(int card=0; card<5; card++)
			cardpicks[card] = m_FinalHandCards[card];
	}
	else
	{
		int cpi = 0;
		for(int card=0; card<7; card++)
			if(m_Cards[card]->isChecked())
				cardpicks[cpi++] = card;
		if(cpi < 5)
			return;		// Not enough cards picked to make a hand.
	}

	// Pack my data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	for(int cpi=0; cpi<5; cpi++)
		out << cardpicks[cpi];

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameTexasHoldem::GameType(), ba);
	accept();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameTexasHoldemFinish::AuGameTexasHoldemFinish(const QByteArray& gameResult, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/GameTexasHoldem.png"));
	setAuDlgTitle(tr("<b>Texas Hold 'Em Poker Game Over</b>"));

	// Game result variables.
	double share;					// Each winner's share of the kitty.
	QStringList winnerNames;		// Names of the winner.
	AuGamePlayingCard finalHand[MAX_GAME_PLAYERS][5];	// The final hand held by each winner.

	// Deserialize the game result from the blob.
	quint8 winnerCount;
	QDataStream in(gameResult);
	in.setVersion(QDataStream::Qt_4_1);
	in >> share;
	in >> winnerCount;				// winner list
	QString name;
	for(int winner=0; winner<winnerCount; winner++)
	{
		in >> name;
		winnerNames.append(name);
		for(int card=0; card<5; card++)
			in >> finalHand[winner][card];
	}

	// Create the overall layout for the dialog.
	QVBoxLayout* lay = new QVBoxLayout;

	// Create widgets to display the result.
	if(winnerCount == 1)
	{
		if(finalHand[0][0].Suit() == AuGamePlayingCard::Joker)
		{
			// The game never made it to the final hand.
			QLabel* lb = new QLabel(tr("<html>%1 <i>WINS</i> %2 By Default<br><br>"
						"All other players left the game.</html>").arg(winnerNames.first())
					.arg(QString("%L1%2").arg((double)share,0,'f',0).arg(g_Au.CurrencySymbol())), this);
			lay->addWidget(lb);
		}
		else
		{
			// Get a description of the winning hand.
			AuGameTexasHoldem::POKERHAND hand;
			AuGamePlayingCard::CARD highCard;	// Highest card among the cards used to make the hand. 2 2 2 2 A = 2 not A
			AuGamePlayingCard::CARD highCard2;	// 2nd highest card for two pairs and full house.
			QMap<AuGamePlayingCard::CARD, int> kickers;	// Cards not used to make the hand.
			AuGameTexasHoldem::Hand(0, finalHand, hand, highCard, highCard2, kickers);

			QLabel* lb = new QLabel(tr("<html>%1 <i>WINS</i> %2 with %3</html>").arg(winnerNames.first())
					.arg(QString("%L1%2").arg((double)share,0,'f',0).arg(g_Au.CurrencySymbol()))
					.arg(AuGameTexasHoldem::HandName(hand, highCard, highCard2, kickers)), this);
			lay->addWidget(lb);

			// Show the winning hand.
			QHBoxLayout* cardsLay = new QHBoxLayout;
			QPixmap cards(":data/PlayingCards.png");
			QBrush eraser(QColor(0,0,0,0));
			for(int cardIndex=0; cardIndex<5; cardIndex++)
			{
				QPixmap card(QSize(78,122));
				card.fill(QColor(0,0,0,0));

				QPainter p(&card);
				p.drawPixmap(0,0, 78,122, cards, finalHand[0][cardIndex].Card()*78,finalHand[0][cardIndex].Suit()*122, 78,122);
				p.end();

				QToolButton* tb = new QToolButton(this);
				tb->setAutoRaise(true);
				tb->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
				tb->setIcon(QIcon(card));
				cardsLay->addWidget(tb);
			}
			lay->addLayout(cardsLay);
		}
	}
	else
	{
		QLabel* lb = new QLabel(tr("<html><i>TIE</i> Between %1 Players!</html>").arg(winnerNames.count()), this);
		lay->addWidget(lb);

		QLabel* lb2 = new QLabel(tr("<html>Each Player Wins %1</html>")
				.arg(QString("%L1%2").arg((double)share,0,'f',0).arg(g_Au.CurrencySymbol())), this);
		lay->addWidget(lb2);

		QPixmap cards(":data/PlayingCards.png");
		QBrush eraser(QColor(0,0,0,0));
		for(int winner=0; winner<winnerNames.count(); winner++)
		{
			// Get a description of the winning hand.
			AuGameTexasHoldem::POKERHAND hand;
			AuGamePlayingCard::CARD highCard;	// Highest card among the cards used to make the hand. 2 2 2 2 A = 2 not A
			AuGamePlayingCard::CARD highCard2;	// 2nd highest card for two pairs and full house.
			QMap<AuGamePlayingCard::CARD, int> kickers;	// Cards not used to make the hand.
			AuGameTexasHoldem::Hand(winner, finalHand, hand, highCard, highCard2, kickers);

			QLabel* lb = new QLabel(tr("<html><b>%1 has %2</b></html>").arg(winnerNames[winner])
					.arg(AuGameTexasHoldem::HandName(hand, highCard, highCard2, kickers)), this);
			lay->addWidget(lb);

			// Show the winning hand.
			QHBoxLayout* cardsLay = new QHBoxLayout;
			for(int cardIndex=0; cardIndex<5; cardIndex++)
			{
				QPixmap card(QSize(78,122));
				card.fill(QColor(0,0,0,0));

				QPainter p(&card);
				p.drawPixmap(0,0, 78,122, cards, finalHand[winner][cardIndex].Card()*78,finalHand[winner][cardIndex].Suit()*122,
						78,122);
				p.end();

				QToolButton* tb = new QToolButton(this);
				tb->setAutoRaise(true);
				tb->setIconSize(QSize(78,122));	// This is the size of the individual cards in the PlayingCards.png file.
				tb->setIcon(QIcon(card));
				cardsLay->addWidget(tb);
			}
			lay->addLayout(cardsLay);
		}
	}

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameTexasHoldemFinish::~AuGameTexasHoldemFinish()
{
}

void AuGameTexasHoldemFinish::setVisible(bool visible)
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

void AuGameTexasHoldemFinish::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemFinish/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemFinish/PosY",-123);
	//QSize szdef = sizeHint();
	//int sizeX = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemFinish/SizeX",szdef.width());
	//int sizeY = g_Au.Settings().readNumEntry("Widget/GameTexasHoldemFinish/SizeY",szdef.height());
	//resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameTexasHoldemFinish::SaveGeometry()
{
	QPoint ps = pos();
	//QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameTexasHoldemFinish/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameTexasHoldemFinish/PosY",ps.y());
	//g_Au.Settings().writeEntry("Widget/GameTexasHoldemFinish/SizeX",sz.width());
	//g_Au.Settings().writeEntry("Widget/GameTexasHoldemFinish/SizeY",sz.height());
}
