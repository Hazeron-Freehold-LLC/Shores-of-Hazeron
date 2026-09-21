//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dicontrolinput.cpp,v 1.63 2022/04/15 16:53:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dicontrolinput.h"
//
#include "auglobal.h"
#include "aujoystick.h"
#include "daperson.h"
#include "dipanelcontrol.h"
#include <QWheelEvent>

// Preferred constructor.
DiControlInput::DiControlInput(const QString& sName, const QString& descText, unsigned char byId,
		Qt::Key iKey, Qt::KeyboardModifiers iKeyMod,
		MOUSETRIGGER dtMouse, Qt::KeyboardModifiers iMouseMod,
		JOYTRIGGER dtJoy,
		int lMinR, int lMaxR,
		bool isStreaming, bool needsTargetInfo,
		WHEELTRIGGER dtWheel,
		quint8 affectsRotation,
		quint8 joyIndex, quint8 joySubIndex, bool joyAxisFull)
{
	m_Active = false;
	m_AffectsRotation = affectsRotation;
	m_bIsStreaming = isStreaming;
	m_eJoyTrigger = dtJoy;
	m_eMouseTrigger = dtMouse;
	m_eWheelTrigger = dtWheel;
	m_byId = byId;
	m_idPanel = -1;
	m_iKeyCode = iKey;
	m_iKeyMod = iKeyMod;
	m_iMouseMod = iMouseMod;
	m_JoyAxisFull = joyAxisFull;
	m_JoyIndex = joyIndex;
	m_JoySubIndex = joySubIndex;
	m_lMaxRange = lMaxR;
	m_lMinRange = lMinR;
	m_lRangeValue = 0;
	m_NeedsTargetInfo = needsTargetInfo;
	m_pxControl = QPoint(0,0);
	m_sName = sName;

	m_TargetLevel = 0;		// Sight acquisition value.
	m_TargetPart = 0;		// Part of target sighted.
	m_TargetShield = false;

	m_DescText = descText;
}

DiControlInput::DiControlInput(const QString &sName, unsigned char byId,
		Qt::Key iKey, Qt::KeyboardModifiers iKeyMod,
		MOUSETRIGGER dtMouse, Qt::KeyboardModifiers iMouseMod,
		JOYTRIGGER dtJoy,
		int lMinR, int lMaxR,
		bool isStreaming, bool needsTargetInfo,
		WHEELTRIGGER dtWheel,
		quint8 affectsRotation,
		quint8 joyIndex, quint8 joySubIndex, bool joyAxisFull)
{
	m_Active = false;
	m_AffectsRotation = affectsRotation;
	m_bIsStreaming = isStreaming;
	m_eJoyTrigger = dtJoy;
	m_eMouseTrigger = dtMouse;
	m_eWheelTrigger = dtWheel;
	m_byId = byId;
	m_idPanel = -1;
	m_iKeyCode = iKey;
	m_iKeyMod = iKeyMod;
	m_iMouseMod = iMouseMod;
	m_JoyAxisFull = joyAxisFull;
	m_JoyIndex = joyIndex;
	m_JoySubIndex = joySubIndex;
	m_lMaxRange = lMaxR;
	m_lMinRange = lMinR;
	m_lRangeValue = 0;
	m_NeedsTargetInfo = needsTargetInfo;
	m_pxControl = QPoint(0,0);
	m_sName = sName;

	m_TargetLevel = 0;		// Sight acquisition value.
	m_TargetPart = 0;		// Part of target sighted.
	m_TargetShield = false;
}

DiControlInput::DiControlInput(const QString &sName, unsigned char byId, qint16 idPanel)
{
	m_Active = false;
	m_AffectsRotation = 0;
	m_bIsStreaming = true;
	m_eJoyTrigger = JT_None;
	m_eMouseTrigger = MT_None;
	m_eWheelTrigger = WT_None;
	m_byId = byId;
	m_idPanel = idPanel;
	m_iKeyCode = Qt::Key_unknown;
	m_iKeyMod = Qt::NoModifier;
	m_iMouseMod = Qt::NoModifier;
	m_JoyAxisFull = 0;
	m_JoyIndex = 0;
	m_JoySubIndex = 0;
	m_lMaxRange = 100;
	m_lMinRange = 0;
	m_lRangeValue = 0;
	m_NeedsTargetInfo = false;
	m_pxControl = QPoint(0,0);
	m_sName = sName;

	m_TargetLevel = 0;		// Sight acquisition value.
	m_TargetPart = 0;		// Part of target sighted.
	m_TargetShield = false;
}

DiControlInput::DiControlInput()
{
	m_Active = false;
	m_AffectsRotation = 0;
	m_bIsStreaming = true;
	m_eJoyTrigger = JT_None;
	m_eMouseTrigger = MT_None;
	m_eWheelTrigger = WT_None;
	m_byId = 0;
	m_idPanel = -1;
	m_iKeyCode = Qt::Key_unknown;
	m_iKeyMod = Qt::NoModifier;
	m_iMouseMod = Qt::NoModifier;
	m_JoyAxisFull = 0;
	m_JoyIndex = 0;
	m_JoySubIndex = 0;
	m_lMaxRange = 100;
	m_lMinRange = 0;
	m_lRangeValue = 0;
	m_NeedsTargetInfo = false;
	m_pxControl = QPoint(0,0);
	m_sName = "";

	m_TargetLevel = 0;		// Sight acquisition value.
	m_TargetPart = 0;		// Part of target sighted.
	m_TargetShield = false;
}

