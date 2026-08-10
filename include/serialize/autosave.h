/*!
 *\file autosave.h
 *\brief Crash-recovery autosave paths and helpers for .cgen documents.
 */
#ifndef AUTOSAVE_H
#define AUTOSAVE_H

#include <filesystem>
#include <string>
#include <string_view>

#include "model/graph_document.h"
#include "model/result.h"

namespace Cgen
{
   /*!
    *\brief Returns the application autosave directory (created if needed).
    */
   std::filesystem::path AutosaveDirectory(void);

   /*!
    *\brief Returns the primary crash-recovery autosave file path.
    */
   std::filesystem::path CrashAutosavePath(void);

   /*!
    *\brief Returns the metadata path storing the original document path.
    */
   std::filesystem::path CrashAutosaveMetaPath(void);

   /*!
    *\brief Returns a sibling .cgen.tmp path for a saved document, or empty.
    *
    *\param[in] documentPath Document .cgen path.
    */
   std::filesystem::path SiblingAutosavePath(std::string_view documentPath);

   /*!
    *\brief Writes an autosave snapshot and optional original-path metadata.
    *
    *\param[in] document Document to snapshot.
    *\param[in] autosavePath Destination .cgen.tmp path.
    *\param[in] originalDocumentPath Path to restore after crash (may be empty).
    *\return Result code.
    */
   Result WriteAutosave(const GraphDocument& document,
                        const std::filesystem::path& autosavePath,
                        std::string_view originalDocumentPath);

   /*!
    *\brief Loads an autosave and restores the original document path.
    *
    *\param[in] autosavePath Autosave file.
    *\param[out] pDocument Destination document.
    *\param[out] pDiagnostics Optional diagnostics.
    *\return Result code.
    */
   Result LoadAutosave(const std::filesystem::path& autosavePath,
                       GraphDocument* pDocument,
                       std::string* pDiagnostics);

   /*!
    *\brief Returns true if the autosave file exists.
    *
    *\param[in] autosavePath Path to check.
    */
   bool AutosaveExists(const std::filesystem::path& autosavePath);

   /*!
    *\brief Deletes an autosave file and its metadata if present.
    *
    *\param[in] autosavePath Autosave file path.
    *\return Result::Ok even if the file was already missing.
    */
   Result ClearAutosave(const std::filesystem::path& autosavePath);
} // namespace Cgen

#endif // AUTOSAVE_H
