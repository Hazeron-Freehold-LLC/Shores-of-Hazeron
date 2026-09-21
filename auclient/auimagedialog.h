//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auimagedialog.h,v 1.7 2022/02/19 17:57:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuImageDialog_H
#define AuImageDialog_H

#include "auclientlib.h"
#include "audialog.h"

class QImage;

class AUCLIENT_DLL AuImageDialog : public AuDialog
{
	Q_OBJECT;
public:
	AuImageDialog(const QString& title, const QImage& image, QWidget* parent=0);
	AuImageDialog(const QString& title, const QString& caption, const QImage& image, QWidget* parent=0);
};

#endif // AuImageDialog_H