// Used to send control activation.
DiControlInput::DiControlInput(int idPanel, unsigned char byId)
{
	m_Active = false;
	m_AffectsRotation = 0;
	m_bIsStreaming = true;
	m_eJoyTrigger = JT_None;
	m_eMouseTrigger = MT_None;
	m_eWheelTrigger = WT_None;
	m_byId = byId;
	m_idPanel = idPanel;
	m_iKeyCode = Qt::Key_unknown;
	m_iKeyMod = Qt::NoModifier;
	m_iMouseMod = Qt::NoModifier;
	m_JoyAxisFull = 0;
	m_JoyIndex = 0;
	m_JoySubIndex = 0;
	m_lMaxRange = 100;
	m_lMinRange = 0;
	m_lRangeValue = 0;
	m_NeedsTargetInfo = false;
	m_pxControl = QPoint(0,0);
	m_sName = "";

	m_TargetLevel = 0;		// Sight acquisition value.
	m_TargetPart = 0;		// Part of target sighted.
	m_TargetShield = false;

	SetMaxRangeValue();
}

// Used to send control activation that includes coordinates.
DiControlInput::DiControlInput(int idPanel, unsigned char byId, QPoint &pxControl)
{
	m_Active = false;
	m_AffectsRotation = 0;
	m_bIsStreaming = true;
	m_eJoyTrigger = JT_None;
	m_eMouseTrigger = MT_None;
	m_eWheelTrigger = WT_None;
	m_byId = byId;
	m_idPanel = idPanel;
	m_iKeyCode = Qt::Key_unknown;
	m_iKeyMod = Qt::NoModifier;
	m_iMouseMod = Qt::NoModifier;
	m_JoyAxisFull = 0;
	m_JoyIndex = 0;
	m_JoySubIndex = 0;
	m_lMaxRange = 100;
	m_lMinRange = 0;
	m_lRangeValue = 0;
	m_NeedsTargetInfo = false;
	m_pxControl = pxControl;
	m_sName = "";

	m_TargetLevel = 0;		// Sight acquisition value.
	m_TargetPart = 0;		// Part of target sighted.
	m_TargetShield = false;

	SetMaxRangeValue();
}

DiControlInput::~DiControlInput()
{

}

DiControlInput::DiControlInput(const DiControlInput &ci)
{
	m_Active = ci.m_Active;
	m_AffectsRotation = ci.m_AffectsRotation;
	m_bIsStreaming = ci.m_bIsStreaming;
	m_DescText = ci.m_DescText;
	m_eJoyTrigger = ci.m_eJoyTrigger;
	m_eMouseTrigger = ci.m_eMouseTrigger;
	m_eWheelTrigger = ci.m_eWheelTrigger;
	m_byId = ci.m_byId;
	m_idPanel = ci.m_idPanel;
	m_iKeyCode = ci.m_iKeyCode;
	m_iKeyMod = ci.m_iKeyMod;
	m_iMouseMod = ci.m_iMouseMod;
	m_JoyAxisFull = ci.m_JoyAxisFull;
	m_JoyIndex = ci.m_JoyIndex;
	m_JoySubIndex = ci.m_JoySubIndex;
	m_lMaxRange = ci.m_lMaxRange;
	m_lMinRange = ci.m_lMinRange;
	m_lRangeValue = ci.m_lRangeValue;
	m_NeedsTargetInfo = ci.m_NeedsTargetInfo;
	m_pxControl = ci.m_pxControl;
	m_sName = ci.m_sName;
	m_TargetId = ci.m_TargetId;
	m_TargetLevel = ci.m_TargetLevel;
	m_TargetLoc = ci.m_TargetLoc;
	m_TargetPart = ci.m_TargetPart;
	m_TargetShield = ci.m_TargetShield;
}

const DiControlInput &DiControlInput::operator=(const DiControlInput &ci)
{
	m_Active = ci.m_Active;
	m_AffectsRotation = ci.m_AffectsRotation;
	m_bIsStreaming = ci.m_bIsStreaming;
	m_DescText = ci.m_DescText;
	m_eJoyTrigger = ci.m_eJoyTrigger;
	m_eMouseTrigger = ci.m_eMouseTrigger;
	m_eWheelTrigger = ci.m_eWheelTrigger;
	m_byId = ci.m_byId;
	m_idPanel = ci.m_idPanel;
	m_iKeyCode = ci.m_iKeyCode;
	m_iKeyMod = ci.m_iKeyMod;
	m_iMouseMod = ci.m_iMouseMod;
	m_JoyAxisFull = ci.m_JoyAxisFull;
	m_JoyIndex = ci.m_JoyIndex;
	m_JoySubIndex = ci.m_JoySubIndex;
	m_lMaxRange = ci.m_lMaxRange;
	m_lMinRange = ci.m_lMinRange;
	m_lRangeValue = ci.m_lRangeValue;
	m_NeedsTargetInfo = ci.m_NeedsTargetInfo;
	m_pxControl = ci.m_pxControl;
	m_sName = ci.m_sName;
	m_TargetId = ci.m_TargetId;
	m_TargetLevel = ci.m_TargetLevel;
	m_TargetLoc = ci.m_TargetLoc;
	m_TargetPart = ci.m_TargetPart;
	m_TargetShield = ci.m_TargetShield;
	return *this;
}

