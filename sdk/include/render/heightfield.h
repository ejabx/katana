/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		heightfield.h
	Author:		Eric Bryant

	Represents heightmap data
*/

#ifndef _HEIGHTFIELD_H
#define _HEIGHTFIELD_H

namespace Katana
{

///
/// Heightfield
/// A 16-bit height map.
///
class Heightfield : public Geometry
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Heightfield);
	KDECLARE_SCRIPT;

	enum {	INVALID_HEIGHT = -1, INVALID_SLOPE = -1 };
	const static Point3 INVALID_VERTEX;
	const static Point3 INVALID_NORMAL;

public:
	/// Default Constructor
	Heightfield();

	/// Constructor which takes a height map and world dimensions
	Heightfield( const char * szHeightMapFile, int worldWidth, int worldHeight );

	/// Destructor
	virtual ~Heightfield();

	/// Loads height map data from a texture
	bool loadFromTexture( const char * szHeightMapFile );

	/// Checks whether the height field contains valid data
	bool isValid() const							{ return m_heightMapData != 0; }

	/// Return the width of the height field
	unsigned int getWidth() const					{ return m_dataWidth; }

	/// Return the height of the height field
	unsigned int getHeight() const					{ return m_dataHeight; }

	/// Return the maximum width of the height field (in world units)
	unsigned int getWorldWidth() const				{ return unsigned int( m_dataWidth * m_worldScaleWidth ); }

	/// Return the maximum height of the height field (in world units)
	unsigned int getWorldHeight() const				{ return unsigned int ( m_dataHeight * m_worldScaleHeight ); }

	/// Given a position within world space, return the corresponding height data. It will
	/// use linear interpolation to determine the Y position.
	unsigned short getHeightAt( unsigned int x, unsigned int z ) const;

	/// Given a position within world space, return the corresponding slope.
	float getSlopeAt( unsigned int x, unsigned int z ) const;

	/// Given a position within world space, return the corresponding vertex.
	Point3 getVertexAt( unsigned int x, unsigned int z ) const;

	/// Given a position within world space, return the cooresponing interpolated normal
	Point3 getNormalAt( unsigned int x, unsigned int z ) const;

	/// Convert the height map into triangles suitable for rendering. Because this derives
	/// from Geometry, you can render this height field by passing it to a VisMesh(Geometry *).
	/// Returns the number of triangles generated.
	int convertToTriangles( float heightScale = 1.0 );

private:
	/// Use bilinear interpolate to determine the height given (X,Y).
	/// The position passed in must be in texture space (limited by m_dataWidth, m_dataHeight)
	unsigned short bilinearInterpolateHeight( float x, float z ) const;

	/// Computes the normal information from the height map data
	void computeNormals();

private:
	unsigned short *	m_heightMapData;		/// Raw height map information
	Point3 *			m_heightmapNormals;		/// Normal information
	unsigned int		m_dataWidth;			/// Width (in pixels) of the height map information
	unsigned int		m_dataHeight;			/// Height (in pixels) of the height map information
	float				m_worldScaleWidth;		/// Factor to scale data width
	float				m_worldScaleHeight;		/// Factor to scale data height
};

KIMPLEMENT_STREAM( Heightfield );
KIMPLEMENT_SCRIPT( Heightfield );

} // Katana

#endif // _HEIGHTFIELD_H