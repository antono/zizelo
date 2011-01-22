#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#include "guri.h"

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

gchar * g_gopher_get (gchar *url) {

	GURI    * uri 	= g_uri_new(url);
	GError  * error = NULL;
	GString * page	= g_string_new("");

	gchar   buffer[1024];
	gint	buffpos  = 0;
	gint	total    = 0;
	gint	received = 0;

	if (!uri->port) uri->port = 70;
	if (!uri->path) uri->path = strdup("/");

	g_debug(uri->hostname);
	g_debug(uri->path);
	g_print("%d", uri->port);

	GSocket * socket = g_gopher_socket_connect(uri->hostname, uri->port, error);

	if (socket) {
		gchar * locator = g_strjoin(NULL, uri->path, "\n", NULL);
		g_socket_send(socket, locator, strlen(locator), NULL, NULL);
		g_free(locator);
		
		while ( (received = g_socket_receive (socket, buffer, 1024, NULL, NULL)) ) {
			for (buffpos = 0; buffpos < received; buffpos++) {
				g_string_append_c(page, buffer[buffpos]);
			}
			g_print("\n\n=>> Got %d/%d bytes\n\n", received, total += received);
		}
		/**page++ = NULL;*/
		/*while (total--) page--;*/
	} else {
		g_warning("%s", error->message);
		g_free(error);
	}

	g_free(uri);
	g_debug("What we got:");
	g_print("%s\n", page->str);
	g_debug("Done!");

	return page->str;
}
