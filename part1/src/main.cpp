#include <iostream>
#include <cpr/cpr.h>
#include <sstream>
#include "my_hash_table.h"

using namespace std;

const string URL = "https://www.gutenberg.org/files/98/98-0.txt";
const string START_PHRASE = "*** START OF THE PROJECT GUTENBERG EBOOK A TALE OF TWO CITIES ***";
const unsigned long TABLE_SIZE = 50000;

int main()
{
    // --------------------------------------------------------------------------------------------------------
    // ENABLE UTF-8 Console-Out
    // Source: https://stackoverflow.com/questions/45575863/how-to-print-utf-8-strings-to-stdcout-on-windows

    // Set console code page to UTF-8 so console known how to interpret string data
    SetConsoleOutputCP(CP_UTF8);

    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
    // --------------------------------------------------------------------------------------------------------

    cout << "--- Part 1 ---" << endl;
    // Get the book as Response via HTTP-GET Request
    cpr::Response r = cpr::Get(cpr::Url{URL});

    // Get only the book's content
    size_t start_index = r.text.find(START_PHRASE) + START_PHRASE.length();
    string book_content = r.text.substr(start_index); // O(n), to be faster we could use a different data structure than string, e.g. Rope: https://en.wikipedia.org/wiki/Rope_(data_structure)

    // Remove punctuation, O(n)
    for (size_t i = 0; i < book_content.length(); i++)
    {
        if (iswpunct(book_content[i]))
        {
            book_content[i] = ' '; // O(1)
        }
    }

    // Insert the book's content into the hash table
    MyHashTable hash_table(TABLE_SIZE);
    stringstream stream(book_content);
    string word;
    int i = 0;
    while (stream >> word)
    {
        hash_table.insert(word.data(), &i);
        i++;
    }

    hash_table.print_all();
}
