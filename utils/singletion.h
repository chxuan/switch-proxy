#pragma once
/*
功能:单例模板
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
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
