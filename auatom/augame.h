//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augame.h,v 1.28 2022/04/15 16:58:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGame_H
#define AuGame_H

#include "auatom.h"
#include "auelapsedtimer.h"
#include "auid.h"

#define MAX_GAME_PLAYERS	4							// A game table has 4 seats, permitting a maximum of 4 players in a game.

class AuDialog;
class DaPerson;
class AuGLEffects;
class AuMatrix4x4;
class QLayout;

class AUATOM_DLL AuGame
{
protected:
	enum STATE
	{
		Setup,											// Game is awaiting setup information from the dealer.
		Turning,										// Game is doing turns between the players.
		Processing,										// Game is processing, probably waiting for time to pass.
		Fanfare,										// Game is announcing the winner and preparing to terminate.
	};

private:
	AuId			m_PlayerIds[MAX_GAME_PLAYERS];		// The ids of the players in the game. The first id is the dealer.
	QString			m_PlayerNames[MAX_GAME_PLAYERS];	// The names of the players in the game.
	quint8			m_State;							// STATE value.
	AuElapsedTimer	m_StateTime;						// Elapsed timer since SetState called last. Started at construction.

protected:
	quint8			m_DealerSeat;						// Seat index of dealer at the table. 0-3
	quint8			m_TurnIndex;						// Index of which player's turn it is now.

public:
	AuGame();
	virtual ~AuGame();
	AuGame(const AuGame& c);

	// Returns a duplicate of this game, allocated on the heap.
	virtual AuGame* Clone() const = 0;

	static AuGame* CreateGame(quint8 gameType);

	virtual AuDialog* CreateFinishUI(const QByteArray& gameResult, QWidget* parent) const;

	virtual AuDialog* CreateSettingsUI(const DaPerson& person, QWidget* parent) const;

	virtual AuDialog* CreateTurnUI(const DaPerson& person, QWidget* parent) const;

	// Z rotation needed to spin game to orient to the person who started the game, even if they have left the game.
	inline quint8 DealerSeat() const
	{
		return m_DealerSeat;
	}

	// Returns the description of this game that appears in the game selection list.
	// The description of the game should include a general overview and the rules.
	// The description is in html format.
	virtual QString Description() const;

	// Returns a blob of the completion results of the game.
	virtual QByteArray Finish(DaPerson* players[MAX_GAME_PLAYERS]) const = 0;

	// Static version is used when checking type read from a serial data stream.
	// This must match the type value returned by the virtual Type() function.
	inline static quint8 GameType()
	{
		return 0;
	}

	// Returns the icon of this game that appears in the game selection list.
	virtual QIcon Icon() const;

	// Returns true if the game has been started, that is, it's not in setup any more.
	// Returns false when in setup.
	inline bool IsStarted() const
	{
		return m_State != Setup;
	}

	// Returns true if the game is in the turning state, when players take their turns.
	// Returns false when in setup, processing or in completion.
	inline bool IsTurning() const
	{
		return m_State == Turning;
	}

	// Lists the types of the known game objects.
	static void ListGameTypes(QList<quint8>& gameTypes);

	// Returns the name of this game that appears in the game selection list.
	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the number of players in the game.
	inline int PlayerCount() const
	{
		int count = 0;
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
			if(!m_PlayerIds[playerIndex].IsNull())
				count++;
		return count;
	}

	inline const AuId& PlayerId(int playerIndex) const
	{
		Q_ASSERT(playerIndex >= 0 && playerIndex < MAX_GAME_PLAYERS);
		return m_PlayerIds[playerIndex];
	}

	inline const QString& PlayerName(int playerIndex) const
	{
		Q_ASSERT(playerIndex >= 0 && playerIndex < MAX_GAME_PLAYERS);
		return m_PlayerNames[playerIndex];
	}

	// Refresh game with corresponding game received from server without breaking client-only render values.
	virtual void Refresh(const AuGame* gameFromServer);

