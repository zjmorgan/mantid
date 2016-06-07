#ifndef MANTIDQT_API_DLLOPTION_H_
#define MANTIDQT_API_DLLOPTION_H_

#include "MantidKernel/System.h"

#ifdef IN_MANTIDQT_API
#define EXPORT_OPT_MANTIDQT_API DLLExport
#define EXTERN_MANTIDQT_API extern
#else
#define EXPORT_OPT_MANTIDQT_API DLLImport
#define EXTERN_MANTIDQT_API extern
#endif /* IN_MANTIDQT_API */

#endif // MANTIDQT_API_DLLOPTION_H_
