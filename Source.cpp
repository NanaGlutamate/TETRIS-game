#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<windows.h>
#include<conio.h>
#define up(i,a,b) for(int i=a;i<=b;i++)
#define down(i,a,b) for(int i=a;i>=b;i--)
static const int WIDTH=15,HIGHT=24,TIME=20,DROP_T=800,TX=2,TY=2,SCORE=7,ROUND=10,RATE=4,CPMS=CLOCKS_PER_SEC/1000;
int score=0,rnd=1,randomN=1,lose=0;
int blockNow=0,blockNext=0,shape=0;
int px=WIDTH/2-2,py=0;
// UI
int tick=0;
static const int COLOR[]={14,13,15,12,2,1,8};
//YELLOW=14,PINK=13,WHITE=15,BLACK=0,RED=12,GREEN=2,BLUE=1,NONE=0;
HANDLE out;
char map[4][4]={};
COORD point={TX*2,TY};
char screen[HIGHT][WIDTH];
char request[HIGHT+3];
//static const char * const Table[]={"□","■"};
int flush();
int init();

int sleep(int t){
    static int clk=clock();int tempClk=0;
    while((tempClk=clock())<clk+t*CPMS)Sleep(1);
    clk=tempClk;
    return 0;
}
// Game logic
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
inline int prt(int mode=1){
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
                if(tick%((DROP_T/TIME-1)/RATE)==0&&tick!=(DROP_T/TIME-1)){down=1;clear();++py;}else input=0;
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
    }//若旋转使得该方块与已有方块重合，会覆盖。若旋转使得方块超边界，数组会越界。
    //Solved
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
                blockNow=blockNext;blockNext=randomN%7;shape=0;px=WIDTH/2-2,py=0;
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
}//若不可能重合，标记input为0。（初检）若input不为0，消除原来块，产生新块。若有重合，若改变为向下移动，固定并打印该块，判断是否满行，判断是否满列，产生新块;若为其它改变，回滚改变;若既下降又有其他改变，回滚其它改变后再次判断。打印新块。

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
    SetConsoleCursorPosition(out,point);
    printf("      Your final score is:\n      %6d              \n                          \n      in                      \n      %6d                  \n                            \n      rounds.                       \n",score,rnd);
    /*init();
    while(1){
        memset(request,1,sizeof(request));
        memset(screen,1,sizeof(screen));
        flush();
        while((scanf("%d",&py))!=EOF){
            memset(request,1,sizeof(request));
            clean();
        }
    }*/
    return 0;
}
int flush(){
    static int color=7;int flag=0;
    up(k,0,HIGHT-1)if(request[k]){
        flag=1;
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
        flag=1;
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
        flag=1;
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
        flag=1;
        point.Y=TY+1+ROUND;
        point.X=(TX+WIDTH+3)*2;
        SetConsoleCursorPosition(out,point);
        printf("%d",rnd);
        point.X=TX*2;
    }
    if(flag){
        point.Y=9+TY+HIGHT;
        SetConsoleCursorPosition(out,point);
    }
    return 0;
}
int init(){
    srand(time(0));
    randomN=rand();
    blockNow=randomN%7;
    srand(randomN*time(0));
    randomN=rand();
    blockNext=randomN%7;
    loadBlock(blocks[blockNow][0]);
    prt();
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
    out=GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(out,7);
    return 0;
}
/*
*TODO:
*
*1.PAUSE
*
*2.旋转时碰到边框自动平移
*/
