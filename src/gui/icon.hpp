#ifndef ICON_HPP_NEROSHOP
#define ICON_HPP_NEROSHOP

#include <image.hpp>
#include <memory> // std::shared_ptr, std::unique_ptr

//typedef std::tuple<unsigned int, unsigned int, unsigned int> dimensions; // width height channel
namespace neroshop {
class Icon {
public:
    Icon();
    ~Icon();
    static bool load_all();
private:
    static bool load_monero(); // source: getmonero.org/press-kit/index.html
    static bool load_wownero(); // source: https://suchwowstuff.xyz/product/wownero-media-kit/
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
    // non-icons8 icons
    static bool load_settings(); // cog//settings
    static bool load_speaker(); // volume_up, volume_down
    static bool load_bookmark();    
    static bool load_flag();  // flag, flag_outline (report)
    static bool load_numbers();       
    static bool load_login();
public:    
    static std::map<std::string, std::shared_ptr<Image>> get;
}; 
}
#endif
