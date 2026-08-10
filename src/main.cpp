/*!
 *\file main.cpp
 *\brief Entry point for Graphical C Code Generator.
 */
#include "cli/command_line.h"
#include "gui/app.h"

int main(int argc, char** pArgv)
{
   if (argc <= 1)
   {
      Cgen::App app;
      return app.Run();
   }
   return Cgen::RunCommandLine(argc, pArgv);
}
