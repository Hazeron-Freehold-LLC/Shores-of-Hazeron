//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchristendialog.h,v 1.8 2022/02/19 17:53:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCHRISTENDIALOG_H
#define AUCHRISTENDIALOG_H

#include "auclientlib.h"
#include <audialog.h>

class AuId;

class AUCLIENT_DLL AuChristenDialog : public AuDialog
{
	Q_OBJECT;
public:
	AuChristenDialog(const AuId& shipId, QWidget* parent=0);
	~AuChristenDialog();

	const QString& Name() const
	{
		return m_Name;
	}

protected slots:
	void Christen();
	void NameChanged(const QString& name);
	void RandomName();

private:
	QString		m_Name;
};

#endif // AUCHRISTENDIALOG_H
