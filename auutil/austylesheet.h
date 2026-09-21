//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austylesheet.h,v 1.19 2022/05/06 14:37:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStyleSheet_H
#define AuStyleSheet_H

#include <QApplication>
#include <QPainter>
#include <QPalette>

// NOTE: The png files referenced in the style sheet url's must be available. They are found in the AuGlobal library.
//       If you only include this file and not the AuGlobal library, you must put them into your resources. AuLauncher does this.

class AuStyleSheet
{
public:
	inline static int FontHeight()
	{
		QImage img(1,1, QImage::Format_ARGB32);
		QPainter p(&img);
		QFontMetrics fmet = p.fontMetrics();
		return fmet.height();
	}

	inline static QColor HazeronBlue()
	{
		return QColor(25, 54, 128);
	}

	inline static QColor HazeronBlue(float intensity)
	{
		QColor hazBlue(HazeronBlue());

		qreal h,s,v;
		hazBlue.getHsvF(&h, &s, &v);

		QColor hazBlueInt;
		hazBlueInt.setHsvF(h, s, v * intensity);
		return hazBlueInt;
	}

	// The lightest blue/grey in the button background image.
	inline static QColor HazeronButton()
	{
		return QColor(38, 52, 89);
	}

	inline static QColor HazeronButton(float intensity)
	{
		QColor hazButton(HazeronButton());

		qreal h,s,v;
		hazButton.getHsvF(&h, &s, &v);

		QColor hazButtonInt;
		hazButtonInt.setHsvF(h, s, v * intensity);
		return hazButtonInt;
	}

	inline static QColor HazeronButtonText()
	{
		return QColor(85,170,255,255);
	}

	// 'intensity' is 0 to 4, where 0 is dark and 4 is bright.
	inline static QColor HazeronGold(int intensity)
	{
		// Clamp intensity.
		if(intensity < 0)
			intensity = 0;
		else if(intensity > 4)
			intensity = 4;

		QColor gold[5] =
		{
			QColor(122, 79, 12, 255),
			QColor(163, 119, 47, 255),
			QColor(212, 163, 77, 255),
			QColor(254, 220, 105, 255),
			QColor(254, 252, 185, 255)
		};

		return gold[intensity];
	}

	// This is the orange color of wormhole lines.
	inline static QColor HazeronOrange()
	{
		return QColor(192, 64, 32);
	}

