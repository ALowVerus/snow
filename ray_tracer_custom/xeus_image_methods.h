#pragma once

#include <fstream>
#include "xtl/xbase64.hpp"
#include "xeus/xjson.hpp"
#include <iostream>

namespace im {
    struct image
    {
        inline image(const std::string& filename)
        {
            std::ifstream fin(filename, std::ios::binary);
            m_buffer << fin.rdbuf();
            
        }
        
        std::stringstream m_buffer;
    };
    
    // This is a xeus-specific call. It runs when the notebook attempts to display a photo.
    xeus::xjson mime_bundle_repr(const image& i)
    {
        auto bundle = xeus::xjson::object();
        bundle["image/png"] = xtl::base64encode(i.m_buffer.str());
        return bundle;
    };
}