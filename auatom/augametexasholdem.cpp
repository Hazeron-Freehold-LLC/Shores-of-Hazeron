//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augametexasholdem.cpp,v 1.60 2022/02/19 17:39:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augametexasholdem.h"
//
#include "aucommpacket.h"
#include "augametexasholdemui.h"
#include "auglobal.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include <QIcon>
#include <QGridLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QSlider>

AuGameTexasHoldem::AuGameTexasHoldem()
{
	m_BackStyle = AuGamePlayingCard::BackStyle0;
	m_BigBlind = 20;
	m_LittleBlind = 10;
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		m_Pot[playerIndex] = 0;
		m_RaisesThisRound[playerIndex] = 0;
	}
	m_SpriteTurnState = LittleBlindAnte;
	m_TurnState = LittleBlindAnte;
}

AuGameTexasHoldem::~AuGameTexasHoldem()
{
}

AuGameTexasHoldem::AuGameTexasHoldem(const AuGameTexasHoldem& c) : AuGame(c), m_Deck(c.m_Deck), m_Sprites(c.m_Sprites)
{
	m_BackStyle = c.m_BackStyle;
	m_BigBlind = c.m_BigBlind;
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		for(int card=0; card<5; card++)
			m_FinalHand[playerIndex][card] = c.m_FinalHand[playerIndex][card];
		m_HoleCard1[playerIndex] = c.m_HoleCard1[playerIndex];
		m_HoleCard2[playerIndex] = c.m_HoleCard2[playerIndex];
		m_Pot[playerIndex] = c.m_Pot[playerIndex];
		m_RaisesThisRound[playerIndex] = c.m_RaisesThisRound[playerIndex];
	}
	m_FlopCard1 = c.m_FlopCard1;
	m_FlopCard2 = c.m_FlopCard2;
	m_FlopCard3 = c.m_FlopCard3;
	m_LittleBlind = c.m_LittleBlind;
	m_RiverCard = c.m_RiverCard;
	m_SpriteTurnState = c.m_SpriteTurnState;
	m_TurnCard = c.m_TurnCard;
	m_TurnState = c.m_TurnState;
}

void AuGameTexasHoldem::operator<<(QDataStream& in)
{
	AuGame::operator<<(in);

	in >> m_BackStyle;
	in >> m_BigBlind;
	m_Deck.clear();
	quint8 cards;
	in >> cards;
	while(cards--)
	{
		AuGamePlayingCard card;
		in >> card;
		m_Deck.append(card);
	}
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		for(int card=0; card<5; card++)
			in >> m_FinalHand[playerIndex][card];
		in >> m_HoleCard1[playerIndex];
		in >> m_HoleCard2[playerIndex];
		in >> m_Pot[playerIndex];
		in >> m_RaisesThisRound[playerIndex];
	}
	in >> m_FlopCard1;
	in >> m_FlopCard2;
	in >> m_FlopCard3;
	in >> m_LittleBlind;
	in >> m_RiverCard;
	in >> m_TurnCard;
	in >> m_TurnState;
}

void AuGameTexasHoldem::operator>>(QDataStream& out) const
{
	AuGame::operator>>(out);

	out << m_BackStyle;
	out << m_BigBlind;
	out << (quint8)m_Deck.count();
	for(QList<AuGamePlayingCard>::const_iterator ci = m_Deck.constBegin(); ci != m_Deck.constEnd(); ci++)
		out << *ci;
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		for(int card=0; card<5; card++)
			out << m_FinalHand[playerIndex][card];
		out << m_HoleCard1[playerIndex];
		out << m_HoleCard2[playerIndex];
		out << m_Pot[playerIndex];
		out << m_RaisesThisRound[playerIndex];
	}
	out << m_FlopCard1;
	out << m_FlopCard2;
	out << m_FlopCard3;
	out << m_LittleBlind;
	out << m_RiverCard;
	out << m_TurnCard;
	out << m_TurnState;
}

void AuGameTexasHoldem::Rx(QDataStream& in)
{
	AuGame::Rx(in);

	in >> m_BackStyle;
	in >> m_BigBlind;
	m_Deck.clear();
	quint8 cards;
	in >> cards;
	while(cards--)
	{
		AuGamePlayingCard card;
		in >> card;
		m_Deck.append(card);
	}
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		for(int card=0; card<5; card++)
			in >> m_FinalHand[playerIndex][card];
		in >> m_HoleCard1[playerIndex];
		in >> m_HoleCard2[playerIndex];
		in >> m_Pot[playerIndex];
		in >> m_RaisesThisRound[playerIndex];
	}
	in >> m_FlopCard1;
	in >> m_FlopCard2;
	in >> m_FlopCard3;
	in >> m_LittleBlind;
	in >> m_RiverCard;
	in >> m_TurnCard;
	in >> m_TurnState;
}

void AuGameTexasHoldem::Tx(QDataStream& out, const DaPerson& sceneAtom) const
{
	AuGame::Tx(out, sceneAtom);

	out << m_BackStyle;
	out << m_BigBlind;
	out << (quint8)m_Deck.count();
	for(QList<AuGamePlayingCard>::const_iterator ci = m_Deck.constBegin(); ci != m_Deck.constEnd(); ci++)
		out << AuGamePlayingCard();		// Transmit bogus cards for the deck. The client doesn't need to know the cards there.
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		for(int card=0; card<5; card++)
			out << m_FinalHand[playerIndex][card];
		if(sceneAtom.Is(PlayerId(playerIndex)))
		{
			out << m_HoleCard1[playerIndex];
			out << m_HoleCard2[playerIndex];
		}
		else
		{
			out << AuGamePlayingCard();
			out << AuGamePlayingCard();
		}
		out << m_Pot[playerIndex];
		out << m_RaisesThisRound[playerIndex];
	}
	out << m_FlopCard1;
	out << m_FlopCard2;
	out << m_FlopCard3;
	out << m_LittleBlind;
	out << m_RiverCard;
	out << m_TurnCard;
	out << m_TurnState;
}

void AuGameTexasHoldem::Refresh(const AuGame* gameFromServer)
{
	AuGame::Refresh(gameFromServer);
	const AuGameTexasHoldem* game = dynamic_cast<const AuGameTexasHoldem*>(gameFromServer);
	Q_ASSERT(game);

	// Refresh everything except client render data.
	m_BackStyle = game->m_BackStyle;
	m_BigBlind = game->m_BigBlind;
	m_Deck = game->m_Deck;
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		for(int card=0; card<5; card++)
			m_FinalHand[playerIndex][card] = game->m_FinalHand[playerIndex][card];
		m_HoleCard1[playerIndex] = game->m_HoleCard1[playerIndex];
		m_HoleCard2[playerIndex] = game->m_HoleCard2[playerIndex];
		m_Pot[playerIndex] = game->m_Pot[playerIndex];
		m_RaisesThisRound[playerIndex] = game->m_RaisesThisRound[playerIndex];
	}
	m_FlopCard1 = game->m_FlopCard1;
	m_FlopCard2 = game->m_FlopCard2;
	m_FlopCard3 = game->m_FlopCard3;
	m_LittleBlind = game->m_LittleBlind;
	m_RiverCard = game->m_RiverCard;
	m_TurnCard = game->m_TurnCard;
	m_TurnState = game->m_TurnState;
}

AuGame* AuGameTexasHoldem::Clone() const
{
	return new AuGameTexasHoldem(*this);
}

QString AuGameTexasHoldem::Description() const
{
	return QObject::tr("<html><img src=\"qrc:data/GameTexasHoldem.png\"> <b>%1</b><br>"
			"A poker card game. "
			"Each player creates a poker hand from two hole cards and five community cards. "
			"Four betting rounds lead to a high-stakes poker.<br><br>"
			"<b>Rules</b><br>"
			"A game plays one hand of %1. "
			"The person who starts the game is the dealer for that hand. "
			"All other people at the table are enrolled as players, "
			"with the little and big blinds to the left of the dealer. "
			"The dealer should be rotated to the left with each new hand.<br><br>"
			"Big and little blinds must be in the range of %2 to %3 each. "
			"Each player must have 50x the big blind in their personal account to play the hand.<br><br>"
			"Players are limited to three raises per betting round. Each hand can have up to four betting rounds.<br><br>"
			"Two players are required to play a hand. %1 cannot be played solitaire.<br><br>"
			"A player folds if they leave their chair.<br><br>"
			"For the complete rules to %1, refer to "
			"<a style=\"color: rgb(51, 255, 51);\" href=\"http://www.pokerlistings.com/poker-rules-texas-holdem\">"
			"Texas Hold 'Em Rules and Game Play</a> by PokerListings.com."
			"</html>")
		.arg(Name())
		.arg(QString("%L1%2").arg(1.,0,'f',0).arg(g_Au.CurrencySymbol()))
		.arg(QString("%L1%2").arg(50000000.,0,'f',0).arg(g_Au.CurrencySymbol()));
}

QIcon AuGameTexasHoldem::Icon() const
{
	return QIcon(":data/GameTexasHoldem.png");
}

QString AuGameTexasHoldem::Name() const
{
	return QObject::tr("Texas Hold 'Em Poker");
}

// Returns a number that is unique to each derived game class.
quint8 AuGameTexasHoldem::Type() const
{
	return GameType();
}

AuDialog* AuGameTexasHoldem::CreateFinishUI(const QByteArray& gameResult, QWidget* parent) const
{
	// Deserialize the game result from the blob.
	double share;					// Each winner's share of the kitty.
	QDataStream in(gameResult);
	in.setVersion(QDataStream::Qt_4_1);
	in >> share;
	if(!share)
		return 0;	// The game ended before anyone put any money into the kitty. Don't show a result.
	return new AuGameTexasHoldemFinish(gameResult, parent);
}

AuDialog* AuGameTexasHoldem::CreateSettingsUI(const DaPerson& person, QWidget* parent) const
{
	Q_UNUSED(person);
	return new AuGameTexasHoldemSetup(*this, parent);
}