// Returns true if this control is active right now, i.e. button pushed.
bool DiControlInput::TestKeyTriggered(const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& keysDown, Qt::KeyboardModifiers modState)
{
	// Test for keyboard keys.
	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>::const_iterator key = keysDown.find(m_iKeyCode);
	if(key != keysDown.end())
	{
		// This key is down.
		if((*key).second.isValid())
		{
			// The key was already down. If the key is not allowed to stream, the user must let up the key and press it again.
			// This does not apply to the run modifier.
			if(!m_bIsStreaming && GetId() != DaPerson::CI_Run)
				return false;
		}

		// Check keyboard modifiers.
		if(m_iKeyMod == modState)
		{
			// The proper modifier bit combination is pressed.
			// Set keyboard activation range value based on the length of time the key has been pressed.
			int msecs = (*key).second.elapsed();
			const int rampTime = 500;
			if(msecs < rampTime)
			{
				int rangeVal = (m_lMaxRange - m_lMinRange) * msecs / rampTime;
				rangeVal += m_lMinRange;
				SetRangeValue(rangeVal);
			}
			else
				SetMaxRangeValue();
			return true;
		}
	}
	return false;
}

bool DiControlInput::TestMouseTriggered(Qt::MouseButtons buttons, Qt::KeyboardModifiers modState, const QPoint &pMouseVec)
{
	switch(m_eMouseTrigger)
	{
	default:
	case MT_None:
		break;
	case MT_Left:
		if(pMouseVec.x() < 0)
		{
			IncreaseRangeValue(abs(pMouseVec.x()));
			return true;
		}
		break;
	case MT_Right:
		if(pMouseVec.x() > 0)
		{
			IncreaseRangeValue(pMouseVec.x());
			return true;
		}
		break;
	case MT_Up:
		if(pMouseVec.y() < 0)
		{
			IncreaseRangeValue(abs(pMouseVec.y()));
			return true;
		}
		break;
	case MT_Down:
		if(pMouseVec.y() > 0)
		{
			IncreaseRangeValue(pMouseVec.y());
			return true;
		}
		break;
	case MT_ButtonLeft:
		if((buttons & Qt::LeftButton) && m_iMouseMod == modState)
		{
			SetMaxRangeValue();	// Button input returns max range value.
			return true;
		}
		break;
	case MT_ButtonMiddle:
		if((buttons & Qt::MidButton) && m_iMouseMod == modState)
		{
			SetMaxRangeValue();	// Button input returns max range value.
			return true;
		}
		break;
	case MT_ButtonRight:
		if((buttons & Qt::RightButton) && m_iMouseMod == modState)
		{
			SetMaxRangeValue();	// Button input returns max range value.
			return true;
		}
		break;
	}
	return false;
}

bool DiControlInput::TestWheelTriggered(const QWheelEvent* e)
{
	//qDebug("e->state() %#x m_iKeyState %#x", e->state(), m_iKeyState);
	switch(m_eWheelTrigger)
	{
	default:
	case WT_None:
		break;
	case WT_AwayFromUser:
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
		if(e->angleDelta().y() > 0)
		{
			IncreaseRangeValue(e->angleDelta().y());
			return true;
		}
#else
		if(e->delta() > 0)
		{
			IncreaseRangeValue(e->delta());
			return true;
		}
#endif
		break;
	case WT_TowardUser:
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
		if(e->angleDelta().y() < 0)
		{
			IncreaseRangeValue(-e->angleDelta().y());
			return true;
		}
#else
		if(e->delta() < 0)
		{
			IncreaseRangeValue(-e->delta());
			return true;
		}
#endif
		break;
	}
	return false;
}

void DiControlInput::SetRangeValue(int lVal)
{
	if(lVal < m_lMinRange)
		m_lRangeValue = lVal;
	else if(lVal > m_lMaxRange)
		m_lRangeValue = m_lMaxRange;
	else
		m_lRangeValue = lVal;
}

