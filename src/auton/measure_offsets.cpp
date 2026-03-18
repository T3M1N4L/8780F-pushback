#include "main.h"
#include "globals.h"
#include <cmath>
#include <iostream>

#include "measure_offsets.h"

/**
 * @brief Measures and calibrates the odometry tracking wheel offsets
 *
 * This function performs multiple iterations of turning the robot to different
 * angles and measures how far each tracking wheel moved. It then calculates
 * the offset (distance from center of rotation) for each wheel.
 *
 * Results are printed to terminal. Update the offset values in globals.cpp
 * TrackingWheel constructors with the average measurements.
 */
void measure_offsets() {
    // Number of iterations to average
    const int iterations = 10;

    // Detect which wheels are configured
    bool has_vertical = (chassis.sensors.vertical1 != nullptr);
    bool has_horizontal = (chassis.sensors.horizontal1 != nullptr);

    // Storage for offset measurements
    double v_offset = 0.0, h_offset = 0.0;

    // Print header
    std::cout << "\n=== TRACKING WHEEL OFFSET CALIBRATION ===" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Configured wheels:" << std::endl;
    if (has_vertical) std::cout << "  - Vertical wheel" << std::endl;
    if (has_horizontal) std::cout << "  - Horizontal wheel" << std::endl;
    if (!has_vertical && !has_horizontal) {
        std::cout << "ERROR: No tracking wheels configured!" << std::endl;
        return;
    }
    std::cout << std::endl;

    // Main calibration loop
    for (int i = 0; i < iterations; i++) {
        // Reset the pose and trackers
        chassis.setPose(0, 0, 0);  // Reset position to origin with 0 heading
        if (has_vertical) chassis.sensors.vertical1->reset();
        if (has_horizontal) chassis.sensors.horizontal1->reset();

        // Set brake mode for turning
        chassis.setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);

        // Alternate between 90 and 270 degree turns
        double target_angle = (i % 2 == 0) ? 90.0 : 270.0;

        // Record heading before turn
        double heading_start = chassis.getPose().theta;  // Returns degrees by default

        // Turn to target heading at reduced speed
        // Using maxSpeed of 63 (half power) to match original
        chassis.turnToHeading(target_angle, 4000, {.maxSpeed = 63});

        // Get final heading after turn completes
        double heading_end = chassis.getPose().theta;

        // Calculate the angle turned in radians
        // Wrap the angle difference to handle crossing 0/360 boundary
        double angle_delta_deg = heading_end - heading_start;
        
        // Normalize angle to [-180, 180] range
        while (angle_delta_deg > 180) angle_delta_deg -= 360;
        while (angle_delta_deg < -180) angle_delta_deg += 360;
        
        double angle_delta_rad = angle_delta_deg * M_PI / 180.0;
        angle_delta_rad = std::fabs(angle_delta_rad);

        // Print iteration results
        std::cout << "Iteration " << (i + 1) << ": Angle turned: " << angle_delta_deg << "°" << std::endl;

        // Calculate offsets for configured wheels only
        if (has_vertical) {
            double v_delta = chassis.sensors.vertical1->getDistanceTraveled();
            double v_iter_offset = (angle_delta_rad > 0.001) ? (v_delta / angle_delta_rad) : 0.0;
            std::cout << "  Vertical: " << v_delta << " in -> offset: " << v_iter_offset << " in" << std::endl;
            v_offset += v_iter_offset;
        }

        if (has_horizontal) {
            double h_delta = chassis.sensors.horizontal1->getDistanceTraveled();
            double h_iter_offset = (angle_delta_rad > 0.001) ? (h_delta / angle_delta_rad) : 0.0;
            std::cout << "  Horizontal: " << h_delta << " in -> offset: " << h_iter_offset << " in" << std::endl;
            h_offset += h_iter_offset;
        }

        // Small delay between iterations
        pros::delay(250);
    }

    // Calculate averages
    if (has_vertical) v_offset /= iterations;
    if (has_horizontal) h_offset /= iterations;

    // Print final results
    std::cout << "\n=== CALIBRATION RESULTS ===" << std::endl;
    if (has_vertical) {
        std::cout << "Average Vertical Offset: " << v_offset << " inches" << std::endl;
        std::cout << "  Update: vertical_tracking_wheel = lemlib::TrackingWheel(..., " << v_offset << ")" << std::endl;
    }
    if (has_horizontal) {
        std::cout << "Average Horizontal Offset: " << h_offset << " inches" << std::endl;
        std::cout << "  Update: horizontal_tracking_wheel = lemlib::TrackingWheel(..., " << h_offset << ")" << std::endl;
    }
    std::cout << "\n=== CALIBRATION COMPLETE ===" << std::endl;
}
