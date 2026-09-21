//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscextrusion.h,v 1.8 2022/02/19 18:23:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScExtrusion_H
#define DdScExtrusion_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScExtrusion : public DdCommand  
{
private:
	QCursor								m_Cursor;
	unsigned int						m_ExtSteps;
	quint8								m_Mode;					// 0=primary boundary, 1=hole, 2=height pt, 3=vector pt
	quint8								m_PartType;				// AuScPart::StaticType of the part being made.
	AuPointArray						m_Points;

public:
	DdScExtrusion(quint8 partType, AuDesignerView* view);
	virtual ~DdScExtrusion();

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

#endif // DdScExtrusion_H
