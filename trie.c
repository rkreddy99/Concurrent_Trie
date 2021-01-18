#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

trie_t init_trie(void){
    // Write your code here
    trie_t trie = (trie_t)malloc(sizeof(_trie_t));
    trie->head = (trie_node_t)malloc(sizeof(_trie_node_t));
    trie->head->is_end = false;
    for (int i = 0; i < 26; i++)
    {
        trie->head->children[i] = NULL;
    }
    #ifndef _NO_HOH_LOCK_TRIE
        pthread_mutex_init(&(trie->head->node_lock), NULL);
    #endif
    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_init(&(trie->lock), NULL);
    #endif
    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_init(&(trie->lock_rw), NULL);
    #endif
    return trie;
} 

void insert(trie_t trie, char* key, int value){
    // // Write your code here
    int length = strlen(key);

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_lock(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_wrlock(&(trie->lock_rw));
    #endif

    trie_node_t curr = trie->head;
    for (int i = 0; i < length; i++)
    {
        int index = key[i] - 97;
        if (!curr->children[index])
        {
            curr->children[index] = (trie_node_t)malloc(sizeof(_trie_node_t));
            curr = curr->children[index];

            for (int j = 0; j < 26; j++)
            {
                curr->children[j] = NULL;
            }
            curr->value = value;
            curr->is_end = false;
            if (i==length-1) curr->is_end = true;
        }
        else
        {
            curr = curr->children[index];
        }
    }
    curr->value = value;
    curr->is_end = true;

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_unlock(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_unlock(&(trie->lock_rw));
    #endif

    #ifndef _NO_HOH_LOCK_TRIE

        pthread_mutex_lock(&(trie->head->node_lock));

        length = strlen(key);
        curr = trie->head;
        trie_node_t pred = curr;

        for ( int i = 0; i < length; i++ )
        {
            int index = key[i] - 97;
            if ( !curr->children[index] )
            {
                curr->children[index] = (trie_node_t)malloc(sizeof(_trie_node_t));
                pred = curr;
                curr = curr->children[index];
                pthread_mutex_init(&(curr->node_lock), NULL);
                pthread_mutex_lock(&(curr->node_lock));
                pred->children[index] = curr;

                for (int j = 0; j < 26; j++)
                {
                    curr->children[j] = NULL;
                }
                curr->is_end = false;
                curr->value = value;
                pthread_mutex_unlock(&(pred->node_lock));
            }
            else
            {
                pred = curr;
                curr = curr->children[index];
                pthread_mutex_lock(&(curr->node_lock));
                pthread_mutex_unlock(&(pred->node_lock));
            }
        }
        curr->value = value;
        curr->is_end = true;
        pthread_mutex_unlock(&(curr->node_lock));

    #endif
}

int find(trie_t trie,char* key, int* val_ptr){
    // Write your code here
    int length = strlen(key);

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_lock(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_rdlock(&(trie->lock_rw));
    #endif

    trie_node_t curr = trie->head;
    #ifndef _NO_HOH_LOCK_TRIE
        trie_node_t pred = curr;
        pthread_mutex_lock(&(curr->node_lock));
    #endif
    for (int i = 0; i < length; i++)
    {
        int index = key[i] - 97;
        if (!curr->children[index]) 
        {

            #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
                pthread_mutex_unlock(&(trie->lock));
            #endif

            #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
                pthread_rwlock_unlock(&(trie->lock_rw));
            #endif

            #ifndef _NO_HOH_LOCK_TRIE
                pthread_mutex_unlock(&(curr->node_lock));
            #endif

            return -1;
        }
        #ifndef _NO_HOH_LOCK_TRIE
            pred = curr;
        #endif
        curr = curr->children[index];
        #ifndef _NO_HOH_LOCK_TRIE
            pthread_mutex_lock(&(curr->node_lock));
            pthread_mutex_unlock(&(pred->node_lock));
        #endif
    }
    if (curr != NULL && curr->is_end) 
    {
        *val_ptr = curr->value;

        #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
            pthread_mutex_unlock(&(trie->lock));
        #endif

        #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
            pthread_rwlock_unlock(&(trie->lock_rw));
        #endif

        #ifndef _NO_HOH_LOCK_TRIE
            pthread_mutex_unlock(&(curr->node_lock));
        #endif

        return 0;
    }

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_unlock(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_unlock(&(trie->lock_rw));
    #endif

    #ifndef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&(curr->node_lock));
    #endif

    return -1;
} 

void delete_kv(trie_t trie, char* key){
    // Write your code here
    int length = strlen(key);
    int p;
    int present = find(trie, key, &p);
    if (present==-1) return ;

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_lock(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_wrlock(&(trie->lock_rw));
    #endif

    trie_node_t curr = trie->head;
    #ifndef _NO_HOH_LOCK_TRIE
        trie_node_t pred = NULL;
        pthread_mutex_lock(&(curr->node_lock));
    #endif
    for (int i = 0; i < length; i++)
    {
        int index = key[i] - 97;
        // if (!curr->children[index]) 
        // {
        //     #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        //         pthread_mutex_unlock(&(trie->lock));
        //     #endif

        //     #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        //         pthread_rwlock_unlock(&(trie->lock_rw));
        //     #endif

        //     #ifndef _NO_HOH_LOCK_TRIE
        //         pthread_mutex_unlock(&(curr->node_lock));
        //     #endif

        //     return NULL;
        // }
        #ifndef _NO_HOH_LOCK_TRIE
            pred = curr;
        #endif
        curr = curr->children[index];
        #ifndef _NO_HOH_LOCK_TRIE
            pthread_mutex_lock(&(curr->node_lock));
            pthread_mutex_unlock(&(pred->node_lock));
        #endif
    }
    if (curr != NULL && curr->is_end) curr->is_end = false;

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_unlock(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_unlock(&(trie->lock_rw));
    #endif

    #ifndef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&(curr->node_lock));
    #endif

    return ;
    
}

char** match(trie_node_t node, char* pref, char** list)
{
	if (node == NULL) return list;
	int pref_len = strlen(pref);
	if (node->is_end)
	{
		// int end = 0;
		for (int i = 0; i<1000; i++)
		{
			if (!list[i])
			{
				list[i] = (char*) malloc(pref_len+1);
				for (int j = 0; j<pref_len; j++) list[i][j] = pref[j];
				list[i][pref_len] = '\0';
				break;
			}
		}
	}
	for (int i = 0; i < 26; ++i)
	{
		if (node->children[i])
		{
			char* _pref = (char*) malloc((pref_len+2)*sizeof(char));
			strcpy(_pref, pref);
			_pref[pref_len+1] = _pref[pref_len];
			_pref[pref_len] = i+97;
			#ifndef _NO_HOH_LOCK_TRIE
				// pthread_mutex_init(&(node->children[i]->node_lock));
				pthread_mutex_lock(&(node->children[i]->node_lock));
			#endif
			list = match(node->children[i], _pref, list);
			free(_pref);
			#ifndef _NO_HOH_LOCK_TRIE
				pthread_mutex_unlock(&(node->children[i]->node_lock));
			#endif
		}
	}
	return list;
}

char** keys_with_prefix(trie_t trie, char* prefix)
{
	int length = strlen(prefix);
	int list_length = 1000;
	char** list = malloc(list_length*sizeof(char*));

	for (int i=0; i<list_length; i++) list[i]=NULL;

	#if defined(_NO_HOH_LOCK_TRIE ) && defined(_S_LOCK_TRIE)
		pthread_mutex_lock(&(trie->lock));
	#endif

	#if defined(_NO_HOH_LOCK_TRIE) && !defined(_S_LOCK_TRIE)
		pthread_rwlock_rdlock(&(trie->lock_rw));
	#endif

	trie_node_t curr = trie->head;
	int check = 0;
	#ifndef _NO_HOH_LOCK_TRIE
	    trie_node_t pred = NULL;
		// pthread_mutex_init(&(curr->node_lock));
		pthread_mutex_lock(&(curr->node_lock));
	#endif

	for (int i = 0; i < length; i++)
	{
		int index = prefix[i] - 97;
        #ifndef _NO_HOH_LOCK_TRIE
		    pred=curr;
        #endif
		curr = curr->children[index];
		#ifndef _NO_HOH_LOCK_TRIE
			pthread_mutex_lock(&(curr->node_lock));
		#endif
		if (curr==NULL)
		{
			check = 1;
			break;
		}
		#ifndef _NO_HOH_LOCK_TRIE
			pthread_mutex_unlock(&(pred->node_lock));
		#endif
	}

	if (check == 0) list = match(curr, prefix, list);

	#ifndef _NO_HOH_LOCK_TRIE
		pthread_mutex_unlock(&(curr->node_lock));
	#endif

	#if defined(_NO_HOH_LOCK_TRIE ) && defined(_S_LOCK_TRIE)
		pthread_mutex_unlock(&(trie->lock));
	#endif

	#if defined(_NO_HOH_LOCK_TRIE) && !defined(_S_LOCK_TRIE)
		pthread_rwlock_unlock(&(trie->lock_rw));
	#endif

	return list;
}

void delete_node(trie_t trie, trie_node_t node){

    for (int i = 0; i < 26; i++)
    {
        if (node->children[i])
        {
            delete_node(trie, node->children[i]);
        }
    }
    #ifndef _NO_HOH_LOCK_TRIE
       pthread_mutex_destroy(&(node->node_lock));
    #endif
    free(node);
}

void delete_trie(trie_t trie){
    // Write your code here
    delete_node(trie, trie->head);

    #if defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_mutex_destroy(&(trie->lock));
    #endif

    #if !defined(_S_LOCK_TRIE) && defined(_NO_HOH_LOCK_TRIE)
        pthread_rwlock_destroy(&(trie->lock_rw));
    #endif

    free(trie);

}