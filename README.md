# Concurrent trie

Various functions are defined in `trie.h` to implement a [Trie](https://en.wikipedia.org/wiki/Trie). The implementation of the functions are filled in `trie.c`. The description of what each function is supposed to do is provided in `trie.h`. 

Trie is made thread safe by using three of these: using a single mutex lock for the entire trie, using a reader writer lock and using hand on hand locking. 
  - Hand on hand locking when `_NO_HOH_LOCK_TRIE` has not been defined. 
  - Otherwise, single mutex lock when `_S_LOCK_TRIE` has been defined, the reader writer lock when it has not been defined.
