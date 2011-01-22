#include <clutter/clutter.h>
#include "gophermap.h"

ClutterText * g_gopher_map_new(gchar *string) {
	return CLUTTER_TEXT(clutter_text_new());
}
