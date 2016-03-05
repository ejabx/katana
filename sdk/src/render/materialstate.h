/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		materialstate.h
	Author:		Eric Bryant

	Sets the renderer into a new material mode
*/

#ifndef _MATERIALSTATE_H
#define _MATERIALSTATE_H

namespace Katana
{

// Forward declarations
struct Material;

///
/// MaterialState 
///
class MaterialState 
	: public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( MaterialState );

public:

	/// Constructor
	MaterialState();

	/// Constructor which takes a material
	MaterialState( shared_ptr<Material> material );

	const shared_ptr<Material> getMaterial() const;

private:
	shared_ptr<Material>	m_material;
};

KIMPLEMENT_STREAM( MaterialState );

//
// MaterialState::Constructor
//
inline MaterialState::MaterialState()
{
}

inline MaterialState::MaterialState( shared_ptr<Material> material )
	: m_material( material )
{
}

//
// MaterialState::getMaterial
//
inline const shared_ptr<Material> MaterialState::getMaterial() const
{
	return m_material;
}

}; // Katana

#endif // _MATERIALSTATE_H