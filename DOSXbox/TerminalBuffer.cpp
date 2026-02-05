#include "TerminalBuffer.h"
#include "Drawing.h"

namespace
{
    char* s_buffer = NULL;
    char* s_baseBuffer = NULL;
    char* s_scrollback = NULL;
    int s_allocRows = 0;
    int s_allocCols = 0;
    int s_cursor_x = 0;
    int s_cursor_y = 0;
    int s_scrollbackCount = 0;
    int s_scrollbackStart = 0;
    int s_scrollOffset = 0;
    std::string s_prompt = "C:\\> ";
    std::string s_inputLine;
    int s_inputCursorPos = 0;  /* position within input line (0..length) */
    unsigned char s_colorAttr = 0x0A;
    unsigned char s_colorAttrDefault = 0x0A;

    static const unsigned int s_colorTable[16] =
    {
        0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
        0xFFAA0000, 0xFFAA00AA, 0xFFAAAA00, 0xFFAAAAAA,
        0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
        0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
    };
}

void TerminalBuffer::SetColorAttribute(unsigned char attr)
{
    s_colorAttr = attr;
}

void TerminalBuffer::ResetColorAttribute()
{
    s_colorAttr = s_colorAttrDefault;
}

unsigned char TerminalBuffer::GetColorAttribute()
{
    return s_colorAttr;
}

unsigned int TerminalBuffer::GetTextColor()
{
    return s_colorTable[s_colorAttr & 0x0F];
}

unsigned int TerminalBuffer::GetBackgroundColor()
{
    return s_colorTable[(s_colorAttr >> 4) & 0x0F];
}

void TerminalBuffer::Init()
{
    int rows = GetRows();
    int cols = GetCols();
    if (rows <= 0 || cols <= 0)
    {
        return;
    }
    if (s_buffer != NULL && s_allocRows == rows && s_allocCols == cols)
    {
        return;
    }
    if (s_buffer != NULL)
    {
        free(s_buffer);
        free(s_baseBuffer);
        free(s_scrollback);
    }
    s_allocRows = rows;
    s_allocCols = cols;
    s_buffer = (char*)malloc((size_t)(rows * cols));
    s_baseBuffer = (char*)malloc((size_t)(rows * cols));
    s_scrollback = (char*)malloc((size_t)(TERMINAL_SCROLLBACK_MAX_ROWS * TERMINAL_MAX_COLS));
    s_scrollbackCount = 0;
    s_scrollbackStart = 0;
    s_scrollOffset = 0;
}

void TerminalBuffer::Clear()
{
    Init();
    int rows = GetRows();
    int cols = GetCols();
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            s_baseBuffer[(row * cols) + col] = ' ';
        }
    }
    s_cursor_x = 0;
    s_cursor_y = 0;
    s_scrollbackCount = 0;
    s_scrollbackStart = 0;
    s_scrollOffset = 0;
}

void TerminalBuffer::SetCursor(int x, int y)
{
    s_cursor_x = (x >= 0 && x < GetCols()) ? x : 0;
    s_cursor_y = (y >= 0 && y < GetRows()) ? y : 0;
}

void TerminalBuffer::Write(std::string message, ...)
{
    Init();

    char buffer[1024];
    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, 1024, message.c_str(), arglist);
    va_end(arglist);
    buffer[1024 - 1] = '\0';

    for (const char* p = buffer; *p; p++)
    {
        if (*p == '\n')
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
            continue;
        }

        if (s_cursor_x >= GetCols())
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
        }

        if (s_cursor_y >= 0 && s_cursor_y < GetRows() && s_cursor_x >= 0 && s_cursor_x < GetCols())
        {
            s_baseBuffer[(s_cursor_y * GetCols()) + s_cursor_x] = *p;
        }
        s_cursor_x++;
    }
}

