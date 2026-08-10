/*!
 *\file command_line.h
 *\brief Headless CLI modes (codegen, self-test, help).
 */
#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <cstdint>

namespace Cgen
{
   /*!
    *\brief Runs CLI handling when the process was started with arguments.
    *
    * Supports --help, --self-test, and --codegen with optional --compile or --run.
    *
    *\param[in] argc Argument count (must be greater than 1).
    *\param[in] pArgv Argument vector.
    *\return Process exit code.
    */
   int32_t RunCommandLine(int32_t argc, char** pArgv);
} // namespace Cgen

#endif // COMMAND_LINE_H
