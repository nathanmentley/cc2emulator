project(
    'compucolor2 emulator',
    'cpp',
    default_options : ['c_std=c2a', 'cpp_std=c++2a']
)

includeDirectories = include_directories('src')

headerLib = declare_dependency(
    include_directories: includeDirectories
)

memoryImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Memory.Impl',
        [
            'src/Compucolor.Memory.Impl/VectorMemory.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

intel8080ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Intel8080.Impl',
        [
            'src/Compucolor.Intel8080.Impl/Intel8080.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080Emulator.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080EmulatorContext.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

smc5027ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Smc5027.Impl',
        [
            'src/Compucolor.Smc5027.Impl/Smc5027Emulator.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

tms5501ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Tms5501.Impl',
        [
            'src/Compucolor.Tms5501.Impl/Tms5501Emulator.cpp',
            'src/Compucolor.Tms5501.Impl/Tms5501EmulatorContext.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

crtImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Crt.Impl',
        [
            'src/Compucolor.Crt.Impl/FontRom.cpp',
            'src/Compucolor.Crt.Impl/CrtEmulator.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

floppyImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Floppy.Impl',
        [
            'src/Compucolor.Floppy.Impl/FloppyEmulator.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

keyboardImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Keyboard.Impl',
        [
            'src/Compucolor.Keyboard.Impl/KeyboardEmulator.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

configurationImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Configuration.Impl',
        [
            'src/Compucolor.Configuration.Impl/Configuration.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

schedulerImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Scheduler.Impl',
        [
            'src/Compucolor.Scheduler.Impl/Scheduler.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

loggerImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Logger.Impl',
        [
            'src/Compucolor.Logger.Impl/Logger.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

compucolorImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Compucolor.Impl',
        [
            'src/Compucolor.Compucolor.Impl/CompucolorEmulator.cpp',
            'src/Compucolor.Compucolor.Impl/CompucolorIntel8080Bus.cpp',
            'src/Compucolor.Compucolor.Impl/Roms.cpp'
        ],
        dependencies: [
            headerLib
        ]
    )
)

appLib = declare_dependency(
    link_with: static_library(
        'Compucolor.App',
        [
            'src/Compucolor.App/get_emulator.cpp',
        ],
        dependencies: [
            headerLib,

            memoryImplLib,
            intel8080ImplLib,
            smc5027ImplLib,
            tms5501ImplLib,
            crtImplLib,
            floppyImplLib,
            keyboardImplLib,
            configurationImplLib,
            schedulerImplLib,
            loggerImplLib,

            compucolorImplLib
        ]
    )
)

executable(
    'Compucolor.App.Impl.Gtk',
    [
        'src/Compucolor.App.Impl.Gtk/GtkDisplay.cpp',
        'src/Compucolor.App.Impl.Gtk/GtkKeyboard.cpp',
        'src/Compucolor.App.Impl.Gtk/GtkLoggerProvider.cpp',
        'src/Compucolor.App.Impl.Gtk/main.cpp'
    ],
    dependencies: [
        dependency('gtk+-3.0'),
        dependency('gdk-pixbuf-2.0'),

        headerLib,
        appLib
    ],
    install: true
)

configure_file(
    input : 'data/app.config',
    output : 'app.config',
    copy : configuration_data()
)