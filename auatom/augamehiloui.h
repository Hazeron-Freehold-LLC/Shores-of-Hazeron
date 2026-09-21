//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augamehiloui.h,v 1.6 2022/02/19 17:31:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameHiLoUi_H
#define AuGameHiLoUi_H

#include <audialog.h>
#include "augame.h"

class AuGameHiLo;
class AuPushButtonValue;
class DaPerson;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;

class AuGameHiLoSetup : public AuDialog
{
	Q_OBJECT;
private:
	AuId				m_PlayerIds[MAX_GAME_PLAYERS];		// Id's of all players in the game.

	// Widgets
	QLineEdit*			m_Ante;
	QSlider*			m_Digits;
	QPushButton*		m_Ok;
	AuPushButtonValue*	m_PlayerKick[MAX_GAME_PLAYERS];
	QLabel*				m_PlayerNames[MAX_GAME_PLAYERS];

public:
	AuGameHiLoSetup(const AuGameHiLo& game, QWidget* parent = 0);
	virtual ~AuGameHiLoSetup();

public slots:
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

class AuGameHiLoTurn : public AuDialog
{
	Q_OBJECT;
private:
	quint8				m_Digits;

	// Widgets
	QLineEdit*			m_Number;
	QPushButton*		m_Ok;

public:
	AuGameHiLoTurn(const AuGameHiLo& game, QWidget* parent = 0);
	virtual ~AuGameHiLoTurn();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void NumberChanged(const QString& nuText);
	void QuitGame();
	void Turn();

signals:
	void Accepted(quint8 gameType, QByteArray turn);	// All AuGame turn dialogs must emit this signal when completed.
	void Quit();										// Emit this signal when a player quits the game.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameHiLoFinish : public AuDialog
{
	Q_OBJECT;
public:
	AuGameHiLoFinish(const QByteArray& gameResult, QWidget* parent = 0);
	virtual ~AuGameHiLoFinish();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGameHiLoUi_H
