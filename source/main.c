// Include libraries
#include <stdbool.h>
#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <sfil.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "eightbit_ttf.h"

// Load images
#include "friskBack0_png.h"
#include "friskBack1_png.h"
#include "friskBack2_png.h"
#include "friskBack3_png.h"
#include "friskFace0_png.h"
#include "friskFace1_png.h"
#include "friskFace2_png.h"
#include "friskFace3_png.h"
#include "friskLeft0_png.h"
#include "friskLeft1_png.h"
#include "friskRight0_png.h"
#include "friskRight1_png.h"
#include "torielHouse1_png.h"
#include "torielHouse2_png.h"
#include "torielHouse31_png.h"
#include "torielHouse32_png.h"
#include "torielHouse4_png.h"
#include "torielHouse5_png.h"
#include "torielHouse6_png.h"

// Sound/Music stuff
u8* buffer;				// Buffering audio file
u32 size;				// Audio file size

// Audio load (play) and stop voids
static void audio_load (const char *audio);
static void audio_stop (void);

// Room variables
int room 		= 1;	// General info
int roomEnter 	= 0;	// Entrances
float room_x1;			// X1 coordinate
float room_y1;			// Y1 coordinate
float room_x2;			// X2 coordinate
float room_y2;			// Y2 coordinate

// Player variables
int player 		= 0;		// General info
int playerDir	= 0;	// Direction
float player_x;			// X coordinate
float player_y;			// Y coordinate
float hsp 		= 0;	// Horizontal speed
float vsp 		= 0;	// Vertical speed

// Text variables
int textWidth 	= 0;	// Width
int textHeight 	= 0;	// Height

// Timing variables
int prevTime 	= 0;	// Previous time
int currTime 	= 0;	// Current time
float dt 		= 0;	// Movement timing
double sprTimer = 0;	// Sprite timing

// Textures and fonts
sf2d_texture 	*curr_tex;
sf2d_texture 	*curr_room;
sf2d_texture 	*tex_torielHouse1;
sf2d_texture 	*tex_torielHouse2;
sf2d_texture 	*tex_torielHouse31;
sf2d_texture 	*tex_torielHouse32;
sf2d_texture 	*tex_torielHouse4;
sf2d_texture 	*tex_torielHouse5;
sf2d_texture 	*tex_torielHouse6;
sftd_font 		*font;

// Multidirectional array to store all player's walking textures
sf2d_texture* tex_arr_friskWalk [4] [4];

const u8* friskFilenames [4] [4] = {

	{friskRight0_png, friskRight1_png, friskRight0_png, friskRight1_png}, 		// Right
	{friskFace0_png, friskFace1_png, friskFace2_png, friskFace3_png,},			// Down
	{friskLeft0_png, friskLeft1_png, friskLeft0_png, friskLeft1_png},			// Left
	{friskBack0_png, friskBack1_png, friskBack2_png, friskBack3_png}			// Up

};

// Constant variables for the player's walking textures
const int FRISK_RIGHT 	= 0;
const int FRISK_FORWARD = 1;
const int FRISK_LEFT 	= 2;
const int FRISK_BACK 	= 3;

// Easter Egg variables
bool easterEgg1 = false;

void init () {

	// Starting services
	sf2d_init ();
	sf2d_set_vblank_wait (0);
	sftd_init ();
	srvInit ();
	aptInit ();
	hidInit (NULL);

	// Starting audio service
	csndInit ();

	// Configuring the right font to use (8bitoperator), and its proprieties
	font = sftd_load_font_mem (eightbit_ttf, eightbit_ttf_size);

	// Configuring graphics in general (images, textures, etc)
	sf2d_set_clear_color (RGBA8 (0x00, 0x00, 0x00, 0xFF));
	tex_torielHouse1 	= sfil_load_PNG_buffer(torielHouse1_png, SF2D_PLACE_RAM);
	tex_torielHouse2 	= sfil_load_PNG_buffer(torielHouse2_png, SF2D_PLACE_RAM);
	tex_torielHouse31 	= sfil_load_PNG_buffer(torielHouse31_png, SF2D_PLACE_RAM);
	tex_torielHouse32 	= sfil_load_PNG_buffer(torielHouse32_png, SF2D_PLACE_RAM);
	tex_torielHouse4 	= sfil_load_PNG_buffer(torielHouse4_png, SF2D_PLACE_RAM);
	tex_torielHouse5 	= sfil_load_PNG_buffer(torielHouse5_png, SF2D_PLACE_RAM);
	tex_torielHouse6 	= sfil_load_PNG_buffer(torielHouse6_png, SF2D_PLACE_RAM);

	// Load Frisk textures
	// Loop over every element in tex_arr_friskWalk and load the PNG buffer
	// For some reason, here you have to declare the loop variables before the loop
	
	int i, j;
	
	for (i = 0; i < 4; i++) { 
		
		for (j = 0; j < 4; j++) {
			
			tex_arr_friskWalk [i] [j] = sfil_load_PNG_buffer (friskFilenames [i] [j], SF2D_PLACE_RAM);
			
		}
		
	}
	
	// Play music
	audio_load("sound/music/home.bin");
	
}

