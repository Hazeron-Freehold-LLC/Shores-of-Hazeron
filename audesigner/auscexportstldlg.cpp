//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscexportstldlg.cpp,v 1.5 2022/02/19 18:25:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscexportstldlg.h"
//
#include "auglobal.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScExportSTLDlg::AuScExportSTLDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Export STL"));
	setAuDlgIcon(QIcon(":data/ScMesh.png"));

	QLabel* lbfile = new QLabel(tr("Choose the .stl file to export."), this);
	lbfile->setWordWrap(true);

	QString fn = g_Au.Settings().readEntry(QString("ExportSTL/File"), ".stl");
	m_FileName = new QPushButton(fn, this);
	connect(m_FileName, SIGNAL(clicked()), SLOT(ChooseFile()));

	QLabel* lbuom = new QLabel(tr("Choose the unit of measure of distances in the .stl file."), this);
	lbuom->setWordWrap(true);

	int uom = g_Au.Settings().readNumEntry(QString("ExportSTL/UnitOfMeasure"), UOM_Inches);
	m_UnitsOfMeasure = new QComboBox(this);
	m_UnitsOfMeasure->addItem(tr("Inches"), UOM_Inches);
	m_UnitsOfMeasure->addItem(tr("Feet"), UOM_Feet);
	m_UnitsOfMeasure->addItem(tr("Millimeters"), UOM_Millimeters);
	m_UnitsOfMeasure->addItem(tr("Meters"), UOM_Meters);
	m_UnitsOfMeasure->setCurrentIndex(m_UnitsOfMeasure->findData(uom));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Exports the selected .stl file."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoExport()));

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
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScExportSTLDlg::~AuScExportSTLDlg()
{
}

void AuScExportSTLDlg::setVisible(bool visible)
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

void AuScExportSTLDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ExportSTLDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ExportSTLDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ExportSTLDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ExportSTLDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScExportSTLDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ExportSTLDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ExportSTLDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ExportSTLDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ExportSTLDlg/SizeY",sz.height());
}

QString AuScExportSTLDlg::FileName() const
{
	QString fileName = m_FileName->text();
	if(fileName == ".stl")
		fileName = "";
	return fileName;
}

void AuScExportSTLDlg::DoExport()
{
	QString fileName = FileName();
	if(!fileName.isEmpty())
	{
		g_Au.Settings().writeEntry("/ExportSTL/File", fileName);

		UNIT_OF_MEASURE uom = (UNIT_OF_MEASURE)m_UnitsOfMeasure->currentData().toInt();
		g_Au.Settings().writeEntry("/ExportSTL/UnitOfMeasure", uom);
	}

	accept();
}

void AuScExportSTLDlg::ChooseFile()
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

	QString loadFile = QFileDialog::getSaveFileName(this, tr("STL File"), fileName, "STL Files (*.stl);;All Files (*.*)", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!loadFile.isEmpty() && loadFile.right(4).compare(".stl", Qt::CaseInsensitive) == 0)
		m_FileName->setText(loadFile);
}

double AuScExportSTLDlg::ScaleToFeet() const
{
	UNIT_OF_MEASURE uom = (UNIT_OF_MEASURE)m_UnitsOfMeasure->currentData().toInt();
	return UnitScaleToFeet(uom);
}
