//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufindtext.cpp,v 1.1 2022/02/19 18:33:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aufindtext.h"
//

AuFindText::AuFindText(QTextEdit* textDoc, QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	m_textDocument = textDoc;

	connect(pbFindNext, SIGNAL(clicked()), this, SLOT(FindExpression()));
	connect(pbClose, SIGNAL(clicked()), this, SLOT(reject()));
}

AuFindText::~AuFindText()
{

}

void AuFindText::FindExpression()
{
	if(m_textDocument)
	{
		QTextCursor findResult = m_textDocument->document()->find(leFind->text(), m_textDocument->textCursor());
		m_textDocument->setTextCursor(findResult);
	}
}

