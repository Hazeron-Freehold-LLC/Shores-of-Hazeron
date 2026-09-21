//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurateblueprint.h,v 1.2 2022/02/19 17:59:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRateBlueprint_H
#define AuRateBlueprint_H

#include "audialog.h"

class QPushButton;
class QTextBrowser;
class QTextEdit;
class QToolButton;

class AuRateBlueprint : public AuDialog
{
	Q_OBJECT;
	int									m_Rating;

	// Widgets
	QTextBrowser*						m_DesignReport;
	QTextEdit*							m_Comment;
	QPushButton*						m_Ok;
	QToolButton*						m_Rate1;
	QToolButton*						m_Rate2;
	QToolButton*						m_Rate3;
	QToolButton*						m_Rate4;
	QToolButton*						m_Rate5;

public:
	AuRateBlueprint(quint32 designId, const QString& designName, const QString& designReport, QWidget* parent=0);
	virtual ~AuRateBlueprint();

	QString Comment() const;

	inline int Rating() const
	{
		return m_Rating;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	void ShowRating();

protected slots:
	void Rate1Clicked();
	void Rate2Clicked();
	void Rate3Clicked();
	void Rate4Clicked();
	void Rate5Clicked();
};

#endif // AuRateBlueprint_H
