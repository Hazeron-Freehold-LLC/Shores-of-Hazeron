//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetwindows.h,v 1.3 2022/02/19 17:47:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetWindows_H
#define AuSetWindows_H

#include <QWidget>

class AuSetWindows : public QWidget
{
	Q_OBJECT;
public:
	AuSetWindows(QWidget *parent=0);
	~AuSetWindows();

protected slots:
	void DockBottomChanged(int state, int value);
	void DockLeftChanged(int state, int value);
	void DockRightChanged(int state, int value);
	void DockTopChanged(int state, int value);
	void ResetWidgets();
	void SetActiveTrans(int value);
	void SetInactiveTrans(int value);
	void TestColorDialog();
	void TestFileDialog();
	void TestFontDialog();
	void ToolBarBottomChanged(int state, int value);
	void ToolBarLeftChanged(int state, int value);
	void ToolBarRightChanged(int state, int value);
	void ToolBarTopChanged(int state, int value);
	void UseNativeColorChanged(int checkState);
	void UseNativeFileChanged(int checkState);
	void UseNativeFontChanged(int checkState);
};

#endif // AuSetWindows_H
