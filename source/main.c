// Include libraries
#include <stdbool.h>
#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "common.h"
#include "sound.h"
#include "texture.h"
#include "room.h"

// Sound variable
struct sound *home;

// Room variables
int room      = 0; // General info
int roomEnter = 0; // Entrances

struct exit *next_exit;
float roomTimer = 255;
// Player variables
// TODO: Make a struct for the player.
// int player    = 0; // General info

// This is one thing that could go into a player struct.
position player_pos;
position camera_pos = {0, 0};

//variables that are used for movement key presses
//  TODO: Consolidate all input variables
int mkey_right = 0;
int mkey_up = 0;
int mkey_left = 0;
int mkey_down = 0;

//Movement variables
float mspeed = .5; // Speed multiplier
float hsp = 0; // Horizontal speed
float vsp = 0; // Vertical speed

// Text variables
int textWidth  = 0; // Width
int textHeight = 0; // Height

// Timing variables
int   prevTime = 0; // Previous time
float dt       = 0; // Movement timing
float sprTimer = 0; // Sprite timing

// Textures and fonts
sf2d_texture *curr_tex;
sftd_font    *font;

// Multidirectional array to store all player's walking textures
// This is one thing that could go into a player struct.
sf2d_texture* tex_arr_friskWalk[4][4];

const char *friskFilenames[4][4] = {
    {"friskRight0", "friskRight1", "friskRight0", "friskRight1"},// Right
    {"friskFace0", "friskFace1", "friskFace2", "friskFace3"}, // Down
    {"friskLeft0", "friskLeft1", "friskLeft0", "friskLeft1"}, // Left
    {"friskBack0", "friskBack1", "friskBack2", "friskBack3"}  // Up
};

enum direction { // I request that you change FORWARD and BACK to UP and DOWN, since forward to the player means back to the in game character. It's confusing to me.
    RIGHT = 0,
    BACK,
    LEFT,
    FORWARD
};

// TODO: This is one thing that could go into a player struct.
enum direction playerDir = FORWARD; // Direction

// Easter Egg variables
bool easterEgg  = false;
int  easterPage = 0;
#define MAX_PAGE 1

// Timer for the player's speed
void timerStep(void) {
    int currTime = osGetTime();

    // Set and calculate the timer
    dt = currTime - prevTime;
    dt *= 0.15; // TODO: Why 0.15?

    // We don't want to dt to be negative.
    // TODO: Can this ever actually happen?
    //Sure can, it's not supposed to though. Better yet... DT shouldn't be a large value either
    if (dt < 0) dt = 0;
    if (abs(dt) > 1) dt = 0.0001; //Just in case.

    // Set previous time to the current time
    prevTime = currTime;
}

void init(void) {
    // Starting services
    sf2d_init();
    sf2d_set_vblank_wait(0);
    sftd_init();
    srvInit();
    aptInit();
    hidInit();
    audio_init();
    //romfsInit();

    // Configuring the right font to use (8bitoperator), and its proprieties
    font = sftd_load_font_file("font/eightbit.ttf");

    // Configuring graphics in general (images, textures, etc)
    sf2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));

    /* Load Frisk textures
       Loop over every element in tex_arr_friskWalk and load the PNG buffer. */

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            tex_arr_friskWalk[i][j] = loadTexture(friskFilenames[i][j]);
        }
    }

    room_init();

    // Reusing 'i' from above.
    // Load room textures.
    for (int i = 0; i < 3; ++i) fillTexture(&rooms[i].bg);

    // TODO: Add actual save loading logic. For now, just assume this room.
    player_pos = rooms[room].exits[0].entrance;

    // Play music
    home = sound_create(BGM);
    if (home != NULL) audio_load_ogg("sound/music/house1.ogg", home);
    else home->status = -1;

    timerStep();
}

