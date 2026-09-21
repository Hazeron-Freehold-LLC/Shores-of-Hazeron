//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubank.cpp,v 1.8 2022/02/19 18:19:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubank.h"
//
#include "auglobal.h"
#include <QDebug>

AuBank::AuBank()
{
	m_Balance = 0;
}

AuBank::AuBank(double balance)
{
	m_Balance = balance;
}

AuBank::AuBank(const AuBank& c) : m_Activity(c.m_Activity)
{
	m_Balance = c.m_Balance;
}

const AuBank& AuBank::operator=(const AuBank& c)
{
	m_Activity = c.m_Activity;
	m_Balance = c.m_Balance;
	return *this;
}

const AuBank& AuBank::operator+=(const AuBank& c)
{
	m_Balance += c.m_Balance;
	// Add the total amounts by category.
	for(QHash<CATEGORY, double>::const_iterator ci = c.m_Activity.cbegin(); ci != c.m_Activity.cend(); ci++)
	{
		QHash<CATEGORY, double>::iterator ai = m_Activity.find(ci.key());
		if(ai != m_Activity.end())
			*ai += *ci;
		else
			m_Activity.insert(ci.key(), *ci);
	}
	return *this;
}

void AuBank::operator>>(QDataStream& out) const
{
	out << m_Balance;
	out << (quint8)m_Activity.size();
	for(QHash<CATEGORY, double>::const_iterator ai = m_Activity.cbegin(); ai != m_Activity.cend(); ai++)
	{
		out << (quint8)ai.key();
		out << *ai;
	}
}

void AuBank::operator<<(QDataStream& in)
{
	in >> m_Balance;
	quint8 c;
	in >> c;
	m_Activity.clear();
	while(c--)
	{
		quint8 cat;
		double amount;
		in >> cat;
		in >> amount;
		m_Activity.insert((CATEGORY)cat, amount);
	}
}

void AuBank::Tx(QDataStream& out) const
{
	out << m_Balance;
	// We don't need to send activity to the client.
}

void AuBank::Rx(QDataStream& in)
{
	in >> m_Balance;
	// Client does not receive bank activity.
}

double AuBank::BalanceStarting() const
{
	double transactions = 0;
	for(QHash<CATEGORY, double>::const_iterator ai = m_Activity.cbegin(); ai != m_Activity.cend(); ai++)
	{
		switch(ai.key())
		{
		case Civ_CreditBdDestroyed:
		case Civ_CreditImmigration:
		case Civ_CreditLoans:
		case Civ_CreditMfg:
		case Civ_CreditMint:
		case Civ_CreditRnD:
		case Civ_CreditSalary:
		case Civ_CreditSales:
		case Civ_CreditWholesale:
		case Gov_CreditDeposit:
		case Gov_CreditTaxIncome:
		case Gov_CreditTaxSales:
		case Gov_CreditTaxSubsidy:
		case Gov_CreditTribute:
			transactions += *ai;
			break;
		case Civ_DebitEmigration:
		case Civ_DebitLoans:
		case Civ_DebitMfg:
		case Civ_DebitPurchases:
		case Civ_DebitRewards:
		case Civ_DebitRnD:
		case Civ_DebitSalary:
		case Civ_DebitWholesale:
		case Civ_DebitWithdrawal:
		case Gov_DebitRewards:
		case Gov_DebitTribute:
		case Gov_DebitWithdrawal:
		case Gov_DebitWorks:
			transactions -= *ai;
			break;
		case CATEGORY_Count:
			break;
		}
	}
	return m_Balance - transactions;
}

void AuBank::Activity(double amount, CATEGORY cat)
{
	if(cat < 0 || cat >= CATEGORY_Count)
	{
		qWarning() << "warning: AuBank::Activity category" << cat << "amount" << amount << "category is bad";
		return;
	}
	if(amount < 0)
	{
		qWarning() << "warning: AuBank::Activity category" << cat << "amount" << amount << "amount is bad";
		return;
	}
	else if(amount == 0)
		return;

	// Accumulate the total amount by category.
	QHash<CATEGORY, double>::iterator ai = m_Activity.find(cat);
	if(ai != m_Activity.end())
		*ai += amount;
	else
		m_Activity.insert(cat, amount);

	// Change the current balance.
	if(IsCredit(cat))
		m_Balance += amount;
	else
		m_Balance -= amount;
}

void AuBank::CivCreditBdDestroyed(double amount)
{
	Activity(amount, Civ_CreditBdDestroyed);
}

void AuBank::CivCreditImmigration(double amount)
{
	Activity(amount, Civ_CreditImmigration);
}

void AuBank::CivCreditLoans(double amount)
{
	Activity(amount, Civ_CreditLoans);
}

void AuBank::CivCreditMint(double amount)
{
	Activity(amount, Civ_CreditMint);
}

