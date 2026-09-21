//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulaunchersettings.h,v 1.7 2022/02/19 18:34:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLauncherSettings_H
#define AuLauncherSettings_H

#include <QDialog>

class QComboBox;
class QLineEdit;

class AuLauncherSettings : public QDialog
{
	Q_OBJECT;

	// Widgets
	QLineEdit*							m_CacheEdit;
	QLineEdit*							m_FilesEdit;
	QComboBox*							m_OpenGLCombo;
	QLineEdit*							m_PicsEdit;

public:
	AuLauncherSettings(const QString& sohFolder, const QString& picFolder, const QString& cacheFolder, int glVer, float myVer,
			const QString& arch, QWidget* parent=0);

	virtual ~AuLauncherSettings();

	QString CacheFolder() const;

	int OpenGLVersion() const;

	QString PictureFolder() const;

	QString SoHFolder() const;

protected slots:
	void ChooseCacheFolder();
	void ChoosePictureFolder();
	void ChooseSoHFolder();
	void OpenAppleOpenGLPage();
};

#endif // AuLauncherSettings_H
