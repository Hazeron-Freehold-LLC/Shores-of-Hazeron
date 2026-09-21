//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auadf.h,v 1.6 2022/02/19 17:49:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuADF_H
#define AuADF_H

#include "auclientlib.h"
#include "audialog.h"

class QSpinBox;

class AUCLIENT_DLL AuADF : public AuDialog
{
	Q_OBJECT;

	int									m_Hundreds;
	int									m_Ones;
	int									m_Tens;

public:
	AuADF(const QString& title, QWidget* parent=0);
	virtual ~AuADF();

	quint16 AccessCode() const;

	virtual QSize sizeHint() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	virtual void paintEvent(QPaintEvent* event);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuADF_H
