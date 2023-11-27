#include "fluidrender/CD3D11RenderContext.h"

#include <GellyD3D.h>

#include <stdexcept>

#include "fluidrender/CD3D11ManagedBuffer.h"
#include "fluidrender/CD3D11ManagedBufferLayout.h"
#include "fluidrender/CD3D11ManagedDepthBuffer.h"
#include "fluidrender/CD3D11ManagedShader.h"
#include "fluidrender/CD3D11to11SharedTexture.h"
#include "fluidrender/IRenderContext.h"

CD3D11RenderContext::CD3D11RenderContext(uint16_t width, uint16_t height)
	: device(nullptr),
	  deviceContext(nullptr),
	  width(width),
	  height(height),
	  textures({}),
	  shaders({}),
	  rasterizerState(nullptr) {
	CreateDeviceAndContext();
}

void CD3D11RenderContext::CreateDeviceAndContext() {
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	DX("Failed to create D3D11 device",
	   D3D11CreateDevice(
		   nullptr,
		   D3D_DRIVER_TYPE_HARDWARE,
		   nullptr,
		   deviceFlags,
		   nullptr,
		   0,
		   D3D11_SDK_VERSION,
		   &device,
		   &featureLevel,
		   &deviceContext
	   ));

#ifdef _DEBUG
	DX("Failed to get D3D11 debug interface",
	   device->QueryInterface(
		   __uuidof(ID3D11Debug), reinterpret_cast<void **>(&debug)
	   ));

	DX("Failed to get D3D11 info queue",
	   debug->QueryInterface(
		   __uuidof(ID3D11InfoQueue), reinterpret_cast<void **>(&infoQueue)
	   ));
#endif

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	DX("Failed to create D3D11 rasterizer state",
	   device->CreateRasterizerState(&rasterizerDesc, &rasterizerState));

	deviceContext->RSSetState(rasterizerState);
}

void CD3D11RenderContext::CreateAllTextures() {
	for (auto &texture : textures) {
		if (!texture.second->Create()) {
			throw std::runtime_error(
				"Failed to create texture's underlying resource"
			);
		}
	}
}

void CD3D11RenderContext::DestroyAllTextures() {
	for (auto &texture : textures) {
		DestroyTexture(texture.first.c_str());
	}
}

void CD3D11RenderContext::CreateAllShaders() {
	for (const auto &shader : shaders) {
		shader->Create();
	}
}

void CD3D11RenderContext::DestroyAllShaders() {
	for (const auto &shader : shaders) {
		shader->Destroy();
	}
}

void *CD3D11RenderContext::GetRenderAPIResource(RenderAPIResource resource) {
	switch (resource) {
		case RenderAPIResource::D3D11Device:
			return device;
		case RenderAPIResource::D3D11DeviceContext:
			return deviceContext;
		default:
			return nullptr;
	}
}

ContextRenderAPI CD3D11RenderContext::GetRenderAPI() {
	return ContextRenderAPI::D3D11;
}

IManagedTexture *CD3D11RenderContext::CreateTexture(
	const char *name, const TextureDesc &desc
) {
	if (textures.find(name) != textures.end()) {
		throw std::logic_error("Texture already exists");
	}

	auto *texture = new CD3D11ManagedTexture();
	texture->SetDesc(desc);
	texture->AttachToContext(this);
	texture->Create();
	textures[name] = texture;
	return texture;
}

GellyObserverPtr<IManagedTexture> CD3D11RenderContext::CreateSharedTexture(
	const char *name, HANDLE sharedHandle
) {
	if (textures.find(name) != textures.end()) {
		throw std::logic_error("Texture already exists");
	}

	const auto texture = new CD3D11to11SharedTexture(sharedHandle);
	texture->AttachToContext(this);
	texture->Create();
	textures[name] = texture;
	return texture;
}

GellyObserverPtr<IManagedShader> CD3D11RenderContext::CreateShader(
	const uint8_t *bytecode, size_t bytecodeSize, ShaderType type
) {
	auto *shader = new CD3D11ManagedShader();
	shader->AttachToContext(this);
	shader->SetBytecode(bytecode, bytecodeSize);
	shader->SetType(type);
	shader->Create();
	shaders.push_back(shader);
	return shader;
}

