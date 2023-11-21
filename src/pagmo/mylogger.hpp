
#ifndef PAGMO_MYLOGGER_HPP
#define PAGMO_MYLOGGER_HPP

#include "../NEAT/genome.h"
#include <pagmo/population.hpp>
#include <string>

class logger {
  public:
    logger();
    logger(bool is_genome, NEAT::Genome* genome);
    bool is_genome();
    void save_hist_score(int generation, pagmo::population* pop);
    void save_to_file(int generation, double* mean, pagmo::population* pop);
    void set_genome(NEAT::Genome* genome);

  private:
    // The individuals are of format genome
    bool m_is_genome;
    // Pointer to base genome
    NEAT::Genome* m_startgen;
};

#endif
