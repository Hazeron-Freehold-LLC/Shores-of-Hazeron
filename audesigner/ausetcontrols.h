//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetcontrols.h,v 1.5 2022/04/15 16:41:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetControls_H
#define AuSetControls_H

#include "audialog.h"
#include "auelapsedtimer.h"
#include "dicontrolinput.h"
#include <QWidget>

class DiControlInputArray;
class QTimer;

class AuSetControls : public QWidget
{
	Q_OBJECT;
public:
	AuSetControls(QWidget *parent=0);
	~AuSetControls();

signals:
	void ControlsChanged();

protected slots:
	void ConfigureJoystick(int joyIndex);

	void SetMouseGrab(int grab);

	void SetMouseLookSensitivity(int mls);

	void SetMouseLookSmoothing(int mls);

	// Control Bindings
	void CharacterControls();
	void CommandControls();
	void DirtBikeControls();
	void EngineerControls();
	void HelmControls();
	void HelicopterControls();
	void NavigatorControls();
	void PowerRelayControls();
	void SailBoatControls();
	void SceneControls();
	void SensorControls();
	void ShieldControls();
	void SpaceRocketControls();
	void SpaceFighterControls();
	void SubmarineControls();
	void SurgeryControls();
	void SUVControls();
	void TransporterControls();
	void WeaponControls();
	void WindowControls();
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBindControls : public AuDialog
{
	Q_OBJECT;
public:
	// When 'doTitles' is true, controls with id 0 are treated as titles only.
	AuBindControls(const QString& name, const DiControlInputArray& controls, QWidget* parent=0, bool doTitles=false,
			bool mouseOk=true, bool joystickOk=true);
	~AuBindControls();

	const DiControlInputArray& Controls() const
	{
		return m_Controls;
	}

	bool IsRestoringDefaults() const
	{
		return m_RestoreDefaults;
	}

	static QString KeyName(Qt::Key key);

	static QString ModifierName(Qt::KeyboardModifiers modifiers);

protected slots:
	void BindJoystick(int ctrlIdx);
	void BindKey(int ctrlIdx);
	void BindMouse(int ctrlIdx);
	void BindWheel(int ctrlIdx);
	void RestoreDefaults();
	void SaveChanges();

private:
	DiControlInputArray		m_Controls;
	bool					m_RestoreDefaults;	// true if RestoreDefaults was used to close the dialog.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBindKey : public AuDialog
{
	Q_OBJECT;
public:
	AuBindKey(const QString& action, Qt::Key key, Qt::KeyboardModifiers keyMod, QWidget* parent=0);
	~AuBindKey();

	Qt::Key Key() const
	{
		return m_Key;
	}

	Qt::KeyboardModifiers Modifier() const
	{
		return m_KeyMod;
	}

protected:
	virtual void keyReleaseEvent(QKeyEvent* e);

protected slots:
	void GrabInput();
	void OkPressed();
	void Unbind();

private:
	bool			m_GrabInput;
	Qt::Key			m_Key;
	Qt::KeyboardModifiers			m_KeyMod;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBindMouse : public AuDialog
{
	Q_OBJECT;
public:
	AuBindMouse(const QString& action, DiControlInput::MOUSETRIGGER mouse, Qt::KeyboardModifiers mouseMod, QWidget* parent=0);
	~AuBindMouse();

	Qt::KeyboardModifiers Modifier() const
	{
		return m_MouseMod;
	}

	DiControlInput::MOUSETRIGGER Mouse() const
	{
		return m_Mouse;
	}

protected slots:
	void OkPressed();
	void Unbind();

private:
	DiControlInput::MOUSETRIGGER	m_Mouse;
	Qt::KeyboardModifiers							m_MouseMod;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBindWheel : public AuDialog
{
	Q_OBJECT;
public:
	AuBindWheel(const QString& action, DiControlInput::WHEELTRIGGER mouse, QWidget* parent=0);
	~AuBindWheel();

	DiControlInput::WHEELTRIGGER Wheel() const
	{
		return m_Wheel;
	}

protected slots:
	void OkPressed();
	void Unbind();

private:
	DiControlInput::WHEELTRIGGER	m_Wheel;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBindJoystick : public AuDialog
{
	Q_OBJECT;
public:
	AuBindJoystick(const QString& action, DiControlInput::JOYTRIGGER joy, int joyIndex, int joySubIndex, bool joyAxisExclusive,
			QWidget* parent=0);
	~AuBindJoystick();

	DiControlInput::JOYTRIGGER JoyAction() const
	{
		return m_Joy;
	}

	int JoyIndex() const
	{
		return m_JoyIndex;
	}

	int JoySubIndex() const
	{
		return m_JoySubIndex;
	}

protected slots:
	void GrabInput();
	void InvertAxis(int);
	void OkPressed();
	void PollJoystick();
	void Unbind();

private:
	bool							m_GrabInput;
	int								m_HatJoyIndex;
	int								m_HatJoySubIndex;
	AuElapsedTimer					m_HatWait;		// For waiting for centered hats.
	DiControlInput::JOYTRIGGER		m_Joy;
	bool							m_JoyAxisExclusive;
	int								m_JoyIndex;		// Joystick index. Which joystick.
	int								m_JoySubIndex;	// Axis, button, ball, or hat index.
	QTimer*							m_timer;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConfigureJoystick : public AuDialog
{
	Q_OBJECT;
public:
	AuConfigureJoystick(int joyIndex, QWidget* parent=0);
	~AuConfigureJoystick();

protected slots:
	void SetEnableJoystick(int value);

private:
	int			m_JoyIndex;
};

#endif // AuSetControls_H
