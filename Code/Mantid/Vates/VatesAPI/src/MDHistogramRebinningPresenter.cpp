#include <vtkUnstructuredGrid.h>
#include <vtkDataSet.h>
#include <vtkBox.h>
#include <vtkFieldData.h>

#include "MantidGeometry/MDGeometry/MDGeometryXMLDefinitions.h"
#include "MantidGeometry/MDGeometry/MDGeometryXMLBuilder.h"
#include "MantidAPI/ImplicitFunctionFactory.h"
#include "MantidMDAlgorithms/NullImplicitFunction.h" 
#include "MantidMDAlgorithms/BoxImplicitFunction.h"
#include "MantidMDAlgorithms/CompositeImplicitFunction.h"
#include "MantidMDAlgorithms/DynamicRebinFromXML.h"
#include "MantidMDAlgorithms/Load_MDWorkspace.h"
#include "MantidVatesAPI/MDHistogramRebinningPresenter.h"
#include "MantidVatesAPI/RebinningActionManager.h"
#include "MantidVatesAPI/vtkDataSetFactory.h"
#include "MantidVatesAPI/MDRebinningView.h"
#include "MantidVatesAPI/Clipper.h"
#include "MantidVatesAPI/vtkDataSetToGeometry.h"
#include "MantidVatesAPI/RebinningCutterXMLDefinitions.h"
#include "MantidVatesAPI/FieldDataToMetadata.h"
#include "MantidVatesAPI/MetadataToFieldData.h"
#include "MantidVatesAPI/ProgressAction.h"

using Mantid::Geometry::MDGeometryXMLParser;
using Mantid::MDAlgorithms::NullImplicitFunction;

namespace Mantid
{
  namespace VATES
  {

    MDHistogramRebinningPresenter::MDHistogramRebinningPresenter(vtkDataSetFactory* factoryChain, vtkDataSet* input, RebinningActionManager* request, MDRebinningView* view, Clipper* clipper) 
      : m_factory(factoryChain), 
      m_inputParser(input), 
      m_input(input), 
      m_request(request), 
      m_view(view), 
      m_box(new NullImplicitFunction()), 
      m_clipper(clipper),
      m_maxThreshold(0),
      m_minThreshold(0),
      m_applyClip(true),
      m_timestep(0)
    {

      vtkDataSetToGeometry parser(input);
      parser.execute();

      using Mantid::Geometry::MDGeometryBuilderXML;
      using Mantid::Geometry::StrictDimensionPolicy;
      MDGeometryBuilderXML<StrictDimensionPolicy> xmlBuilder;

      Mantid::Geometry::VecIMDDimension_sptr dimensions =parser.getAllDimensions();
      DimensionVec::iterator it = dimensions.begin();
      for(;it != dimensions.end(); ++it)
      {
        xmlBuilder.addOrdinaryDimension(*it);
      }
      if(parser.hasXDimension())
      {
        xmlBuilder.addXDimension(parser.getXDimension());
      }
      if(parser.hasYDimension())
      {
        xmlBuilder.addYDimension(parser.getYDimension());
      }
      if(parser.hasZDimension())
      {
        xmlBuilder.addZDimension(parser.getZDimension());
      }
      if(parser.hasTDimension())
      {
        xmlBuilder.addTDimension(parser.getTDimension());
      }

      //Apply the geometry.
      m_serializer.setGeometryXML(xmlBuilder.create());
      //Apply the workspace name after extraction from the input xml.
      m_serializer.setWorkspaceName( findExistingWorkspaceName(m_input, XMLDefinitions::metaDataId().c_str()));
      //Apply the workspace location after extraction from the input xml.
      m_serializer.setWorkspaceLocation( findExistingWorkspaceLocation(m_input, XMLDefinitions::metaDataId().c_str()));
      //Set-up a default box.
      m_box = constructBoxFromInput();
    }

    std::string MDHistogramRebinningPresenter::constructGeometryXML(
      DimensionVec dimensions,
      Dimension_sptr dimensionX,
      Dimension_sptr dimensionY,
      Dimension_sptr dimensionZ,
      Dimension_sptr dimensiont)
    {
      using Mantid::Geometry::MDGeometryBuilderXML;
      using Mantid::Geometry::StrictDimensionPolicy;
      MDGeometryBuilderXML<StrictDimensionPolicy> xmlBuilder;
      DimensionVec::iterator it = dimensions.begin();
      for(;it != dimensions.end(); ++it)
      {
        xmlBuilder.addOrdinaryDimension(*it);
      }
      xmlBuilder.addXDimension(dimensionX);
      xmlBuilder.addYDimension(dimensionY);
      xmlBuilder.addZDimension(dimensionZ);
      xmlBuilder.addTDimension(dimensiont);
      return xmlBuilder.create();
    }