AuDialog* AuGameTexasHoldem::CreateTurnUI(const DaPerson& person, QWidget* parent) const
{
	Q_UNUSED(person);
	switch(m_TurnState)
	{
	case LittleBlindAnte:
	case BigBlindAnte:
	case PreflopBettingRound:
	case PostflopBettingRound:
	case TurnBettingRound:
	case FinalBettingRound:
	case Showdown:
		// These game turn states make a ui for the current player.
		return new AuGameTexasHoldemTurn(*this, parent);

	case DealHoleCard1:
	case DealHoleCard2:
	case BurnFlopCard:
	case DealFlopCard1:
	case DealFlopCard2:
	case DealFlopCard3:
	case BurnTurnCard:
	case DealTurnCard:
	case BurnRiverCard:
	case DealRiverCard:
		// These game turn states do not make a ui for the current player.
		break;
	}
	return 0;
}

QString AuGameTexasHoldem::TurnUIName() const
{
	// UI is probably not valid if turn state changes.
	return AuGame::TurnUIName() + QString(":%1").arg(m_TurnState);
}

bool AuGameTexasHoldem::Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings)
{
	// Deserialize the data in the game settings blob.
	QDataStream in(gameSettings);
	in.setVersion(QDataStream::Qt_4_1);

	// Card back style
	in >> m_BackStyle;
	if(m_BackStyle < AuGamePlayingCard::BackStyle0 || m_BackStyle > AuGamePlayingCard::BackStyle3)
		return false;						// Invalid card back style.

	// Big blind
	in >> m_BigBlind;
	if(m_BigBlind < 1 || m_BigBlind > 50000000)
		return false;						// Limited to 50M.

	// Little blind
	in >> m_LittleBlind;
	if(m_LittleBlind < 1 || m_LittleBlind > 50000000)
		return false;						// Limited to 50M.

	// The list of ids is the players not rejected from the game by the dealer.
	AuId playerId;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		in >> playerId;
		if(PlayerId(player).IsNull())
		{
			// Game data does not think there is a player at this position.
			players[player] = 0;
		}
		else if(!players[player] || !players[player]->Is(playerId) || PlayerId(player) != playerId ||
				players[player]->AccountBalance() < m_BigBlind*50)
		{
			// The player atom is no longer there.
			//  or
			// The player atom is not the person listed there in the game (not likely since we just updated it)
			//  or
			// The player id at that position in the game does not match the one read from the settings. Player probably kicked.
			//  or
			// The player doesn't have enough money to enter the game. 50x the big blind is needed to start.
			if(!player)
				return false;					// Player 0 left. The game is not valid.
			RemovePlayer(PlayerId(player));		// Remove the player from the game.
		}
	}

	// Do we still have enough players for a game?
	if(PlayerCount() < 2)
		return false;

	// Shuffle the deck.
	ShuffleDeck();

	// First turn state is the little blind ante.
	SelectNextPlayer();						// The next player after the dealer antes the little blind.

	// Base class sets the turning state.
	return AuGame::Start(players, gameSettings);
}

void AuGameTexasHoldem::ShuffleDeck()
{
	// Construct a standard 52-card deck.
	QList<AuGamePlayingCard> deck;
	for(int suit = AuGamePlayingCard::Clubs; suit <= AuGamePlayingCard::Spades; suit++)
		for(int card = AuGamePlayingCard::Ace; card <= AuGamePlayingCard::King; card++)
			deck.append(AuGamePlayingCard((AuGamePlayingCard::SUIT)suit, (AuGamePlayingCard::CARD)card));
	Q_ASSERT(deck.count() == 52);

	// Now shuffle those cards into our game deck by picking them randomly one-by-one.
	m_Deck.clear();
	while(deck.count() > 1)
		m_Deck.append(deck.takeAt(g_Au.Dice().Roll(1, deck.count(), -1)));	// There is some irony in using dice to do this.
	m_Deck.append(deck.takeFirst());
	Q_ASSERT(deck.isEmpty());
	Q_ASSERT(m_Deck.count() == 52);
}

// Returns 0=no change, 1=data changed, 2=game over; do finish
int AuGameTexasHoldem::Tick()
{
	// The base class does nothing so there is no reason to call it.
	// Do something based on my current game state.
	switch(State())
	{
	case Setup:
		// Do nothing while waiting for setup to be compled.
		return 0;

	case Turning:
		return DoTurn();

	case Processing:
		// We don't do anything in this state.
		Q_ASSERT(false);	// We don't actually use this state in this game.
		SetState(Fanfare);
		return 1;		// Game state changed.

	case Fanfare:
		// Show a graphical representation of the result on the client at this time.
		if(StateTime().elapsed() < 10000)
			return 0;	// No change. Waiting for fanfare to be complete.

		// This game is over.
		//qDebug() << "Fanfare complete. Game over";
		return 2;		// Game Over
	}

	// Game Over if we get this far. The state value must be invalid.
	qWarning() << Type() << Name() << "invalid game state" << State();
	return 2;
}

// Returns 0=no change, 1=data changed, 2=game over
int AuGameTexasHoldem::DoTurn()
{
	// Game is over when there's only one player left in the game.
	if(PlayerCount() < 2)
	{
		//qDebug() << "DoTurn players left" << PlayerCount();
		return 2;
	}

	switch(m_TurnState)
	{
	case LittleBlindAnte:
	case BigBlindAnte:
		// Server does nothing while waiting for the blinds to ante.
		return 0;

	case DealHoleCard1:
		// Server deals the first hole card to each player.
		//qDebug() << "DealHoleCard1";
		for(int player=0; player<MAX_GAME_PLAYERS; player++)
		{
			m_HoleCard1[player] = DealCard();
			//qDebug() << m_HoleCard1[player];
		}
		m_TurnState = DealHoleCard2;
		return 1;

	case DealHoleCard2:
		// Server deals the first hole card to each player.
		//qDebug() << "DealHoleCard2";
		for(int player=0; player<MAX_GAME_PLAYERS; player++)
		{
			m_HoleCard2[player] = DealCard();
			//qDebug() << m_HoleCard2[player];
		}

		// Preflop betting round starts with the player to the left of the big blind.
		m_TurnIndex = 0;					// Dealer
		SelectNextPlayer();					// Little blind has already bet the little blind amount.
		SelectNextPlayer();					// Big blind has already bet the big blind amount.
		SelectNextPlayer();					// First player in the preflop betting round.
		m_TurnState = PreflopBettingRound;
		return 1;

	case PreflopBettingRound:
	case PostflopBettingRound:
	case TurnBettingRound:
	case FinalBettingRound:
	case Showdown:
		// Server does nothing while waiting for the betting rounds and showdown round to be complete.
		return 0;

	case BurnFlopCard:
		//qDebug() << "BurnFlopCard";
		DealCard();
		m_TurnState = DealFlopCard1;
		return 1;

	case DealFlopCard1:
		//qDebug() << "DealFlopCard1";
		m_FlopCard1 = DealCard();
		m_TurnState = DealFlopCard2;
		return 1;

	case DealFlopCard2:
		//qDebug() << "DealFlopCard2";
		m_FlopCard2 = DealCard();
		m_TurnState = DealFlopCard3;
		return 1;

	case DealFlopCard3:
		//qDebug() << "DealFlopCard3";
		m_FlopCard3 = DealCard();
		m_TurnState = PostflopBettingRound;
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			m_RaisesThisRound[playerIndex] = 0;
		return 1;

	case BurnTurnCard:
		//qDebug() << "BurnTurnCard";
		DealCard();
		m_TurnState = DealTurnCard;
		return 1;

	case DealTurnCard:
		//qDebug() << "DealTurnCard";
		m_TurnCard = DealCard();
		m_TurnState = TurnBettingRound;
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			m_RaisesThisRound[playerIndex] = 0;
		return 1;

	case BurnRiverCard:
		//qDebug() << "BurnRiverCard";
		DealCard();
		m_TurnState = DealRiverCard;
		return 1;

	case DealRiverCard:
		//qDebug() << "DealRiverCard";
		m_RiverCard = DealCard();
		m_TurnState = FinalBettingRound;
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			m_RaisesThisRound[playerIndex] = 0;
		return 1;
	}

	// Game Over if we get this far. The state value must be invalid.
	qWarning() << Type() << Name() << "invalid turn state" << m_TurnState;
	return 2;
}

AuGamePlayingCard AuGameTexasHoldem::DealCard()
{
	return m_Deck.takeFirst();
}

