#ifndef COUNTERPOINT_ENGINE_H
#define COUNTERPOINT_ENGINE_H

#include <math.h>
#include <string.h>

/* ── Interval classification ────────────────────────────────────────── */

typedef struct {
    int semitones;
} Interval;

Interval interval_between(int a, int b);
int interval_is_perfect_consonance(int semitones);
int interval_is_imperfect_consonance(int semitones);
int interval_is_consonance(int semitones);
int interval_is_dissonance(int semitones);
const char* interval_name(int semitones);

/* ── Note ────────────────────────────────────────────────────────────── */

typedef struct {
    int midi;
} Note;

double note_frequency(int midi);
Interval note_interval_to(int a, int b);

/* ── Voice pair (cantus firmus + counterpoint) ──────────────────────── */

typedef struct {
    int *cf;
    int *cp;
    int length;
} VoicePair;

VoicePair voice_pair_new(int *cf, int *cp, int length);
int voice_pair_consonance_count(VoicePair *vp);
int voice_pair_dissonance_count(VoicePair *vp);

/* ── Violations ──────────────────────────────────────────────────────── */

typedef enum {
    VIOLATION_NONE = 0,
    VIOLATION_DISSONANCE,
    VIOLATION_PARALLEL_FIFTHS,
    VIOLATION_PARALLEL_OCTAVES,
    VIOLATION_LARGE_LEAP,
    VIOLATION_VOICE_CROSSING,
} ViolationType;

typedef struct {
    ViolationType type;
    int index;
    int detail;
} Violation;

/* ── Check result ────────────────────────────────────────────────────── */

#define MAX_VIOLATIONS 64

typedef struct {
    Violation violations[MAX_VIOLATIONS];
    int violation_count;
    double score;
} CheckResult;

int check_result_is_valid(CheckResult *r);

/* ── Counterpoint checker ───────────────────────────────────────────── */

typedef struct {
    int allow_imperfect;
    int max_leap;
} CounterpointChecker;

CounterpointChecker checker_default(void);
CheckResult checker_check(CounterpointChecker *c, VoicePair *vp);
double checker_score(CounterpointChecker *c, VoicePair *vp);

#endif
