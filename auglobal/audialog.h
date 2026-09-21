//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audialog.h,v 1.20 2022/02/19 18:30:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDialog_H
#define AuDialog_H

#include "audlgtitle.h"
#include <QDialog>

class QUrl;
class QVBoxLayout;

class AUGLOBAL_DLL AuDialog : public QDialog
{
    Q_OBJECT;
private:
	bool			m_hasFocus;
	bool			m_IsResizing;		// True if the widget is being resized.
	bool			m_IsResizingCursor;	// True if we have changed the cursor into a resizing cursor.
	bool			m_MoveBottom;		// True if bottom edge is moving during resize.
	bool			m_MoveLeft;			// True if left edge is moving during resize.
	bool			m_MoveRight;		// True if right edge is moving during resize.
	bool			m_MoveTop;			// True if top edge is moving during resize.

	// Widgets
    AuDlgTitleBar	*m_titleBar;
    QVBoxLayout		*m_vbox;

public:
    AuDialog(QWidget* parent, bool bHasTitle=true);
    ~AuDialog();
    
    void changeEvent(QEvent *e);

	inline bool IsResizing() const
	{
		return m_IsResizing;
	}

    void paintEvent(QPaintEvent *pe);
    
    void setAuDlgTitle(const QString& sTitle);

    void setAuDlgIcon(QIcon Icon);

    void setAuDlgLayout(QLayout* layout);

	// Sets the literal 'url' to open when the help button is pressed.
	// Replaces wiki topic.
	void SetHelpUrl(const QUrl& url);

	// 'topic' is appended to https://www.hazeron.com/wiki/index.php/
	// Spaces in 'topic' are converted to underscores '_'.
	// Replaces help url.
	void SetWikiTopic(const QString& topic);
    
    inline AuDlgTitleBar* titleBar() const
	{
		return m_titleBar;
	}

protected:
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	void UnsetResizeCursor();
};

#endif // AuDialog_H

