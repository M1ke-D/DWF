#include "my_hash_table.h"

using namespace std;

/// @brief Constructor
/// @param size
/// @param word
MyHashTable::MyHashTable(unsigned long size)
{
  this->size = size;
  this->count = 0;
  this->missed_count = 0;
  this->collision_count = 0;
  this->first_item = nullptr;
  this->last_item = nullptr;
  this->items = (MyItem **)calloc(this->size, sizeof(MyItem *));
  for (unsigned long i = 0; i < this->size; i++)
  {
    this->items[i] = nullptr;
  }
}

/// @brief Calculates the hash of a key, which will be used as index in the hash table.
/// @details Using djb2 algorithm, source: http://www.cse.yorku.ca/%7Eoz/hash.html
/// @param key
/// @return
unsigned long MyHashTable::hash(char *key)
{
  unsigned long hash = 5381;
  int c;
  while (c = *key++)
  {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash % this->size;
}

/// @brief Create a pointer to a MyItem, which is part of the hash table.
/// @param key
/// @param value
/// @return
MyItem *MyHashTable::create_my_item(char *key, int *value)
{
  MyItem *item = (MyItem *)malloc(sizeof(MyItem));
  size_t key_length = strlen(key) + 1;
  item->key = (char *)malloc(key_length);
  item->value = (int *)malloc(sizeof(int));
  item->prev_item = (MyItem *)malloc(sizeof(MyItem));
  item->next_item = (MyItem *)malloc(sizeof(MyItem));
  strcpy_s(item->key, key_length, key);
  *(item->value) = *value;
  item->prev_item = nullptr;
  item->next_item = nullptr;
  return item;
}

/// @brief Insert the item at the corresponding index and increases the count.
/// @param index
/// @param item
void MyHashTable::insert_my_item(unsigned long index, MyItem *item)
{
  this->items[index] = item;
  this->count++;
}

/// @brief Free the item.
/// @param item
void MyHashTable::free_my_item(MyItem *item)
{
  free(item->key);
  free(item->value);
  free(item);
}

/// @brief Print all non-empty entries of the hash table and the statistics.
void MyHashTable::print_all()
{
  cout << "***My Hash Table***" << endl;
  for (unsigned long i = 0; i < this->size; i++)
  {
    MyItem *item = this->items[i];
    if (item != nullptr)
    {
      string prev_key = item->prev_item == nullptr ? "-" : item->prev_item->key;
      string next_key = item->next_item == nullptr ? "-" : item->next_item->key;
      cout << "Index: " << i
           << "\t Value: " << *(item->value)
           << "\t Key: " << left << setw(30) << item->key
           << "\t Prev Key: " << left << setw(30) << prev_key
           << "\t Next Key: " << next_key
           << endl;
    }
  }

  cout << "\n\n------------------\n\n"
       << "Count: " << this->count
       << "\nNr. of not inserted items: " << this->missed_count
       << "\nNr. of collisions: " << this->collision_count
       << endl;
}

// -----------------------------------------------------------
// To be implemented with O(1)-complexity!
// -----------------------------------------------------------

/// @brief Insert key-value pair as MyItem or updates the key's existing value.
/// @param key
/// @param value
void MyHashTable::insert(char *key, int *value)
{
  MyItem *new_item = create_my_item(key, value);
  unsigned long index = hash(key);
  MyItem *existing_item = this->items[index];

  // Key does not yet exist: insert the item.
  if (existing_item == nullptr) // O(1)
  {
    insert_my_item(index, new_item);
  }

  // Item does already exist: update its value.
  else if (strcmp(existing_item->key, key) == 0) // O(k), where k is the longer length of the keys being compared
  {
    *(existing_item->value) = *value;
  }

  // Handle collision with linear probing.
  else // O(n), where n is the size of the hash table: worst case, if the list is clustered/full
  {
    this->collision_count++;
    bool insert_failed = true;
    for (unsigned long i = 1; i < this->size; i++)
    {
      unsigned long free_index = ((index + i) % this->size);
      MyItem *next_slot = this->items[free_index];
      // Insert or update the item
      if (next_slot == nullptr)
      {
        insert_my_item(free_index, new_item);
        insert_failed = false;
        break;
      }
      else if (strcmp(next_slot->key, key) == 0)
      {
        *(next_slot->value) = *value;
        insert_failed = false;
        break;
      }
    }

    // Check if the list is full if the item was not yet inserted/updated.
    if (insert_failed && this->count >= this->size)
    {
      cerr << "Cannot insert (key: "
           << *key << ", value: " << *value
           << ") because hash table is full !!!"
           << endl;
      // throw "The hash table is full";
      this->missed_count++;
      free_my_item(new_item);
    }
  }

  // Track most recently updated key "last"
  MyItem *last_item = get_last();
  if (last_item != nullptr)
  {
    new_item->prev_item = last_item;
    last_item->next_item = new_item;
  }
  this->last_item = new_item;
  if (this->first_item == nullptr)
  {
    this->first_item = new_item;
  }
}

/// @brief Find MyItem from the hash table by key.
/// @param key
/// @return
MyItem *MyHashTable::find(char *key)
{
  unsigned long hash_index = hash(key);
  MyItem *item;
  for (unsigned long i = 0; i < this->size; i++)
  {
    unsigned long index = (hash_index + i) % this->size;
    item = this->items[index];
    if (item == nullptr || strcmp(item->key, key) == 0)
    {
      break;
    }
  }
  return item;
}

/// @brief Removes MyItem from the hash table by key.
/// @param key
void MyHashTable::remove(char *key)
{
  unsigned long hash_index = hash(key);
  for (unsigned long i = 0; i < this->size; i++)
  {
    unsigned long index = (hash_index + i) % this->size;
    MyItem *item = this->items[index];
    if (item == nullptr)
    {
      return;
    }
    else if (strcmp(item->key, key) == 0)
    {
      // Update last/first references
      if (this->last_item == item)
      {
        this->last_item = item->prev_item;
      }
      if (this->first_item == item)
      {
        this->first_item = item->next_item;
      }
      if (item->prev_item != nullptr)
      {
        item->prev_item->next_item = item->next_item;
      }
      if (item->next_item != nullptr)
      {
        item->next_item->prev_item = item->prev_item;
      }

      // Remove the item
      this->items[index] = nullptr; // used for empty comparison
      free_my_item(item);
      this->count--;
      break;
    }
  }
}

/// @brief Get the value of the corresponding key.
/// @param key
/// @return
int *MyHashTable::get(char *key)
{
  MyItem *item = find(key);
  return item == nullptr ? nullptr : item->value;
}

/// @brief Get the most recently inserted/changed item.
/// @return
MyItem *MyHashTable::get_last() { return this->last_item; } // O(1)

/// @brief Get the least recently inserted/changed item.
/// @return
MyItem *MyHashTable::get_first() { return this->first_item; } // O(1)
