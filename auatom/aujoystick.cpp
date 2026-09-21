//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujoystick.cpp,v 1.8 2022/02/19 17:40:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aujoystick.h"
//

AuJoystick::AuJoystick()
{
}

AuJoystick::~AuJoystick()
{
	// Close all the joysticks that we opened.
	int joyIndex=0;
	for(QVector<SDL_Joystick*>::iterator jit = m_Joysticks.begin(); jit != m_Joysticks.end(); jit++, joyIndex++)
		if(*jit)
		{
			SDL_JoystickClose(*jit);
			*jit = 0;
		}
}

AuJoystick::AuJoystick(const AuJoystick& c) : m_Joysticks(c.m_Joysticks)
{
}

int AuJoystick::Initialize()
{
	if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		qDebug("SDL failed to initialize joystick subsystem. Joystick will not be available.");
		return 1;
	}
	else
	{
		// Open all the joysticks SDL can find.
		m_Joysticks.clear();
		for(int joyIndex=0; joyIndex<SDL_NumJoysticks(); joyIndex++)
			m_Joysticks.append(SDL_JoystickOpen(joyIndex));
	}
	return 0;
}

