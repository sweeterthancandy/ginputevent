#ifndef INCLUDE_EMULATION_H
#define INCLUDE_EMULATION_H
#if 0
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <linux/input.h>
#include <linux/uinput.h>
#include <boost/variant.hpp>
#include <regex>

#endif

#include "KeyboardCulture.h"

struct ExecutionContext;
struct linux_keyboard_device;

struct Instruction{
        virtual void Execute(ExecutionContext& ctx)const=0;
};

struct ExecutionContext{
        ExecutionContext();
        linux_keyboard_device* Device(){ return device_; }
        void PostInstruction();
        void SleepBetweenMs(unsigned ms){
                sleep_between_ = ms;
        }
private:
        linux_keyboard_device* device_;
        unsigned sleep_between_{0};
};

struct Program{
        void Push(Instruction* instr){
                vec_.push_back(instr);
        }
        void Run(ExecutionContext& ctx)const{
                for( auto const& _ : vec_){
                        _->Execute(ctx);
                        ctx.PostInstruction();
                }
        }
private:
        std::vector<Instruction*> vec_;
};


#include "EmulationFrontend.h"




#endif // INCLUDE_EMULATION_H
