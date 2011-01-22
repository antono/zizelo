#include <glib.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>
#include <stdlib.h>
#include <string.h>

#include "zizelo.h"
#include "gophernet.h"

GtkTreeModel * addressbar_autocomplete_model_new (void) {

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


static gboolean on_addressbar_change (GtkEntry *entry, gpointer user_data) {
	gchar * text = gtk_entry_get_text(entry);

	g_print("%s\n", text);

	if (strstr(text, g_strdup(".")) || strlen(text) == 0) {
		gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_OPEN);
	} else {
		gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	}
	return TRUE;
}

/* Handles Address Bar <enter> */
static gboolean on_addressbar_activate (GtkEntry *entry, gpointer user_data) {

	GError * error = NULL;
	gchar  * url = gtk_entry_get_text(entry);
	gchar  * gopher_url_schema = strdup("gopher://");
	gchar  * full_url;

	if (!strstr(url, gopher_url_schema)) {
		full_url = g_strdup_printf("%s%s", gopher_url_schema, url);
		gtk_entry_set_text(entry, full_url);
	} else {
		full_url = url;
	}

	g_debug("Requested %s\n", full_url);

	/* Handle ASCII */
	gchar * page = g_gopher_get(full_url);
	gchar * page_utf8;

	if (g_utf8_validate (page, -1, NULL)) {
		page_utf8 = page;
	} else {
		g_debug("Trying to convert");
		page_utf8 = g_convert(page, -1, "UTF-8", "ASCII", NULL, NULL, &error);
		g_debug("Done");
		g_debug("Printing UTF-8 page:");
		g_print("%s", page_utf8);
	}

	if (error) {
		g_debug(error->message);
		g_free(error);
	} else {
		g_debug("No errors in conversion");
	}

	g_debug("applying markup");
	gchar * markup = g_strjoin("\n", "<tt>", page_utf8, "</tt>", NULL);
	g_print("%s\n", markup);
	g_debug("done");
	clutter_text_set_markup(CLUTTER_TEXT(current_page), markup);
	g_debug("drawing done");

	return TRUE;
}


/*
 *static gboolean on_stage_button_press (ClutterStage *stage, ClutterEvent *event, gpointer data)
 *{
 *  float x = 0;
 *  float y = 0;
 *  clutter_event_get_coords (event, &x, &y);
 *
 *  g_print ("Stage clicked at (%f, %f)\n", x, y);
 *
 *  return TRUE; [> Stop further handling of this event. <]
 *}
 */

int main(int argc, char *argv[]) {

	/*g_url*/
	gtk_clutter_init (&argc, &argv);

	/* create window */
	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (window, "hide", G_CALLBACK(gtk_main_quit), NULL);

	/* Vbox */
	GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show (vbox);

	/* Addressbar */
	GtkWidget *entry = gtk_entry_new();
	gtk_entry_set_max_length (GTK_ENTRY(entry), 50);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_OPEN);
	gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
	gtk_widget_show (entry);

	g_signal_connect (entry, "changed",
		       G_CALLBACK(on_addressbar_change), NULL);
	g_signal_connect (entry, "activate",
			G_CALLBACK(on_addressbar_activate), NULL);


	/* Addressbar autocomplete */
	GtkEntryCompletion *autocomplete = gtk_entry_completion_new ();
	gtk_entry_set_completion (GTK_ENTRY (entry), autocomplete);
	g_object_unref (autocomplete);

	/* Create a tree model and use it as the completion model */
	GtkTreeModel *completion_model = addressbar_autocomplete_model_new();
	gtk_entry_completion_set_model (autocomplete, completion_model);
	g_object_unref (completion_model);
	gtk_entry_completion_set_text_column (autocomplete, 0); // Use model column 0 as the text column


	/* create stage */
	ClutterColor 	stage_color 	= { 0x00, 0x00, 0x00, 0xff }; /* Black */
	GtkWidget 	*clutter_widget	= gtk_clutter_embed_new();
	gtk_box_pack_start(GTK_BOX (vbox), clutter_widget, TRUE, TRUE, 0);
	gtk_widget_show(clutter_widget);
	gtk_widget_set_size_request(clutter_widget, 600, 500);

	/* Get the stage and set its size and color: */
	stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(clutter_widget));
	clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);
	clutter_actor_show (stage);

	/* Create Clutter Text Actor */
	ClutterColor	text_color	= { 0x33, 0xff, 0x33, 0xff };
	ClutterColor	cursor_color	= { 0xff, 0x33, 0x33, 0xff };
	ClutterText	*text		= clutter_text_new();

	current_page = text;

	clutter_text_set_color (CLUTTER_TEXT(text), &text_color);
	clutter_text_set_cursor_color (CLUTTER_TEXT(text), &cursor_color);
	clutter_text_set_selectable (CLUTTER_TEXT(text), TRUE);
	clutter_actor_set_reactive (CLUTTER_ACTOR(text), TRUE);
	clutter_text_set_markup(CLUTTER_TEXT(text), strdup("<tt>hello world!</tt>"));
	clutter_stage_set_key_focus (CLUTTER_STAGE(stage), CLUTTER_ACTOR(text));
	clutter_actor_set_position(CLUTTER_ACTOR(text), 10, 10);
	clutter_container_add (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR(text), NULL);

	/* Connect a signal handler to handle mouse clicks and key presses on the stage: */ 
	/*g_signal_connect (stage, "button-press-event",*/
	/*                G_CALLBACK (on_stage_color_change), NULL);*/
	/*g_signal_connect (stage, "button-press-event",*/
	/*                G_CALLBACK (on_stage_button_press), NULL);*/
	/*g_signal_connect (stage, "button-press-event",*/
	/*                G_CALLBACK (on_stage_click), NULL);*/

	gtk_widget_show(window);

	/* Start the main loop, so we can respond to events: */
	gtk_main();

	return EXIT_SUCCESS;
}
