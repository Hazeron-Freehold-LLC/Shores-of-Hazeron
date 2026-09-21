//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augametexasholdem.h,v 1.19 2022/02/19 17:31:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameTexasHoldem_H
#define AuGameTexasHoldem_H

#include "augame.h"
#include "augameplayingcard.h"
#include <QColor>
#include <QDebug>
#include <augleffects.h>
#include <aupoint.h>

class AuGameTexasHoldem : public AuGame
{
public:
	enum POKERHAND
	{
		Invalid,
		HighCard,
		OnePair,
		TwoPair,
		ThreeOfAKind,
		Straight,
		Flush,
		FullHouse,
		FourOfAKind,
		StraightFlush,
		RoyalFlush,
	};

	enum TURNSTATE
	{
		LittleBlindAnte,
		BigBlindAnte,
		DealHoleCard1,
		DealHoleCard2,
		PreflopBettingRound,
		BurnFlopCard,
		DealFlopCard1,
		DealFlopCard2,
		DealFlopCard3,
		PostflopBettingRound,
		BurnTurnCard,
		DealTurnCard,
		TurnBettingRound,
		BurnRiverCard,
		DealRiverCard,
		FinalBettingRound,
		Showdown,
	};

private: 
	quint8						m_BackStyle;					// Back style is an index into the Joker texture row, Three...King.
	quint32						m_BigBlind;						// Big blind amount.
	QList<AuGamePlayingCard>	m_Deck;
	AuGamePlayingCard			m_FinalHand[MAX_GAME_PLAYERS][5];	// The final hand picked by each player.
	AuGamePlayingCard			m_FlopCard1;
	AuGamePlayingCard			m_FlopCard2;
	AuGamePlayingCard			m_FlopCard3;
	AuGamePlayingCard			m_HoleCard1[MAX_GAME_PLAYERS];
	AuGamePlayingCard			m_HoleCard2[MAX_GAME_PLAYERS];
	quint32						m_LittleBlind;					// Little blind amount.
	quint32						m_Pot[MAX_GAME_PLAYERS];		// Pot contributions by player. Total is the entire pot.
	quint8						m_RaisesThisRound[MAX_GAME_PLAYERS];	// Number of raises by each player in a betting round.
	AuGamePlayingCard			m_RiverCard;
	AuGamePlayingCard			m_TurnCard;
	quint8						m_TurnState;					// Turn state of this holdem game.

	// Render data used on client. Not serialized.
	QList<AuGamePlayingCardSprite>	m_Sprites;					// Sprites on render client.
	quint8						m_SpriteTurnState;				// Turn state in which sprites were last created.

public:
	AuGameTexasHoldem();
	virtual ~AuGameTexasHoldem();
	AuGameTexasHoldem(const AuGameTexasHoldem& c);

	inline quint8 BackStyle() const
	{
		return m_BackStyle;
	}

	// Returns the current bet of a player.
	inline quint32 Bet(const AuId& personId) const
	{
		int playerIndex = PlayerIndex(personId);
		if(playerIndex < 0)
			return 0;	// Player is not in this game.
		return m_Pot[playerIndex];
	}

	// Returns the big blind.
	inline quint32 BigBlind() const
	{
		return m_BigBlind;
	}

	// Returns the amount that must be matched when calling. This is the highest current bet.
	inline quint32 CallBet() const
	{
		quint32 highestBet = m_Pot[0];
		for(int player=1; player<MAX_GAME_PLAYERS; player++)
			if(m_Pot[player] > highestBet)
				highestBet = m_Pot[player];
		return highestBet;
	}

	// Returns a duplicate of this game, allocated on the heap.
	virtual AuGame* Clone() const;

	virtual AuDialog* CreateFinishUI(const QByteArray& gameResult, QWidget* parent) const;

	virtual AuDialog* CreateSettingsUI(const DaPerson& person, QWidget* parent) const;

	virtual AuDialog* CreateTurnUI(const DaPerson& person, QWidget* parent) const;

	// Returns the description of this game that appears in the game selection list.
	// The description of the game should include a general overview and the rules.
	// The description is in html format.
	virtual QString Description() const;

	// Returns a blob of the completion results of the game.
	virtual QByteArray Finish(DaPerson* players[MAX_GAME_PLAYERS]) const;

	const AuGamePlayingCard& FlopCard1() const
	{
		return m_FlopCard1;
	}

	const AuGamePlayingCard& FlopCard2() const
	{
		return m_FlopCard2;
	}

	const AuGamePlayingCard& FlopCard3() const
	{
		return m_FlopCard3;
	}

	// Static version is used when checking type read from a serial data stream.
	// This must match the type value returned by the virtual Type() function.
	inline static quint8 GameType()
	{
		return 2;
	}

	// Evaluate the final hand of a given player.
	// 'hand' is the hand category.
	// 'highCard' is the highest card among the cards used to make the hand. 2 2 2 2 A = 2 not A
	// 'highCard2' is the second card for two pairs and full house
	// 'kickers' lists the remaining cards, if any, in rank order. AceHigh will be used when appropriate.
	static void Hand(int playerIndex, const AuGamePlayingCard finalHand[MAX_GAME_PLAYERS][5], POKERHAND& hand,
			AuGamePlayingCard::CARD& highCard, AuGamePlayingCard::CARD& highCard2, QMap<AuGamePlayingCard::CARD, int>& kickers);

