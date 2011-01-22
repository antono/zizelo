typedef struct {
  const GArray  *parts;
  const GString *raw_page;
} GlopherPage;


typedef struct {
  const ClutterActor *link;
  const gchar uri;
  const gchar type;
  const gchar title;
} GlopherLink;

typedef struct {
  const ClutterText *text;
} GlopherText;

GlopherPage * gloper_page_new(GString *);
GlopherPage * gloper_page_parse(GlopherPage *);
