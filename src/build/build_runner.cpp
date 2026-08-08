/*!
 *\file build_runner.cpp
 *\brief gcc build and program execution helpers.
 */
#include "build/build_runner.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <stdio.h>
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <stdio.h>
#define POPEN popen
#define PCLOSE pclose
#endif

namespace Cgen
{
   namespace
   {
      BuildResult RunCommand(std::string_view command)
      {
         BuildResult result;
         result.command = std::string(command);
         FILE* pPipe = POPEN(result.command.c_str(), "r");
         if (pPipe == nullptr)
         {
            result.result = Result::IoError;
            result.output = "Failed to start process.\n";
            result.exitCode = -1;
            return result;
         }

         std::array<char, 512> buffer {};
         while (fgets(buffer.data(), static_cast<int>(buffer.size()), pPipe) != nullptr)
         {
            result.output.append(buffer.data());
         }
         const int closeCode = PCLOSE(pPipe);
         if (closeCode == -1)
         {
            result.result = Result::Error;
            result.exitCode = -1;
            return result;
         }
#ifdef _WIN32
         result.exitCode = closeCode;
#else
         if (WIFEXITED(closeCode))
         {
            result.exitCode = WEXITSTATUS(closeCode);
         }
         else
         {
            result.exitCode = -1;
         }
#endif
         if (result.exitCode != 0)
         {
            result.result = Result::Error;
         }
         else
         {
            result.result = Result::Ok;
         }
         return result;
      }
   } // namespace

   BuildRunner::BuildRunner(std::string_view outputDirectory)
      : _outputDirectory(outputDirectory)
   {
      std::filesystem::create_directories(_outputDirectory);
      const std::filesystem::path root(_outputDirectory);
      _sourcePath = (root / "generated.c").string();
#ifdef _WIN32
      _executablePath = (root / "program.exe").make_preferred().string();
#else
      _executablePath = (root / "program").string();
#endif
   }

   Result BuildRunner::WriteSource(std::string_view source)
   {
      std::filesystem::create_directories(_outputDirectory);
      std::ofstream output(_sourcePath, std::ios::binary);
      if (!output.is_open())
      {
         return Result::IoError;
      }
      output.write(source.data(), static_cast<std::streamsize>(source.size()));
      if (!output.good())
      {
         return Result::IoError;
      }
      return Result::Ok;
   }

   BuildResult BuildRunner::Compile(void)
   {
      const std::filesystem::path exePath(_executablePath);
      const std::filesystem::path sourcePath(_sourcePath);
      std::ostringstream command;
      command << "gcc -std=c99 -Wall -Wextra -O0 -o \"" << exePath.string() << "\" \""
              << sourcePath.string() << "\" 2>&1";
      return RunCommand(command.str());
   }

   BuildResult BuildRunner::Run(void)
   {
      const std::filesystem::path exePath =
         std::filesystem::absolute(std::filesystem::path(_executablePath)).make_preferred();
      std::ostringstream command;
#ifdef _WIN32
      command << "\"" << exePath.string() << "\" 2>&1";
#else
      command << "\"" << exePath.string() << "\" 2>&1";
#endif
      return RunCommand(command.str());
   }

   const std::string& BuildRunner::GetSourcePath(void) const
   {
      return _sourcePath;
   }

   const std::string& BuildRunner::GetExecutablePath(void) const
   {
      return _executablePath;
   }
} // namespace Cgen
