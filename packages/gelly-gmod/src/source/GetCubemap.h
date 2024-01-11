#ifndef GETCUBEMAP_H
#define GETCUBEMAP_H

#include "Signatures.h"
#include "detail/d3d9/Texture.h"

/**
 * \brief Since it's not localized to where ever you are rendering, this
 * function will return the currently active cubemap in the game at the camera's
 * position. \return Returns the currently active cubemap in the game.
 */
IDirect3DBaseTexture9* GetCubemap();

#endif //GETCUBEMAP_H