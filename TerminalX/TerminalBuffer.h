#pragma once

#include "External.h"

#include <string>

#define TERMINAL_FONT_SIZE_WIDTH 8
#define TERMINAL_FONT_SIZE_HEIGHT 16
#define TERMINAL_MAX_COLS 255
#define TERMINAL_MAX_ROWS 255
#define TERMINAL_MAX_VERTS (TERMINAL_MAX_ROWS * TERMINAL_MAX_COLS * 6)
/** Max scrollback lines (for TYPE 64K and Page Up/Down) */
#define TERMINAL_SCROLLBACK_MAX_ROWS 1024

class TerminalBuffer
{
public:
    static void Init();
    static void Clear();
    static void SetCursor(int x, int y);
    static void Write(std::string message, ...);
    /** Write a string in full (no 1024-char limit). Use for command output (e.g. TYPE). */
    static void WriteRaw(const std::string& s);
    static void ScrollUp();
    /** Page Up / Page Down scrollback. Call when user presses those keys. */
    static void ScrollPageUp();
    static void ScrollPageDown();
    /** Scroll to bottom (e.g. when user starts typing). */
    static void ScrollToBottom();
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
    /** Set the entire input line (e.g. for command history). Truncates to fit. Cursor placed at end. */
    static void SetInputLine(const std::string& s);
    /** Cursor position within the input line (0..length). */
    static int GetInputCursorPos();
    /** Replace input segment [start, end) with replacement and place cursor after it. */
    static void ReplaceInputRange(int start, int end, const std::string& replacement);
    static const std::string& GetInputLine();
    /** Move cursor left/right within the input line (for editing). */
    static void MoveInputCursorLeft();
    static void MoveInputCursorRight();
    static void UpdateInputRow();
    static int GetInputCursorX();
    static int GetInputCursorY();

    /* COLOR command: attribute byte (high nibble = background, low = foreground), default 0x0A */
    static void SetColorAttribute(unsigned char attr);
    static void ResetColorAttribute();
    static unsigned char GetColorAttribute();
    static unsigned int GetTextColor();
    static unsigned int GetBackgroundColor();
};
