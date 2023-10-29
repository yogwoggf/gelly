#ifndef GELLY_IMANAGEDTEXTURE_H
#define GELLY_IMANAGEDTEXTURE_H

#include <GellyInterface.h>

#include <cstdint>

class IRenderContext;

enum class GellyTextureFormat : uint8_t {
	R8G8B8A8_UNORM,
	R32G32B32A32_FLOAT,
};

/**
 * Texture access flags.
 * These are used to determine how the texture will be used.
 * For example, using D3D11, a texture that is read from and written to will
 * be created with an accompanying SRV and UAV.
 * A texture that is only read from will only have an SRV.
 */
enum class GellyTextureAccess : uint8_t {
	READ = 0b01,
	WRITE = 0b10,
};

struct GellyTextureDesc {
	GellyTextureFormat format;
	GellyTextureAccess access;

	uint16_t width;
	uint16_t height;
};

/**
 * Interface for implementing managed textures.
 * These are textures that are created and managed by the renderer, and make no
 * promises about the underlying resource's lifetime.
 *
 * However, notice that there is no standard API for accessing the underlying
 * resource. This is because many rendering APIs have different ways of
 * representing textures.
 */
gelly_interface IManagedTexture {
	friend class IRenderContext;

protected:
	virtual bool Create() = 0;
	virtual void Destroy() = 0;

public:
	virtual ~IManagedTexture() = 0;

	virtual void SetDesc(const GellyTextureDesc &desc) = 0;
	[[nodiscard]] virtual const GellyTextureDesc &GetDesc() const = 0;
	virtual void AttachToContext(IRenderContext * context) = 0;
};

#endif	// GELLY_IMANAGEDTEXTURE_H