void render () {
	
	// Start frame on the top screen
	sf2d_start_frame (GFX_TOP, GFX_LEFT);
	
	// Draw the background (or in this case, the room)
	sf2d_draw_texture (curr_room, 40, 0);
	
	// Draw the player's sprite
	sf2d_draw_texture (curr_tex, (int) player_x, (int) player_y);
	
	// End frame
	sf2d_end_frame ();
	
	// If the easter egg variable is true, then activate it
	if (easterEgg1) {
		
		// Start frame on the bottom screen
		sf2d_start_frame (GFX_BOTTOM, GFX_LEFT);
		
		// Draw the easter egg
		sftd_draw_text (font, 10, 140,  RGBA8 (255, 0, 0, 255), 16, "* You IDIOT.");
		sftd_draw_text (font, 10, 170,  RGBA8 (255, 255, 255, 255), 16, "* Nah, this is just");
		sftd_draw_text (font, 10, 200,  RGBA8 (255, 255, 255, 255), 16, "   a simple test.");
		
		// Debug stuff
		sftd_draw_textf (font, 10, 10, RGBA8 (255, 0, 0, 255), 12, "FPS: %f", sf2d_get_fps ());
		sftd_draw_textf (font, 10, 30, RGBA8 (255, 0, 0, 255), 12, "Sprite Timer: %f", sprTimer);
		
		// End frame
		sf2d_end_frame ();
		
	};
	
}

// Timer for the player's speed
void timerStep () {
	
	// Set previous time as current time
	prevTime = currTime;
	
	// Set current time as the 3DS' OS RTC
	currTime = osGetTime();
	
	// Set and calculate the timer
	dt = currTime - prevTime;
	dt *= 0.15;
	
	// We don't want to dt to be negative.
	if (dt < 0) {
		
		dt = 0;
		
	}
	
}