    void MDHistogramRebinningPresenter::forumulateBinChangeRequest(MDGeometryXMLParser& old_geometry, MDGeometryXMLParser& new_geometry)
    {
      if(old_geometry.hasXDimension() && new_geometry.hasXDimension())
      {
        if(old_geometry.getXDimension()->getNBins() != new_geometry.getXDimension()->getNBins())
        {
          m_request->ask(RecalculateAll);
        }
      }
      if(old_geometry.hasYDimension() && new_geometry.hasYDimension())
      {
        if(old_geometry.getYDimension()->getNBins() != new_geometry.getYDimension()->getNBins())
        {
          m_request->ask(RecalculateAll);
        }
      }
      if(old_geometry.hasZDimension() && new_geometry.hasZDimension())
      {
        if(old_geometry.getZDimension()->getNBins() != new_geometry.getZDimension()->getNBins())
        {
          m_request->ask(RecalculateAll);
        }
      }
      if(old_geometry.hasTDimension() && new_geometry.hasTDimension())
      {
        if(old_geometry.getTDimension()->getNBins() != new_geometry.getTDimension()->getNBins())
        {
          m_request->ask(RecalculateAll);
        }
      }
    }


    ImplicitFunction_sptr MDHistogramRebinningPresenter::constructBoxFromVTKBox(vtkBox* box) const
    {
      using namespace Mantid::MDAlgorithms;

      double originX, originY, originZ, width, height, depth;

      //To get the box bounds, we actually need to evaluate the box function. There is not this restriction on planes.
      m_clipper->SetInput(m_input);
      m_clipper->SetClipFunction(box);
      m_clipper->SetInsideOut(true);
      m_clipper->Update();
      vtkDataSet* clipperOutput = m_clipper->GetOutput();
      //Now we can get the bounds.
      double* bounds = clipperOutput->GetBounds();

      originX = (bounds[1] + bounds[0]) / 2;
      originY = (bounds[3] + bounds[2]) / 2;
      originZ = (bounds[5] + bounds[4]) / 2;
      width = sqrt(pow(bounds[1] - bounds[0], 2));
      height = sqrt(pow(bounds[3] - bounds[2], 2));
      depth = sqrt(pow(bounds[5] - bounds[4], 2));

      //Create domain parameters.
      OriginParameter originParam = OriginParameter(originX, originY, originZ);
      WidthParameter widthParam = WidthParameter(width);
      HeightParameter heightParam = HeightParameter(height);
      DepthParameter depthParam = DepthParameter(depth);

      //Create the box. This is specific to this type of presenter and this type of filter. Other rebinning filters may use planes etc.
      BoxImplicitFunction* boxFunction = new BoxImplicitFunction(widthParam, heightParam, depthParam,
        originParam);

      return ImplicitFunction_sptr(boxFunction);
    }

    ImplicitFunction_sptr MDHistogramRebinningPresenter::constructBoxFromInput() const
    {
      using namespace Mantid::MDAlgorithms;

      double originX, originY, originZ, width, height, depth;

      vtkDataSetToGeometry metaDataProcessor(m_input);
      metaDataProcessor.execute();

      originX = (metaDataProcessor.getXDimension()->getMaximum() + metaDataProcessor.getXDimension()->getMinimum()) / 2;
      originY = (metaDataProcessor.getYDimension()->getMaximum() + metaDataProcessor.getYDimension()->getMinimum()) / 2;
      originZ = (metaDataProcessor.getZDimension()->getMaximum() + metaDataProcessor.getZDimension()->getMinimum()) / 2;
      width = metaDataProcessor.getXDimension()->getMaximum() - metaDataProcessor.getXDimension()->getMinimum();
      height = metaDataProcessor.getYDimension()->getMaximum() - metaDataProcessor.getYDimension()->getMinimum();
      depth = metaDataProcessor.getZDimension()->getMaximum() - metaDataProcessor.getZDimension()->getMinimum();

      //Create domain parameters.
      OriginParameter originParam = OriginParameter(originX, originY, originZ);
      WidthParameter widthParam = WidthParameter(width);
      HeightParameter heightParam = HeightParameter(height);
      DepthParameter depthParam = DepthParameter(depth);

      //Create the box. This is specific to this type of presenter and this type of filter. Other rebinning filters may use planes etc.
      BoxImplicitFunction* boxFunction = new BoxImplicitFunction(widthParam, heightParam, depthParam,
        originParam);

      return ImplicitFunction_sptr(boxFunction);
    }

