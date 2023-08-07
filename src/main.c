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
    int x;
    int y;
    int dx;
    int dy;
} props;

void initialize_properties() {
    props.x = (400 - TEXT_WIDTH) / 2;
    props.y = (240 - TEXT_HEIGHT) / 2;
    props.dx = 1;
    props.dy = 2;
}

struct {
    PDMenuItem *menu_item;
    PDMenuItem *menu_item_check;
    PDMenuItem *menu_item_options;
} menu_items;

int scale = 1;
int scaled = 1;
int inverted = 0;

void menu_item_check_callback(void *userdata) {
    PlaydateAPI *pd = (PlaydateAPI*) userdata;
    inverted = inverted ? 0 : 1;
    pd->display->setInverted(inverted);
    eventHandler(pd, kEventResume, 0);
}

void menu_item_callback(void *userdata) {
    PlaydateAPI *pd = (PlaydateAPI*) userdata;
//    scale = (2 * scale) % 8;
//    if (scale == 0)
//        scale = 1;
//
//    props.x = (400 - TEXT_WIDTH * scale) / 2 / scale;
//    props.y = (240 - TEXT_HEIGHT * scale) / 2 / scale;
//    props.dx = 1;
//    props.dy = 2;
//
//    pd->display->setScale(scale);
    eventHandler(pd, kEventTerminate, 0);
}

void menu_item_options_callback(void *userdata) {
    PlaydateAPI *pd = (PlaydateAPI*) userdata;
    int scaled_idx = pd->system->getMenuItemValue(menu_items.menu_item_options);
    scaled = 1 << scaled_idx;

    props.x = (400 - TEXT_WIDTH * scaled) / 2 / scaled;
    props.y = (240 - TEXT_HEIGHT * scaled) / 2 / scaled;
    props.dx = 1;
    props.dy = 2;

    pd->display->setScale(scaled);
    eventHandler(pd, kEventResume, 0);
}

void initialize_menu(PlaydateAPI *pd) {
    menu_items.menu_item_check = pd->system->addCheckmarkMenuItem("Invert", inverted, menu_item_check_callback, pd);
    const char *scales[] = {"1", "2", "4"};
    menu_items.menu_item_options = pd->system->addOptionsMenuItem("Scale", scales, 3, menu_item_options_callback, pd);
    menu_items.menu_item = pd->system->addMenuItem("Quit", menu_item_callback, pd);
}


int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
    PDButtons current, pushed, released;

//	(void)arg; // arg is currently only used for event = kEventKeyPressed
    pd->system->logToConsole("arg=%d", arg);

	if ( event == kEventInit )
	{
        pd->system->logToConsole("kEventInit\n");

		const char* err;
		font = pd->graphics->loadFont(font_path, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, font_path, err);

        initialize_properties();
        initialize_menu(pd);

        // Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and
        // doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
	}

//    if (event == kEventTerminate) {
//        pd->system->logToConsole("kEventTerminate\n");
//        return 0;
//    }

    if (event == kEventKeyReleased && props.dx < 0) {
        pd->system->logToConsole("kEventKeyReleased\n");
        props.dx = -props.dx;
    }

//    pd->system->logToConsole("Checking button state...\n");
//    pd->system->getButtonState(&current, &pushed, &released);
//    if (current || pushed || released) {
//        pd->system->logToConsole("Current=%d, Pushed=%d, Released=%d\n", current, pushed, released);
//    }

	return 0;
}


static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);
	pd->graphics->drawText("Hello World!", strlen("Hello World!"), kASCIIEncoding, props.x, props.y);

	props.x += props.dx;
	props.y += props.dy;
	
	if (props.x < 0 || props.x * scaled > (LCD_COLUMNS - TEXT_WIDTH * scaled))
		props.dx = -props.dx;
	
	if (props.y < 0 || props.y * scaled > (LCD_ROWS - TEXT_HEIGHT * scaled))
		props.dy = -props.dy;
        
	pd->system->drawFPS(0,0);

	return 1;
}

