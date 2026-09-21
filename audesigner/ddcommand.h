//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddcommand.h,v 1.9 2022/02/19 18:23:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdCommand_H
#define DdCommand_H

#include "aupoint.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QString>

class AuAtomRenderInfo;
class AuBdReport;
class AuDesignerView;
class AuGLEffects;
class AuMatrix4x4;
class DaPerson;
class QOpenGLWidget;

class DdCommand  
{
protected:
	AuDesignerView*						m_pWnd;					// The AuDesignerView that initiated this command.

public:
	DdCommand(AuDesignerView* pWnd);
	virtual ~DdCommand();

	// A point was entered by the user, either with the mouse or by entering something into the edit control of AuDesignerBar.
	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
	{
		Q_UNUSED(fromKeyboard);
		Q_UNUSED(keymods);
		return loc;
	}

	virtual AuMatrix4x4 DesignPlane() const;

	// Keyboard monitoring functions.
	// Forwards the key event to AuView::DesignerKeyReceived for coordinate input into the AuDesignerBar.
	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent*);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyUp(QKeyEvent*)
	{
		return false;
	}

	// Mouse monitoring functions.
	virtual void OnButtonDblClk(QMouseEvent*)
	{
	}

	virtual void OnButtonDown(QMouseEvent*);

	virtual void OnButtonUp(QMouseEvent*)
	{
	}

	virtual void OnMouseMove(QMouseEvent*)
	{
	}

	virtual void OnMouseWheel(QWheelEvent*)
	{
	}

	// Cursor rendering functions.
	// Create device objects used to render the cursor.
	virtual void CreateDeviceObjects(AuGLEffects&)
	{
	}

	// Delete device objects used to render the cursor.
	virtual void DeleteDeviceObjects(AuGLEffects&)
	{
	}

	// This override calls the other RenderCursor using parameters of 'ari.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& camera, DaPerson* sceneAtom,
			const QPoint& pxCursor, AuAtomRenderInfo& ari, bool snappedToExistingShape);

protected:	// Old version
	// Render "cursor" objects in the scene.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& camera, DaPerson* sceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
	{
		Q_UNUSED(projection);
		Q_UNUSED(camera);
		Q_UNUSED(sceneAtom);
		Q_UNUSED(pxCursor);
		Q_UNUSED(re);
		Q_UNUSED(glw);
		Q_UNUSED(snappedToExistingShape);
	}

public:
	// Run level 0 kills all running commands when it starts or restarts.
	// Run level 1 kills only run level 1 commands when it starts or restarts.
	// Allows temporary push of current running level 0 command to allow run of a level 1 command, then resume the level 0 command.
	// Commands to move the design grid provide an example of level 1 commands.
	virtual int RunLevel() const
	{
		return 0;
	}

	virtual void SetConstructionProcess(const AuBdReport& report)
	{
		Q_UNUSED(report);
	}

	// Set appropriate device cursor. Default sets Cross cursor if pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* pSceneAtom, const QPoint& /*pxCursor*/);

	// 'constructionMaterial' is a COMMODITYID.
	virtual void SetMaterial(int constructionMaterial)
	{
		Q_UNUSED(constructionMaterial);
	}

	// Called once when the command is started, right after it's been constructed.
	// Calls UpdatePrompt.
	virtual void Start();

	virtual void Terminate();

	// Updates the prompt text on the AuDesignerBAr. New designer uses this. Old designers do not.
	virtual void UpdatePrompt() const
	{
	}

	// Returns a topic appropriate for a call to AuFrame::ShowWikiTopic.
	// Returns QString() if no wiki topic exists.
	virtual QString WikiTopic() const
	{
		return QString();
	}

protected:
	// 'mvp' is the model view projection needed to calculate the view location of 'designPlaneLoc'.
	// 'loc' is the location of the blip, in coordinates relative to the parent design environment.
	// 'designPlane' is the current design plane, so we can show the coordinates of the location relative to it.
	void DrawBlip(const AuMatrix4x4& mvp, const AuPoint& loc, const AuMatrix4x4& designPlane, AuGLEffects& re, QOpenGLWidget* glw,
			bool snappedToExistingShape);

	// Calls AuFrame::ShowWikiTopic if my WikiTopic function returns anything.
	// Returns true if my WikiTopic is not blank.
	// Returns false if my WikiTopic is blank.
	bool ShowWiki();
};

#endif // DdCommand_H
