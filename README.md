# Grncache

Grncache is a simple plugin to investigate Groonga's internal cache.

## How to setup

Note that Grncache requires Groonga's source code because it uses internal headers.

```
% ./autogen.sh
% ./configure --with-groonga-source-dir=PATH_TO_GROONGA_SOURCE_DIRECTORY
% make
% sudo make install
```

Confirm `grncache.so` is correctly installed under "`pkg-config --variable=pluginsdir groonga`/grncache". Typically, that path is equivalent to `/usr/lib/groonga/plugins/grncache/grncache.so`.

## How to use

### Prerequisite

Register `Grncache` plugin.

```
> plugin_register grncache/grncache
```

Or

```
% curl http://localhost:10041/d/plugin_register?name=grncache/grncache
```

### Command line interface

Here is the example how to show `Grncache` status.

```
> grncache status
[[0,1435596749.90995,4.48226928710938e-05],{"cache_entries":0,"max_cache_entries":100,"cache_fetched":0,"cache_hit":0,"cache_hit_rate":0.0}]
```

Here is the example how to show `Grncache` all entries.

```
> grncache dump
[[0,1435596809.91652,4.22000885009766e-05],,[[3],[{"grn_id":3,"nref":0,"timeval":"2015-06-30 01:53:29.916194","value":"[[[1],[[\"_id\",\"UInt32\"],[\"_key\",\"ShortText\"],[\"title\",\"ShortText\"]],[3,\"http://pgroonga.github.io/\",\"PGroonga!\"]]]"},{"grn_id":2,"nref":0,"timeval":"2015-06-30 01:53:29.915816","value":"[[[1],[[\"_id\",\"UInt32\"],[\"_key\",\"ShortText\"],[\"title\",\"ShortText\"]],[2,\"http://mroonga.org/\",\"Mroonga!\"]]]"},{"grn_id":1,"nref":0,"timeval":"2015-06-30 01:53:29.915307","value":"[[[1],[[\"_id\",\"UInt32\"],[\"_key\",\"ShortText\"],[\"title\",\"ShortText\"]],[1,\"http://groonga.org/\",\"Groonga!\"]]]"}]]
```

Here is the example how to show `Grncache` which contains "github".

```
> grncache match github
[[0,1435596809.91657,2.43186950683594e-05],,[[3],[{"grn_id":3,"nref":0,"timeval":"2015-06-30 01:53:29.916194","value":"[[[1],[[\"_id\",\"UInt32\"],[\"_key\",\"ShortText\"],[\"title\",\"ShortText\"]],[3,\"http://pgroonga.github.io/\",\"PGroonga!\"]]]"}]]
```

### HTTP query

Here is the example how to show `Grncache` status.

```
% curl http://localhost:10041/d/grncache?action=status
```

Here is the example how to show `Grncache` all entries.

```
% curl http://localhost:10041/d/grncache?action=dump
```

## Limitations

`Grncache` requires Groonga 3.0.8 or later because it uses `grn_cache_current_get()` API.

## TODO

* filter dump data
