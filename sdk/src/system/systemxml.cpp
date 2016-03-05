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

#include <list>
#include <istream>
#include <fstream>
#include <boost/utility.hpp>

#include "katana_core_includes.h"
#include "systemxml.h"
#include "base/kstring.h"

// --------------------------------------------------------
// Helper Functions
// --------------------------------------------------------

void eat_whitespace( char & c, std::istream & in )
{
	while ( c == ' ' || c == '\r' || c == '\n' || c == '\t' )
		in.get( c );
}

std::string get_name( char & c, std::istream & in )
{
	std::string result;
	eat_whitespace( c, in );
	while ( std::strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-.", c ) != 0 )
	{
		result += c;
		in.get( c );
	}
	return result;
}

void eat_delim( char & c, std::istream & in, char delim )
{
	eat_whitespace( c, in );
	if ( c != delim )
	{
		return;	// ERROR
	}

	in.get( c );
}

std::string get_value( char & c, std::istream & in )
{
	std::string result;
	while ( c != '\"' )
	{
		result += c;
		in.get( c );
	}
	if ( in.peek() != '\"' )
		in.get( c );

	return result;
}

bool compareStrings( const string & a, const string & b )
{
	return ( stricmp( a.c_str(), b.c_str() ) == 0 );
}

// --------------------------------------------------------
// _XML_Element/_XML_Attribute
// --------------------------------------------------------

//
// _XML_Attribute
//
struct _XML_Attribute
{
    std::string name;
    std::string value;

	_XML_Attribute(){}
	_XML_Attribute( const std::string & name, const std::string & value )
		: name(name), value(value) {}
};

//
// Typedefs
//
typedef boost::shared_ptr< _XML_Element_Ptr >	element_ptr;
typedef std::list< _XML_Element_Ptr  >			element_list;
typedef std::list< _XML_Attribute >				attribute_list;

//
// _XML_Element
//
class _XML_Element
    : boost::noncopyable  // because deep copy sematics would be required
{
public:
	std::string     name;
	attribute_list  attributes;
	element_list    elements;
	std::string     content;

    _XML_Element() {}
    explicit _XML_Element( const std::string & name ) : name(name) {}

	static _XML_Element_Ptr	parse( std::istream & in );
	static void				write( const _XML_Element & e, std::ostream & ostr );
};

//
// _XML_Element::parse
//
_XML_Element_Ptr _XML_Element::parse( std::istream & in )
{
	char c = 0;  // current character
	_XML_Element_Ptr e( new _XML_Element );

	in.get( c );
	if ( c == 0 ) return NULL;
	if ( c == '<' ) in.get( c );

	e->name = get_name( c, in );
	eat_whitespace( c, in );

	// attributes
	while ( c != '>' )
	{
		_XML_Attribute a;
		a.name = get_name( c, in );
		a.value = get_value( c, in );

        eat_delim( c, in, '=' );
        eat_delim( c, in, '\"' );

        a.value = get_value( c, in );

		e->attributes.push_back( a );
		eat_whitespace( c, in );
	}

	in.get( c ); // next after '>'
	eat_whitespace( c, in );

	// sub-elements
	while ( c == '<' )
	{
		if ( in.peek() == '/' ) break;
		e->elements.push_back( parse( in ) );
		in.get( c ); // next after '>'
		eat_whitespace( c, in );
	}

	// content
	if ( c != '<' )
	{
		while ( c != '<' )
		{
			e->content += c;
			in.get( c );
		}
	}

	in.get( c ); // next after '<'

	eat_delim( c, in, '/' );
	std::string end_name( get_name( c, in ) ); 
	if ( !compareStrings( e->name, end_name ) )
	{
		return NULL;
	}

	eat_delim( c, in, '>' );
	return e;
}

//
// _XML_Element::write
//
void _XML_Element::write( const _XML_Element & e, std::ostream & ostr )
{
	ostr << "<" << e.name.c_str();

	if ( !e.attributes.empty() )
	{
		for( attribute_list::const_iterator itr = e.attributes.begin();
				itr != e.attributes.end(); ++itr )
		{
			ostr << " " << itr->name.c_str() << "=\"" << itr->value.c_str() << "\"";
		}
	}

	ostr << ">";
	if ( !e.elements.empty() )
	{
		ostr << "\n";
		for( element_list::const_iterator itr = e.elements.begin();
				itr != e.elements.end(); ++itr )
		{
			write( **itr, ostr );
		}
	}

	if ( !e.content.empty() )
	{
		ostr << e.content.c_str();
	}

	ostr << "</" << e.name.c_str() << ">\n";
}

// --------------------------------------------------------
// XML_Node
// --------------------------------------------------------

//
// XML_Node::Constructor
//
XML_Node::XML_Node( const char * szNodeID )
{
	m_pElement = new _XML_Element;
	m_pElement->name = szNodeID;
}

XML_Node::XML_Node( XML_Node & parent, const char * szNodeID )
{
	m_pElement = new _XML_Element;
	m_pElement->name = szNodeID;

	parent.m_pElement->elements.push_back( m_pElement );
}

//
// XML_Node::getNode
//
XML_Node XML_Node::getNode( const char * szNodeID ) const
{
	if ( m_pElement )
	{
		if ( compareStrings( m_pElement->name, szNodeID ) )
			return XML_Node( m_pElement );

		for( element_list::iterator iter = m_pElement->elements.begin();
			iter != m_pElement->elements.end();
			iter++ )
		{
			XML_Node node( (*iter) );
			if ( compareStrings( (*iter)->name, szNodeID ) )
				return node;
			else
				node.getNode( szNodeID );
		}
	}

	return XML_Node();
}

//
// XML_Node::getAttributeString
//
string XML_Node::getAttributeString( const char * szAttribName ) const
{
	if ( m_pElement )
	{
		for( attribute_list::iterator iter = m_pElement->attributes.begin();
			iter != m_pElement->attributes.end();
			iter++ )
		{
			if ( compareStrings( (*iter).name, szAttribName ) )
				return (*iter).value.c_str();
		}
	}

	return string();
}

//
// XML_Node::getAttributeInteger
//
int XML_Node::getAttributeInteger( const char * szAttribName ) const
{
	if ( m_pElement )
	{
		for( attribute_list::iterator iter = m_pElement->attributes.begin();
			iter != m_pElement->attributes.end();
			iter++ )
		{
			if ( compareStrings( (*iter).name, szAttribName ) )
				return atoi( (*iter).value.c_str() );
		}
	}

	return 0;
}

//
// XML_Node::getAttributeFloat
//
float XML_Node::getAttributeFloat( const char * szAttribName ) const
{
	if ( m_pElement )
	{
		for( attribute_list::iterator iter = m_pElement->attributes.begin();
			iter != m_pElement->attributes.end();
			iter++ )
		{
			if ( compareStrings( (*iter).name, szAttribName ) )
				return (float)atof( (*iter).value.c_str() );
		}
	}

	return 0.f;
}

//
// XML_Node::getAttributeBoolean
//
bool XML_Node::getAttributeBoolean( const char * szAttribName ) const
{
	if ( m_pElement )
	{
		for( attribute_list::iterator iter = m_pElement->attributes.begin();
			iter != m_pElement->attributes.end();
			iter++ )
		{
			if ( compareStrings( (*iter).name, szAttribName ) )
				return ( compareStrings( (*iter).value, "true" ) );

		}
	}

	return false;
}

//
// XML_Node::addAttribute
//
bool XML_Node::addAttribute( const string & key, const string & value )
{
	if ( !m_pElement ) return false;
	
	m_pElement->attributes.push_back( _XML_Attribute( key, value ) );

	return true;
}

bool XML_Node::addAttribute( const string & key, long value )
{
	if ( !m_pElement ) return false;
	
	m_pElement->attributes.push_back( _XML_Attribute( key, kstring( value ).c_str() ) );

	return true;
}

bool XML_Node::addAttribute( const string & key, float value )
{
	if ( !m_pElement ) return false;

	m_pElement->attributes.push_back( _XML_Attribute( key, kstring( value ).c_str() ) );

	return true;
}

bool XML_Node::addAttribute( const string & key, bool value )
{
	if ( !m_pElement ) return false;

	m_pElement->attributes.push_back( _XML_Attribute( key, value ? "true" : "false" ) );

	return true;
}

//
// XML_Node::getValue
//
string XML_Node::getValue() const
{
	if ( m_pElement )
	{
		return m_pElement->content.c_str();
	}

	return string();
}

//
// XML_Node::setValue
// 
bool XML_Node::setValue( const string & value )
{
	if ( !m_pElement ) return false;

	m_pElement->content = value;

	return true;
}

//
// XML_Node::_getElement
//
_XML_Element * XML_Node::_getElement()
{
	return m_pElement;
}

// --------------------------------------------------------
// SystemXML
// --------------------------------------------------------

//
// Constructor
//
SystemXML::SystemXML( XML_Node & node ) :
	m_pRootElement( node._getElement() )
{
}

SystemXML::SystemXML( const char * szXMLFile ) :
	m_pRootElement( NULL )
{
	parseXML( szXMLFile );
}

SystemXML::~SystemXML()
{
	delete m_pRootElement;
}

//
// SystemXML::parseXML
//
bool SystemXML::parseXML( const char * szXMLFile )
{
	// Delete the existing root before we parse it
	if ( m_pRootElement ) delete m_pRootElement;

	// Load the file using stream
	std::ifstream inputFile( szXMLFile );
	if ( !inputFile.good() ) return false;

	// Parse the elements of the file and place it in our tree
	m_pRootElement = _XML_Element::parse( inputFile );
	if ( m_pRootElement == NULL )
		return false;

	return true;
}

//
// SystemXML::saveXML
//
bool SystemXML::saveXML( const char * szXMLFile )
{
	// Save the output XML file
	std::ofstream outputFile( szXMLFile );

	// Write the root element to disk
	_XML_Element::write( *m_pRootElement, outputFile );

	return true;
}

//
// SystemXML::getNode
//
XML_Node SystemXML::getNode( const char * szNodeID )
{
	// Constructor a XML_Node from the root XML element
	XML_Node rootNode( m_pRootElement );

	// Search for and return the node
	return rootNode.getNode( szNodeID );
}