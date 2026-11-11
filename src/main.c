#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// TODO add buttons to reset, sliders to adjust variables

typedef struct
{
  Vector2 position;
  Vector2 velocity;
  Color col;
} Boid;

bool gravityOn = true;
const int num_boids = 190;
const int pull = 1000;
const int separation = 10;
const int vel_scale = 2;
const double gravScale = 100;
const double velLimit = 10;
const float binding_strength = 10;
const float col_smooth = 500.0f;

Vector2 addV(Vector2 a, Vector2 b)
{
  Vector2 _v = {a.x + b.x, a.y + b.y};
  return _v;
}

double dotV(Vector2 a, Vector2 b) { return (double)(a.x * b.x + a.y * b.y); }

double lenV(Vector2 v) { return (sqrt(dotV(v, v))); }

Vector2 divV(Vector2 v, double s)
{
  Vector2 _v = {v.x / s, v.y / s};
  return _v;
}

Vector2 flipV(Vector2 v)
{
  Vector2 _v = {-v.x, -v.y};
  return _v;
}

Vector2 subV(Vector2 a, Vector2 b) { return addV(a, flipV(b)); }

Vector2 normaliseV(Vector2 v)
{
  double len = lenV(v);
  if (len == 0)
  {
    Vector2 zero = {0, 0};
    return zero;
  }
  Vector2 _v = divV(v, len);
  return _v;
}

Vector2 getNormalV(Vector2 v)
{
  Vector2 normV = normaliseV(v);
  Vector2 normal = {-normV.y, normV.x};
  return normal;
}

Vector2 multV(Vector2 v, double s)
{
  Vector2 _v = {v.x * s, v.y * s};
  return _v;
}

double distV(Vector2 a, Vector2 b) { return lenV(subV(a, b)); }

void renderBoid(Boid *b)
{
  const float size = 10.0f;
  Vector2 pos = b->position;
  Vector2 direction = normaliseV(b->velocity);
  if (direction.x == 0 && direction.y == 0)
  {
    direction = (Vector2){0, -1};
  }

  Vector2 normal = getNormalV(direction);
  Vector2 tip = addV(pos, multV(direction, size));
  Vector2 base = addV(pos, multV(direction, -size * 0.6f));
  Vector2 wing = multV(normal, size * 0.55f);
  Vector2 vert1 = tip;
  Vector2 vert2 = addV(base, flipV(wing));
  Vector2 vert3 = addV(base, wing);
  DrawTriangle(vert1, vert2, vert3, b->col);
}

void initBoids(Boid *boids)
{

  for (int i = 0; i < num_boids; i++)
  {
    Vector2 pos = {rand() % GetScreenWidth(), rand() % GetScreenHeight()};
    Vector2 vel = {rand() % 10 - 5, rand() % 10 - 5};
    boids[i] = (Boid){pos, vel, WHITE};
    ;
  }
}

void renderBoids(Boid *boids)
{
  for (int i = 0; i < num_boids; i++)
  {
    renderBoid(&boids[i]);
  }
}

Vector2 rule1(int j, Boid *boids)
{
  Vector2 pc_j = {0, 0};
  for (int i = 0; i < num_boids; i++)
  {
    if (i != j)
    {
      pc_j = addV(pc_j, boids[i].position);
    }
  }
  pc_j = divV(pc_j, (num_boids - 1));
  float hue = (360 * (distV(pc_j, boids[j].position) / col_smooth));
  boids[j].col = ColorFromHSV(hue, 1, 1);
  return divV(subV(pc_j, boids[j].position), pull);
}

Vector2 rule2(int j, Boid *boids)
{
  Vector2 c = {0, 0};
  for (int i = 0; i < num_boids; i++)
  {
    if (i != j)
    {
      if (distV(boids[i].position, boids[j].position) < separation)
      {
        c = subV(c, subV(boids[i].position, boids[j].position));
      }
    }
  }
  return c;
}

Vector2 rule3(int j, Boid *boids)
{
  Vector2 pv_j = {0, 0};
  for (int i = 0; i < num_boids; i++)
  {
    if (i != j)
    {
      pv_j = addV(pv_j, boids[i].velocity);
    }
  }
  pv_j = divV(pv_j, (num_boids - 1));
  return divV(subV(pv_j, boids[j].velocity), 8);
}
Vector2 mouseGravity(Boid b)
{
  bool mouseDown = IsMouseButtonDown(0);
  Vector2 mouse = GetMousePosition();
  Vector2 toMouse = subV(mouse, b.position);
  double r = lenV(toMouse);
  if (r == 0)
  {
    return (Vector2){0, 0};
  }
  return divV(multV(toMouse, gravScale * (mouseDown ? 10 : 1)), r * r);
}

Vector2 bound_boid(Boid b)
{
  Vector2 v = {0, 0};
  if (b.position.x <= 0)
  {
    v.x = binding_strength;
  }
  if (b.position.x > GetScreenWidth())
  {
    v.x = -binding_strength;
  }
  if (b.position.y <= 0)
  {
    v.y = binding_strength;
  }
  if (b.position.y > GetScreenHeight())
  {
    v.y = -binding_strength;
  }
  return v;
}

void moveBoids(Boid *boids)
{
  Vector2 v1, v2, v3, v4, v5;
  Vector2 zero = {0, 0};
  for (int i = 0; i < num_boids; i++)
  {
    v1 = rule1(i, boids);
    v2 = rule2(i, boids);
    v3 = rule3(i, boids);
    v4 = gravityOn ? mouseGravity(boids[i]) : zero;
    v5 = bound_boid(boids[i]);

    boids[i].velocity =
        addV(addV(addV(addV(addV(boids[i].velocity, v1), v2), v3), v4), v5);
    boids[i].position = addV(boids[i].position, boids[i].velocity);
  }
}

void limitVelocity(Boid *b)
{
  if (lenV(b->velocity) > velLimit)
  {
    b->velocity = multV(normaliseV(b->velocity), velLimit);
  }
}

void limitBoids(Boid *boids)
{ // limit boid speed
  for (int i = 0; i < num_boids; i++)
  {
    limitVelocity(&boids[i]);
  }
}

int main()
{

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(800, 600, "boids");
  SetTargetFPS(60); // Set target FPS (maximum)

  int HEIGHT = GetScreenHeight();
  int WIDTH = GetScreenWidth();
  float theta = 0;
  // allocate memory for boids array
  Boid *boids = (Boid *)malloc(sizeof(Boid[num_boids]));
  srand(time(NULL));
  initBoids(boids);
  SearchAndSetResourceDir("resources");
  int frame = 0;
  while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or
  // presses the Close button on the window
  {

    // drawing
    BeginDrawing();
    ClearBackground(BLACK);
    if (frame % 2 == 0)
    {
      moveBoids(boids);
      limitBoids(boids);
    }
    renderBoids(boids);
    EndDrawing();
    frame++;
  }

  // cleanup
  // unload our texture so it can be cleaned up

  // destroy the window and cleanup the OpenGL context
  CloseWindow();
  free(boids);
  return 0;
}
