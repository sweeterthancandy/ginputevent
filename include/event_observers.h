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
        I'm only intrested in the form 

 */

struct backspace_logger : event_observer{
        struct KeyStat{
                explicit KeyStat(int key_):key{key_}{}
                int Key()const{
                        return key;
                }
                int Kept()const{
                        return kept;
                }
                int Deleted()const{
                        return deleted;
                }
                int Total()const{
                        return kept + deleted;
                }
                double Percent()const{
                        if( Deleted() == 0 )
                                return .0;
                        return Deleted() * 100.0 / ( Total()  );
                }

                friend std::ostream& operator<<(std::ostream& ostr, KeyStat const& self){
                        ostr << "kept = " << self.kept;
                        ostr << ", deleted = " << self.deleted;
                        return ostr;
                }
                int key;
                int kept{0};
                int deleted{0};
        };
        // I'm only intrested when I mistype a charcter or 2, then delete them.
        // when I delete 20 charcters, it's probably not a typo, ie I want
        // to deteched

        enum { max_history = 3 };
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

                        render(std::cout);

                        if( running_ )
                                start();
                });
        }
        void render(std::ostream& ostr)const{

                KeyStat sigma(0);

                std::vector<KeyStat const*> ordered;
                for( auto const& stat : stat_){
                        ordered.push_back( &stat.second );
                }
                std::sort(ordered.begin(), ordered.end(),
                          [](auto l, auto r){ return l->Deleted() > r->Deleted();
                });


                std::vector< std::vector< std::string > > buffer;
                
                std::vector<std::string> header = { "Key", "Kept", "Del", "Total", "%"};
                buffer.push_back(header);


                auto add = [&](auto s, auto  ptr){
                        std::vector<std::string> line;
                        
                        char percent[64];
                        std::sprintf(percent, "%2.f", ptr->Percent());

                        line.push_back(s);
                        line.push_back(boost::lexical_cast<std::string>(ptr->Kept()));
                        line.push_back(boost::lexical_cast<std::string>(ptr->Deleted()));
                        line.push_back(boost::lexical_cast<std::string>(ptr->Total()));
                        line.push_back(percent);

                        buffer.push_back(std::move(line));
                };

                for(auto ptr : ordered ){
                        KeyDecl const* decl = culture.FromKey(ptr->Key());

                        add( decl->Literal(), ptr);


                        sigma.kept += ptr->Kept();
                        sigma.deleted += ptr->Deleted();

                }

                add("Total", &sigma);
                
                
                std::vector<std::string::size_type> widths;
                for(auto const& line : buffer ){
                        for(unsigned i=0;i!=line.size();++i){
                                for(; widths.size() < line.size();)
                                        widths.emplace_back(0);
                                widths[i] = std::max(widths[i], line[i].size());
                        }
                }
                for(auto const& line : buffer ){
                        for(unsigned i=0;i!=line.size();++i){
                                ostr << "|";
                                int pad = widths[i] - line[i].size();
                                int left = pad / 2;
                                int right = pad - left;
                                if( left )
                                        ostr << std::string(left, ' ');
                                ostr << line[i];
                                if( right )
                                        ostr << std::string(right, ' ');
                        }
                        std::cout << "|\n";
                }


        }
        void stop()override{
                running_ = false;
        }
private:
        void emit(int key){
                if( key == KEY_BACKSPACE ){
                        int key = 0;
                        if( history_.size() ){
                                key = history_.back();
                                history_.pop_back();
                                emit_deleted( key );
                        }
                } else{
                        KeyDecl const* decl = culture.FromKey(key);
                        if(! decl ){
                                return;
                        }
                        if( ! decl->IsLiteral())
                                return;

                        ++total_keys_;
                        if( stat_.count(key) == 0 )
                                stat_.emplace(key, key);
                        ++stat_.find(key)->second.kept;
                        history_.push_back( key );
                }

                for( ; history_.size() > max_history ; )
                        history_.pop_back();
        }
        void emit_deleted(int key){
                auto iter = stat_.find(key);
                ++iter->second.deleted;
                --iter->second.kept;
        }
        StaticKeyboardCulture culture;
        bool running_{true};
        boost::asio::io_service& io_;
        boost::asio::deadline_timer timer_;
        // keep a history of literals pressed
        std::list<int> history_;
        std::map<int, KeyStat> stat_;
        size_t total_keys_{0};
};

#endif // PRETTY_PRINTER_H