void TerminalBuffer::WriteRaw(const std::string& s)
{
    Init();
    for (size_t i = 0; i < s.size(); i++)
    {
        char c = s[i];
        if (c == '\n')
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
            continue;
        }

        if (s_cursor_x >= GetCols())
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
        }

        if (s_cursor_y >= 0 && s_cursor_y < GetRows() && s_cursor_x >= 0 && s_cursor_x < GetCols())
        {
            s_baseBuffer[(s_cursor_y * GetCols()) + s_cursor_x] = c;
        }
        s_cursor_x++;
    }
}

void TerminalBuffer::ScrollUp()
{
    Init();
    int rows = GetRows();
    int cols = GetCols();
    if (rows == 0 || cols == 0)
    {
        return;
    }
    /* Push top row of base buffer into scrollback ring */
    int cap = TERMINAL_SCROLLBACK_MAX_ROWS;
    int phys = (s_scrollbackStart + s_scrollbackCount) % cap;
    int copyCols = (cols < TERMINAL_MAX_COLS) ? cols : TERMINAL_MAX_COLS;
    for (int col = 0; col < copyCols; col++)
    {
        s_scrollback[phys * TERMINAL_MAX_COLS + col] = s_baseBuffer[0 * cols + col];
    }
    for (int col = copyCols; col < TERMINAL_MAX_COLS; col++)
    {
        s_scrollback[phys * TERMINAL_MAX_COLS + col] = ' ';
    }
    if (s_scrollbackCount >= cap)
    {
        s_scrollbackStart = (s_scrollbackStart + 1) % cap;
    }
    else
    {
        s_scrollbackCount++;
    }
    /* Shift base buffer up */
    for (int row = 0; row < rows - 1; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            s_baseBuffer[(row * cols) + col] = s_baseBuffer[((row + 1) * cols) + col];
        }
    }
    for (int col = 0; col < cols; col++)
    {
        s_baseBuffer[((rows - 1) * cols) + col] = ' ';
    }
}

void TerminalBuffer::ScrollPageUp()
{
    Init();
    int pageRows = GetRows() - 1;
    if (pageRows <= 0)
    {
        return;
    }
    int maxOffset = s_scrollbackCount;
    s_scrollOffset += pageRows;
    if (s_scrollOffset > maxOffset)
    {
        s_scrollOffset = maxOffset;
    }
}

void TerminalBuffer::ScrollPageDown()
{
    int pageRows = GetRows() - 1;
    if (pageRows <= 0)
    {
        return;
    }
    s_scrollOffset -= pageRows;
    if (s_scrollOffset < 0)
    {
        s_scrollOffset = 0;
    }
}

void TerminalBuffer::ScrollToBottom()
{
    s_scrollOffset = 0;
}

static void RefreshViewBuffer()
{
    int rows = TerminalBuffer::GetRows();
    int cols = TerminalBuffer::GetCols();
    if (s_scrollOffset == 0)
    {
        for (int i = 0; i < rows * cols; i++)
        {
            s_buffer[i] = s_baseBuffer[i];
        }
        return;
    }
    int cap = TERMINAL_SCROLLBACK_MAX_ROWS;
    /* Top scrollOffset rows from scrollback (newest first) */
    for (int r = 0; r < s_scrollOffset && r < rows; r++)
    {
        int logical = s_scrollbackCount - s_scrollOffset + r;
        if (logical < 0)
        {
            for (int col = 0; col < cols; col++)
            {
                s_buffer[(r * cols) + col] = ' ';
            }
        }
        else
        {
            int phys = (s_scrollbackStart + logical) % cap;
            int copyCols = (cols < TERMINAL_MAX_COLS) ? cols : TERMINAL_MAX_COLS;
            for (int col = 0; col < copyCols; col++)
            {
                s_buffer[(r * cols) + col] = s_scrollback[phys * TERMINAL_MAX_COLS + col];
            }
            for (int col = copyCols; col < cols; col++)
            {
                s_buffer[(r * cols) + col] = ' ';
            }
        }
    }
    /* Content rows from base buffer (rows 0..rows-2), then input row (rows-1) */
    int contentFromBase = rows - 1 - s_scrollOffset;
    if (contentFromBase > 0)
    {
        for (int r = 0; r < contentFromBase; r++)
        {
            for (int col = 0; col < cols; col++)
            {
                s_buffer[(s_scrollOffset + r) * cols + col] = s_baseBuffer[r * cols + col];
            }
        }
    }
    /* Last row is always the input row from base buffer */
    for (int col = 0; col < cols; col++)
    {
        s_buffer[(rows - 1) * cols + col] = s_baseBuffer[(rows - 1) * cols + col];
    }
}

