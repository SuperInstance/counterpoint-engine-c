# counterpoint-engine-c

C99 port of [counterpoint-engine-rs](https://github.com/SuperInstance/counterpoint-engine-rs) — species counterpoint rule checker with interval classification, first-species rules, and consonance scoring.

## What This Gives You

- **Interval classification** — Consonance (perfect/imperfect) and dissonance detection
- **First species rules** — Parallel 5ths/octaves, voice crossing, leap limits
- **Scoring** — Consonance ratio minus violation penalties, with configurable thresholds
- **Zero dependencies** — C99, only `<math.h>` and `<stdlib.h>`

## Quick Start

```c
#include "counterpoint_engine.h"

int cf[] = {60, 62, 64, 65, 67};  /* C D E F G */
int cp[] = {64, 65, 67, 69, 71};  /* E F G A B */
VoicePair vp = voice_pair_new(cf, cp, 5);

CounterpointChecker checker = checker_default();
CheckResult result = checker_check(&checker, &vp);

if (check_result_is_valid(&result)) {
    printf("Valid! Score: %.2f\n", result.score);
}
```

## Build & Test

```bash
make            # build static library
make test       # build and run tests
make clean
```

## API Reference

| Function | Description |
|----------|-------------|
| `voice_pair_new(cf, cp, n)` | Create a cantus firmus + counterpoint pair |
| `checker_default()` | Default rule checker |
| `checker_check(&checker, &vp)` | Evaluate all rules, return `CheckResult` |
| `check_result_is_valid(&result)` | Did the pair pass all rules? |
| `result.score` | Consonance score (higher = better) |
| `result.violations` | Array of `Violation` structs |

## How It Fits

- **[counterpoint-engine-rs](https://github.com/SuperInstance/counterpoint-engine-rs)** — The Rust original; this is the C port
- **[constraint-instrument](https://github.com/SuperInstance/constraint-instrument)** — Counterpoint rules feed into constraint terrain definitions
- **[flux-algebra-c](https://github.com/SuperInstance/flux-algebra-c)** — Harmonic analysis for counterpoint evaluation
- **[creative-engine-c](https://github.com/SuperInstance/creative-engine-c)** — Chaotic dynamics for generating candidate counterpoint lines

## Testing

10 tests covering interval classification, consonance detection, parallel fifth/octave rules, voice crossing, and scoring.

```bash
make test
```

## Installation

```bash
git clone https://github.com/SuperInstance/counterpoint-engine-c.git
cd counterpoint-engine-c
make
```

## License

MIT

Part of the [SuperInstance OpenConstruct](https://github.com/SuperInstance) ecosystem.
