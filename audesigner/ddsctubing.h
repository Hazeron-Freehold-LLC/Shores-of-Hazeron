//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctubing.h,v 1.7 2022/02/19 18:24:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScTubing_H
#define DdScTubing_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScTubing : public DdCommand  
{
private:
	QCursor								m_Cursor;
	AuPoint								m_Guide;
	quint8								m_Mode;					// 0=tubing path, 1=tubing profile, 2=guide point
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::StaticType of the part being made.
	AuPointArray						m_Points;				// Tubing path
	AuPointArray						m_PointsShape;			// Tubing shape

public:
	DdScTubing(quint8 partType, AuDesignerView* view);
	virtual ~DdScTubing();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual QString WikiTopic() const;

protected:
	bool MakeTube();

	bool MakeTube(double diameter, int sides);

	virtual void UpdatePrompt() const;
};

#endif // DdScTubing_H
