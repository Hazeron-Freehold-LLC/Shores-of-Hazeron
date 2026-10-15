//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameclonepok.h,v 1.7 2022/02/19 17:31:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameClonepok_H
#define AuGameClonepok_H

#include "augame.h"
#include "augameplayingcard.h"
#include "augleffects.h"
#include "aupoint.h"
#include <QColor>

class AuItem;
class DpBody;

class AuGameClonepokTile
{
public:
	enum TYPE
	{
		Clone,
		Power,
		Action
	};

	enum POWER
	{
		Neutral,
		Atmospheric,
		Biological,
		Chemical,
		Electromagnetic,
		Kinetic,
		Meat,
		Plant,
		Projectile,
		Psionic,
		Radial,
		Sonic,
		Thermal,
		Water,
	};

public:
	quint32								m_HitPoints;			// Hit points remaining to this tile.
	quint8								m_Id;					// Unique to each tile. 0 is invalid.
	AuItem*								m_Item;					// Gear item corresponding to this tile.
	quint8								m_Player;				// Index of player who owns this tile.
	quint8								m_Space;				// Location of tile in map spaces.
																// Coastal spaces are 0-7, starting at 0 degrees, counterclockwise.
																// Inland spaces are 8-11, starting at 45 degrees, counterclockwise.
																// Cone space is 12.

	// Transient render data.
	DpBody*								m_pBody;
	QPoint								m_MapLoc;

public:
	AuGameClonepokTile(quint8 space, quint8 unitId, quint8 playerIndex, const AuItem& item);

	AuGameClonepokTile()
	{
		m_HitPoints = 0;
		m_Id = 0;
		m_Item = 0;
		m_pBody = 0;
		m_Player = 0;
		m_Space = 0;
	}

	AuGameClonepokTile(const AuGameClonepokTile& c);

