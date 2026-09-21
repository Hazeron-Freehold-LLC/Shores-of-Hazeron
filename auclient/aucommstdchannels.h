//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommstdchannels.h,v 1.6 2022/02/19 17:54:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCOMMSTDCHANNELS_H
#define AUCOMMSTDCHANNELS_H

#include "auclientlib.h"
#include <audialog.h>
#include <QIcon>

class AUCLIENT_DLL AuCommStdChannels : public AuDialog
{
	Q_OBJECT;
public:
	AuCommStdChannels(const QStringList& openChannels, QWidget* parent=0);
	~AuCommStdChannels();

	QString ChannelName() const
	{
		return m_ChannelName;
	}

protected slots:
	void EnableControls();
	void OpenChannel();

private:
	QString			m_ChannelName;	// Stores final selection result.
};

#endif // AUCOMMSTDCHANNELS_H
