#pragma once

#include <array>
#include <functional>
#include <list>
#include <vector>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include "utils.hpp"

using xcb_event_t = int;

struct bgh_event_t
{
    xcb_event_t event;
    std::function<void(bgh_ctx& ctx, xcb_generic_event_t*)> func;
};

struct bgh_event_handler
{
    std::vector<bgh_event_t> events;
    bgh_ctx& ctx;
    void handle(xcb_generic_event_t* event);
    bgh_event_handler(bgh_ctx& ctx);
};

void bgh_ev_map(bgh_ctx& ctx, xcb_generic_event_t* event);
void bgh_ev_unmap(bgh_ctx& ctx, xcb_generic_event_t* event);
void bgh_ev_create(bgh_ctx& ctx, xcb_generic_event_t* event);
void bgh_ev_expose(bgh_ctx& ctx, xcb_generic_event_t* event);
void bgh_render(bgh_ctx& ctx, xcb_generic_event_t* event);
