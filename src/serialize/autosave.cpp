/*!
 *\file autosave.cpp
 *\brief Crash-recovery autosave helpers.
 */
#include "serialize/autosave.h"

#include <fstream>

#include "serialize/cgen_serializer.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Cgen
{
   namespace
   {
      std::filesystem::path MetaPathForAutosave(
         const std::filesystem::path& autosavePath)
      {
         return autosavePath.string() + ".meta";
      }

      Result WriteMetaFile(const std::filesystem::path& metaPath,
                           std::string_view originalDocumentPath)
      {
         std::ofstream output(metaPath, std::ios::trunc);
         if (!output.is_open())
         {
            return Result::IoError;
         }
         output << originalDocumentPath << '\n';
         if (!output.good())
         {
            return Result::IoError;
         }
         return Result::Ok;
      }

      std::string ReadMetaFile(const std::filesystem::path& metaPath)
      {
         std::ifstream input(metaPath);
         if (!input.is_open())
         {
            return std::string();
         }
         std::string value;
         std::getline(input, value);
         return value;
      }
   } // namespace

   std::filesystem::path AutosaveDirectory(void)
   {
#ifdef _WIN32
      char appData[MAX_PATH] = {};
      const DWORD length =
         GetEnvironmentVariableA("LOCALAPPDATA", appData, MAX_PATH);
      if ((length > 0) && (length < MAX_PATH))
      {
         const std::filesystem::path directory =
            std::filesystem::path(appData) / "GraphicalCCodeGenerator";
         std::error_code errorCode;
         std::filesystem::create_directories(directory, errorCode);
         return directory;
      }
#endif
      const std::filesystem::path directory =
         std::filesystem::temp_directory_path() / "GraphicalCCodeGenerator";
      std::error_code errorCode;
      std::filesystem::create_directories(directory, errorCode);
      return directory;
   }

   std::filesystem::path CrashAutosavePath(void)
   {
      return AutosaveDirectory() / "autosave.cgen.tmp";
   }

   std::filesystem::path CrashAutosaveMetaPath(void)
   {
      return MetaPathForAutosave(CrashAutosavePath());
   }

   std::filesystem::path SiblingAutosavePath(std::string_view documentPath)
   {
      if (documentPath.empty())
      {
         return {};
      }
      return std::filesystem::path(std::string(documentPath)).string() + ".tmp";
   }

   Result WriteAutosave(const GraphDocument& document,
                        const std::filesystem::path& autosavePath,
                        std::string_view originalDocumentPath)
   {
      if (autosavePath.empty())
      {
         return Result::InvalidArgument;
      }
      const Result saveResult = SaveCgenFile(document, autosavePath.string());
      if (IsErr(saveResult))
      {
         return saveResult;
      }
      const std::filesystem::path metaPath = MetaPathForAutosave(autosavePath);
      return WriteMetaFile(metaPath, originalDocumentPath);
   }

   Result LoadAutosave(const std::filesystem::path& autosavePath,
                       GraphDocument* pDocument,
                       std::string* pDiagnostics)
   {
      if (pDocument == nullptr)
      {
         return Result::InvalidArgument;
      }
      if (autosavePath.empty())
      {
         return Result::InvalidArgument;
      }
      const Result loadResult =
         LoadCgenFile(autosavePath.string(), pDocument, pDiagnostics);
      if (IsErr(loadResult))
      {
         return loadResult;
      }
      const std::string originalPath =
         ReadMetaFile(MetaPathForAutosave(autosavePath));
      pDocument->SetFilePath(originalPath);
      pDocument->SetDirty(true);
      return Result::Ok;
   }

   bool AutosaveExists(const std::filesystem::path& autosavePath)
   {
      if (autosavePath.empty())
      {
         return false;
      }
      std::error_code errorCode;
      return std::filesystem::is_regular_file(autosavePath, errorCode);
   }

   Result ClearAutosave(const std::filesystem::path& autosavePath)
   {
      if (autosavePath.empty())
      {
         return Result::Ok;
      }
      std::error_code errorCode;
      std::filesystem::remove(autosavePath, errorCode);
      std::filesystem::remove(MetaPathForAutosave(autosavePath), errorCode);
      return Result::Ok;
   }
} // namespace Cgen
