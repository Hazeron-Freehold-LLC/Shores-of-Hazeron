//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscimport3dsdlg.cpp,v 1.6 2022/02/19 18:25:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscimport3dsdlg.h"
//
#include "auglobal.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScImport3dsDlg::AuScImport3dsDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Import 3ds"));
	setAuDlgIcon(QIcon(":data/ScMesh.png"));

	QLabel* lbfile = new QLabel(tr("Choose the .3ds file to import."), this);
	lbfile->setWordWrap(true);

	QString fn = g_Au.Settings().readEntry(QString("Import3ds/File"), ".3ds");
	m_FileName = new QPushButton(fn, this);
	connect(m_FileName, SIGNAL(clicked()), SLOT(ChooseFile()));

	QLabel* lbuom = new QLabel(tr("Choose the unit of measure of distances in the .3ds file."), this);
	lbuom->setWordWrap(true);

	int uom = g_Au.Settings().readNumEntry(QString("Import3ds/UnitOfMeasure"), UOM_Inches);
	m_UnitsOfMeasure = new QComboBox(this);
	m_UnitsOfMeasure->addItem(tr("Inches"), UOM_Inches);
	m_UnitsOfMeasure->addItem(tr("Feet"), UOM_Feet);
	m_UnitsOfMeasure->addItem(tr("Millimeters"), UOM_Millimeters);
	m_UnitsOfMeasure->addItem(tr("Meters"), UOM_Meters);
	m_UnitsOfMeasure->setCurrentIndex(m_UnitsOfMeasure->findData(uom));

	QLabel* lbtexture = new QLabel(tr("<html>"
				"Imported faces refer to the <i>Custom</i> slot of the design's texture palette. "
				"A texture can be imported with the 3ds file. "
				"It will be loaded into the custom slot of the texture palette. "
				"The imported texture will replace any texture that is already loaded into the custom slot. "
				"</html>"), this);
	lbtexture->setWordWrap(true);

	bool textoo = g_Au.Settings().readBoolEntry(QString("Import3ds/TextureToo"), true);
	m_TextureToo = new QCheckBox(tr("Import texture to Custom palette slot."), this);
	m_TextureToo->setChecked(textoo);

	QString tn = g_Au.Settings().readEntry(QString("Import3ds/TextureFile"), "");
	m_TextureFileName = new QPushButton(tn, this);
	connect(m_TextureFileName, SIGNAL(clicked()), SLOT(ChooseTextureFile()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Imports the selected .3ds file."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoImport()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbfile);
	lay->addWidget(m_FileName);
	lay->addWidget(lbuom);
	lay->addWidget(m_UnitsOfMeasure);
	lay->addWidget(lbtexture);
	lay->addWidget(m_TextureToo);
	lay->addWidget(m_TextureFileName);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScImport3dsDlg::~AuScImport3dsDlg()
{
}

void AuScImport3dsDlg::setVisible(bool visible)
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

void AuScImport3dsDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/Import3dsDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/Import3dsDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/Import3dsDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/Import3dsDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScImport3dsDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/Import3dsDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/Import3dsDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/Import3dsDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/Import3dsDlg/SizeY",sz.height());
}

QString AuScImport3dsDlg::FileName() const
{
	QString fileName = m_FileName->text();
	if(fileName == ".3ds")
		fileName = "";
	return fileName;
}

QString AuScImport3dsDlg::TextureFileName() const
{
	return m_TextureFileName->text();
}

void AuScImport3dsDlg::DoImport()
{
	QString fileName = FileName();
	if(!fileName.isEmpty())
	{
		g_Au.Settings().writeEntry("/Import3ds/File", fileName);

		UNIT_OF_MEASURE uom = (UNIT_OF_MEASURE)m_UnitsOfMeasure->currentData().toInt();
		g_Au.Settings().writeEntry("/Import3ds/UnitOfMeasure", uom);

		bool textoo = TextureToo();
		g_Au.Settings().writeEntry("/Import3ds/TextureToo", textoo);

		if(textoo)
		{
			QString texFileName = TextureFileName();
			if(!texFileName.isEmpty())
				g_Au.Settings().writeEntry("/Import3ds/TextureFile", texFileName);
		}
	}

	accept();
}

void AuScImport3dsDlg::ChooseFile()
{
	// Get file name and open the file.
	QString fileName = FileName();
	if(fileName.isEmpty())
		fileName = QDir::currentPath();
	else
	{
		QFileInfo lastFileInfo(fileName);
		if(!lastFileInfo.exists())
			fileName = lastFileInfo.absolutePath();	// Use last known file folder.
	}

	QString loadFile = QFileDialog::getOpenFileName(this, tr("3ds File"), fileName, "3ds Files (*.3ds);;All Files (*.*)", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!loadFile.isEmpty() &&
			(loadFile.right(4).compare(".3ds", Qt::CaseInsensitive) == 0 ||
			 loadFile.right(5).compare(".3dsN", Qt::CaseInsensitive) == 0 ||
			 loadFile.right(5).compare(".3dsX", Qt::CaseInsensitive) == 0))
		m_FileName->setText(loadFile);
}

void AuScImport3dsDlg::ChooseTextureFile()
{
	// Get file name and open the file.
	QString fileName = FileName();
	if(fileName.isEmpty())
		fileName = QDir::currentPath();
	else
	{
		QFileInfo lastFileInfo(fileName);
		if(!lastFileInfo.exists())
			fileName = lastFileInfo.absolutePath();	// Use last known file folder.
	}

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Texture File"), fileName, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!loadFile.isEmpty())
		m_TextureFileName->setText(loadFile);
}

double AuScImport3dsDlg::ScaleToFeet() const
{
	UNIT_OF_MEASURE uom = (UNIT_OF_MEASURE)m_UnitsOfMeasure->currentData().toInt();
	return UnitScaleToFeet(uom);
}

bool AuScImport3dsDlg::TextureToo() const
{
	return m_TextureToo->isChecked();
}