GellyObserverPtr<IManagedBuffer> CD3D11RenderContext::CreateBuffer(
	const BufferDesc &desc
) {
	auto *buffer = new CD3D11ManagedBuffer();
	buffer->SetDesc(desc);
	buffer->AttachToContext(this);
	buffer->Create();
	return buffer;
}

GellyInterfaceVal<IManagedBufferLayout> CD3D11RenderContext::CreateBufferLayout(
	const BufferLayoutDesc &desc
) {
	auto *layout = new CD3D11ManagedBufferLayout();
	layout->SetLayoutDesc(desc);
	layout->AttachToContext(this);
	layout->Create();
	return layout;
}

GellyInterfaceVal<IManagedDepthBuffer> CD3D11RenderContext::CreateDepthBuffer(
	const DepthBufferDesc &desc
) {
	auto *depthBuffer = new CD3D11ManagedDepthBuffer();
	depthBuffer->SetDesc(desc);
	depthBuffer->AttachToContext(this);
	depthBuffer->Create();
	return depthBuffer;
}

void CD3D11RenderContext::DestroyTexture(const char *name) {
	const auto texture = textures.find(name);
	if (texture == textures.end()) {
		throw std::logic_error("Texture does not exist");
	}

	texture->second->Destroy();
	delete texture->second;
	textures.erase(texture);
}

void CD3D11RenderContext::SetDimensions(uint16_t width, uint16_t height) {
	this->width = width;
	this->height = height;
}

void CD3D11RenderContext::GetDimensions(uint16_t &width, uint16_t &height) {
	width = this->width;
	height = this->height;
}

void CD3D11RenderContext::SubmitWork() {
	deviceContext->Flush();

	// well-known query method to synchronize the GPU after
	// the commands finish executing

	D3D11_QUERY_DESC queryDesc = {};
	queryDesc.Query = D3D11_QUERY_EVENT;

	ID3D11Query *query;
	DX("Failed to create D3D11 query", device->CreateQuery(&queryDesc, &query));

	deviceContext->End(query);
	while (deviceContext->GetData(query, nullptr, 0, 0) == S_FALSE) {
		// spin
	}

	query->Release();

	if (const auto removeResult = device->GetDeviceRemovedReason();
		removeResult != S_OK) {
		DestroyAllTextures();
		DestroyAllShaders();
		ReleaseDevice();
		CreateDeviceAndContext();
		CreateAllTextures();
		CreateAllShaders();
	}
}

void CD3D11RenderContext::Draw(
	const uint32_t vertexCount, const uint32_t startVertex
) {
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	deviceContext->RSSetViewports(1, &viewport);
	deviceContext->RSSetState(rasterizerState);
	deviceContext->Draw(vertexCount, startVertex);
}

void CD3D11RenderContext::ResetPipeline() {
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	deviceContext->ClearState();
}

void CD3D11RenderContext::SetRasterizerFlags(RasterizerFlags flags) {
	if (rasterizerFlags == flags) {
		return;
	}

	rasterizerFlags = flags;

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	if ((flags & RasterizerFlags::DISABLE_CULL) != 0) {
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
	}

	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	rasterizerState->Release();

	DX("Failed to create D3D11 rasterizer state",
	   device->CreateRasterizerState(&rasterizerDesc, &rasterizerState));
	deviceContext->RSSetState(rasterizerState);
}

CD3D11RenderContext::~CD3D11RenderContext() {
	DestroyAllTextures();
	DestroyAllShaders();
	ReleaseDevice();
}

void CD3D11RenderContext::ReleaseDevice() {
	if (deviceContext) {
		deviceContext->Release();
		deviceContext = nullptr;
	}

	if (device) {
		device->Release();
		device = nullptr;
	}
}

#ifdef _DEBUG
void CD3D11RenderContext::PrintDebugInfo() {
	if (infoQueue == nullptr) {
		return;
	}

	const auto messageCount = infoQueue->GetNumStoredMessages();
	for (UINT i = 0; i < messageCount; i++) {
		SIZE_T messageLength;
		infoQueue->GetMessage(i, nullptr, &messageLength);

		auto *message = static_cast<D3D11_MESSAGE *>(malloc(messageLength));
		infoQueue->GetMessage(i, message, &messageLength);

		printf("%s\n", message->pDescription);

		free(message);
	}
}
#endif
