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

    // Part of the initializing step
    // TODO: Make this some kind of function, right now it is not explicit
    // TODO: Multi-screen support
    xcb_composite_redirect_subwindows(conn, screen->root, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(conn, screen->root, XCB_CW_EVENT_MASK, values);
    xcb_generic_error_t* error = xcb_request_check(conn, cookie);

    if (error) {
        std::cout << "Error " << error->error_code << std::endl;
        xcb_disconnect(conn);
        exit(-1);
    } else {
        free(error);
    }

    xcb_flush(conn);
}

bgh_ctx::~bgh_ctx()
{
    xcb_disconnect(conn);
}
