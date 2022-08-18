#include "message_box.hpp"

neroshop::MessageBox::MessageBox()
#if defined(NEROSHOP_USE_DOKUN_UI)
 : box(nullptr), button_list({}), edit_list({}), label_list({}) {//, button0(nullptr), button1(nullptr), button2(nullptr), edit0(nullptr), edit1(nullptr) {
#else
{
#endif
    initialize();
    if(!first) first = this;
    if((first != this) && !second) second = this;//if((first != this) && (second != this) && !third) third = this;
}
////////////////////
neroshop::MessageBox::MessageBox(const std::string& text, int label_index) : neroshop::MessageBox() {
    set_text(text);
}
////////////////////
neroshop::MessageBox::MessageBox(const std::string& text, int red, int green, int blue, int label_index) : neroshop::MessageBox() {
    set_text(text, red, green, blue, 
    #if defined(NEROSHOP_USE_DOKUN_UI)
        label_list[label_index]->get_color().w);
    #else
        label_index);
    #endif
}
////////////////////
neroshop::MessageBox::MessageBox(const std::string& text, int red, int green, int blue, double alpha, int label_index) : neroshop::MessageBox() {
    set_text(text, red, green, blue, alpha);
}
////////////////////
neroshop::MessageBox::MessageBox(const std::string& text, std::string color, int label_index) : neroshop::MessageBox() {
    set_text(text, color);
}
////////////////////
neroshop::MessageBox::~MessageBox() {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    // delete button(s), edit(s), and label(s)
    destroy_children();
    // no need to delete box now that it is a shared_ptr
    // delete box
    /*if(box) {
        delete box;
        box = nullptr;
    }*/
    // I guess this is how to properly delete a smart pointer
    box.reset();
	// clear the bottom-level gui list
    bottom_level_gui_list.clear();
    #endif
}
////////////////////
neroshop::MessageBox * neroshop::MessageBox::first(nullptr);
////////////////////
neroshop::MessageBox * neroshop::MessageBox::second(nullptr);
////////////////////
//neroshop::MessageBox * neroshop::MessageBox::third(nullptr);
////////////////////
////////////////////
void neroshop::MessageBox::initialize() 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(box) return; // box must be uninitialized before it can be initialized
#ifdef NEROSHOP_DEBUG0    
    std::cout << "message_box initialized\n";
