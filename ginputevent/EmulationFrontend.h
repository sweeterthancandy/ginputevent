#ifndef INCLUDE_EMULATIONFRONTEND_H
#define INCLUDE_EMULATIONFRONTEND_H

#include <string>
#include <vector>
#include <sstream>

#include <boost/variant.hpp>
#include <boost/io/ios_state.hpp>

#include "Emulation.h"

namespace ginputevent{


namespace Frontend{

        /*
                 \a = \x07 = alert (bell)
                 \b = \x08 = backspace
                 \t = \x09 = horizonal tab
                 \n = \x0A = newline (or line feed)
                 \v = \x0B = vertical tab
                 \f = \x0C = form feed
                 \r = \x0D = carriage return
                 \e = \x1B = escape (non-standard GCC extension)
         */
        struct Key{
                Key(std::string const& c):char_string_{c}{}
                auto const& Get()const{ return char_string_; }
                friend std::ostream& operator<<(std::ostream& ostr,         Key const& self){
                        auto map_char = [](char c)->std::string{
                                switch(c){
                                        case '\a': return  "\\a";
                                        case '\b': return  "\\b";
                                        case '\t': return  "\\t";
                                        case '\n': return  "\\n";
                                        case '\v': return  "\\v";
                                        case '\f': return  "\\f";
                                        case '\r': return  "\\r";
                                        case ' ': return  "<space>";
                                        default:
                                        if( std::isgraph(c) ){
                                                return std::string{c};
                                        } else {
                                                return  "<nograph>";
                                        }
                                }
                        };
                        std::string escaped_str;
                        for(char c : self.char_string_ ){
                                escaped_str += map_char(c);
                        }
                        std::stringstream hex_ss;
                        hex_ss << std::hex;
                        for(size_t idx=0;idx!=self.char_string_.size();++idx){
                                if( idx != 0 )
                                        hex_ss << ",";
                                hex_ss << "0x" << static_cast<int>(self.char_string_[idx]);
                        }
                        boost::io::ios_flags_saver ifs( ostr );
                        ostr << "Key{" ;
                        ostr << "len=" << self.char_string_.size();
                        ostr << ", str=" << escaped_str;
                        ostr << ", hex=" << hex_ss.str();
                        ostr << "}";
                        return ostr;
                }
        private:
                std::string char_string_;
        };

        struct MouseRel{
                MouseRel(int x_, int y_):x{x_}, y{y_}{}
                int x;
                int y;
        };
        
        struct MouseAbs{
                MouseAbs(int x_, int y_):x{x_}, y{y_}{}
                int x;
                int y;
        };

        enum VectorType{
                VectorType_Nested,
                VectorType_Single,
        };


        struct Vector{
                explicit Vector(VectorType type = VectorType_Single):type_{type}{}
                void Push(Key const& lit){
                        vec_.push_back(lit);
                }
                auto begin()const{ return vec_.begin(); }
                auto end()const{ return vec_.end(); }
                auto GetType()const{ return type_; }
        private:
                VectorType type_;
                std::vector<Key> vec_;
        };

        struct SleepMs{
                SleepMs(unsigned ms):ms_{ms}{}
                auto Get()const{ return ms_; }
        private:
                unsigned ms_;
        };

        template<class... Args>
        Vector MakeNestedVector(Args&&... args){
                Vector vec(VectorType_Nested);
                int aux[] = { ( vec.Push(Key{args}), 0)... };
                return vec;
        }
        template<class... Args>
        Vector MakeVector(Args&&... args){
                Vector vec(VectorType_Single);
                int aux[] = { ( vec.Push(Key{args}), 0)... };
                return vec;
        }
        inline
        Vector MakeVectorFromLiteral(std::string const& lit){
                Vector vec(VectorType_Single);
                for( char c : lit ){
                        vec.Push(Key{std::string{c}});
                }
                return vec;
        }

        using Variant = boost::variant< Key, MouseRel, MouseAbs, Vector, SleepMs >;


        Program Compile(KeyboardCulture const& culture, std::vector<Frontend::Variant> const& vec);


} // Frontend
} // end namespace ginputevent


#endif // INCLUDE_EMULATIONFRONTEND_H
