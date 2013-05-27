#include "consolehck.h"
#include "utf8.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

unsigned int const UTF8_MAX_CHARS = 4;




consolehckConsole* consolehckConsoleNew(float const width, float const height)
{
  consolehckConsole* console = calloc(1, sizeof(consolehckConsole));

  console->input.input = consolehckStringBufferNew(128);
  console->input.prompt = consolehckStringBufferNew(16);
  console->output.text = consolehckStringBufferNew(1024);
  console->output.offset = 0;
  console->inputCallbacks = NULL;
  console->numInputCallbacks = 0;
  console->object = glhckPlaneNew(width, height);

  glhckTexture* consoleTexture = glhckTextureNew();
  glhckTextureCreate(consoleTexture, GLHCK_TEXTURE_2D, 0, width, height, 0, 0, GLHCK_RGBA, GLHCK_DATA_UNSIGNED_BYTE, 0, NULL);
  glhckTextureParameter(consoleTexture, glhckTextureDefaultParameters());
  glhckMaterial* consoleMaterial = glhckMaterialNew(consoleTexture);
  glhckTextureFree(consoleTexture);
  glhckObjectMaterial(console->object, consoleMaterial);
  glhckMaterialFree(consoleMaterial);

  console->text = glhckTextNew(1024,1024);
  glhckTextColorb(console->text, 192, 192, 192, 255);
  console->fontSize = 14;
  console->fontId = glhckTextFontNewKakwafont(console->text, &console->fontSize);
  console->margin = 4;

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

  kmMat4 previousProjection = *glhckRenderGetProjection();

  kmMat4 ortho;
  kmMat4OrthographicProjection(&ortho, 0, width, 0, height, -1, 1);
  glhckRenderProjectionOnly(&ortho);

  glhckColorb const previousClearColor = *glhckRenderGetClearColor();
  glhckRenderClearColorb(64, 64, 64, 255);
  glhckRenderClear(GLHCK_COLOR_BUFFER);
  glhckRenderClearColor(&previousClearColor);

  glhckTextClear(console->text);
  glhckRect rect = {console->margin, console->margin, width - console->margin * 2, height - console->margin * 2};
  consolehckTextRenderUnicode(console->text, &rect, console->output.offset, CONSOLEHCK_WRAP, console->fontId, console->fontSize, console->output.text->data);
  glhckTextRender(console->text);

  glhckObject* promptBackground = glhckPlaneNew(width, console->fontSize);
  glhckMaterial* promptBackgroundMaterial = glhckMaterialNew(NULL);
  glhckMaterialDiffuseb(promptBackgroundMaterial, 0, 0, 0, 255);
  glhckObjectMaterial(promptBackground, promptBackgroundMaterial);
  glhckMaterialFree(promptBackgroundMaterial);
  glhckObjectPositionf(promptBackground, width/2, console->fontSize/2 + console->margin, 0);
  glhckObjectRender(promptBackground);
  glhckObjectFree(promptBackground);

  glhckTextClear(console->text);
  float const inputY = height - console->margin;
  float promptRight;

  int utf8PromptLength = utf8EncodedStringLength(console->input.prompt->data);
  char* utf8Prompt = calloc(utf8PromptLength + 1, 1);
  utf8EncodeString(console->input.prompt->data, utf8Prompt);
  glhckTextStash(console->text, console->fontId, console->fontSize, console->margin, inputY, utf8Prompt, &promptRight);
  free(utf8Prompt);

  int utf8InputLength = utf8EncodedStringLength(console->input.input->data);
  char* utf8Input = calloc(utf8InputLength + 1, 1);
  utf8EncodeString(console->input.input->data, utf8Input);
  glhckTextStash(console->text, console->fontId, console->fontSize, promptRight, inputY, utf8Input, NULL);
  free(utf8Input);

  glhckTextRender(console->text);

  glhckRenderProjectionOnly(&previousProjection);

  glhckFramebufferEnd(frameBuffer);
  glhckFramebufferFree(frameBuffer);
}

