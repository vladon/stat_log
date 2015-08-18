#include "stat_log/loggers/shared_memory_logger.h"

using LoggerType = stat_log::shared_mem_logger;

template <typename Tag, typename ...Args>
void writeStatProxy(Args... args);

template <bool IsOperational>
void initializeStatistics();

// LoggerType& logInfo();
//
void genStats_MAC();
void genStats_SIS();
void genStats_HW_INTF();
