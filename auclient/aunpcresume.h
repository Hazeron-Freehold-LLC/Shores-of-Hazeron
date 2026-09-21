//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunpcresume.h,v 1.3 2022/02/19 17:59:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuNpcResume_H
#define AuNpcResume_H

#include "audialog.h"

class DaSentient;

class AuNpcResume : public AuDialog
{
	Q_OBJECT;
public:
	AuNpcResume(const DaSentient& sent, QWidget* parent=0);
	virtual ~AuNpcResume();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	virtual QSize sizeHint() const;
};

#endif // AuNpcResume_H
