#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>


#include <random>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PADDLE_WIDTH = 100.f;
const float PADDLE_HEIGHT = 20.f;
const float BALL_RADIUS = 10.f;
const float BRICK_WIDTH = 80.f;
const float BRICK_HEIGHT = 30.f;
const int BRICK_ROWS = 5;
const int BRICK_COLUMNS = 10;
const float INITIAL_BALL_SPEED = 10.f;
const float PADDLE_SPEED = 17.f;
const float POWERUP_SPEED = 3.f;
const float POWERUP_SIZE = 15.f;

class PowerUp {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool active;
    int type; // 0 - �������������� �����, 1 - ���������� ���������, 2 - ����������� �������

    PowerUp(float x, float y) : active(true) {
        shape.setPosition(x, y);
        shape.setRadius(POWERUP_SIZE);
        shape.setOrigin(POWERUP_SIZE, POWERUP_SIZE);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 2);
        type = dist(gen);

        switch (type) {
        case 0: shape.setFillColor(sf::Color::Green); break;   // ���. �����
        case 1: shape.setFillColor(sf::Color::Yellow); break;   // ���������� ���������
        case 2: shape.setFillColor(sf::Color::Magenta); break;  // ����������� �������
        }

        velocity.y = POWERUP_SPEED;
    }

    void update() {
        shape.move(velocity);
    }

    float left() const { return shape.getPosition().x - shape.getRadius(); }
    float right() const { return shape.getPosition().x + shape.getRadius(); }
    float top() const { return shape.getPosition().y - shape.getRadius(); }
    float bottom() const { return shape.getPosition().y + shape.getRadius(); }
};

class Brick {
public:
    sf::RectangleShape shape;
    bool isDestroyed;

    Brick(float x, float y, sf::Color color) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(BRICK_WIDTH, BRICK_HEIGHT));
        shape.setFillColor(color);
        shape.setOrigin(BRICK_WIDTH / 2.f, BRICK_HEIGHT / 2.f);
        isDestroyed = false;
    }
};

class Ball {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool active;

    Ball(float x, float y) : active(true) {
        shape.setPosition(x, y);
        shape.setRadius(BALL_RADIUS);
        shape.setFillColor(sf::Color::Red);
        shape.setOrigin(BALL_RADIUS, BALL_RADIUS);

        velocity.x = INITIAL_BALL_SPEED * (rand() % 2 ? 1 : -1);
        velocity.y = -INITIAL_BALL_SPEED;
    }

    void update() {
        if (!active) return;

        shape.move(velocity);

        if (left() < 0 || right() > WINDOW_WIDTH) {
            velocity.x = -velocity.x;
        }
        if (top() < 0) {
            velocity.y = -velocity.y;
        }
    }

    float left() const { return shape.getPosition().x - shape.getRadius(); }
    float right() const { return shape.getPosition().x + shape.getRadius(); }
    float top() const { return shape.getPosition().y - shape.getRadius(); }
    float bottom() const { return shape.getPosition().y + shape.getRadius(); }
};

class Paddle {
public:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float originalWidth;

    Paddle(float x, float y) {
        shape.setPosition(x, y);
        originalWidth = PADDLE_WIDTH;
        shape.setSize(sf::Vector2f(originalWidth, PADDLE_HEIGHT));
        shape.setFillColor(sf::Color::Blue);
        shape.setOrigin(originalWidth / 2.f, PADDLE_HEIGHT / 2.f);
    }

    void update() {
        shape.move(velocity);

        if (left() < 0) {
            shape.setPosition(shape.getSize().x / 2.f, shape.getPosition().y);
        }
        if (right() > WINDOW_WIDTH) {
            shape.setPosition(WINDOW_WIDTH - shape.getSize().x / 2.f, shape.getPosition().y);
        }
    }

    void resize(float factor) {
        float newWidth = originalWidth * factor;
        shape.setSize(sf::Vector2f(newWidth, PADDLE_HEIGHT));
        shape.setOrigin(newWidth / 2.f, PADDLE_HEIGHT / 2.f);
    }

    float left() const { return shape.getPosition().x - shape.getSize().x / 2.f; }
    float right() const { return shape.getPosition().x + shape.getSize().x / 2.f; }
    float top() const { return shape.getPosition().y - shape.getSize().y / 2.f; }
    float bottom() const { return shape.getPosition().y + shape.getSize().y / 2.f; }
};