bool AuGameTexasHoldem::TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn)
{
	//qDebug() << "AuGameTexasHoldem::TurnComplete" << person->Name() << "turnState" << m_TurnState;
	// Base class verifies that it is actually personId's turn.
	if(!AuGame::TurnComplete(person, players, gameTurn))
	{
		//qDebug() << "TurnComplete not their turn" << person->Name();
		// Announce the turn at voice range for other players and any audience present.
		// The id of the player is used for voice range distance calc.
		QString text = QObject::tr("%1, your last input was ignored. It is not your turn.").arg(person->Name());
		AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
				person->Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(msg);
		return false;					// Must not be personId's turn.
	}

	int playerIndex = PlayerIndex(person->Id());
	if(playerIndex < 0)
	{
		//qDebug() << "TurnComplete not in game" << person->Name();
		// Announce the turn at voice range for other players and any audience present.
		// The id of the player is used for voice range distance calc.
		QString text = QObject::tr("%1, your last input was ignored. You are not a player in this game.").arg(person->Name());
		AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
				person->Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(msg);
		return false;					// That person is not in this game.
	}

	// Deserialize the data in the game turn blob.
	quint8 ui8;	// 0=call, 1=check, 2=raise. Fold causes player to quit; turn is never completed then.
	QDataStream in(gameTurn);
	in.setVersion(QDataStream::Qt_4_1);
	in >> ui8;

	// Decide what to do based on the current turn state.
	switch(m_TurnState)
	{
	case LittleBlindAnte:
		if(ui8 == 0)
		{
			// Call
			if(person->AccountBalance() < m_LittleBlind)
			{
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("%1 can't afford to pay the %2 little blind bet.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)m_LittleBlind,0,'f',0).arg(g_Au.CurrencySymbol()));
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				return false;								// The person doesn't have the money.
			}
			person->DebitAccount(m_LittleBlind);		// Take the ante from their account.
			m_Pot[playerIndex] = m_LittleBlind;

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 pays the %2 little blind bet.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_LittleBlind,0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Cycle the turn index to the next player.
			SelectNextPlayer();
			m_TurnState = BigBlindAnte;						// Next turn is big blind ante.
		}
		else if(ui8 == 7)
		{
			// Fold
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 folds without paying the %2 little blind bet.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_LittleBlind,0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Remove the current player and cycle the turn index to the next player.
			if(!RemovePlayer(TurnId()))
			{
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("No players remain in the game.");
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				break;	// No players left in the game.
			}
		}
		else
		{
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1, your last input was ignored.").arg(person->Name());
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);
			return false;
		}
		break;

	case BigBlindAnte:
		if(ui8 == 0)
		{
			// Call
			if(person->AccountBalance() < m_BigBlind)
			{
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("%1 can't afford to pay the %2 big blind bet.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)m_BigBlind,0,'f',0).arg(g_Au.CurrencySymbol()));
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				return false;								// The person doesn't have the money.
			}
			person->DebitAccount(m_BigBlind);			// Take the ante from their account.
			m_Pot[playerIndex] = m_BigBlind;

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 pays the %2 big blind bet.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_BigBlind,0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Cycle the turn index to the player to the left of the dealer.
			m_TurnIndex = 0;								// The dealer.
			SelectNextPlayer();								// The person after the dealer.
			m_TurnState = DealHoleCard1;					// Next turn is the deal of the hole card.
		}
		else if(ui8 == 7)
		{
			// Fold
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 folds without paying the %2 big blind bet.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_BigBlind,0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Remove the current player and cycle the turn index to the next player.
			if(!RemovePlayer(TurnId()))
			{
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("No players remain in the game.");
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				break;	// No players left in the game.
			}
		}
		else
		{
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1, your last input was ignored.").arg(person->Name());
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);
			return false;
		}
		break;

	case PreflopBettingRound:
	case PostflopBettingRound:
	case TurnBettingRound:
	case FinalBettingRound:
		// 0=call, 1=check, 2=raise. Fold causes player to quit; turn is never completed then.
		if(ui8 == 0)
		{
			// Call
			// Determine how much the player needs to bet to call.
			quint32 callBet = CallBet() - m_Pot[playerIndex];
			if(person->AccountBalance() < callBet)
			{
				//qDebug() << "call account" << person->AccountBalance() << "callBet" << callBet;
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("%1 can't afford the %2 needed to call the bet.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)callBet,0,'f',0).arg(g_Au.CurrencySymbol()));
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				return false;								// The person doesn't have the money.
			}
			person->DebitAccount(callBet);				// Take the call bet from their account.
			m_Pot[playerIndex] += callBet;

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 calls; bet is now %2.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_Pot[playerIndex],0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Cycle the turn index to the next player.
			SelectNextPlayer();

			if(IsGameTurn())
			{
				//qDebug() << "IsGameTurn";
				// The game turn cycle has gone around the table.
				// Move to the next turn state if all bets are equal.
				if(AllBetsMatch())
				{
					// Announce the turn at voice range for other players and any audience present.
					// The id of the player is used for voice range distance calc.
					QString text;
					if(m_TurnState == PreflopBettingRound)
						text = QObject::tr("Preflop betting round is over. Dealing the flop.");
					else if(m_TurnState == PostflopBettingRound)
						text = QObject::tr("Postflop betting round is over. Dealing the turn.");
					else if(m_TurnState == TurnBettingRound)
						text = QObject::tr("Turn betting round is over. Dealing the river.");
					else if(m_TurnState == FinalBettingRound)
						text = QObject::tr("Final betting round is over. Showdown.");
					else
						text = QObject::tr("Betting round is over. Nobody knows happens next.");
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(),
							person->FindParentSector()->Id(), person->Galaxy(),
							AuCommStdChannel::Close, AuCommStdChannel::Audible);
					g_Au.SendAtomChat(msg);

					m_TurnState++;							// Increment to next turn state in sequence, whatever it is.
					if(m_TurnState == Showdown)
					{
						// Entering the showdown. The person who bet on the river card is the first person to show their cards.
						for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
							if(!PlayerId(playerIndex).IsNull() && m_RaisesThisRound[playerIndex])
							{
								m_TurnIndex = playerIndex-1;
								SelectNextPlayer();				// Should bump to the river better unless he left the game.
								break;
							}
						//qDebug() << "entering showdown, player to show first is" << m_TurnIndex;
					}
				}
			}
		}
		else if(ui8 == 1)
		{
			// Check
			// Determine how much the player needs to bet to call.
			if(CallBet() != m_Pot[playerIndex])
			{
				//qDebug() << "call bet" << CallBet() << "does not match pot" << m_Pot[playerIndex];
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("%1 can't check; current bet of %2 does not match call bet of %3.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)m_Pot[playerIndex],0,'f',0).arg(g_Au.CurrencySymbol()))
					.arg(QString("%L1%2").arg((double)CallBet(),0,'f',0).arg(g_Au.CurrencySymbol()));
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				return false;								// Can't check if bet is short.
			}

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 checks; bet stays at %2.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_Pot[playerIndex],0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Cycle the turn index to the next player.
			SelectNextPlayer();

			if(IsGameTurn())
			{
				//qDebug() << "IsGameTurn";
				// The game turn cycle has gone around the table.
				// Move to the next turn state if all bets are equal.
				if(AllBetsMatch())
				{
					// Announce the turn at voice range for other players and any audience present.
					// The id of the player is used for voice range distance calc.
					QString text;
					if(m_TurnState == PreflopBettingRound)
						text = QObject::tr("Preflop betting round is over. Dealing the flop.");
					else if(m_TurnState == PostflopBettingRound)
						text = QObject::tr("Postflop betting round is over. Dealing the turn.");
					else if(m_TurnState == TurnBettingRound)
						text = QObject::tr("Turn betting round is over. Dealing the river.");
					else if(m_TurnState == FinalBettingRound)
						text = QObject::tr("Final betting round is over. Showdown.");
					else
						text = QObject::tr("Betting round is over. Nobody knows happens next.");
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(),
							person->FindParentSector()->Id(), person->Galaxy(),
							AuCommStdChannel::Close, AuCommStdChannel::Audible);
					g_Au.SendAtomChat(msg);

					m_TurnState++;							// Increment to next turn state in sequence, whatever it is.
					if(m_TurnState == Showdown)
					{
						// Entering the showdown. The person who bet on the river card is the first person to show their cards.
						for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
							if(!PlayerId(playerIndex).IsNull() && m_RaisesThisRound[playerIndex])
							{
								m_TurnIndex = playerIndex-1;
								SelectNextPlayer();				// Should bump to the river better unless he left the game.
								break;
							}
						//qDebug() << "entering showdown, player to show first is" << m_TurnIndex;
					}
				}
			}
		}
		else if(ui8 == 2)
		{
			// Raise
			// Limit the number of raises in each round.
			if(m_RaisesThisRound[playerIndex] >= 3)
			{
				//qDebug() << "raises this round" << m_RaisesThisRound[playerIndex];
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("%1 has already raised 3 times in this betting round.").arg(person->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				return false;								// Limited to 3 raises in a round.
			}
			m_RaisesThisRound[playerIndex]++;

			// Determine how much the player needs to bet to raise.
			quint32 raiseBet = (CallBet() - m_Pot[playerIndex]);
			if(m_TurnState == PreflopBettingRound || m_TurnState == PostflopBettingRound)
				raiseBet += m_BigBlind;						// Raise is equal to the big blind in the first two betting rounds.
			else
				raiseBet += m_BigBlind*2;					// Raise is doubled in final two betting rounds.
			if(person->AccountBalance() < raiseBet)
			{
				//qDebug() << "raise account" << person->AccountBalance() << "raiseBet" << raiseBet;
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("%1 can't afford the %2 needed to raise the bet.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)raiseBet,0,'f',0).arg(g_Au.CurrencySymbol()));
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				return false;								// The person doesn't have the money.
			}
			person->DebitAccount(raiseBet);			// Take the raise bet from their account.
			m_Pot[playerIndex] += raiseBet;

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 raises the bet to %2.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_Pot[playerIndex],0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Cycle the turn index to the next player.
			SelectNextPlayer();

			if(IsGameTurn())
			{
				//qDebug() << "IsGameTurn";
				// The game turn cycle has gone around the table.
				// Move to the next turn state if all bets are equal.
				if(AllBetsMatch())
				{
					// Announce the turn at voice range for other players and any audience present.
					// The id of the player is used for voice range distance calc.
					QString text;
					if(m_TurnState == PreflopBettingRound)
						text = QObject::tr("Preflop betting round is over. Dealing the flop.");
					else if(m_TurnState == PostflopBettingRound)
						text = QObject::tr("Postflop betting round is over. Dealing the turn.");
					else if(m_TurnState == TurnBettingRound)
						text = QObject::tr("Turn betting round is over. Dealing the river.");
					else if(m_TurnState == FinalBettingRound)
						text = QObject::tr("Final betting round is over. Showdown.");
					else
						text = QObject::tr("Betting round is over. Nobody knows happens next.");
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(),
							person->FindParentSector()->Id(), person->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					g_Au.SendAtomChat(msg);

					m_TurnState++;							// Increment to next turn state in sequence, whatever it is.
					if(m_TurnState == Showdown)
					{
						// Entering the showdown. The person who bet on the river card is the first person to show their cards.
						for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
							if(!PlayerId(playerIndex).IsNull() && m_RaisesThisRound[playerIndex])
							{
								m_TurnIndex = playerIndex-1;
								SelectNextPlayer();				// Should bump to the river better unless he left the game.
								break;
							}
						//qDebug() << "entering showdown, player to show first is" << m_TurnIndex;
					}
				}
			}
		}
		else if(ui8 == 7)
		{
			// Fold
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 folds, forfeits %2 bet.").arg(person->Name())
				.arg(QString("%L1%2").arg((double)m_Pot[playerIndex],0,'f',0).arg(g_Au.CurrencySymbol()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Remove the current player and cycle the turn index to the next player.
			if(!RemovePlayer(TurnId()))
			{
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("No players remain in the game.");
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				break;	// No players left in the game.
			}

			if(IsGameTurn())
			{
				//qDebug() << "IsGameTurn";
				// The game turn cycle has gone around the table.
				// Move to the next turn state if all bets are equal.
				if(AllBetsMatch())
				{
					// Announce the turn at voice range for other players and any audience present.
					// The id of the player is used for voice range distance calc.
					QString text;
					if(m_TurnState == PreflopBettingRound)
						text = QObject::tr("Preflop betting round is over. Dealing the flop.");
					else if(m_TurnState == PostflopBettingRound)
						text = QObject::tr("Postflop betting round is over. Dealing the turn.");
					else if(m_TurnState == TurnBettingRound)
						text = QObject::tr("Turn betting round is over. Dealing the river.");
					else if(m_TurnState == FinalBettingRound)
						text = QObject::tr("Final betting round is over. Showdown.");
					else
						text = QObject::tr("Betting round is over. Nobody knows happens next.");
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(),
							person->FindParentSector()->Id(), person->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					g_Au.SendAtomChat(msg);

					m_TurnState++;							// Increment to next turn state in sequence, whatever it is.
					if(m_TurnState == Showdown)
					{
						// Entering the showdown. The person who bet on the river card is the first person to show their cards.
						for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
							if(!PlayerId(playerIndex).IsNull() && m_RaisesThisRound[playerIndex])
							{
								m_TurnIndex = playerIndex-1;
								SelectNextPlayer();				// Should bump to the river better unless he left the game.
								break;
							}
						//qDebug() << "entering showdown, player to show first is" << m_TurnIndex;
					}
				}
			}
		}
		else
		{
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1, your last input was ignored.").arg(person->Name());
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);
			return false;
		}
		break;

	case Showdown:
		// First five bytes are the indices of the cards selected for the final hand.
		// 0=hole card 1, 1=hole card 2, 2=flop card1, 3=flop card 2, 4=flop card 3, 5=turn card, 6=river card
		if(ui8 < 7)
		{
			// We already received the first card index in ui8. Get the remaining four.
			quint8 ci[5];
			ci[0] = ui8;
			in >> ci[1];
			in >> ci[2];
			in >> ci[3];
			in >> ci[4];

			if(ci[0] > 6 || ci[1] > 6 || ci[2] > 6 || ci[3] > 6 || ci[4] > 6)
				return false;	// Bogus data. Can't use the same card more than once. Cheater?
			else if(ci[0] == ci[1] || ci[0] == ci[2] || ci[0] == ci[3] || ci[0] == ci[4])
				return false;	// Bogus data. Can't use the same card more than once. Cheater?
			else if(ci[1] == ci[2] || ci[1] == ci[3] || ci[1] == ci[4])
				return false;	// Bogus data. Can't use the same card more than once. Cheater?
			else if(ci[2] == ci[3] || ci[2] == ci[4])
				return false;	// Bogus data. Can't use the same card more than once. Cheater?
			else if(ci[3] == ci[4])
				return false;	// Bogus data. Can't use the same card more than once. Cheater?

			for(int card=0; card<5; card++)
			{
				if(ci[card] == 0)
					m_FinalHand[playerIndex][card] = m_HoleCard1[playerIndex];
				else if(ci[card] == 1)
					m_FinalHand[playerIndex][card] = m_HoleCard2[playerIndex];
				else if(ci[card] == 2)
					m_FinalHand[playerIndex][card] = m_FlopCard1;
				else if(ci[card] == 3)
					m_FinalHand[playerIndex][card] = m_FlopCard2;
				else if(ci[card] == 4)
					m_FinalHand[playerIndex][card] = m_FlopCard3;
				else if(ci[card] == 5)
					m_FinalHand[playerIndex][card] = m_TurnCard;
				else if(ci[card] == 6)
					m_FinalHand[playerIndex][card] = m_RiverCard;
			}

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 shows %2.").arg(person->Name()).arg(HandName(playerIndex));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Cycle the turn index to the next player who is still in the game.
			SelectNextPlayer();

			// When all players have shown their hands. we'll move to the fanfare state to announce the winner.
			// We don't use the normal turn sequence. The showdown sequence started with the player who bet on the river card.
			int nextPlayerIndex = PlayerIndex(TurnId());
			if(m_FinalHand[nextPlayerIndex][0].Suit() != AuGamePlayingCard::Joker)
			{
				//qDebug() << "showdown complete, do Fanfare";
				SetState(Fanfare);		// We've gone full circle. All players have shown or mucked their hand. Announce the winner.
			}
		}
		else if(ui8 == 7)
		{
			// Fold
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1 mucks %2 hand.").arg(person->Name()).arg(person->HerHis(false));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);

			// Remove the current player and cycle the turn index to the next player.
			if(!RemovePlayer(TurnId()))
			{
				// Announce the turn at voice range for other players and any audience present.
				// The id of the player is used for voice range distance calc.
				QString text = QObject::tr("No players remain in the game.");
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
						person->Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				g_Au.SendAtomChat(msg);
				break;	// No players left in the game.
			}

			// When all players have shown their hands. we'll move to the fanfare state to announce the winner.
			// We don't use the normal turn sequence. The showdown sequence started with the player who bet on the river card.
			int nextPlayerIndex = PlayerIndex(TurnId());
			if(nextPlayerIndex >= 0 && m_FinalHand[nextPlayerIndex][0].Suit() != AuGamePlayingCard::Joker)
			{
				//qDebug() << "showdown complete, do Fanfare";
				SetState(Fanfare);		// We've gone full circle. All players have shown or mucked their hand. Announce the winner.
			}
		}
		else
		{
			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text = QObject::tr("%1, your last input was ignored.").arg(person->Name());
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);
			return false;
		}
		break;

	case DealHoleCard1:
	case DealHoleCard2:
	case BurnFlopCard:
	case DealFlopCard1:
	case DealFlopCard2:
	case DealFlopCard3:
	case BurnTurnCard:
	case DealTurnCard:
	case BurnRiverCard:
	case DealRiverCard:
		// Ignore during this turn state. Somebody pushed a button on a stale ui.
		break;
	}

	// Turn complete.
	return true;
}

void AuGameTexasHoldem::EvaluateWinner(QList<int>& winners) const
{
	if(PlayerCount() == 1)
	{
		// Only one player left; that person wins by default.
		winners.clear();
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			if(!PlayerId(playerIndex).IsNull())
			{
				//qDebug() << "EvaluateWinner only one player left" << PlayerName(playerIndex);
				winners.append(playerIndex);
				break;
			}
		return;
	}

	// Evaluate the hands of the remaining players.
	//qDebug() << "AuGameTexasHoldem::EvaluateWinner";
	POKERHAND hand[MAX_GAME_PLAYERS];
	AuGamePlayingCard::CARD highCard[MAX_GAME_PLAYERS];	// Highest card among the cards used to make the hand. 2 2 2 2 A = 2 not A
	AuGamePlayingCard::CARD highCard2[MAX_GAME_PLAYERS];	// Second card for two pairs and full house
	QMap<AuGamePlayingCard::CARD, int> kickers[MAX_GAME_PLAYERS];	// Cards not used to make the hand.
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		Hand(playerIndex, hand[playerIndex], highCard[playerIndex], highCard2[playerIndex], kickers[playerIndex]);
		//qDebug() << " playerIndex" << playerIndex <<
			//HandName(hand[playerIndex], highCard[playerIndex], highCard2[playerIndex], kickers[playerIndex]);
	}

	// We now know each player's hand and their high card and kickers, when applicable.
	// Check for the highest hand/card combination of all hands.
	POKERHAND highestHand = Invalid;
	AuGamePlayingCard::CARD highestCard = AuGamePlayingCard::Ace;	// Low ace
	AuGamePlayingCard::CARD highestCard2 = AuGamePlayingCard::Ace;	// Low ace
	QMap<AuGamePlayingCard::CARD, int> highestKickers;
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		if(hand[playerIndex] > highestHand)
		{
			// Player hand category is better than the best hand found so far.
			//if(highestHand != Invalid)
				//qDebug() << "hand" << hand[playerIndex] << "beats" << highestHand;
			highestHand = hand[playerIndex];
			highestCard = highCard[playerIndex];
			highestCard2 = highCard2[playerIndex];
			highestKickers = kickers[playerIndex];
		}
		else if(hand[playerIndex] == highestHand && highestHand != Invalid)
		{
			// Player hand category is the same as the best hand found so far.
			if(highCard[playerIndex] > highestCard)
			{
				// Player hand high card is better than the best hand found so far.
				//qDebug() << "high card" << highCard[playerIndex] << "beats" << highestCard;
				highestHand = hand[playerIndex];
				highestCard = highCard[playerIndex];
				highestCard2 = highCard2[playerIndex];
				highestKickers = kickers[playerIndex];
			}
			else if(highCard[playerIndex] == highestCard)
			{
				if(highCard2[playerIndex] > highestCard2)
				{
					// Player hand secondary high card is better than the best hand secondary high card found so far.
					// This only affects two pairs and full house.
					//qDebug() << "second high card" << highCard2[playerIndex] << "beats" << highestCard2;
					highestHand = hand[playerIndex];
					highestCard = highCard[playerIndex];
					highestCard2 = highCard2[playerIndex];
					highestKickers = kickers[playerIndex];
				}
				else if(!highestKickers.isEmpty())
				{
					// Player hand category and high card(s) are the same as the best hand found so far; compare kickers.
					// When hand category matches, the number of kickers must also match.
					Q_ASSERT(kickers[playerIndex].count() == highestKickers.count());

					// Compare kickers, starting from the highest rank.
					QMap<AuGamePlayingCard::CARD, int>::const_iterator pki = kickers[playerIndex].constEnd() - 1;
					QMap<AuGamePlayingCard::CARD, int>::const_iterator hki = highestKickers.constEnd() - 1;
					for(; pki != kickers[playerIndex].constBegin()-1 && hki != highestKickers.constBegin()-1; pki--, hki--)
					{
						if(pki.key() > hki.key())
						{
							// Found a higher set of kickers for an otherwise equal hand.
							//qDebug() << "kicker card" << pki.key() << "beats" << hki.key();
							highestHand = hand[playerIndex];
							highestCard = highCard[playerIndex];
							highestCard2 = highCard2[playerIndex];
							highestKickers = kickers[playerIndex];
							break;
						}
					}
				}
			}
		}
	//qDebug() << "EvaluateWinner best hand" << highestHand << "high card" << highestCard << "kickers";
	//for(QMap<AuGamePlayingCard::CARD, int>::const_iterator hki = highestKickers.constEnd() - 1;
			//hki != highestKickers.constBegin()-1; hki--)
		//qDebug() << " " << hki.key();

	// Check for ties.
	winners.clear();
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		if(hand[playerIndex] == highestHand && highCard[playerIndex] == highestCard && highCard2[playerIndex] == highestCard2 &&
				kickers[playerIndex] == highestKickers)
			winners.append(playerIndex);

	// Announce winner(s) at voice range.
}

QString AuGameTexasHoldem::HandName(int playerIndex) const
{
	POKERHAND hand;
	AuGamePlayingCard::CARD highCard;	// Highest card among the cards used to make the hand. 2 2 2 2 A = 2 not A
	AuGamePlayingCard::CARD highCard2;	// 2nd highest card for two pairs and full house.
	QMap<AuGamePlayingCard::CARD, int> kickers;	// Cards not used to make the hand.
	Hand(playerIndex, hand, highCard, highCard2, kickers);
	return HandName(hand, highCard, highCard2, kickers);
}

//static
QString AuGameTexasHoldem::HandName(POKERHAND hand, AuGamePlayingCard::CARD highCard, AuGamePlayingCard::CARD highCard2,
		const QMap<AuGamePlayingCard::CARD, int>& kickers)
{
	switch(hand)
	{
	case Invalid:
		break;
	case HighCard:
		{
			QString hand = QObject::tr("%1 high").arg(AuGamePlayingCard::CardName(highCard));
			for(QMap<AuGamePlayingCard::CARD, int>::const_iterator ki = kickers.constEnd()-1; ki != kickers.constBegin()-1; ki--)
				hand += ", " + AuGamePlayingCard::CardName(ki.key());
			return hand;
		}
	case OnePair:
		{
			QString hand = QObject::tr("a pair of %1s").arg(AuGamePlayingCard::CardName(highCard));
			for(QMap<AuGamePlayingCard::CARD, int>::const_iterator ki = kickers.constEnd()-1; ki != kickers.constBegin()-1; ki--)
				hand += ", " + AuGamePlayingCard::CardName(ki.key());
			return hand;
		}
	case TwoPair:
		{
			QString hand = QObject::tr("two pair, %1s over %2s and a %3").arg(AuGamePlayingCard::CardName(highCard))
				.arg(AuGamePlayingCard::CardName(highCard2)).arg(AuGamePlayingCard::CardName(kickers.begin().key()));
			return hand;
		}
	case ThreeOfAKind:
		{
			QString hand = QObject::tr("three of a kind, %1s a %2 and a %3").arg(AuGamePlayingCard::CardName(highCard))
				.arg(AuGamePlayingCard::CardName(kickers.begin().key()))
				.arg(AuGamePlayingCard::CardName((kickers.begin()+1).key()));
			return hand;
		}
	case Straight:
		return QObject::tr("a straight, %1 high").arg(AuGamePlayingCard::CardName(highCard));
	case Flush:
		{
			QString hand = QObject::tr("a flush, %1 high").arg(AuGamePlayingCard::CardName(highCard));
			for(QMap<AuGamePlayingCard::CARD, int>::const_iterator ki = kickers.constEnd()-1; ki != kickers.constBegin()-1; ki--)
				hand += ", " + AuGamePlayingCard::CardName(ki.key());
			return hand;
		}
	case FullHouse:
		return QObject::tr("a full house, %1s over %2s").arg(AuGamePlayingCard::CardName(highCard))
			.arg(AuGamePlayingCard::CardName(highCard2));
	case FourOfAKind:
		{
			QString hand = QObject::tr("four of a kind, %1s and a %2").arg(AuGamePlayingCard::CardName(highCard))
				.arg(AuGamePlayingCard::CardName(kickers.begin().key()));
			return hand;
		}
	case StraightFlush:
		return QObject::tr("a straight flush, %1 high").arg(AuGamePlayingCard::CardName(highCard));
	case RoyalFlush:
		return QObject::tr("a royal flush");
	}
	return QObject::tr("Invalid Hand");
}

void AuGameTexasHoldem::Hand(int playerIndex, POKERHAND& hand, AuGamePlayingCard::CARD& highCard,
		AuGamePlayingCard::CARD& highCard2, QMap<AuGamePlayingCard::CARD, int>& kickers) const
{
	// Score the hands of the remaining players.
	if(PlayerId(playerIndex).IsNull())
	{
		// Player gets 0 score when not in game. Either folded or no player was ever there.
		hand = Invalid;
		highCard = AuGamePlayingCard::Ace;
		highCard2 = AuGamePlayingCard::Ace;	// n/a
		kickers.clear();
	}
	else
	{
		// Check for the possible poker hands.
		Hand(playerIndex, m_FinalHand, hand, highCard, highCard2, kickers);
	}
}

//static
void AuGameTexasHoldem::Hand(int playerIndex, const AuGamePlayingCard finalHand[MAX_GAME_PLAYERS][5], POKERHAND& hand,
		AuGamePlayingCard::CARD& highCard, AuGamePlayingCard::CARD& highCard2, QMap<AuGamePlayingCard::CARD, int>& kickers)
{
	// Check for the possible poker hands.
	// Check for a flush and aces and make an ordered list of the cards so we can check for straights.
	bool anyAces = false;								// Set true if any card in the hand is an ace.
	bool isFlush = true;								// Will be set false if this isn't true.
	QMap<AuGamePlayingCard::CARD, int> kindsOfCards;	// Cards counted by card type, to check for N of a kind.
	for(int ci=0; ci<5; ci++)
	{
		// Watch for aces.
		AuGamePlayingCard::CARD card = finalHand[playerIndex][ci].Card();
		if(card == AuGamePlayingCard::Ace)
			anyAces = true;

		// Watch for flush.
		if(ci && finalHand[playerIndex][ci].Suit() != finalHand[playerIndex][0].Suit())
			isFlush = false;

		// Count kinds of different cards.
		if(kindsOfCards.contains(card))
			kindsOfCards[card]++;
		else
			kindsOfCards.insert(card, 1);
	}

	// We have an ordered list (map). Check for straights with aces low, which are currently low valued in the order.
	bool isStraight = (kindsOfCards.count() == 5) && ((kindsOfCards.begin()+4).key() - kindsOfCards.begin().key() == 4);
	if(anyAces && !isStraight)
	{
		// There are aces and it's not an ace-low straight.
		// Substitute the Aces for AceHigh's and check again. We'll treat the aces as AceHigh from here on, for best card order.
		kindsOfCards.clear();							// Cards counted by card type, to check for N of a kind.
		for(int ci=0; ci<5; ci++)
		{
			// Watch for aces.
			AuGamePlayingCard::CARD card = finalHand[playerIndex][ci].Card();
			if(card == AuGamePlayingCard::Ace)
				card = AuGamePlayingCard::AceHigh;		// Change Ace to AceHigh for other hand evals.

			// Count kinds of different cards.
			if(kindsOfCards.contains(card))
				kindsOfCards[card]++;
			else
				kindsOfCards.insert(card, 1);
		}

		// Check for ace-high straight.
		isStraight = (kindsOfCards.count() == 5) && ((kindsOfCards.begin()+4).key() - kindsOfCards.begin().key() == 4);
	}

	// What did we get?
	if(isStraight && isFlush && (kindsOfCards.begin()+4).key() == AuGamePlayingCard::AceHigh)
	{
		hand = RoyalFlush;
		highCard = AuGamePlayingCard::AceHigh;
		highCard2 = AuGamePlayingCard::Ace;	// n/a
		kickers.clear();	// No kickers. The hand uses all five of the cards.
	}
	else if(isStraight && isFlush)
	{
		hand = StraightFlush;
		highCard = (kindsOfCards.begin()+4).key();
		highCard2 = AuGamePlayingCard::Ace;	// n/a
		kickers.clear();	// No kickers. The hand uses all five of the cards.
	}
	else if(kindsOfCards.count() == 2)
	{
		// Only two kinds of cards. Could only be four of a kind or full house.
		if(*(kindsOfCards.begin()) == 4)
		{
			hand = FourOfAKind;
			highCard = kindsOfCards.begin().key();
			highCard2 = AuGamePlayingCard::Ace;	// n/a

			kickers.clear();
			kickers.insert((kindsOfCards.begin()+1).key(), 1);
		}
		else if(*(kindsOfCards.begin()) == 1)
		{
			hand = FourOfAKind;
			highCard = (kindsOfCards.begin()+1).key();
			highCard2 = AuGamePlayingCard::Ace;	// n/a

			kickers.clear();
			kickers.insert(kindsOfCards.begin().key(), 1);
		}
		else
		{
			hand = FullHouse;
			highCard = (kindsOfCards.begin()+1).key();
			highCard2 = kindsOfCards.begin().key();
			kickers.clear();	// No kickers. The hand uses all five of the cards.
		}
	}
	else if(isFlush)
	{
		hand = Flush;
		highCard = (kindsOfCards.begin()+4).key();
		highCard2 = AuGamePlayingCard::Ace;	// n/a

		// The kickers are the lower four cards in the hand.
		kickers = kindsOfCards;
		kickers.erase(kickers.begin()+4);		// Remove the last one, which is the high card.
	}
	else if(isStraight)
	{
		hand = Straight;
		highCard = (kindsOfCards.begin()+4).key();
		highCard2 = AuGamePlayingCard::Ace;	// n/a
		kickers.clear();	// No kickers. The hand uses all five of the cards.
	}
	else if(kindsOfCards.count() == 3)
	{
		// Only three kinds of cards. Could only be three of a kind or two pair.
		if(*(kindsOfCards.begin()) == 3)
		{
			hand = ThreeOfAKind;
			highCard = kindsOfCards.begin().key();
			highCard2 = AuGamePlayingCard::Ace;	// n/a

			// Kickers are the two remaining cards.
			kickers.clear();
			kickers.insert((kindsOfCards.begin()+1).key(), 1);
			kickers.insert((kindsOfCards.begin()+2).key(), 1);
		}
		else if(*(kindsOfCards.begin()+1) == 3)
		{
			hand = ThreeOfAKind;
			highCard = (kindsOfCards.begin()+1).key();
			highCard2 = AuGamePlayingCard::Ace;	// n/a

			// Kickers are the two remaining cards.
			kickers.clear();
			kickers.insert((kindsOfCards.begin()).key(), 1);
			kickers.insert((kindsOfCards.begin()+2).key(), 1);
		}
		else if(*(kindsOfCards.begin()+2) == 3)
		{
			hand = ThreeOfAKind;
			highCard = (kindsOfCards.begin()+2).key();
			highCard2 = AuGamePlayingCard::Ace;	// n/a

			// Kickers are the two remaining cards.
			kickers.clear();
			kickers.insert((kindsOfCards.begin()).key(), 1);
			kickers.insert((kindsOfCards.begin()+1).key(), 1);
		}
		else
		{
			hand = TwoPair;
			if(*(kindsOfCards.begin()) == 2)
			{
				// First entry is a pair.
				if(*(kindsOfCards.begin()+1) == 2)
				{
					// Second entry is the second pair, which is higher in value.
					highCard = (kindsOfCards.begin()+1).key();
				}
				else
				{
					// Third entry is the second pair, which is higher in value.
					highCard = (kindsOfCards.begin()+2).key();
				}
				highCard2 = kindsOfCards.begin().key();
			}
			else
			{
				// Second and third entries are the two pairs. The last is higher in value.
				highCard = (kindsOfCards.begin()+2).key();
				highCard2 = (kindsOfCards.begin()+1).key();
			}

			// Kicker is the one remaining card.
			kickers.clear();
			if(*kindsOfCards.begin() == 1)
				kickers.insert((kindsOfCards.begin()).key(), 1);
			else if(*(kindsOfCards.begin()+1) == 1)
				kickers.insert((kindsOfCards.begin()+1).key(), 1);
			else
				kickers.insert((kindsOfCards.begin()+2).key(), 1);
		}
	}
	else if(kindsOfCards.count() == 4)
	{
		hand = OnePair;
		if(*(kindsOfCards.begin()) == 2)
			highCard = kindsOfCards.begin().key();
		else if(*(kindsOfCards.begin()+1) == 2)
			highCard = (kindsOfCards.begin()+1).key();
		else if(*(kindsOfCards.begin()+2) == 2)
			highCard = (kindsOfCards.begin()+2).key();
		else
			highCard = (kindsOfCards.begin()+3).key();
		highCard2 = AuGamePlayingCard::Ace;	// n/a

		// Kickers are the three remaining cards.
		kickers.clear();
		for(QMap<AuGamePlayingCard::CARD, int>::const_iterator oci = kindsOfCards.begin(); oci != kindsOfCards.end(); oci++)
			if(*oci == 1)
				kickers.insert(oci.key(), 1);
	}
	else
	{
		hand = HighCard;
		highCard = (kindsOfCards.begin()+4).key();
		highCard2 = AuGamePlayingCard::Ace;	// n/a

		// The kickers are the lower four cards in the hand.
		kickers = kindsOfCards;
		kickers.erase(kickers.begin()+4);		// Remove the last one, which is the high card.
	}
}

// Called when tick returns 2.
QByteArray AuGameTexasHoldem::Finish(DaPerson* players[MAX_GAME_PLAYERS]) const
{
	// Evaluate winner(s) and distribute winnings.
	QList<int> winners;
	EvaluateWinner(winners);
	double share;
	if(winners.count() == 1)
	{
		share = Pot();
		DaPerson* person = players[winners.first()];
		if(person)
		{
			person->CreditAccount(share);

			// Announce the turn at voice range for other players and any audience present.
			// The id of the player is used for voice range distance calc.
			QString text;
			if(PlayerCount() == 1 && m_FinalHand[winners.first()][0].Suit() == AuGamePlayingCard::Joker)
				text = QObject::tr("%1 wins %2 by default, last player in the game.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)share,0,'f',0).arg(g_Au.CurrencySymbol()));
			else
				text = QObject::tr("%1 wins %2 with %3.").arg(person->Name())
					.arg(QString("%L1%2").arg((double)share,0,'f',0).arg(g_Au.CurrencySymbol())).arg(HandName(winners.first()));
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(), person->FindParentSector()->Id(),
					person->Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);
		}
	}
	else if(winners.count() > 1)
	{
		share = (double)Pot() / (double)winners.count();
		QString text = QObject::tr("%1 winners split the %2 pot for %3 each. ").arg(winners.count())
			.arg(QString("%L1%2").arg((double)Pot(),0,'f',0).arg(g_Au.CurrencySymbol()))
			.arg(QString("%L1%2").arg((double)share,0,'f',0).arg(g_Au.CurrencySymbol()));

		AuId anyWinnerId;
		AuId anyWinnerSectorId;
		GALAXY anyWinnerGalaxy = GALAXY_Count;
		for(QList<int>::const_iterator wi = winners.constBegin(); wi != winners.constEnd(); wi++)
			if(players[*wi])
			{
				players[*wi]->CreditAccount(share);
				anyWinnerId = players[*wi]->Id();
				anyWinnerSectorId = players[*wi]->FindParentSector()->Id();
				anyWinnerGalaxy = players[*wi]->Galaxy();
				text += QObject::tr("%1 had %2. ").arg(players[*wi]->Name()).arg(HandName(*wi));
			}

		// Announce the turn at voice range for other players and any audience present.
		// The id of the player is used for voice range distance calc.
		AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), anyWinnerId, Name(), anyWinnerSectorId, anyWinnerGalaxy,
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(msg);
	}
	else
	{
		// Announce the turn at voice range for other players and any audience present.
		// The id of the player is used for voice range distance calc.
		DaPerson* person;
		if(players[0])
			person = players[0];
		else if(players[1])
			person = players[1];
		else if(players[2])
			person = players[2];
		else
			person = players[3];
		if(person)
		{
			QString text = QObject::tr("A winner could not be determined.");
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), person->Id(), Name(),
					person->FindParentSector()->Id(), person->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(msg);
		}
		share = 0;
	}

	// Pack the final results into a blob.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << share;

	// List winners and their hands.
	out << (quint8)winners.count();
	for(QList<int>::const_iterator wi = winners.constBegin(); wi != winners.constEnd(); wi++)
	{
		if(players[*wi])
		{
			out << players[*wi]->Name();
			int playerIndex = PlayerIndex(players[*wi]->Id());
			for(int card=0; card<5; card++)
				out << m_FinalHand[playerIndex][card];
		}
		else
		{
			out << QObject::tr("Nobody");
			out << AuGamePlayingCard(AuGamePlayingCard::Joker, AuGamePlayingCard::Black);
			out << AuGamePlayingCard(AuGamePlayingCard::Joker, AuGamePlayingCard::Black);
			out << AuGamePlayingCard(AuGamePlayingCard::Joker, AuGamePlayingCard::Black);
			out << AuGamePlayingCard(AuGamePlayingCard::Joker, AuGamePlayingCard::Black);
			out << AuGamePlayingCard(AuGamePlayingCard::Joker, AuGamePlayingCard::Black);
		}
	}

	return ba;
}

