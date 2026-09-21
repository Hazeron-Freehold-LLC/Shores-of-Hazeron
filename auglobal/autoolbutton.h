//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autoolbutton.h,v 1.5 2022/02/19 18:31:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuToolButtonExpand_H
#define AuToolButtonExpand_H

#include "augloballib.h"
#include <QToolButton>
#include <QWidget>

class QSpinBox;
class QToolButton;

class AUGLOBAL_DLL AuToolButtonExpand : public QWidget
{
	Q_OBJECT;
	QToolButton*						m_ExpandButton;
	QToolButton*						m_MainButton;

public:
	AuToolButtonExpand(QWidget* parent=0);
	void SetExpandToolTip(const QString& tip);
	void SetIcon(const QIcon& icon);
	void SetIconSize(const QSize& size);
	void SetToolTip(const QString& tip);
	virtual void setVisible(bool visible);

signals:
	void Clicked();
	void ExpandClicked();

protected slots:
	virtual void BaseButtonClicked();
	virtual void BaseExpandClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuToolButtonExpandValue : public AuToolButtonExpand
{
	Q_OBJECT;
	int									m_Value;

public:
	AuToolButtonExpandValue(int value, QWidget* parent=0);

signals:
	void Clicked(int value);
	void ExpandClicked(int value);

protected slots:
	virtual void BaseButtonClicked();
	virtual void BaseExpandClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuToolButtonValue : public QToolButton
{
	Q_OBJECT;
	int		m_Value;

public:
	AuToolButtonValue(int value, QWidget* parent=0) : QToolButton(parent)
	{
		m_Value = value;
		setAutoRaise(true);
		connect(this, SIGNAL(clicked()), SLOT(BaseClicked()));
	}
	AuToolButtonValue(const QIcon& icon, int value, QWidget* parent=0) : QToolButton(parent)
	{
		m_Value = value;
		setAutoRaise(true);
		setIcon(icon);
		connect(this, SIGNAL(clicked()), SLOT(BaseClicked()));
	}
	AuToolButtonValue(const char* iconName, int value, QWidget* parent=0) : QToolButton(parent)
	{
		m_Value = value;
		setAutoRaise(true);
		setIcon(QIcon(iconName));
		connect(this, SIGNAL(clicked()), SLOT(BaseClicked()));
	}

	inline void SetValue(int value)
	{
		m_Value = value;
	}

	inline int Value() const
	{
		return m_Value;
	}

signals:
	void ClickedValue(int);

protected slots:
	void BaseClicked()
	{
		// Emit my version of the clicked signal that passes the value.
		emit ClickedValue(m_Value);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuToolButtonNumberValue : public QWidget
{
	Q_OBJECT;
	QToolButton*						m_Button;
	QSpinBox*							m_SpinBox;
	int									m_Value;

public:
	AuToolButtonNumberValue(int value, QWidget* parent=0);

	inline QToolButton* ToolButton()
	{
		return m_Button;
	}

	inline QSpinBox* SpinBox()
	{
		return m_SpinBox;
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
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuIconButton : public QToolButton
{
	Q_OBJECT;

public:
	AuIconButton(const QIcon& icon, QWidget* parent = 0);
	AuIconButton(const char* imageName, QWidget* parent = 0);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuIconButtonValue : public AuIconButton
{
	Q_OBJECT;
	int		m_Value;

public:
	AuIconButtonValue(const QIcon& icon, int value, QWidget* parent=0) : AuIconButton(icon, parent)
	{
		m_Value = value;
		connect(this, SIGNAL(clicked()), SLOT(BaseClicked()));
	}
	AuIconButtonValue(const char* imageName, int value, QWidget* parent=0) : AuIconButton(imageName, parent)
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
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuIconButtonData : public QToolButton
{
	Q_OBJECT;
	QVariant							m_Data;

public:
	AuIconButtonData(const QIcon& icon, const QVariant& data, QWidget* parent = 0);
	AuIconButtonData(const char* imageName, const QVariant& data, QWidget* parent = 0);

	inline const QVariant& Data() const
	{
		return m_Data;
	}

	inline void SetData(const QVariant& data)
	{
		m_Data = data;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuHelpButton : public AuIconButton
{
	Q_OBJECT;
	QString								m_Help;

public:
	AuHelpButton(const QString& topicOrUrl, QWidget* parent = 0);
	AuHelpButton(QWidget* parent = 0);

public:
	inline void SetTopicOrUrl(const QString& topicOrUrl)
	{
		m_Help = topicOrUrl;
	}

protected slots:
	void ShowHelp() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuToolButtonExpand_H
