//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitlibopendlg.h,v 1.2 2022/02/19 18:22:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItLibOpenDlg_H
#define AuItLibOpenDlg_H

#include "audialog.h"

class AuId;
class AuItWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTextBrowser;

class AuItLibOpenDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_DesignId;
	QLineEdit*							m_DesignName;
	QTextBrowser*						m_DesignReport;
	AuItWidget*							m_Designs;
	QLabel*								m_DesignThumb;
	QPushButton*						m_Ok;
	QPushButton*						m_RequestDesigns;
	QSplitter*							m_Splitter;

public:
	AuItLibOpenDlg(const AuId& architectId, QWidget* parent=0);
	virtual ~AuItLibOpenDlg();

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
	void OpenDesign();
	void RefreshThumb();
	void RequestDesigns();
	void SplitterMoved(int pos, int index);
};

#endif // AuItLibOpenDlg_H
