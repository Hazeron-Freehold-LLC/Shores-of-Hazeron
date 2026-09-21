//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchristendialog.cpp,v 1.5 2022/02/19 18:10:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auchristendialog.h"
//
#include "auglobal.h"
#include "dgdata.h"
#include <QDesktopServices>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QUrl>

AuChristenDialog::AuChristenDialog(const AuId& shipId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/MsgChristen.png"));
	setAuDlgTitle("Christen Spacecraft");

	QLabel* lbl = new QLabel(tr("<html>Let it be recorded, that on this day, and forever more, this fine vessel is named"
				"</html>"), this);
	lbl->setWordWrap(true);

	QString idtext = QString("%1 %2 ").arg(shipId.StringId()).arg(g_Au.MiddleDotSymbol());
	QLabel* lbtail = new QLabel(idtext, this);

	QLabel* lbl2 = new QLabel(".", this);

	QLabel* lblicon = new QLabel(this);
	lblicon->setPixmap(QPixmap(":data/MsgChristen.png").scaled(20,20));

	QLineEdit* le = new QLineEdit(this);
	le->setObjectName("name");
	le->setMaxLength(MaxNameLength());
	le->setFocus();
	le->setToolTip(tr("<html>Enter a name for the ship. "
				"A name must be no more than %1 characters and "
				"it must not contain non-printable characters or certain reserved characters such as &lt; &gt; &amp; and |."
				"</html>").arg(MaxNameLength()));
	connect(le, SIGNAL(textChanged(const QString&)),
			SLOT(NameChanged(const QString&)));

	QPushButton* pbrandom = new QPushButton(this);
	pbrandom->setText(tr("Suggest a Name"));
	pbrandom->setToolTip(tr("Suggests a name for the ship."));
	connect(pbrandom, SIGNAL(clicked(bool)), SLOT(RandomName()));

	QPushButton* pbok = new QPushButton(tr("I christen this spacecraft. May God bless her\n"
				"and all who sail in her."));
	pbok->setEnabled(false);
	pbok->setDefault(true);
	pbok->setObjectName("ok");
	connect(pbok, SIGNAL(clicked()), SLOT(Christen()));

	QHBoxLayout* nlay = new QHBoxLayout;
	nlay->addWidget(lblicon);
	nlay->addWidget(lbtail);
	nlay->addWidget(le);
	nlay->addWidget(lbl2);
	nlay->addWidget(pbrandom);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbl);
	lay->addLayout(nlay);
	lay->addWidget(pbok);
	lay->addStretch();
	setAuDlgLayout(lay);
}

AuChristenDialog::~AuChristenDialog()
{
}

void AuChristenDialog::Christen()
{
	QLineEdit* le = findChild<QLineEdit*>("name");
	if(!le)
		reject();
	else
	{
		m_Name = le->text().trimmed();
		accept();
	}
}

void AuChristenDialog::RandomName()
{
	QLineEdit* name = findChild<QLineEdit*>("name");
	if(!name)
		return;

	// Generate random names until we get one that's ok.
	QString randomname = g_Au.Dice().RandomShipName();
	name->setText(randomname);
}

void AuChristenDialog::NameChanged(const QString& name)
{
	QPushButton* pbok = findChild<QPushButton*>("ok");
	if(!pbok)
		return;
	if(name.isEmpty())
	{
		pbok->setText(tr("I christen this spacecraft. May God bless her\n"
					"and all who sail in her."));
		pbok->setEnabled(false);
		pbok->setToolTip(tr("<html>Please enter a name for the ship.</html>"));
	}
	else
	{
		if(name.length() > MaxNameLength()/2)
			pbok->setText(tr("I christen this spacecraft %1.\n"
						"May God bless her and all who sail in her.").arg(name));
		else
			pbok->setText(tr("I christen this spacecraft %1. May God bless her\n"
						"and all who sail in her.").arg(name));
		pbok->setEnabled(IsNameValid(name.trimmed()));
	}
}
