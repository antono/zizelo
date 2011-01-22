/* GNet - Networking library URI parser
 * Copyright (C) 2000-2003  David Helder, David Bolcsfoldi, Eric Williams
 * Copyright (C) 2007       Tim-Philipp Müller <tim centricular net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include <glib.h>
#include <string.h>
#include "guri.h"

static void   field_unescape (gchar *str);
static gchar* field_escape (gchar *str, guchar mask);

#define USERINFO_ESCAPE_MASK	0x01
#define PATH_ESCAPE_MASK	0x02
#define QUERY_ESCAPE_MASK	0x04
#define FRAGMENT_ESCAPE_MASK	0x08

static const guchar neednt_escape_table[] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x0f, 0x00, 0x0c, 
	0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x0f, 
	0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/*
Perl code to generate above table:

#!/usr/bin/perl

$ok = "abcdefghijklmnopqrstuvwxyz" . 
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ" .
      "0123456789" .
      "-_.!~*'()";
$userinfo_ok = ';:&=+\$,';
$path_ok     = ':\@&=+\$,;/';
$query_ok    = ';/?:\@&=+\$,';
$fragment_ok = ';/?:\@&=+\$,';

for ($i = 0; $i < 32; $i++)
{
    print "  ";
    for ($j = 0; $j < 8; $j++)
    {
	$num = 0;
	$letter = chr(($i * 8) + $j);

	$num |= 0b0001  if (index($userinfo_ok, $letter) != -1);
	$num |= 0b0010  if (index($path_ok,     $letter) != -1);
	$num |= 0b0100  if (index($query_ok,    $letter) != -1);
	$num |= 0b1000  if (index($fragment_ok, $letter) != -1);
	$num |= 0b1111  if (index($ok,          $letter) != -1);

	printf "0x%02x, ", $num;
    }
    print "\n";
}
*/

typedef struct {
  const gchar *str;
  guint len;
} StringPart;

#define DUP_STRING_PART(sp)  (sp.len > 0) ? g_strndup (sp.str, sp.len) : NULL

/* our own ISSPACE.  ANSI isspace is locale dependent and g_ascii_isspace()
 * does not recognise #10 as space */
#define ISSPACE(C) (((C) >= 9 && (C) <= 13) || (C) == ' ')

static gboolean
g_uri_parse (const gchar * uri, StringPart * scheme, StringPart * userinfo,
    StringPart * hostname, guint * port, StringPart * path,
    StringPart * query, StringPart * fragment)
{
  const StringPart empty = { NULL, 0 };
  const gchar *p, *temp;

  *scheme = *userinfo = *hostname = *path = *query = *fragment = empty;
  *port = 0;

  /* Skip initial whitespaces */
  p = uri;
  while (*p && ISSPACE(*p))
    ++p;

  /* Error out if it's just a string of space */
  if (*p == '\0')
    return FALSE;

  /* Scheme */
  temp = p;
  while (*p && *p != ':' && *p != '/' && *p != '?' && *p != '#')
    ++p;
  if (*p == ':') {
    scheme->str = temp;
    scheme->len = (guint) (p - temp);
    ++p;
  } else {
    /* This char is NUL, /, ?, or # */
    p = temp;
  }

  /* Authority */
  if (*p == '/' && p[1] == '/') {
    p += 2;

    /* Userinfo */
    temp = p;
    while (*p && *p != '@' && *p != '/' ) /* Look for @ or / */
      ++p;
    if (*p == '@') { /* Found userinfo */
      userinfo->str = temp;
      userinfo->len = (guint) (p - temp);
      ++p;
    } else {        
      p = temp;
    }

    /* Hostname */

    /* Check for no hostname at all (e.g. file:// URIs) */
    if (*p == '/')
      goto path;

    /* Check for IPv6 canonical hostname in brackets */
    if (*p == '[') {
      p++;  /* Skip [ */
      temp = p;

      while (*p && *p != ']')
        ++p;

      if ((p - temp) == 0)
        return FALSE;

      hostname->str = temp;
      hostname->len = (guint) (p - temp);

      if (*p)
        ++p;  /* Skip ] (if there) */
    } else {
      temp = p;
      while (*p && *p != '/' && *p != '?' && *p != '#' && *p != ':')
        ++p;

      if ((p - temp) == 0) 
        return FALSE;

      hostname->str = temp;
      hostname->len = (guint) (p - temp);
    }

    /* Port */
    if (*p == ':') {
      for (++p; isdigit((int)*p); ++p)
        *port = (*port) * 10 + (*p - '0');
    }
  }

  /* Path (we are liberal and won't check if it starts with /) */

path:

  temp = p;
  while (*p && *p != '?' && *p != '#')
    ++p;
  if (p != temp) {
    path->str = temp;
    path->len = (guint) (p - temp);
  }

  /* Query */
  if (*p == '?') {
    temp = p + 1;
    while (*p && *p != '#')
      ++p;
    query->str = temp;
    query->len = (guint) (p - temp);
  }

  /* Fragment */
  if (*p == '#') {
    ++p;
    fragment->str = p;
    fragment->len = strlen (p);
  }

  return TRUE;
}

