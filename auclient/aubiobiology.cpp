//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiobiology.cpp,v 1.23 2022/05/04 19:48:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubiobiology.h"
//
#include "auanimalia.h"
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

AuBioBiology::AuBioBiology(QWidget *parent) : QWidget(parent)
{
	m_timer = 0;

	lbdesc = new QLabel(this);
	lbdesc->setWordWrap(true);

	lbgender = new QLabel(this);
	lbgender->setWordWrap(true);

	lbborn = new QLabel(this);
	lbborn->setWordWrap(true);

	lblineage = new QLabel(this);
	lblineage->setWordWrap(true);

	ckautoeat = new QCheckBox(tr("Automatically Eat Food"), this);
	ckautoeat->setToolTip(tr("<html><b>Automatically Eat Food</b><br>"
				"When checked, the avatar will eat a meal of food from personal gear whenever hunger is less than 10%. "
				"The avatar eats items found in top level gear slots and inside containers. "
				"The avatar does not eat food items stored in containers that are inside containers. "
				"</html>"));
	connect(ckautoeat, SIGNAL(stateChanged(int)), SLOT(AutoEatChanged(int)));

	m_UpgradeDNA = new QPushButton(tr("Upgrade DNA..."), this);
	m_UpgradeDNA->setIcon(QIcon(":data/DNA.png"));
	m_UpgradeDNA->setVisible(false);
	m_UpgradeDNA->setToolTip(tr("<html><b>Upgrade DNA...</b><br>"
				"Changes the avatar DNA from generation 3 to generation 4.<br>"
				"%1 This button is only visible if your avatar body has generation 3 DNA.<br>"
				"%1 An avatar can only do this once."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_UpgradeDNA, SIGNAL(clicked()), SLOT(UpgradeDNA()));

	pbnamebaby = new QPushButton(tr("Name the Baby..."), this);
	pbnamebaby->setIcon(QIcon(":data/Pacifier.png"));
	pbnamebaby->setVisible(false);
	pbnamebaby->setToolTip(tr("<html><b>Name the Baby</b><br>"
				"Assigns a name to the baby you are carrying.<br>"
				"%1 This button is visible if your avatar is pregnant.<br>"
				"%1 If the baby is not named, a random name is assigned at birth."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbnamebaby, SIGNAL(clicked()), SLOT(NameTheBaby()));

	lbhandedness = new QLabel(this);
	lbhandedness->setWordWrap(true);

	lbhp = new QLabel(this);
	lbhp->setWordWrap(true);

	lbstam = new QLabel(this);
	lbstam->setWordWrap(true);

	lbweight = new QLabel(this);
	lbweight->setWordWrap(true);

	lbhunger = new QLabel(this);
	lbhunger->setWordWrap(true);

	m_ImmunizationIcon = new QLabel(this);
	m_ImmunizationIcon->setPixmap(QPixmap(":data/c_Antitoxin.png").scaled(16,16));
	m_ImmunizationIcon->hide();

	m_Immunization = new QLabel(this);
	m_Immunization->hide();

	// Layout the dialog.
	QHBoxLayout* ilay = new QHBoxLayout;
	ilay->addWidget(m_ImmunizationIcon);
	ilay->addWidget(m_Immunization);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbdesc, row,0, 1,2);
	row++;
	lay->addWidget(lblineage, row,0);
	row++;
	lay->addWidget(lbborn, row,0);
	row++;
	lay->addWidget(lbgender, row,0);
	lay->addWidget(pbnamebaby, row,1);
	row++;
	lay->addWidget(lbhandedness, row,0);
	lay->addWidget(m_UpgradeDNA, row,1);
	row++;
	lay->addWidget(lbweight, row,0, 1,2);
	row++;
	lay->addWidget(lbhp, row,0, 1,2);
	lay->addLayout(ilay, row,1);
	row++;
	lay->addWidget(lbstam, row,0, 1,2);
	row++;
	lay->addWidget(lbhunger, row,0);
	lay->addWidget(ckautoeat, row,1);
	row++;

	lay->setColumnStretch(0, 1);
	lay->setRowStretch(row, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);
}

AuBioBiology::~AuBioBiology()
{
}

QSize AuBioBiology::sizeHint() const
{
	return QSize(400,300);
}

void AuBioBiology::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuBioBiology::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Get the scene atom so we can show their information.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(!person)
	{
		view->UnlockSceneAtom();
		return;
	}

	int dnaVersion = 0;
	QString specieName;
	QString sdesc;
	AuDNABase* dna = person->DNAnew();
	if(dna)
	{
		dnaVersion = dna->Version();
		const DgData& gd = g_Au.LockDataForRead();
		specieName = gd.SpeciesName(dna->Species(), false);	// Return "" if the specie has not been named.
		g_Au.UnlockData();
		if(specieName.isEmpty())
			sdesc = tr("Species <b>%1</b>").arg(dna->SpeciesDescription());
		else
			sdesc = tr("Species <b>%2, %1</b>").arg(dna->SpeciesDescription()).arg(specieName);
		delete dna;
	}

	QString lineage;
	if(person->IsLineageHighborn())
		lineage = tr("Highborn");
	else if(person->IsLineageDemiavatar())
		lineage = tr("Demiavatar");
	else
		lineage = tr("Avatar");
	QString slineage = tr("Lineage <b>%1</b>").arg(lineage);

	QString sborn;
	if(person->IsLineageHighborn() || person->IsLineageDemiavatar())
		sborn = tr("Born <b>%1</b>").arg(person->CreatedTime().UTC().toLocalTime().toString());
	else
		sborn = tr("Decanted <b>%1</b>").arg(person->CreatedTime().UTC().toLocalTime().toString());

	QString gender;
	if(!person->IsFullGrown())
		gender = tr("Gender <b>%1 Child, %2 Minutes to Maturity</b>")
			.arg(GenderName(person->Gender()))
			.arg(person->MinutesToFullGrown());
	else
		gender = tr("Gender <b>%1 Adult</b>")
			.arg(GenderName(person->Gender()));

	const AuConPregnant* preg = (const AuConPregnant*)person->FindConditionClone(AuConPregnant::ConditionType());
	if(preg)
	{
		if(!preg->BabyName().isEmpty())
			pbnamebaby->setText(preg->BabyName() + "...");
		else
			pbnamebaby->setText(tr("Name the Baby..."));
		AuTime dueDate = preg->DueDate();
		delete preg;
		gender += tr("<br>Pregnant <b>Due %1</b>")
				.arg(dueDate.UTC().toLocalTime().toString());
	}
	else
		pbnamebaby->setText(tr("Name the Baby..."));

	QString handedness = tr("Handedness <b>%1</b>").arg(person->IsLeftHanded()?tr("Left Handed"):tr("Right Handed"));

	QString weight = tr("Body Weight <b>%L1kg</b> carrying <b>%L2kg</b> total <b>%L3kg</b>")
		.arg(person->Mass() * KilogramsPerPound(), 0,'f',1)
		.arg(person->Burden() * KilogramsPerPound(), 0,'f',1)
		.arg((person->Mass() + person->Burden()) * KilogramsPerPound(), 0,'f',1);

	QString sstam = tr("Stamina <b>%1 of %2</b>").arg(person->CurrentStamina()).arg(person->MaxStamina());

	QString shunger = tr("Hunger <b>%1 of %2</b>").arg(person->CurrentHunger()).arg(person->MaxHunger());

	QString shp = tr("Health <b>%1 of %2</b>").arg(person->CurrentHitPoints()).arg(person->MaxHitPoints());

	bool autoEat = person->AutoEat();

	const AuConAntitoxin* anti = dynamic_cast<const AuConAntitoxin*>(person->FindConditionLock(AuConAntitoxin::ConditionType()));
	quint8 effQ;
	if(anti)
		effQ = anti->EffectiveQuality();
	else
		effQ = 0;
	person->UnlockConditions();

	view->UnlockSceneAtom();

	lbdesc->setText(sdesc);
	lblineage->setText(slineage);
	lbborn->setText(sborn);
	lbhandedness->setText(handedness);
	lbgender->setText(gender);
	pbnamebaby->setVisible(preg);
	m_UpgradeDNA->setVisible(dnaVersion == 3);
	lbweight->setText(weight);
	lbstam->setText(sstam);
	lbhunger->setText(shunger);
	lbhp->setText(shp);
	ckautoeat->blockSignals(true);
	ckautoeat->setChecked(autoEat);
	ckautoeat->blockSignals(false);

	m_ImmunizationIcon->setVisible(effQ != 0);
	m_Immunization->setVisible(effQ != 0);
	if(effQ)
		m_Immunization->setText(tr("Immunization Level %1").arg(effQ));
}

