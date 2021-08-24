# Envy

[![License](https://img.shields.io/apm/l/atomic-design-ui.svg?)](LICENSE)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/github/PatrickTorgerson/Envy?svg=true)](https://ci.appveyor.com/project/PatrickTorgerson/Envy)
[![Last Commit](https://img.shields.io/github/last-commit/PatrickTorgerson/Envy)](https://github.com/PatrickTorgerson/Envy/commits/main)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/PatrickTorgerson/Envy)](https://github.com/PatrickTorgerson/Envy/releases)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ba76a4e0bbbf46b39fb04cd0b3788ce4)](https://www.codacy.com/gh/PatrickTorgerson/Envy/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PatrickTorgerson/Envy&amp;utm_campaign=Badge_Grade)

[![Lines of Code](https://tokei.rs/b1/github.com/PatrickTorgerson/Envy?category=code)](https://github.com/PatrickTorgerson/Envy)
[![Code Size](https://img.shields.io/github/languages/code-size/PatrickTorgerson/Envy)](https://github.com/PatrickTorgerson/Envy)

A lite game engine written in C++20 for Windows.

## Example

```cpp
#include <Envy.hpp>

int main(int argc, char** argv)
{
    Envy::engine::description desc {};

    desc.window.title = L"Example Application";

    Envy::engine::run(desc, argc, argv);

    return 0;
}
```

## Building

Envy currently only supports 64 bit Windows 10.
You will need cmake 3.7 or later to generate build files for your prefered build system (currently only MSBuild).
Envy's dependencies are either included in source or as submodules, so if cloning the repository you will need to clone recursivly to also clone all submodules.

> `git clone --recursive https://github.com/PatrickTorgerson/Envy`

Or you can download an archive from the github site.
Once you have the source you can generate build files with cmake. First navigate to a build directory

`cd ${ENVY_SOURCE_DIR}/build`

Where **${ENVY_SOURCE_DIR}** is the directory you have Envy's source. Now we can run cmake.

`cmake -G "Visual Studio 16 2019" ..`

Now you can either oppen EnvyEngine.sln with Visual Studio, or run MSBuild form the commandline.

`msbuild EnvyEngine.sln /p:Configuration=Debug`

This should produce a **Envy.lib** that you can link with your own project.