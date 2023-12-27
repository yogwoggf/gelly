#ifndef GELLY_CD3D11DEBUGFLUIDRENDERER_H
#define GELLY_CD3D11DEBUGFLUIDRENDERER_H

#include <GellyFluidSim.h>
#include <GellyInterfaceRef.h>

#include "CD3D11DebugFluidTextures.h"
#include "CD3D11ManagedBuffer.h"
#include "IFluidRenderer.h"
#include "IManagedBufferLayout.h"
#include "renderdoc_app.h"

class CD3D11DebugFluidRenderer : public IFluidRenderer {
private:
	GellyInterfaceVal<IRenderContext> context;
	/**
	 * The particle data comes from here, but the renderer does not own it.
	 */
	GellyInterfaceVal<ISimData> simData;
	CD3D11DebugFluidTextures outputTextures;

	Gelly::FluidRenderSettings settings;
	int maxParticles{};

	FluidRenderParams cbufferData{};

	bool lowBitMode = false;

	struct {
		GellyInterfaceVal<IManagedBuffer> positions;
		GellyInterfaceVal<IManagedBufferLayout> positionsLayout;
		GellyInterfaceVal<IManagedBuffer> fluidRenderCBuffer;
		GellyInterfaceVal<IManagedDepthBuffer> depthBuffer;

		GellyInterfaceVal<IManagedBuffer> screenQuad;
		GellyInterfaceVal<IManagedBufferLayout> screenQuadLayout;
	} buffers;

	struct {
		GellyInterfaceVal<IManagedTexture> unfilteredDepth;
		GellyInterfaceVal<IManagedTexture> unfilteredThickness;

		/**
		 * \brief When in low-bit mode, this texture is the one in the filter
		 * flipping instead of the output texture. This is so that all of our
		 * internal processes can continue to enjoy high precision, but we can
		 * output an encoded depth at the final stage.
		 */
		GellyInterfaceVal<IManagedTexture> untransformedDepth;
	} internalTextures{};

	struct {
		GellyInterfaceVal<IManagedShader> splattingPS;
		GellyInterfaceVal<IManagedShader> splattingVS;
		GellyInterfaceVal<IManagedShader> splattingGS;

		GellyInterfaceVal<IManagedShader> thicknessPS;
		GellyInterfaceVal<IManagedShader> thicknessVS;
		GellyInterfaceVal<IManagedShader> thicknessGS;

		GellyInterfaceVal<IManagedShader> screenQuadVS;

		GellyInterfaceVal<IManagedShader> filterDepthPS;
		GellyInterfaceVal<IManagedShader> estimateNormalPS;
		GellyInterfaceVal<IManagedShader> filterThicknessPS;
		GellyInterfaceVal<IManagedShader> encodeDepthPS;
	} shaders{};

#ifdef _DEBUG
	RENDERDOC_API_1_1_2 *renderDocApi = nullptr;
#endif

	void CreateBuffers();
	void CreateTextures();
	void CreateShaders();

	void RenderUnfilteredDepth();
	void RenderFilteredDepth();
	void RenderNormals();
	void RenderThickness();
	void RenderFilteredThickness();
	void EncodeDepth();

public:
	CD3D11DebugFluidRenderer();
	~CD3D11DebugFluidRenderer() override = default;

	void SetSimData(GellyObserverPtr<ISimData> simData) override;
	void AttachToContext(GellyObserverPtr<IRenderContext> context) override;
	GellyObserverPtr<IFluidTextures> GetFluidTextures() override;

	void Render() override;
	void EnableLowBitMode() override;

	void SetSettings(const Gelly::FluidRenderSettings &settings) override;
	void SetPerFrameParams(const Gelly::FluidRenderParams &params) override;

#ifdef _DEBUG
	bool EnableRenderDocCaptures() override;
#endif
};

#endif	// GELLY_CD3D11DEBUGFLUIDRENDERER_H