/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		terrainpatch.h
	Author:		Eric Bryant

	Terrain Patch.
*/

#ifndef _TERRAIN_PATCH_H
#define _TERRAIN_PATCH_H

namespace Katana
{
//
// Forward Declarations
//
class Heightfield;
class Terrain;

///
/// TerrainVertex
///
struct TerrainVertex
{
	Point3 position;
	Point2 texture;
};

///
/// TerrainPatch
///
class TerrainPatch : public Streamable
{
	KDECLARE_STREAM(TerrainPatch);

public:
	enum
	{
		VIS_BASE = 0,
		VIS_LAYER0 = 2,
		VIS_LAYER1 = 4,
		VIS_LAYER2 = 8,
		VIS_LAYER3 = 16,
		VIS_LAYER4 = 32,
		VIS_LAYERALL = 63,
	};
	enum
	{
		MAXIMUM_SUBDIVISION = 4,
		PATCH_VERTEX_WIDTH = 17,
		PATCH_VERTEX_HEIGHT = 17,
		MAXIMUM_VERTICES = PATCH_VERTEX_WIDTH * PATCH_VERTEX_HEIGHT,
		MAXIMUM_INDICES = MAXIMUM_VERTICES * 3,
	};
public: 
	/// Constructor
	TerrainPatch();

	/// Constructor which links to an existing terrain
	TerrainPatch( Terrain * parentTerrain, unsigned int indexX, unsigned int indexZ );

	/// Store the source heightmap with an offset
	void setHeightMap( Heightfield & heightmap, unsigned int posX, unsigned posZ );

	/// Store the scale of the patch in world coordinates
	void setScale( const Point3 & scale )												{ m_worldScale = scale; }

	/// Store the translation of the patch in world coordinates
	void setWorldTranslation( const Point3 & translation )								{ m_worldTranslation = translation; }

	/// Returns the translation of the patch to world coordinates
	Point3 getWorldTranslation() const													{ return m_worldTranslation; }

	/// Calculate the patch screen errors
	void calculateErrors();

	/// Calculate the minimum and maximum Y coordinates
	void calculateMinMaxY();

	/// Setup the neighboring relation between this patch
	void setNeighbors( TerrainPatch * left, TerrainPatch * right, TerrainPatch * bottom, TerrainPatch * top );

	/// Retrieve the tesselated vertices for this patch
	vector<TerrainVertex> & getPatchVertices()											{ return m_patchVertices; }

	/// Retrieve the indices for this patch
	vector<unsigned short>& getPatchIndices()											{ return m_patchIndices; }

	/// Returns whether the patch is visible
	bool isVisible() const																{ return m_visible; }

	/// Sets the new tesselation level for the next update
	void setTesselation( int tess )														{ m_newTesselation = tess; }

	/// Sets the new real tesselation level
	void setRealTesselation( int tess )													{ m_oldRealTesselation = m_realTesselation; m_newRealTesselation = tess; }

	/// Retrieve the current tesselation level
	int getTesselation() const															{ return m_tesselation; }

	/// Retrieve the new tesselation level
	int getNewTesselation() const														{ return m_newTesselation; }

	/// Retrieve the real tesselation level
	int getRealTesselation() const														{ return m_realTesselation; }

	/// Retrieve the new vertices required for tesselation (as a result of the updateTesselation)
	int getNewVertexCount() const														{ return m_newVertexCount; }

	/// Retrieve the new indices required for tesselation (as a result of the updateTesselation)
	int getNewIndexCount() const														{ return m_newIndexCount; }

	/// Retrieves the new tesselation level (taking into account the neighboring patch may be null by first
	/// checking this *this* pointer). This makes the following code valid: m_leftPatch->getNewTesselationSafe() even
	/// if m_leftPatch == NULL.
	int getNewTesselationSafe() const;

	/// Updates the tesselation by filling in the vertex and index buffers
	virtual bool updateTesselation();

	/// Patches may need additional update passes to adapt to neighbors. This is because our
	/// tesselation may depend on the tesselation of our neighboring patches.
	virtual bool updateTesselation2()													{ return false; }
	virtual bool updateTesselation3()													{ return false; }

