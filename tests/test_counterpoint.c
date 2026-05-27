/*
 * Tests for counterpoint-engine-c
 */
#include "counterpoint_engine.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define ASSERT_FEQ(a, b, eps) do { \
    if (fabs((a) - (b)) > (eps)) { \
        fprintf(stderr, "FAIL %s:%d: %.10f != %.10f\n", \
                __FILE__, __LINE__, (double)(a), (double)(b)); \
        return 1; \
    } \
} while(0)

static int test_interval(void)
{
    assert(ce_interval(60, 67) == 7);   /* C4 to G4 = P5 */
    assert(ce_interval(67, 60) == 7);   /* reverse */
    assert(ce_interval(60, 72) == 12);  /* C4 to C5 = P8 */
    printf("  PASS interval\n");
    return 0;
}

static int test_interval_class(void)
{
    assert(ce_interval_class(0) == 0);
    assert(ce_interval_class(7) == 5);  /* P5 = IC 5 */
    assert(ce_interval_class(6) == 6);  /* tritone */
    assert(ce_interval_class(5) == 5);  /* P4 = IC 5 */
    printf("  PASS interval_class\n");
    return 0;
}

static int test_consonance(void)
{
    assert(ce_is_perfect_consonance(0));   /* unison */
    assert(ce_is_perfect_consonance(7));   /* P5 */
    assert(ce_is_perfect_consonance(12));  /* P8 */
    assert(!ce_is_perfect_consonance(3));

    assert(ce_is_imperfect_consonance(3));  /* m3 */
    assert(ce_is_imperfect_consonance(4));  /* M3 */
    assert(ce_is_imperfect_consonance(9));  /* M6 */

    assert(ce_is_dissonance(1));  /* m2 */
    assert(ce_is_dissonance(6));  /* tritone */
    assert(!ce_is_dissonance(5)); /* P4 */

    printf("  PASS consonance\n");
    return 0;
}

static int test_interval_name(void)
{
    assert(strcmp(ce_interval_name(0), "P1") == 0);
    assert(strcmp(ce_interval_name(7), "P5") == 0);
    assert(strcmp(ce_interval_name(12), "P8") == 0);
    printf("  PASS interval_name\n");
    return 0;
}

static int test_pitch_class(void)
{
    assert(ce_pitch_class(60) == 0);   /* C4 */
    assert(ce_pitch_class(69) == 9);   /* A4 */
    assert(ce_octave(60) == 4);
    assert(ce_midifrom_pc(0, 4) == 60);
    printf("  PASS pitch_class\n");
    return 0;
}

static int test_scale(void)
{
    assert(ce_is_in_scale(0, 0, CE_MODE_MAJOR));   /* C in C major */
    assert(ce_is_in_scale(2, 0, CE_MODE_MAJOR));   /* D in C major */
    assert(!ce_is_in_scale(1, 0, CE_MODE_MAJOR));  /* C# not in C major */
    assert(ce_is_in_scale(1, 1, CE_MODE_MAJOR));   /* C# in C# major */

    /* Scale degrees */
    assert(ce_scale_degree(0, 0, CE_MODE_MAJOR) == 0);  /* C = degree 1 */
    assert(ce_scale_degree(7, 0, CE_MODE_MAJOR) == 4);  /* G = degree 5 */
    assert(ce_scale_degree(1, 0, CE_MODE_MAJOR) == -1); /* C# not in scale */
    printf("  PASS scale\n");
    return 0;
}

static int test_motion_classification(void)
{
    /* Parallel: both voices move in same direction by same amount */
    assert(ce_classify_motion(60, 62, 67, 69) == CE_MOTION_PARALLEL);

    /* Similar: same direction, different intervals */
    assert(ce_classify_motion(60, 62, 64, 65) == CE_MOTION_SIMILAR);

    /* Contrary: opposite directions */
    assert(ce_classify_motion(60, 62, 67, 65) == CE_MOTION_CONTRARY);

    /* Oblique: one voice stays */
    assert(ce_classify_motion(60, 60, 67, 69) == CE_MOTION_OBLIQUE);
    printf("  PASS motion_classification\n");
    return 0;
}

static int test_parallel_perfect(void)
{
    /* Parallel fifths: C-G to D-A */
    assert(ce_has_parallel_perfect(60, 62, 67, 69));

    /* Not parallel: different intervals */
    assert(!ce_has_parallel_perfect(60, 62, 64, 67));

    /* Parallel octaves */
    assert(ce_has_parallel_perfect(60, 62, 72, 74));
    printf("  PASS parallel_perfect\n");
    return 0;
}

static int test_hidden_perfect(void)
{
    /* Hidden fifth: similar motion into P5 (different intervals, same direction) */
    /* v1: 60→64 (up 4), v2: 64→71 (up 7), landing interval = 7 = P5 */
    assert(ce_has_hidden_perfect(60, 64, 64, 71));

    /* Contrary motion into P5 is OK */
    assert(!ce_has_hidden_perfect(60, 64, 72, 67));
    printf("  PASS hidden_perfect\n");
    return 0;
}

static int test_melodic_rules(void)
{
    assert(ce_is_allowed_melodic_interval(2));   /* step */
    assert(ce_is_allowed_melodic_interval(7));   /* P5 leap */
    assert(ce_is_allowed_melodic_interval(12));  /* octave */
    assert(!ce_is_allowed_melodic_interval(6));  /* tritone */
    assert(!ce_is_allowed_melodic_interval(13)); /* > P8 */
    printf("  PASS melodic_rules\n");
    return 0;
}

