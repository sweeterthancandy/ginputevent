#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include "KeyboardCulture.h"

struct event_observer{
        virtual ~event_observer()=default;
        virtual void Accept(const std::string& dev, const struct input_event& ev)=0;
        virtual void stop(){}
        virtual void start(){}
};

struct pretty_printer : event_observer{
        void Accept(const std::string& dev, const struct input_event& ev)override{
                std::stringstream sstr;
                sstr << dev << " - ";
                sstr << input_event_to_string(ev,aux::colour_formatter());
                std::cout << sstr.str() << std::endl;
        }
};

/*
        This prints key stream as a key logging format.
 */

struct backspace_logger : event_observer{
        enum { max_history = 100 };
        explicit backspace_logger(boost::asio::io_service& io)
                :io_{io}, timer_{io_}
        {}
        void Accept(const std::string& dev, const struct input_event& ev)override{
                if( ev.type == EV_KEY ){
                        switch(ev.value){
                        case 0: // key up
                                break;
                        case 1: // key down
                                emit( ev.code);
                                break;
                        case 2: // repeat
                                emit( ev.code);
                                break;
                        }
                }
        }
        void start()override{
                timer_.expires_from_now(boost::posix_time::seconds(1));
                timer_.async_wait( [this](boost::system::error_code ec){
                        for( auto const& stat : stats_){
                                std::cout << stat.first << " => " << stat.second << "\n";
                        }
                        std::cout << "here\n";

                        if( running_ )
                                start();
                });
        }
        void stop()override{
                running_ = false;
        }
private:
        void emit(int key){
                static StaticKeyboardCulture culture;
                KeyDecl const* decl = culture.FromKey(key);
                if(!! decl ){
                        std::cout << "decl = " << *decl << "\n";
                }
                if( key == KEY_BACKSPACE ){
                        int key = 0;
                        if( history_.size() ){
                                key = history_.back();
                                history_.pop_back();
                                emit_deleted( key );
                        }
                } else{
                        history_.push_back( key );
                }

                for( ; history_.size() > max_history ; )
                        history_.pop_back();
        }
        void emit_deleted(int key){
                ++stats_[key];
        }
        bool running_{true};
        boost::asio::io_service& io_;
        boost::asio::deadline_timer timer_;
        // keep a history of literals pressed
        std::list<int> history_;
        std::map<int, size_t> stats_;
        size_t total_keys_{0};
};

#endif // PRETTY_PRINTER_H
