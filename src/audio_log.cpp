#include "audio_log.h"
src::logger_mt g_logger;
void g_InitLog()
{
    logging::formatter formatter=
            expr::stream
            << "[" <<expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << " " << expr::attr< boost::log::aux::thread::id >("ThreadID")
            << " " << logging::trivial::severity
            << "] " << expr::smessage;

    logging::add_common_attributes();

    auto console_sink=logging::add_console_log();
    auto file_sink=logging::add_file_log
            (
                keywords::open_mode = std::ios::app,
                keywords::file_name = "%Y%m%d.log",
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)

                );

//        file_sink->locked_backend()->set_file_collector(sinks::file::make_collector(
//            keywords::target="logs",        //文件夹名
//            keywords::max_size=50*1024*1024,    //文件夹所占最大空间
//            keywords::min_free_space=100*1024*1024  //磁盘最小预留空间
//            ));

//    file_sink->set_filter(log_severity>=Log_Warning);   //日志级别过滤

//    file_sink->locked_backend()->scan_for_files();

    console_sink->set_formatter(formatter);
    file_sink->set_formatter(formatter);
    file_sink->locked_backend()->auto_flush(true);

    logging::core::get()->add_global_attribute("Scope",attrs::named_scope());
    logging::core::get()->add_sink(console_sink);
    logging::core::get()->add_sink(file_sink);
}

