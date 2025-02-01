#include "utils.hpp"
#include <events.hpp>
#include <iostream>
#include <xcb/composite.h>
#include <xcb/render.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

bgh_event_handler::bgh_event_handler(bgh_ctx& ctx)
    : ctx(ctx)
{
    this->events = {
        { XCB_MAP_NOTIFY, bgh_ev_map },
        { XCB_UNMAP_NOTIFY, bgh_ev_unmap },
        { XCB_CREATE_NOTIFY, bgh_ev_create },
        { XCB_EXPOSE, bgh_ev_expose },
        { XCB_DESTROY_NOTIFY, bgh_ev_destroy },
        { XCB_CONFIGURE_NOTIFY, bgh_ev_configure }
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
    auto event = (xcb_map_notify_event_t*)gen_event;

    for (auto&& win : ctx.global_windows)
    {
        if (win->id == event->window)
        {
            bgh_update_window(ctx, win->id);
            break;
        }
    }

    printf("Window mapped: %d\n", event->window);
    bgh_draw_screen(ctx);
}

void bgh_ev_unmap(bgh_ctx &ctx, xcb_generic_event_t *gen_event)
{
    auto event = (xcb_unmap_notify_event_t*)gen_event;
    bgh_draw_screen(ctx);
    printf("Window unmapped: %d\n", event->window);
}

void bgh_ev_create(bgh_ctx &ctx, xcb_generic_event_t *gen_event)
{
    auto event = (xcb_create_notify_event_t*)gen_event;
    bgh_update_window(ctx, event->window);
    printf("Create event triggered win %d\n", event->window);
}

void bgh_ev_expose(bgh_ctx &ctx, xcb_generic_event_t *event)
{
    bgh_draw_screen(ctx);
    printf("Expose\n");
}

void bgh_ev_destroy(bgh_ctx& ctx, xcb_generic_event_t* gen_event)
{
    auto event = (xcb_destroy_notify_event_t*)gen_event;

    for (int i = 0; i < ctx.global_windows.size(); i++)
    {
        auto win = ctx.global_windows[i];

        if (win->id == event->window)
        {
            delete win; // Run the destructor of the window
                        // then remove from global stack
            ctx.global_windows.erase(ctx.global_windows.begin() + i);
            break;
        }
    }
    bgh_draw_screen(ctx);

    printf("Destroyed window %d\n", event->window);
}

void bgh_ev_configure(bgh_ctx& ctx, xcb_generic_event_t* gen_event)
{
    auto event = (xcb_configure_notify_event_t*)gen_event;

    bgh_update_window(ctx, event->window);
    bgh_draw_screen(ctx);
}

void bgh_update_window(bgh_ctx &ctx, xcb_window_t window)
{
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(ctx.conn, window);
    xcb_get_geometry_reply_t* geometry = xcb_get_geometry_reply(ctx.conn, cookie, nullptr);

    for (auto&& g_win : ctx.global_windows)
    {
        if (g_win->id == window)
        {
            g_win->set_pixmap(ctx); // Create a new pixmap for this window
                                     // TODO: Check if we really need to set a new pixmap every update?
            g_win->set_geometry(geometry);
            free(geometry);
            return; // Return if a new window was found and updated
        }
    }

    // If no window found, create one and push it to global window list
    auto win = new bgh_xcb_window(ctx, window);
    win->set_pixmap(ctx);
    win->set_geometry(geometry);
    free(geometry);

    win->add_global();
}

void bgh_draw_screen(bgh_ctx &ctx)
{
    // Clear overlay
    xcb_render_color_t clear = {0, 0, 0, 0};
    xcb_rectangle_t rect = { 0, 0, ctx.screen->width_in_pixels, ctx.screen->height_in_pixels };
    xcb_render_fill_rectangles(
        ctx.conn,
        XCB_RENDER_PICT_OP_CLEAR,
        ctx.overlay_window->picture,
        clear,
        1,
        &rect
    );

    for (auto&& win : ctx.global_windows)
    {
        xcb_render_picture_t picture = win->set_picture(ctx, ctx.rgba_format);

        xcb_render_composite(
            ctx.conn,
            XCB_RENDER_PICT_OP_OVER,
            picture,
            XCB_NONE,
            ctx.overlay_window->picture,
            0, 0, // src x, y
            0, 0, // mask x, y
            win->x, win->y,
            win->w, win->h
        );

        xcb_render_free_picture(ctx.conn, picture);
    }

    xcb_flush(ctx.conn);
}
