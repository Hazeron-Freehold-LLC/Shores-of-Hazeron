//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameclonepok.cpp,v 1.13 2022/02/19 17:39:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augameclonepok.h"
//
#include "augameclonepokui.h"
#include "auglobal.h"
#include "auitem.h"
#include "daperson.h"
#include <QDebug>
#include <QIcon>

static GLuint	s_TableTopTex = 0;		// GameClonepokBoard.png texture id
static AuGLVAO	s_TableTopModel;		// GL model of the surface of the game table.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepokTile::AuGameClonepokTile(quint8 space, quint8 unitId, quint8 playerIndex, const AuItem& item)
{
	m_HitPoints = 0;	// Unknown until played.
	m_Id = unitId;
	m_Item = item.Clone();
	m_pBody = 0;
	m_Player = playerIndex;
	m_Space = space;
}

AuGameClonepokTile::AuGameClonepokTile(const AuGameClonepokTile& c)
{
	m_HitPoints = c.m_HitPoints;
	m_Id = c.m_Id;
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
	m_MapLoc = c.m_MapLoc;
	m_pBody = 0;
	m_Player = c.m_Player;
	m_Space = c.m_Space;
}

AuGameClonepokTile::~AuGameClonepokTile()
{
	if(m_Item)
		delete m_Item;
	if(m_pBody)
		delete m_pBody;
}

const AuGameClonepokTile& AuGameClonepokTile::operator=(const AuGameClonepokTile& c)
{
	m_HitPoints = c.m_HitPoints;
	m_Id = c.m_Id;
	if(m_Item)
		delete m_Item;
	if(m_pBody)
		delete m_pBody;
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
	m_MapLoc = c.m_MapLoc;
	m_pBody = 0;
	m_Player = c.m_Player;
	m_Space = c.m_Space;
	return *this;
}

void AuGameClonepokTile::operator>>(QDataStream& out) const
{
	out << m_HitPoints;
	out << m_Id;
	out << m_Player;
	out << m_Space;

	if(m_Item)
	{
		out << (quint8)1;
		out << m_Item->Pack();
	}
	else
		out << (quint8)0;
}

void AuGameClonepokTile::operator<<(QDataStream& in)
{
	in >> m_HitPoints;
	in >> m_Id;
	in >> m_Player;
	in >> m_Space;

	quint8 hasItem;
	in >> hasItem;
	if(hasItem)
	{
		QByteArray ba;
		in >> ba;
		m_Item = AuItem::Unpack(ba);
	}
	else
		m_Item = 0;
}

