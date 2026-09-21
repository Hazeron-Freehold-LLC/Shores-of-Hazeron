//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscroomvoidspindle.h,v 1.4 2022/02/19 18:24:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScRoomVoidSpindle_H
#define DdScRoomVoidSpindle_H

#include "audef.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScRoomVoidSpindle : public DdCommand  
{
private:
	QCursor								m_Cursor;
	GRAVITY								m_GravityType;
	HOLD_ACCESS							m_HoldAccess;
	quint8								m_Mode;
	quint8								m_PersonnelAccess;
	AuPointArray						m_Points;
	int									m_SpinAxis;
	int									m_SpinSteps;
	AuPoint								m_UpStart;

public:
	DdScRoomVoidSpindle(AuDesignerView* view);
	virtual ~DdScRoomVoidSpindle();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual QString WikiTopic() const;

protected:
	bool MakeSpindle(quint16 steps, int axis, GRAVITY gravityType, const AuPoint& gravityN, HOLD_ACCESS holdAccess,
			quint8 personnelAccess);

	virtual void UpdatePrompt() const;
};

#endif // DdScRoomVoidSpindle_H
