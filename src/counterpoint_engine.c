/*
 * counterpoint_engine — species counterpoint rules, voice leading,
 *                        interval math, consonance tables
 *
 * C99 port of the Python counterpoint-engine library.
 * For embedded music synthesis and bare-metal audio.
 */

#include "counterpoint_engine.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ─── Consonance table ─────────────────────────────────────── */

/* 0=dissonant, 1=perfect consonance, 2=imperfect consonance */
const int CE_CONSONANCE_TABLE[25] = {
    1, /*  0: unison    */
    0, /*  1: m2        */
    0, /*  2: M2        */
    2, /*  3: m3        */
    2, /*  4: M3        */
    1, /*  5: P4 (note: imperfect against bass in some species) */
    0, /*  6: tritone   */
    1, /*  7: P5        */
    2, /*  8: m6        */
    2, /*  9: M6        */
    0, /* 10: m7        */
    0, /* 11: M7        */
    1, /* 12: P8        */
    0, 0, 0, 0, 0, 0, /* 13-18: compound dissonances */
    2, /* 19: m10 (= m3 + P8) */
    2, /* 20: M10 (= M3 + P8) */
    1, /* 21: P12 (= P5 + P8) */
    2, 2, 0, /* 22-24 */
};

/* ─── Interval math ────────────────────────────────────────── */

int ce_interval(int a, int b)
{
    int diff = b - a;
    return (diff >= 0) ? diff : -diff;
}

int ce_interval_class(int interval)
{
    int ic = interval % 12;
    return (ic > 6) ? (12 - ic) : ic;
}

int ce_is_perfect_consonance(int interval)
{
    int ic = ce_interval_class(interval);
    return (ic == 0 || ic == 5 || ic == 7 || interval == 12 || interval == 0);
    /* Unison, P4, P5, P8 */
}

int ce_is_imperfect_consonance(int interval)
{
    int ic = interval % 12;
    return (ic == 3 || ic == 4 || ic == 8 || ic == 9);
    /* m3, M3, m6, M6 */
}

int ce_is_consonance(int interval)
{
    return ce_is_perfect_consonance(interval) || ce_is_imperfect_consonance(interval);
}

int ce_is_dissonance(int interval)
{
    return !ce_is_consonance(interval);
}

static const char *interval_names[] = {
    "P1", "m2", "M2", "m3", "M3", "P4", "A4/d5", "P5",
    "m6", "M6", "m7", "M7", "P8"
};

const char *ce_interval_name(int semitones)
{
    if (semitones >= 0 && semitones <= 12)
        return interval_names[semitones];
    return "?";
}

/* ─── Pitch class utilities ────────────────────────────────── */

int ce_pitch_class(int midi) { return midi % 12; }
int ce_octave(int midi) { return midi / 12 - 1; }
int ce_midifrom_pc(int pc, int octave) { return pc + (octave + 1) * 12; }

/* ─── Scale / mode ─────────────────────────────────────────── */

void ce_scale_degrees(CeMode mode, int degrees[7])
{
    switch (mode) {
    case CE_MODE_MAJOR:
        degrees[0]=0; degrees[1]=2; degrees[2]=4; degrees[3]=5;
        degrees[4]=7; degrees[5]=9; degrees[6]=11;
        break;
    case CE_MODE_MINOR:
        degrees[0]=0; degrees[1]=2; degrees[2]=3; degrees[3]=5;
        degrees[4]=7; degrees[5]=8; degrees[6]=10;
        break;
    case CE_MODE_DORIAN:
        degrees[0]=0; degrees[1]=2; degrees[2]=3; degrees[3]=5;
        degrees[4]=7; degrees[5]=9; degrees[6]=10;
        break;
    case CE_MODE_MIXOLYDIAN:
        degrees[0]=0; degrees[1]=2; degrees[2]=4; degrees[3]=5;
        degrees[4]=7; degrees[5]=9; degrees[6]=10;
        break;
    default:
        ce_scale_degrees(CE_MODE_MAJOR, degrees);
    }
}

int ce_is_in_scale(int pc, int tonic, CeMode mode)
{
    int degrees[7];
    ce_scale_degrees(mode, degrees);
    for (int i = 0; i < 7; i++) {
        if (((tonic + degrees[i]) % 12) == (pc % 12))
            return 1;
    }
    return 0;
}

int ce_scale_degree(int pc, int tonic, CeMode mode)
{
    int degrees[7];
    ce_scale_degrees(mode, degrees);
    int target = (pc - tonic + 120) % 12;
    for (int i = 0; i < 7; i++) {
        if (degrees[i] == target)
            return i;
    }
    return -1;
}

