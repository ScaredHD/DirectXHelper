#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace dxh
{

template<
  typename Duration = std::chrono::milliseconds,
  typename Clock = std::chrono::high_resolution_clock>
struct TimeTrack {
  TimeTrack() = default;

  void Start()
  {
    if (isRunning) {
      return;
    }
    isRunning = true;
    lastStart = Clock::now();
  }

  void Pause()
  {
    if (!isRunning) {
      return;
    }

    isRunning = false;
    timeElapsedBeforeLastPause += Clock::now() - lastStart;
  }

  typename Clock::time_point lastStart = Clock::now();
  Duration timeElapsedBeforeLastPause{};
  bool isRunning = true;
};

/**
 * @class Timer
 * @brief Manages multiple named timers for measuring elapsed time.
 *
 * This class provides functionality to start, pause, reset, and remove timers identified by string
 * names. Each timer tracks its elapsed duration and can be queried for the time elapsed or
 * duration.
 *
 * @tparam Duration The duration type used for time measurement (e.g., std::chrono::milliseconds).
 * @tparam Clock The clock type used for time measurement (e.g., std::chrono::steady_clock).
 *
 * Public Methods:
 * - typename Duration::rep TimeElapsed(std::string name): Returns the elapsed time for the
 * specified timer.
 * - Duration DurationElapsed(std::string name): Returns the duration elapsed for the specified
 * timer.
 * - void StartTimer(std::string name): Starts or resumes the specified timer.
 * - void PauseTimer(std::string name): Pauses the specified timer.
 * - void ResetTimer(std::string name): Resets the specified timer to zero.
 * - void RemoveTimer(std::string name): Removes the specified timer from the manager.
 * - bool HasTimer(std::string name): Checks if a timer with the given name exists.
 *
 * Private Members:
 * - std::unordered_map<std::string, TimeTrack<Duration, Clock>> timers_: Stores the timers by name.
 */
template<
  typename Duration = std::chrono::milliseconds,
  typename Clock = std::chrono::high_resolution_clock>
class Timer
{
public:
  typename Duration::rep TimeElapsed(std::string name) { return DurationElapsed(name).count(); }

  Duration DurationElapsed(std::string name)
  {
    if (!HasTimer(name)) {
      return {};
    }
    auto track = timers_[name];
    auto timeElapsed = track.timeElapsedBeforeLastPause;
    if (track.isRunning) {
      timeElapsed += std::chrono::duration_cast<Duration>(Clock::now() - track.lastStart);
    }
    return timeElapsed;
  }

  void StartTimer(std::string name)
  {
    if (!HasTimer(name)) {
      timers_.insert({name, TimeTrack{}});
    }
    timers_[name].Start();
  }

  void PauseTimer(std::string name)
  {
    if (!HasTimer(name)) {
      return;
    }
    timers_[name].Pause();
  }

  void ResetTimer(std::string name)
  {
    if (HasTimer(name)) {
      timers_[name] = {};
    }
  }

  void RemoveTimer(std::string name) { timers_.erase(timers_.find(name)); }

  bool HasTimer(std::string name) { return static_cast<bool>(timers_.count(name)); }

private:
  std::unordered_map<std::string, TimeTrack<Duration, Clock>> timers_{};
};

}  // namespace dxh
