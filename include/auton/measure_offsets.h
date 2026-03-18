#pragma once

/**
 * @brief Measures and calibrates the odometry tracking wheel offsets
 * 
 * This auton runs multiple iterations of turning the robot to different angles
 * and measures how far the tracking wheels have moved. It calculates the offset
 * (distance from center of rotation) for each tracker and prints the results.
 * 
 * After running this, update the offset values in globals.cpp for the TrackingWheel
 * constructors based on the printed results.
 * 
 * @note Make sure the robot has sufficient space to turn in place during this auton
 */
void measure_offsets();
