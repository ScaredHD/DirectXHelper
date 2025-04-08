#ifndef DXH_TIMER_H_
#define DXH_TIMER_H_

#include <chrono>
#include <string>
#include <unordered_map>

namespace dxh
{

template<typename Duration = std::chrono::milliseconds,
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

template<typename Duration = std::chrono::milliseconds,
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
    timers[name].Pause();
  }

  void ResetTimer(std::string name)
  {
    if (HasTimer(name)) {
      timers_[name] = {};
    }
  }

  void RemoveTimer(std::string name) { timers_.erase(timers_.find(name)); }

  bool HasTimer(std::string name)
  {
    if (timers_.count(name)) {
      return true;
    }
    return false;
  }

private:
  std::unordered_map<std::string, TimeTrack<Duration, Clock>> timers_{};
};


class EventTimer {
public:
  
private:
  
};

}  // namespace dxh


#endif  // DXH_TIMER_H_