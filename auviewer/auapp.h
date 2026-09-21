//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auapp.h,v 1.6 2022/02/19 18:42:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuApp_H
#define AuApp_H

#include <QApplication>

class Au3dsGLSLModelCache;
class AuEther;
class AuFrame;
class AuGLModelCache;
class DgData;

class AuApp : public QApplication
{
	Q_OBJECT;

private:
	DgData*								m_Data;
	AuEther*							m_Etherium;				// The global etherium.
	AuFrame*							m_Frame;				// main frame widget
	AuGLModelCache*						m_ModelCache;

public:
	AuApp(int& argc, char** argv);
	~AuApp();

	inline AuFrame* Frame()
	{
		return m_Frame;
	}

	void HandleSignal(int value);   // Called to handle a signal caught in main.cpp.

	inline void SetFrame(AuFrame* frame)
	{
		Q_ASSERT(frame);
		m_Frame = frame;
	}

	void SetStyle();//set the Hazeron black style
};

#define auApp	((AuApp*)qApp)

#endif // AuApp_H
