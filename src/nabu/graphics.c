/*
 * NABU screen drawing for FujiNet Battleship.
 * G2 cell-write and CP437 font routines are in screen.c -- drawn from
 * terminal and CP437 demo code. Battleship font by productiondave.
 */

#include "../misc.h"

static uint8_t cursor_col;
static uint8_t current_player_count;
extern uint8_t shipSize[5];
extern uint8_t shipPlaceIndex;
extern uint8_t shipPlacements[5];
extern uint8_t nabu_placement_preview_active;
extern uint8_t nabu_placement_preview_pos;
extern void nabu_vdp_init_cp437(void);
extern void nabu_vdp_clear(void);
extern void nabu_vdp_text_at(uint8_t x, uint8_t y, const char *s);
extern void nabu_vdp_text_colour_at(uint8_t x, uint8_t y, const char *s, uint8_t fg, uint8_t bg);
extern void nabu_vdp_char_at(uint8_t x, uint8_t y, uint8_t c);

#define CP437_HLINE 0xC4  /* box horizontal */
#define CP437_VLINE 0xFE  /* box vertical */
#define CP437_TL 0xDA     /* box top left */
#define CP437_TR 0xBF     /* box top right */
#define CP437_BL 0xC0     /* box bottom left */
#define CP437_BR 0xD9     /* box bottom right */

#define BS_EMPTY  0x80    /* water tile */
#define BS_HIT    0x81    /* hit tile */
#define BS_MISS   0x82    /* miss tile - forgot to use it in place of "o" */
#define BS_CURSOR 0x88    /* cursor tile */

#define BS_CARRIER_STERN_H 0x90  /* carrier stern, horizontal */
#define BS_CARRIER_MID_H   0x91  /* carrier mid, horizontal */
#define BS_CARRIER_BOW_H   0x92  /* carrier bow, horizontal */
#define BS_CARRIER_STERN_V 0x93  /* carrier stern, vertical */
#define BS_CARRIER_MID_V   0x94  /* carrier mid, vertical */
#define BS_CARRIER_BOW_V   0x95  /* carrier bow, vertical */

#define BS_BATTLESHIP_STERN_H 0x98  /* battleship stern, horizontal */
#define BS_BATTLESHIP_MID_H   0x99  /* battleship mid, horizontal */
#define BS_BATTLESHIP_BOW_H   0x9A  /* battleship bow, horizontal */
#define BS_BATTLESHIP_STERN_V 0x9B  /* battleship stern, vertical */
#define BS_BATTLESHIP_MID_V   0x9C  /* battleship mid, vertical */
#define BS_BATTLESHIP_BOW_V   0x9D  /* battleship bow, vertical */

#define BS_CRUISER_STERN_H 0xA0  /* cruiser stern, horizontal */
#define BS_CRUISER_MID_H   0xA1  /* cruiser mid, horizontal */
#define BS_CRUISER_BOW_H   0xA2  /* cruiser bow, horizontal */
#define BS_CRUISER_STERN_V 0xA3  /* cruiser stern, vertical */
#define BS_CRUISER_MID_V   0xA4  /* cruiser mid, vertical */
#define BS_CRUISER_BOW_V   0xA5  /* cruiser bow, vertical */

#define BS_SUB_STERN_H 0xA8  /* sub stern, horizontal */
#define BS_SUB_MID_H   0xA9  /* sub mid, horizontal */
#define BS_SUB_BOW_H   0xAA  /* sub bow, horizontal */
#define BS_SUB_STERN_V 0xAB  /* sub stern, vertical */
#define BS_SUB_MID_V   0xAC  /* sub mid, vertical */
#define BS_SUB_BOW_V   0xAD  /* sub bow, vertical */

#define BS_DESTROYER_STERN_H 0xB0  /* destroyer stern, horizontal */
#define BS_DESTROYER_BOW_H   0xB1  /* destroyer bow, horizontal */
#define BS_DESTROYER_STERN_V 0xB2  /* destroyer stern, vertical */
#define BS_DESTROYER_BOW_V   0xB3  /* destroyer bow, vertical */

