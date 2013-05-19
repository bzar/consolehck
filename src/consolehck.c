#include "consolehck.h"
#include "utf8.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

unsigned int const UTF8_MAX_CHARS = 4;

consolehckConsole* consolehckConsoleNew(float const width, float const height, char const* const fontFilename)
{
  consolehckConsole* console = calloc(1, sizeof(consolehckConsole));

  console->input.input = consolehckStringBufferNew(128);
  console->input.prompt = consolehckStringBufferNew(16);
  console->output.text = consolehckStringBufferNew(1024);
  console->inputCallbacks = NULL;
  console->numInputCallbacks = 0;
  console->object = glhckPlaneNew(width, height);

  glhckTexture* consoleTexture = glhckTextureNew(NULL, NULL, NULL);
  glhckTextureCreate(consoleTexture, GLHCK_TEXTURE_2D, 0, width, height, 0, 0, GLHCK_RGBA, GLHCK_DATA_UNSIGNED_BYTE, 0, NULL);
  glhckTextureParameter(consoleTexture, glhckTextureDefaultParameters());
  glhckMaterial* consoleMaterial = glhckMaterialNew(consoleTexture);
  glhckTextureFree(consoleTexture);
  glhckObjectMaterial(console->object, consoleMaterial);
  glhckMaterialFree(consoleMaterial);

  console->text = glhckTextNew(1024,1024);
  glhckTextColorb(console->text, 192, 192, 192, 255);
  console->fontId = glhckTextNewFont(console->text, fontFilename);
  console->fontSize = 14;

  return console;
}

void consolehckConsoleFree(consolehckConsole* console)
{
  consolehckStringBufferFree(console->input.input);
  consolehckStringBufferFree(console->input.prompt);
  consolehckStringBufferFree(console->output.text);
  free(console->inputCallbacks);
  glhckObjectFree(console->object);
  free(console);
}


void consolehckConsoleUpdate(consolehckConsole* console)
{
  glhckTexture* consoleTexture = glhckMaterialGetTexture(glhckObjectGetMaterial(console->object));
  int width, height;
  glhckTextureGetInformation(consoleTexture, NULL, &width, &height, NULL, NULL, NULL, NULL);

  glhckFramebuffer* const frameBuffer = glhckFramebufferNew(GLHCK_FRAMEBUFFER_DRAW);
  glhckFramebufferRecti(frameBuffer, 0, 0, width, height);
  glhckFramebufferAttachTexture(frameBuffer, consoleTexture, GLHCK_COLOR_ATTACHMENT0);

  glhckFramebufferBegin(frameBuffer);

  glhckColorb const previousClearColor = *glhckRenderGetClearColor();
  glhckRenderClearColorb(64, 64, 64, 255);
  glhckRenderClear(GLHCK_COLOR_BUFFER);
  glhckRenderClearColor(&previousClearColor);

  glhckTextClear(console->text);

  /* Work through the character data backwards and find newline-separated lines.
   * For each line determine if wrapping is required. If no wrapping is required, render the line.
   * If wrapping is required, find the last non-rendered wrap-line within the line and render them
   * until the entire line is rendered.
   */
  unsigned int const numVisibleLines = height / console->fontSize + (height % console->fontSize == 0 ? 0 : 1);
  unsigned int lineStart = console->output.text->length;
  unsigned int lineLength = 0;
  unsigned int currentLine = 1;
  while(numVisibleLines > currentLine && lineStart > 0)
  {
    lineLength = 0;
    --lineStart;

    // Find the next line of text in raw char data
    while(lineStart > 0 && console->output.text->data[lineStart - 1] != '\n')
    {
      --lineStart;
      ++lineLength;
    }

    // Skip rendering empty lines
    if(lineLength == 0)
    {
      ++currentLine;
      continue;
    }

    // Copy line to a null-terminated char array for processing
    char* const line = calloc(lineLength + 1, 1);
    memcpy(line, console->output.text->data + lineStart, lineLength);
    line[lineLength] = '\0';

    kmVec2 minv, maxv;
    glhckTextGetMinMax(console->text, console->fontId, console->fontSize, line, &minv, &maxv);

    if(maxv.x <= width)
    {
      // No wrapping required
      float lineY = height - currentLine * console->fontSize;
      glhckTextStash(console->text, console->fontId, console->fontSize, 0, lineY, line, 0);
      ++currentLine;
    }
    else
    {
      // Wrapping required
      consolehckStringBuffer* const buffer = consolehckStringBufferNew(lineLength);

      // Until all wrap-lines rendered
      while(numVisibleLines > currentLine && lineLength > 0)
      {
        // Find the last non-rendered wrap-line
        unsigned int linePosition;
        for(linePosition = 0; linePosition < lineLength; ++linePosition)
        {
          consolehckStringBufferPushChar(buffer, line[linePosition]);
          glhckTextGetMinMax(console->text, console->fontId, console->fontSize, buffer->data, &minv, &maxv);

          if(maxv.x > width)
          {
            consolehckStringBufferClear(buffer);
            --linePosition;
          }
        }

        // Render wrap-line
        float const lineY = height - currentLine * console->fontSize;
        glhckTextStash(console->text, console->fontId, console->fontSize, 0, lineY, buffer->data, NULL);
        ++currentLine;
        lineLength -= buffer->length;
        consolehckStringBufferClear(buffer);
      }

      consolehckStringBufferFree(buffer);
    }

    free(line);
  }

  float promptRight;
  float const inputY = height;
  glhckTextStash(console->text, console->fontId, console->fontSize, 0, inputY, console->input.prompt->data, &promptRight);
  glhckTextStash(console->text, console->fontId, console->fontSize, promptRight, inputY, console->input.input->data, NULL);

  glhckTextRender(console->text);

  glhckFramebufferEnd(frameBuffer);
  glhckFramebufferFree(frameBuffer);
}

