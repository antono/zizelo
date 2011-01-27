#include <glib.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>
#include <stdlib.h>
#include <string.h>

#include "zizelo.h"
#include "page.h"
#include "gophernet.h"

#define IFACE "data/zizelo.glade"

GtkBuilder	*ui_builder;
ClutterActor 	*stage;
ClutterActor 	*viewport;
ZzPage 		*current_page = NULL;

GtkWidget 	*addressbar;

G_MODULE_EXPORT void
on_about_menu_item_activate_cb (GtkMenuItem *menuitem, gpointer     user_data)
{
	GtkWidget *about_dialog = GTK_WIDGET (gtk_builder_get_object (ui_builder, "zizelo_about"));
	gtk_dialog_run (GTK_DIALOG (about_dialog));
	gtk_widget_hide (about_dialog);
}

G_MODULE_EXPORT void
on_addressbar_activate_cb (GtkEntry *entry, gpointer user_data) {

	gchar * url = gtk_entry_get_text(entry);
	gchar * gopher_url_schema = strdup("gopher://");
	gchar * full_url;

	if (!strstr(url, gopher_url_schema)) {
		full_url = g_strdup_printf("%s%s", gopher_url_schema, url);
	} else {
		full_url = url;
	}

	gtk_entry_set_text(entry, full_url);
	zz_open(full_url, TRUE);

	return TRUE;
}

G_MODULE_EXPORT void
on_addressbar_changed_cb (GtkEntry *entry, gpointer user_data) {

	gchar * text = gtk_entry_get_text(entry);

	g_print("%s\n", text);

	if (strstr(text, g_strdup(".")) || strlen(text) == 0) {
		gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_OPEN);
	} else {
		gtk_entry_set_icon_from_stock (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	}
	return TRUE;
}

void zz_open(gchar *uri, gboolean menu) {
	//g_debug("Requesting %s\n", uri);
	
	GString * page = g_gopher_get(uri);

	if (current_page) clutter_container_remove_actor(CLUTTER_CONTAINER(viewport), current_page->actor);

	current_page = zz_page_new(page, menu);

	// Change addressbar content
	gtk_entry_set_text(GTK_ENTRY(addressbar), uri);

	// Add page actor to viewport
	clutter_container_add(CLUTTER_CONTAINER(viewport), current_page->actor, NULL);
	clutter_actor_set_position(CLUTTER_ACTOR(current_page->actor), 20, 20);

	// Resize viewport
	gfloat width;
	gfloat height;

	clutter_actor_get_size (current_page->actor, &width, &height);
	g_debug("-----------> %f, %f", width, height);
	clutter_actor_set_size (viewport, width/2, height/2);

	g_debug("Done");
}

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


int main(int argc, char *argv[]) {

	gtk_clutter_init (&argc, &argv);

	// Using GtkBuilder
	GError* error = NULL;
	ui_builder = gtk_builder_new ();

	if (!gtk_builder_add_from_file (ui_builder, IFACE, &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	} 

	gtk_builder_connect_signals (ui_builder, NULL);

	GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (ui_builder, "zizelo"));
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	GtkWidget *vbox = GTK_WIDGET (gtk_builder_get_object (ui_builder, "vbox"));
	gtk_widget_set_size_request (GTK_WIDGET(vbox), 600, 400);

	addressbar = GTK_WIDGET (gtk_builder_get_object (ui_builder, "addressbar"));

	/* Addressbar autocomplete */
	GtkEntryCompletion *autocomplete = gtk_entry_completion_new ();
	gtk_entry_set_completion (GTK_ENTRY (addressbar), autocomplete);
	g_object_unref (autocomplete);

	/* Create a tree model and use it as the completion model */
	GtkTreeModel *completion_model = addressbar_autocomplete_model_new();
	gtk_entry_completion_set_model (autocomplete, completion_model);
	g_object_unref (completion_model);
	gtk_entry_completion_set_text_column (autocomplete, 0); // Use model column 0 as the text column

	/* add table for scrollbars and stage */
	GtkWidget *table = gtk_table_new (2, 2, FALSE);
	gtk_box_pack_start(GTK_BOX (vbox), table, TRUE, TRUE, 0);
	gtk_widget_show (table);

	/* create stage */
	ClutterColor 	stage_color 	= { 0x00, 0x00, 0x00, 0xff }; /* Black */
	GtkWidget 	*clutter_widget	= gtk_clutter_embed_new();
	gtk_table_attach (GTK_TABLE (table), clutter_widget,
			0, 1,
			0, 1,
			GTK_EXPAND | GTK_FILL,
			GTK_EXPAND | GTK_FILL,
			0, 0);
	gtk_widget_show(clutter_widget);
	gtk_widget_set_size_request(clutter_widget, 300, 300);

	/* Get the stage and set its size and color: */
	stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(clutter_widget));
	clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);
	clutter_actor_show (stage);

	/* Create a viewport actor to be able to scroll actor. By passing NULL it
	 * will create new GtkAdjustments. */
	viewport = gtk_clutter_viewport_new (NULL, NULL, NULL);
	clutter_container_add_actor (CLUTTER_CONTAINER (stage), viewport);
	clutter_actor_set_position(CLUTTER_ACTOR(viewport), 20, 20);
	clutter_actor_set_size (viewport, 600, 400);

	/* Create scrollbars and connect them to viewport: */
	GtkAdjustment *h_adjustment = NULL;
	GtkAdjustment *v_adjustment = NULL;
	gtk_clutter_scrollable_get_adjustments (GTK_CLUTTER_SCROLLABLE (viewport), &h_adjustment, &v_adjustment);

	GtkWidget *scrollbar_v = gtk_vscrollbar_new (v_adjustment);

	gtk_table_attach (GTK_TABLE (table), scrollbar_v,
			1, 2,
			0, 1,
			0, GTK_EXPAND | GTK_FILL,
			0, 0);

	gtk_widget_show (scrollbar_v);

	GtkWidget *scrollbar_h = gtk_hscrollbar_new (h_adjustment);
	gtk_table_attach (GTK_TABLE (table), scrollbar_h,
			0, 1,
			1, 2,
			GTK_EXPAND | GTK_FILL, 0,
			0, 0);

	gtk_widget_show (scrollbar_h);



	gtk_widget_show_all(window);

	zz_open(g_strdup("gopher://antono.info:70/"), TRUE);
	gtk_main();
	return EXIT_SUCCESS;
}
