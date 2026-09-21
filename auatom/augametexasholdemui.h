//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augametexasholdemui.h,v 1.5 2022/02/19 17:31:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameTexasHoldemUi_H
#define AuGameTexasHoldemUi_H

#include <audialog.h>
#include "augame.h"

class AuGameTexasHoldem;
class AuPushButtonValue;
class DaPerson;
class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;

class AuGameTexasHoldemSetup : public AuDialog
{
	Q_OBJECT;
private:
	AuId				m_PlayerIds[MAX_GAME_PLAYERS];		// Id's of all players in the game.

	// Widgets
	QToolButton*		m_BackStyle[4];
	QSpinBox*			m_BigBlind;
	QLabel*				m_BigBlindLabel;
	QSpinBox*			m_LittleBlind;
	QPushButton*		m_Ok;
	AuPushButtonValue*	m_PlayerKick[MAX_GAME_PLAYERS];
	QLabel*				m_PlayerNames[MAX_GAME_PLAYERS];

public:
	AuGameTexasHoldemSetup(const AuGameTexasHoldem& game, QWidget* parent = 0);
	virtual ~AuGameTexasHoldemSetup();

public slots:
	void BackStyleSelected0();
	void BackStyleSelected1();
	void BackStyleSelected2();
	void BackStyleSelected3();
	void BigBlindChanged(int bigblind);
	void DoubleBlinds();
	virtual void setVisible(bool visible);
	void StartGame();
	void KickPlayer(int playerIndex);

protected:
	void RestoreGeometry();
	void SaveGeometry();

signals:
	void Accepted(quint8 gameType, QByteArray settings);	// All AuGame setup dialogs must emit this signal when completed.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameTexasHoldemTurn : public AuDialog
{
	Q_OBJECT;
private:
	QList<int>			m_FinalHandCards;					// Final cards, in the order selected.

	// Widgets
	QCheckBox*			m_Cards[7];
	QPushButton*		m_Show;

public:
	AuGameTexasHoldemTurn(const AuGameTexasHoldem& game, QWidget* parent = 0);
	virtual ~AuGameTexasHoldemTurn();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void EnableControls();
	void FlopCard1Selected();
	void FlopCard2Selected();
	void FlopCard3Selected();
	void HoleCard1Selected();
	void HoleCard2Selected();
	void TurnCardSelected();
	void RiverCardSelected();
	void Showdown();
	void TurnCall();
	void TurnCheck();
	void TurnFold();
	void TurnRaise();

signals:
	void Accepted(quint8 gameType, QByteArray turn);	// All AuGame turn dialogs must emit this signal when completed.
	void Quit();										// Emit this signal when a player quits the game.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameTexasHoldemFinish : public AuDialog
{
	Q_OBJECT;
public:
	AuGameTexasHoldemFinish(const QByteArray& gameResult, QWidget* parent = 0);
	virtual ~AuGameTexasHoldemFinish();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGameTexasHoldemUi_H
