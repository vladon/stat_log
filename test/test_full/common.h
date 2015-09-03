#include "mac_stat_tags.h"
#include "sis_stat_tags.h"

/*********************************
 * Stat Tag consolidation for the
 * MAC/SiS layers
 *********************************/

struct TOP_MAC_SIS
{
   SL_NAME = "TOP_LEVEL";
   using ChildTypes = MAKE_STAT_LIST
   (
      (MAC)
      (SIS)
   );
};

constexpr bool IsOperational = false;

template <bool IsOperational>
void initializeStatistics();

void handleCommandLineArgs(int argc, char** argv);

void genStats_MAC();
void genStats_SIS();
void genStats_HW_INTF();

