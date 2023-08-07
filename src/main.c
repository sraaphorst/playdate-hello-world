/**
 * main.c
 *
 * By Sebastian Raaphorst, 2023.
 * Adapted from Playdate Hello World C example.
 */

#include <stdlib.h>
#include "pd_api.h"

static int update(void* userdata);
const char* font_path = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif

// The width and height of the text.
#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

// If the view is flipped and FLIP_BUTTONS is set, the buttons act normally, i.e. are not flipped.
#define FLIP_BUTTONS 1

// Speed increase for X and Y and maximums.
#define X_MULTIPLIER_DELTA 2
#define X_MULTIPLIER_MAX 6
#define Y_MULTIPLIER_DELTA 1
#define Y_MULTIPLIER_MAX 3

// The initial movement in each direction.
#define X_DELTA 2
#define Y_DELTA 1

// Properties of the view.
struct {
    int x, y;
    int dx, dy;
    int x_multiplier, y_multiplier;

    int scaled;
    int inverted;
    int flip_x, flip_y;
} props;

void initialize_properties() {
    props.x = (400 - TEXT_WIDTH) / 2;
    props.y = (240 - TEXT_HEIGHT) / 2;
    props.dx = 0;
    props.dy = 0;
    props.x_multiplier = 0;
    props.y_multiplier = 0;

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
//    props.dx = 1;
//    props.dy = 2;

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


void buttonHandler(PlaydateAPI *pd) {
    PDButtons pushed;
    pd->system->getButtonState(NULL, &pushed, NULL);

    // Reverse the direction if appropriate.
    // Even if we are flipped, we want to make the buttons act as they should.
    PDButtons modified = 0;
#ifdef FLIP_BUTTONS
    if (props.flip_x) {
        if (pushed & kButtonRight) modified |= kButtonLeft;
        if (pushed & kButtonLeft)  modified |= kButtonRight;
    } else
        modified |= (pushed & kButtonRight) | (pushed & kButtonLeft);
    if (props.flip_y) {
        if (pushed & kButtonDown) modified |= kButtonUp;
        if (pushed & kButtonUp)   modified |= kButtonDown;
    } else
        modified += (pushed & kButtonUp) | (pushed & kButtonDown);
#else
    modified = pushed;
#endif

    if ((modified & kButtonRight && props.dx < 0)
        || (modified & kButtonLeft && props.dx > 0))
        props.dx = -props.dx;
    if ((modified & kButtonDown && props.dy < 0)
        || (modified & kButtonUp && props.dy > 0))
        props.dy = -props.dy;
}

int min(int i1, int i2) {
    return i1 < i2 ? i1 : i2;
}

int max(int i1, int i2) {
    return i1 > i2 ? i1 : i2;
}

void crankHandler(PlaydateAPI *pd) {
    // If the crank is docked or there is no change, return.
    if (pd->system->isCrankDocked())
        return;
    float crank_change = pd->system->getCrankChange();
    if (crank_change == 0)
        return;

    // Change the speed depending on the angle of the crank.
    float crank_angle = pd->system->getCrankAngle();

    // If there is no movement, start the movement.
    // If the crank change is positive, move in positive direction.
    if (crank_change > 0 && !props.dx && !props.dy) {
        props.dx = X_DELTA;
        props.dy = Y_DELTA;
    }
    // If crank change is negative, move in negative direction.
    if (crank_change < 0 && !props.dx && !props.dy) {
        props.dx = -X_DELTA;
        props.dy = -Y_DELTA;
    }

    // If crank change is positive and has moved the crank from before 360 to after 0, accelerate.
    // Example: crank_angle = 5, crank_change = 10 -> change (was originally 355).
    // Example: crank_angle = 5, crank_change = 2 -> do not change (was originally 3).
    // Example: crank_angle = 359, crank_change = 10 -> do not change (was originally 349).
    if (crank_change > 0 && (crank_angle - crank_change) < 0) {
        props.x_multiplier = min(X_MULTIPLIER_MAX, props.x_multiplier + X_MULTIPLIER_DELTA);
        props.y_multiplier = min(Y_MULTIPLIER_MAX, props.y_multiplier + Y_MULTIPLIER_DELTA);
    }
    // If crank change is negative and has moved the crank from after 0 to before 360, decelerate.
    // Example: crank_angle = 355, crank_change = -10 -> change (was originally 5).
    // Example: crank_angle = 355, crank_change = -2 -> do not change (was originally 357).
    // Example: crank_angle = 1, crank_change = -10 -> do not change (was originally 11).
    if (crank_change < 0 && (crank_angle - crank_change) > 360) {
        props.x_multiplier = max(0, props.x_multiplier - X_MULTIPLIER_DELTA);
        props.y_multiplier = max(0, props.y_multiplier - Y_MULTIPLIER_DELTA);
    }
    pd->system->logToConsole("Crank change is %.2f, angle is %.2f", (double)crank_change, (double)crank_angle);
    pd->system->logToConsole("x_multiplier=%d, y_multiplier=%d", props.x_multiplier, props.y_multiplier);
}


int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t) {
	if (event == kEventInit) {
		const char* err;
		font = pd->graphics->loadFont(font_path, &err);
		
		if (font == NULL)
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, font_path, err);

        // Initialize the properties and the menu items.
        initialize_properties();
        initialize_menu(pd);

        // NOTE: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and
        // doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
	}

	return 0;
}


static int update(void* userdata) {
	PlaydateAPI* pd = userdata;

    // Check for directional or speed changes based on buttons and crank.
    buttonHandler(pd);
    crankHandler(pd);

	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);
	pd->graphics->drawText("Hello World!", strlen("Hello World!"), kASCIIEncoding, props.x, props.y);

    // Change the x and y coordinates, flipping the speed if necessary.
	props.x += props.dx * props.x_multiplier;
	props.y += props.dy * props.y_multiplier;
	if (props.x < 0 || props.x * props.scaled > (LCD_COLUMNS - TEXT_WIDTH * props.scaled))
		props.dx = -props.dx;
	if (props.y < 0 || props.y * props.scaled > (LCD_ROWS - TEXT_HEIGHT * props.scaled))
		props.dy = -props.dy;

    // Don't display the frames per second.
    // pd->system->drawFPS(0,0);

	return 1;
}
