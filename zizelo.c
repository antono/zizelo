#include <glib.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>
#include <stdlib.h>
#include <string.h>

#include "page.h"
#include "gophernet.h"
#include "zizelo.h"

#define BUILDER "data/zizelo.ui"


G_MODULE_EXPORT void
on_about_menu_item_activate_cb (GtkMenuItem *menuitem, gpointer user_data)
{
  GtkWidget *about_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zizelo_about"));
  gtk_dialog_run (GTK_DIALOG (about_dialog));
  gtk_widget_hide (about_dialog);
}

G_MODULE_EXPORT void
on_addressbar_activate (GtkEntry *entry, gpointer user_data)
{
  gchar * full_url;
  gchar * url = gtk_entry_get_text(entry);
  gchar * gopher_url_schema = strdup("gopher://");

  if (!strstr(url, gopher_url_schema)) {
    full_url = g_strdup_printf("%s%s", gopher_url_schema, url);
  } else {
    full_url = url;
  }

  gtk_entry_set_text(entry, full_url);
  zz_open(full_url, TRUE);
}

G_MODULE_EXPORT void
on_addressbar_changed (GtkEntry *entry, gpointer user_data)
{
  gchar * text = gtk_entry_get_text(entry);

  if (strstr(text, g_strdup(".")) || strlen(text) == 0) {
    gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_OPEN);
  } else {
    gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
  }
}

static gboolean
on_viewport_scroll_event (ClutterActor *_self,
                          ClutterEvent *event,
                          gpointer _page)
{
  ClutterActor *view = viewport;
  ClutterActor *page = current_page->actor;

  if (!page) g_debug("No scrollable :(");
  if (!view) g_debug("No user data :(");

  gfloat scrollable_height = clutter_actor_get_height (page);
  gfloat viewport_height   = clutter_actor_get_height (view);


  /* no need to scroll if the scrollable is shorter than the viewport */
  if (scrollable_height < viewport_height)
    return TRUE;

  gfloat y = clutter_actor_get_y (page);


  /* determine the direction the mouse was scrolled */
  ClutterScrollDirection direction;
  direction = clutter_event_get_scroll_direction (event);

  /* replace these stubs with real code to move the actor etc. */
  switch (direction)
    {
    case CLUTTER_SCROLL_UP:
      y += scroll_factor;
      break;
    case CLUTTER_SCROLL_DOWN:
      y -= scroll_factor;
      break;
    case CLUTTER_SCROLL_RIGHT:
      g_debug ("Scrolled right");
      break;
    case CLUTTER_SCROLL_LEFT:
      g_debug ("Scrolled left");
      break;
    }

  y = CLAMP (y, viewport_height - scrollable_height, 0.0);

  scroll_factor = viewport_height * 0.225;

  clutter_actor_animate (page,
                         CLUTTER_EASE_OUT_CUBIC,
                         300,
                         "y", y,
                         NULL);

  return TRUE; /* event has been handled */
}

void
zz_open(gchar *uri, gboolean is_menu)
{
  ZzOpenData *user_data = g_new(ZzOpenData, 1);

  user_data->is_menu = is_menu;
  user_data->uri = uri;

  g_gopher_get_async(NULL, uri, zz_open_handle_result, user_data);
}

void
zz_open_handle_result (GObject *source,
                       GSimpleAsyncResult *result,
                       gpointer user_data)
{
  ZzOpenData *data = user_data;
  ZzPage  * page;
  GError  * error;
  GString * string;

  string = g_gopher_get_finish (source, result, &error);
  page = zz_page_new(string, data->is_menu);
  page->open_data = data;

  zz_display_page(page);
}

void
zz_display_page (ZzPage *page)
{
  if (current_page)
    clutter_container_remove_actor(CLUTTER_CONTAINER(viewport),
                                   current_page->actor);

  current_page = page;

  // Change addressbar content
  gtk_entry_set_text(GTK_ENTRY(addressbar), page->open_data->uri);

  // Add page actor to viewport
  clutter_container_add(CLUTTER_CONTAINER(viewport), current_page->actor, NULL);
  clutter_actor_set_position(CLUTTER_ACTOR(current_page->actor), 20, 20);

  // Resize viewport
  gfloat width;
  gfloat height;

  clutter_actor_get_size (stage, &width, &height);
  clutter_actor_set_size (viewport, width, height);
}


void on_stage_allocation_changed (ClutterActor *actor,
                                  const ClutterActorBox  *allocation,
                                  ClutterAllocationFlags  flags,
                                  gpointer                user_data)
{
  gfloat stage_width  = clutter_actor_box_get_width  (allocation);
  gfloat stage_height = clutter_actor_box_get_height (allocation);

  scroll_factor = stage_height * 0.125;
  clutter_actor_set_size (viewport, stage_width, stage_height);
}

GtkTreeModel *
addressbar_autocomplete_model_new (void) {

  GtkListStore *store;
  GtkTreeIter iter;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "antono.info", -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "floodgap.com", -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "gophernicus.org", -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "about:", -1);

  return GTK_TREE_MODEL (store);
}

void
zz_bind_autocomplete (GtkWidget *addressbar) {
  /* Addressbar autocomplete */
  GtkEntryCompletion *autocomplete = gtk_entry_completion_new ();
  gtk_entry_set_completion (GTK_ENTRY (addressbar), autocomplete);
  g_object_unref (autocomplete);

  /* Create a tree model and use it as the completion model */
  GtkTreeModel *completion_model = addressbar_autocomplete_model_new();
  gtk_entry_completion_set_model (autocomplete, completion_model);
  g_object_unref (completion_model);
  // Use model column 0 as the text column
  gtk_entry_completion_set_text_column (autocomplete, 0);
}

int
main(int argc, char *argv[]) {

  gtk_clutter_init (&argc, &argv);

  // Using GtkBuilder
  GError* error = NULL;
  builder = gtk_builder_new ();

  if (!gtk_builder_add_from_file (builder, BUILDER, &error))
    {
      g_warning ("Couldn't load builder file: %s", error->message);
      g_error_free (error);
    }

  gtk_builder_connect_signals (builder, NULL);

  GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, "zizelo"));
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  addressbar = GTK_WIDGET (gtk_builder_get_object (builder, "addressbar"));

  zz_bind_autocomplete (GTK_WIDGET (addressbar));

  /* create clutter widget */
  ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };

  GtkWidget *clutter_widget = gtk_clutter_embed_new();

  GtkWidget *vbox = GTK_WIDGET (gtk_builder_get_object (builder, "vbox"));
  gtk_box_pack_start (GTK_BOX (vbox), clutter_widget, TRUE, TRUE, 0);
  gtk_box_reorder_child (GTK_BOX (vbox), clutter_widget, 2);

  /* Get the stage and set its size and color: */
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (clutter_widget));
  clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
  g_signal_connect (stage,
                    "allocation-changed",
                    G_CALLBACK (on_stage_allocation_changed),
                    NULL);

  /* Create a viewport actor */
  viewport = clutter_group_new ();

  clutter_actor_set_reactive (viewport, TRUE);
  clutter_actor_add_constraint (viewport,
                                clutter_align_constraint_new (stage,
                                                              CLUTTER_BIND_Y,
                                                              0.5));
  clutter_actor_set_clip_to_allocation (viewport, TRUE);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), viewport);

  g_signal_connect (viewport,
                    "scroll-event",
                    G_CALLBACK (on_viewport_scroll_event),
                    current_page);

  gtk_widget_show_all(window);

  zz_open(g_strdup("gopher://antono.info:70/"), TRUE);

  gtk_main();


  return EXIT_SUCCESS;
}
