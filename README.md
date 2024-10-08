# Digital Wave Finance - Round 1 Interview Questions
This repository contains the result of the interview questions. An individual project has been created for each of the two parts. They can be found in the following folders:

- _part1_
- _part2_

They have been implemented on a Windows machine using Visual Studio Code and were not yet tested on other platforms.

## Implementation & Review - Part 1
The content of the book is received using the [cpr](https://docs.libcpr.org/introduction.html) library as part of `src/main.cpp`. Afterwards the words are extracted by iterating over a [stringstream](https://cplusplus.com/reference/sstream/stringstream). This is for sure not the fastest or most elegant solution but the words are received without manual work or thounsands of code lines.

Everything related to the hash table can be found in `src/my_hash_table.cpp` and `include/my_hash_table.h`. 
All parts related to the hash table are implemented as class `MyHashTable` to be reusable. The required functions with O(1)-complexity are at the bottom of the cpp-file. I assumed it's about time-complexity (not space complexity). The following sections are dedicated to the details of the hash table implementation.

### hash(key)
As hash function I used the algorithm [djb2](http://www.cse.yorku.ca/%7Eoz/hash.html) with the goal to spread the items (containing the words) accross the data-set without building clusters or causing too many collisions. This is working well but might be improved by further analysing the inserted items. It iterates over all characters of a given word, which results in **O(K)**, where K is the length of a word. Compared to the huge amount of words this might be neglectable. Or it could be considered constant if the max-length of a word is defined e.g. as 50.

### get(key)
Uses `find(key)` that returns the item and not just the value. Thanks to the hash function it is able to get the item/value with **O(1)** time-complexity. But, in the worst-case scenario it could also result in **O(N)**-complexity (N is the number of items) if the hash function is bad. 
_Example: A hash function that causes too many collisions or clusters the elements means that searching for the element requires iterating over all the elements and comparing their keys to find the correct element._

### insert(key, value) and remove(key)
Like `get(key)`. It's **O(1)** but with a "bad" hash function it can result in **O(N)** or worse.

### get_last() and get_first()
To get the most/least recent modified item, the pointers `last_item` and `first_item` have been added to `MyHashTable`. These allow access to the related items with a time-complexity of **O(1)**. To make sure these pointers are updated whenever new items are inserted, updated or removed, each `MyItem` stores a reference to `prev_item`/`next_item` an form a kind of linked list. With that the insert/remove functions get a little slower because they have to update the references too. But they are still fast since just 3 items are involved and can be accessed in O(1) time. Thus the insert and remove functions remain O(1).
_Example: When the last-item (most recently changed) is deleted, the last-key needs to be set to the deleted-item's previous key so that `get_last()` will return the "new" last-item. On the other hand, if an item in the "middle" is deleted, the neighbours need to be linked together. If a new item is inserted it needs to be linked to the item that was inserted before and the last-item pointer must be updated._


An alternative would have been to store the last/first item-keys in an additional data structure like a list (vector) or a stack. Compared to my previous idea this would have reduced the time-complexity to **O(N)**, where N is the number of items in the hash table if we had to iterate over the list to find the last/first item.
_For example, if the first-item (least recently updated) will be changed later, it has to be moved to the very end of the list because it's not the first anymore. Of course this depends on the selected data structure and its implementation..._

### Final thoughts
The performance (e.g. numbers of collisions or missing inserts) depend on the size of the hash table. The book contains 11'611 unique words that are inserted into the hash table. If the capacity `TABLE_SIZE` is less some words won't be inserted. If the size is equal all words will be inserted but without a good performance because it is causing a lot of collisions. The number of collisions decreases as the size of the hash table increases. At least to a certain extent. This is visualized in the following table.

| Table Size | # Inserted | # Not Inserted | # Collisions |
|------------|------------|----------------|--------------|
| 11'600     | 11'600     | 11             | 18'254       |
| 11'611     | 11'611     | 0              | 18'714       |
| 20'000     | 11'611     | 0              | 12'368       |
| 50'000     | 11'611     | 0              | 4'175        |
| 100'000    | 11'611     | 0              | 2'278        |

The [GNU gperf](https://www.gnu.org/software/gperf) is a hash function generator that would have been interesting to test and to compare against djb2 or other algorithms. But this is for another day...


## Implementation & Review - Part 2
Similar to part 1 the library [cpr](https://docs.libcpr.org/introduction.html) is used to retrieve the aggregated trades from: https://fapi.binance.com/fapi/v1/aggTrades. Inside `main.cpp` the query parameters: `LIMIT` and `SYMBOL` can be changed. Furthermore, with `REDIRECT_FILEOUT` you can define if the output should be redirected to a file instead of the console. Two options are implemented in order to parse the JSON string of trades so that the speed measurements can be compared. The Options 1 and 2 are explained in the following sections. Below you can find an overview of the functions.

| Function                 | Option | Explanation                                                                            |
|--------------------------|--------|----------------------------------------------------------------------------------------|
| `main`                   | 1, 2   | Get the trade data and run the two options.                                            |
| `print_aggtrade_json`    | 1      | Parse the JSON string by iterating over every character and print them directly.       |
| `print_aggtrades`        | 2      | Print the parsed `AggTrade` objects.                                                   |
| `parse_aggtrade_json`    | 2      | Parse the JSON into a que of `AggTrade` objects by iterating in jumps.                 |
| `verify_aggtrade_format` | 2      | Simple verification to check whether the JSON format still matches the implementation. |
| `parse_single_aggtrade`  | 2      | Parse one `AggTrade` object from the JSON.                                             |
| `get_next_index`         | 2      | Find the next index that signals the end of the current attribute's value.             |

When the program is executed the trades per option are printed to the console or the file, followed by the measurement for both options.

### Option 1
To solve the task and print the trades in the specified format it is enough to iterate over the characters and directly print them. The advantage of this option is its simplicity. On the other hand the program does not gain any information about the parsed data and thus cannot process it any further should this become necessary.

With the current implementation the time-complexity is **O(N)**, where N is the number of characters received from the API. 
Improvement ideas:
- The performance of the implementation can be improved by re-ordering the `if` statements based on their probability to be true (the most common should be checked first). Although, the effects would be minor and would of course not change the time-complexity. 
- Another idea is to not write each character to a stream and use a faster data structure.
- To improve the time-complexity one has to find a faster algorithm than iterating over all characters. This is what option 2 is trying to do.

### Option 2
This option takes a different approach then option 1. It iterates with jumps over the received JSON string and extracts just the values per object (`AggTrade`). For example, the start index is 6 and not 0 because the first value of the first object starts at index 6: `[{"a":12345,...`. With that some checks/steps can be skipped and time is saved.

Another advantage of this option is that the data is stored and can be further processed. If the data is stored with a suitable data-structure, it is faster than directly print each character as seen in option 1. For that a queue has been selected with a time-complexity of **O(1)** when inserting new items or retrieving the first/last item. The queue is not suitable to iterate over all items or access a specific item. But this can be done to a later point in time where performance does not matter that much. Or it can be replaced with a different data-structure.

As this option relies on the format of the JSON string containing the trades, a control mechanism must be added to ensure the program works reliably. The function `verify_aggtrade_format` does a basic verification of the received json. If the format has been changed, the verification fails and parsing is not continued. In this case the implementation needs to be adapted, which is a disadvantage.

The overall time-complexity is **O(M)**, where M is the number of characters of the "attribute-values" for all trades that have to be parsed. Also, M is smaller than N from option 1.

### Final thoughts
In terms of speed the following functions have been measured:
- Option 1: `print_aggtrade_json`
- Option 2: `parse_aggtrade_json`, which uses: `verify_aggtrade_format`, `parse_single_aggtrade`, `get_next_index`
_For option 2 the printing is not part of the measurement because it's not part of the parsing algorithm. The printing can be done at any time since the data has already been parsed and could also be saved._

| # Trades | Option 1<br>Console [ms]            | Option 2<br>Console  [ms]           | Option 1<br>File  [ms]               | Option 2<br>File  [ms]              |
|---------:|-------------------------------------|-------------------------------------|--------------------------------------|-------------------------------------|
| 100      | All: 257.744    <br>Avg: 2.57744    | All: 0.7045   <br>Avg: 0.007045     | All: 2.1166   <br>Avg: 0.021166      | All: 1.1423   <br>Avg: 0.011423     |
| 500      | All: 1259.36    <br>Avg: 2.51872    | All: 3.3444   <br>Avg: 0.0066888    | All: 8.0488   <br>Avg: 0.0160976     | All: 3.1781   <br>Avg: 0.0063562    |
| 1000     | All: 2508.99    <br>Avg: 2.50899    | All: 6.7182   <br>Avg: 0.0067182    | All: 15.9433   <br>Avg: 0.0159433    | All: 6.4659   <br>Avg: 0.0064659    |

The table above shows the measured time per option (Option 1/2 and Console/File output) with a different amount of data (nr. of trades). In all cases option 2 is faster than option 1. This is probably not only due to the algorithm but also to the stream output. Without that the difference might be much smaller. The table also indicates that the average time (time to parse a single trade) is getting smaller the more trades have to be parsed. This is because not a single trade is measured but the average is taken. Thus the overhead of the full parsing algorithm is shared accross all trades. Furthermore, the output to a console is much slower than to the file.
_Side note: Maybe it would have been better to use an output other than a stream for option 1 to compare the speed of the two algorithms..._


## Conclusion
The speed results from the time-complexity of the algorithms used as well as from the efficient use of instructions and of third-party libraries. My solution might not be the best but I believe it is a good starting point.
It was a pleasure to dive deeper into the technical details of the algorithms and to deal with C++ in general. In my opinion, I have gained a very good insight into the nature of tasks carried out at DWF.

# Getting Started
Use [CMake](https://cmake.org/) to build the projects. You can follow the steps below. The projects have been compiled with [GCC 13.1.0](https://gcc.gnu.org/releases.html) during development.

Steps using command line:

1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. `cmake --build .`
5. Run: _Debug/part1.exe_ or _Debug/part2.exe_

# Task
The solutions must be provided in C / C++. Please mention all your steps and explain what led you to choose your solution. You can briefly comment on other solutions and ideas which you had while solving this task.

### Part 1: Data Structures and Algorithms
Create a data-set of words from the book https://www.gutenberg.org/files/98/98-0.txt. Implement a fixed sized hash table by using linear probing to resolve collisions. Assume that the keys are the words from the given data-set and the hash table’s values are integers. You need to implement the following functions with O(1)-complexity:

- insert(key, value): inserts a new key-value pair or replaces a key’s existing value,
- remove(key): removes the corresponding key-value pair,
- get(key): returns the value of the corresponding key,
- get_last(): returns the most recently inserted or changed key-value pair,
- get_first(): returns the least recently inserted or changed key-value pair

### Part 2: Trading Specific Algorithms
Review the API of Binance USD(S)-M Futures that can be found in https://binance-docs.github.io/apidocs/futures/en/#change-log. Then
- implement connectivity to the end-point GET /fapi/v1/aggTrades,
- receive a stream of trades. Write a parsing algorithm to parse the incoming stream of trades and print each trade in the form:
 ```[
    {
        "a": 26129, // Aggregate tradeId
        "p": "0.01633102", // Price
        "q": "4.70443515", // Quantity
        "f": 27781, // First tradeId
        "l": 27781, // Last tradeId
        "T": 1498793709153, // Timestamp
        "m": true, // Was the buyer the maker?
    }
]
```
- measure the speed at which singular trades are parsed and comment on the algorithmic complexity of your parsing algorithm.
