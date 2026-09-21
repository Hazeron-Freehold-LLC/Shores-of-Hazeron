//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audockwidget.h,v 1.13 2022/02/19 18:30:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDockWidget_H
#define AuDockWidget_H

#include "augloballib.h"
#include <QDockWidget>

class AUGLOBAL_DLL AuDockWidget : public QDockWidget
{
	Q_OBJECT;
	QString								m_OriginalTitle;

public:
	// Use this constructor to show a custom AuDockTitle titlebar. 
	// 'titleIcon' appears on the title bar of the dock widget.
	// 'titleText' appears on the title bar of the dock widget. May contain html.
	// 'windowTitle' appears on tabs and the popup menu for showing/hiding dock widgets. May not contain html.
	AuDockWidget(const QIcon& titleIcon, const QString& titleText, const QString& windowTitle, QWidget* parent=0);

	AuDockWidget(const QString& titleIcon, const QString& titleText, const QString& windowTitle, QWidget* parent=0);

	// Use this constructor for the default title-bar behavior of a QDockWidget.
	AuDockWidget(QWidget* parent=0);

	virtual ~AuDockWidget();

	// Returns the original 'titleText' parameter that was passed to the constructor. Useful when implementing UpdateTitleShortcut.
	const QString& OriginalTitle() const
	{
		return m_OriginalTitle;
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
	}

	// Sets the literal 'url' to open when the help button is pressed.
	// Replaces wiki topic.
	void SetHelpUrl(const QUrl& url);

	void SetTitle(const QString& text);

	// 'topic' is appended to https://www.hazeron.com/wiki/index.php/
	// Spaces in 'topic' are converted to underscores '_'.
	// Replaces help url.
	void SetWikiTopic(const QString& topic);

	// Dockbars may show the key sequence to show/hide them on the title bar. This function is called when control bindings are
	// loaded or changed, to set it properly.
	virtual void UpdateTitleShortcut()
	{
	}

protected:
	virtual void changeEvent(QEvent* event);
};

#endif // AuDockWidget_H