void consolehckConsoleFont(consolehckConsole* console, char const* filename)
{
  console->fontId = glhckTextNewFont(console->text, filename);
}

void consolehckConsoleFontSize(consolehckConsole* console, unsigned int const fontSize)
{
  console->fontSize = fontSize;
}


void consolehckConsoleOutputChar(consolehckConsole* console, char const c)
{
  consolehckStringBufferPushChar(console->output.text, c);
}

void consolehckConsoleOutputUnicodeChar(consolehckConsole* console, unsigned int const c)
{
  consolehckStringBufferPushUnicodeChar(console->output.text, c);
}

void consolehckConsoleOutputString(consolehckConsole* console, char const* c)
{
  consolehckStringBufferPushString(console->output.text, c);
}

void consolehckConsoleOutputUnicodeString(consolehckConsole* console, unsigned int const* c)
{
  consolehckStringBufferPushUnicodeString(console->output.text, c);
}

void consolehckConsoleInputClear(consolehckConsole* console)
{
  consolehckStringBufferClear(console->input.input);
}

void consolehckConsoleInputChar(consolehckConsole* console, char const c)
{
  consolehckStringBufferPushChar(console->input.input, c);
}

void consolehckConsoleInputUnicodeChar(consolehckConsole* console, unsigned int const c)
{
  consolehckStringBufferPushUnicodeChar(console->input.input, c);
}

void consolehckConsoleInputString(consolehckConsole* console, char const* c)
{
  consolehckStringBufferPushString(console->input.input, c);
}

void consolehckConsoleInputUnicodeString(consolehckConsole* console, unsigned int const* c)
{
  consolehckStringBufferPushUnicodeString(console->input.input, c);
}

void consolehckConsoleInputPropmt(consolehckConsole* console, char const* c)
{
  consolehckStringBufferClear(console->input.prompt);
  consolehckStringBufferPushString(console->input.prompt, c);
}

void consolehckConsolePropmtUnicode(consolehckConsole* console, unsigned int const* c)
{
  consolehckStringBufferClear(console->input.prompt);
  consolehckStringBufferPushUnicodeString(console->input.prompt, c);
}

void consolehckConsoleInputEnter(consolehckConsole* console)
{
  unsigned int i;
  for(i = 0; i < console->numInputCallbacks; ++i)
  {
    (console->inputCallbacks[i])(console, console->input.input->data);
  }
}

