#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<windows.h>
#include<conio.h>
#define up(i,a,b) for(int i=a;i<=b;i++)
#define down(i,a,b) for(int i=a;i>=b;i--)
#define WIDTH (15)
#define HIGHT (24)
#define TIME (20)
#define DROP_T (800)
#define TX (2)
#define TY (2)
#define SCORE (7)
#define ROUND (10)
#define RATE (8)
#define CPMS (CLOCKS_PER_SEC/1000)
#define SAVE "record.dat"

int score=0,rnd=1,randomN=1,lose=0;
int blockNow=0,blockNext=0,shape=0;
int px=WIDTH/2-1,py=0;
int tick=0;
static const int COLOR[]={14,13,15,12,2,1,8};
HANDLE out;
char map[4][4]={};
COORD point={TX*2,TY};
char screen[HIGHT][WIDTH];
char request[HIGHT+3];
int flush();
int init();
int loadBlock(unsigned short);
int clear();
int prt(int);
int collision();

typedef struct{
    char*name;
    int score;
}PLAYER;PLAYER player[11];
int cmp(const void*a,const void*b){return ((PLAYER*)a)->score-((PLAYER*)b)->score;}
int record(int pScore){
    FILE *file=fopen(SAVE,"r");char str[10][40],name[21];int i;char*tmp;
    if(file==NULL){
        file=fopen(SAVE,"w");
        fprintf(file,"0:??\n0:??\n0:??\n5:?\n10:x\n50:xx\n100:xxx\n200:xxxx\n400:xxxxx\n600:xxxxxx\n");
        fclose(file);
        file=fopen(SAVE,"r");
    }
    for(int i=0;i<10;i++){
        fgets(str[i],40,file);
        if((tmp=strchr(str[i],'\n'))!=NULL)*tmp='\0';
    }
    if(sscanf(str[0],"%d",&(player[0].score))!=1)return 1;
    if(player[0].score>=pScore){
        printf("\n\n  You have not break records.\n");
        return 0;
    }
    if((player[0].name=strchr(str[0],':')+1)==(NULL+1))return 1;
    for(i=1;i<10;i++){
        if(sscanf(str[i],"%d",&(player[i].score))!=1)return 1;
        if((player[i].name=strchr(str[i],':')+1)==(NULL+1))return 1;
    }
    printf("\n\n  New Records! entry your name(in 20 charaters,no space):\n");
    scanf("%20s",name);
    player[10].score=pScore;player[10].name=name;
    qsort(player,11,sizeof(PLAYER),cmp);
    fclose(file);file=fopen(SAVE,"w");
    printf("\n\n");
    for(i=1;i<11;i++)fprintf(file,"%d:%s\n",player[i].score,player[i].name);
    for(i=10;i>0;i--)fprintf(stdout,"    No.%2d:%21s, score:%5d\n",11-i,player[i].name,player[i].score);
    fclose(file);
    return 0;
}
int sleep(int t){
    static int clk=-1;int tempClk=0;
    if(clk==-1)clk=clock();
    while((tempClk=clock())<clk+t*CPMS)Sleep(1);
    clk=tempClk;
    return 0;
}
char input=0;
static unsigned short blocks[7][4]={{0b0100010001100000,0b0010111000000000,0b1100010001000000,0b0000111010000000},{0b0100010011000000,0b0000111000100000,0b0110010001000000,0b1000111000000000},{0b1100110000000000},{0b0100111000000000,0b0100110001000000,0b0000111001000000,0b0100011001000000},{0b0100010001000100,0b0000111100000000},{0b0100110010000000,0b1100011000000000},{0b1000110001000000,0b0110110000000000}};
inline int loadBlock(unsigned short a){
    up(i,0,15)map[0][i]=!!(a&((1<<15)>>i));
    return 0;
}
inline int clear(){
    up(i,0,3)up(j,0,3)if(map[i][j]){
        screen[i+py][px+j]=0;request[i+py]=1;
    }
    return 0;
}
inline int prt(int mode){
    up(i,0,3)up(j,0,3)if(map[i][j]){
        screen[i+py][px+j]=blockNow+1;request[i+py]=mode;
    }
    return 0;
}
inline int collision(){
    up(i,0,3)up(j,0,3)if(map[i][j])if((i+py>=HIGHT||j+px<0||j+px>=WIDTH)||(screen[py+i][px+j]))return 1;
    return 0;
}
int clean(){
    int dis=1,last=py+3<(HIGHT-1)?py+3:(HIGHT-1),flag=1,tscore=0;
    for(int i=py;i<HIGHT&&i<py+4;++i){
        up(j,0,WIDTH-1){
            if(screen[i][j]==0)
                goto Q;
        }
        request[i]=1;
        ++tscore;
        memset(screen[i],8,WIDTH);
        Q:;
    }
    while(last>=0&&last>=py&&screen[last][0]!=8)--last;
    if(last<py||last<0){
        return 0;
    }
    score+=tscore*(tscore+1)/2;
    flush();
    sleep(DROP_T/3);
    for(;last-dis>=0&&flag;--last){
        flag=0;
        while(last-dis>=0&&screen[last-dis][0]==8)dis++;
        if(last-dis>=0)up(j,0,WIDTH-1){
            if(screen[last][j]!=screen[last-dis][j]){
                request[last]=1;
                screen[last][j]=screen[last-dis][j];
            }
            if(screen[last-dis][j]){
                flag=1;
                screen[last-dis][j]=0;
                request[last-dis]=1;
            }
        }
    }
    memset(screen[last-dis+2],0,(dis-1)*sizeof(screen[0]));
    memset(request+last-dis+2,1,dis-1);
    request[HIGHT+1]=1;
    flush();
    sleep(DROP_T/3);
    return 0;
}
int next(){
    int down=0;
    if(input){
        switch(input){
            case 's':
                if((tick*RATE)%(DROP_T/TIME)==0&&tick!=0&&tick!=(DROP_T/TIME-1)){down=1;clear();++py;}else input=0;
                break;
            case 'w':
                clear();
                if(shape!=0)--shape;
                else{
                    shape=3;
                    while(!blocks[blockNow][shape])--shape;
                }
                loadBlock(blocks[blockNow][shape]);
                break;
            case 'a':
                if(px>=0){clear();--px;}else input=0;
                break;
            case 'd':
                if(px<WIDTH-1){clear();++px;}else input=0;
                break;
        }
    }
    if(tick==DROP_T/TIME-1){
        if(!input)clear();
        ++py;
        down=1;
    }
    if(input||down){
        CHECK:
        if(collision()){
            if(input=='a')
                ++px;
            else if(input=='d')
                --px;
            else if(input=='w'){
                if(shape<3&&blocks[blockNow][shape+1]!=0)++shape;else shape=0;
                loadBlock(blocks[blockNow][shape]);
            }
            if(down&&input!='s'){input='s';goto CHECK;}
            if(down){
                --py;prt(0);
                clean();
                srand(randomN*time(0));
                randomN=rand();
                blockNow=blockNext;blockNext=randomN%7;shape=0;px=WIDTH/2-1,py=0;
                loadBlock(blocks[blockNow][0]);
                if(collision()){
                    lose=1;
                    return 0;
                }
                ++rnd;
                request[HIGHT+2]=request[HIGHT]=1;
                prt(1);
                return 0;
            }
            prt(0);
            return 0;
        }
        prt(1);
    }
    return 0;
}
int flush(){
    static int color=7;
    up(k,0,HIGHT-1)if(request[k]){
        request[k]=0;
        point.Y=k+TY;
        SetConsoleCursorPosition(out,point);
        up(i,0,WIDTH-1){
            if(screen[k][i]==0){
                fputs("  ",stdout);
            }else{
                if(screen[k][i]!=color)
                    SetConsoleTextAttribute(out,color=screen[k][i]);
                printf("■");
            }
        }
    }
    if(request[HIGHT]){
        if(color!=(blockNext+1))
            SetConsoleTextAttribute(out,color=(blockNext+1));
        request[HIGHT]=0;
        point.X=(TX+WIDTH+2)*2;
        up(i,0,3){
            point.Y=TY+1+i;
            SetConsoleCursorPosition(out,point);
            up(j,0,3)
                printf("%s",(blocks[blockNext][0]&((1<<15)>>(i*4+j)))?"■":"  ");
        }
    }
    if(request[HIGHT+1]){
        if(7!=color)
            SetConsoleTextAttribute(out,color=7);
        request[HIGHT+1]=0;
        point.Y=TY+1+SCORE;
        point.X=(TX+WIDTH+3)*2;
        SetConsoleCursorPosition(out,point);
        printf("%d",score);
        point.X=TX*2;
    }
    if(request[HIGHT+2]){
        if(7!=color)
            SetConsoleTextAttribute(out,color=7);
        request[HIGHT+2]=0;
        point.Y=TY+1+ROUND;
        point.X=(TX+WIDTH+3)*2;
        SetConsoleCursorPosition(out,point);
        printf("%d",rnd);
        point.X=TX*2;
    }
    return 0;
}
int init(){
    out=GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo={1,0};
    SetConsoleCursorInfo(out,&cursorInfo);
    srand(time(0));
    randomN=rand();
    blockNow=randomN%7;
    srand(randomN*time(0));
    randomN=rand();
    blockNext=randomN%7;
    loadBlock(blocks[blockNow][0]);
    prt(1);
    memset(request,1,sizeof(request));
    SetConsoleTitle("   TETRIS");
    system("cls");
    up(i,1,TY-1)printf("\n");
    up(i,1,TX-1)printf("  ");
    up(i,-1,WIDTH)printf("□");
    printf("  next: \n");
    up(i,1,HIGHT){
        up(i,1,TX-1)printf("  ");
        printf("□");
        up(j,1,WIDTH)printf("  ");
        printf("□");
        if(i==SCORE+1)printf("  score:");else if(i==ROUND+1)printf("  round:");
        printf("\n");
    }
    printf("  ");
    up(i,-1,WIDTH)printf("□");
    printf("\n\n    Press 'w' to roll\n    Press 'a' or 'd' to move\n    Press 's' to drop faster\n\n    Press '=' to exit");
    SetConsoleTextAttribute(out,7);
    return 0;
}
int main(){
    system("cls");
    printf("\n\n\n      Press space to start");
    while(getch()!=' ');
    sleep(TIME);
    system("cls");
    init();
    while(!kbhit()||((input=getch())!='=')){
        next();
        flush();
        if(lose)break;
        input=0;
        if((++tick)>=(DROP_T/TIME))tick=0;
        sleep(TIME);
    }
    point.X=0;point.Y=TY+HIGHT+2;
    SetConsoleTextAttribute(out,15);
    SetConsoleCursorPosition(out,point);
    system("cls");
    printf("\n\n\n\n  GameOver,your score is %d in %d rounds.",score,rnd);
    if(record(score)==1)printf("\n  record error.\n");
    system("pause");
    return 0;
}
