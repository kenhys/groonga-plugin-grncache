/* -*- c-basic-offset: 2; indent-tabs-mode: nil -*- */
/* Copyright(C) 2015 HAYASHI Kentaro <kenhys@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define GRN_CHECK_VERSION(major,minor,micro)				\
  (GRN_MAJOR_VERSION > (major) ||					\
   (GRN_MAJOR_VERSION == (major) && GRN_MINOR_VERSION > (minor)) ||	\
   (GRN_MAJOR_VERSION == (major) && GRN_MINOR_VERSION == (minor) && GRN_MICRO_VERSION >= (micro)))

#include "config.h"

#include <string.h>

#if GRN_CHECK_VERSION(4,0,8)
  #include "grn.h"
  #include "grn_ctx.h"
  #include "grn_db.h"
  #include "grn_ii.h"
  #include "grn_output.h"
#else
  #include "groonga_in.h"
  #include "ctx_impl.h"
  #include "db.h"
  #include "ii.h"
  #include "output.h"
#endif
#include <groonga/plugin.h>

#define VAR GRN_PROC_GET_VAR_BY_OFFSET
#define TEXT_VALUE_LEN(x) GRN_TEXT_VALUE(x), GRN_TEXT_LEN(x)

typedef struct _grn_cache_entry grn_cache_entry;

struct _grn_cache {
  grn_cache_entry *next;
  grn_cache_entry *prev;
  grn_hash *hash;
  grn_mutex mutex;
  uint32_t max_nentries;
  uint32_t nfetches;
  uint32_t nhits;
};

struct _grn_cache_entry {
  grn_cache_entry *next;
  grn_cache_entry *prev;
  grn_obj *value;
  grn_timeval tv;
  grn_id id;
  uint32_t nref;
};

/* status [HEADER, {CACHE_STATISTICS}] */
static void
output_grncache_status(grn_ctx *ctx, grn_cache_statistics *statistics)
{
  double cache_hit_rate;

  GRN_OUTPUT_MAP_OPEN("RESULT", 5);
  GRN_OUTPUT_CSTR("cache_entries");
  GRN_OUTPUT_INT32(statistics->nentries);
  GRN_OUTPUT_CSTR("max_cache_entries");
  GRN_OUTPUT_INT32(statistics->max_nentries);
  GRN_OUTPUT_CSTR("cache_fetched");
  GRN_OUTPUT_INT32(statistics->nfetches);
  GRN_OUTPUT_CSTR("cache_hit");
  GRN_OUTPUT_INT32(statistics->nhits);
  GRN_OUTPUT_CSTR("cache_hit_rate");
  if (statistics->nfetches == 0) {
    GRN_OUTPUT_FLOAT(0.0);
  } else {
    cache_hit_rate = (double)statistics->nhits / (double)statistics->nfetches;
    GRN_OUTPUT_FLOAT(cache_hit_rate * 100.0);
  }
  GRN_OUTPUT_MAP_CLOSE();
}

/* dump [HEADER, [[N], [{CACHE_ENTRY},...]]],*/
static void
output_grncache_dump(grn_ctx *ctx, grn_cache *cache, grn_cache_statistics *statistics)
{
  grn_cache_entry *entry;
  uint32_t nentries;
  char buf[GRN_TIMEVAL_STR_SIZE];

  GRN_OUTPUT_ARRAY_OPEN("RESULT", 2);
  GRN_OUTPUT_ARRAY_OPEN("CACHE_COUNT", 1);
  GRN_OUTPUT_INT32(statistics->nentries);
  GRN_OUTPUT_ARRAY_CLOSE();

  if (statistics->nentries == 0) {
    GRN_OUTPUT_ARRAY_CLOSE();
    return;
  }

  GRN_OUTPUT_ARRAY_OPEN("CACHE_ENTRIES", statistics->nentries);
  entry = cache->next;
  nentries = statistics->nentries;
  while (entry && nentries > 0) {
    GRN_OUTPUT_MAP_OPEN("CACHE_ENTRY", 4);
    GRN_OUTPUT_CSTR("grn_id");
    GRN_OUTPUT_INT32(entry->id);
    GRN_OUTPUT_CSTR("nref");
    GRN_OUTPUT_INT32(entry->nref);
    GRN_OUTPUT_CSTR("timeval");
#if GRN_CHECK_VERSION(5,0,3)
    grn_timeval2str(ctx, &(entry->tv), &buf[0], GRN_TIMEVAL_STR_SIZE);
#else
    grn_timeval2str(ctx, &(entry->tv), &buf[0]);
#endif
    GRN_OUTPUT_STR(buf, strlen(buf));
    GRN_OUTPUT_CSTR("value");
    GRN_OUTPUT_STR(GRN_TEXT_VALUE(entry->value), GRN_TEXT_LEN(entry->value));
    GRN_OUTPUT_MAP_CLOSE();
    entry = entry->next;
    nentries--;
  }
  GRN_OUTPUT_ARRAY_CLOSE();
}

