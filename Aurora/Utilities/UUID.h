#pragma once
#include <string>
#include <iostream>

namespace Aurora
{
    class UUID
    {
    public:
        std::string ToString() const
        {

        }

        bool operator==(const UUID& otherUUID)
        {

        }

        bool operator!=(const UUID& otherUUID)
        {
            !(*this == otherUUID);
        }

    private:

    };

    std::ostream& operator<<(std::ostream& os, const UUID& uuid)
    {
        
    }
}