static const uint8_t board_x_2p[2] = {5, 18};
static const uint8_t board_y_2p[2] = {5, 5};
static const uint8_t tray_x_2p[2] = {1, 28};
static const uint8_t tray_y_2p[2] = {5, 5};
static const uint8_t board_x_4p[4] = {5, 5, 17, 17};
static const uint8_t board_y_4p[4] = {14, 3, 3, 14};
static const uint8_t tray_x_4p[4] = {1, 1, 28, 28};
static const uint8_t tray_y_4p[4] = {14, 3, 3, 14};
static const uint8_t legend_ship_offset[5][2] = {
    {2, 0},
    {1, 0},
    {0, 0},
    {0, 5},
    {1, 6}
};

/* Get board coordinates for a quadrant. */
static void board_origin(uint8_t quadrant, uint8_t *x, uint8_t *y)
{
    if (current_player_count > 2 && quadrant < 4) {
        *x = board_x_4p[quadrant];
        *y = board_y_4p[quadrant];
        return;
    }

    if (quadrant > 0) {
        *x = board_x_2p[1];
        *y = board_y_2p[1];
    } else {
        *x = board_x_2p[0];
        *y = board_y_2p[0];
    }
}

/* Get tray coordinates for a player. */
static void tray_origin(uint8_t player, uint8_t *x, uint8_t *y)
{
    if (current_player_count > 2 && player < 4) {
        *x = tray_x_4p[player];
        *y = tray_y_4p[player];
        return;
    }

    if (player > 0) {
        *x = tray_x_2p[1];
        *y = tray_y_2p[1];
    } else {
        *x = tray_x_2p[0];
        *y = tray_y_2p[0];
    }
}

/* Resolve the ship tile set. */
static uint8_t resolve_ship_index(uint8_t quadrant, uint8_t size, uint8_t pos)
{
    uint8_t i;
    uint8_t slot = (uint8_t)(quadrant == 0 ? 0 : 5);

    if (quadrant == 0) {
        for (i = 0; i < shipPlaceIndex && i < 5; ++i) {
            if (shipSize[i] == size && shipPlacements[i] == pos)
                return i;
        }

        if (nabu_placement_preview_active &&
            shipPlaceIndex < 5 &&
            pos == nabu_placement_preview_pos &&
            shipSize[shipPlaceIndex] == size) {
            return shipPlaceIndex;
        }
    }

    for (i = 0; i < 5; ++i) {
        if (shipSize[i] == size && clientState.game.myShips[slot + i] == pos)
            return i;
    }

    if (size == 5) return 0;
    if (size == 4) return 1;
    if (size == 2) return 4;
    return 2;
}

static uint8_t ship_stern_h(uint8_t index, uint8_t size)
{
    if (index == 3)
        return BS_SUB_STERN_H;

    switch (size) {
    case 5: return BS_CARRIER_STERN_H;
    case 4: return BS_BATTLESHIP_STERN_H;
    case 3: return BS_CRUISER_STERN_H;
    case 2: return BS_DESTROYER_STERN_H;
    default: return BS_SUB_STERN_H;
    }
}

static uint8_t ship_mid_h(uint8_t index, uint8_t size)
{
    if (index == 3)
        return BS_SUB_MID_H;

    switch (size) {
    case 5: return BS_CARRIER_MID_H;
    case 4: return BS_BATTLESHIP_MID_H;
    case 3: return BS_CRUISER_MID_H;
    case 2: return BS_DESTROYER_STERN_H;
    default: return BS_SUB_MID_H;
    }
}

static uint8_t ship_bow_h(uint8_t index, uint8_t size)
{
    if (index == 3)
        return BS_SUB_BOW_H;

    switch (size) {
    case 5: return BS_CARRIER_BOW_H;
    case 4: return BS_BATTLESHIP_BOW_H;
    case 3: return BS_CRUISER_BOW_H;
    case 2: return BS_DESTROYER_BOW_H;
    default: return BS_SUB_BOW_H;
    }
}

