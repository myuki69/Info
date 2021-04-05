File compression tool - README

This program can compress data files using two passes, first LZW with 12 bits codes and then Huffman.

It is pretty easy to do. The program takes 3 arguments: <mode>, <input> and <output>.
Mode can either be -c or -d (compress or decompress). The -c parameter accepts any file as input, while -d expects a file compressed using this tool.

The passes are independed and a temporary file is generated to hold data between them, mostly because Huffman needs two passes over the raw data.
One to determine the frequencies for each character and the other to do actual compression.

Implementation:
Each algorithm implementation is defined in its own separate class. An instance can compress and decompress data.
For LZW i use two dictionaries: a trie for compression and a hash table for decompression. When compressing you need to add to the old code
a new character for the input and search for that in the dictionary. This is easily done with a trie because you can just save the node
corresponding to the old code and look for the new character in its sons. It basically works like a DFA. For decompression on the other hand
you read a code and need to output its corresponding string, and searching codes in a trie takes O(n) time, while a hash table is O(1).

To compress data using this class you need to call the compress() and compressFinal() functions. As long as there is data you can call compress(),
but at the end of the data stream only the final version guarantees correct output. A code does not fit in a single byte so after each output from
the LZW algorithm what fits is written to the output and the remainder is cached. The cached bits are carryed over to the next calls of the function.
The "final" version pads the extra bits with 0 and output the last char in the output stream too. About the parameters: input size can be whatever
size you want, but the output size needs to be able to handle the worst case, that is for each 8 bit input character a 12-bit code is output.

The decompression has only one variant (no final function) and no risk of out-of-bounds situations because it uses a dynamic std::string-like structure.

The same stand true for the Huffman algorithm, but its output also has a final variant.
To use it, first instantiate the object, then either load a saved tree by calling "deserialize()" or call updateFrequency to update characters frequency
and explicitly build a new tree using "buildTree()". After this is done use the functions the same way as lzw's.

The only thing you need to be care about is to save the code lenghts in the output file so you can decompress the data later. Do that easily using "serialize()".
This does not store the tree or the frequencies, it only stores the code lenghts because that is sufficient to build a cannonical encoding.
Read more: https://en.wikipedia.org/wiki/Canonical_Huffman_code

To compile with mingw/gcc just type "make" or compile each file from command line.

