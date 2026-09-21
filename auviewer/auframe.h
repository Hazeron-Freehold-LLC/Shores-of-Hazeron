//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auframe.h,v 1.12 2022/04/15 21:05:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuFrame_H
#define AuFrame_H

#include "audef.h"
#include <QComboBox>
#include <QMainWindow>
#include <QMutex>

class AuDNASettingsDlg;
class AuView;
class QOpenGLWidget;
class QTimer;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFrame : public QMainWindow
{
	Q_OBJECT;
private:
	AuDNASettingsDlg*					m_DNASettings;
	AuView*								m_View;

public:
	AuFrame(QWidget *parent=0);
	~AuFrame();

	virtual QSize sizeHint() const;

	inline AuView* View() const
	{
		return m_View;
	}

protected:
	virtual void closeEvent(QCloseEvent* event);

public slots:
	void DNASettings();

	void LoadMeatLine();

	void LoadMeatProfile();

	void NewMeatLine();

	void NewMeatProfile();

	void ResetMenuBar(int mode);

	void SaveMeatLine();

	void SaveMeatProfile();

	void ViewBody();

	void ViewFemalePrimate();

	void ViewItem();

	void ViewMalePrimate();

	void ViewModel();
};

#endif // AuFrame_H
