#ifndef COUNTERPOINT_ENGINE_H
#define COUNTERPOINT_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Constants ────────────────────────────────────────────── */

#define CE_MAX_VOICES      8
#define CE_MAX_NOTES       1024
#define CE_MAX_SPECIES     5
#define CE_SCALE_SIZE      12

/* Interval constants (semitones) */
#define CE_UNISON   0
#define CE_m2       1
#define CE_M2       2
#define CE_m3       3
#define CE_M3       4
#define CE_P4       5
#define CE_TRITONE  6
#define CE_P5       7
#define CE_m6       8
#define CE_M6       9
#define CE_m7       10
#define CE_M7       11
#define CE_P8       12

/* ─── Note representation ──────────────────────────────────── */

typedef struct {
    int pitch;      /* MIDI pitch (0-127) or pitch class */
    int duration;   /* in some time unit */
    int voice;      /* voice index */
} CeNote;

/* ─── Interval math ────────────────────────────────────────── */

/* Compute interval between two pitches (in semitones, always positive) */
int  ce_interval(int pitch_a, int pitch_b);

/* Interval class (0-6) */
int  ce_interval_class(int interval);

/* Check if interval is a consonance */
int  ce_is_perfect_consonance(int interval);
int  ce_is_imperfect_consonance(int interval);
int  ce_is_consonance(int interval);
int  ce_is_dissonance(int interval);

/* Interval name */
const char *ce_interval_name(int semitones);

/* ─── Pitch class utilities ────────────────────────────────── */

int  ce_pitch_class(int midi_pitch);
int  ce_octave(int midi_pitch);
int  ce_midifrom_pc(int pc, int octave);  /* octave 0-10 */

/* ─── Scale / mode ─────────────────────────────────────────── */

typedef enum {
    CE_MODE_MAJOR = 0,
    CE_MODE_MINOR = 1,
    CE_MODE_DORIAN = 2,
    CE_MODE_MIXOLYDIAN = 3
} CeMode;

/* Scale degrees for given mode (returns 7 pitch class offsets from tonic) */
void ce_scale_degrees(CeMode mode, int degrees[7]);

/* Check if a pitch class belongs to a scale */
int  ce_is_in_scale(int pc, int tonic, CeMode mode);

/* Get the scale degree (0-6) of a pitch class in a key, or -1 */
int  ce_scale_degree(int pc, int tonic, CeMode mode);

/* ─── Species counterpoint rules ───────────────────────────── */

typedef enum {
    CE_SPECIES_1 = 1,  /* Note against note */
    CE_SPECIES_2 = 2,  /* Two notes against one */
    CE_SPECIES_3 = 3,  /* Four notes against one */
    CE_SPECIES_4 = 4,  /* Syncopated / suspended */
    CE_SPECIES_5 = 5,  /* Florid */
} CeSpecies;

typedef struct {
    int     tonic;         /* pitch class of tonic */
    CeMode  mode;
    int     cantus_firmus[CE_MAX_NOTES];
    int     cf_length;
    int     species;
    int     min_pitch;     /* range constraints */
    int     max_pitch;
} CeConstraints;

/* Rule checking result */
typedef struct {
    int    violation;    /* 1 if rule violated, 0 if OK */
    int    rule_id;      /* which rule */
    int    position;     /* note index where violation occurs */
    char   message[128];
} CeRuleResult;

/* ─── Voice leading ────────────────────────────────────────── */

/* Motion types between two voices */
typedef enum {
    CE_MOTION_PARALLEL = 0,
    CE_MOTION_SIMILAR  = 1,
    CE_MOTION_CONTRARY = 2,
    CE_MOTION_OBLIQUE  = 3,
} CeMotionType;

CeMotionType ce_classify_motion(int v1_from, int v1_to,
                                 int v2_from, int v2_to);

/* Check for parallel fifths/octaves between successive sonorities */
int  ce_has_parallel_perfect(int v1_a, int v1_b,
                              int v2_a, int v2_b);

/* Check for direct/hidden fifths (similar motion into perfect consonance) */
int  ce_has_hidden_perfect(int v1_from, int v1_to,
                            int v2_from, int v2_to);

/* ─── Consonance table ─────────────────────────────────────── */

/* Complete consonance table for all intervals 0-24 */
extern const int CE_CONSONANCE_TABLE[25];  /* 1=perfect, 2=imperfect, 0=dissonant */

/* ─── Melodic rules ────────────────────────────────────────── */

/* Maximum allowed melodic interval (in semitones) */
int  ce_is_allowed_melodic_interval(int interval);

/* Check for forbidden melodic outlines (e.g., tritone outline) */
int  ce_has_tritone_outline(const int *pitches, int n);

/* ─── Analysis ─────────────────────────────────────────────── */

/* Analyze intervals between cantus firmus and counterpoint.
   Returns number of dissonant intervals found. */
int  ce_analyze_intervals(const int *cf, const int *cp, int n,
                           int *intervals_out);

/* Count voice-leading errors in a counterpoint */
typedef struct {
    int parallel_perfect;
    int hidden_perfect;
    int dissonances;
    int illegal_leaps;
    int tritone_outlines;
    int range_violations;
    int total_errors;
} CeErrorReport;

CeErrorReport ce_analyze_errors(const int *cf, const int *cp, int n,
                                 const CeConstraints *constraints);

/* ─── Counterpoint generation (backtracking) ───────────────── */

/* Generate a first-species counterpoint above a cantus firmus.
   Returns the number of notes in the generated counterpoint,
   or 0 if no solution found. */
int  ce_generate_first_species(const int *cf, int cf_length,
                                int *counterpoint_out,
                                const CeConstraints *constraints);

/* ─── Laman rigidity check (simplified) ────────────────────── */

/* Check if a graph with n vertices and given edges is Laman rigid.
   edges is packed as [v1, v2, v3, v4, ...] (pairs).
   n_edges is the number of edges (not the array length).
   Uses the 2n-3 edge count + Henneberg construction (simplified). */
int  ce_is_laman_rigid(int n_vertices, const int *edges, int n_edges);

#ifdef __cplusplus
}
#endif

#endif /* COUNTERPOINT_ENGINE_H */