bool DiControlInput::TestJoystickTriggered(const AuJoystick* joystick)
{
	// Ignore this control input if the joystick is disabled.
	if(!g_Au.IsJoystickEnabled(m_JoyIndex))
		return false;

	// Ignore all joystick controls if the indexed joystick isn't there.
	if(m_JoyIndex >= joystick->Count())
		return false;

	switch(m_eJoyTrigger)
	{
	case JT_None:
		break;
	case JT_AxisNegative:
		{
			// Get the value from the joystick.
			int axisvalue = joystick->AxisValue(m_JoyIndex, m_JoySubIndex);
			//qDebug("Control %d: joyIndex %d axis %d: negative axisvalue %d", GetId(), m_JoyIndex, m_JoySubIndex, axisvalue);
			if(axisvalue < 0)
			{
				// Scale the value from the joystick into the range requested.
				int range = m_lMaxRange - m_lMinRange;	// Range of values wanted.
				float axisSquared = (float)axisvalue * (float)axisvalue;
				int squaredMax = 32767*32767;
				int value = (int)round(axisSquared * range / squaredMax);	// Squared for curved distribution.
				if(value)
				{
					// Don't activate unless it is a streaming command or is currently inactive.
					if(m_bIsStreaming || !m_Active)
					{
						value = value + m_lMinRange;			// Adjust to the minimum range.
						SetRangeValue(value);
						//qDebug("Control %d: joyIndex %d axis %d: negative axisvalue %d value %d",
								//GetId(), m_JoyIndex, m_JoySubIndex, axisvalue, value);
						m_Active = true;
						return true;
					}
				}
				else
					m_Active = false;
			}
			else
				m_Active = false;
		}
		break;
	case JT_AxisPositive:
		{
			// Get the value from the joystick.
			int axisvalue = joystick->AxisValue(m_JoyIndex, m_JoySubIndex);
			//qDebug("Control %d: joyIndex %d axis %d: positive axisvalue %d", GetId(), m_JoyIndex, m_JoySubIndex, axisvalue);
			if(axisvalue > 0)
			{
				// Scale the value from the joystick into the range requested.
				int range = m_lMaxRange - m_lMinRange;	// Range of values wanted.
				float axisSquared = (float)axisvalue * (float)axisvalue;
				int squaredMax = 32767*32767;
				int value = (int)round(axisSquared * range / squaredMax);	// Squared for curved distribution.
				if(value)
				{
					// Don't activate unless it is a streaming command or is currently inactive.
					if(m_bIsStreaming || !m_Active)
					{
						value = value + m_lMinRange;			// Adjust to the minimum range.
						SetRangeValue(value);
						//qDebug("Control %d: joyIndex %d axis %d: positive axisvalue %d value %d",
								//GetId(), m_JoyIndex, m_JoySubIndex, axisvalue, value);
						m_Active = true;
						return true;
					}
				}
				else
					m_Active = false;
			}
			else
				m_Active = false;
		}
		break;
	case JT_Axis:
		{
			// Get the value from the joystick.
			int axisvalue = joystick->AxisValue(m_JoyIndex, m_JoySubIndex);

			// Scale the value from the joystick into the range requested.
			int value;
			if(m_lMinRange < 0)
			{
				// Range above and below zero might be asymmetrical, as with the throttle on the Space Rocket.
				if(axisvalue >= 0)
					value = (int)round(axisvalue * m_lMaxRange / 32767.f);
				else
					value = (int)round(axisvalue * m_lMinRange / -32767.f);
			}
			else
			{
				// Range is all above zero so a linear distribution works.
				int range = m_lMaxRange;			// Range of values wanted.
				value = (int)round((axisvalue + 32767) * range / 65534.f);
				value = value + m_lMinRange;		// Adjust to the minimum range.
			}
			SetRangeValue(value);
			//qDebug("JT_Axis axisvalue %d value %d", axisvalue, value);
			return true;
		}
		break;
	case JT_AxisInvert:
		{
			// Get the value from the joystick, inverting the sign.
			int axisvalue = -joystick->AxisValue(m_JoyIndex, m_JoySubIndex);

			// Scale the value from the joystick into the range requested.
			int value;
			if(m_lMinRange < 0)
			{
				// Range above and below zero might be asymmetrical, as with the throttle on the Space Rocket.
				if(axisvalue >= 0)
					value = (int)round(axisvalue * m_lMaxRange / 32767.f);
				else
					value = (int)round(axisvalue * m_lMinRange / -32767.f);
			}
			else
			{
				// Range is all above zero so a linear distribution works.
				int range = m_lMaxRange;			// Range of values wanted.
				value = (int)round((axisvalue + 32767) * range / 65534.f);
				value = value + m_lMinRange;		// Adjust to the minimum range.
			}
			SetRangeValue(value);
			//qDebug("JT_AxisInvert axisvalue %d value %d", axisvalue, value);
			return true;
		}
		break;
	case JT_BallXPositive:
		{
			// Get the relative value from the track ball.
			int dx = 0;
			int dy = 0;
			joystick->BallState(m_JoyIndex, m_JoySubIndex, dx, dy);
			if(dx > 0)
			{
				// Scale the value from the joystick into the range requested.
				int range = m_lMaxRange - m_lMinRange;	// Range of values wanted.
				int value = dx;
				if(value > range)
					value = range;
				if(value)
				{
					// Don't activate unless it is a streaming command or is currently inactive.
					if(m_bIsStreaming || !m_Active)
					{
						value = value + m_lMinRange;			// Adjust to the minimum range.
						SetRangeValue(value);
						m_Active = true;
						return true;
					}
				}
				else
					m_Active = false;
			}
			else
				m_Active = false;
		}
		break;
	case JT_BallXNegative:
		{
			// Get the relative value from the track ball.
			int dx = 0;
			int dy = 0;
			joystick->BallState(m_JoyIndex, m_JoySubIndex, dx, dy);
			if(dx < 0)
			{
				// Scale the value from the joystick into the range requested.
				int range = m_lMaxRange - m_lMinRange;	// Range of values wanted.
				int value = abs(dx);
				if(value > range)
					value = range;
				if(value)
				{
					// Don't activate unless it is a streaming command or is currently inactive.
					if(m_bIsStreaming || !m_Active)
					{
						value = value + m_lMinRange;			// Adjust to the minimum range.
						SetRangeValue(value);
						m_Active = true;
						return true;
					}
				}
				else
					m_Active = false;
			}
			else
				m_Active = false;
		}
		break;
	case JT_BallYPositive:
		{
			// Get the relative value from the track ball.
			int dx = 0;
			int dy = 0;
			joystick->BallState(m_JoyIndex, m_JoySubIndex, dx, dy);
			if(dy > 0)
			{
				// Scale the value from the joystick into the range requested.
				int range = m_lMaxRange - m_lMinRange;	// Range of values wanted.
				int value = dy;
				if(value > range)
					value = range;
				if(value)
				{
					// Don't activate unless it is a streaming command or is currently inactive.
					if(m_bIsStreaming || !m_Active)
					{
						value = value + m_lMinRange;			// Adjust to the minimum range.
						SetRangeValue(value);
						m_Active = true;
						return true;
					}
				}
				else
					m_Active = false;
			}
			else
				m_Active = false;
		}
		break;
	case JT_BallYNegative:
		{
			// Get the relative value from the track ball.
			int dx = 0;
			int dy = 0;
			joystick->BallState(m_JoyIndex, m_JoySubIndex, dx, dy);
			if(dy < 0)
			{
				// Scale the value from the joystick into the range requested.
				int range = m_lMaxRange - m_lMinRange;	// Range of values wanted.
				int value = abs(dy);
				if(value > range)
					value = range;
				if(value)
				{
					// Don't activate unless it is a streaming command or is currently inactive.
					if(m_bIsStreaming || !m_Active)
					{
						value = value + m_lMinRange;			// Adjust to the minimum range.
						SetRangeValue(value);
						m_Active = true;
						return true;
					}
				}
				else
					m_Active = false;
			}
			else
				m_Active = false;
		}
		break;
	case JT_Button:
		if(joystick->ButtonState(m_JoyIndex, m_JoySubIndex))
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Button input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatDown:		// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_DOWN)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatCentered:	// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_CENTERED)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatLeft:		// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_LEFT)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatLeftDown:	// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_LEFTDOWN)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatLeftUp:		// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_LEFTUP)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatRight:		// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_RIGHT)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatRightDown:	// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_RIGHTDOWN)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatRightUp:		// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_RIGHTUP)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	case JT_HatUp:			// Hat position
		if(joystick->HatState(m_JoyIndex, m_JoySubIndex) == SDL_HAT_UP)
		{
			// Don't activate unless it is a streaming command or is currently inactive.
			if(m_bIsStreaming || !m_Active)
			{
				SetMaxRangeValue();		// Hat input returns max range value.
				m_Active = true;
				return true;
			}
		}
		else
			m_Active = false;
		break;
	}
	return false;
}

