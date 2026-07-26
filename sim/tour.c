#include "tour.h"

/*
 * At ~30 fps (33 ms/frame) this is a ~7 second loop:
 *   - scroll down the main menu
 *   - open Music
 *   - scroll the song list
 *   - play a track and watch Now Playing progress
 *   - back out to the menu
 */
static const tour_event_t EVENTS[] = {
    {  15, UI_KEY_NEXT   },  /* Music -> Photos */
    {  25, UI_KEY_NEXT   },  /* -> Videos */
    {  35, UI_KEY_NEXT   },  /* -> Extras */
    {  45, UI_KEY_PREV   },  /* back up ... */
    {  55, UI_KEY_PREV   },
    {  65, UI_KEY_PREV   },  /* back to Music */
    {  78, UI_KEY_SELECT },  /* open Songs */
    {  92, UI_KEY_NEXT   },
    { 102, UI_KEY_NEXT   },
    { 112, UI_KEY_NEXT   },  /* land on "Reptilia" */
    { 128, UI_KEY_SELECT },  /* play -> Now Playing */
    { 250, UI_KEY_MENU   },  /* back to Songs */
    { 265, UI_KEY_MENU   },  /* back to Menu (via songs' MENU handler is Menu) */
};

const tour_event_t *tour_events(int *count)
{
    *count = (int)(sizeof(EVENTS) / sizeof(EVENTS[0]));
    return EVENTS;
}

int tour_total_frames(void)
{
    return 285;
}
