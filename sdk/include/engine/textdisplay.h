/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		textdisplay.h
	Author:		Eric Bryant

	System responsible for displaying text onto the screen. Generally 
	used for drawing debug text onto the screen (because of speed).
*/

#ifndef TEXTDISPLAY_H
#define TEXTDISPLAY_H

namespace Katana
{

// Forward declarations
class Texture;
class VertexBuffer;
class TextDisplay;
struct SceneContext;

///
/// TextDrawOperation
/// Structure which encapsulates everything to draw colored, scaled
/// text onto the screen at a specific position
///
struct TextDrawOperation
{
	TextDrawOperation();	/// Constructor
	unsigned int uiX;		/// X coordinate to draw the text (subject to the current resolution, in pixels)
	unsigned int uiY;		/// Y coordinate to draw the text (subject to the current resolution, in pixels)
	string	strText;		/// Text to draw onto the screen
	ColorA	colorTop;		/// Top gradient color for the text
	ColorA	colorBottom;	/// Bottom gradient color for the text
	float	fScale;			/// Scale to draw the text

private:
	float	fX;				/// X coordinate to draw the text (in screen space [0..1])
	float	fY;				/// Y coordinate to draw the text (in screen space [0..1])

	friend TextDisplay;
};

///
/// TextDisplay
///
class TextDisplay
{
public:

	/// Constructor
	TextDisplay();

	/// Sets the current font and widths used for drawing text
	void setFont( const string & fontTextureName, const string & fontWidthsFileName );
	void setFont( shared_ptr<Texture> fontTexture, vector<char> fontWidths );

	/// Draws text onto the screen. Note this function does not immediately draw the text,
	/// but places it within the text drawing queue for batch rendering.
	void drawText( unsigned int x, unsigned int y, const string & strText );

	/// Draws text onto the screen with the specified color. Note this function does not immediately draw the text,
	/// but places it within the text drawing queue for batch rendering.
	void drawText( unsigned int x, unsigned int y, const string & strText, const ColorA & color );

	/// Draws text onto the screen with a gradient from colorTop to colorBottom. Note this function does not immediately draw the text,
	/// but places it within the text drawing queue for batch rendering.
	void drawText( unsigned int x, unsigned int y, const string & strText, const ColorA & colorTop, const ColorA & colorBottom );

	/// Draws text onto the screen given a text draw operation
	void drawText( const TextDrawOperation & textDrawOp );

	/// Actually draws the text onto the screen. Clients should not execute this function, but infact the
	/// owner of this class (the GameEngine) should call this function all other objects are rendered
	void flushTextQueue( SceneContext * context );

protected:

	/// Representative of a vertex in the vertex buffer for a single character entry
	struct CharacterVertex { float x, y, z, rhw; unsigned long color; float tu, tv; };

	/// Function that fills the vertex buffer with the text operation results
	void fillTextVertexBuffer( ksafearray<CharacterVertex> & characterVertexData, const TextDrawOperation & textDrawOp );

protected:

	/// Vector of text draw operations, which are acted upon during flushTextQueue
	vector<TextDrawOperation>	m_textDrawOperations;

	/// Vertex buffer used to draw the text drawing operations
	shared_ptr<VertexBuffer>	m_textVertexBuffer;

	/// Texture which contains the font information
	shared_ptr<Texture>			m_fontTexture;

	/// Font widths. Because the font are generally proportial, this determines
	/// the font widths for each character in the font texture
	vector<char>				m_fontWidths;

	/// Running count of the number of character already drawn to the screen
	unsigned int				m_currentCharacterCount;

	/// This is the total number of character which can be rendering in one flushTextQueue
	/// operation. It determines the size of the m_textVertexBuffer. The default is 1024
	unsigned int				m_maximumCharacters;

	/// Height and width of a character glyph within the font texture (generally, height or width of texture / 16)
	unsigned int				m_cellHeight;
	unsigned int				m_cellWidth;
};

//
// Inline
//

//
// TextDrawOperation::constructor
//
inline TextDrawOperation::TextDrawOperation() 
	: uiX( 0 )
	, uiY( 0 )
	, colorTop( ColorA( 1, 1, 1, 1 ) )
	, colorBottom( ColorA( 1, 1, 1, 1 ) )
	, fScale( 1.f )
	, fX( -1.f )
	, fY( -1.f )
{
}

//
// TextDisplay::constructor
//
inline TextDisplay::TextDisplay()
	: m_maximumCharacters( 1024 )
	, m_cellWidth( 0 )
	, m_cellHeight( 0 )
{
}

//
// TextDisplay::drawText
//
inline void TextDisplay::drawText( unsigned int x, unsigned int y, const string & strText )
{
	TextDrawOperation textOp;
	textOp.uiX = x; 
	textOp.uiY = y; 
	textOp.strText = strText;
	textOp.colorTop = textOp.colorBottom = ColorA( 1, 1, 1, 1 );

	drawText( textOp );
}

inline void TextDisplay::drawText( unsigned int x, unsigned int y, const string & strText, const ColorA & color )
{
	TextDrawOperation textOp;
	textOp.uiX = x; 
	textOp.uiY = y; 
	textOp.strText = strText;
	textOp.colorTop = textOp.colorBottom = color;

	drawText( textOp );
}

inline void TextDisplay::drawText( unsigned int x, unsigned int y, const string & strText, const ColorA & colorTop, const ColorA & colorBottom )
{
	TextDrawOperation textOp;
	textOp.uiX = x; 
	textOp.uiY = y; 
	textOp.strText = strText;
	textOp.colorTop = colorTop;
	textOp.colorBottom = colorBottom;

	drawText( textOp );
}

}; // Katana

#endif // TEXTDISPLAY_H