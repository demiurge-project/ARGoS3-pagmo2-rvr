/*
 * mpi.hpp
 *
 *  Created on: 7 May 2019
 *  Author: Julian Ruddick
 */

#ifndef PAGMO_MYPROBLEM
#define PAGMO_MYPROBLEM

#include <mpi.h>
#include <pagmo/problem.hpp>

namespace pagmo {

struct mpi_thread {
    /// Constructor from dimension
    /**
     * @param dim problem dimension.
     *
     * @throw std::invalid_argument if \p dim is less than 2.
     */

    mpi_thread(int proc = 0, int dim = 0, int runs = 0, double lbound = 0, double ubound = 0,
               MPI::Intercomm g_com = 0)
        : m_proc(proc), m_dim(dim), m_runs(runs), m_lbound(lbound), m_ubound(ubound),
          m_g_com(g_com) {
        m_thread_safety = thread_safety::none;
        m_has_batch_fitness = true;
    };
    /// Fitness computation
    /**
     * Computes the fitness for this UDP.
     *
     * @param x the decision vector.
     *
     * @return the fitness of \p x.
     */
    vector_double fitness(const vector_double& x) const {
        std::cout << "Entered single fitness" << std::endl;
        MPI::Status status;
        double retval;
        bool cont = true;

        int proc_used = 0;
        // Send continue value to binary
        m_g_com.Send(&cont, 1, MPI::BOOL, proc_used, 1);
        m_g_com.Send(&m_runs, 1, MPI::INT, proc_used, 1);
        m_g_com.Send(x.data(), m_dim, MPI::DOUBLE, proc_used, 1);
        m_g_com.Recv(&retval, 1, MPI::DOUBLE, proc_used, MPI::ANY_TAG, status);

        return {retval};
    }

    bool has_batch_fitness() const { return m_has_batch_fitness; }

    vector_double batch_fitness(const vector_double& dvs) const {
        std::cout << "Entered batch fitness" << std::endl;
        int n_dvs = dvs.size() / m_dim;
        vector_double retval(n_dvs);

        int nGroup = ceil((double)n_dvs / m_proc);
        int ind;
        bool cont = true;
        double dFitness;
        MPI::Status status;

        for (int r = 0; r < nGroup; r++) {
            std::cout << "Group: " << r << std::endl;
            for (int p = 0; p < m_proc && p + r * m_proc < n_dvs; p++) {
                std::cout << "Process: " << p << std::endl;
                ind = p + r * m_proc;
                const double* data_ptr = dvs.data() + ind * m_dim;

                m_g_com.Send(&cont, 1, MPI::BOOL, p, 1);
                m_g_com.Send(&m_runs, 1, MPI::INT, p, 1);
                m_g_com.Send(data_ptr, m_dim, MPI::DOUBLE, p, 1);
            }

            for (int p = 0; p < m_proc && p + r * m_proc < n_dvs; p++) {
                m_g_com.Recv(&dFitness, 1, MPI::DOUBLE, MPI::ANY_SOURCE, MPI::ANY_TAG, status);
                ind = r * m_proc + status.Get_source();

                retval[ind] = dFitness;
            }
        }
        return retval;
    }

    /// Box-bounds
    /**
     * @return the lower and upper bounds for each decision vector component.
     */
    std::pair<vector_double, vector_double> get_bounds() const {
        return {vector_double(m_dim, m_lbound), vector_double(m_dim, m_ubound)};
    }
    /// Problem name
    /**
     * @return a string containing the problem name.
     */
    std::string get_name() const { return "MPI ARGOS problem"; }

    /// Problem's thread safety level.
    /**
     * If the UDP satisfies pagmo::has_get_thread_safety, then this method will return the output of
     * its <tt>%get_thread_safety()</tt> method. Otherwise, thread_safety::basic will be returned.
     * That is, pagmo assumes by default that is it safe to operate concurrently on distinct UDP
     * instances.
     *
     * @return the thread safety level of the UDP.
     */
    thread_safety get_thread_safety() const { return m_thread_safety; }

    /// Object serialization
    /**
     * This method will save/load \p this into the archive \p ar.
     *
     * @param ar target archive.
     *
     * @throws unspecified any exception thrown by the serialization of the UDP and of primitive
     * types.
     */
    template <typename Archive> void serialize(Archive& ar, unsigned) {
        ar &m_dim; 
    }

    /// Problem dimensions
    int m_proc;
    int m_dim;
    int m_runs;
    double m_lbound;
    double m_ubound;
    MPI::Intercomm m_g_com;
    thread_safety m_thread_safety;
    bool m_has_batch_fitness;
};

} // namespace pagmo

/* PAGMO_REGISTER_PROBLEM(pagmo::mpi_thread) */
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::mpi_thread)

#endif
