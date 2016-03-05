/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemxml.h
	Author:		Eric Bryant
	Credits:	Beman Dawes (C) 2002

	Class which loads and saves a simple xml file.
	The original code was found in the boost library:
	http://www.boost.org
	as tinyxml.
*/

#ifndef _SYSTEMXML_H
#define _SYSTEMXML_H

//
// Forward Declarations
//
class _XML_Element;
typedef _XML_Element * _XML_Element_Ptr;

namespace Katana
{

///
/// XML_Node
///
class XML_Node
{
public:
	/// Constructs a blank node
	XML_Node() : m_pElement( NULL ) {}

	/// Constructs a node and associates an _XML_Element with it
	XML_Node( _XML_Element * element ) : m_pElement( element ) {}

	/// Constructs a __XML_Element with the given name
	XML_Node( const char * szNodeID );

	/// Constructs a  __XML_Element with the given name and parented to parent
	XML_Node( XML_Node & parent, const char * szNodeID );

	/// Retrieves a child node
	XML_Node getNode( const char * szNodeID ) const;

	/// Searchs for a retrieves the value of an attribute associated with this node
	string	getAttributeString( const char * szAttribName ) const;
	int		getAttributeInteger( const char * szAttribName ) const;
	float	getAttributeFloat( const char * szAttribName ) const;
	bool	getAttributeBoolean( const char * szAttribName ) const;

	/// Associates an attribute (key/value pair) with this node
	bool addAttribute( const string & key, const string & value );
	bool addAttribute( const string & key, long value );
	bool addAttribute( const string & key, float value );
	bool addAttribute( const string & key, bool value );

	/// Get the value associated with this node
	string getValue() const;

	/// Sets the value associated with thsi node
	bool setValue( const string & value );

	/// Private method to return the internal element
	_XML_Element * _getElement();

private:
	/// Internal XML tree position
	_XML_Element * m_pElement;
};


///
/// SystemXML
///
class SystemXML
{
public:
	/// Constructs a System XML given a root XML_Node
	SystemXML( XML_Node & node );

	/// Consturcts a XML Tree given a XML file name
	SystemXML( const char * szXMLFile );

	/// Destructor
	~SystemXML();

	/// Parses the XML file and stores the tree representation internally
	bool parseXML( const char * szXMLFile );

	/// Saves the  XML file to disk assuming we have a tree
	bool saveXML( const char * szXMLFile );

	/// Check whether the XML file is valid
	bool isValid() const										{ return ( m_pRootElement ? true : false ); }

	/// Retrieves a node from the XML tree
	XML_Node getNode( const char * szNodeID );

private:
	_XML_Element *	m_pRootElement;		/// Pointer to the root tree representation of the XML data
};

}; // Katana

#endif // _SYSTEMXML_H