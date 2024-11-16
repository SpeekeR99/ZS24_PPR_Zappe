#include "arg_parser.h"

option::option(std::string name, std::string desc, bool has_value, bool required)
    : name(std::move(name)), desc(std::move(desc)), has_value(has_value), required(required) {
    /* Nothing to do here */
}

arg_parser::arg_parser(int argc, char **argv)
    : argc(argc), argv(argv) {
    /* Nothing to do here */
}

void arg_parser::add_option(const option &opt) {
    this->options.push_back(opt);
}

std::map<std::string, std::string> arg_parser::parse_args() {
    /* Prepare the map of arguments */
    std::map<std::string, std::string> args;

    /* Iterate over the arguments */
    for (auto i = 1; i < this->argc; i++) {
        /* Get the argument */
        std::string arg = this->argv[i];
        /* For later use of known arguments */
        bool known = false;

        /* Print help message and exit successfully */
        if (arg == "-h" || arg == "--help") {
            this->print_help();
            exit(EXIT_SUCCESS);
        }

        /* Iterate over all the known options */
        for (const auto &opt : this->options) {
            /* If the argument is known */
            if (arg == opt.name) {
                known = true;  /* Mark it as known */
                /* If the option must have a value */
                if (opt.has_value) {
                    /* Check if the value is provided */
                    if (++i < this->argc) {
                        args[opt.name] = this->argv[i];
                    /* If the value is not provided */
                    } else {
                        std::cerr << "Option " << opt.name << " requires a value" << std::endl;
                        this->print_usage();
                        exit(EXIT_FAILURE);
                    }
                /* If the option does not have a value, store an empty string as value */
                } else {
                    args[opt.name] = "";
                }
            }
        }

        /* If the argument is not known, print an error message and exit */
        if (!known) {
            std::cerr << "Unknown option: " << arg << std::endl;
            this->print_usage();
            exit(EXIT_FAILURE);
        }
    }

    /* Check if all required options are provided */
    for (const auto &opt : this->options) {
        if (opt.required && args.find(opt.name) == args.end()) {
            std::cerr << "Option " << opt.name << " is required" << std::endl;
            this->print_usage();
            exit(EXIT_FAILURE);
        }
    }

    return args;
}

void arg_parser::print_usage() {
    /* Simple usage message */
    std::cerr << "Usage: " << this->argv[0] << " ";
    for (const auto &opt : this->options) {
        /* Optional options are enclosed in square brackets */
        if (!opt.required)
            std::cerr << "[ ";

        /* Print the option name and "<value>" if the option requires a value */
        std::cerr << opt.name << " ";
        if (opt.has_value)
            std::cerr << "<value> ";

        /* Optional options are enclosed in square brackets */
        if (!opt.required)
            std::cerr << "] ";
    }
    std::cerr << std::endl;
}

void arg_parser::print_help() {
    /* Simple help message; start of by printing the usage message */
    this->print_usage();
    std::cout << std::endl;

    /* List all the options with their descriptions */
    for (const auto &opt : this->options) {
        std::cout << opt.name << " - " << opt.desc;
        if (opt.required)
            std::cout << " (required)";
        if (opt.has_value)
            std::cout << " (value required)";
        std::cout << std::endl;
    }
}
