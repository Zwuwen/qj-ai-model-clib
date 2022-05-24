//
// Created by notI on 2022/5/23.
//

#include "Loggers.h"

string Loggers:: PATH("/data/logs/AI/clib-rknnx/");
bool Loggers::init_flag_=false;
mutex Loggers::log_lock_;

void Loggers::init_multi_sink()
{
    std::lock_guard<std::mutex> lk(log_lock_);
    if(init_flag_) return;
    init_flag_ = true;
    if(!create_directory()) {
        PATH = string("/data/clib-rknnx.");
    }
    auto console_sink = make_shared< stdout_color_sink_mt>();
    console_sink->set_level( spdlog::level::trace);
    console_sink->set_pattern( "%H:%M:%S.%e %^%L%$ %@ %v");

    auto file_sink = make_shared< rotating_file_sink_mt>(PATH+"log", 1048576 * 5, 3);
    file_sink->set_level( spdlog::level::trace);
    file_sink->set_pattern( "%Y-%m-%d %H:%M:%S.%f %L %@ %v");
    auto logger = shared_ptr<spdlog::logger>(new spdlog::logger("multi_sink",{console_sink, file_sink}));
    logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(logger);
//    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(10));
}

bool Loggers::create_directory() {
    struct stat info{};
    if(stat( PATH.c_str(), &info ) != 0 ){
        mkdir(PATH.c_str(), 0777);
        return true;
    };
    if( info.st_mode & S_IFDIR ) return true;
    std::remove(PATH.c_str());
    mkdir(PATH.c_str(), 0777);

    return true;
}
