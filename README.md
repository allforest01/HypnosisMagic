# HypnosisMagic

## Requirements

### macOS
```
brew install gcc
brew install make
brew install opencv
brew install sdl2
brew install glew
```

### Windows (MSYS2/UCRT)
```
pacman -S base-devel
pacman -S mingw-w64-ucrt-x86_64-toolchain
pacman -S mingw-w64-ucrt-x86_64-opencv
pacman -S mingw-w64-ucrt-x86_64-SDL2
pacman -S mingw-w64-ucrt-x86_64-glew
```

## Build and run

### macOS
```
make BUILD_TYPE=server && ./build/macOS/server
```
```
make BUILD_TYPE=client && ./build/macOS/client
```

### Windows (MSYS2/UCRT)
```
make BUILD_TYPE=server && ./build/Windows/server.exe
```
```
make BUILD_TYPE=client && ./build/Windows/client.exe
```
