#ifndef DIRECTORY_MONITOR_H
#define DIRECTORY_MONITOR_H

#include <memory>
#include <array>
#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>
#include <sys/inotify.h>

namespace ginputevent{

enum tag{
        tag_created,
        tag_deleted,
        tag_existing,
};

struct directory_monitor : std::enable_shared_from_this<directory_monitor>{
        using sig_t = void(tag, const std::string&);
        using signal_t = boost::signals2::signal<sig_t>;

        explicit directory_monitor(boost::asio::io_service& io_, const std::string& dir);
        void start(){
                start_read();
        }
        void stop(){
                running_ = false;
        }
        boost::signals2::connection connect(const signal_t::slot_type& sub){
                return sig_.connect(sub);
        }
private:
        void start_read();
private:
        std::string dir_{"/dev/input"};
        boost::asio::io_service& io_;
        boost::asio::io_service::work work_;
        boost::asio::deadline_timer timer_;
        std::array<char,sizeof(struct inotify_event) * 1024> ev_buffer_;

        bool running_{true};

        int fd;
        int wd;
        fd_set descriptors;
        struct timeval time_to_wait = {0};
        signal_t sig_;
};

} // end namespace ginputevent
#endif // DIRECTORY_MONITOR_H
