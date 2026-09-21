//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auselcontiguousdlg.cpp,v 1.3 2022/02/19 18:26:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auselcontiguousdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuSelContiguousDlg::AuSelContiguousDlg(float angleTol, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Select Contiguous Faces"));
	setAuDlgIcon(QIcon(":data/DesignerSelFaces.png"));

	QLabel* lbInfo = new QLabel(tr("<html><p>Faces that share a common edge with the currently selected face(s) will be selected. "
				"</p>"
				"<p>"
				"Enter the angle allowed between the facing direction of faces, to select them. "
				"<br>"
				"%1 A small angle will select contiguous faces that are in the same plane as the selected face(s). "
				"<br>"
				"%1 An angle of 180 or larger will select all contiguous faces, regardless of their angle to each other. "
				"</p>"
				"</html>")
			.arg(g_Au.MiddleDotSymbol()));
	lbInfo->setWordWrap(true);

	QString cur = QVariant(angleTol).toString();
	m_AngleEdit = new QLineEdit(cur, this);

	QPushButton* okButton = new QPushButton(tr("Select"), this);
	okButton->setToolTip(tr("<html"
			"Select faces."
			"</html>"));
	okButton->setDefault(true);
	okButton->setAutoDefault(true);
	connect(okButton, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(okButton);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbInfo);
	lay->addWidget(m_AngleEdit);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuSelContiguousDlg::~AuSelContiguousDlg()
{
}

void AuSelContiguousDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuSelContiguousDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SelContiguousDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SelContiguousDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SelContiguousDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SelContiguousDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSelContiguousDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SelContiguousDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SelContiguousDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SelContiguousDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SelContiguousDlg/SizeY",sz.height());
}

float AuSelContiguousDlg::AngleTolerance() const
{
	return m_AngleEdit->text().toDouble();
}
