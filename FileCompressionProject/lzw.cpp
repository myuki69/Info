#include "lzw.h"
#include <iostream>

#include <vector>
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
    tStoredWords = 256;

    //create hashmap and insert every ascii char
    for (int i = 0; i < 256; ++i)
    {
        //H[i] = "";
        H[i] = (unsigned char)i;
    }
    hStoredWords = 256;
}

lzw::~lzw()
{
    H.clear();
    delete T;
    //delete [] buf;
}

void lzw::reset()
{
    //delete old data
    H.clear();
    delete T;

    //clear inner params
    carry = 0;
    nr_carry_bits = 0;
    last_code = 0;
    old.clear();
    inputsize = 0;
    outputsize = 0;

    //create new
    T = new trie();
    for (int i = 0; i < 256; ++i)
    {
        //cout<<"Inserting "<<i<<"\n";
        T->insert((const char*)&i, 1);
    }
    tStoredWords = 256;

    //create hashmap and insert every ascii char
    for (int i = 0; i < 256; ++i)
    {
        //H[i] = "";
        H[i] = (unsigned char)i;
    }
    hStoredWords = 256;
}

int lzw::compress(char * src, char * dst, int inputSize, int & outputSize)
{
    inputsize += inputSize;

    for(int i = 0; i < inputSize; ++i)
    {
        char P = src[i];
        //if(isprint(P))
        //    cout<<"[DEBUG] Next "<<P<<"\n";
        //else
        //    cout<<"[DEBUG] Next 0x"<<hex<<(unsigned int)P<<dec<<"\n";

        int code;
        //if dictionary is not full
        if(tStoredWords < (1 << codesize))
        {
            code = T->find_incremental_and_add(P);
            tStoredWords++;
        }
        else    //if it is full don't add
            code = T->find_incremental(P);

        if(code != -1)   //found the word, try with more chars
        {
            last_code = code;
        }

        else    //did not find the word, output its code
        {
            //cout<<"Outputting code "<<last_code<<"\n";

            //combine the 2 numbers
            carry = carry | ((1uLL * last_code) << nr_carry_bits);

            //determine how many full bytes there are
            int full_bytes = (codesize + nr_carry_bits) / 8;
            nr_carry_bits = (codesize + nr_carry_bits) % 8;

            //copy the full bytes
            memcpy(dst + outputSize, &carry, full_bytes);
            outputSize += full_bytes;

            //stats
            outputsize += full_bytes;

            //save the rest
            carry = carry >> (8 * full_bytes);

            //advance in trie
            last_code = T->find_incremental(P);
            //cout<<"[DEBUG] Last code is now "<<last_code<<"\n";
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
            //cout<<"Last code before EOF: "<<last_code<<"\n";

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
            //cout<<"Next "<<P<<"\n";

            int code;
            //if dictionary is not full
            if(tStoredWords < (1 << codesize))
            {
                code = T->find_incremental_and_add(P);
                tStoredWords++;
            }
            else    //if it is full don't add
                code = T->find_incremental(P);

            if(code != -1)   //found the word, try with more chars
            {
                last_code = code;
            }

            else    //did not find the word, output its code
            {
                //cout<<"Outputting code "<<last_code<<"\n";
                //redundant carry clear
                carry &= ((1uLL << nr_carry_bits) - 1);

                //combine the 2 numbers
                carry = carry | ((1uLL * last_code) << nr_carry_bits);

                //determine how many full bytes there are
                int full_bytes = (codesize + nr_carry_bits) / 8;
                nr_carry_bits = (codesize + nr_carry_bits) % 8;

                //copy the full bytes
                memcpy(dst + outputSize, &carry, full_bytes);
                outputSize += full_bytes;

                //stats
                outputsize += full_bytes;

                //save the rest
                carry = carry >> (8 * full_bytes);

                //advance in trie
                last_code = T->find_incremental(P);
            }
        }
    }

    return 1;
}

int lzw::decompress(char * src, int inputSize, bstring & buf)
{
    //stats
    inputsize += inputSize;

    for(int i = 0; i < inputSize;)
    {
        int nw = 0; //next input code

        //determine how many bytes to read
        int full_bytes = (codesize - nr_carry_bits) % 8 ? (codesize - nr_carry_bits + 8) / 8 : (codesize - nr_carry_bits) / 8;

        //check if in the input stream are enough bytes
        if(inputSize - i < full_bytes)
        {
            //if not we can't fetch another code so end decompression and return the number of bytes processed
            //memcpy(dst, buf.data(), outputSize);
            //outputsize += outputSize;
            return i;
        }
        //cout<<"["<<i<<"] Reading from stream "<<full_bytes<<" bytes. Leftover "<<nr_carry_bits<<" bits\n";

        //initialize code with what's left
        nw = carry;
        //cout<<"["<<i<<"] Initial code is 0x"<<nw<<"\n";

        //read from raw data stream
        memcpy(&carry, src + i, full_bytes);
        i += full_bytes;

        //complete the code with the new data
        nw = nw | (carry << nr_carry_bits);

        //truncate code
        nw &= ((1<<codesize) - 1);
        //cout<<"["<<i<<"] Code is 0x"<<nw<<"\n";

        //there are 8*full_bytes - (codesize - nr_carry_bits) bits left in the carry
        carry = carry >> (codesize - nr_carry_bits);
        //cout<<"["<<i<<"] Carry is 0x"<<carry<<"\n";

        //compute the number of leftover bits
        nr_carry_bits = (codesize - nr_carry_bits + 8) % 8;

        if(H.find(nw) != H.end())    //the code is indexed
        {
            //"print" code
            //cout<<H[nw].data()<<"\n";
            buf += H[nw];
            outputsize += H[nw].size();

            unsigned char c = H[nw][0];

            if(old.size() && hStoredWords < (1 << codesize))
            {
                //insert new string into the dictionary
                H[hStoredWords++] = old + c;
            }

            old = H[nw];
        }

        else
        {
            //cout<<"Code 0x"<<nw<<" not found!\n";
            unsigned char c = old[0];
            old = old + c;
            //cout<<old.data()<<"\n";
            buf += old;
            outputsize += old.size();

            if(hStoredWords < (1 << codesize))
                H[hStoredWords++] = old;
        }

    }

    return inputSize;
}

void lzw::printInfo()
{
    cout<<"LZW compression summary:\n";
    cout<<"Read input bytes: "<<inputsize<<"\n";
    cout<<"Written output bytes "<<outputsize<<"\n";
    cout<<endl;
}
