#ifndef MESSAGE_HPP_NEROSHOP
#define MESSAGE_HPP_NEROSHOP

#include <box.hpp>
#include <button.hpp>
#include <edit.hpp>
#include <timer.hpp>
#include <memory> // std::shared_ptr, std::make_shared

#include "debug.hpp"

namespace neroshop {
class Message {
public:
    Message();
    Message(const std::string& text, int label_index = 0);
    Message(const std::string& text, int red, int green, int blue, int label_index = 0);
    Message(const std::string& text, int red, int green, int blue, double alpha = 1.0, int label_index = 0);
    Message(const std::string& text, std::string color, int label_index = 0);
    ~Message();
    void initialize();
    void show();
    void hide();
    void draw();
    void center(unsigned int window_width, unsigned int window_height);
    void restore(); // restores original text color, clears text, etc.
    void add_label(int relative_x = 0, int relative_y = 0);
    void add_button(const std::string& text, int relative_x = 0, int relative_y = 0, int width = 100, int height = 50);//void add_button(int relative_x = 0, int relative_y = 0, std::string text = "");
    void add_edit(int relative_x = 0, int relative_y = 0, int width = 300, int height = 30);
    // setters
    void set_text(const std::string& text, int label_index = 0);
    void set_text(const std::string& text, int red, int green, int blue, int label_index = 0);
    void set_text(const std::string& text, int red, int green, int blue, double alpha, int label_index = 0);
    void set_text(const std::string& text, std::string color, int label_index = 0);
    void set_title(const std::string& title);
    void set_position(int x, int y);
    void set_width(int width);
    void set_height(int height);
    void set_size(int width, int height);
    void set_bottom_level_gui_list(const std::vector<GUI *>& bottom_level_gui_list);    
    // getters
    std::string get_text(int label_index = 0) const;
    std::string get_title_text() const;
    int get_x() const;
    int get_y() const;
    Vector2 get_position() const;
    int get_width() const;
    int get_height() const;
    Vector2 get_size() const;
    // objects
    static Message * get_first(); // returns the first and original message box
    static Message * get_second();
    //static Message * get_third();
    static Message * get_singleton();
    static Message * get_doubleton();
    //static Message * get_tripleton();
    Box * get_box() const;
    Button * get_button(int index) const;
    Edit * get_edit(int index) const;
    dokun::Label * get_label(int index) const;
    int get_button_count() const;
    int get_edit_count() const;
    int get_label_count() const;
    // boolean
    bool is_visible();
private: // https://codereview.stackexchange.com/questions/160053/c-erasing-an-object-from-vector-of-pointers/160058#160058
    static Message * first; // static objects have a static lifetime so I guess there's no need for shared_ptr here whatsoever   source: https://stackoverflow.com/questions/41751514/are-shared-ptr-on-static-objects-good#comment70695503_41751514
    static Message * second;
    //static Message * third;
    std::unique_ptr<Box> box;
    std::vector<std::shared_ptr<dokun::Label>> label_list; // since I can't do multi-lined labels :/
    std::vector<std::shared_ptr<Button>> button_list;//std::vector<Button*> button_list;   
    std::vector<std::shared_ptr<Edit>> edit_list;//std::vector<Edit*> edit_list;
    std::vector<GUI *> bottom_level_gui_list;
    void on_draw();
    void destroy_children(); // No longer a need for this now that I'm using shared pointers
    void draw_children();
    void hide_children();
    void show_children();
    void adjust_box();
};
}
#endif
