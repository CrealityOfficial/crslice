#include <math.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "timeestimateklipper.h"

#include "timeEstimate.h"
#include "utils/math.h"
#include "settings/Settings.h"
#include "settings/types/Ratio.h"


namespace cura52
{
#define MINIMUM_PLANNER_SPEED 0.05 

	static inline double min(double x, double y)
	{
		return (x < y) ? x : y;
	}
	static inline double min(double a, double b, double c)
	{
		double d = min(a, b);
		return min(d, c);
	}
	static inline double max(double x, double y)
	{
		return (x > y) ? x : y;
	}

	static inline double square(double x)
	{
		return x * x;
	}

	static inline Velocity maxAllowableSpeed(const Acceleration& acceleration,
											 const Velocity& target_velocity,
											 double distance)
	{
		return sqrt(target_velocity * target_velocity - 2 * acceleration * distance);
	}

	static inline float estimateAccelerationDistance(const Velocity & initial_rate,
													 const Velocity & target_rate,
													 const Acceleration & acceleration)
	{
		if (acceleration == 0)
		{
			return 0.0;
		}
		return (square(target_rate) - square(initial_rate)) / (2.0 * acceleration);
	}

	static inline double intersectionDistance(const Velocity & initial_rate, 
											  const Velocity & final_rate,
											  const Acceleration & acceleration, 
											  double distance)
	{
		if (acceleration == 0.0){ return 0.0; }
		return (2.0 * acceleration * distance - square(initial_rate) + square(final_rate)) / (4.0 * acceleration);
	}

	static inline double accelerationTimeFromDistance(const Velocity & initial_feedrate,
													  const Velocity & distance,
													  const Acceleration & acceleration)
	{
		double discriminant = square(initial_feedrate) - 2 * acceleration * -distance;
		discriminant = std::max(0.0, discriminant);
		return (-initial_feedrate + sqrt(discriminant)) / acceleration;
	}



	TimeEstimateKlipper::TimeEstimateKlipper()
	{
	}

	TimeEstimateKlipper::~TimeEstimateKlipper()
	{
	}

	void TimeEstimateKlipper::reset()
	{
		extra_time = 0.0;
		blocks.clear();
		moves.clear();
	}

	//void TimeEstimateKlipper::setAccel(const Acceleration& acc)
	//{
	//	this->max_accel = acc;
	//}

	/*
	void TimeEstimateKlipper::calculateTrapezoidForBlock(Block* block, const Ratio entry_factor, const Ratio exit_factor)
	{
		const Velocity initial_feedrate = block->nominal_feedrate * entry_factor;
		const Velocity final_feedrate = block->nominal_feedrate * exit_factor;

		double accelerate_distance = estimateAccelerationDistance(initial_feedrate, block->nominal_feedrate, block->acceleration);
		const double decelerate_distance = estimateAccelerationDistance(block->nominal_feedrate, final_feedrate, -block->acceleration);

		// Calculate the size of Plateau of Nominal Rate.
		double plateau_distance = block->distance - accelerate_distance - decelerate_distance;

		// Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
		// have to use intersection_distance() to calculate when to abort acceleration and start braking
		// in order to reach the final_rate exactly at the end of this block.
		if (plateau_distance < 0)
		{
			accelerate_distance = intersectionDistance(initial_feedrate, final_feedrate, block->acceleration, block->distance);
			accelerate_distance = std::max(accelerate_distance, 0.0); // Check limits due to numerical round-off
			accelerate_distance = std::min(accelerate_distance, block->distance);//(We can cast here to unsigned, because the above line ensures that we are above zero)
			plateau_distance = 0;
		}

		block->accelerate_until = accelerate_distance;
		block->decelerate_after = accelerate_distance + plateau_distance;
		block->initial_feedrate = initial_feedrate;
		block->final_feedrate = final_feedrate;
	}
	*/

