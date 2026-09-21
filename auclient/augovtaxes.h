//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovtaxes.h,v 1.8 2022/02/19 17:57:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovTaxes_H
#define AuGovTaxes_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QWidget>

class DaPerson;
class QLineEdit;

class AUCLIENT_DLL AuGovTaxes : public QWidget
{
	Q_OBJECT;
private:
	bool								m_DataValid;			// true if data in controls is valid

	// Widgets
	QLineEdit*							leincome;
	QLineEdit*							lesales;
	QLineEdit*							lesubsidy;

public:
	AuGovTaxes(QWidget* parent=0);
	void UpdateEmpireRecord();

protected slots:
	void ApplyChanges();

signals:
	void TaxesChanged(double income, double sales, double subsidy);

protected:
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovTaxesDock : public AuDockWidget
{
public:
	AuGovTaxes*					m_Content;

public:
	AuGovTaxesDock(QWidget* parent) : AuDockWidget(":data/GovTaxes.png", tr("<b>Taxes</b>"), tr("Govern Taxes"), parent)
	{
		m_Content = new AuGovTaxes(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernTaxesAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernTaxesAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovTaxes_H
