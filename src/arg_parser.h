#pragma once

#include <iostream>
#include <vector>
#include <map>

/* This, and the computations, are the only files where I found OOP to be useful */

/**
 * Option class
 * Represents a command line option
 * Option has a name, description, whether it has a value and whether it is required
 */
class option {
public:
    /** Option name */
    std::string name;
    /** Option description */
    std::string desc;
    /** Whether the option has a value */
    bool has_value;
    /** Whether the option is required */
    bool required;

    /**
     * Constructor
     * @param name Name of the option
     * @param desc Description of the option
     * @param has_value Whether the option has a value
     * @param required Whether the option is required
     */
    option(std::string name, std::string desc, bool has_value = false, bool required = false);

    /**
     * Destructor
     */
    ~option() = default;
};

/**
 * Argument parser class
 * Heavily inspired by Python's argparse module which I have been using a lot lately
 * Parses command line arguments and returns a map of options
 */
class arg_parser {
private:
    /** Number of arguments */
    int argc;
    /** Argument values */
    char **argv;
    /** List of options */
    std::vector<option> options;

public:
    /**
     * Constructor
     * @param argc Number of arguments
     * @param argv Argument values
     */
    arg_parser(int argc, char **argv);

    /**
     * Destructor
     */
    ~arg_parser() = default;

    /**
     * Add an option to the parser
     * @param opt Option to add
     */
    void add_option(const option &opt);

    /**
     * Parse the arguments
     * @return Map of options (key is the option name, value is the user provided value)
     */
    [[nodiscard]] std::map<std::string, std::string> parse_args() const;

    /**
     * Print usage message
     */
    void print_usage() const;

    /**
     * Print help message
     */
    void print_help() const;
};