static gchar *
parse_inplace_munge_string_part (StringPart * part)
{
  if (part->len == 0)
    return NULL;
  ((gchar*)part->str)[part->len] = '\0';
  return (gchar *) part->str;
}

/**
 *  g_uri_parse_inplace:
 *  @guri: pointer to an uninitialised #GURI structure (usually on the stack)
 *  @uri: a writable URI string (which will be modified by this function!)
 *  @hostname: a preallocated array of size @len, into which the hostname will
 *             be copied if the URI has a hostname. This array should live
 *             as long as @guri may be used, since @guri might reference it.
 *             You may pass NULL here if you know your URIs won't contain a
 *             hostname field
 *  @size: size of @hostname in bytes
 *
 *  This function parses an URI string very efficiently and without allocating
 *  any memory on the heap. It does this by modifying the passed-in string
 *  @uri and changing field separators into string terminators in place. The
 *  result will then be put into the already-allocated and usually
 *  uninitialised, GURI structure @guri.
 *
 *  Because the slash (') which separates the hostname from the path in an URI
 *  is needed as the first character of the path, it cannot be turned into a
 *  string terminator for the hostname. This makes it necessary for the caller
 *  (ie. you) to pass in an array where the hostname can be put.
 *
 *  If the URI has been parsed successfully, the fields of the GURI structure
 *  will either point into the given string @uri (or to @hostname in case of
 *  the hostname field), or be NULL. No freeing or de-initialising of the
 *  GURI structure is required.
 *
 *  The only g_uri_*() functions you are allowed to call with this URI
 *  structure are g_uri_clone(), g_uri_equal(), g_uri_hash(),
 *  g_uri_get_string() and g_uri_unescape() (the latter unescapes the
 *  string fragments in place, so is safe to use here).
 *
 *  You must not call any other g_uri_*() functions with this GURI,
 *  especially not any g_uri_set() functions.
 *
 *  Under normal circumstances, you should never need to use this function.
 *  Use g_uri_new() instead.
 *  
 *  Returns: TRUE if @uri was parsed successfully and #GURI is usable,
 *           or FALSE if there was a failure.
 *
 *  Since: 2.0.8
 *
 **/
gboolean
g_uri_parse_inplace (GURI * guri, gchar * uri, gchar * hostname, gsize len)
{
  StringPart scheme, usr, host, path, query, frag;
  guint port;

  if (!g_uri_parse (uri, &scheme, &usr, &host, &port, &path, &query, &frag))
    return FALSE;

  if (host.len >= len)
    return FALSE;

  if (host.len > 0) {
    if (hostname == NULL)
      return FALSE;

    strncpy (hostname, host.str, host.len);
    hostname[host.len] = '\0';
  }

  guri->scheme = parse_inplace_munge_string_part (&scheme);
  guri->userinfo = parse_inplace_munge_string_part (&usr);
  guri->hostname = (host.len > 0) ? hostname : NULL;
  guri->path = parse_inplace_munge_string_part (&path);
  guri->query = parse_inplace_munge_string_part (&query);
  guri->fragment = parse_inplace_munge_string_part (&frag);
  guri->port = port;

  return TRUE;
}

