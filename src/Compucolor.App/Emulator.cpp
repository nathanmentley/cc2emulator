#include "Emulator.h"

Compucolor::App::Emulator::Emulator(
    std::shared_ptr<Logger::ILoggerProvider> loggerProvider
):
    _loggerProvider(loggerProvider),
    _emulator(App::get_emulator(_loggerProvider)),
    _thread({})
{
}

void Compucolor::App::Emulator::Start()
{
    if (!_thread.has_value())
    {
        _thread = std::thread(
            [this]() {
                _emulator->Start();
            }
        );
    }
}

void Compucolor::App::Emulator::Stop()
{
    if (_thread.has_value())
    {
        _emulator->Stop();

        _thread->join();

        _thread.reset();
    }
}

void Compucolor::App::Emulator::SetDisplay(Common::IDisplay* display)
{
    _emulator->SetDisplay(display);
}

void Compucolor::App::Emulator::OnKeyUp(Keyboard::CompucolorIIKey key)
{
    _emulator->OnKeyUp(key);
}

void Compucolor::App::Emulator::OnKeyDown(Keyboard::CompucolorIIKey key)
{
    _emulator->OnKeyDown(key);
}