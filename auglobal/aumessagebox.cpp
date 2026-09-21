//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumessagebox.cpp,v 1.5 2022/02/19 18:33:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumessagebox.h"
//
#include "audlgtitle.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPainter>
#include <QStyle>
#include <QVBoxLayout>

//constructer
AuMessageBox::AuMessageBox(int icon, const QString &title, const QString &text, QString /*buttons*/, QWidget *parent,
		Qt::WindowFlags f) : QDialog( parent, f | Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::MSWindowsFixedSizeDialogHint)
{	 
    m_titleBar = new AuDlgTitleBar(this); 
    m_titleBar->SetHasFocus(true);
   
    m_vbox = new QVBoxLayout(this);
    
    //////////////////////////
    QLabel *iconLabel = new QLabel;
    iconLabel->setObjectName(QLatin1String("au_msgboxex_icon_label"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    QLabel *label = new QLabel(text);
    label->setObjectName(QLatin1String("au_msgbox_label"));
    label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this)));
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setOpenExternalLinks(true);
    label->setContentsMargins(2, 0, 0, 0);
    label->setIndent(9);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setObjectName(QLatin1String("au_msgbox_buttonbox"));
    buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));

    QGridLayout *grid = new QGridLayout;

    grid->setMargin(3);
    grid->setSpacing(0);
    //setContentsMargins(0, 0, 24, 20);
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    grid->addWidget(label, 0, 1, 1, 1);
    // -- leave space for information label --
    grid->setRowStretch(1, 100);
    grid->setRowMinimumHeight(2, 10);
    grid->addWidget(buttonBox, 3, 1, 1, 1);

    grid->setSizeConstraint(QLayout::SetNoConstraint);
    //q->setLayout(grid);

    if (!title.isEmpty() || !text.isEmpty()) 
    {
        //m_titleBar->setTitle(title);
        m_titleBar->SetText(title);
		setWindowTitle(title);
    }
    setModal(true);
    
    //title bar
    m_vbox->addWidget(m_titleBar);
	m_vbox->setStretchFactor(m_titleBar, 0);
	m_vbox->insertStretch ( 1, 0 );
    m_vbox->setMargin(3);
    m_vbox->setSpacing(0);
    
    m_vbox->insertLayout(1, grid , 1);
    
    //label->setText(text);
    buttonBox->addButton(QDialogButtonBox::Ok);
    
    //set icon
    QStyle *style = QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
    QIcon tmpIcon;
    switch(icon) 
    {
    case AuMessageBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, this);
        break;
    case AuMessageBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, this);
        break;
    case AuMessageBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, this);
        break;
    case AuMessageBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, this);
    default:
        break;
    }
    if(!tmpIcon.isNull())
    {   
        iconLabel->setPixmap(tmpIcon.pixmap(iconSize, iconSize));
        m_titleBar->setTitleIcon(tmpIcon);
    }
}

AuMessageBox::~AuMessageBox()
{
}

//paint event for title bar main wiget
void AuMessageBox::paintEvent(QPaintEvent* pe)
{
    QDialog::paintEvent(pe);

    QPainter painter(this); 

    // Draw border
	QRect outLine(0,0,width()-1,height()-1);
	painter.setPen(QPen(QColor(93,135,221,255), 3));
	painter.drawRect(outLine);      

    painter.end();
}
