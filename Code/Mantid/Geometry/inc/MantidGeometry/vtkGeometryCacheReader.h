#ifndef VTKGEOMETRYCACHEREADER_H
#define VTKGEOMETRYCACHEREADER_H

#include "MantidKernel/System.h"
#include "MantidKernel/Logger.h"

namespace Mantid
{

  namespace Geometry
  {
    /*!
    \class vtkGeometryCacheReader
    \brief Reads the Geometry Cache from the file to the Object
    \author Srikanth Nagella
    \date January 2009
    \version 1.0

    This class reads the geometry (triangles) cached in the vtk format file and copies them to the object.

    Copyright &copy; 2008 STFC Rutherford Appleton Laboratories

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    */
	class DLLExport vtkGeometryCacheReader
    {
    private:

      static Kernel::Logger& PLog;           ///< The official logger
	  Poco::XML::Document* mDoc;
	  Poco::XML::Element*  mRoot;
	  Poco::XML::DOMParser*  pParser;
	  std::string          mFileName;
	  //Private Methods
	  void Init();
	  Poco::XML::Element* vtkGeometryCacheReader::getElementByObjectName(std::string name);
	  void vtkGeometryCacheReader::readPoints(Poco::XML::Element* pEle,int *noOfPoints,double** points);
	  void vtkGeometryCacheReader::readTriangles(Poco::XML::Element* pEle,int *noOfTriangles,int** faces);
    public:
		vtkGeometryCacheReader(std::string filename);       ///< Constructor
		~vtkGeometryCacheReader();      ///< Destructor
		void readCacheForObject(Object *obj);
    };

  }   // NAMESPACE Geometry

}  // NAMESPACE Mantid

#endif