// Main part of the coding, where everything works (or not)
int main (int argc, char **argv) {

	init();

	// Main loop
	while (aptMainLoop ()) {
	
	// Verify button presses
	hidScanInput ();
	
	// Unsigned variables for different types of button presses
	u32 kDown = hidKeysDown ();
	u32 kHeld = hidKeysHeld();
	// u32 kUp = hidKeysUp();
	
	// Exit homebrew
	if (kDown & KEY_START) {
		
		break;
		
	}
	
	// Activate first easter egg
	else if (kDown & KEY_SELECT) {
		
		easterEgg1 = true;
		
	}
	
	timerStep ();
	
	// If no movement, set the sprite timer to 0
	if (kDown & KEY_UP || kDown & KEY_DOWN || kDown & KEY_LEFT || kDown & KEY_RIGHT) {
		
		sprTimer = 0;
		
	}
	
	// Reset horizontal and vertical speeds
	vsp = 0; 
	hsp = 0;
	
	// Player movement (pretty easy to understand)
	if (kHeld & KEY_UP) {
		
		if (!(kHeld & KEY_DOWN)) {
			
			vsp 		= -.5;			// Vertical speed to negative .5
			playerDir 	= FRISK_BACK;	// Player direction = back
			
		}
		
	}
	
	if (kHeld & KEY_DOWN) {
		
		vsp 		= .5;				// Vertical speed to .5
		playerDir 	= FRISK_FORWARD;	// Player direction = up
		
	}
	
	if (kHeld & KEY_LEFT) {
		
		if (!(kHeld & KEY_RIGHT)) {
			
			hsp 		= -.5;			// Vertical speed to negative .5
			playerDir 	= FRISK_LEFT;	// Player direction = left
			
		}
		
	}
	
	if (kHeld & KEY_RIGHT) {
		
		hsp 		= .5;				// Vertical speed to .5
		playerDir 	= FRISK_RIGHT;		// Player direction = right
		
	}
	
	// Diagonal movement speed fix
	if (vsp != 0) {
		
		if (hsp != 0) {
			
			vsp *= .8;
			hsp *= .8;
			
		}
		
	}
	
	// Collision test before movement
	if ((player_x + hsp) >= room_x2) {
		
		hsp = 0;
		
	}
	
	if ((player_x + hsp) <= room_x1) {
		
		hsp = 0;
		
	}
	
	if ((player_y + vsp) >= room_y2) {
		
		vsp = 0;
		
	}
	
	if ((player_y + vsp) <= room_y1) {
		
		vsp = 0;
		
	}
	
	// Actual movement calculation
	player_x += hsp * dt;
	player_y += vsp * dt;
	
	// Player sprites
	if (hsp == 0 && vsp == 0) {
		
		curr_tex = tex_arr_friskWalk [playerDir] [0];
		
	}
	
	else {
		
		curr_tex = tex_arr_friskWalk [playerDir] [(int) floor (sprTimer)];
		
	}
	
	//Sprite animation timer
	sprTimer += (.03 * dt);
	
	while (sprTimer >= 4) {
		
		sprTimer -= 4;
		
	}
	
	// Localization/rooms
	if (room == 1) {
		
		if (roomEnter == 0) {
			
			curr_room = tex_torielHouse1;
			player_x 	= 190;
			player_y 	= 160;
			room_x1 	= 77;
			room_y1 	= 60;
			room_x2 	= 305;
			room_y2 	= 188;
			roomEnter	= 255;
			
		}
		
		if (roomEnter == 1) {
			
			curr_room = tex_torielHouse1;
			player_x 	= 78;
			player_y 	= 160;
			room_x1 	= 77;
			room_y1 	= 60;
			room_x2 	= 305;
			room_y2 	= 188;
			roomEnter 	= 255;
			
		}
		
		if (roomEnter == 2) {
			
			curr_room = tex_torielHouse1;
			player_x 	= 304;
			player_y 	= 160;
			room_x1 	= 77;
			room_y1 	= 60;
			room_x2 	= 305;
			room_y2 	= 188;
			roomEnter 	= 255;
			
		}
		
		if (player_y >= 145 && player_y <= 195 && player_x <= 78 && playerDir == FRISK_LEFT) { // this needs work!
			
			room 		= 2;
			roomEnter 	= 0;
			
		}
		
		if (player_y >= 145 && player_y <= 195 && player_x >= 281 && playerDir == FRISK_RIGHT) { // this needs work!
			
			room 		= 3;
			roomEnter 	= 0;
			
		}
		
	}
	
	if (room == 2) {
		
		if (roomEnter == 0) {
			
			curr_room = tex_torielHouse2;
			player_x 	= 319;
			player_y 	= 160;
			room_x1 	= 60;
			room_y1 	= 69;
			room_x2 	= 320;
			room_y2 	= 190;
			roomEnter 	= 255;
			
		}
		
		if (player_y >= 145 && player_y <= 195 && player_x >= 319 && playerDir == FRISK_RIGHT) { // this needs work!
			
			room = 1;
			roomEnter 	= 1;
			
		}
		
	}
	
	if (room == 3) {
		
		if (roomEnter == 0) {
			
			curr_room = tex_torielHouse31;
			player_x 	= 41;
			player_y 	= 131;
			room_x1 	= 40;
			room_y1 	= 116;
			room_x2 	= 340;
			room_y2 	= 156;
			roomEnter 	= 255;
			
		}
		
		if (roomEnter == 1) {
			
			curr_room = tex_torielHouse31;
			player_x 	= 339;
			player_y 	= 131;
			room_x1 	= 40;
			room_y1 	= 116;
			room_x2 	= 340;
			room_y2 	= 156;
			roomEnter 	= 255;
			
		}
		
		if (player_y >= 116 && player_y <= 156 && player_x <= 41 && playerDir == FRISK_LEFT) { // this needs work!
			
			room 		= 1;
			roomEnter 	= 2;
			
		}
		
		if (player_y >= 116 && player_y <= 156 && player_x >= 339 && playerDir == FRISK_RIGHT) { // this needs work!
			
			room 		= 4;
			roomEnter 	= 0;
			
		}
		
	}

	render ();
	
	// Swap sf2d framebuffers and wait for VBlank
	sf2d_swapbuffers ();
	
}

	// Free images/textures/fonts from memory
	int i, j;
	
	for (i = 0; i < 4; i++) { 
		
		for (j = 0; j < 4; j++) {
			
			sf2d_free_texture(tex_arr_friskWalk[i][j]);
			
		}
		
	}

	sf2d_free_texture (tex_torielHouse1);
	sf2d_free_texture (tex_torielHouse2);
	sf2d_free_texture (tex_torielHouse31);
	sf2d_free_texture (tex_torielHouse32);
	sf2d_free_texture (tex_torielHouse4);
	sf2d_free_texture (tex_torielHouse5);
	sf2d_free_texture (tex_torielHouse6);
	sftd_free_font (font);

	// Exit services
	sf2d_fini ();
	sftd_fini ();
	audio_stop ();
	csndExit ();
	hidExit ();
	aptExit ();
	srvExit ();

	return 0;
}

// Audio load/play
void audio_load (const char *audio) {
	
	FILE *file = fopen (audio, "rb");
	fseek (file, 0, SEEK_END);
	off_t size = ftell (file);
	fseek (file, 0, SEEK_SET);
	buffer = linearAlloc (size);
	off_t bytesRead = fread (buffer, 1, size, file);
	fclose (file);
	csndPlaySound (8, SOUND_FORMAT_16BIT | SOUND_REPEAT, 44100, 1, 0, buffer, buffer, size);
	
}

// Audio stop
void audio_stop (void) {
	
	csndExecCmds (true);
	CSND_SetPlayState (0x8, 0);
	// memset (buffer, 0, size);
	GSPGPU_FlushDataCache (NULL, buffer, size);
	linearFree (buffer);
	
}
