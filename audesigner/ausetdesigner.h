//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetdesigner.h,v 1.3 2022/02/19 18:23:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetDesigner_H
#define AuSetDesigner_H

#include <QImage>
#include <QPushButton>
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSetDesigner : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_BackColorButton;
	QPushButton*						m_FontButton;
	QPushButton*						m_GridColorButton;
	QPushButton*						m_GridXColorButton;
	QPushButton*						m_GridYColorButton;
	QPushButton*						m_GridZColorButton;
	QPushButton*						m_LadderColorButton;
	QPushButton*						m_LineColorButton;
	QPushButton*						m_SelColorButton;
	QComboBox*							m_UnitOfMeasure;
	QLabel*								m_VertSizeLabel;
	QPushButton*						m_WalkColorButton;

public:
	AuSetDesigner(QWidget *parent=0);
	~AuSetDesigner();

protected slots:
	void BackColorClicked();
	void FontClicked();
	void GridColorClicked();
	void GridXColorClicked();
	void GridYColorClicked();
	void GridZColorClicked();
	void LadderColorClicked();
	void LineColorClicked();
	void SelColorClicked();
	void SetVertexSize(int value);
	void UnitOfMeasureChanged(int index);
	void WalkColorClicked();
};

#endif // AuSetDesigner_H