QDebug operator<<(QDebug dbg, const AuGameClonepokTile& o)
{
	if(o.m_Item)
		dbg.nospace() << "id " << o.m_Id << " player " << o.m_Player << " " << o.m_HitPoints << "hp " << " space " << o.m_Space
			<< " item " << o.m_Item->ListDesc();
	else
		dbg.nospace() << "id " << o.m_Id << " player " << o.m_Player << " " << o.m_HitPoints << "hp " << " space " << o.m_Space;
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepok::AuGameClonepok()
{
	m_BasicClones = 3;
	m_StackSize = 30;
	m_TreasureTileAnte = 3;
	m_TurnState = StackBuilding;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		m_Pot[playerIndex] = 0;
}

AuGameClonepok::~AuGameClonepok()
{
}

AuGameClonepok::AuGameClonepok(const AuGameClonepok& c) : AuGame(c), m_Stacks(c.m_Stacks), m_TilesInPlay(c.m_TilesInPlay),
	m_TreasureTiles(c.m_TreasureTiles)
{
	m_BasicClones = c.m_BasicClones;
	m_StackSize = c.m_StackSize;
	m_TreasureTileAnte = c.m_TreasureTileAnte;
	m_TurnState = c.m_TurnState;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		m_Pot[playerIndex] = c.m_Pot[playerIndex];
}

void AuGameClonepok::operator>>(QDataStream& out) const
{
	AuGame::operator>>(out);

	out << m_BasicClones;
	out << m_Stacks;
	out << m_StackSize;
	out << m_TilesInPlay;
	out << m_TreasureTileAnte;
	out << m_TreasureTiles;
	out << m_TurnState;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		out << m_Pot[playerIndex];
}

void AuGameClonepok::operator<<(QDataStream& in)
{
	AuGame::operator<<(in);

	in >> m_BasicClones;
	in >> m_Stacks;
	in >> m_StackSize;
	in >> m_TilesInPlay;
	in >> m_TreasureTileAnte;
	in >> m_TreasureTiles;
	in >> m_TurnState;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		in >> m_Pot[playerIndex];
}

void AuGameClonepok::Tx(QDataStream& out, const DaPerson& sceneAtom) const
{
	AuGame::Tx(out, sceneAtom);

	out << m_BasicClones;
	out << m_StackSize;
	out << m_TreasureTileAnte;
	out << m_TurnState;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		out << m_Pot[playerIndex];

	m_Stacks.Tx(out);
	m_TilesInPlay.Tx(out);
	m_TreasureTiles.Tx(out);
}

void AuGameClonepok::Rx(QDataStream& in)
{
	AuGame::Rx(in);

	in >> m_BasicClones;
	in >> m_StackSize;
	in >> m_TreasureTileAnte;
	in >> m_TurnState;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		in >> m_Pot[playerIndex];

	m_Stacks.Rx(in);
	m_TilesInPlay.Rx(in);
	m_TreasureTiles.Rx(in);
}

void AuGameClonepok::Refresh(const AuGame* gameFromServer)
{
	AuGame::Refresh(gameFromServer);
	const AuGameClonepok* game = dynamic_cast<const AuGameClonepok*>(gameFromServer);
	Q_ASSERT(game);

	// Refresh everything except client render data.
	m_BasicClones = game->m_BasicClones;
	m_Stacks.Refresh(game->m_Stacks);
	m_StackSize = game->m_StackSize;
	m_TilesInPlay.Refresh(game->m_TilesInPlay);
	m_TreasureTileAnte = game->m_TreasureTileAnte;
	m_TreasureTiles.Refresh(game->m_TreasureTiles);
	m_TurnState = game->m_TurnState;

	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		m_Pot[playerIndex] = game->m_Pot[playerIndex];
}

AuGame* AuGameClonepok::Clone() const
{
	return new AuGameClonepok(*this);
}

QString AuGameClonepok::Description() const
{
	return QObject::tr("<html><img src=\"qrc:data/GameClonepokIcon.png\"> <b>%1</b>"
			"<p>Players pit creatures against each other in a struggle to conquer an island. "
			"Players clone creatures for their horde using DNA samples in their possession. "
			"</p>"

			"<b>Rules</b>"

			"<p><i>Terrain</i><br>"
			"The game board depicts a volcanic island divided into thirteen spaces. "
			"The center space is the cone of a dormant volcano; it is the highest altitude terrain. "
			"The eight spaces around the outside edge are coastal terrain, which is the lowest altitude. "
			"The remaining four spaces between coastal terrain and the cone are inland terrain, at mid altitude. "
			"</p>"

			"<p><i>Setup</i><br>"
			"The dealer configures the game. "
			"Settings include ante for all players, the cost of cloning new creatures and game limits. "
			"The dealer also chooses a DNA sample kit to be given to the winner, see <i>Victory</i>. "
			"This DNA sample kit is lost if the game ends without a winner. "
			"</p>"

			"<p><i>Turns</i><br>"
			"Each player takes a turn, starting with the first player to the left of the dealer. "
			"In turn, each player must do one of two things, clone a creature or move a horde. "
			"Turn sequence is clockwise, passing to the left as each player completes their turn. "
			"</p>"

			"<p><i>Cloning</i><br>"
			"A player clones a new creature for their horde by selecting a DNA sample in their possession. "
			"The DNA sample is not consumed though it could wear down due to usage; the quality of the DNA sample affects this. "
			"A DNA sample may be used again in a later turn to clone another creature of the same kind. "
			"The cloned creature enters the coastal terrain in front of the player's chair. "
			"Battle ensues if the space is occupied by an opponent. "
			"</p>"

			"<p><i>Moving</i><br>"
			"A player moves a horde by choosing one or more of their creatures on the board. "
			"The chosen creatures do not have to occupy the same space. "
			"</p>"

			"<p>The player then chooses a space where the horde is to move. "
			"All chosen creatures will move to the destination space if they can. "
			"Creatures can move only one space; they will not move if they are farther away. "
			"Aquatic creatures can only move to coastal terrain. "
			"Battle ensues if the destination space is occupied by an opponent. "
			"</p>"

			"<p><i>Battle</i><br>"
			"Battle is resolved when creatures move into a space containing opposing creatures. "
			"Non-flying creatures on higher terrain have a combat advantage over non-flying creatures on lower terrain. "
			"Terrain of each attacking creature is the space it moves from, coastal terrain for new clones. "
			"Destroyed creatures are removed from play. "
			"Surviving creatures remain to occupy the terrain. "
			"</p>"

			"<p><i>Defeat</i><br>"
			"A player is defeated when they have no creatures remaining on the board after a battle. "
			"Money paid into the pot stays there. "
			"A defeated player is skipped in subsequent turns, though they may remain at the table to watch. "
			"</p>"

			"<p><i>Forfeit</i><br>"
			"Players may depart an active game at any time. "
			"Money paid into the pot stays there. "
			"A player who forfeits the game is skipped in subsequent turns, though they may remain at the table to watch. "
			"</p>"

			"<p><i>Victory</i><br>"
			"Victory is achieved when only one player remains in the game after the last opponent is defeated or forfeits. "
			"The winner collects the pot, if any. "
			"</p>"

			"<p>In addition, the winner may take a DNA sample of one creature that was cloned in the game. "
			"The DNA sample kit set up by the dealer is used to supply the winner's sample; "
			"it is not taken from the gear of a player. "
			"This does not cause additional wear on the DNA sample that was originally used. "
			"Any DNA can be sampled that was used in the game, regardless if the player is still present who used it. "
			"</p>"

			"</html>").arg(Name());
}

QIcon AuGameClonepok::Icon() const
{
	return QIcon(":data/GameClonepokIcon.png");
}

QString AuGameClonepok::Name() const
{
	return QObject::tr("Clonēpok");
}

// Returns a number that is unique to each derived game class.
quint8 AuGameClonepok::Type() const
{
	return GameType();
}

AuDialog* AuGameClonepok::CreateFinishUI(const QByteArray& gameResult, QWidget* parent) const
{
	return new AuGameClonepokFinish(gameResult, parent);
}

AuDialog* AuGameClonepok::CreateSettingsUI(const DaPerson& person, QWidget* parent) const
{
	return new AuGameClonepokSetup(person, *this, parent);
}

AuDialog* AuGameClonepok::CreateTurnUI(const DaPerson& person, QWidget* parent) const
{
	return new AuGameClonepokTurn(person, *this, parent);
}

QString AuGameClonepok::TurnUIName() const
{
	// UI is probably not valid if turn state changes.
	return AuGame::TurnUIName() + QString(":%1").arg(m_TurnState);
}

bool AuGameClonepok::Start(DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameSettings)
{
	// Deserialize the data in the game settings blob.
	QDataStream in(gameSettings);
	in.setVersion(QDataStream::Qt_4_1);

	// Bet ante.
	quint32 ante;
	in >> ante;
	if(ante > 100000000)
		return false;						// Ante limited to 100M.

	// Basic clones required.
	in >> m_BasicClones;
	if(m_BasicClones < 1 || m_BasicClones > 6)
		return false;

	// Stack size.
	in >> m_StackSize;
	if(m_StackSize < 10 || m_StackSize > 60)
		return false;

	// Treasure cards.
	in >> m_TreasureTileAnte;
	if(m_TreasureTileAnte < 1 || m_TreasureTileAnte > 6)
		return false;

	// Players need enough money to ante up.
	quint32 moneyToStart = ante;

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
				players[player]->AccountBalance() < moneyToStart)
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
	//DHH if(PlayerCount() < 2)
		//DHH return false;

	/*
	// Set up DNA sample kit for winner.
	DaPerson* person = players[0];
	if(!person)
		return false;

	// Get the DNA sample kit.
	if(kit.canConvert<AuGearPos>())
	{
		// A null gear pos indicates no sample kit. The winner will not get a DNA sample.
		AuGearPos gpos = kit.value<AuGearPos>();
		if(gpos.Slot() != AuGearSlot::Invalid)
		{
			// Kit is in the dealer's gear.
			AuGear* gear = person->LockGearForWrite();
			const AuItem* item = gear->ItemAt(gpos);
			if(item && item->CommodityId() == CID_DNASampleKit)
			{
				m_SampleKit = item->Clone();
				gear->Erase(gpos);
				person->UpdateGearWeight();
				person->UnlockGear();
			}
			else
			{
				person->UnlockGear();
				return false;		// This failure should not continue.
			}
		}
	}
	else if(kit.canConvert<QByteArray>())
	{
		// Unpack the sample kit from the byte array.
		m_SampleKit = AuItem::Unpack(kit.toByteArray());
		m_SampleKit->SetCount(1);	// We only need one of them.

		// Remove the sample kit from the ship or city.
		bool gotOne = false;
		DaPerson* person = players[0];
		if(person && m_SampleKit)
		{
			if(person->IsInSpacecraft())
			{
				// Kit is in the dealer ship's hold.
				DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
					gotOne = ship->RemoveCargo(*m_SampleKit, 1);
			}
		}

		// If we did not get one from stock, delete the kit. Somebody is trying to pull a fast one maybe.
		if(!gotOne)
		{
			delete m_SampleKit;
			m_SampleKit = 0;
			return false;		// This failure should not continue.
		}
	}
	*/

	// Start up players.
	for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
	{
		if(ante > 0)
		{
			DaPerson* person = players[playerIndex];
			if(person)
				person->DebitAccount(ante);						// Take the ante from their account.
		}
		m_Pot[playerIndex] = ante;
	}

	// Base class sets the game into 'Turning' state.
	return AuGame::Start(players, gameSettings);
}

