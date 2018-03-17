#ifndef INCLUDE_KEYBOARDCULTURE_H
#define INCLUDE_KEYBOARDCULTURE_H

#include <map>
#include <vector>
#include <iostream>
#include <linux/input.h>
#include <linux/uinput.h>

namespace ginputevent{


struct KeyDecl{
        explicit KeyDecl(std::string const& str, std::string const& literal, std::string const& key_str, int key, int msc, bool upper, bool lit)
                :str_{str}, literal_{literal}, key_str_{key_str}, key_{key}, msc_{msc}, upper_{upper}, lit_{lit}
        {}
        auto Str()const{ return str_; }
        auto IsUpper()const{ return upper_; }
        auto LinuxKey()const{ return key_; }
        auto LinuxKeyStr()const{ return key_str_; }
        auto LinuxMsc()const{ return msc_; }
        auto IsLiteral()const{ return lit_; }
        auto Literal()const{ return literal_; }
        friend std::ostream& operator<<(std::ostream& ostr, KeyDecl const& self){
                return ostr << "{ "
                        << "str=" << self.str_
                        << ", lit=" << self.literal_
                        << ", key=" << self.key_str_ << "(" << self.key_ << ")"
                        << ", msc= " << self.msc_
                        << ", upper=" << std::boolalpha << self.IsUpper()
                        << "}";
        }
private:
        std::string str_;
        std::string literal_;
        std::string key_str_;
        int key_;
        int msc_;
        bool upper_;
        bool lit_;
};

struct KeyboardCulture{
        KeyDecl const * Map(std::string const& key)const{
                auto iter = decl_.find(key);
                if( iter == decl_.end())
                        return nullptr;
                return &iter->second;
        }
        KeyDecl const * FromKey(int key)const{
                auto iter = from_key_.find(key);
                if( iter == from_key_.end())
                        return nullptr;
                return &iter->second;
        }

        auto begin()const { return aux_.begin(); }
        auto end()const { return aux_.end(); }
        void Add(std::string const& str, std::string const& literal,
                 std::string const& key_str, int key, int msc, 
                 bool is_upper, bool is_literal)
        {
                auto decl = KeyDecl{str, literal, key_str, key, msc, is_upper, is_literal};
                decl_.emplace(str, decl );
                from_key_.emplace(key, decl);
                aux_.push_back( decl );
        }
private:
        std::map<std::string, KeyDecl> decl_;
        std::map<int, KeyDecl> from_key_;
        std::vector<KeyDecl> aux_;
};


struct StaticKeyboardCulture : KeyboardCulture{
        StaticKeyboardCulture(){
                #define STATIC_KEY_DECL(STR, LIT, KEY, MSC, IS_UPPER, IS_LIT) \
                        Add(STR, LIT, #KEY, KEY, MSC, IS_UPPER, IS_LIT);
                #include "static_key_def.h"
                #undef STATIC_KEY_DECL
        }
};

} // end namespace ginputevent


#endif // INCLUDE_KEYBOARDCULTURE_H
