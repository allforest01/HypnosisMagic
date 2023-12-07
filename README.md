# HypnosisMagic

## Requirements

### Mac OS X
```
brew install gcc
brew install make
brew install opencv
brew install sdl2
brew install glew
```

### MSYS2
```
pacman -S mingw-w64-ucrt-x86_64-toolchain
pacman -S mingw-w64-ucrt-x86_64-make
pacman -S mingw-w64-ucrt-x86_64-opencv
pacman -S mingw-w64-ucrt-x86_64-SDL2
pacman -S mingw-w64-ucrt-x86_64-glew
```

## Build and run

### Server
`make BUILD_TYPE=server && ./src/server`

### Client
`make BUILD_TYPE=client && ./src/client`