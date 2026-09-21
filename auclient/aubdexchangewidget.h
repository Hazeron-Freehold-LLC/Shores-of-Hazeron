//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdexchangewidget.h,v 1.7 2022/02/19 17:50:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Provides a QTableWidget derived widget for showing files in the design library.

#ifndef AuBdExchangeWidget_H
#define AuBdExchangeWidget_H

#include "auclientlib.h"
#include <QStandardItemModel>
#include <QTableWidget>

class AuBdDesignExchangeEntryClient;

class AUCLIENT_DLL AuBdExchangeWidget : public QTableWidget
{
	Q_OBJECT;
	quint32								m_CurrentId;

public:
	AuBdExchangeWidget(QWidget* parent=0);
	virtual ~AuBdExchangeWidget();

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
	void AddDesign(quint32 designId, const AuBdDesignExchangeEntryClient& design);

	// Returns -1 if no selection.
	int CurrentRow() const;
};

#endif // AuBdExchangeWidget_H
