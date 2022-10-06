#ifndef ICON_HPP_NEROSHOP
#define ICON_HPP_NEROSHOP

#define NEROSHOP_RESOURCES_FOLDER "resources/images" //"assets/images"
#define NEROSHOP_ASSETS_FOLDER NEROSHOP_RESOURCES_FOLDER

#include <png.h>

#include <iostream>
#include <memory> // std::shared_ptr, std::unique_ptr
#include <unordered_map> // std::unordered_map
#include <tuple> // std::tuple
#include <filesystem> // std::filesystem

#include "../core/debug.hpp"

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
    static bool load_bell();    // names: "bell" (notifications)
    static bool load_config();  // names: "config"
    static bool load_paid();    // names: "paid" (verified_purchase)
    static bool load_image();   // names: "image", "gallery" (image_placeholder)
    static bool load_eye();     // names: "eye", "hide" (toggle visiblity)
    static bool load_mail();    // names: "mail", "send" (messages)
    static bool load_link();    // names: "external_link", "link"
    static bool load_shop();
    static bool load_copy();    // names: "copy", "clipboard"
    static bool load_views(); // names: "grid", "list"
    static bool load_triangle();
    static bool load_theme(); // names: "sun", moon, "bulb"
    static bool load_settings(); // "cog", "tools"
    static bool load_privacy();
    //static bool load_();
    // non-icons8 icons
    static bool load_speaker(); // volume_up, volume_down
    static bool load_bookmark();    
    static bool load_flag();  // flag, flag_outline (report)
    static bool load_numbers();       
    static bool load_login();
    // private - writes data to png file
    static bool write_png(unsigned char * data, unsigned int width, unsigned int height, const std::string& filename);
public:    
    // public - creates resource directories and calls write_png
    static bool export_png(unsigned char * data, unsigned int width, unsigned int height, const std::string& name);

    template<class T> T static get(const std::string& name) {
        if constexpr ((std::is_same<T, unsigned char *>::value) || (std::is_same<T, void *>::value)) { 
            return std::get<0>(image_map[name]); // data
        }
        if constexpr ((std::is_same<T, unsigned int>::value) || (std::is_same<T, int>::value)) {
            return std::get<1>(image_map[name]); // width or height (both are equal so it doesn't matter) // maybe rename width to size and height to channel?
        }
    }
    // Usage: int size = Icon::get<int>("upload"); unsigned char * data = Icon::get<unsigned char *>("upload");//std::cout << "Icon info (upload): \n" << "data=" << data << "\n" << "size=" << size << "\n";
    static std::unordered_map<std::string, std::tuple<unsigned char *, unsigned int, unsigned int>> image_map;
}; 

}
#endif
