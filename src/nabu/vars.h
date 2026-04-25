#ifndef NABU_KEYMAP_H
#define NABU_KEYMAP_H

/*
 * NABU CP/M platform values for the first text-focused stage.
 */

#define NABU_BATTLE_VERSION "1.00.91"  /* shown in screen corners */

/* routes network and AppKey calls to NABULIB/RetroNET instead of FujiNet SIO */
#define CUSTOM_FUJINET_CALLS

/* No joystick path is wired in the first text-focused stage */
#define JOY_UP(v) 0
#define JOY_DOWN(v) 0
#define JOY_LEFT(v) 0
#define JOY_RIGHT(v) 0
#define JOY_BTN_1(v) 0
#define JOY_BTN_2(v) 0

#define WIDTH 40   /* screen columns */
#define HEIGHT 25  /* screen rows */

#define ROLL_SOUND_MOD 4          /* sound effect period divisor */
#define ROLL_FRAMES 24            /* animation frames per dice roll */
#define SCORE_CURSOR_ALT 0        /* alternate score cursor -- not used on NABU */
#define BOTTOM_HEIGHT 4           /* bottom panel row count */
#define SCORES_X 11               /* score panel column */
#define GAMEOVER_PROMPT_Y HEIGHT - 2  /* game over prompt row */
#define QUERY_SUFFIX ""           /* no extra query suffix on NABU */
#define ROLL_X WIDTH - 25         /* dice roll animation column */
#define TIMER_X 12                /* timer display column */
#define TIMER_NUM_OFFSET_X 0      /* timer digit horizontal offset */
#define TIMER_NUM_OFFSET_Y 0      /* timer digit vertical offset */

#define ICON_TEXT_CURSOR '>'     /* text mode selection pointer */
#define ICON_MARK '*'            /* confirmed hit marker */
#define ICON_MARK_ALT '+'        /* alternate hit marker */
#define ICON_PLAYER 'P'          /* player position glyph */
#define ICON_SPEC 'S'            /* spectator glyph */
#define ICON_CURSOR '#'          /* game grid cursor */
#define ICON_CURSOR_ALT '@'      /* alternate cursor glyph */
#define ICON_CURSOR_BLIP '+'     /* cursor blink frame glyph */

#define KEY_LEFT_ARROW  0xE1     /* NABU left arrow scancode */
#define KEY_LEFT_ARROW_2 'h'     /* vi-style left */
#define KEY_LEFT_ARROW_3 'H'     /* vi-style left, uppercase */

#define KEY_RIGHT_ARROW  0xE0    /* NABU right arrow scancode */
#define KEY_RIGHT_ARROW_2 'l'    /* vi-style right */
#define KEY_RIGHT_ARROW_3 'L'    /* vi-style right, uppercase */

#define KEY_UP_ARROW  0xE2      /* NABU up arrow scancode */
#define KEY_UP_ARROW_2 'k'      /* up */
#define KEY_UP_ARROW_3 'K'      /* up, uppercase */

#define KEY_DOWN_ARROW  0xE3    /* NABU down arrow scancode */
#define KEY_DOWN_ARROW_2 'j'    /* down */
#define KEY_DOWN_ARROW_3 'J'    /* down, uppercase */

#define KEY_RETURN    '\r'      /* enter / confirm */
#define KEY_ESCAPE    'q'       /* quit */
#define KEY_ESCAPE_ALT 'Q'      /* quit, uppercase */
#define KEY_SPACEBAR  ' '       /* fire / place / confirm */
#define KEY_BACKSPACE '\b'      /* delete character */

#endif
