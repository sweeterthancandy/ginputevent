#ifndef INCLUDE_EMULATIONFRONTEND_H
#define INCLUDE_EMULATIONFRONTEND_H

#include <string>
#include <vector>

#include <boost/variant.hpp>

#include "Emulation.h"

namespace Frontend{

        struct Key{
                Key(std::string const& c):c_{c}{}
                auto const& Get()const{ return c_; }
        private:
                std::string c_;
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

#endif // INCLUDE_EMULATIONFRONTEND_H
