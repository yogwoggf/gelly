#ifndef GELLY_GELLYENGINEGMOD_H
#define GELLY_GELLYENGINEGMOD_H

#include <GellyEngine.h>

class GellyEngineGMod {
private:
	GellyScene *scene;

	static MeshUploadInfo ProcessBSP(uint8_t *data, size_t dataSize);
	static void FreeBSP(MeshUploadInfo &info);

public:
	GellyEngineGMod(
		int maxParticles, int maxColliders, void *d3d11Device = nullptr
	);

	~GellyEngineGMod();

	GellyScene *GetScene();

	void AddBSP(const std::string &mapName, uint8_t *data, size_t dataSize);
};

#endif	// GELLY_GELLYENGINEGMOD_H
