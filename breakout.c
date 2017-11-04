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
  attron(A_BOLD);

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
    getmaxyx(stdscr, max.y, max.x);
    int paddle = getPaddlePosition(port, max.x, 4);

    clear();

    mvprintw(max.y - 1, paddle, "====");
    mvprintw(position.y, position.x, "o");

    for (int row = 0; row < 8; row++)
    {
      for (int col = 0; col < 14; col++)
      {
        int color = (row + 2) / 2;
        attron(COLOR_PAIR(color));
        mvprintw(row, 4 * col, "[__]");
        attroff(COLOR_PAIR(color));
      }
    }

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
