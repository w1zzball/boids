#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include <math.h>
#include <stdio.h>

typedef struct {
  Vector2 position;
  Vector2 velocity;
} Boid;

Vector2 addV(Vector2 a, Vector2 b) {
  Vector2 _v = {a.x + b.x, a.y + b.y};
  return _v;
}

double dotV(Vector2 a, Vector2 b) { return (double)(a.x * b.x + a.y * b.y); }

double lenV(Vector2 v) { return (sqrt(dotV(v, v))); }

Vector2 divV(Vector2 v, double s) {
  Vector2 _v = {v.x / s, v.y / s};
  return _v;
}

Vector2 flipV(Vector2 v) {
  Vector2 _v = {-v.x, -v.y};
  return _v;
}

Vector2 normaliseV(Vector2 v) {
  double len = lenV(v);
  if (len == 0) {
    Vector2 zero = {0, 0};
    return zero;
  }
  Vector2 _v = divV(v, len);
  return _v;
}

Vector2 getNormalV(Vector2 v) {
  Vector2 normV = normaliseV(v);
  Vector2 normal = {-normV.y, normV.x};
  return normal;
}

Vector2 multV(Vector2 v, double s) {
  Vector2 _v = {v.x * s, v.y * s};
  return _v;
}

void renderBoid(Boid *b) {
  const float size = 10.0f;
  Vector2 pos = b->position;
  Vector2 direction = normaliseV(b->velocity);
  if (direction.x == 0 && direction.y == 0) {
    direction = (Vector2){0, -1};
  }

  Vector2 normal = getNormalV(direction);
  Vector2 tip = addV(pos, multV(direction, size));
  Vector2 base = addV(pos, multV(direction, -size * 0.6f));
  Vector2 wing = multV(normal, size * 0.55f);
  Vector2 vert1 = tip;
  Vector2 vert2 = addV(base, flipV(wing));
  Vector2 vert3 = addV(base, wing);
  DrawTriangle(vert1, vert2, vert3, WHITE);
}
const int num_boids = 10;

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(800, 600, "boids");
  int HEIGHT = GetScreenHeight();
  int WIDTH = GetScreenWidth();
  float theta = 0;
  SearchAndSetResourceDir("resources");
  while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or
                               // presses the Close button on the window
  {
    // drawing
    BeginDrawing();
    ClearBackground(BLACK);
    Vector2 mouse = GetMousePosition();

    float u_x = sin(theta);
    float u_y = cos(theta);
    Vector2 v = {u_x, u_y};
    Vector2 p = {WIDTH / 2, HEIGHT / 2};
    Boid b = {mouse, v};
    renderBoid(&b);
    theta += 0.01f;
    EndDrawing();
  }

  // cleanup
  // unload our texture so it can be cleaned up

  // destroy the window and cleanup the OpenGL context
  CloseWindow();
  return 0;
}
