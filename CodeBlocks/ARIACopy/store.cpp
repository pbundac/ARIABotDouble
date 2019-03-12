#include "Aria.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


char input;
ArRobot robot;
ArSonarDevice sonar;

int main(int argc, char** argv)
{

  // Initialize some global data
  Aria::init();

  // This object parses program options from the command line
  ArArgumentParser parser(&argc, argv);

  // Load some default values for command line arguments from /etc/Aria.args
  // (Linux) or the ARIAARGS environment variable.
  parser.addDefaultArgument("-rh 10.0.126.14");

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
  if (!robotConnector.connectRobot())
  {
    // Error connecting:
    // if the user gave the -help argumentp, then just print out what happened,
    // and continue so options can be displayed later.
    if (!parser.checkHelpAndWarnUnparsed())
    {
      ArLog::log(ArLog::Terse, "Could not connect to robot, will not have parameter file so options displayed later may not include everything");
    }
    // otherwise abort
    else
    {
      ArLog::log(ArLog::Terse, "Error, could not connect to robot.");
      Aria::logOptions();
      Aria::exit(1);
    }
  }

  if(!robot.isConnected())
  {
    ArLog::log(ArLog::Terse, "Internal error: robot connector succeeded but ArRobot::isConnected() is false!");
  }

  // Connector for laser rangefinders
  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);

  // Connector for compasses
  ArCompassConnector compassConnector(&parser);

  // Parse the command line options. Fail and print the help message if the parsing fails
  // or if the help was requested with the -help option
  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
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

  // Sleep for a second so some messages from the initial responses
  // from robots and cameras and such can catch up
   // add a set of actions that combine together to effect the wander behavior
ArActionStallRecover recover;
ArActionBumpers bumpers;
ArActionAvoidFront avoidFrontNear("Avoid Front Near", 225, 0);
ArActionAvoidFront avoidFrontFar;
ArActionConstantVelocity constantVelocity("Constant Velocity", 400);
robot.addAction(&recover, 100);
robot.addAction(&bumpers, 75);
robot.addAction(&avoidFrontNear, 50);
robot.addAction(&avoidFrontFar, 49);
robot.addAction(&constantVelocity, 25);
  ArUtil::sleep(1000);

    robot.setRotVel(0);
    robot.setVel(250);

    ArUtil::sleep(3000);

    robot.setRotVel(0);
    robot.setVel(-250);

    ArUtil::sleep(3000);

    robot.setRotVel(0);
    robot.setRotVel(30);
    ArUtil::sleep(3000);

    robot.setRotVel(0);
    robot.setVel(-400);
    ArUtil::sleep(3000);

  Aria::exit(0);
 return 0;

}
