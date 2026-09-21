//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumessagebox.h,v 1.7 2022/02/19 18:31:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMessageBox_H
#define AuMessageBox_H

#include "augloballib.h"
#include <QDialog>

class AuDlgTitleBar;
class QVBoxLayout;

class AUGLOBAL_DLL AuMessageBox : public QDialog
{
    Q_OBJECT
    Q_ENUMS(AuIcon)
private:
	// Widgets
    AuDlgTitleBar*		m_titleBar;
    QVBoxLayout*		m_vbox;

public:
	enum AuIcon 
	{
        NoIcon = 0,
        Information = 1,
        Warning = 2,
        Critical = 3,
        Question = 4
    };
    
    AuMessageBox(int icon, const QString &title, const QString &text, QString buttons, QWidget *parent = 0,
			Qt::WindowFlags f = Qt::Dialog);
    ~AuMessageBox();
        
    //the dialog layout with the title bar
    inline AuDlgTitleBar* titleBar() const
	{
		return m_titleBar;
	}
        
    void paintEvent(QPaintEvent *pe);
};

#endif // AuMessageBox_H
