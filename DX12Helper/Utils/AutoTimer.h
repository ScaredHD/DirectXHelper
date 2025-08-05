#pragma once

#include <chrono>
#include <string>

#ifndef DXH_DECLARE_SCOPED_AUTO_TIMER
  #define DXH_DECLARE_SCOPED_AUTO_TIMER(countType, resVarName, timeUnit)             \
    for (struct {                                                                    \
           dxh::AutomaticTimer<timeUnit, countType> _dxh_##resVarName##_auto_timer_; \
           bool cond;                                                                \
         } _dxh_##resVarName##_struct = {dxh::AutomaticTimer<timeUnit, countType>{   \
                                             resVarName, &resVarName##_duration},    \
                                         true};                                      \
         _dxh_##resVarName##_struct.cond; _dxh_##resVarName##_struct.cond = false)
#endif


#ifndef DXH_SCOPED_AUTO_TIMER
  #define DXH_SCOPED_AUTO_TIMER(countType, resVarName, timeUnit)                \
    countType resVarName{};                                                     \
    std::chrono::duration<countType, timeUnit::period> resVarName##_duration{}; \
    DXH_DECLARE_SCOPED_AUTO_TIMER(countType, resVarName, timeUnit)
#endif

#ifndef DXH_SCOPED_AUTO_TIMER_OUT_RESULT
  #define DXH_SCOPED_AUTO_TIMER_OUT_RESULT(outerScopeVar, timeUnit)                            \
    dxh::AutomaticTimer<timeUnit, decltype(outerScopeVar)> _dxh_##outerScopeVar##_auto_timer_{ \
        outerScopeVar};
#endif

namespace dxh
{

using Minutes = std::chrono::minutes;
using Millisecond = std::chrono::milliseconds;
using Second = std::chrono::seconds;

template<typename Duration>
std::string ToString(Duration duration)
{
  auto count = duration.count();
  auto str = std::to_string(count);

  using p = typename Duration::period;
  if (std::is_same_v<p, std::ratio<1>>) {
    return str + " seconds";
  }
  if (std::is_same_v<p, std::milli>) {
    return str + " milliseconds";
  }
  if (std::is_same_v<p, std::micro>) {
    return str + " microseconds";
  }
  if (std::is_same_v<p, std::nano>) {
    return str + " nanoseconds";
  }
  if (std::is_same_v<p, std::ratio<60>>) {
    return str + " minutes";
  }
}

template<typename Unit = std::chrono::milliseconds, typename CountType = long long,
         typename Clock = std::chrono::high_resolution_clock>
class AutomaticTimer
{
public:
  using DurationType = std::chrono::duration<CountType, typename Unit::period>;

  explicit AutomaticTimer(CountType& outTimeElapsed, DurationType* const outDuration = nullptr)
      : timeElapsed_{outTimeElapsed},
        duration_{outDuration},
        start_{Clock::now()}
  {
  }

  ~AutomaticTimer()
  {
    auto end = Clock::now();
    auto t = std::chrono::duration_cast<DurationType>(end - start_);
    timeElapsed_ = t.count();
    if (duration_) {
      *duration_ = t;
    }
  }

private:
  CountType& timeElapsed_;
  DurationType* const duration_;
  std::chrono::time_point<Clock> start_;
};

}  // namespace dxh