#include <iostream>
#include <iomanip>

#ifndef __MY_HASH_TABLE_H__
#define __MY_HASH_TABLE_H__

/// @brief Hash table entry
typedef struct MyItem
{
    char *key;
    int *value;

    // To keep track of the insertion/modifaction order of items.
    MyItem *prev_item; // previous item
    MyItem *next_item; // successor item
} MyItem;

class MyHashTable
{
private:
    /// @brief Items stored in the hash table.
    MyItem **items;
    /// @brief Capacity of the hash table.
    unsigned long size;
    /// @brief Current number of elements stored in the hash table.
    unsigned long count;
    /// @brief Count the number of collisions.
    unsigned long collision_count;
    /// @brief Count the number of not inserted items.
    unsigned long missed_count;
    /// @brief The most recently inserted/changed item in the hash table.
    MyItem *last_item;
    /// @brief The least recently inserted/changed item in the hash table.
    MyItem *first_item;
    unsigned long MyHashTable::hash(char *word);
    MyItem *create_my_item(char *key, int *value);
    void insert_my_item(unsigned long index, MyItem *item);
    void free_my_item(MyItem *item);
    MyItem *find(char *key);

public:
    MyHashTable(unsigned long size);
    void insert(char *key, int *value);
    void remove(char *key);
    int *get(char *key);
    MyItem *get_last();
    MyItem *get_first();
    void print_all();
};

#endif
