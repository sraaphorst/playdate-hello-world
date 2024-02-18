# playdate-hello-world

This is a test project using C exclusively to learn about the Playdate API.

The API is documented [here](https://sdk.play.date/2.0.1/Inside%20Playdate%20with%20C.html).

The song is:
[FASSounds - Good Night](https://pixabay.com/users/fassounds-3433550)

### Build Configuration

For CMake, (CMD-,), add a configuration called "Device" with:
* CMake options:
```
-DCMAKE_TOOLCHAIN_FILE=/Users/vorpal/Developer/PlaydateSDK/C_API/buildsupport/arm.cmake
```
* Build directory: `device`

### Run Configuration

* Name: `Device`
* Target: `hello_world_DEVICE`
* Executable: `$HOME/Developer/PlaydateSDK/bin/Playdate Simulator.app/Contents/MacOS/Playdate Simulator`
* Program Arguments: `$HOME/dev/C/playdate-hello-world/hello_world.pdx`

## Building

* This should produce the `hello_world.pdx` directory.

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Alternatively, you can just run `make` from the main directory.

* You can then run the device configuration.