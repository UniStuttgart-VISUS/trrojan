/// <copyright file="output.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::output trrojan::open_output
 */
template<class I>
trrojan::output trrojan::open_output(I cmdLineBegin, I cmdLineEnd) {
    trrojan::output retval;

    auto output = trrojan::find_argument("--output", cmdLine.begin(),
        cmdLine.end());
    if (output != cmdLine.end()) {
        retval = make_output(*output);
    } else {
        log::instance().write_line(trrojan::log_level::warning, "You have not "
            "specified an output file. Please do so using the --output "
            "option.");
        retval = std::make_shared<console_output>();
    }

    if (std::dynamic_pointer_cast<csv_output>(retval) != nullptr) {
        auto params = basic_output_params::create<csv_output_params>(*output,
            cmdLineBegin, cmdLineEnd)
        retval->open(params);

    } else if (std::dynamic_pointer_cast<excel_output>(retval) != nullptr) {
        auto params = basic_output_params::create<excel_output_params>(*output,
            cmdLineBegin, cmdLineEnd)
        retval->open(params);

    } else if (std::dynamic_pointer_cast<console_output>(retval) != nullptr) {
        auto params = console_output_params::create();
        retval->open(params);
    }

    return retval;
}
