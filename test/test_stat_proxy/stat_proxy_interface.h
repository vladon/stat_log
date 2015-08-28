#include "stat_log/loggers/shared_memory_logger.h"
#include "stat_log/util/compile_proxy.h"

using LoggerGenerator = stat_log::shared_mem_logger_generator;
using LoggerRetriever = stat_log::shared_mem_logger_retriever;

template <bool IsOperational>
void initializeStatistics();

void genStats_MAC();
void genStats_SIS();
void genStats_HW_INTF();
