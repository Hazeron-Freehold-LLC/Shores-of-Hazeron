//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augamehilo.cpp,v 1.28 2022/02/19 17:39:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augamehilo.h"
//
#include "augamehiloui.h"
#include <auglobal.h>
#include "daperson.h"
#include <QIcon>
#include <QGridLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QSlider>

static AuGLVAO s_DigitVAO;				// GL model for displaying a digit.
static GLuint s_GameTexture = 0;		// HUDItems texture id

QColor s_DigitCol[] =
{
	QColor(255,0,0),
	QColor(0,255,0),
	QColor(0,0,255),
	QColor(255,255,0),
	QColor(0,255,255),
	QColor(255,0,255),
};
const int s_DigitCols = sizeof(s_DigitCol) / sizeof(QColor);

AuGameHiLoSprite::AuGameHiLoSprite()
{
	m_Color = s_DigitCol[g_Au.Dice().Roll(1,s_DigitCols,-1)];

	m_Digit = g_Au.Dice().Roll(1, 10, -1);				// 0-9

	float radius = g_Au.Dice().Roll(1,20)/10.f;			// Radius from center of table.
	float angle = g_Au.Dice().Roll(1,360,-1)*DEG2RAD;	// 0-359 degrees Z rotation, in radians
	m_Loc = AuPoint(radius,0).RotateZ(angle);			// Starting location for the sprite.

	m_Rot = g_Au.Dice().Roll(1,360,-1);					// 0-359 degrees Z rotation, in degrees

	m_Speed = g_Au.Dice().Roll(1,10) / 50.f;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameHiLo::AuGameHiLo()
{
	m_Ante = 100;
	m_Digits = 4;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		m_PlayerNumber[player] = -1;
	m_RandomNumber = 0;
}

AuGameHiLo::~AuGameHiLo()
{
}

AuGameHiLo::AuGameHiLo(const AuGameHiLo& c) : AuGame(c), m_Sprites(c.m_Sprites)
{
	m_Ante = c.m_Ante;
	m_Digits = c.m_Digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		m_PlayerNumber[player] = c.m_PlayerNumber[player];
	m_RandomNumber = c.m_RandomNumber;
}

void AuGameHiLo::operator<<(QDataStream& in)
{
	AuGame::operator<<(in);

	in >> m_Ante;
	in >> m_Digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		in >> m_PlayerNumber[player];
	in >> m_RandomNumber;
}

void AuGameHiLo::operator>>(QDataStream& out) const
{
	AuGame::operator>>(out);

	out << m_Ante;
	out << m_Digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		out << m_PlayerNumber[player];
	out << m_RandomNumber;
}

void AuGameHiLo::Rx(QDataStream& in)
{
	AuGame::Rx(in);

	in >> m_Ante;
	in >> m_Digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		in >> m_PlayerNumber[player];
	in >> m_RandomNumber;
}

void AuGameHiLo::Tx(QDataStream& out, const DaPerson& sceneAtom) const
{
	AuGame::Tx(out, sceneAtom);

	out << m_Ante;
	out << m_Digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		out << m_PlayerNumber[player];
	out << m_RandomNumber;
}

void AuGameHiLo::Refresh(const AuGame* gameFromServer)
{
	AuGame::Refresh(gameFromServer);
	const AuGameHiLo* game = dynamic_cast<const AuGameHiLo*>(gameFromServer);
	Q_ASSERT(game);

	// Refresh everything except client render data.
	m_Ante = game->m_Ante;
	m_Digits = game->m_Digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		m_PlayerNumber[player] = game->m_PlayerNumber[player];
	m_RandomNumber = game->m_RandomNumber;
}

AuGame* AuGameHiLo::Clone() const
{
	return new AuGameHiLo(*this);
}

QString AuGameHiLo::Description() const
{
	return QObject::tr("<html><img src=\"qrc:data/GameHiLo.png\"> <b>%1</b><br>"
			"Players bet on a random number.<br><br>"
			"<b>Rules</b><br>"
			"The dealer sets the ante and the number of digits that will be randomly generated.<br><br>"
			"Each player chooses a number.<br><br>"
			"After each player has chosen their number, "
			"a random number is generated with the number of digits specified by the dealer. "
			"The random number may have leading or trailing zeros, "
			"so a 5 digit number could produce 00120 as easily as 99999.<br><br>"
			"Ante is collected into the kitty at the time the random number is generated. "
			"Players who do not have enough money are disqualified at that time.<br><br>"
			"The player who choses the number that is closest to the random number, whether higher or lower, "
			"wins the money in the kitty. "
			"In case of a tie, the kitty is divided evenly between the winners."
			"</html>").arg(Name());
}

QIcon AuGameHiLo::Icon() const
{
	return QIcon(":data/GameHiLo.png");
}

QString AuGameHiLo::Name() const
{
	return QObject::tr("HiLo Number Game");
}

// Returns a number that is unique to each derived game class.
quint8 AuGameHiLo::Type() const
{
	return GameType();
}

AuDialog* AuGameHiLo::CreateFinishUI(const QByteArray& gameResult, QWidget* parent) const
{
	return new AuGameHiLoFinish(gameResult, parent);
}

AuDialog* AuGameHiLo::CreateSettingsUI(const DaPerson& person, QWidget* parent) const
{
	Q_UNUSED(person);
	return new AuGameHiLoSetup(*this, parent);
}

AuDialog* AuGameHiLo::CreateTurnUI(const DaPerson& person, QWidget* parent) const
{
	Q_UNUSED(person);
	return new AuGameHiLoTurn(*this, parent);
}

bool AuGameHiLo::Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings)
{
	// Deserialize the data in the game settings blob.
	QDataStream in(gameSettings);
	in.setVersion(QDataStream::Qt_4_1);

	// Bet ante.
	in >> m_Ante;
	if(m_Ante > 10000000)
		return false;						// Ante limited to 10M.

	// Random digits.
	in >> m_Digits;
	if(m_Digits < 1 || m_Digits > 6)
		return false;						// Invalid number of random digits to generate.

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
				players[player]->AccountBalance() < m_Ante)
		{
			// The player atom is no longer there.
			//  or
			// The player atom is not the person listed there in the game (not likely since we just updated it)
			//  or
			// The player id at that position in the game does not match the one read from the settings. Player probably kicked.
			//  or
			// The player doesn't have enough money to cover the ante.
			if(!player)
				return false;					// Player 0 left. The game is not valid.
			RemovePlayer(PlayerId(player));		// Remove the player from the game.
		}
	}

	// Do we still have enough players for a game?
	if(PlayerCount() < 2)
		return false;

	// Base class sets the turning state.
	return AuGame::Start(players, gameSettings);
}

