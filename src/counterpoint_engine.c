#include "counterpoint_engine.h"
#include <stdlib.h>

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

/* ── Voice pair ──────────────────────────────────────────────────────── */

VoicePair voice_pair_new(int *cf, int *cp, int length) {
    VoicePair vp = { cf, cp, length };
    return vp;
}

int voice_pair_consonance_count(VoicePair *vp) {
    int count = 0;
    for (int i = 0; i < vp->length; i++) {
        if (interval_is_consonance(vp->cf[i] - vp->cp[i])) count++;
    }
    return count;
}

int voice_pair_dissonance_count(VoicePair *vp) {
    return vp->length - voice_pair_consonance_count(vp);
}

/* ── Checker ─────────────────────────────────────────────────────────── */

CounterpointChecker checker_default(void) {
    CounterpointChecker c = { 1, 8 };
    return c;
}

CheckResult checker_check(CounterpointChecker *c, VoicePair *vp) {
    CheckResult r;
    r.violation_count = 0;
    r.score = 0.0;

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
        if ((prev_iv == 0 && curr_iv == 0) || (prev_iv == 12 && curr_iv == 12)) {
            if (r.violation_count < MAX_VIOLATIONS) {
                r.violations[r.violation_count].type = VIOLATION_PARALLEL_OCTAVES;
                r.violations[r.violation_count].index = i;
                r.violations[r.violation_count].detail = prev_iv;
                r.violation_count++;
            }
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

    /* Score: consonance ratio minus penalty */
    double n = (double)vp->length;
    double cons_ratio = (double)voice_pair_consonance_count(vp) / n;
    double penalty = (double)r.violation_count * 0.1;
    r.score = cons_ratio - penalty;
    if (r.score < 0.0) r.score = 0.0;

    return r;
}

int check_result_is_valid(CheckResult *r) {
    return r->violation_count == 0;
}

double checker_score(CounterpointChecker *c, VoicePair *vp) {
    CheckResult r = checker_check(c, vp);
    return r.score;
}
