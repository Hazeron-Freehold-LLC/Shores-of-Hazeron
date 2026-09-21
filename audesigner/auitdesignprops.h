//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitdesignprops.h,v 1.2 2022/02/19 18:21:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItDesignProps_H
#define AuItDesignProps_H

#include "audialog.h"
#include "auitdesign.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTextBrowser;
class QTextEdit;

class AuItDesignProps : public AuDialog
{
	Q_OBJECT;
public:
	// Values
	AuItDesign							m_DesignRec;

private:
	// Widgets
	QTextBrowser*						m_Analysis;
	QTextEdit*							m_Description;
	QComboBox*							m_GroundOrient;
	QComboBox*							m_CommoditiesCombo;
	QLabel*								m_CommoditiesInfo;
	QLineEdit*							m_Name;
	QPushButton*						m_Ok;
	QSplitter*							m_Splitter;

public:
	AuItDesignProps(const AuItDesign& itRec, QWidget* parent=0);
	virtual ~AuItDesignProps();

public slots:
	virtual void setVisible(bool visible);

private slots:
	void CommodityChanged(int index);
	void DescriptionChanged();
	void EnableControls();
	void GroundOrientationChanged(int index);
	void NameChanged(const QString& text);
	void SplitterMoved(int pos, int index);
	void UpdateReport();

private:
	void ListCommodities();
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuItDesignProps_H