    void MDHistogramRebinningPresenter::updateModel()
    {
      if(m_view->getTimeStep() != m_timestep)
      {
        m_request->ask(RecalculateVisualDataSetOnly);
        m_timestep = m_view->getTimeStep();
      }
      if(m_view->getMaxThreshold() != m_maxThreshold)
      {
        m_request->ask(RecalculateAll);
        m_maxThreshold = m_view->getMaxThreshold();
      }
      if(m_view->getMinThreshold() != m_minThreshold)
      {
        m_request->ask(RecalculateAll);
        m_minThreshold = m_view->getMinThreshold();
      }
      if(m_view->getApplyClip() != m_applyClip)
      {
        //check it's a box.
        //extract a box.
        //compare boxes.
        vtkBox* box = dynamic_cast<vtkBox*>(m_view->getImplicitFunction());
        if(NULL != box && m_view->getApplyClip())
        {
          m_box = constructBoxFromVTKBox(box);
        }
        else
        {
          m_box = constructBoxFromInput();
        }
        m_request->ask(RecalculateAll);
        m_applyClip = m_view->getApplyClip();
      }
      addFunctionKnowledge();

      if(m_view->getAppliedGeometryXML() != m_serializer.getWorkspaceGeometry())
      {
        Mantid::Geometry::MDGeometryXMLParser old_geometry(m_serializer.getWorkspaceGeometry());
        old_geometry.execute();
        Mantid::Geometry::MDGeometryXMLParser new_geometry(m_view->getAppliedGeometryXML());
        new_geometry.execute();
        //Detect dimension swapping only of 4 dimensions.
        if(old_geometry.getNonIntegratedDimensions().size() == 4 && new_geometry.getNonIntegratedDimensions().size() == 4)
        {
          //TODO, can we add the proxy view onto the front of the m_chain here!
          m_request->ask(RecalculateVisualDataSetOnly);
        }
        else
        {
          m_request->ask(RecalculateAll);
        }
        forumulateBinChangeRequest(old_geometry, new_geometry);

        m_serializer.setGeometryXML( m_view->getAppliedGeometryXML() );
      }
    }


    void MDHistogramRebinningPresenter::addFunctionKnowledge()
    {
      //Add existing functions.
      Mantid::MDAlgorithms::CompositeImplicitFunction* compFunction = new Mantid::MDAlgorithms::CompositeImplicitFunction;
      compFunction->addFunction(m_box);
      Mantid::API::ImplicitFunction* existingFunctions = findExistingRebinningDefinitions(m_input, XMLDefinitions::metaDataId().c_str());
      if (existingFunctions != NULL)
      {
        compFunction->addFunction(ImplicitFunction_sptr(existingFunctions));
      }
      //Apply the implicit function.
      m_serializer.setImplicitFunction(ImplicitFunction_sptr(compFunction));
    }

    vtkUnstructuredGrid* MDHistogramRebinningPresenter::execute(ProgressAction& eventHandler)
    {
      using namespace Mantid::API;
      const std::string outputWorkspace = XMLDefinitions::RebinnedWSName();
      //Rebin using member variables.
      if(RecalculateAll == m_request->action())
      {
        //Get the input workspace location and name.
        std::string wsLocation = m_serializer.getWorkspaceLocation();
        std::string wsName = m_serializer.getWorkspaceName();

        Mantid::API::IMDWorkspace_sptr baseWs = constructMDWorkspace(wsLocation);
        AnalysisDataService::Instance().addOrReplace(wsName, baseWs);

        Mantid::MDAlgorithms::DynamicRebinFromXML xmlRebinAlg;
        xmlRebinAlg.setRethrows(true);
        xmlRebinAlg.initialize();

        xmlRebinAlg.setPropertyValue("OutputWorkspace", outputWorkspace);

        //Use the serialisation utility to generate well-formed xml expressing the rebinning operation.
        std::string xmlString = m_serializer.createXMLString();
        xmlRebinAlg.setPropertyValue("XMLInputString", xmlString);

        Poco::NObserver<ProgressAction, Mantid::API::Algorithm::ProgressNotification> observer(eventHandler, &ProgressAction::handler);
        //Add observer.
        xmlRebinAlg.addObserver(observer);
        //Run the rebinning algorithm.
        xmlRebinAlg.execute();
        //Remove observer
        xmlRebinAlg.removeObserver(observer);
      }

      //Use the generated workspace to access the underlying image, which may be rendered.
      IMDWorkspace_sptr outputWs = boost::dynamic_pointer_cast<IMDWorkspace>(
        AnalysisDataService::Instance().retrieve(outputWorkspace));

      //TODO handle 4D swapping case via proxy vtkDataSetFactory.
      m_factory->initialize(outputWs);
      vtkUnstructuredGrid* temp = static_cast<vtkUnstructuredGrid*>(m_factory->create());
      persistReductionKnowledge(temp, this->m_serializer, XMLDefinitions::metaDataId().c_str());
      m_request->reset();
      //TODO. Add xml back onto dataset. Persist!
      return temp;
    }

