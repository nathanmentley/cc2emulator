#include <Compucolor/App/Replay/ReplayBundle.h>

#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include <Compucolor/App/Replay/ReplayKeyNames.h>

namespace
{
    [[noreturn]] void InvalidBundle(
        const std::string& source,
        const std::string& message
    )
    {
        throw std::runtime_error("Invalid " + source + ": " + message);
    }

    uint64_t ReadUnsigned(
        const boost::json::value& value,
        const std::string& source,
        const std::string& field
    )
    {
        if (value.is_uint64())
        {
            return value.as_uint64();
        }

        if (value.is_int64() && value.as_int64() >= 0)
        {
            return static_cast<uint64_t>(value.as_int64());
        }

        InvalidBundle(source, field + " must be a non-negative integer");
    }

    const boost::json::value& RequireMember(
        const boost::json::object& object,
        const char* name,
        const std::string& source
    )
    {
        try
        {
            return object.at(name);
        }
        catch (const std::exception&)
        {
            InvalidBundle(source, "missing '" + std::string(name) + "'");
        }
    }
}

Compucolor::Impl::App::Replay::ReplayBundle::ReplayBundle(
    boost::json::value state,
    std::vector<ReplayEvent> input,
    const uint64_t stopNanoseconds,
    std::string createdBy
):
    _state(std::move(state)),
    _input(std::move(input)),
    _stopNanoseconds(stopNanoseconds),
    _createdBy(std::move(createdBy))
{
}

Compucolor::Impl::App::Replay::ReplayBundle Compucolor::Impl::App::Replay::ReplayBundle::ParseFile(
    const std::string& path
)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open replay bundle: " + path);
    }

    const std::string contents{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    return Parse(contents, path);
}

Compucolor::Impl::App::Replay::ReplayBundle Compucolor::Impl::App::Replay::ReplayBundle::Parse(
    const std::string_view contents,
    const std::string_view sourceView
)
{
    const std::string source(sourceView);

    boost::json::value root;

    try
    {
        root = boost::json::parse(contents);
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error(
            "Invalid " + source + ": malformed JSON: " + error.what()
        );
    }

    if (!root.is_object())
    {
        InvalidBundle(source, "top-level value must be an object");
    }

    const auto& object = root.as_object();

    const uint64_t version = ReadUnsigned(
        RequireMember(object, "version", source),
        source,
        "version"
    );

    if (version != 1)
    {
        InvalidBundle(source, "unsupported version " + std::to_string(version));
    }

    const auto& state = RequireMember(object, "state", source);

    if (!state.is_object())
    {
        InvalidBundle(source, "state must be an object");
    }

    const uint64_t stateNanoseconds = ReadUnsigned(
        RequireMember(state.as_object(), "emulatedNanoseconds", source),
        source,
        "state.emulatedNanoseconds"
    );

    const auto& stop = RequireMember(object, "stop", source);

    if (!stop.is_object())
    {
        InvalidBundle(source, "stop must be an object");
    }

    const uint64_t stopNanoseconds = ReadUnsigned(
        RequireMember(stop.as_object(), "emulatedNanoseconds", source),
        source,
        "stop.emulatedNanoseconds"
    );

    if (stopNanoseconds <= stateNanoseconds)
    {
        InvalidBundle(
            source,
            "stop.emulatedNanoseconds must be greater than state.emulatedNanoseconds"
        );
    }

    const auto& input = RequireMember(object, "input", source);

    if (!input.is_array())
    {
        InvalidBundle(source, "input must be an array");
    }

    std::vector<ReplayEvent> events;
    events.reserve(input.as_array().size());

    uint64_t previousTime = stateNanoseconds;

    for (std::size_t i = 0; i < input.as_array().size(); i++)
    {
        const std::string prefix = "input[" + std::to_string(i) + "]";
        const auto& entry = input.as_array()[i];

        if (!entry.is_object())
        {
            InvalidBundle(source, prefix + " must be an object");
        }

        const auto& eventObject = entry.as_object();
        const uint64_t time = ReadUnsigned(
            RequireMember(eventObject, "t", source),
            source,
            prefix + ".t"
        );

        if (time < stateNanoseconds)
        {
            InvalidBundle(
                source,
                prefix + ".t precedes the state's emulated time"
            );
        }

        if (time < previousTime)
        {
            InvalidBundle(source, prefix + ".t is out of order");
        }

        if (time > stopNanoseconds)
        {
            InvalidBundle(source, prefix + ".t is after the stop time");
        }

        const auto& keyValue = RequireMember(eventObject, "key", source);

        if (!keyValue.is_string())
        {
            InvalidBundle(source, prefix + ".key must be a string");
        }

        const std::string keyName(keyValue.as_string().c_str());
        const auto key = ReplayKeyNames::FromString(keyName);

        if (!key.has_value())
        {
            InvalidBundle(source, prefix + ".key is unknown: " + keyName);
        }

        const auto& downValue = RequireMember(eventObject, "down", source);

        if (!downValue.is_bool())
        {
            InvalidBundle(source, prefix + ".down must be a boolean");
        }

        events.push_back(
            ReplayEvent{
                time,
                key.value(),
                downValue.as_bool()
            }
        );

        previousTime = time;
    }

    std::string createdBy;

    if (const auto* value = object.if_contains("createdBy"))
    {
        if (!value->is_string())
        {
            InvalidBundle(source, "createdBy must be a string");
        }

        createdBy = value->as_string().c_str();
    }

    return ReplayBundle(
        state,
        std::move(events),
        stopNanoseconds,
        std::move(createdBy)
    );
}

boost::json::value Compucolor::Impl::App::Replay::ReplayBundle::ToJson() const
{
    boost::json::object root;
    root["version"] = 1;

    if (!_createdBy.empty())
    {
        root["createdBy"] = _createdBy;
    }

    root["state"] = _state;

    boost::json::array input;
    input.reserve(_input.size());

    for (const auto& event : _input)
    {
        boost::json::object entry;
        entry["t"] = event.emulatedNanoseconds;
        entry["key"] = ReplayKeyNames::ToString(event.key);
        entry["down"] = event.down;
        input.push_back(std::move(entry));
    }

    root["input"] = std::move(input);

    boost::json::object stop;
    stop["emulatedNanoseconds"] = _stopNanoseconds;
    root["stop"] = std::move(stop);

    return boost::json::value(std::move(root));
}
