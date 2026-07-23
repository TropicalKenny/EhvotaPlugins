#include "PreviewLaunchOptions.h"

#include "PreviewScenario.h"

#include <charconv>
#include <string_view>
#include <system_error>
#include <utility>

namespace ehvota::preview {
namespace {

constexpr std::string_view scenarioOption { "--scenario" };
constexpr std::string_view widthOption { "--width" };
constexpr std::string_view heightOption { "--height" };
constexpr std::string_view stepOption { "--step" };

bool isLongOption(const std::string& argument, const std::string_view option)
{
    return argument == option
        || (argument.size() > option.size()
            && argument[option.size()] == '='
            && std::string_view(argument).substr(0, option.size()) == option);
}

bool isSupportedOption(const std::string& argument)
{
    return isLongOption(argument, scenarioOption)
        || isLongOption(argument, widthOption)
        || isLongOption(argument, heightOption)
        || isLongOption(argument, stepOption);
}

std::optional<std::string_view> valueForOption(
    const std::vector<std::string>& arguments,
    const std::string_view option)
{
    for (const auto& argument : arguments) {
        if (!isLongOption(argument, option))
            continue;

        if (argument.size() == option.size())
            return std::string_view {};

        return std::string_view(argument).substr(option.size() + 1);
    }

    return std::nullopt;
}

std::string toString(const std::string_view value)
{
    return { value.begin(), value.end() };
}

std::optional<int> parseBoundedInteger(
    const std::vector<std::string>& arguments,
    const std::string_view option,
    const int minimum,
    const int maximum,
    const std::string_view units,
    std::string& error)
{
    const auto value = valueForOption(arguments, option);
    if (!value.has_value())
        return std::nullopt;

    int parsed {};
    const auto* begin = value->data();
    const auto* end = begin + value->size();
    const auto result = std::from_chars(begin, end, parsed);

    if (value->empty() || result.ec != std::errc {} || result.ptr != end) {
        error = toString(option) + " requires an integer value using "
            + toString(option) + "=<value>.";
        return std::nullopt;
    }

    if (parsed < minimum || parsed > maximum) {
        error = toString(option) + " must be between " + std::to_string(minimum)
            + " and " + std::to_string(maximum);
        if (!units.empty())
            error += " " + toString(units);
        error += ".";
        return std::nullopt;
    }

    return parsed;
}

} // namespace

const std::string& defaultLaunchScenarioId()
{
    const auto& scenarios = allScenarios();
    static const std::string empty;
    return scenarios.empty() ? empty : scenarios.front().id;
}

LaunchOptionsResult parseLaunchOptions(const std::vector<std::string>& arguments)
{
    for (const auto& argument : arguments) {
        if (!isSupportedOption(argument))
            return { std::nullopt, "Unknown option: " + argument };
    }

    LaunchOptions options;
    options.scenarioId = defaultLaunchScenarioId();
    if (options.scenarioId.empty())
        return { std::nullopt, "No preview scenarios are registered." };

    if (const auto scenarioId = valueForOption(arguments, scenarioOption)) {
        if (scenarioId->empty())
            return { std::nullopt, "--scenario requires a value using --scenario=<id>." };
        options.scenarioId = toString(*scenarioId);
    }

    if (findScenario(options.scenarioId) == nullptr)
        return { std::nullopt, "Unknown preview scenario: " + options.scenarioId };

    std::string error;
    if (const auto width = parseBoundedInteger(
            arguments,
            widthOption,
            minimumLaunchWidth,
            maximumLaunchWidth,
            "logical pixels",
            error)) {
        options.contentWidth = *width;
    }
    else if (!error.empty()) {
        return { std::nullopt, error };
    }

    if (const auto height = parseBoundedInteger(
            arguments,
            heightOption,
            minimumLaunchHeight,
            maximumLaunchHeight,
            "logical pixels",
            error)) {
        options.contentHeight = *height;
    }
    else if (!error.empty()) {
        return { std::nullopt, error };
    }

    if (const auto step = parseBoundedInteger(
            arguments,
            stepOption,
            minimumLaunchStep,
            maximumLaunchStep,
            "steps",
            error)) {
        options.journeyStep = *step;
    }
    else if (!error.empty()) {
        return { std::nullopt, error };
    }

    return { std::move(options), {} };
}

} // namespace ehvota::preview
