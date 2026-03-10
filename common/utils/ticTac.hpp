
#pragma once

#include <chrono>

using TicClock = std::chrono::high_resolution_clock;

class TicTacTimer {
public:
  TicTacTimer() : t0(TicClock::now()) {}
  void tic() { t0 = TicClock::now(); };

  inline TicClock::duration tac() { return TicClock::now() - t0; };

  template <typename _Ratio> inline double tac() {
    return std::chrono::duration<double, _Ratio>(tac()).count();
  }

private:
  TicClock::time_point t0;
};
