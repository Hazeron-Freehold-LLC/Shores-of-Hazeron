//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupushbutton.h,v 1.8 2022/02/19 18:31:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPushButton_H
#define AuPushButton_H

#include "augloballib.h"
#include <QGridLayout>
#include <QPushButton>
#include <QSpinBox>

class QMouseEvent;
class QPaintEvent;
class QToolButton;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuHueSelector : public QWidget
{
	Q_OBJECT;
private:
	quint8			m_Hue;			// The hue value, from 0 to 255.
	quint8			m_Saturation;	// The saturation value, from 0 to 255.

public:
	AuHueSelector(QWidget* parent=0);

	// Returns the color set in the control, assuming a maximum V, in the HSV model.
	QColor Color() const;

	inline quint8 Hue() const
	{
		return m_Hue;
	}

	virtual QSize sizeHint() const;

public slots:
	void SetHue(quint8 hue);
	void SetSaturation(quint8 sat);

signals:
	void HueChanged(quint8 hue);

protected:
	virtual void mousePressEvent(QMouseEvent* ev);
	virtual void paintEvent(QPaintEvent* ev);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuPushButton : public QPushButton
{
	Q_OBJECT;
public:
	AuPushButton(QWidget* parent=0) : QPushButton(parent)
	{
	}
	AuPushButton(const QString& text, QWidget* parent=0) : QPushButton(text, parent)
	{
	}

protected:
    void mouseDoubleClickEvent(QMouseEvent* me);

signals:
	void DoubleClicked();
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuPushButtonValue : public QPushButton
{
	Q_OBJECT;
public:
	AuPushButtonValue(int value, QWidget* parent=0) : QPushButton(parent)
	{
		m_Value = value;
		connect(this, SIGNAL(clicked()), SLOT(BaseClicked()));
	}
	AuPushButtonValue(const QString& text, int value, QWidget* parent=0) : QPushButton(text, parent)
	{
		m_Value = value;
		connect(this, SIGNAL(clicked()), SLOT(BaseClicked()));
	}

signals:
	void ClickedValue(int);

protected slots:
	void BaseClicked()
	{
		// Emit my version of the clicked signal that passes the value.
		emit ClickedValue(m_Value);
	}

private:
	int		m_Value;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuPushButtonNumberValue : public QWidget
{
	Q_OBJECT;
public:
	AuPushButtonNumberValue(int value, QWidget* parent=0) : QWidget(parent)
	{
		m_Value = value;

		QPushButton* cb = new QPushButton;
		cb->setObjectName("pb");
		connect(cb, SIGNAL(clicked()), SLOT(BaseClicked()));

		QSpinBox* sb = new QSpinBox;
		sb->setAlignment(Qt::AlignRight);
		sb->setObjectName("sb");
		connect(sb, SIGNAL(valueChanged(int)), SLOT(BaseNumberChanged(int)));

		QGridLayout* hlay = new QGridLayout(this);
		hlay->addWidget(cb, 0,0);
		hlay->setColumnStretch(0, 1);
		hlay->addWidget(sb, 0,1);
		hlay->setMargin(0);
		hlay->setSpacing(2);
		setLayout(hlay);
	}

	QPushButton* PushButton()
	{
		return findChild<QPushButton*>("pb");
	}

	QSpinBox* SpinBox()
	{
		return findChild<QSpinBox*>("sb");
	}

signals:
	void ClickedValue(int value);
	void CurrentNumberChanged(int number, int value);

protected slots:
	void BaseClicked()
	{
		// Emit my version of the clicked signal that passes the value.
		emit ClickedValue(m_Value);
	}

	void BaseNumberChanged(int value)
	{
		// Emit my version of the number changed signal that passes the value.
		emit CurrentNumberChanged(value, m_Value);
	}

private:
	int		m_Value;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuColorButton : public QPushButton
{
	Q_OBJECT;
	QColor								m_Color;
	QList<QColor>						m_CustomColors;

public:
	AuColorButton(QWidget* parent=0);

	// Returns the color set in the control, assuming a maximum V, in the HSV model.
	QColor Color() const;

	virtual QSize sizeHint() const;

public slots:
	void SetColor(QColor color);
	void SetCustomColors(const QList<QColor>& customColors);

signals:
	void ColorChanged(QColor color);

protected:
	virtual void paintEvent(QPaintEvent* ev);

private slots:
	void PickColor();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuPushButtonExpand : public QWidget
{
	Q_OBJECT;
	QToolButton*						m_ExpandButton;
	AuPushButton*						m_MainButton;

public:
	AuPushButtonExpand(QWidget* parent=0);
	void SetFocusPolicy(Qt::FocusPolicy policy);
	void SetExpandToolTip(const QString& tip);
	void SetText(const QString& text);
	void SetIconSize(const QSize& size);
	void SetStyleSheet(const QString& styleSheet);
	void SetToolTip(const QString& tip);
	virtual void setVisible(bool visible);
	QString Text() const;

protected slots:
	virtual void BaseButtonClicked();
	virtual void BaseButtonDoubleClicked();
	virtual void BaseExpandClicked();

signals:
	void Clicked();
	void DoubleClicked();
	void ExpandClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuPushButtonExpandValue : public AuPushButtonExpand
{
	Q_OBJECT;
	int									m_Value;

public:
	AuPushButtonExpandValue(int value, QWidget* parent=0);

	inline int Value() const
	{
		return m_Value;
	}

protected slots:
	virtual void BaseButtonClicked();
	virtual void BaseButtonDoubleClicked();
	virtual void BaseExpandClicked();

signals:
	void Clicked(int value);
	void DoubleClicked(int value);
	void ExpandClicked(int value);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuPushButton_H
