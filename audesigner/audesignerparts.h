//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerparts.h,v 1.2 2022/02/19 18:21:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerParts_H
#define AuDesignerParts_H

#include "audockwidget.h"
#include "auscface.h"
#include <QTableWidget>
#include <QWidget>

class AuScParts;
class AuScSelection;
class QTableWidgetItem;
class QTimer;

class AuDesignerPartsTable : public QTableWidget
{
	Q_OBJECT;
	int									m_ButtonsPressed;
	QPoint								m_PressPos;

public:
	AuDesignerPartsTable(QWidget* parent=Q_NULLPTR);

protected:
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);

signals:
	void MouseDoubleClicked();
	void MouseReleasedInside();
	void MouseReleasedOutside();
	void MouseRightClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDesignerParts : public QWidget
{
	Q_OBJECT;
	AuScParts*							m_Parts;				// My copy of the parts, for updates.
	AuScSelection*						m_PartSel;
	QTimer*								m_timer;				// UpdateContents timer

	// Widgets
	AuDesignerPartsTable*				m_PartsTable;

public:
	AuDesignerParts(QWidget* parent=0);
	virtual ~AuDesignerParts();

	AuScIdSet SelectedParts() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	void RestoreSettings();
	void SaveSettings();
	void SetPartSelected(quint16 partId, bool isSelected);
	virtual void showEvent(QShowEvent* event);

public slots:
	void EnableControls();

protected slots:
	void OpacityChanged(int iOpacity, int partId);
	void PartClicked(QTableWidgetItem* item);
	void PartsRightClicked();
	void SelectionChanged();
	void StateChanged(int index, int partId);
	void UpdateContents();

signals:
	void GoToSelection();
	void PartsSelectionChanged(AuScIdSet selected);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDesignerPartsDock : public AuDockWidget
{
	Q_OBJECT;
public:
	AuDesignerParts*					m_Parts;

public:
	AuDesignerPartsDock(QWidget* parent);

	void EnableControls()
	{
		m_Parts->EnableControls();
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas);

	virtual void UpdateTitleShortcut();

signals:
	void GoToSelection();
	void PartsSelectionChanged(AuScIdSet selected);
};

#endif // AuDesignerParts_H
