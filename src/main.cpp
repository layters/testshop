// neroshop
#include "../include/neroshop.hpp"
using namespace neroshop;
// dokun-ui
#include <build.hpp>
#include DOKUN_HEADER
using namespace dokun;

lua_State * neroshop::lua_state = luaL_newstate(); // lua_state should be initialized by default
int main() {
    uv_loop_t *loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);
    // Will quit as nothing to process in this event loop
    
    printf("Now quitting\n");
    
    uv_loop_close(loop);
    free(loop);    
    return 0;
}
