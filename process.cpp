#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <climits>
#include <cassert>

using namespace std;

class Trie
{
  unsigned int offsets[26];
  vector<unsigned int> ids;
  unsigned int position;
  fstream &f;
  int depth;

public:
  static Trie *from_offset(fstream &file, unsigned int offset)
  {
    Trie *t = new Trie(file);
    file.seekg(offset);
    file.read((char*)&t->depth, sizeof(depth));
    file.read((char*)t->offsets, 26 * sizeof(*t->offsets));
    t->position = file.tellg();
  }

  Trie(fstream &_f) : f(_f) {}

  Trie *next(char c)
  {
    int idx = c - 'a';
    if (c >= 'A' && c <= 'Z')
      idx = c - 'A';
    if (offsets[idx] == UINT_MAX)
      return NULL;
    Trie *res = Trie::from_offset(f, offsets[idx]);
    assert(res->depth == depth + 1);
    return res;
  }

  void find(string const &token, unordered_set<unsigned int> &elems,
	    unordered_set<unsigned int> *union_checker=NULL, int idx=0)
  {
    if (idx >= token.size())
      {
	f.seekg(position);
	unsigned int n_ids;
	f.read((char*)&n_ids, sizeof(n_ids));
	unsigned int *indexes = new unsigned int[n_ids];
	f.read((char*)indexes, n_ids * sizeof(unsigned int));
	for (unsigned int i = 0; i < n_ids; ++i)
	  {
	    if (union_checker)
	      {
		if (union_checker->find(indexes[i]) != union_checker->end())
		  elems.insert(indexes[i]);
	      }
	    else
	      elems.insert(indexes[i]);
	  }
      }
    else
      {
	if (!isalpha(token[idx]))
	  {
	    find(token, elems, union_checker, idx + 1);
	    return;
	  }
	Trie *nxt = next(token[idx]);
	if (!nxt)
	  return;
	nxt->find(token, elems, union_checker, idx + 1);
	delete nxt;
      }
  }
};

int main()
{
  ifstream sids("full_ids");
  vector<string> ids;
  while (sids)
    {
      string tmp;
      sids >> tmp;
      ids.push_back(tmp);
    }
  cout << "Successfully loaded ids" << endl;
  fstream trie("trie");
  trie.seekg(-(int)sizeof(unsigned int), ios::end);
  unsigned int offset_first_trie;
  trie.read((char*)&offset_first_trie, sizeof(offset_first_trie));
  Trie *t = Trie::from_offset(trie, offset_first_trie);
  string line;
  cout << "> ";
  while (cin)
    {
      getline(cin, line);
      if (!cin)
	break;
      istringstream tokenizer(line);
      string token;
      tokenizer >> token;
      if (!tokenizer)
	continue;
      unordered_set<unsigned int> elems;
      t->find(token, elems);
      while (tokenizer)
	{
	  unordered_set<unsigned int> others;
	  tokenizer >> token;
	  if (!tokenizer)
	    break;
	  t->find(token, others, &elems);
	  elems.swap(others);
	}
      for (auto it = elems.begin(); it != elems.end(); ++it)
	{
	  cout << ids[*it] << endl;
	}
      cout << "> ";
    }
}
