project(
    'compucolor2 emulator',
    'cpp',
    version: '0.0.1',
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
        'Compucolor.Impl.Memory',
        [
            'src/Compucolor.Impl.Memory/PluginFactory.cpp',
            'src/Compucolor.Impl.Memory/VectorMemory.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)
memoryImplTests = executable(
    'Compucolor.Impl.Tests.Memory',
    [
        'test/Compucolor.Impl.Tests.Memory/main.cpp',
        'test/Compucolor.Impl.Tests.Memory/VectorMemoryTests.cpp',
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
test('Compucolor.Impl.Tests.Memory', memoryImplTests)

intel8080ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Intel8080',
        [
            'src/Compucolor.Impl.Intel8080/Intel8080.cpp',
            'src/Compucolor.Impl.Intel8080/Intel8080Emulator.cpp',
            'src/Compucolor.Impl.Intel8080/Intel8080EmulatorContext.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

smc5027ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Smc5027',
        [
            'src/Compucolor.Impl.Smc5027/Smc5027Emulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)
smc5027ImplTests = executable(
    'Compucolor.Impl.Tests.Smc5027',
    [
        'test/Compucolor.Impl.Tests.Smc5027/main.cpp',
        'test/Compucolor.Impl.Tests.Smc5027/Smc5027EmulatorTests.cpp'
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
test('Compucolor.Impl.Tests.Smc5027', smc5027ImplTests)

tms5501ImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Tms5501',
        [
            'src/Compucolor.Impl.Tms5501/Tms5501Emulator.cpp',
            'src/Compucolor.Impl.Tms5501/Tms5501EmulatorContext.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

crtImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Crt',
        [
            'src/Compucolor.Impl.Crt/FontRom.cpp',
            'src/Compucolor.Impl.Crt/CrtEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)
crtImplTests = executable(
    'Compucolor.Impl.Tests.Crt',
    [
        'test/Compucolor.Impl.Tests.Crt/main.cpp',
        'test/Compucolor.Impl.Tests.Crt/CrtEmulatorTests.cpp',
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
test('Compucolor.Impl.Tests.Crt', crtImplTests)

floppyImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Floppy',
        [
            'src/Compucolor.Impl.Floppy/FloppyEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

keyboardImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Keyboard',
        [
            'src/Compucolor.Impl.Keyboard/KeyboardEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

configurationImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Configuration',
        [
            'src/Compucolor.Impl.Configuration/Configuration.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

schedulerImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Scheduler',
        [
            'src/Compucolor.Impl.Scheduler/Scheduler.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

loggerImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Logger',
        [
            'src/Compucolor.Impl.Logger/Logger.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

compucolorImplLib = declare_dependency(
    link_with: static_library(
        'Compucolor.Impl.Compucolor',
        [
            'src/Compucolor.Impl.Compucolor/CompucolorEmulator.cpp',
            'src/Compucolor.Impl.Compucolor/CompucolorIntel8080Bus.cpp',
            'src/Compucolor.Impl.Compucolor/Roms.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ]
    )
)

executable(
    'Compucolor.Impl.App.Sfml',
    [
        'src/Compucolor.Impl.App.Sfml/Display.cpp',
        'src/Compucolor.Impl.App.Sfml/ErrorHandler.cpp',
        'src/Compucolor.Impl.App.Sfml/get_emulator.cpp',
        'src/Compucolor.Impl.App.Sfml/KeyboardTranslator.cpp',
        'src/Compucolor.Impl.App.Sfml/main.cpp',
        'src/Compucolor.Impl.App.Sfml/Program.cpp'
    ],
    dependencies: [
        dependency('sfml-graphics'),
        dependency('threads'),

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
        compucolorImplLib,

        publicHeaderLib,
        internalHeaderLib
    ],
    install: true
)

configure_file(
    input : 'data/app.config',
    output : 'app.config',
    copy : configuration_data()
)