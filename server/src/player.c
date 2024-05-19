#include "../header/player.h"



Player * createplayer(int sock,int mode){
  Player *p =malloc(sizeof(Player));
  if(p==NULL){
        perror("problem de malloc dans createplayer");
        return p;
  }
 
  p->Ready = 0;
  p->mode=mode;
  p->sockcom=sock;
  p->moveaction.num=-1;
  p->moveaction.action=-1;

  return p;

}

void free_player(Player *p)
{

  debug_printf("free player \n");
  close(p->sockcom);
  //pthread_mutex_destroy(p->lockstats);
  free(p);
}