#endif
    // create a message_box - box size set in MessageBox::restore()
    box = std::unique_ptr<Box>(new Box());//std::make_shared<Box>();
    //box->set_outline(true);
    box->set_draggable(true);
    box->set_color(167,173,186);//(101,115,126);//(54,69,79);//(112,128,144);//(17,17,24);//(72, 88, 111, 1.0);
    // message_box label - label defaults set in MessageBox::restore()
    dokun::Label * box_label = new dokun::Label();
    box_label->set_font(*new dokun::Font(DOKUN_DEFAULT_FONT_PATH));
    box->set_label(*box_label);
    // message_box title bar
    box->set_title_bar(true);
    box->set_title_bar_color(21, 34, 56, 1.0);//(15, 46, 83, 1.0);
    // message_box title bar label
    dokun::Label * title_label = new dokun::Label();
    title_label->set_font(*new dokun::Font(DOKUN_DEFAULT_FONT_PATH));
    title_label->set_string("neroshop");
    title_label->set_alignment("center");
    box->set_title_bar_label(*title_label);
    // box label will be pushed into position 0 of label_list
    std::shared_ptr<dokun::Label> box_label_ptr (box_label);//converts raw_ptr to shared_ptr
    label_list.push_back(box_label_ptr);//std::cout << "(label_list[0] == box->label): " << (String(label_list[0].get()) == String(box_label)) << "" << std::endl;
    box->hide(); // by default msg_box is hidden
    #endif
}
////////////////////
void neroshop::MessageBox::show()
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    // if message is already being showned, create a second instance of message or ?
    box->show();
    // children have their own visibility so they can be hidden even when box is visible
    #endif
}
////////////////////
void neroshop::MessageBox::hide()
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->hide();
    // children have their own visibility but are hidden when box is not visible, since they can only appear on top of box
    #endif
}
////////////////////
void neroshop::MessageBox::draw()
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    on_draw();
    box->draw();
    // draw children too
    draw_children();
    #endif
}
////////////////////
void neroshop::MessageBox::center(unsigned int window_width, unsigned int window_height) 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->set_position((window_width / 2) - (box->get_width() / 2), (window_height / 2) - (box->get_height() / 2));    
    #endif
}
////////////////////
void neroshop::MessageBox::restore() 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    // restore defaults
    for(auto labels : label_list) {
        labels->clear(); // clear label string
        labels->set_color((box->get_color() > Vector4(0, 0, 0, 1.0)) ? Vector4(0, 0, 0, 1.0) : Vector4(255, 255, 255, 1.0));//box->get_label()->set_color((box->get_color() > Vector4(0, 0, 0, 1.0)) ? Vector4(0, 0, 0, 1.0) : Vector4(255, 255, 255, 1.0)); // restore original color (on hide)
        labels->set_alignment("center");
    }
    for(auto buttons : button_list) {}
    for(auto edits : edit_list) {
        if(edits->is_empty()) continue; // if edit is already empty then skip it
        // un-comment the line below when ready for release
        ////edits->clear_all(); // clear all text from edit
    }
    // hide all buttons, edits, labels, etc. (only works now cuz I didn't set_parent to box)
    hide_children();
    //std::cout << "message_box restored\n";
    #endif
}
////////////////////
void neroshop::MessageBox::add_label(int relative_x, int relative_y) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    // set the limit of labels box can hold to 3
    if(label_list.size() > 2) {
        neroshop::print("You've reached the maximum number of box labels (3)", 1);
        return;
    }
    //////////////////////////////
    std::shared_ptr<dokun::Label> label = std::make_shared<dokun::Label>();
	//std::cout << "label.get() = "<< label.get() << std::endl;
	//std::cout << "label.use_count() = " << label.use_count() << std::endl;    
    label->set_font(*new dokun::Font(DOKUN_DEFAULT_FONT_PATH));
    //////////////////////////////
    // get previous label in label_list
    //int previous_label_index = label_list.size() - 1;
    //std::cout << "last label index: " << previous_label_index << std::endl;    
    //////////////////////////////
    // set label relative position
    label->set_relative_position(relative_x, relative_y);
    //////////////////////////////
    //////////////////////////////
    label_list.push_back(label);
    #endif
}
////////////////////
void neroshop::MessageBox::add_button(const std::string& text, int relative_x, int relative_y, int width, int height) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    std::shared_ptr<Button> button = std::make_shared<Button>();//Button * button = new Button();
	//std::cout << "button.get() = "<< button.get() << std::endl;
	//std::cout << "button.use_count() = " << button.use_count() << std::endl;    
    //////////////////////////////
    button->set_size(width, height);
    //////////////////////////////
    // button label
    dokun::Label * button_label = new dokun::Label(text);
    button_label->set_alignment("center"); // center
    button->set_label(* button_label); // set
    //////////////////////////////
    // button relative position
    button->set_relative_position(relative_x, relative_y);
    //////////////////////////////
    button_list.push_back(button);
    #endif
}
////////////////////
void neroshop::MessageBox::add_edit(int relative_x, int relative_y, int width, int height) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    std::shared_ptr<Edit> edit = std::make_shared<Edit>();//Edit * edit = new Edit();
	//std::cout << "edit.get() = "<< edit.get() << std::endl;
	//std::cout << "edit.use_count() = " << edit.use_count() << std::endl;
	//////////////////////////////
	edit->set_character_limit(256);
	edit->set_size(width, height);    
    //////////////////////////////
    // edit label
    dokun::Label * edit_label = new dokun::Label();
    edit_label->set_font(*new dokun::Font(DOKUN_DEFAULT_FONT_PATH));
	edit_label->set_color(32, 32, 32);
	edit->set_label(* edit_label);
	//////////////////////////////
	// edit relative position
    edit->set_relative_position(relative_x, relative_y);// center the x-axis position
    //////////////////////////////
    // scale to fit
    //if(edit->get_width() > box->get_width()) edit->set_width(box->get_width() - 10);
    //if(edit->get_height() > box->get_height()) edit->set_height(box->get_height() - 10);
    //////////////////////////////
    edit_list.push_back(edit);
    #endif
}
////////////////////
void neroshop::MessageBox::destroy_children() {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(button_list.empty() && edit_list.empty() && label_list.empty()) return;
    // SHARED_PTRS ARE AUTOMATICALLY DELETED SO THEY CANNOT BE MANUALLY DELETED
	// I guess this is how to properly delete a smart pointer
	for(auto labels : label_list) labels.reset();
	for(auto buttons : button_list) buttons.reset();
	for(auto edits : edit_list) edits.reset();    
    // its ok to use normal for-loops since I am using less than 10 buttons
    // 1. remove objects from vector
    // 2. delete objects
    /*while(!button_list.empty()) {
        Button * button = button_list.front();
        button_list.erase(button_list.begin());
        if(button) {
            delete button;
            //button = nullptr;
        }
    }*/
    // this causes munmap_chunk(): invalid pointer, segfault, double free or corruption (out), etc.
    /*while(!edit_list.empty()) {
        Edit * edit = edit_list.front();
        edit_list.erase(edit_list.begin());
        if(edit) {
            delete edit;
            edit = nullptr;
        }
    }*/
    //button_list[i].reset(); // deletes managed object
    //edit_list[i].reset();
    //if(i != 0) label_list[i].reset();
    #endif
}
////////////////////
void neroshop::MessageBox::draw_children() {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(button_list.empty() && edit_list.empty() && label_list.empty()) return;
    for(auto buttons : button_list) {
        //if(buttons == nullptr) continue;
        // set position relative to box
        buttons->set_position(box->get_x() + buttons->get_relative_x(), box->get_y() + buttons->get_relative_y());
        // make sure that child cannot go past box bounds
	    if(buttons->get_relative_x() >= (box->get_width() - buttons->get_width())) { 
	        buttons->set_position(box->get_x() + (box->get_width() - buttons->get_width()), buttons->get_y()); 
	        buttons->set_relative_position(box->get_width() - buttons->get_width(), buttons->get_relative_y());
	    }
	    if(buttons->get_x() <= box->get_x()) buttons->set_position(box->get_x(), buttons->get_y());
        if(buttons->get_y() <= box->get_y()) buttons->set_position(buttons->get_x(), box->get_y());
	    if(buttons->get_relative_y() >= (box->get_height() - buttons->get_height())) {
	        buttons->set_position(buttons->get_x(), box->get_y() + (box->get_height() - buttons->get_height()));
	        buttons->set_relative_position(buttons->get_relative_x(), (box->get_height() - buttons->get_height()) );
	    }
        // draw buttons
        buttons->draw();
    }  
    for(auto edits : edit_list) {
        //if(edits == nullptr) continue;
        // set position relative to box
        edits->set_position(box->get_x() + edits->get_relative_x(), box->get_y() + edits->get_relative_y());
        // make sure that child cannot go past box bounds
	    if(edits->get_relative_x() >= (box->get_width() - edits->get_width())) { 
	        edits->set_position(box->get_x() + (box->get_width() - edits->get_width()), edits->get_y());
	        edits->set_relative_position(box->get_width() - edits->get_width(), edits->get_relative_y());
	    }
	    if(edits->get_x() <= box->get_x()) edits->set_position(box->get_x(), edits->get_y());
        if(edits->get_y() <= box->get_y()) edits->set_position(edits->get_x(), box->get_y());
	    if(edits->get_relative_y() >= (box->get_height() - edits->get_height())) {
	        edits->set_position(edits->get_x(), box->get_y() + (box->get_height() - edits->get_height()));
	        edits->set_relative_position(edits->get_relative_x(), (box->get_height() - edits->get_height()));
	    }
        // draw edits        
        edits->draw();
    }
    for(auto labels : label_list) {
        if(labels == label_list[0]) continue;//NO need to draw the default box label, the box will draw it automatically ^_^, so we will skip it for now
        // set position relative to box
        labels->set_position(box->get_x() + labels->get_relative_x(), box->get_y() + labels->get_relative_y());
        // make sure that child cannot go past box bounds
	    if(labels->get_relative_x() >= (box->get_width() - (labels->get_string().length() * 10)/*labels->get_width()*/)) { 
	        labels->set_position(box->get_x() + (box->get_width() - (labels->get_string().length() * 10)/*labels->get_width()*/), labels->get_y());
	        labels->set_relative_position(box->get_width() - (labels->get_string().length() * 10)/*labels->get_width()*/, labels->get_relative_y());
	    }
	    if(labels->get_x() <= box->get_x()) labels->set_position(box->get_x(), labels->get_y());
        if(labels->get_y() <= box->get_y()) labels->set_position(labels->get_x(), box->get_y());
	    if(labels->get_relative_y() >= (box->get_height() - 10/*labels->get_height()*/)) {
	        labels->set_position(labels->get_x(), box->get_y() + (box->get_height() - 10/*labels->get_height()*/));
	        labels->set_relative_position(labels->get_relative_x(), (box->get_height() - 10/*labels->get_height()*/));
	    }
        // draw labels
        labels->draw();
    }
    #endif
}
////////////////////
void neroshop::MessageBox::hide_children() {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(button_list.empty() && edit_list.empty() && label_list.empty()) return;
    for(auto buttons : button_list) {
        //if(buttons == nullptr) continue;
        buttons->set_visible(false);
    }
    for(auto edits : edit_list) {
        //if(edits == nullptr) continue;
        edits->set_visible(false);
    }
    for(auto labels : label_list) {
        labels->set_visible(false);
    }
    #endif
}
////////////////////
void neroshop::MessageBox::show_children() {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(button_list.empty() && edit_list.empty() && label_list.empty()) return;
    for(auto buttons : button_list) {
        //if(buttons == nullptr) continue;
        buttons->set_visible(true);
    }
    for(auto edits : edit_list) {
        //if(edits == nullptr) continue;
        edits->set_visible(true);
    }
    for(auto labels : label_list) {
        labels->set_visible(true);
    }
    #endif
}
////////////////////
void neroshop::MessageBox::adjust_box() {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    // adjust box width based on label width
    std::vector<int> label_widths = {};
    for(auto labels : label_list) label_widths.push_back(labels->get_string().length() * 10);//(labels->get_width());//std::cout << "label: \"" << labels->get_string() << "\" (width: " << (labels->get_string().length() * 10)/*labels->get_width()*/ << ")" << std::endl;
    // get the biggest label_width in label_list
    int biggest_width_label = *max_element(label_widths.begin(), label_widths.end());//std::cout << "and the biggest width is: " << biggest_width_label << std::endl;
    int text_width = biggest_width_label;
    int box_width  = box->get_width();
    // if text_width bypasses box width
    if(text_width >= box->get_width()) {
        // increase box width by (text_width - box_width)
        box->set_width(box_width + (text_width - box_width) + 100); // add 100 to the width //same as: box->set_width(biggest_width_label + 100);//(label_list[biggest_width_label]->get_width() + 100);//(box->get_label()->get_width() + 100);//* 1.5);       
        //return; // exit function
    } 
    #endif
}
////////////////////
////////////////////
void neroshop::MessageBox::set_text(const std::string& text, int label_index) 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(label_index > (label_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::set_text(): attempting to access invalid index" + std::string("\033[0m"));
    label_list[label_index]->set_string(text);//box->get_label()->set_string(text);
    // adjust box size based on label width and maybe other box contents
    adjust_box();
    #endif
}
////////////////////
void neroshop::MessageBox::set_text(const std::string& text, int red, int green, int blue, int label_index) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(label_index > (label_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::set_text(): attempting to access invalid index" + std::string("\033[0m"));
    label_list[label_index]->set_string(text);//box->get_label()->set_string(text);
    // set text color
    label_list[label_index]->set_color(red, green, blue, label_list[label_index]->get_color().w);//box->get_label()->set_color(red, green, blue, box->get_label()->get_color().w);
    // adjust box size based on label width and maybe other box contents
    adjust_box();
    #endif
}
////////////////////
void neroshop::MessageBox::set_text(const std::string& text, int red, int green, int blue, double alpha, int label_index) 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(label_index > (label_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::set_text(): attempting to access invalid index" + std::string("\033[0m"));
    label_list[label_index]->set_string(text);//box->get_label()->set_string(text);
    // set text color
    label_list[label_index]->set_color(red, green, blue, alpha);//box->get_label()->set_color(red, green, blue, alpha);
    // adjust box size based on label width and maybe other box contents
    adjust_box();
    #endif
}
////////////////////
void neroshop::MessageBox::set_text(const std::string& text, std::string color, int label_index) 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    if(label_index > (label_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::set_text(): attempting to access invalid index" + std::string("\033[0m"));
    label_list[label_index]->set_string(text);//box->get_label()->set_string(text);
    // set text color
    if(neroshop::string::lower(color) == "white") /*box->get_label()*/label_list[label_index]->set_color(255, 255, 255, 1.0);
    if(neroshop::string::lower(color) == "black") /*box->get_label()*/label_list[label_index]->set_color(0, 0, 0, 1.0);
    if(neroshop::string::lower(color) == "red") /*box->get_label()*/label_list[label_index]->set_color(255, 0, 0, 1.0);
    if(neroshop::string::lower(color) == "yellow") /*box->get_label()*/label_list[label_index]->set_color(255, 255, 0, 1.0);
    if(neroshop::string::lower(color) == "lime") /*box->get_label()*/label_list[label_index]->set_color(0, 255, 0, 1.0);
    if(neroshop::string::lower(color) == "cyan" || neroshop::string::lower(color) == "aqua") /*box->get_label()*/label_list[label_index]->set_color(0, 255, 255, 1.0);
    if(neroshop::string::lower(color) == "blue") /*box->get_label()*/label_list[label_index]->set_color(0, 0, 255, 1.0);  
    if(neroshop::string::lower(color) == "magenta" || neroshop::string::lower(color) == "fuchsia") /*box->get_label()*/label_list[label_index]->set_color(255, 0, 255, 1.0);
    if(neroshop::string::lower(color) == "silver") /*box->get_label()*/label_list[label_index]->set_color(192, 192, 192, 1.0);
    if(neroshop::string::lower(color) == "gray" || neroshop::string::lower(color) == "grey") /*box->get_label()*/label_list[label_index]->set_color(128, 128, 128, 1.0);
    if(neroshop::string::lower(color) == "maroon") /*box->get_label()*/label_list[label_index]->set_color(128, 0, 0, 1.0);
    if(neroshop::string::lower(color) == "olive") /*box->get_label()*/label_list[label_index]->set_color(128, 128, 0, 1.0);
    if(neroshop::string::lower(color) == "green") /*box->get_label()*/label_list[label_index]->set_color(0, 128, 0, 1.0);
    if(neroshop::string::lower(color) == "purple" || neroshop::string::lower(color) == "violet") /*box->get_label()*/label_list[label_index]->set_color(128, 0, 128, 1.0);
    if(neroshop::string::lower(color) == "teal") /*box->get_label()*/label_list[label_index]->set_color(0, 128, 128, 1.0);
    if(neroshop::string::lower(color) == "navy") /*box->get_label()*/label_list[label_index]->set_color(0, 0, 128, 1.0);
    if(neroshop::string::lower(color) == "gold") /*box->get_label()*/label_list[label_index]->set_color(255, 215, 0, 1.0);
    if(neroshop::string::lower(color) == "royal blue") /*box->get_label()*/label_list[label_index]->set_color(65, 105, 225, 1.0);
    //if(neroshop::string::lower(color) == "") /*box->get_label()*/label_list[label_index]->set_color(255, 255, 255, 1.0);        
    // adjust box size based on label width and maybe other box contents
    adjust_box();
    #endif
}
////////////////////
void neroshop::MessageBox::set_title(const std::string& title) 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->get_title_bar_label()->set_string(title);
    #endif
}
////////////////////
void neroshop::MessageBox::set_position(int x, int y) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->set_position(x, y);
    #endif
}
////////////////////
#if defined(NEROSHOP_USE_DOKUN_UI)
void neroshop::MessageBox::set_bottom_level_gui_list(const std::vector<GUI *>& bottom_level_gui_list) {
    this->bottom_level_gui_list = bottom_level_gui_list;
}
#endif
////////////////////
////////////////////
void neroshop::MessageBox::set_width(int width) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->set_width(width);
    #endif
}
////////////////////
void neroshop::MessageBox::set_height(int height) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->set_height(height);
    #endif
}
////////////////////
void neroshop::MessageBox::set_size(int width, int height) {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    box->set_size(width, height);
    #endif
}
////////////////////
////////////////////
std::string neroshop::MessageBox::get_text(int label_index) const {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return label_list[label_index]->get_string();
    #endif
    return "";
}
////////////////////
std::string neroshop::MessageBox::get_title_text() const {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_title_bar_label()->get_string();
    #endif
    return "";
}
////////////////////
////////////////////
int neroshop::MessageBox::get_x() const {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_x();
    #endif
    return 0;
}
////////////////////
int neroshop::MessageBox::get_y() const {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_y();
    #endif
    return 0;
}
////////////////////
#if defined(NEROSHOP_USE_DOKUN_UI)
Vector2 neroshop::MessageBox::get_position() const {
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_position();
}
#endif
////////////////////
////////////////////
int neroshop::MessageBox::get_width() const {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_width();
    #endif
    return 0;
}
////////////////////
int neroshop::MessageBox::get_height() const {
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_height();
    #endif
    return 0;
}
////////////////////
#if defined(NEROSHOP_USE_DOKUN_UI)
Vector2 neroshop::MessageBox::get_size() const {
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->get_size();
}
#endif
////////////////////
// If a object is allocated inside a function, then it means you do not own it
// which is why the buttons were not showing up
neroshop::MessageBox * neroshop::MessageBox::get_first() {
    return first;
}
////////////////////
neroshop::MessageBox * neroshop::MessageBox::get_second() {
    return second;
}
////////////////////
neroshop::MessageBox * neroshop::MessageBox::get_singleton() {
    return get_first();
}
////////////////////
neroshop::MessageBox * neroshop::MessageBox::get_doubleton() {
    return get_second();
}
////////////////////
////////////////////
#if defined(NEROSHOP_USE_DOKUN_UI)
Box * neroshop::MessageBox::get_box() const {
    return box.get();
}
////////////////////
Button * neroshop::MessageBox::get_button(int index) const {
    if(index > (button_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::get_button(int): attempting to access invalid index" + std::string("\033[0m"));
    if(!button_list[index].get()) neroshop::print("button " + std::to_string(index) + " is nullptr", 1);
    return button_list[index].get();
}
////////////////////
Edit * neroshop::MessageBox::get_edit(int index) const {
    if(index > (edit_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::get_edit(int): attempting to access invalid index" + std::string("\033[0m"));
    if(!edit_list[index].get()) neroshop::print("edit " + std::to_string(index) + " is nullptr", 1);
    return edit_list[index].get();
}
////////////////////
dokun::Label * neroshop::MessageBox::get_label(int index) const {
    if(index > (label_list.size()-1)) throw std::runtime_error(std::string("\033[0;91m") + "neroshop::MessageBox::get_label(int): attempting to access invalid index" + std::string("\033[0m"));
    if(!label_list[index].get()) neroshop::print("label " + std::to_string(index) + " is nullptr", 1);
    return label_list[index].get();
}
////////////////////
////////////////////
////////////////////
int neroshop::MessageBox::get_button_count() const {
    return button_list.size();
}
////////////////////
int neroshop::MessageBox::get_edit_count() const {
    return edit_list.size();
}
////////////////////
int neroshop::MessageBox::get_label_count() const {
    return label_list.size();
}
#endif
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::MessageBox::is_visible() 
{
    #if defined(NEROSHOP_USE_DOKUN_UI)
    if(!box) throw std::runtime_error("message box is not initialized");
    return box->is_visible();
    #endif
    return false;
}
////////////////////
void neroshop::MessageBox::on_draw() { // call this function BEFORE calling draw()
    #if defined(NEROSHOP_USE_DOKUN_UI)
    // deactivates all gui at the bottom-level of the top-level gui element (this) so that bottom-level gui elements may not receive any input from the user while the top-level gui element (this) is visible       
    for(auto bottom_guis : bottom_level_gui_list) {
        if(box->is_visible()) bottom_guis->set_active(false); else bottom_guis->set_active(true); // or use: if(box->is_collided(*bottom_guis)) but is_visible works much better since title_bar would also be applied
    }	
	//if(dokun::Keyboard::is_pressed(DOKUN_KEY_ESCAPE) && !MessageBox::is_visible()) {
	    // do you wish to exit the program
		//window.destroy();
	//}		    
	if(dokun::Keyboard::is_pressed(DOKUN_KEY_ESCAPE)) MessageBox::hide();
	if(!MessageBox::is_visible()) MessageBox::restore(); // restore defaults if hidden
	#endif
}
////////////////////
