// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_CURVEFITTING_TABLEWORKSPACEDOMAINCREATOR_H_
#define MANTID_CURVEFITTING_TABLEWORKSPACEDOMAINCREATOR_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/IDomainCreator.h"
#include "MantidDataObjects/TableWorkspace.h"
#include "MantidKernel/cow_ptr.h"

#include <boost/weak_ptr.hpp>
#include <list>

namespace Mantid {
namespace API {
class FunctionDomain;
class FunctionDomain1D;
class FunctionValues;
class TableWorkspace;
} // namespace API

namespace CurveFitting {

class DLLExport TableWorkspaceDomainCreator : public API::IDomainCreator {
public:
  // Constructor
  TableWorkspaceDomainCreator(Kernel::IPropertyManager *fit,
                              const std::string &workspacePropertyName,
                              DomainType domainType = Simple);
  /// Constructor
  TableWorkspaceDomainCreator(DomainType domainType = Simple);

  // Declare properties that specify the dataset within the workspace to fit to.
  void declareDatasetProperties(const std::string &suffix = "",
                                bool addProp = true) override;

  // Create a domain from the input workspace
  void createDomain(boost::shared_ptr<API::FunctionDomain> &domain,
                    boost::shared_ptr<API::FunctionValues> &values,
                    size_t i0 = 0) override;

  // Create an output workspace.
  boost::shared_ptr<API::Workspace> createOutputWorkspace(
      const std::string &baseName, API::IFunction_sptr function,
      boost::shared_ptr<API::FunctionDomain> domain,
      boost::shared_ptr<API::FunctionValues> values,
      const std::string &outputWorkspacePropertyName) override;

  /// Set the workspace
  /// @param ws :: workspace to set.
  void setWorkspace(API::ITableWorkspace_sptr ws) { m_tableWorkspace = ws; }
  /// Set the startX and endX
  /// @param startX :: Start of the domain
  /// @param endX :: End of the domain
  void setRange(double startX, double endX) {
    m_startX = startX;
    m_endX = endX;
  }
  /// Set max size for Sequential and Parallel domains
  /// @param maxSize :: Maximum size of each simple domain
  void setMaxSize(size_t maxSize) { m_maxSize = maxSize; }

  size_t getDomainSize() const override;

  void initFunction(API::IFunction_sptr function) override;

private:
  /// Calculate size and starting iterator in the X array
  std::pair<size_t, size_t> getXInterval() const;
  /// Set all parameters
  void setParameters() const;
  /// Get the names of the X, Y and Error columns
  std::vector<std::string>
  getXYEColumnNames(API::ITableWorkspace_sptr ws) const;
  /// Creates the blank output workspace of the correct size
  boost::shared_ptr<API::MatrixWorkspace>
  createEmptyResultWS(const size_t nhistograms, const size_t nyvalues);
  /// Set initial values for parameters with default values.
  void setInitialValues(API::IFunction &function);
  // Unrolls function into its constituent parts if it is a composite and adds
  // it to the list. Note this is recursive
  void
  appendCompositeFunctionMembers(std::list<API::IFunction_sptr> &functionList,
                                 const API::IFunction_sptr &function) const;
  // Create separate Convolutions for each component of the model of a
  // convolution
  void appendConvolvedCompositeFunctionMembers(
      std::list<API::IFunction_sptr> &functionList,
      const API::IFunction_sptr &function) const;
  /// Add the calculated function values to the workspace
  void addFunctionValuesToWS(
      const API::IFunction_sptr &function,
      boost::shared_ptr<API::MatrixWorkspace> &ws, const size_t wsIndex,
      const boost::shared_ptr<API::FunctionDomain> &domain,
      boost::shared_ptr<API::FunctionValues> resultValues) const;
  /// Check workspace is in the correct form
  void setAndValidateWorkspace(API::Workspace_sptr ws) const;

  /// Store workspace property name
  std::string m_workspacePropertyName;
  /// Store startX property name
  std::string m_startXPropertyName;
  /// Store endX property name
  std::string m_endXPropertyName;
  /// Store XColumnName property name
  std::string m_xColumnPropertyName;
  /// Store YColumnName property name
  std::string m_yColumnPropertyName;
  /// Store errorColumnName property name
  std::string m_errorColumnPropertyName;

  /// The input TableWorkspace
  mutable API::ITableWorkspace_sptr m_tableWorkspace;
  /// startX
  mutable double m_startX;
  /// endX
  mutable double m_endX;
  /// Max size for seq domain
  mutable size_t m_maxSize;
  /// Ranges that must be excluded from fit
  mutable std::vector<double> m_exclude;
  /// Store the created domain and values
  boost::weak_ptr<API::FunctionDomain1D> m_domain;
  boost::weak_ptr<API::FunctionValues> m_values;
  /// Store maxSize property name
  std::string m_maxSizePropertyName;
  /// Store the Exclude property name
  std::string m_excludePropertyName;

  /// Store number of the first row used in fitting
  size_t m_startRowNo;
};

} // namespace CurveFitting
} // namespace Mantid

#endif /*MANTID_CURVEFITTING_TABLEWORKSPACEDOMAINCREATOR_H_*/
