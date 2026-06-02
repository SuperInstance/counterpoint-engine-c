#include "counterpoint_engine.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int test_count = 0;
#define TEST(name) do { printf("  ✓ %s\n", name); test_count++; } while(0)

/* ── Original tests (10) ───────────────────────────────────────────── */

static void test_interval_consonance(void) {
    assert(interval_is_perfect_consonance(0));
    assert(interval_is_perfect_consonance(7));
    assert(interval_is_perfect_consonance(12));
    assert(interval_is_imperfect_consonance(3));
    assert(interval_is_imperfect_consonance(4));
    assert(interval_is_dissonance(6));
    assert(interval_is_dissonance(1));
    TEST("interval consonance classification");
}

static void test_interval_between(void) {
    assert(interval_between(60, 67).semitones == 7);
    assert(interval_between(67, 60).semitones == 7);
    assert(interval_between(60, 60).semitones == 0);
    TEST("interval between");
}

static void test_interval_names(void) {
    assert(strcmp(interval_name(0), "P1") == 0);
    assert(strcmp(interval_name(7), "P5") == 0);
    assert(strcmp(interval_name(6), "TT") == 0);
    TEST("interval names");
}

static void test_note_frequency(void) {
    assert(fabs(note_frequency(69) - 440.0) < 0.01);
    assert(fabs(note_frequency(60) - 261.63) < 0.1);
    TEST("note frequency");
}

static void test_consonant_counterpoint(void) {
    int cf[] = {60, 62, 64, 65, 67};
    int cp[] = {64, 65, 67, 69, 71};
    VoicePair vp = voice_pair_new(cf, cp, 5);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(r.score > 0.5);
    TEST("consonant counterpoint");
}

static void test_dissonant_counterpoint(void) {
    int cf[] = {60, 62, 64};
    int cp[] = {66, 68, 70};
    VoicePair vp = voice_pair_new(cf, cp, 3);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(r.violation_count > 0);
    TEST("dissonant counterpoint");
}

static void test_parallel_fifths(void) {
    int cf[] = {60, 62};
    int cp[] = {67, 69};
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(count_violations_by_type(&r, VIOLATION_PARALLEL_FIFTHS) > 0);
    TEST("parallel fifths detected");
}

static void test_large_leap(void) {
    int cf[] = {60, 62};
    int cp[] = {60, 72};
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c;
    c.allow_imperfect = 1;
    c.max_leap = 7;
    CheckResult r = checker_check(&c, &vp);
    assert(count_violations_by_type(&r, VIOLATION_LARGE_LEAP) > 0);
    TEST("large leap detected");
}

static void test_voice_crossing(void) {
    int cf[] = {72, 72};
    int cp[] = {60, 60};
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(count_violations_by_type(&r, VIOLATION_VOICE_CROSSING) > 0);
    TEST("voice crossing detected");
}

static void test_voice_pair_counts(void) {
    int cf[] = {60, 62, 64};
    int cp[] = {64, 65, 67};
    VoicePair vp = voice_pair_new(cf, cp, 3);
    assert(voice_pair_consonance_count(&vp) == 3);
    assert(voice_pair_dissonance_count(&vp) == 0);
    TEST("voice pair consonance counts");
}

/* ── New tests (target: 20+) ──────────────────────────────────────── */

static void test_note_name_basic(void) {
    assert(strcmp(note_name(60), "C4") == 0);
    assert(strcmp(note_name(69), "A4") == 0);
    assert(strcmp(note_name(61), "C#4") == 0);
    TEST("note names");
}

static void test_note_name_octaves(void) {
    assert(strcmp(note_name(48), "C3") == 0);
    assert(strcmp(note_name(72), "C5") == 0);
    assert(strcmp(note_name(0), "C-1") == 0);
    TEST("note names across octaves");
}

static void test_melody_range(void) {
    int melody[] = {60, 64, 67, 72, 64};
    assert(melody_range(melody, 5) == 12);
    /* Single note */
    assert(melody_range(melody, 1) == 0);
    TEST("melody range");
}

static void test_melody_range_null(void) {
    assert(melody_range(NULL, 5) == 0);
    assert(melody_range(NULL, 0) == 0);
    TEST("melody range null/empty");
}

static void test_has_interval_pattern(void) {
    int melody[] = {60, 67, 64, 72};
    assert(has_interval_pattern(melody, 4, 7));
    assert(!has_interval_pattern(melody, 4, 6));
    TEST("interval pattern detection");
}

static void test_has_interval_pattern_edge(void) {
    int melody[] = {60};
    assert(!has_interval_pattern(melody, 1, 7));
    assert(!has_interval_pattern(NULL, 5, 7));
    TEST("interval pattern edge cases");
}