	inline static void MakePalette(QPalette& palette)
	{
		QColor hazBlue5(HazeronBlue());
		QColor hazBlue4(HazeronBlue(.8f));
		QColor hazBlue3(HazeronBlue(.6f));
		QColor hazBlue2(HazeronBlue(.4f));
		QColor hazBlue1(HazeronBlue(.2f));
		QColor hazBlue0(HazeronBlue(.1f));

		QColor hazButtonText(HazeronButtonText());
		QColor hazButtonTextDisabled(42,85,128,255);
		QColor hazHighlightedBase(102,148,242,255);
		QColor hazHighlightedText(0,0,0,255);
		QColor hazText(Qt::lightGray);
		QColor hazToolTipText(93,135,221,255);
		QColor hazWidgetBase(Qt::black);

		palette.setBrush(QPalette::Disabled, QPalette::Window, hazWidgetBase);
		palette.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(Qt::darkGray));
		palette.setBrush(QPalette::Disabled, QPalette::Base, hazBlue2);
		palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, hazBlue0);
		palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, hazBlue1);
		palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, hazToolTipText);
		palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(Qt::gray));
		palette.setBrush(QPalette::Disabled, QPalette::Button, hazBlue3);
		palette.setBrush(QPalette::Disabled, QPalette::ButtonText, hazButtonTextDisabled);
		palette.setBrush(QPalette::Disabled, QPalette::BrightText, QColor(Qt::lightGray));
		palette.setBrush(QPalette::Disabled, QPalette::Light, hazBlue5);
		palette.setBrush(QPalette::Disabled, QPalette::Midlight, hazBlue4);
		palette.setBrush(QPalette::Disabled, QPalette::Dark, hazBlue1);
		palette.setBrush(QPalette::Disabled, QPalette::Mid, hazBlue2);
		palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(Qt::black));
		palette.setBrush(QPalette::Disabled, QPalette::Highlight, hazHighlightedBase);
		palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, hazHighlightedText);
		palette.setBrush(QPalette::Disabled, QPalette::Link, QColor(Qt::white));
		palette.setBrush(QPalette::Disabled, QPalette::LinkVisited, QColor(Qt::lightGray));

		palette.setBrush(QPalette::Active, QPalette::Window, hazWidgetBase);
		palette.setBrush(QPalette::Active, QPalette::WindowText, hazText);
		palette.setBrush(QPalette::Active, QPalette::Base, hazBlue2);
		palette.setBrush(QPalette::Active, QPalette::AlternateBase, hazBlue0);
		palette.setBrush(QPalette::Active, QPalette::ToolTipBase, hazBlue1);
		palette.setBrush(QPalette::Active, QPalette::ToolTipText, hazToolTipText);
		palette.setBrush(QPalette::Active, QPalette::Text, hazText);
		palette.setBrush(QPalette::Active, QPalette::Button, hazBlue3);
		palette.setBrush(QPalette::Active, QPalette::ButtonText, hazButtonText);
		palette.setBrush(QPalette::Active, QPalette::BrightText, QColor(Qt::white));
		palette.setBrush(QPalette::Active, QPalette::Light, hazBlue5);
		palette.setBrush(QPalette::Active, QPalette::Midlight, hazBlue4);
		palette.setBrush(QPalette::Active, QPalette::Dark, hazBlue1);
		palette.setBrush(QPalette::Active, QPalette::Mid, hazBlue2);
		palette.setBrush(QPalette::Active, QPalette::Shadow, QColor(Qt::black));
		palette.setBrush(QPalette::Active, QPalette::Highlight, hazHighlightedBase);
		palette.setBrush(QPalette::Active, QPalette::HighlightedText, hazHighlightedText);
		palette.setBrush(QPalette::Active, QPalette::Link, QColor(Qt::white));
		palette.setBrush(QPalette::Active, QPalette::LinkVisited, QColor(Qt::lightGray));
		
		palette.setBrush(QPalette::Inactive, QPalette::Window, hazWidgetBase);
		palette.setBrush(QPalette::Inactive, QPalette::WindowText, hazText);
		palette.setBrush(QPalette::Inactive, QPalette::Base, hazBlue2);
		palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, hazBlue0);
		palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, hazBlue1);
		palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, hazToolTipText);
		palette.setBrush(QPalette::Inactive, QPalette::Text, hazText);
		palette.setBrush(QPalette::Inactive, QPalette::Button, hazBlue3);
		palette.setBrush(QPalette::Inactive, QPalette::ButtonText, hazButtonText);
		palette.setBrush(QPalette::Inactive, QPalette::BrightText, QColor(Qt::white));
		palette.setBrush(QPalette::Inactive, QPalette::Light, hazBlue5);
		palette.setBrush(QPalette::Inactive, QPalette::Midlight, hazBlue4);
		palette.setBrush(QPalette::Inactive, QPalette::Dark, hazBlue1);
		palette.setBrush(QPalette::Inactive, QPalette::Mid, hazBlue2);
		palette.setBrush(QPalette::Inactive, QPalette::Shadow, QColor(Qt::black));
		palette.setBrush(QPalette::Inactive, QPalette::Highlight, hazHighlightedBase);
		palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, hazHighlightedText);
		palette.setBrush(QPalette::Inactive, QPalette::Link, QColor(Qt::white));
		palette.setBrush(QPalette::Inactive, QPalette::LinkVisited, QColor(Qt::lightGray));
	}

	inline static void SetStyle(bool paletteOnly)
	{
		QPalette palette;
		MakePalette(palette);
		qApp->setPalette(palette);
		if(paletteOnly)
			return;

		QColor hazBlue5(HazeronBlue());
		QColor hazBlue4(HazeronBlue(.8f));
		QColor hazBlue2(HazeronBlue(.4f));
		QColor hazBlue1(HazeronBlue(.2f));

		QColor hazButtonBlueDim(HazeronButton(.5f));

		QColor hazGoldLight(HazeronGold(4));
		QColor hazGoldMidlight(HazeronGold(3));
		QColor hazGoldMid(HazeronGold(1));
		QColor hazGoldDark(HazeronGold(0));

		QColor hazButtonText(HazeronButtonText());
		QColor hazButtonTextDisabled(42,85,128,255);
		QColor hazButtonTextHover(171,213,255,255);
		QColor hazEditText(Qt::lightGray);
		QColor hazEditTextDisabled(Qt::gray);
		QColor hazGroupLines(HazeronOrange());
		QColor hazHeaderText(171,213,255,255);
		QColor hazHighlightedBase(102,148,242,255);
		QColor hazHighlightedText(0,0,0,255);
		QColor hazMenuBase(0,0,0,255);
		QColor hazMenuText(192,192,192,255);
		QColor hazScrollBase(37,64,121,255);
		QColor hazScrollBorder(0,0,0,255);
		QColor hazText(Qt::lightGray);
		QColor hazToolTipText(93,135,221,255);
		QColor hazWidgetBase(Qt::black);

		// Configure style sheet for tool tips.
		QString styleSheet;
		{
			styleSheet += QString("QLabel "
					"{ "
						"color: %2; "
					"} ")
				.arg(hazButtonText.name());

			styleSheet += QString("QCheckBox { "
						"color: %1; "
					"} ")
				.arg(hazButtonText.name());

			styleSheet += QString("QCheckBox:disabled { "
						"color: %1; "
					"} ")
				.arg(hazButtonTextDisabled.name());

			styleSheet += QString("QCheckBox::indicator:unchecked { "
						"image: url(:data/hazCheckUnchecked.png); "
					"} ");

			styleSheet += QString("QCheckBox::indicator:checked { "
						"image: url(:data/hazCheckChecked.png); "
					"} ");

			styleSheet += QString("QToolTip "
					"{ "
						"color: %1; "
						"background-color: %2; "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazToolTipText.name())
				.arg(hazBlue1.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name())
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name());

			styleSheet += QString("QPushButton "
					"{ "
						"border: 2px solid black; "
						"border-radius: 5px; "
						"border-image: url(:data/hazBlackbutton.png); "
						"padding: 2px 2px 2px 2px; "
					"} ");

			styleSheet += QString("QPushButton:hover "
					"{ "
						"color: %1; "
						"background-color: %2; "
						"border-image: none; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazButtonTextHover.name())
				.arg(hazButtonBlueDim.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name())
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name());

			styleSheet += QString("QPushButton:checked "
					"{ "
						"color: %1; "
						"background-color: %2; "
						"border-image: none; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazButtonTextHover.name())
				.arg(hazWidgetBase.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name())
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name());

			styleSheet += QString("QPushButton:pressed "
					"{ "
						"padding: 3 1 1 3; "
					"} ");

			styleSheet += QString("QToolButton "
					"{ "
						"border-radius: 5px; "
						"color: %1; "
						"padding: 2; "
					"} ")
				.arg(hazButtonText.name());

			styleSheet += QString("QToolButton:hover "
					"{ "
						"border-image: url(:data/hazBlackbutton.png); "
					"} ");

			styleSheet += QString("QToolButton:pressed, QToolButton:on, QToolButton:checked "
					"{ "
						"padding: 3 1 1 3; "
						"border-image: none; "
						"background-color: %1; "
						"border: 1px solid white; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazWidgetBase.name())
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name());

			styleSheet += QString("QGroupBox "
					"{ "
						"background-color: %1; "
						"border: 2px solid white; "
						"border-radius: 7px; "
						"border-color: %2; "
						"margin-top: 8px; "
						"padding-top: 8px; "
					"} ")
				.arg(hazWidgetBase.name())
				.arg(hazGroupLines.name());

			styleSheet += QString("QGroupBox::title { "
						"subcontrol-origin: margin; "
						"subcontrol-position: top center; "
						"padding: 2; "
						"background-color: %1; "
						"color: %2; "
					"} ")
				.arg(hazWidgetBase.name())
				.arg(hazText.name());

			styleSheet += QString("QLineEdit { "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"padding: 2; "
						"background: %1; "
						"color: %2; "
						"selection-color: %3; "
						"selection-background-color: %4; "
					"} ")
				.arg(hazBlue2.name())
				.arg(hazEditText.name())
				.arg(hazHighlightedText.name())
				.arg(hazHighlightedBase.name());

			styleSheet += QString("QLineEdit::!read-only { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue1.name())
				.arg(hazBlue4.name())
				.arg(hazBlue5.name())
				.arg(hazBlue2.name());

			styleSheet += QString("QLineEdit:read-only, QLineEdit::disabled { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue5.name())
				.arg(hazBlue2.name())
				.arg(hazBlue1.name())
				.arg(hazBlue4.name());

			styleSheet += QString("QLineEdit:focus { "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name());

			styleSheet += QString("QComboBox { "
						"border: 1px solid white; "
						"padding: 1px 4px 1px 4px; "
						"border-radius: 5px; "
						"border-image: url(:data/hazBlackbutton.png); "
						"background: %1; "
						"color: %2; "
						"selection-color: %3; "
						"selection-background-color: %4; "
					"} ")
				.arg(hazBlue2.name())
				.arg(hazEditText.name())
				.arg(hazHighlightedText.name())
				.arg(hazHighlightedBase.name());

			styleSheet += QString("QComboBox:on { "
						"padding-top: 3px; "
						"padding-left: 5px; "
						"background: %1; "
						"color: %2; "
					"} ")
				.arg(hazMenuBase.name())
				.arg(hazMenuText.name());

			styleSheet += QString("QComboBox::drop-down { "
						"subcontrol-origin: padding; "
						"subcontrol-position: top right; "
						"position: relative; "
						"border-image: url(:data/hazBlackbutton.png); "
						"right: 5; "
						"width: 15px; "
						"color: %2; "
					"} ")
				.arg(hazMenuText.name());

			styleSheet += QString("QComboBox::down-arrow { "
						"image: url(:data/hazUnExpand.png); "
					"} ");

			styleSheet += QString("QComboBox::down-arrow:on { "
						"top: 1px; "
						"left: 1px; "
					"} ");

			styleSheet += QString("QComboBox QAbstractItemView { "
						"background: %1; "
						"color: %2; "
						"selection-background-color: %3; "
						"selection-color: %4; "
						"border: 1px solid white; "
						"border-top-left-radius: 5px; "
						"border-bottom-left-radius: 5px; "
						"border-top-right-radius: 2px; "
						"border-bottom-right-radius: 2px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue2.name())
				.arg(hazEditText.name())
				.arg(hazHighlightedBase.name())
				.arg(hazHighlightedText.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name())
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name());

			styleSheet += QString("QDialog { "
						"background: %1; "
					"} ")
				.arg(hazWidgetBase.name());

			styleSheet += QString("QScrollArea { "
						"border: 0px solid black; "
					"} ");

			styleSheet += QString("QScrollBar:horizontal { "
						"background: %1; "
						"border: 2px solid %2; "
						"height: 14px; "
					"} ")
				.arg(hazScrollBase.name())
				.arg(hazScrollBorder.name());

			styleSheet += QString("QScrollBar:vertical { "
						"background: %1; "
						"border: 2px solid %2; "
						"width: 14px; "
					"} ")
				.arg(hazScrollBase.name())
				.arg(hazScrollBorder.name());

			styleSheet += QString("QSlider::groove:horizontal { "
						"background: %1; "
						"border: 1px solid %2; "
						"border-radius: 2px; "
						"height: 4px; "
						"margin: 3px; "
					"} ")
				.arg(hazWidgetBase.name())
				.arg(hazBlue5.name());

			styleSheet += QString("QSlider::handle { "
						"image: url(:data/hazSliderHandle.png); "
						"margin: -6; "
					"} ");

			styleSheet += QString("QListView { "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue5.name())
				.arg(hazBlue2.name())
				.arg(hazBlue1.name())
				.arg(hazBlue4.name());

			styleSheet += QString("QListView::item { "
						"border-radius: 4px; "
					"} ");

			styleSheet += QString("QListView::item:selected { "
						"background-color: %1; "
						"color: %2; "
					"} ")
				.arg(hazHighlightedBase.name())
				.arg(hazHighlightedText.name());

			styleSheet += QString("QTableView { "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue5.name())
				.arg(hazBlue2.name())
				.arg(hazBlue1.name())
				.arg(hazBlue4.name());

			styleSheet += QString("QTableView::item { "
						"border-radius: 4px; "
					"} ");

			styleSheet += QString("QTableView::item:selected { "
						"background-color: %1; "
						"color: %2; "
					"} ")
				.arg(hazHighlightedBase.name())
				.arg(hazHighlightedText.name());

			styleSheet += QString("QTreeView { "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue5.name())
				.arg(hazBlue2.name())
				.arg(hazBlue1.name())
				.arg(hazBlue4.name());

			styleSheet += QString("QTreeView::item { "
						"border-radius: 4px; "
					"} ");

			styleSheet += QString("QTreeView::item:selected { "
						"background-color: %1; "
						"color: %2; "
					"} ")
				.arg(hazHighlightedBase.name())
				.arg(hazHighlightedText.name());

			styleSheet += QString("QTreeView::branch:has-siblings { "
						"border-image: url(:data/hazBranchVLine.png) 0; "
						"border-radius: 5px; "
					"} ");

			styleSheet += QString("QTreeView::branch:!has-siblings:has-children, "
					"QTreeView::branch:!has-siblings:adjoins-item { "
						"border-image: url(:data/hazBranchEnd.png) 0; "
						"border-radius: 5px; "
					"} ");

			styleSheet += QString("QTreeView::branch:adjoins-item:!has-children { "
						"image: url(:data/hazBranchItem.png); "
						"border-radius: 5px; "
					"} ");

			styleSheet += QString("QTreeView::branch:open:has-children { "
						"image: url(:data/hazBranchOpen.png); "
						"border-radius: 5px; "
					"} ");

			styleSheet += QString("QTreeView::branch:closed:has-children { "
						"image: url(:data/hazBranchClosed.png); "
						"border-radius: 5px; "
					"} ");

			styleSheet += QString("QHeaderView::section { "
						"color: %1; "
						"border-radius: 5px; "
						"margin: 0 1 0 0; "
						"border-image: url(:data/hazBlackbutton.png); "
					"} ")
				.arg(hazHeaderText.name());

			styleSheet += QString("QHeaderView::down-arrow { "
						"image: url(:data/hazHeaderSortDown.png); "
						"margin: -4; "
					"} ");

			styleSheet += QString("QHeaderView::up-arrow { "
						"image: url(:data/hazHeaderSortUp.png); "
						"margin: -4; "
					"} ");

			styleSheet += QString("QTextEdit { "
						"background: %1; "
						"color: %2; "
						"selection-color: %3; "
						"selection-background-color: %4; "
						"padding: 2; "
						"border: 1px solid white; "
						"border-radius: 5px; "
					"} ")
				.arg(hazBlue2.name())
				.arg(hazEditText.name())
				.arg(hazHighlightedText.name())
				.arg(hazHighlightedBase.name());

			styleSheet += QString("QTextEdit:!read-only { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue1.name())
				.arg(hazBlue4.name())
				.arg(hazBlue5.name())
				.arg(hazBlue2.name());

			styleSheet += QString("QTextEdit:read-only { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue5.name())
				.arg(hazBlue2.name())
				.arg(hazBlue1.name())
				.arg(hazBlue4.name());

			styleSheet += QString("QTextEdit:!read-only:focus { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name());

			styleSheet += QString("QTextEdit:read-only:focus { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name())
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name());

			styleSheet += QString("QSpinBox { "
						"border: 1px solid white; "
						"border-top-left-radius: 5px; "
						"border-bottom-left-radius: 5px; "
						"border-top-right-radius: 2px; "
						"border-bottom-right-radius: 2px; "
						"padding: 2; "
						"background: %1; "
						"color: %2; "
						"selection-color: %3; "
						"selection-background-color: %4; "
					"} ")
				.arg(hazBlue2.name())
				.arg(hazEditText.name())
				.arg(hazHighlightedText.name())
				.arg(hazHighlightedBase.name());

			styleSheet += QString("QSpinBox:focus { "
						"border: 1px solid white; "
						"border-radius: 5px; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazGoldDark.name())
				.arg(hazGoldMidlight.name())
				.arg(hazGoldLight.name())
				.arg(hazGoldMid.name());

			styleSheet += QString("QSpinBox:disabled { "
						"color: %1; "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazEditTextDisabled.name())
				.arg(hazBlue5.name())
				.arg(hazBlue2.name())
				.arg(hazBlue1.name())
				.arg(hazBlue4.name());

			styleSheet += QString("QSpinBox::!disabled:!focus { "
						"border-top-color: %6; "
						"border-right-color: %7; "
						"border-bottom-color: %8; "
						"border-left-color: %9; "
					"} ")
				.arg(hazBlue1.name())
				.arg(hazBlue4.name())
				.arg(hazBlue5.name())
				.arg(hazBlue2.name());

			styleSheet += QString("QRadioButton::indicator "
					"{ "
						"width: 14px; "
						"height: 14px; "
					"} ");

			styleSheet += QString("QRadioButton::indicator:checked "
					"{ "
						"border-image: url(:data/hazRadioChecked.png); "
					"} ");

			styleSheet += QString("QRadioButton::indicator:unchecked "
					"{ "
						"border-image: url(:data/hazRadioUnchecked.png); "
					"} ");

			styleSheet += QString("QTabWidget::pane { " /* The tab widget frame */
						"border-top: 2px solid %1; "
					"} ")
				.arg(hazWidgetBase.name());

			styleSheet += "QTabWidget::tab-bar { "
						"left: 5px; " /* move to the right by 5px */
					"} ";

			/* Style the tab using the tab sub-control. Note that
				   it reads QTabBar _not_ QTabWidget */
			styleSheet += QString("QTabBar::tab:top { "
						"background-color: %1; "
						"border: 2px solid %2; "
						"border-bottom-color: %2; "
						"border-bottom-width: 1; "
						"border-top-left-radius: 5px; "
						"border-top-right-radius: 16px; "
						"min-width: 8ex; "
						"padding: 2px 1em 2px 1em; "
					"} ")
				.arg(hazWidgetBase.name())
				.arg(hazBlue5.name());

			styleSheet += QString("QTabBar::tab:bottom { "
						"background-color: %1; "
						"border: 2px solid %2; "
						"border-top-color: %2; "
						"border-top-width: 1; "
						"border-bottom-left-radius: 5px; "
						"border-bottom-right-radius: 16px; "
						"min-width: 8ex; "
						"padding: 2px 1em 2px 1em; "
					"} ")
				.arg(hazWidgetBase.name())
				.arg(hazBlue5.name());

			styleSheet += "QTabBar::tab:top:!selected { "
						"margin-top: 2px; " /* make non-selected tabs look smaller */
						"border-image: url(:data/hazBlackbutton.png); "
					"} ";

			styleSheet += "QTabBar::tab:bottom:!selected { "
						"margin-bottom: 2px; " /* make non-selected tabs look smaller */
						"border-image: url(:data/hazBlackbutton.png); "
					"} ";

			/* make use of negative margins for overlapping tabs */
			styleSheet += QString("QTabBar::tab:top:selected { "
						/* expand/overlap to the left and right by 4px */
						"margin-left: -4px; "
						"margin-right: -4px; "
						"border-bottom-color: %1; "
					"} ")
				.arg(hazWidgetBase.name());

			styleSheet += QString("QTabBar::tab:bottom:selected { "
						/* expand/overlap to the left and right by 4px */
						"margin-left: -4px; "
						"margin-right: -4px; "
						"border-top-color: %1; "
					"} ")
				.arg(hazWidgetBase.name());

			styleSheet += "QTabBar::tab:first:selected { "
						"margin-left: 0; " /* the first selected tab has nothing to overlap with on the left */
					"} ";

			styleSheet += "QTabBar::tab:last:selected { "
						"margin-right: 0; " /* the last selected tab has nothing to overlap with on the right */
					"} ";

			styleSheet += "QTabBar::tab:only-one { "
						"margin: 0; " /* if there is only one tab, we don't want overlapping margins */
					"} ";

			styleSheet += "QTabBar::tear { "
						//"image: url(tear_indicator.png); "
					"} ";

			styleSheet += "QTabBar::scroller { " /* the width of the scroll buttons */
						"width: 20px; "
					"} ";

			styleSheet += "QTabBar QToolButton { " /* the scroll buttons are tool buttons */
						//"border-image: url(scrollbutton.png) 2; "
						"border-width: 2px; "
					"} ";

			styleSheet += "QTabBar QToolButton::right-arrow { " /* the arrow mark in the tool buttons */
						"image: url(:data/hazRightArrow.png); "
					"} ";

			styleSheet += "QTabBar QToolButton::left-arrow { "
						"image: url(:data/hazLeftArrow.png); "
					"} ";

			styleSheet += "QTabBar::close-button { "
						"image: none; "
						"subcontrol-position: left; "
					"} ";

			styleSheet += "QTabBar::close-button:hover { "
						"image: url(:data/TabClose.png); "
					"} ";
		}
		qApp->setStyleSheet(styleSheet);
	}

	inline static int TabHeight()
	{
		return FontHeight() + 2;
	}
};

#endif // AuStyleSheet_H
