#include <math.h>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

#define PI 3.14159265
#define CWIDTH 8
#define CHEIGHT 8
#define CSIZE 64
#define SCREENWIDTH 1024 
#define SCREENHEIGHT 512

// g++ -c main.cpp
// g++ main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system
// ./sfml-app

int worldMap[CWIDTH][CHEIGHT] = {
  {1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,1},
  {1,0,1,0,0,1,0,1},
  {1,0,0,1,1,0,0,0},
  {1,0,1,0,0,0,0,0},
  {1,1,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1},
};
void renderMap(sf::RenderWindow* window) {
  std::vector<sf::RectangleShape> rectangles;
  int x, y, xw, yh;
  for (x = 0; x < 8; ++x) {
    for (y = 0; y < 8; ++y) {
      xw = x * CSIZE;
      yh = y * CSIZE;

      sf::RectangleShape rect(sf::Vector2f(63, 63));

      if (worldMap[y][x] == 0) {
        rect.setFillColor(sf::Color(0, 0, 0));
      } else {
        rect.setFillColor(sf::Color(255, 255, 255));
      }

      rect.setPosition(sf::Vector2f(xw, yh));

      window->draw(rect);
    }
  }
}

struct Player {
  float pa;  // angle

  sf::Vector2f position;
  sf::Vector2f dir;
  sf::Vector2f plane;

  float speed;

  Player() : position{2.f, 2.f}, dir{-1.f, 0.f}, plane{0.f, 1.f}, speed(10.f), pa(0.0f) {}
};

void updatePlayer(Player* player) {
  float rotSpeed = 0.003;

  // Rotation matrix based on Lode Vandevenne's implementation
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
    float oldDirX = player->dir.x;

    player->dir.x = player->dir.x * cos(-rotSpeed) - player->dir.y * sin(-rotSpeed);
    player->dir.y = oldDirX * sin(-rotSpeed) + player->dir.y * cos(-rotSpeed);

    float oldPlaneX = player->plane.x;

    player->plane.x = player->plane.x * cos(-rotSpeed) - player->plane.y * sin(-rotSpeed);
    player->plane.y = oldPlaneX * sin(-rotSpeed) + player->plane.y * cos(-rotSpeed);
  }
  // Rotation matrix based on Lode Vandevenne's implementation
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
    float oldDirX = player->dir.x;

    player->dir.x = player->dir.x * cos(rotSpeed) - player->dir.y * sin(rotSpeed);
    player->dir.y = oldDirX * sin(rotSpeed) + player->dir.y * cos(rotSpeed);

    float oldPlaneX = player->plane.x;

    player->plane.x = player->plane.x * cos(rotSpeed) - player->plane.y * sin(rotSpeed);
    player->plane.y = oldPlaneX * sin(rotSpeed) + player->plane.y * cos(rotSpeed);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
    player->position.x -= player->dir.x * player->speed * 0.016f;
    player->position.y -= player->dir.y * player->speed * 0.016f;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
    player->position.x += player->dir.x * player->speed * 0.016f;
    player->position.y += player->dir.y * player->speed * 0.016f;
  }
}
void renderPlayer(sf::RenderWindow* window, Player* p, float length) {
  sf::CircleShape shape(8);
  shape.setFillColor(sf::Color(0, 255, 0));
  shape.setPosition(sf::Vector2f(p->position.x - 8, p->position.y - 8));

  sf::Vertex centerView[] = {sf::Vertex(p->position, sf::Color::Red), sf::Vertex(p->position + p->dir * length, sf::Color::Red)};

  sf::Vertex fov[] = {
      sf::Vertex(p->position + p->dir * length - p->plane * length, sf::Color::Blue),  // pos vector
      sf::Vertex(p->position + p->dir * length + p->plane * length, sf::Color::Blue)   // pos vector + dir vector
  };

  window->draw(centerView, 2, sf::Lines);  // drawing direction vector

  window->draw(fov, 2, sf::Lines);

  window->draw(shape);
}

bool checkWall(sf::Vector2f coords) {
  int x = trunc(coords.x / CSIZE);
  int y = trunc(coords.y / CSIZE);
  if (x < 8 && y < 8) {
    return (worldMap[y][x] == 1 ? true : false);
  }
  return false;
}

float mag(sf::Vector2f vec) { return (sqrt(vec.x * vec.x + vec.y * vec.y)); }

