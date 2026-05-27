# counterpoint-engine-c

C99 port of the [counterpoint-engine](https://github.com/SuperInstance/counterpoint-engine) Python library — species counterpoint rules, voice leading, interval math, and consonance tables.

Designed for embedded music synthesis and bare-metal audio processing.

## What It Does

- **Interval math** — compute intervals, interval classes, consonance/dissonance classification
- **Pitch class utilities** — MIDI pitch ↔ pitch class ↔ octave conversion
- **Scale/mode support** — major, minor, Dorian, Mixolydian scale degrees and membership
- **Voice leading** — classify motion types (parallel, similar, contrary, oblique)
- **Rule checking** — parallel fifths/octaves, hidden fifths, tritone outlines, illegal leaps
- **Species counterpoint generation** — backtracking solver for first-species counterpoint
- **Laman rigidity** — simplified structural rigidity check for constraint graphs
- **Consonance table** — 25-entry lookup table for intervals 0-24 semitones

## C API

```c
#include <counterpoint_engine.h>

/* Interval classification */
int is_consonant = ce_is_consonance(7);   /* P5 → yes */
const char *name = ce_interval_name(7);    /* "P5" */

/* Voice leading */
int parallel = ce_has_parallel_perfect(60, 62, 67, 69);  /* parallel fifths */

/* Generate first-species counterpoint */
int cf[] = {60, 62, 64, 65, 67};  /* C D E F G */
int cp[5];
CeConstraints c = {.tonic = 0, .mode = CE_MODE_MAJOR, .min_pitch = 60, .max_pitch = 79};
int n = ce_generate_first_species(cf, 5, cp, &c);

/* Analyze errors */
CeErrorReport rpt = ce_analyze_errors(cf, cp, 5, &c);
printf("Errors: %d\n", rpt.total_errors);
```

## Build

```sh
make        # builds libcounterpoint_engine.a
make test   # builds and runs tests
make clean
```

No dependencies beyond C99, `libc`, and `libm`.

## Port Notes

Ported from ~6K lines of Python implementing species counterpoint as constraint satisfaction with Laman rigidity. The C port focuses on:

- Core interval arithmetic and consonance lookup tables
- Voice-leading rule engine (parallel/hidden perfect intervals, motion classification)
- Backtracking first-species counterpoint generator
- Simplified Laman rigidity check (edge count + min degree)

The Python library's canon generation and tensor-MIDI output are omitted in favor of the core algorithms suitable for embedded use.

## License

MIT
