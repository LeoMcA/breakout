#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "arduino-serial/arduino-serial-lib.h"

#define DEBUG 1
#define X_MAX 56
#define Y_MAX 44

typedef struct xy {
  int x;
  int y;
} xy;

typedef struct ball {
  xy position;
  xy direction;
} ball;

typedef struct paddle {
  xy position;
  int width;
} paddle;

void setup_color ();
WINDOW *setup_border ();
WINDOW *setup_game ();

void draw_bricks ();
void draw_ball ();
void draw_paddle ();

void move_ball ();

void update_paddle_position (int fd);

WINDOW *game_window;
ball b = { .position = { 0, 0 }, .direction = { 1, 1 }};
paddle p = { .position = { 0, Y_MAX - 5 }, .width = 4 };

int main (void)
{
  initscr();
  noecho();
  curs_set(0); // disable cursor blinking

  setup_color();
  setup_border();
  game_window = setup_game();

  int port = serialport_init("/dev/ttyACM0", 9600);

  while (1)
  {
    wclear(game_window);

    update_paddle_position(port);

    draw_paddle();
    draw_ball();

    move_ball();

    draw_bricks();

    wrefresh(game_window);
  }

  endwin();

  return 0;
}

void setup_color ()
{
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
}

WINDOW *setup_border ()
{
  WINDOW *game_border;
  game_border = newwin(Y_MAX + 2, X_MAX + 2, 0, 0);
  box(game_border, 0, 0);
  wrefresh(game_border);
  return game_border;
}

WINDOW *setup_game ()
{
  WINDOW *game_window;
  game_window = newwin(Y_MAX, X_MAX, 1, 1);
  wattron(game_window, A_BOLD);
  return game_window;
}

int row_color;
void draw_bricks ()
{
  for (int row = 0; row < 8; row++)
  {
    // int row_color; // initializing variable here causes everything to break
    row_color = (row + 2) / 2;
    for (int col = 0; col < 14; col++)
    {
      wattron(game_window, COLOR_PAIR(row_color));
      mvwprintw(game_window, 4 + row, 4 * col, "[__]");
      wattroff(game_window, COLOR_PAIR(row_color));
    }
  }
}

void draw_ball ()
{
  mvwprintw(game_window, b.position.y, b.position.x, "o");
}

void draw_paddle ()
{
  mvwprintw(game_window, p.position.y, p.position.x, "====");
}

void move_ball ()
{
  b.position.x += b.direction.x;
  b.position.y += b.direction.y;

  if (b.position.x >= X_MAX - 1 || b.position.x <= 0)
  {
    b.direction.x *= -1;
  }

  if (b.position.y >= Y_MAX - 1 || b.position.y <= 0)
  {
    b.direction.y *= -1;
  }
}

void update_paddle_position (int fd)
{
  int b[1];
  while (!b[0])
  {
    read(fd, b, 1);
  }
  int position = (b[0] - 1) * (X_MAX - p.width) / 254;
  if (DEBUG) mvwprintw(game_window, 0, 0, "paddle position: %d", position);
  p.position.x = position;
}
