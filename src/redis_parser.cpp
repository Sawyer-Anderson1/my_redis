#include <iostream>
#include <string>
#include <sstream>

using namespace std;

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

    // ...
};

// calling the istream a buffer, since it originally was before conversion
RespValue resp_parser(ssize_t num_bytes, istream& buffer) {
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
          break;

        // booleans
        case '#':
          break;

        // doubles
        case ',':
          break;
          
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