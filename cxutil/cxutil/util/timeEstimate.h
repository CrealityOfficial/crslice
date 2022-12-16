#ifndef CXUTIL_TIME_ESTIMATE_H
#define CXUTIL_TIME_ESTIMATE_H
#include <stdint.h>
#include <vector>
#include "cxutil/settings/Duration.h"
#include "cxutil/settings/Velocity.h"
#include "cxutil/settings/PrintFeature.h"
#include "cxutil/settings/Ratio.h"

namespace cxutil
{
    struct TimeEstimateParam
    {
        Velocity machine_max_feedrate_x;
        Velocity machine_max_feedrate_y;
        Velocity machine_max_feedrate_z;
        Velocity machine_max_feedrate_e;
        Acceleration machine_max_acceleration_x;
        Acceleration machine_max_acceleration_y;
        Acceleration machine_max_acceleration_z;
        Acceleration machine_max_acceleration_e;
        Velocity machine_max_jerk_xy;
        Velocity machine_max_jerk_z;
        Velocity machine_max_jerk_e;
        Velocity machine_minimum_feedrate;
        Acceleration machine_acceleration;

        bool relative_extrusion;
        bool machine_use_extruder_offset_to_offset_coords;
    };

    /*!
     *  The TimeEstimateCalculator class generates a estimate of printing time calculated with acceleration in mind.
     *  Some of this code has been adapted from the Marlin sources.
     */

    class TimeEstimateCalculator
    {
    public:
        constexpr static unsigned int NUM_AXIS = 4;
        constexpr static unsigned int X_AXIS = 0;
        constexpr static unsigned int Y_AXIS = 1;
        constexpr static unsigned int Z_AXIS = 2;
        constexpr static unsigned int E_AXIS = 3;

        class Position
        {
        public:
            Position() { for (unsigned int n = 0; n < NUM_AXIS; n++) axis[n] = 0; }
            Position(double x, double y, double z, double e) { axis[0] = x; axis[1] = y; axis[2] = z; axis[3] = e; }
            double axis[NUM_AXIS];

            double& operator[](const int n) { return axis[n]; }
        };

        class Block
        {
        public:
            bool recalculate_flag;

            double accelerate_until;
            double decelerate_after;
            Velocity initial_feedrate;
            Velocity final_feedrate;

            Velocity entry_speed;
            Velocity max_entry_speed;
            bool nominal_length_flag;

            Velocity nominal_feedrate;
            double maxTravel;
            double distance;
            Acceleration acceleration;
            Position delta;
            Position absDelta;

            PrintFeatureType feature;
        };

    private:
        Velocity max_feedrate[NUM_AXIS] = { 600, 600, 40, 25 }; // mm/s
        Velocity minimumfeedrate = 0.01;
        Acceleration acceleration = 3000;
        Acceleration max_acceleration[NUM_AXIS] = { 9000, 9000, 100, 10000 };
        Velocity max_xy_jerk = 20.0;
        Velocity max_z_jerk = 0.4;
        Velocity max_e_jerk = 5.0;
        Duration extra_time = 0.0;

        Position previous_feedrate;
        Velocity previous_nominal_feedrate;

        Position currentPosition;

        std::vector<Block> blocks;
    public:
        /*!
         * \brief Set the movement configuration of the firmware.
         * \param settings_base Where to get the settings from.
         */
        void setFirmwareDefaults(const TimeEstimateParam& param);
        void setPosition(Position newPos);
        void plan(Position newPos, Velocity feedRate, PrintFeatureType feature);
        void addTime(const Duration& time);
        void setAcceleration(const Acceleration& acc); //!< Set the default acceleration to \p acc
        void setMaxXyJerk(const Velocity& jerk); //!< Set the max xy jerk to \p jerk

        void reset();

        std::vector<Duration> calculate();
    private:
        void reverse_pass();
        void forward_pass();
        void recalculate_trapezoids();

        void calculate_trapezoid_for_block(Block* block, const Ratio entry_factor, const Ratio exit_factor);
        void planner_reverse_pass_kernel(Block* previous, Block* current, Block* next);
        void planner_forward_pass_kernel(Block* previous, Block* current, Block* next);
    };

}//namespace cxutil
#endif//CXUTIL_TIME_ESTIMATE_H
