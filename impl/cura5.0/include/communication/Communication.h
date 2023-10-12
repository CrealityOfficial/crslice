//  Copyright (c) 2018-2022 Ultimaker B.V.
//  CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

namespace cura52
{
    class Scene;
    /*
     * An abstract class to provide a common interface for all methods of
     * communicating instructions from and to CuraEngine.
     */
    class Communication
    {
    public:
        /*
         * \brief Close the communication channel.
         */
        virtual ~Communication() {}

        /*
         * \brief Test if there are more slices to be queued.
         */
        virtual bool hasSlice() const = 0;

        virtual Scene* createSlice() = 0;
    };

} //namespace cura52

#endif //COMMUNICATION_H

