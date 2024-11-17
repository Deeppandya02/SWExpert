#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <string>

// Constants
const int GRID_SIZE = 1000; // Grid dimensions
const int CELL_SIZE = 50;   // Size of each cell
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

// Mock grid data
std::vector<std::vector<int>> map(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));

void initializeGrid() {
    // Populate the grid with sample data
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            map[y][x] = (x + y) % 2; // Alternate pattern
        }
    }
}

void drawNumberOnCell(sf::RenderWindow& window, int number, float x, float y) {
    // Represent numbers using smaller rectangles within the cell
    const int blockSize = 10; // Size of blocks used for visual representation
    float offsetX = x;
    float offsetY = y;
    
    // Draw blocks for each digit
    while (number > 0) {
        int digit = number % 10;

        // Draw a block for the digit
        sf::RectangleShape block(sf::Vector2f(blockSize, blockSize));
        block.setFillColor(sf::Color::Red);
        block.setPosition(offsetX + (digit * (blockSize + 2)), offsetY);
        window.draw(block);

        number /= 10;
        offsetY += blockSize + 2; // Stack blocks vertically
    }
}

int main() {
    // Initialize the grid
    initializeGrid();

    // Create a window
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Camera Example with Numbers");

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

                // Calculate and draw number (row + column index sum)
                int number = y + x;
                drawNumberOnCell(window, number, (x * CELL_SIZE) - cameraX + 5, (y * CELL_SIZE) - cameraY + 5);
            }
        }

        // Display the frame
        window.display();
    }

    return 0;
}
