#include "main.h"
#include "globals.h"
#include "auton/autonRoutines.h"
#include "auton/autonFunctions.h"
#include "robodash/api.h"
#include <sys/_intsup.h>

// Create robodash console
rd::Console console("Console", &controller);

// Keep auton configuration in main.cpp like the previous workflow.
rd::Selector selector({
    {"Angular Test Auton", angular_test_auton, "", 60, "Heading turn diagnostic", 15000, "angular_test"},
    {"Lateral Test Auton", lateral_test_auton, "", 300, "Linear drive diagnostic", 15000, "lateral_test"},
    {"Skills Auton", skills_auton, "", 180, "Full skills run", 60000, "skills"},
}, &controller);

// Create image widget
rd::Image teamLogo("/img/gengy.bin", "Gengar");

// Create motor telemetry screen with motor groups and individual motors
// Automatically displays all motors from groups plus individual motors
std::vector<std::tuple<pros::MotorGroup*, const char*>> motor_groups = {
	{&leftMotors, "LFT"},
	{&rightMotors, "RGT"}
};
std::vector<std::tuple<pros::Motor*, const char*>> individual_motors = {
	{&intakeMotor, "INT"},
	{&topMotor, "TOP"}
};
rd::MotorTelemetry motorTelemetry("Motor Telemetry", motor_groups, individual_motors, &controller);

// Create PID tuner screen
rd::PIDTuner pidTuner("PID Tuner", &chassis, &controller);

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
    // Clear controller LCD on startup
    controller.clear();
    pros::delay(50); // Wait for clear to complete
    
    console.println("Initializing robot...");
    console.println("Calibrating sensors...");
    
    chassis.calibrate(); // calibrate sensors
    rightMotors.set_brake_mode(pros::E_MOTOR_BRAKE_COAST); // set brake mode to hold for better driver control
    leftMotors.set_brake_mode(pros::E_MOTOR_BRAKE_COAST); // set brake mode to hold for better driver control
    console.println("Calibration complete!");
 
    // Configure PID tuner increment values (optional)
    pidTuner.set_increments(1, 0.001, 0.5, 0.1);
    
    // ============================= PID Tuner Mode ============================= //
    // Toggle between PID tuner values and lemlib defaults
    // Keep enabled so tuner edits update the shared global controller settings and live chassis PID.
    pidTuner.set_use_tuner_pid(true);
    
    console.println("Robot initialized successfully!");

 
    // the default rate is 50. however, if you need to change the rate, you
    // can do the following.
    // lemlib::bufferedStdout().setRate(...);
    // If you use bluetooth or a wired connection, you will want to have a rate of 10ms
 
    // for more information on how the formatting for the loggers
    // works, refer to the fmtlib docs
 
    // thread to for position logging to console
    pros::Task screenTask([&]() {
        while (true) {
            // Position logging - not needed with new PID tuner screen
            pros::delay(1000);
        }
    });
    
// Background task to update motor telemetry
	pros::Task telemetryTask([&]() {
		while (true) {
			motorTelemetry.auto_update();
            pros::delay(50); // Update every 50ms
        }
    });
    
    // Background task to update PID tuner telemetry
    pros::Task pidTunerTask([&]() {
        while (true) {
            pidTuner.update();
            pros::delay(100); // Update every 100ms
        }
    });
    
    // Background task to update console (for controller scrolling)
    pros::Task consoleTask([&]() {
        while (true) {
            console.update();
            pros::delay(50); // Update every 50ms
        }
    });
    
}

/**
 * Runs once when entering disabled mode
 */
void disabled() {}

/**
 * Runs once when competition control is connected
 * Use this to focus the selector on screen
 */
void competition_initialize() {
    selector.focus();
}

/**
 * Runs the autonomous routine
 */
void autonomous() {
    console.println("=== AUTONOMOUS STARTED ===");
    selector.run_auton();
    console.println("=== AUTONOMOUS COMPLETE ===");
}

/**     
 * Runs in driver control
 */
void opcontrol() {
    printf("\n=== DRIVER CONTROL STARTED ===\n");
    fflush(stdout);
    
    // controller
    // loop to continuously update motors
    while (true) {
        // When PID tuner view is active, block normal driver/mechanism controls
        // so controller inputs only affect PID tuner.
        if (pidTuner.is_active()) {
            chassis.curvature(0, 0);
            intake_stop();
            pros::delay(10);
            continue;
        }

        // get joystick positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        // move the chassis with curvature drive
        chassis.curvature(leftY, rightX);
        
        // Button mappings for intake/scoring functions
        bool r1_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
        bool r2_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);
        bool l1_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
        bool l2_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);
        
        if (r1_pressed && r2_pressed) {
            score_longgoal();  // R1+R2 → score long goal
        }
        else if (r1_pressed) {
            storage();  // R1 → storage
        }
        else if (l1_pressed) {
            score_bottomgoal();  // L1 → score low goal
        }
        else {
            intake_stop();  // Stop intake when no buttons pressed
        }
        // Wing control: L2 held extends wing (up)
        if (l2_pressed) {
            wing.extend();
        } else {
            wing.retract();
        }
        
        // Button mappings for tongue and middle goal
        bool x_pressed = controller.get_digital(pros::E_CONTROLLER_DIGITAL_X);

        // A click toggles tongue
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
            tongue.toggle();
        }

        // X held scores middle goal (pulls down pulldown), otherwise stay extended
        if (x_pressed) {
            score_midgoal();
        } else {
            pulldown.extend();
        }

        // delay to save resources
        pros::delay(10);
    }
}
