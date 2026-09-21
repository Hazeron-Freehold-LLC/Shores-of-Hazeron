//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsole.h,v 1.71 2022/04/14 16:41:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpConsole_H
#define DpConsole_H

#include "auatom.h"

class AuAtomRenderInfo;
class AuGLEffects;
class AuId;
class AuMatrix4x4;
class AuQuadArray;
class AuElapsedTimer;
class DaAtom;
class DaCreature;
class DaPerson;
class DaVessel;

class AUATOM_DLL DpConsole
{
protected:
	// OpenGL
	GLuint				m_glLowerConsole;	// OpenGL texture id.
	int					m_LastETESecs;		// Last ETE in seconds.

public:	// Constructors
	DpConsole();
	virtual ~DpConsole();
	DpConsole(const DpConsole &rhs);

public:
	// AnimateDeviceObjectsLower animates the secondary display, which is the same on all consoles of a given ship.
	void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const DaCreature& sceneAtom);

	// 'inBuilding' is true if this console is in a building, false if this console is in a spacecraft.
	void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	void DeleteGL();

	void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels) const;

	virtual quint64 MemoryUsage() const;

	void PanelDimensions(quint16 idPanel, double& width, double& height) const;

	// Renders only the command chair.
	void RenderChair(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari);

	// Renders the chair and the console screen.
	void RenderConsole(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari,
			GLuint texScreen) const;

public:
	// Adjusts static adjustments to the position of the console display screen.
	static void Raise(const AuId& who);					// Raise console display screen.
	static void Lower(const AuId& who);					// Lower console display screen.
	static void MoveFarther(const AuId& who);			// Move console display screen farther away from viewer.
	static void MoveCloser(const AuId& who);			// Move console display screen closer to viewer.
	static void RotateAway(const AuId& who);			// Tip console display screen away from viewer.
	static void RotateToward(const AuId& who);			// Tip console display screen toward viewer.
	static void RestoreAdjustments(const AuId& who);	// Restore console display screen adjustments.
};

#endif // DpConsole_H
