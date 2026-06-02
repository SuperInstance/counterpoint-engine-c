#ifndef COUNTERPOINT_ENGINE_H
#define COUNTERPOINT_ENGINE_H

#include <math.h>
#include <string.h>
#include <stdint.h>

/* ── Interval classification ────────────────────────────────────────── */

/** A musical interval measured in semitones */
typedef struct {
    int semitones;
} Interval;

/**
 * Compute the interval between two MIDI note numbers.
 * @param a  First MIDI note
 * @param b  Second MIDI note
 * @return   Interval (absolute semitone distance mod 12)
 */
Interval interval_between(int a, int b);

/**
 * Check if an interval (in semitones) is a perfect consonance (P1, P5, P8).
 */
int interval_is_perfect_consonance(int semitones);

/**
 * Check if an interval is an imperfect consonance (m3, M3, m6, M6).
 */
int interval_is_imperfect_consonance(int semitones);

/**
 * Check if an interval is any consonance (perfect or imperfect).
 */
int interval_is_consonance(int semitones);

/**
 * Check if an interval is a dissonance (not consonant).
 */
int interval_is_dissonance(int semitones);

/**
 * Get the interval name as a string (e.g., "P5", "m3", "TT").
 * @param semitones  Interval in semitones (0-11)
 * @return           Static string name
 */
const char* interval_name(int semitones);

/* ── Note ────────────────────────────────────────────────────────────── */

/**
 * Convert a MIDI note number to frequency in Hz.
 * A4 (MIDI 69) = 440 Hz, using equal temperament.
 * @param midi  MIDI note number
 * @return      Frequency in Hz
 */
double note_frequency(int midi);

/**
 * Compute the interval between two MIDI notes.
 * @param a  First MIDI note
 * @param b  Second MIDI note
 * @return   Interval structure
 */
Interval note_interval_to(int a, int b);

/* ── Voice pair (cantus firmus + counterpoint) ──────────────────────── */

/** A pair of voices for counterpoint analysis */
typedef struct {
    int *cf;      /**< Cantus firmus (pointer to MIDI note array) */
    int *cp;      /**< Counterpoint voice (pointer to MIDI note array) */
    int length;   /**< Number of notes in each voice */
} VoicePair;

/**
 * Create a voice pair from two arrays of MIDI notes.
 * Does not copy data; pointers must remain valid.
 */
VoicePair voice_pair_new(int *cf, int *cp, int length);

/**
 * Count consonant intervals in a voice pair.
 */
int voice_pair_consonance_count(VoicePair *vp);

/**
 * Count dissonant intervals in a voice pair.
 */
int voice_pair_dissonance_count(VoicePair *vp);

/* ── Violations ──────────────────────────────────────────────────────── */

/** Types of counterpoint rule violations */
typedef enum {
    VIOLATION_NONE = 0,
    VIOLATION_DISSONANCE,        /**< Dissonant interval */
    VIOLATION_PARALLEL_FIFTHS,   /**< Consecutive perfect fifths */
    VIOLATION_PARALLEL_OCTAVES,  /**< Consecutive octaves/unisons */
    VIOLATION_LARGE_LEAP,        /**< Leap exceeds max_leap in counterpoint */
    VIOLATION_VOICE_CROSSING,    /**< CP below CF */
    VIOLATION_PARALLEL_UNIONS,   /**< Repeated unisons */
} ViolationType;

/** A single counterpoint violation */
typedef struct {
    ViolationType type;  /**< Type of violation */
    int index;           /**< Position in the voice pair */
    int detail;          /**< Additional info (semitones, leap size, etc.) */
} Violation;

/* ── Check result ────────────────────────────────────────────────────── */

#define MAX_VIOLATIONS 64

/** Result of checking a voice pair for counterpoint violations */
typedef struct {
    Violation violations[MAX_VIOLATIONS];  /**< Found violations */
    int violation_count;                    /**< Number of violations */
    double score;                           /**< Quality score (0-1) */
} CheckResult;

/**
 * Check if a CheckResult has no violations.
 */
int check_result_is_valid(CheckResult *r);

/* ── Counterpoint checker ───────────────────────────────────────────── */

/** Configuration for the counterpoint rule checker */
typedef struct {
    int allow_imperfect;  /**< Whether imperfect consonances are allowed */
    int max_leap;         /**< Maximum allowed leap in semitones */
} CounterpointChecker;

/**
 * Create a checker with default settings.
 * allow_imperfect = 1, max_leap = 8
 */
CounterpointChecker checker_default(void);

/**
 * Check a voice pair for counterpoint violations.
 * @param c   Checker configuration
 * @param vp  Voice pair to check
 * @return    Check result with violations and score
 */
CheckResult checker_check(CounterpointChecker *c, VoicePair *vp);

/**
 * Compute a quality score for a voice pair.
 * Score = consonance ratio - penalty for violations.
 * @param c   Checker configuration
 * @param vp  Voice pair to score
 * @return    Score (0.0 to 1.0)
 */
double checker_score(CounterpointChecker *c, VoicePair *vp);

/* ── Additional utility ─────────────────────────────────────────────── */

/**
 * Count the number of violations of a specific type.
 * @param r     Check result
 * @param type  Violation type to count
 * @return      Number of violations of that type
 */
int count_violations_by_type(CheckResult *r, ViolationType type);

/**
 * Check if a melody has a specific interval pattern.
 * @param melody  Array of MIDI notes
 * @param n       Length
 * @param interval Target interval in semitones
 * @return        1 if pattern exists, 0 otherwise
 */
int has_interval_pattern(int *melody, int n, int interval);

/**
 * Compute the range of a melody (max - min).
 * @param melody  Array of MIDI notes
 * @param n       Length
 * @return        Range in semitones
 */
int melody_range(int *melody, int n);

/**
 * Get the MIDI note name as a string (e.g., "C4", "A#4").
 * Uses sharps for accidentals.
 * @param midi  MIDI note number
 * @return      Static string note name
 */
const char* note_name(int midi);

/**
 * Run a counterpoint checking benchmark.
 * @param iterations  Number of iterations
 * @return            Elapsed time in seconds
 */
double benchmark_counterpoint(int iterations);

#endif
