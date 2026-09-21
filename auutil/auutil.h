//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auutil.h,v 1.5 2022/02/19 18:42:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuUtil_H
#define AuUtil_H

#include <QtCore>

/* Defines needed for dll linkage.
#if defined AUUTIL
#define AUUTIL_DLL Q_DECL_EXPORT
#else
#define AUUTIL_DLL Q_DECL_IMPORT
#endif
*/
#define AUUTIL_DLL	// Defined as nothing for static lib.

class QByteArray;
class QDataStream;
class QTcpSocket;

class AUUTIL_DLL AuUtil
{
public:
	AuUtil();

	static const char* CopyrightYear();

	static inline const QString LoginMasterName()
	{
		return QString("Login.Hazeron.net");
	}

	static inline quint16 LoginMasterPort()
	{
		quint16 portBase = ('A'<<8)|'u';	// 16757
		return portBase;
	}

	static const char* LoginVersion();

	static inline quint16 MailMasterPort()
	{
		return LoginMasterPort() + 1;		// 16758
	}

	static inline QString MailMasterName()
	{
		return QString("Mail.Hazeron.net");
	}

	// 'keyBase' != 0 is used to muddle the version-based key.
	// 'keyBase' == 0 returns the pure version-based key.
	// Never returns 0.
	static quint16 NetCryptKey(quint16 keyBase);

	// Returns the number of bytes that would be required to store 'pktSize'.
	// Returns a number from 1 to 4.
	// Asserts that pktSize is less than 0x40000000 (1,073,741,824). This imposes a maximum size limit of one gigabyte per packet.
	// Let us hope nobody tries to send one that big.
	inline static int PacketSizeBytes(quint32 pktSize)
	{
		if(pktSize < 0x40)
			return 1;
		else if(pktSize < 0x4000)
			return 2;
		else if(pktSize < 0x400000)
			return 3;
		else if(pktSize < 0x40000000)
			return 4;
		else
		{
			Q_ASSERT(false);
			return 4;
		}
	}

	// Reads 'pktSize' from 'socket'.
	// 'pktSize' and 'pktSizeBytesLeft' are input and output parameters.
	// When 'pktSizeBytesLeft' is 0 on return, the 'pktSize' value is valid.
	// This function reads as many bytes needed that are available in the 'socket'. More than one call to this function could be
	// required to read all the bytes of the packet size. As data is read from the socket, 'pktSize' is constructed. The values of
	// 'pktSize' and 'pktSizeBytesLeft' must initially be 0 and they need to be preserved between calls to this function.
	// Returns the number of packet size bytes read so far.
	static int ReadPacketSize(QTcpSocket& socket, quint32& pktSize, int& pktSizeBytesLeft);

	static int ReadPacketSize(QDataStream& in, quint32& pktSize, int& pktSizeBytesLeft);

	static inline QString SceneMasterName()
	{
		return QString("Scene.Hazeron.net");
	}

	static inline quint16 SceneMasterPort()
	{
		return LoginMasterPort() + 2;		// 16759
	}

	static const char* SceneVersion();

	static const char* Version();

	// Appends 1 to 4 bytes to the 'buffer', as necessary to express 'pktSize'.
	// Returns the number of bytes written.
	static int WritePacketSize(QByteArray& buffer, quint32 pktSize);

	// Writes 1 to 4 bytes into the 'stream', as necessary to express 'pktSize'.
	// Returns the number of bytes written.
	static int WritePacketSize(QDataStream& stream, quint32 pktSize);

	// Writes 1 to 4 bytes into the 'socket', as necessary to express 'pktSize'.
	// Returns the number of bytes written.
	static int WritePacketSize(QTcpSocket& socket, quint32 pktSize);

	static QString VersionFull();
};

#endif // AuUtil_H
