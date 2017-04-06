#include "token.h"
#include "assert.h"
#include <Windows.h>

struct __tag_token_sec_s *
__token_decode (char *chuf, int len) {
  
  int i = 0; /** count index **/
  int latch = 0; /* sec init ?? **/
  int lsp = 1; /* last is space ?*/
  int lsc = 0; /* last 's pos **/
  int *apv = NULL; /* always pointer previus's len **/
  char *pblk_toksec = NULL;
  struct __tag_token_s *k = NULL;
  struct __tag_token_sec_s *s = NULL;
  
  if (len == 0)
    return NULL;
  
  for (; i != len; i++)  
  {
    if (chuf[i] == ' ') {
      if (lsp == 0) {
        /* e.g. |nnnnn.. | **/
        *apv = i - lsc;
         lsp = 1;
      }
      continue;
    } else if (latch == 0)  {
      /* first appear, init sec and first item. **/
      pblk_toksec = malloc ( sizeof (struct __tag_token_sec_s)
                          +  sizeof (struct __tag_token_s) );
                          
      assert (pblk_toksec != NULL);
      
      k = (void *)    pblk_toksec;
      s = (void *)&   pblk_toksec[sizeof (struct __tag_token_s)];
      
      s->level = k;
      s->len = 1;

      latch = 1;
    } else {
      
      struct __tag_token_s *P, *C;
      
      if (lsp == 0)
        continue;
      
      k = malloc ( sizeof (struct __tag_token_s));
                          
      assert (k != NULL);
      s->len++;
      
      /* insert list's tail **/
      P = s->level;
      C = P->level;
      
      for (; C != NULL; C = (P = C)->level)
        ; P->level = k;
    }

    k->cuuf = & chuf[i];
    k->level = NULL;
    k->len = -1;
    
    apv = & k->len;
    lsp = 0;
    lsc = i;
  }
  
  if (apv != NULL ) 
    *apv = len - lsc;
  return s;
}

void __token_free (struct __tag_token_sec_s *__token_sec) {

  struct __tag_token_sec_s *S = NULL;
  struct __tag_token_s *P, *C;
  
  if (__token_sec != NULL)
    return;
  
  P = __token_sec->level;
  C = P->level;  
  
  for (free (P); C != NULL; C = P) {
    P = C->level;
    free (C);
  }
}