//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujoystick.h,v 1.12 2022/02/19 17:31:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuJoystick_H
#define AuJoystick_H

#include "auatom.h"
#include <QVector>
#if defined(Q_OS_WIN)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

class AUATOM_DLL AuJoystick
{
public:
	AuJoystick();
	virtual ~AuJoystick();
	AuJoystick(const AuJoystick& c);

	// Returns the number of axes a joystick has.
	inline int Axes(int joyIndex) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickNumAxes(m_Joysticks[joyIndex]);
	}

	// Returns the current input value of a joystick axis, in the range -32768 to 32767.
	inline qint16 AxisValue(int joyIndex, int axis) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickGetAxis(m_Joysticks[joyIndex], axis);
	}

	// Returns the number of balls a joystick has.
	inline int Balls(int joyIndex) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickNumBalls(m_Joysticks[joyIndex]);
	}

	// Returns relative trackball motion.
	inline void BallState(int joyIndex, int ball, int& dx, int& dy) const
	{
		Q_ASSERT(joyIndex < Count());
		SDL_JoystickGetBall(m_Joysticks[joyIndex], ball, &dx, &dy);
	}

	// Returns the number of buttons a joystick has.
	inline int Buttons(int joyIndex) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickNumButtons(m_Joysticks[joyIndex]);
	}

	// Returns the on/off boolean state of a button.
	inline bool ButtonState(int joyIndex, int button) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickGetButton(m_Joysticks[joyIndex], button) ? true : false;
	}

	// Return the number of joysticks.
	inline int Count() const
	{
		return m_Joysticks.count();
	}

	inline int Hats(int joyIndex) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickNumHats(m_Joysticks[joyIndex]);
	}

	// Returns the state of a hat.
	// Returns an OR'd combination of one or more of the following:
	//  SDL_HAT_CENTERED
	//  SDL_HAT_UP
	//  SDL_HAT_RIGHT
	//  SDL_HAT_DOWN
	//  SDL_HAT_LEFT
	//  SDL_HAT_RIGHTUP
	//  SDL_HAT_RIGHTDOWN
	//  SDL_HAT_LEFTUP
	//  SDL_HAT_LEFTDOWN
	inline quint8 HatState(int joyIndex, int hat) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickGetHat(m_Joysticks[joyIndex], hat);
	}

	// Initializes joystick support. Returns 0 on success.
	int Initialize();

	// Returns the name of a joystick.
	inline const char* Name(int joyIndex) const
	{
		Q_ASSERT(joyIndex < Count());
		return SDL_JoystickName(m_Joysticks[joyIndex]);
	}

	// Update current joystick values.
	void Poll() const
	{
		SDL_JoystickUpdate();
	}

private:
	QVector<SDL_Joystick*>	m_Joysticks;	// Array of open joysticks.
};

#endif // AuJoystick_H
