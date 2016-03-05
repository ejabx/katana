/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kstring.cpp
	Author:		Eric Bryant

	Extends the stl string class
*/

#include <string>
#include <stdarg.h>
#include "kstring.h"
using namespace Katana;

// Static variables
unsigned int kstring::m_decimalPrecision = 7;

//
// Constructors
//

kstring::kstring()
{
}

kstring::kstring(const char * str)
{	
	if ( str ) 
	{
		assign(str);
	}
}

kstring::kstring(const char * str, int len)
{	
	if ( str ) 
	{
		assign(str, len);
	}
}

kstring::kstring(double number)
{
	set(number);
}

kstring::kstring(long number)
{
	set(number);
}

//
// Destructor
//
kstring::~kstring()
{
}

// 
// set
//
void kstring::set(const char * szStr)
{	
	if ( szStr ) 
	{
		assign(szStr);
	}
}

void kstring::set(const char * szStr, int nLength)
{	
	if ( szStr ) 
	{
		assign(szStr, nLength);
	}
}

void kstring::set(unsigned int uiIndex, const char ch)
{
	at(uiIndex) = ch;
}

void kstring::set(long l)
{
	char szBuffer[50];
	ltoa(l, szBuffer, 10);

	assign(szBuffer);
}

void kstring::set(double d)
{
	char szBuffer[50];
	_gcvt(d, m_decimalPrecision, szBuffer);	// Microsoft Specific

	assign(szBuffer);
}

void kstring::set(float f)
{
	char szBuffer[50];
	_gcvt(f, m_decimalPrecision, szBuffer);	// Microsoft Specific

	assign(szBuffer);
}

void kstring::set(const kstring & str)
{
	*this = str;
}

//
// append
//
void kstring::appendDouble(double d)
{
	char szBuffer[50];
	_gcvt(d, m_decimalPrecision, szBuffer);	// Microsoft Specific

	append(szBuffer);
}

void kstring::appendInt(int i)
{
	char szBuffer[50];
	_ltoa(i, szBuffer, 10);

	append(szBuffer);
}

//
// format
//
void kstring::format(char * szStr, ...)
{
	va_list l_va;
	char szOutput[2048];	// HACK: Ok, this number is hard coded

	try
	{
		va_start(l_va, szStr);
		vsprintf(szOutput, szStr, l_va);
		va_end(l_va);
	}
	catch(...)
	{
		return;	// THROW
	}

	assign(szOutput);
}

//
// trim
//
void kstring::trim()
{
	// WHITESPACE: spaces/tabs at the beginning or end of the string

	// Get the first index of the first non-whitespace character
	size_t start_index = 0, end_index = 0;
	std::string local_string(c_str());
	const char * localstring = c_str();

	bool found_non_whitespace_character = false;

	// Find first non-whitespace character
	while ( !found_non_whitespace_character )
	{
		if ( ( *localstring == '\t' ) ||
		     ( *localstring == ' ' ) )
		{
			localstring++;
			start_index++;
		}
		else
		{
			found_non_whitespace_character = true;
			end_index = start_index + 1;
		}
	}

	// Find the last non-whitespace character
	end_index = size() - 1;

	while ( ( *(localstring + end_index) == '\t' ) ||
		    ( *(localstring + end_index) == ' ' ) ||
			( *(localstring + end_index) == '\n' ) )
	{
		end_index--;
	}

	if ( found_non_whitespace_character )
	{
		// There wasn't any whitespace in the string, do nothing
		if ( start_index == 0 && end_index == ( size() - 1 ) )
			return;

		// Reassign the new string
		assign(localstring, end_index);
	}
}

//
// strip
//
bool kstring::strip(const char * szToken, bool bStripBefore)
{
	size_t nStartPosition = find(szToken);
	if ( nStartPosition < 0 )
		return false;

	if ( bStripBefore )
		assign( substr(0, nStartPosition) );
	else
		assign( substr(nStartPosition + 1, size()) );

	return true;
}

bool kstring::strip(const char * szToken, kstring & str, bool bStripBefore)
{
	size_t nStartPosition = find(szToken);
	if ( nStartPosition < 0 )
		return false;

	if ( bStripBefore )
		assign( substr(0, nStartPosition) );
	else
		assign( substr(nStartPosition + 1, size()) );

	return true;
}

//
// getNumericToken
//

float kstring::getNumericToken(const char * szToken) const
{
	std::string localstring(*this);
	size_t nStartPosition = localstring.find(szToken);
	if ( nStartPosition >= 0 )
	{
		size_t nEndPosition = localstring.find_first_of(",)", nStartPosition + 1);
		if ( nEndPosition >= 0)
		{
			localstring = localstring.substr(nStartPosition + 2, nEndPosition - nStartPosition - 2);
			return (float)atof(localstring.c_str());
		}
	}

	return 0;
}

//
// getNumericToken
//
bool kstring::getNumericToken(const char * szToken, int nIndex, float & fValue) const
{
	std::string localstring(*this);

	size_t nStartPosition = localstring.find_first_of(szToken);
	if ( nStartPosition < 0 )
		return false;

	localstring = localstring.substr(nStartPosition + 1, localstring.size()); 
	nStartPosition = localstring.find_first_of(",");

	while ( nStartPosition >= 0)
	{
		if ( nIndex-- == 0 )
			break;

		localstring = localstring.substr(nStartPosition + 1, localstring.size()); 
		nStartPosition = localstring.find(",");
	}

	fValue = (float)atof(localstring.c_str());

	return true;
}

//
// Casts
//

kstring::operator const char*() const
{	
	return c_str();
}

kstring::operator double() const
{	
	// Find the first non-alphabetic character
	const char * szFirstNumber = c_str();

	while ( 0 != isalpha(*szFirstNumber) )
		szFirstNumber++;

	return atof(szFirstNumber);	// Remember, a float/double has the same size
}

kstring::operator long() const
{	
	// Find the first non-alphabetic character
	const char * szFirstNumber = c_str();

	while ( 0 != isalpha(*szFirstNumber) )
		szFirstNumber++;

	return atol(szFirstNumber);
}

//
// operator+
// Concatenates two strings and returns a kstring
//
kstring operator+(const kstring & s1, const char * pS2 )
{
	kstring temp( s1 );
	temp.append( pS2 );

	return temp;
}