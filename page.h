typedef struct {
	gchar *uri;
	gchar *type;
	gboolean is_menu;
} ZzOpenData;

typedef struct {
	ClutterActor *actor;
	ClutterLayoutManager *layout;
	ZzOpenData *open_data;
	GString *raw_page;
	GString *raw_page_clean;
	GArray *parts;
} ZzPage;


typedef struct {
	ClutterActor *actor;
	gchar type;
	gchar *uri;
	gchar *title;
} ZzLink;

typedef struct {
	ClutterText *actor;
	gchar *raw;
} ZzText;

ZzPage	* zz_page_new(GString *, gboolean);
ZzPage	* zz_page_parse(ZzPage *);
ZzLink	* zz_link_new(gchar *, gchar *, gchar);
ZzText	* zz_text_new(gchar *);

gchar	* zz_gophermap_get_segment(gchar *, gint);
gchar	* zz_gopherlink_to_uri(gchar *, gchar *);
gchar	* zz_monospace(gchar *);

void	zz_page_add_link(ZzPage *, gchar *, gchar *, gchar);
void	zz_page_add_text(ZzPage *, gchar *);

gboolean on_link_click_cb (ClutterActor *actor, ClutterEvent *event, ZzLink *);
gboolean on_link_mouse_enter_cb (ClutterActor *actor, ClutterEvent *event, gpointer user_data);
gboolean on_link_mouse_leave_cb (ClutterActor *actor, ClutterEvent *event, gpointer user_data);
