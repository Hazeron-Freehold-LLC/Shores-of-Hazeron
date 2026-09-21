//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubank.h,v 1.7 2022/02/19 18:18:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBank_H
#define AuBank_H

#include <QDataStream>
#include <QHash>

class AuBank
{
public:
	enum CATEGORY
	{
		// Civilian Account
		Civ_CreditImmigration,			// immigration of citizens personal bank accounts.
		Civ_CreditLoans,				// loan payments.
		Civ_CreditMint,					// local money production.
		Civ_CreditRnD,					// RnD, due to canceling an RnD mfg process.
		Civ_CreditSalary,				// work performed by citizens.
		Civ_CreditSales,				// sales of local products.
		Civ_CreditWholesale,			// wholesale trade with other cities.
		Civ_DebitEmigration,			// emigration of citizens.
		Civ_DebitLoans,					// loans made to citizens.
		Civ_DebitPurchases,				// purchase of products.
		Civ_DebitRewards,				// rewards paid out.
		Civ_DebitRnD,					// research and development.
		Civ_DebitSalary,				// paying for work.
		Civ_DebitWholesale,				// wholesale trade with other cities.
		Civ_DebitWithdrawal,			// people withdrawing their money from the system.

		// Government Account
		Gov_CreditDeposit,				// deposit by govt official.
		Gov_CreditTaxIncome,			// income tax.
		Gov_CreditTaxSales,				// sales tax.
		Gov_CreditTaxSubsidy,			// subsidy tax.
		Gov_CreditTribute,				// tribute from subordinate cities, for capital cities.
		Gov_DebitRewards,				// rewards paid out.
		Gov_DebitTribute,				// tribute paid to capital.
		Gov_DebitWithdrawal,			// withdrawal by govt official.
		Gov_DebitWorks,					// municipal works projects.

		Civ_CreditBdDestroyed,			// transfer of funds from a destroyed building
		Civ_DebitMfg,					// Manufacturing metal from money.
		Civ_CreditMfg,					// Due to canceling a smelter mfg process.
		CATEGORY_Count					// LAST CATEGORY
	};

private:
	QHash<CATEGORY, double>				m_Activity;				// Activity that resulted in the current balance.
	double								m_Balance;				// Current balance.

public:	// Construction
	AuBank();
	AuBank(double balance);
	AuBank(const AuBank& c);

public:	// Operators
	const AuBank& operator=(const AuBank& c);

	const AuBank& operator+=(const AuBank& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void Activity(double amount, CATEGORY cat);

	inline double BalanceEnding() const
	{
		return m_Balance;
	}
	double BalanceStarting() const;

	static QString CategoryName(CATEGORY cat);

	void CivCreditBdDestroyed(double amount);
	void CivCreditImmigration(double amount);
	void CivCreditLoans(double amount);
	void CivCreditMint(double amount);
	void CivCreditMfg(double amount);
	void CivCreditRnD(double amount);
	void CivCreditSalary(double amount);
	void CivCreditSales(double amount);
	void CivCreditWholesale(double amount);
	void CivDebitEmigration(double amount);
	void CivDebitLoans(double amount);
	void CivDebitMfg(double amount);
	void CivDebitPurchases(double amount);
	void CivDebitRewards(double amount);
	void CivDebitRnD(double amount);
	void CivDebitSalary(double amount);
	void CivDebitWholesale(double amount);
	void CivDebitWithdrawal(double amount);

	inline void ClearActivity()
	{
		m_Activity.clear();
	}

	void GovCreditDeposit(double amount);
	void GovCreditTaxIncome(double amount);
	void GovCreditTaxSales(double amount);
	void GovCreditTaxSubsidy(double amount);
	void GovCreditTribute(double amount);
	void GovDebitRewards(double amount);
	void GovDebitTribute(double amount);
	void GovDebitWithdrawal(double amount);
	void GovDebitWorks(double amount);

	QString HtmlReport() const;

	static inline bool IsCredit(CATEGORY cat)
	{
		switch(cat)
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
			return true;
		case Civ_DebitEmigration:
		case Civ_DebitLoans:
		case Civ_DebitMfg:
		case Civ_DebitPurchases:
		case Civ_DebitRewards:
		case Civ_DebitRnD:
		case Civ_DebitSalary:
		case Civ_DebitWholesale:
		case Civ_DebitWithdrawal:
			break;
		case Gov_CreditDeposit:
		case Gov_CreditTaxIncome:
		case Gov_CreditTaxSales:
		case Gov_CreditTaxSubsidy:
		case Gov_CreditTribute:
			return true;
		case Gov_DebitRewards:
		case Gov_DebitTribute:
		case Gov_DebitWithdrawal:
		case Gov_DebitWorks:
			break;
		case CATEGORY_Count:
			break;
		}
		return false;
	}

	static inline bool IsDebit(CATEGORY cat)
	{
		switch(cat)
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
			return true;
		case Gov_CreditDeposit:
		case Gov_CreditTaxIncome:
		case Gov_CreditTaxSales:
		case Gov_CreditTaxSubsidy:
		case Gov_CreditTribute:
			break;
		case Gov_DebitRewards:
		case Gov_DebitTribute:
		case Gov_DebitWithdrawal:
		case Gov_DebitWorks:
			return true;
		case CATEGORY_Count:
			break;
		}
		return false;
	}

	void Rx(QDataStream& in);
	void Tx(QDataStream& out) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuBank& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBank& c)
{
	c << in;
	return in;
}

#endif // AuBank_H
