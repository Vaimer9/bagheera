#pragma once
#include <concepts>
#include <cstdint>
#include <vector>
#include <xcb/xcb.h>
#include <string>
#include <optional>
#include <xcb/render.h>

struct bgh_xcb_window;

struct bgh_ctx
{
    xcb_screen_t* screen;
    xcb_connection_t* conn;
    std::vector<bgh_xcb_window*> global_windows;
    xcb_render_pictforminfo_t* rgb_format;
    xcb_render_pictforminfo_t* rgba_format;
    bgh_xcb_window* root_window;
    bgh_xcb_window* overlay_window;

    bgh_ctx();
    ~bgh_ctx();
};

struct bgh_xcb_window
{
    bgh_ctx& ctx;
    int id;
    int x, y; // TODO: Can this be negative?
    int pixmap; // Maybe a Pixmap class in the future?
    uint32_t h, w;
    uint32_t border_w;
    xcb_render_picture_t picture;

    bgh_xcb_window(bgh_ctx& ctx);
    bgh_xcb_window(bgh_ctx& ctx, int id);
    int set_pixmap(bgh_ctx& ctx); // Generates the id on its own
    int set_pixmap(bgh_ctx& ctx, int pix_id);
    int set_picture(bgh_ctx& ctx, xcb_render_pictforminfo_t* render_format);
    void add_global();
    void set_geometry(xcb_get_geometry_reply_t* geometry);
};

// This is a very tedious process
// So a different function
std::array<xcb_render_pictforminfo_t*, 2> bgh_get_pict_formats(bgh_ctx& ctx);

// Error handling
void bgh_handle(xcb_generic_error_t* error, std::string msg); // This will FREE the memory
void bgh_handle(void* error, std::string msg);
void bgh_handle(xcb_generic_error_t* error); // Default error message
