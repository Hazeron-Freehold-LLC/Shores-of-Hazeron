//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommopenchannel.h,v 1.6 2022/02/19 17:54:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCOMMOPENCHANNEL_H
#define AUCOMMOPENCHANNEL_H

#include "auclientlib.h"
#include <audialog.h>
#include <QIcon>

class AUCLIENT_DLL AuCommOpenChannel : public AuDialog
{
	Q_OBJECT;
public:
	AuCommOpenChannel(const QString& defChannelName, const QStringList& reservedNames,
			QWidget* parent=0);
	~AuCommOpenChannel();

	QString ChannelName() const
	{
		return m_ChannelName;
	}

	QString NarratorChannelDesc() const
	{
		return "";	//DHH no narrator channels
	}

protected slots:
	void EnableControls();
	void OpenChannel();

private:
	QString			m_ChannelName;		// Stores final selection result.
	QStringList		m_ReservedNames;	// Names that are std channels or already open.
};

#endif // AUCOMMOPENCHANNEL_H
