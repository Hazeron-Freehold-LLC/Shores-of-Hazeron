//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovtaxes.cpp,v 1.5 2022/02/19 18:16:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovtaxes.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include <auglobal.h>
#include <daperson.h>
#include <dgdata.h>
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

void AuGovTaxesDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernTaxesUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovTaxes::AuGovTaxes(QWidget* parent) : QWidget(parent)
{
	m_DataValid = false;

	// Taxes
	QLabel* lbtax = new QLabel(tr("<html>"
				"Taxation provides the empire with operating capital. "
				"An empire must declare a capital to collect taxes.<br><br>"
				"Cities collect taxes. "
				"Taxes are stored in the Government Account of banks. "
				"Cities remit taxes to the capital once per day.<br><br>"
				"Fractional percentages for tax rates may be specified in .5 percent increments.</html>"));
	lbtax->setWordWrap(true);

	QLabel* lbincome = new QLabel(tr("Income Tax"));
	lbincome->setToolTip(tr("<html>Income tax is collected from all income transactions. "
				"This includes income from completing quests and crew shares of spacecraft income. "
				"Cities produce income tax revenue based on the population.</html>"));
	lbincome->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	leincome = new QLineEdit();
	leincome->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbincp = new QLabel("%");

	QLabel* lbsales = new QLabel(tr("Sales Tax"));
	lbsales->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	lbsales->setToolTip(tr("<html>Sales tax is collected from all trade transactions. "
				"This applies to sales of all items and cargo. "
				"Cities produce sales tax revenue based on the population.</html>"));
	lesales = new QLineEdit();
	lesales->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbsalesp = new QLabel("%");

	QLabel* lbsubsidy = new QLabel(tr("Fleet Tax"));
	lbsubsidy->setToolTip(tr("<html>Fleet tax is collected on sales of cargo and passenger fares using fleet "
				"spacecraft. It is not collected on trade activities using company spacecraft.</html>"));
	lbsubsidy->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	lesubsidy = new QLineEdit();
	lesubsidy->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbsubsidyp = new QLabel("%");

	QPushButton* pbapply = new QPushButton(tr("Apply Taxation Changes"));
	connect(pbapply, SIGNAL(clicked()), SLOT(ApplyChanges()));
	pbapply->setToolTip(tr("<html><b>Apply Taxation Changes</b><br>"
				"Changes the tax rates of the empire to match the values entered.<br>"
				"Only the emperor or vice-emperor can change tax rates."
				"</html>"));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtax, row,0, 1,3);
	row++;
	lay->addWidget(lbincome, row,0);
	lay->addWidget(leincome, row,1);
	lay->addWidget(lbincp, row,2);
	row++;
	lay->addWidget(lbsales, row,0);
	lay->addWidget(lesales, row,1);
	lay->addWidget(lbsalesp, row,2);
	row++;
	lay->addWidget(lbsubsidy, row,0);
	lay->addWidget(lesubsidy, row,1);
	lay->addWidget(lbsubsidyp, row,2);
	row++;
	lay->addWidget(pbapply, row,0, 1,2);
	row++;
	lay->setColumnStretch(2, 1);
	lay->setRowStretch(row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	connect(this, SIGNAL(TaxesChanged(double, double, double)), auApp, SLOT(GovSetTaxes(double, double, double)));
}

void AuGovTaxes::UpdateEmpireRecord()
{
	// If this widget is not visible, mark the data as invalid.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovTaxes::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_DataValid = false;	// Our trigger to update contents.
		return;
	}

	//qDebug("AuGovTaxes::UpdateEmpireRecord page is visible. Update contents.");
	// Get new empire record data
	DgEmpire emp;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		emp.m_id = person->EmpireId();
	pView->UnlockSceneAtom();
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(emp);
	g_Au.UnlockData();
	m_DataValid = true;

	leincome->setText(QVariant(emp.IncomeTax()).toString());

	lesales->setText(QVariant(emp.SalesTax()).toString());

	lesubsidy->setText(QVariant(emp.SubsidyTax()).toString());
}

void AuGovTaxes::ApplyChanges()
{
	double incomeTax = 0;
	{
		incomeTax = QVariant(leincome->text()).toDouble();
		if(incomeTax > 100)
		{
			incomeTax = 100;
			leincome->setText("100");
		}
		else
		{
			double tax = FROUND(incomeTax, .5);
			if(tax != incomeTax)
			{
				incomeTax = tax;
				leincome->setText(QVariant(incomeTax).toString());
			}
		}
	}

	double salesTax = 0;
	{
		salesTax = QVariant(lesales->text()).toDouble();
		if(salesTax > 100)
		{
			salesTax = 100;
			lesales->setText("100");
		}
		else
		{
			double tax = FROUND(salesTax, .5);
			if(tax != salesTax)
			{
				salesTax = tax;
				lesales->setText(QVariant(salesTax).toString());
			}
		}
	}

	double subsidyTax = 0;
	{
		subsidyTax = QVariant(lesubsidy->text()).toDouble();
		if(subsidyTax > 100)
		{
			subsidyTax = 100;
			lesubsidy->setText("100");
		}
		else
		{
			double tax = FROUND(subsidyTax, .5);
			if(tax != subsidyTax)
			{
				subsidyTax = tax;
				lesubsidy->setText(QVariant(subsidyTax).toString());
			}
		}
	}

	emit TaxesChanged(incomeTax, salesTax, subsidyTax);
}

void AuGovTaxes::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if(!m_DataValid)
		UpdateEmpireRecord();
}