void AuBioBiology::UpgradeDNA()
{
	QString lastFile = g_Au.Settings().readEntry("Settings/LastDNAFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("Hazeron DNA (*.dna)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Hazeron DNA File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("Settings/LastDNAFile", loadFile);

	// Read the DNA data from the file.
	AuDNA dna;
	int result = dna.LoadFile(loadFile);
	switch(result)
	{
	case 0:	// ok
		break;
	case 1:	// cannot open file
		{
			QString smsg = QObject::tr("Unable to open file for reading.\n\n%1").arg(loadFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		}
		return;
	case 2:	// bad file version
		{
			QString smsg = QObject::tr("File does not contain Hazeron DNA.\n\n%1").arg(loadFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		}
		return;
	}

	// Check DNA for validity.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(!person || person->DNAVersion() != 3)
	{
		view->UnlockSceneAtom();
		return;
	}
	AuDNA::CHROMATID chroma = person->Chromatid();
	HANDEDNESS hand = person->Handedness();
	view->UnlockSceneAtom();

	AuAnimalia animalia(dna, chroma, hand, Mature);
	int dnaCheck = animalia.AvatarCheck();
	if(dnaCheck)
	{
		QString smsg = QObject::tr("DNA is not valid for use by an avatar.");
		if(dnaCheck & 0x0001)
			smsg += "\n" + tr("Ability Points Bad");
		if(dnaCheck & 0x0002)
			smsg += "\n" + tr("Hit Points Bad");
		if(dnaCheck & 0x0024)
			smsg += "\n" + tr("Eye Height Bad");
		if(dnaCheck & 0x0050)
			smsg += "\n" + tr("Head Height Bad");
		if(dnaCheck & 0x0008)
			smsg += "\n" + tr("CG Height Bad");
		if(dnaCheck & 0x0080)
			smsg += "\n" + tr("Lungs Bad");
		QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		return;
	}
	else
	{
		QString smsg = QObject::tr("The DNA looks ok for use by your avatar.\n\n"
				"You will have to restart your client after the change.\n\n"
				"The avatar at the startup screen may take several minutes to change to the new DNA. "
				"The avatar can safely join the game. "
				"You do not have to wait for the starup screen to catch up.");
		int result = QMessageBox::information(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel);
		if(result != QMessageBox::Ok)
			return;
	}

	// DNA looks good.
	auApp->UpgradeDNA(dna);
}

void AuBioBiology::NameTheBaby()
{
	bool ok;
	QString name = QInputDialog::getText(this, g_Au.ApplicationName(),
			tr("Enter a name for the baby you are carrying."),
			QLineEdit::Normal, "", &ok).trimmed();
	if(!ok)
		return;

	if(!IsNameValid(name))
	{
		QMessageBox::critical(this, g_Au.ApplicationName(),
				tr("The baby name is invalid. A valid name must be less than %1 characters "
					"and must not contain certain reserved characters such as | and newline.")
				.arg(MaxNameLength()+1),
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		return;
	}

	auApp->NameTheBaby(name);
}

void AuBioBiology::AutoEatChanged(int state)
{
	auApp->SetAutoEat(state == Qt::Checked);
}

