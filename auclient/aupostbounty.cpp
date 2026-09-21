//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupostbounty.cpp,v 1.3 2022/02/19 18:16:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupostbounty.h"
//
#include <auglobal.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

static int s_MinBounty = 10000;

AuPostBounty::AuPostBounty(const QString& victimName, double account, int mode, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/JobBountyHunter.png"));

	m_Account = account;
	m_Reward = 10000;
	m_VictimName = victimName;

	QString title;
	QString reason;
	switch(mode)
	{
	case 0:
		title = tr("Post Bounty On Sender");
		reason = tr("<p>The sender of the selected message was %1.</p>"
					"<p>Enter your reason for posting this bounty on %1.</p>")
				.arg(victimName);
		break;
	case 1:
		title = tr("Post Bounty On Target");
		reason = tr("<p>The last target spotted was %1.</p>"
					"<p>Enter your reason for posting this bounty on %1.</p>")
				.arg(victimName);
		break;
	default:
		Q_ASSERT(false);
		break;
	}
	setAuDlgTitle(title);

	QLabel* lbreason = new QLabel(tr("<html><b>%1</b>"
								"<p>The money is taken from your personal account when you post the bounty. "
								"Multiple bounties may be posted on a single target.</p>"
								"<p>Bounties on people and spacecraft require destruction of the target to collect. "
								"Bounties on cities require capture of the town square to collect.</p>"
								"<p>Bounties are only paid to players actively pursuing a bounty hunter job, with the victim as "
								"its target. "
								"The first bounty hunter to complete the job collects the money. "
								"All bounties posted on the target are paid at once when a bounty hunter collects.</p>"
								"%2</html>")
							.arg(title).arg(reason), this);
	lbreason->setWordWrap(true);

	const int maxReasonLength = 100;
	m_ReasonEdit = new QLineEdit(this);
	m_ReasonEdit->setMaxLength(maxReasonLength);
	m_ReasonEdit->setToolTip(tr("<html>The reason must be no more than %1 characters "
				"and it must not contain non-printable characters or certain reserved characters such as &lt; &gt; &amp; and |."
				"</html>").arg(maxReasonLength));
	connect(m_ReasonEdit, SIGNAL(textChanged(const QString&)), SLOT(ReasonChanged(const QString&)));

	QString sfund = tr("Personal Account %L1%2").arg(m_Account, 0,'f',0).arg(g_Au.CurrencySymbol());
	QLabel* lbfunds = new QLabel(sfund, this);

	QLabel* lbamt = new QLabel(tr("<html>Bounty<br><small>Minimum %L1%2</small></html").arg(s_MinBounty).arg(g_Au.CurrencySymbol()),
			this);

	m_RewardEdit = new QLineEdit(QString("%1").arg(m_Reward), this);
	connect(m_RewardEdit, SIGNAL(textChanged(const QString&)), SLOT(RewardChanged(const QString&)));

	m_Ok = new QPushButton(tr("Post Bounty"));
	m_Ok->setEnabled(false);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbreason, row,0, 1,2);
	row++;
	lay->addWidget(m_ReasonEdit, row,0, 1,2);
	row++;
	lay->addWidget(lbfunds, row,0, 1,2);
	row++;
	lay->addWidget(lbamt, row,0);
	lay->addWidget(m_RewardEdit, row,1);
	row++;
	lay->addWidget(m_Ok, row,0, 1,2);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuPostBounty::~AuPostBounty()
{
}

void AuPostBounty::EnableControls()
{
	if(m_Reward >= s_MinBounty && m_Reward < m_Account &&
			!m_Reason.isEmpty() && IsTextValid(m_Reason, 100))
		m_Ok->setEnabled(true);
	else
		m_Ok->setEnabled(false);
}

void AuPostBounty::ReasonChanged(const QString& reason)
{
	m_Reason = reason.trimmed();
	EnableControls();
}

void AuPostBounty::RewardChanged(const QString& reward)
{
	m_Reward = QVariant(reward).toDouble();
	EnableControls();
}
