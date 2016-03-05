/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		texture.h
	Author:		Eric Bryant

	A texture resource
*/

#ifndef _TEXTURE_H
#define _TEXTURE_H

namespace Katana
{

//
// Forward Declarations
//
class Color;
class ColorA;

///
/// Texture
///
class Texture : public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Texture);
	KDECLARE_SCRIPT;

public:
	/// Default Index of Texture (for binding purposes)
	enum { INVALID_BIND_ID = -1 };

	/// Pixel Format of Texture
	enum PixelFormat {
		NONE = 0,
		UNKNOWN,
		RGB,
		RGBA,
		BGR_EXT,
		GREYSCALE,
	};

	/// Repeat Mode of Texture
	enum RepeatMode {
		CLAMP_ST,
		WRAP_ST,
		CLAMP_S_WRAP_T,
		WRAP_S_CLAMP_T,
	};

	/// Filtering Mode of Texture
	enum FilterMode {
        NEAREST,
		LINEAR,
        TRILINEAR,
        NEAREST_MIPMAP,
		LINEAR_MIPMAP,
	};

public:
	/// No Texture
	Texture();

	/// Blank texture with color
	Texture(const ColorA & rgba);
	Texture(float fR, float fG, float fB, float fA = 0.f);

	/// Texture from resource (e.g., bitmap)
	Texture(const char * szTexturePath);

	/// Texture from resource (e.g., bimap). However, it will be loaded only when needed by the renderer.
	Texture(const char * szTexturePath, bool bDelayLoad);

	/// Texture from resource. Sets up the default filter and repeat modes
	Texture(const char * szTexturePath, FilterMode filter, RepeatMode repeat );

	/// Texture from memory. We specify the image size and bit depth.
	Texture( unsigned char * pData, unsigned int uiWidth, unsigned uiHeight, unsigned int uiBitDepth, PixelFormat format );

	/// Destructor
	virtual ~Texture();

	/// Performs a delay loading (which is similar to a load, except a flag is
	/// set if this operation is valid).
	virtual bool delayLoad();

	/// Access the texture data
	virtual unsigned char * getData()								{ return m_data; }

	/// Unload a texture from system memory (but not the video memory)
	bool Unload();

	/// Has the texture been loaded
	bool isLoaded() const											{ return m_loaded; }

	/// Should the texture data stay resident when binded by the renderer
	/// (and thus loaded into video memory)
	void setStayResident(bool bStay)								{ m_stayResident = bStay; }
	bool getStayResident() const									{ return m_stayResident; }

	/// Set/Get the cache id
	void setBindID(int iID)											{ m_bindID = iID; }
	int getBindID() const											{ return m_bindID; }

	/// Set/Get the texture stage
	void setTextureStage(unsigned int uiStage)						{ m_textureStage = uiStage; }
	unsigned int getTextureStage() const							{ return m_textureStage; }

	/// Set/Get filter/repeat mode properties
	void setFilterMode(FilterMode eFilter)							{ m_filterMode = eFilter; }
	FilterMode getFilterMode() const								{ return m_filterMode; }

	void setRepeatMode(RepeatMode eRepeat)							{ m_repeatMode = eRepeat; }
	RepeatMode getRepeatMode() const								{ return m_repeatMode; }

	/// Retrieve image statistic
	unsigned int getWidth() const									{ return m_textureWidth; }
	unsigned int getHeight() const									{ return m_textureHeight; }
	unsigned int getBitDepth() const								{ return m_bitDepth; }
	unsigned int getMipMaps() const									{ return m_mipMaps; }
	bool		 getCubeMap() const									{ return m_cubeMap; }
	PixelFormat	 getPixelFormat() const								{ return m_pixelFormat; }
	unsigned int getDataSize() const								{ return m_dataSize; }

	/// Resamples the Texture to a different Width/Height (Only 8/24/32Bit Supported)
	bool Resample(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp = false);

	/// Adds an Blank (Opaque) Alpha Channel to this Texture (from 24Bit => 32Bit)
	bool addAlphaChannel(float fDefaultAlpha = 1.f);
	bool addAlphaChannel(shared_ptr<Texture> apTexture);

	/// Modify the gamma of the texture by a factor
	void adjustGamma( float fFactor );

public:
	/// Static function to create a texture from a resource. It's the clients
	/// responsiblity to destroy the newly created texture
	static shared_ptr<Texture> Create(const char * szTextureName);

	/// Static function creates a blank function with the following parameters
	static shared_ptr<Texture> Create(unsigned int uiWidth, unsigned int uiHeight, 
									  unsigned int uiBitDepth, unsigned char * pImageData, 
								 	  PixelFormat eFormat = BGR_EXT,
									  RepeatMode eRepeat = WRAP_ST, 
									  FilterMode eFilter = LINEAR_MIPMAP);


	/// Static function to load a texture from a resource into an existing
	/// texture class.
	static bool Load(const char * szTextureName, Texture & texture);

	/// Static function which will export the texture to file. The file
	/// name extension determine the type of file created (TGA Only)
	static bool Export(const char * szTextureName, const Texture & texture);

protected:

	/// Resamples a 8 Bit, 24 Bit or 32 Bit Texture
	bool Resample8 (unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp);
	bool Resample24(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp);
	bool Resample32(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp);

	/// Perform Bilinear Interpolation (for Resampling)
	static unsigned char BilinearInterpolateTexture(unsigned char * pTextureData, unsigned int uiImageCols,
													float fX, float fZ, char cColorIndex);

protected:
	/// Pointer to texture data
	unsigned char * m_data;

	/// The raw size of the texture data in bytes (set during LoadTexture()),
	/// used during Seralization of the texture data
	unsigned int	m_dataSize;

	/// Is the texture already loaded
	bool			m_loaded;

	/// When loaded into video memory (via bind), should we keep the local texture data
	bool			m_stayResident;

	/// ID determines if the texture has already been binded
	int				m_bindID;

	/// The texture stage this texture is used for
	unsigned int	m_textureStage;

	/// Cache the result of DelayLoad() so we don't retry Loading again.
	bool			m_delayLoadFailed;

	/// Resource path (stored for delay loading)
	string			m_resourcePath;

	/// Image statistics
	unsigned int	m_textureWidth;		/// Texture Width
	unsigned int	m_textureHeight;	/// Texture Height
	unsigned int	m_bitDepth;			/// Bit Depth (16,24,32)
	unsigned int	m_mipMaps;			/// # of mip maps (if any)
	bool			m_cubeMap;			/// Is this texture a cube map?
	PixelFormat		m_pixelFormat;		/// Pixel format (RGB, RGBA, etc)
	RepeatMode		m_repeatMode;		/// Repeat Mode
	FilterMode		m_filterMode;		/// Filtering Mode
};

KIMPLEMENT_STREAM( Texture );
KIMPLEMENT_SCRIPT( Texture );

}; // Katana

#endif // _TEXTURE_H