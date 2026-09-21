//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdgearsave.h,v 1.2 2022/02/19 18:21:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdGearSave_H
#define AuBdGearSave_H

#include "audialog.h"
#include "augear.h"

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

class AuBdGearSave : public AuDialog
{
	Q_OBJECT;
private:
	AuGearPos							m_Media;				// Media selected in gear, if accepted.
	QString								m_MediaName;

	// Widgets
	QRadioButton*						m_CanCopy;
	QRadioButton*						m_CanMfg;
	QRadioButton*						m_CanView;
	QLineEdit*							m_DesignId;
	QLineEdit*							m_DesignName;
	QTextBrowser*						m_DesignReport;
	AuBdWidget*							m_Designs;
	QLabel*								m_DesignThumb;
	QComboBox*							m_GearMedia;
	QComboBox*							m_MinStance;
	QPushButton*						m_Ok;
	QPushButton*						m_RequestDesigns;
	QSplitter*							m_Splitter;
	QCheckBox*							m_UseOnce;

public:
	AuBdGearSave(const AuId& architectId, QWidget* parent=0);
	virtual ~AuBdGearSave();

	bool CanCopy() const;

	bool CanView() const;

	// Returns the currently selected design id, or 0 when saving to a new design.
	quint32 DesignId() const;

	// Returns the currently selected design name.
	QString DesignName() const;

	const AuGearPos& MediaPos() const
	{
		return m_Media;
	}

	POLITICAL_STANCE MinStance() const;

	quint32 Price() const;

	// Sets the initial states of the selected design id and design name boxes.
	void SetDesign(quint32 id, const QString& name, const QString& report);

	bool UseOnce() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListMedia();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DesignSelectionChanged();
	void EnableControls();
	void GearMediaChanged(int index);
	void RefreshThumb();
	void RequestDesigns();
	void SaveDesign();
	void SplitterMoved(int pos, int index);
};

#endif // AuBdGearSave_H
