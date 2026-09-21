//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulauncherdialog.h,v 1.27 2022/04/15 20:36:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLauncherDialog_H
#define AuLauncherDialog_H

#include "auelapsedtimer.h"
#include <QMap>
#include <QSettings>
#include <QSslError>
#include <QUrl>
#include <QWidget>

class QLabel;
class QListWidget;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class QToolButton;

class AuLauncherNetworkReply : public QObject
{
	Q_OBJECT;
	Q_DISABLE_COPY(AuLauncherNetworkReply);
public:
	QNetworkReply*						m_NetReply;
	QUrl								m_Url;

public:	// Constructors
	AuLauncherNetworkReply()
	{
		m_NetReply = 0;
	}

	AuLauncherNetworkReply(const QUrl& url, QNetworkReply* reply);

	/*
	AuLauncherNetworkReply(const AuLauncherNetworkReply& c) : QObject()
	{
		m_NetReply = c.m_NetReply;
		m_Url = c.m_Url;
	}

public:	// Operators
	const AuLauncherNetworkReply& operator=(const AuLauncherNetworkReply& c)
	{
		//QObject::operator=(c);
		m_NetReply = c.m_NetReply;
		m_Url = c.m_Url;
		return *this;
	}
	*/

public:	// Methods

protected slots:
	void BaseDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
	void DownloadProgress(QUrl url, qint64 bytesReceived, qint64 bytesTotal);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuLauncherDialog : public QWidget
{
	Q_OBJECT;
private:
	QString								m_Arch;					// Architecture folder for download.
	QImage								m_Backdrop;
	QStringList							m_BackdropHistory;		// The most recent backdrop choices, up to 100.
	AuElapsedTimer						m_BackdropTime;
	int									m_CommState;			// 0=nothing, 1=waiting for checksums, 2=waiting for files, 3=error
	QNetworkAccessManager*				m_Net;
	QMap<QString, AuLauncherNetworkReply*>	m_NetReplies;		// Key is file path, value is active network reply.
	QSettings							m_Settings;
	QTimer*								m_Timer;

	// Widgets
	QLabel*								m_ErrorMessage;
	QListWidget*						m_Files;
	QToolButton*						m_Init;
	QToolButton*						m_LaunchCommander;
	QToolButton*						m_LaunchMail;
	QToolButton*						m_LaunchSoH;
	//QToolButton*						m_LaunchStoryEditor;
	QToolButton*						m_SettingsButton;
	QLabel*								m_UpdateMessage;
	QToolButton*						m_WebSite;

public:
	AuLauncherDialog(QWidget* parent=0);
	virtual ~AuLauncherDialog();

	virtual QSize sizeHint() const;

public slots:
	void CheckForLauncherUpdates();
	void CheckForProgramUpdates();
	void InitInstall();
	void LaunchCommander();
	void LaunchMail();
	void LaunchSoH();
	void LaunchStoryEditor();
	void NetSSLErrors(QNetworkReply* reply, const QList<QSslError>& errors);
	void UpdateFiles();

protected slots:
	void DownloadProgress(QUrl url, qint64 bytesReceived, qint64 bytesTotal);
	void HideLauncherButtons();
	void LoadNextBackdrop();
	void LoadPreviousBackdrop();
	void LoadRandomBackdrop();
	void NetReplyFinished(QNetworkReply* reply);
	void Settings();
	void ShowLauncherButtons();
	void ToggleVisibleButtons();
	void UpdateStatus();
	void WebSite();

protected:
	QString Arch() const;
	bool CanLaunchCommander() const;
	bool CanLaunchMail() const;
	bool CanLaunchSoH() const;
	bool CanLaunchStoryEditor() const;
	QString FileDir() const;
	void FixSymbolicLinks();
	void FixSymbolicLink(const QString& app, const QString& qtLib);
	virtual void hideEvent(QHideEvent* event);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void paintEvent(QPaintEvent* pe);
	virtual void showEvent(QShowEvent* event);
	virtual void wheelEvent(QWheelEvent* e);
};

#endif // AuLauncherDialog_H
