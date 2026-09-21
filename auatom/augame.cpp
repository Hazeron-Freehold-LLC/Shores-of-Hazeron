//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augame.cpp,v 1.15 2022/04/08 15:24:40 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augame.h"
//
#include "augameclonepok.h"
#include "augamehilo.h"
#include "augametexasholdem.h"
#include "daperson.h"
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

AuGame::AuGame()
{
	m_DealerSeat = 0;
	m_State = Setup;
	m_StateTime.start();
	m_TurnIndex = 0;
}

AuGame::~AuGame()
{
}

AuGame::AuGame(const AuGame& c)
{
	m_DealerSeat = c.m_DealerSeat;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		m_PlayerIds[player] = c.m_PlayerIds[player];
		m_PlayerNames[player] = c.m_PlayerNames[player];
	}
	m_State = c.m_State;
	m_StateTime = c.m_StateTime;
	m_TurnIndex = c.m_TurnIndex;
}

void AuGame::operator<<(QDataStream& in)
{
	quint8 u8;
	in >> u8;

	// Top four bits are the state.
	if(m_State != u8 >> 4)
	{
		// State changed.
		m_State = u8 >> 4;
		m_StateTime.restart();
	}

	m_TurnIndex = u8 & 0x0f;	// Bottom four bits are the turn index.
	QTime stateTime;
	in >> stateTime;
	in >> m_DealerSeat;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		in >> m_PlayerIds[player];
		in >> m_PlayerNames[player];
	}
}

void AuGame::operator>>(QDataStream& out) const
{
	quint8 u8 = (m_State << 4) | m_TurnIndex;	// Top four bits are state. Bottom four bits are turn index.
	out << u8;
	QTime stateTime;
	out << stateTime;
	out << m_DealerSeat;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		out << m_PlayerIds[player];
		out << m_PlayerNames[player];
	}
}

void AuGame::Rx(QDataStream& in)
{
	quint8 u8;
	in >> u8;

	// Top four bits are the state.
	if(m_State != u8 >> 4)
	{
		// State changed.
		m_State = u8 >> 4;
		m_StateTime.restart();
	}

	m_TurnIndex = u8 & 0x0f;	// Bottom four bits are the turn index.
	in >> m_DealerSeat;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		in >> m_PlayerIds[player];
		in >> m_PlayerNames[player];
	}
}

void AuGame::Tx(QDataStream& out, const DaPerson& /*sceneAtom*/) const
{
	quint8 u8 = (m_State << 4) | m_TurnIndex;	// Top four bits are state. Bottom four bits are turn index.
	out << u8;
	out << m_DealerSeat;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		out << m_PlayerIds[player];
		out << m_PlayerNames[player];
	}
}

void AuGame::Refresh(const AuGame* gameFromServer)
{
	// Refresh everything except client render data.
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		m_PlayerIds[player] = gameFromServer->m_PlayerIds[player];
		m_PlayerNames[player] = gameFromServer->m_PlayerNames[player];
	}
	m_DealerSeat = gameFromServer->m_DealerSeat;
	m_State = gameFromServer->m_State;
	m_StateTime = gameFromServer->m_StateTime;
	m_TurnIndex = gameFromServer->m_TurnIndex;
}

QString AuGame::Description() const
{
	return QObject::tr("<html><b>%1</b><br>"
			"Describe the game and its rules here."
			"</html>").arg(Name());
}

QIcon AuGame::Icon() const
{
	return QIcon(":data/Game.png");
}

QString AuGame::Name() const
{
	return QObject::tr("Game");
}

quint8 AuGame::Type() const
{
	return GameType();
}

// Lists the types of the known game.
//static
void AuGame::ListGameTypes(QList<quint8>& gameTypes)
{
#if !defined(QT_NO_DEBUG)
	gameTypes << AuGameClonepok::GameType();
#endif
	gameTypes << AuGameHiLo::GameType();
	gameTypes << AuGameTexasHoldem::GameType();
}

//static
AuGame* AuGame::CreateGame(quint8 gameType)
{
	if(gameType == AuGameClonepok::GameType())
		return new AuGameClonepok;
	else if(gameType == AuGameHiLo::GameType())
		return new AuGameHiLo;
	else if(gameType == AuGameTexasHoldem::GameType())
		return new AuGameTexasHoldem;
	return 0;
}

// Base class does not create a finish ui.
AuDialog* AuGame::CreateFinishUI(const QByteArray& /*gameResult*/, QWidget* /*parent*/) const
{
	return 0;
}

// Base class does not create a settings ui.
AuDialog* AuGame::CreateSettingsUI(const DaPerson& person, QWidget* parent) const
{
	Q_UNUSED(person);
	Q_UNUSED(parent);
	return 0;
}

// Base class does not create a turn ui.
AuDialog* AuGame::CreateTurnUI(const DaPerson& person, QWidget* parent) const
{
	Q_UNUSED(person);
	Q_UNUSED(parent);
	return 0;
}

QString AuGame::TurnUIName() const
{
	// Code like this is needed if the turn UI changes due to players leaving the game.
	//QString turnUiName = QString("%1:%2:%3").arg(Name()).arg(m_State).arg(m_TurnIndex);
	//for(int playerIndex=0; playerIndex<MAX_GAME_PLAYERS; playerIndex++)
		//turnUiName += QString(":%1").arg(m_PlayerIds[playerIndex].GetID());
	return QString("%1:%2:%3").arg(Name()).arg(m_State).arg(m_TurnIndex);
}

bool AuGame::Start(DaPerson* /*players*/[MAX_GAME_PLAYERS], const QByteArray& /*gameSettings*/)
{
	// Game can only be started from the setup state.
	if(State() == Setup)
	{
		SetState(Turning);	// Change to turning state.
		return true;
	}
	return false;
}

int AuGame::Tick()
{
	return 0;
}

bool AuGame::TurnComplete(DaPerson* person, DaPerson* /*players*/[MAX_GAME_PLAYERS], const QByteArray& /*gameTurn*/)
{
	return IsTurning() && person->Is(TurnId());
}
