//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushutdowndlg.h,v 1.2 2022/02/19 18:40:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuShutdownDlg_H
#define AuShutdownDlg_H

#include <QDialog>

class QLineEdit;

class AuShutdownDlg : public QDialog
{
	Q_OBJECT;
private:
	QString								m_HaltMsg;
	qint16								m_ShutdownSecs;
	QString								m_WarnMsg;

	// Widgets
	QLineEdit*							m_leHalt;
	QLineEdit*							m_leSecs;
	QLineEdit*							m_leWarn;

public:
	AuShutdownDlg(QWidget* parent=0);
	virtual ~AuShutdownDlg();

	inline const QString& HaltMsg() const
	{
		return m_HaltMsg;
	}

	inline const QString& WarnMsg() const
	{
		return m_WarnMsg;
	}

	inline qint16 ShutdownSeconds() const
	{
		return m_ShutdownSecs;
	}

public slots:
	virtual void setVisible(bool visible);
	void Shutdown();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuShutdownHaltDlg : public QDialog
{
	Q_OBJECT;
private:
	QString								m_HaltMsg;

	// Widgets
	QLineEdit*							m_leHalt;

public:
	AuShutdownHaltDlg(QWidget* parent=0);
	virtual ~AuShutdownHaltDlg();

	inline const QString& Message() const
	{
		return m_HaltMsg;
	}

public slots:
	virtual void setVisible(bool visible);
	void HaltShutdown();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuShutdownDlg_H
