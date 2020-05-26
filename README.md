# MYL Game Engine


## Building on MacOS

Init & update submodules to fetch dependencies
```sh
git submodule init
git submodule update
```

Install build tools & system dependencies
```sh
brew install cmake make spdlog sfml boost
```

Create and switch into build dir
```sh
mkdir -p build
cd build
```

Run cmake
```sh
cmake ..
```

Run make with `MACOSX_DEPLOYMENT_TARGET` set to the version of an installed MacOS SDK.

SDKs are installed in `/Library/Developer/CommandLineTools/SDKs/`. For `10.15` to work you need to have `MacOSX10.15.sdk` in this directory.

For mor info see this [issue](https://github.com/LuaJIT/LuaJIT/issues/538#issuecomment-566476097) and this stack overflow [answer](https://apple.stackexchange.com/a/359049/254201)

```sh
env MACOSX_DEPLOYMENT_TARGET="10.15" make
```
