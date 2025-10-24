#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

constexpr string NAN = "nan";
constexpr string POSITIVE_INF = "inf";
constexpr string NEGATIVE_INF = "-inf";

enum class RespType {
    SimpleString,
    SimpleError,
    Integer,
    BulkString,
    Array,
    Null,
    Boolean,
    Double,
    BigNumber,
    BulkError,
    VerbatimString,
    Map,
    Attribute,
    Set,
    Push
};

struct RespValue {
    RespType type;

    // For SimpleString, SimpleError, 
    // BulkString, BulkError, VerbatimString
    string string_value;

    // For Integer
    long long int_value;

    // For Array
    vector<RespValue> resp_array_elements;

    // For Boolean
    bool bool_value;

    // For Doubles
    long double double_value;
    long long integral;
    long long fractional;
    long long exponent;

    // ...
};

// calling the istream a buffer, since it originally was before conversion
RespValue resp_parser(istream& buffer) {
    // read the first character (to determine type)
    char type_prefix;
    buffer.get(type_prefix);

    // switch statement to parse the RESP data types and content/values
    switch (type_prefix) {
        // simple strings
        case '+':
            // get string value
            string line;
            getline(buffer, line, '\r');

            // ignore/skip \n
            buffer.ignore(1);

            // Return RespValue of the corresponding RespType
            RespValue simple_string = {RespType::SimpleString};
            simple_string.string_value = line;
            return simple_string;

        // simple errors
        case '-':
            // get the error message
            string err_msg;
            getline(buffer, err_msg, '\r');

            // ignore/skip \n
            buffer.ignore(1);

            // Define the RespValue and return it
            RespValue error_message = {RespType::SimpleError};
            error_message.string_value = err_msg;
            return error_message;  

        // integers
        case ':':
            // get the num which may have an optional sign
            string num_with_optional_sign;
            getline(buffer, num_with_optional_sign, '\r');

            // Ignore \n
            buffer.ignore(1);

            // Define the RespValue and return it
            RespValue integer = {RespType::Integer};
            // convert the number with sign to int (long long)
            integer.int_value = stoll(num_with_optional_sign);

            return integer;

        // bulk strings
        case '$':
            // get the length for the bulk string
            string str_len;
            getline(buffer, str_len, '\r');

            // Skip '\n'
            buffer.ignore(1);

            // Then get the string value
            string line;
            getline(buffer, line, '\r');

            // Skip '\n'
            buffer.ignore(1);

            // Then define the RespValue and return it
            RespValue bulk_string = {RespType::BulkString};
            // assign the values
            bulk_string.string_value = line;
            bulk_string.int_value = stoll(str_len);

            return bulk_string;

        // arrays
        case '*':
            // determine array length
            string arr_len_string;
            getline(buffer, arr_len_string, '\r');

            // ignore the \n
            buffer.ignore(1);
            
            // convert the arr length string to int
            int arr_len = stoi(arr_len_string);

            // Create the resp array
            RespValue resp_array_element = {RespType::Array};
            for (int i = 0; i < arr_len; i++) {
                // recursively call the resp parser to get the array's elements
                resp_array_element.resp_array_elements.push_back(resp_parser(buffer));
            }

            return resp_array_element;

        // nulls
        case '_':
            // ignore the \r and \n
            buffer.ignore(2);

            // Define and return  the RespValue
            return {RespType::Null};

        // booleans
        case '#':
            // determine the boolean value
            string boolean_value;
            getline(buffer, boolean_value, '\r');

            // Define the RespValue
            RespValue boolean = {RespType::Boolean};
            // convert the boolean_value to the appopriate values
            if (boolean_value == 't') {
                boolean.bool_value = true;
            } else if (boolean_value == 'f') {
                boolean.bool_value = false;
            } else {
                cerr << "No correct boolean value provided" << endl;
            }

            return boolean;

        // doubles
        case ',':
            // what we need to possibly find
            string integral_str;
            string fractional_str;
            string exponent_str;
            
            // define the delimiters that are possibly found within the values (not the CLRF)
            char decimal_delimiter = '.'; // between <integral>[.<fractional>]
            // Exponent delimiter (optional) [.<fractional>][<E|e>[sign]<exponent>]
            char exponent_delimiter_upper = 'E';
            char exponent_delimiter_lower = 'e';
            char cl_delimiter = '\r';

            // reads through till the CLRF ends (\n), capturing \r so I can parse value end positions
            string line;
            getline(buffer, line, '\n')
            
            size_t decimal_pos = line.find(decimal_delimiter);
            size_t exponent_upper_pos = line.find(exponent_delimiter_upper);
            size_t exponent_lower_pos = line.find(exponent_delimiter_lower);
            size_t cl_pos = line.find(cl_delimiter);

            size_t decimal = string::npos;
            
            // first case where there is both an integral and fractional value, and may have an exponent value
            if (decimal_pos != string::npos) {
                // extract integral_str and fractional_str
                integral_str = line.substr(0, decimal_pos);
                
                
                // Cases where we can parse for fractional end position:
                // Where the end position is up to the cl_pos or where the end position is to an exponent
                if (exponent_upper_pos != string::npos || exponent_lower_pos != string::npos) {
                    // get the delimiter position
                    if (exponent_upper_pos != string::npos) {
                        fractional_pos = exponent_upper_pos;
                    } else if (exponent_lower_pos != string::npos) {
                        fractional_pos = exponent_lower_pos;
                    }

                    fractional_str = line.substr(decimal_pos, fractional_pos - decimal_pos);

                    // then parse for exponents
                    exponent_str = line.substr(fractional_pos, cl_pos - fractional_pos);
                } else if (cl_pos != string::npos) { 
                    // the case where there were not exponents
                    fractional_str = line.substr(decimal_pos, cl_pos - decimal_pos);
                }
            } else {
                // there were no fractional value, but may still be an exponent value with the integral value, 
                // or the value is inf, -inf, nan
                
                // case where there is an exponent value (so value of integral is not inf, -inf, or nan)
                if (exponent_upper_pos != string::npos || exponent_lower_pos != string::npos) {
                    if (exponent_upper_pos != string::npos) {
                        exponent_pos = exponent_upper_pos;
                    } else if (exponent_lower_pos != string::npos) {
                        exponent_pos = exponent_lower_pos;
                    }

                    // get integral and exponent
                    integral_str = line.substr(0, exponent_pos);
                    exponent_str = line.substr(exponent_pos, cl_pos - exponent_pos);
                } else {
                    // could be just an integer or inf, -inf, nan
                    integral_str = line.substr(0, cl_pos);
                }
            }

            // Define the RespValue
            RespValue resp_double = {RespType::Double};

            // add all the appropriate values for the double, check if the different value options are empty or not
            // integral is a given (always in the protocol)
            long double decimal_num = stoll(integral_str);
            resp_double.integral = decimal_num;

            if (!fractional_str.empty()) {
                num_decimal_places = fractional_str.length();

                resp_double.fractional = stoll(fractional_str);

                // then use those values to calculate the actual double value
                long double decimal_num = decimal_num + static_cast<long double>(resp_double.fractional) / pow(10, num_decimal_places);
            }
            if (!exponent_str.empty()) {
                resp_double.exponent = stoll(exponent_str);

                // then use those values to calculate the actual double value
                long double decimal_num = pow(decimal_num, resp_double.exponent)
            }
            
            // add the decimal_num to resp_double's double_value and return it
            resp_double.double_value = decimal_num;
            return resp_double;
          
        // big numbers
        case '(':
          break;

        // bulk errors
        case '!':
          break;

        // verbatim strings
        case '=':
          break;

        // maps
        case '%':
          break;

        // attributes
        case '|':
          break;

        // sets
        case '~':
          break;

        // pushes
        case '>':
          break;

        default:
          break;
    }
}