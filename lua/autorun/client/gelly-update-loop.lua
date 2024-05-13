-- Should never be zero, or the simulation will not run
local FPS_AT_MAX_PARTICLES = 2
START_SIM_FPS = 144

GELLY_SIM_TIMESCALE = 10
GELLY_KILL_PLAYER_ON_CONTACT = false

local function calculateThrottledDeltaTime()
	return 1
		/ math.floor(
			(FPS_AT_MAX_PARTICLES - START_SIM_FPS)
					* (gelly.GetStatus().ActiveParticles / gelly.GetStatus().MaxParticles)
				+ START_SIM_FPS
		)
end

hook.Add("GellyLoaded", "gelly.update-loop", function()
	hook.Add("PostDrawOpaqueRenderables", "gelly.composite", function(depth, skybox)
		gelly.Composite()
	end)

	timer.Create("gelly.update", 1 / START_SIM_FPS, 0, function()
		local dt = calculateThrottledDeltaTime()
		gelly.Simulate(dt * GELLY_SIM_TIMESCALE)
		timer.Adjust("gelly.update", dt, nil, nil)
	end)
end)