	void TimeEstimateKlipper::plan(Position newPos, Velocity feedRate, PrintFeatureType feature)
	{
		Block block;
		memset(&block, 0, sizeof(block));

		Move move;
		memset(&move, 0, sizeof(move));

		move.theF = feedRate;
		double square_corner_velocity = 5.0;
		double scv2 = square_corner_velocity * square_corner_velocity;
		move.junction_deviation = scv2 * (sqrt(2.) - 1.) / acceleration/*max_accel*/;
		move.accel = acceleration/*max_accel*/;
		move.max_velocity = 1000.0;

		for (size_t n = 0; n < NUM_AXIS; n++)
		{
			move.axes_d[n] = newPos[n] - currentPosition[n];
		}
		move.move_d = 0.0;
		for (size_t n = 0; n < 3; n++)
		{
			move.move_d += (move.axes_d[n] * move.axes_d[n]);
		}
		move.move_d = sqrt(move.move_d);      //dL = sqrt(dX*dX + dY*dY + dZ*dZ)

		double velocity = min(feedRate, move.max_velocity);
		move.is_kinematic_move = true;

		double inv_move_d = 0.;
		if (move.move_d < .000000001)
		{
			move.move_d = abs(move.axes_d[3]);
			inv_move_d = 0.;
			if (move.move_d)
			{
				inv_move_d = 1.0 / move.move_d;
			}
			move.accel = 99999999.9;
			velocity = feedRate;
			move.is_kinematic_move = false;
		}
		else
		{
			inv_move_d = 1.0 / move.move_d;
		}

		for (size_t n = 0; n < NUM_AXIS; n++)
		{
			move.axes_r[n] = inv_move_d * move.axes_d[n];  //[dX/dL, dY/dL, dZ/dL, dE/dL]
		}
		move.min_move_t = move.move_d / velocity;

		double max_accel_to_decel = 0.0;
		double requested_accel_to_decel = 10000.0;
		max_accel_to_decel = min(requested_accel_to_decel, move.accel);

		move.max_start_v2 = 0.;
		move.max_cruise_v2 = velocity * velocity;
		move.delta_v2 = 2.0 * move.move_d * move.accel;
		move.max_smoothed_v2 = 0.;
		move.smooth_delta_v2 = 2.0 * move.move_d * max_accel_to_decel;

		moves.push_back(move);

		//=====================================================

		block.feature = feature;

		for (size_t n = 0; n < NUM_AXIS; n++)
		{
			block.delta[n] = newPos[n] - currentPosition[n];
			block.absDelta[n] = std::abs(block.delta[n]);
			block.maxTravel = std::max(block.maxTravel, block.absDelta[n]);
		}
		if (block.maxTravel <= 0)
		{
			return;
		}
		if (feedRate < minimumfeedrate)
		{
			feedRate = minimumfeedrate;
		}
		block.distance = sqrtf(square(block.absDelta[0]) + square(block.absDelta[1]) + square(block.absDelta[2]));
		if (block.distance == 0.0)
		{
			block.distance = block.absDelta[3];
		}
		block.nominal_feedrate = feedRate;

		Position current_feedrate;
		Position current_abs_feedrate;
		Ratio feedrate_factor = 1.0;
		for (size_t n = 0; n < NUM_AXIS; n++)
		{
			current_feedrate[n] = (block.delta[n] * feedRate) / block.distance;
			current_abs_feedrate[n] = std::abs(current_feedrate[n]);
			if (current_abs_feedrate[n] > max_feedrate[n])
			{
				feedrate_factor = std::min(feedrate_factor, Ratio(max_feedrate[n] / current_abs_feedrate[n]));
			}
		}
		//TODO: XY_FREQUENCY_LIMIT

		if (feedrate_factor < 1.0)
		{
			for (size_t n = 0; n < NUM_AXIS; n++)
			{
				current_feedrate[n] *= feedrate_factor;
				current_abs_feedrate[n] *= feedrate_factor;
			}
			block.nominal_feedrate *= feedrate_factor;
		}

		block.acceleration = acceleration;
		for (size_t n = 0; n < NUM_AXIS; n++)
		{
			if (block.acceleration * (block.absDelta[n] / block.distance) > max_acceleration[n])
			{
				block.acceleration = max_acceleration[n];
			}
		}

		Velocity vmax_junction = max_xy_jerk / 2;
		Ratio vmax_junction_factor = 1.0;
		if (current_abs_feedrate[Z_AXIS] > max_z_jerk / 2)
		{
			vmax_junction = std::min(vmax_junction, max_z_jerk / 2);
		}
		if (current_abs_feedrate[E_AXIS] > max_e_jerk / 2)
		{
			vmax_junction = std::min(vmax_junction, max_e_jerk / 2);
		}
		vmax_junction = std::min(vmax_junction, block.nominal_feedrate);
		const Velocity safe_speed = vmax_junction;

		if ((blocks.size() > 0) && (previous_nominal_feedrate > 0.0001))
		{
			const Velocity xy_jerk = sqrt(square(current_feedrate[X_AXIS] - previous_feedrate[X_AXIS]) + square(current_feedrate[Y_AXIS] - previous_feedrate[Y_AXIS]));
			vmax_junction = block.nominal_feedrate;
			if (xy_jerk > max_xy_jerk)
			{
				vmax_junction_factor = Ratio(max_xy_jerk / xy_jerk);
			}
			const double z_jerk = std::abs(current_feedrate[Z_AXIS] - previous_feedrate[Z_AXIS]);
			if (z_jerk > max_z_jerk)
			{
				vmax_junction_factor = std::min(vmax_junction_factor, Ratio(max_z_jerk / z_jerk));
			}
			const double e_jerk = std::abs(current_feedrate[E_AXIS] - previous_feedrate[E_AXIS]);
			if (e_jerk > max_e_jerk)
			{
				vmax_junction_factor = std::min(vmax_junction_factor, Ratio(max_e_jerk / e_jerk));
			}
			vmax_junction = std::min(previous_nominal_feedrate, vmax_junction * vmax_junction_factor); // Limit speed to max previous speed
		}

		block.max_entry_speed = vmax_junction;

		const Velocity v_allowable = maxAllowableSpeed(-block.acceleration, MINIMUM_PLANNER_SPEED, block.distance);
		block.entry_speed = std::min(vmax_junction, v_allowable);
		block.nominal_length_flag = block.nominal_feedrate <= v_allowable;
		block.recalculate_flag = true; // Always calculate trapezoid for new block

		previous_feedrate = current_feedrate;
		previous_nominal_feedrate = block.nominal_feedrate;

		currentPosition = newPos;

		calculateTrapezoidForBlock(&block, Ratio(block.entry_speed / block.nominal_feedrate), Ratio(safe_speed / block.nominal_feedrate));

		blocks.push_back(block);
	}


