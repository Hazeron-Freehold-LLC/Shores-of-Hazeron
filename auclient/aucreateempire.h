//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucreateempire.h,v 1.6 2022/02/19 17:55:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCreateEmpire_H
#define AuCreateEmpire_H

#include "auclientlib.h"
#include "audialog.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;

class AUCLIENT_DLL AuCreateEmpire : public AuDialog
{
	Q_OBJECT;
	QImage								m_Flag;

	// Widgets
	QToolButton*						m_FlagButton;
	QLineEdit*							m_NameEdit;
	QLabel*								m_NameReason;
	QPushButton*						m_OkButton;

public:
	AuCreateEmpire(QWidget* parent=0);
	virtual ~AuCreateEmpire();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void ChooseFlag();
	void CreateEmpire();
	void EnableControls();
};

#endif // AuCreateEmpire_H