// Returns true if keys/controls conflict between 'this' control and 'ciCompare'.
bool DiControlInput::IsDuplicate(const DiControlInput &ciCompare) const
{
	if(m_eJoyTrigger != JT_None &&
			m_eJoyTrigger == ciCompare.m_eJoyTrigger)
		return true;

	if(m_iKeyCode != Qt::Key_unknown &&
			m_iKeyCode == ciCompare.m_iKeyCode &&
			m_iKeyMod == ciCompare.m_iKeyMod)
		return true;

	if(m_eMouseTrigger != MT_None &&
			m_eMouseTrigger == ciCompare.m_eMouseTrigger &&
			m_iMouseMod == ciCompare.m_iMouseMod)
		return true;

	if(m_eWheelTrigger != WT_None &&
			m_eWheelTrigger == ciCompare.m_eWheelTrigger)
		return true;

	return false;
}

void DiControlInput::Tx(QDataStream& out) const
{
	// Decide what data this packet will have.
	quint8 hasWhat = 0;
	if(m_pxControl != QPoint(0,0))
		hasWhat |= 0x80;	// Top bit for pxControl
	if(m_idPanel == -1)
		hasWhat |= 0x40;
	if(m_lRangeValue == m_lMaxRange)
		hasWhat |= 0x20;
	if(!m_TargetId.IsNull())
		hasWhat |= 0x10;

	hasWhat |= m_TargetLevel;	// assumes targetlevel in range of 0..15

	out << hasWhat;
	out << m_byId;

	// Send the data that hasWhat says we're sending.
	if(hasWhat & 0x80)
		out << m_pxControl;
	if(!(hasWhat & 0x40))
		out << m_idPanel;
	if(!(hasWhat & 0x20))
		out << m_lRangeValue;
	if(hasWhat & 0x10)
	{
		out << m_TargetId;
		out << m_TargetPart;
		out << (float)m_TargetLoc.x;
		out << (float)m_TargetLoc.y;
		out << (float)m_TargetLoc.z;
		quint8 tbits = 0;
		if(m_TargetShield)
			tbits |= 0x01;
		out << tbits;
	}
}

