# counterpoint-engine-c

C port of [counterpoint-engine](https://github.com/SuperInstance/counterpoint-engine) — species counterpoint rule checker.

## Features

- **Interval classification**: consonance (perfect/imperfect) and dissonance detection
- **First species rules**: parallel 5ths/octaves, voice crossing, leap limits
- **Scoring**: consonance ratio minus violation penalties
- **Zero dependencies**: C99, only `math.h`

## Build & Test

```bash
make test    # build and run tests
make clean
```

## API

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

## License

MIT

Part of the [SuperInstance OpenConstruct](https://github.com/SuperInstance/OpenConstruct) ecosystem.
