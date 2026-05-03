/*
 * NABU timing, random numbers, and general routines.
 */

#include "../misc.h"

static uint16_t tick_count;
static uint8_t rng_seeded;

extern uint16_t nabu_get_session_id(void);

/* Reads the Z80 R register for a random starting seed -- same approach as NIALL. */
static uint16_t nabu_get_r_seed(void)
{
    __asm
    ld a, r
    ld l, a
    ld h, #0
    ret
    __endasm;
    return 0;
}

/* Advance the local tick counter. */
void nabu_tick(void)
{
    tick_count++;
}

/* Reset the local timer. */
void resetTimer()
{
    tick_count = 0;
}

/* Return elapsed local ticks. */
uint16_t getTime()
{
    return tick_count;
}

/* Return to CP/M. */
void quit()
{
    vdp_disableVDPReadyInt();
    exit(0);
}

/* Run platform housekeeping. */
void housekeeping()
{
}

/* Return the local tick rate. */
uint8_t getJiffiesPerSecond()
{
    return 60;
}

/* Return a random number. */
uint8_t getRandomNumber(uint8_t maxExclusive)
{
    uint16_t seed;

    if (maxExclusive == 0)
        return 0;

    if (!rng_seeded)
    {
        seed = nabu_get_session_id();
        seed ^= nabu_get_r_seed();
        seed ^= (uint16_t)(tick_count << 8);
        srand(seed);
        rng_seeded = 1;
    }

    return (uint8_t)(rand() % maxExclusive);
}
