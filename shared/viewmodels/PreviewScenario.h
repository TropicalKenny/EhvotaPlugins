#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::preview {

enum class ProductArea {
    refHub,
    refTime,
    stemPrep
};

enum class Tone {
    neutral,
    liveMix,
    reference,
    proposed,
    positive,
    caution,
    error,
    lowConfidence
};

struct Badge {
    std::string text;
    Tone tone { Tone::neutral };
};

struct Metric {
    std::string label;
    std::string value;
    std::string detail;
    Tone tone { Tone::neutral };
};

struct Panel {
    std::string title;
    std::string summary;
    std::vector<Metric> metrics;
};

struct Scenario {
    std::string id;
    std::string journey;
    std::string title;
    std::string description;
    ProductArea area { ProductArea::refHub };
    std::string activeView;
    std::string reference;
    std::string sectionPair;
    std::vector<Badge> badges;
    std::vector<Panel> panels;
    std::string inspectorTitle;
    std::vector<std::string> inspectorLines;
};

[[nodiscard]] const std::vector<Scenario>& allScenarios();
[[nodiscard]] const Scenario* findScenario(std::string_view id);
[[nodiscard]] std::vector<std::string> validateScenarioCatalogue();

} // namespace ehvota::preview
