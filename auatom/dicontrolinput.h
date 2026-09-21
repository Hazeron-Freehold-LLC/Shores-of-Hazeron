//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dicontrolinput.h,v 1.46 2022/04/15 16:53:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DiControlInput_H
#define DiControlInput_H

#include "auatom.h"
#include "auelapsedtimer.h"
#include "auhash.h"
#include "aupoint.h"
#include <QPoint>
#include <QString>
#include <unordered_map>

class AuJoystick;
class DiPanelControl;
class QWheelEvent;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DiControlInput  
{
public:
	enum JOYTRIGGER
	{
		JT_None,
		JT_Axis,			// The whole axis range, such as a throttle.
		JT_AxisInvert,		// The whole axis range with values inverted, such as a throttle.
		JT_AxisNegative,	// Stick negative deflection, such as pitch, roll or yaw.
		JT_AxisPositive,	// Stick positive deflection, such as pitch, roll or yaw.
		JT_BallXNegative,	// Track ball negative relative motion
		JT_BallXPositive,	// Track ball negative relative motion
		JT_BallYNegative,	// Track ball negative relative motion
		JT_BallYPositive,	// Track ball negative relative motion
		JT_Button,			// Button
		JT_HatCentered,		// Hat position
		JT_HatDown,			// Hat position
		JT_HatLeft,			// Hat position
		JT_HatLeftDown,		// Hat position
		JT_HatLeftUp,		// Hat position
		JT_HatRight,		// Hat position
		JT_HatRightDown,	// Hat position
		JT_HatRightUp,		// Hat position
		JT_HatUp,			// Hat position
	};
	enum MOUSETRIGGER
	{
		MT_None,
		MT_Left,
		MT_Right,
		MT_Up,
		MT_Down,
		MT_ButtonLeft,
		MT_ButtonMiddle,
		MT_ButtonRight,
	};
	enum WHEELTRIGGER
	{
		WT_None,
		WT_AwayFromUser,
		WT_TowardUser,
	};

private:
	bool			m_Active;			// Set when activated so we can avoid streaming non-streaming joystick controls.
	quint8			m_AffectsRotation;	// 0=no, 1=scene atom, 2=scene parent
	QString			m_sName;			// Human readable name of this control input.
	QString			m_DescText;			// Human readable text visible when configuring this control.
	qint16			m_lMinRange;		// Range minimum for range type input.
	qint16			m_lMaxRange;		// Range maximum for range type input.
	Qt::Key			m_iKeyCode;			// Key that activates this control input.
	Qt::KeyboardModifiers	m_iKeyMod;	// Modifier key(s) required.
	MOUSETRIGGER	m_eMouseTrigger;	// Device activity that triggers this command.
	Qt::KeyboardModifiers	m_iMouseMod;	// Mouse modifier(s) required.
	JOYTRIGGER		m_eJoyTrigger;		// Device activity that triggers this command.
	bool			m_JoyAxisFull;		// True if the whole joystick axis is required, e.g. throttle control. Prevents mapping of
										// keys, mice, wheel, hats, or buttons. Only JT_Axis  or JT_AxisInvert are allowed.
	quint8			m_JoyIndex;			// Which joystick.
	quint8			m_JoySubIndex;		// Axis, Button, Ball, or Hat index, depending on context of m_eJoyTrigger.
	WHEELTRIGGER	m_eWheelTrigger;	// Device activity that triggers this command.
	bool			m_bIsStreaming;		// true enables this control to stream until released. false requires key release between
										// activations
	bool			m_NeedsTargetInfo;	// true if this input needs to be accompanied by targetting information.

	// Minimum values needed to signal 'activation'.
	qint16			m_idPanel;			// Panel id unique within an atom. -1 is null
	quint8			m_byId;				// Id number of this control input
	qint16			m_lRangeValue;		// Value of a range type input.

	// Optional value for 'activation'.
	QPoint			m_pxControl;		// Pixel location in the control texture map that was hit when the control was activated.