// Returns 0=no change, 1=data changed, 2=game over
int AuGameHiLo::Tick()
{
	// The base class does nothing so there is no reason to call it.
	// Do something based on my current game state.
	switch(State())
	{
	case Setup:
		// Do nothing while waiting for setup to be compled.
		return 0;

	case Turning:
		// This game is still waiting for players to finish their turns. What if somebody left?
		// Check for all players' numbers to be submitted.
		for(int player=0; player<MAX_GAME_PLAYERS; player++)
			if(!PlayerId(player).IsNull() && m_PlayerNumber[player] < 0)
				return 0;	// No change. Still waiting for a player to enter their number.

		// All players have entered their numbers.
		// Start processing the result.
		SetState(Processing);
		return 1;		// Game state changed.

	case Processing:
		// Let the game grind away doing nothing for 5 seconds before completion.
		// Show some "drum roll" animation on the client during this time.
		if(StateTime().elapsed() < 5000)
			return 0;	// No change. Waiting for game processing to be complete.

		// Generate a random number with the number of digits specified.
		m_RandomNumber = 0;
		for(int dig=0; dig<m_Digits; dig++)
			m_RandomNumber = m_RandomNumber * 10 + g_Au.Dice().Roll(1, 10, -1);

		// Change to Fanfare state so the game table has enough time to graphically depict the result.
		SetState(Fanfare);
		return 1;		// Game state changed.

	case Fanfare:
		// Show a graphical representation of the result on the client at this time.
		if(StateTime().elapsed() < 3000)
			return 0;	// No change. Waiting for game processing to be complete.

		// This game is over.
		return 2;		// Game Over
	}

	// Game Over if we get this far. The state value must be invalid.
	qWarning() << Type() << Name() << "invalid game state" << State();
	return 2;
}

