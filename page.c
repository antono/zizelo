#include <glib.h>
#include <clutter/clutter.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#include "gophernet.h"
#include "guri.h"
#include "page.h"

ZzPage *
zz_page_new (GString *raw_page, gboolean is_menu)
{
  ZzPage *page = g_new(ZzPage, 1);

  page->layout = clutter_box_layout_new ();
  clutter_box_layout_set_vertical (CLUTTER_BOX_LAYOUT(page->layout), TRUE);

  page->actor = clutter_box_new (page->layout);
  page->raw_page = raw_page;
  page->raw_page_clean = g_string_new("");
  page->parts = g_array_new(TRUE, TRUE, sizeof(gpointer));

  if (is_menu) {
    zz_page_parse(page);
  } else {
    zz_page_add_text(page, raw_page->str);
  }

  return page;
}

ZzPage *
zz_page_parse (ZzPage *page)
{
  gchar **lines = g_strsplit(page->raw_page->str, "\r\n", -1);
  gchar *line;
  gchar type;

  while (*lines && !(strcmp(*lines, ".") == 0)) {

    line = *lines;
    type = line[0];

    line++;

    gchar * desc = zz_gophermap_get_segment(line, 0);

    g_string_append (page->raw_page_clean, desc);
    g_string_append (page->raw_page_clean, "\n");

    if (type == 'i') {
      // g_debug("STRPD:\t%s\n", desc);
      zz_page_add_text(page, desc);
    } else if (type == '0') {
      // g_debug("DIR.:\t%s\t%s", desc, zz_gopherlink_to_uri(strdup("gopher"), line));
      g_string_append (page->raw_page_clean,
                       zz_gopherlink_to_uri(strdup("gopher"),line));
      g_string_append (page->raw_page_clean, "\n");
      zz_page_add_link(page,
                       desc,
                       zz_gopherlink_to_uri(strdup("gopher"), line),
                       type);
    } else if (type == '1' || type == 'I') {
      // g_debug("FILE:\t%s\t%s", desc, zz_gopherlink_to_uri("gopher", line));
      g_string_append (page->raw_page_clean,
                       zz_gopherlink_to_uri(strdup("gopher"),line));
      g_string_append (page->raw_page_clean, "\n");
      zz_page_add_link(page,
                       desc,
                       zz_gopherlink_to_uri(strdup("gopher"), line),
                       type);
    } else if (type == '8') {
      // g_debug("FILE:\t%s\t%s", desc, zz_gopherlink_to_uri("telnet", line), type);
      g_string_append (page->raw_page_clean,
                       zz_gopherlink_to_uri(strdup("telnet"), line));
      g_string_append (page->raw_page_clean, "\n");
      zz_page_add_link(page,
                       desc,
                       zz_gopherlink_to_uri(strdup("telnet"), line),
                       type);
    } else if (type == 'h') {
      // g_debug("FILE:\t%s\t%s", desc, zz_gopherlink_to_uri("http", line));
      g_string_append (page->raw_page_clean,
                       zz_gopherlink_to_uri(strdup("http"),line));
      g_string_append (page->raw_page_clean, "\n");
      zz_page_add_link(page,
                       desc,
                       zz_gopherlink_to_uri(strdup("http"), line),
                       type);
    }
    lines++;
  }

  // g_debug("----- Links on the page %d", page->parts->len);

  return page;
}

ZzLink *
zz_link_new(gchar *title, gchar *uri, gchar type)
{
  ZzLink *self = g_new(ZzLink, 1);

  ClutterColor text_color = { 0xff, 0xff, 0x00, 0xff };

  self->title = title;
  self->uri = uri;
  self->type = type;
  self->actor = clutter_text_new();

  clutter_text_set_markup (CLUTTER_TEXT(self->actor), zz_monospace(title));
  clutter_text_set_color (CLUTTER_TEXT(self->actor), &text_color);
  clutter_actor_set_reactive (CLUTTER_ACTOR(self->actor), TRUE);

  g_signal_connect (self->actor,
                    "button-press-event",
                    G_CALLBACK(on_link_click_cb),
                    self);
  g_signal_connect (self->actor,
                    "enter-event",
                    G_CALLBACK (on_link_mouse_enter_cb),
                    self);
  g_signal_connect (self->actor,
                    "leave-event",
                    G_CALLBACK (on_link_mouse_leave_cb),
                    self);

  return self;
}

void
zz_page_add_link (ZzPage *page, gchar *desc, gchar *uri, gchar type) {

  ZzLink *link = zz_link_new(desc, uri, type);
  g_array_append_val(page->parts, link);
  clutter_box_layout_pack(CLUTTER_BOX_LAYOUT(page->layout), link->actor,
                          TRUE, // Expand
                          TRUE, // X-fill
                          FALSE, // Y-fill
                          CLUTTER_BOX_ALIGNMENT_CENTER,
                          CLUTTER_BOX_ALIGNMENT_CENTER);
}


ZzText *
zz_text_new(gchar * raw) {

  ZzText *self = g_new(ZzText, 1);

  ClutterColor text_color = { 0x33, 0xff, 0x33, 0xff };

  self->actor = clutter_text_new();
  self->raw = raw;

  clutter_text_set_markup (CLUTTER_TEXT(self->actor), zz_monospace(raw));
  clutter_text_set_color (CLUTTER_TEXT(self->actor), &text_color);
  clutter_actor_set_reactive (CLUTTER_ACTOR(self->actor), TRUE);

  return self;
}

void zz_page_add_text (ZzPage *self, gchar *raw) {
  ZzText *text = zz_text_new(raw);

  g_array_append_val(self->parts, text);

  clutter_box_layout_pack(CLUTTER_BOX_LAYOUT(self->layout),
                          CLUTTER_ACTOR(text->actor),
                          TRUE, // Expand
                          TRUE, // X-fill
                          FALSE, // Y-fill
                          CLUTTER_BOX_ALIGNMENT_CENTER,
                          CLUTTER_BOX_ALIGNMENT_CENTER);
}

gboolean on_link_click_cb (ClutterActor *actor,
                           ClutterEvent *event,
                           ZzLink *link) {
  g_debug("Link clicked: %s", link->uri);
  if (link->type == '1') g_debug("Gophermap");
  zz_open(link->uri, (link->type == '1'));
  return TRUE;
}

gboolean on_link_mouse_enter_cb (ClutterActor *actor,
                                 ClutterEvent *event,
                                 gpointer user_data) {
  ClutterColor text_color = { 0xff, 0xff, 0xff, 0xff };
  clutter_text_set_color (CLUTTER_TEXT(actor), &text_color);
  return TRUE;
}

gboolean on_link_mouse_leave_cb (ClutterActor *actor,
                                 ClutterEvent *event,
                                 gpointer user_data) {
  ClutterColor text_color = { 0xff, 0xff, 0x00, 0xff };
  clutter_text_set_color (CLUTTER_TEXT(actor), &text_color);
  return TRUE;
}


gchar * zz_monospace(gchar * text) {
  return g_markup_printf_escaped ("<tt>%s</tt>", text);
}

gchar * zz_gophermap_get_segment(gchar *line, gint num) {
  gchar **segments = g_strsplit(line, "\t", -1);
  return segments[num];
}

gchar * zz_gopherlink_to_uri(gchar *schema, gchar *line) {
  GURI *uri = g_uri_new_fields(schema,
                               zz_gophermap_get_segment(line, 2),
                               atoi(zz_gophermap_get_segment(line, 3)),
                               zz_gophermap_get_segment(line, 1));
  return g_uri_get_string(uri);
}
