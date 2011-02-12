GtkBuilder		*builder;
ClutterActor		*stage;
ClutterActor		*viewport;
ZzPage			*current_page = NULL;
GtkWidget		*addressbar;
gint			scroll_factor = 30;

void zz_open (gchar *uri, gboolean is_menu);
void zz_open_handle_result (GObject *source, GSimpleAsyncResult *result, gpointer user_data);
void zz_display_page(ZzPage *page);
