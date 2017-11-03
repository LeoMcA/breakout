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

  xy position = { 0, 0 };
  xy direction = { 1, 1 };
  xy max;

  int port = serialport_init("/dev/ttyACM0", 9600);

  while (1)
  {
    getmaxyx(stdscr, max.y, max.x);
    int paddle = getPaddlePosition(port, max.x, 5);

    clear();

    mvprintw(max.y - 1, paddle, "xxxxx");
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
