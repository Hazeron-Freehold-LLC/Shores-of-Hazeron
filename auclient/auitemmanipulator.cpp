//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemmanipulator.cpp,v 1.10 2022/05/05 19:29:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitemmanipulator.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auitemdesc.h"
#include "daitem.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

AuItemManipulator::AuItemManipulator(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Item Manipulator"));
	setAuDlgIcon(QIcon(":data/OrderGetItem.png"));

	m_Item = 0;
	m_OwnerLabel = new QLabel();

	m_RelItem = new QRadioButton(tr("Relative to Item"));
	m_RelItem->setToolTip(tr("<html>Move and spin relative to the item.</html>"));
	m_RelItem->setChecked(true);

	m_RelEnv = new QRadioButton(tr("Relative to Environment"));
	m_RelEnv->setToolTip(tr("<html>Move and spin relative to the item's environment.</html>"));

	// Move
	QLabel* lbMove = new QLabel(tr("Move"));

	m_MoveDist = new QComboBox();
	m_MoveDist->setEditable(true);
	m_MoveDist->addItem(tr(".001"));	// 1mm
	m_MoveDist->addItem(tr(".01"));		// 1cm
	m_MoveDist->addItem(tr(".1"));		// 1dm
	m_MoveDist->addItem(tr("1"));		// 1m
	m_MoveDist->setCurrentText(tr(".1"));
	m_MoveDist->setToolTip("<html>Enter the distance to move the item with each button click, in meters.</html>");

	QToolButton* mvUp = new QToolButton;
	mvUp->setIcon(QIcon(":data/MoveItemZPos.png"));
	mvUp->setToolTip("<html>Moves the item up.</html>");
	connect(mvUp, SIGNAL(clicked()), SLOT(MoveUp()));

	QToolButton* mvDown = new QToolButton;
	mvDown->setIcon(QIcon(":data/MoveItemZNeg.png"));
	mvDown->setToolTip("<html>Moves the item down.</html>");
	connect(mvDown, SIGNAL(clicked()), SLOT(MoveDown()));

	QToolButton* mvLeft = new QToolButton;
	mvLeft->setIcon(QIcon(":data/MoveItemXNeg.png"));
	mvLeft->setToolTip("<html>Moves the item left.</html>");
	connect(mvLeft, SIGNAL(clicked()), SLOT(MoveLeft()));

	QToolButton* mvRight = new QToolButton;
	mvRight->setIcon(QIcon(":data/MoveItemXPos.png"));
	mvRight->setToolTip("<html>Moves the item right.</html>");
	connect(mvRight, SIGNAL(clicked()), SLOT(MoveRight()));

	QToolButton* mvForward = new QToolButton;
	mvForward->setIcon(QIcon(":data/MoveItemYPos.png"));
	mvForward->setToolTip("<html>Moves the item forward.</html>");
	connect(mvForward, SIGNAL(clicked()), SLOT(MoveForward()));

	QToolButton* mvBackward = new QToolButton;
	mvBackward->setIcon(QIcon(":data/MoveItemYNeg.png"));
	mvBackward->setToolTip("<html>Moves the item backward.</html>");
	connect(mvBackward, SIGNAL(clicked()), SLOT(MoveBackward()));

	// Rotate
	QLabel* lbSpin = new QLabel(tr("Spin"));

	m_SpinAngle = new QComboBox();
	m_SpinAngle->setEditable(true);
	m_SpinAngle->addItem(tr("1"));
	m_SpinAngle->addItem(tr("5"));
	m_SpinAngle->addItem(tr("11.25"));
	m_SpinAngle->addItem(tr("22.5"));
	m_SpinAngle->addItem(tr("45"));
	m_SpinAngle->addItem(tr("90"));
	m_SpinAngle->addItem(tr("180"));
	m_SpinAngle->setCurrentText(tr("5"));
	m_SpinAngle->setToolTip("<html>Enter the angle to spin the item with each button click, in degrees.</html>");

	QToolButton* rotXPos = new QToolButton;
	rotXPos->setIcon(QIcon(":data/SpinItemXPos.png"));
	rotXPos->setToolTip("<html>Spins the item around the X axis, in the positive direction. "
			"The X axis is usually side to side on the item.</html>");
	connect(rotXPos, SIGNAL(clicked()), SLOT(RotateXPos()));

	QToolButton* rotXNeg = new QToolButton;
	rotXNeg->setIcon(QIcon(":data/SpinItemXNeg.png"));
	rotXNeg->setToolTip("<html>Spins the item around the X axis, in the negative direction. "
			"The X axis is usually side to side on the item.</html>");
	connect(rotXNeg, SIGNAL(clicked()), SLOT(RotateXNeg()));

	QToolButton* rotYPos = new QToolButton;
	rotYPos->setIcon(QIcon(":data/SpinItemYPos.png"));
	rotYPos->setToolTip("<html>Spins the item around the Y axis, in the positive direction. "
			"The Y axis is usually front to back on the item.</html>");
	connect(rotYPos, SIGNAL(clicked()), SLOT(RotateYPos()));

	QToolButton* rotYNeg = new QToolButton;
	rotYNeg->setIcon(QIcon(":data/SpinItemYNeg.png"));
	rotYNeg->setToolTip("<html>Spins the item around the Y axis, in the negative direction. "
			"The Y axis is usually front to back on the item.</html>");
	connect(rotYNeg, SIGNAL(clicked()), SLOT(RotateYNeg()));

	QToolButton* rotZPos = new QToolButton;
	rotZPos->setIcon(QIcon(":data/SpinItemZPos.png"));
	rotZPos->setToolTip("<html>Spins the item around the Z axis, in the positive direction. "
			"The Z axis is usually up and down on the item.</html>");
	connect(rotZPos, SIGNAL(clicked()), SLOT(RotateZPos()));

	QToolButton* rotZNeg = new QToolButton;
	rotZNeg->setIcon(QIcon(":data/SpinItemZNeg.png"));
	rotZNeg->setToolTip("<html>Spins the item around the Z axis, in the negative direction. "
			"The Z axis is usually up and down on the item.</html>");
	connect(rotZNeg, SIGNAL(clicked()), SLOT(RotateZNeg()));

	QToolButton* info = new QToolButton();
	info->setIcon(QIcon(":data/MsgSolicitItem.png"));
	info->setToolTip("<html>Shows information about the item.</html>");
	connect(info, SIGNAL(clicked()), SLOT(Info()));

	QPushButton* grabIt = new QPushButton(tr("Take"));
	grabIt->setToolTip("<html>Picks up the item. The item is added to your inventory if space permits.</html>");
	connect(grabIt, SIGNAL(clicked()), SLOT(Take()));

	QPushButton* reset = new QPushButton(tr("Reset"));
	reset->setToolTip("<html>Restores the position and spin of the item. "
			"The item is restored to the way it was when you last clicked on it.</html>");
	connect(reset, SIGNAL(clicked()), SLOT(Reset()));

	QGridLayout* moveLay = new QGridLayout;
	int row=0;
	moveLay->addWidget(mvUp, row,3, 2,2);
	row+=2;
	moveLay->addWidget(mvLeft, row,0, 2,2);
	moveLay->addWidget(mvBackward, row,2, 2,2);
	moveLay->addWidget(mvForward, row,4, 2,2);
	moveLay->addWidget(mvRight, row,6, 2,2);
	row+=2;
	moveLay->addWidget(mvDown, row,3, 2,2);
	row+=2;
	moveLay->setRowStretch(row, 1);
	moveLay->setColumnStretch(8, 1);

	QGridLayout* spinLay = new QGridLayout;
	row=0;
	spinLay->addWidget(rotXPos, row,1, 2,2);
	spinLay->addWidget(rotYPos, row,3, 2,2);
	spinLay->addWidget(rotZPos, row,5, 2,2);
	row+=2;
	spinLay->addWidget(rotXNeg, row,1, 2,2);
	spinLay->addWidget(rotYNeg, row,3, 2,2);
	spinLay->addWidget(rotZNeg, row,5, 2,2);
	row+=2;
	spinLay->setRowStretch(row, 1);
	spinLay->setColumnStretch(7, 1);

	QGridLayout* lay = new QGridLayout;
	row=0;
	lay->addWidget(info, row,0, 2,2);
	lay->addWidget(m_OwnerLabel, row,2, 1,8);
	row++;
	lay->addWidget(m_RelItem, row,2, 1,8);
	row++;
	lay->addWidget(m_RelEnv, row,2, 1,8);
	row++;
	lay->addWidget(lbMove, row,0, 1,5);
	lay->addWidget(m_MoveDist, row,5, 1,5);
	row++;
	lay->addLayout(moveLay, row,1, 1,8);
	row++;
	lay->addWidget(lbSpin, row,0, 1,5);
	lay->addWidget(m_SpinAngle, row,5, 1,5);
	row++;
	lay->addLayout(spinLay, row,1, 1,8);
	row++;
	lay->addWidget(grabIt, row,0, 1,5);
	lay->addWidget(reset, row,5, 1,5);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuItemManipulator::~AuItemManipulator()
{
	if(m_Item)
		delete m_Item;
}

void AuItemManipulator::setVisible(bool visible)
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

void AuItemManipulator::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ItemManipulator/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ItemManipulator/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ItemManipulator/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ItemManipulator/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuItemManipulator::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ItemManipulator/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ItemManipulator/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ItemManipulator/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ItemManipulator/SizeY",sz.height());
}

