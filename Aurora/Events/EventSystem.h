#pragma once
#include "EventUtilities.h"

/* Personal Notes:

    - We are currently proceeding with a "blocking" event system. This means that all received events are dealt with on the spot. A future implementation could be to 
    buffer events in an event queue and process them during the "events" part of the update stage.
*/

namespace Aurora
{

}