	// Targetting values set when input activated.
	AuId			m_TargetId;			// Atom targetted.
	quint8			m_TargetLevel;		// Sight acquisition value.
	AuPoint			m_TargetLoc;		// Sighted location on target.
	quint32			m_TargetPart;		// Part of target sighted.
	bool			m_TargetShield;

public:
	DiControlInput();
	DiControlInput(const QString &sName, unsigned char byId,
			Qt::Key iKey, Qt::KeyboardModifiers iKeyMod=Qt::NoModifier,
			MOUSETRIGGER dtMouse=MT_None, Qt::KeyboardModifiers iMouseMod=Qt::NoModifier,
			JOYTRIGGER dtJoy=JT_None,
			int lMinR=0, int lMaxR=100,
			bool isStreaming=true, bool needsTargetInfo=false,
			WHEELTRIGGER dtWheel=WT_None,
			quint8 affectsRotation=0,
			quint8 joyIndex=0, quint8 joySubIndex=0, bool joyAxisFull=false);
	DiControlInput(const QString &sName, const QString& descText, unsigned char byId,
			Qt::Key iKey, Qt::KeyboardModifiers iKeyMod=Qt::NoModifier,
			MOUSETRIGGER dtMouse=MT_None, Qt::KeyboardModifiers iMouseMod=Qt::NoModifier,
			JOYTRIGGER dtJoy=JT_None,
			int lMinR=0, int lMaxR=100,
			bool isStreaming=true, bool needsTargetInfo=false,
			WHEELTRIGGER dtWheel=WT_None,
			quint8 affectsRotation=0,
			quint8 joyIndex=0, quint8 joySubIndex=0, bool joyAxisFull=false);
	DiControlInput(int idPanel, unsigned char byId);	// Used to send control activation.
	DiControlInput(int idPanel, unsigned char byId, QPoint &pxControl); // Used to send
						// control activation that includes control texel coordinates.
	DiControlInput(const QString &sName, unsigned char byId, qint16 idPanel);	// Used when listing panel controls in the view.
	virtual ~DiControlInput();
	DiControlInput(const DiControlInput &ci);
	const DiControlInput &operator=(const DiControlInput &ci);
	
	inline bool AffectsParentRotation() const
	{
		return m_AffectsRotation == 2;
	}

	inline bool AffectsRotation() const
	{
		return m_AffectsRotation == 1;
	}

	inline void ClearRange()
	{
		SetRangeValue(0);
	}

	inline const QString& Description() const
	{
		return m_DescText;
	}

	//bool TestActive(const QPoint &pMouseMovementVec, const JOYINFOEX &joy,
	// const JOYCAPS &joycaps);		// Returns true if this control is active right
	// now, i.e. button pushed.
	inline int GetRangeValue() const
	{
		return m_lRangeValue;
	}
	inline unsigned char GetId() const
	{
		return m_byId;
	}

	inline JOYTRIGGER Joystick() const
	{
		return m_eJoyTrigger;
	}

	inline bool JoystickAxisExclusive() const
	{
		return m_JoyAxisFull;
	}

	inline int JoystickIndex() const
	{
		return m_JoyIndex;
	}

	inline quint8 JoystickSubIndex() const
	{
		return m_JoySubIndex;
	}

	inline Qt::Key Key() const
	{
		return m_iKeyCode;
	}

	inline Qt::KeyboardModifiers KeyModifier() const
	{
		return m_iKeyMod;
	}

	inline MOUSETRIGGER Mouse() const
	{
		return m_eMouseTrigger;
	}

	inline Qt::KeyboardModifiers MouseModifier() const
	{
		return m_iMouseMod;
	}

	inline WHEELTRIGGER MouseWheel() const
	{
		return m_eWheelTrigger;
	}

	inline const QString& Name() const
	{
		return m_sName;
	}

	inline bool NeedsTargetInfo() const
	{
		return m_NeedsTargetInfo;
	}

	bool operator==(const DiControlInput& c) const
	{
		Q_ASSERT(false);				//DHH Function added to satisfy dll export build in Windows. Does anybody use it?
		return m_iKeyCode == c.m_iKeyCode;
	}

	inline int PanelId() const
	{
		return m_idPanel;
	}

	inline void SetJoystick(JOYTRIGGER joy, int joyIndex, int joySubIndex)
	{
		if(joy == JT_None)
		{
			m_eJoyTrigger = joy;
		}
		else if(m_JoyAxisFull)
		{
			Q_ASSERT(joy == JT_Axis || joy == JT_AxisInvert);	// The only joystick trigger actions permitted for this control.
			m_eJoyTrigger = joy;
			m_JoyIndex = joyIndex;
			m_JoySubIndex = joySubIndex;
		}
		else
		{
			m_eJoyTrigger = joy;
			m_JoyIndex = joyIndex;
			m_JoySubIndex = joySubIndex;
		}
	}

	inline void SetKey(Qt::Key key, Qt::KeyboardModifiers modifier)
	{
		m_iKeyCode = key;
		m_iKeyMod = modifier;
	}

	inline void SetMouse(MOUSETRIGGER mouse, Qt::KeyboardModifiers modifier)
	{
		m_eMouseTrigger = mouse;
		m_iMouseMod = modifier;
	}

