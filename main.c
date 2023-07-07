// main-specific state, passed down to functions through args.
#include "common.h"
// inline function defs, #defines, constants
#include "defines.h"

// components/modules
#include "audio.h"
// the core
#include "cpu.h"
#include "video.h"

#include <stdlib.h>

CommonState *make_common_state() {
  CommonState *cs = (CommonState *)malloc(sizeof(CommonState));
  cs->is_running = malloc(1);
  return cs;
}

void clean_common_state(CommonState *cs) {
  free(cs->is_running);
  free(cs);
}

int main(int argc, char *argv[]) {
  CommonState *cs;

  { // the main initializer. call all the module inits.
    cs = make_common_state();

    { // create the cpu, read the file and handle the commandline args.
      if (argc < 2) {
        printf("Pass a path to a rom file.\n");
        return 1;
      }

      char *rom_path = argv[1];

      FILE *file = fopen(rom_path, "rb"); // Open the file in binary mode

      if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
      }

      cpu_init(file);

      fclose(file); // Close the file
    }

    video_init();
  }

  // the main loop, call all the update functions.
  for (;;) {
    // both can close the application for different reasons, marshall these
    // two variables into a single cs->is_running.
    cpu_update(cs->is_running);
    video_update(cs->is_running); // prefer the video update? how can i stop the
    // two modules from overwriting changes to the is_running signal? i could
    // check after each module update? should the cpu brk even close the
    // application?

    printf("cs: %d\n", *cs->is_running);
    if (*cs->is_running == 0)
      break;
  }

  // the main cleanup, call all the destructor functions.
  {
    cpu_clean();
    video_clean();
    clean_common_state(cs);
  }

  return 0;
}
