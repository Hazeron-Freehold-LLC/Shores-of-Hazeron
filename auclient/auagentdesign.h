//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentdesign.h,v 1.9 2022/02/19 17:49:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentDesign_H
#define AuAgentDesign_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class AUCLIENT_DLL AuAgentDesign : public QWidget
{
	Q_OBJECT;
private:
	// Widgets

public:
	AuAgentDesign(QWidget* parent=0);
	virtual ~AuAgentDesign();

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void EnterDesigner();
	void LoadBdDesign();
	void LoadScDesign();
};

#endif // AuAgentDesign_H
