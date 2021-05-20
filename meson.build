project(
    'compucolor2 emulator',
    'cpp',
    default_options : ['c_std=c2a', 'cpp_std=c++2a']
)

publicHeaderLib = declare_dependency(
    include_directories: include_directories('include')
)

internalHeaderLib = declare_dependency(
    include_directories: include_directories('src')
)

testHeaderLib = declare_dependency(
    include_directories: include_directories('test')
)

memoryImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Memory.Impl',
        [
            'src/Compucolor.Memory.Impl/VectorMemory.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)
memoryImplTests = executable(
    'Compucolor.Tests.Memory.Impl',
    [
        'test/Compucolor.Tests.Memory.Impl/main.cpp',
        'test/Compucolor.Tests.Memory.Impl/VectoryMemoryTests.cpp',
    ],
    dependencies: [
        dependency('gtest'),
        dependency('gmock'),

        publicHeaderLib,
        internalHeaderLib,
        testHeaderLib,

        memoryImplLib
    ]
)
test('Compucolor.Tests.Memory.Impl', memoryImplTests)

intel8080ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Intel8080.Impl',
        [
            'src/Compucolor.Intel8080.Impl/Intel8080.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080Emulator.cpp',
            'src/Compucolor.Intel8080.Impl/Intel8080EmulatorContext.cpp'
        ],
        dependencies: [
            publicHeaderLib
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
            publicHeaderLib
        ]
    )
)
smc5027ImplTests = executable(
    'Compucolor.Tests.Smc5027.Impl',
    [
        'test/Compucolor.Tests.Smc5027.Impl/main.cpp',
        'test/Compucolor.Tests.Smc5027.Impl/Smc5027EmulatorTests.cpp'
    ],
    dependencies: [
        dependency('gtest'),
        dependency('gmock'),

        publicHeaderLib,
        internalHeaderLib,
        testHeaderLib,

        smc5027ImplLib
    ]
)
test('Compucolor.Tests.Smc5027.Impl', smc5027ImplTests)

tms5501ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Tms5501.Impl',
        [
            'src/Compucolor.Tms5501.Impl/Tms5501Emulator.cpp',
            'src/Compucolor.Tms5501.Impl/Tms5501EmulatorContext.cpp'
        ],
        dependencies: [
            publicHeaderLib
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
            publicHeaderLib
        ]
    )
)
crtImplTests = executable(
    'Compucolor.Tests.Crt.Impl',
    [
        'test/Compucolor.Tests.Crt.Impl/main.cpp',
        'test/Compucolor.Tests.Crt.Impl/CrtEmulatorTests.cpp',
    ],
    dependencies: [
        dependency('gtest'),
        dependency('gmock'),

        publicHeaderLib,
        internalHeaderLib,
        testHeaderLib,

        crtImplLib
    ]
)
test('Compucolor.Tests.Crt.Impl', crtImplTests)

floppyImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Floppy.Impl',
        [
            'src/Compucolor.Floppy.Impl/FloppyEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
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
            publicHeaderLib
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
            publicHeaderLib
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
            publicHeaderLib
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
            publicHeaderLib
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
            publicHeaderLib
        ]
    )
)

appLib = declare_dependency(
    link_with: static_library(
        'Compucolor.App',
        [
            'src/Compucolor.App/get_emulator.cpp'
        ],
        dependencies: [
            publicHeaderLib,
            internalHeaderLib,

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

# executable(
#     'Compucolor.App.Impl.Gtk',
#     [
#         'src/Compucolor.App.Impl.Gtk/GtkDisplay.cpp',
#         'src/Compucolor.App.Impl.Gtk/GtkKeyboard.cpp',
#         'src/Compucolor.App.Impl.Gtk/GtkLoggerProvider.cpp',
#         'src/Compucolor.App.Impl.Gtk/main.cpp'
#     ],
#     dependencies: [
#         dependency('gtk+-3.0'),
#         dependency('gdk-pixbuf-2.0'),
#         dependency('threads'),
# 
#         publicHeaderLib,
#         internalHeaderLib,
#         appLib
#     ],
#     install: true
# )

executable(
    'Compucolor.App.Impl.Sfml',
    [
        'src/Compucolor.App.Impl.Sfml/Display.cpp',
        'src/Compucolor.App.Impl.Sfml/ErrorHandler.cpp',
        'src/Compucolor.App.Impl.Sfml/KeyboardTranslator.cpp',
        'src/Compucolor.App.Impl.Sfml/main.cpp',
        'src/Compucolor.App.Impl.Sfml/Program.cpp'
    ],
    dependencies: [
        dependency('sfml-graphics'),
        dependency('threads'),

        publicHeaderLib,
        internalHeaderLib,
        appLib
    ],
    install: true
)

configure_file(
    input : 'data/app.config',
    output : 'app.config',
    copy : configuration_data()
)