// Returns 0=no change, 1=data changed, 2=game over
int AuGameClonepok::Tick()
{
	// The base class does nothing so there is no reason to call it.
	// Do something based on my current game state.
	switch(State())
	{
	case Setup:
		// Do nothing while waiting for setup to be done.
		return 0;

	case Turning:
		// Do nothing while waiting for player to decide to clone or move.
		return 0;

	case Processing:
		//DHH Move horde and resolve combat.
		SetState(Turning);
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

QByteArray AuGameClonepok::Finish(DaPerson* players[MAX_GAME_PLAYERS]) const
{
	/*
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
	*/
	QByteArray ba;
	return ba;
}

bool AuGameClonepok::TurnComplete(DaPerson* person, DaPerson* players[MAX_GAME_PLAYERS], const QByteArray& gameTurn)
{
	// Base class verifies that it is actually personId's turn.
	if(!AuGame::TurnComplete(person, players, gameTurn))
		return false;					// Must not be personId's turn.

	int playerIndex = PlayerIndex(person->Id());
	if(playerIndex < 0)
		return false;					// That person is not in this game.

	// Deserialize the data in the game turn blob.
	QDataStream in(gameTurn);
	in.setVersion(QDataStream::Qt_4_1);

	quint8 doWhat;
	in >> doWhat;

	/*
	if(doWhat == 1)
	{
		// Clone a creature.
		DhDNA dna;
		
		in >> dna;

		quint8 playerSpaces[MAX_GAME_PLAYERS] = {6, 4, 2, 0};
		m_Tiles.CreateNewTile(playerSpaces[playerIndex], playerIndex, dna);

		// Cycle the turn index to the next player.
		SelectNextPlayer();
	}
	else if(doWhat == 2)
	{
		// Move a horde.
		quint8 toSpace;
		QSet<quint8> moveIds;

		in >> toSpace;
		in >> moveIds;

		//DHH Set up to move and resolve combat in Tick.

		SetState(Processing);
	}
	else
		return false;	// Bogus
		*/

	return true;
}

void AuGameClonepok::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	// Insure my texture is loaded and objects are created.
	if(!s_TableTopTex)
		s_TableTopTex = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Clonepok", ":data/GameClonepokBoard.png");

	// Create table top model.
	if(!s_TableTopModel.VAO())
	{
		const float topDrop = -2.f * 0.0032808399f;		// Drop from 0 plane, to keep cards from bleeding through.

		QList<AuTriTexNS> tris;

		AuPoint n(0,0,1);
		double pvx = 0;
		double pvy = 0;
		float ptx = 0;
		float pty = 0;
		for(int facet=16; facet>=0; facet--)
		{
			float angle;
			if(facet == 16)
				angle = 0;
			else
				angle = (PIf*2) * facet / 16;
			float ca = cosf(angle);
			float sa = sinf(angle);
			float vx = ca * AuGame::TableTopRadius();
			float vy = sa * AuGame::TableTopRadius();
			float tx = .5f + ca * .5f;
			float ty = .5f + sa * .5f;

			if(facet < 16)
				tris << AuTriTexNS(AuPointTexNS(0,0,topDrop, .5f,.5f, n, 1),
						AuPointTexNS(pvx,pvy,topDrop, ptx,pty, n, 1),
						AuPointTexNS(vx,vy,topDrop, tx,ty, n, 1));

			pvx = vx;
			pvy = vy;
			ptx = tx;
			pty = ty;
		}

		s_TableTopModel.CreateForModelTexFog(re, tris, GL_STATIC_DRAW);
	}

	// Set up for rendering objects that are lit using environmental lighting and fog, such as table, buttons, cards and chips.
	// Select vertex shader and configure its parameters.
	const float minAmbient = .5f;
	bool useFog = re.HasFog();
	AuPoint ambientUp = model.ProjectVector(0,0,1);
	if(useFog)
	{
		re.GLSL().ModelTexFogShader().Select(projection, re);
		re.GLSL().ModelTexFogShader().SetModelView(model);
		re.GLSL().ModelTexFogShader().SetTexture(s_TableTopTex);

		// Ambient light
		float ambientR, ambientG, ambientB, ambientA;
		re.CurrentAmbientLight(ambientR, ambientG, ambientB, ambientA);
		// Impose a minimum ambience so the table and chips glow in dim light.
		if(ambientR < minAmbient)
			ambientR = minAmbient;
		if(ambientG < minAmbient)
			ambientG = minAmbient;
		if(ambientB < minAmbient)
			ambientB = minAmbient;
		re.GLSL().ModelTexFogShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().ModelTexFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Fog
		float fogR, fogG, fogB, fogDensity;
		re.CurrentFog(fogR, fogG, fogB, fogDensity);
		re.GLSL().ModelTexFogShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Render the table top.
		s_TableTopModel.SelectGL();
		s_TableTopModel.DrawGL();
		s_TableTopModel.DeselectGL();

		// Render bets as stacks of chips.
		RenderBets(model, re, true);

		re.GLSL().ModelTexFogShader().Deselect();
	}
	else
	{
		re.GLSL().ModelTexShader().Select(projection, re);
		re.GLSL().ModelTexShader().SetModelView(model);
		re.GLSL().ModelTexShader().SetTexture(s_TableTopTex);

		// Ambient light
		float ambientR, ambientG, ambientB, ambientA;
		re.CurrentAmbientLight(ambientR, ambientG, ambientB, ambientA);
		// Impose a minimum ambience so the table and chips glow in dim light.
		if(ambientR < minAmbient)
			ambientR = minAmbient;
		if(ambientG < minAmbient)
			ambientG = minAmbient;
		if(ambientB < minAmbient)
			ambientB = minAmbient;
		re.GLSL().ModelTexShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().ModelTexShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Render the table top.
		s_TableTopModel.SelectGL();
		s_TableTopModel.DrawGL();
		s_TableTopModel.DeselectGL();

		// Render bets as stacks of chips.
		RenderBets(model, re, false);

		re.GLSL().ModelTexShader().Deselect();
	}
}

void AuGameClonepok::RenderBets(const AuMatrix4x4& model, AuGLEffects& re, bool useFog)
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

void AuGameClonepok::RenderModel(const AuMatrix4x4& model, AuGLVAO& vao, float locX, float locY, float locZ, float rotX,
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
