#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <linux/input.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/preprocessor.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/program_options.hpp>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "event_monitor.h"

#include "linux_event_to_string.h"
#include "formatter_detail.h"
#include "directory_monitor.h"
#include "event_observers.h"


namespace{


struct driver{
        driver():signals_(io_){}
        int run(int argc, char** argv){
                try{
                        bool do_create_existing = true;

                        for(int arg_iter=1; arg_iter < argc;){
                                int d = argc - arg_iter;
                                std::string arg = argv[arg_iter];
                                switch(d){
                                default:
                                case 2:
                                        if( arg == "--observer" ){
                                                std::string param = argv[arg_iter+1];
                                                if( param == "pretty" ){
                                                        obs_.push_back( std::make_shared<pretty_printer>() );
                                                } else if( param == "backspace-stats" ){
                                                        obs_.push_back( std::make_shared<backspace_logger>(io_) );
                                                } else{
                                                        std::cerr << "unknown observer \"" << param << "\"\n";
                                                        return EXIT_FAILURE;
                                                }
                                                arg_iter += 2;
                                                continue;
                                        }
                                case 1:
                                        if( arg == "--dont-create-existing"){
                                                do_create_existing = false;
                                                ++arg_iter;
                                                continue;
                                        }
                                        if( arg == "--help" ){
                                                std::cout 
                                                        << argv[0] << " <options>\n"
                                                        << "             --dont-create-existing\n";
                                                ++arg_iter;
                                                return EXIT_SUCCESS;
                                        }
                                }
                                std::cerr << "unknown arg \"" << arg << "\"\n";
                                return EXIT_FAILURE;
                        }

                        if( obs_.empty() )
                                obs_.push_back( std::make_shared<pretty_printer>() );


                        start_monitor_();
                        if( do_create_existing ){
                                create_existing_();
                        }
                        start_signal_monitor_();
                                        
                        for( auto ptr : obs_ ){
                                ptr->start();
                        }

                        io_.run();


                } catch( const std::exception& e){
                        std::cerr 
                                << "Caught exception: "
                                << boost::diagnostic_information(e)
                                ;
                        return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
        }
private:
        void start_signal_monitor_(){
                signals_.add( SIGINT );
                signals_.add( SIGTERM );
                signals_.async_wait( [this](const boost::system::error_code& ec, int sig){
                        switch(sig){
                                case SIGINT:
                                case SIGTERM:
                                        mon_->stop();
                                        mon_.reset();
                                        for( auto& p : monitors_ ){
                                                p.second->stop();
                                        }
                                        for( auto ptr : obs_ ){
                                                ptr->stop();
                                        }
                                        monitors_.clear();
                                        break;
                                default:
                                        assert( 0 && "unexpcted signal");
                        }
                });
        }
        void start_monitor_(){
                mon_ = std::make_shared<directory_monitor>(io_, "/dev/input");
                mon_->connect([this](tag t, std::string const& dir){
                        process_event_(t, dir);
                });
                mon_->start();
        }
        void create_existing_(){
                namespace bf = boost::filesystem;
                for(bf::directory_iterator iter("/dev/input/"),end;iter!=end;++iter){
                        auto p = iter->path();
                        auto dev = p.filename().string();
                       
                      
                        if( ! bf::is_directory( iter->path() ) )
                        {
                                process_event_(tag_existing, iter->path().string());
                        }
                }
        }
        void process_event_(tag t, std::string const& dir){
                switch(t){
                        case tag_existing:
                        case tag_created:{
                                std::cerr << dir << " created\n";
                                auto sptr = event_monitor::create(io_, dir);
                                if( sptr ){
                                        sptr->start();
                                        for( auto ptr : obs_ ){
                                                sptr->connect( [ptr](std::string const& dev, const struct input_event& ev){
                                                        ptr->Accept(dev, ev);
                                                });
                                        }
                                        monitors_.emplace(dir, sptr);
                                }
                                break;
                        }
                        case tag_deleted:{
                                std::cerr << dir << " deleted\n";
                                auto iter = monitors_.find(dir);
                                if( iter == monitors_.end()){
                                        std::cerr << "can't find it\n";
                                        break;
                                }
                                iter->second->stop();
                                monitors_.erase(iter);
                                break;
                        }
                }
        }
        boost::asio::io_service io_;
        boost::asio::signal_set signals_;
        std::vector<std::shared_ptr<event_observer> > obs_;
        std::shared_ptr<directory_monitor> mon_;
        std::map<std::string, std::shared_ptr<event_monitor> > monitors_;
};

} // anon

int main(int argc, char** argv){
        driver d;
        return d.run(argc, argv);
}