	std::vector<Duration> TimeEstimateKlipper::calculate()
	{
		reversePass();
		forwardPass();
		recalculateTrapezoids();

		
		for (unsigned int n = 1; n < moves.size(); n++)
		{
			Move& pre = moves[n - 1];
			Move& block = moves[n];
			if (block.is_kinematic_move && pre.is_kinematic_move)
			{
				double junction_cos_theta = -(block.axes_r[0] * pre.axes_r[0] + block.axes_r[1] * pre.axes_r[1] + block.axes_r[2] * pre.axes_r[2]);
				if (!(junction_cos_theta > 0.999999))
				{
					junction_cos_theta = max(junction_cos_theta, -0.999999);
					double sin_theta_d2 = sqrt(0.5 * (1.0 - junction_cos_theta));
					double R_jd = sin_theta_d2 / (1. - sin_theta_d2);
					double tan_theta_d2 = sin_theta_d2 / sqrt(0.5 * (1.0 + junction_cos_theta));
					double move_centripetal_v2 = .5 * block.move_d * tan_theta_d2 * block.accel;
					double prev_move_centripetal_v2 = (.5 * pre.move_d * tan_theta_d2 * pre.accel);
					double a1 = min(R_jd * block.junction_deviation * block.accel, R_jd * pre.junction_deviation * pre.accel);
					double a2 = min(move_centripetal_v2, prev_move_centripetal_v2, pre.max_start_v2 + pre.delta_v2);
					double a3 = min(block.max_cruise_v2, pre.max_cruise_v2);
					block.max_start_v2 = min(a1, a2, a3);
					block.max_smoothed_v2 = min(block.max_start_v2, pre.max_smoothed_v2 + pre.smooth_delta_v2);
				}
			}
		}

		unsigned int i0 = 0, i1 = 0;
		while (true)
		{
			double flushtime = 0.0;
			while (true)
			{
				if (i1 >= moves.size() - 1) { break; }
				Move a = moves[i1];
				flushtime += a.min_move_t;
				if (flushtime > 2.0) { break; }
				else { i1++; }
			}

			if (i1 >= moves.size() - 1) { break; }
			size_t flush_count = i1 - i0 + (size_t)1;
			std::vector<MyStruct> delayed;
			double next_end_v2 = 0.0;
			double next_smoothed_v2 = 0.0;
			double peak_cruise_v2 = 0.;

			for (unsigned int i = i1; i > i0; i--)
			{
				Move& move = moves[i];
				Block& block = blocks[i];
				double reachable_start_v2 = next_end_v2 + move.delta_v2;
				double start_v2 = min(move.max_start_v2, reachable_start_v2);
				double reachable_smoothed_v2 = next_smoothed_v2 + move.smooth_delta_v2;
				double smoothed_v2 = min(move.max_smoothed_v2, reachable_smoothed_v2);
				if (smoothed_v2 < reachable_smoothed_v2)
				{
					if (smoothed_v2 + move.smooth_delta_v2 > next_smoothed_v2 || !delayed.empty())
					{
						if (update_flush_count && peak_cruise_v2)
						{
							flush_count = i;
							update_flush_count = false;
						}
						peak_cruise_v2 = min(move.max_cruise_v2, (smoothed_v2 + reachable_smoothed_v2) * .5);
						if (!delayed.empty())
						{
							if (!update_flush_count && i < flush_count)
							{
								double mc_v2 = peak_cruise_v2;
								for (MyStruct a : delayed)
								{
									Move& m = a.mymove;
									Block& b = a.myblock;
									double ms_v2 = a.mystart;
									double me_v2 = a.myend;
									mc_v2 = min(mc_v2, ms_v2);
									double initial_feedrate = min(ms_v2, mc_v2);
									double nominal_feedrate = mc_v2;
									double final_feedrate = min(me_v2, mc_v2);
									double half_inv_accel = .5 / m.accel;

									initial_feedrate = min(initial_feedrate, square(b.initial_feedrate));
									nominal_feedrate = min(nominal_feedrate, square(b.nominal_feedrate));
									final_feedrate = min(final_feedrate, square(b.final_feedrate));

									m.accel_d = (nominal_feedrate - initial_feedrate) * half_inv_accel;
									m.decel_d = (nominal_feedrate - final_feedrate) * half_inv_accel;
									m.cruise_d = m.move_d - m.accel_d - m.decel_d;

									b.initial_feedrate = sqrt(initial_feedrate);
									b.nominal_feedrate = sqrt(nominal_feedrate);
									b.final_feedrate = sqrt(final_feedrate);
									b.accelerate_until = m.accel_d;
									b.decelerate_after = m.accel_d + m.cruise_d;
								}
							}
							delayed.clear();
						}
					}
					if (!update_flush_count && i < flush_count)
					{
						double cruise_v2 = min((start_v2 + reachable_start_v2) * 0.5, move.max_cruise_v2, peak_cruise_v2);
						double initial_feedrate = min(start_v2, cruise_v2);
						double nominal_feedrate = cruise_v2;
						double final_feedrate = min(next_end_v2, cruise_v2);
						double half_inv_accel = .5 / move.accel;

						initial_feedrate = min(initial_feedrate, square(block.initial_feedrate));
						nominal_feedrate = min(nominal_feedrate, square(block.nominal_feedrate));
						final_feedrate = min(final_feedrate, square(block.final_feedrate));

						move.accel_d = (nominal_feedrate - initial_feedrate) * half_inv_accel;
						move.decel_d = (nominal_feedrate - final_feedrate) * half_inv_accel;
						move.cruise_d = move.move_d - move.accel_d - move.decel_d;

						block.initial_feedrate = sqrt(initial_feedrate);
						block.nominal_feedrate = sqrt(nominal_feedrate);
						block.final_feedrate = sqrt(final_feedrate);
						block.accelerate_until = move.accel_d;
						block.decelerate_after = move.accel_d + move.cruise_d;
					}
				}
				else
				{
					MyStruct aa(block, move, start_v2, next_end_v2);
					delayed.push_back(aa);
				}
				next_end_v2 = start_v2;
				next_smoothed_v2 = smoothed_v2;
			}

			i1++;
			if (i1 >= blocks.size() - 1) { break; }
			i0 = i1;
		}
		

		std::vector<Duration> totals(static_cast<unsigned char>(PrintFeatureType::NumPrintFeatureTypes), 0.0);
		totals[static_cast<unsigned char>(PrintFeatureType::NoneType)] = extra_time; // Extra time (pause for minimum layer time, etc) is marked as NoneType
		for (unsigned int n = 0; n < blocks.size(); n++)
		{
			const Block& block = blocks[n];
			const double plateau_distance = block.decelerate_after - block.accelerate_until;

			totals[static_cast<unsigned char>(block.feature)] += accelerationTimeFromDistance(block.initial_feedrate, block.accelerate_until, block.acceleration);
			totals[static_cast<unsigned char>(block.feature)] += plateau_distance / block.nominal_feedrate;
			totals[static_cast<unsigned char>(block.feature)] += accelerationTimeFromDistance(block.final_feedrate, (block.distance - block.decelerate_after), block.acceleration);
		}
		return totals;
	}

