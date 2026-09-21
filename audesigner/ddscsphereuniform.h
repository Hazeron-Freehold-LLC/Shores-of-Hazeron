//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscsphereuniform.h,v 1.7 2022/02/19 18:24:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScSphereUniform_H
#define DdScSphereUniform_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScSphereUniform : public DdCommand  
{
private:
	QCursor								m_Cursor;
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::StaticType of the part being made.
	AuPointArray						m_Points;

public:
	DdScSphereUniform(quint8 partType, AuDesignerView* view);
	virtual ~DdScSphereUniform();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual QString WikiTopic() const;

protected:
	bool MakeSphere(const AuPoint& center, double radius, quint16 eqSteps, quint16 texReps);

	virtual void UpdatePrompt() const;
};

#endif // DdScSphereUniform_H
