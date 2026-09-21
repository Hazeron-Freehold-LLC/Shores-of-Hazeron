//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglslerr.h,v 1.2 2022/02/19 18:29:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGLSLErr_H
#define AuGLSLErr_H

#include <QDialog>

class AuGLSLErr : public QDialog
{
	Q_OBJECT;
public:
	AuGLSLErr(const QString& title, const QString& errMsg, QWidget* parent=0);
	virtual ~AuGLSLErr();
};

#endif // AuGLSLErr_H
