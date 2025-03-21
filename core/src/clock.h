#pragma once
#include "defines.h"

struct Clock {
  f64 start_time;
  f64 elapsed;
};

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non-started clocks.
KAPI void clock_update(Clock* clock);

// Starts the provided clock. Resets elapsed.
KAPI void clock_start(Clock* clock);

// Stops the provided clock. Does not reset elapsed time.
KAPI void clock_stop(Clock* clock);