void AuGameTexasHoldem::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	Q_UNUSED(projection);
	// Set up for rendering objects that are lit using environmental lighting and fog, such as table, buttons, cards and chips.
	// Select vertex shader and configure its parameters.
	bool useFog = re.HasFog();
	AuPoint ambientUp = model.ProjectVector(0,0,1);
	if(useFog)
	{
		re.GLSL().ModelTexFogShader().Select(projection, re);
		re.GLSL().ModelTexFogShader().SetModelView(model);
		re.GLSL().ModelTexFogShader().SetTexture(AuGamePlayingCard::TextureId(re));

		// Ambient light
		float ambientR, ambientG, ambientB, ambientA;
		re.CurrentAmbientLight(ambientR, ambientG, ambientB, ambientA);
		re.GLSL().ModelTexFogShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().ModelTexFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Fog
		float fogR, fogG, fogB, fogDensity;
		re.CurrentFog(fogR, fogG, fogB, fogDensity);
		re.GLSL().ModelTexFogShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Render the table top.
		AuGamePlayingCard::PokerTableTopModel(re).SelectGL();
		AuGamePlayingCard::PokerTableTopModel(re).DrawGL();
		AuGamePlayingCard::PokerTableTopModel(re).DeselectGL();

		// Render buttons.
		RenderButtons(model, re, true);

		// Render bets as stacks of chips.
		RenderBets(model, re, true);

		// Render all cards.
		RenderCards(model, re, true);

		re.GLSL().ModelTexFogShader().Deselect();
	}
	else
	{
		re.GLSL().ModelTexShader().Select(projection, re);
		re.GLSL().ModelTexShader().SetModelView(model);
		re.GLSL().ModelTexShader().SetTexture(AuGamePlayingCard::TextureId(re));

		// Ambient light
		float ambientR, ambientG, ambientB, ambientA;
		re.CurrentAmbientLight(ambientR, ambientG, ambientB, ambientA);
		re.GLSL().ModelTexShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().ModelTexShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Render the table top.
		AuGamePlayingCard::PokerTableTopModel(re).SelectGL();
		AuGamePlayingCard::PokerTableTopModel(re).DrawGL();
		AuGamePlayingCard::PokerTableTopModel(re).DeselectGL();

		// Render buttons.
		RenderButtons(model, re, false);

		// Render bets as stacks of chips.
		RenderBets(model, re, false);

		// Render all cards.
		RenderCards(model, re, false);

		re.GLSL().ModelTexShader().Deselect();
	}
}