static int test_tritone_outline(void)
{
    int outline[] = {60, 63, 66};  /* C to F# through D# — tritone outline */
    assert(ce_has_tritone_outline(outline, 3));

    int clean[] = {60, 62, 64};  /* C-D-E — no tritone */
    assert(!ce_has_tritone_outline(clean, 3));
    printf("  PASS tritone_outline\n");
    return 0;
}

static int test_analyze_intervals(void)
{
    /* CF: C D E F, CP: G A B C */
    int cf[] = {60, 62, 64, 65};
    int cp[] = {67, 69, 71, 72};
    int intervals[4];
    int diss = ce_analyze_intervals(cf, cp, 4, intervals);
    assert(diss == 0);
    assert(intervals[0] == 7);  /* P5 */
    assert(intervals[3] == 7);  /* P5 */
    printf("  PASS analyze_intervals\n");
    return 0;
}

static int test_error_report(void)
{
    /* CF: C D E F, CP with a dissonance */
    int cf[] = {60, 62, 64, 65};
    int cp[] = {67, 63, 71, 72};  /* 63 is m3 above 62... that's a consonance actually */
    /* Actually let's use a real dissonance */
    int cp2[] = {67, 63, 70, 72};  /* 62→63 is m2 above... check: 64 to 70 is tritone! */
    CeErrorReport rpt = ce_analyze_errors(cf, cp2, 4, NULL);
    assert(rpt.dissonances >= 1);  /* E to Bb = tritone */
    printf("  PASS error_report (diss=%d, total=%d)\n",
           rpt.dissonances, rpt.total_errors);
    return 0;
}

static int test_generate_first_species(void)
{
    /* Simple cantus firmus in C major: C D E F G */
    int cf[] = {60, 62, 64, 65, 67};
    int cp[5];
    CeConstraints c;
    memset(&c, 0, sizeof(c));
    c.tonic = 0;
    c.mode = CE_MODE_MAJOR;
    c.min_pitch = 60;
    c.max_pitch = 79;

    int result = ce_generate_first_species(cf, 5, cp, &c);
    assert(result == 5);
    /* First note should be perfect consonance with CF */
    assert(ce_is_perfect_consonance(ce_interval(cf[0], cp[0])));
    /* Last note should be on tonic */
    assert(ce_pitch_class(cp[4]) == 0);
    /* No dissonances */
    for (int i = 0; i < 5; i++) {
        assert(ce_is_consonance(ce_interval(cf[i], cp[i])));
    }
    printf("  PASS generate_first_species\n  (");
    for (int i = 0; i < 5; i++) printf("%d ", cp[i]);
    printf(")\n");
    return 0;
}

static int test_generate_longer_cf(void)
{
    /* Longer cantus firmus: C C D E F G G */
    int cf[] = {60, 60, 62, 64, 65, 67, 67};
    int cp[7];
    CeConstraints c;
    memset(&c, 0, sizeof(c));
    c.tonic = 0;
    c.mode = CE_MODE_MAJOR;
    c.min_pitch = 60;
    c.max_pitch = 79;

    int result = ce_generate_first_species(cf, 7, cp, &c);
    assert(result == 7);
    printf("  PASS generate_longer_cf\n  (");
    for (int i = 0; i < 7; i++) printf("%d ", cp[i]);
    printf(")\n");
    return 0;
}

static int test_laman_rigid(void)
{
    /* Triangle: 3 vertices, 3 edges — rigid */
    int edges1[] = {0, 1, 1, 2, 0, 2};
    assert(ce_is_laman_rigid(3, edges1, 3));

    /* Two vertices connected: rigid */
    int edges2[] = {0, 1};
    assert(ce_is_laman_rigid(2, edges2, 1));

    /* Three vertices, only 2 edges: NOT rigid */
    int edges3[] = {0, 1, 1, 2};
    assert(!ce_is_laman_rigid(3, edges3, 2));

    printf("  PASS laman_rigid\n");
    return 0;
}

static int test_consonance_table(void)
{
    assert(CE_CONSONANCE_TABLE[0] == 1);   /* unison: perfect */
    assert(CE_CONSONANCE_TABLE[7] == 1);   /* P5: perfect */
    assert(CE_CONSONANCE_TABLE[3] == 2);   /* m3: imperfect */
    assert(CE_CONSONANCE_TABLE[4] == 2);   /* M3: imperfect */
    assert(CE_CONSONANCE_TABLE[6] == 0);   /* tritone: dissonant */
    assert(CE_CONSONANCE_TABLE[1] == 0);   /* m2: dissonant */
    printf("  PASS consonance_table\n");
    return 0;
}

/* ─── Main ─────────────────────────────────────────────────── */

typedef int (*test_fn)(void);

int main(void)
{
    test_fn tests[] = {
        test_interval,
        test_interval_class,
        test_consonance,
        test_interval_name,
        test_pitch_class,
        test_scale,
        test_motion_classification,
        test_parallel_perfect,
        test_hidden_perfect,
        test_melodic_rules,
        test_tritone_outline,
        test_analyze_intervals,
        test_error_report,
        test_generate_first_species,
        test_generate_longer_cf,
        test_laman_rigid,
        test_consonance_table,
    };
    int n = sizeof(tests) / sizeof(tests[0]);
    int failures = 0;
    printf("Running %d tests...\n", n);
    for (int i = 0; i < n; i++) {
        if (tests[i]()) failures++;
    }
    printf("\n%s: %d/%d passed\n",
           failures ? "FAIL" : "OK", n - failures, n);
    return failures;
}