void DiControlInput::Rx(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;
	in >> m_byId;

	if(hasWhat & 0x80)
		in >> m_pxControl;
	if(hasWhat & 0x40)
		m_idPanel = -1;
	else
		in >> m_idPanel;
	if(hasWhat & 0x20)
		m_lRangeValue = m_lMaxRange;
	else
		in >> m_lRangeValue;

	if(hasWhat & 0x10)
	{
		in >> m_TargetId;

		// Targeting information present.
		m_TargetLevel = hasWhat & 0x0f;
		in >> m_TargetPart;
		float x,y,z;
		in >> x;
		in >> y;
		in >> z;
		m_TargetLoc = AuPoint(x,y,z);
		quint8 tbits;
		in >> tbits;
		m_TargetShield = tbits & 0x01;
	}
	else
	{
		// No targeting information.
		m_TargetId = AuId();
		m_TargetLevel = 0;
		m_TargetLoc = AuPoint(0,0);
		m_TargetPart = 0;
		m_TargetShield = false;
	}
}

void DiControlInput::LoadSettings(const QString& sectionName)
{
	// A data stream was used to make a blob of binary data.
	QString entry = QString("/%1/%2").arg(sectionName).arg(GetId());
	QByteArray ba;
	g_Au.Settings().readEntry(entry, ba);
	if(ba.isEmpty())
		return;

	quint8 ui8;
	quint32 ui32;
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);

	in >> ui32;
	m_iKeyCode = (Qt::Key)ui32;
	in >> ui32;
	m_iKeyMod = (Qt::KeyboardModifiers)ui32;
	in >> ui8;
	m_eMouseTrigger = (MOUSETRIGGER)ui8;
	in >> ui32;
	m_iMouseMod = (Qt::KeyboardModifiers)ui32;
	in >> ui8;
	m_eJoyTrigger = (JOYTRIGGER)ui8;
	in >> m_JoyIndex;
	in >> m_JoySubIndex;
	in >> ui8;
	m_eWheelTrigger = (WHEELTRIGGER)ui8;
}

