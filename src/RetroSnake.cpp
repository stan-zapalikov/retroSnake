#include "raylib.h"
#include "raymath.h"
#include <deque>

Color lightGreen = { 173, 204, 96, 255 };
Color darkGreen = { 43, 51, 24, 255 };

float cellSize = 30;
float cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool eventTriggered(double interval) {
	double currentTime = GetTime();
	if (currentTime - lastUpdateTime >= interval) {
		lastUpdateTime = currentTime;
		return true;
	}
	else return false;
}

bool elementInDeque(Vector2 element, std::deque<Vector2> deque) {
	for (unsigned int i = 0; i < deque.size(); i++) {
		if (Vector2Equals(deque[i], element)) {
			return true;
		}
	}
	return false;
}

class Food {
public:
	Vector2 position;
	Texture2D texture;

	Food(std::deque<Vector2> snakeBody) {
		Image image = LoadImage("food.png");
		texture = LoadTextureFromImage(image);
		UnloadImage(image);
		position = GenerateRandomPos(snakeBody);
	}

	void Draw() {
		DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
	}

	Vector2 GenerateRandomCell() {
		float x = GetRandomValue(0, cellCount - 1);
		float y = GetRandomValue(0, cellCount - 1);
		return Vector2{ x, y };
	}

	Vector2 GenerateRandomPos(std::deque<Vector2> snakeBody) {
		Vector2 position = GenerateRandomCell();

		while (elementInDeque(position, snakeBody)) {
			position = GenerateRandomCell();
		}

		return position;
	}

	~Food() {
		UnloadTexture(texture);
	}
};

class Snake {
public:
	std::deque<Vector2> body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
	Vector2 direction = { 1, 0 };
	bool addSegment = false;

	void Draw() {
		for (unsigned int i = 0; i < body.size(); i++) {
			float x = body[i].x;
			float y = body[i].y;
			
			Rectangle segment = Rectangle{ offset + x * cellSize, offset + y * cellSize, cellSize, cellSize };
			DrawRectangleRounded(segment, 0.5, 6, darkGreen);
		}
	}

	void Update() {

		if (addSegment) {
			body.push_front(Vector2Add(body[0], direction));
			addSegment = false;
		}
		else {
			body.pop_back();
			body.push_front(Vector2Add(body[0], direction));
		}

	}

	void Reset() {
		body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
		direction = { 1, 0 };
	}


};

class Game {
public:
	Sound eatSound;
	Sound wallSound;
	Snake snake = Snake();
	Food food = Food(snake.body);
	bool running = true;
	int score = 0;

	Game() {
		InitAudioDevice();
		eatSound = LoadSound("eat.mp3");
		wallSound = LoadSound("wall.mp3");
	}

	~Game() {
		UnloadSound(eatSound);
		UnloadSound(wallSound);
		CloseAudioDevice();
	}

	void Draw() {
		food.Draw();
		snake.Draw();
	}

	void Update() {
		if (running) {
			snake.Update();
			CheckCollisionWithFood();
			CheckCollisionWithEdges();
			CheckCollisionWithTail();
		}
	}

	void CheckCollisionWithFood() {
		if (Vector2Equals(snake.body[0], food.position)) {
			food.position = food.GenerateRandomPos(snake.body);
			snake.addSegment = true;
			score++;
			PlaySound(eatSound);
		}
	}

	void CheckCollisionWithEdges() {
		if (snake.body[0].x == cellCount || snake.body[0].x == -1) {
			GameOver();
		}
		if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
			GameOver();
		}
	}

	void GameOver() {
		snake.Reset();
		food.position = food.GenerateRandomPos(snake.body);
		running = false;
		PlaySound(wallSound);
		score = 0;
	}
	

	void CheckCollisionWithTail() {
		std::deque<Vector2> headlessBody = snake.body;
		headlessBody.pop_front();
		if (elementInDeque(snake.body[0], headlessBody)) {
			GameOver();
		}
	}
};

int main()
{

	InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake");
	SetTargetFPS(60);
	Game game = Game();


	while (!WindowShouldClose()) {
		BeginDrawing();
		
		ClearBackground(lightGreen);
		DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, cellSize * cellCount + 10, cellSize * cellCount + 10 }, 5, darkGreen);
		DrawText("Snake", offset - 5, 20, 40, darkGreen);
		DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
		game.Draw();
		if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
			game.snake.direction = { 0, -1 };
			game.running = true;
		}

		if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
			game.snake.direction = { 0, 1 };
			game.running = true;
		}

		if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
			game.snake.direction = { -1, 0 };
			game.running = true;
		}

		if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
			game.snake.direction = { 1, 0 };
			game.running = true;
		}

		if (eventTriggered(0.2)) {
			game.Update();
		}



		EndDrawing();
	}

	CloseWindow();
}
