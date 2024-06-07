# Template: C++17
This is a template project for creating C++17 projects using CMake and vcpkg

## How to Use This Template
### Cloning the Repository
#### Option 1: Using GitHub
First, you can click on the [Use This Template button](https://github.com/new?template_name=template_cpp17&template_owner=ygunayer) to create a new GitHub repository based on this template. Then you can simply clone your repository and get to working

#### Option 2: Cloning Manually
Alternatively, you can just clone this repository locally and get to work immediately. This option allows you to make the decision to make a new Git repository at a later time at your own leisure.

```bash
$ git clone https://github.com/ygunayer/template_cpp17 <some project name>
```

Later, when you'd like to create your own Git repository from this code, simply erase the entire Git history, create an initial commit, and add the remote URL for your repo. Feel free to customize any of the commands below:

```bash
# erase git history and start from scratch
$ rm -rf .git && \
  git init

# create a new intial commit
$ git add . && \
  git commit -m "Initial commit"

# add the remote url and push to the main branch
$ git remote add origin <your remote repo url> && \
  git push -u origin main
```

### Renaming the Project
Once a new codebase is created off this template, simply rename the project in the CMakeLists.txt file.

**CMakeLists.txt**
```diff
...

# Rename `template_cpp17` to your project name (e.g. `my_project`)
-project(template_cpp17 LANGUAGES CXX)
+project(my_project LANGUAGES CXX)

set(CXX_STANDARD 17)
...
```

## Prerequisites
You must have the following installed on your machine:

- A C++ compiler like clang, gcc, g++
- CMake
- vcpkg

## Building
### Generator Setup
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
# On UNIX -- replace `template_cpp17` with your project name (e.g. `my_project`)
$ ./bin/template_cpp17
Hello, world!

# On Windows -- replace `template_cpp17` with your project name (e.g. `my_project`)
$ .\bin\template_cpp17.exe
Hello, world!
```

As you make more changes on the code or the CMake configuration you can keep using the `cmake --build build/` command to rebuild your program. This will even install or uninstall the relevant `vcpkg` dependencies automatically if you've added or removed some of them.

## Adding and Installing Dependencies
You can use `vcpkg add port ...` to add dependencies.

```bash
$ vcpkg add wxwidgets
```

Then, add the `find_package(...)` and `target_link_libraries` directives to your `CMakeLists.txt` file and run Ninja to compile your program. This should install the dependencies automatically and rebuild the project.

```bash
$ ninja
```

## Additional Notes
This example uses the `fmt` package as a dependency to demonstrate how dependencies can be added and installed automatically using regular CMake commands. Feel free to remove it and add more depending on your use case.

## License
MIT