QByteArray AuGameHiLo::Finish(DaPerson* players[MAX_GAME_PLAYERS]) const
{
	// Take ante from the players. Invalidate the ones that don't have the money.
	double kitty = 0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(players[player])
		{
			if(!players[player]->Is(PlayerId(player)) || players[player]->AccountBalance() < m_Ante)
				players[player] = 0;							// Delist switched players and players that don't have the money.
			else
			{
				players[player]->DebitAccount(m_Ante);		// Take the ante from their account.
				kitty += m_Ante;
			}
		}

	// Choose a winner.
	int diff[MAX_GAME_PLAYERS];
	int bestDiff = 100000000;	// invalid because it's more than the max digits allowed
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(players[player])
		{
			diff[player] = labs(m_PlayerNumber[player] - m_RandomNumber);
			if(diff[player] < bestDiff)
				bestDiff = diff[player];
		}

	// List the players and their guesses. List the winners; there could be ties.
	QStringList playerNames;
	QList<quint32> playerNumbers;
	QStringList winnerNames;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(players[player])
		{
			playerNames.append(PlayerName(player));
			playerNumbers.append(m_PlayerNumber[player]);
			if(diff[player] == bestDiff)
				winnerNames.append(PlayerName(player));		// This guy's a winner!
		}

	// Calculate each winner's share of the kitty.
	double share;
	if(winnerNames.count() == 1)
		share = kitty;
	else
		share = kitty / winnerNames.count();

	// Give each player their share of the winnings now.
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(players[player] && diff[player] == bestDiff)
			players[player]->CreditAccount(share);

	// Pack the final results into a blob.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << m_RandomNumber;
	out << m_Digits;
	out << share;
	out << (quint8)winnerNames.count();
	for(QStringList::const_iterator wni = winnerNames.constBegin(); wni != winnerNames.constEnd(); wni++)
		out << *wni;
	out << (quint8)playerNames.count();
	for(int player=0; player<playerNames.count(); player++)
	{
		out << playerNames[player];
		out << playerNumbers[player];
	}
	return ba;
}

bool AuGameHiLo::TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn)
{
	// Base class verifies that it is actually personId's turn.
	if(!AuGame::TurnComplete(person, players, gameTurn))
		return false;					// Must not be personId's turn.

	int playerIndex = PlayerIndex(person->Id());
	if(playerIndex < 0)
		return false;					// That person is not in this game.

	// Take the ante from the person.
	if(person->AccountBalance() < m_Ante)
		return false;					// The person doesn't have the money.

	// Deserialize the data in the game turn blob.
	QDataStream in(gameTurn);
	in.setVersion(QDataStream::Qt_4_1);
	in >> m_PlayerNumber[playerIndex];	// The number chosen by the player.

	// Cycle the turn index to the next player.
	SelectNextPlayer();

	// Check for all players' turns to be completed.
	Tick();							// Tick function changes state to processing when all players are done entering data.
	return true;
}

void AuGameHiLo::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	// Insure my texture is loaded and objects are created.
	if(!s_GameTexture)
	{
		// The HUDItems texture has a set of 10 digits.
		s_GameTexture = re.HUDItemsTexture();

		// Variables used for calculating texture coordinates in s_GameTexture.
		const float himaxx = 1024;	// width of the texture
		const float himaxy = 64;	// height of the texture

		/*
		float s = ((float)digval*22.8f) / himaxx;
		float t = 15.5f / himaxy;
		float u = ((float)(digval+1)*22.8f) / himaxx;
		float v = 47.5f / himaxy;
		*/
		float s = 0;
		float t = 15.5f / himaxy;
		float u = 22.8f / himaxx;
		float v = 47.5f / himaxy;

		// Create a model for displaying each digit.
		// Quad, facing down the -Y axis.
		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-1,0,-1, s,t);
		tri0.m_V1 = AuPointTex(-1,0,1, s,v);
		tri0.m_V2 = AuPointTex(1,0,1, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,0,1, u,v);
		tri1.m_V1 = AuPointTex(1,0,-1, u,t);
		tri1.m_V2 = AuPointTex(-1,0,-1, s,t);

		// Quad, facing down the +Y axis.
		AuTriTex tri2;
		tri2.m_V0 = AuPointTex(1,0,-1, s,t);
		tri2.m_V1 = AuPointTex(1,0,1, s,v);
		tri2.m_V2 = AuPointTex(-1,0,1, u,v);
		AuTriTex tri3;
		tri3.m_V0 = AuPointTex(-1,0,1, u,v);
		tri3.m_V1 = AuPointTex(-1,0,-1, u,t);
		tri3.m_V2 = AuPointTex(1,0,-1, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1 << tri2 << tri3;

		s_DigitVAO.CreateForTexture(re, tris, GL_STATIC_DRAW);
	}

	re.GLSL().TexOffsetShader().Select();
	re.GLSL().TexOffsetShader().SetTexture(s_GameTexture);
	re.GLSL().TexOffsetShader().SetDiscardAlpha(.9f);

	RenderSprites(projection, model, re);

	re.GLSL().TexOffsetShader().Deselect();
}

