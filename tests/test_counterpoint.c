#include "counterpoint_engine.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ── Interval tests ──────────────────────────────────────────────────── */

static void test_interval_consonance(void) {
    assert(interval_is_perfect_consonance(0));   /* P1 */
    assert(interval_is_perfect_consonance(7));   /* P5 */
    assert(interval_is_perfect_consonance(12));  /* P8 */
    assert(interval_is_imperfect_consonance(3)); /* m3 */
    assert(interval_is_imperfect_consonance(4)); /* M3 */
    assert(interval_is_dissonance(6));           /* TT */
    assert(interval_is_dissonance(1));           /* m2 */
    printf("  ✓ interval consonance classification\n");
}

static void test_interval_between(void) {
    assert(interval_between(60, 67).semitones == 7);  /* C-G = P5 */
    assert(interval_between(67, 60).semitones == 7);  /* G-C = P5 */
    assert(interval_between(60, 60).semitones == 0);  /* unison */
    printf("  ✓ interval between\n");
}

static void test_interval_names(void) {
    assert(strcmp(interval_name(0), "P1") == 0);
    assert(strcmp(interval_name(7), "P5") == 0);
    assert(strcmp(interval_name(6), "TT") == 0);
    printf("  ✓ interval names\n");
}

/* ── Note tests ──────────────────────────────────────────────────────── */

static void test_note_frequency(void) {
    assert(fabs(note_frequency(69) - 440.0) < 0.01);     /* A4 = 440Hz */
    assert(fabs(note_frequency(60) - 261.63) < 0.1);     /* C4 ≈ 261.63Hz */
    printf("  ✓ note frequency (A4=%.2f, C4=%.2f)\n", note_frequency(69), note_frequency(60));
}

/* ── Counterpoint tests ──────────────────────────────────────────────── */

static void test_consonant_counterpoint(void) {
    /* C E G A B: consonant with CF C D E F G */
    int cf[] = {60, 62, 64, 65, 67};
    int cp[] = {64, 65, 67, 69, 71};
    VoicePair vp = voice_pair_new(cf, cp, 5);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(r.score > 0.5);
    printf("  ✓ consonant counterpoint: score=%.3f, violations=%d\n", r.score, r.violation_count);
}

static void test_dissonant_counterpoint(void) {
    /* Tritones */
    int cf[] = {60, 62, 64};
    int cp[] = {66, 68, 70};
    VoicePair vp = voice_pair_new(cf, cp, 3);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(r.violation_count > 0);
    printf("  ✓ dissonant counterpoint: %d violations\n", r.violation_count);
}

static void test_parallel_fifths(void) {
    /* C-G, D-A: parallel 5ths */
    int cf[] = {60, 62};
    int cp[] = {67, 69};
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    int found = 0;
    for (int i = 0; i < r.violation_count; i++) {
        if (r.violations[i].type == VIOLATION_PARALLEL_FIFTHS) found = 1;
    }
    assert(found);
    printf("  ✓ parallel fifths detected\n");
}

static void test_large_leap(void) {
    int cf[] = {60, 62};
    int cp[] = {60, 72}; /* octave leap */
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c;
    c.allow_imperfect = 1;
    c.max_leap = 7;
    CheckResult r = checker_check(&c, &vp);
    int found = 0;
    for (int i = 0; i < r.violation_count; i++) {
        if (r.violations[i].type == VIOLATION_LARGE_LEAP) found = 1;
    }
    assert(found);
    printf("  ✓ large leap detected\n");
}

static void test_voice_crossing(void) {
    int cf[] = {72, 72};
    int cp[] = {60, 60};
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    int found = 0;
    for (int i = 0; i < r.violation_count; i++) {
        if (r.violations[i].type == VIOLATION_VOICE_CROSSING) found = 1;
    }
    assert(found);
    printf("  ✓ voice crossing detected\n");
}

static void test_voice_pair_counts(void) {
    int cf[] = {60, 62, 64};
    int cp[] = {64, 65, 67};
    VoicePair vp = voice_pair_new(cf, cp, 3);
    assert(voice_pair_consonance_count(&vp) == 3);
    assert(voice_pair_dissonance_count(&vp) == 0);
    printf("  ✓ voice pair consonance counts\n");
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== counterpoint-engine tests ===\n\n");

    test_interval_consonance();
    test_interval_between();
    test_interval_names();
    test_note_frequency();
    test_consonant_counterpoint();
    test_dissonant_counterpoint();
    test_parallel_fifths();
    test_large_leap();
    test_voice_crossing();
    test_voice_pair_counts();

    printf("\n✅ All 10 tests passed!\n");
    return 0;
}
