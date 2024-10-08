#include <iostream>
#include <chrono>
#include <queue>
#include <cpr/cpr.h>

using namespace std;

const bool REDIRECT_FILEOUT = true; // True: redirect cout to FILE_NAME; False: keep default
const string FILE_NAME = "out.txt";
const int LIMIT = 500; // Allowed values: [1, 1000]
const string SYMBOL = "BTCUSDT";
const string URL = "https://fapi.binance.com/fapi/v1/aggTrades?symbol=" + SYMBOL + "&limit=" + to_string(LIMIT);

typedef struct AggTrade
{
    unsigned long long AggregateTradeId;
    double Price;
    double Quantity;
    unsigned long long FirstTrade;
    unsigned long long LastTrade;
    time_t Timestamp;
    bool BuyerIsMaker;

} AggTrade;

size_t print_aggtrade_json(const string &json, ostream &out);
void print_aggtrades(queue<AggTrade *> &trades, ostream &out);
queue<AggTrade *> *parse_aggtrade_json(const string &json);
bool verify_aggtrade_format(const string &json);
tuple<AggTrade *, size_t> parse_single_aggtrade(const string &json, const size_t start_index = 6);
size_t get_next_index(const string &json, const size_t start_idx);

int main(int, char **)
{
    using chrono_clock = std::chrono::high_resolution_clock;
    using chrono_tp = std::chrono::high_resolution_clock::time_point;
    using chrono_ms = std::chrono::milliseconds;
    using std::chrono::duration;
    using std::chrono::duration_cast;

    ostream *output_stream;
    if (REDIRECT_FILEOUT)
    {
        output_stream = new ofstream(FILE_NAME);
    }
    else
    {
        output_stream = &cout;
    }

    *output_stream << "--- Part 2 ---\n";
    stringstream stats_stream;
    cpr::Response r = cpr::Get(cpr::Url{URL});
    string data = r.text;

    // ---------------------------------------------------------------------------------------------------------
    // Option 1
    // ---------------------------------------------------------------------------------------------------------

    chrono_tp print_t1 = chrono_clock::now();
    *output_stream << "AggTrades Option 1:\n";
    size_t trade_count = print_aggtrade_json(data, *output_stream);
    chrono_tp print_t2 = chrono_clock::now();
    // duration print_ms = duration_cast<chrono_ms>(print_t2 - print_t1); // milliseconds as  int
    duration<double, std::milli> print_ms = print_t2 - print_t1; // milliseconds as double

    stats_stream << "\n--------------------------------\n"
                 << "Option 1: \"Print-Parser\"\n"
                 << "---\n"
                 << "Total Time: " << print_ms.count() << "ms\n"
                 << "Nr. of AggTrade: " << trade_count << '\n'
                 << "Time per AggTrade: " << (print_ms.count() / trade_count) << "ms\n";

    // ---------------------------------------------------------------------------------------------------------
    // Option 2
    // ---------------------------------------------------------------------------------------------------------

    chrono_tp parse_t1 = chrono_clock::now();
    queue<AggTrade *> *trades = parse_aggtrade_json(data);
    chrono_tp parse_t2 = chrono_clock::now();
    duration<double, std::milli> parse_ms = parse_t2 - parse_t1; // milliseconds as double

    stats_stream << "\n--------------------------------\n"
                 << "Option 2: \"Object-Parser\"\n"
                 << "---\n"
                 << "Total Time: " << parse_ms.count() << "ms\n"
                 << "Nr. of AggTrade: " << (*trades).size() << '\n'
                 << "Time per AggTrade: " << (parse_ms.count() / (*trades).size()) << "ms\n";
    *output_stream << "AggTrades Option 2:\n";
    print_aggtrades(*trades, *output_stream);

    // ---------------------------------------------------------------------------------------------------------
    // Result: Print statistics
    // ---------------------------------------------------------------------------------------------------------
    *output_stream << stats_stream.str() << '\n';
    if (REDIRECT_FILEOUT)
    {
        cout << "Part 2 done, see: " << FILE_NAME << endl;
    }
}

/// @brief Print the trades from their JSON response and return the number of parsed trades.
/// @param json AggTrade data as JSON string.
/// @param out Output stream
/// @return Nr of trades as size_t
size_t print_aggtrade_json(const string &json, ostream &out)
{
    size_t i = 0;
    size_t trade_count = 0;
    while (i < json.length())
    {
        if (json[i] == '[')
        {
            out << "[\n";
        }
        else if (json[i] == ']')
        {
            out << "\n]\n";
        }
        else if (json[i] == '{')
        {
            trade_count++;
            out << "  {\n    ";
        }
        else if (json[i] == '}')
        {
            out << "\n  }";
        }
        else if (json[i] == ',')
        {
            if (json[i + 1] == '\"')
            {
                out << ",\n    ";
            }
            else
            {
                out << ",\n";
            }
        }
        else if (json[i] == ':')
        {
            out << ": ";
        }
        else
        {
            out << json[i];
        }
        i++;
    }
    return trade_count;
}

