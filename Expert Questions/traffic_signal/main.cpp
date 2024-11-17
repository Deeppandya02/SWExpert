#include <stdio.h>
#include <fstream>

static unsigned long long seed = 5;

static int pseudo_rand(void)
{
    seed = seed * 25214903917ULL + 11ULL;
    return (seed >> 16) & 0x3fffffff;
}

static const long long PENALTY = 10'000'000'000'000LL;
static const int MAX_TC = 1;
static const int MAXN = 1000;
static const int MAXM = 1000;
static const int ROAD_CNT = 100;

static const int UP = 0;
static const int RIGHT = 1;
static const int DOWN = 2;
static const int LEFT = 3;

static const int TURN_LEFT = 3;
static const int TURN_RIGHT = 1;
static const int GO_STRAIGHT = 0;

static const int UPLEFT = 1;
static const int RIGHTUP = 2;
static const int DOWNRIGHT = 3;
static const int LEFTDOWN = 4;
static const int VERTICAL = 5;
static const int HORIZONTAL = 6;

struct Coordinates
{
    int y, x;
    Coordinates()
    {
        y = x = 0;
    }
    Coordinates(int y_, int x_)
    {
        y = y_;
        x = x_;
    }
};

struct TrafficSignal
{
    int signal, next_signal;
    TrafficSignal()
    {
        signal = next_signal = 0;
    }
};

struct Vehicle
{
    int x, y, dir, dest_y, dest_x;
};

static Coordinates src[ROAD_CNT*2], dest[ROAD_CNT*2];
static int map[MAXN][MAXM], crossroad_id[MAXN][MAXM];
static TrafficSignal signal_list[ROAD_CNT*ROAD_CNT];
static Vehicle vehicles[MAXN];
static int vehicle_on_map[MAXN][MAXM];
static int is_movable[MAXN];
static int turn_dir[MAXN];
static int K, L;

static int map_bak[MAXN][MAXM], crossroad_id_bak[MAXN][MAXM];
static Vehicle vehicles_bak[MAXN];

int dy[4] = {-1, 0, 1, 0};
int dx[4] = {0, 1, 0, -1};

static int min_x, min_y, max_x, max_y;

void change_signal(int cross_id, int next_signal)
{
    if (next_signal <= 0 || next_signal > 6) return;
    signal_list[cross_id].signal = 0;
    signal_list[cross_id].next_signal = next_signal;
}