/* ─── Voice leading ────────────────────────────────────────── */

CeMotionType ce_classify_motion(int v1_from, int v1_to,
                                 int v2_from, int v2_to)
{
    int d1 = v1_to - v1_from;
    int d2 = v2_to - v2_from;
    if (d1 == 0 || d2 == 0) return CE_MOTION_OBLIQUE;
    if ((d1 > 0 && d2 > 0) || (d1 < 0 && d2 < 0)) {
        return (d1 == d2) ? CE_MOTION_PARALLEL : CE_MOTION_SIMILAR;
    }
    return CE_MOTION_CONTRARY;
}

int ce_has_parallel_perfect(int v1_a, int v1_b,
                             int v2_a, int v2_b)
{
    int int1 = ce_interval(v1_a, v2_a);
    int int2 = ce_interval(v1_b, v2_b);
    if (int1 == int2 && ce_is_perfect_consonance(int1)) {
        /* Same perfect interval approached by similar or parallel motion */
        CeMotionType m = ce_classify_motion(v1_a, v1_b, v2_a, v2_b);
        if (m == CE_MOTION_PARALLEL || m == CE_MOTION_SIMILAR)
            return 1;
    }
    return 0;
}

int ce_has_hidden_perfect(int v1_from, int v1_to,
                           int v2_from, int v2_to)
{
    int interval = ce_interval(v1_to, v2_to);
    if (!ce_is_perfect_consonance(interval)) return 0;
    CeMotionType m = ce_classify_motion(v1_from, v1_to, v2_from, v2_to);
    if (m == CE_MOTION_SIMILAR) return 1;
    return 0;
}

/* ─── Melodic rules ────────────────────────────────────────── */

int ce_is_allowed_melodic_interval(int interval)
{
    int abs_iv = (interval >= 0) ? interval : -interval;
    /* Allow up to P8, forbid > P8 */
    if (abs_iv > 12) return 0;
    /* Forbid augmented 2nd, tritone as melodic intervals */
    int mod12 = abs_iv % 12;
    if (mod12 == 6) return 0;  /* tritone */
    if (abs_iv > 8 && abs_iv < 12 && mod12 != 0) return 0; /* large dissonant leaps */
    return 1;
}

int ce_has_tritone_outline(const int *pitches, int n)
{
    /* Check if any 3-note outline spans a tritone */
    for (int i = 0; i < n - 2; i++) {
        int span = pitches[i+2] - pitches[i];
        int abs_span = (span >= 0) ? span : -span;
        if (abs_span == 6) {
            /* Check that the middle note doesn't break the outline */
            int min = (pitches[i] < pitches[i+2]) ? pitches[i] : pitches[i+2];
            int max = (pitches[i] < pitches[i+2]) ? pitches[i+2] : pitches[i];
            if (pitches[i+1] >= min && pitches[i+1] <= max)
                return 1;
        }
    }
    return 0;
}

/* ─── Analysis ─────────────────────────────────────────────── */

int ce_analyze_intervals(const int *cf, const int *cp, int n,
                          int *intervals_out)
{
    int dissonant = 0;
    for (int i = 0; i < n; i++) {
        int iv = ce_interval(cf[i], cp[i]);
        if (intervals_out) intervals_out[i] = iv;
        if (ce_is_dissonance(iv)) dissonant++;
    }
    return dissonant;
}

CeErrorReport ce_analyze_errors(const int *cf, const int *cp, int n,
                                 const CeConstraints *c)
{
    CeErrorReport rpt;
    memset(&rpt, 0, sizeof(rpt));

    for (int i = 0; i < n; i++) {
        /* Dissonance check */
        int iv = ce_interval(cf[i], cp[i]);
        if (ce_is_dissonance(iv)) rpt.dissonances++;

        /* Range check */
        if (c && (cp[i] < c->min_pitch || cp[i] > c->max_pitch))
            rpt.range_violations++;

        /* Melodic leap check */
        if (i > 0) {
            int mel_iv = cp[i] - cp[i-1];
            if (!ce_is_allowed_melodic_interval(mel_iv))
                rpt.illegal_leaps++;

            /* Parallel/hidden perfect intervals */
            if (ce_has_parallel_perfect(cf[i-1], cf[i], cp[i-1], cp[i]))
                rpt.parallel_perfect++;
            if (ce_has_hidden_perfect(cf[i-1], cf[i], cp[i-1], cp[i]))
                rpt.hidden_perfect++;
        }
    }

    /* Tritone outline check */
    if (ce_has_tritone_outline(cp, n))
        rpt.tritone_outlines++;

    rpt.total_errors = rpt.parallel_perfect + rpt.hidden_perfect +
                       rpt.dissonances + rpt.illegal_leaps +
                       rpt.tritone_outlines + rpt.range_violations;
    return rpt;
}

