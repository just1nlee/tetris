#include <iostream>
#include <Windows.h> 
#include <thread>
#include <chrono>
#include <cstdlib>
#include <vector>

using namespace std::chrono_literals;

std::wstring tetromino[7];

int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int nScreenWidth = 80;
int nScreenHeight = 30;

int rotate(int px, int py, int r) {
	switch (r % 4) {
	case 0: return py * 4 + px;
	case 1: return 12 + py - (px * 4);
	case 2: return 15 - (py * 4) - px;
	case 3: return 3 - py + (px * 4);
	}
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {
			int pi = rotate(px, py, nRotation);

			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosY + py < nFieldHeight) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false;
				}
			}
		}
	}
	return true;
}

int main() {
	// Create assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");

	pField = new unsigned char[nFieldWidth * nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;


	// Game logic
	bool bGameOver = false;

	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHold = true;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;

	int nScore = 0;

	std::vector<int> vLines;

	while (!bGameOver) {
		// Game timing
		std::this_thread::sleep_for(50ms); // Game tick
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		// Input
		for (int k = 0; k < 4; k++) {							// R   L   D  Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		// Game logic

		// Left key
		if (bKey[1]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) {
				nCurrentX -= 1;
			}
		}

		// Right key
		if (bKey[0]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) {
				nCurrentX += 1;
			}
		}

		// Down key
		if (bKey[2]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY += 1;
			}
		}

		// Rotate key
		if (bKey[3]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) {
				nCurrentRotation += (1 && bRotateHold);
			}
			bRotateHold = false;
		}
		else {
			bRotateHold = true;

		}

		if (bForceDown) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
			else {
				// Lock current piece in field
				for (int px = 0; px < 4; px++) {
					for (int py = 0; py < 4; py++) {
						if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}
				nPieceCount++;
				if (nPieceCount % 10 == 0) {
					if (nSpeed >= 10) {
						nSpeed--;
					}
				}

				// Check if lines are full
				for (int py = 0; py < 4; py++) {
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++) {
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}
						if (bLine) {
							// Remove line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++) {
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							}
							vLines.push_back(nCurrentY + py);
						}	
					}
				}
				// Score
				nScore += 25;
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				// Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// If piece does not fit
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}

		// Render output

		// Draw field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		// Draw current piece
		for (int px = 0; px < 4; px++) {
			for (int py = 0; py < 4; py++) {
				if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X') {
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65; // ASCII for A
				}
			}
		}

		// Draw score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);
		
		if (!vLines.empty()) {
			// Display frame to draw lines
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			std::this_thread::sleep_for(400ms); // Delay to show effect to player

			for (auto& v : vLines) {
				for (int px = 1; px < nFieldWidth - 1; px++) {
					for (int py = v; py > 0; py--) {
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}

			vLines.clear();
		}
		// Display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	// Game over message
	CloseHandle(hConsole);
	std::cout << "GAME OVER! SCORE: " << nScore << "\n";
	system("PAUSE");
		
	return 0;
}