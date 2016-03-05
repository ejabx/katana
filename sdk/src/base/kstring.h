/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kstring.h
	Author:		Eric Bryant

	Extends the stl string class
*/

#ifndef _KSTRING_H
#define _KSTRING_H

#include <string>

namespace Katana
{

///
/// kstring
/// Extends the stl string class
///

class kstring : public std::string
{
public:
	/// Default Constructor
	kstring();

	/// Standard String Constructor with length
	kstring(const char * str);

	/// Standard String Constructor with length
	kstring(const char * str, int len);

	/// Standard String Constructor for double numbers
	kstring(double number);

	/// Standard String Constructor for integers
	kstring(long number);

	/// Destructor
	~kstring();

	/// Set contents of the string
	void set(const char *);
	void set(long l);
	void set(double d);
	void set(float f);
	void set(const char *, int);
	void set(unsigned int uiIndex, const char ch);
	void set(const kstring & str);

	void appendDouble(double d);				/// Append Double
	void appendInt(int i);					/// Append Integer

	/// Sprintf functionality
	void format(char *, ...);

	/// Left and Right trim white spaces from string
	void trim();

	/// Retrives a numeric token from the string
	float getNumericToken(const char *) const;

	/// Retrieves a numeric token from the string starting at an index
	bool getNumericToken(const char * szToken, int nIndex, float & fValue) const;

	/// Strips the characters before or aften the taken
	bool strip(const char * szToken, bool bStripBefore=true);

	/// Strips the characters before or aften the taken
	bool strip(const char * szToken, kstring & str, bool bStripBefore=true);

	// Operator overloads
	operator const char*() const;		/// Cast a standard string
	operator double() const;			/// Cast as double
	operator long() const;				/// Cast as long

	/// Sets the Precision for Decimal converions
	static void setPrecision(unsigned int decimals)			{ m_decimalPrecision = decimals; }

	/// Return the Precision
	static unsigned getPrecision()							{ return m_decimalPrecision; }

	/// Checks whether the pointer is valid
	bool isValid() const									{ return (*c_str() != NULL) ? true : false; }

	// Concatenates two strings and returns a kstring
	friend kstring operator+(const kstring &, const char *);

private:
	/// Floating point precision used when converting numbers
	static unsigned int	m_decimalPrecision;
};

} // Katana

#endif // _KSTRING_H