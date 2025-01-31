#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <stdlib.h>

#include "../include/utils.hpp"
#include "../include/events.hpp"

int main ()
{
    auto ctx = bgh_ctx();

    bgh_event_handler event_handler(ctx);

    xcb_generic_event_t* e;
    while ((e = xcb_wait_for_event(ctx.conn)))
    {
        event_handler.handle(e);
        free(e);
    }

    // Exit routine
    for (auto&& win_ptr : ctx.global_windows)
    {
        delete win_ptr;
    }

    return 0;
}
