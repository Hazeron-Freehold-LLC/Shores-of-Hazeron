//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitwidget.h,v 1.2 2022/02/19 17:57:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Provides a QTableWidget derived widget for showing files in the design library.

#ifndef AuItWidget_H
#define AuItWidget_H

#include "auclientlib.h"
#include "auid.h"
#include <QTableWidget>

class AuBdDesignListEntry;

class AUCLIENT_DLL AuItWidget : public QTableWidget
{
	Q_OBJECT;
	AuId								m_ArchitectId;			// Local architect id.
	quint32								m_CurrentId;
	bool								m_IsPublishMode;		// true shows only designs that can be published.

public:
	AuItWidget(const AuId& architectId, bool publishMode, QWidget* parent=0);
	virtual ~AuItWidget();

	// Returns 0 if no design is selected.
	quint32 CurrentDesignId() const;

	// Returns empty string if no design is selected.
	QString CurrentDesignName() const;

	// Returns empty string if no design is selected.
	QString CurrentDesignReport() const;

	// Returns the AuDesign::STATE of the currently selected design.
	// Returns AuDesign::Deleted if no design is selected. We don't list deleted designs so there is no ambiguity.
	quint8 CurrentDesignState() const;

	// Fills 'designs' with the id and name of the selected designs.
	void SelectedDesigns(QHash<quint32, QString>& designs) const;

	// Fills 'designs' with the id and name of the selected designs that are in a 'designState'.
	void SelectedDesigns(QHash<quint32, QString>& designs, quint8 designState) const;

public slots:
	void RefreshContents();
	void SetCurrent(quint32 designId);

protected:
	void AddDesign(quint32 designId, const AuBdDesignListEntry& design);

	// Returns -1 if no selection.
	int CurrentRow() const;
};

#endif // AuItWidget_H