static uint8_t ship_stern_v(uint8_t index, uint8_t size)
{
    if (index == 3)
        return BS_SUB_STERN_V;

    switch (size) {
    case 5: return BS_CARRIER_STERN_V;
    case 4: return BS_BATTLESHIP_STERN_V;
    case 3: return BS_CRUISER_STERN_V;
    case 2: return BS_DESTROYER_STERN_V;
    default: return BS_SUB_STERN_V;
    }
}

static uint8_t ship_mid_v(uint8_t index, uint8_t size)
{
    if (index == 3)
        return BS_SUB_MID_V;

    switch (size) {
    case 5: return BS_CARRIER_MID_V;
    case 4: return BS_BATTLESHIP_MID_V;
    case 3: return BS_CRUISER_MID_V;
    case 2: return BS_DESTROYER_STERN_V;
    default: return BS_SUB_MID_V;
    }
}

static uint8_t ship_bow_v(uint8_t index, uint8_t size)
{
    if (index == 3)
        return BS_SUB_BOW_V;

    switch (size) {
    case 5: return BS_CARRIER_BOW_V;
    case 4: return BS_BATTLESHIP_BOW_V;
    case 3: return BS_CRUISER_BOW_V;
    case 2: return BS_DESTROYER_BOW_V;
    default: return BS_SUB_BOW_V;
    }
}

/* Draw a repeated character run. */
static void draw_repeat_char(uint8_t x, uint8_t y, uint8_t c, uint8_t count)
{
    uint8_t i;

    for (i = 0; i < count; ++i)
        nabu_vdp_char_at((uint8_t)(x + i), y, c);
}

/* Draw attack markers over a board. */
static void draw_attack_overlay(uint8_t quadrant, uint8_t *field)
{
    uint8_t x0;
    uint8_t y0;
    uint8_t x;
    uint8_t y;
    uint8_t cell;

    board_origin(quadrant, &x0, &y0);

    for (y = 0; y < 10; ++y) {
        for (x = 0; x < 10; ++x) {
            cell = field[(uint8_t)(y * 10 + x)];
            if (cell == FIELD_ATTACK)
                nabu_vdp_char_at((uint8_t)(x0 + x), (uint8_t)(y0 + y), BS_HIT);
            else if (cell == FIELD_MISS)
                nabu_vdp_char_at((uint8_t)(x0 + x), (uint8_t)(y0 + y), BS_MISS);
        }
    }
}

/* Clear the screen. */
void resetScreen()
{
    nabu_vdp_clear();
}

/* Advance the colour index. */
uint8_t cycleNextColor()
{
    cursor_col++;
    return cursor_col;
}

/* Draw normal text. */
void drawText(uint8_t x, uint8_t y, const char *s)
{
    nabu_vdp_text_at(x, y, s);
}

/* Draw alternate text. */
void drawTextAlt(uint8_t x, uint8_t y, const char *s)
{
    nabu_vdp_text_at(x, y, s);
}

/* Draw coloured text. */
void drawTextColor(uint8_t x, uint8_t y, const char *s, uint8_t fg, uint8_t bg)
{
    nabu_vdp_text_colour_at(x, y, s, fg, bg);
}

/* Draw one icon tile. */
void drawIcon(uint8_t x, uint8_t y, uint8_t icon)
{
    nabu_vdp_char_at(x, y, icon);
}

