//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetoverlay.h,v 1.2 2022/02/19 18:23:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetOverlay_H
#define AuSetOverlay_H

#include <QImage>
#include <QPushButton>
#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class QSlider;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuViewButtonStyleButton : public QPushButton
{
	Q_OBJECT;
	QImage								m_Image;

public:
	AuViewButtonStyleButton(const QImage& image, QWidget* parent=0);

	void SetImage(const QImage& image)
	{
		m_Image = image;
	}

protected:
	virtual void paintEvent(QPaintEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSetOverlay : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QCheckBox*							ckdrawminime;
	QCheckBox*							ckshowwindowsize;
	QSlider*							bodystatsize;
	QCheckBox*							m_ActionButtons;
	QLabel*								m_ButtonSize;
	AuViewButtonStyleButton*			m_ButtonStyle;
	QLabel*								m_ButtonTrans;
	QPushButton*						m_FontAtomButton;
	QLabel*								m_HealthBarSize;
	QLabel*								m_ResourceIconSize;
	QCheckBox*							m_ShowNet;
	QCheckBox*							m_ShowThreads;
	QSlider*							slbuttonsize;
	QSlider*							slbuttontrans;
	QSlider*							slresourcesize;

public:
	AuSetOverlay(QWidget *parent=0);
	~AuSetOverlay();

protected slots:
	void ButtonStyleClicked();
	void FontAtomClicked();
	void LeftRightCursorChanged(int value);
	void LeftRightOrientationChanged(int value);
	void SetBodyStatSize(int value);
	void SetButtonSize(int value);
	void SetButtonTransparency(int value);
	void SetDrawMiniMe(int value);
	void SetShowActionButtons(int value);
	void SetShowNet(int value);
	void SetShowThreads(int value);
	void SetShowWindowSize(int value);
	void SetResourceIconSize(int value);
	void ViewButtonCheckChanged(int checkState, int vb);
};

#endif // AuSetOverlay_H