bool isColliding(const Ball& ball, const Paddle& paddle) {
    return ball.right() >= paddle.left() && ball.left() <= paddle.right() &&
        ball.bottom() >= paddle.top() && ball.top() <= paddle.bottom();
}

bool isColliding(const Ball& ball, const Brick& brick) {
    return ball.right() >= brick.shape.getPosition().x - brick.shape.getSize().x / 2.f &&
        ball.left() <= brick.shape.getPosition().x + brick.shape.getSize().x / 2.f &&
        ball.bottom() >= brick.shape.getPosition().y - brick.shape.getSize().y / 2.f &&
        ball.top() <= brick.shape.getPosition().y + brick.shape.getSize().y / 2.f;
}

bool isColliding(const Paddle& paddle, const PowerUp& powerup) {
    return powerup.right() >= paddle.left() && powerup.left() <= paddle.right() &&
        powerup.bottom() >= paddle.top() && powerup.top() <= paddle.bottom();
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Arkanoid");
    window.setFramerateLimit(60);

    std::vector<Ball> balls;
    balls.emplace_back(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

    Paddle paddle(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 50.f);

    std::vector<Brick> bricks;
    std::vector<PowerUp> powerups;

    // �������� ��������
    for (int i = 0; i < BRICK_ROWS; ++i) {
        for (int j = 0; j < BRICK_COLUMNS; ++j) {
            float x = j * (BRICK_WIDTH + 5.f) + BRICK_WIDTH / 2.f + 10.f;
            float y = i * (BRICK_HEIGHT + 5.f) + BRICK_HEIGHT / 2.f + 50.f;

            sf::Color color;
            switch (i) {
            case 0: color = sf::Color::Red; break;
            case 1: color = sf::Color(255, 128, 0); break;
            case 2: color = sf::Color::Yellow; break;
            case 3: color = sf::Color::Green; break;
            case 4: color = sf::Color::Blue; break;
            }

            bricks.emplace_back(x, y, color);
        }
    }

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10.f, 10.f);

    int score = 0;
    int lives = 3;
    bool isGameOver = false;
    bool isPaused = false;
    bool isWelcomeScreen = true;  // ����� ��������� ��� ��������������� ������
    sf::Clock powerupClock;
    bool paddleEnlarged = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    if (isWelcomeScreen) {
                        // ������ �� ������ �� �������������� ������
                    }
                    else {
                        isPaused = !isPaused;
                    }
                }

                if (event.key.code == sf::Keyboard::Return) {  // Enter ��� ������ ����
                    if (isWelcomeScreen) {
                        isWelcomeScreen = false;
                        isPaused = false;
                    }
                }

                if (event.key.code == sf::Keyboard::Space) {
                    if (isGameOver) {
                        // ���������� ���� ����� Game Over
                        bricks.clear();
                        for (int i = 0; i < BRICK_ROWS; ++i) {
                            for (int j = 0; j < BRICK_COLUMNS; ++j) {
                                float x = j * (BRICK_WIDTH + 5.f) + BRICK_WIDTH / 2.f + 10.f;
                                float y = i * (BRICK_HEIGHT + 5.f) + BRICK_HEIGHT / 2.f + 50.f;

                                sf::Color color;
                                switch (i) {
                                case 0: color = sf::Color::Red; break;
                                case 1: color = sf::Color(255, 128, 0); break;
                                case 2: color = sf::Color::Yellow; break;
                                case 3: color = sf::Color::Green; break;
                                case 4: color = sf::Color::Blue; break;
                                }

                                bricks.emplace_back(x, y, color);
                            }
                        }
                        score = 0;
                        lives = 3;
                        isGameOver = false;
                        paddle.resize(1.0f);
                        paddleEnlarged = false;
                        balls.clear();
                        balls.emplace_back(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
                        paddle.shape.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 50.f);
                    }
                    else if (isPaused) {
                        // ���������� ���� ��� ������� Space �� ����� �����
                        bricks.clear();
                        for (int i = 0; i < BRICK_ROWS; ++i) {
                            for (int j = 0; j < BRICK_COLUMNS; ++j) {
                                float x = j * (BRICK_WIDTH + 5.f) + BRICK_WIDTH / 2.f + 10.f;
                                float y = i * (BRICK_HEIGHT + 5.f) + BRICK_HEIGHT / 2.f + 50.f;

                                sf::Color color;
                                switch (i) {
                                case 0: color = sf::Color::Red; break;
                                case 1: color = sf::Color(255, 128, 0); break;
                                case 2: color = sf::Color::Yellow; break;
                                case 3: color = sf::Color::Green; break;
                                case 4: color = sf::Color::Blue; break;
                                }

                                bricks.emplace_back(x, y, color);
                            }
                        }
                        score = 0;
                        lives = 3;
                        isPaused = false;
                        paddle.resize(1.0f);
                        paddleEnlarged = false;
                        balls.clear();
                        balls.emplace_back(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
                        paddle.shape.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 50.f);
                    }
                    else if (balls.empty()) {
                        // ���������� ������ ������, ���� ��� ������ ��������
                        balls.emplace_back(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
                        paddle.shape.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 50.f);
                    }
                }
            }
        }

        if (isGameOver || isPaused || isWelcomeScreen) {
            window.clear(sf::Color::Black);

            window.draw(paddle.shape);
            for (auto& ball : balls) {
                if (ball.active) window.draw(ball.shape);
            }
            for (auto& brick : bricks) {
                if (!brick.isDestroyed) window.draw(brick.shape);
            }
            for (auto& powerup : powerups) {
                if (powerup.active) window.draw(powerup.shape);
            }

            if (isGameOver) {
                scoreText.setString("Game Over! Score: " + std::to_string(score) + "\nPress SPACE to restart");
            }
            else if (isWelcomeScreen) {
                scoreText.setString("Welcome to Arkanoid!\nPress ENTER to start\nESC to pause");
            }
            else if (isPaused) {
                scoreText.setString("Paused\nScore: " + std::to_string(score) + " Lives: " + std::to_string(lives) +
                    "\nBalls: " + std::to_string(balls.size()) + "\nPress SPACE to restart\nESC to continue");
            }

            scoreText.setPosition(WINDOW_WIDTH / 2.f - scoreText.getLocalBounds().width / 2.f,
                WINDOW_HEIGHT / 2.f - scoreText.getLocalBounds().height / 2.f);
            window.draw(scoreText);

            window.display();
            continue;
        }

        // ���������� ����������
        paddle.velocity.x = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            paddle.velocity.x = -PADDLE_SPEED;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            paddle.velocity.x = PADDLE_SPEED;
        }

        // ���������� ������� ���������
        paddle.update();

        // ���������� �������
        for (auto& ball : balls) {
            if (!ball.active) continue;

            ball.update();

            // �������� ������������ � ����������
            if (isColliding(ball, paddle)) {
                float hitPosition = (ball.shape.getPosition().x - paddle.shape.getPosition().x) / (paddle.shape.getSize().x / 2.f);
                ball.velocity.x = hitPosition * INITIAL_BALL_SPEED * 1.5f;
                ball.velocity.y = -std::abs(ball.velocity.y);

                float speed = std::sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
                ball.velocity.x = ball.velocity.x / speed * INITIAL_BALL_SPEED * 1.1f;
                ball.velocity.y = ball.velocity.y / speed * INITIAL_BALL_SPEED * 1.1f;
            }

            // �������� ������������ � ���������
            for (auto& brick : bricks) {
                if (!brick.isDestroyed && isColliding(ball, brick)) {
                    brick.isDestroyed = true;
                    score += 10;

                    // � ������ 30% ������� ����� ��� ���������� �������
                    if (rand() % 100 < 30) {
                        powerups.emplace_back(brick.shape.getPosition().x, brick.shape.getPosition().y);
                    }

                    float overlapLeft = ball.right() - (brick.shape.getPosition().x - brick.shape.getSize().x / 2.f);
                    float overlapRight = (brick.shape.getPosition().x + brick.shape.getSize().x / 2.f) - ball.left();
                    float overlapTop = ball.bottom() - (brick.shape.getPosition().y - brick.shape.getSize().y / 2.f);
                    float overlapBottom = (brick.shape.getPosition().y + brick.shape.getSize().y / 2.f) - ball.top();

                    bool ballFromLeft = std::abs(overlapLeft) < std::abs(overlapRight);
                    bool ballFromTop = std::abs(overlapTop) < std::abs(overlapBottom);

                    float minOverlapX = ballFromLeft ? overlapLeft : overlapRight;
                    float minOverlapY = ballFromTop ? overlapTop : overlapBottom;

                    if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                        ball.velocity.x = -ball.velocity.x;
                    }
                    else {
                        ball.velocity.y = -ball.velocity.y;
                    }

                    break;
                }
            }

            // �������� ������ �� ������ �������
            if (ball.bottom() > WINDOW_HEIGHT) {
                ball.active = false;
            }
        }

        // �������� ���������� �������
        balls.erase(std::remove_if(balls.begin(), balls.end(),
            [](const Ball& ball) { return !ball.active; }), balls.end());

        // ���� ������� �� �������� - ������ �����
        if (balls.empty()) {
            lives--;
            if (lives <= 0) {
                isGameOver = true;
            }
            else {
                balls.emplace_back(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
                paddle.shape.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 50.f);
            }
        }

        // ���������� �������
        for (auto& powerup : powerups) {
            if (!powerup.active) continue;

            powerup.update();

            // �������� ������������ � ����������
            if (isColliding(paddle, powerup)) {
                powerup.active = false;

                switch (powerup.type) {
                case 0: // �������������� �����
                    lives++;
                    break;

                case 1: // ���������� ���������
                    paddle.resize(1.5f);
                    paddleEnlarged = true;
                    powerupClock.restart();
                    break;

                case 2: // ����������� �������
                    std::vector<Ball> newBalls;
                    for (auto& ball : balls) {
                        if (!ball.active) continue;

                        // ������� 2 ����� ������ ��� ������ 30 � -30 �������� � ��������
                        float speed = std::sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);

                        // ������ ����� �����
                        Ball newBall1 = ball;
                        float angle1 = 30.f * 3.14159265f / 180.f;
                        newBall1.velocity.x = ball.velocity.x * std::cos(angle1) - ball.velocity.y * std::sin(angle1);
                        newBall1.velocity.y = ball.velocity.x * std::sin(angle1) + ball.velocity.y * std::cos(angle1);
                        // ����������� � �������� �� ��������
                        float len1 = std::sqrt(newBall1.velocity.x * newBall1.velocity.x + newBall1.velocity.y * newBall1.velocity.y);
                        newBall1.velocity.x = newBall1.velocity.x / len1 * speed;
                        newBall1.velocity.y = newBall1.velocity.y / len1 * speed;
                        newBalls.push_back(newBall1);

                        // ������ ����� �����
                        Ball newBall2 = ball;
                        float angle2 = -30.f * 3.14159265f / 180.f;
                        newBall2.velocity.x = ball.velocity.x * std::cos(angle2) - ball.velocity.y * std::sin(angle2);
                        newBall2.velocity.y = ball.velocity.x * std::sin(angle2) + ball.velocity.y * std::cos(angle2);
                        // ����������� � �������� �� ��������
                        float len2 = std::sqrt(newBall2.velocity.x * newBall2.velocity.x + newBall2.velocity.y * newBall2.velocity.y);
                        newBall2.velocity.x = newBall2.velocity.x / len2 * speed;
                        newBall2.velocity.y = newBall2.velocity.y / len2 * speed;
                        newBalls.push_back(newBall2);
                    }

                    // ��������� ����� ������
                    for (auto& newBall : newBalls) {
                        balls.push_back(newBall);
                    }
                    break;
                }
            }

            // �������� �������, ��������� �� �����
            if (powerup.top() > WINDOW_HEIGHT) {
                powerup.active = false;
            }
        }

        // �������� ���������� �������
        powerups.erase(std::remove_if(powerups.begin(), powerups.end(),
            [](const PowerUp& powerup) { return !powerup.active; }), powerups.end());

        // �������� ������� �������� ����������� ��������� (10 ������)
        if (paddleEnlarged && powerupClock.getElapsedTime().asSeconds() > 10.f) {
            paddle.resize(1.0f);
            paddleEnlarged = false;
        }

        // �������� ������ (��� ������� ���������)
        bool allDestroyed = true;
        for (auto& brick : bricks) {
            if (!brick.isDestroyed) {
                allDestroyed = false;
                break;
            }
        }

        if (allDestroyed) {
            isGameOver = true;
            scoreText.setString("You Win! Score: " + std::to_string(score) + "\nPress SPACE to play again");
        }

        // ���������
        window.clear(sf::Color::Black);

        // ��������� ��������
        window.draw(paddle.shape);
        for (auto& ball : balls) {
            if (ball.active) window.draw(ball.shape);
        }
        for (auto& brick : bricks) {
            if (!brick.isDestroyed) window.draw(brick.shape);
        }
        for (auto& powerup : powerups) {
            if (powerup.active) window.draw(powerup.shape);
        }

        // ���������� � ��������� ������
        scoreText.setString("Score: " + std::to_string(score) +
            " Lives: " + std::to_string(lives) +
            " Balls: " + std::to_string(balls.size()));
        scoreText.setPosition(10.f, 10.f);
        window.draw(scoreText);

        window.display();
    }

    return 0;
}