/* From shared gameplay: draw or hide one ship. */
void drawShip(uint8_t quadrant, uint8_t size, uint8_t pos, bool hide)
{
    uint8_t index;
    uint8_t x0;
    uint8_t y0;
    uint8_t vertical = 0;
    uint8_t i;

    board_origin(quadrant, &x0, &y0);
    index = resolve_ship_index(quadrant, size, pos);

    if (pos > 99) {
        vertical = 1;
        pos -= 100;
    }

    for (i = 0; i < size; ++i) {
        uint8_t tile = hide ? BS_EMPTY : 0;
        uint8_t x = (uint8_t)(x0 + (pos % 10));
        uint8_t y = (uint8_t)(y0 + (pos / 10));

        if (!hide) {
            if (vertical) {
                if (i == 0)
                    tile = ship_stern_v(index, size);
                else if (i + 1 == size)
                    tile = ship_bow_v(index, size);
                else
                    tile = ship_mid_v(index, size);
            } else {
                if (i == 0)
                    tile = ship_stern_h(index, size);
                else if (i + 1 == size)
                    tile = ship_bow_h(index, size);
                else
                    tile = ship_mid_h(index, size);
            }
        }

        nabu_vdp_char_at(x, y, tile);
        pos = (uint8_t)(pos + (vertical ? 10 : 1));
    }
}

/* From shared gameplay: draw one tray ship. */
void drawLegendShip(uint8_t player, uint8_t index, uint8_t size, uint8_t status)
{
    uint8_t x0;
    uint8_t y0;
    uint8_t row;
    uint8_t base_x;
    uint8_t base_y;

    tray_origin(player, &x0, &y0);
    base_x = (uint8_t)(x0 + legend_ship_offset[index][0]);
    base_y = (uint8_t)(y0 + legend_ship_offset[index][1]);

    for (row = 0; row < size; ++row)
        nabu_vdp_char_at(base_x, (uint8_t)(base_y + row), ' ');

    for (row = 0; row < size; ++row) {
        uint8_t tile;

        if (status == LEGEND_SHIP_DESTROYED) {
            tile = BS_HIT;
        } else if (size == 2) {
            tile = (row == 0) ? BS_DESTROYER_STERN_V : BS_DESTROYER_BOW_V;
        } else {
            if (row == 0)
                tile = ship_stern_v(index, size);
            else if (row + 1 == size)
                tile = ship_bow_v(index, size);
            else
                tile = ship_mid_v(index, size);
        }

        nabu_vdp_char_at(base_x, (uint8_t)(base_y + row), tile);
    }
}

/* From shared gameplay: draw a player label. */
void drawPlayerName(uint8_t player, const char *name, bool active)
{
    uint8_t x0;
    uint8_t y0;
    char line[14];

    board_origin(player, &x0, &y0);
    if (y0 > 0)
        y0--;

    if (current_player_count > 2) {
        sprintf(line, "%-8.8s", name);
        drawText(x0, y0, line);
        if (x0 > 0) {
            if (active)
                nabu_vdp_text_colour_at((uint8_t)(x0 - 1), y0, ">", 3, 1);
            else
                nabu_vdp_char_at((uint8_t)(x0 - 1), y0, ' ');
        }
    } else {
        if (active) {
            nabu_vdp_text_colour_at(x0, y0, ">", 3, 1);
            sprintf(line, "%-8.8s", name);
            drawText((uint8_t)(x0 + 1), y0, line);
        } else {
            sprintf(line, " %-8.8s", name);
            drawText(x0, y0, line);
        }
    }
}

/* From shared gameplay: draw the endgame message. */
void drawEndgameMessage(const char *message)
{
    nabu_vdp_text_at(0, 1, message);
}

/* From shared gameplay: draw one gamefield. */
void drawGamefield(uint8_t quadrant, uint8_t *field)
{
    uint8_t x0;
    uint8_t y0;
    uint8_t x;
    uint8_t y;
    uint8_t cell;

    board_origin(quadrant, &x0, &y0);

    for (y = 0; y < 10; ++y) {
        for (x = 0; x < 10; ++x) {
            uint8_t tile = BS_EMPTY;

            cell = field[(uint8_t)(y * 10 + x)];
            if (cell == FIELD_ATTACK)
                tile = BS_HIT;
            else if (cell == FIELD_MISS)
                tile = BS_MISS;

            nabu_vdp_char_at((uint8_t)(x0 + x), (uint8_t)(y0 + y), tile);
        }
    }
}

