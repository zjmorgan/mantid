#include "MantidCrystal/HardThresholdBackground.h"
#include "MantidAPI/IMDIterator.h"

namespace Mantid {
namespace Crystal {

//----------------------------------------------------------------------------------------------
/** Constructor
 */
HardThresholdBackground::HardThresholdBackground(
    const double thresholdSignal,
    const Mantid::API::MDNormalization normalization)
    : m_thresholdSignal(thresholdSignal), m_normalization(normalization) {}

HardThresholdBackground *HardThresholdBackground::clone() const {
  return new HardThresholdBackground(*this);
}

void HardThresholdBackground::configureIterator(
    Mantid::API::IMDIterator *const iterator) const {
  iterator->setNormalization(m_normalization);
}

bool HardThresholdBackground::isBackground(
    Mantid::API::IMDIterator *iterator) const {
  return iterator->getNormalizedSignal() <= m_thresholdSignal;
}

} // namespace Crystal
} // namespace Mantid
