project(
    'compucolor2 emulator',
    'cpp',
    default_options : ['c_std=c17', 'cpp_std=c++17']
)

includeDirectories = include_directories('src')

coreLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Core',
        [
            'src/Compucolor.Core/BaseEmulator.cpp',
            'src/Compucolor.Core/ByteArrayMemory.cpp'
        ]
    ),
    include_directories: includeDirectories
)

intel8080ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Intel8080.Impl',
        [
            'src/Compucolor.Intel8080.Impl/Intel8080.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080Emulator.cpp'
        ],
        dependencies: [
            meson.get_compiler('cpp').find_library('fruit'),

            coreLib
        ]
    ),
    include_directories: includeDirectories
)

crtImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Crt.Impl',
        [
            'src/Compucolor.Crt.Impl/FontRom.cpp',
            'src/Compucolor.Crt.Impl/CrtEmulator.cpp'
        ],
        dependencies: [
            meson.get_compiler('cpp').find_library('fruit'),

            coreLib
        ]
    ),
    include_directories: includeDirectories
)

compucolorImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Compucolor.Impl',
        [
            'src/Compucolor.Compucolor.Impl/CompucolorEmulator.cpp',
            'src/Compucolor.Compucolor.Impl/Roms.cpp'
        ],
        dependencies: [
            meson.get_compiler('cpp').find_library('fruit'),

            coreLib,
            intel8080ImplLib
        ]
    ),
    include_directories: includeDirectories
)

appLib = declare_dependency(
    link_with: static_library(
        'Compucolor.App',
        [
            'src/Compucolor.App/get_emulator.cpp',
        ],
        dependencies: [
            meson.get_compiler('cpp').find_library('fruit'),

            coreLib,
            intel8080ImplLib,
            crtImplLib,
            compucolorImplLib
        ]
    ),
    include_directories: includeDirectories
)

executable(
    'Compucolor.App.Impl.Gtk',
    [
        'src/Compucolor.App.Impl.Gtk/GtkDisplay.cpp',
        'src/Compucolor.App.Impl.Gtk/main.cpp'
    ],
    dependencies: [
        dependency('gtk+-3.0'),
        dependency('gdk-pixbuf-2.0'),

        appLib
    ],
    include_directories: includeDirectories
)