static void make_tc() {
    K = 0, L = 0;
    min_y = MAXN-1, max_y = 0;
    min_x = MAXM-1, max_x = 0;
    for (int y = 0; y < MAXM; y++)
    {
        for (int x = 0; x < MAXM; x++)
        {
            crossroad_id[y][x] = 0;
            map[y][x] = 0;
        }
    }

    for (int i = 0; i < ROAD_CNT; i++)
    {
        int dir = pseudo_rand() % 2;
        if (dir == 0)
        {
            int y = pseudo_rand() % (MAXM - 200) + 100;
            if(map[y-1][0] == 1 || map[y][0] == 1 || map[y+1][0] == 1 || map[y+2][0] == 1) continue;

            for (int x = 0; x < MAXM; x++)
            {
                map[y][x] = map[y + 1][x] = 1;
            }
            src[K] = Coordinates(y + 1, 0);
            dest[K++] = Coordinates(y, 0);
            src[K] = Coordinates(y, MAXM - 1);
            dest[K++] = Coordinates(y + 1, MAXM - 1);
            if (min_y > y) min_y = y;
            if (max_y < y) max_y = y;
        }
        // Similar for vertical roads
        else
        {
            int x = pseudo_rand() % (MAXM - 200) + 100;
            if(map[0][x-1] == 1 || map[0][x] == 1 || map[0][x+1] == 1 || map[0][x+2] == 1)
                continue;
            for (int y = 0; y < MAXM; y++)
            {
                map[y][x] = map[y][x + 1] = 1;
            }
            src[K] = Coordinates(0, x + 1);
            dest[K++] = Coordinates(0, x );
            src[K] = Coordinates(MAXM - 1, x);
            dest[K++] = Coordinates(MAXM - 1, x + 1);
            if (min_x > x) min_x = x;
            if (max_x < x) max_x = x;
        }
    }

    L = 0;
    for (int y = 100; y < MAXM - 100; y++)
    {
        if (map[y - 1][0] == 0 && map[y][0] == 1)
        {
            for (int x = 100; x < MAXM-100; x++)
            {
                if (map[y-1][x] == 1 && map[y-1][x-1] == 0)
                {
                    L++;
                    crossroad_id[y][x] = crossroad_id[y+1][x] = crossroad_id[y][x+1] = crossroad_id[y+1][x+1] = L; 
                    signal_list[L].signal = 1;
                    signal_list[L].next_signal = 1;
                }
            }
        }
    }

    for (int i = MAXN - 1; i >= 0; i--)
    {
        int src_idx = 0, dest_idx = 0;
        do {
            src_idx = pseudo_rand() % K;
            dest_idx = pseudo_rand() % K;
        } while (src_idx == dest_idx);
        vehicles[i].y = src[src_idx].y;
        vehicles[i].x = src[src_idx].x;
        vehicles[i].dest_y = dest[dest_idx].y;
        vehicles[i].dest_x = dest[dest_idx].x;
        if(vehicles[i].y < 100)
        {
            src[src_idx].y++;
            vehicles[i].dir = DOWN;
        }
        else if(vehicles[i].y >= MAXM - 100)
        {
            src[src_idx].y--;
            vehicles[i].dir = UP;
        }
        else if(vehicles[i].x < 100)
        {
            src[src_idx].x++;
            vehicles[i].dir = RIGHT;
        }
        else if(vehicles[i].x >= MAXM - 100)
        {
            src[src_idx].x--;
            vehicles[i].dir = LEFT;
        }
    }
    for(int y = 0; y < MAXN; y++)
    {
        for(int x = 0; x < MAXM; x++)
        {
            vehicle_on_map[y][x] = 0;
            map_bak[y][x] = map[y][x];
            crossroad_id_bak[y][x] = crossroad_id[y][x];
        }
    }

    for(int i = 0; i < MAXN; i++)
    {
        vehicles_bak[i] = vehicles[i];
        vehicle_on_map[vehicles[i].y][vehicles[i].x] = 1;
    }
}

static int get_nextdir(Vehicle& tar)
{
    int dir = tar.dir;
    int y = tar.y;
    int x = tar.x;
    int dest_y = tar.dest_y;
    int dest_x = tar.dest_x;
    int next_y = y + dy[dir];
    int next_x = x + dx[dir];
    if(dest_y < next_y - 1)
    {
        if(dest_x < next_x - 1)
        {
            if(dir == RIGHT)
            {
                return TURN_LEFT;
            }
            else if(dir == DOWN)
            {
                return TURN_RIGHT;
            }
            else if(dir == UP)
            {
                if(next_y <= min_y + 1)
                {
                    return TURN_LEFT;
                }
            }
            else if(dir == LEFT)
            {
                if(next_x <= min_x + 1)
                {
                    return TURN_RIGHT;
                }
            }
        }
        else if(dest_x > next_x + 1)
        {
            if(dir == RIGHT)
            {
                if(next_x > max_x - 1)
                {
                    return TURN_LEFT;
                }
            }
            else if(dir == DOWN)
            {
                return TURN_LEFT;
            }
            else if(dir == UP)
            {
                if(next_y <= min_y + 1)
                {
                    return TURN_RIGHT;
                }
            }
            else if(dir == LEFT)
            {
                return TURN_RIGHT;
            }
        }
        else
        {
            if(dir == RIGHT)
            {
                return TURN_LEFT;
            }
            else if(dir == LEFT)
            {
                return TURN_RIGHT;
            }
        }
    }
    else if(dest_y > next_y + 1)
    {
        if(dest_x < next_x - 1)
        {
            if(dir == RIGHT)
            {
                return TURN_RIGHT;
            }
            else if(dir == DOWN)
            {
                if(next_y > max_y - 1)
                {
                    return TURN_RIGHT;
                }
            }
            else if(dir == UP)
            {
                return TURN_LEFT;
            }
            else if(dir == LEFT)
            {
                if(next_x <= min_x + 1)
                {
                    return TURN_LEFT;
                }
            }
        }
        else if(dest_x > next_x + 1)
        {
            if(dir == RIGHT)
            {
                if(next_x > max_x - 1)
                {
                    return TURN_RIGHT;
                }
            }
            else if(dir == DOWN)
            {
                if(next_y > max_y - 1)
                {
                    return TURN_LEFT;
                }
            }
            else if(dir == UP)
            {
                return TURN_RIGHT;
            }
            else if(dir == LEFT)
            {
                return TURN_LEFT;
            }
        }
        else
        {
            if(dir == RIGHT)
            {
                return TURN_RIGHT;
            }
            else if(dir == LEFT)
            {
                return TURN_LEFT;
            }
        }
    }
    else
    {
        if(dest_x < next_x - 1)
        {
            if(dir == UP)
            {
                return TURN_LEFT;
            }
            else if(dir == DOWN)
            {
                return TURN_RIGHT;
            }
        }
        else if(dest_x > next_x + 1)
        {
            if(dir == UP)
            {
                return TURN_RIGHT;
            }
            else if(dir == DOWN)
            {
                return TURN_LEFT;
            }
        }
    }
    return GO_STRAIGHT;
}

