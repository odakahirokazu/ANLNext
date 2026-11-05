# anlnext/status.py
#
# @author: Hirokazu Odaka
# @date: 2024-08-30
#

import anlnext.anlnextpy

### aliases of ANL status
AS_PROCEED             = anlnext.anlnextpy.ANLStatus_proceed
AS_PROCEED_WITH_ERROR  = anlnext.anlnextpy.ANLStatus_proceed_with_error
AS_SKIP                = anlnext.anlnextpy.ANLStatus_skip
AS_SKIP_WITH_ERROR     = anlnext.anlnextpy.ANLStatus_skip_with_error
AS_QUIT                = anlnext.anlnextpy.ANLStatus_quit
AS_QUIT_WITH_ERROR     = anlnext.anlnextpy.ANLStatus_quit_with_error
AS_QUIT_ALL            = anlnext.anlnextpy.ANLStatus_quit_all
AS_QUIT_ALL_WITH_ERROR = anlnext.anlnextpy.ANLStatus_quit_all_with_error
AS_CRITICAL_ERROR_TO_FINALIZE = anlnext.anlnextpy.ANLStatus_critical_error_to_finalize
AS_CRITICAL_ERROR_TO_TERMINATE = anlnext.anlnextpy.ANLStatus_critical_error_to_terminate

AS_OK = AS_PROCEED
