//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auimagedialog.cpp,v 1.3 2022/02/19 18:16:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auimagedialog.h"
//
#include "auglobal.h"
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

AuImageDialog::AuImageDialog(const QString& title, const QImage& image, QWidget* parent) : AuDialog(parent)
{
	if(!title.isEmpty())
		setAuDlgTitle(title);

	// Convert the device-independent image to a device-dependent pixmap.
	QPixmap pm = QPixmap::fromImage(image);

	// Create the label to display the image.
	QLabel* lb = new QLabel(this);
	lb->setPixmap(pm);

	// Layout the dialog with only the pixmap label.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuImageDialog::AuImageDialog(const QString& title, const QString& caption, const QImage& image, QWidget* parent) : AuDialog(parent)
{
	if(!title.isEmpty())
		setAuDlgTitle(title);

	// Convert the device-independent image to a device-dependent pixmap.
	QPixmap pm = QPixmap::fromImage(image);

	// Create the label to display the image.
	QLabel* lb = new QLabel(this);
	lb->setPixmap(pm);

	// Create the label to display the caption.
	QLabel* lbcap = new QLabel(caption, this);
	lbcap->setWordWrap(true);

	// Layout the dialog with only the pixmap label.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->addWidget(lbcap);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}