	// Update the projected screen errors
	void updateProjectedErrors( const Matrix4 & screenProjection );
	
	// Retrieve the projected error for a given subdivision level
	float getProjectedError( int tess )													{ return m_errors[tess].difference; }

protected:

	/// Retrieves the vertex at the given position within the height map
	Point3 getVertex( int positionX, int positionZ );

	/// Calculates the error for the patch for the specified tesselation
	void calculateError( int tesselation );

	/// Creates the triangles for this patch based on this patch's tesselation and
	/// the tesselation level of our neighbors
	void createTesselation( int center, int left, int right, int top, int bottom );

private:

	/// Tesselation functions
	void reduceTesselation( int tesselation, TerrainVertex * pData );
	void reduceBorders( TerrainVertex * pData );
	void makeSimpler( int tesselation, TerrainVertex * pData );
	void calculateInbetweenVertex( int x, int y, int xc, int yc, int pow, TerrainVertex * pData);
	int	 calculateDiagonalVertices( int x, int y, int xc, int yc, int pow, TerrainVertex * pData, float& nHeightLeftTop, int& idxLT, float& nHeightRightBottom, int& idxRB );
	void addTriangleRow( int nZ, int nPow, bool nLeft, bool nRight, bool nBottom, bool nTop );
	void addTriangleBottomRow( int nPow, bool nLeft, bool nRight );
	void addTriangleTopRow( int nPow, bool nLeft, bool nRight );
	void addTrianglePatch( bool nLeft, bool nRight, bool nBottom, bool nTop );

	float getHeight(int nX, int nZ) const;
	void getVertex( int nX, int nZ, Point3 & point ) const;
	void getTexCoord( int nX, int nZ, Point2 & tcoord ) const;
	void getNormal(int nX, int nZ, Point3 & normal) const;
	unsigned short getIndex(int nX, int nZ);
	void addIndex( unsigned short nIdx );
	void addLastIndexAgain();

protected:
	struct Error
	{
		Error()	{ difference = 0.f; }
		Point3 correctVal;
		Point3 realVal;
		float  difference;
	};

protected:
	/// Parented terrain object
	Terrain *				m_parentTerrain;

	/// The position of the terrain page within the terrain "grid" of pages
	int						m_gridPositionX, m_gridPositionZ;

	/// The position in the heighmap to retrieve to heightmap data
	int						m_heightMapX, m_heightMapZ;

	/// The minimum and maximum Y values in the heightmap
	float					m_minHeightY, m_maxHeightY;

	/// Neighboring patch information
	TerrainPatch *			m_leftPatch, * m_rightPatch, * m_bottomPatch, * m_topPatch;

	/// Patch Tesselation Levels
	int						m_tesselation, m_newTesselation, m_realTesselation, m_newRealTesselation, m_oldRealTesselation;

	/// Neighboring Tesselation Levels
	int						m_leftTesslation, m_rightTesselation, m_topTesselation, m_bottomTesselation;

	/// Scaling from height map to world coordinates
	Point3					m_worldScale;

	/// Translation from height map to world coordinates
	Point3					m_worldTranslation;

	/// These are the errors for each tesselation level of this patch
	Error					m_errors[MAXIMUM_SUBDIVISION + 2];

	/// This is the current screen error for this patch
	float					m_currentError;

	/// Flags whether the patch is active for rendering
	bool					m_active;

	/// Flags whether ths patch is currently visible
	bool					m_visible;

	/// Has the vertex and index buffer been initialized?
	bool					m_initBuffers;

	/// Number of new vertices and indices to add to the patch
	int						m_newVertexCount, m_newIndexCount;

	/// Reference to the heightmap data
	shared_ptr<Heightfield>	m_heightmap;

	/// Array of terrain patch vertices
	vector<TerrainVertex>	m_patchVertices;

	/// Array of terrain patch indices and index mapping
	vector<unsigned short>	m_patchIndices;
	vector<unsigned short>	m_patchIndexMap;
};

KIMPLEMENT_STREAM( TerrainPatch );

} // Katana

#endif // _TERRAIN_PATCH_H