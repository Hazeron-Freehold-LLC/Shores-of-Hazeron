//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitlibdeletedlg.h,v 1.2 2022/02/19 18:22:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItLibDeleteDlg_H
#define AuItLibDeleteDlg_H

#include "audialog.h"

class AuItWidget;
class AuId;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTextBrowser;

class AuItLibDeleteDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QTextBrowser*						m_DesignReport;
	AuItWidget*							m_Designs;
	QLabel*								m_DesignThumb;
	QPushButton*						m_Ok;
	QPushButton*						m_RequestDesigns;
	QSplitter*							m_Splitter;

public:
	AuItLibDeleteDlg(const AuId& architectId, QWidget* parent=0);
	virtual ~AuItLibDeleteDlg();

	// Returns the currently selected design id, or 0 when saving to a new design.
	quint32 DesignId() const;

	// Returns the currently selected design name.
	QString DesignName() const;

	// Sets the initial states of the selected design id and design name boxes.
	void SetDesign(quint32 id, const QString& name, const QString& report);

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DesignSelectionChanged();
	void DeleteDesign();
	void RefreshThumb();
	void RequestDesigns();
	void SplitterMoved(int pos, int index);
};

#endif // AuItLibDeleteDlg_H
