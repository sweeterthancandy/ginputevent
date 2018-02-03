#ifndef INCLUDE_KEYBOARDCULTURE_H
#define INCLUDE_KEYBOARDCULTURE_H
struct KeyDecl{
        explicit KeyDecl(std::string const& str, std::string const& literal, std::string const& key_str, int key, int msc, bool upper)
                :str_{str}, literal_{literal}, key_str_{key_str}, key_{key}, msc_{msc}, upper_{upper}
        {}
        auto Str()const{ return str_; }
        auto IsUpper()const{ return upper_; }
        auto LinuxKey()const{ return key_; }
        auto LinuxKeyStr()const{ return key_str_; }
        auto LinuxMsc()const{ return msc_; }
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
};

struct KeyboardCulture{
        KeyDecl const * Map(std::string const& key)const{
                auto iter = decl_.find(key);
                if( iter == decl_.end())
                        return nullptr;
                return &iter->second;
        }
        auto begin()const { return aux_.begin(); }
        auto end()const { return aux_.end(); }
        void Add(std::string const& str, std::string const& literal,
                 std::string const& key_str, int key, int msc, 
                 bool is_upper)
        {
                auto decl = KeyDecl{str, literal, key_str, key, msc, is_upper};
                decl_.emplace(str, decl );
                aux_.push_back( decl );
        }
private:
        std::map<std::string, KeyDecl> decl_;
        std::vector<KeyDecl> aux_;
};

struct StaticKeyboardCulture : KeyboardCulture{
        StaticKeyboardCulture(){
                #define STATIC_KEY_DECL(STR, LIT, KEY, MSC, IS_UPPER) \
                        Add(STR, LIT, #KEY, KEY, MSC, IS_UPPER);
                #include "static_key_def.h"
                #undef STATIC_KEY_DECL
        }
};
#endif // INCLUDE_KEYBOARDCULTURE_H
