/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		blendstate.h
	Author:		Eric Bryant

	Sets the renderer into alpha blending mode.
*/

#ifndef _BLENDSTATE_H
#define _BLENDSTATE_H

namespace Katana
{

///
/// BlendOp
/// Blending operation which current corresponding to D3DBLENDOP.
///
enum BlendOp
{
	BLEND_ZERO				= 1,
    BLEND_ONE				= 2,
    BLEND_SRCCOLOR			= 3,
    BLEND_INVSRCCOLOR		= 4,
    BLEND_SRCALPHA			= 5,
    BLEND_INVSRCALPHA		= 6,
    BLEND_DESTALPHA			= 7,
    BLEND_INVDESTALPHA		= 8,
    BLEND_DESTCOLOR			= 9,
    BLEND_INVDESTCOLOR		= 10,
    BLEND_SRCALPHASAT		= 11,
    BLEND_BOTHSRCALPHA		= 12,
    BLEND_BOTHINVSRCALPHA	= 13,
};


///
/// BlendState
/// Use this RenderState to put the renderer into alpha blending mode
///
class BlendState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( BlendState );
	KDECLARE_SCRIPT;

public:

	/// Constructor
	BlendState();

	/// Constructor which can disable blending
	BlendState( bool enable );

	/// Constructor which take default blending operations
	BlendState( BlendOp sourceOp, BlendOp destOp );

	/// Sets whether blending is enabled
	void setEnableBlend( bool enable )						{ m_enableBlending = enable; }

	/// Sets the source operation
	void setSourceOperation( BlendOp sourceOp )				{ m_sourceOp = sourceOp; }

	/// Sets the destination operation
	void setDestinationOperation( BlendOp destOp )			{ m_destOp = destOp; }

	/// Gets whether blending is enabled
	bool getEnableBlend() const								{ return m_enableBlending; }

	/// Get the source operation
	BlendOp getSourceOperation() const						{ return m_sourceOp; }

	/// Gets the destination operation
	BlendOp getDestinationOperation() const					{ return m_destOp; }

private:
	/// Is blending enabled
	bool	m_enableBlending;

	/// Source operation
	BlendOp	m_sourceOp;

	/// Destination operation
	BlendOp m_destOp;
};

KIMPLEMENT_STREAM( BlendState );
KIMPLEMENT_SCRIPT( BlendState );

} // Katana

#endif // _BLENDSTATE_H