/// @brief Print all AggTrades until the queue is empty.
/// @param trades Queue of AggTrades.
/// @param out Output stream.
void print_aggtrades(queue<AggTrade *> &trades, ostream &out)
{
    streamsize orig_precision = out.precision();
    out << std::fixed
        << setprecision(8)
        << "[\n";

    while (!trades.empty())
    {
        AggTrade *trade = trades.front();
        trades.pop();

        if (trade != nullptr)
        {
            out << "  {\n"
                << "    \"a\": " << trade->AggregateTradeId << ",\n"
                // Uncomment to match precision of the current API response (and Option 1):
                // << "    \"p\": \"" << setprecision(2) << trade->Price << "\",\n"
                // << "    \"q\": \"" << setprecision(3) << trade->Quantity << "\",\n"
                // Precision based on the example in the task:
                << "    \"p\": \"" << trade->Price << "\",\n"
                << "    \"q\": \"" << trade->Quantity << "\",\n"
                << "    \"f\": " << trade->FirstTrade << ",\n"
                << "    \"l\": " << trade->LastTrade << ",\n"
                << "    \"T\": " << trade->Timestamp << ",\n"
                << "    \"m\": " << (trade->BuyerIsMaker ? "true" : "false") << '\n';
            if (trades.empty())
            {
                out << "  }\n";
            }
            else
            {
                out << "  },\n";
            }
        }
    }
    out << "]\n"
        << defaultfloat
        << setprecision(orig_precision);
}

/// @brief Parse a JSON string of AggTrades.
/// @param json Contains the AggTrades.
/// @return Parsed AggTrades as queue<AggTrade *> *
queue<AggTrade *> *parse_aggtrade_json(const string &json)
{
    if (!verify_aggtrade_format(json))
    {
        cerr << "ERROR: The JSON does not match the implemented format and thus cannot be parsed!";
    }
    queue<AggTrade *> *trades = new queue<AggTrade *>();
    size_t idx = 6;
    AggTrade *aggtrade;
    while (idx < json.length())
    {
        tie(aggtrade, idx) = parse_single_aggtrade(json, idx);
        (*trades).push(aggtrade);
    }
    return trades;
}

/// @brief Verify whether the JSON corresponds to the implementation. This is only a basic verification.
/// @param json AggTrade as JSON string.
/// @return True if the JSON matches the implementation and can be parsed.
bool verify_aggtrade_format(const string &json)
{
    bool res = true;
    size_t parameter_count = 7;
    char *param_names = new char[parameter_count]{'a', 'p', 'q', 'f', 'l', 'T', 'm'};
    size_t idx = 5; // first semicolon of the first object

    for (int i = 0; i < parameter_count; i++)
    {
        if (json[idx - 2] != param_names[i])
        {
            res = false;
            break;
        }
        idx = json.find(':', idx + 1);
    }
    return res;
}

/// @brief Parse an AggTrade JSON object.
/// @param json Must not contain any whitespaces.
/// @return Parsed AggTrade and end-index as tuple<AggTrade *, size_t>
tuple<AggTrade *, size_t> parse_single_aggtrade(const string &json, const size_t start_index)
{
    /*
        Index gaps based on JSON list of AggTrade objects (without whitespaces):
        {
            "a": 2353610640,
            "p": "62037.70",
            "q": "0.002",
            "f": 5445196068,
            "l": 5445196068,
            "T": 1728147158528,
            "m": false
        }
    */
    const size_t GAP_A_P = 6;
    const size_t GAP_P_Q = 7;
    const size_t GAP_Q_F = 6;
    const size_t GAP_F_L = 5;
    const size_t GAP_L_T = 5;
    const size_t GAP_T_M = 5;
    const size_t GAP_OBJECT = 6;
    if (json.length() <= start_index)
    {
        return make_tuple(nullptr, start_index); // empty array
    }
    AggTrade *res = new AggTrade;
    size_t idx_start = start_index;
    size_t idx_end;

    try
    {
        idx_end = get_next_index(json, idx_start);
        string value = json.substr(idx_start, idx_end - idx_start);
        res->AggregateTradeId = stoull(value);

        idx_start = idx_end + GAP_A_P;
        idx_end = get_next_index(json, idx_start);
        value = json.substr(idx_start, idx_end - idx_start);
        res->Price = stod(value);

        idx_start = idx_end + GAP_P_Q;
        idx_end = get_next_index(json, idx_start);
        value = json.substr(idx_start, idx_end - idx_start);
        res->Quantity = stod(value);

        idx_start = idx_end + GAP_Q_F;
        idx_end = get_next_index(json, idx_start);
        value = json.substr(idx_start, idx_end - idx_start);
        res->FirstTrade = stoull(value);

        idx_start = idx_end + GAP_F_L;
        idx_end = get_next_index(json, idx_start);
        value = json.substr(idx_start, idx_end - idx_start);
        res->LastTrade = stoull(value);

        idx_start = idx_end + GAP_L_T;
        idx_end = get_next_index(json, idx_start);
        value = json.substr(idx_start, idx_end - idx_start);
        res->Timestamp = (time_t)stoull(value);

        idx_start = idx_end + GAP_L_T;
        idx_end = get_next_index(json, idx_start);
        value = json.substr(idx_start, idx_end - idx_start);
        res->BuyerIsMaker = value[0] == 't' ? true : false;

        idx_end += GAP_OBJECT;
    }
    catch (exception &e)
    {
        cerr << "ERROR parse_single_aggtrade: " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "ERROR parse_single_aggtrade: " << "Something went wrong." << endl;
    }
    return make_tuple(res, idx_end + 1);
}

/// @brief Find the next end-index of a JSON value.
/// @param json JSON containing AggTrade values.
/// @param start_idx Where the JSON value starts.
/// @return The values end-index as size_t.
size_t get_next_index(const string &json, const size_t start_idx)
{
    size_t i = start_idx;
    size_t size = json.length();
    while (
        json[i] != ',' &&
        json[i] != '}' &&
        json[i] != '\"' &&
        i < size)
    {
        i++;
    }
    return i;
}
