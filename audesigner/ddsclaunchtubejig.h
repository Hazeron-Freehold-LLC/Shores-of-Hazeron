//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsclaunchtubejig.h,v 1.7 2022/02/19 18:23:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScLaunchTubeJig_H
#define DdScLaunchTubeJig_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScLaunchTubeJig : public DdCommand  
{
private:
	QCursor								m_Cursor;
	quint8								m_Mode;					// 0=primary boundary, 1=hole, 2=height pt, 3=vector pt
	AuPointArray						m_Points;
	int									m_Steps;				// non-zero when round

public:
	DdScLaunchTubeJig(AuDesignerView* view);
	virtual ~DdScLaunchTubeJig();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual QString WikiTopic() const;

protected:
	bool MakeJig(const AuPoint& vec);

	virtual void UpdatePrompt() const;
};

#endif // DdScLaunchTubeJig_H
