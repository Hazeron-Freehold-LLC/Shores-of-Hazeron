//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dipanelcontrol.h,v 1.3 2022/02/19 17:36:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DIPANELCONTROL_H
#define DIPANELCONTROL_H

#include <qrect.h>

// DiPanelControl is a helper used to determine hits on panel controls.
// See AIntPilotConsole::SampleControlPanelInput.
class DiPanelControl
{
public:
	bool		m_bContinuous;	// TRUE if this control provides continuous input while the button is depressed.
	bool		m_bWantCoords;	// TRUE if the coordinates on the display screen are desired.
	QRect		m_rcControl;	// The control rectangle in the panel texture.
	unsigned int		m_uiControl;	// The control identifier.

	DiPanelControl(unsigned int uiControl, const QRect &rcControl, bool bContinuous=false, bool bWantCoords=false)
	{
		m_bContinuous = bContinuous;
		m_bWantCoords = bWantCoords;
		m_uiControl = uiControl;
		m_rcControl = rcControl;
	}
};

#endif // DIPANELCONTROL_H