void AuBank::CivCreditMfg(double amount)
{
	Activity(amount, Civ_CreditMfg);
}

void AuBank::CivCreditRnD(double amount)
{
	Activity(amount, Civ_CreditRnD);
}

void AuBank::CivCreditSalary(double amount)
{
	Activity(amount, Civ_CreditSalary);
}

void AuBank::CivCreditSales(double amount)
{
	Activity(amount, Civ_CreditSales);
}

void AuBank::CivCreditWholesale(double amount)
{
	Activity(amount, Civ_CreditWholesale);
}

void AuBank::CivDebitEmigration(double amount)
{
	Activity(amount, Civ_DebitEmigration);
}

void AuBank::CivDebitLoans(double amount)
{
	Activity(amount, Civ_DebitLoans);
}

void AuBank::CivDebitPurchases(double amount)
{
	Activity(amount, Civ_DebitPurchases);
}

void AuBank::CivDebitRewards(double amount)
{
	Activity(amount, Civ_DebitRewards);
}

void AuBank::CivDebitMfg(double amount)
{
	Activity(amount, Civ_DebitMfg);
}

void AuBank::CivDebitRnD(double amount)
{
	Activity(amount, Civ_DebitRnD);
}

void AuBank::CivDebitSalary(double amount)
{
	Activity(amount, Civ_DebitSalary);
}

void AuBank::CivDebitWholesale(double amount)
{
	Activity(amount, Civ_DebitWholesale);
}

void AuBank::CivDebitWithdrawal(double amount)
{
	Activity(amount, Civ_DebitWithdrawal);
}

void AuBank::GovCreditDeposit(double amount)
{
	Activity(amount, Gov_CreditDeposit);
}

void AuBank::GovCreditTaxIncome(double amount)
{
	Activity(amount, Gov_CreditTaxIncome);
}

void AuBank::GovCreditTaxSales(double amount)
{
	Activity(amount, Gov_CreditTaxSales);
}

void AuBank::GovCreditTaxSubsidy(double amount)
{
	Activity(amount, Gov_CreditTaxSubsidy);
}

void AuBank::GovCreditTribute(double amount)
{
	Activity(amount, Gov_CreditTribute);
}

void AuBank::GovDebitRewards(double amount)
{
	Activity(amount, Gov_DebitRewards);
}

void AuBank::GovDebitTribute(double amount)
{
	Activity(amount, Gov_DebitTribute);
}

void AuBank::GovDebitWithdrawal(double amount)
{
	Activity(amount, Gov_DebitWithdrawal);
}

void AuBank::GovDebitWorks(double amount)
{
	Activity(amount, Gov_DebitWorks);
}

