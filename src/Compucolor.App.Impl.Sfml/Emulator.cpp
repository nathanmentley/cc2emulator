#include <Compucolor.App.Impl.Sfml/Emulator.h>

Compucolor::App::Impl::Smfl::Emulator::Emulator():
    _loggerProvider(
        std::shared_ptr<Logger::ILoggerProvider>(
            new LoggerProvider(Logger::LogLevel::Trace)
        )
    ),
    _emulator(App::get_emulator(_loggerProvider)),
    _isRunning(false)
{
}

void Compucolor::App::Impl::Smfl::Emulator::Start()
{
    if (!_isRunning)
    {
        _thread = std::thread(
            [this]() {
                _emulator->Start();
            }
        );

        _isRunning = true;
    }
}

void Compucolor::App::Impl::Smfl::Emulator::Stop()
{
    if (_isRunning)
    {
        _emulator->Stop();

        _thread.join();

        _isRunning = false;
    }
}

void Compucolor::App::Impl::Smfl::Emulator::SetDisplay(Common::IDisplay* display)
{
    _emulator->SetDisplay(display);
}

void Compucolor::App::Impl::Smfl::Emulator::OnKeyUp(Keyboard::CompucolorIIKey key)
{
    _emulator->OnKeyUp(key);
}

void Compucolor::App::Impl::Smfl::Emulator::OnKeyDown(Keyboard::CompucolorIIKey key)
{
    _emulator->OnKeyDown(key);
}