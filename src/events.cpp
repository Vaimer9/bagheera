#include "utils.hpp"
#include <events.hpp>
#include <iostream>
#include <xcb/composite.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

bgh_event_handler::bgh_event_handler(bgh_ctx& ctx)
    : ctx(ctx)
{
    this->events = {
        { XCB_MAP_NOTIFY, bgh_ev_map },
        { XCB_UNMAP_NOTIFY, bgh_ev_unmap },
        { XCB_CREATE_NOTIFY, bgh_ev_create },
        { XCB_EXPOSE, bgh_ev_expose }
    };
}

void bgh_event_handler::handle(xcb_generic_event_t* event)
{
    for (auto&& ev_h : events)
    {
        if (ev_h.event == (event->response_type & ~0x80))
            ev_h.func(this->ctx, event);
    }
}

void bgh_ev_map(bgh_ctx& ctx, xcb_generic_event_t* gen_event)
{
    printf("Mapping\n");
    // auto event = (xcb_map_notify_event_t*)gen_event;
    //
    // xcb_pixmap_t pixmap;
    // xcb_composite_name_window_pixmap(ctx.conn, event->window, xcb_generate_id(ctx.conn));
}

void bgh_ev_unmap(bgh_ctx &ctx, xcb_generic_event_t *event)
{
    printf("Unmapping\n");
}

void bgh_ev_create(bgh_ctx &ctx, xcb_generic_event_t *event)
{
    printf("Creating\n");
}

void bgh_ev_expose(bgh_ctx &ctx, xcb_generic_event_t *event)
{
    printf("Expose\n");
}
