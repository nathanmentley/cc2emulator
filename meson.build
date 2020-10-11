project(
    'compucolor2 emulator',
    'cpp',
    default_options : ['c_std=c17', 'cpp_std=c++17']
)

includeDirectories = include_directories('src')

coreLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Core',
        [
            'src/Compucolor.Core/BaseEmulator.cpp',
            'src/Compucolor.Core/ByteArrayMemory.cpp'
        ]
    ),
    include_directories: includeDirectories
)

intel8080ImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Intel8080.Impl',
        [
            'src/Compucolor.Intel8080.Impl/Intel8080.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080Emulator.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080EmulatorContext.cpp'
        ],
        dependencies: [
            coreLib
        ]
    ),
    include_directories: includeDirectories
)

smc5027ImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Smc5027.Impl',
        [
            'src/Compucolor.Smc5027.Impl/Smc5027Emulator.cpp'
        ],
        dependencies: [
            coreLib
        ]
    ),
    include_directories: includeDirectories
)

tms5501ImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Tms5501.Impl',
        [
            'src/Compucolor.Tms5501.Impl/Tms5501Emulator.cpp',
            'src/Compucolor.Tms5501.Impl/Tms5501EmulatorContext.cpp'
        ],
        dependencies: [
            coreLib
        ]
    ),
    include_directories: includeDirectories
)

crtImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Crt.Impl',
        [
            'src/Compucolor.Crt.Impl/FontRom.cpp',
            'src/Compucolor.Crt.Impl/CrtEmulator.cpp'
        ],
        dependencies: [
            coreLib
        ]
    ),
    include_directories: includeDirectories
)

floppyImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Floppy.Impl',
        [
            'src/Compucolor.Floppy.Impl/FloppyEmulator.cpp'
        ],
        dependencies: [
            coreLib
        ]
    ),
    include_directories: includeDirectories
)

keyboardImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Keyboard.Impl',
        [
            'src/Compucolor.Keyboard.Impl/KeyboardEmulator.cpp'
        ],
        dependencies: [
            coreLib
        ]
    ),
    include_directories: includeDirectories
)

compucolorImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Compucolor.Impl',
        [
            'src/Compucolor.Compucolor.Impl/CompucolorEmulator.cpp',
            'src/Compucolor.Compucolor.Impl/CompucolorIntel8080Bus.cpp',
            'src/Compucolor.Compucolor.Impl/Roms.cpp'
        ],
        dependencies: [
            coreLib,

            intel8080ImplLib,
            smc5027ImplLib,
            tms5501ImplLib,
            crtImplLib,
            floppyImplLib,
            keyboardImplLib
        ]
    ),
    include_directories: includeDirectories
)

appLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.App',
        [
            'src/Compucolor.App/get_emulator.cpp',
        ],
        dependencies: [
            coreLib,

            intel8080ImplLib,
            smc5027ImplLib,
            tms5501ImplLib,
            crtImplLib,
            floppyImplLib,
            keyboardImplLib,

            compucolorImplLib
        ]
    ),
    include_directories: includeDirectories
)

executable(
    'Compucolor.App.Impl.Gtk',
    [
        'src/Compucolor.App.Impl.Gtk/GtkDisplay.cpp',
        'src/Compucolor.App.Impl.Gtk/GtkKeyboard.cpp',
        'src/Compucolor.App.Impl.Gtk/main.cpp'
    ],
    dependencies: [
        dependency('gtk+-3.0'),
        dependency('gdk-pixbuf-2.0'),

        appLib
    ],
    include_directories: includeDirectories
)