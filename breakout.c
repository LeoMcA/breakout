#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "arduino-serial/arduino-serial-lib.h"

typedef struct xy {
  int x;
  int y;
} xy;

int getPaddlePosition (int fd, int screenWidth, int paddleWidth);

int main (void)
{
  initscr();
  noecho();
  curs_set(0);

  WINDOW *game_border;
  game_border = newwin(46, 58, 0, 0);
  box(game_border, 0, 0);
  wrefresh(game_border);

  WINDOW *game_window;
  game_window = newwin(44, 56, 1, 1);
  wattron(game_window, A_BOLD);

  xy position = { 0, 0 };
  xy direction = { 1, 1 };
  xy max;

  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);

  int port = serialport_init("/dev/ttyACM0", 9600);

  while (1)
  {
    getmaxyx(game_window, max.y, max.x);
    int paddle = getPaddlePosition(port, max.x, 4);

    wclear(game_window);

    mvwprintw(game_window, max.y - 5, paddle, "====");
    mvwprintw(game_window, position.y, position.x, "o");

    for (int row = 0; row < 8; row++)
    {
      for (int col = 0; col < 14; col++)
      {
        int color = (row + 2) / 2;
        wattron(game_window, COLOR_PAIR(color));
        mvwprintw(game_window, 4 + row, 4 * col, "[__]");
        wattroff(game_window, COLOR_PAIR(color));
      }
    }

    wrefresh(game_window);

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
  }

  endwin();

  return 0;
}

int getPaddlePosition (int fd, int screenWidth, int paddleWidth)
{
  int b[1];
  while (!b[0])
  {
    read(fd, b, 1);
  }
  return (b[0] - 1) * (screenWidth - paddleWidth) / 254;
}
