#include "qaptiva_compiler_qdmi/device.h"

#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

using namespace pybind11::literals;


namespace {
    /// Pointer to the global python interpreter
    std::unique_ptr<pybind11::scoped_interpreter> global_python_interpreter = nullptr;
}


/**
 * @brief A handle for a device session
 * @details A session is a mandatory structure introduced by QDMI. For Qaptiva Access, the session
 * is configured globaly, and not managed by the user. This structure is then empty.
 *
 * To create a first configuration, the user must have executed the following python code:
 *
 * ~~~~~{.py}
 * from qat.qlmaas import QLMaaSConnection
 *
 * server_hostname = "..."
 * conn = QLMaaSConnection(server_hostname)
 * conn.create_config()
 * ~~~~~
 */
struct QAPTIVA_COMPILER_QDMI_Device_Session_impl_d { /* Nothing in it */ };


/**
 * @brief A handle for a Qaptiva Access job
 * @details Object containing all the information to create a Qaptiva Access job for compilation.
 * The underlying compiler will always be NISQCompiler (with a predefined gate set - defining the
 * topology is not supported yet). More information on this compiler can be found on the
 * [myQLM documentation](https://myqlm.github.io/04_api_reference/module_qat/module_plugins/nisqcompiler.html).
 */
struct QAPTIVA_COMPILER_QDMI_Device_Job_impl_d {
    /// Pointer to the session
    QAPTIVA_COMPILER_QDMI_Device_Session session = nullptr;

    /// The job ID
    std::string job_id = "";

    /// Target gate set - CUSTOM1
    std::string target_gate_set = "";

    /// Enumeration containing the program format - this compiler only support OpenQASM 2
    QDMI_Program_Format program_format = QDMI_Program_Format::QDMI_PROGRAM_FORMAT_QASM2;

    /// Program content, encoded using OpenQASM 2
    std::string program_content = "";
};


/**
 * @brief Initializes a Qaptiva Access device
 * @details This function creates a new Python global interpreter and will try to create a
 * connection to Qaptiva Access.
 * If the connection cannot be created, an error will be returned.
 */
int QAPTIVA_COMPILER_QDMI_device_initialize() {
    // Create a python interpreter if needed
    if (global_python_interpreter == nullptr) {
        global_python_interpreter = std::make_unique<pybind11::scoped_interpreter>();
    }

    pybind11::object qaptiva_access_connection;

    // Try to create a connection
    // If an exception is raised here, the Qaptiva Access connection is not properly configured so "QDMI_ERROR_FATAL"
    // will be returned
    try {
        qaptiva_access_connection = pybind11::module_::import("qat.qlmaas").attr("QLMaaSConnection")();
    } catch(pybind11::error_already_set &) {
        global_python_interpreter = nullptr;
        return QDMI_ERROR_FATAL;
    }

    // Checks if the NISQCompiler is available on the server side
    // If the compiler is not available on the server side, "QDMI_ERROR_NOTFOUND" will be returned
    try {
        qaptiva_access_connection.attr("get_plugin")("qat.plugins:NISQCompiler");
    } catch(pybind11::error_already_set &) {
        global_python_interpreter = nullptr;
        return QDMI_ERROR_NOTFOUND;
    }

    return QDMI_SUCCESS;
}


/**
 * @brief Finalizes the Qaptiva Access device
 * @details This function will destroy the global python interpreter
 */
int QAPTIVA_COMPILER_QDMI_device_finalize() {
    global_python_interpreter = nullptr;
    return QDMI_SUCCESS;
}


/*
 * C++ functions to manage a session.
 * A session is:
 *  - allocated
 *  - initialized (nothing to do - done by allocation)
 *  - configured (nothing to do)
 *  - freed
 */
