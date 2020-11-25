// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidQtWidgets/Common/FitScriptGeneratorPresenter.h"
#include "MantidQtWidgets/Common/FitScriptGeneratorModel.h"

#include <stdexcept>

namespace MantidQt {
namespace MantidWidgets {

FitScriptGeneratorPresenter::FitScriptGeneratorPresenter(
    FitScriptGeneratorView *view, FitScriptGeneratorModel *model)
    : m_view(view), m_model(model) {
  m_view->subscribePresenter(this);
}

FitScriptGeneratorPresenter::~FitScriptGeneratorPresenter() {}

void FitScriptGeneratorPresenter::notifyPresenter(ViewEvent const &event) {
  switch (event) {
  case ViewEvent::RemoveClicked:
    return;
  case ViewEvent::StartXChanged:
    return;
  case ViewEvent::EndXChanged:
    return;
  }

  throw std::runtime_error("Failed to notify the FitScriptGeneratorPresenter.");
}

void FitScriptGeneratorPresenter::openFitScriptGenerator() { m_view->show(); }

} // namespace MantidWidgets
} // namespace MantidQt
