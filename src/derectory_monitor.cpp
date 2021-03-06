#include "ginputevent/directory_monitor.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

namespace ginputevent{

        
directory_monitor::directory_monitor(boost::asio::io_service& io_, const std::string& dir)
                :io_(io_)
                ,work_(io_) // keep alive
                ,timer_(io_)
        {
                fd = inotify_init();

                wd = inotify_add_watch( fd, dir_.c_str(), 
                                        IN_MODIFY | IN_CREATE | IN_DELETE );

                time_to_wait.tv_sec = 0;
                time_to_wait.tv_usec = 0;

        }
void directory_monitor::start_read(){
        enum { EVENT_SIZE = sizeof (struct inotify_event) ,
                BUF_LEN = 1024 * ( EVENT_SIZE + 16 ) };
        timer_.expires_from_now(boost::posix_time::milliseconds(10));
        timer_.async_wait( [this](boost::system::error_code ec){
                FD_ZERO ( &descriptors );
                FD_SET ( fd, &descriptors );
                 
                         
                int return_value = select ( fd + 1, &descriptors, NULL, NULL, &time_to_wait);
                 
                if ( return_value < 0 ) {
                        perror("select");
                }
                else if ( ! return_value ) {
                }
                else if ( FD_ISSET ( fd, &descriptors ) ) {
                        char buffer[BUF_LEN];
                        int length = read( fd, buffer, BUF_LEN );  

                        if ( length < 0 ) {
                                perror( "read" );
                        }  

                        for(int i=0;  i < length ; ) {
                                struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
                                if ( event->len ) {
                                        if ( event->mask & IN_CREATE ) {
                                                if ( ! (event->mask & IN_ISDIR ) ) {
                                                        sig_(tag_created, dir_ + "/" + event->name); 
                                                }
                                        }
                                        else if ( event->mask & IN_DELETE ) {
                                                if ( ! ( event->mask & IN_ISDIR ) ) {
                                                        sig_(tag_deleted, dir_+ "/" + event->name); 
                                                }
                                        }
                                }
                                i += EVENT_SIZE + event->len;
                        }
                }

                if( running_ ){
                        start_read();
                }
        });
}
} // end namespace ginputevent

