#include "lzw.h"
#include <iostream>

using namespace std;

void lzw::init()
{
    //create trie

}

lzw::lzw(int sz)
{
    if(sz < 8)
    {
        cout<<"Unable to set code size smaller than of ascii characters! New code size is 8 bits.\n";
        this->codesize = 8;
    }
    else if(sz > 32)
    {
        cout<<"Unable to set code size greater than 32 bits! New code size is 32 bits.\n";
        this->codesize = 32;
    }
    else
        this->codesize = sz;

    //init stats
    inputsize = 0;
    outputsize = 0;

    carry = 0;
    nr_carry_bits = 0;

    last_code = -1;

    //create trie and insert every ASCII char
    T = new trie();
    for (int i = 0; i < 256; ++i)
    {
        //cout<<"Inserting "<<i<<"\n";
        T->insert((const char*)&i, 1);
    }
}

lzw::~lzw()
{
    delete T;
    //delete [] buf;
}

int lzw::compress(char * src, char * dst, int inputSize, int & outputSize)
{
    inputsize += inputSize;

    for(int i = 0; i < inputSize; ++i)
    {
        char P = src[i];
        cout<<"[DEBUG] Next "<<P<<"\n";

        int code = T->find_incremental_and_add(P);
        if(code != -1)   //found the word, try with more chars
        {
            last_code = code;
        }

        else    //did not find the word, output its code
        {
            cout<<"Outputting code "<<last_code<<"\n";

            //combine the 2 numbers
            carry = carry | ((1uLL * last_code) << nr_carry_bits);

            //determine how many full bytes there are
            int full_bytes = (codesize + nr_carry_bits) / 8;
            nr_carry_bits = (codesize + nr_carry_bits) % 8;

            //copy the full bytes
            memcpy(dst + outputSize, &carry, full_bytes);
            outputSize += full_bytes;

            //stats
            outputsize++;

            //save the rest
            carry = carry >> (8 * full_bytes);

            //advance in trie
            last_code = T->find_incremental(P);
        }
    }

    return 1;
}

int lzw::compressFinal(char * src, char * dst, int inputSize, int & outputSize)
{
    inputsize += inputSize;

    for(int i = 0; i <= inputSize; ++i)
    {
        if(i == inputSize)
        {
            cout<<"Last code before EOF: "<<last_code<<"\n";

            //redundant carry clear
            carry &= ((1uLL << nr_carry_bits) - 1);

            //combine the 2 numbers
            carry = carry | ((1uLL * last_code) << nr_carry_bits);

            int sz = (codesize + nr_carry_bits + 8) / 8;
            memcpy(dst + outputSize, &carry, sz); //pad with zero
            outputSize += sz;

            //stats
            outputsize += sz;
        }

        else
        {
            char P = src[i];
            cout<<"Next "<<P<<"\n";

            int code = T->find_incremental_and_add(P);
            if(code != -1)   //found the word, try with more chars
            {
                last_code = code;
            }

            else    //did not find the word, output its code
            {
                cout<<"Outputting code "<<last_code<<"\n";
                //redundant carry clear
                carry &= ((1uLL << nr_carry_bits) - 1);

                //combine the 2 numbers
                carry = carry | ((1uLL * last_code) << nr_carry_bits);

                //determine how many full bytes there are
                int full_bytes = (codesize + nr_carry_bits) / 8;
                nr_carry_bits = (codesize + nr_carry_bits) % 8;

                //copy the full bytes
                memcpy(dst + outputSize, &carry, full_bytes);
                uint64_t temp = 0;
                memcpy(&temp, &carry, full_bytes);

                outputSize += full_bytes;

                //stats
                outputsize++;

                //save the rest
                carry = carry >> (8 * full_bytes);

                //advance in trie
                last_code = T->find_incremental(P);
            }
        }
    }

    return 1;
}