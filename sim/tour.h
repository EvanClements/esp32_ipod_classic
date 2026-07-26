/*
 * tour.h — The scripted "demo tour" of the UI.
 *
 * The tour is plain data: a list of (frame, key) events. The capture harness
 * plays it back at a fixed timestep so the recorded animation is byte-for-byte
 * reproducible on any machine, regardless of CI load. To change what the demo
 * shows, edit the table in tour.c — no capture code changes needed.
 */
#ifndef TOUR_H
#define TOUR_H

#include "ui.h"

typedef struct {
    int      frame;  /* frame index at which to fire this input */
    ui_key_t key;
} tour_event_t;

/* Returns the event table and its length, plus the total frame count to
 * render (enough to let the final animation settle). */
const tour_event_t *tour_events(int *count);
int tour_total_frames(void);

#endif /* TOUR_H */