void renderRays(sf::RenderWindow* window, Player* p, int rayNum, int depth = 8) {
  float cameraX;

  int unit = CWIDTH * CHEIGHT;  // 64

  sf::Vector2f deltaDist = {1, 1};
  sf::Vector2f rayDir = {1, 1};
  sf::Vector2f step;       // direction of step to take
  sf::Vector2f rayLength;  // distance to closest box

  float perpWallDist;  // distance from camera plane to a wall
  int side;
  int mapX, mapY;

  for (int i = 0; i < rayNum; i++) {
    mapX = trunc(p->position.x / CSIZE);  // which grid on map
    mapY = trunc(p->position.y / CSIZE);  // which grid on map

    cameraX = 2 * i / (float)rayNum - 1;  // x coords on camera vec

    rayDir.x = p->dir.x + p->plane.x * cameraX;
    rayDir.y = p->dir.y + p->plane.y * cameraX;

    deltaDist = {sqrt(1 + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)), sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y))};

    if (rayDir.x < 0) {
      step.x = -1;
      rayLength.x = (p->position.x / CSIZE - float(mapX)) * deltaDist.x;
    } else {
      step.x = 1;
      rayLength.x = (float(mapX + 1.0) - p->position.x / CSIZE) * deltaDist.x;
    }

    if (rayDir.y < 0) {
      step.y = -1;
      rayLength.y = (p->position.y / CSIZE - float(mapY)) * deltaDist.y;
    } else {
      step.y = 1;
      rayLength.y = (float(mapY + 1.0) - p->position.y / CSIZE) * deltaDist.y;
    }

    int hit = 0;
    float maxDistance = 20;
    float distance = 0;

    while (hit == 0 && distance < maxDistance) {
      // Walk
      if (rayLength.x < rayLength.y) {
        rayLength.x += deltaDist.x;
        mapX += step.x;
        distance = rayLength.x;
        side = 0;
      } else {
        rayLength.y += deltaDist.y;
        mapY += step.y;
        distance = rayLength.y;
        side = 1;
      }

      // check if a ray hit a wall

      if (worldMap[mapY][mapX] == 1) {
        hit = 1;
      }
    }

    if (hit == 1) {
      sf::Vector2f collision = p->position + (rayDir * distance * 32.f);

      // sf::Vector2f collision = p->position + (rayDir * distance);

      if (side == 0)
        perpWallDist = (rayLength.x - deltaDist.x);
      else
        perpWallDist = (rayLength.y - deltaDist.y);

      float lineHeight = (float)SCREENHEIGHT / perpWallDist;

      // std::cout<<collision.x << ", " << collision.y <<"\n";
      // std::cout<<mapX << ", " << mapY <<"\n";
      // std::cout<<distance << "\n";
      sf::Vertex ray[] = {sf::Vertex(p->position, sf::Color::Red), sf::Vertex(collision, sf::Color::Red)};
      sf::CircleShape hit(5);
      hit.setOutlineThickness(1);
      hit.setOutlineColor(sf::Color::Red);
      hit.setPosition(collision);

      sf::Color color;

      if (side == 1) {
        color = sf::Color::Blue;
      } else {
        color = sf::Color::Red;
      }

      sf::RectangleShape rect;
      rect.setSize(sf::Vector2f(10, lineHeight));

      rect.setFillColor(color);
      rect.setPosition(sf::Vector2f(512 + i * 6, CSIZE));
      

      window->draw(rect);
      window->draw(hit);
      window->draw(ray, 2, sf::Lines);
    }
  }
}

void editMap(sf::RenderWindow* window, sf::Vector2i coords) {
    int x = trunc(coords.x / CSIZE);
    int y = trunc(coords.y / CSIZE);
    // std::cout<< x << " " << y;
    if(worldMap[y][x] == 1) { worldMap[y][x] = 0;}
    else {worldMap[y][x] = 1;}
    
}

int main() {
  Player player;

  player.dir.x = cos(player.pa);
  player.dir.y = sin(player.pa);

  sf::RenderWindow window(sf::VideoMode(SCREENWIDTH, SCREENHEIGHT), "window");
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) window.close();

      if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2i position = sf::Mouse::getPosition(window);
        
        editMap(&window, position);
      }
    }

    
    window.clear(sf::Color(128, 128, 128));

    updatePlayer(&player);

    renderMap(&window);

    renderPlayer(&window, &player, 10);

    renderRays(&window, &player, 150);

    window.display();
  }

  return 0;
}