void AuGameTexasHoldem::RenderButtons(const AuMatrix4x4& model, AuGLEffects& re, bool useFog)
{
	const float centerDistButtons = 1.45f;

	// Dealer button.
	AuGLVAO vao = AuGamePlayingCard::HoldemButtonModel(re, 0);
	RenderModel(model, vao, 0,-centerDistButtons,0, 0,0,0, re, useFog);
	//RenderModel(model, AuGamePlayingCard::HoldemButtonModel(re, 0), 0,-centerDistButtons,0, 0,0,0, re, useFog);

	// Blind buttons
	if(m_TurnState == LittleBlindAnte)
		return;

	// Little blind has not been determined yet.
	// Locate the first player to the left of the dealer slot.
	// Test all four positions because we may end up back at the dealer as the big blind.
	int whichBlind = 0;
	for(int testPos=0; testPos<MAX_GAME_PLAYERS; testPos++)
	{
		// Start the search at the next index after the dealer.
		int playerIndex = testPos+1;
		if(playerIndex >= MAX_GAME_PLAYERS)
			playerIndex = 0;	// Wrap back to the dealer.

		if(m_Pot[playerIndex])
		{
			if(!whichBlind)
			{
				// Found the little blind.
				// The first person to the left of the dealer who has bet is the little blind, even if they have left the game.
				AuGLVAO vao = AuGamePlayingCard::HoldemButtonModel(re, 1);
				if(playerIndex == 0)
					RenderModel(model, vao, -.5f,-centerDistButtons,0, 0,0,0, re, useFog);
				else if(playerIndex == 1)
					RenderModel(model, vao, -centerDistButtons,.5f,0, 0,0,-90, re, useFog);
				else if(playerIndex == 2)
					RenderModel(model, vao, .5f,centerDistButtons,0, 0,0,180, re, useFog);
				else //if(playerIndex == 3)
					RenderModel(model, vao, centerDistButtons,-.5f,0, 0,0,90, re, useFog);

				// When we're at the big blind ante. The big blind hasn't been determined yet.
				if(m_TurnState == BigBlindAnte)
					break;	// Stop looking for the big blind.
			}
			else
			{
				// Found the big blind.
				// The second person to the left of the dealer who has bet is the big blind, even if they have left the game.
				AuGLVAO vao = AuGamePlayingCard::HoldemButtonModel(re, 2);
				if(playerIndex == 0)
					RenderModel(model, vao, -.5f,-centerDistButtons,0, 0,0,0, re, useFog);
				else if(playerIndex == 1)
					RenderModel(model, vao, -centerDistButtons,.5f,0, 0,0,-90, re, useFog);
				else if(playerIndex == 2)
					RenderModel(model, vao, .5f,centerDistButtons,0, 0,0,180, re, useFog);
				else //if(playerIndex == 3)
					RenderModel(model, vao, centerDistButtons,-.5f,0, 0,0,90, re, useFog);
				break;	// Stop looking for blinds.
			}
			whichBlind++;
		}
	}
}

