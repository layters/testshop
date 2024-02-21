[![License](https://img.shields.io/github/license/i2p/i2psam.svg)](https://github.com/i2p/i2psam/blob/master/LICENSE)

# i2psam

A C++ library for the [SAM v3 API](https://geti2p.net/en/docs/api/samv3).

## Development Status

The library will require SAM v3.1 server.  
Pre-release (ongoing refactoring work and migration to C++11).

## Usage

### Library

Copy the files `i2psam.cpp` and `i2psam.h` into your codebase.  
You can also build the library `libi2psam.a`:

```
make
```

### Example

See `eepget.cpp` for example TCP client usage.  
Build with:

```
make eepget
```
