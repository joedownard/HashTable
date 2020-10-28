#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/**
 * HashTable struct, stores an array of pointers to the top level buckets and keeps a track of the number of buckets
 */
struct HashTable {
    int numBuckets; // holds the number of buckets in the table
    struct Bucket *buckets[]; // holds a pointer an array of pointers to buckets
};

/**
 * Bucket struct, stores a key-value pair and a pointer to a chained bucket
 */
struct Bucket {
    struct Bucket *chainedBucket; // holds a pointer to a bucket chained to this bucket
    char *key; // holds a char array representing the key
    int value; // holds the value associated with the key
};

/**
 * A function that creates a hashtable struct, allocates enough memory as demanded by the number of buckets wanted
 * @param numBuckets The number of top level buckets required
 * @return The constructed HashTable struct.
 */
struct HashTable* constructHashTable (int numBuckets) {
    struct HashTable *table = malloc(sizeof(struct HashTable) + sizeof(struct Bucket) * numBuckets); // creates a new Hashtable and allocate enough memory for the number of buckets wanted
    table->numBuckets = numBuckets; // store the number of buckets available
    for (int i = 0; i < table->numBuckets; i++) {
        table->buckets[i] = malloc(sizeof(struct Bucket)); // allocate memory for all of the buckets
        table->buckets[i]->key = ""; // initialize the key for all of the buckets to be an empty string
    }
    return table;
}

/**
 * A function to delete and free the memory of a bucket and any subsequently chained buckets
 * @param bucket The bucket to destroy
 */
void destroyBucket (struct Bucket *bucket) {
    if (bucket->key != "") { // if the key of this bucket is not empty, then there must be a chained bucket
        destroyBucket(bucket->chainedBucket); // recursively delete the buckets in the chain
        free(bucket); // free the bucket the function was initially called on
    } else { // when the key is empty there must be no more buckets chained, so we can begin freeing the memory
        free(bucket); // this will free all the buckets lower in the chain
    }
}

/**
 * A function to delete and free the memory of a hashtable and all bucket chains
 * @param table The table to delete
 */
void destroyHashTable (struct HashTable *table) {
    for (int i = 0; i < table->numBuckets; i++) { // iterate through all the buckets in the table
        destroyBucket(table->buckets[i]); // delete and free each bucket chain individually
    }
    free(table);
}

/**
 * A fairly efficient hashing fucntion written by Dan Bernstein.
 * Dan Bernstein, 1990. DJB2 Hashing function [computer program]. Available from: https://groups.google.com/forum/?nomobile=true#!searchin/comp.lang.c/Dan$20Bernstein$20%7Csort:date/comp.lang.c/VByoIO8GySs/2XN9iGTpgmsJ [Accessed 02 May 2020].
 * @param str, the string to hash
 * @return the hash
 */
unsigned long hash(unsigned char *str) {// use an optimised hash function by Dan Bernstein
    unsigned long hash = 5381; // start at 5381, special number
    int c;

    while (c = *str++) { // loop through all the characters in the string until we get to character \0, terminating char.
        hash = ((hash << 5) + hash) + c; // (hash * 2^5) + hash + c
    }

    return hash;
}

/**
 * A function to take a key-value and place them into a bucket. This bucket will then be chained onto the given bucket.
 * The functions calls itself recursively until it finds a free bucket then returns the entire bucket chain.
 * @param bucket The bucket to chain onto.
 * @param key The key value.
 * @param value The value corresponding with the key.
 * @return The bucket chain.
 */
struct Bucket* chainValue(struct Bucket *bucket, char *key, int value) {
    if (bucket->key != "") {
        bucket->chainedBucket = chainValue(bucket->chainedBucket, key, value); // recurse if the current bucket is already filled
    } else {
        bucket->chainedBucket = malloc(sizeof(struct Bucket)); // allocates the next bucket in the chain memory to allow for further chaining
        bucket->chainedBucket->key = ""; // initializes the next bucket in the chains key value to show it is last in the chain
        bucket->value = value; // sets the key-pair value
        bucket->key = key;
    }
    return bucket;
}

/**
 * Adds the given key-pair value to the hashtable
 * @param table The table to add to
 * @param key The key to add
 * @param value The corresponding value to add
 */
void addToTable (struct HashTable *table, char *key, int value) {
    int boundedHash = hash(key) % table->numBuckets;
    table->buckets[boundedHash] = chainValue(table->buckets[boundedHash], key, value);
}

/**
 * Searches a bucket and it chain for the key provided
 * @param bucket The bucket to search
 * @param key The key to serch for
 * @return The bucket with the key in it.
 */