void render(void) {
    // Start frame on the top screen
    sf2d_start_frame(GFX_TOP, GFX_LEFT);

    // Draw the background (or in this case, the room)
    sf2d_draw_texture(rooms[room].bg.tex,
                      rooms[room].bg.pos.x - (int)camera_pos.x,
                      rooms[room].bg.pos.y - (int)camera_pos.y);

    // Draw the player's sprite
    sf2d_draw_texture(curr_tex,
                      (int)player_pos.x - (int)camera_pos.x,
                      (int)player_pos.y - (int)camera_pos.y);

    sf2d_draw_rectangle(0, 0, 800, 240, RGBA8(0x00, 0x00, 0x00, 0xFF - (int)roomTimer));

    // End frame
    sf2d_end_frame();

    // Start frame on the bottom screen
    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);

    // If the easter egg variable is true, then activate it
    if (easterEgg) {
        // Draw the easter egg
        sftd_draw_text(font, 10, 140,  RGBA8(255, 0, 0, 255), 16, "* You IDIOT.");
        sftd_draw_text(font, 10, 170,  RGBA8(255, 255, 255, 255), 16, "* Nah, this is just");
        sftd_draw_text(font, 10, 200,  RGBA8(255, 255, 255, 255), 16, "   a simple test.");
        int y = -10;
        // Debug stuff
        switch (easterPage) {
            case 0:
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 0, 0, 255), 12, "FPS: %f", sf2d_get_fps());
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 0, 0, 255), 12, "Sprite Timer: %f", sprTimer);
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 255, 255, 255), 12, "Player X: %f, Y: %f", player_pos.x, player_pos.y);
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 255, 255, 255), 12, "Camera X: %f, Y: %f", camera_pos.x, camera_pos.y);
                break;
            case 1:
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 255, 255, 255), 12, "Block Position: %lu", home->block_pos);
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 255, 255, 255), 12, "Block: %u", home->block);
                sftd_draw_textf(font, 10, y+=20, RGBA8(255, 0, 0, 255), 12, "Status: %li", home->status);
                break;
        }
    };
    // End frame
    sf2d_end_frame();
}

inline float fclamp(float value, float min, float max) {
    return fmin(max, fmax(value, min));
}

