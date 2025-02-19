/*
* This file is part of MQT QFR library which is released under the MIT license.
* See file README.md or go to https://www.cda.cit.tum.de/research/quantum/ for more information.
*/

#pragma once

#include "Ecc.hpp"
#include "QuantumComputation.hpp"
namespace ecc {
    class Q7Steane: public Ecc {
    public:
        Q7Steane(std::shared_ptr<qc::QuantumComputation> qc, std::size_t measureFq):
            Ecc(
                    {ID::Q7Steane, N_REDUNDANT_QUBITS, N_CORRECTING_BITS, "Q7Steane", {{N_CORRECTING_BITS, "qecc"}}}, std::move(qc), measureFq) {}

    protected:
        void writeEncoding() override;

        void measureAndCorrect() override;
        void measureAndCorrectSingle(bool xSyndrome);

        void writeDecoding() override;

        void mapGate(const qc::Operation& gate) override;

        static constexpr std::size_t N_REDUNDANT_QUBITS = 7;
        static constexpr std::size_t N_CORRECTING_BITS  = 3;

        static constexpr std::array<Qubit, 4> DECODING_CORRECTION_VALUES = {1, 2, 4, 7}; //values with odd amount of '1' bits
    };
} // namespace ecc
