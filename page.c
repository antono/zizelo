#include <glib.h>
#include <clutter/clutter.h>
#include <gio/gio.h>
#include <page.h>
#include <stdlib.h>
#include <string.h>
#include <guri.h>

ZizeloPage	* zz_page_new (GString *);
ZizeloPage	* zz_page_parse (ZizeloPage *);
gchar		* zz_gophermap_get_segment(gchar *, gint);
gchar		* zz_gopherlink_to_uri(gchar *, gchar *);

ZizeloPage * zz_page_new (GString *raw_page) {
	ZizeloPage *page;
	page = g_new(ZizeloPage, 1);
	page->parts = g_array_new(TRUE, TRUE, 10);
	page->raw_page = raw_page;
	return page;
}

ZizeloPage * zz_page_parse (ZizeloPage *page) {

	gchar **lines = g_strsplit(page->raw_page->str, "\r\n", -1);
	gchar *line;
	gchar type;

	while (lines && !(strcmp(*lines, ".") == 0) ) {
		line = *lines;
		type = line[0];
		line++;
		gchar * desc = zz_gophermap_get_segment(line, 0);
		if (type == 'i') {
			g_debug("STRPD:\t%s", desc);
		}
		if (type == '0') {
			g_debug("DIR.:\t%s\t%s", desc, zz_gopherlink_to_uri(strdup("gopher"), line));
		}
		if (type == '1') {
			g_debug("FILE:\t%s\t%s", desc, zz_gopherlink_to_uri("gopher", line));
		}
		if (type == '8') {
			g_debug("FILE:\t%s\t%s", desc, zz_gopherlink_to_uri("telnet", line));
		}
		if (type == 'h') {
			g_debug("FILE:\t%s\t%s", desc, zz_gopherlink_to_uri("http", line));
		}
		lines++;
	}

	return page;
}

gchar * zz_gophermap_get_segment(gchar *line, gint num) {
	gchar **segments = g_strsplit(line, "\t", -1);
	/*g_debug("SEGMENT #%i: %s", num, segments[0]);*/
	return segments[num];
}

gchar * zz_gopherlink_to_uri(gchar *schema, gchar *line) {
	GURI *uri = g_uri_new_fields(schema,
			zz_gophermap_get_segment(line, 2),
			atoi(zz_gophermap_get_segment(line, 3)),
			zz_gophermap_get_segment(line, 1));
	return g_uri_get_string(uri);
}

int main() {
	g_type_init();

	GFile *file = g_file_new_for_path("./gopher.txt");
	gchar *path = g_file_get_uri(file);

	if (path) {
	       g_debug("Reading: %s\n", path);
	} else {
	       g_debug("There is no path...");
	}

	gchar *raw = strdup("");

	GFileInputStream *stream = g_file_read(file, NULL, NULL);
	g_input_stream_read (stream, raw, 10000, NULL, NULL);

	GString *string = g_string_new(raw);
	zz_page_parse(zz_page_new(string));

	return 0;
}
