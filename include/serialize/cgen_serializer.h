/*!
 *\file cgen_serializer.h
 *\brief Load and save .cgen flowchart documents.
 */
#ifndef CGEN_SERIALIZER_H
#define CGEN_SERIALIZER_H

#include <string>
#include <string_view>

#include "model/graph_document.h"
#include "model/result.h"

namespace Cgen
{
   /*!
    *\brief Saves a document to a .cgen file.
    *
    *\param[in] document Document to save.
    *\param[in] filePath Destination path.
    *\return Result code.
    */
   Result SaveCgenFile(const GraphDocument& document, std::string_view filePath);

   /*!
    *\brief Loads a document from a .cgen file.
    *
    *\param[in] filePath Source path.
    *\param[out] pDocument Destination document.
    *\param[out] pDiagnostics Optional parse diagnostics.
    *\return Result code.
    */
   Result LoadCgenFile(std::string_view filePath,
                       GraphDocument* pDocument,
                       std::string* pDiagnostics);
} // namespace Cgen

#endif // CGEN_SERIALIZER_H
