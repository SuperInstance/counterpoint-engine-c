#define _POSIX_C_SOURCE 199309L
#include "counterpoint_engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ── Interval classification ────────────────────────────────────────── */

Interval interval_between(int a, int b) {
    Interval iv;
    iv.semitones = abs(a - b) % 12;
    return iv;
}

int interval_is_perfect_consonance(int s) {
    s = abs(s) % 12;
    return s == 0 || s == 7 || s == 12;
}

int interval_is_imperfect_consonance(int s) {
    s = abs(s) % 12;
    return s == 3 || s == 4 || s == 8 || s == 9;
}

int interval_is_consonance(int s) {
    return interval_is_perfect_consonance(s) || interval_is_imperfect_consonance(s);
}

int interval_is_dissonance(int s) {
    return !interval_is_consonance(s);
}

const char* interval_name(int s) {
    s = abs(s) % 12;
    switch (s) {
        case 0:  return "P1";
        case 1:  return "m2";
        case 2:  return "M2";
        case 3:  return "m3";
        case 4:  return "M3";
        case 5:  return "P4";
        case 6:  return "TT";
        case 7:  return "P5";
        case 8:  return "m6";
        case 9:  return "M6";
        case 10: return "m7";
        case 11: return "M7";
        default: return "?";
    }
}

/* ── Note ────────────────────────────────────────────────────────────── */

double note_frequency(int midi) {
    return 440.0 * pow(2.0, (midi - 69) / 12.0);
}

Interval note_interval_to(int a, int b) {
    return interval_between(a, b);
}

/* ── Voice pair ──────────────────────────────────────────────────────── */

VoicePair voice_pair_new(int *cf, int *cp, int length) {
    VoicePair vp;
    vp.cf = cf;
    vp.cp = cp;
    vp.length = length;
    return vp;
}

int voice_pair_consonance_count(VoicePair *vp) {
    if (!vp || !vp->cf || !vp->cp || vp->length <= 0) return 0;
    int count = 0;
    for (int i = 0; i < vp->length; i++) {
        if (interval_is_consonance(vp->cf[i] - vp->cp[i])) count++;
    }
    return count;
}

int voice_pair_dissonance_count(VoicePair *vp) {
    if (!vp || vp->length <= 0) return 0;
    return vp->length - voice_pair_consonance_count(vp);
}

/* ── Checker ─────────────────────────────────────────────────────────── */

CounterpointChecker checker_default(void) {
    CounterpointChecker c;
    c.allow_imperfect = 1;
    c.max_leap = 8;
    return c;
}

CheckResult checker_check(CounterpointChecker *c, VoicePair *vp) {
    CheckResult r;
    memset(&r, 0, sizeof(r));

    if (!c || !vp || !vp->cf || !vp->cp || vp->length <= 0) {
        r.score = 0.0;
        return r;
    }

    /* Check consonances */
    for (int i = 0; i < vp->length; i++) {
        int iv = abs(vp->cf[i] - vp->cp[i]) % 12;
        if (interval_is_dissonance(iv) && r.violation_count < MAX_VIOLATIONS) {
            r.violations[r.violation_count].type = VIOLATION_DISSONANCE;
            r.violations[r.violation_count].index = i;
            r.violations[r.violation_count].detail = iv;
            r.violation_count++;
        }
    }

    /* Check parallel 5ths / octaves */
    for (int i = 1; i < vp->length; i++) {
        int prev_iv = abs(vp->cf[i-1] - vp->cp[i-1]) % 12;
        int curr_iv = abs(vp->cf[i] - vp->cp[i]) % 12;

        if (prev_iv == 7 && curr_iv == 7 && r.violation_count < MAX_VIOLATIONS) {
            r.violations[r.violation_count].type = VIOLATION_PARALLEL_FIFTHS;
            r.violations[r.violation_count].index = i;
            r.violations[r.violation_count].detail = 7;
            r.violation_count++;
        }
        if ((prev_iv == 0 && curr_iv == 0) && r.violation_count < MAX_VIOLATIONS) {
            r.violations[r.violation_count].type = VIOLATION_PARALLEL_OCTAVES;
            r.violations[r.violation_count].index = i;
            r.violations[r.violation_count].detail = 0;
            r.violation_count++;
        }
    }

    /* Check leaps in counterpoint */
    for (int i = 1; i < vp->length; i++) {
        int leap = abs(vp->cp[i] - vp->cp[i-1]);
        if (leap > c->max_leap && r.violation_count < MAX_VIOLATIONS) {
            r.violations[r.violation_count].type = VIOLATION_LARGE_LEAP;
            r.violations[r.violation_count].index = i;
            r.violations[r.violation_count].detail = leap;
            r.violation_count++;
        }
    }

    /* Voice crossing: CP below CF */
    for (int i = 0; i < vp->length; i++) {
        if (vp->cp[i] < vp->cf[i] && r.violation_count < MAX_VIOLATIONS) {
            r.violations[r.violation_count].type = VIOLATION_VOICE_CROSSING;
            r.violations[r.violation_count].index = i;
            r.violations[r.violation_count].detail = 0;
            r.violation_count++;
        }
    }

    /* Score */
    double n = (double)vp->length;
    double cons_ratio = (double)voice_pair_consonance_count(vp) / n;
    double penalty = (double)r.violation_count * 0.1;
    r.score = cons_ratio - penalty;
    if (r.score < 0.0) r.score = 0.0;

    return r;
}

int check_result_is_valid(CheckResult *r) {
    if (!r) return 0;
    return r->violation_count == 0;
}

double checker_score(CounterpointChecker *c, VoicePair *vp) {
    CheckResult r = checker_check(c, vp);
    return r.score;
}

/* ── Additional utility ─────────────────────────────────────────────── */

int count_violations_by_type(CheckResult *r, ViolationType type) {
    if (!r) return 0;
    int count = 0;
    for (int i = 0; i < r->violation_count; i++) {
        if (r->violations[i].type == type) count++;
    }
    return count;
}

int has_interval_pattern(int *melody, int n, int interval) {
    if (!melody || n < 2) return 0;
    for (int i = 1; i < n; i++) {
        if (abs(melody[i] - melody[i-1]) == interval) return 1;
    }
    return 0;
}

int melody_range(int *melody, int n) {
    if (!melody || n <= 0) return 0;
    int mn = melody[0], mx = melody[0];
    for (int i = 1; i < n; i++) {
        if (melody[i] < mn) mn = melody[i];
        if (melody[i] > mx) mx = melody[i];
    }
    return mx - mn;
}

const char* note_name(int midi) {
    static const char *names[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    static char buf[16];
    int pc = midi % 12;
    int oct = (midi / 12) - 1;
    sprintf(buf, "%s%d", names[pc], oct);
    return buf;
}

double benchmark_counterpoint(int iterations) {
    /* Create a test voice pair */
    int cf[8] = {60, 62, 64, 65, 67, 69, 71, 72};
    int cp[8] = {64, 65, 67, 69, 72, 71, 72, 72};
    VoicePair vp = voice_pair_new(cf, cp, 8);
    CounterpointChecker c = checker_default();

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < iterations; i++) {
        checker_check(&c, &vp);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1e9;
}
