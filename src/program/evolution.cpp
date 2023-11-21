// MPI
#include <mpi.h>

// Standard C++ Library
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

// NEAT
#include "../NEAT/genome.h"
#include "../NEAT/neat.h"

// ARGOS
#include <argos3/core/simulator/argos_command_line_arg_parser.h>
#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/visualization/visualization.h>
#include <argos3/core/utility/plugins/dynamic_loading.h>
#include <argos3/plugins/robots/rvr/simulator/rvr_entity.h>

// Controller
#include "../NEATController.h"

// Loop function
#include <argos3/demiurge/loop-functions/RVRCoreLoopFunctions.h>

// pagmo
#include "../pagmo/mylogger.hpp"
#include "../pagmo/problems/mpi_argos.hpp"

#include "../pagmo/algorithms/cmaes.hpp"
#include "../pagmo/algorithms/xnes.hpp"
/* #include <pagmo/bfe.hpp> */
#include <pagmo/batch_evaluators/member_bfe.hpp>
#include <pagmo/island.hpp>

/* using namespace pagmo; */
/**
 * Print resulting gene from candidate
 */
using namespace pagmo;

void print_cadidate_gen(vector_double x, double fitness, NEAT::Genome* gen, const char* filename) {
    const double* best_x = x.data();
    NEAT::Network* best_net = gen->genesis(gen->genome_id);
    best_net->set_weights((double*)best_x, x.size());

    NEAT::Genome* best_gen = new NEAT::Genome(best_net);

    std::ofstream oFile(filename);
    oFile << "/*Fitness: " << -fitness << "*/" << std::endl;
    oFile.close();
    best_gen->print_to_filename(filename);
}

void load_params(const char* filename) {
    std::ifstream paramFile(filename);

    char curword[128];

    paramFile >> curword;
    paramFile >> NEAT::pop_size;

    paramFile >> curword;
    paramFile >> NEAT::budget;

    paramFile >> curword;
    paramFile >> NEAT::num_runs_per_gen;

    paramFile >> curword;
    paramFile >> NEAT::weight_lower_bound;

    paramFile >> curword;
    paramFile >> NEAT::weight_upper_bound;

    paramFile >> curword;
    paramFile >> NEAT::step_size;

    paramFile >> curword;
    paramFile >> NEAT::use_cmaes;

    std::cout << "pop_size: " << NEAT::pop_size << std::endl;
    std::cout << "budget: " << NEAT::budget << std::endl;
    std::cout << "num_runs_per_gen: " << NEAT::num_runs_per_gen << std::endl;
    std::cout << "weight_lower_bound: " << NEAT::weight_lower_bound << std::endl;
    std::cout << "weight_upper_bound: " << NEAT::weight_upper_bound << std::endl;
    std::cout << "step_size: " << NEAT::step_size << std::endl;

    paramFile.close();
}

const std::string ExplainParameters() {
    std::string strExplanation = "The possible parameters are: \n\n"
                                 " -g \t start genome file for the robots \n"
                                 " -m \t nb cores to start MPI with\n"
                                 " -b \t path of the scheduler binary\n"
                                 " -p \t the parameter file for optim algorithm\n";
    return strExplanation;
}

/**
 * Main program.
 */
int main(int argc, char* argv[]) {

    std::string unGenome;
    std::string unBinaries;
    std::string unParameters;
    UInt32 num_para_proc;
    CARGoSCommandLineArgParser cACLAP;
    cACLAP.AddArgument<std::string>('g', "genome", "genome file for your robots", unGenome);
    cACLAP.AddArgument<UInt32>('m', "nbcores", "number of parallel processes", num_para_proc);
    cACLAP.AddArgument<std::string>('b', "binaries",
                                    "binaries to parallelise for objective function", unBinaries);
    cACLAP.AddArgument<std::string>('p', "parameter file",
                                    "Files containing parameters for XNES/CMA-ES algorithm",
                                    unParameters);

    std::string configFile = cACLAP.GetExperimentConfigFile();

    cACLAP.Parse(argc, argv);
    load_params(unParameters.c_str());
    num_para_proc--;
    cACLAP.PrintUsage(argos::LOG);

    try {
        std::cout << "PARALLEL RUN" << std::endl;

        // Initializes the MPI execution environment.
        MPI::Init();

        long unsigned int lambda = NEAT::pop_size;

        if (lambda < num_para_proc) {
            num_para_proc = lambda;
        }

        // Spawns a number of identical binaries.
        MPI::Intercomm g_com = MPI::COMM_WORLD.Spawn(unBinaries.c_str(), (const char**)argv,
                                                     num_para_proc, MPI::Info(), 0);

        // Get number of links
        NEAT::Genome* startgen = NEAT::Genome::new_Genome_load(unGenome.c_str());
        std::vector<NEAT::Link*> links = startgen->genesis(startgen->genome_id)->getlinks();

        /* std::vector<double> x0; // why ? */
        /* int dim1 = 0; */
        /* for (std::vector<NEAT::Link*>::iterator it = links.begin(); it != links.end(); ++it) { */
        /*     x0.push_back((*it)->weight); */
        /*     dim1++; */
        /* } */
        auto dim = links.size();
        /* std::cout << "dim1: " << dim1 << " dim: " << dim << std::endl; */

        int budget = NEAT::budget;
        int num_runs_per_gen = NEAT::num_runs_per_gen;

        int generations = budget / num_runs_per_gen / lambda;

        generations -= 1;

        double lbound = NEAT::weight_lower_bound;
        double ubound = NEAT::weight_upper_bound;

        double step_size = NEAT::step_size;

        problem prob{mpi_thread(num_para_proc, dim, num_runs_per_gen, lbound, ubound, g_com)};
        member_bfe memb;
        algorithm* algo;
        logger* mylogger = new logger(true, startgen);

        bool force_bound = true;
        if (step_size <= 0.0) {
            force_bound = false;
            step_size = -step_size;
        }

        if (NEAT::use_cmaes) {
            std::cout << "CMA-ES algorithm started" << std::endl;
            algorithm* cm = new algorithm{cmaes(generations, -1, -1, -1, -1, step_size, 1e-6, 1e-6,
                                                false, force_bound, mylogger)};
            algo = cm;
        } else {
            std::cout << "XNES algorithm started" << std::endl;
            algorithm* xn = new algorithm{
                xnes(generations, -1, -1, -1, step_size, 1e-6, 1e-6, false, force_bound, mylogger)};
            algo = xn;
        }
        algo->set_verbosity(1);

        island isl{*algo, prob, memb, lambda};

        isl.evolve();
        isl.wait();

        std::cout << "Final score: " << -isl.get_population().champion_f()[0] << '\n';

        population pop = isl.get_population();

        const char* file_best = "best_seen.txt";
        print_cadidate_gen(pop.champion_x(), pop.champion_f()[0], startgen, file_best);

        // Sends a signal to terminate the children.
        std::cout << "Parent: Terminate children" << std::endl;
        bool cont = false;
        for (long unsigned int j = 0; (j < num_para_proc); j++) {
            g_com.Send(&cont, 1, MPI::BOOL, j, 1);
        }

        // Terminates MPI execution environment.
        MPI_Finalize();
        delete mylogger;
        delete algo;
        return 0;

    } catch (std::exception& ex) {
        THROW_ARGOSEXCEPTION(ExplainParameters())
    }
}
/* } // namespace pagmo */
