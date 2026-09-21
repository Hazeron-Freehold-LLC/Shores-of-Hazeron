//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupiratebuilding.h,v 1.3 2022/02/19 17:59:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPirateBuilding_H
#define AuPirateBuilding_H

#include "audialog.h"
#include "dbindustry.h"

class QComboBox;
class QLineEdit;

class AuPirateBuilding : public AuDialog
{
	Q_OBJECT;
	quint32								m_EmpireId;

	// Widgets
	QLineEdit*							m_Blueprint;
	QComboBox*							m_Industries;

public:
	AuPirateBuilding(quint32 empireId, QWidget* parent=0);
	virtual ~AuPirateBuilding();

	quint32 Blueprint() const;

	INDUSTRYID Industry() const;

public slots:
	virtual void setVisible(bool visible);
	void IndustryChanged(const QString& iname);

protected:
	void RestoreGeometry();
	void SaveGeometry();

signals:
	void BlueprintChanged(quint32 bp);
};

#endif // AuPirateBuilding_H
