#include "TestbedWindow.h"

#include "Gelly.h"
#include "Rendering.h"
#include "Scene.h"
#include "Textures.h"
#include "imgui.h"

DEFINE_UI_DATA(TestbedWindow, thresholdRatio, 16.5f);
DEFINE_UI_DATA(TestbedWindow, particleRadius, 0.03f);
DEFINE_UI_DATA(TestbedWindow, rasterizerFlags, 0u);
DEFINE_UI_DATA(TestbedWindow, lastRasterizerFlags, 0u);

constexpr unsigned int MAX_FRAME_TIME_HISTORY = 512;
static float frameTimeHistory[MAX_FRAME_TIME_HISTORY] = {};
static unsigned int frameTimeHistoryIndex = 0;

using namespace testbed;

IMPLEMENT_WINDOW(TestbedWindow) {
	ImGui::Begin("Testbed");
	if (ImGui::CollapsingHeader("Frame Stats")) {
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame time: %.2f ms", ImGui::GetIO().DeltaTime * 1000.0f);

		// collect frame time history
		if (frameTimeHistoryIndex == MAX_FRAME_TIME_HISTORY) {
			frameTimeHistoryIndex = 0;
		}

		frameTimeHistory[frameTimeHistoryIndex] =
			ImGui::GetIO().DeltaTime * 1000.0f;
		frameTimeHistoryIndex++;

		ImGui::PlotLines(
			"Frame time history",
			frameTimeHistory,
			MAX_FRAME_TIME_HISTORY,
			static_cast<int>(frameTimeHistoryIndex),
			nullptr,
			0.0f,
			25.f,
			ImVec2(0, 80)
		);
	}

	if (ImGui::CollapsingHeader("Gelly Integration")) {
		ImGui::Text("Gelly renderer backend: D3D11");
		ImGui::Text("Gelly simulation backend: FleX using D3D11");

		ImGui::Text(
			"Sim max particle count: %d",
			GetGellyFluidSim()->GetSimulationData()->GetMaxParticles()
		);

		ImGui::Text(
			"Sim active particle count: %d",
			GetGellyFluidSim()->GetSimulationData()->GetActiveParticles()
		);

		if (ImGui::CollapsingHeader("Textures")) {
			ImGui::Image(
				GetTextureSRV(GELLY_ALBEDO_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GELLY_DEPTH_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);
			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GELLY_NORMAL_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GELLY_POSITIONS_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GELLY_THICKNESS_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);
		}

		if (ImGui::CollapsingHeader("Render Settings")) {
			auto settings = GetGellyFluidRenderSettings();
			if (ImGui::SliderInt(
					"Smoothing iterations", &settings.filterIterations, 0, 100
				)) {
				// Update only if changed
				UpdateGellyFluidRenderSettings(settings);
			}

			ImGui::SliderFloat(
				"Threshold ratio",
				&UI_DATA(TestbedWindow, thresholdRatio),
				0.0f,
				30.0f
			);

			// ideally this'd be driven by the simulation but
			// particle radius varies WILDLY with simulation techniques so
			// it actually is best to just tune it manually
			ImGui::SliderFloat(
				"Particle radius",
				&UI_DATA(TestbedWindow, particleRadius),
				0.0f,
				1.0f
			);
		}
	}

	if (ImGui::CollapsingHeader("Scene Info")) {
		const auto sceneMetadata = GetCurrentSceneMetadata();
		ImGui::Text("Scene file: %s", sceneMetadata.filepath);
		ImGui::Text("Triangles: %d", sceneMetadata.triangles);
	}

	if (ImGui::CollapsingHeader("Rendering")) {
		UI_DATA(TestbedWindow, lastRasterizerFlags) =
			UI_DATA(TestbedWindow, rasterizerFlags);

		if (ImGui::Button("Toggle wireframe")) {
			UI_DATA(TestbedWindow, rasterizerFlags) ^=
				RASTERIZER_FLAG_WIREFRAME;
		}

		if (ImGui::Button("Toggle no culling")) {
			UI_DATA(TestbedWindow, rasterizerFlags) ^= RASTERIZER_FLAG_NOCULL;
		}

		if (ImGui::CollapsingHeader("Buffers")) {
			ImGui::Image(
				GetTextureSRV(GBUFFER_DEPTH_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GBUFFER_POSITION_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GBUFFER_NORMAL_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			ImGui::SameLine();

			ImGui::Image(
				GetTextureSRV(GBUFFER_ALBEDO_TEXNAME),
				ImVec2(128, 128),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);
		}
	}

	ImGui::End();
}