/**
 *  g_uri_new
 *  @uri: URI string
 *
 *  Creates a #GURI from a string.  Empty fields are set to NULL.  The
 *  parser does not validate the URI -- it will accept some malformed
 *  URI.  URIs are usually in the form
 *  scheme://userinfo@hostname:port/path?query#fragment
 *
 *  URIs created from user input are typically unescaped.  URIs
 *  created from machine input (e.g. received over the internet) are
 *  typically escaped.
 *  
 *  Returns: a new #GURI, or NULL if there was a failure.
 *
 **/
GURI *
g_uri_new (const gchar * uri)
{
  StringPart scheme, usr, host, path, query, frag;
  guint port;
  GURI *guri;

  g_return_val_if_fail (uri != NULL, NULL);

  if (!g_uri_parse (uri, &scheme, &usr, &host, &port, &path, &query, &frag))
    return NULL;

  guri = g_new (GURI, 1);
  guri->scheme = DUP_STRING_PART (scheme);
  guri->userinfo = DUP_STRING_PART (usr);
  guri->hostname = DUP_STRING_PART (host);
  guri->path = DUP_STRING_PART (path);
  guri->query = DUP_STRING_PART (query);
  guri->fragment = DUP_STRING_PART (frag);
  guri->port = port;

  return guri;
}


/**
 *  g_uri_new_fields
 *  @scheme: scheme
 *  @hostname: host name
 *  @port: port
 *  @path: path
 *
 *  Creates a #GURI from the fields.  This function uses the most
 *  common fields.  Use g_uri_new_fields_all() to specify all
 *  fields.
 *
 *  Returns: a new #GURI.
 *
 **/
GURI*     
g_uri_new_fields (const gchar* scheme, const gchar* hostname, 
		     const gint port, const gchar* path)
{
  GURI* uri = NULL;

  uri = g_new0 (GURI, 1);
  if (scheme)		uri->scheme = g_strdup (scheme);
  if (hostname)		uri->hostname = g_strdup (hostname);
  uri->port = port;
  if (path)		uri->path = g_strdup (path);

  return uri;
}


/**
 *  g_uri_new_fields_all
 *  @scheme: scheme
 *  @userinfo: user info
 *  @hostname: host name
 *  @port: port
 *  @path: path
 *  @query: query
 *  @fragment: fragment
 *
 *  Creates a #GURI from all fields.
 *
 *  Returns: a new #GURI.
 *
 **/
GURI*
g_uri_new_fields_all (const gchar* scheme, const gchar* userinfo, 
			 const gchar* hostname, const gint port, 
			 const gchar* path, 
			 const gchar* query, const gchar* fragment)
{
  GURI* uri = NULL;

  uri = g_new0 (GURI, 1);
  if (scheme)		uri->scheme   = g_strdup (scheme);
  if (userinfo)		uri->userinfo = g_strdup (userinfo);
  if (hostname)		uri->hostname = g_strdup (hostname);
  uri->port = port;
  if (path)		uri->path     = g_strdup (path);
  if (query)		uri->query    = g_strdup (query);
  if (fragment)		uri->fragment = g_strdup (fragment);

  return uri;
}


/**
 *  g_uri_clone:
 *  @uri: a #GURI
 * 
 *  Copies a #GURI.
 *
 *  Returns: a copy of @uri.
 *
 **/
