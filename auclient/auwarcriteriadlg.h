//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auwarcriteriadlg.h,v 1.6 2022/02/19 18:03:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuWarCriteriaDlg_H
#define AuWarCriteriaDlg_H

#include "auclientlib.h"
#include "audialog.h"

class AuWarCriteria;
class QLabel;
class QSlider;

class AUCLIENT_DLL AuWarCriteriaDlg : public AuDialog
{
	Q_OBJECT;

	QHash<quint8, QLabel*>				m_SliderLabels;			// Slider buddy labels, keyed by doc type.
	QHash<quint8, QSlider*>				m_Sliders;				// Sliders, keyed by doc type.

public:
	AuWarCriteriaDlg(const AuWarCriteria& curCrit, QWidget* parent=0);
	virtual ~AuWarCriteriaDlg();

	AuWarCriteria Criteria() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void Clear();
	void Default();
	void SliderChanged(int value);
};

#endif // AuWarCriteriaDlg_H
