#include <ncurses.h>
#include <unistd.h>

typedef struct xy {
  int x;
  int y;
} xy;

int main (void) {
  initscr();
  noecho();
  curs_set(0);

  xy position = { 0, 0 };
  xy direction = { 1, 1 };
  xy max;

  while (1)
  {
    getmaxyx(stdscr, max.y, max.x);

    clear();
    mvprintw(position.y, position.x, "o");
    refresh();

    position.x += direction.x;
    position.y += direction.y;

    if (position.x >= max.x - 1 || position.x <= 0)
    {
      direction.x *= -1;
    }

    if (position.y >= max.y - 1 || position.y <= 0)
    {
      direction.y *= -1;
    }

    usleep(100000);
    // sleep(1);
  }

  endwin();

  return 0;
}
