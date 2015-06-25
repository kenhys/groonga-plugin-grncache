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

## Limitations

`Grncache` supports Groonga 4.0.3 or later.
