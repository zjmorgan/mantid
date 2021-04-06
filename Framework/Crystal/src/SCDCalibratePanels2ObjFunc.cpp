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
    IPeaksWorkspace_sptr &pws, const std::string componentName) {
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

  //std::vector<V3D> hkls;
  //hkls.reserve(pws->getNumberPeaks());
  // std::vector<V3D> qvs;
  // qvs.reserve(pws->getNumberPeaks());

  // std::vector<V3D> intHKLs;
  // intHKLs.reserve(pws->getNumberPeaks());
  // std::vector<V3D> HKLs;
  // HKLs.reserve(pws->getNumberPeaks());

  const double c = 3.956034012071464e-07;

  // for (int i = 0; i < pws->getNumberPeaks(); ++i) {

  //   auto r = pws->getPeak(i).getGoniometerMatrix();

  //   double L1 = -pws->getInstrument()->getSource()->getPos().Z();    
  //   double L2 =  pws->getInstrument()->getDetector(pws->getPeak(i).getDetectorID())->getPos().norm();

  //   V3D pos = pws->getInstrument()->getDetector(pws->getPeak(i).getDetectorID())->getPos();
    
  //   double tof = tofs[i];

  //   double lamda = c*tof/(L1+L2)*1e+4;
    
  //   double two_theta = acos(pos[2]/L2);
  //   double az = atan2(pos[1],pos[0]);
    
  //   double k = 2*PI/lamda;
    
  //   V3D ql = V3D(-k*sin(two_theta)*cos(az), -k*sin(two_theta)*sin(az), -k*(cos(two_theta)-1));

  //   auto r_inv = r.Transpose();

  //   V3D qv = r_inv * ql;

  //   dqvs.push_back(qv-pws->getPeak(i).getQSampleFrame());

  // }

  //pws = recalculateUBIndexPeaks(pws);

  // NOTE: when optimizing T0, a none component will be passed in.
  if (m_cmpt != "none/sixteenpack") {
    // rotation
    pws = rotateInstrumentComponentBy(vx, vy, vz, drotang, m_cmpt, pws);

    // translation
    pws = moveInstruentComponentBy(dx, dy, dz, m_cmpt, pws);
  }

  //auto lat = pws->sample().getOrientedLattice();
  //auto ub = lat.getUB();

  //auto ub_inv = ub.Invert();

  // for (int i = 0; i < pws->getNumberPeaks(); ++i) {
  //   pws->removePeak(i);
  // }

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
    // cache TOF
    //double tof = pws->getPeak(i).getTOF();

    // auto r = pws->getPeak(i).getGoniometerMatrix();

    // //auto ub = pws->sample().getOrientedLattice().getUB();
    // //V3D qv_target = r * ub * pws->getPeak(i).getIntHKL();
    // //qv_target *= 2 * PI;

    // //double lamda = 4 * PI * fabs(qv_target.Z()) / qv_target.norm2();

    double L1 = -inst->getSource()->getPos().Z();    
    double L2 =  inst->getDetector(pws->getPeak(i).getDetectorID())->getPos().norm();

    // V3D pos = pws->getInstrument()->getDetector(pws->getPeak(i).getDetectorID())->getPos();
    
    double tof = m_tofs[i];

    double lamda = c*tof/(L1+L2)*1e+4;
    
    // double two_theta = acos(pos[2]/L2);
    // double az = atan2(pos[1],pos[0]);

    // //double d = lamda/(2*sin(two_theta/2));

    // //double lamda = pws->getPeak(i).getWavelength();
    
    //double k = 2*PI/lamda;
    
    // V3D ql = V3D(-k*sin(two_theta)*cos(az), -k*sin(two_theta)*sin(az), -k*(cos(two_theta)-1));

    // auto r_inv = r.Transpose();

    // V3D qv = r_inv * ql;

    // qvs.push_back(qv);

    //V3D qv = lat.hklFromQ(qs);

    //Units::Wavelength wl;
    //wl.initialize(pws->getPeak(i).getL1(), pws->getPeak(i).getL2(), pws->getPeak(i).getScattering(), 0,
    //            pws->getPeak(i).getInitialEnergy(), 0.0);
    //pws->getPeak(i).setWavelength(wl.singleFromTOF(tofs[i]));

    //pws->getPeak(i).setQSampleFrame(pws->getPeak(i).getQSampleFrame(), pws->getPeak(i).getL2());

    // Peak pk = Peak(pws->getPeak(i));
    // pk.setInstrument(inst);
    // //pk.removeContributingDetector(pws->getPeak(i).getDetectorID());
    // pk.setDetectorID(pws->getPeak(i).getDetectorID());
    // Units::Wavelength wl;
    // wl.initialize(pk.getL1(), pk.getL2(), pk.getScattering(), 0,
    //                 pk.getInitialEnergy(), 0.0);
    // pk.setWavelength(wl.singleFromTOF(m_tofs[i]));

    Peak pk = Peak(inst, pws->getPeak(i).getDetectorID(), lamda);

    pk.setGoniometerMatrix(pws->getPeak(i).getGoniometerMatrix());  
    pk.setRunNumber(pws->getPeak(i).getRunNumber());

    pw->addPeak(pk);



    //V3D qv = pk.getQSampleFrame();

    //qvs.push_back(qv);

    //pws->addPeak(pk);

    //Peak pk = pws->getPeak(i);

    //pk.setInstrument(pws->getInstrument());
    //pk.setWavelength(lamda);

    //pws->getPeak(i).removeContributingDetector(pk.getDetectorID());
    //pws->getPeak(i).setQSampleFrame(pk.getQSampleFrame(), pk.getL2());
    //pws->getPeak(i).setDetectorID(pk.getDetectorID());

    //V3D hkl = lat.hklFromQ(pk.getQSampleFrame());
    
    // pk.setWavelength(lamda);

    //std::unique_ptr<Peak> pk;

    //pk = std::make_unique<Peak>(pws->getInstrument(), pws->getPeak(i).getDetectorID(), lamda);

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
  //auto lat = pws->sample().getOrientedLattice();

  //pws = recalculateUBIndexPeaks(pws);
  //for (int i = 0; i < pws->getNumberPeaks(); ++i) {
  //    intHKLs.push_back(pws->getPeak(i).getIntHKL());
  //}

  // determine the lattice constants
  //Kernel::Matrix<double> UB(3, 3);

  //for (int i = 0; i < pws->getNumberPeaks(); ++i) {
  //    HKLs.push_back(lat.hklFromQ(qvs[i]));
  //}

  // IndexingUtils::Optimize_UB(UB, intHKLs, qvs);

  // lat.setUB(UB);

  // for (int i = 0; i < pws->getNumberPeaks(); ++i) {
  //     HKLs.push_back(lat.hklFromQ(qvs[i]));
  // }

  pw = recalculateUBIndexPeaks(pw);

  double residual = 0;

  for (int i = 0; i < pw->getNumberPeaks(); ++i) {
    
    V3D diffHKL = pw->getPeak(i).getHKL()-pw->getPeak(i).getIntHKL();

    for (int j = 0; j < 3; ++j) {
      out[i * 3 + j] = diffHKL[j];
    }

     residual += diffHKL.norm();
  }

  std::ostringstream msgiter;
  msgiter.precision(4);

  std::string bank = pw->getPeak(0).getBankName();

  double Dy = pw->getPeak(0).getDetPos().Y()-m_pws->getPeak(0).getDetPos().Y();

  msgiter << "residual@iter_" << n_iter << " " << dy << " " << Dy << " " << bank << ": " << residual << "\n";
  if (bank == std::string("bank52")) {
    g_log.notice() << msgiter.str();
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
 * @brief update UB matrix embeded in the peakworkspace using lattice constants
 *        and redo the peak indexation afterwards
 *
 * @param pws
 */
 IPeaksWorkspace_sptr SCDCalibratePanels2ObjFunc::recalculateUBIndexPeaks(IPeaksWorkspace_sptr &pws) const {
   
  // IAlgorithm_sptr calcUB_alg =
  //     Mantid::API::AlgorithmFactory::Instance().create("FindUBUsingIndexedPeaks", -1);
  // calcUB_alg->initialize();
  // calcUB_alg->setChild(true);
  // calcUB_alg->setLogging(LOGCHILDALG);
  // calcUB_alg->setProperty("PeaksWorkspace", pws);
  // calcUB_alg->setProperty("Tolerance", 1.0); // values
  // calcUB_alg->executeAsChildAlg();

  IAlgorithm_sptr idxpks_alg =
      Mantid::API::AlgorithmFactory::Instance().create("IndexPeaks", -1);
  idxpks_alg->initialize();
  idxpks_alg->setChild(true);
  idxpks_alg->setLogging(LOGCHILDALG);
  idxpks_alg->setProperty("PeaksWorkspace", pws);
  idxpks_alg->setProperty("CommonUBForAll", false); 
  idxpks_alg->setProperty("RoundHKLs", false); 
  idxpks_alg->setProperty("Tolerance", 10.0); 
  idxpks_alg->executeAsChildAlg();

  return pws;
}

} // namespace Crystal
} // namespace Mantid