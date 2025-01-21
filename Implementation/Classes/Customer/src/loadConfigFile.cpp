#include "main.h"

using namespace std;

namespace {
vector<string> CONFIG_PARAMS = {
    "ORDER_LIMIT",         "MAX_CUST_QTY",         "CANCEL_PROB",
    "LOGOUT_PROB",         "SHOPPING_PROB",        "MAX_PRODUCTS",
    "RESTOCK_QTY",         "CYCLE_CUST_GEN_RATIO", "INIT_CUST_BASE",
    "CYCLE_PROD_GEN_RATIO"
};

vector<char> PROBLEMATIC_CHARS = {
    '\n', // Newline 
    '\r', // Carriage return
    '\t', // Horizontal tab
    '\b', // Backspace
    '\f', // Form feed
    '\v', // Vertical tab
    '\0', // Null character
    '\\', // Backslash
    '/',  // Slash
    ':',  // Colon
    '*',  // Asterisk
    '?',  // Question mark
    '"', // Double quote
    '<',  // Less than
    '>',  // Greater than
    '|',  // Pipe
    ' ',  // Space
    ',',  // Comma
    ';',  // Semi-colon
    '=',  // Equals
    '%',  // Percent sign
    '.'   // Dot
};

} // namespace

optional<YAML::Node> loadConfigFile(const string &configPath) noexcept(false) {
    /* PARAM configPath path length check */
    if (configPath.length() > PATH_MAX)
        throw invalid_argument("param: configPath path too long");

    filesystem::path pathObj(configPath);

    /* PARAM configPath is empty check */
    if (pathObj.empty())
        throw invalid_argument("param: configPath is empty");

    /* PARAM configPath no filename check */
    if (!pathObj.has_filename())
        throw invalid_argument("param: configPath contains no filename");

    /* PARAM configPath checks filename length */
    if (pathObj.filename().string().length() > NAME_MAX)
        throw invalid_argument("param: configPath filename too long");

        /* PARAM configPath invalid filename check */
#if 0
	const string ERROR_MESSAGE = (
	    "param: configPath filename is invalid, "
	    "remove these characters from the filename "
	    "{=, dot, comma, colon, semi-colon, space, \\, |, /, *, %, <, >, ?}\n"
	    "{\\r, \\n, \\b, \\t, \\v, \\f, \\0}"
	);
#endif
    const string ERROR_MESSAGE =
        "param: configPath filename contains invalid chars";

    // stem is the filename without the extension, e.g. file.txt has stem = file
    const string stem = pathObj.stem().string();
    for (const char probChar : PROBLEMATIC_CHARS) {
        if (stem.find(probChar) != string::npos)
            throw invalid_argument(ERROR_MESSAGE);
    }

    /* PARAM configPath extension check */
    const filesystem::path EXPECTED_EXT1 = ".yaml";
    const filesystem::path EXPECTED_EXT2 = ".yml";
    const filesystem::path CURR_EXT = pathObj.extension();
    if (CURR_EXT != EXPECTED_EXT1 && CURR_EXT != EXPECTED_EXT2)
        throw invalid_argument(
            "param: configPath extension not equal '.yaml' nor '.yml'");

    /* PARAM configPath existence check */
    if (!filesystem::exists(pathObj)) {
        cerr << "Err: " << configPath << " not found!" << endl;
        cerr << "Please provide yaml config file with following "
                "configurations:\n";
        for (auto &param : CONFIG_PARAMS)
            cerr << param << ": #\n";
        cerr << endl;
        throw invalid_argument("param: missing yaml config file");
    }

    /* PARAM configPath owner read permission check */
    auto filePermissions = filesystem::status(pathObj).permissions();
    if ((filePermissions & filesystem::perms::owner_read) ==
        filesystem::perms::none)
        throw invalid_argument(
            "param: configPath file no owner read permission"
        );

    /* PARAM configPath config file params checks */
    optional<YAML::Node> customerParams;
    try {
        customerParams = make_optional<YAML::Node>(YAML::LoadFile(configPath));
    } catch (const YAML::Exception &e) {
        cerr << "Err: " << e.what() << " when loading YAML file from "
             << configPath << endl;
        return nullopt;
    }

    if (!customerParams.has_value())
        return nullopt;

    // checks for all the config file parameters defined in CONFIG_PARAMS
    for (auto &param : CONFIG_PARAMS) {
        try {
            // tries to check if param key exists
            (*customerParams)[param];
        } catch (const YAML::Exception &e) {
            cerr << "Err: " << e.what() << " when accessing " << param
                 << " from " << configPath << "\n"
                 << "Perhaps it's missing or it's syntax is wrong...\n"
                 << "Please check " << configPath << "..." << endl;
            return nullopt;
        }

        // missing key:value pair of the YAML map
        if (!(*customerParams)[param]) {
            cerr << "Err: "
                 << "Missing '" << param << ": #' "
                 << "from " << configPath << endl;
            return nullopt;
        }

        // present key, but missing value
        if ((*customerParams)[param].IsNull()) {
            cerr << "Err: "
                 << "Missing value for " << param << "from " << configPath
                 << endl;
            return nullopt;
        }

        // present key, but value not a YAML scalar (boolean, integer,
        // floating-point, string)
        if (!(*customerParams)[param].IsScalar()) {
            cerr << "Err: " << param << " value must be a positive integer "
                 << "from " << configPath << endl;
            return nullopt;
        }

        try {
            // tries to convert them into unsigned long
            (*customerParams)[param].as<unsigned long>();
        } catch (const YAML::TypedBadConversion<unsigned long> &e) {
            cerr << "Err: " << param << " value must be a positive integer "
                 << "from " << configPath << endl;
            return nullopt;
        }
    }

    return customerParams;
}
