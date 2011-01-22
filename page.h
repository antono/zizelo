typedef struct {
  const GArray  *parts;
  const GString *raw_page;
} ZizeloPage;


typedef struct {
  const ClutterActor *link;
  const gchar type;
  const gchar uri;
  const gchar title;
} ZizeloLink;

typedef struct {
  const ClutterText *text;
} ZizeloText;

ZizeloPage * zz_page_new(GString *);
ZizeloPage * zz_page_parse(ZizeloPage *);
