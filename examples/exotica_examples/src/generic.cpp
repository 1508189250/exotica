#include <exotica/Exotica.h>

using namespace exotica;

void run()
{
    ros::NodeHandle nhg_;

    // Specify fixed base robot in the base frame
    // Use all joints
    Initializer kinematica("Kinematica",std::map<std::string,boost::any>({
                    { "Root",Initializer("Limb",{ {"Segment",std::string("base")} }) },
                    { "Joints",std::string("lwr_arm_0_joint,lwr_arm_1_joint,lwr_arm_2_joint,lwr_arm_3_joint,lwr_arm_4_joint,lwr_arm_5_joint,lwr_arm_6_joint") }
                             }));
    // Scene using kinematica setup from above
    Initializer scene("Scene",{{"Name",std::string("MyScene")},{"Solver",kinematica}});
    // End-effector task map with two position frames
    Initializer map("exotica/EffPosition",{
                        {"Name",std::string("Position")},
                        {"Scene",std::string("MyScene")},
                        {"EndEffector",std::vector<Initializer>({
                             Initializer("Limb",{{"Segment",std::string("lwr_arm_6_link")}}),
                             Initializer("Limb",{{"Segment",std::string("lwr_arm_6_link")},{"Frame",Eigen::VectorTransform(0,0,0.5)}})
                                        }) } });
    // Create a task using the map above (goal will be specified later)
    Initializer task("exotica/TaskSqrError",{
                         {"Name",std::string("MinimizeError")},
                         {"Map",std::string("Position")},
                         {"Rho",1e2}
                     });
    Eigen::VectorXd W(7);
    W << 7,6,5,4,3,2,1;

    Initializer problem("exotica/IKProblem",{
                            {"Name",std::string("MyProblem")},
                            {"PlanningScene",scene},
                            {"Maps",std::vector<Initializer>({map})},
                            {"Tasks",std::vector<Initializer>({task})},
                            {"W",W},
                            {"Tolerance",1e-5},
                        });

    Initializer solver("exotica/IKsolver",{
                            {"Name",std::string("MySolver")},
                            {"MaxIt",1},
                        });

    HIGHLIGHT_NAMED("GenericLoader","Loaded from a hardcoded generic initializer.");

    // Initialize

    PlanningProblem_ptr any_problem = Setup::createProblem(problem);
    MotionSolver_ptr any_solver = Setup::createSolver(solver);

    // Assign the problem to the solver
    any_solver->specifyProblem(any_problem);

    // Create the initial configuration
    Eigen::VectorXd q = Eigen::VectorXd::Zero(any_problem->scene_->getNumJoints());
    Eigen::MatrixXd solution;


    ROS_INFO_STREAM("Calling solve() in an infinite loop");

    // Publish the states to rviz
    ros::Publisher jointStatePublisher_ = nhg_.advertise<sensor_msgs::JointState>("/joint_states", 1);
    sensor_msgs::JointState jnt;
    jnt.name = any_problem->scene_->getSolver().getJointNames();
    jnt.position.resize(jnt.name.size());
    double t = 0.0;
    ros::Rate loop_rate(500.0);
    ros::WallTime init_time = ros::WallTime::now();

    while (ros::ok())
    {
      ros::WallTime start_time = ros::WallTime::now();

      // Update the goal if necessary
      // e.g. figure eight
      t = ros::Duration((ros::WallTime::now() - init_time).toSec()).toSec();
      Eigen::VectorXd goal(6);
      goal << 0.6,
              -0.1 + sin(t * 2.0 * M_PI * 0.5) * 0.1,
              0.5 + sin(t * M_PI * 0.5) * 0.2,
              1.1,
              -0.1 + sin(t * 2.0 * M_PI * 0.5) * 0.1,
              0.5 + sin(t * M_PI * 0.5) * 0.2;
      any_solver->setGoal("MinimizeError", goal);

      // Solve the problem using the IK solver
      try
      {
        any_solver->Solve(q, solution);
      }
      catch (SolveException e)
      {
        // Ignore failures
      }
      double time = ros::Duration((ros::WallTime::now() - start_time).toSec()).toSec();
      ROS_INFO_STREAM_THROTTLE(0.5,
        "Finished solving in "<<time<<"s. Solution ["<<solution<<"]");
      q = solution.row(solution.rows() - 1);

      jnt.header.stamp = ros::Time::now();
      jnt.header.seq++;
      for (int j = 0; j < solution.cols(); j++)
      jnt.position[j] = q(j);
      jointStatePublisher_.publish(jnt);

      ros::spinOnce();
      loop_rate.sleep();
    }

    // All classes will be destroyed at this point.
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "ExoticaManualInitializationExampleNode");
    ROS_INFO_STREAM("Started");

    // Run demo code
    run();

    // Clean up
    // Run this only after all the exoica classes have been disposed of!
    Setup::Destroy();
}
