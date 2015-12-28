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

#include <groonga.h>
#include <groonga/plugin.h>

#ifdef USE_ONIGURUMA
#include <oniguruma.h>
#endif

#include "grncache.h"

/* status [HEADER, {CACHE_STATISTICS}] */
static void
output_grncache_status(grn_ctx *ctx, grn_cache_statistics *statistics)
{
  double cache_hit_rate;

  grn_ctx_output_map_open(ctx, "result", 5);
  grn_ctx_output_cstr(ctx, "cache_entries");
  grn_ctx_output_int32(ctx, statistics->nentries);
  grn_ctx_output_cstr(ctx, "max_cache_entries");
  grn_ctx_output_int32(ctx, statistics->max_nentries);
  grn_ctx_output_cstr(ctx, "cache_fetched");
  grn_ctx_output_int32(ctx, statistics->nfetches);
  grn_ctx_output_cstr(ctx, "cache_hit");
  grn_ctx_output_int32(ctx, statistics->nhits);
  grn_ctx_output_cstr(ctx, "cache_hit_rate");
  if (statistics->nfetches == 0) {
    grn_ctx_output_float(ctx, 0.0);
  } else {
    cache_hit_rate = (double)statistics->nhits / (double)statistics->nfetches;
    grn_ctx_output_float(ctx, cache_hit_rate * 100.0);
  }
  grn_ctx_output_map_close(ctx);
}

/* dump [HEADER, [[N], [{CACHE_ENTRY},...]]],*/
static void
output_grncache_dump(grn_ctx *ctx, grn_cache *cache, grn_cache_statistics *statistics, const char *text)
{
  grn_cache_entry *entry;
  uint32_t nentries;
  char buf[GRN_TIMEVAL_STR_SIZE];

#ifdef USE_ONIGURUMA
  int ret;
  regex_t *regex;
  OnigErrorInfo errorInfo;

  if (text) {
    ret = onig_new(&regex, (OnigUChar*)text, (OnigUChar*)text + strlen(text),
                   ONIG_OPTION_SINGLELINE,
                   ONIG_ENCODING_UTF8,
                   ONIG_SYNTAX_DEFAULT, &errorInfo);
    if (ret != ONIG_NORMAL) {
      fprintf(stderr, "failed to onig_new\n");
    }
  }
#endif

  grn_ctx_output_array_open(ctx, "result", 2);
  grn_ctx_output_array_open(ctx, "cache_count", 1);
  grn_ctx_output_int32(ctx, statistics->nentries);
  grn_ctx_output_array_close(ctx);

  if (statistics->nentries == 0) {
    grn_ctx_output_array_close(ctx);
    return;
  }

  grn_ctx_output_array_open(ctx, "CACHE_ENTRIES", statistics->nentries);
  entry = cache->next;
  nentries = statistics->nentries;
  while (entry && nentries > 0) {
#ifdef USE_ONIGURUMA
    if (text) {
      ret = onig_search(regex,
                        (UChar*)GRN_TEXT_VALUE(entry->value),
                        (UChar*)GRN_TEXT_VALUE(entry->value) + GRN_TEXT_LEN(entry->value),
                        (UChar*)GRN_TEXT_VALUE(entry->value),
                        (UChar*)GRN_TEXT_VALUE(entry->value) + GRN_TEXT_LEN(entry->value),
                        NULL,
                        ONIG_OPTION_POSIX_REGION);
      if (ret < 0) {
        entry = entry->next;
        nentries--;
        continue;
      }
    }
#endif
    grn_ctx_output_map_open(ctx, "cache_entry", 4);
    grn_ctx_output_cstr(ctx, "grn_id");
    grn_ctx_output_int32(ctx, entry->id);
    grn_ctx_output_cstr(ctx, "nref");
    grn_ctx_output_int32(ctx, entry->nref);
    grn_ctx_output_cstr(ctx, "timeval");
#if GRN_CHECK_VERSION(5,0,3)
    grn_timeval2str(ctx, &(entry->tv), &buf[0], GRN_TIMEVAL_STR_SIZE);
#else
    grn_timeval2str(ctx, &(entry->tv), &buf[0]);
#endif
    grn_ctx_output_str(ctx, buf, strlen(buf));
    grn_ctx_output_cstr(ctx, "value");
    grn_ctx_output_str(ctx, GRN_TEXT_VALUE(entry->value), GRN_TEXT_LEN(entry->value));
    grn_ctx_output_map_close(ctx);
    entry = entry->next;
    nentries--;
  }
  grn_ctx_output_array_close(ctx);
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
  grn_obj *var;

  cache = grn_cache_current_get(ctx);

  grn_cache_get_statistics(ctx, cache, &statistics);

  var = grn_plugin_proc_get_var_by_offset(ctx, user_data, 0);
  if (GRN_TEXT_LEN(var) > 0) {
    query = GRN_TEXT_VALUE(var);
    if (strcmp("status", GRN_TEXT_VALUE(var)) == 0) {
      mode = GRN_CACHE_STATUS;
    } else if (strcmp("dump", GRN_TEXT_VALUE(var)) == 0) {
      mode = GRN_CACHE_DUMP;
    } else if (strcmp("match", GRN_TEXT_VALUE(var)) == 0) {
      mode = GRN_CACHE_MATCH;
      var = grn_plugin_proc_get_var_by_offset(ctx, user_data, 1);
      if (GRN_TEXT_LEN(var) > 0) {
        query = GRN_TEXT_VALUE(var);
      }
    } else {
      /* --status something */
      mode = GRN_CACHE_STATUS;
    }
  } else if (GRN_TEXT_LEN(grn_plugin_proc_get_var_by_offset(ctx, user_data, 1)) > 0) {
    /* --dump something */
    mode = GRN_CACHE_DUMP;
  } else if (GRN_TEXT_LEN(grn_plugin_proc_get_var_by_offset(ctx, user_data, 2)) > 0) {
    /* --match something */
    mode = GRN_CACHE_MATCH;
    query = GRN_TEXT_VALUE(grn_plugin_proc_get_var_by_offset(ctx, user_data, 2));
  } else {
    GRN_LOG(ctx, GRN_LOG_ERROR,
            "nonexistent grncache option: <%s>",
            GRN_TEXT_VALUE(var));
    return NULL;
  }
  switch (mode) {
  case GRN_CACHE_STATUS:
    output_grncache_status(ctx, &statistics);
    break;
  case GRN_CACHE_DUMP:
    output_grncache_dump(ctx, cache, &statistics, query);
    break;
  case GRN_CACHE_MATCH:
    if (!query) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "empty query to match:");
    }
    output_grncache_dump(ctx, cache, &statistics, query);
    break;
  default:
    GRN_LOG(ctx, GRN_LOG_ERROR,
            "nonexistent option name: <%s>",
            GRN_TEXT_VALUE(var));
    return NULL;
  }

  return NULL;
}

grn_rc
GRN_PLUGIN_INIT(grn_ctx *ctx)
{
#ifdef USE_ONIGURUMA
  onig_init();
#endif
  return ctx->rc;
}

grn_rc
GRN_PLUGIN_REGISTER(grn_ctx *ctx)
{
  grn_expr_var vars[3];
#if GRN_CHECK_VERSION(4,0,3)
  grn_plugin_expr_var_init(ctx, &vars[0], "status", -1);
  grn_plugin_expr_var_init(ctx, &vars[1], "dump", -1);
  grn_plugin_expr_var_init(ctx, &vars[2], "match", -1);
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
#ifdef USE_ONIGURUMA
  onig_end();
#endif
  return ctx->rc;
}
