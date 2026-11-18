#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#undef NDEBUG
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void check_sweeps(void)
{
    const xge_vec2_t start = {0, 2}, finish = {0, -2}, normal = {0, 2};
    const xge_vec2_t left = {-4, 2}, right = {4, 2};
    const xge_rect_t rect = {0, 0, 8, 8};
    xge_particle_hit_t contact, legacy, saved;
    bool hit = false;
    assert(xgeParticleSweepPlane(start, finish, .5f, normal, 0, &hit, &contact) == XGE_OK && hit);
    assert(contact.fFraction == .375f && contact.tPosition.fY == .5f && contact.tNormal.fY == 1);
    assert(xgeParticleCollidePlane(start, finish, .5f, normal, 0, &legacy) == 1);
    assert(memcmp(&contact, &legacy, sizeof(contact)) == 0);
    assert(xgeParticleSweepRect(left, right, .5f, rect, &hit, &contact) == XGE_OK && hit);
    assert(contact.fFraction == .4375f && contact.tPosition.fX == -.5f && contact.tNormal.fX == -1);
    assert(xgeParticleCollideRect(left, right, .5f, rect, &legacy) == 1);
    assert(memcmp(&contact, &legacy, sizeof(contact)) == 0);

    saved = contact;
    assert(xgeParticleSweepPlane(start, start, .5f, normal, 0, &hit, &contact) == XGE_OK && !hit);
    assert(xgeParticleSweepRect(left, left, .5f, rect, &hit, &contact) == XGE_OK && !hit);
    assert(memcmp(&contact, &saved, sizeof(contact)) == 0);
    hit = true;
    assert(xgeParticleSweepPlane(start, finish, -.5f, normal, 0, &hit, &contact) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeParticleSweepPlane(start, finish, .5f, (xge_vec2_t){0, 0}, 0, &hit, &contact) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeParticleSweepPlane(start, finish, .5f, normal, INFINITY, &hit, &contact) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeParticleSweepRect(left, right, .5f, (xge_rect_t){0, 0, 0, 8}, &hit, &contact) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeParticleSweepRect(left, right, .5f, rect, NULL, &contact) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeParticleCollideRect(left, right, -1, rect, &legacy) == 0);
    assert(hit && memcmp(&contact, &saved, sizeof(contact)) == 0);
}

static int32 poll_from_other_thread(void *context)
{
    xge_particle_event_t event = {.iType = 321};
    bool available = true;
    assert(xgeParticleWorldPollEvent(context, &available, &event) == XGE_ERROR_BUSY);
    assert(available && event.iType == 321);
    xrtClearError();
    return 0;
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge_particle_world_desc_t settings;
    xgeParticleWorldDescInit(&settings);
    settings.iMaxEffects = 2;
    settings.iMaxParticles = 8;
    settings.iMaxEvents = 2;
    xge_particle_world world;
    assert(xgeParticleWorldCreateOwned(&world, &settings) == XGE_OK);
    xge_particle_emitter_t emitter;
    xgeParticleEmitterInit(&emitter);
    emitter.bAutomatic = 0;
    emitter.iMaxParticles = 8;
    xge_particle_definition definition;
    assert(xgeParticleDefinitionCreate(&definition, &emitter, 1) == XGE_OK);
    xge_particle_effect effect;
    assert(xgeParticlePlay(world, definition, NULL, &effect) == XGE_OK);

    xge_particle_event_t event = {.iType = 321};
    bool available = true;
    assert(xgeParticleWorldPollEvent(world, &available, &event) == XGE_OK && !available && event.iType == 321);
    uint32_t emitted;
    assert(xgeParticleEmit(world, effect, 0, 2, &emitted) == XGE_OK && emitted == 2);
    xthread *thread = xrtThreadCreate(poll_from_other_thread, world, 0);
    assert(thread && xrtThreadWait(thread) == XWAIT_OK);
    xrtThreadDestroy(thread);
    assert(xgeParticleWorldPollEvent(world, NULL, &event) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeParticleWorldPollEvent(world, &available, NULL) == XGE_ERROR_INVALID_ARGUMENT);
    assert(!available && event.iType == 321 && xgeParticleStats(world).iPendingEvents == 2);
    assert(xgeParticleWorldPollEvent(world, &available, &event) == XGE_OK && available);
    assert(event.iType == XGE_PARTICLE_EVENT_BIRTH && event.iEffect == effect && event.iEmitter == 0);
    uint64_t first = event.iParticle;
    assert(xgeParticleEmit(world, effect, 0, 1, &emitted) == XGE_OK && emitted == 1);
    assert(xgeParticleEventPoll(world, &event) == 1 && event.iParticle == first + 1);
    assert(xgeParticleWorldPollEvent(world, &available, &event) == XGE_OK && available);
    assert(event.iParticle == first + 2 && xgeParticleEventPoll(world, &event) == 0);

    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for (int i = 0; i < 1000; ++i) {
        assert(xgeParticleState(world, effect) == XGE_PARTICLE_PLAYING);
        assert(xgeParticleWorldPollEvent(world, &available, &event) == XGE_OK && !available);
        check_sweeps();
    }
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount == after.AllocCount && before.FreeCount == after.FreeCount);
    assert(before.ReallocCount == after.ReallocCount && before.LiveCount == after.LiveCount);
    assert(event.iParticle == first + 2);
    assert(xgeParticleRelease(world, effect) == XGE_OK);
    assert(!xrtGetError());
    assert(xgeParticleState(world, effect) == XGE_ERROR_NOT_FOUND);
    xrtClearError();
    assert(xgeParticleWorldFreeChecked(world) == XGE_OK);
    xgeParticleDefinitionFree(definition);
    puts("PARTICLE_QUERIES_NATIVE_OK");
    return 0;
}