GURI*     
g_uri_clone (const GURI* uri)
{
  GURI* uri2;

  g_return_val_if_fail (uri, NULL);

  uri2 = g_new0 (GURI, 1);
  uri2->scheme   = g_strdup (uri->scheme);
  uri2->userinfo = g_strdup (uri->userinfo);
  uri2->hostname = g_strdup (uri->hostname);
  uri2->port     = uri->port;
  uri2->path 	 = g_strdup (uri->path);
  uri2->query    = g_strdup (uri->query);
  uri2->fragment = g_strdup (uri->fragment);

  return uri2;
}


/** 
 *  g_uri_delete:
 *  @uri: a #GURI
 *
 *  Deletes a #GURI.
 *
 **/
void
g_uri_delete (GURI* uri)
{
  if (uri)
    {
      g_free (uri->scheme);
      g_free (uri->userinfo);
      g_free (uri->hostname);
      g_free (uri->path);
      g_free (uri->query);
      g_free (uri->fragment);
      g_free (uri);
    }
}




#define SAFESTRCMP(A,B) (((A)&&(B))?(strcmp((A),(B))):((A)||(B)))

/**
 *  g_uri_equal
 *  @p1: a #GURI
 *  @p2: another #GURI
 *
 *  Compares two #GURI's for equality.
 *
 *  Returns: TRUE if they are equal; FALSE otherwise.
 *
 **/
gboolean
g_uri_equal (gconstpointer p1, gconstpointer p2)
{
  const GURI* uri1 = (const GURI*) p1;
  const GURI* uri2 = (const GURI*) p2;

  g_return_val_if_fail (uri1, FALSE);
  g_return_val_if_fail (uri2, FALSE);

  if (uri1->port == uri2->port &&
      !SAFESTRCMP(uri1->scheme, uri2->scheme) &&
      !SAFESTRCMP(uri1->userinfo, uri2->userinfo) &&
      !SAFESTRCMP(uri1->hostname, uri2->hostname) &&
      !SAFESTRCMP(uri1->path, uri2->path) &&
      !SAFESTRCMP(uri1->query, uri2->query) &&
      !SAFESTRCMP(uri1->fragment, uri2->fragment))
    return TRUE;

  return FALSE;
}


/**
 *  g_uri_hash
 *  @p: a #GURI
 *
 *  Creates a hash code for @p for use with GHashTable. 
 *
 *  Returns: hash code for @p.
 *
 **/
guint
g_uri_hash (gconstpointer p)
{
  const GURI* uri = (const GURI*) p;
  guint h = 0;

  g_return_val_if_fail (uri, 0);

  if (uri->scheme)	h =  g_str_hash (uri->scheme);
  if (uri->userinfo)	h ^= g_str_hash (uri->userinfo);
  if (uri->hostname)	h ^= g_str_hash (uri->hostname);
  h ^= uri->port;
  if (uri->path)	h ^= g_str_hash (uri->path);
  if (uri->query)	h ^= g_str_hash (uri->query);
  if (uri->fragment)	h ^= g_str_hash (uri->fragment);
  
  return h;
}


/**
 *  g_uri_escape
 *  @uri: a #GURI
 *
 *  Escapes the fields in a #GURI.  Network protocols use escaped
 *  URIs.  People use unescaped URIs.
 *
 **/
void
g_uri_escape (GURI* uri)
{
  g_return_if_fail (uri);
  
  uri->userinfo = field_escape (uri->userinfo, USERINFO_ESCAPE_MASK);
  uri->path     = field_escape (uri->path,     PATH_ESCAPE_MASK);
  uri->query    = field_escape (uri->query,    QUERY_ESCAPE_MASK);
  uri->fragment = field_escape (uri->fragment, FRAGMENT_ESCAPE_MASK);
}


/**
 *  g_uri_unescape
 *  @uri: a #GURI
 *
 *  Unescapes the fields in the URI.  Network protocols use escaped
 *  URIs.  People use unescaped URIs.
 *
 **/