void move_vehicles() {
    for(int i = 0; i < MAXN; i++)
    {
        is_movable[i] = 0;
    }

    for (int i = 0; i < MAXN; i++)
    {
        int x = vehicles[i].x;
        int y = vehicles[i].y;
        int dir = vehicles[i].dir;
        int next_x = x + dx[dir];
        int next_y = y + dy[dir];

        if(vehicles[i].dest_x == x && vehicles[i].dest_y == y)
        {
            continue;
        }

        if(vehicle_on_map[next_y][next_x] == 1)
        {
            continue;
        }

        if(crossroad_id[next_y][next_x] == 0)
        {
            is_movable[i] = 1;
        }
        else
        {
            if(crossroad_id[y][x] == 0)
            {
                int signal = signal_list[crossroad_id[next_y][next_x]].signal;
                int turn_dir = get_nextdir(vehicles[i]);
                if(turn_dir == RIGHT)
                {
                    is_movable[i] = 1;
                }
                else
                {
                    if(dir == UP && signal == UPLEFT)
                    {
                        is_movable[i] = 1;
                    }
                    else if(dir == RIGHT && signal == RIGHTUP)
                    {
                        is_movable[i] = 1;
                    }
                    else if(dir == DOWN && signal == DOWNRIGHT)
                    {
                        is_movable[i] = 1;
                    }
                    else if(dir == LEFT && signal == LEFTDOWN)
                    {
                        is_movable[i] = 1;
                    }

                    if(turn_dir == GO_STRAIGHT)
                    {
                        if(dir == UP || dir == DOWN)
                        {
                            if(signal == VERTICAL)
                            {
                                is_movable[i] = 1;
                            }
                        }
                        else
                        {
                            if(signal == HORIZONTAL)
                            {
                                is_movable[i] = 1;
                            }
                        }
                    }
                }
            }
            else
            {
                is_movable[i] = 1;
            }
        }

    }

    for (int i = 0; i < MAXN; i++) {
        if (!is_movable[i]) continue;

        int x = vehicles[i].x;
        int y = vehicles[i].y;
        int dir = vehicles[i].dir;
        int next_x = x + dx[dir];
        int next_y = y + dy[dir];

        if (vehicle_on_map[next_y][next_x] == 0)
        {
            vehicle_on_map[y][x] = 0;
            vehicle_on_map[next_y][next_x] = 1;
            vehicles[i].x = next_x;
            vehicles[i].y = next_y;
            
            if(turn_dir[i] == TURN_RIGHT)
            {
                vehicles[i].dir = (dir + 1) % 4;
            }
            else if(turn_dir[i] == TURN_LEFT)
            {
                int next_dir = (dir + 3) % 4;
                if(next_dir == UP)
                {
                    if(map[next_y + 1][next_x + 1] == 0)
                    {
                        vehicles[i].dir = next_dir;
                        turn_dir[i] = GO_STRAIGHT;
                    }
                }
                else if(next_dir == RIGHT)
                {
                    if(map[next_y + 1][next_x - 1] == 0)
                    {
                        vehicles[i].dir = next_dir;
                        turn_dir[i] = GO_STRAIGHT;
                    }
                }
                else if(next_dir == DOWN)
                {
                    if(map[next_y - 1][next_x - 1] == 0)
                    {
                        vehicles[i].dir = next_dir;
                        turn_dir[i] = GO_STRAIGHT;
                    }
                }
                else if(next_dir == LEFT)
                {
                    if(map[next_y - 1][next_x + 1] == 0)
                    {
                        vehicles[i].dir = next_dir;
                        turn_dir[i] = GO_STRAIGHT;
                    }
                }
            }

            // Check if the vehicle has reached its destination
            if (vehicles[i].x == vehicles[i].dest_x && vehicles[i].y == vehicles[i].dest_y) {
                vehicle_on_map[vehicles[i].y][vehicles[i].x] = 0;
            }
        }
    }

    // Update signals for crossroads
    for(int i = 0; i < L; i++)
    {
        signal_list[i].signal = signal_list[i].next_signal;
    }
}

