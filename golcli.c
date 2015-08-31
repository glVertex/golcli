#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>

int XSIZE = -1;
int YSIZE = -1;
int delay = 50000;
char fillChar = 'O';
char bgChar = ' ';


char*** boards;
char** topBoard;
char** bottomBoard;

void initBoards(){
        boards = malloc(2*sizeof(char**));
	int i,n;
        for(n=0; n<2; n++){
            boards[n] = malloc(XSIZE*sizeof(char*));
            for(i=0; i<XSIZE; i++){
                boards[n][i] = malloc(YSIZE*sizeof(char));
            }
        }
}

void randomFillBoards(){
    srand(time(NULL) + rand());
    
    int x,y;
    for(x=0;x<XSIZE;x++){
        for(y=0;y<YSIZE;y++){
            boards[0][x][y] = (rand() % 100 < 30) ? 1 : 0;
        }
    }
    
}

void fileFillBoards(char* filePath, char bgFChar){
    int x = 0;
    int y = 0;
    char buf;
    
    FILE* inFile = fopen(filePath,"r");
    
    while(!feof(inFile) && y<YSIZE){
        fread(&buf,1,1,inFile);
        if(buf=='\n'){
            x = 0;
            y++;
        }else {
            boards[0][x][y] = buf == bgFChar ? 0 : 1;
            x++;
            if(x>=XSIZE){
                x=0;
                y++;
            }
        }
   }
    
    fclose(inFile);
}


int normCoord(int val, int size){
    while(val<0){
        val+=size;
    }
    while(val>=size){
        val-=size;
    }
    return val;
}

int getNears(char** brd, int x, int y){
   int res = 0;
   int dx,dy;
   int lx,ly;
   for(dx=-1;dx<=1;dx++){
       for(dy=-1;dy<=1;dy++){
           if(dx!=0 || dy!=0){
               lx=normCoord(x+dx,XSIZE);
               ly=normCoord(y+dy,YSIZE);
               if(brd[lx][ly] == 1){
                   res++;
               }
           }
       }
   }
   return res;
}

void liveBoard(char** srcB, char** dstB){
    int x,y;
    for(x=0;x<XSIZE;x++){
        for(y=0;y<YSIZE;y++){
            int nears = getNears(srcB,x,y);
            if(nears < 2){
                dstB[x][y] = 0;
            }else if(nears == 2){
                dstB[x][y] = srcB[x][y];
            }else if(nears == 3){
                dstB[x][y] = 1;
            }else{
                dstB[x][y] = 0;
            }
        }
    }
}

void gotoxy(int x, int y){
	//printf("\x1b[%i;%iH",x,y);
	move(y,x);
}

void start(){
	initscr();
	savetty();
	nonl();
	cbreak();
	noecho();
	timeout(0);
	leaveok(stdscr, TRUE);
	curs_set(0);
}

void finish(int sigage){
	curs_set(1);
	clear();
	refresh();
	resetty();
	endwin();
	exit(0);
}

void drawBoard(char** board){
    int x,y;
    for(x=0;x<XSIZE;x++){
        for(y=0;y<YSIZE;y++){
            gotoxy(x,y);
            addch(board[x][y]==0 ? bgChar : fillChar);
        }
    }
}

/**
 * Parser for input params
 * @param argc - count CLI params
 * @param argv - CLI params
 * @param key - request key
 * @return -1 - not found, 0 - found, argv[N] - found with param
 */
char* getCLIParam(int argc, char** argv, const char* key){
    int i;
    for(i=0;i<argc;i++){
        if(strcmp(key, argv[i]) == 0){
            if(i+1 < argc){
                if(argv[i+1][0] != '-'){
                    return argv[i+1];
                }
            }
            return (char*) 0;
        }
    }
    return (char*) -1;
}

int isParam(char* param){
    return param == (char*)0 || param == (char*) -1 ? 0 : 1;
}

int isParamExist(char* param){
    return param == (char*) -1 ? 0 : 1;
}

void printHelp(){
    printf("golcli 1.0. Conway's Game of Life with ncurses interaface.\n\n");
    printf("Usage: golcli [OPTIONS]\n\n");
    printf(
    "\
Options:\n\
    -h   Help\n\
    -x   Width\n\
    -y   Height\n\
    -d   Delay between frames (ms, default: 50)\n\
    -cf  Foreground char\n\
    -cb  Background char\n\
    -m   Map file\n\
    -mcb Background char in map file (default: ' ')\n"
            );
}

int main(int argc, char** argv){

        char* cSizeX = getCLIParam(argc,argv,"-x");
        char* cSizeY = getCLIParam(argc,argv,"-y");
        char* cDelay = getCLIParam(argc,argv,"-d");
        char* cFillChar = getCLIParam(argc,argv,"-cf");
        char* cBgChar = getCLIParam(argc,argv,"-cb");
        char* cInFile = getCLIParam(argc,argv,"-m");
        char* cFileBgChar = getCLIParam(argc,argv,"-mcb");
        char* cHelp = getCLIParam(argc,argv,"-h");
      
        if(isParamExist(cHelp)){
            printHelp();
            exit(0);
        }
        
        if(isParam(cSizeX)){XSIZE = atoi(cSizeX);}
        if(isParam(cSizeY)){YSIZE = atoi(cSizeY);}
        if(isParam(cDelay)){delay = atoi(cDelay) * 1000;}
        if(isParam(cFillChar)){fillChar = cFillChar[0];}
        if(isParam(cBgChar)){bgChar = cBgChar[0];}
        
        
	start();

	signal(SIGINT,finish);

        if(XSIZE <= 0){ XSIZE = getmaxx(stdscr); }
        if(YSIZE <= 0){ YSIZE = getmaxy(stdscr); }
        
        initBoards();
        
        if(isParam(cInFile)){
            char bgFile = bgChar;
            if(isParam(cFileBgChar)){
                bgFile = cFileBgChar[0];
            }
            fileFillBoards(cInFile,bgFile);
        }else{
            randomFillBoards();
        }
               
        topBoard = boards[0];
        bottomBoard = boards[1];
    
        int run = 1;
	char k;
	while(run){
                drawBoard(topBoard);
                
                bottomBoard = topBoard == boards[0] 
                        ? boards[1]
                        : boards[0];
                
                liveBoard(topBoard, bottomBoard);
                
                topBoard = bottomBoard;
                
		k = wgetch(stdscr);
		switch(k){
			case 'q':{
                            run = 0;
                            break;
			}
			case -1:{
                            break;
			}
		}
                usleep(delay);
	}

	finish(0);
	return 0;
}