void consolehckConsoleFont(consolehckConsole* console, char const* filename)
{
  console->fontId = glhckTextFontNew(console->text, filename);
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

void consolehckConsoleOutputOffset(consolehckConsole* console, int const offset)
{
  console->output.offset = offset;
}

int consolehckConsoleOutputGetOffset(consolehckConsole *console)
{
  return console->output.offset;
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

char consolehckConsoleInputPopChar(consolehckConsole* console)
{
  return consolehckStringBufferPopChar(console->input.input);
}

unsigned int consolehckConsoleInputPopUnicodeChar(consolehckConsole* console)
{
  return consolehckStringBufferPopUnicodeChar(console->input.input);
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
    if((console->inputCallbacks[i])(console, console->input.input->data) == CONSOLEHCK_STOP)
      break;
  }
}

void consolehckConsoleInputCallbackRegister(consolehckConsole* console, consolehckInputCallback callback)
{
  consolehckInputCallback* old = console->inputCallbacks;
  console->inputCallbacks = calloc(console->numInputCallbacks + 1, sizeof(consolehckInputCallback));
  if(old != NULL)
  {
    memcpy(console->inputCallbacks, old, console->numInputCallbacks * sizeof(consolehckInputCallback));
  }
  console->inputCallbacks[console->numInputCallbacks] = callback;
  console->numInputCallbacks += 1;
}

consolehckStringBuffer* consolehckStringBufferNew(unsigned int const initialSize)
{
  consolehckStringBuffer* const buffer = calloc(1, sizeof(consolehckStringBuffer));
  buffer->bufferSize = initialSize;
  buffer->data = calloc(buffer->bufferSize, sizeof(unsigned int));
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
  memcpy(copy->data, buffer->data, buffer->length * sizeof(unsigned int));
  copy->length = buffer->length;

  return copy;
}

void consolehckStringBufferResize(consolehckStringBuffer* buffer, unsigned int const newSize)
{
  unsigned int const oldLength = buffer->length;
  unsigned int* oldData = buffer->data;

  buffer->data = calloc(newSize, sizeof(unsigned int));
  buffer->bufferSize = newSize;
  buffer->length = newSize > oldLength ? oldLength : newSize - 1;

  memcpy(buffer->data, oldData, buffer->length * sizeof(unsigned int));
  free(oldData);
}

void consolehckStringBufferClear(consolehckStringBuffer *buffer)
{
  memset(buffer->data, 0, buffer->bufferSize);
  buffer->length = 0;
}

void consolehckStringBufferPushChar(consolehckStringBuffer* buffer, char const c)
{
  unsigned int codepoint;
  unsigned int state = 0;
  assert(!utf8Decode(&state, &codepoint, c));
  consolehckStringBufferPushUnicodeChar(buffer, codepoint);
}

void consolehckStringBufferPushUnicodeChar(consolehckStringBuffer* buffer, unsigned int const c)
{
  if(buffer->bufferSize <= buffer->length + 1)
  {
    consolehckStringBufferResize(buffer, buffer->bufferSize * 2);
  }

  buffer->data[buffer->length] = c;
  buffer->length += 1;
  buffer->data[buffer->length] = 0;
}

void consolehckStringBufferPushString(consolehckStringBuffer* buffer, char const* c)
{
  int numCodepoints;
  utf8CountCodePoints(c, &numCodepoints);
  unsigned int* codepoints = calloc(numCodepoints + 1, sizeof(unsigned int));
  utf8DecodeString(c, codepoints);
  codepoints[numCodepoints] = 0;
  consolehckStringBufferPushUnicodeString(buffer, codepoints);
  free(codepoints);
}

void consolehckStringBufferPushUnicodeString(consolehckStringBuffer* buffer, unsigned int const* c)
{
  int const num = unicodeStringLength(c);
  if(buffer->bufferSize <= buffer->length + num)
  {
    unsigned int newSize = buffer->bufferSize;
    while(newSize <= buffer->length + num)
    {
      newSize *= 2;
    }
    consolehckStringBufferResize(buffer, newSize);
  }

  memcpy(buffer->data + buffer->length, c, num * sizeof(unsigned int));
  buffer->length += num;
  buffer->data[buffer->length] = 0;
}

char consolehckStringBufferPopChar(consolehckStringBuffer* buffer)
{
  unsigned int const codepoint = consolehckStringBufferPopUnicodeChar(buffer);
  if(codepoint == 0)
    return '\0';

  assert(utf8EncodedLength(codepoint) == 1);
  char c;
  utf8Encode(codepoint, &c, 1);
  return c;
}

unsigned int consolehckStringBufferPopUnicodeChar(consolehckStringBuffer* buffer)
{
  if(buffer->length == 0)
  {
    return 0;
  }

  buffer->length -= 1;
  unsigned int const result = buffer->data[buffer->length];
  buffer->data[buffer->length] = 0;

  return result;
}

void consolehckTextRenderUnicode(glhckText* textObject, glhckRect const* rect, int const offset, consolehckWrapMode wrapMode, unsigned int fontId, unsigned int fontSize, unsigned int const* const str)
{
  /* Work through the character data backwards and find newline-separated lines.
   * For each line determine if wrapping is required. If no wrapping is required, render the line.
   * If wrapping is required, find the last non-rendered wrap-line within the line and render them
   * until the entire line is rendered.
   */

  int const lineOffset = offset % (int) fontSize;
  int const firstVisibleLine = offset / (int) fontSize + 1;
  unsigned int const numVisibleLines = rect->h / fontSize + 1;
  unsigned int lineStart = unicodeStringLength(str);
  unsigned int lineLength = 0;
  int currentLine = 1;

  while(numVisibleLines > currentLine && lineStart > 0)
  {
    lineLength = 0;
    --lineStart;

    // Find the next line of text in raw char data
    while(lineStart > 0 && str[lineStart - 1] != '\n')
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

    // Copy line to a null-terminated unicode array for processing
    unsigned int* const line = calloc(lineLength + 1, sizeof(unsigned int));
    memcpy(line, str + lineStart, lineLength * sizeof(unsigned int));
    line[lineLength] = 0;
    int utf8LineLength = utf8EncodedStringLength(line);
    char* const utf8Line = calloc(utf8LineLength + 1, 1);
    utf8EncodeString(line, utf8Line);
    utf8Line[utf8LineLength] = 0;

    kmVec2 minv, maxv;
    glhckTextGetMinMax(textObject, fontId, fontSize, utf8Line, &minv, &maxv);

    if(maxv.x <= rect->w || wrapMode == CONSOLEHCK_NO_WRAP)
    {
      // No wrapping required
      float lineY = rect->h - (currentLine - firstVisibleLine + 1) * fontSize + lineOffset;

      if(currentLine >= firstVisibleLine)
      {
        glhckTextStash(textObject, fontId, fontSize, rect->x, rect->y + lineY, utf8Line, 0);
      }
      ++currentLine;
    }
    else
    {
      // Until all wrap-lines rendered
      char* const utf8WrapLine = calloc(utf8LineLength + 1, 1);
      while(numVisibleLines > currentLine && lineLength > 0)
      {
        // Find the last non-rendered wrap-line
        unsigned int linePosition;
        unsigned int wrapLineLength = 0;
        unsigned int utf8WrapLineLength = 0;
        unsigned int utf8WrapLineStart = 0;

        for(linePosition = 0; linePosition < lineLength; ++linePosition)
        {
          int charLength = utf8EncodedLength(line[linePosition]);
          memcpy(utf8WrapLine + utf8WrapLineLength, utf8Line + utf8WrapLineStart + utf8WrapLineLength, charLength);
          utf8WrapLineLength += charLength;
          utf8WrapLine[utf8WrapLineLength] = '\0';
          ++wrapLineLength;

          glhckTextGetMinMax(textObject, fontId, fontSize, utf8WrapLine, &minv, &maxv);

          if(maxv.x > rect->w)
          {
            memset(utf8WrapLine, 0, utf8WrapLineLength);
            utf8WrapLineStart += utf8WrapLineLength - charLength;
            utf8WrapLineLength = 0;
            wrapLineLength = 0;
            --linePosition;
          }
        }

        // Render wrap-line
        float const lineY = rect->h - (currentLine - firstVisibleLine + 1) * fontSize + lineOffset;
        if(currentLine >= firstVisibleLine)
        {
          glhckTextStash(textObject, fontId, fontSize, rect->x, rect->y + lineY, utf8WrapLine, NULL);
        }
        ++currentLine;
        lineLength -= wrapLineLength;
      }
      free(utf8WrapLine);
    }

    free(line);
    free(utf8Line);
  }
}


