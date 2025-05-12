/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  parsemsg.h
//
#pragma once
#include <stdint.h>
#include <string>

class BufferReader
{
private:
	uint8_t *m_pBuf;
	size_t m_iSize;
	size_t m_iRead;
	bool m_bBad;
public:
	BufferReader(const char *buf, size_t size) :
		m_pBuf((uint8_t *)buf), m_iSize(size), m_iRead(0), m_bBad(false)
	{
	}
	~BufferReader() {};
	template<typename T> T read(void);
	std::string readString(void);
};

template<typename T>
T BufferReader::read(void)
{
	if (m_bBad)
		return -1;
	else if (m_iRead + sizeof(T) > m_iSize)
	{
		m_bBad = true;
		return -1;
	}
	else if (sizeof(T) == 1)
		return m_pBuf[m_iRead++];
	T t = *(T *)(m_pBuf + m_iRead);
	m_iRead += sizeof(T);
	return t;
}

std::string BufferReader::readString(void)
{
	if (m_bBad)
		return "";
	std::string string;
	uint8_t ch;
	while (m_iRead < m_iSize && (ch = read<uint8_t>()) != 0)
		string += ch;
	return string;
}