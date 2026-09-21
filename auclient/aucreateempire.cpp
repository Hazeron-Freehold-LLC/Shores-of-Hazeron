//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucreateempire.cpp,v 1.7 2022/02/19 18:10:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucreateempire.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "dgdata.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

AuCreateEmpire::AuCreateEmpire(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Create Empire"));
	setAuDlgIcon(QIcon(":data/CommGovernment.png"));

	QLabel* picLabel = new QLabel(this);
	picLabel->setPixmap(QPixmap(":data/HandcraftFlag.png"));

	QLabel* infoLabel = new QLabel(tr("<html>"
				"<p>"
				"You are a free avatar so you must create an empire before you craft a flag or claim a world. "
				"The new empire will be created with you as its leader. "
				"After your empire is created, you can handcraft flags and claim worlds. "
				"</p>"
				"<p>"
				"An empire is what you make of it. "
				"It constitutes an association of some kind, large or small, permanent or temporary. "
				"Your empire could be a simple clan, a band of pirates, a corporation, "
				"or an imperium destined to conquer the stars. "
				"</p>"
				"<p>"
				"The flag is a visual token of your empire. "
				"Citizens of your empire will live and die for the flag. "
				"They will treat it with respect, fly it at appropriate times, and wrap their loved ones in it when they die. "
				"Choose a flag that is worthy of respect. "
				"</p>"
				"<p>"
				"The flag picture must be at least 2x2 pixels in size. "
				"If the picture is larger than %1x%1 pixels, it will be reduced to fit within that size. "
				"<u>You can change your empire's flag picture at any time later.</u> "
				"</p>"
				"<p>"
				"Empire names must be unique in the universe. "
				"<u>The name of an empire can never be changed.</u> "
				"Check proper capitalization and spelling. "
				"Avoid starting empire names with <i>The</i>. "
				"</p>"
				"</html>").arg(DgEmpire::FlagImageMax()), this);
	infoLabel->setWordWrap(true);
	infoLabel->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	m_FlagButton = new QToolButton(this);
	QPixmap flagpix = QPixmap(":data/SurrenderFlag.png");
	m_FlagButton->setIcon(QIcon(flagpix));
	m_FlagButton->setIconSize(QSize(64,64));
	m_FlagButton->setToolTip(tr("<html>"
				"Choose the picture of the flag for the new empire. "
				"</html>"));
	m_FlagButton->setMaximumSize(QSize(68,68));
	m_FlagButton->setMinimumSize(QSize(68,68));
	connect(m_FlagButton, SIGNAL(clicked()), SLOT(ChooseFlag()));

	m_NameEdit = new QLineEdit(this);
	m_NameEdit->setToolTip(tr("<html>"
			"Enter a name for the new empire. "
			"The name must be less than %1 characters and "
			"it must not contain certain reserved characters such as | and newline. "
			"</html>").arg(MaxNameLength()+1));
	connect(m_NameEdit, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

	m_NameReason = new QLabel(this);

	m_OkButton = new QPushButton(tr("Create Empire"), this);
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	m_OkButton->setEnabled(false);
	m_OkButton->setToolTip(tr("<html>"
				"Creates a new empire with the flag and name provided. "
				"</html>"));
	connect(m_OkButton, SIGNAL(clicked()), SLOT(CreateEmpire()));

	// Layout
	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_NameReason);
	blay->addWidget(m_OkButton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* elay = new QHBoxLayout;
	elay->addWidget(m_FlagButton);
	elay->addWidget(m_NameEdit);
	elay->setMargin(0);
	elay->setSpacing(2);

	QHBoxLayout* llay = new QHBoxLayout;
	llay->addWidget(picLabel);
	llay->addWidget(infoLabel);
	llay->setMargin(0);
	llay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(llay);
	lay->addLayout(elay);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCreateEmpire::~AuCreateEmpire()
{
}

void AuCreateEmpire::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuCreateEmpire::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CreateEmpire1/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CreateEmpire1/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CreateEmpire1/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CreateEmpire1/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCreateEmpire::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CreateEmpire1/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CreateEmpire1/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CreateEmpire1/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CreateEmpire1/SizeY",sz.height());
}

void AuCreateEmpire::ChooseFlag()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastFlagFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString file = QFileDialog::getOpenFileName(this, tr("Choose Flag Image"), lastFile, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!file.isEmpty())
	{
		// Store last known file folder.
		g_Au.Settings().writeEntry("/Settings/LastFlagFile", file);

		QImage imFlag;
		if(imFlag.load(file))
		{
			if(imFlag.width() > DgEmpire::FlagImageMax() || imFlag.height() > DgEmpire::FlagImageMax())
			{
				// Resize the image.
				m_Flag = imFlag.scaled(DgEmpire::FlagImageMax(),DgEmpire::FlagImageMax(), Qt::KeepAspectRatio,
						Qt::SmoothTransformation);
			}
			else
			{
				m_Flag = imFlag;
				if(m_Flag.width() < 2 || m_Flag.height() < 2)
				{
					QMessageBox::critical(this, g_Au.ApplicationName(),
							tr("<html>"
								"The flag picture is too small. "
								"Please select a picture that is at least 2 pixels in width and height. "
								"</html>"),
							QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
					m_Flag = QImage();	// Discard invalid image.
				}
			}
			m_Flag = m_Flag.convertToFormat(QImage::Format_Indexed8);	// Reduce color depth for size.

			QPixmap flagpix = QPixmap::fromImage(m_Flag);
			m_FlagButton->blockSignals(true);
			m_FlagButton->setIcon(QIcon(flagpix));
			m_FlagButton->setIconSize(m_Flag.size());
			m_FlagButton->blockSignals(false);
			EnableControls();
		}
		else
		{
			QMessageBox::critical(this, g_Au.ApplicationName(),
					tr("The flag image could not be loaded. "
						"Many image formats are supported but some JPEG sub-formats give us trouble. "
						"If the image can be loaded with a paint program, try saving it as a different "
						"format of image such as PNG or BMP. "),
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void AuCreateEmpire::CreateEmpire()
{
	auApp->CreateNewEmpire(m_NameEdit->text().trimmed(), m_Flag);
	accept();
}

void AuCreateEmpire::EnableControls()
{
	QString reason;
	QString name = m_NameEdit->text().trimmed();
	if(name.isEmpty())
		reason = tr("Empire name needed.");
	else if(!IsNameValid(name))
		reason = tr("Empire name is not valid.");
	else
	{
		const DgData& gd = g_Au.LockDataForRead();
		quint32 empireId = gd.EmpireId(name);
		g_Au.UnlockData();
		if(empireId)
			reason = tr("Empire name already exists.");
		else if(m_Flag.isNull())
			reason = tr("Flag picture needed.");
	}
	m_OkButton->setEnabled(reason.isEmpty());
	m_NameReason->setText(reason);
}