const char* TerminalBuffer::GetBuffer()
{
    Init();
    RefreshViewBuffer();
    return s_buffer;
}

int TerminalBuffer::GetCols()
{
    return Drawing::GetBufferWidth() / TERMINAL_FONT_SIZE_WIDTH;
}

int TerminalBuffer::GetRows()
{
    return Drawing::GetBufferHeight() / TERMINAL_FONT_SIZE_HEIGHT;
}

int TerminalBuffer::GetCursorX()
{
    return s_cursor_x;
}

int TerminalBuffer::GetCursorY()
{
    return s_cursor_y;
}

void TerminalBuffer::SetPrompt(const std::string& prompt)
{
    s_prompt = prompt;
}

const std::string& TerminalBuffer::GetPrompt()
{
    return s_prompt;
}

void TerminalBuffer::AppendInputChar(char c)
{
    if (c < 32 || c > 126)
    {
        return;
    }
    int cols = GetCols();
    int promptLen = (int)s_prompt.length();
    if ((int)s_inputLine.length() + promptLen >= cols)
    {
        return;
    }
    if (s_inputCursorPos >= (int)s_inputLine.length())
    {
        s_inputLine += c;
        s_inputCursorPos = (int)s_inputLine.length();
    }
    else
    {
        s_inputLine.insert((size_t)s_inputCursorPos, 1, c);
        s_inputCursorPos++;
    }
}

void TerminalBuffer::BackspaceInput()
{
    if (s_inputCursorPos > 0 && s_inputCursorPos <= (int)s_inputLine.length())
    {
        s_inputLine.erase((size_t)(s_inputCursorPos - 1), 1);
        s_inputCursorPos--;
    }
}

void TerminalBuffer::ClearInputLine()
{
    s_inputLine.clear();
    s_inputCursorPos = 0;
}

void TerminalBuffer::SetInputLine(const std::string& s)
{
    Init();
    int cols = GetCols();
    int promptLen = (int)s_prompt.length();
    int maxLen = cols - promptLen;
    if (maxLen <= 0)
    {
        s_inputLine.clear();
        return;
    }
    if ((int)s.length() <= maxLen)
    {
        s_inputLine = s;
    }
    else
    {
        s_inputLine = s.substr(0, (size_t)maxLen);
    }
    s_inputCursorPos = (int)s_inputLine.length();
}

void TerminalBuffer::MoveInputCursorLeft()
{
    if (s_inputCursorPos > 0)
    {
        s_inputCursorPos--;
    }
}

void TerminalBuffer::MoveInputCursorRight()
{
    if (s_inputCursorPos < (int)s_inputLine.length())
    {
        s_inputCursorPos++;
    }
}

const std::string& TerminalBuffer::GetInputLine()
{
    return s_inputLine;
}

void TerminalBuffer::UpdateInputRow()
{
    Init();
    int rows = GetRows();
    int cols = GetCols();
    if (rows == 0)
    {
        return;
    }
    int inputRow = rows - 1;
    std::string line = s_prompt + s_inputLine;
    for (int col = 0; col < cols; col++)
    {
        char ch = (col < (int)line.length()) ? line[col] : ' ';
        s_baseBuffer[(inputRow * cols) + col] = ch;
    }
}

int TerminalBuffer::GetInputCursorX()
{
    return (int)s_prompt.length() + s_inputCursorPos;
}

int TerminalBuffer::GetInputCursorY()
{
    int rows = GetRows();
    return (rows > 0) ? (rows - 1) : 0;
}
