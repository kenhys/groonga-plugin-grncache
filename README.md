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

Confirm `grncache.so` is correctly installed under `pkg-config --variable=pluginsdir groonga`/grncache. Typically, that path is equivalent to `/usr/lib/groonga/plugins/grncache/grncache.so`.

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
```

Here is the example how to show `Grncache` all entries.

```
> grncache dump
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

`Grncache` requires Groonga 3.0.8 or later.

## TODO

* filter dump data
