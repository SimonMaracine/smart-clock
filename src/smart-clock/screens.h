#ifndef SCREENS_H
#define SCREENS_H

typedef void(*screen_mode_func)(bool repaint);

void change_screen(screen_mode_func screen);

extern screen_mode_func current_screen;

#endif  // SCREENS_H
