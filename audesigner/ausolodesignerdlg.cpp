//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausolodesignerdlg.cpp,v 1.11 2022/02/19 18:26:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausolodesignerdlg.h"
//
#include "audna.h"
#include "audnasettings.h"
#include "auglobal.h"
#include "auhaircolorwidget.h"
#include "auskincolorwidget.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

AuSoloDesignerDlg::AuSoloDesignerDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Avatar Settings"));
	setAuDlgIcon(QIcon(":data/Contacts.png"));
	SetWikiTopic("Designer");

	QLabel* lbIco = new QLabel(this);
	lbIco->setPixmap(QPixmap(":data/Designer.png").scaled(256,144,Qt::KeepAspectRatio,Qt::SmoothTransformation));

	QString lastName = g_Au.Settings().readEntry("Widget/SoloDesigner/Name", tr("Human"));
	GENDER lastGender = (GENDER)g_Au.Settings().readNumEntry("Widget/SoloDesigner/Gender", Male);
	HANDEDNESS lastHand = (HANDEDNESS)g_Au.Settings().readNumEntry("Widget/SoloDesigner/Handedness", RightHanded);

	m_DNA.InitDefaultHuman();
	QByteArray ba;
	g_Au.Settings().readEntry("Widget/SoloDesigner/DNA", ba);
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_5_7);
	AuDNABase* dna = AuDNABase::ReadDynamic(in);
	if(dna)
	{
		const AuDNA* dna4 = dynamic_cast<const AuDNA*>(dna);
		if(dna4)
			m_DNA = *dna4;
		delete dna;
	}

	QLabel* lbName = new QLabel(tr("Avatar Name"), this);
	lbName->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Name = new QLineEdit(lastName, this);

	m_Hand = new QComboBox(this);
	m_Hand->addItem(tr("Right Handed"), RightHanded);
	m_Hand->addItem(tr("Left Handed"), LeftHanded);
	m_Hand->setCurrentIndex(m_Hand->findData(lastHand));

	m_Gender = new QComboBox(this);
	m_Gender->addItem(tr("Male"), Male);
	m_Gender->addItem(tr("Female"), Female);
	m_Gender->setCurrentIndex(m_Gender->findData(lastGender));

	m_DNAButton = new QPushButton(QPixmap(":data/DNA.png"), tr("DNA"), this);
	connect(m_DNAButton, SIGNAL(clicked()), SLOT(DNAClicked()));

	QPushButton* pb = new QPushButton(tr("Enter Designer"), this);
	pb->setDefault(true);
	pb->setAutoDefault(true);
	connect(pb, SIGNAL(clicked()), SLOT(OkClicked()));

	QGridLayout* glay = new QGridLayout;
	int row=0;
	glay->addWidget(lbIco, row,0, 1,2);
	row++;
	glay->addWidget(lbName, row,0);
	glay->addWidget(m_Name, row,1);
	row++;
	glay->addWidget(m_Hand, row,1);
	row++;
	glay->addWidget(m_Gender, row,1);
	row++;
	glay->addWidget(m_DNAButton, row,1);
	row++;
	glay->setRowStretch(row,1);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pb);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(glay);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuSoloDesignerDlg::~AuSoloDesignerDlg()
{
}

void AuSoloDesignerDlg::setVisible(bool visible)
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

void AuSoloDesignerDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SoloDesigner/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SoloDesigner/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SoloDesigner/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SoloDesigner/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSoloDesignerDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SoloDesigner/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SoloDesigner/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SoloDesigner/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SoloDesigner/SizeY",sz.height());
}

void AuSoloDesignerDlg::OkClicked()
{
	// Save settings.
	g_Au.Settings().writeEntry("Widget/SoloDesigner/Name", Name());
	g_Au.Settings().writeEntry("Widget/SoloDesigner/Gender", Gender());
	g_Au.Settings().writeEntry("Widget/SoloDesigner/Handedness", Handedness());

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);
	m_DNA.WriteDynamic(out);
	g_Au.Settings().writeEntry("Widget/SoloDesigner/DNA", ba);

	accept();
}

QString AuSoloDesignerDlg::Name() const
{
	QString name = m_Name->text();
	if(!IsNameValid(name))
		name = tr("Human");
	return name;
}

GENDER AuSoloDesignerDlg::Gender() const
{
	return (GENDER)m_Gender->currentData().toInt();
}

HANDEDNESS AuSoloDesignerDlg::Handedness() const
{
	return (HANDEDNESS)m_Hand->currentData().toInt();
}

AuDNA AuSoloDesignerDlg::DNA() const
{
	return m_DNA;
}

void AuSoloDesignerDlg::DNAClicked()
{
	AuDNASettingsDlg dnaDlg(this);
	dnaDlg.SetDNA(m_DNA);
	if(dnaDlg.exec() != QDialog::Accepted)
		return;
	m_DNA = dnaDlg.DNA();
}

