//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontactnew.cpp,v 1.10 2022/02/19 18:36:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucontactnew.h"
//
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuContactNew::AuContactNew(bool isBuddy, QWidget* parent) : AuDialog(parent)
{
	if(isBuddy)
	{
		setAuDlgIcon(QIcon(":data/ContactBuddies.png"));
		setAuDlgTitle("Add Contact");
	}
	else
	{
		setAuDlgIcon(QIcon(":data/ContactBlocked.png"));
		setAuDlgTitle("Add Block");
	}

	QLabel* lbname = new QLabel("Name");

	m_NameEdit = new QLineEdit();
	m_NameEdit->setToolTip(tr("<html><b>Name</b><br>"
				"Enter the name of the contact."
				"</html>"));

	QLabel* lbid = new QLabel("Id");

	m_IdEdit = new QLineEdit();
	m_IdEdit->setToolTip(tr("<html><b>Id</b><br>"
				"Enter the id of the contact, usually expressed as a series of capital letters. "
				"Entry into this control is not case sensitive; input will be converted to upper case."
				"</html>"));

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setDefault(true);
	pbok->setAutoDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(OkClicked()));

	QGridLayout* lay = new QGridLayout();
	int row=0;
	lay->addWidget(lbname, row,0);
	lay->addWidget(m_NameEdit, row,1, 1,2);
	row++;
	lay->addWidget(lbid, row,0);
	lay->addWidget(m_IdEdit, row,1, 1,2);
	row++;
	lay->addWidget(pbok, row,2);
	lay->setColumnStretch(1, 1);
	lay->setSpacing(2);
	lay->setMargin(4);
	setAuDlgLayout(lay);
}

void AuContactNew::OkClicked()
{
	//qDebug("AuContactNew::OkClicked");
	AuId id = ContactId();
	QString name = ContactName();

	if(!id.IsNull() && IsNameValid(name))
		accept();
}

AuId AuContactNew::ContactId() const
{
	AuId id(m_IdEdit->text().trimmed());
	return id;
}

QString AuContactNew::ContactName() const
{
	return m_NameEdit->text().trimmed();
}
