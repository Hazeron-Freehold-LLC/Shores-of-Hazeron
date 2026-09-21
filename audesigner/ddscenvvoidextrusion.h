//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscenvvoidextrusion.h,v 1.5 2022/02/19 18:23:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScEnvVoidExtrusion_H
#define DdScEnvVoidExtrusion_H

#include "audef.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScEnvVoidExtrusion : public DdCommand  
{
private:
	QCursor								m_Cursor;
	unsigned int						m_ExtSteps;
	AuPoint								m_ExtVec;
	quint8								m_Mode;					// 0=primary boundary, 1=hole, 2=height pt, 3=vector pt
	AuPointArray						m_Points;

public:
	DdScEnvVoidExtrusion(AuDesignerView* view);
	virtual ~DdScEnvVoidExtrusion();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual QString WikiTopic() const;

protected:
	bool MakeExtrusion(const AuPoint& vec);

	virtual void UpdatePrompt() const;
};

#endif // DdScEnvVoidExtrusion_H