void consolehckConsoleInputCallbackRegister(consolehckConsole* console, consolehckInputCallback callback)
{
  consolehckInputCallback* old = console->inputCallbacks;
  console->inputCallbacks = calloc(console->numInputCallbacks + 1, sizeof(consolehckInputCallback));
  if(old != NULL)
  {
    memccpy(console->inputCallbacks, old, console->numInputCallbacks, sizeof(consolehckInputCallback));
  }
  console->inputCallbacks[console->numInputCallbacks] = callback;
  console->numInputCallbacks += 1;
}

consolehckStringBuffer* consolehckStringBufferNew(unsigned int const initialSize)
{
  consolehckStringBuffer* const buffer = calloc(1, sizeof(consolehckStringBuffer));
  buffer->bufferSize = initialSize;
  buffer->data = calloc(buffer->bufferSize, 1);
  memset(buffer->data, 0, buffer->bufferSize);
  buffer->length = 0;

  return buffer;
}

void consolehckStringBufferFree(consolehckStringBuffer* buffer)
{
  free(buffer->data);
  buffer->data = NULL;
  buffer->bufferSize = 0;
  buffer->length = 0;
  free(buffer);
}

consolehckStringBuffer* consolehckStringBufferCopy(consolehckStringBuffer const* buffer)
{
  consolehckStringBuffer* const copy = consolehckStringBufferNew(buffer->bufferSize);
  memcpy(copy->data, buffer->data, buffer->length);
  copy->length = buffer->length;

  return copy;
}

void consolehckStringBufferResize(consolehckStringBuffer* buffer, unsigned int const newSize)
{
  unsigned int const oldLength = buffer->length;
  char* oldData = buffer->data;

  buffer->data = calloc(newSize, 1);
  buffer->bufferSize = newSize;
  buffer->length = newSize > oldLength ? oldLength : newSize - 1;

  memset(buffer->data, 0, buffer->bufferSize);
  memcpy(buffer->data, oldData, buffer->length);
  free(oldData);
}

void consolehckStringBufferClear(consolehckStringBuffer *buffer)
{
  memset(buffer->data, 0, buffer->bufferSize);
  buffer->length = 0;
}

void consolehckStringBufferPushChar(consolehckStringBuffer* buffer, char const c)
{
  if(buffer->bufferSize <= buffer->length + 1)
  {
    consolehckStringBufferResize(buffer, buffer->bufferSize * 2);
  }

  buffer->data[buffer->length] = c;
  buffer->length += 1;
  buffer->data[buffer->length] = '\0';
}

void consolehckStringBufferPushUnicodeChar(consolehckStringBuffer* buffer, unsigned int const c)
{
  char chars[UTF8_MAX_CHARS + 1];
  int numChars = utf8Encode(c, chars, UTF8_MAX_CHARS);
  chars[numChars] = '\0';
  consolehckStringBufferPushString(buffer, chars);
}

void consolehckStringBufferPushString(consolehckStringBuffer* buffer, char const* c)
{
  int const num = strlen(c);
  if(buffer->bufferSize <= buffer->length + num)
  {
    unsigned int newSize = buffer->bufferSize;
    while(newSize <= buffer->length + num)
    {
      newSize *= 2;
    }
    consolehckStringBufferResize(buffer, buffer->bufferSize * 2);
  }

  memcpy(buffer->data + buffer->length, c, num);
  buffer->length += num;
  buffer->data[buffer->length] = '\0';
}

void consolehckStringBufferPushUnicodeString(consolehckStringBuffer* buffer, unsigned int const* c)
{
  unsigned int const* p = c;
  unsigned int encodedLength = 0;
  while(*p != 0)
  {
    encodedLength += utf8EncodedLength(*p);
    ++p;
  }

  char* const chars = calloc(encodedLength, 1);

  p = c;
  unsigned int pos = 0;
  while(*p != 0)
  {
    pos += utf8Encode(*p, chars + pos, UTF8_MAX_CHARS);
    ++p;
  }

  consolehckStringBufferPushString(buffer, chars);
}

char consolehckStringBufferPopChar(consolehckStringBuffer* buffer)
{
  if(buffer->length == 0)
  {
    return '\0';
  }

  buffer->length -= 1;
  char const result = buffer->data[buffer->length];
  buffer->data[buffer->length] = '\0';

  return result;
}
