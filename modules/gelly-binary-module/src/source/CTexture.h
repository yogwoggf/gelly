#ifndef GELLY_CTEXTURE_H
#define GELLY_CTEXTURE_H

typedef void **CTexture;
typedef unsigned long long ShaderAPITextureHandle_t;

ShaderAPITextureHandle_t GetCTextureHandle(CTexture *texture);
const char *GetCTextureName(CTexture *texture);

#endif	// GELLY_CTEXTURE_H
