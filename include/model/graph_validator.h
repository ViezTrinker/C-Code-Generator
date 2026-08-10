/*!
 *\file graph_validator.h
 *\brief Static checks for flowchart documents before codegen.
 */
#ifndef GRAPH_VALIDATOR_H
#define GRAPH_VALIDATOR_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "model/graph_document.h"
#include "model/node.h"

namespace Cgen
{
   /*!
    *\brief Severity of a validation finding.
    */
   enum class ValidationSeverity: int8_t
   {
      Warning = 0,
      Error = 1
   };

   /*!
    *\brief One validation finding, optionally tied to a node.
    */
   struct ValidationIssue
   {
      ValidationSeverity severity = ValidationSeverity::Warning;
      NodeId nodeId = 0;
      std::string message;
   };

   /*!
    *\brief Collection of validation findings.
    */
   struct ValidationReport
   {
      std::vector<ValidationIssue> issues;
   };

   /*!
    *\brief Runs graph checks used by Generate/Build.
    *
    *\param[in] document Flowchart document.
    *\return Validation report (may be empty).
    */
   ValidationReport ValidateGraph(const GraphDocument& document);

   /*!
    *\brief Builds a per-node severity map (Error wins over Warning).
    *
    *\param[in] report Validation report.
    *\param[out] pOutMap Destination map (cleared first). Nodes with id 0 are skipped.
    */
   void BuildNodeSeverityMap(
      const ValidationReport& report,
      std::unordered_map<NodeId, ValidationSeverity>* pOutMap);
} // namespace Cgen

#endif // GRAPH_VALIDATOR_H
