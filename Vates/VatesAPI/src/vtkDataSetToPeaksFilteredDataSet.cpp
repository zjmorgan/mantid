#include "MantidVatesAPI/vtkDataSetToPeaksFilteredDataSet.h"
#include "MantidGeometry/Crystal/IPeak.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidDataObjects/NoShape.h"
#include "MantidDataObjects/PeakShapeEllipsoid.h"
#include "MantidDataObjects/PeakShapeSpherical.h"
#include "MantidGeometry/Crystal/PeakShape.h"
#include "MantidKernel/SpecialCoordinateSystem.h"
#include "MantidKernel/V3D.h"
#include "MantidKernel/ReadLock.h"
#include "MantidKernel/WarningSuppressions.h"
#include "MantidVatesAPI/ProgressAction.h"

#include <vtkExtractSelection.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFieldData.h>
#include <vtkIdList.h>
#include <vtkFieldData.h>

#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>


namespace Mantid
{
namespace VATES
{
  /**
    * Standard constructor for object.
    * @param input : The dataset to peaks filter
    * @param output : The resulting peaks filtered dataset
    */
vtkDataSetToPeaksFilteredDataSet::vtkDataSetToPeaksFilteredDataSet(
    vtkSmartPointer<vtkUnstructuredGrid> input,
    vtkSmartPointer<vtkUnstructuredGrid> output)
    : m_inputData(input), m_outputData(output), m_isInitialised(false),
      m_radiusNoShape(0.2), m_radiusType(0), m_radiusFactor(2),
      m_defaultRadius(0.1), m_coordinateSystem(0) {
  if (nullptr == input) {
    throw std::runtime_error("Cannot construct "
                             "vtkDataSetToPeaksFilteredDataSet with NULL input "
                             "vtkUnstructuredGrid");
  }
  if (nullptr == output) {
    throw std::runtime_error("Cannot construct "
                             "vtkDataSetToPeaksFilteredDataSet with NULL "
                             "output vtkUnstructuredGrid");
  }
}

vtkDataSetToPeaksFilteredDataSet::~vtkDataSetToPeaksFilteredDataSet() {}

/**
  * Set the value for the underlying peaks workspace
  * @param peaksWorkspaces : A list of peak workspace names.
  * @param radiusNoShape : The peak radius for no shape.
  * @param radiusType : The type of the radius: Radius(0), Outer Radius(10,
 * Inner Radius(1)
  * @param coordinateSystem: A coordinate system.
  */
void vtkDataSetToPeaksFilteredDataSet::initialize(
    const std::vector<Mantid::API::IPeaksWorkspace_sptr> &peaksWorkspaces,
    double radiusNoShape, int radiusType, int coordinateSystem) {
  m_peaksWorkspaces = peaksWorkspaces;
  m_radiusNoShape = radiusNoShape;
  m_radiusType = radiusType;
  m_isInitialised = true;
  m_coordinateSystem = coordinateSystem;
}

/**
  * Process the input data. First, get all the peaks and their associated
  * geometry. Then filter
  * through the input to find the peaks which lie within a peak. Then apply then
  * to the output data.
  * Then update the metadata. See
  * http://www.vtk.org/Wiki/VTK/Examples/Cxx/PolyData/ExtractSelection
  * @param progressUpdating The handle for the progress bar.
  */
void vtkDataSetToPeaksFilteredDataSet::execute(
    ProgressAction &progressUpdating) {
  if (!m_isInitialised) {
    throw std::runtime_error("vtkDataSetToPeaksFilteredDataSet needs "
                             "initialize run before executing");
  }

  // Get the peaks location and the radius information
  std::vector<std::pair<Mantid::Kernel::V3D, double>> peaksInfo =
      getPeaksInfo(m_peaksWorkspaces);

  // Compare each element of the vtk data set and check which ones to keep
  vtkPoints *points = m_inputData->GetPoints();

  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfComponents(1);

  double progressFactor =
      1.0 / static_cast<double>(points->GetNumberOfPoints());
  for (int i = 0; i < points->GetNumberOfPoints(); i++) {
    progressUpdating.eventRaised(double(i) * progressFactor);
    double point[3];
    points->GetPoint(i, point);

    // Compare to Peaks
    const size_t numberOfPeaks = peaksInfo.size();
    // size_t counter = 0;
    // while (counter < numberOfPeaks) {
    for (size_t counter = 0; counter < numberOfPeaks; ++counter) {
      // Calcuate the differnce between the vtkDataSet point and the peak. Needs
      // to be smaller than the radius
      double squaredDifference = 0.;
      for (unsigned k = 0; k < 3; k++) {
        squaredDifference += pow(point[k] - peaksInfo[counter].first[k], 2);
      }

      if (squaredDifference <=
          (peaksInfo[counter].second * peaksInfo[counter].second)) {
        ids->InsertNextValue(i);
        break;
      }
    }
  }

  // Create the selection node and tell it the type of selection
  auto selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
  selectionNode->SetFieldType(vtkSelectionNode::POINT);
  selectionNode->SetContentType(vtkSelectionNode::INDICES);
  selectionNode->SetSelectionList(ids);

  auto selection = vtkSmartPointer<vtkSelection>::New();
  selection->AddNode(selectionNode);

  // We are not setting up a pipeline here, cannot access vtkAlgorithmOutput
  auto extractSelection = vtkSmartPointer<vtkExtractSelection>::New();
  extractSelection->SetInputData(0, m_inputData);
  extractSelection->SetInputData(1, selection);
  extractSelection->Update();

  // Extract
  m_outputData->ShallowCopy(extractSelection->GetOutput());
}

/**
 * Get the peaks information which is the position and the largest radius of the
 * peak.
 * @param peaksWorkspaces A list of peaks workspaces
 * @returns A list of pair information which contains the position and the
 * radius.
 */
std::vector<std::pair<Mantid::Kernel::V3D, double>>
vtkDataSetToPeaksFilteredDataSet::getPeaksInfo(
    const std::vector<Mantid::API::IPeaksWorkspace_sptr> &peaksWorkspaces) {
  std::vector<std::pair<Mantid::Kernel::V3D, double>> peaksInfo;
  // Iterate over all peaksworkspaces and add the their info to the output
  // vector
  for (const auto &workspace : peaksWorkspaces) {
    const auto coordinateSystem =
        static_cast<Mantid::Kernel::SpecialCoordinateSystem>(
            m_coordinateSystem);
    int numPeaks = workspace->getNumberPeaks();
    // Iterate over all peaks for the workspace
    for (int i = 0; i < numPeaks; i++) {
      Mantid::Geometry::IPeak *peak = workspace->getPeakPtr(i);
      addSinglePeak(peak, coordinateSystem, peaksInfo);
    }
  }
  return peaksInfo;
}

GCC_DIAG_OFF(strict-aliasing)
  /**
   * Add information for a single peak to the peakInfo vector.
   * @param peak The peak from which the information will be extracted.
   * @param coordinateSystem The coordinate system in which the peaks position should be retrieved.
   * @param peaksInfo A reference to the vector containing peak information.
   */
  void vtkDataSetToPeaksFilteredDataSet::addSinglePeak(Mantid::Geometry::IPeak* peak, const Mantid::Kernel::SpecialCoordinateSystem coordinateSystem, std::vector<std::pair<Mantid::Kernel::V3D, double>>& peaksInfo)
  {
    double radius = m_defaultRadius;
    const Mantid::Geometry::PeakShape& shape = peak->getPeakShape();
    std::string shapeName = shape.shapeName();

    // Get the radius and the position for the correct peak shape
    if (shapeName == Mantid::DataObjects::PeakShapeSpherical::sphereShapeName())
    {
      const Mantid::DataObjects::PeakShapeSpherical& sphericalShape = dynamic_cast<const Mantid::DataObjects::PeakShapeSpherical&>(shape);
      if (m_radiusType == 0)
      {
        radius = sphericalShape.radius();
      }
      else if (m_radiusType == 1)
      {
          boost::optional<double> radOut = sphericalShape.backgroundOuterRadius();
          if (radOut.is_initialized()) {
            radius = radOut.get();
          }
      }
      else if (m_radiusType == 2)
      {
          boost::optional<double> radIn = sphericalShape.backgroundInnerRadius();
          if (radIn.is_initialized()) { 
           radius = radIn.get();
          }
      }
      else 
      {
        throw std::invalid_argument("The shperical peak shape does not have a radius. \n");
      }
    }
    else if (shapeName == Mantid::DataObjects::PeakShapeEllipsoid::ellipsoidShapeName())
    {
      const Mantid::DataObjects::PeakShapeEllipsoid& ellipticalShape = dynamic_cast<const Mantid::DataObjects::PeakShapeEllipsoid&>(shape);
      if (m_radiusType == 0)
      {
        radius = ellipticalShape.radius();
      }
      else if (m_radiusType == 1)
      {
        const std::vector<double> &radii =
            ellipticalShape.abcRadiiBackgroundOuter();
        radius = *(std::max_element(radii.begin(), radii.end()));
      }
      else if (m_radiusType == 2)
      {
        const std::vector<double> &radii =
            ellipticalShape.abcRadiiBackgroundInner();
        radius = *(std::max_element(radii.begin(), radii.end()));
      }
      else 
      {
        throw std::invalid_argument("The ellipsoidal peak shape does not have a radius. \n");
      }
    }
    else
    {
      radius = m_radiusNoShape;
    }

    // Get the position in the correct frame.
    switch(coordinateSystem)
    {
      case(Mantid::Kernel::SpecialCoordinateSystem::HKL):
        peaksInfo.emplace_back(peak->getHKL(), radius * m_radiusFactor);
        break;
      case(Mantid::Kernel::SpecialCoordinateSystem::QLab):
        peaksInfo.emplace_back(peak->getQLabFrame(), radius * m_radiusFactor);
        break;
      case(Mantid::Kernel::SpecialCoordinateSystem::QSample):
        peaksInfo.emplace_back(peak->getQSampleFrame(),
                               radius * m_radiusFactor);
        break;
      default:
        throw std::invalid_argument("The special coordinate systems don't match.");
    }
  }
//GCC_DIAG_ON(strict-aliasing)
  /**
   * Get the radiys for no shape
   * @returns The shape of the radius.
   */
  double vtkDataSetToPeaksFilteredDataSet::getRadiusNoShape(){
    return m_radiusNoShape;
  }

  /**
   * Get the radius factor which is used to calculate the radius of the culled data set around each peak
   * The culling radius is the radius of the peak times the radius factor.
   * @returns The radius factor.
   */
  double vtkDataSetToPeaksFilteredDataSet::getRadiusFactor() {
    return m_radiusFactor;
  }
}
}