static void test_count_violations_by_type(void) {
    int cf[] = {60, 62, 64};
    int cp[] = {66, 68, 70};  /* Tritone-heavy → dissonances */
    VoicePair vp = voice_pair_new(cf, cp, 3);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    int diss = count_violations_by_type(&r, VIOLATION_DISSONANCE);
    assert(diss > 0);
    TEST("count violations by type");
}

static void test_count_violations_null(void) {
    assert(count_violations_by_type(NULL, VIOLATION_DISSONANCE) == 0);
    TEST("count violations null safety");
}

static void test_check_result_is_valid(void) {
    CheckResult r;
    memset(&r, 0, sizeof(r));
    r.violation_count = 0;
    assert(check_result_is_valid(&r));
    r.violation_count = 1;
    assert(!check_result_is_valid(&r));
    assert(!check_result_is_valid(NULL));
    TEST("check_result_is_valid");
}

static void test_checker_null_safety(void) {
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, NULL);
    assert(r.violation_count == 0);
    r = checker_check(NULL, NULL);
    assert(r.violation_count == 0);
    TEST("checker null safety");
}

static void test_voice_pair_null_safety(void) {
    assert(voice_pair_consonance_count(NULL) == 0);
    assert(voice_pair_dissonance_count(NULL) == 0);
    VoicePair vp = {NULL, NULL, 0};
    assert(voice_pair_consonance_count(&vp) == 0);
    TEST("voice pair null safety");
}

static void test_note_interval_to(void) {
    Interval iv = note_interval_to(60, 67);
    assert(iv.semitones == 7);
    TEST("note_interval_to");
}

static void test_note_frequency_range(void) {
    /* Lower notes have lower frequencies */
    assert(note_frequency(60) < note_frequency(72));
    assert(note_frequency(0) < note_frequency(127));
    /* Octave doubles frequency */
    assert(fabs(note_frequency(60) * 2.0 - note_frequency(72)) < 0.1);
    TEST("note frequency range & octave doubling");
}

static void test_perfect_counterpoint(void) {
    /* Valid counterpoint: all consonances, no parallels, no crossing */
    int cf[] = {60, 62, 64};
    int cp[] = {67, 65, 64};  /* P5, m3, P1 */
    VoicePair vp = voice_pair_new(cf, cp, 3);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(check_result_is_valid(&r));
    assert(r.score > 0.9);
    TEST("perfect counterpoint (valid)");
}

static void test_parallel_octaves(void) {
    int cf[] = {60, 62, 64};
    int cp[] = {72, 74, 76};  /* Octave parallel motion */
    VoicePair vp = voice_pair_new(cf, cp, 3);
    CounterpointChecker c = checker_default();
    CheckResult r = checker_check(&c, &vp);
    assert(count_violations_by_type(&r, VIOLATION_PARALLEL_OCTAVES) > 0);
    TEST("parallel octaves detected");
}

static void test_score_bounds(void) {
    int cf[] = {60, 62};
    int cp[] = {60, 62};
    VoicePair vp = voice_pair_new(cf, cp, 2);
    CounterpointChecker c = checker_default();
    double s = checker_score(&c, &vp);
    assert(s >= 0.0);
    assert(s <= 1.0);
    TEST("score bounds [0, 1]");
}

static void test_benchmark_counterpoint(void) {
    double elapsed = benchmark_counterpoint(100000);
    assert(elapsed >= 0.0);
    printf("    Counterpoint benchmark: 100k iterations in %.4f s\n", elapsed);
    TEST("counterpoint benchmark");
}

static void test_all_interval_names(void) {
    /* Test all 12 interval names are valid */
    for (int i = 0; i < 12; i++) {
        const char *name = interval_name(i);
        assert(name != NULL);
        assert(strlen(name) >= 2);
    }
    TEST("all 12 interval names valid");
}

static void test_negative_midi(void) {
    /* Negative intervals should still work via abs */
    assert(interval_between(-5, 0).semitones == 5);
    TEST("negative MIDI interval");
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== counterpoint-engine tests ===\n\n");

    /* Original */
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

    /* New */
    test_note_name_basic();
    test_note_name_octaves();
    test_melody_range();
    test_melody_range_null();
    test_has_interval_pattern();
    test_has_interval_pattern_edge();
    test_count_violations_by_type();
    test_count_violations_null();
    test_check_result_is_valid();
    test_checker_null_safety();
    test_voice_pair_null_safety();
    test_note_interval_to();
    test_note_frequency_range();
    test_perfect_counterpoint();
    test_parallel_octaves();
    test_score_bounds();
    test_benchmark_counterpoint();
    test_all_interval_names();
    test_negative_midi();

    printf("\n✅ All %d tests passed!\n", test_count);
    return 0;
}
