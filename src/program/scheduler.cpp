// MPI
#include <mpi.h>

// Standard C++ Library
#include <iostream>
#include <string>
#include <vector>

// NEAT
#include "../NEAT/genome.h"

// ARGOS
#include <argos3/core/simulator/argos_command_line_arg_parser.h>
#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/utility/plugins/dynamic_loading.h>
#include <argos3/plugins/robots/rvr/simulator/rvr_entity.h>

// Controller
#include "../NEATController.h"

// Loop function
#include <argos3/demiurge/loop-functions/RVRCoreLoopFunctions.h>

// isnan function
#include <math.h>

/**
 * Child process: Initializes the MPI execution environment, ARGoS, and waits for the parent to give
 * the random seed and the genome (string). Once it has received all the necessary information, it
 * creates the genome/network, and launches the experiment. At the end of the experiment, it sends
 * back the fitness to the parent.
 */

int main(int argc, char* argv[]) {
    // Initializes the MPI execution environment.
    MPI::Init();

    // Gets the parent intercommunicator of current spawned process.
    MPI::Intercomm parent_comm = MPI::Comm::Get_parent();

    // Gets the rank of the process.
    int id = MPI::COMM_WORLD.Get_rank();

    std::cout << "Hello, I'm the process " << id << "." << std::endl;

    /* Configure the command line options */
    std::string unGenome;
    std::string unBinaries;
    std::string unParameters;
    UInt32 num_para_proc;
    CARGoSCommandLineArgParser cACLAP;
    cACLAP.AddArgument<std::string>('g', "genome", "genome file for your robots", unGenome);
    cACLAP.AddArgument<UInt32>('m', "nbcores", "number of parallel processes", num_para_proc);
    cACLAP.AddArgument<std::string>('b', "binaries",
                                    "binaries to parallelise for objective function", unBinaries);
    cACLAP.AddArgument<std::string>(
        'p', "parameter file", "Files containing parameters for CMA-ES algorithm", unParameters);

    cACLAP.Parse(argc - 1, argv + 1);

    // Initialization of ARGoS
    auto& cSimulator = argos::CSimulator::GetInstance();
    argos::CDynamicLoading::LoadAllLibraries();
    cSimulator.SetExperimentFileName(cACLAP.GetExperimentConfigFile());
    cSimulator.LoadExperiment();
    static auto& cLoopFunctions = dynamic_cast<RVRCoreLoopFunctions&>(cSimulator.GetLoopFunctions());

    // Initialization random seed
    unsigned int g_unRandomSeed = 1;
    time_t t;
    srand((unsigned)time(&t));
    g_unRandomSeed = rand();
    argos::CRandom::CreateCategory("cmaes", g_unRandomSeed);
    std::vector<UInt32> vecRandomSeed;
    argos::CRandom::CRNG* pRNG = argos::CRandom::CreateRNG("cmaes");

    // Load the network
    auto cEntities = cSimulator.GetSpace().GetEntitiesByType("controller");
    for (auto ent : cEntities) {
        auto* pcEntity = any_cast<CControllableEntity*>(ent.second);
        try {
            auto& cController = dynamic_cast<CRVRNEATController&>(pcEntity->GetController());
            cController.LoadNetwork(unGenome);

        } catch (std::exception& ex) {
            LOGERR << "Error while setting network: " << ex.what() << std::endl;
        }
    }

    // Waiting for the parent to give us some work to do.
    while (true) {

        // If we received an end signal, we get out of the while loop.
        bool cont;
        parent_comm.Recv(&cont, 1, MPI::BOOL, 0, 1);

        if (!cont) {
            std::cout << "ID" << id << ": END" << std::endl;
            break;
        }

        MPI::Status status;
        parent_comm.Probe(0, 1, status);
        int nNum_runs_per_gen = 0;
        parent_comm.Recv(&nNum_runs_per_gen, 1, MPI::INT, 0, 1);
        vecRandomSeed.resize(nNum_runs_per_gen);

        // Receiving the weights of the
        parent_comm.Probe(0, 1, status);

        int nNum_weights = status.Get_count(MPI::DOUBLE);
        double* weights = new double[nNum_weights];

        parent_comm.Recv(weights, nNum_weights, MPI::DOUBLE, 0, 1);

        for (int i = 0; i < nNum_runs_per_gen; i++) {
            vecRandomSeed[i] = pRNG->Uniform(argos::CRange<UInt32>(0, UINT32_MAX));
        }

        // Create the new genome, and the network
        // NEAT::Genome* genome = new NEAT::Genome(id, vecTraits, vecNodes, vecGenes);
        // NEAT::Network* net = genome->genesis(genome->genome_id);

        // Launch the experiment with the correct random seed and network,
        // and evaluate the average fitness

        // Change the weights in each controller
        for (auto ent : cEntities) {
            auto* pcEntity = any_cast<CControllableEntity*>(ent.second);
            try {
                auto& cController = dynamic_cast<CRVRNEATController&>(pcEntity->GetController());
                // Change the weights of the controller's network
                cController.set_weights_network(weights, nNum_weights);
                // cController.Display(1);
            } catch (std::exception& ex) {
                LOGERR << "Error while setting network: " << ex.what() << std::endl;
            }
        }

        double dFitness = 0.0;
        for (int j = 0; j < nNum_runs_per_gen; j++) {
            try {

                cSimulator.Reset();
                cSimulator.Execute();
                dFitness -= cLoopFunctions.GetObjectiveFunction();
            } catch (...) {
                dFitness = 0.0;
                // cSimulator.Reset();
                std::cout << "ERROR: ARGoS" << std::endl;
                // std::cout << "j=" << j << std::endl;
                std::cout << "length=" << vecRandomSeed.size() << std::endl;
                std::cout << "seed=" << vecRandomSeed[j] << std::endl;
            }
        }

        // Compute the average fitness
        if (nNum_runs_per_gen > 0) {
            dFitness /= nNum_runs_per_gen;
        }

        // Send the fitness to the parent.
        // std::cout << "ID" << id << " --> Average fitness = " << dFitness << std::endl;
        parent_comm.Send(&dFitness, 1, MPI::DOUBLE, 0, 1);

        // Removing stuffs
        delete[] weights;
        // delete genome;
        // delete net;
    }

    // Remove category
    if (CRandom::ExistsCategory("cmaes")) {
        CRandom::RemoveCategory("cmaes");
    }

    // Dispose of ARGoS stuff
    cSimulator.Destroy();

    // Terminates MPI execution environment.
    MPI_Finalize();

    return 0;
}