    std::string MDHistogramRebinningPresenter::getAppliedGeometryXML() const
    {
      return m_serializer.getWorkspaceGeometry();
    }

    MDHistogramRebinningPresenter::~MDHistogramRebinningPresenter()
    {
    }

    Mantid::API::ImplicitFunction*  MDHistogramRebinningPresenter::findExistingRebinningDefinitions(
      vtkDataSet* inputDataSet, const char* id)
    {
      using Mantid::Geometry::MDGeometryXMLDefinitions;
      Mantid::API::ImplicitFunction* function = NULL;

      FieldDataToMetadata convert;
      std::string xmlString = convert(inputDataSet->GetFieldData(), id);
      if (false == xmlString.empty())
      {
        Poco::XML::DOMParser pParser;
        Poco::XML::Document* pDoc = pParser.parseString(xmlString);
        Poco::XML::Element* pRootElem = pDoc->documentElement();
        Poco::XML::Element* functionElem = pRootElem->getChildElement(MDGeometryXMLDefinitions::functionElementName());
        if(NULL != functionElem)
        {
          function = Mantid::API::ImplicitFunctionFactory::Instance().createUnwrapped(functionElem);
        }
      }
      return function;
    }

    std::string MDHistogramRebinningPresenter::findExistingWorkspaceName(vtkDataSet *inputDataSet, const char* id)
    {
      using Mantid::Geometry::MDGeometryXMLDefinitions;
      FieldDataToMetadata convert;
      std::string xmlString = convert(inputDataSet->GetFieldData(), id);

      Poco::XML::DOMParser pParser;
      Poco::XML::Document* pDoc = pParser.parseString(xmlString);
      Poco::XML::Element* pRootElem = pDoc->documentElement();
      Poco::XML::Element* wsNameElem = pRootElem->getChildElement(MDGeometryXMLDefinitions::workspaceNameElementName());
      if(wsNameElem == NULL)
      {
        throw std::runtime_error("The element containing the workspace name must be present.");
      }
      return wsNameElem->innerText();
    }

    //Get the workspace location from the xmlstring.
    std::string MDHistogramRebinningPresenter::findExistingWorkspaceLocation(vtkDataSet *inputDataSet, const char* id)
    {
      using Mantid::Geometry::MDGeometryXMLDefinitions;
      FieldDataToMetadata convert;
      std::string xmlString = convert(inputDataSet->GetFieldData(), id);

      Poco::XML::DOMParser pParser;
      Poco::XML::Document* pDoc = pParser.parseString(xmlString);
      Poco::XML::Element* pRootElem = pDoc->documentElement();
      Poco::XML::Element* wsLocationElem = pRootElem->getChildElement(MDGeometryXMLDefinitions::workspaceLocationElementName());
      if(wsLocationElem == NULL)
      {
        throw std::runtime_error("The element containing the workspace location must be present.");
      }
      return wsLocationElem->innerText();
    }

    Mantid::API::IMDWorkspace_sptr MDHistogramRebinningPresenter::constructMDWorkspace(const std::string& wsLocation)
    {
      using namespace Mantid::MDDataObjects;
      using namespace Mantid::Geometry;
      using namespace Mantid::API;

      Mantid::MDAlgorithms::Load_MDWorkspace wsLoaderAlg;
      wsLoaderAlg.initialize();
      std::string wsId = "InputMDWs";
      wsLoaderAlg.setPropertyValue("inFilename", wsLocation);
      wsLoaderAlg.setPropertyValue("MDWorkspace", wsId);
      wsLoaderAlg.execute();
      Workspace_sptr result=AnalysisDataService::Instance().retrieve(wsId);
      MDWorkspace_sptr workspace = boost::dynamic_pointer_cast<MDWorkspace>(result);

      return workspace;
    }

    void MDHistogramRebinningPresenter::persistReductionKnowledge(vtkDataSet* out_ds, const
      RebinningKnowledgeSerializer& xmlGenerator, const char* id)
    {
      vtkFieldData* fd = vtkFieldData::New();

      MetadataToFieldData convert;
      convert(fd, xmlGenerator.createXMLString().c_str(), id);

      out_ds->SetFieldData(fd);
      fd->Delete();
    }

  }
}