#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::preview {

struct JourneyStep {
    std::string title;
    std::string instruction;
};

struct JourneyDefinition {
    int number {};
    std::string id;
    std::string title;
    std::string primaryScenarioId;
    std::vector<JourneyStep> steps;
};

[[nodiscard]] const std::vector<JourneyDefinition>& allJourneys();
[[nodiscard]] const JourneyDefinition* findJourneyForScenario(std::string_view scenarioId);
[[nodiscard]] std::vector<std::string> validateJourneyCatalogue();

class JourneyReplay final {
public:
    [[nodiscard]] bool selectForScenario(std::string_view scenarioId);
    [[nodiscard]] bool setStepNumber(std::size_t oneBasedStep);
    [[nodiscard]] bool next();
    [[nodiscard]] bool previous();

    [[nodiscard]] const JourneyDefinition* journey() const noexcept;
    [[nodiscard]] const JourneyStep* currentStep() const noexcept;
    [[nodiscard]] std::size_t currentStepNumber() const noexcept;
    [[nodiscard]] std::size_t totalSteps() const noexcept;
    [[nodiscard]] bool canGoNext() const noexcept;
    [[nodiscard]] bool canGoPrevious() const noexcept;
    [[nodiscard]] double progress() const noexcept;

private:
    const JourneyDefinition* journey_ { nullptr };
    std::size_t stepIndex_ {};
};

} // namespace ehvota::preview
