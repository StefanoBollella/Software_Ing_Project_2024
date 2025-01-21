#include "main.h"

using namespace std;

static vector<string> CONFIG_PARAMS = {
	"INITIAL_WAITING_THRESHOLD",
	"RANGE_PROB",
	"REDUCTION_FACTOR",
	"MAX_ID_PRODUCTS",
	"MAX_SUPPLIER_QUEUE",
	"Q_GEN_RATIO",
	"N_INTERVALS",
	"MIN_PRODUCT_Q",
	"MAX_PRODUCT_Q"
};

static vector<char> PROBLEMATIC_CHARS = {
    '\n',   // Newline
    '\r',   // Carriage return
    '\t',   // Horizontal tab
    '\b',   // Backspace
    '\f',   // Form feed
    '\v',   // Vertical tab
    '\0',   // Null character
    '\\',   // Backslash
    '/',    // Slash
    ':',    // Colon
    '*',    // Asterisk
    '?',    // Question mark
    '\"',   // Double quote
    '<',    // Less than
    '>',    // Greater than
    '|',    // Pipe
    ' ',    // Space
    ',',    // Comma
    ';',    // Semi-colon
    '=',    // Equals
    '%',    // Percent sign
    '.'     // Dot
};

optional<YAML::Node> loadConfigFile(const string& configPath) noexcept(false)
{
	/*PARAM configPath path length check*/
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
	const string ERROR_MESSAGE = "param: configPath filename contains invalid chars";

	
	//stem is the filename without the extension, e.g. file.txt has stem = file
	const string stem = pathObj.stem().string();
	for (const char probChar : PROBLEMATIC_CHARS)
	{
		if (stem.find(probChar) != string::npos) 
			throw invalid_argument(ERROR_MESSAGE);
	}

	/*PARAM configPath extension check*/
	const filesystem::path EXPECTED_EXT1 = ".yaml";
	const filesystem::path EXPECTED_EXT2 = ".yml";
	const filesystem::path CURR_EXT = pathObj.extension();
	if (CURR_EXT != EXPECTED_EXT1 && CURR_EXT != EXPECTED_EXT2)
		throw invalid_argument("param: configPath extension not equal '.yaml' nor '.yml'");

	/*PARAM configPath existence check*/
	if ( !filesystem::exists(pathObj) )
	{
		cerr << "Err: " << configPath << " not found!" << endl;
		cerr << "Please provide yaml config file with following configurations:\n";
		for (auto &param : CONFIG_PARAMS)
			cerr << param << ": #\n";
		cerr << endl;
		throw invalid_argument("param: missing yaml config file");
	}

	/*PARAM configPath owner read permission check*/
	auto filePermissions = filesystem::status(pathObj).permissions();
	if ((filePermissions & filesystem::perms::owner_read) == filesystem::perms::none)
		throw invalid_argument("param: configPath file no owner read permission");

   
	/*PARAM configPath config file params checks*/
	 optional<YAML::Node> customerParams;
	 try {
		customerParams = make_optional<YAML::Node>(YAML::LoadFile(configPath));
	 }
	 catch (const YAML::Exception& e) {
		cerr << "Err: "  << e.what() 
		     << " when loading YAML file from " << configPath << endl;
		return nullopt;
	 }

	 if (!customerParams.has_value())
		return nullopt;

  for (auto &param : CONFIG_PARAMS) {
		//Check if the parameter exists
	if (!(*customerParams)[param]) {
		    cerr << "Err: Missing '" << param << ": #' from " << configPath << endl;
		    return nullopt;
	 }

	 //Check if the parameter has a value
	 if ((*customerParams)[param].IsNull()) {
		    cerr << "Err: Missing value for " << param << " from " << configPath << endl;
		    return nullopt;
	  }

	  //Check if the parameter is a scalar (boolean, integer, floating-point, string)
		
          if (!(*customerParams)[param].IsScalar()) {
	       cerr << "Err: " << param << " value must be a number from " << configPath << endl;
		 return nullopt;
          }

          //Convert the parameter to the appropriate type based on the parameter name
	  if (param == "INITIAL_WAITING_THRESHOLD" || param == "RANGE_PROB" || param == "REDUCTION_FACTOR") {
		    
		try {
		       (*customerParams)[param].as<double>();
		} 
		catch (const YAML::TypedBadConversion<double>& e) {
		       cerr << "Err: " << param << " value must be a double from " << configPath << endl;
		       return nullopt;
		    }
		} 
	   else {
		  try {
		              (*customerParams)[param].as<unsigned int>(); // Use unsigned int instead of unsigned long
		   } 
		   catch (const YAML::TypedBadConversion<unsigned int>& e) {
		         cerr << "Err: " << param << " value must be an unsigned int from " << configPath << endl;
		         return nullopt;
		    }
	    }
     }
  return customerParams;
}

