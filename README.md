# SysY Runtime Library

The source code of the SysY runtime library.

## Building from Source

Run the following command lines:

```sh
git clone https://github.com/pku-minic/sysy-runtime-lib.git
cd sysy-runtime-lib
make
# or `make NO_LIBC=1`
```

The output (`libsysy.a`) will be generated in directory `build`.

## Running Test

```sh
make test
```

## Copyright and License

Copyright (C) 2022-2026 MaxXing. License GPLv3.
