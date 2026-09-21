//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausccopysymmetrical.h,v 1.2 2022/02/19 18:22:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScCopySymmetrical_H
#define AuScCopySymmetrical_H

#include "audialog.h"

class QRadioButton;
class QSpinBox;

class AuScCopySymmetrical : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QRadioButton*						m_XAxis;
	QRadioButton*						m_YAxis;
	QRadioButton*						m_ZAxis;

public:
	AuScCopySymmetrical(QWidget* parent=0);
	virtual ~AuScCopySymmetrical();

	// 0=x, 1=y, 2=z
	int Axis() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoAxis();
};

#endif // AuScCopySymmetrical_H
