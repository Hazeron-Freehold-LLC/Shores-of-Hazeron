//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audlgtitle.h,v 1.12 2022/02/19 18:30:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDlgTitle_H
#define AuDlgTitle_H

#include "augloballib.h"
#include <QUrl>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QToolButton;

class AUGLOBAL_DLL AuDlgTitleBar : public QWidget
{
	Q_OBJECT;
private:
	QPoint				clickPos;
	QAction*			closeAct;
	bool				m_hasFocus;
	QString				m_Help;
	QAction*			m_HelpAction;
	QUrl				m_HelpUrl;
	QAction*			maxAct;
	bool				maxNormal;
	QPixmap				maxPix;
	QAction*			minAct;
	QPixmap				restorePix;
	QPoint				startPos;

	// Widgets
	QToolButton*		m_close;
	QHBoxLayout*		m_hbox_TitleBar;
	QToolButton*		m_HelpButton;
	QToolButton*		m_Icon;
	QLabel*				m_label;	//title text
	QToolButton*		m_minimize;
	QToolButton*		m_maximize;

public:
    AuDlgTitleBar(QWidget *parent);
    
	inline void SetHasFocus(bool hasFocus)
	{
		m_hasFocus = hasFocus;
	}

	// Sets the literal 'url' to open when the help button is pressed.
	// Replaces wiki topic.
	void SetHelpUrl(const QUrl& url);

    void setMax(bool bSet = true);

    void setMin(bool bSet = true);

	void SetText(const QString& titleText);

    void setTitleIcon(QIcon Icon);

	// 'topic' is appended to https://www.hazeron.com/wiki/index.php/
	// Spaces in 'topic' are converted to underscores '_'.
	// Replaces help url.
	void SetWikiTopic(const QString& topic);
 
public slots:
	void ShowHelp();
    void showIconMenu();
    void showMaxRestore();
    void showSmall();
 
protected:
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent *event);
    
    void contextMenuEvent(QContextMenuEvent *);
    
private:
    void createActions();
    void createSysMenu(QPoint point);
};

#endif // AuDlgTitle_H

