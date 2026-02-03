#pragma once

#include "External.h"

#include <string>

#define TERMINAL_FONT_SIZE_WIDTH 8
#define TERMINAL_FONT_SIZE_HEIGHT 16
#define TERMINAL_MAX_COLS 255
#define TERMINAL_MAX_ROWS 255
#define TERMINAL_MAX_VERTS (TERMINAL_MAX_ROWS * TERMINAL_MAX_COLS * 6)

class TerminalBuffer
{
public:
    static void Init();
    static void Clear();
    static void SetCursor(int x, int y);
    static void Write(std::string message, ...);
    static void ScrollUp();
    static const char* GetBuffer();
    static int GetCols();
    static int GetRows();
    static int GetCursorX();
    static int GetCursorY();

    /* Input line (current command being typed on last row) */
    static void SetPrompt(const std::string& prompt);
    static const std::string& GetPrompt();
    static void AppendInputChar(char c);
    static void BackspaceInput();
    static void ClearInputLine();
    static const std::string& GetInputLine();
    static void UpdateInputRow();
    static int GetInputCursorX();
    static int GetInputCursorY();
};
