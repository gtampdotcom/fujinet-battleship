/*
 * NABU sound -- not yet implemented.
 * AY sound was attempted in 1.00.48/1.00.49, caused a CP/M startup crash,
 * rolled back in 1.00.50. Do not revisit until gameplay is stable.
 * Updated Sound - Based on Dave's amazing help!
 */

#include "../misc.h"

#define AY_CHA_TONE_L 0
#define AY_CHA_TONE_H 1
#define AY_NOISE_GEN  6
#define AY_ENABLES    7
#define AY_CHA_AMPL   8
#define AY_CHC_AMPL   10
#define AY_ENV_PERIOD_L 11
#define AY_ENV_PERIOD_H 12
#define AY_ENV_SHAPE  13
#define AY_ENV_SHAPE_FADE_OUT 0x09

static uint8_t pew_frames;
static uint8_t pew_sweep;
static uint8_t pew_active;

static void enable_tone_a_only(void)
{
    uint8_t mixer;

    mixer = ayRead(AY_ENABLES);
    mixer = (uint8_t)((mixer & 0xC0u) | 0x3Eu);
    ayWrite(AY_ENABLES, mixer);
}

static void enable_noise_c_only(void)
{
    uint8_t mixer;

    mixer = ayRead(AY_ENABLES);
    mixer = (uint8_t)((mixer & 0xC0u) | 0x1Fu);
    ayWrite(AY_ENABLES, mixer);
}

void initSound() {}
void disableKeySounds() {}
void enableKeySounds() {}
void soundCursor() {}
void soundSelect() {}
void soundStop()
{
    pew_frames = 0;
    pew_active = 0;
    ayWrite(AY_CHA_AMPL, 0);
}
void soundJoinGame() {}
void soundMyTurn() {}
void soundGameDone() {}
void soundTick()
{
    if (pew_frames > 0) {
        pew_frames--;
        pew_sweep = (uint8_t)(pew_sweep + 16);
        ayWrite(AY_CHA_TONE_L, pew_sweep);
    } else if (pew_active) {
        pew_active = 0;
        ayWrite(AY_CHA_AMPL, 0);
    }
}
void soundPlaceShip() {}
void soundAttack()
{
    pew_frames = 8;
    pew_sweep = 0x50;
    pew_active = 1;
    enable_tone_a_only();
    ayWrite(AY_CHA_TONE_L, 0);
    ayWrite(AY_CHA_TONE_H, pew_sweep);
    ayWrite(AY_CHA_AMPL, 0x0A);
}
void soundInvalid() {}
void soundHit()
{
    enable_noise_c_only();
    ayWrite(AY_CHC_AMPL, 0x10);
    ayWrite(AY_NOISE_GEN, 0x1F);
    ayWrite(AY_ENV_PERIOD_L, 0x00);
    ayWrite(AY_ENV_PERIOD_H, 0x0F);
    ayWrite(AY_ENV_SHAPE, AY_ENV_SHAPE_FADE_OUT);
}
void soundSink() {}
void soundMiss()
{
    enable_noise_c_only();
    ayWrite(AY_CHC_AMPL, 0x10);
    ayWrite(AY_NOISE_GEN, 0x08);
    ayWrite(AY_ENV_PERIOD_L, 0x80);
    ayWrite(AY_ENV_PERIOD_H, 0x03);
    ayWrite(AY_ENV_SHAPE, AY_ENV_SHAPE_FADE_OUT);
}