/* ─── First-species counterpoint generation (backtracking) ── */

/* Candidate pitches for each CF note */
static int get_candidates(int cf_note, int prev_cp, int is_first,
                           int is_last, const CeConstraints *c,
                           int *candidates, int max_cand)
{
    int count = 0;
    int tonic = c ? c->tonic : 0;
    CeMode mode = c ? c->mode : CE_MODE_MAJOR;
    int lo = c ? c->min_pitch : cf_note + 4;
    int hi = c ? c->max_pitch : cf_note + 19;

    for (int p = lo; p <= hi && count < max_cand; p++) {
        /* Must be consonant with CF */
        int iv = ce_interval(cf_note, p);
        if (ce_is_dissonance(iv)) continue;

        /* Must be in scale */
        if (!ce_is_in_scale(p % 12, tonic, mode)) continue;

        /* First note: must be perfect consonance (unison, P5, P8) */
        if (is_first && !ce_is_perfect_consonance(iv)) continue;

        /* Last note: must resolve to tonic or P8 */
        if (is_last && (p % 12) != tonic) continue;

        /* Melodic rules (if not first) */
        if (!is_first) {
            int mel_iv = p - prev_cp;
            int abs_iv = (mel_iv >= 0) ? mel_iv : -mel_iv;
            if (abs_iv > 12) continue;  /* no leaps > P8 */
            if ((abs_iv % 12) == 6) continue; /* no tritone leaps */
        }

        candidates[count++] = p;
    }
    return count;
}

static int backtrack(const int *cf, int n, int pos, int prev_cp,
                      int *cp, const CeConstraints *c)
{
    if (pos == n) return 1;  /* success */

    int is_first = (pos == 0);
    int is_last  = (pos == n - 1);

    int candidates[64];
    int n_cand = get_candidates(cf[pos], prev_cp, is_first, is_last, c,
                                 candidates, 64);

    for (int i = 0; i < n_cand; i++) {
        cp[pos] = candidates[i];

        /* Check voice leading with previous note */
        if (pos > 0) {
            if (ce_has_parallel_perfect(cf[pos-1], cf[pos], cp[pos-1], cp[pos]))
                continue;
            if (ce_has_hidden_perfect(cf[pos-1], cf[pos], cp[pos-1], cp[pos]))
                continue;
        }

        /* Check tritone outline in last 3 notes */
        if (pos >= 2) {
            int segment[3] = {cp[pos-2], cp[pos-1], cp[pos]};
            if (ce_has_tritone_outline(segment, 3))
                continue;
        }

        if (backtrack(cf, n, pos + 1, cp[pos], cp, c))
            return 1;
    }

    return 0;  /* backtrack */
}

int ce_generate_first_species(const int *cf, int cf_length,
                                int *counterpoint_out,
                                const CeConstraints *constraints)
{
    if (cf_length <= 0 || cf_length > CE_MAX_NOTES) return 0;

    int result = backtrack(cf, cf_length, 0, 0, counterpoint_out, constraints);
    return result ? cf_length : 0;
}

/* ─── Laman rigidity (simplified 2D check) ─────────────────── */

int ce_is_laman_rigid(int n_vertices, const int *edges, int n_edges)
{
    if (n_vertices < 2) return 1;

    /* Necessary condition: |E| = 2|V| - 3 */
    int required = 2 * n_vertices - 3;
    if (n_edges < required) return 0;

    /* For a proper check we'd need the full Laman condition
       (generic rigidity matrix has rank 2n-3).
       Simplified: check edge count + min degree >= 2 for all vertices. */
    int min_degree = n_edges;  /* overestimate */
    int degrees[CE_MAX_NOTES] = {0};
    for (int i = 0; i < n_edges; i++) {
        int v1 = edges[2*i], v2 = edges[2*i + 1];
        if (v1 >= 0 && v1 < n_vertices) degrees[v1]++;
        if (v2 >= 0 && v2 < n_vertices) degrees[v2]++;
    }
    for (int i = 0; i < n_vertices; i++) {
        if (degrees[i] < min_degree) min_degree = degrees[i];
    }

    /* Every vertex in a rigid graph must have degree >= 2
       (except the trivial cases) */
    if (n_vertices > 2 && min_degree < 2) return 0;

    return 1;
}