void AuGameTexasHoldem::RenderBets(const AuMatrix4x4& model, AuGLEffects& re, bool useFog)
{
	//const float centerDistChips = AuGame::TableTopRadius() - (AuGamePlayingCard::PokerChipRadius() + .0833);
	const float centerDistChips = AuGamePlayingCard::PokerChipRadius() + .0833*2;

	// Make stacks of chips.
	AuDice dice;						// Construct my own dice so I can spin the chips randomly consistently.
	AuMatrix4x4 bM(model);
	bM.RotateZ(-(22.5 + 11.25) * DEG2RAD);	// Rotate to the left side of the current player.
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		quint32 bet = m_Pot[playerIndex];
		if(bet)
		{
			dice.Seed(bet);					// The same amount of money always produces the same stacking.
			float xpos = 0;
			float ypos = centerDistChips;
			quint32 chipDenom = 100000000;	// Each chip denomination adds a zero. We have 9 chip denominations.
			int chipModelIndex = 8;
			int stacks = 0;
			while(bet)
			{
				// Break down the player's bet into stacks of nine in each denomination.
				if(bet >= chipDenom)
				{
					quint32 chips = bet / chipDenom;
					bet -= chips * chipDenom;

					float zpos = 0;
					for(unsigned int chip=0; chip<chips; chip++)
					{
						float zrot = dice.Roll(1,360);
						AuGLVAO vao = AuGamePlayingCard::PokerChipModel(re, chipModelIndex);
						RenderModel(bM, vao, xpos,-ypos,zpos, 0,0,zrot, re, useFog);
						zpos += AuGamePlayingCard::PokerChipThickness();
					}

					stacks++;
					if(stacks == 4)
					{
						xpos += AuGamePlayingCard::PokerChipRadius()*2;				// Start another row of chips.
						ypos = centerDistChips - AuGamePlayingCard::PokerChipRadius();
					}
					else if(stacks == 7)
					{
						xpos += AuGamePlayingCard::PokerChipRadius()*2;				// Start another row of chips.
						ypos = centerDistChips - AuGamePlayingCard::PokerChipRadius()*2;
					}
					else
						ypos += AuGamePlayingCard::PokerChipRadius()*2;				// Move away from the center by a chip diameter.
				}

				chipDenom /= 10;
				chipModelIndex--;
			}
		}
		bM.RotateZ(-90*DEG2RAD);	// Rotate to next player position. RenderModel updates the shader projections.
	}
}

