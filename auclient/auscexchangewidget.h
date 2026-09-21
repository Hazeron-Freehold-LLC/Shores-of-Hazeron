//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscexchangewidget.h,v 1.7 2022/02/19 18:00:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Provides a QTableWidget derived widget for showing files in the design library.

#ifndef AuScExchangeWidget_H
#define AuScExchangeWidget_H

#include "auclientlib.h"
#include <QStandardItemModel>
#include <QTableWidget>

class AuScDesignExchangeEntryClient;

class AUCLIENT_DLL AuScExchangeWidget : public QTableWidget
{
	Q_OBJECT;
	quint32								m_CurrentId;

public:
	AuScExchangeWidget(QWidget* parent=0);
	virtual ~AuScExchangeWidget();

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

public slots:
	void RefreshContents();
	void SetCurrent(quint32 designId);

protected:
	void AddDesign(quint32 designId, const AuScDesignExchangeEntryClient& design);

	// Returns -1 if no selection.
	int CurrentRow() const;
};

#endif // AuScExchangeWidget_H
