# A particle that switches the depth test leaves it switched for the particles drawn after it, so
# RenderParticles must switch it before an off-screen particle is skipped: otherwise a culled light
# no longer restores the test an explosion turned off, and later particles draw through geometry.
if(NOT DEFINED MU_PARTICLE_SOURCE)
    message(FATAL_ERROR "MU_PARTICLE_SOURCE is required")
endif()
file(READ "${MU_PARTICLE_SOURCE}" source)

string(FIND "${source}" "void RenderParticles(" begin)
if(begin EQUAL -1)
    message(FATAL_ERROR "RenderParticles is missing")
endif()
string(SUBSTRING "${source}" ${begin} -1 rest)
string(FIND "${rest}" "\n}" end)
string(SUBSTRING "${rest}" 0 ${end} body)

string(FIND "${body}" "EnableDepthTest();" enable_at)
string(FIND "${body}" "DisableDepthTest();" disable_at)
string(FIND "${body}" "TestFrustrum(o->Position" cull_at)
if(enable_at EQUAL -1 OR disable_at EQUAL -1 OR cull_at EQUAL -1)
    message(FATAL_ERROR "RenderParticles' depth test switches or its frustum cull are missing")
endif()
if(NOT enable_at LESS cull_at OR NOT disable_at LESS cull_at)
    message(FATAL_ERROR "RenderParticles must switch the depth test before it skips an off-screen particle")
endif()
