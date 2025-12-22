#pragma once

#include <memory>
#include <string>
#include <utility>

#include <Compucolor/Common/IPluginManager.h>
#include <Compucolor/Common/PluginLoader.h>

namespace Compucolor::Common
{
    class PluginManager: public IPluginManager {
        public:
            PluginManager(
                std::shared_ptr<Logger::ILoggerProvider> loggerProvider,
                std::shared_ptr<IDisplay> display,
                const std::string &loggerLibrary,
                const std::string &intel8080Library,
                const std::string &memoryLibrary,
                const std::string &schedulerLibrary,
                const std::string &keyboardLibrary,
                const std::string &floppyLibrary,
                const std::string &smc5027Library,
                const std::string &crtLibrary,
                const std::string &tms5501Library,
                const std::string &compucolorLibrary
            ):
                _loggerProvider(std::move(loggerProvider)),
                _display(std::move(display)),
                _loggerLoader(
                    std::make_shared<PluginLoader<Logger::ILogger>>(
                        loggerLibrary
                    )
                ),
                _intel8080PluginLoader(
                    std::make_shared<PluginLoader<Intel8080::IIntel8080Emulator>>(
                        intel8080Library
                    )
                ),
                _memoryPluginLoader(
                    std::make_shared<PluginLoader<Memory::IMemory>>(
                        memoryLibrary
                    )
                ),
                _schedulerPluginLoader(
                    std::make_shared<PluginLoader<Scheduler::IScheduler>>(
                        schedulerLibrary
                    )
               ),
                _keyboardPluginLoader(
                    std::make_shared<PluginLoader<Keyboard::IKeyboardEmulator>>(
                        keyboardLibrary
                    )
                ),
                _floppyPluginLoader(
                    std::make_shared<PluginLoader<Floppy::IFloppyEmulator>>(
                        floppyLibrary
                    )
                ),
                _smc5027PluginLoader(
                    std::make_shared<PluginLoader<Smc5027::ISmc5027Emulator>>(
                        smc5027Library
                    )
                ),
                _crtPluginLoader(
                    std::make_shared<PluginLoader<Crt::ICrtEmulator>>(
                        crtLibrary
                    )
                ),
                _tms5501PluginLoader(
                    std::make_shared<PluginLoader<Tms5501::ITms5501Emulator>>(
                        tms5501Library
                    )
                ),
                _compucolorPluginLoader(
                    std::make_shared<PluginLoader<ICompucolorEmulator>>(
                        compucolorLibrary
                    )
                )
        {
        };

            std::shared_ptr<Logger::ILoggerProvider> GetLoggerProvider() override {
                return _loggerProvider;
            }

            std::shared_ptr<IDisplay> GetDisplay() override {
                return _display;
            }

            std::shared_ptr<Logger::ILogger> GetLogger() override {
                return _loggerLoader->Create(this);
            }

            std::shared_ptr<Intel8080::IIntel8080Emulator> GetIntel8080() override {
                return _intel8080PluginLoader->Create(this);
            }

            std::shared_ptr<Memory::IMemory> GetMemory() override {
                return _memoryPluginLoader->Create(this);
            }

            std::shared_ptr<Scheduler::IScheduler> GetScheduler() override {
                return _schedulerPluginLoader->Create(this);
            }

            std::shared_ptr<Keyboard::IKeyboardEmulator> GetKeyboard() override {
                return _keyboardPluginLoader->Create(this);
            }

            std::shared_ptr<Floppy::IFloppyEmulator> GetFloppy() override {
                return _floppyPluginLoader->Create(this);
            }

            std::shared_ptr<Smc5027::ISmc5027Emulator> GetSmc5027() override {
                return _smc5027PluginLoader->Create(this);
            }

            std::shared_ptr<Crt::ICrtEmulator> GetCrt() override {
                return _crtPluginLoader->Create(this);
            }

            std::shared_ptr<Tms5501::ITms5501Emulator> GetTms5501() override {
                return _tms5501PluginLoader->Create(this);
            }

            std::shared_ptr<ICompucolorEmulator> GetCompucolor() override {
                return _compucolorPluginLoader->Create(this);
            }
    private:
            std::shared_ptr<Logger::ILoggerProvider> _loggerProvider;
            std::shared_ptr<IDisplay> _display;
            std::shared_ptr<PluginLoader<Logger::ILogger>> _loggerLoader;
            std::shared_ptr<PluginLoader<Intel8080::IIntel8080Emulator>> _intel8080PluginLoader;
            std::shared_ptr<PluginLoader<Memory::IMemory>> _memoryPluginLoader;
            std::shared_ptr<PluginLoader<Scheduler::IScheduler>> _schedulerPluginLoader;
            std::shared_ptr<PluginLoader<Keyboard::IKeyboardEmulator>> _keyboardPluginLoader;
            std::shared_ptr<PluginLoader<Floppy::IFloppyEmulator>> _floppyPluginLoader;
            std::shared_ptr<PluginLoader<Smc5027::ISmc5027Emulator>> _smc5027PluginLoader;
            std::shared_ptr<PluginLoader<Crt::ICrtEmulator>> _crtPluginLoader;
            std::shared_ptr<PluginLoader<Tms5501::ITms5501Emulator>> _tms5501PluginLoader;
            std::shared_ptr<PluginLoader<ICompucolorEmulator>> _compucolorPluginLoader;
    };
}