#pragma once
#include <xcb/xcb.h>

struct bgh_ctx
{
    xcb_screen_t* screen;
    xcb_connection_t* conn;
    bgh_ctx();
    ~bgh_ctx();
};
