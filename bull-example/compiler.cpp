#include <string>
#include <array>
#include <iostream>

#include "qaptiva_compiler_qdmi/device.h"


int main() {
    std::string input_bell = R"(
        OPENQASM 2.0;

        qreg q[2];

        h q[0];
        cx q[0],q[1];
    )";
    std::string target_gate_set = "IONS";

    // Initialize device
    QAPTIVA_COMPILER_QDMI_device_initialize();

    // Initialize session
    QAPTIVA_COMPILER_QDMI_Device_Session session;
    QAPTIVA_COMPILER_QDMI_device_session_alloc(&session);
    QAPTIVA_COMPILER_QDMI_device_session_init(session);

    // Initialize job
    QAPTIVA_COMPILER_QDMI_Device_Job job;
    QAPTIVA_COMPILER_QDMI_device_session_create_device_job(session, &job);
    QAPTIVA_COMPILER_QDMI_device_job_set_parameter(job, QDMI_DEVICE_JOB_PARAMETER_PROGRAM, input_bell.size(), input_bell.c_str());
    QAPTIVA_COMPILER_QDMI_device_job_set_parameter(job, QDMI_DEVICE_JOB_PARAMETER_CUSTOM1, target_gate_set.size(), target_gate_set.c_str());

    QAPTIVA_COMPILER_QDMI_device_job_submit(job);
    QAPTIVA_COMPILER_QDMI_device_job_wait(job, 0UL);

    std::array<char, 1024UL> buffer;
    size_t output_size;
    QAPTIVA_COMPILER_QDMI_device_job_get_results(job, QDMI_JOB_RESULT_CUSTOM1, buffer.size(), buffer.data(), &output_size);
    std::string output { buffer.data(), output_size };

    // Print everything
    std::cout << "Compiling a Bell pair circuit using 'NISQCompiler(target_gate_set=\"" << target_gate_set << "\")'\n"
              << "\n"
              << "The generated OpenQASM 2 circuit is:\n"
              << "\"\"\"\n"
              << output << "\n"
              << "\"\"\"" << std::endl;

    // Clean-up
    QAPTIVA_COMPILER_QDMI_device_job_free(job);
    QAPTIVA_COMPILER_QDMI_device_session_free(session);
    QAPTIVA_COMPILER_QDMI_device_finalize();
}
