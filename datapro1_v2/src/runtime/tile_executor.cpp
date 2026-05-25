#include "dp1v2/runtime/tile_executor.hpp"

#include <chrono>
#include <stdexcept>

#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/task_arena.h>

namespace dp1v2 {
namespace {

void validateExecutionContract(
    const std::vector<TileTask>& tasks,
    const std::vector<TileResult>& results,
    const TileExecutionConfig& config)
{
    if (results.size() != tasks.size()) {
        throw std::logic_error("tile executor results must be pre-sized to tasks size");
    }
    if (config.num_threads < 0) {
        throw std::logic_error("tile executor num_threads must be >= 0");
    }
}

TileExecutionSummary summarizeResults(
    const std::vector<TileResult>& results,
    const std::uint64_t duration_ns)
{
    TileExecutionSummary summary{};
    summary.total_tasks = results.size();
    summary.duration_ns = duration_ns;

    for (const TileResult& result : results) {
        switch (result.status) {
        case TileResultStatus::Completed:
            ++summary.completed_tasks;
            break;
        case TileResultStatus::Failed:
            ++summary.failed_tasks;
            break;
        case TileResultStatus::Unsupported:
            ++summary.unsupported_tasks;
            break;
        case TileResultStatus::Skipped:
        case TileResultStatus::Disabled:
            break;
        }
    }

    return summary;
}

} // namespace

TileExecutionSummary TileExecutor::execute(
    const std::vector<TileTask>& tasks,
    std::vector<TileResult>& results,
    const TileProcessor& processor,
    const TileExecutionConfig& config) const
{
    validateExecutionContract(tasks, results, config);

    const auto start_time = std::chrono::steady_clock::now();

    if (config.num_threads == 0) {
        oneapi::tbb::parallel_for(
            std::size_t{0},
            tasks.size(),
            [&](const std::size_t index) {
                results[index] = processor.process(tasks[index]);
            });
    } else {
        oneapi::tbb::task_arena arena(config.num_threads);
        arena.execute([&] {
            oneapi::tbb::parallel_for(
                std::size_t{0},
                tasks.size(),
                [&](const std::size_t index) {
                    results[index] = processor.process(tasks[index]);
                });
        });
    }

    const auto end_time = std::chrono::steady_clock::now();
    const auto duration_ns = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time).count());

    return summarizeResults(results, duration_ns);
}

} // namespace dp1v2