	~AuGameClonepokTile();

public:	// Operators
	const AuGameClonepokTile& operator=(const AuGameClonepokTile& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	void Refresh(const AuGameClonepokTile& u)
	{
		// Only refresh variables expected to change during the course of a game.
		m_HitPoints = u.m_HitPoints;
		m_Player = u.m_Player;
		m_Space = u.m_Space;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGameClonepokTile& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGameClonepokTile& o)
{
	o << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuGameClonepokTile& o);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameClonepokTileList : public QList<AuGameClonepokTile>
{
private:
	mutable quint8						m_NextId;

public:	// Constructors
	inline AuGameClonepokTileList() : QList<AuGameClonepokTile>()
	{
		m_NextId = 0;
	}

	inline AuGameClonepokTileList(const AuGameClonepokTileList& c) : QList<AuGameClonepokTile>(c)
	{
		m_NextId = c.m_NextId;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_NextId;
		out << (quint8)count();
		for(const_iterator ui = cbegin(); ui != cend(); ui++)
			out << *ui;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		in >> m_NextId;

		quint8 c;
		in >> c;
		while(c--)
		{
			AuGameClonepokTile u;
			in >> u;
			append(u);
		}
	}

public:	// Methods
	inline void CreateNewTile(quint8 space, quint8 playerIndex, const AuItem& item)
	{
		append(AuGameClonepokTile(space, NextId(), playerIndex, item));
	}

	inline const_iterator FindId(quint8 unitId) const
	{
		for(const_iterator ui = cbegin(); ui != cend(); ui++)
			if((*ui).m_Id == unitId)
				return ui;
		return cend();
	}

	inline quint8 NextId() const
	{
		// This game can never spawn more than 10 clones per player so we don't have to worry about roll over.
		m_NextId++;
		return m_NextId;
	}

	inline void Refresh(const AuGameClonepokTileList& o)
	{
		// Refresh each of my units with its incoming corresponding unit.
		// Remove my units that no longer exist.
		iterator ui = begin();
		while(ui != end())
		{
			const_iterator oi = o.FindId((*ui).m_Id);
			if(oi != o.cend())
			{
				// Found incoming unit that corresponds to my unit; refresh it.
				(*ui).Refresh(*oi);
				ui++;
			}
			else
				ui = erase(ui);	// No incoming unit corresponds to my unit; remove it.
		}

		// Add new units that arrived.
		for(const_iterator oi = o.cbegin(); oi != o.cend(); oi++)
			if(FindId((*oi).m_Id) == cend())
				append(*oi);	// Add incoming unit that is not in my list.
	}

	inline void Tx(QDataStream& out) const
	{
		out << (quint8)count();
		for(const_iterator ui = cbegin(); ui != cend(); ui++)
			out << *ui;
	}

	inline void Rx(QDataStream& in)
	{
		clear();
		m_NextId = 0;

		quint8 c;
		in >> c;
		while(c--)
		{
			AuGameClonepokTile u;
			in >> u;
			append(u);
		}
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGameClonepokTileList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGameClonepokTileList& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameClonepok : public AuGame
{
public:
	enum TURNSTATE
	{
		StackBuilding,					// Players build stacks of tiles from corresponding gear.
		Landfall,						// Players draw initial hand and play one to six Basic clones to their beachhead.
		Turn,							// Players do turns clockwise until someone wins, starting with random player.
	};

private:
	quint8								m_BasicClones;			// Starting minimum number of Basic clones in players' stacks.
	quint32								m_Pot[MAX_GAME_PLAYERS];	// Pot contributions by player. Total is the entire pot.
	AuGameClonepokTileList				m_Stacks;				// All tiles in players' stacks.
	quint8								m_StackSize;			// Number of tiles each player must have in their stack to start.
	AuGameClonepokTileList				m_TilesInPlay;			// All tiles on the game board.
	quint8								m_TreasureTileAnte;		// Number of treasure tiles each player puts into the pot.
	AuGameClonepokTileList				m_TreasureTiles;		// Tiles in the treasure pile.
	quint8								m_TurnState;			// Current game TURNSTATE.

public:	// Constructors
	AuGameClonepok();
	virtual ~AuGameClonepok();
	AuGameClonepok(const AuGameClonepok& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// Returns the starting minimum number of Basic clone tiles.
	inline quint8 BasicClones() const
	{
		return m_BasicClones;
	}

	// Returns the current bet of a player.
	inline quint32 Bet(const AuId& personId) const
	{
		int playerIndex = PlayerIndex(personId);
		if(playerIndex < 0)
			return 0;	// Player is not in this game.
		return m_Pot[playerIndex];
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

	// Static version is used when checking type read from a serial data stream.
	// This must match the type value returned by the virtual Type() function.
	inline static quint8 GameType()
	{
		return 3;
	}

	// Returns the icon of this game that appears in the game selection list.
	virtual QIcon Icon() const;

	// Returns the name of this game that appears in the game selection list.
	virtual QString Name() const;

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

	virtual void Rx(QDataStream& in);

	// Returns the starting stack size for all players.
	inline quint8 StackSize() const
	{
		return m_StackSize;
	}

	// Starts this game.
	// Receives the blob of game settings that is emitted by the game setup dialog when the dealer presses 'Start Game'.
	// Returns false if the resulting setup is not valid.
	// The game state is not changed when the setup is not valid.
	virtual bool Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings);

	// Called every other second after the game is started.
	// Returns 0=no change, 1=data changed, 2=game over
	virtual int Tick();

	// All tiles on the game board.
	inline const AuGameClonepokTileList& TilesInPlay() const
	{
		return m_TilesInPlay;
	}

	// Returns the starting minimum number of Basic clone tiles.
	inline quint8 TreasureTileAnte() const
	{
		return m_TreasureTileAnte;
	}

	// Called with a blob of data when a player completes their turn.
	// Returns false if no game data changes as a result.
	virtual bool TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn);

	// Returns the current game phase.
	inline TURNSTATE TurnState() const
	{
		return (TURNSTATE)m_TurnState;
	}

	// Returns the name of the current turn UI. Used to validate the ui is appropriate for the game state. UI object name should
	// vary with game state.
	virtual QString TurnUIName() const;

	// Transmit only what the client needs to prevent cheating.
	virtual void Tx(QDataStream& out, const DaPerson& sceneAtom) const;

	// Returns a number that is unique to each derived game class.
	virtual quint8 Type() const;

protected:
	void RenderBets(const AuMatrix4x4& model, AuGLEffects& re, bool useFog);

	void RenderModel(const AuMatrix4x4& model, AuGLVAO& vao, float locX, float locY, float locZ, float rotX, float rotY, float rotZ,
			AuGLEffects& re, bool useFog);
};

#endif // AuGameClonepok_H
