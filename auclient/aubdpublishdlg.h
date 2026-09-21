//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpublishdlg.h,v 1.7 2022/02/19 17:50:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdPublishDlg_H
#define AuBdPublishDlg_H

#include "auclientlib.h"
#include "audialog.h"
#include "audef.h"

class AuBdWidget;
class AuId;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSplitter;
class QTextBrowser;

class AUCLIENT_DLL AuBdPublishDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QRadioButton*						m_CanCopy;
	QRadioButton*						m_CanMfg;
	QRadioButton*						m_CanView;
	QTextBrowser*						m_DesignReport;
	AuBdWidget*							m_Designs;
	QLabel*								m_DesignThumb;
	QComboBox*							m_MinStance;
	QPushButton*						m_Ok;
	QLineEdit*							m_Price;
	QPushButton*						m_RequestDesigns;
	QSplitter*							m_Splitter;
	QCheckBox*							m_UseOnce;

public:
	AuBdPublishDlg(const AuId& architectId, QWidget* parent=0);
	virtual ~AuBdPublishDlg();

	bool CanCopy() const;

	bool CanView() const;

	void DesignIds(QList<quint32>& designs) const;

	POLITICAL_STANCE MinStance() const;

	quint32 Price() const;

	// Sets the initial states of the selected design id and design name boxes.
	void SetDesign(quint32 id, const QString& name, const QString& report);

	bool UseOnce() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DesignSelectionChanged();
	void EnableControls();
	void PriceChanged(const QString& text);
	void PublishDesign();
	void RefreshThumb();
	void RequestDesigns();
	void SplitterMoved(int pos, int index);
};

#endif // AuBdPublishDlg_H