// Main part of the coding, where everything works (or not)
int main(void) {
    init();

    // Main loop
    while (aptMainLoop()) {

        // Verify button presses
        hidScanInput();

        // Unsigned variables for different types of button presses
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        // u32 kUp = hidKeysUp();

        // Exit homebrew
        if (kDown & KEY_START) {
            break;
        }

        // Activate first easter egg
        else if (kDown & KEY_SELECT) {
            easterEgg = !easterEgg;
        }

        // Change pages for the easterEgg/debug menu.
        else if (kDown & KEY_R) {
            if (++easterPage > MAX_PAGE) easterPage = 0;
        }
        else if (kDown & KEY_L) {
            if (--easterPage < 0) easterPage = MAX_PAGE;
        }


        timerStep();

        // If no movement, set the sprite timer to 0
        //  TODO: I think this part isn't needed, or is wrong lol.
        // The following says that is a key is pressed (not held) reset the sprite timer
        // Contradicts the comment.
        if (kDown & KEY_UP || kDown & KEY_DOWN || kDown & KEY_LEFT || kDown & KEY_RIGHT) {
            sprTimer = 0;
        }

        // Reset horizontal and vertical speeds
        vsp = 0;
        hsp = 0;

        // Player movement (pretty easy to understand)
        // TODO: Would it be possible to make this less... iffy? --- Sure. :P
        //Thank GML for being VERY similar to C++ :)
        //I'll explain my thought process clearly here.
        //I'm going to seperate input, movement, and direction into different segments.
        //  input
        mkey_right = (kHeld & KEY_RIGHT); //assuming booleans work how I think they do here.. this should make mkey_right 1 when the right key is pressed.
        mkey_up = -(kHeld & KEY_BACK);  // -1 when held.
        mkey_left = -(kHeld & KEY_LEFT);  // ^^^^
        mkey_down = (kHeld & KEY_FORWARD);  // 1 when held ... also why FORWARD/BACK?? It's confusing.

        //  direction - Too lazy to write explaination. Can do it later if you like. I'm sleepy.
        if (dir_key > 0) { // The original direction key isn't pressed:
          if ((kDown & KEY_RIGHT) && dir_key == 0)) ||  ((kDown & KEY_BACK) && dir_key == 1)) || ((kDown & KEY_LEFT) && dir_key == 2)) || ((kDown & KEY_FORWARD) && dir_key == 3)) {
            dir_key = -1;
          }
        }
        if (dir_key == -1) {
          if (kDown & KEY_RIGHT) {
            dir_key = 0;
          }
          else if (kDown & KEY_BACK) {
            dir_key = 1;
          }
          else if (kDown & KEY_LEFT) {
            dir_key = 2;
          }
          else if (kDown & KEY_FORWARD) {
            dir_key = 3;
          }
          else { dir_key = -1; }
        }
        if (dir_key > 0) {
          player_dir = dir_key;
        }

        //  movement
        hsp = (mkey_left + mkey_right) * mspeed; //set speed for easier movement speed modifying.
        vps = (mkey_up + mkey_down) * mspeed;
        //  TODO: vvvvv This isn't in Undertale . But it's a feature most top down games have. Feel free to remove it. vvvvv
        if (abs(vsp) > 0) && (abs(hsp) > 0) { //halve horizonal and vertical speeds when moving diagonally.
          hsp *= .5;
          vsp *= .5;
        }

        // Movement calculation... AND proper room colision.
        // TODO: Consider a function for translating and/or clamping coordinates directly?
        player_pos.x = fclamp(player_pos.x + hsp * dt,
                              rooms[room].collision[0].x,
                              rooms[room].collision[1].x);

        player_pos.y = fclamp(player_pos.y + vsp * dt,
                              rooms[room].collision[0].y,
                              rooms[room].collision[1].y);

        // Scrolling calculation.
        // TODO: Make these constants better/customizable.
        if (player_pos.x - camera_pos.x >= 300) {
            camera_pos.x = player_pos.x - 300;
        }
        else if (player_pos.x - camera_pos.x <= 100) {
            camera_pos.x = player_pos.x - 100;
        }
        camera_pos.x = fclamp(camera_pos.x, 0, rooms[room].scroll_max.x);

        if (player_pos.y - camera_pos.y >= 200) {
            camera_pos.y = player_pos.y - 200;
        }
        else if (player_pos.y - camera_pos.y <= 50) {
            camera_pos.y = player_pos.y - 50;
        }
        camera_pos.y = fclamp(camera_pos.y, 0, rooms[room].scroll_max.y);

        // Player sprites
        if (hsp == 0 && vsp == 0) curr_tex = tex_arr_friskWalk[playerDir][0];

        else curr_tex = tex_arr_friskWalk[playerDir][(int)floor(sprTimer)];

        // Sprite animation timer
        // TODO: Why .15 * .03 * actual time?
        sprTimer += (.03 * dt);

        while (sprTimer >= 4) {
            sprTimer -= 4;
        }

        if (!next_exit){
            if (roomTimer < 255) {
                roomTimer = fmin(roomTimer + (4 * dt), 255);
            }
            next_exit = exit_room(room, &player_pos);
        }
        else {
            roomTimer -= 4 * dt;
            if (roomTimer <= 0) {
                room = next_exit->room_id;
                player_pos = next_exit->entrance;
                next_exit = NULL;
                roomTimer = 0;
            }
        }

        render();

        // Swap sf2d framebuffers and wait for VBlank
        sf2d_swapbuffers();
    }

    // Free images/textures/fonts from memory

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            sf2d_free_texture(tex_arr_friskWalk[i][j]);
        }
    }

    for (int i = 0; i < 3; ++i) {
        sf2d_free_texture(rooms[i].bg.tex);
    }

    sftd_free_font(font);

    // Exit services
    sf2d_fini();
    sftd_fini();
    sound_stop(home);
    audio_stop();
    hidExit();
    aptExit();
    srvExit();

    return 0;
}