int QAPTIVA_COMPILER_QDMI_device_session_alloc(QAPTIVA_COMPILER_QDMI_Device_Session * session) {
    if (session == nullptr)
        return QDMI_ERROR_INVALIDARGUMENT;

    *session = new QAPTIVA_COMPILER_QDMI_Device_Session_impl_d;
    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_session_set_parameter(
    QAPTIVA_COMPILER_QDMI_Device_Session /* session */,
    QDMI_Device_Session_Parameter /* param */,
    size_t /* size */,
    const void * /* value */
) {
    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_session_init(QAPTIVA_COMPILER_QDMI_Device_Session /* session */) {
    return QDMI_SUCCESS;
}


void QAPTIVA_COMPILER_QDMI_device_session_free(QAPTIVA_COMPILER_QDMI_Device_Session session) {
    delete session;
}


// Useless device related functions
int QAPTIVA_COMPILER_QDMI_device_session_query_device_property(
    QAPTIVA_COMPILER_QDMI_Device_Session /* session */,
    QDMI_Device_Property /* property */,
    size_t /* size */,
    void * /* value */,
    size_t * /* size_ret */
) {
    // No property to query
    return QDMI_ERROR_BADSTATE;
}


int QAPTIVA_COMPILER_QDMI_device_session_query_site_property(
    QAPTIVA_COMPILER_QDMI_Device_Session /* session */,
    QAPTIVA_COMPILER_QDMI_Site /* site */,
    QDMI_Site_Property /* prop */,
    size_t /* size */,
    void * /* value */,
    size_t * /* size_ret */
) {
    // No property to query
    return QDMI_ERROR_NOTSUPPORTED;
}


int QAPTIVA_COMPILER_QDMI_device_session_query_operation_property(
    QAPTIVA_COMPILER_QDMI_Device_Session /* session */,
    QAPTIVA_COMPILER_QDMI_Operation /* operation */,
    size_t /* num_sites */,
    const QAPTIVA_COMPILER_QDMI_Site * /* sites */,
    size_t /* num_params */,
    const double * /* params */,
    QDMI_Operation_Property /* prop */,
    size_t /* size */,
    void * /* value */,
    size_t * /* size_ret */
) {
    // No property to query
    return QDMI_ERROR_NOTSUPPORTED;
}


/*
 * C++ functions to manage a QDMI job. A QDMI can be:
 *  - created from a session
 *  - configured (by adding parameters)
 *  - submitted
 *  - joined / wait
 *  - analyzed / get result -> the output of a compiler is another circuit (and a classical processing,
 *    not returned, to repair the result). The QDMI interface uses "CUSTOM1" to return the result
 *  - deleted
 */
int QAPTIVA_COMPILER_QDMI_device_session_create_device_job(
    QAPTIVA_COMPILER_QDMI_Device_Session session,
    QAPTIVA_COMPILER_QDMI_Device_Job * job
) {
    (*job) = new QAPTIVA_COMPILER_QDMI_Device_Job_impl_d;
    (*job)->session = session;
    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_set_parameter(
    QAPTIVA_COMPILER_QDMI_Device_Job job,
    QDMI_Device_Job_Parameter param,
    size_t size,
    const void * value
) {
    switch (param) {
    case QDMI_DEVICE_JOB_PARAMETER_PROGRAM:
        job->program_content = std::string(reinterpret_cast<const char *>(value), size);
        break;
    case QDMI_DEVICE_JOB_PARAMETER_CUSTOM1:
        job->target_gate_set = std::string(reinterpret_cast<const char *>(value), size);
        break;
    default:
        return QDMI_ERROR_NOTSUPPORTED;
    }

    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_query_property(
    QAPTIVA_COMPILER_QDMI_Device_Job job,
    QDMI_Device_Job_Property prop,
    size_t size,
    void * value,
    size_t * size_ret
) {
    if (job == nullptr)
        return QDMI_ERROR_INVALIDARGUMENT;

    // Retrieve property value
    std::string_view result;

    switch (prop) {
    case QDMI_DEVICE_JOB_PARAMETER_PROGRAM: {
        if (job->program_content.empty())
            return QDMI_ERROR_BADSTATE;

        result = job->program_content;
        break;
    }
    case QDMI_DEVICE_JOB_PARAMETER_CUSTOM1:
        if (job->target_gate_set.empty())
            return QDMI_ERROR_BADSTATE;

        result = job->target_gate_set;
        break;
    case QDMI_DEVICE_JOB_PROPERTY_ID:
        if (job->job_id.empty())
            return QDMI_ERROR_BADSTATE;

        result = job->job_id;
        break;
    default:
        return QDMI_ERROR_INVALIDARGUMENT;
    }

    // Copy result
    if (size < result.size())
        return QDMI_ERROR_OUTOFMEM;

    std::memcpy(value, result.data(), result.size());
    *size_ret = result.size();

    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_submit(QAPTIVA_COMPILER_QDMI_Device_Job job) {
    if (not global_python_interpreter) {
        return QDMI_ERROR_BADSTATE;
    }

    auto locals = pybind11::dict("target_gate_set"_a=job->target_gate_set, "program_content"_a=job->program_content);

    try {
        pybind11::exec(R"(
            from qlmaas.plugins import NISQCompiler
            from qat.devices import LineDevice
            from qat.interop.openqasm import OqasmParser

            circuit = OqasmParser().compile(program_content)
            compiler = NISQCompiler(target_gate_set=target_gate_set)
            object_to_be_compiled = circuit.to_job()

            line_device = LineDevice(circuit.nbqbits)
            submitted_job = compiler.compile(object_to_be_compiled, line_device)
            job_id = submitted_job.job_id
        )", pybind11::globals(), locals);
    } catch (pybind11::error_already_set &) {
        return QDMI_ERROR_FATAL;
    }

    job->job_id = locals["job_id"].cast<std::string>();
    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_cancel(QAPTIVA_COMPILER_QDMI_Device_Job job) {
    if (not global_python_interpreter or job->job_id.empty()) {
        return QDMI_ERROR_BADSTATE;
    }

    auto locals = pybind11::dict("job_id"_a=job->job_id);

    try {
        pybind11::exec(R"(
            from qlmaas.utils import get_job

            is_cancelled = get_job(job_id).cancel()
            assert is_cancelled, 'Could not cancel job (e.g., failed, done, cancelled)'
        )", pybind11::globals(), locals);
    } catch(pybind11::error_already_set&) {
        return QDMI_ERROR_FATAL;
    }

    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_check(QAPTIVA_COMPILER_QDMI_Device_Job job, QDMI_Job_Status * status) {
     if (not global_python_interpreter or job->job_id.empty()) {
        return QDMI_ERROR_BADSTATE;
    }

    auto locals = pybind11::dict("job_id"_a=job->job_id);

    try {
        pybind11::exec(R"(
            from qlmaas.utils import get_job

            status = get_job(job_id).get_status()
        )", pybind11::globals(), locals);
    } catch(pybind11::error_already_set&) {
        return QDMI_ERROR_FATAL;
    }

    // Python method "get_status()" returns a strignified status.
    // Qaptiva Access statuses are documented here:
    // https://qlm.bull.com/qlm-doc/04_api_reference/module_qat/module_comm/:qlm:module_qlmaas/jobstatus.html
    if (auto python_status = locals["status"].cast<std::string>(); python_status == "waiting") {
        *status = QDMI_JOB_STATUS_QUEUED;
    }
    else if (python_status == "running") {
        *status = QDMI_JOB_STATUS_RUNNING;
    }
    else if (python_status == "done") {
        *status = QDMI_JOB_STATUS_DONE;
    }
    else if (python_status == "cancelled") {
        *status = QDMI_JOB_STATUS_CANCELED;
    }
    else if (python_status == "failed") {
        *status = QDMI_JOB_STATUS_FAILED;
    }
    else {
        return QDMI_ERROR_FATAL;
    }

    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_wait(QAPTIVA_COMPILER_QDMI_Device_Job job, size_t /* unsupported_timeout */) {
    auto locals = pybind11::dict("job_id"_a=job->job_id);

    try {
        pybind11::exec(R"(
            from qlmaas.utils import get_job

            get_job(job_id).join()
        )", pybind11::globals(), locals);
    } catch(pybind11::error_already_set &) {
        return QDMI_ERROR_FATAL;
    }

    return QDMI_SUCCESS;
}


int QAPTIVA_COMPILER_QDMI_device_job_get_results(
    QAPTIVA_COMPILER_QDMI_Device_Job job,
    QDMI_Job_Result result_type,
    size_t size,
    void * data,
    size_t * size_ret
) {
    if (result_type != QDMI_JOB_RESULT_CUSTOM1)
        return QDMI_ERROR_NOTSUPPORTED;

    auto locals = pybind11::dict("job_id"_a=job->job_id);

    try {
        pybind11::exec(R"(
            from qlmaas.utils import get_job

            qaptiva_to_oqasm = {
                "H": "h", "X": "x", "Y": "y", "Z": "z",
                "RX": "rx", "RY": "ry", "RZ": "rz", "PH": "ph", "K": "k",
                "CNOT": "cx", "CSIGN": "cz",
            }
            compiled_circuit = get_job(job_id).get_result().circuit
            oqasm_lines = ["OPENQASM 2.0;", f"qreg q[{compiled_circuit.nbqbits}];", ""]

            for gate, params, qbits in compiled_circuit.iterate_simple():
                oqasm_gate = qaptiva_to_oqasm[gate]

                if params:
                    oqasm_gate += "[" + ",".join(str(angle) for angle in params) + "]"

                oqasm_lines.append(oqasm_gate + " " + ",".join(f"q[{i}]" for i in qbits) + ";")

            oqasm_output = "\n".join(oqasm_lines)
        )", pybind11::globals(), locals);
    } catch(pybind11::error_already_set &) {
        return QDMI_ERROR_FATAL;
    }

    auto result_str = locals["oqasm_output"].cast<std::string>();
    std::memcpy(data, result_str.data(), result_str.size());
    *size_ret = result_str.size();

    return QDMI_SUCCESS;
}


void QAPTIVA_COMPILER_QDMI_device_job_free(QAPTIVA_COMPILER_QDMI_Device_Job job) {
    delete job;
}
