#include "../header/player.h"

 
 Player * createplayer(int id, int sock,int idEq)
  {

    Player *p =malloc(sizeof(Player));
    if(p==NULL){
        perror("problem de malloc dans createplayer");
        return p;
    }
    p->sockcom=sock;
    p->id = id;
    p->idEq = idEq;
    p->Ready = 0;
    p->len = 0;

    return p;
  }

void free_player(Player *p)
{

  debug_printf("free player \n");
  close(p->sockcom);
  pthread_mutex_destroy(p->lockstats);
  free(p);
}


void cancellastmove(A_R *tab, int size)
{
  for (int i = 0; i < size; i--)
  {
    if (tab[size - i].action < 4 && tab[size - i].action >= 0)
    {
      memcpy(tab + (size - i), tab + (size - i) + 1, size - i - 1);
      return;
    }
  }
}

int insererAction(Player *p, A_R action)
{
  if (p->len >= 20 - 1)
    return 1;

  int i;
  for (i = 0; i < p->len; i++)
  {
    if (p->tabAction[i].num > action.num)
    {
      break;
    }
  }
  memmove(&p->tabAction[i + 1], &p->tabAction[i], (p->len - i) * sizeof(A_R));
  p->tabAction[i] = action;
  p->len += 1;
  return 0;
}
