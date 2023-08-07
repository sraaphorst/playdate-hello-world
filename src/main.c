//
//  main.c
//  Extension
//
//  Created by Dave Hayden on 7/30/14.
//  Copyright (c) 2014 Panic, Inc. All rights reserved.
//

#include <stdlib.h>

#include "pd_api.h"

static int update(void* userdata);
const char* font_path = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif

// To reflect the changes in this, you must do:
// cd build ; cmake .. ; make
#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

struct {
    int x, y;
    int dx, dy;

    int scaled;
    int inverted;
    int flip_x, flip_y;
} props;

void initialize_properties() {
    props.x = (400 - TEXT_WIDTH) / 2;
    props.y = (240 - TEXT_HEIGHT) / 2;
    props.dx = 1;
    props.dy = 2;

    props.inverted = 0;
    props.scaled = 1;
    props.flip_x = 0;
    props.flip_y = 0;
}

struct {
    PDMenuItem *menu_item_invert;
    PDMenuItem *menu_item_scale;
    PDMenuItem *menu_item_flip;
} menu_items;

void menu_item_invert_callback(void *userdata) {
    PlaydateAPI *pd = (PlaydateAPI*) userdata;
    props.inverted = props.inverted ? 0 : 1;
    pd->display->setInverted(props.inverted);
    eventHandler(pd, kEventResume, 0);
}

void menu_item_scale_callback(void *userdata) {
    PlaydateAPI *pd = (PlaydateAPI*) userdata;

    // Returns 0 -> 1, 1 -> 2, 2-> 4.
    int scale_idx = pd->system->getMenuItemValue(menu_items.menu_item_scale);
    props.scaled = 1 << scale_idx;

    props.x = (400 - TEXT_WIDTH * props.scaled) / 2 / props.scaled;
    props.y = (240 - TEXT_HEIGHT * props.scaled) / 2 / props.scaled;
    props.dx = 1;
    props.dy = 2;

    pd->display->setScale(props.scaled);
    eventHandler(pd, kEventResume, 0);
}

void menu_item_flip_callback(void *userdata) {
    PlaydateAPI *pd = (PlaydateAPI*) userdata;

    // Returns 0 -> no flip, 1 -> flip x, 2 -> flip y, 3 -> flip both.
    int flip_idx = pd->system->getMenuItemValue(menu_items.menu_item_flip);
    props.flip_x = (flip_idx % 2) == 1;
    props.flip_y = flip_idx >= 2;

    pd->display->setFlipped(props.flip_x, props.flip_y);
    eventHandler(pd, kEventResume, 0);
}

void initialize_menu(PlaydateAPI *pd) {
    menu_items.menu_item_invert = pd->system->addCheckmarkMenuItem("Invert", props.inverted,
                                                                   menu_item_invert_callback, pd);

    const char *scales[] = {"1", "2", "4"};
    menu_items.menu_item_scale = pd->system->addOptionsMenuItem("Scale", scales, 3, menu_item_scale_callback, pd);

    const char *flips[] = {"-", "X", "Y", "XY"};
    menu_items.menu_item_flip = pd->system->addOptionsMenuItem("Flip", flips, 4, menu_item_flip_callback, pd);
}


int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
	if (event == kEventInit) {
        pd->system->logToConsole("kEventInit event\n");

		const char* err;
		font = pd->graphics->loadFont(font_path, &err);
		
		if (font == NULL)
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, font_path, err);

        initialize_properties();
        initialize_menu(pd);

        // NOTE: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and
        // doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
	}

    if (event == kEventKeyReleased && props.dx < 0) {
        pd->system->logToConsole("kEventKeyReleased event\n");
        props.dx = -props.dx;
    }

	return 0;
}


static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);
	pd->graphics->drawText("Hello World!", strlen("Hello World!"), kASCIIEncoding, props.x, props.y);

    // Change the x and y coordinates, flipping the speed if necessary.
	props.x += props.dx;
	props.y += props.dy;
	if (props.x < 0 || props.x * props.scaled > (LCD_COLUMNS - TEXT_WIDTH * props.scaled))
		props.dx = -props.dx;
	if (props.y < 0 || props.y * props.scaled > (LCD_ROWS - TEXT_HEIGHT * props.scaled))
		props.dy = -props.dy;

    // Don't display the frames per second.
    // pd->system->drawFPS(0,0);

	return 1;
}
