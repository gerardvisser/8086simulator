#ifndef __VIDEO_MODES_INCLUDED
#define __VIDEO_MODES_INCLUDED

#include <8086/VideoCard.h>

namespace videoModes {
  void setMode (VideoCard& videoCard, Memory& memory, int mode);
};

#endif