void
g_uri_unescape (GURI* uri)
{
  g_return_if_fail (uri);

  if (uri->userinfo)
    field_unescape (uri->userinfo);
  if (uri->path)
    field_unescape (uri->path);
  if (uri->query)
    field_unescape (uri->query);
  if (uri->fragment)
    field_unescape (uri->fragment);
}


static gchar*
field_escape (gchar *signed_str, guchar mask)
{
  gint len;
  gint i;
  gboolean must_escape = FALSE;
  guchar *str;
  gchar *dst;
  gint j;

  str = (guchar*) signed_str;

  if (str == NULL)
    return NULL;

  /* Roughly calculate buffer size */
  len = 0;
  for (i = 0; str[i]; i++)
    {
      if (neednt_escape_table[str[i]] & mask)
	len++;
      else
	{
	  len += 3;
	  must_escape = TRUE;
	}
    }

  /* Don't escape if unnecessary */
  if (must_escape == FALSE)
    return signed_str;
	
  /* Allocate buffer */
  dst = (gchar*) g_malloc(len + 1);

  /* Copy */
  for (i = j = 0; str[i]; i++, j++)
    {
      /* Unescaped character */
      if (neednt_escape_table[str[i]] & mask)
	{
	  dst[j] = str[i];
	}

      /* Escaped character */
      else
	{
	  dst[j] = '%';

	  if (((str[i] & 0xf0) >> 4) < 10)
	    dst[j+1] = ((str[i] & 0xf0) >> 4) + '0';
	  else
	    dst[j+1] = ((str[i] & 0xf0) >> 4) + 'a' - 10;

	  if ((str[i] & 0x0f) < 10)
	    dst[j+2] = (str[i] & 0x0f) + '0';
	  else
	    dst[j+2] = (str[i] & 0x0f) + 'a' - 10;

	  j += 2;  /* and j is incremented in loop too */
	}
    }
  dst[j] = '\0';

  g_free (signed_str);
  return dst;
}



static void
field_unescape (gchar* s)
{
  gchar* src;
  gchar* dst;

  for (src = dst = s; *src; ++src, ++dst)
    {
      if (src[0] == '%' && src[1] != '\0' && src[2] != '\0')
	{
	  gint high, low;

	  if ('a' <= src[1] && src[1] <= 'f')
	    high = src[1] - 'a' + 10;
	  else if ('A' <= src[1] && src[1] <= 'F')
	    high = src[1] - 'A' + 10;
	  else if ('0' <= src[1] && src[1] <= '9')
	    high = src[1] - '0';
	  else  /* malformed */
	    goto regular_copy;

	  if ('a' <= src[2] && src[2] <= 'f')
	    low = src[2] - 'a' + 10;
	  else if ('A' <= src[2] && src[2] <= 'F')
	    low = src[2] - 'A' + 10;
	  else if ('0' <= src[2] && src[2] <= '9')
	    low = src[2] - '0';
	  else  /* malformed */
	    goto regular_copy;

	  *dst = (char)((high << 4) + low);
	  src += 2;
	}
      else
	{
	regular_copy:
	  /* micro-optimisation: a lot of URIs do not include escape
	   * sequences. By testing the pointer addresses we can avoid
	   * a lot of reading+writing of the same data */
	  if (dst != src)
	    *dst = *src;
	}
    }

  *dst = '\0';
}



/**
 *  g_uri_get_string
 *  @uri: a #GURI
 *
 *  Gets a string representation of a #GURI.  This function does not
 *  escape or unescape the fields first.  Call g_uri_escape() or
 *  g_uri_unescape first if necessary.
 *
 *  Returns: a string.
 *
 **/
