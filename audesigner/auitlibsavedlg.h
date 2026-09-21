//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitlibsavedlg.h,v 1.2 2022/02/19 18:22:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItLibSaveDlg_H
#define AuItLibSaveDlg_H

#include "audialog.h"

class AuId;
class AuItWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTextBrowser;

class AuItLibSaveDlg : public AuDialog
{
	Q_OBJECT;
private:
	QImage								m_Thumb;

	// Widgets
	QLineEdit*							m_DesignId;
	QLineEdit*							m_DesignName;
	QTextBrowser*						m_DesignReport;
	AuItWidget*							m_Designs;
	QLabel*								m_DesignThumb;
	QPushButton*						m_Replace;
	QPushButton*						m_RequestDesigns;
	QPushButton*						m_SaveNew;
	QSplitter*							m_Splitter;

public:
	AuItLibSaveDlg(const AuId& architectId, const QImage& thumb, QWidget* parent=0);
	virtual ~AuItLibSaveDlg();

	// Returns the currently selected design id, or 0 when saving to a new design.
	quint32 DesignId() const;

	// Returns the currently selected design name.
	QString DesignName() const;

	// Returns the currently selected design state.
	quint8 DesignState() const;

	// Sets the initial states of the selected design id and design name boxes.
	void SetDesign(quint32 id, const QString& name, const QString& report);

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DesignSelectionChanged();
	void EnableControls();
	void NameEdited(const QString& name);
	void RefreshDesigns();
	void RefreshThumb();
	void Replace();
	void RequestDesigns();
	void SaveNew();
	void SplitterMoved(int pos, int index);
};

#endif // AuItLibSaveDlg_H
