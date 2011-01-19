#include <gio/gio.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>
#include <stdlib.h>
#include <string.h>

ClutterActor *stage = NULL;

gchar g_gopher_request(gchar *);

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
	gchar * url = gtk_entry_get_text(entry);
	gchar * gopher_url_shema = "gopher://";
	gchar * full_url;

	if (!strcasestr(url, gopher_url_shema)) {
		full_url = g_strdup_printf("%s%s", gopher_url_shema, url);
		gtk_entry_set_text(entry, full_url);
	} else {
		full_url = url;
	}

	g_debug("Requested %s\n", full_url);
	g_gopher_request(full_url);

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

gchar g_gopher_request (gchar *url) {

	GSocket * socket;
	GError  * error;
	gchar   * locator;
	gchar   * page;

	gchar   buffer[2048];
	gint 	total 	= 0;
	gint 	size 	= 0;

	socket = g_gopher_socket_connect("antono.info", 70, &error);

	if (socket) {
		locator = strdup("/about/antono\n");

		g_socket_send(socket, locator, strlen(locator), NULL, NULL);
		
		while ( (size = g_socket_receive(socket, buffer, 2048, NULL, NULL)) ) {
			g_print("\n\n=>> Got %d bytes\n\n", total += size);
			g_print("%s", buffer);
		}

		g_debug("Done!");
	} else {
		g_warning("%s", error->message);
		error = NULL;
	}
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

int main(int argc, char *argv[])
{
  ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff }; /* Black */

  gtk_clutter_init (&argc, &argv);

  /* create window */
  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "hide", G_CALLBACK(gtk_main_quit), NULL);

  /* Vbox */
  GtkWidget *vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show (vbox);

  /* Addressbar */
  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_max_length (entry, 255);
  gtk_entry_set_icon_from_stock (entry, GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_OPEN);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
  gtk_widget_show (entry);

  g_signal_connect (entry, "changed",
		  G_CALLBACK(on_addressbar_change), NULL);
  g_signal_connect (entry, "activate",
		  G_CALLBACK(on_addressbar_activate), NULL);

  /* create stage */
  GtkWidget *clutter_widget = gtk_clutter_embed_new();
  gtk_box_pack_start(GTK_BOX (vbox), clutter_widget, TRUE, TRUE, 0);
  gtk_widget_show(clutter_widget);

  gtk_widget_set_size_request(clutter_widget, 200, 100);

  /* Get the stage and set its size and color: */
  stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(clutter_widget));
  clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);

  /* Show the stage: */
  clutter_actor_show (stage);

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