QString AuBank::HtmlReport() const
{
	QString rpt = "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(56, 89, 56);\">"
		"<tbody>";

	// Starting balance
	rpt += QObject::tr("<tr>"
			"<td style=\"vertical-align: top;\"><b>Starting Balance</b>"
			"</td>"
			"<td style=\"vertical-align: top;\">"
			"</td>"
			"<td style=\"vertical-align: top;\">"
			"</td>"
			"<td style=\"vertical-align: top; text-align: right;\">%L1%2"
			"</td>"
		"</tr>").arg(BalanceStarting(), 0, 'f', 0).arg(AuGlobal::CurrencySymbol());

	// Activity
	// Income
	QMultiMap<QString, double> sortedIncome;
	for(QHash<CATEGORY, double>::const_iterator ai = m_Activity.cbegin(); ai != m_Activity.cend(); ai++)
	{
		if(!IsCredit(ai.key()))
			continue;
		sortedIncome.insert(CategoryName(ai.key()), *ai);
	}
	if(!sortedIncome.empty())
	{
		rpt += QObject::tr("<tr>"
				"<td style=\"vertical-align: top;\"><b>Income</b>"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
			"</tr>");
		double subtotal = 0;
		for(QMultiMap<QString, double>::const_iterator si = sortedIncome.cbegin(); si != sortedIncome.cend(); si++)
		{
			subtotal += *si;
			rpt += QString("<tr>"
					"<td style=\"vertical-align: top;\">"
					"</td>"
					"<td style=\"vertical-align: top; text-align: right;\">%1"
					"</td>"
					"<td style=\"vertical-align: top; text-align: right;\">%L2%3"
					"</td>"
					"<td style=\"vertical-align: top;\">"
					"</td>"
				"</tr>")
				.arg(si.key())
				.arg(*si, 0,'f',0)
				.arg(AuGlobal::CurrencySymbol());
		}
		rpt += QObject::tr("<tr>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top; text-align: right;\">Sub Total"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top; text-align: right;\">%L1%2"
				"</td>"
			"</tr>")
			.arg(subtotal, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol());
	}
	else
	{
		rpt += QObject::tr("<tr>"
				"<td style=\"vertical-align: top;\"><b>Income</b>"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top; text-align: right;\">0%2"
				"</td>"
			"</tr>").arg(AuGlobal::CurrencySymbol());
	}

	// Expenses
	QMultiMap<QString, double> sortedExpense;
	for(QHash<CATEGORY, double>::const_iterator ai = m_Activity.cbegin(); ai != m_Activity.cend(); ai++)
	{
		if(!IsDebit(ai.key()))
			continue;
		sortedExpense.insert(CategoryName(ai.key()), *ai);
	}
	if(!sortedExpense.empty())
	{
		rpt += QObject::tr("<tr>"
				"<td style=\"vertical-align: top;\"><b>Expense</b>"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
			"</tr>");
		double subtotal = 0;
		for(QMultiMap<QString, double>::const_iterator si = sortedExpense.cbegin(); si != sortedExpense.cend(); si++)
		{
			subtotal += *si;
			rpt += QString("<tr>"
					"<td style=\"vertical-align: top;\">"
					"</td>"
					"<td style=\"vertical-align: top; text-align: right;\">%1"
					"</td>"
					"<td style=\"vertical-align: top; text-align: right;\">%L2%3"
					"</td>"
					"<td style=\"vertical-align: top;\">"
					"</td>"
				"</tr>")
				.arg(si.key())
				.arg(*si, 0,'f',0)
				.arg(AuGlobal::CurrencySymbol());
		}
		rpt += QObject::tr("<tr>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top; text-align: right;\">Sub Total"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top; text-align: right;\">%L1%2"
				"</td>"
			"</tr>")
			.arg(-subtotal, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol());
	}
	else
	{
		rpt += QObject::tr("<tr>"
				"<td style=\"vertical-align: top;\"><b>Expense</b>"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top;\">"
				"</td>"
				"<td style=\"vertical-align: top; text-align: right;\">0%2"
				"</td>"
			"</tr>").arg(AuGlobal::CurrencySymbol());
	}

	// Ending balance
	rpt += QObject::tr("<tr>"
			"<td style=\"vertical-align: top;\"><b>Ending Balance</b>"
			"</td>"
			"<td style=\"vertical-align: top;\">"
			"</td>"
			"<td style=\"vertical-align: top;\">"
			"</td>"
			"<td style=\"vertical-align: top; text-align: right;\">%L1%2"
			"</td>"
		"</tr>").arg(BalanceEnding(), 0,'f',0).arg(AuGlobal::CurrencySymbol());

	rpt += "</tbody>"
		"</table>";
	return rpt;
}

//static
QString AuBank::CategoryName(CATEGORY cat)
{
	if(cat < 0 || cat >= CATEGORY_Count)
		return QString("Bad Bank Category %1").arg(cat);

	QString catNames[CATEGORY_Count] =
	{
		QObject::tr("Citizen Relocation"),	// Civ_CreditImmigration
		QObject::tr("Citizen Loan Payments"),	// Civ_CreditLoans
		QObject::tr("Minted Bullion"),	// Civ_CreditMint
		QObject::tr("Research and Development Canceled"),	// Civ_CreditRnD
		QObject::tr("Payroll"),	// Civ_CreditSalary
		QObject::tr("Product Sales"),	// Civ_CreditSales
		QObject::tr("Wholesale Sales"),	// Civ_CreditWholesale
		QObject::tr("Citizen Relocation"),	// Civ_DebitEmigration
		QObject::tr("Citizen Loans"),	// Civ_DebitLoans
		QObject::tr("Product Purchases"),	// Civ_DebitPurchases
		QObject::tr("Rewards"),	// Civ_DebitRewards
		QObject::tr("Research and Development"),	// Civ_DebitRnD
		QObject::tr("Payroll"),	// Civ_DebitSalary
		QObject::tr("Wholesale Purchases"),	// Civ_DebitWholesale
		QObject::tr("Withdrawals"),	// Civ_DebitWithdrawal
		QObject::tr("Deposit by Govt Official"),	// Gov_CreditDeposit
		QObject::tr("Income Tax"),	// Gov_CreditTaxIncome
		QObject::tr("Sales Tax"),	// Gov_CreditTaxSales
		QObject::tr("Subsidy Tax"),	// Gov_CreditTaxSubsidy
		QObject::tr("Tribute Received"),	// Gov_CreditTribute
		QObject::tr("Rewards"),	// Gov_DebitRewards
		QObject::tr("Tribute Paid"),	// Gov_DebitTribute
		QObject::tr("Withdrawal by Govt Official"),	// Gov_DebitWithdrawal
		QObject::tr("Public Works"),	// Gov_DebitWorks
		QObject::tr("Transfer from Destroyed Building"),	// Civ_CreditBdDestroyed
		QObject::tr("Smelted Metal"),	// Civ_DebitMfg
		QObject::tr("Smelter Canceled"),	// Civ_CreditMfg
	};
	return catNames[cat];
}

