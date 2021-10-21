/*
 * This file is part of JKQ QFR library which is released under the MIT license.
 * See file README.md or go to http://iic.jku.at/eda/research/quantum/ for more information.
 */

#include "algorithms/QPE.hpp"

namespace qc {
    QPE::QPE(dd::QubitCount nq, bool exact):
        precision(nq) {
        if (exact) {
            // if an exact solution is wanted, generate a random n-bit number and convert it to an appropriate phase
            std::uint_least64_t max          = 1ULL << nq;
            auto                distribution = std::uniform_int_distribution<std::uint_least64_t>(0, max - 1);
            std::uint_least64_t theta        = 0;
            while (theta == 0) {
                theta = distribution(mt);
            }
            lambda = 0.;
            for (std::size_t i = 0; i < nq; ++i) {
                if (theta & (1 << (nq - i - 1))) {
                    lambda += 1. / (1 << i);
                }
            }
            createCircuit();
        } else {
            // if an inexact solution is wanted, generate a random n+1-bit number (that has its last bit set) and convert it to an appropriate phase
            std::uint_least64_t max          = 1ULL << (nq + 1);
            auto                distribution = std::uniform_int_distribution<std::uint_least64_t>(0, max - 1);
            std::uint_least64_t theta        = 0;
            while (theta == 0 && (theta & 1) == 0) {
                theta = distribution(mt);
            }
            lambda = 0.;
            for (std::size_t i = 0; i <= nq; ++i) {
                if (theta & (1 << (nq - i))) {
                    lambda += 1. / (1 << i);
                }
            }
            createCircuit();
        }
    }

    QPE::QPE(dd::fp lambda, dd::QubitCount precision):
        lambda(lambda), precision(precision) {
        createCircuit();
    }

    std::ostream& QPE::printStatistics(std::ostream& os) const {
        os << "QPE Statistics:\n";
        os << "\tn: " << static_cast<std::size_t>(nqubits + 1) << std::endl;
        os << "\tm: " << getNindividualOps() << std::endl;
        os << "\tlambda: " << lambda << "π" << std::endl;
        os << "\tprecision: " << static_cast<std::size_t>(precision) << std::endl;
        os << "--------------" << std::endl;
        return os;
    }

    void QPE::createCircuit() {
        addQubitRegister(1, "psi");
        addQubitRegister(precision, "q");
        addClassicalRegister(precision, "c");

        //Hadamard Layer
        for (dd::QubitCount i = 1; i <= precision; i++) {
            h(static_cast<dd::Qubit>(i));
        }
        //prepare eigenvalue
        x(0);

        //Controlled Phase Rotation
        for (dd::QubitCount i = 0; i < precision; i++) {
            // normalize angle
            const auto angle = std::remainder((1U << i) * lambda, 2.0);
            phase(0, dd::Control{static_cast<dd::Qubit>(precision - i)}, angle * dd::PI);
        }

        //Inverse QFT
        for (dd::QubitCount i = 1; i <= precision; ++i) {
            for (dd::QubitCount j = 1; j < i; j++) {
                auto iQFT_lambda = -dd::PI / (1U << (i - j));
                if (j == i - 1) {
                    sdag(static_cast<dd::Qubit>(i), dd::Control{static_cast<dd::Qubit>(i - 1)});
                } else if (j == i - 2) {
                    tdag(static_cast<dd::Qubit>(i), dd::Control{static_cast<dd::Qubit>(i - 2)});
                } else {
                    phase(static_cast<dd::Qubit>(i), dd::Control{static_cast<dd::Qubit>(j)}, iQFT_lambda);
                }
            }
            h(static_cast<dd::Qubit>(i));
        }

        //Measure Results
        for (dd::QubitCount i = 0; i < nqubits - 1; i++) {
            measure(static_cast<dd::Qubit>(i + 1), i);
        }
    }
} // namespace qc