enum {
  GRN_CACHE_NONE,
  GRN_CACHE_STATUS,
  GRN_CACHE_DUMP,
  GRN_CACHE_MATCH,
};

static grn_obj *
command_grncache(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
  grn_cache *cache;
  grn_cache_statistics statistics;
  const char *query = NULL;
  int mode = GRN_CACHE_NONE;

  cache = grn_cache_current_get(ctx);

  grn_cache_get_statistics(ctx, cache, &statistics);

  if (GRN_TEXT_LEN(VAR(0)) > 0) {
    query = GRN_TEXT_VALUE(VAR(0));
    if (strcmp("status", GRN_TEXT_VALUE(VAR(0))) == 0) {
      mode = GRN_CACHE_STATUS;
    } else if (strcmp("dump", GRN_TEXT_VALUE(VAR(0))) == 0) {
      mode = GRN_CACHE_DUMP;
    } else if (strcmp("match", GRN_TEXT_VALUE(VAR(0))) == 0) {
      mode = GRN_CACHE_MATCH;
      if (GRN_TEXT_LEN(VAR(1)) > 0) {
        query = GRN_TEXT_VALUE(VAR(1));
      }
    } else {
      /* --status something */
      mode = GRN_CACHE_STATUS;
    }
  } else if (GRN_TEXT_LEN(VAR(1)) > 0) {
    /* --dump something */
    mode = GRN_CACHE_DUMP;
  } else if (GRN_TEXT_LEN(VAR(2)) > 0) {
    /* --match something */
    mode = GRN_CACHE_MATCH;
    query = GRN_TEXT_VALUE(VAR(2));
  } else {
    ERR(GRN_INVALID_ARGUMENT, "nonexistent grncache option: <%s>",
        GRN_TEXT_VALUE(VAR(0)));
    return NULL;
  }
  switch (mode) {
  case GRN_CACHE_STATUS:
    output_grncache_status(ctx, &statistics);
    break;
  case GRN_CACHE_DUMP:
    output_grncache_dump(ctx, cache, &statistics);
    break;
  case GRN_CACHE_MATCH:
    if (!query) {
      ERR(GRN_INVALID_ARGUMENT, "empty query to match:");
    }
    break;
  default:
    ERR(GRN_INVALID_ARGUMENT, "nonexistent option name: <%s>",
        GRN_TEXT_VALUE(VAR(0)));
    return NULL;
  }

  return NULL;
}

grn_rc
GRN_PLUGIN_INIT(grn_ctx *ctx)
{
  return ctx->rc;
}

grn_rc
GRN_PLUGIN_REGISTER(grn_ctx *ctx)
{
  grn_expr_var vars[3];
#if GRN_CHECK_VERSION(4,0,3)
  grn_plugin_expr_var_init(ctx, &vars[0], "status", -1);
  grn_plugin_expr_var_init(ctx, &vars[1], "dump", -1);
  grn_plugin_expr_var_init(ctx, &vars[2], "search", -1);
  grn_plugin_command_create(ctx, "grncache", -1, command_grncache, 3, vars);
#else

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0
#define DEF_VAR(v,x) do {                       \
  (v).name = (x);\
  (v).name_size = (x) ? sizeof(x) - 1 : 0;\
  GRN_TEXT_INIT(&(v).value, 0);\
} while (0)

#define DEF_COMMAND(name,func,nvars,vars)\
  (grn_proc_create(ctx, CONST_STR_LEN(name),\
                   GRN_PROC_COMMAND, (func), NULL, NULL, (nvars), (vars)))

  DEF_VAR(vars[0], "action");
  DEF_COMMAND("grncache", command_grncache, 1, vars);
#undef DEF_VAR
#undef DEF_COMMAND
#endif
  return ctx->rc;
}

grn_rc
GRN_PLUGIN_FIN(grn_ctx *ctx)
{
  return ctx->rc;
}
