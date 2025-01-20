#include <cstdint>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <iostream>
#include <vector>

int main ()
{
    xcb_connection_t* conn = xcb_connect (NULL, NULL);
    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    xcb_composite_redirect_subwindows(conn, screen->root, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };

    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(conn, screen->root, mask, values);
    xcb_generic_error_t* error = xcb_request_check(conn, cookie);

    if (error) {
        std::cout << "Error " << error->error_code << std::endl;
        xcb_disconnect(conn);
        return -1;
    } else {
        free(error);
    }

    xcb_flush(conn);

    xcb_generic_event_t* e;
    while ((e = xcb_wait_for_event(conn)))
    {
        switch (e->response_type & ~0x80) {
            case XCB_EXPOSE: {
                printf("Window exposed\n");
            }
            case XCB_CREATE_NOTIFY: {
                printf("Window created\n");
            }
            default: {
                printf("Got default event: %d\n", e->response_type);
            }
        }
        free(e);
    }

    xcb_disconnect(conn);

    return 0;
}