void AuGameHiLo::RenderSprites(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	float scale;
	QList<AuGameHiLoSprite>::iterator si = m_Sprites.begin();
	while(si != m_Sprites.end())
	{
		// Render the digit sprite.
		switch(State())
		{
		case Setup:
			// Float upwards.
			(*si).m_Loc.z += (*si).m_Speed/2.;

			// Spin.
			(*si).m_Rot += 10;

			// Scale.
			scale = .05f;

			re.GLSL().TexOffsetShader().SetColor((*si).m_Color.redF(), (*si).m_Color.greenF(), (*si).m_Color.blueF(), 1);
			break;

		case Turning:
			// Float upwards.
			(*si).m_Loc.z += (*si).m_Speed;

			// Spin.
			(*si).m_Rot += 10;

			// Scale.
			scale = .1f;

			re.GLSL().TexOffsetShader().SetColor((*si).m_Color.redF(), (*si).m_Color.greenF(), (*si).m_Color.blueF(), 1);
			break;

		case Processing:
			// Randomly select digits.
			(*si).m_Digit = g_Au.Dice().Roll(1,10,-1);

			// Float upwards.
			(*si).m_Loc.z += (*si).m_Speed;
			(*si).m_Speed += .01f;			// Accelerate when processing.

			// Spin.
			(*si).m_Rot += 10;

			// Scale
			scale = .15f;

			re.GLSL().TexOffsetShader().SetColor(0, .1f + (*si).m_Digit/10.f, 0, 1);
			break;

		case Fanfare:
			// Float upwards.
			(*si).m_Loc.z += (*si).m_Speed;

			// Spin.
			(*si).m_Rot++;

			// Scale
			scale = .3f;

			re.GLSL().TexOffsetShader().SetColor((*si).m_Color.redF(), (*si).m_Color.greenF(), (*si).m_Color.blueF(), 1);
			break;

		default:
			continue;
		}

		// Disappear when the sprite has gone too high, before we draw it too high.
		if((*si).m_Loc.z > 5)
		{
			si = m_Sprites.erase(si);
			continue;
		}

		// Render the sprite.
		AuMatrix4x4 digm(model);
		digm.Translate((*si).m_Loc);
		digm.RotateZ((*si).m_Rot*DEG2RAD);
		digm.Scale(scale, scale, scale);
		re.GLSL().TexOffsetShader().SetProjectionModelView(projection * digm);
		const float himaxx = 1024;	// width of the texture
		float offset = ((float)(*si).m_Digit*22.8f) / himaxx;
		re.GLSL().TexOffsetShader().SetTextureOffset(offset, 0);

		s_DigitVAO.SelectGL();
		s_DigitVAO.DrawGL();
		s_DigitVAO.DeselectGL();

		si++;
	}

	// Add more sprites.
	if(State() == Fanfare)
	{
		if(m_Sprites.count() > m_Digits)
		{
			// Must have just changed to Fanfare state. Arrange sprites to show final result.
			m_Sprites.clear();
			float rot = g_Au.Dice().Roll(1,360,-1);
			QString result = QString("%1").arg(m_RandomNumber, m_Digits, 10, QChar('0'));
			for(int dig=0; dig<m_Digits; dig++)
			{
				AuGameHiLoSprite sprite;
				sprite.m_Color = QColor(255, 166, 3);
				sprite.m_Digit = QString(result[dig]).toInt();
				sprite.m_Loc.x = 0;
				sprite.m_Loc.y = 0;
				sprite.m_Loc.z = (m_Digits * .6) - (dig * .6);
				sprite.m_Rot = rot;
				sprite.m_Speed = 0;
				m_Sprites.append(sprite);
			}
		}
	}
	else if(m_Sprites.count() < 10)
	{
		int add = g_Au.Dice().Roll(1,3,-1);
		while(add--)
			m_Sprites.append(AuGameHiLoSprite());
	}
}
