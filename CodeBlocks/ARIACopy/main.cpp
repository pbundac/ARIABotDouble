#include "Aria.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

char input;
ArRobot robot;
ArSonarDevice sonar;
pthread_mutex_t muteXity;

// Wandering mode
void * wander(void * a){
    printf("Wandering mode initialized! \n");
    ArActionStallRecover recover;
    ArActionBumpers bumpers;
    ArActionAvoidFront avoidFrontNear("Avoid Front Near", 225, 0);
    ArActionAvoidFront avoidFrontFar;
    ArActionConstantVelocity constantVelocity("ConstantVelocity", 200);

    scanf(" %c", &input);

    while(1){
        usleep(500);

        robot.lock();
        robot.clearDirectMotion();
        robot.enableMotors();
        robot.unlock();

        pthread_mutex_lock(&muteXity);

        robot.addAction(&recover, 100);
        robot.addAction(&bumpers, 75);
        robot.addAction(&avoidFrontNear, 50);
        robot.addAction(&avoidFrontFar, 49);
        robot.addAction(&constantVelocity, 25);


        while (input != 't'){
            printf("They see me wanderin'... they hatin'...\n");
            printf("Press t to enter teleoperations mode: ");
            scanf(" %c", &input);
        }

        pthread_mutex_unlock(&muteXity);
    }
}

// Tele-Operation mode
void * teleop(void * a){
    printf("Teleoperations mode initialized! \n");

    printf("Press 8 to move forward.\n");
    printf("Press 2 to move backwards.\n");
    printf("Press 4 to turn left.\n");
    printf("Press 6 to turn right.\n");
    printf("Press 0 to halt movement.\n");
    while(1){
        // printf("Teleoperations Mode Initiated: ");
        usleep(500);
        robot.lock();
        robot.clearDirectMotion();
        robot.enableMotors();
        robot.unlock();
        pthread_mutex_lock(&muteXity);
        scanf(" %c", &input);

        while(input != 'w'){
            switch(input){
                case '8':
                    printf("Forward movement initiated.\n");
                    robot.setVel(250);
                    break;
                case '2':
                    printf("Backward movement initiated.\n");
                    robot.setVel(-250);
                    break;
                case '6':
                    printf("Right side movement initiated.\n");
                    robot.setRotVel(-30);
                    break;
                case '4':
                    robot.setRotVel(30);
                    break;
                case '0':
                    printf("Movement halted.\n");
                    robot.stop();
                    break;
                default:
                    printf("Invalid choice.\n");
                    break;
                }
            printf("Move! Get out the way!\n");
            printf("Press w to enter wandering mode: ");
            scanf(" %c", &input);
        }
    pthread_mutex_unlock(&muteXity);
    }
}

int main(int argc, char** argv){
    // Initialize some global data
    Aria::init();

    // This object parses program options from the command line
    ArArgumentParser parser(&argc, argv);

    // Load some default values for command line arguments from /etc/Aria.args
    // (Linux) or the ARIAARGS environment variable.
    parser.addDefaultArgument("-rh 10.0.126.18");

    // Central object that is an interface to the robot and its integrated
    // devices, and which manages control of the robot by the rest of the program.
    //ArRobot robot;

    // Object that connects to the robot or simulator using program options
    ArRobotConnector robotConnector(&parser, &robot);

    // If the robot has an Analog Gyro, this object will activate it, and
    // if the robot does not automatically use the gyro to correct heading,
    // this object reads data from it and corrects the pose in ArRobot

    // Connect to the robot, get some initial data from it such as type and name,
    // and then load parameter files for this robot.
    if (!robotConnector.connectRobot()){
        // Error connecting:
        // if the user gave the -help argumentp, then just print out what happened,
        // and continue so options can be displayed later.
        if (!parser.checkHelpAndWarnUnparsed()){
          ArLog::log(ArLog::Terse, "Could not connect to robot, will not have parameter file so options displayed later may not include everything");
        }
        // otherwise abort
        else{
          ArLog::log(ArLog::Terse, "Error, could not connect to robot.");
          Aria::logOptions();
          Aria::exit(1);
        }
    }

    if(!robot.isConnected()){
        ArLog::log(ArLog::Terse, "Internal error: robot connector succeeded but ArRobot::isConnected() is false!");
      }

    // Connector for laser rangefinders
    ArLaserConnector laserConnector(&parser, &robot, &robotConnector);

    // Connector for compasses
    ArCompassConnector compassConnector(&parser);

    // Parse the command line options. Fail and print the help message if the parsing fails
    // or if the help was requested with the -help option
    if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed()){
        Aria::logOptions();
        Aria::exit(1);
        return 1;
    }

    // Attach sonarDev to the robot so it gets data from it.
    robot.addRangeDevice(&sonar);


    // Start the robot task loop running in a new background thread. The 'true' argument means if it loses
    // connection the task loop stops and the thread exits.
    robot.runAsync(true);
    robot.lock();
    robot.clearDirectMotion();
    robot.stop();
    robot.enableMotors();
    robot.unlock();

    // Create threads and begin wandering and teleop functionality.
    pthread_t thread1, thread2;

    pthread_mutex_init(&muteXity, NULL);

    pthread_create(&thread1, NULL, wander, NULL);
    pthread_create(&thread2, NULL, teleop, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&muteXity);

    pthread_exit(NULL);

    Aria::exit(0);
    return 0;
    }
