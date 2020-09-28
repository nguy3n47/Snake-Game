/*
__/\\\\\\\\\\\____/\\\\\\\\\______/\\\\\\\\\\\\\\\_
 _\/////\\\///___/\\\///////\\\___\///////\\\/////__
  _____\/\\\_____\///______\//\\\________\/\\\_______
   _____\/\\\_______________/\\\/_________\/\\\_______
	_____\/\\\____________/\\\//___________\/\\\_______
	 _____\/\\\_________/\\\//______________\/\\\_______
	  _____\/\\\_______/\\\/_________________\/\\\_______
	   __/\\\\\\\\\\\__/\\\\\\\\\\\\\\\_______\/\\\_______
		_\///////////__\///////////////________\///________

AUTHOR	: VU CAO NGUYEN  [18600187]
AUTHOR	: KHUC KHAI HOAN [18600093]
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x500

#include <windows.h>
#include <conio.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <fstream>
#include <mmsystem.h>


using namespace std;

#define MAX_SIZE_SNAKE 30
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 6

void ResizeConsole(int width, int height)
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, width, height, TRUE);
}
// Color
void TextColor(int x)
{
	HANDLE mau;
	mau = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(mau, x);
}

// Ẩn con trỏ chuột
void HideCursor()
{
	HANDLE hOut;
	CONSOLE_CURSOR_INFO ConCurInf;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	ConCurInf.dwSize = 10;
	ConCurInf.bVisible = FALSE;
	SetConsoleCursorInfo(hOut, &ConCurInf);
}

struct Gate {
	POINT center; // Điểm trung tâm
	int direct;	  // Hướng 1-trên 2-phải 3-dưới 4-trái
};
POINT snake[MAX_SIZE_SNAKE]; // Con rắn
POINT food[MAX_SIZE_FOOD];   // Thức ăn
Gate G; // Cổng
int CHAR_LOCK; // Biên xác định hướng không thể di chuyển (ở một thời điểm có một hướng snake không thể di chuyển) 
int MOVING;    // Biên xác định hướng di chuyển của snake (snake có 3 hướng có thể di chuyển)
int SPEED;     // Level, tốc độ của snake
int HEIGH_CONSOLE, WIDTH_CONSOLE; // Kích thước màn hình console
int SCORE;
int FOOD_INDEX; // Chỉ số thức ăn đang có trên màn hình
int SIZE_SNAKE; // Kích thước của snake, lúc đầu 6 và tối đa là 10
int STATE; // Trạng thái sống hay chết của snake
char MSSV[16] = { '1', '8', '6', '0', '0', '1', '8', '7', '1', '8', '6', '0', '0', '0', '9', '3' };
int dy[4][5] = { { -1, 0, 0, 0, -1 }, { -1, -1, 0, 1, 1 }, { 1, 0, 0, 0, 1 }, { -1, -1, 0, 1, 1 } };
int dx[4][5] = { { -1, -1, 0, 1, 1 }, { 1, 0, 0, 0, 1 }, { -1, -1, 0, 1, 1 }, { -1, 0, 0, 0, -1 } };

void ClearSnakeAndFood(char* str);
void DrawGate(char* str);
void clearInfor();

void ShowConsoleCursor(bool showFlag) {
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = showFlag;
	SetConsoleCursorInfo(out, &cursorInfo);
}
void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

bool IsValid(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE; i++) {
		if (snake[i].x == x & snake[i].y == y)
			return false;
	}
	return true;
}


void GenerateFood() {
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - 1) + 1;
			y = rand() % (HEIGH_CONSOLE - 1) + 1;
		} while (!IsValid(x, y));
		food[i] = { x, y };
	}
}

bool OkGate(Gate g) {
	for (int k = 0; k < 5; k++) {
		int i = G.center.x + dx[G.direct - 1][k];
		int j = G.center.y + dy[G.direct - 1][k];
		if (!IsValid(i, j))  return false;
	}
	return true;
}

void GenerateGate() {
	int x, y;
	srand(time(NULL));
	G.center.x = rand() % (WIDTH_CONSOLE - 3) + 2;
	G.center.y = rand() % (HEIGH_CONSOLE - 3) + 2;
	do {
		G.direct = rand() % 4 + 1;
	} while (!OkGate(G));
}

void ResetData() {
	CHAR_LOCK = 'A';
	MOVING = 'D';
	SPEED = 1;
	SCORE = 0;
	FOOD_INDEX = 0;
	WIDTH_CONSOLE = 80;
	HEIGH_CONSOLE = 21;
	SIZE_SNAKE = 6;
	snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
	snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
	snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
	GenerateFood();
	GenerateGate();
}

void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
	TextColor(10);
	GotoXY(x, y);
	cout << char(201);
	for (int i = 1; i < width; i++) cout << char(205);
	cout << char(187);
	GotoXY(x, height + y); cout << char(200);
	for (int i = 1; i < width; i++) cout << char(205);
	cout << char(188);
	for (int i = y + 1; i < height + y; i++) {
		GotoXY(x, i); cout << char(186);
		GotoXY(x + width, i); cout << char(186);
	}
	GotoXY(curPosX, curPosY);
}

void StartGame() {
	system("cls");
	ResetData();
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
	STATE = 1;
}

void ExitGame(HANDLE t) {
	system("cls");
	TerminateThread(t, NULL);
}

void PauseGame(HANDLE t) {
	SuspendThread(t);
}

void DrawSnakeAndFood(const char* str) {
	if (FOOD_INDEX < MAX_SIZE_FOOD) {
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		TextColor(4);
		cout << char(4);
	}
	TextColor(10);
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		printf("%c", MSSV[i % 16]);
	}
}
void DrawGate(const char* str) {
	for (int k = 0; k < 5; k++) {
		int i = G.center.x + dx[G.direct - 1][k];
		int j = G.center.y + dy[G.direct - 1][k];
		GotoXY(i, j);
		TextColor(4);
		printf(str);
	}
}

void ClearSnakeAndFood(const char* str) {
	GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
	printf(str);
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		TextColor(4);
		printf(str);
	}
}

bool isClash(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE; i++)
		if (snake[i].x == x && snake[i].y == y) return true;
	return false;
}

void Eat() {
	if (FOOD_INDEX == MAX_SIZE_FOOD) snake[SIZE_SNAKE] = G.center;
	else snake[SIZE_SNAKE] = food[FOOD_INDEX];

	if (FOOD_INDEX == MAX_SIZE_FOOD - 1) {
		GenerateGate();
		SIZE_SNAKE++;
		FOOD_INDEX++;
		SCORE++;

	}
	else if (FOOD_INDEX == MAX_SIZE_FOOD) {
		DrawGate(" ");
		FOOD_INDEX = 0;
		if (SPEED == MAX_SPEED) {
			PlaySound(TEXT("Gate.wav"), NULL, SND_SYNC);
			SPEED = 1;
			SIZE_SNAKE = 6;

		}
		else {
			PlaySound(TEXT("Gate.wav"), NULL, SND_SYNC);
			Sleep(0);
			SPEED++;
		}
		GenerateFood();
	}
	else {
		FOOD_INDEX++;
		SCORE++;
		SIZE_SNAKE++;

	}

}


void ProcessDead() {
	STATE = 0;
	clearInfor();
	ClearSnakeAndFood(" ");
	ClearSnakeAndFood("x");
	GotoXY(0, HEIGH_CONSOLE + 2);
	printf("YOU LOSE! Type 'Y' to CONTINUE or anykey to EXIT\nType 'T' to LOAD GAME");
	PlaySound(TEXT("FAIL.wav"), NULL, SND_SYNC);
}

bool CollideGate(int x, int y) {
	for (int k = 0; k < 5; k++) {
		int i = G.center.x + dx[G.direct - 1][k];
		int j = G.center.y + dy[G.direct - 1][k];
		if (x == i && y == j) return true;
	}
	return false;
}

// Các hàm di chuyển
void MoveRight() {
	if (snake[SIZE_SNAKE - 1].x + 1 >= WIDTH_CONSOLE) {
		ProcessDead();
	}
	else if (isClash(snake[SIZE_SNAKE - 1].x + 1, snake[SIZE_SNAKE - 1].y)) {
		ProcessDead();
	}
	else if (FOOD_INDEX == MAX_SIZE_FOOD) {
		if (snake[SIZE_SNAKE - 1].x + 1 == G.center.x && snake[SIZE_SNAKE - 1].y == G.center.y && G.direct == 4) {
			PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
			Sleep(0);
			Eat();
		}
		else if (CollideGate(snake[SIZE_SNAKE - 1].x + 1, snake[SIZE_SNAKE - 1].y)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x + 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
		PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
		Sleep(0);
		Eat();
	}
	// Chinh lại tọa độ
	for (int i = 0; i < SIZE_SNAKE - 1; i++) {
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}

	snake[SIZE_SNAKE - 1].x++;
}

void MoveLeft() {

	if (snake[SIZE_SNAKE - 1].x - 1 <= 0) {
		ProcessDead();
	}
	else if (isClash(snake[SIZE_SNAKE - 1].x - 1, snake[SIZE_SNAKE - 1].y)) {
		ProcessDead();
	}
	else if (FOOD_INDEX == MAX_SIZE_FOOD) {
		if (snake[SIZE_SNAKE - 1].x - 1 == G.center.x && snake[SIZE_SNAKE - 1].y == G.center.y && G.direct == 2) {
			PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
			Sleep(0);
			Eat();
		}
		else if (CollideGate(snake[SIZE_SNAKE - 1].x - 1, snake[SIZE_SNAKE - 1].y)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x - 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
		PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
		Sleep(0);
		Eat();
	}
	// Chỉnh lại tọa độ
	for (int i = 0; i < SIZE_SNAKE - 1; i++) {
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}

	snake[SIZE_SNAKE - 1].x--;
}

void MoveDown() {
	if (snake[SIZE_SNAKE - 1].y + 1 >= HEIGH_CONSOLE) {
		ProcessDead();
	}
	else if (isClash(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)) {
		ProcessDead();
	}
	else if (FOOD_INDEX == MAX_SIZE_FOOD) {
		if (snake[SIZE_SNAKE - 1].x == G.center.x && snake[SIZE_SNAKE - 1].y + 1 == G.center.y && G.direct == 1) {
			PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
			Sleep(0);
			Eat();
		}
		else if (CollideGate(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y + 1 == food[FOOD_INDEX].y) {
		PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
		Sleep(0);
		Eat();
	}

	// Chỉnh lại tọa độ

	for (int i = 0; i < SIZE_SNAKE - 1; i++) {
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}

	snake[SIZE_SNAKE - 1].y++;
}

void MoveUp() {

	if (snake[SIZE_SNAKE - 1].y - 1 <= 0) {
		ProcessDead();
	}
	else if (isClash(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y - 1)) {
		ProcessDead();
	}
	else if (FOOD_INDEX == MAX_SIZE_FOOD) {
		if (snake[SIZE_SNAKE - 1].x == G.center.x && snake[SIZE_SNAKE - 1].y - 1 == G.center.y && G.direct == 3) {
			PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
			Sleep(0);
			Eat();
		}
		else if (CollideGate(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y - 1)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y - 1 == food[FOOD_INDEX].y) {
		PlaySound(TEXT("eat.wav"), NULL, SND_ASYNC);
		Sleep(0);
		Eat();
	}

	// Chỉnh lại tọa độ

	for (int i = 0; i < SIZE_SNAKE - 1; i++) {
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}

	snake[SIZE_SNAKE - 1].y--;
}

void printInfor() {
	GotoXY(WIDTH_CONSOLE / 1.5, HEIGH_CONSOLE + 3);
	printf("LEVEL:  %d", SPEED);
	GotoXY(WIDTH_CONSOLE / 5, HEIGH_CONSOLE + 3);
	printf("SCORES:  %d", SCORE);
	GotoXY(85, 0);
	printf("-------------- HOW TO PLAY --------------");
	GotoXY(85, 2);
	printf("P : PAUSE GAME");
	GotoXY(85, 4);
	printf("ESC : EXIT GAME");
	GotoXY(85, 6);
	printf("L : SAVE GAME");
	GotoXY(85, 8);
	printf("  W  ");
	GotoXY(85, 9);
	printf("A S D : MOVE SNAKE");
	GotoXY(85, 11);
	printf("-------------- INFORMATION --------------");
	GotoXY(85, 13);
	printf("Game : SNAKE");
	GotoXY(85, 15);
	printf("Version : v1.0");
	GotoXY(85, 17);
	printf("Code By: I2T");
	GotoXY(85, 19);
	printf("AUTHOR 1 : VU CAO NGUYEN  [18600187]");
	GotoXY(85, 21);
	printf("AUTHOR 2 : KHUC KHAI HOAN [18600093]");
}

void clearInfor() {
	GotoXY(WIDTH_CONSOLE / 1.5, HEIGH_CONSOLE + 1);
	printf("                                                        ");
	GotoXY(WIDTH_CONSOLE / 1.5, HEIGH_CONSOLE + 3);
	printf("                                                        ");
	GotoXY(WIDTH_CONSOLE / 5, HEIGH_CONSOLE + 3);
	printf("                                                        ");
	GotoXY(85, 0);
	printf("                                                        ");
	GotoXY(85, 2);
	printf("                                                        ");
	GotoXY(85, 4);
	printf("                                                        ");
	GotoXY(85, 6);
	printf("                                                        ");
	GotoXY(85, 8);
	printf("                                                        ");
	GotoXY(85, 9);
	printf("                                                        ");
	GotoXY(85, 11);
	printf("                                                        ");
	GotoXY(85, 13);
	printf("                                                        ");
	GotoXY(85, 15);
	printf("                                                        ");
	GotoXY(85, 17);
	printf("                                                        ");
	GotoXY(85, 19);
	printf("                                                        ");
	GotoXY(85, 21);
	printf("                                          ");
}

void SaveGame(char* filename) {
	FILE* f = fopen(filename, "wb");
	fwrite(&SIZE_SNAKE, 1, sizeof(int), f);
	fwrite(&snake, SIZE_SNAKE, sizeof(POINT), f);
	fwrite(&FOOD_INDEX, 1, sizeof(int), f);
	fwrite(&SCORE, 1, sizeof(int), f);
	fwrite(&food, MAX_SIZE_FOOD, sizeof(POINT), f);
	fwrite(&G, 1, sizeof(Gate), f);
	fwrite(&SPEED, 1, sizeof(int), f);
	fwrite(&CHAR_LOCK, 1, sizeof(char), f);
	fwrite(&MOVING, 1, sizeof(char), f);
	fclose(f);
}

bool LoadGame(char* filename) {
	FILE* f = fopen(filename, "rb");
	if (f == NULL) {
		return false;
	}
	StartGame();
	fread(&SIZE_SNAKE, 1, sizeof(int), f);
	fread(&snake, SIZE_SNAKE, sizeof(POINT), f);
	fread(&FOOD_INDEX, 1, sizeof(int), f);
	fread(&SCORE, 1, sizeof(int), f);
	fread(&food, MAX_SIZE_FOOD, sizeof(POINT), f);
	fread(&G, 1, sizeof(Gate), f);
	fread(&SPEED, 1, sizeof(int), f);
	fread(&CHAR_LOCK, 1, sizeof(char), f);
	fread(&MOVING, 1, sizeof(char), f);
	fclose(f);
	return true;
}

void ThreadFunc() {
	while (1) {
		if (STATE == 1) {
			ClearSnakeAndFood(" ");
			printInfor();
			switch (MOVING) {
			case 'A': {
				CHAR_LOCK = 'D';
				MoveLeft();
				break;
			}
			case 'D': {
				CHAR_LOCK = 'A';
				MoveRight();
				break;
			}
			case 'S': {
				CHAR_LOCK = 'W';
				MoveDown();
				break;
			}
			case 'W': {
				CHAR_LOCK = 'S';
				MoveUp();
				break;
			}
			}
			if (STATE == 1) {
				if (FOOD_INDEX == MAX_SIZE_FOOD) {
					DrawGate("x");
					DrawSnakeAndFood(" ");
				}
				else DrawSnakeAndFood(" ");
			}
			Sleep(180 / (SPEED));
		}
	}
}

void Intro() {
	//intro.txt
	ifstream files;
	ofstream file;

	char kytu;
	GotoXY(5, 5); TextColor(10);
	files.open("data/intro.txt", ios::out);
	if (files.fail()) { cout << "File not Found!"; }
	if (!files.eof()) {
		while (!files.eof()) {
			kytu = files.get();
			Sleep(5);
			cout << kytu;
		}
	}
}

void main() {
	int temp;
	char filename[50];
	ResizeConsole(1000, 520);
	FixConsoleWindow();
	TextColor(10);
	Intro();
	printf("\n\n\n\t\t\t\t\t\t       Type 'T' to LOAD GAME \n\t\t\t\t\t\t    Another key to START NEW GAME!");
	PlaySound(TEXT("start.wav"), NULL, SND_SYNC);
	int s = toupper(_getch());
	//Game
	ShowConsoleCursor(false);
	StartGame();

	thread t1(ThreadFunc);
	HANDLE handle_t1 = t1.native_handle();
	if (s == 'T') goto LoadGame;
	while (1) {
		temp = toupper(_getch());
		GotoXY(0, 23);
		if (STATE == 1) {
			if (temp == 'P') {
				PauseGame(handle_t1);
				continue;
			}
			if (temp == 'L') {
				PauseGame(handle_t1);
				clearInfor();
				ShowConsoleCursor(true);
				GotoXY(0, HEIGH_CONSOLE + 3);
				printf("ENTER A FILE NAME: "); gets_s(filename);
				SaveGame(filename);
				GotoXY(0, HEIGH_CONSOLE + 3);
				printf("                                           ");
				ShowConsoleCursor(false);
				ResumeThread(handle_t1);
			}
			else if (temp == 'T') {
			LoadGame:
				PauseGame(handle_t1);
				system("cls");
				ShowConsoleCursor(true);
				printf("ENTER A FILE NAME: "); gets_s(filename);
				while (!LoadGame(filename)) {
					printf("File not Found!\nENTER A FILE NAME: ");
					gets_s(filename);
				}
				ShowConsoleCursor(false);
				ResumeThread(handle_t1);
			}
			else if (temp == 27) {
				ExitGame(handle_t1);
				exit(0);
			}
			else {
				ResumeThread(handle_t1);
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')) {
					MOVING = temp;
				}
			}
		}
		else {
			if (temp == 89) {
				StartGame();
			}
			else if (temp == 'T') {
				goto LoadGame;
			}
			else {
				ExitGame(handle_t1);
				exit(0);
			}
		}
	}
}

/*
Game Snake v1.0 - Final Project KTLT
Information of Technology (IT) - University of Science (HCMUS)
_[_I2T_]_
*/