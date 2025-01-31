#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <utils.hpp>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <iostream>
#include <stdlib.h>
#include <xcb/render.h>

bgh_ctx::bgh_ctx()
{
    this->conn = xcb_connect(NULL, NULL);
    this->screen = xcb_setup_roots_iterator(xcb_get_setup(this->conn)).data;

    // this->root_window = this->screen->root;
    this->root_window = new bgh_xcb_window(*this);
    // Part of the initializing step
    // TODO: Make this some kind of function, right now it is not explicit
    // TODO: Multi-screen support
    xcb_composite_redirect_subwindows(conn, screen->root, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

    uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
    xcb_void_cookie_t cookie =
        xcb_change_window_attributes_checked(conn, screen->root, XCB_CW_EVENT_MASK, values);
    bgh_handle(xcb_request_check(conn, cookie), "Setting the flags failed");

    this->overlay_window = new bgh_xcb_window(*this);
    // Get the overlay window from x11
    xcb_composite_get_overlay_window_cookie_t overlay_cookie
        = xcb_composite_get_overlay_window(this->conn, this->root_window->id);
    xcb_composite_get_overlay_window_reply_t* overlay_reply
        = xcb_composite_get_overlay_window_reply(this->conn, overlay_cookie, nullptr);
    if (overlay_reply)
    {
        this->overlay_window->id = overlay_reply->overlay_win;
        free(overlay_reply);
    }
    xcb_map_window(this->conn, this->overlay_window->id);

    // Get the rendering formats for opaque and transparent drawings
    auto formats = bgh_get_pict_formats(*this);
    this->rgb_format = formats[0];
    this->rgba_format = formats[1];

    // The root window picture will probably stay unused, but its better to keep it anyways
    xcb_render_picture_t root_pic = xcb_generate_id(this->conn);
    xcb_render_create_picture(this->conn, root_pic, this->root_window->id, rgb_format->id, 0, nullptr);
    this->root_window->picture = root_pic;

    // NOTE: We are not using the .set_picture() method on root and overlay window because
    // it is not required to make pixmaps for overlay and root
    // so we can just use the window data to make a picture
    // however this is slower for normal windows
    xcb_render_picture_t overlay_pic = xcb_generate_id(this->conn);
    xcb_render_create_picture(this->conn, overlay_pic, this->overlay_window->id, rgba_format->id, 0, nullptr);
    this->overlay_window->picture = overlay_pic;

    xcb_flush(conn);
}

bgh_ctx::~bgh_ctx()
{
    xcb_composite_release_overlay_window(this->conn, this->overlay_window->id);
    xcb_disconnect(conn);

    delete this->root_window;
    delete this->overlay_window;
    delete this->rgb_format;
    delete this->rgba_format;
}

bgh_xcb_window::bgh_xcb_window(bgh_ctx& ctx):
    ctx(ctx)
{
    ctx.global_windows.push_back(this); // Add self to the total window list
    this->id = xcb_generate_id(ctx.conn); // Generate an id for self
}

bgh_xcb_window::bgh_xcb_window(bgh_ctx& ctx, int id):
    ctx(ctx)
{
    this->id = id;
}

void bgh_xcb_window::add_global()
{
    ctx.global_windows.push_back(this);
}

// Only to be used when no pixmap has been set
// Exit if already there
int bgh_xcb_window::set_pixmap(bgh_ctx& ctx)
{
    if (!this->pixmap)
    {
        this->pixmap = xcb_generate_id(ctx.conn);
        xcb_composite_name_window_pixmap(ctx.conn, this->id, this->pixmap);
    }
    return this->pixmap; 
}

int bgh_xcb_window::set_pixmap(bgh_ctx& ctx, int id)
{
    if (this->pixmap)
    {
        xcb_free_pixmap(ctx.conn, this->pixmap);
    }
    return this->pixmap = id;
}

void bgh_xcb_window::set_geometry(xcb_get_geometry_reply_t* geometry)
{
    this->x = geometry->x;
    this->y = geometry->y;
    this->h = geometry->height;
    this->w = geometry->width;
}

int bgh_xcb_window::set_picture(bgh_ctx& ctx, xcb_render_pictforminfo_t* render_format)
{
    xcb_render_picture_t pic = xcb_generate_id(ctx.conn);
    xcb_render_create_picture(ctx.conn, pic, this->pixmap, render_format->id, 0, nullptr);
    return this->picture = pic;
}

std::array<xcb_render_pictforminfo_t*, 2> bgh_get_pict_formats(bgh_ctx& ctx)
{
    auto rgb_f = (xcb_render_pictforminfo_t*)malloc(sizeof(xcb_render_pictforminfo_t));
    auto rgba_f = (xcb_render_pictforminfo_t*)malloc(sizeof(xcb_render_pictforminfo_t));
    // Expect the bgh_ctx class to free ^^

    // Get all the available formats for rendering (i.e the non alpha and alpha channel ones)
    // Non-alpha channel format is just for the root window (ideally)
    xcb_render_query_pict_formats_cookie_t formats_cookie =
        xcb_render_query_pict_formats(ctx.conn);
    xcb_render_query_pict_formats_reply_t* formats_reply =             // No err
        xcb_render_query_pict_formats_reply(ctx.conn, formats_cookie, nullptr);
    xcb_render_pictforminfo_iterator_t formats_iter =
        xcb_render_query_pict_formats_formats_iterator(formats_reply);

    while (formats_iter.rem)
    {
        xcb_render_pictforminfo_t* format = formats_iter.data;
        switch (format->depth)
        {
            // TODO: Add more steps for checking
            // Eg. Check for rbg masks to be 0xff
            case 24: memcpy(rgb_f, format, sizeof(xcb_render_pictforminfo_t));
            case 32: memcpy(rgba_f, format, sizeof(xcb_render_pictforminfo_t));
        }

        xcb_render_pictforminfo_next(&formats_iter);
    }

    free(formats_reply);
    return {rgb_f, rgba_f};
}

// Error handling
void bgh_handle(xcb_generic_error_t* err, std::string msg)
{
    if (err) {
        std::cout << msg << std::endl;
        std::cout << "Error code: " << err->error_code << std::endl;
    }
    free(err);
}

void bgh_handle(void* err, std::string* msg)
{
    if (err) std::cout << msg << std::endl;
    free(err);
}

void bgh_handle(xcb_generic_error_t* err)
{
    bgh_handle(err, "Encountered error");
}
