//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerbar.h,v 1.3 2022/02/19 18:21:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerBar_H
#define AuDesignerBar_H

#include "aupoint.h"
#include <QToolBar>

class QAction;
class QEvent;
class QKeyEvent;
class QLabel;
class QLineEdit;

class AuDesignerBar : public QToolBar
{
	Q_OBJECT;
private:
	// Widgets
	QAction*							m_DesignProps;
	QAction*							m_GravityToggle;
	QLineEdit*							m_InputEdit;			// Edit control for entering coordinates.
	QLabel*								m_InputPrompt;			// Prompter for current command.
	QAction*							m_PreviewBd;
	QAction*							m_PreviewPeople;
	QAction*							m_PreviewSettings;
	QAction*							m_PreviewToggle;
	QAction*							m_RefreshObstructions;
	QAction*							m_SolidsBlockPath;

public:
	AuDesignerBar(QWidget* parent=0);
	virtual ~AuDesignerBar();

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	bool KeyReceived(QKeyEvent* e);

	void SetGravity(bool g);

	void SetPreview(bool p);

	void SetSolidsBlockPath(bool block);

public slots:
	void EnableControls();
	void GravityToggle();
	void PreviewBd();
	void PreviewPeople();
	void PreviewSettings();
	void PreviewToggle();
	void SetInputPrompt(QString prompt);
	void SolidsBlockPath();

protected:
	virtual void changeEvent(QEvent* event);
	virtual void keyPressEvent(QKeyEvent* e);

signals:
	void CoordinateEntered(QString, int/*Qt::KeyboardModifiers*/);
};

#endif // AuDesignerBar_H
