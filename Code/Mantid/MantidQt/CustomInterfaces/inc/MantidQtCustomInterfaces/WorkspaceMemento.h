#ifndef MANTID_CUSTOMINTERFACES_MEMENTO_H_
#define MANTID_CUSTOMINTERFACES_MEMENTO_H_

#include "MantidKernel/System.h"
#include <string>
#include "MantidAPI/MatrixWorkspace.h"


namespace MantidQt
{
  namespace CustomInterfaces
  {
    /** @class WorkspaceMemento

    A memento carrying basic information about an existing workspace.

    @author Owen Arnold
    @date 30/08/2011

    Copyright &copy; 2007-8 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

    class DLLExport WorkspaceMemento
    {
    public:
      /**
      Getter for the id of the workspace
      @return the id of the workspace
      */
      virtual std::string getId() const = 0;
      /**
      Getter for the type of location where the workspace is stored
      @ return the location type
      */
      virtual std::string locationType() const = 0;
      /**
      Check that the workspace has not been deleted since instantiating this memento
      @return true if still in specified location
      */
      virtual bool checkStillThere() const = 0;
      /**
      Getter for the workspace itself
      @returns the matrix workspace
      @throw if workspace has been moved since instantiation.
      */
      virtual Mantid::API::MatrixWorkspace_sptr fetchIt() const = 0;
      /// Destructor
      virtual ~WorkspaceMemento(){};
    };
  }
}

#endif