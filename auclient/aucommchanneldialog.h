//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommchanneldialog.h,v 1.6 2022/02/19 17:54:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommChannelDialog_H
#define AuCommChannelDialog_H

#include "auclientlib.h"
#include "audialog.h"

class AuCommChannel;

class AUCLIENT_DLL AuCommChannelDialog : public AuDialog
{
	Q_OBJECT;
	AuCommChannel*						m_Channel;

public:
	AuCommChannelDialog(QWidget* parent=0);
	virtual ~AuCommChannelDialog();

	void SetChannelWidget(AuCommChannel* channel);

public slots:
	virtual void setVisible(bool visible);

protected:
	virtual void closeEvent(QCloseEvent* e);
	virtual void keyPressEvent(QKeyEvent* event);
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void CloseChannel();
	void RestoreChannel();

signals:
	void CloseChannel(QString channelName);
	void RestoreChannel(AuCommChannel* channel);
};

#endif // AuCommChannelDialog_H
