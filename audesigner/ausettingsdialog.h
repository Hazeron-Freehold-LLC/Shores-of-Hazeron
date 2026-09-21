//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausettingsdialog.h,v 1.3 2022/02/19 18:23:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSettingsDialog_H
#define AuSettingsDialog_H

#include <audialog.h>

class AuSetRender;
class QStackedLayout;
class QTabBar;

class AuSettingsDialog : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	AuSetRender*	m_PageScene;
	QStackedLayout* m_Pages;
	QTabBar*		m_Tabs;

public:
	AuSettingsDialog(bool showF12, QWidget* parent=0);
	virtual ~AuSettingsDialog();

	void SetOpenGLInfo(int textureUnits, int uniformLocs);

signals:
	void ControlsChanged();
	void MessageLimitChanged(int limit);
	void SoundGainChanged(int sound);
	void SoundOverrideChanged(int sound);

protected:
	virtual void keyPressEvent(QKeyEvent* event);
};

#endif // AuSettingsDialog_H
