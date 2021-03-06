#ifndef EVENT_MONITOR_H
#define EVENT_MONITOR_H

#include <boost/signals2/signal.hpp>

//struct input_event {
	//struct timeval time;
	//__u16 type;
	//__u16 code;
	//__s32 value;
//};

namespace ginputevent{

struct event_monitor : std::enable_shared_from_this<event_monitor>{

        using sig_t = void(const std::string&,const struct input_event&);
        using signal_t = boost::signals2::signal<sig_t>;

        static std::shared_ptr<event_monitor> create(boost::asio::io_service& io, const std::string& dev){
                int fd = open(dev.c_str(), O_RDONLY);
                if( fd < 0 )
                        return std::shared_ptr<event_monitor>{};
                return std::make_shared<event_monitor>(io, fd, dev);
        }
        explicit event_monitor(boost::asio::io_service& io, int fd, std::string const& dev)
                :work_(io) // keep alive
                ,desc_(io)
                ,dev_(dev)
        {
                desc_.assign( fd );
        }
        void start(){
                start_read();
        }
        void stop(){
                desc_.cancel();
        }
        boost::signals2::connection connect(const signal_t::slot_type& sub){
                return sig_.connect(sub);
        }
private:
        void start_read(){
                auto self = shared_from_this();
                boost::asio::async_read( desc_, boost::asio::buffer(ev_buffer_),
                        [this,self](const boost::system::error_code& ec, size_t n){
                                if( ec ){
                                        // must of been cancelled
                                 
                                } else {
                                        assert( n == sizeof(struct input_event) && "unexpected read size");
                                        sig_(dev_, *reinterpret_cast<struct input_event*>(ev_buffer_.data()));
                                        start_read();
                                }
                });
        }
private:
        boost::asio::io_service::work work_;
        boost::asio::posix::stream_descriptor desc_;
        std::string dev_;
        boost::array<char,sizeof(struct input_event)> ev_buffer_;

        signal_t sig_;
};

} // end namespace ginputevent

#endif // EVENT_MONITOR_H
