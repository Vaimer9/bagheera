#pragma once
#include <concepts>
#include <cstdint>
#include <vector>
#include <xcb/xcb.h>
#include <string>

struct bgh_ctx
{
    xcb_screen_t* screen;
    xcb_connection_t* conn;
    int root_window;
    int overlay_window;

    bgh_ctx();
    ~bgh_ctx();
};

struct bgh_xcb_window
{
    static std::vector<bgh_xcb_window*> global_windows;

    bgh_ctx& ctx;
    int id;
    int x, y;
    int pixmap; // Maybe a Pixmap class in the future?
    uint32_t h, w;

    bgh_xcb_window(bgh_ctx& ctx);
    int set_pixmap(bgh_ctx& ctx); // Generates the id on its own
    int set_pixmap(int pix_id);
};

// Error handling
void bgh_handle(xcb_generic_error_t* error, std::string msg); // This will FREE the memory
void bgh_handle(void* error, std::string msg);
void bgh_handle(xcb_generic_error_t* error); // Default error message