void AuGameTexasHoldem::RenderCards(const AuMatrix4x4& model, AuGLEffects& re, bool useFog)
{
	const float centerDistBurn = AuGamePlayingCard::PlayingCardHeight() + .25f*.0833f;	// Distance from center to burn cards.
	const float centerDistButtons = 1.45f;
	const float centerDistHand = 2.1f;								// Distance from center to players' cards.
	const float cardSpacing = AuGamePlayingCard::PlayingCardWidth() + .25f*.0833f;		// Distance between cards laid in a row.

	// As the game progresses, more and more cards are visible.
	// These are stacked in reverse order so they can cascade without breaking.
	int showPlayerHand = -1;	// player index whose hand will be shown on the screen. The current scene's player, if any.
	switch(m_TurnState)
	{
	case Showdown:
	case FinalBettingRound:
		// River card is on the table.
		{
			AuGamePlayingCard::SUIT suit = m_RiverCard.Suit();
			AuGamePlayingCard::CARD card = m_RiverCard.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, cardSpacing*2,centerDistBurn,0, -90,0,0, re, useFog);
		}
		[[fallthrough]];
	case DealRiverCard:
		// Burning river card is on the table.
		{
			AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, cardSpacing*2,-centerDistBurn,0, 90,0,0, re, useFog);
		}
		if(IsTurning() && m_SpriteTurnState < DealRiverCard)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = DealRiverCard;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(cardSpacing*2,0,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case BurnRiverCard:
		if(IsTurning() && m_SpriteTurnState < BurnRiverCard)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = BurnRiverCard;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(cardSpacing*2,-centerDistBurn,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case TurnBettingRound:
		// Turn card is on the table.
		{
			AuGamePlayingCard::SUIT suit = m_TurnCard.Suit();
			AuGamePlayingCard::CARD card = m_TurnCard.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, cardSpacing,centerDistBurn,0, -90,0,0, re, useFog);
		}
		[[fallthrough]];
	case DealTurnCard:
		// Burning turn card is on the table.
		{
			AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, cardSpacing,-centerDistBurn,0, 90,0,0, re, useFog);
		}
		if(IsTurning() && m_SpriteTurnState < DealTurnCard)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = DealTurnCard;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(cardSpacing,0,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case BurnTurnCard:
		if(IsTurning() && m_SpriteTurnState < BurnTurnCard)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = BurnTurnCard;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(cardSpacing,-centerDistBurn,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case PostflopBettingRound:
		// Third flop card is on the table.
		{
			AuGamePlayingCard::SUIT suit = m_FlopCard3.Suit();
			AuGamePlayingCard::CARD card = m_FlopCard3.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, 0,centerDistBurn,0, -90,0,0, re, useFog);
		}
		[[fallthrough]];
	case DealFlopCard3:
		// Second flop card is on the table.
		{
			AuGamePlayingCard::SUIT suit = m_FlopCard2.Suit();
			AuGamePlayingCard::CARD card = m_FlopCard2.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, -cardSpacing,centerDistBurn,0, -90,0,0, re, useFog);
		}
		if(IsTurning() && m_SpriteTurnState < DealFlopCard3)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = DealFlopCard3;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(0,0,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case DealFlopCard2:
		// First flop card is on the table.
		{
			AuGamePlayingCard::SUIT suit = m_FlopCard1.Suit();
			AuGamePlayingCard::CARD card = m_FlopCard1.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, -cardSpacing*2,centerDistBurn,0, -90,0,0, re, useFog);
		}
		if(IsTurning() && m_SpriteTurnState < DealFlopCard2)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = DealFlopCard2;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(-cardSpacing,0,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case DealFlopCard1:
		// Burning flop card is on the table.
		{
			AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(model, cardModel, 0,-centerDistBurn,0, 90,0,0, re, useFog);
		}
		if(IsTurning() && m_SpriteTurnState < DealFlopCard1)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = DealFlopCard1;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(-cardSpacing*2,0,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case BurnFlopCard:
		if(IsTurning() && m_SpriteTurnState < BurnFlopCard)
		{
			// Create a sprite of the dealt card.
			m_SpriteTurnState = BurnFlopCard;

			AuGamePlayingCardSprite cardSprite;
			cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
			cardSprite.m_Loc = cardSprite.m_Start;
			cardSprite.m_End = AuPoint(0,-centerDistBurn,0);
			m_Sprites.append(cardSprite);
		}
		[[fallthrough]];
	case PreflopBettingRound:
		// Second hole card is on the table.
		if(IsTurning() && m_TurnState != Showdown)
		{
			if(!PlayerId(0).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard2[0].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, cardSpacing/2,-centerDistHand,0, 90,0,0, re, useFog);
				}
			}

			if(!PlayerId(1).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard2[1].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, -centerDistHand,-cardSpacing/2,0, 90,0,-90, re, useFog);
				}
			}

			if(!PlayerId(2).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard2[2].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, -cardSpacing/2,centerDistHand,0, 90,0,180, re, useFog);
				}
			}

			if(!PlayerId(3).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard2[3].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, centerDistHand,cardSpacing/2,0, 90,0,90, re, useFog);
				}
			}
		}
		[[fallthrough]];
	case DealHoleCard2:
		// First hole card is on the table.
		if(IsTurning() && m_TurnState != Showdown)
		{
			if(!PlayerId(0).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard1[0].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, -cardSpacing/2,-centerDistHand,0, 90,0,0, re, useFog);
				}
				else
					showPlayerHand = 0;					// This card is valid so this player is the scene atom.
			}

			if(!PlayerId(1).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard1[1].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, -centerDistHand,cardSpacing/2,0, 90,0,-90, re, useFog);
				}
				else
					showPlayerHand = 1;					// This card is valid so this player is the scene atom.
			}

			if(!PlayerId(2).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard1[2].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, cardSpacing/2,centerDistHand,0, 90,0,180, re, useFog);
				}
				else
					showPlayerHand = 2;					// This card is valid so this player is the scene atom.
			}

			if(!PlayerId(3).IsNull())
			{
				AuGamePlayingCard::SUIT suit = m_HoleCard1[3].Suit();
				if(suit == AuGamePlayingCard::Joker)
				{
					// Draw unknown cards face down in front of the player. Known cards are rendered below in the 'hand'.
					AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
					RenderModel(model, cardModel, centerDistHand,-cardSpacing/2,0, 90,0,90, re, useFog);
				}
				else
					showPlayerHand = 3;					// This card is valid so this player is the scene atom.
			}
		}
		if(IsTurning() && m_SpriteTurnState < DealHoleCard2)
		{
			// Create up to four sprites to deal the second hole card.
			m_SpriteTurnState = DealHoleCard2;

			if(!PlayerId(0).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(cardSpacing/2,-centerDistHand,0);
				m_Sprites.append(cardSprite);
			}

			if(!PlayerId(1).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(-centerDistHand,-cardSpacing/2,0);
				m_Sprites.append(cardSprite);
			}

			if(!PlayerId(2).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(-cardSpacing/2,centerDistHand,0);
				m_Sprites.append(cardSprite);
			}

			if(!PlayerId(3).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(centerDistHand,cardSpacing/2,0);
				m_Sprites.append(cardSprite);
			}
		}
		[[fallthrough]];
	case DealHoleCard1:
		if(IsTurning() && m_SpriteTurnState < DealHoleCard1)
		{
			// Create up to four sprites to deal the first hole card.
			m_SpriteTurnState = DealHoleCard1;

			if(!PlayerId(0).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(-cardSpacing/2,-centerDistHand,0);
				m_Sprites.append(cardSprite);
			}

			if(!PlayerId(1).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(-centerDistHand,cardSpacing/2,0);
				m_Sprites.append(cardSprite);
			}

			if(!PlayerId(2).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(cardSpacing/2,centerDistHand,0);
				m_Sprites.append(cardSprite);
			}

			if(!PlayerId(3).IsNull())
			{
				AuGamePlayingCardSprite cardSprite;
				cardSprite.m_Start = AuPoint(0,-AuGame::TableTopRadius(), 1);	// 1' above the edge of the table in front of the dealer.
				cardSprite.m_Loc = cardSprite.m_Start;
				cardSprite.m_End = AuPoint(centerDistHand,-cardSpacing/2,0);
				m_Sprites.append(cardSprite);
			}
		}
		[[fallthrough]];
	case BigBlindAnte:
	case LittleBlindAnte:
		break;
	}

	// Show the hand of the current player.
	if(showPlayerHand >= 0 && m_TurnState != Showdown)
	{
		// Rotate to the player who is looking at their hand.
		AuMatrix4x4 cm(model);
		if(showPlayerHand == 1)
			cm.RotateZ(-90*DEG2RAD);
		else if(showPlayerHand == 2)
			cm.RotateZ(180*DEG2RAD);
		else if(showPlayerHand == 3)
			cm.RotateZ(90*DEG2RAD);

		// These cards are shown close to the edge of the table, in an overlapping row.
		float xpos = -AuGamePlayingCard::PlayingCardWidth() * 1.5f;
		float ypos = -AuGame::TableTopRadius();
		float zpos = AuGamePlayingCard::PlayingCardHeight() * .5f;
		float tipangle = -15;
		if(m_HoleCard1[showPlayerHand].Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_HoleCard1[showPlayerHand].Suit();
			AuGamePlayingCard::CARD card = m_HoleCard1[showPlayerHand].Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
		if(m_HoleCard2[showPlayerHand].Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_HoleCard2[showPlayerHand].Suit();
			AuGamePlayingCard::CARD card = m_HoleCard2[showPlayerHand].Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
		if(m_FlopCard1.Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_FlopCard1.Suit();
			AuGamePlayingCard::CARD card = m_FlopCard1.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
		if(m_FlopCard2.Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_FlopCard2.Suit();
			AuGamePlayingCard::CARD card = m_FlopCard2.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
		if(m_FlopCard3.Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_FlopCard3.Suit();
			AuGamePlayingCard::CARD card = m_FlopCard3.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
		if(m_TurnCard.Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_TurnCard.Suit();
			AuGamePlayingCard::CARD card = m_TurnCard.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
		if(m_RiverCard.Suit() != AuGamePlayingCard::Joker)
		{
			AuGamePlayingCard::SUIT suit = m_RiverCard.Suit();
			AuGamePlayingCard::CARD card = m_RiverCard.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
			RenderModel(cm, cardModel, xpos,ypos,zpos, tipangle,0,0, re, useFog);
			xpos += AuGamePlayingCard::PlayingCardWidth()/2;
			ypos -= AuGamePlayingCard::PlayingCardThickness();
		}
	}

	// Show the final hands of players who have shown them.
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		if(!PlayerId(playerIndex).IsNull() && m_FinalHand[playerIndex][0].Suit() != AuGamePlayingCard::Joker)
		{
			// Render final hand.
			AuMatrix4x4 cm(model);
			if(playerIndex == 1)
				cm.RotateZ(-90*DEG2RAD);
			else if(playerIndex == 2)
				cm.RotateZ(180*DEG2RAD);
			else if(playerIndex == 3)
				cm.RotateZ(90*DEG2RAD);

			// Render their cards.
			float xpos = -cardSpacing*2;
			for(int cardIndex=0; cardIndex<5; cardIndex++)
			{
				AuGamePlayingCard::SUIT suit = m_FinalHand[playerIndex][cardIndex].Suit();
				AuGamePlayingCard::CARD card = m_FinalHand[playerIndex][cardIndex].Card();
				AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);
				RenderModel(cm, cardModel, xpos,-centerDistHand,0, -90,0,0, re, useFog);
				xpos += cardSpacing;
			}
		}
	}

	// Show the deck of cards.
	AuGLVAO cardModel = AuGamePlayingCard::FacelessCardModel(re, m_BackStyle - AuGamePlayingCard::BackStyle0);
	float zpos = 0;		// vertical position of each card
	for(QList<AuGamePlayingCard>::const_iterator di = m_Deck.constBegin(); di != m_Deck.constEnd(); di++)
	{
		RenderModel(model, cardModel, cardSpacing*2,-centerDistButtons,zpos, -90,0,0, re, useFog);
		zpos += AuGamePlayingCard::PlayingCardThickness();
	}

	// Render flying card sprites.
	QList<AuGamePlayingCardSprite>::iterator si = m_Sprites.begin();
	while(si != m_Sprites.end())
	{
		// Move the sprite.
		AuPoint mvec = (*si).m_End - (*si).m_Start;
		mvec.Unit(.1);					// Amount of movement this frame.
		(*si).m_Loc += mvec;
		if((*si).m_Loc.Distance((*si).m_End) > .1)
		{
			// Spin the card.
			(*si).m_Rot += 5;

			// Draw the card sprite at its nu location.
			AuGamePlayingCard::SUIT suit = (*si).m_Card.Suit();
			AuGamePlayingCard::CARD card = (*si).m_Card.Card();
			AuGLVAO cardModel = AuGamePlayingCard::CardModel(re, card, suit, m_BackStyle - AuGamePlayingCard::BackStyle0);

			// Cards sprites are drawn face up. The faceless card type is used when secrecy is wanted.
			RenderModel(model, cardModel, (*si).m_Loc.x,(*si).m_Loc.y,(*si).m_Loc.z, -90,0,(*si).m_Rot, re, useFog);
			si++;
		}
		else
		{
			// Remove the card sprite.
			si = m_Sprites.erase(si);
		}
	}
}

void AuGameTexasHoldem::RenderModel(const AuMatrix4x4& model, AuGLVAO& vao, float locX, float locY, float locZ, float rotX,
		float rotY, float rotZ, AuGLEffects& re, bool useFog)
{
	AuMatrix4x4 mv(model);
	mv.Translate(locX, locY, locZ);
	if(rotZ)
		mv.RotateZ(rotZ * DEG2RAD);
	if(rotY)
		mv.RotateY(rotY * DEG2RAD);
	if(rotX)
		mv.RotateX(rotX * DEG2RAD);

	if(useFog)
		re.GLSL().ModelTexFogShader().SetModelView(mv);
	else
		re.GLSL().ModelTexShader().SetModelView(mv);

	vao.SelectGL();
	vao.DrawGL();
	vao.DeselectGL();
}
