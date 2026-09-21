//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audocktitle.h,v 1.9 2022/02/19 18:30:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDockTitle_H
#define AuDockTitle_H

#include "augloballib.h"
#include <QUrl>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QToolButton;

class AUGLOBAL_DLL AuDockTitle : public QWidget
{
	Q_OBJECT;
private:
	QAction*			m_CloseAction;
	QAction*			m_FloatAction;
	QString				m_Help;
	QAction*			m_HelpAction;
	QUrl				m_HelpUrl;
	Qt::Orientation		m_Orientation;							// Orientation of this title bar.

	// Widgets
	QToolButton*		m_CloseButton;
	QToolButton*		m_FloatButton;
	QHBoxLayout*		m_hbox_TitleBar;
	QToolButton*		m_HelpButton;
	QToolButton*		m_Icon;
	QLabel*				m_label;	//title text

public:
    AuDockTitle(QWidget* parent);
 
	virtual QSize minimumSizeHint() const;

	// Sets the literal 'url' to open when the help button is pressed.
	// Replaces wiki topic.
	void SetHelpUrl(const QUrl& url);

	void SetText(const QString& titleText);

    void setTitleIcon(QIcon Icon);

	// 'topic' is appended to https://www.hazeron.com/wiki/index.php/
	// Spaces in 'topic' are converted to underscores '_'.
	// Replaces help url.
	void SetWikiTopic(const QString& topic);

	virtual QSize sizeHint() const;

public slots:
	void ShowHelp();
    void showIconMenu();
	void ToggleFloat();
 
protected:
    void mouseDoubleClickEvent(QMouseEvent* me);
    void mouseMoveEvent(QMouseEvent* me);
    void mousePressEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);
    
    void contextMenuEvent(QContextMenuEvent*);
    
private:
    void createActions();
    void createSysMenu(QPoint point);
};
#endif // AuDockTitle_H