void DiControlInput::SaveSettings(const QString& sectionName) const
{
	// Use a data stream to make a blob of binary data.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);

	out << (quint32)m_iKeyCode;
	out << (quint32)m_iKeyMod;
	out << (quint8)m_eMouseTrigger;
	out << (quint32)m_iMouseMod;
	out << (quint8)m_eJoyTrigger;
	out << m_JoyIndex;
	out << m_JoySubIndex;
	out << (quint8)m_eWheelTrigger;

	QString entry = QString("/%1/%2").arg(sectionName).arg(GetId());
	g_Au.Settings().writeEntry(entry, ba);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Turns off the run modifier bit(s) in modState if they are being used to run at the moment.
// Turns off the bits in modState that we don't use.
// The run modifier only applies to the forward movement control of a person.
// If the run modifier is used to modify a person's forward movement and the run modifier key happens to be a keyboard modifier,
// we must ignore it as a keyboard modifier. In other words, while used to run, the run modifier key will not also act as a
// keyboard modifier.
// For example, left-shift is the default run modifier. When left-shift is held for the purpose of running forward, we clear
// the left-shift modifier from the mods bits. Otherwise a person can't reload, jump, climb, dive, etc. while running. Also,
// the run modifier would not work unless it defined itself as the keyboard modifier also.
Qt::KeyboardModifiers DiControlInputArray::FilterModState(const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& keysDown, Qt::KeyboardModifiers modState)
{
	// We only care about certain modifiers. Turn off the bits for the ones we don't use.
	modState = (Qt::KeyboardModifiers)(modState&(Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier|Qt::MetaModifier));

	// Filter run modifier.
	int movingForward = 0;	// move forward command: 0=unknown, 1=inactive, 2=active
	int movingForwardContinuously = 0;	// move forward continuously command: 0=unknown, 1=inactive, 2=active
	int movingLeft = 0;	// move left command: 0=unknown, 1=inactive, 2=active
	int movingRight = 0;	// move right command: 0=unknown, 1=inactive, 2=active
	int runMod = 0;			// run modifier command: 0=unknown, 1=inactive, 2=active
	Qt::Key runKey = Qt::Key_unknown;
	for(iterator ici = begin(); ici != end(); ici++)
	{
		// Look for the person's move forward and run modifier commands to be active.
		if((*ici).GetId() == DaPerson::CI_MoveForward)
		{
			movingForward = (*ici).TestKeyTriggered(keysDown, Qt::NoModifier) ? 2 : 1;
		}
		else if((*ici).GetId() == DaPerson::CI_MoveForwardContinuously)
		{
			movingForwardContinuously = (*ici).TestKeyTriggered(keysDown, Qt::NoModifier) ? 2 : 1;
		}
		else if((*ici).GetId() == DaPerson::CI_MoveLeft)
		{
			movingLeft = (*ici).TestKeyTriggered(keysDown, Qt::NoModifier) ? 2 : 1;
		}
		else if((*ici).GetId() == DaPerson::CI_MoveRight)
		{
			movingRight = (*ici).TestKeyTriggered(keysDown, Qt::NoModifier) ? 2 : 1;
		}
		else if((*ici).GetId() == DaPerson::CI_Run)
		{
			runKey = (*ici).Key();
			if(runKey != Qt::Key_Shift &&
					runKey != Qt::Key_Control &&
					runKey != Qt::Key_Alt &&
					runKey != Qt::Key_Meta)
				break;	// The run modifier does not overlap a keyboard modifier so this logic is not needed.
			runMod = (*ici).TestKeyTriggered(keysDown, Qt::NoModifier) ? 2 : 1;
		}
		else
			continue;	// Not a move forward command or run modifier.

		// Keep looping until we know the state of all three commands.
		if(!movingForward || !movingForwardContinuously || !movingLeft || !movingRight || !runMod)
			continue;

		// We know the state of both the move forward and run modifier commands so we can decide what to do.
		if((movingForward == 2 || movingForwardContinuously == 2 || movingLeft == 2 || movingRight == 2) && runMod == 2)
		{
			// Moving forward|left|right holding run mod.
			// Run mod overlaps a keyboard modifier.
			// Turn off the bit(s) in the modState that correspond to the active run modifier key.
			if(runKey == Qt::Key_Shift)
				modState = (Qt::KeyboardModifiers)(modState & ~Qt::ShiftModifier);		// Turn off shift bits.
			else if(runKey == Qt::Key_Control)
				modState = (Qt::KeyboardModifiers)(modState & ~Qt::ControlModifier);	// Turn off ctrl bits.
			else if(runKey == Qt::Key_Alt)
				modState = (Qt::KeyboardModifiers)(modState & ~Qt::AltModifier);		// Turn off alt bits.
			else if(runKey == Qt::Key_Meta)
				modState = (Qt::KeyboardModifiers)(modState & ~Qt::MetaModifier);		// Turn off meta bits.
		}
		break;
	}
	return modState;
}

// Fills an array with this array's active key controls at this time.
// To control mouse sensitivity, modulate the mouse vector before passing it.
// Any non-zero movement will trigger MT_ controls.
void DiControlInputArray::SampleKeyInput(DiControlInputArray &ciActive, const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& keysDown,
		Qt::KeyboardModifiers modState,
		const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel)
{
	// Init the active controls array to empty.
	ciActive.clear();

	// Fill the active controls array with the controls that are active.
	for(iterator ici = begin(); ici != end(); ici++)
		if((*ici).TestKeyTriggered(keysDown, modState))
		{
			if((*ici).NeedsTargetInfo())
				(*ici).SetTarget(targetId, targetPart, targetShield, targetLoc, targetLevel);
			ciActive.append(*ici);
		}
}

// Fills an array with this array's active joystick controls at this time.
// Returns true if any active joystick event is streaming input.
bool DiControlInputArray::SampleJoystickInput(DiControlInputArray &ciActive, const AuJoystick* joystick,
		const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel)
{
	// Don't do this at all if no joysticks have been detected.
	if(joystick->Count() <= 0)
		return false;

	// Poll all joysticks for current readings.
	joystick->Poll();

	// Fill the active controls array with the controls that are active.
	bool stream = false;
	for(iterator ici = begin(); ici != end(); ici++)
		if((*ici).TestJoystickTriggered(joystick))
		{
			if((*ici).NeedsTargetInfo())
				(*ici).SetTarget(targetId, targetPart, targetShield, targetLoc, targetLevel);
			ciActive.append(*ici);
			if((*ici).IsStreamer())
				stream = true;
		}
	return stream;
}

// Fills an array with this array's active mouse controls at this time.
// Returns true if any active mouse event is streaming input; realistically only one will be active.
bool DiControlInputArray::SampleMouseInput(DiControlInputArray &ciActive, Qt::MouseButtons buttons, Qt::KeyboardModifiers modState,
		const QPoint &pMouseMovementVec,
		const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel)
{
	// Init the active controls array to empty.
	//ciActive.clear();

	// Fill the active controls array with the controls that are active.
	bool stream = false;
	for(iterator ici = begin(); ici != end(); ici++)
		if((*ici).TestMouseTriggered(buttons, modState, pMouseMovementVec))
		{
			if((*ici).NeedsTargetInfo())
				(*ici).SetTarget(targetId, targetPart, targetShield, targetLoc, targetLevel);
			ciActive.append(*ici);
			if((*ici).IsStreamer())
				stream = true;
		}
	return stream;
}

// Fills an array with the current active controls.
void DiControlInputArray::SampleWheelInput(DiControlInputArray &ciActive, const QWheelEvent* e,
		const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel)
{
	//qDebug("Sampling wheel input vec %d,%d", pMouseMovementVec.x(),
			//pMouseMovementVec.y());

	// Init the active controls array to empty.
	ciActive.clear();

	// Fill the active controls array with the controls that are active.
	for(iterator ici = begin(); ici != end(); ici++)
		if((*ici).TestWheelTriggered(e))
		{
			if((*ici).NeedsTargetInfo())
				(*ici).SetTarget(targetId, targetPart, targetShield, targetLoc, targetLevel);
			ciActive.append(*ici);
		}
}

bool DiControlInputArray::HasId(int lId) const
{
	for(int ici=0; ici<count(); ici++)
	{
		const DiControlInput &ci = at(ici);
		if(ci.GetId() == lId)
			return true;
	}
	return false;
}

// Removes a given id if it's in the list.
void DiControlInputArray::RemoveId(int lId)
{
	for(int ici=count()-1; ici>=0; ici--)
	{
		const DiControlInput &ci = at(ici);
		if(ci.GetId() == lId)
			erase(begin()+ici);
	}
}

// Returns true if any of the contents affects the rotation of the parent atom.
// See AuView for usage of this.
bool DiControlInputArray::AffectsParentRotation() const
{
	for(DiControlInputArray::const_iterator it = constBegin(); it != constEnd();
			it++)
		if((*it).AffectsParentRotation())
			return true;
	return false;
}

// Returns true if any of the contents affects the rotation of the scene atom.
// See AuView for usage of this.
bool DiControlInputArray::AffectsRotation() const
{
	for(DiControlInputArray::const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).AffectsRotation())
			return true;
	return false;
}

