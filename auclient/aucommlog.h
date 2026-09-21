//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommlog.h,v 1.6 2022/02/19 17:54:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommLog_H
#define AuCommLog_H

#include "auclientlib.h"
#include "audialog.h"

class QTextEdit;

class AUCLIENT_DLL AuCommLog : public AuDialog
{
	Q_OBJECT;
private:
	QString								m_ChannelName;

	// Widgets
	QTextEdit*							m_Log;

public:
	AuCommLog(const QString& channel, const QString& channelIcon, const QString& logText, QWidget* parent=0);
	virtual ~AuCommLog();

public slots:
	virtual void setVisible(bool visible);
	void ClearLog();
	void ScrollEnd();
	void ScrollHome();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuCommLog_H
