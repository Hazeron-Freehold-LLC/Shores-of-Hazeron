//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufindtext.h,v 1.3 2022/02/19 18:30:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUFINDTEXT_H
#define AUFINDTEXT_H

#include "augloballib.h"
#include <QDialog>
#include <QTextEdit>
#include "uiFindText.h"

class AUGLOBAL_DLL AuFindText : public QDialog, public Ui::FindText
{
	Q_OBJECT

public:
	AuFindText(QTextEdit* textDoc, QWidget *parent = 0);
	~AuFindText();

	QTextEdit*	m_textDocument;
public slots:
	void FindExpression();
};

#endif //AUFINDTEXT_H

