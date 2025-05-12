#pragma once
#include <string>

struct UrlEncoding
{
private:
	unsigned char toHex(unsigned char x);
	unsigned char fromHex(unsigned char x);
public:
	UrlEncoding(void);
	~UrlEncoding(void);
	std::string toUrl(const std::string &str);
	std::string toText(const std::string &str);
};