#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <math.h>
#include "arduino-serial/arduino-serial-lib.h"

#define DEBUG 0
#define X_MAX 56
#define Y_MAX 44

typedef struct xy {
  int x;
  int y;
} xy;

typedef struct xy_float {
  float x;
  float y;
} xy_float;

typedef struct ball {
  xy_float position;
  xy_float direction;
  float speed;
} ball;

typedef struct paddle {
  xy position;
  int width;
} paddle;

void setup_color ();
void create_border_window (int x, int y);
void create_game_window (int x, int y);

void center_windows();
void draw_game_window();

void draw_bricks ();
void draw_ball ();
void draw_paddle ();

void move_ball ();
float calc_ball_x ();
float calc_ball_y ();
void ball_speedup(int i);

void update_paddle_position (int fd);

WINDOW *border_window;
WINDOW *game_window;
xy screen_max = { 0, 0 };
ball b = { .position = { X_MAX / 2, Y_MAX / 2 }, .direction = { 1, 1 }, .speed = 0.1};
paddle p = { .position = { 0, Y_MAX - 5 }, .width = 4 };
bool bricks[14][8] = { [0 ... 13][0 ... 7] = true };
int destroyed_bricks = 0;
bool hit_third_row = false;
bool hit_fourth_row = false;
int points = 0;
int lives = 3;

float rel;

int main (void)
{
  initscr(); // creates strscr variable
  noecho();
  curs_set(0); // disable cursor blinking

  setup_color();

  int port = serialport_init("/dev/ttyACM0", 9600);

  while (1)
  {
    center_windows();
    draw_game_window(port);
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

void create_border_window (int x, int y)
{
  border_window = newwin(Y_MAX + 2, X_MAX + 2, y - 1, x - 1);
  box(border_window, 0, 0);
  wrefresh(border_window);
}

void create_game_window (int x, int y)
{
  game_window = newwin(Y_MAX, X_MAX, y, x);
  wattron(game_window, A_BOLD);
}

void center_windows ()
{
  int max_x, max_y;
  getmaxyx(stdscr, max_y, max_x);
  if (max_x != screen_max.x || max_y != screen_max.y)
  {
    screen_max.x = max_x;
    screen_max.y = max_y;
    int x = (screen_max.x - X_MAX) / 2;
    int y = (screen_max.y - Y_MAX) / 2;
    delwin(border_window);
    delwin(game_window);
    clear();
    refresh();
    create_border_window(x, y);
    create_game_window(x, y);
  }
}

void draw_game_window(int port)
{
  wclear(game_window);

  move_ball();
  update_paddle_position(port);

  draw_bricks();
  draw_ball();
  draw_paddle();

  mvwprintw(game_window, 0, 0, "points: %d", points);
  mvwprintw(game_window, 1, 0, "lives: %d", lives);

  if (DEBUG) {
    mvwprintw(game_window, 1, 0, "paddle ball rel: %f", rel);
  }

  wrefresh(game_window);
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
      if (bricks[col][row])
      {
        wattron(game_window, COLOR_PAIR(row_color));
        mvwprintw(game_window, 4 + row, 4 * col, "[__]");
        wattroff(game_window, COLOR_PAIR(row_color));
      }
    }
  }
}

void draw_ball ()
{
  mvwprintw(game_window, round(b.position.y), round(b.position.x), "o");
}

void draw_paddle ()
{
  mvwprintw(game_window, p.position.y, p.position.x, "====");
}

float angle; // TODO: why do I have to put these up here to make things not break?
int col;
int row;
void move_ball ()
{
  float x = calc_ball_x();
  float y = calc_ball_y();

  if (x > X_MAX - 1 || x < 0)
  {
    // collision with vertical walls
    b.direction.x *= -1;
    x = calc_ball_x();
  }

  if (y < 0)
  {
    // collision with top wall
    b.direction.y *= -1;
    y = calc_ball_y();
  }

  if (y > Y_MAX - 1)
  {
    // collision with bottom wall
    x = X_MAX / 2;
    y = Y_MAX / 2;
    if (--lives < 1)
    {
      b.speed = 0;
    }
  }

  if (x > p.position.x - 1 && x < p.position.x + p.width && ceil(y) == p.position.y)
  {
    // collision with paddle
    rel = (x - p.position.x + 1) / (p.width + 1);
    angle = -1 * M_PI * (2 + 18 * rel) / 20;
    b.direction.x = cosf(angle);
    b.direction.y = sinf(angle);
    x = calc_ball_x();
    y = calc_ball_y();
  }

  if (round(y) >= 4 && round(y) <= 12)
  {
    // potential collision with brick
    col = round(x) / 4;
    row = round(y) - 5;
    if (bricks[col][row])
    {
      // collision with brick
      bricks[col][row] = false;
      switch (row)
      {
        case 0:
        case 1:
          points += 7;
          ball_speedup(400);
          break;
        case 2:
        case 3:
          points += 5;
          ball_speedup(300);
          break;
        case 4:
        case 5:
          points += 3;
          break;
        default:
          points++;
      }
      ball_speedup(++destroyed_bricks);
      b.direction.y *= -1;
      y = calc_ball_y();
    }
  }

  b.position.x = x;
  b.position.y = y;
}

float calc_ball_x ()
{
  return b.position.x + b.direction.x * b.speed;
}

float calc_ball_y ()
{
  return b.position.y + b.direction.y * b.speed;
}

void ball_speedup (int i)
{
  switch (i)
  {
    case 4:
      b.speed = 0.2;
      break;
    case 12:
      b.speed = 0.3;
      break;
    case 300:
      if (!hit_third_row)
      {
        hit_third_row = true;
        b.speed = 0.4;
      }
      break;
    case 400:
      if (!hit_fourth_row)
      {
        hit_fourth_row = true;
        b.speed = 0.5;
      }
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
