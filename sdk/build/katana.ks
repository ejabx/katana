-- Katana Engine Global Constants

-- Device Constants (for Input:EnableDevice/Input:DisableDevice)
INPUT_KEYBOARD	= 1;
INPUT_MOUSE		= 2;
INPUT_JOYSTICK	= 4;

-- Controller Constants (for Input:AddController)
KEYBOARD_CONTROLLER = 1;
MOUSE_CONTROLLER	= 2;
JOYSTICK_CONTROLLER = 3;
SCRIPT_CONTROLLER	= 4;

-- Primitive Types (for Geometry.primitive_type)
POINTS			= 1;
LINES			= 2;
LINESTRIP		= 3;
TRIANGLE_LIST	= 4;
TRIANGLE_STRIP	= 5;
TRIANGLE_FAN	= 6;

-- Buffer Types (for Geometry.enable_buffers)
VERTEX			= 1;
COLOR			= 2;
INDEX			= 4;
NORMALS			= 8;
TEXTURE_0		= 16;
TEXTURE_1		= 32;
TEXTURE_2		= 64;
TEXTURE_3		= 128;
TANGENT_S		= 256;
TANGENT_T		= 512;
TANGENT_ST		= 1024;

-- Blending Types (for Node:AddTextureState)
BLEND_NONE		= 0;
BLEND_REPLACE	= 1;
BLEND_MODULATE	= 2;
BLEND_DECAL		= 3;
BLEND_ADD		= 4;
BLEND_ADD_SIGNED= 5;
BLEND_DETAIL	= 6;
BLEND_ALPHA		= 7;

-----------------------------------------------

object = "blob"
shader = "shadow";
texture_name = "rock";
total_objects = 1;
total_lights = 3;
distance_between_objects = 300;

local export_node = ObjectFactory:Load( "../data/" .. object .. ".kf" );
local export_mesh = export_node:GetChild( 0 ):GetChild( 0 );

local root_node = ObjectFactory:Create( "VisNode" );
local parent_node = ObjectFactory:Create( "VisNode" );

local mesh_node = ObjectFactory:Create( "VisNode" );
parent_node:AddChild( mesh_node );
root_node:AddChild( parent_node );

for i = 0, total_objects - 1 do
	for j = 0, total_objects - 1 do
		mesh = ObjectFactory:Create( "VisMesh" );
		mesh:SetGeometry( export_mesh:GetGeometry() );
		
		x = i - ((total_objects - 1)/2);
		y = j - ((total_objects - 1)/2);
		mesh:SetTranslation( x * distance_between_objects, 0, y * distance_between_objects );
		mesh:SetRotation( 180, { 1, 0, 0 } );
		mesh_node:AddChild( mesh );
	end
end

vislight = {};
for l = 0, total_lights - 1 do
	local lightcolor = { 0.2, 0.2, 0.2, 1 };
	lightcolor[l+1] = 0.4;
	
	local light = ObjectFactory:Create( "Light" );	
	light.diffuse = lightcolor;	
	light.range = 100;
	vislight[l] = ObjectFactory:Create( "VisLight" );
	vislight[l]:SetLight( light );
	root_node:AddChild( vislight[l] );
end

--Debug:ShowNormals( true );
--Debug:ShowLights( true );
--Debug:ShowTangents( true );

--parent:AddShaderState( "../data/diffuse_vs.cg", "", "../data/cliff.tga" );
root_node:AddShaderState( "../data/" .. shader .. "_vs.cg", "../data/" .. shader .. "_ps.cg", 
						  "../data/" .. texture_name .. ".dds", "../data/" .. texture_name .. "_bump.dds", 
					      "", "../data/illumination.tga" );

Scene:AddController( MOUSE_CONTROLLER, parent_node, 0.5, 2 );
Scene:AddController( KEYBOARD_CONTROLLER );
Input:DisableDevice( INPUT_JOYSTICK );
Scene:AddController( SCRIPT_CONTROLLER, "OnTick" );

-- Place this at the end so the intermediate objects will
-- receive the OnAttach() event.
Scene:AddNode( root_node );

-- Sample function to toggle the light back and forth
positionx = -500;
direction = 1000;
rotation = 0;

local default_light = ObjectFactory:Create( "Light" );
default_light.diffuse = { 1, 1, 1, 1 };	
default_light.range = 100;	
default_vislight = ObjectFactory:Create( "VisLight" );
--default_vislight:SetLight( default_light );
--root_node:AddChild( default_vislight );
root_node:SetTranslation( 0, 0, -500 );

function OnTick( deltaTime )
	if ( deltaTime ~= 0 ) then
		positionx = positionx + (deltaTime * direction);
		if ( positionx <= -500 or positionx >= 500 ) then
			direction = -direction; 
			if ( positionx <= -500 ) then positionx = -500 end;
			if ( positionx >=  500 ) then positionx =  500 end;
		end;
		
--		vislight[0]:SetTranslation( -positionx, positionx, -500 );
--		vislight[1]:SetTranslation( positionx, 0, -500 );
--		vislight[2]:SetTranslation( -positionx, 0, -500 );
		
		rotation = rotation + (deltaTime * 100);
		if ( rotation >= 360 ) then rotation = 0; end;
		mesh_node:SetRotation( rotation, { 0, 1, 0 } );
	end
end