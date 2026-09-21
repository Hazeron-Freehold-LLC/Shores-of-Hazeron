//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdsite.h,v 1.8 2022/02/19 18:23:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdBdSite_H
#define DdBdSite_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdBdSite : public DdCommand  
{
private:
	int									m_Blend;
	int									m_Grade;
	int									m_Grass;
	int									m_Pavement;
	AuPointArray						m_Points;
	int									m_Rocks;
	bool								m_Round;
	int									m_Shrubs;
	int									m_Steps;
	int									m_Trees;

public:
	DdBdSite(AuDesignerView* view);
	virtual ~DdBdSite();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual void Start();

	virtual QString WikiTopic() const;

protected:
	void MakeLotAgricultural();
	void MakeLotCommercial();
	void MakeLotIndustrial();
	void MakeLotMine();
	void MakeLotResidential();
	void MakeLotShore();
	bool MakeOutline(const AuPointArray& points, int gradeMode, int blend, int pavement, int rocks, int grass, int shrubs,
			int trees);
	virtual void UpdatePrompt() const;
};

#endif // DdBdSite_H
