#define NOB_IMPLEMENTATION
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "./includes/nob/nob.h"

#define BUILD_FOLDER "./build"
#define SRC_FOLDER "./src"
#define DEPS_FOLDER "./includes"
#define TEMP_OBJ_FOLDER "./objs"

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    Procs procs = {0};

    if(!mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    if(!mkdir_if_not_exists(TEMP_OBJ_FOLDER)) return 1;

    static struct {
        const char *dep_path;
        const char *obj_path;
    } dep_targets[] = {
    };

    nob_cc(&cmd);
    nob_cc_flags(&cmd);

    nob_cc_inputs(&cmd, SRC_FOLDER"/main.c");
    nob_cc_output(&cmd, BUILD_FOLDER"/elc");
    nob_cmd_append(&cmd, "-I./includes/raylib");
    nob_cmd_append(&cmd, "-L./includes/raylib");
    nob_cmd_append(&cmd, "-lraylib");
    nob_cmd_append(&cmd, "-lm");

    if(!cmd_run(&cmd)) return 1;

    nob_cmd_append(&cmd, "./build/elc");

    if(!cmd_run(&cmd)) return 1;
}
