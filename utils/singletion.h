#pragma once

template<typename T>
class singletion
{
public:
    static T& instance() 
    {
        static T t;
        return t; 
    }

    singletion() = delete;
    singletion(const singletion&) = delete;
    singletion& operator=(const singletion&) = delete;
};
