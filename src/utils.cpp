#include <cstdlib>
#include <utils.hpp>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <iostream>
#include <stdlib.h>

bgh_ctx::bgh_ctx()
{
    this->conn = xcb_connect(NULL, NULL);
    this->screen = xcb_setup_roots_iterator(xcb_get_setup(this->conn)).data;

    this->root_window = this->screen->root;
    // Part of the initializing step
    // TODO: Make this some kind of function, right now it is not explicit
    // TODO: Multi-screen support
    xcb_composite_redirect_subwindows(conn, screen->root, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

    uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
    xcb_void_cookie_t cookie =
        xcb_change_window_attributes_checked(conn, screen->root, XCB_CW_EVENT_MASK, values);

    bgh_handle(xcb_request_check(conn, cookie), "Setting the flags failed");

    this->overlay_window = xcb_generate_id(this->conn);

    // Get the overlay window from x11
    xcb_composite_get_overlay_window_cookie_t overlay_cookie
        = xcb_composite_get_overlay_window(this->conn, this->root_window);
    xcb_composite_get_overlay_window_reply_t* overlay_reply
        = xcb_composite_get_overlay_window_reply(this->conn, overlay_cookie, nullptr);

    if (overlay_reply)
    {
        this->overlay_window = overlay_reply->overlay_win;
        free(overlay_reply);
    }

    xcb_map_window(this->conn, this->overlay_window);
    xcb_flush(conn);
}

bgh_ctx::~bgh_ctx()
{
    xcb_disconnect(conn);
}

bgh_xcb_window::bgh_xcb_window(bgh_ctx& ctx):
    ctx(ctx)
{
    this->global_windows.push_back(this); // Add self to the total window list
    this->id = xcb_generate_id(ctx.conn); // Generate an id for self
}

int bgh_xcb_window::set_pixmap(bgh_ctx& ctx)
{
    this->pixmap = xcb_generate_id(ctx.conn);
    xcb_composite_name_window_pixmap(ctx.conn, this->id, this->pixmap);
    return this->pixmap; 
}

int bgh_xcb_window::set_pixmap(int id)
{
    return this->pixmap = id;
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
