//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augamehilo.h,v 1.18 2022/02/19 17:31:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameHiLo_H
#define AuGameHiLo_H

#include "augame.h"
#include <QColor>
#include <augleffects.h>
#include <aupoint.h>

class AuGameHiLoSprite
{
public:
	QColor		m_Color;
	quint8		m_Digit;	// Digit displayed on the sprite.
	AuPoint		m_Loc;		// Location relative to center of game table top.
	float		m_Rot;
	float		m_Speed;

	AuGameHiLoSprite();
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameHiLo : public AuGame
{
private:
	quint32			m_Ante;								// Ante required to play the game.
	quint8			m_Digits;							// Number of digits that will be randomly generated.
	qint32			m_PlayerNumber[MAX_GAME_PLAYERS];	// The number chosen by each player. -1 indicates number not entered yet.
	qint32			m_RandomNumber;						// The final result.

	// Render data used on client. Not serialized.
	QList<AuGameHiLoSprite>	m_Sprites;					// Sprites on render client.

public:
	AuGameHiLo();
	virtual ~AuGameHiLo();
	AuGameHiLo(const AuGameHiLo& c);

	// Returns the ante.
	inline quint32 Ante() const
	{
		return m_Ante;
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

	// Returns the ante.
	inline quint8 Digits() const
	{
		return m_Digits;
	}

	// Returns a blob of the completion results of the game.
	virtual QByteArray Finish(DaPerson* players[MAX_GAME_PLAYERS]) const;

	// Static version is used when checking type read from a serial data stream.
	// This must match the type value returned by the virtual Type() function.
	inline static quint8 GameType()
	{
		return 1;
	}

	// Returns the icon of this game that appears in the game selection list.
	virtual QIcon Icon() const;

	// Returns the name of this game that appears in the game selection list.
	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Refresh game with corresponding game received from server without breaking client-only render values.
	virtual void Refresh(const AuGame* gameFromServer);

	// Renders this game in the environment. 'model' is the transformation to the center of the table top, rotated to the table.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	virtual void Rx(QDataStream& in);

	// Starts this game.
	// Receives the blob of game settings that is emitted by the game setup dialog when the dealer presses 'Start Game'.
	// Returns false if the resulting setup is not valid.
	// The game state is not changed when the setup is not valid.
	virtual bool Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings);

	// Called every other second after the game is started.
	// Returns 0=no change, 1=data changed, 2=game over
	virtual int Tick();

	// Called with a blob of data when a player completes their turn.
	// Returns false if no game data changes as a result.
	virtual bool TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn);

	// Transmit only what the client needs to prevent cheating.
	virtual void Tx(QDataStream& out, const DaPerson& sceneAtom) const;

	// Returns a number that is unique to each derived game class.
	virtual quint8 Type() const;

protected:
	void RenderSprites(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);
};

#endif // AuGameHiLo_H
