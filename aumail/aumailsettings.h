//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailsettings.h,v 1.7 2022/02/19 18:36:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailSettings_H
#define AuMailSettings_H

#include "aumaillib.h"
#include "audialog.h"

class QCheckBox;
class QComboBox;
class QSpinBox;

class AUMAIL_DLL AuMailSettings : public AuDialog
{
	Q_OBJECT;
private:
	int									m_IconIndex;
	QTimer*								m_timer;				// UpdateContents timer

	// Widgets
	QCheckBox*							citycb;
	QSpinBox*							cityhr;
	QComboBox*							ckmail;
	QCheckBox*							fleetcb;
	QSpinBox*							fleethr;
	QCheckBox*							officercb;
	QSpinBox*							officerhr;

public:
	AuMailSettings(QWidget* parent=0);
	virtual ~AuMailSettings();

public slots:
	virtual void setVisible(bool visible);

protected:
	virtual void hideEvent(QHideEvent* event);
	void RestoreGeometry();
	void SaveGeometry();
	virtual void showEvent(QShowEvent* event);

protected slots:
	void Accept();
	void RestoreDefaults();
	void UpdateContents();
};

#endif // AuMailSettings_H
