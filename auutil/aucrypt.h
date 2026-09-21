//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucrypt.h,v 1.7 2022/04/08 19:15:14 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCrypt_H
#define AuCrypt_H

#include "auutil.h"
#include <QString>

class AUUTIL_DLL AuCrypt : public QByteArray
{
public:
	// Encryption/decryption of this byte array.
	void Crypt(short int wKey);

	void EncryptByteArray(const QByteArray& ba, short int wKey);

	// Encryption/decryption of the contents of pointers to/from this array.
	void EncryptBytes(const unsigned char *pByte, int lBytes, short int wKey);
	void DecryptBytes(unsigned char *pByte, int lBytes, short int wKey) const;

	// String encryption/decryption to/from this array.
	void EncryptString(const QString &sEncrypt, short int wKey);
	QString DecryptString(short int wKey) const;
	void EncryptLocalString(const QString &sEncrypt, short int wKey);	// Hostname included in encryption algorithm.
	QString DecryptLocalString(short int wKey) const;					// Returns QString() if hostname incorrect.

	// 'short int' encryption/decryption to/from this array.
	void EncryptShortInt(short int wEncrypt, short int wKey)
	{
		EncryptBytes((unsigned char *)&wEncrypt, sizeof(short int), wKey);
	}
	short int DecryptShortInt(short int wKey) const
	{
		short int wDecrypt;
		DecryptBytes((unsigned char *)&wDecrypt, sizeof(short int), wKey);
		return(wDecrypt);
	}

	// 'int' encryption/decryption to/from this array.
	void EncryptInt(int lEncrypt, short int wKey)
	{
		EncryptBytes((unsigned char *)&lEncrypt, sizeof(int), wKey);
	}
	int DecryptInt(short int wKey) const
	{
		int lDecrypt;
		DecryptBytes((unsigned char *)&lDecrypt, sizeof(int), wKey);
		return(lDecrypt);
	}

	// Check Sum
	unsigned int ByteSum() const;

	//QString ToASCII() const;			// Returns contents as a string of ASCII characters only. Very inefficient.
	//void FromASCII(const QString& s);	// Converts ToASCII formatted string s back into a byte array.
};

#endif // AuCrypt_H
