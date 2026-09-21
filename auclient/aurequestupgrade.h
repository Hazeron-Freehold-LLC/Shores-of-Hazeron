//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurequestupgrade.h,v 1.7 2022/02/19 18:00:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRequestUpgrade_H
#define AuRequestUpgrade_H

#include "auclientlib.h"
#include "dbcommodity.h"
#include <QWidget>

class AuServiceDialog;
class QListWidgetItem;

class AUCLIENT_DLL AuRequestUpgrade : public QWidget
{
	Q_OBJECT;
public:
	AuRequestUpgrade(AuServiceDialog* parent=0);

	inline const QString& SystemDesc() const
	{
		return m_SystemDesc;
	}

	inline SYSTEM SystemId() const
	{
		return m_System;
	}

	inline quint16 SystemWhich() const
	{
		return m_SystemWhich;
	}

protected slots:
	void ItemDoubleClicked(QListWidgetItem* item);
	void OkPressed();
	void RequestUpgrade();
	void SelectionChanged();
	void UpdateContents();

private:
	SYSTEM						m_System;
	QString						m_SystemDesc;
	quint16						m_SystemWhich;
};

#endif // AuRequestUpgrade_H