bool verify()
{
    for(int i = 0; i < MAXN; i++)
    {
        if(vehicles[i].x != vehicles[i].dest_x || vehicles[i].y != vehicles[i].dest_y)
        {
            return false;
        }
    }
    return true;
}

extern void init(int N, int map[][MAXM], int crossroad_id[][MAXM], Vehicle vehicles[MAXN]);
extern bool process();

// ---------------------------- SFML ----------------------------

#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <string>

// Constants
const int GRID_SIZE = 1000; // Grid dimensions
const int CELL_SIZE = 50;   // Size of each cell
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

// ---------------------------- SFML ----------------------------


int main()
{
    long long gTotalScore = 0;
    for(int i = 0; i < MAX_TC; i++)
    {
        make_tc();

        std::ofstream outfile("game_state.txt");
        outfile << "Number of roads: " << K << "\n";
        outfile << "Number of crossroads: " << L << "\n";
        outfile << "Roads:\n";
        for (int j = 0; j < K; j++) {
            outfile << "Source: (" << src[j].y << ", " << src[j].x << ") ";
            outfile << "Destination: (" << dest[j].y << ", " << dest[j].x << ")\n";
        }

        outfile << "Crossroads:\n";
        for (int y = 0; y < MAXN; y++) {
            for (int x = 0; x < MAXM; x++) {
            if (crossroad_id[y][x] != 0) {
                outfile << "Crossroad ID: " << crossroad_id[y][x] << " at (" << y << ", " << x << ")\n";
            }
            }
        }
        outfile.close();
        
        init(MAXM, map_bak, crossroad_id_bak, vehicles_bak);

        // Create a window
        sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Traffic Simulation");

        // Camera position
        int cameraX = 0, cameraY = 0;

        // Main loop
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            // Handle camera movement
            const int CAMERA_STEP = 10;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) cameraY -= CAMERA_STEP;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) cameraY += CAMERA_STEP;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) cameraX -= CAMERA_STEP;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) cameraX += CAMERA_STEP;

            // Prevent camera from going out of bounds
            cameraX = std::max(0, std::min(cameraX, GRID_SIZE * CELL_SIZE - SCREEN_WIDTH));
            cameraY = std::max(0, std::min(cameraY, GRID_SIZE * CELL_SIZE - SCREEN_HEIGHT));

            // Clear the window
            window.clear();

            // Calculate visible grid area
            int startX = cameraX / CELL_SIZE;
            int startY = cameraY / CELL_SIZE;
            int endX = std::min(startX + SCREEN_WIDTH / CELL_SIZE + 1, GRID_SIZE);
            int endY = std::min(startY + SCREEN_HEIGHT / CELL_SIZE + 1, GRID_SIZE);

            // Render visible grid cells
            for (int y = startY; y < endY; ++y) {
                for (int x = startX; x < endX; ++x) {
                    // Create cell rectangle
                    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                    cell.setPosition((x * CELL_SIZE) - cameraX, (y * CELL_SIZE) - cameraY);

                    // Set cell color
                    cell.setFillColor(map[y][x] == 1 ? sf::Color::Green : sf::Color::Black);
                    cell.setOutlineColor(sf::Color::White);
                    cell.setOutlineThickness(1);

                    // Draw the cell
                    window.draw(cell);
                }
            }

            // Display the frame
            window.display();
        }
        bool is_finished = false;
        
        while(is_finished == false)
        {
            is_finished = process();
            move_vehicles();
            gTotalScore++;
        }

        if(verify() == false)
        {
            gTotalScore = PENALTY;
        }
    }

    printf("%lld\n", gTotalScore);
}