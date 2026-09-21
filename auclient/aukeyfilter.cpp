//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aukeyfilter.cpp,v 1.3 2022/02/19 18:16:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aukeyfilter.h"
//
#include <auglobal.h>
#include <QDebug>
#include <QKeyEvent>

AuKeyFilter::AuKeyFilter(QObject* parent) : QObject(parent)
{
}

bool AuKeyFilter::eventFilter(QObject* obj, QEvent* event)
{
	bool handled = QObject::eventFilter(obj, event);
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if(!keyEvent->isAutoRepeat())
		{
			emit KeyPressed(keyEvent);
		}
	}
	else if(event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if(!keyEvent->isAutoRepeat())
		{
			emit KeyReleased(keyEvent);
		}
	}
	return handled;
}

//static
QString AuKeyFilter::KeyName(Qt::Key key)
{
	switch(key)
	{
	default:
		return QChar(key);
	case Qt::Key_Escape:
		return "Escape";
	case Qt::Key_Tab:
		return "Tab";
	case Qt::Key_Backtab:
		return "Backtab";
	case Qt::Key_Backspace:
		return "Backspace";
	case Qt::Key_Return:
		return "Return";
	case Qt::Key_Enter:
		return "Enter";
	case Qt::Key_Insert:
		return "Insert";
	case Qt::Key_Delete:
		return "Delete";
	case Qt::Key_Pause:
		return "Pause";
	case Qt::Key_Print:
		return "Print";
	case Qt::Key_SysReq:
		return "SysReq";
	case Qt::Key_Clear:
		return "Clear";
	case Qt::Key_Home:
		return "Home";
	case Qt::Key_End:
		return "End";
	case Qt::Key_Left:
		return "Left Arrow";
	case Qt::Key_Up:
		return "Up Arrow";
	case Qt::Key_Right:
		return "Right Arrow";
	case Qt::Key_Down:
		return "Down Arrow";
	case Qt::Key_PageUp:
		return "Page Up";
	case Qt::Key_PageDown:
		return "Page Down";
	case Qt::Key_Shift:
		return "Shift";
	case Qt::Key_Control:
		return "Control";
	case Qt::Key_Meta:
		return "Meta";
	case Qt::Key_Alt:
		return "Alt";
	case Qt::Key_AltGr:
		return "AltGr";
	case Qt::Key_CapsLock:
		return "Caps Lock";
	case Qt::Key_NumLock:
		return "Num Lock";
	case Qt::Key_ScrollLock:
		return "Scroll Lock";
	case Qt::Key_F1:
		return "F1";
	case Qt::Key_F2:
		return "F2";
	case Qt::Key_F3:
		return "F3";
	case Qt::Key_F4:
		return "F4";
	case Qt::Key_F5:
		return "F5";
	case Qt::Key_F6:
		return "F6";
	case Qt::Key_F7:
		return "F7";
	case Qt::Key_F8:
		return "F8";
	case Qt::Key_F9:
		return "F9";
	case Qt::Key_F10:
		return "F10";
	case Qt::Key_F11:
		return "F11";
	case Qt::Key_F12:
		return "F12";
	case Qt::Key_Super_L:
		return "Super L";
	case Qt::Key_Super_R:
		return "Super R";
	case Qt::Key_Menu:
		return "Menu";
	case Qt::Key_Hyper_L:
		return "Hyper L";
	case Qt::Key_Hyper_R:
		return "Hyper R";
	case Qt::Key_Help:
		return "Help";
	case Qt::Key_Direction_L:
		return "Direction L";
	case Qt::Key_Direction_R:
		return "Direction R";
	case Qt::Key_Space:
		return "Space";
	}
}

//static
QString AuKeyFilter::ModifierName(Qt::KeyboardModifiers modifiers)
{
	QString modname;
	if(modifiers & Qt::AltModifier)
		modname += "Alt+";
	if(modifiers & Qt::ControlModifier)
		modname += "Ctrl+";
	if(modifiers & Qt::MetaModifier)
		modname += "Meta+";
	if(modifiers & Qt::ShiftModifier)
		modname += "Shift+";
	return modname;
}