	inline void SetTarget(const AuId& idTarget, quint32 part, bool shield, const AuPoint& loc, quint8 level)
	{
		m_TargetId = idTarget;
		m_TargetLevel = level;
		m_TargetLoc = loc;
		m_TargetPart = part;
		m_TargetShield = shield;
	}

	inline void SetWheel(WHEELTRIGGER wheel)
	{
		m_eWheelTrigger = wheel;
	}

	inline const QPoint &HitLocation() const
	{
		return m_pxControl;
	}
	bool IsDuplicate(const DiControlInput &ciCompare) const;	// Returns true if
					// keys/controls conflict between 'this' control and 'ciCompare'.
	inline bool IsStreamer() const
	{
		return m_bIsStreaming;
	}
	inline void SetMaxRangeValue()
	{
		SetRangeValue(m_lMaxRange);
	}

	inline const AuId& TargetId() const
	{
		return m_TargetId;
	}

	inline unsigned int TargetLevel() const
	{
		return m_TargetLevel;
	}

	inline AuPoint TargetLoc() const
	{
		return m_TargetLoc;
	}

	inline unsigned int TargetPart() const
	{
		return m_TargetPart;
	}

	inline float TargetRange(const AuPoint& eyeLoc) const
	{
		return (m_TargetLoc - eyeLoc).Length();
	}

	inline bool TargetShield() const
	{
		return m_TargetShield;
	}

	bool TestJoystickTriggered(const AuJoystick* joystick);

	// Returns true if a keyboard control is active right now, i.e. button pushed.
	bool TestKeyTriggered(const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& keysDown, Qt::KeyboardModifiers modState);

	bool TestMouseTriggered(Qt::MouseButtons buttons, Qt::KeyboardModifiers modState, const QPoint &pMouseVec);

	bool TestWheelTriggered(const QWheelEvent* e);

	void Rx(QDataStream& in);
	void Tx(QDataStream& out) const;

	void LoadSettings(const QString& sectionName);
	void SaveSettings(const QString& sectionName) const;

private:
	//int GetRangeValue(const JOYINFOEX &joy, const JOYCAPS &joycaps) const;
	inline void IncreaseRangeValue(int lValue)
	{
		SetRangeValue(m_lRangeValue + lValue);
	}

	void SetRangeValue(int lValue);
};

class AUATOM_DLL DiControlInputArray : public QVector<DiControlInput>
{
public:
	bool AffectsParentRotation() const;

	bool AffectsRotation() const;

	inline void ClearRanges()
	{
		for(iterator ci = begin(); ci != end(); ci++)
			(*ci).ClearRange();
	}

	bool ContainsOnlyModifiers() const;	// Returns true if the contents are only key modifiers, not worth submitting as input.

	// Turns off the run modifier bit(s) in modState if they are being used to run at the moment.
	// Turns off the bits in modState that we don't use.
	Qt::KeyboardModifiers FilterModState(const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& keysDown, Qt::KeyboardModifiers modState);

	bool HasId(int lId) const;		// true if any of the control inputs is lId
	void RemoveId(int lId);			// Removes a given id if it's in the list.
	void RemoveDuplicates(const DiControlInputArray &aCompare);
	void RemoveExcessStreamers(const AuElapsedTimer& streamLast, int inputInterval);
	void RemoveIfDuplicate(const DiControlInput &ciCheck);
	void Rx(QDataStream& in);
	void SampleControls(qint16 idPanel, const QPoint &pxHit, bool bContinuous,
			const DiPanelControl *pControls, unsigned int uiControlCount);	// Fills this array with active panel controls.

	// Fills an array with the current active controls.
	void SampleKeyInput(DiControlInputArray &ciActive, const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& keysDown,
			Qt::KeyboardModifiers modState,
			const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel);

	// Fills an array with the current active controls.
	bool SampleJoystickInput(DiControlInputArray &ciActive, const AuJoystick* joystick,
			const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel);

	// Fills an array with the current active controls.
	bool SampleMouseInput(DiControlInputArray &ciActive, Qt::MouseButtons buttons, Qt::KeyboardModifiers modState,
			const QPoint &pMouseMovementVec,
			const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel);

	// Fills an array with the current active controls.
	void SampleWheelInput(DiControlInputArray &ciActive, const QWheelEvent* e,
			const AuId& targetId, quint32 targetPart, bool targetShield, const AuPoint& targetLoc, quint8 targetLevel);

	void Tx(QDataStream& out) const;
	void UpdateStreamTime(AuElapsedTimer& streamLast);

	void LoadSettings(const QString& sectionName);
	void SaveSettings(const QString& sectionName) const;
};

#endif // DiControlInput_H
