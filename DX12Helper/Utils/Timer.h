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
    timeElapsedBeforeLastPause += std::chrono::duration_cast<Duration>(Clock::now() - lastStart);
  }

  typename Clock::time_point lastStart = Clock::now();
  Duration timeElapsedBeforeLastPause{};
  bool isRunning = true;
};

template<
  typename Duration = std::chrono::milliseconds,
  typename Clock = std::chrono::high_resolution_clock>
class Timer
{
public:
  typename Duration::rep TimeElapsed(std::string name) { return DurationElapsed(name).count(); }

  Duration DurationElapsed(std::string name)
  {
    if (!Has(name)) {
      return {};
    }
    auto track = timers[name];
    auto timeElapsed = track.timeElapsedBeforeLastPause;
    if (track.isRunning) {
      timeElapsed += std::chrono::duration_cast<Duration>(Clock::now() - track.lastStart);
    }
    return timeElapsed;
  }

  void Start(std::string name)
  {
    if (!Has(name)) {
      timers.insert({name, TimeTrack<Duration, Clock>{}});
    }
    timers[name].Start();
  }

  void Pause(std::string name)
  {
    if (!Has(name)) {
      return;
    }
    timers[name].Pause();
  }

  void Reset(std::string name)
  {
    if (Has(name)) {
      timers[name] = {};
    }
  }

  void Remove(std::string name) { timers.erase(timers.find(name)); }

  bool Has(std::string name) { return static_cast<bool>(timers.count(name)); }

private:
  std::unordered_map<std::string, TimeTrack<Duration, Clock>> timers{};
};

}  // namespace dxh
