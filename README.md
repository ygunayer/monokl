# monokl
Monokl is a lightweight and user-friendly image viewer that runs on Linux, MacOS and Windows.

![Screenshot on MacOS](./ss-macos.png)

> [!WARNING]  
This project is a work in progress.

## Usage
Drag and drop any number of files and folders onto your monokl window and it will load all valid images from in those folders non-recursively.

You can then browse those images using the right and left arrows, as well as home and end buttons. See the following list of keyboard shortcuts

| Key Combination | Action |
|-----------------|--------|
| Left Arrow | Go to previous image. Wraps around to the end |
| Right Arrow | Go to next image. Wraps around to the beginning |
| Home | Go to the first image |
| End | Go to the last image |
| F | Toggle the current image as favorite |
| Shift+F | Toggle between showing only favorited images, or all of them |

## Development
### Building
#### Prerequisites
You must have the following installed on your machine:

- A C++ compiler like clang, gcc, g++
- CMake
- vcpkg

#### Generator Setup
To compile the code you must first generate the build files for your favorite build tool using CMake.

You can run the following command to see which build tool options are available.
```bash
$ cmake -G
CMake Error: No generator specified for -G

Generators
* Unix Makefiles               = Generates standard UNIX makefiles.
  Ninja                        = Generates build.ninja files.
  Ninja Multi-Config           = Generates build-<Config>.ninja files.
  Watcom WMake                 = Generates Watcom WMake makefiles.
  Xcode                        = Generate Xcode project files.
  CodeBlocks - Ninja           = Generates CodeBlocks project files
                                 (deprecated).
  CodeBlocks - Unix Makefiles  = Generates CodeBlocks project files
                                 (deprecated).
  CodeLite - Ninja             = Generates CodeLite project files
                                 (deprecated).
  CodeLite - Unix Makefiles    = Generates CodeLite project files
                                 (deprecated).
  Eclipse CDT4 - Ninja         = Generates Eclipse CDT 4.0 project files
                                 (deprecated).
  Eclipse CDT4 - Unix Makefiles= Generates Eclipse CDT 4.0 project files
                                 (deprecated).
  Kate - Ninja                 = Generates Kate project files (deprecated).
  Kate - Ninja Multi-Config    = Generates Kate project files (deprecated).
  Kate - Unix Makefiles        = Generates Kate project files (deprecated).
  Sublime Text 2 - Ninja       = Generates Sublime Text 2 project files
                                 (deprecated).
  Sublime Text 2 - Unix Makefiles
                               = Generates Sublime Text 2 project files
                                 (deprecated).
```

From this list you can choose whichever generator you prefer and run CMake again. In this example I used Ninja
```bash
$ cmake -G Ninja -B build
```

> **Note:** It's a good convention to use `-B build` to specify the folder for build artifacts because it helps keep the project root clean.

> This is additionally useful if you use Microsoft's CMake extension pack with Visual Studio Code because it also uses `build/` as the root folder for build artifacts. This way your CLI commands will not interfere with the CMake extension's behavior or vice-versa.

> Despite this, the `.gitignore` files contain directives to ignore most of these files if placed at the root folder (and of course, the `build/` folder as well).

### Compiling the Code
Once the build files are generated, you can now run CMake again with the `--build` flag so it compiles the code with the generator you've selected.

```bash
$ cmake --build build/
```

You can now run the compiled binary.

```bash
# On Linux and MacOS
$ ./bin/monokl
Hello, world!

# On Windows
$ .\bin\monokl.exe
Hello, world!
```

## License
MIT
