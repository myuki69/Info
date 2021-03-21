#include "trie.h"
#include <iostream>

using namespace std;

trie::trie()
{
    T = new tNode();
}

trie::~trie()
{
    cout<<"TBA!\n";
}

void trie::insert_inner(tNode* x, const unsigned char* str, int sz)
{
    //failsafe
    if(!x)
        throw runtime_error("Fatal error! Inserting into null node!\n");

    if(sz == 0) //end of the word, mark
    {
        x->isWord = 1;
        return;
    }

    else    //more of the word remaining
    {
        unsigned char c = *str;
        //cout<<"[DEBUG]"<<(unsigned int)c<<"\n";
        if(x->sons[(unsigned int)c] && sz)  //we already have a word with this prefix
            insert_inner(x->sons[(unsigned int)c], str + 1, sz - 1);

        else    //need to create new node
        {
            x->sons[(unsigned int)c] = new tNode();
            insert_inner(x->sons[(unsigned int)c], str + 1, sz - 1);
        }

        return;
    }
}

void trie::print_inner(tNode* x)
{
    static string ans = "";

    //failsafe
    if(!x)
        throw runtime_error("Fatal error! Printing null node!\n");

    if(x->isWord)
        cout<<ans<<"\n";

    for(int i = 0; i < 256; ++i)
    {
        if(x->sons[i])
        {
            ans.push_back(i);
            print_inner(x->sons[i]);
            ans.pop_back();
        }
    }
}

int trie::find_inner(tNode* x, const unsigned char* str, int sz)
{
    if(!x)
        return -1;

    if(sz == 0)
    {
        if(x->isWord)
            return 1;
        else
            return -1;
    }
    //cout<<"[DEBUG]"<<(unsigned int)(*str)<<"\n";
    return find_inner(x->sons[(unsigned int)(*str)], str + 1, sz - 1);

}

void trie::insert(const char * str, int sz)
{
    if(sz > 0)
        insert_inner(T, (const unsigned char*)str, sz);
}

int trie::find(const char * str, int sz)
{
    if(sz > 0)
        return find_inner(T, (const unsigned char*)str, sz);
    else
        return -1;
}

void trie::print()
{
    print_inner(T);
}
