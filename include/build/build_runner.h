/*!
 *\file build_runner.h
 *\brief Compile and run generated C with gcc.
 */
#ifndef BUILD_RUNNER_H
#define BUILD_RUNNER_H

#include <cstdint>
#include <string>
#include <string_view>

#include "model/result.h"

namespace Cgen
{
   /*!
    *\brief Result of a build or run invocation.
    */
   struct BuildResult
   {
      Result result = Result::Ok;
      int32_t exitCode = 0;
      std::string output;
      std::string command;
   };

   /*!
    *\brief Writes source to disk, compiles with gcc, optionally runs the binary.
    */
   class BuildRunner
   {
   public:
      /*!
       *\brief Constructs a runner rooted at an output directory.
       *
       *\param[in] outputDirectory Directory for .c and executable artifacts.
       */
      explicit BuildRunner(std::string_view outputDirectory);

      /*!
       *\brief Sets output artifact names from a .cgen path or bare base name.
       *
       * Produces `<base>.c` and `<base>` / `<base>.exe` under the output directory.
       * If \p cgenPathOrBase is empty, uses "untitled".
       *
       *\param[in] cgenPathOrBase Path ending in .cgen, or a bare stem.
       */
      void SetArtifactBaseName(std::string_view cgenPathOrBase);

      /*!
       *\brief Writes C source to the configured .c path.
       *
       *\param[in] source C source text.
       *\return Result code.
       */
      Result WriteSource(std::string_view source);

      /*!
       *\brief Compiles the configured .c file with gcc.
       *
       *\return Build result including compiler log.
       */
      BuildResult Compile(void);

      /*!
       *\brief Runs the compiled program and captures stdout/stderr.
       *
       *\return Run result including program log.
       */
      BuildResult Run(void);

      /*!
       *\brief Returns the generated C file path.
       */
      const std::string& GetSourcePath(void) const;

      /*!
       *\brief Returns the executable path.
       */
      const std::string& GetExecutablePath(void) const;

      /*!
       *\brief Runs clang-format on the written source when available.
       *
       *\return Result::Ok when formatted or clang-format is missing; IoError on failure.
       */
      Result TryClangFormatSource(void);

   private:
      void RefreshPaths(void);

      std::string _outputDirectory;
      std::string _baseName;
      std::string _sourcePath;
      std::string _executablePath;
   };
} // namespace Cgen

#endif // BUILD_RUNNER_H
