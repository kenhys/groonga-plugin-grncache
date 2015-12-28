#ifndef __GRNCACHE_H_INCLUDED
#define __GRNCACHE_H_INCLUDED

#define GRN_CHECK_VERSION(major,minor,micro)				\
  (GRN_MAJOR_VERSION > (major) ||					\
   (GRN_MAJOR_VERSION == (major) && GRN_MINOR_VERSION > (minor)) ||	\
   (GRN_MAJOR_VERSION == (major) && GRN_MINOR_VERSION == (minor) && GRN_MICRO_VERSION >= (micro)))

/* Use internal definition */

#include <pthread.h>
typedef pthread_mutex_t grn_mutex;

#include <inttypes.h>
typedef struct {
  int64_t tv_sec;
  int32_t tv_nsec;
} grn_timeval;

#ifndef GRN_TIMEVAL_STR_SIZE
#define GRN_TIMEVAL_STR_SIZE 0x100
#endif

typedef struct {
  uint32_t nentries;
  uint32_t max_nentries;
  uint32_t nfetches;
  uint32_t nhits;
} grn_cache_statistics;

/* Use exported but not public function */
grn_rc grn_timeval2str(grn_ctx *ctx, grn_timeval *tv, char *buf, size_t buf_size);

/* Use private function, so grncache can not support Windows. */
void grn_cache_get_statistics(grn_ctx *ctx, grn_cache *cache,
                              grn_cache_statistics *statistics);

/* for Grncache internal use */

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
#endif