gchar*
g_uri_get_string (const GURI* uri)
{
  gchar* rv = NULL;
  GString* buffer = NULL;
  
  g_return_val_if_fail (uri, NULL);

  buffer = g_string_sized_new (16);

  if (uri->scheme)
    g_string_sprintfa (buffer, "%s:", uri->scheme);

  if (uri->userinfo  || uri->hostname  || uri->port ||
      (uri->scheme && uri->hostname == NULL && uri->path && *uri->path == '/'))
    {
      g_string_append (buffer, "//");
    }

  if (uri->userinfo)
    {
      buffer = g_string_append (buffer, uri->userinfo);
      buffer = g_string_append_c (buffer, '@');
    }

  /* Add brackets around the hostname if it's IPv6 */
  if (uri->hostname)
    {
      if (strchr(uri->hostname, ':') == NULL) 
	buffer = g_string_append (buffer, uri->hostname); 
      else
	g_string_sprintfa (buffer, "[%s]", uri->hostname);
    }

  if (uri->port)
    g_string_sprintfa (buffer, ":%d", uri->port);

  if (uri->path)
    {
      if (*uri->path == '/' ||
	  !(uri->userinfo || uri->hostname || uri->port))
	g_string_append (buffer, uri->path);
      else
	g_string_sprintfa (buffer, "/%s", uri->path);
    }

  if (uri->query)
    g_string_sprintfa (buffer, "?%s", uri->query);

  if (uri->fragment)
    g_string_sprintfa (buffer, "#%s", uri->fragment);
  
  /* Free only GString not data contained, return the data instead */
  rv = buffer->str;
  g_string_free (buffer, FALSE); 
  return rv;
}


/**
 *  g_uri_set_scheme
 *  @uri: a #GURI
 *  @scheme: scheme
 *
 *  Sets a #GURI's scheme.
 *
 **/
void
g_uri_set_scheme (GURI* uri, const gchar* scheme)
{
  g_return_if_fail (uri);

  g_free (uri->scheme);
  uri->scheme = NULL;

  if (scheme)
    uri->scheme = g_strdup (scheme);
}


/**
 *  g_uri_set_userinfo
 *  @uri: a #GURI
 *  @userinfo: user info
 *
 *  Sets a #GURI's user info.
 *
 **/
void
g_uri_set_userinfo (GURI* uri, const gchar* userinfo)
{
  g_return_if_fail (uri);

  g_free (uri->userinfo);
  uri->userinfo = NULL;

  if (userinfo)
    uri->userinfo = g_strdup (userinfo);
}


/**
 *  g_uri_set_hostname
 *  @uri: a #GURI
 *  @hostname: host name
 *
 *  Sets a #GURI's host name.
 *
 **/
void
g_uri_set_hostname (GURI* uri, const gchar* hostname)
{
  g_return_if_fail (uri);

  g_free (uri->hostname);
  uri->hostname = NULL;

  if (hostname)
    uri->hostname = g_strdup (hostname);
}


/**
 *  g_uri_set_port
 *  @uri: a #GURI
 *  @port: port
 *
 *  Set a #GURI's port.
 *
 **/
void	
g_uri_set_port (GURI* uri, gint port)
{
  uri->port = port;
}


/**
 *  g_uri_set_path
 *  @uri: a #GURI
 *  @path: path
 *
 *  Set a #GURI's path.
 *
 **/
void
g_uri_set_path (GURI* uri, const gchar* path)
{
  g_return_if_fail (uri);

  g_free (uri->path);
  uri->path = NULL;

  if (path)
    uri->path = g_strdup (path);
}



/**
 *  g_uri_set_query
 *  @uri: a #GURI
 *  @query: query
 *
 *  Set a #GURI's query.
 *
 **/
void
g_uri_set_query (GURI* uri, const gchar* query)
{
  g_return_if_fail (uri);

  g_free (uri->query);
  uri->query = NULL;

  if (query)
    uri->query = g_strdup (query);
}


/**
 *  g_uri_set_fragment
 *  @uri: a #GURI
 *  @fragment: fragment
 *
 *  Set a #GURI's fragment.
 *
 **/
void
g_uri_set_fragment (GURI* uri, const gchar* fragment)
{
  g_return_if_fail (uri);

  g_free (uri->fragment);
  uri->fragment = NULL;

  if (fragment)
    uri->fragment = g_strdup (fragment);
}
