#include "consolehck.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

consolehckConsole* consolehckConsoleNew(float const width, float const height)
{
  consolehckConsole* console = malloc(sizeof(consolehckConsole));

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
  console->fontId = glhckTextNewFont(console->text, "test/fonts/DejaVuSans.ttf");
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

  glhckFramebuffer* frameBuffer = glhckFramebufferNew(GLHCK_FRAMEBUFFER_DRAW);
  glhckFramebufferRecti(frameBuffer, 0, 0, width, height);
  glhckFramebufferAttachTexture(frameBuffer, consoleTexture, GLHCK_COLOR_ATTACHMENT0);

  glhckFramebufferBegin(frameBuffer);

  glhckColorb previousClearColor = *glhckRenderGetClearColor();
  glhckRenderClearColorb(64, 64, 64, 255);
  glhckRenderClear(GLHCK_COLOR_BUFFER);
  glhckRenderClearColor(&previousClearColor);

  /* Work through the character data backwards and find newline-separated lines.
   * For each line determine if wrapping is required. If no wrapping is required, render the line.
   * If wrapping is required, find the last non-rendered wrap-line within the line and render them
   * until the entire line is rendered.
   */
  unsigned int numVisibleLines = height / console->fontSize;
  unsigned int linePosition = console->output.text->length;
  unsigned int lineLength = 0;
  unsigned int currentLine = 1;
  while(numVisibleLines > currentLine && linePosition > 0)
  {
    // Find the next line of text in raw char data
    lineLength = 0;
    --linePosition;
    while(linePosition > 0 && console->output.text->data[linePosition - 1] != '\n')
    {
      --linePosition;
      ++lineLength;
    }

    // Skip rendering empty lines
    if(lineLength == 0)
    {
      ++currentLine;
      continue;
    }

    // Copy line to a null-terminated char array for processing
    char* line = malloc(lineLength + 1);
    memcpy(line, console->output.text->data + linePosition, lineLength);
    line[lineLength] = '\0';

    kmVec2 minv, maxv;
    glhckTextGetMinMax(console->text, console->fontId, console->fontSize, line, &minv, &maxv);

    if(maxv.x <= width)
    {
      // No wrapping required
      printf("-- %i: %s\n\n", currentLine, line);
      float lineY = height - currentLine * console->fontSize;
      glhckTextStash(console->text, console->fontId, console->fontSize, 0, lineY, line, 0);
      glhckTextRender(console->text);
      ++currentLine;
    }
    else
    {
      // Wrapping required
      consolehckStringBuffer* buffer = consolehckStringBufferNew(lineLength);

      // Until all wrap-lines rendered
      while(lineLength > 0)
      {
        // Find the last non-rendered wrap-line
        unsigned int i;
        for(i = 0; i < lineLength; ++i)
        {
          consolehckStringBufferPushChar(buffer, line[i]);
          glhckTextGetMinMax(console->text, console->fontId, console->fontSize, buffer->data, &minv, &maxv);

          if(maxv.x > width)
          {
            consolehckStringBufferClear(buffer);
            --i;
          }
        }

        // Render wrap-line
        float lineY = height - currentLine * console->fontSize;
        printf("-- %i: %s\n\n", currentLine, buffer->data);
        glhckTextStash(console->text, console->fontId, console->fontSize, 0, lineY, buffer->data, NULL);
        glhckTextRender(console->text);
        ++currentLine;
        lineLength -= buffer->length;
        consolehckStringBufferClear(buffer);
      }

      consolehckStringBufferFree(buffer);
    }

    free(line);
  }

  glhckFramebufferEnd(frameBuffer);
  glhckFramebufferFree(frameBuffer);
}


void consolehckConsoleOutputChar(consolehckConsole* console, char const c)
{
  consolehckStringBufferPushChar(console->output.text, c);
}

void consolehckConsoleOutputString(consolehckConsole* console, char const* c)
{
  consolehckStringBufferPushString(console->output.text, c);
}


void consolehckConsoleInputChar(consolehckConsole* console, char const c)
{
  consolehckStringBufferPushChar(console->input.input, c);
}

void consolehckConsoleInputString(consolehckConsole* console, char const* c)
{
  consolehckStringBufferPushString(console->input.input, c);
}


void consolehckConsoleInputEnter(consolehckConsole* console)
{

}

void consolehckConsoleInputCallbackRegister(consolehckConsole* console, consolehckInputCallback callback)
{

}

consolehckStringBuffer* consolehckStringBufferNew(unsigned int const initialSize)
{
  consolehckStringBuffer* buffer = malloc(sizeof(consolehckStringBuffer));
  buffer->bufferSize = initialSize;
  buffer->data = malloc(buffer->bufferSize);
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
  consolehckStringBuffer* copy = consolehckStringBufferNew(buffer->bufferSize);
  memcpy(copy->data, buffer->data, buffer->length);
  copy->length = buffer->length;

  return copy;
}

void consolehckStringBufferResize(consolehckStringBuffer* buffer, unsigned int const newSize)
{
  unsigned int oldLength = buffer->length;
  char* oldData = buffer->data;

  buffer->data = malloc(newSize);
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

void consolehckStringBufferPushString(consolehckStringBuffer* buffer, char const* c)
{
  int num = strlen(c);
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

char consolehckStringBufferPopChar(consolehckStringBuffer* buffer)
{
  if(buffer->length == 0)
  {
    return '\0';
  }

  buffer->length -= 1;
  char result = buffer->data[buffer->length];
  buffer->data[buffer->length] = '\0';

  return result;
}