	/*
	void TimeEstimateKlipper::plannerReversePassKernel(Block* previous, Block* current, Block* next)
	{
		(void)previous;
		if (!current || !next)
		{
			return;
		}

		// If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
		// If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
		// check for maximum allowable speed reductions to ensure maximum possible planned speed.
		if (current->entry_speed != current->max_entry_speed)
		{
			// If nominal length true, max junction speed is guaranteed to be reached. Only compute
			// for max allowable speed if block is decelerating and nominal length is false.
			if ((!current->nominal_length_flag) && (current->max_entry_speed > next->entry_speed))
			{
				current->entry_speed = std::min(current->max_entry_speed, maxAllowableSpeed(-current->acceleration, next->entry_speed, current->distance));
			}
			else
			{
				current->entry_speed = current->max_entry_speed;
			}
			current->recalculate_flag = true;
		}
	}

	void TimeEstimateKlipper::reversePass()
	{
		Block* block[3] = { nullptr, nullptr, nullptr };
		for (size_t n = blocks.size() - 1; int(n) >= 0; n--)
		{
			block[2] = block[1];
			block[1] = block[0];
			block[0] = &blocks[n];
			plannerReversePassKernel(block[0], block[1], block[2]);
		}
	}

	void TimeEstimateKlipper::plannerForwardPassKernel(Block* previous, Block* current, Block* next)
	{
		(void)next;
		if (!previous)
		{
			return;
		}

		// If the previous block is an acceleration block, but it is not long enough to complete the
		// full speed change within the block, we need to adjust the entry speed accordingly. Entry
		// speeds have already been reset, maximized, and reverse planned by reverse planner.
		// If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
		if (!previous->nominal_length_flag)
		{
			if (previous->entry_speed < current->entry_speed)
			{
				const Velocity entry_speed = std::min(current->entry_speed, maxAllowableSpeed(-previous->acceleration, previous->entry_speed, previous->distance));

				// Check for junction speed change
				if (current->entry_speed != entry_speed)
				{
					current->entry_speed = entry_speed;
					current->recalculate_flag = true;
				}
			}
		}
	}

	void TimeEstimateKlipper::forwardPass()
	{
		Block* block[3] = { nullptr, nullptr, nullptr };
		for (size_t n = 0; n < blocks.size(); n++)
		{
			block[0] = block[1];
			block[1] = block[2];
			block[2] = &blocks[n];
			plannerForwardPassKernel(block[0], block[1], block[2]);
		}
		plannerForwardPassKernel(block[1], block[2], nullptr);
	}

	void TimeEstimateKlipper::recalculateTrapezoids()
	{
		Block* current;
		Block* next = nullptr;

		for (unsigned int n = 0; n < blocks.size(); n++)
		{
			current = next;
			next = &blocks[n];
			if (current)
			{
				// Recalculate if current block entry or exit junction speed has changed.
				if (current->recalculate_flag || next->recalculate_flag)
				{
					// NOTE: Entry and exit factors always > 0 by all previous logic operations.
					calculateTrapezoidForBlock(current, Ratio(current->entry_speed / current->nominal_feedrate), Ratio(next->entry_speed / current->nominal_feedrate));
					current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
				}
			}
		}
		// Last/newest block in buffer. Exit speed is set with MINIMUM_PLANNER_SPEED. Always recalculated.
		if (next != nullptr)
		{
			calculateTrapezoidForBlock(next, Ratio(next->entry_speed / next->nominal_feedrate), Ratio(MINIMUM_PLANNER_SPEED / next->nominal_feedrate));
			next->recalculate_flag = false;
		}
	}
	*/
}