struct Bucket* searchBucket (struct Bucket *bucket, char *key) {
    if (bucket->key != "") {
        if (bucket->key == key) {
            return bucket;
        } else {
            return searchBucket(bucket->chainedBucket, key);
        }
    } else {
        return 0;
    }
}

/**
 * Searches the table for a key by hashing it and searching the bucket at that index.
 * @param table The table to search through
 * @param key The key to search for
 * @return The bucket the key is in
 */
struct Bucket* searchTable (struct HashTable *table, char *key) {
    int boundedHash = hash(key) % table->numBuckets;
    return searchBucket(table->buckets[boundedHash], key);
}

/**
 * Prints out the value of the key specified
 * @param table The table to search for the key in and print the value of it
 * @param key The key to search for and print the value of
 */
void printKeyValue (struct HashTable *table, char *key) {
    struct Bucket *bucket = searchTable(table, key); // searches for the bucket with the key
    if (bucket != 0) {
        printf("%s: %d\n", key, bucket->value); // prints the value if it is found
    } else {
        printf("%s doesn't exist!\n", key); // prints an error if it cant be found
    }
}

/**
 * A function to delete a bucket from a chain and free the memory. Then reforms the bucket chain without the specified bucket.
 * @param bucket The bucket chain to delete the bucket with the key in from.
 * @param key The key corresponding to the bucket to be removed.
 * @return The bucket chain without the deleted bucket.
 */
struct Bucket* reformChainExcluding(struct Bucket *bucket, char *key) {
    if (bucket->key != "") {
        if (bucket->key == key) {
            struct Bucket *tmpPointer = bucket->chainedBucket; // Holds the chain from the bucket being removed temporarily
            free(bucket); // frees the memory from the deleted bucket.
            return tmpPointer;
        } else {
            bucket->chainedBucket = reformChainExcluding(bucket->chainedBucket, key); // recurse if the key was not found in this bucket
        }
    }
    return bucket; // returns the original bucket chain if there was an issue
}

/**
 * A function to remove the key from the table.
 * @param table The hashtable to remove the key from.
 * @param key The key to remove.
 */
void removeFromTable (struct HashTable *table, char *key) {
    int boundedHash = hash(key) % table->numBuckets; // Hashes the key and finds the top level bucket associated with it.
    table->buckets[boundedHash] = reformChainExcluding(table->buckets[boundedHash], key); // Removes the key by removing it associated bucket.
}

/**
 * Prints the key-value pair in the bucket provided
 * @param bucket The bucket to print
 */
void printBucket(struct Bucket *bucket) {
    if (bucket->key != "") {
        printBucket(bucket->chainedBucket); // Print the chained bucket too
        printf("%s:%d ", bucket->key, bucket->value);
    }
}

/**
 * Prints the entire table provided
 * @param table
 */
void printTable (struct HashTable *table) {
    for (int i = 0; i < table->numBuckets; i++) { // Loop through all top level buckets and print them and their chains
        printf("\n[%d] ", i);
        printBucket(table->buckets[i]);
    }
    printf("\n");
}

/**
 * Function to read an array into the table.
 * @param table The hashtable to add to
 * @param names The array of strings to add
 * @param length The length of the array
 */
void readIntoTable (struct HashTable *table, char *names[], int length) {
    for (int i = 0; i < length; i++) {
        if (names[i][0] != '\0') {
            addToTable(table, names[i], 1);
        }
    }
}


int main() {
    FILE *inputFile = fopen("names.txt", "r");
    if (!inputFile) return 0;
    char *names[10000];

    int c;
    int i = 0; // counter for what word we are currently on
    int j = 0; // counter for what position in the word we are currently at

    names[0] = calloc(32, sizeof(char));
    while ((c = fgetc(inputFile)) != EOF) { // A loop to read in the names provided in names.txt
        if (c == ',') {
            *(names[i] + j) = '\0';
            i++;
            names[i] = calloc(32, sizeof(char)); // allocate the next string pointer enough memory to hold a string
            j = 0;
        } else if (c != 34){
            *(names[i] + j) = c;
            j++;
        }
    }


    struct HashTable *table = constructHashTable(5000); // setup the hashtable with 5000 top level buckets.
    int length = i + 1;
    readIntoTable(table, names, length); // read the names array into the table

    printTable(table); // print out the table

    destroyHashTable(table); // free the table

    for (int i = 0; i < length-1; i++) {
        free(names[i]); // ensure we free all dynamically allocated memory
    }

    return 0;
}
