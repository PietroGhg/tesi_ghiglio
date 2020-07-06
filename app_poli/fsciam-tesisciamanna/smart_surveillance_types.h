#ifndef SMART_SURVEILLANCE_TYPES_
#define SMART_SURVEILLANCE_TYPES_

#include "Object.h"
enum{ FRAME_SENT=11, FRAME_RECEIVED, ARRAY_LENGHT,OBJ_SENT,OBJ_RECEIVED, READY, STOP_EXECUTION, CLOSE};

struct gui_result_t
{
  int id;
  Object obj;
};
struct sys_performance_t
{
  int processed_frames;
  float avg_fps;
};
#endif //SMART_SURVEILLANCE_TYPES_
