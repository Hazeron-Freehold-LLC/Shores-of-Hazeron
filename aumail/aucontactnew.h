//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontactnew.h,v 1.6 2022/02/19 18:36:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuContactNew_H
#define AuContactNew_H

#include "audialog.h"
#include "auid.h"
#include "aumaillib.h"

class QLineEdit;

class AUMAIL_DLL AuContactNew : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_IdEdit;
	QLineEdit*							m_NameEdit;

public:
	AuContactNew(bool isBuddy, QWidget* parent=0);	// isBuddy=false -> blocked

	AuId ContactId() const;

	QString ContactName() const;

protected slots:
	void OkClicked();
};

#endif // AuContactNew_H
