# Copyright 2026
#
# Licensed under the Apache License, Version 2.0 with LLVM Exceptions (the
# "License"); you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://github.com/Munich-Quantum-Software-Stack/QDMI-Devices/blob/develop/LICENSE
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# file trapped_ions_scheduler.py
# @brief Utility function to generate a trapped-ions schedule from a circuit
# ------------------------------------------------------------------------------

from qat.core import Circuit


def _qubit_placement_to_str(qubit_placement):
    " Stringify qubit placement "
    return "{" + ", ".join(f"{val}: q{idx}" for idx, val in enumerate(qubit_placement)) + "}"


def generate_schedule_from_circuit(circuit: Circuit):
    """
    Generates a execution schedule for trapped-ions devices given an input circuit.

    Args:
        circuit (Circuit): the circuit to generate a schedule

    Returns:
        str: the schedule of the circuit for trapped-ions devices
    """
    nbqbits = circuit.nbqbits
    start_index = -1 * nbqbits
    qubit_placement = list(range(start_index, start_index + nbqbits))
    schedule_lines = [_qubit_placement_to_str(qubit_placement)]
    for gate, params, qbits in circuit.iterate_simple():
        if len(qbits) == 1:
            if qubit_placement[qbits[0]] != 0:
                # Shuttling is needed since the qubit is not already at the LIZ
                schedule_lines.append("SHUTTLE")
                start_index = -1 * qbits[0]
                qubit_placement = list(range(start_index, start_index + nbqbits))
                schedule_lines.append(_qubit_placement_to_str(qubit_placement))
            schedule_lines.append(f"APPLY {gate} " + ",".join(str(angle) for angle in params))
        else:
            # Two qubits gate
            first_qubit, second_qubit = sorted(qbits)
            if not (qubit_placement[first_qubit] == -1 and qubit_placement[second_qubit] == 1):
                schedule_lines.append("SHUTTLE")
                qubit_placement = [(i - first_qubit - 1) if i <= first_qubit else (i - second_qubit + 1) for i in range(nbqbits)]
                schedule_lines.append(_qubit_placement_to_str(qubit_placement))
            schedule_lines.append("MERGE")
            schedule_lines.append(f"APPLY {gate} " + ",".join(str(angle) for angle in params))
            schedule_lines.append("SPLIT")
    schedule_output = "\n".join(schedule_lines)
    return schedule_output
