#include "GellyFluidSim.h"

#include "fluidsim/CD3D11SimContext.h"

ISimContext *Gelly::CreateD3D11SimContext(
	ID3D11Device *device, ID3D11DeviceContext *deviceContext
) {
	auto *context = new CD3D11SimContext();
	context->SetAPIHandle(SimContextHandle::D3D11_DEVICE, device);
	context->SetAPIHandle(
		SimContextHandle::D3D11_DEVICE_CONTEXT, deviceContext
	);
	return context;
}

IFluidSimulation *Gelly::CreateD3D11DebugFluidSimulation(
	const GellyObserverPtr<ISimContext> context, const int maxParticles
) {
	auto *sim = new CD3D11DebugFluidSimulation();
	sim->AttachToContext(context);
	sim->Initialize(maxParticles);

	return sim;
}