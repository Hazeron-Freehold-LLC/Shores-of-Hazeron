//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerbar2.h,v 1.2 2022/02/19 18:21:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerBar2_H
#define AuDesignerBar2_H

#include "aupoint.h"
#include <QToolBar>

class QAction;
class QComboBox;
class QEvent;
class QLabel;
class QLineEdit;

class AuDesignerBar2 : public QToolBar
{
	Q_OBJECT;
private:
	// Widgets
	QLabel*								m_AnchorDesc;			// Description of selected anchor element.
	QLineEdit*							m_BlipCoord;			// Coordinate at reference blip location.
	QAction*							m_FindEdges;
	QAction*							m_FindFaces;
	QAction*							m_FindVertices;
	QAction*							m_GridFollowsWork;		// Design plane follows work toggle.
	QComboBox*							m_GridIncs;
	QAction*							m_GridXY;
	QAction*							m_GridXZ;
	QAction*							m_GridYZ;
	QAction*							m_MoveGrid;
	QAction*							m_ObjectIsolate;
	QAction*							m_ObjectMode;
	QAction*							m_PositionGrid;
	QAction*							m_ShowGrid;				// Show grid.
	QAction*							m_ShowBacks;			// Show back sides of solid faces.
	QAction*							m_ShowEdges;			// Show edges.
	QAction*							m_ShowFaces;			// Show solid faces.
	QAction*							m_SnapToEdges;			// Snap-to-Edges toggle
	QAction*							m_SnapToGrid;			// Snap-to-Grid toggle
	QAction*							m_SnapToVertices;		// Snap-to-Vertices toggle

public:
	AuDesignerBar2(QWidget* parent=0);
	virtual ~AuDesignerBar2();

	void SetFindEdges(bool findEm);

	void SetFindFaces(bool findEm);

	void SetFindVertices(bool findEm);

	void SetGridFollowsWork(bool follow);

	void SetObjectIsolate(bool objectIsolate);

	void SetObjectMode(bool objectMode);

	void SetShowBacks(bool showEm);

	void SetShowEdges(bool showEm);

	void SetShowFaces(bool showEm);

	void SetShowGrid(bool showEm);

	void SetSnapToEdges(bool snap);

	void SetSnapToGrid(bool snap);

	void SetSnapToVertices(bool snap);

public slots:
	void EnableControls();
	void FindEdges();
	void FindFaces();
	void FindVertices();
	void GridFollowsWork();
	void GridSpacingChanged(int);
	void ObjectIsolate();
	void ObjectMode();
	void SetAnchorDesc(QString desc);
	void SetBlipCoordinate(AuPoint coord);
	void ShowBacks();
	void ShowEdges();
	void ShowFaces();
	void ShowGrid();
	void SnapToEdges();
	void SnapToGrid();
	void SnapToVertices();

protected:
	virtual void changeEvent(QEvent* event);
};

#endif // AuDesignerBar2_H
