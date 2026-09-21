//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupictureframesettings.h,v 1.6 2022/02/19 17:59:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPictureFrameSettings_H
#define AuPictureFrameSettings_H

#include "auclientlib.h"
#include <audialog.h>

class QLineEdit;
class QCheckBox;

class AUCLIENT_DLL AuPictureFrameSettings : public AuDialog
{
	Q_OBJECT;

private:
	// Widgets
	QCheckBox*							m_HideURLCheck;
	QCheckBox*							m_RotateCheck;
	QLineEdit*							m_URLEdit;

public:
	AuPictureFrameSettings(const QString& url, bool hideURL, bool rotatePicture, QWidget* parent=0);
	virtual ~AuPictureFrameSettings();

	bool HideURL() const;

	QString PictureURL() const;

	bool RotatePicture() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuPictureFrameSettings_H
