# Welcome to the hell
This is a project aiming to add new things to classic Voxlap using DLL injection, right now the code looks really disgusting, shit, awful, and spaghetti... Later we fix it, or just make it worst.

Check what aos.dll differs from classic Ace of Spades: [CHANGES.md](CHANGES.md)

### Building
First of all you need GCC and git, https://www.mingw-w64.org/

```sh
git clone https://github.com/DryByte/aos.dll.git && cd aos.dll
```
```sh
git submodule update --recursive --init
```
```sh
mkdir build && cd build
cmake .. && make
```
After compiling it, copy libaos.dll to your AoS folder and patch the client.exe

### Known issues
* RapidJson related error in wine-discord-rpc
Clean your build folder and add the following flags to cmake `-DRAPIDJSONTEST=OFF -DRAPIDJSON=../Extern/wine-discord-rpc/thirdparty/rapidjson-1.1.0`

### Installing/injecting
You can use your favorite DLL injector, or use [AoS Patcher](https://github.com/DryByte/AoS-Patcher) for auto inject when game opens up

## Contributing
Feel free to fork, star, report issues, open PRs, etc.