	// Removes a player from the game.
	// Returns false when the game is no longer playable.
	inline bool RemovePlayer(const AuId& personId)
	{
		// When the player whose turn it is gets removed, select the next player.
		bool selectNext = (IsTurning() && personId == TurnId());	// Wait until the end to select the next player.

		// Remove the player and check to see if any players remain.
		bool anyPlayersRemain = false;
		for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		{
			if(m_PlayerIds[playerIndex] == personId)
			{
				m_PlayerIds[playerIndex] = AuId(0);
				m_PlayerNames[playerIndex].clear();
			}
			else if(!m_PlayerIds[playerIndex].IsNull())
				anyPlayersRemain = true;
		}

		// Game is not playable if no players remain.
		if(!anyPlayersRemain)
			return false;

		// A game in the Setup state becomes invalid if the dealer leaves.
		if(m_State == Setup && m_PlayerIds[0].IsNull())
			return false;

		// Sequence to the next player when the current player is removed.
		if(selectNext)
			SelectNextPlayer();

		return true;
	}

	// Renders this game in the environment. 'model' is the transformation to the center of the table top, rotated to the table.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re) = 0;

	virtual void Rx(QDataStream& in);

	// Sets the seat index at the table, so the rotation of the game rendering can be calculated.
	inline void SetDealerSeat(quint8 seatIndex)
	{
		Q_ASSERT(seatIndex < MAX_GAME_PLAYERS);
		m_DealerSeat = seatIndex;
	}

	inline void SetPlayer(int playerIndex, const AuId& personId, const QString& personName)
	{
		Q_ASSERT(playerIndex >= 0 && playerIndex < MAX_GAME_PLAYERS);
		m_PlayerIds[playerIndex] = personId;
		m_PlayerNames[playerIndex] = personName;
	}

	// Starts this game.
	// Receives the blob of game settings that is emitted by the game setup dialog when the dealer presses 'Start Game'.
	// Returns false if the resulting setup is not valid.
	// The game state is not changed when the setup is not valid.
	// Base class returns false if current state is not Setup. Otherwise, sets state to Turning and returns true.
	virtual bool Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings);

	// Returns the radius of the game table top playing surface, in feet.
	inline static float TableTopRadius()
	{
		return 2.875f;							// Feet in radius to the edge of the table top.
	}

	// Called every other second after the game is started.
	// Returns 0=no change, 1=data changed, 2=game over
	// Base class does nothing and returns 0; there is no need to call it.
	virtual int Tick();

	// Called with a blob of data when a player completes their turn.
	// Returns false if no game data changes as a result.
	// Base class returns false if it's not personId's turn.
	virtual bool TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn);

	// Returns the id of the player whose turn it is now.
	// This may return a null id, permitting the game to function in a state where it is nobody's turn right now.
	// An example might be a slot machine after having pulled the lever. It is nobody's turn until the result is displayed.
	inline AuId TurnId() const
	{
		if(m_State == Turning && m_TurnIndex < MAX_GAME_PLAYERS)
			return m_PlayerIds[m_TurnIndex];
		else
			return AuId(0);
	}

	inline int TurnIndex() const
	{
		return m_TurnIndex;
	}

	// Returns the name of the current turn UI. Used to validate the ui is appropriate for the game state. UI object name should
	// vary with game state.
	// This base class returns "Name:State:TurnIndex".
	virtual QString TurnUIName() const;

	// Transmit only what the client needs to prevent cheating.
	virtual void Tx(QDataStream& out, const DaPerson& sceneAtom) const;

	// Returns a number that is unique to each derived game class.
	virtual quint8 Type() const;

protected:
	// Returns -1 if the player id is not found.
	inline int PlayerIndex(const AuId& personId) const
	{
		if(!personId.IsNull())
			for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
				if(m_PlayerIds[playerIndex] == personId)
					return playerIndex;
		return -1;	// Not found.
	}

	// Changes the current player index to the next valid player, or a null player if there are no other players.
	inline void SelectNextPlayer()
	{
		// Loop enough to wrap around but not quite reach the same player again.
		for(int tryNext=1; tryNext<MAX_GAME_PLAYERS; tryNext++)
		{
			m_TurnIndex++;
			if(m_TurnIndex >= MAX_GAME_PLAYERS)
				m_TurnIndex = 0;					// Wrap around.
			if(!m_PlayerIds[m_TurnIndex].IsNull())
				break;
		}
	}

	inline void SetState(STATE nuState)
	{
		m_State = nuState;
		m_StateTime.restart();
	}

	inline STATE State() const
	{
		return (STATE)m_State;
	}

	inline const AuElapsedTimer& StateTime() const
	{
		return m_StateTime;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGame& game)
{
	game >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGame& game)
{
	game << in;
	return in;
}

#endif // AuGame_H