// Returns true if the contents are only modifiers, input not worth submitting.
bool DiControlInputArray::ContainsOnlyModifiers() const
{
	for(DiControlInputArray::const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).GetId() != DaPerson::CI_Run)
			return false;
	return true;
}

// Removes controls from this control array that are already claimed in the 'aCompare' control array.
void DiControlInputArray::RemoveDuplicates(const DiControlInputArray &aCompare)
{
	for(DiControlInputArray::const_iterator it = aCompare.constBegin(); it != aCompare.constEnd(); it++)
		RemoveIfDuplicate(*it);
}

// Removes controls from this control array that overlap the keys/controls claimed by the 'ciCheck' control input.
void DiControlInputArray::RemoveIfDuplicate(const DiControlInput &ciCheck)
{
	// Check for overlap with existing control.
	DiControlInputArray::iterator it = begin();
	while(it != end())
		if((*it).IsDuplicate(ciCheck))
			it = erase(it);	// The control at 'ici' overlaps keys/controls claimed by 'ciCheck'.
		else
			it++;
}

// Samples controls on a panel. Appends active controls to this array.
// So far, no panel controls require the operator's sighted/target info.
void DiControlInputArray::SampleControls(qint16 idPanel, const QPoint &pxHit, bool bContinuous, const DiPanelControl *pControls,
		unsigned int uiControlCount)
{
	const DiPanelControl *pc = pControls;
	for(unsigned int ic=0; ic<uiControlCount; ic++, pc++)
	{
		// Skip non-continuous controls when the caller is looking for continuous
		// streaming input.
		if(bContinuous && !pc->m_bContinuous)
			continue;

		if(pc->m_rcControl.contains(pxHit))
		{
			if(pc->m_bWantCoords)
			{
				// This control needs to know the id and the location within the
				// control texture.
				QPoint pxControl;
				pxControl.setX(pxHit.x() - pc->m_rcControl.left());
				pxControl.setY(pc->m_rcControl.bottom() - pxHit.y());
				DiControlInput ci(idPanel, pc->m_uiControl, pxControl);
				append(ci);
			}
			else
			{
				// Only the Id matters when activating this control.
				DiControlInput ci(idPanel, pc->m_uiControl);
				append(ci);
			}
		}
	}
}

void DiControlInputArray::Rx(QDataStream& in)
{
	clear();
	quint8 num;
	in >> num;
	while(num)
	{
		DiControlInput ci;
		ci.Rx(in);
		append(ci);
		num--;
	}
}

void DiControlInputArray::Tx(QDataStream& out) const
{
	out << (quint8)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it).Tx(out);
}

void DiControlInputArray::RemoveExcessStreamers(const AuElapsedTimer& streamLast, int inputInterval)
{
	// Don't remove streamers if the last time is null. This is probably the first streaming input to be sent.
	if(!streamLast.isValid())
		return;

	qint64 millis = streamLast.elapsed();
	if(millis < inputInterval)
	{
		// We sent streaming input recently.
		// Remove all the streaming input from this array.
		iterator it = begin();
		while(it != end())
		{
			if((*it).IsStreamer())
				it = erase(it);
			else
				it++;
		}
	}
}

// Updates the streamLast time with the current time if any streamers are in the batch.
void DiControlInputArray::UpdateStreamTime(AuElapsedTimer& streamLast)
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).IsStreamer())
		{
			streamLast.restart();
			break;
		}
}

void DiControlInputArray::LoadSettings(const QString& sectionName)
{
	for(iterator it = begin(); it != end(); it++)
		(*it).LoadSettings(sectionName);
}

void DiControlInputArray::SaveSettings(const QString& sectionName) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it).SaveSettings(sectionName);
}

