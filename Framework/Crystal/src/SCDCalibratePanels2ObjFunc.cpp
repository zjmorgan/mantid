// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

#include "MantidCrystal/SCDCalibratePanels2ObjFunc.h"
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/Run.h"
#include "MantidAPI/Sample.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidGeometry/Crystal/IndexingUtils.h"
#include "MantidGeometry/Crystal/OrientedLattice.h"
#include "MantidGeometry/Instrument.h"
#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions/round.hpp>
#include <cmath>

namespace Mantid {
namespace Crystal {

using namespace Mantid::API;
using namespace Mantid::CurveFitting;
using namespace Mantid::DataObjects;
using namespace Mantid::Geometry;
using namespace Mantid::Kernel;

namespace {
// static logger
Logger g_log("SCDCalibratePanels2ObjFunc");
} // namespace

DECLARE_FUNCTION(SCDCalibratePanels2ObjFunc)

/// ---------------///
/// Core functions ///
/// ---------------///
SCDCalibratePanels2ObjFunc::SCDCalibratePanels2ObjFunc() {
  // parameters
  declareParameter("DeltaX", 0.0, "relative shift along X");
  declareParameter("DeltaY", 0.0, "relative shift along Y");
  declareParameter("DeltaZ", 0.0, "relative shift along Z");
  // rotation axis is defined as (1, theta, phi)
  // https://en.wikipedia.org/wiki/Spherical_coordinate_system
  declareParameter("Theta", PI / 4, "Polar coordinates theta in radians");
  declareParameter("Phi", PI / 2, "Polar coordinates phi in radians");
  // rotation angle
  declareParameter("DeltaRotationAngle", 0.0,
                   "angle of relative rotation in degree");
  declareParameter("DeltaT0", 0.0, "delta of TOF");
}

void SCDCalibratePanels2ObjFunc::setPeakWorkspace(
    IPeaksWorkspace_sptr &pws, const std::string componentName, 
    const std::vector<Mantid::Kernel::Matrix<double>> UBMatrix, const std::vector<int> UBRun) {
  m_pws = pws->clone();
  m_cmpt = componentName;

  // Special adjustment for CORELLI
  Instrument_sptr inst =
      std::const_pointer_cast<Instrument>(m_pws->getInstrument());
  if (inst->getName().compare("CORELLI") == 0 && m_cmpt != "moderator")
    // the second check is just to ensure that no accidental passing in
    // a bank name with sixteenpack already appended
    if (!boost::algorithm::ends_with(m_cmpt, "/sixteenpack"))
      m_cmpt.append("/sixteenpack");

  // Set the iteration count
  n_iter = 0;

  for (int i = 0; i < m_pws->getNumberPeaks(); ++i) {
    m_tofs.push_back(m_pws->getPeak(i).getTOF());
  }

  for (std::size_t i = 0; i < UBRun.size(); ++i){
    m_UMatrix.push_back(UBMatrix[i]);
    m_URun.push_back(UBRun[i]);
  }

}

/**
 * @brief Evalute the objective function with given feature vector X
 *
 * @param out     :: Q_calculated, which means yValues should be set to
 * Q_measured when setting up the Fit algorithm
 * @param xValues :: feature vector [shiftx3, rotx3, T0]
 * @param order   :: dimensionality of feature vector
 */
void SCDCalibratePanels2ObjFunc::function1D(double *out, 
                                            const double *xValues,
                                            const size_t order) const {
  // Get the feature vector component (numeric type)
  //-- delta in translation
  const double dx = getParameter("DeltaX");
  const double dy = getParameter("DeltaY");
  const double dz = getParameter("DeltaZ");
  //-- delta in rotation/orientation as angle axis pair
  //   using polar coordinates to ensure a unit vector
  //   (r, theta, phi) where r=1
  const double theta = getParameter("Theta");
  const double phi = getParameter("Phi");
  // compute the rotation axis
  double vx = sin(theta) * cos(phi);
  double vy = sin(theta) * sin(phi);
  double vz = cos(theta);
  //
  const double drotang = getParameter("DeltaRotationAngle");

  //-- delta in TOF
  // const double dT0 = getParameter("DeltaT0");
  //-- NOTE: given that these components are never used as
  //         one vector, there is no need to construct a
  //         xValues
  UNUSED_ARG(xValues);
  UNUSED_ARG(order);

  // -- always working on a copy only
  IPeaksWorkspace_sptr pws = m_pws->clone();

  // Debugging related
  IPeaksWorkspace_sptr pws_ref = m_pws->clone();

  const double c = 3.956034012071464e-07;

  // NOTE: when optimizing T0, a none component will be passed in.
  if (m_cmpt != "none/sixteenpack") {
    // rotation
    pws = rotateInstrumentComponentBy(vx, vy, vz, drotang, m_cmpt, pws);

    // translation
    pws = moveInstruentComponentBy(dx, dy, dz, m_cmpt, pws);
  }

  Instrument_const_sptr inst = pws->getInstrument();
  IPeaksWorkspace_sptr pw = std::make_shared<PeaksWorkspace>();
  pw->setInstrument(inst);

  auto lattice = std::make_unique<OrientedLattice>(pws->sample().getOrientedLattice());
  pw->mutableSample().setOrientedLattice(std::move(lattice));

  // TODO:
  // need to do something with dT0

  // calculate residual
  // double residual = 0.0;
  for (int i = 0; i < pws->getNumberPeaks(); ++i) {

    double L1 = -inst->getSource()->getPos().Z();    
    double L2 =  inst->getDetector(pws->getPeak(i).getDetectorID())->getPos().norm();
    
    double tof = m_tofs[i];

    double lamda = c*tof/(L1+L2)*1e+4;

    Peak pk = Peak(inst, pws->getPeak(i).getDetectorID(), lamda);

    pk.setGoniometerMatrix(pws->getPeak(i).getGoniometerMatrix());  
    pk.setRunNumber(pws->getPeak(i).getRunNumber());
    pk.setBankName(pws->getPeak(i).getBankName());
    pk.setIntHKL(pws->getPeak(i).getIntHKL());
    pk.setHKL(pws->getPeak(i).getHKL());

    pw->addPeak(pk);

    //V3D qv = pk->getQSampleFrame();
    //for (int j = 0; j < 3; ++j)
    //  out[i * 3 + j] = qv[j];

    // check the difference between n and target
    // auto ubm = pws->sample().getOrientedLattice().getUB();
    // V3D qv_target = ubm * pws->getPeak(i).getIntHKL();
    // qv_target *= 2 * PI;
    // V3D delta_qv = qv - qv_target;
    // residual += delta_qv.norm2();
  }

  auto B_pws = pws->sample().getOrientedLattice().getB();

  pw = recalculateUBIndexPeaks(pw);

  auto UB = pw->sample().getOrientedLattice().getUB();

  for (int i = 0; i < pw->getNumberPeaks(); ++i) {
    
    //V3D HKL = pw->getPeak(i).getHKL();

    auto ind = std::distance(m_URun.begin(), std::find(m_URun.begin(), m_URun.end(), pw->getPeak(i).getRunNumber()));
    
    auto U = m_UMatrix[ind];
    auto U_inv = m_UMatrix[ind];
    U_inv.Invert();

    auto B = U_inv*UB;

    V3D md = V3D(B[0][0],B[1][1],B[2][2]);
    V3D od = V3D(B[1][2],B[0][2],B[0][1]);

    auto UB_inv = U*B;
    UB_inv.Invert();

    V3D HKL = UB_inv * pw->getPeak(i).getQSampleFrame() / (2 * M_PI);

    for (int j = 0; j < 3; ++j) {
      out[i * 3 + j + 0] = HKL[j];
      out[i * 3 + j + 3] = md[j];
      out[i * 3 + j + 6] = od[j];
    }

  }

  n_iter += 1;

  // V3D dtrans = V3D(dx, dy, dz);
  // V3D rotaxis = V3D(vx, vy, vz);
  // residual /= pws->getNumberPeaks();
  // std::ostringstream msgiter;
  // msgiter.precision(8);
  // msgiter << "residual@iter_" << n_iter << ": " << residual << "\n"
  //         << "-- (dx, dy, dz) = " << dtrans << "\n"
  //         << "-- ang@axis = " << drotang << "@" << rotaxis << "\n\n";
  // g_log.information() << msgiter.str();
}

// -------///
// Helper ///
// -------///

/**
 * @brief Translate the component of given workspace by delta_(x, y, z)
 *
 * @param deltaX  :: The shift along the X-axis in m
 * @param deltaY  :: The shift along the Y-axis in m
 * @param deltaZ  :: The shift along the Z-axis in m
 * @param componentName  :: string representation of a component
 * @param pws  :: input workspace (mostly peaksworkspace)
 */
IPeaksWorkspace_sptr SCDCalibratePanels2ObjFunc::moveInstruentComponentBy(
    double deltaX, double deltaY, double deltaZ, std::string componentName,
    IPeaksWorkspace_sptr &pws) const {
  // Workspace_sptr inputws = std::dynamic_pointer_cast<Workspace>(pws);

  // move instrument is really fast, even with zero input
  IAlgorithm_sptr mv_alg = Mantid::API::AlgorithmFactory::Instance().create(
      "MoveInstrumentComponent", -1);
  //
  mv_alg->initialize();
  mv_alg->setChild(true);
  mv_alg->setLogging(LOGCHILDALG);
  mv_alg->setProperty("Workspace", pws);
  mv_alg->setProperty("ComponentName", componentName);
  mv_alg->setProperty("X", deltaX);
  mv_alg->setProperty("Y", deltaY);
  mv_alg->setProperty("Z", deltaZ);
  mv_alg->setProperty("RelativePosition", true);
  mv_alg->executeAsChildAlg();

  return pws;
}

/**
 * @brief Rotate the instrument by angle axis
 *
 * @param rotVx  :: x of rotation axis
 * @param rotVy  :: y of rotation axis
 * @param rotVz  :: z of rotation axis
 * @param rotAng  :: rotation angle (in degree)
 * @param componentName  :: component name
 * @param pws  :: peak workspace
 * @return IPeaksWorkspace_sptr
 */
IPeaksWorkspace_sptr SCDCalibratePanels2ObjFunc::rotateInstrumentComponentBy(
    double rotVx, double rotVy, double rotVz, double rotAng,
    std::string componentName, IPeaksWorkspace_sptr &pws) const {
  // rotate
  IAlgorithm_sptr rot_alg = Mantid::API::AlgorithmFactory::Instance().create(
      "RotateInstrumentComponent", -1);
  //
  rot_alg->initialize();
  rot_alg->setChild(true);
  rot_alg->setLogging(LOGCHILDALG);
  rot_alg->setProperty("Workspace", pws);
  rot_alg->setProperty("ComponentName", componentName);
  rot_alg->setProperty("X", rotVx);
  rot_alg->setProperty("Y", rotVy);
  rot_alg->setProperty("Z", rotVz);
  rot_alg->setProperty("Angle", rotAng);
  rot_alg->setProperty("RelativeRotation", true);
  rot_alg->executeAsChildAlg();

  return pws;
}

/**
 * @brief update UB matrix embeded in the peakworkspace using indexed peaks
 *        and redo the peak indexation afterwards
 *
 * @param pws
 */
 IPeaksWorkspace_sptr SCDCalibratePanels2ObjFunc::recalculateUBIndexPeaks(IPeaksWorkspace_sptr &pws) const {
   
  IAlgorithm_sptr calcUB_alg =
      Mantid::API::AlgorithmFactory::Instance().create("FindUBUsingIndexedPeaks", -1);
  calcUB_alg->initialize();
  calcUB_alg->setChild(true);
  calcUB_alg->setLogging(LOGCHILDALG);
  calcUB_alg->setProperty("PeaksWorkspace", pws);
  calcUB_alg->setProperty("Tolerance", 1.0); // values
  calcUB_alg->executeAsChildAlg();

  IAlgorithm_sptr idxpks_alg =
      Mantid::API::AlgorithmFactory::Instance().create("IndexPeaks", -1);
  idxpks_alg->initialize();
  idxpks_alg->setChild(true);
  idxpks_alg->setLogging(LOGCHILDALG);
  idxpks_alg->setProperty("PeaksWorkspace", pws);
  idxpks_alg->setProperty("CommonUBForAll", false); 
  idxpks_alg->setProperty("RoundHKLs", false); 
  idxpks_alg->setProperty("Tolerance", 1.0); 
  idxpks_alg->executeAsChildAlg();

  return pws;
}

} // namespace Crystal
} // namespace Mantid