void AuItemManipulator::SetItem(const DaItem& item)
{
	setAuDlgTitle(item.Name());
	if(m_Item)
		delete m_Item;
	if(item.HasItem())
	{
		m_Item = item.LockItem()->Clone();
		item.UnlockItem();
	}
	m_ItemId = item.Id();
	m_ItemStaticRoom = item.StaticRoom();
	m_ItemTransform = item.Transform();

	m_OwnerLabel->setText(tr("Displayed by %1").arg(item.OwnerName()));
}

void AuItemManipulator::MoveBackward()
{
	double amount = QVariant(m_MoveDist->currentText()).toDouble() * FeetPerMeter();
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemMoveBackward(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::MoveDown()
{
	double amount = QVariant(m_MoveDist->currentText()).toDouble() * FeetPerMeter();
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemMoveDown(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::MoveForward()
{
	double amount = QVariant(m_MoveDist->currentText()).toDouble() * FeetPerMeter();
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemMoveForward(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::MoveLeft()
{
	double amount = QVariant(m_MoveDist->currentText()).toDouble() * FeetPerMeter();
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemMoveLeft(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::MoveRight()
{
	double amount = QVariant(m_MoveDist->currentText()).toDouble() * FeetPerMeter();
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemMoveRight(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::MoveUp()
{
	double amount = QVariant(m_MoveDist->currentText()).toDouble() * FeetPerMeter();
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemMoveUp(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::Reset()
{
	auApp->ItemReset(m_ItemId, m_ItemTransform, m_ItemStaticRoom);
}

void AuItemManipulator::RotateXNeg()
{
	double amount = QVariant(m_SpinAngle->currentText()).toDouble() * DEG2RAD;
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemRotateXNeg(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::RotateXPos()
{
	double amount = QVariant(m_SpinAngle->currentText()).toDouble() * DEG2RAD;
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemRotateXPos(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::RotateYNeg()
{
	double amount = QVariant(m_SpinAngle->currentText()).toDouble() * DEG2RAD;
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemRotateYNeg(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::RotateYPos()
{
	double amount = QVariant(m_SpinAngle->currentText()).toDouble() * DEG2RAD;
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemRotateYPos(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::RotateZNeg()
{
	double amount = QVariant(m_SpinAngle->currentText()).toDouble() * DEG2RAD;
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemRotateZNeg(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::RotateZPos()
{
	double amount = QVariant(m_SpinAngle->currentText()).toDouble() * DEG2RAD;
	bool relativeToItem = m_RelItem->isChecked();
	auApp->ItemRotateZPos(m_ItemId, amount, relativeToItem);
}

void AuItemManipulator::Take()
{
	auApp->ItemTake(m_ItemId);

	// Nothing left to do.
	accept();
}

void AuItemManipulator::Info()
{
	if(!m_Item)
		return;
	AuItemDesc* dlg = new AuItemDesc(*m_Item, this);
	connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()));
	dlg->show();
}

QSize AuItemManipulator::sizeHint() const
{
	return QSize(800,600);
}

