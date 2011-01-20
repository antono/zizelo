#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>
#include <stdlib.h>
#include <string.h>

ClutterActor *stage = NULL;
ClutterActor *current_page = NULL;

gchar * g_gopher_request(gchar *);
char * strcasestr (const char *haystack, const char *needle);


/*
 *static gboolean on_stage_color_change (ClutterStage *stage, ClutterEvent *event, gpointer user_data) {
 *        static gboolean already_changed = FALSE;
 *
 *        g_warning("Stage color change!");
 *        if (already_changed) {
 *                ClutterColor stage_color =  { 0x00, 0x00, 0x00, 0xff };
 *                clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);
 *        } else {
 *                ClutterColor stage_color =  { 0x20, 0x20, 0x20, 0xff };
 *                clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);
 *        }
 *
 *        already_changed = !already_changed;
 *
 *        return TRUE;
 *}
 */

static gboolean on_addressbar_change (GtkEntry *entry, gpointer user_data) {
	g_print("%s\n", gtk_entry_get_text(entry));
	return TRUE;
}

/* Handles Address Bar <enter> */
static gboolean on_addressbar_activate (GtkEntry *entry, gpointer user_data) {
	GError * error = NULL;
	gchar  * url = gtk_entry_get_text(entry);
	gchar  * gopher_url_shema = "gopher://";
	gchar  * full_url;

	if (!strcasestr(url, gopher_url_shema)) {
		full_url = g_strdup_printf("%s%s", gopher_url_shema, url);
		gtk_entry_set_text(entry, full_url);
	} else {
		full_url = url;
	}

	g_debug("Requested %s\n", full_url);
	gchar * page = g_gopher_request(full_url);

	g_debug("Printing page:");
	g_print("%s\n", page);

	g_debug("Trying to convert");
	gchar  * page_utf8 = g_convert(page, -1, "UTF-8", "ASCII", NULL, NULL, &error);
	g_debug("Done");

	g_debug("Printing UTF-8 page:");
	g_print("%s", page_utf8);

	if (error) {
		g_debug(error->message);
	} else {
		g_debug("No errors in conversion");
	}



	if (g_utf8_validate (page_utf8, -1, NULL)) {
		g_debug("applying markup");
		gchar * markup = g_strjoin("\n", "<tt>", page_utf8, "</tt>");
		g_print("%s\n", markup);
		g_debug("done");

		clutter_text_set_markup(CLUTTER_TEXT(current_page), markup);
	} else {
		g_debug("Page is invalid utf8");
	}

	return TRUE;
}

GSocket * g_gopher_socket_connect(gchar *host, gint port, GError *error) {
	GSocket            * socket;
	GSocketClient      * client;
	GSocketConnection  * connection;
	GSocketConnectable * addr;

	addr       = g_network_address_new (host, port);
	client     = g_socket_client_new ();
	connection = g_socket_client_connect (client, addr, NULL, &error);
	socket     = g_socket_connection_get_socket (connection);

	if (socket) {
		return socket;
	} else {
		g_debug("Cannot create socket");
		return NULL;
	}
}

gchar * g_gopher_request (gchar *url) {

	GError  * error;
	gchar   * locator;
	gchar   * page = "";

	gchar   buffer[4096];
	gint 	total 	= 0;
	gint 	size 	= 0;

	GSocket * socket = g_gopher_socket_connect("localhost", 70, error);

	if (socket) {
		locator = strdup("/about/antono\n");

		g_socket_send(socket, locator, strlen(locator), NULL, NULL);
		
		while ( (size = g_socket_receive(socket, buffer, 4096, NULL, NULL)) ) {
			g_print("\n\n=>> Got %d bytes\n\n", total += size);
			page = g_strjoin(NULL, page, g_strdup(buffer));
		}

	} else {
		g_warning("%s", error->message);
		g_free(error);
	}

	g_debug("What we got:");
	g_print("%s\n", page);
	g_debug("Done!");

	return page;
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

	/*gchar *uris  = strdup("gopher://antono.info/\nhttp://antono.info/");*/
	/*list  **urls = g_uri_list_extract_uris (uris);*/

	/*while (*urls++ != NULL) {*/
	/*       g_print("%s\n", uris++);*/
	/*}*/


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

