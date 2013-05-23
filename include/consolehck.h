#ifndef CONSOLEHCK_H
#define CONSOLEHCK_H

#include "glhck/glhck.h"

struct consolehckConsole;
typedef void (* consolehckInputCallback)(struct consolehckConsole*, unsigned int const*);

typedef enum consolehckWrapMode {
  CONSOLEHCK_NO_WRAP, CONSOLEHCK_WRAP
} consolehckWrapMode;

typedef struct consolehckStringBuffer {
  unsigned int* data;
  unsigned int length;
  unsigned int bufferSize;
} consolehckStringBuffer;

typedef struct consolehckTextArea {
  consolehckStringBuffer* text;
} consolehckTextArea;

typedef struct consolehckInputLine {
  consolehckStringBuffer* prompt;
  consolehckStringBuffer* input;
} consolehckInputLine;

typedef struct consolehckConsole {
  consolehckTextArea output;
  consolehckInputLine input;
  consolehckInputCallback* inputCallbacks;
  unsigned int numInputCallbacks;

  glhckText* text;
  unsigned int fontId;
  unsigned int fontSize;
  float margin;
  glhckObject* object;
} consolehckConsole;

consolehckConsole* consolehckConsoleNew(float const width, float const height);
void consolehckConsoleFree(consolehckConsole* console);

void consolehckConsoleUpdate(consolehckConsole* console);
void consolehckConsoleFont(consolehckConsole* console, char const* filename);
void consolehckConsoleFontSize(consolehckConsole* console, const unsigned int fontSize);

void consolehckConsoleOutputChar(consolehckConsole* console, char const c);
void consolehckConsoleOutputUnicodeChar(consolehckConsole* console, unsigned int const c);
void consolehckConsoleOutputString(consolehckConsole* console, char const* c);
void consolehckConsoleOutputUnicodeString(consolehckConsole* console, unsigned int const* c);

void consolehckConsoleInputClear(consolehckConsole* console);
void consolehckConsoleInputChar(consolehckConsole* console, char const c);
void consolehckConsoleInputUnicodeChar(consolehckConsole* console, unsigned int const c);
void consolehckConsoleInputString(consolehckConsole* console, char const* c);
void consolehckConsoleInputUnicodeString(consolehckConsole* console, unsigned int const* c);
char consolehckConsoleInputPopChar(consolehckConsole* console);
unsigned int consolehckConsoleInputPopUnicodeChar(consolehckConsole* console);

void consolehckConsolePropmt(consolehckConsole* console, char const* c);
void consolehckConsolePropmtUnicode(consolehckConsole* console, unsigned int const* c);

void consolehckConsoleInputEnter(consolehckConsole* console);
void consolehckConsoleInputCallbackRegister(consolehckConsole* console, consolehckInputCallback callback);


consolehckStringBuffer *consolehckStringBufferNew(unsigned int const initialSize);
void consolehckStringBufferFree(consolehckStringBuffer* buffer);
consolehckStringBuffer* consolehckStringBufferCopy(consolehckStringBuffer const* buffer);
void consolehckStringBufferResize(consolehckStringBuffer *buffer, unsigned int const newSize);
void consolehckStringBufferClear(consolehckStringBuffer *buffer);

void consolehckStringBufferPushChar(consolehckStringBuffer* buffer, char const c);
void consolehckStringBufferPushUnicodeChar(consolehckStringBuffer* buffer, unsigned int const c);
void consolehckStringBufferPushString(consolehckStringBuffer* buffer, char const* c);
void consolehckStringBufferPushUnicodeString(consolehckStringBuffer* buffer, unsigned int const* c);
char consolehckStringBufferPopChar(consolehckStringBuffer* buffer);
unsigned int consolehckStringBufferPopUnicodeChar(consolehckStringBuffer* buffer);

void consolehckTextRenderUnicode(glhckText* textObject, glhckRect const* rect, consolehckWrapMode wrapMode, unsigned int fontId, unsigned int fontSize, unsigned int const* const str);

#endif