/* From shared gameplay: update one gamefield cell. */
void drawGamefieldUpdate(uint8_t quadrant, uint8_t *gamefield, uint8_t attackPos, uint8_t anim)
{
    uint8_t x0;
    uint8_t y0;
    uint8_t tile;
    uint8_t cell;

    anim = anim;
    board_origin(quadrant, &x0, &y0);
    cell = gamefield[attackPos];
    tile = BS_EMPTY;
    if (cell == FIELD_ATTACK)
        tile = BS_HIT;
    else if (cell == FIELD_MISS)
        tile = BS_MISS;

    nabu_vdp_char_at((uint8_t)(x0 + (attackPos % 10)), (uint8_t)(y0 + (attackPos / 10)), tile);
}

/* From shared gameplay: draw attack markers over a gamefield. */
void drawGamefieldOverlay(uint8_t quadrant, uint8_t *gamefield)
{
    draw_attack_overlay(quadrant, gamefield);
}

/* From shared gameplay: draw the attack cursor. */
void drawGamefieldCursor(uint8_t quadrant, uint8_t x, uint8_t y, uint8_t *gamefield, uint8_t blink)
{
    uint8_t x0;
    uint8_t y0;

    gamefield = gamefield;
    blink = blink;
    board_origin(quadrant, &x0, &y0);
    nabu_vdp_char_at((uint8_t)(x0 + x), (uint8_t)(y0 + y), BS_CURSOR);
}

/* From shared gameplay: draw the clock icon. */
void drawClock()
{
}

/* From shared gameplay: draw the connection icon. */
void drawConnectionIcon(bool show)
{
    show = show;
}

/* Draw one blank tile. */
void drawBlank(uint8_t x, uint8_t y)
{
    nabu_vdp_char_at(x, y, ' ');
}

/* Draw a run of blank tiles. */
void drawSpace(uint8_t x, uint8_t y, uint8_t w)
{
    draw_repeat_char(x, y, ' ', w);
}

/* Draw a horizontal line. */
void drawLine(uint8_t x, uint8_t y, uint8_t w)
{
    draw_repeat_char(x, y, CP437_HLINE, w);
}

/* Draw a box. */
void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    uint8_t row;

    if (w < 2 || h < 2)
        return;

    nabu_vdp_char_at(x, y, CP437_TL);
    nabu_vdp_char_at((uint8_t)(x + w - 1), y, CP437_TR);
    nabu_vdp_char_at(x, (uint8_t)(y + h - 1), CP437_BL);
    nabu_vdp_char_at((uint8_t)(x + w - 1), (uint8_t)(y + h - 1), CP437_BR);

    if (w > 2) {
        draw_repeat_char((uint8_t)(x + 1), y, CP437_HLINE, (uint8_t)(w - 2));
        draw_repeat_char((uint8_t)(x + 1), (uint8_t)(y + h - 1), CP437_HLINE, (uint8_t)(w - 2));
    }

    for (row = 1; row + 1 < h; ++row) {
        nabu_vdp_char_at(x, (uint8_t)(y + row), CP437_VLINE);
        nabu_vdp_char_at((uint8_t)(x + w - 1), (uint8_t)(y + row), CP437_VLINE);
    }
}

/* From shared gameplay: reset and prepare the board layout. */
void drawBoard(uint8_t playerCount)
{
    current_player_count = playerCount;
    resetScreen();
}

/* Save the screen buffer. */
bool saveScreenBuffer()
{
    return false;
}

/* Restore the screen buffer. */
void restoreScreenBuffer()
{
}

/* Initialise graphics mode. */
void initGraphics()
{
    vdp_clearVRAM(); /* Suggested screen corruption buster by GTAMP */
    nabu_vdp_init_cp437();
}

/* Reset graphics mode. */
void resetGraphics()
{
}
