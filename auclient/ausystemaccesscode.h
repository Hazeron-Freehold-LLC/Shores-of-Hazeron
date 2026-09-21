//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemaccesscode.h,v 1.6 2022/02/19 18:02:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSystemAccessCode_H
#define AuSystemAccessCode_H

#include "auclientlib.h"
#include "audialog.h"

class QSpinBox;

class AUCLIENT_DLL AuSystemAccessCode : public AuDialog
{
	Q_OBJECT;

	// Widgets
	QSpinBox*							m_DigitHundreds;
	QSpinBox*							m_DigitOnes;
	QSpinBox*							m_DigitTens;

public:
	AuSystemAccessCode(const QString& title, QWidget* parent=0);
	virtual ~AuSystemAccessCode();

	quint16 AccessCode() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuSystemAccessCode_H
