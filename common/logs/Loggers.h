//
// Created by notI on 2022/5/23.
//

#ifndef AI_CLIB_LOGGERS_H
#define AI_CLIB_LOGGERS_H

#define SPDLOG_ACTIVE_LEVEL  SPDLOG_LEVEL_TRACE

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog//sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;
using namespace spdlog::sinks;


class Loggers {
public:
    static void init_multi_sink();
    static string PATH;
    static mutex log_lock_;
    static bool init_flag_;
private:
    static bool create_directory();
};

#endif //AI_CLIB_LOGGERS_H
