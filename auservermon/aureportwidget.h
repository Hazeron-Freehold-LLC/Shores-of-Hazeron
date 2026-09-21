//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aureportwidget.h,v 1.8 2022/02/19 18:40:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuReportWidget_H
#define AuReportWidget_H

#include "auid.h"
#include "dtmgrclient.h"
#include <QWidget>

class AuHistoPair;
class AuTableWidget;
class QSettings;
class QTableWidgetItem;

class AuReportWidget : public QWidget
{
	Q_OBJECT;
private:
	QList<AuStatusReport>	m_Status;
	QString					m_ServerName;
	quint16					m_ServerPort;
	int						m_TMinus;			// milliseconds offset from last update
	int						m_TMinusCountDown;	// seconds before m_TMinus is reset to 0.

	// Widgets
	AuTableWidget*			m_Clients;
	AuHistoPair*			m_HistoPair;

public:
	AuReportWidget(QWidget* parent=0);
	virtual ~AuReportWidget();

	void RestoreSettings(const QString& key, QSettings& settings);

	void SaveSettings(const QString& key, QSettings& settings);

	void SetStatus(const QString& serverName, quint16 serverPort, const QList<AuStatusReport>& status)
	{
		m_ServerName = serverName;
		m_ServerPort = serverPort;
		m_Status = status;
	}

signals:
	void KickUnit(QString, quint16, quint32);
	void TMinus(int ms);

protected slots:
	void ClientClicked(QTableWidgetItem* item);
	void SetTMinus(int ms);
	void SystemRightClicked(AuId systemId);
	void TimerUpdate();
};

#endif // AuReportWidget_H
