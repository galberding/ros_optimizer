#ifndef GA_PATH_GENERATOR_H
#define GA_PATH_GENERATOR_H
#include <random>
#include <cmath>
#include <algorithm>
#include "../tools/path_tools.h"

// #ifdef __DEBUG__
// #undef __DEBUG__
// #define __DEBUG__ true
// #endif

using namespace path;
using namespace logging;

namespace ga{

  struct genome{
    genome(){};
    genome(float fitness):fitness(fitness){};
    genome(PAs actions):actions(actions){};
    bool operator < (const genome& gen) const
    {
        return (fitness < gen.fitness);
    }

    int id = 0;
    PAs actions;
    WPs waypoints;
    float fitness = 0;
  };

  using Genpool = std::deque<genome>;
  using mutaPair = pair<void (*)(genome &gen, std::normal_distribution<float> angleDist, std::normal_distribution<float> distanceDist, std::mt19937 generator), int>;
  using Mutation_conf = map<string, mutaPair>;



  bool compareFitness(const struct genome &genA, const struct genome &genB);
  genome roulettWheelSelection(Genpool &currentPopulation, std::uniform_real_distribution<float> selDistr, std::mt19937 generator);

  int randRange(int lower, int upper);
  void validateGen(genome &gen);
///////////////////////////////////////////////////////////////////////////////
//                             Mutation Functions                            //
///////////////////////////////////////////////////////////////////////////////

  void addAction(genome &gen, std::normal_distribution<float> angleDist, std::normal_distribution<float> distanceDist, std::mt19937 generator);
  void removeAction(genome &gen, std::normal_distribution<float> angleDist, std::normal_distribution<float> distanceDist, std::mt19937 generator);
  void addAngleOffset(genome &gen, std::normal_distribution<float> angleDist, std::normal_distribution<float> distanceDist, std::mt19937 generator);
  void addDistanceOffset(genome &gen, std::normal_distribution<float> angleDist, std::normal_distribution<float> distanceDist, std::mt19937 generator);
  void swapRandomAction(genome &gen, std::normal_distribution<float> angleDist, std::normal_distribution<float> distanceDist, std::mt19937 generator);
  // void swapRandomActionRegion(genome &gen);



  struct executionConfig {
    executionConfig(){} // default constructor
    executionConfig(string dir, string name, shared_ptr<GridMap> gmap, Position start, vector<Position> ends)
      :logDir(dir),
       logName(name),
       gmap(gmap),
       start(start),
       ends(ends){
      fitnessStr = make_shared<std::ostringstream>(std::ostringstream());
      logStr = make_shared<std::ostringstream>(std::ostringstream());
    }
    // Logger
    string logDir = "";
    // string logFitness = "";
    string logName = "";

    // If parameter is set we want to store it under this filename
    string fitnessName = "";
    float fitnessWeight = 0.5;
    // string fitnessStr;
    // std::ostringstream logStr;

    shared_ptr<std::ostringstream> fitnessStr;
    shared_ptr<std::ostringstream> logStr;

        // Robot Config
    string obstacleName = "map";
    rob_config rob_conf = {
      {RobotProperty::Width_cm, 1},
      {RobotProperty::Height_cm, 1},
      {RobotProperty::Drive_speed_cm_s, 50},
      {RobotProperty::Clean_speed_cm_s, 20}};

    // Map
    shared_ptr<GridMap> gmap;

    // GA
    int maxIterations = 1000;
    int currentIter = 0;
    int initIndividuals = 1000;
    int initActions = 50;
    Position start;
    vector<Position> ends;

    // Selection
    int selectIndividuals = 25;
    int selectKeepBest = 10;

    // crossover
    // Length of the segment that will be transferred to the other gen
    float crossLength = 0.4;

    // Mutation
    vector<string> mutaFunctions = {"addAction", "removeAction", "addAngleOffset", "addDistanceOffset"};
    vector<int> probas = {10, 10};
    Mutation_conf muta = {
      // {"addAction", make_pair(addAction, 10)},
      // {"removeAction", make_pair(removeAction, 10)},
      // {"addAngleOffset", make_pair(addAngleOffset, 70)},
      // {"addDistanceOffset", make_pair(addDistanceOffset, 70)},
      // {"swapRandomAction", make_pair(swapRandomAction, 10)},
    };
    float distMu = 4;
    float distDev = 0.9;
    float angleMu = 0;
    float angleDev = 40;

    string config_to_string(){

      string str;
      str += argsToCsv("maxIterations", "initIndividuals", "selectIndividuals", "selectKeepBest", "fitnessWeight");
      str += argsToCsv(maxIterations, initIndividuals, selectIndividuals, selectKeepBest, fitnessWeight);

      return str;
    }

  };



///////////////////////////////////////////////////////////////////////////////
//                                     GA                                    //
///////////////////////////////////////////////////////////////////////////////

  struct GA {
    std::mt19937 generator;
    std::normal_distribution<float> distanceDistr, angleDistr;
    std::uniform_real_distribution<float> selectionDist;
    Mutation_conf muta_conf;
    executionConfig eConf;




    GA(int seed, float distMu, float distDev, float angleMu, float angleDev, Mutation_conf muta_conf):
      generator(seed),
      distanceDistr{distMu, distDev},
      angleDistr{angleMu, angleDev},
      selectionDist{0,1},
      muta_conf(muta_conf){};

    GA(int seed, executionConfig& conf)
      :generator(seed),
       distanceDistr{conf.distMu, conf.distDev},
       angleDistr{conf.angleMu, conf.angleDev},
       selectionDist{0,1},
       muta_conf(conf.muta),
       eConf(conf){}

    virtual void populatePool(Genpool &currentPopuation, Position start, WPs endpoints, int individuals, int initialActions);
    virtual void selection(Genpool &currentPopuation, Genpool &selection, int individuals, int keepBest = 0);
    virtual void crossover(Genpool &currentSelection, Genpool &newPopulation);
    virtual void mating(genome &par1, genome &par2, Genpool& newPopulation);
    virtual void mutation(Genpool& currentPopulation, Mutation_conf& muat_conf);
    virtual void evalFitness(Genpool &currentPopulation, Robot &rob);
    virtual float calFitness(float cdist,
			      float dist,
			      int crossed,
			      float cSpeed_m_s,
			      float speed_m_s,
			      int freeSpace);
    void optimizePath();
    void gridSearch();




  };

  /////////////////////////////////////////////////////////////////////////////
  //                           Dual Point Crossover                          //
  /////////////////////////////////////////////////////////////////////////////

  struct _Dual_Point_Crossover : GA{
    // Use constructor of GA
    using GA::GA;
    virtual void mating(genome &par1, genome &par2, Genpool& newPopulation) override;

  };


  /////////////////////////////////////////////////////////////////////////////
  //                                  GA V2                                   //
  /////////////////////////////////////////////////////////////////////////////

  struct GA_V2 : _Dual_Point_Crossover{
    using _Dual_Point_Crossover::_Dual_Point_Crossover;
  };

}
#endif /* GA_PATH_GENERATOR_H */