	// Return the name of a hand.
	static QString HandName(POKERHAND hand, AuGamePlayingCard::CARD highCard, AuGamePlayingCard::CARD highCard2,
			const QMap<AuGamePlayingCard::CARD, int>& kickers);

	const AuGamePlayingCard& HoleCard1(int playerIndex) const
	{
		return m_HoleCard1[playerIndex];
	}

	const AuGamePlayingCard& HoleCard2(int playerIndex) const
	{
		return m_HoleCard2[playerIndex];
	}

	// Returns the icon of this game that appears in the game selection list.
	virtual QIcon Icon() const;

	// Returns the little blind.
	inline quint32 LittleBlind() const
	{
		return m_LittleBlind;
	}

	// Returns the name of this game that appears in the game selection list.
	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the total amount in the pot right now.
	inline quint32 Pot() const
	{
		// Pot is tallied whether or not the player id at that index is null. When a player folds, they are no longer in the game
		// but the money they put into the pot remains until the end of the game.
		quint32 pot = 0;
		for(int player=0; player<MAX_GAME_PLAYERS; player++)
			pot += m_Pot[player];
		return pot;
	}

	// Refresh game with corresponding game received from server without breaking client-only render values.
	virtual void Refresh(const AuGame* gameFromServer);

	// Renders this game in the environment. 'model' is the transformation to the center of the table top, rotated to the table.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	const AuGamePlayingCard& RiverCard() const
	{
		return m_RiverCard;
	}

	virtual void Rx(QDataStream& in);

	// Starts this game.
	// Receives the blob of game settings that is emitted by the game setup dialog when the dealer presses 'Start Game'.
	// Returns false if the resulting setup is not valid.
	// The game state is not changed when the setup is not valid.
	virtual bool Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings);

	// Called every other second after the game is started.
	// Returns 0=no change, 1=data changed, 2=game over
	virtual int Tick();

	const AuGamePlayingCard& TurnCard() const
	{
		return m_TurnCard;
	}

	inline TURNSTATE TurnState() const
	{
		return (TURNSTATE)m_TurnState;
	}

	// Called with a blob of data when a player completes their turn.
	// Returns false if no game data changes as a result.
	virtual bool TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn);

	// Returns the name of the current turn UI. Used to validate the ui is appropriate for the game state. UI object name should
	// vary with game state.
	virtual QString TurnUIName() const;

	// Transmit only what the client needs to prevent cheating.
	virtual void Tx(QDataStream& out, const DaPerson& sceneAtom) const;

	// Returns a number that is unique to each derived game class.
	virtual quint8 Type() const;

protected:
	// Returns true if the current bet of all remaining players matches.
	inline bool AllBetsMatch() const
	{
		quint32 curBet = 0;
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			if(!PlayerId(playerIndex).IsNull())
			{
				if(!curBet)
					curBet = m_Pot[playerIndex];
				else if(curBet != m_Pot[playerIndex])
					return false;
			}
		return true;
	}

	AuGamePlayingCard DealCard();

	int DoTurn();

	// Fills winners with the playerIndexes of the winning players.
	void EvaluateWinner(QList<int>& winners) const;

	// Evaluate the final hand of a given player.
	// 'hand' is the hand category.
	// 'highCard' is the highest card among the cards used to make the hand. 2 2 2 2 A = 2 not A
	// 'highCard2' is the second card for two pairs and full house
	// 'kickers' lists the remaining cards, if any, in rank order. AceHigh will be used when appropriate.
	void Hand(int playerIndex, POKERHAND& hand, AuGamePlayingCard::CARD& highCard,
			AuGamePlayingCard::CARD& highCard2, QMap<AuGamePlayingCard::CARD, int>& kickers) const;

	// Call out the final hand of a player.
	QString HandName(int playerIndex) const;

	// Query for beginning of a game turn cycle, allowing for players to be randomly missing.
	// Returns true if the current TurnId is the first person remaining in the game that is not player 0, allowing for the dealer
	// to have left.
	// The dealer was player 0. The game turn is complete when it is the person's turn who is next after the dealer slot.
	inline bool IsGameTurn() const
	{
		AuId turnId = TurnId();
		for(int playerIndex=1; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			if(PlayerId(playerIndex) == turnId)
				return true;		// The first non-null id we found after the dealer matches the current turn id.
			else if(!PlayerId(playerIndex).IsNull())
				return false;		// Found a non-null player id that does not match the current turn id.
		return false;
	}

	void RenderBets(const AuMatrix4x4& model, AuGLEffects& re, bool useFog);

	void RenderButtons(const AuMatrix4x4& model, AuGLEffects& re, bool useFog);

	void RenderCards(const AuMatrix4x4& model, AuGLEffects& re, bool useFog);

	void RenderModel(const AuMatrix4x4& model, AuGLVAO& vao, float locX, float locY, float locZ, float rotX, float rotY, float rotZ,
			AuGLEffects& re, bool useFog);

	void ShuffleDeck();
};

#endif // AuGameTexasHoldem_H
