// logger by Ken H

#include "mylogger.hpp"
#include <pagmo/io.hpp>

logger::logger() {
    m_is_genome = false;
    // log file
}

logger::logger(bool is_genome, NEAT::Genome* genome) : m_is_genome{is_genome}, m_startgen(genome) {}

bool logger::is_genome() { return m_is_genome; }

void logger::save_hist_score(int generation, pagmo::population* pop) {

    size_t ind_best = pop->best_idx();

    std::ifstream f("hist_score.txt");
    bool new_file = not f.good();
    f.close();

    std::ofstream os("hist_score.txt", std::ios_base::app);
    if (new_file) {
        os << "generation,fitness" << std::endl;
    }

    os << generation << "," << -pop->get_f()[ind_best][0] << std::endl;
    os.close();
}

void logger::save_to_file(int generation, double* mean, pagmo::population* pop) {

    std::string filename = "gen/champ_" + std::to_string(generation) + ".dat";
    std::ofstream os(filename);

    if (m_is_genome) {
        size_t ind_best = pop->best_idx();
        const double* best_x = pop->get_x()[ind_best].data();
        NEAT::Network* best_net = m_startgen->genesis(m_startgen->genome_id);
        best_net->set_weights((double*)best_x, pop->get_x()[ind_best].size());
        NEAT::Genome* best_gen = new NEAT::Genome(best_net);

        pagmo::stream(os, "/*Fitness: ", -pop->get_f()[ind_best][0], "*/\n");

        best_gen->print_to_file(os);

        std::string mean_filename = "gen/mean_" + std::to_string(generation) + ".dat";
        std::ofstream mean_os(mean_filename);

        NEAT::Network* mean_net = m_startgen->genesis(m_startgen->genome_id);
        mean_net->set_weights(mean, pop->get_x()[ind_best].size());
        NEAT::Genome* mean_gen = new NEAT::Genome(mean_net);

        pagmo::stream(mean_os, "/*Mean value, no fitness value associated.*/\n");
        mean_gen->print_to_file(mean_os);

        mean_os.close();
    } else {
        os << this;
    }
    os.close();
}

void logger::set_genome(NEAT::Genome* genome) {
    m_startgen = genome;
    m_is_genome = true;
}
