project(
    'compucolor2 emulator',
    'cpp',
    version: '0.0.2',
    default_options : ['c_std=c2a', 'cpp_std=c++2a']
)

project_root = meson.source_root()  # absolute path to project root

install_prefix = join_paths(project_root, 'dist')  # install inside project dir

publicHeaderLib = declare_dependency(
    include_directories: include_directories('include'),
    dependencies: [
        dependency('boost', modules : ['filesystem'])
    ]
)

testHeaderLib = declare_dependency(
    include_directories: include_directories('test')
)

memoryImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Memory',
        [
            'src/Compucolor.Impl.Memory/PluginFactory.cpp',
            'src/Compucolor.Impl.Memory/VectorMemory.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
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
        testHeaderLib,

        memoryImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Memory/'
)
test('Compucolor.Impl.Tests.Memory', memoryImplTests)

intel8080ImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Intel8080',
        [
            'src/Compucolor.Impl.Intel8080/PluginFactory.cpp',
            'src/Compucolor.Impl.Intel8080/Intel8080.cpp',
            'src/Compucolor.Impl.Intel8080/Intel8080Emulator.cpp',
            'src/Compucolor.Impl.Intel8080/Intel8080EmulatorContext.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)

intel8080ImplTests = executable(
    'Compucolor.Impl.Tests.Intel8080',
    [
        'test/Compucolor.Impl.Tests.Intel8080/main.cpp',
        'test/Compucolor.Impl.Tests.Intel8080/Intel8080EmulatorTests.cpp'
    ],
    dependencies: [
        dependency('gtest'),
        dependency('gmock'),

        publicHeaderLib,
        testHeaderLib,

        intel8080ImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Intel8080/'
)
test('Compucolor.Impl.Tests.Intel8080', intel8080ImplTests)

smc5027ImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Smc5027',
        [
            'src/Compucolor.Impl.Smc5027/PluginFactory.cpp',
            'src/Compucolor.Impl.Smc5027/Smc5027Emulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
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
        testHeaderLib,

        smc5027ImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Smc5027/'
)
test('Compucolor.Impl.Tests.Smc5027', smc5027ImplTests)

tms5501ImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Tms5501',
        [
            'src/Compucolor.Impl.Tms5501/PluginFactory.cpp',
            'src/Compucolor.Impl.Tms5501/Tms5501Emulator.cpp',
            'src/Compucolor.Impl.Tms5501/Tms5501EmulatorContext.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)

tms5501ImplTests = executable(
    'Compucolor.Impl.Tests.Tms5501',
    [
        'test/Compucolor.Impl.Tests.Tms5501/main.cpp',
        'test/Compucolor.Impl.Tests.Tms5501/Tms5501EmulatorTests.cpp'
    ],
    dependencies: [
        dependency('gtest'),
        dependency('gmock'),

        publicHeaderLib,
        testHeaderLib,

        tms5501ImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Tms5501/'
)
test('Compucolor.Impl.Tests.Tms5501', tms5501ImplTests)

crtImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Crt',
        [
            'src/Compucolor.Impl.Crt/PluginFactory.cpp',
            'src/Compucolor.Impl.Crt/FontRom.cpp',
            'src/Compucolor.Impl.Crt/CrtEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
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
        testHeaderLib,

        crtImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Crt/'
)
test('Compucolor.Impl.Tests.Crt', crtImplTests)

floppyImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Floppy',
        [
            'src/Compucolor.Impl.Floppy/PluginFactory.cpp',
            'src/Compucolor.Impl.Floppy/FloppyEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)

floppyImplTests = executable(
    'Compucolor.Impl.Tests.Floppy',
    [
        'test/Compucolor.Impl.Tests.Floppy/main.cpp',
        'test/Compucolor.Impl.Tests.Floppy/FloppyEmulatorTests.cpp'
    ],
    dependencies: [
        dependency('gtest'),

        publicHeaderLib,
        testHeaderLib,

        floppyImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Floppy/'
)
test('Compucolor.Impl.Tests.Floppy', floppyImplTests)

keyboardImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Keyboard',
        [
            'src/Compucolor.Impl.Keyboard/PluginFactory.cpp',
            'src/Compucolor.Impl.Keyboard/KeyboardEmulator.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)

keyboardImplTests = executable(
    'Compucolor.Impl.Tests.Keyboard',
    [
        'test/Compucolor.Impl.Tests.Keyboard/main.cpp',
        'test/Compucolor.Impl.Tests.Keyboard/KeyboardEmulatorTests.cpp'
    ],
    dependencies: [
        dependency('gtest'),

        publicHeaderLib,
        testHeaderLib,

        keyboardImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Keyboard/'
)
test('Compucolor.Impl.Tests.Keyboard', keyboardImplTests)

schedulerImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Scheduler',
        [
            'src/Compucolor.Impl.Scheduler/PluginFactory.cpp',
            'src/Compucolor.Impl.Scheduler/Scheduler.cpp',
            'src/Compucolor.Impl.Scheduler/ScheduledTask.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)

loggerImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Logger',
        [
            'src/Compucolor.Impl.Logger/PluginFactory.cpp',
            'src/Compucolor.Impl.Logger/Logger.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)

compucolorImplLib = declare_dependency(
    link_with: shared_library(
        'Compucolor.Impl.Compucolor',
        [
            'src/Compucolor.Impl.Compucolor/PluginFactory.cpp',
            'src/Compucolor.Impl.Compucolor/CompucolorEmulator.cpp',
            'src/Compucolor.Impl.Compucolor/CompucolorIntel8080Bus.cpp',
            'src/Compucolor.Impl.Compucolor/Roms.cpp'
        ],
        dependencies: [
            publicHeaderLib
        ],
        install: true,
        install_dir: install_prefix + '/plugins'
    )
)
compucolorImplTests = executable(
    'Compucolor.Impl.Tests.Compucolor',
    [
        'test/Compucolor.Impl.Tests.Compucolor/main.cpp',
        'test/Compucolor.Impl.Tests.Compucolor/CompucolorIntel8080BusTests.cpp',
        'test/Compucolor.Impl.Tests.Compucolor/CompucolorEmulatorTests.cpp'
    ],
    dependencies: [
        dependency('gtest'),
        dependency('gmock'),
        publicHeaderLib,
        testHeaderLib,
        compucolorImplLib
    ],
    include_directories: 'src/Compucolor.Impl.Compucolor/'
)
test('Compucolor.Impl.Tests.Compucolor', compucolorImplTests)

appSettingsTests = executable(
    'Compucolor.Impl.Tests.App',
    [
        'test/Compucolor.Impl.Tests.App/main.cpp',
        'test/Compucolor.Impl.Tests.App/SettingsTests.cpp',
        'test/Compucolor.Impl.Tests.App/AsciiKeyMapTests.cpp',
        'test/Compucolor.Impl.Tests.App/SoundSynthTests.cpp',
        'src/Compucolor.Impl.App.Sfml/Settings.cpp',
        'src/Compucolor.Impl.App.Sfml/AsciiKeyMap.cpp',
        'src/Compucolor.Impl.App.Sfml/SoundSynth.cpp'
    ],
    dependencies: [
        dependency('gtest'),

        publicHeaderLib,
        testHeaderLib
    ],
    include_directories: 'src/Compucolor.Impl.App.Sfml/'
)
test('Compucolor.Impl.Tests.App', appSettingsTests)

sfmlGraphicsDep = dependency('sfml-graphics')

# The imgui-sfml wrap looks up SFML as 'sfml'; reuse the system SFML instead of building another.
meson.override_dependency('sfml', sfmlGraphicsDep)

# imgui-sfml pulls in imgui itself, so configure imgui as a static library first;
# otherwise it is built shared and the installed app cannot find libimgui.so.
imguiDep = dependency(
    'imgui',
    fallback: ['imgui', 'imgui_dep'],
    default_options: ['default_library=static']
)

imguiSfmlDep = dependency(
    'imgui-sfml',
    fallback: ['imgui-sfml', 'imgui_sfml_dep'],
    default_options: ['default_library=static']
)

executable(
    'Compucolor.Impl.App.Sfml',
    [
        'src/Compucolor.Impl.App.Sfml/AudioOutput.cpp',
        'src/Compucolor.Impl.App.Sfml/Configuration.cpp',
        'src/Compucolor.Impl.App.Sfml/Display.cpp',
        'src/Compucolor.Impl.App.Sfml/ErrorHandler.cpp',
        'src/Compucolor.Impl.App.Sfml/Gui.cpp',
        'src/Compucolor.Impl.App.Sfml/KeyboardTranslator.cpp',
        'src/Compucolor.Impl.App.Sfml/main.cpp',
        'src/Compucolor.Impl.App.Sfml/Program.cpp',
        'src/Compucolor.Impl.App.Sfml/Settings.cpp',
        'src/Compucolor.Impl.App.Sfml/AsciiKeyMap.cpp',
        'src/Compucolor.Impl.App.Sfml/SoundSynth.cpp'
    ],
    dependencies: [
        sfmlGraphicsDep,
        dependency('sfml-audio'),
        imguiSfmlDep,
        dependency('threads'),

        publicHeaderLib
    ],
    install: true,
    install_dir: install_prefix + '/bin'
)

configure_file(
    input : 'data/app.config',
    output : 'app.config',
    copy : true
)

configure_file(
    input : 'data/cc2_emu.sh',
    output : 'cc2_emu.sh',
    copy : true
)

install_data('data/app.config', install_dir: install_prefix)
install_data('data/cc2_emu.sh', install_dir: install_prefix, install_mode: 'rwxr--r--')

install_subdir('data/roms', install_dir: install_prefix)
install_subdir('data/disks', install_dir: install_prefix)

python = find_program('python3')
checker = meson.project_source_root() / 'scripts' / 'check_imports.py'

custom_target(
    'check_parent_includes',
    output: 'check_parent_includes.stamp',
    command: [
        python,
        checker,
        meson.current_source_dir()
    ],
    build_by_default: true
)
