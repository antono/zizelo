#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#include "guri.h"

GSocket * g_gopher_socket_connect(gchar *host, gint port, GError *error) {

	GSocket			* socket;
	GSocketClient		* client;
	GSocketConnection	* connection;
	GSocketConnectable	* addr;
	GTimer			* timer;

	timer = g_timer_new();
	g_timer_start(timer);

	addr		= g_network_address_new (host, port);
	client		= g_socket_client_new ();
	connection	= g_socket_client_connect (client, addr, NULL, &error);
	socket		= g_socket_connection_get_socket (connection);

	g_timer_stop(timer);
	g_debug ("Connection took %f", g_timer_elapsed(timer, NULL));
	g_timer_destroy(timer);

	if (socket) {
		return socket;
	} else {
		g_debug("Cannot create socket");
		return NULL;
	}
}

GString * g_gopher_get (gchar *url, GSimpleAsyncResult *result) {

	GURI    * uri 	= g_uri_new (url);
	GError  * error = NULL;
	GString * page	= g_string_new ("");
	GTimer	* timer = g_timer_new ();

	gchar   buffer[1024];
	gint	buffpos  = 0;
	gint	total    = 0;
	gint	received = 0;

	if (!uri->port) uri->port = 70;
	if (!uri->path) uri->path = strdup("/");

	g_debug("%s", uri->hostname);
	g_debug("%s", uri->path);
	g_debug("%d", uri->port);

	GSocket * socket = g_gopher_socket_connect(uri->hostname, uri->port, error);

	if (socket) {
		g_timer_start (timer);

		gchar * locator = g_strjoin(NULL, uri->path, "\n", NULL);
		g_socket_send_with_blocking(socket, locator, strlen(locator), FALSE, NULL, NULL);
		g_free(locator);
		
		while ( (received = g_socket_receive_with_blocking (socket, buffer, 1024, FALSE, NULL, NULL)) ) {
			for (buffpos = 0; buffpos < received; buffpos++) {
				g_string_append_c(page, buffer[buffpos]);
			}
			/*g_print("\n\n=>> Got %d/%d bytes\n\n", received, total += received);*/
		}

		g_timer_stop (timer);
		g_debug("Got page in %f", g_timer_elapsed (timer, NULL));
		g_timer_destroy (timer);
	} else {
		g_warning("%s", error->message);
		g_free(error);
	}



	/* TODO Convert if needed */

	/*gchar * page_utf8;*/

	/*if (error) {*/
	/*        g_debug(error->message);*/
	/*        g_free(error);*/
	/*} else {*/
	/*        g_debug("No errors in conversion");*/
	/*}*/
	/*if (g_utf8_validate (page->str, -1, NULL)) {*/
	/*        page_utf8 = page->str;*/
	/*} else {*/
	/*        g_debug("Trying to convert");*/
	/*        page_utf8 = g_convert(page->str, -1, "UTF-8", "ASCII", NULL, NULL, &error);*/
	/*        g_debug("Done");*/
	/*        g_debug("Printing UTF-8 page:");*/
	/*        g_print("%s", page_utf8);*/
	/*}*/


	g_free(uri);

	/*g_debug("What we got:");*/
	/*g_print("%s\n", page->str);*/
	/*g_debug("Done!");*/


	if (result && page != NULL)
	{
		g_simple_async_result_set_op_res_gpointer (result, page, g_string_free);
		g_debug("here");
		g_simple_async_result_complete (result);
		g_object_unref (result);
		return;
	}

	return page;
}

//
// Async Networking
//

void
g_gopher_get_async (GObject *self, gchar *url, GAsyncReadyCallback callback, gpointer user_data)
{
	GString *string;
	GSimpleAsyncResult *result;

	result = g_simple_async_result_new (NULL,
			callback,
			user_data,
			g_gopher_get_async);

	g_gopher_get(url, result);
}


GString *
g_gopher_get_finish (GObject *self, GAsyncResult *result, GError **error)
{
	GSimpleAsyncResult *simple;
	GString *string;

	simple = (GSimpleAsyncResult *) result;
	string = g_simple_async_result_get_op_res_gpointer (simple);

	return string;
}
