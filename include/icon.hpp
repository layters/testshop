#ifndef ICON_HPP_NEROSHOP
#define ICON_HPP_NEROSHOP

#include <memory> // std::shared_ptr, std::unique_ptr
// dokun-ui
#include "image.hpp"

// icons8.com/icon/set/ecommerce/material--white
//typedef std::tuple<unsigned int, unsigned int, unsigned int> dimensions; // width height channel
namespace neroshop {
class Icon {
public:
    Icon();
    ~Icon();
    static bool load_all(); // loads all images
private:
    static bool load_monero(); // getmonero.org
    static bool load_wownero(); // https://suchwowstuff.xyz/product/wownero-media-kit/
    // icons8    
    static bool load_cart(); // cart
    static bool load_search(); // search
    static bool load_user(); // user
    static bool load_heart(); // heart (favorite)
    static bool load_star();
    static bool load_circle(); // circle, circle_outline    
    static bool load_trash();
    static bool load_order();
    static bool load_upload();
    static bool load_console(); // terminal or daemon
    static bool load_wallet();
    static bool load_door();
    static bool load_info();
    static bool load_bell(); // notifications
    static bool load_config();
    static bool load_paid(); // verified_purchase
    static bool load_image(); // image_placeholder
    static bool load_eye(); // show or hide (visibility)
    static bool load_mail(); // "mail", "send" (messages)
    static bool load_link(); // "external_link", "link"
    static bool load_shop();
    //static bool load_();
    // not from icons8 yet
    static bool load_settings(); // cog//settings
    static bool load_speaker(); // volume_up, volume_down
    static bool load_bookmark();    
    static bool load_flag();  // flag, flag_outline (report)
    static bool load_numbers();       
    static bool load_login();
public:    
    static std::map<std::string, std::shared_ptr<Image>> get; // a map is basically a std::vector of std::pairs // same as: static std::vector<std::pair<std::string, std::shared_ptr<Image>>> get;
}; 
}
#endif
// name, data, width, height, channel, depth
//    Image * image_ = new Image(const_cast<char *>(), 64, 64, 1, 4);
//    Icon::get.insert(std::make_pair("", image_));


// Icon::get["cog"]->get_data();
// Icon::get_icon()[]
// std::cout <<  << icon_list["cog"] << std::endl;
