#include "consolehck.h"
#include "GL/glfw3.h"

#include <stdio.h>

int const WIDTH = 800;
int const HEIGHT = 480;

static char RUNNING = 1;
static void windowCloseCallback(GLFWwindow* w)
{
  (void) w;
  RUNNING = 0;
}

static void windowCharCallback(GLFWwindow* w, unsigned int c)
{
  consolehckConsole* console = glfwGetWindowUserPointer(w);
  consolehckConsoleInputUnicodeChar(console, c);
  consolehckConsoleUpdate(console);
}

static void windowKeyCallback(GLFWwindow* w, int key, int action)
{
  consolehckConsole* console = glfwGetWindowUserPointer(w);
  if(key == GLFW_KEY_ENTER && action == GLFW_PRESS)
  {
    consolehckConsoleInputEnter(console);
    consolehckConsoleUpdate(console);
  }
  else if(key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    consolehckConsoleInputPopUnicodeChar(console);
    consolehckConsoleUpdate(console);
  }
  else if(key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    consolehckConsoleOutputOffset(console, consolehckConsoleOutputGetOffset(console) + 12);
    consolehckConsoleUpdate(console);
  }
  else if(key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    consolehckConsoleOutputOffset(console, consolehckConsoleOutputGetOffset(console) - 12);
    consolehckConsoleUpdate(console);
  }
}

static consolehckContinue inputEnterCallback(consolehckConsole* console, unsigned int const* c)
{
  consolehckConsoleOutputUnicodeString(console, c);
  consolehckConsoleOutputChar(console, '\n');
  consolehckConsoleInputClear(console);
  consolehckConsoleUpdate(console);

  return CONSOLEHCK_CONTINUE;
}

static char const* const LOREM_IPSUM = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

void run(GLFWwindow* window);

int main(int argc, char** argv)
{
  GLFWwindow* window;
  if (!glfwInit())
     return -1;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  if (!(window = glfwCreateWindow(WIDTH, HEIGHT, "consolehck - simple.c", NULL, NULL)))
     return -1;

  glfwMakeContextCurrent(window);

  glfwSwapInterval(0);

  if (!glhckContextCreate(argc, argv))
     return -1;

  if (!glhckDisplayCreate(WIDTH, HEIGHT, GLHCK_RENDER_AUTO))
     return -1;

  run(window);

  glhckContextTerminate();
  glfwTerminate();

  return 0;
}

void run(GLFWwindow* window)
{
  consolehckConsole* console = consolehckConsoleNew(512, 256);
  glfwSetWindowUserPointer(window, console);
  glfwSetWindowCloseCallback(window, windowCloseCallback);
  glfwSetCharCallback(window, windowCharCallback);
  glfwSetKeyCallback(window, windowKeyCallback);
  consolehckConsoleInputCallbackRegister(console, inputEnterCallback);

  glhckObjectPositionf(console->object, WIDTH/2.0f, HEIGHT/2.0f, 0);

  consolehckConsoleInputPropmt(console, "consolehck>");
  consolehckConsoleInputString(console, "Hello Input!");
  consolehckConsoleOutputString(console, "1 Hello Output!\n");
  consolehckConsoleOutputString(console, "2 Hello Output!\n3 Hello Output!\n");
  consolehckConsoleOutputString(console, LOREM_IPSUM);
  consolehckConsoleOutputChar(console, '\n');
  consolehckConsoleUpdate(console);

  glhckRenderStatePush2D(WIDTH, HEIGHT, -1, 1);

  while(RUNNING && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
  {
    glfwPollEvents();
    glhckObjectRender(console->object);
    glfwSwapBuffers(window);
    glhckRenderClear(GLHCK_DEPTH_BUFFER | GLHCK_COLOR_BUFFER);
  }

  consolehckConsoleFree(console);
}
