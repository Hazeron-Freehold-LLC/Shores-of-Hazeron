//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailclientapp.h,v 1.5 2022/02/19 18:37:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUMAILCLIENTAPP_H
#define AUMAILCLIENTAPP_H

#include <QApplication>

class DgData;

class AuMailClientApp : public QApplication
{
	Q_OBJECT;
public:
	AuMailClientApp(int& argc, char** argv);
	~AuMailClientApp();
	
private:
	DgData*			m_Data;
};

#define auMailClientApp	((AuMailClientApp*)qApp)

#endif // AUMAILCLIENTAPP_H
