#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>



#include "colors.h"
#include "Matrix.h"

using namespace std;


/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

char saved_key = 0;
int tty_raw(int fd);	/* put terminal into a raw mode */
int tty_reset(int fd);	/* restore terminal's mode */
  
/* Read 1 character - echo defines echo mode */
char getch() {
  char ch;
  int n;
  while (1) {
    tty_raw(0);
    n = read(0, &ch, 1);
    tty_reset(0);
    if (n > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
        if (saved_key != 0) {
          ch = saved_key;
          saved_key = 0;
          break;
        }
      }
    }
  }
  return ch;
}

void sigint_handler(int signo) {
  // cout << "SIGINT received!" << endl;
  // do nothing;
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void registerInterrupt() {
  struct sigaction act, oact;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGINT, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

//7가지 블록
// -1: 배열의 끝
int T0D0[] = { 1, 1, 1, 1, -1 }; //2x2
int T0D1[] = { 1, 1, 1, 1, -1 };
int T0D2[] = { 1, 1, 1, 1, -1 };
int T0D3[] = { 1, 1, 1, 1, -1 };

int T1D0[] = { 0, 1, 0, 1, 1, 1, 0, 0, 0, -1 }; //3x3
int T1D1[] = { 0, 1, 0, 0, 1, 1, 0, 1, 0, -1 };
int T1D2[] = { 0, 0, 0, 1, 1, 1, 0, 1, 0, -1 };
int T1D3[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, -1 };

int T2D0[] = { 1, 0, 0, 1, 1, 1, 0, 0, 0, -1 }; //3x3
int T2D1[] = { 0, 1, 1, 0, 1, 0, 0, 1, 0, -1 };
int T2D2[] = { 0, 0, 0, 1, 1, 1, 0, 0, 1, -1 };
int T2D3[] = { 0, 1, 0, 0, 1, 0, 1, 1, 0, -1 };

int T3D0[] = { 0, 0, 1, 1, 1, 1, 0, 0, 0, -1 }; //3x3
int T3D1[] = { 0, 1, 0, 0, 1, 0, 0, 1, 1, -1 };
int T3D2[] = { 0, 0, 0, 1, 1, 1, 1, 0, 0, -1 };
int T3D3[] = { 1, 1, 0, 0, 1, 0, 0, 1, 0, -1 };

int T4D0[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 }; //3x3
int T4D1[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };
int T4D2[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D3[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };

int T5D0[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 }; //3x3
int T5D1[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };
int T5D2[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D3[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };

int T6D0[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 }; //4x4
int T6D1[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
int T6D2[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D3[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
  
int *setOfBlockArrays[] = {  // 28개의 도형의 주소를 모아놓음
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

void drawScreen(Matrix *screen, int wall_depth)
{
  int dy = screen->get_dy();
  int dx = screen->get_dx();
  int dw = wall_depth;
  int **array = screen->get_array();

  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	      cout << "□ ";
      else if (array[y][x] == 1)
	      cout << "■ ";
      else if (array[y][x] == 10)
	      cout << "◈ ";
      else if (array[y][x] == 20)
	      cout << "★ ";
      else if (array[y][x] == 30)
	      cout << "● ";
      else if (array[y][x] == 40)
	      cout << "◆ ";
      else if (array[y][x] == 50)
	      cout << "▲ ";
      else if (array[y][x] == 60)
	      cout << "♣ ";
      else if (array[y][x] == 70)
	      cout << "♥ ";
      else
	      cout << "X ";
    }
    cout << endl;
  }
}
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

#define SCREEN_DY  10
#define SCREEN_DX  10
#define SCREEN_DW  3

#define ARRAY_DY (SCREEN_DY + SCREEN_DW)
#define ARRAY_DX (SCREEN_DX + 2*SCREEN_DW)

int arrayScreen[ARRAY_DY][ARRAY_DX] = {  // 충돌을 막아주기 위해 벽 두께를 3으로 만들어줌
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },  
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },  
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};

int arrayBlk[3][3] = {
  { 0, 1, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 },
};

int arrayBlk2[3][3] = {
  { 1, 0, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 },
};



// void deleteFullLines(Matrix *screen) {
//   int dy = screen->get_dy();
//   int dx = screen->get_dx();
//   Matrix *screen_get = new Matrix(screen);  
//   Matrix *line;
//   int size[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//   for(int i = 0; i < dy; i++) {
//     line = screen_get->clip(i, 0, i+1, dx);
//     if (line->sum() == dx) {
//       size[i] = 1;
//     }
//     delete line;
//   }
  
//   Matrix *newScreen = new Matrix(dy, dx);
//   int y = dy - 1;
//   for (int j = dy - 1; j >= 0; j--) {
//     line = screen_get->clip(j, 0, j+1, dx);
//     if (size[j] == 1) {
//       delete line;
//       break;
//     }
    
//     newScreen->paste(line, y, 0);
//     y--;
//     delete line;
//   }
//   screen->paste(newScreen, 0, 0);
//   delete newScreen;
//   delete screen_get;
// }
Matrix* deleteFullLines(const Matrix& screen)
{

  Matrix *screen_get = new Matrix(screen);
  Matrix *line;
  Matrix *newScreen = new Matrix((int *)arrayScreen, ARRAY_DY, ARRAY_DX);
  int y = 9;

  for(int i = 9; i >= 0; i--)
  {
    line = screen_get->clip(i, 0, i+1, ARRAY_DX);
    
      if(line->sum() ==ARRAY_DX)
      {
        delete line;
        continue;
      }
    
    
    newScreen->paste(line, y, 0);
    y--;
    delete line;
  }

  screen_get->paste(newScreen, 0, 0);
  delete newScreen;

  return screen_get;
}


int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));
  char key;
  int top = 0, left = 4;
  int blkType = rand() % MAX_BLK_TYPES;
  int blkDegree = 0;
  
  int size[] = {2, 3, 3, 3, 3, 3, 4};
  Matrix *setOfBlockObjects[7][4];
  for (int i = 0; i < MAX_BLK_TYPES; i++) {
    for (int j = 0; j < MAX_BLK_DEGREES; j++) {
      setOfBlockObjects[i][j] = new Matrix((int *) setOfBlockArrays[i*MAX_BLK_DEGREES + j], size[i], size[i]);
    }
  }

  
  Matrix *iScreen = new Matrix((int *) arrayScreen, ARRAY_DY, ARRAY_DX);
  Matrix *currBlk = setOfBlockObjects[blkType][blkDegree];
  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
  Matrix *tempBlk2 = tempBlk->add(currBlk);
  delete tempBlk;
  

  Matrix *oScreen = new Matrix(iScreen);
  oScreen->paste(tempBlk2, top, left);
  delete tempBlk2;
  drawScreen(oScreen, SCREEN_DW);
  delete oScreen;
  
  while ((key = getch()) != 'q') {
    cout << "nput" << endl;
    cout << top << " " << left <<endl;
    
    
    switch (key) {
      case 'a': left--; break;
      case 'd': left++; break;
      case 's': top++; break;
      case 'w': {
          blkDegree = (blkDegree + 1) % 4;
          currBlk = (setOfBlockObjects[blkType][blkDegree]);
        }; break;
      case ' ': {
          while (true) {  // " " 눌렸을 때 최하단으로 블럭 이동
            top++;
            tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
            tempBlk2 = tempBlk->add(currBlk);
            delete tempBlk;
            if (tempBlk2->anyGreaterThan(1)) {
              top--;
              break;
            }
          }  }; break;
      default: cout << "wrong key input" << endl;
      }
      
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
      if (tempBlk2->anyGreaterThan(1)) {
        delete tempBlk2;
        switch (key) {
          case 'a': left++; break;
          case 'd': left--; break;
          case 's': top--; break;
          case 'w': {
              blkDegree = (blkDegree + 3) % 4;
              currBlk = (setOfBlockObjects[blkType][blkDegree]);
            };  break;
          case ' ': top--; break;
          default: cout << "wrong key input" << endl;
        }
        tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
        tempBlk2 = tempBlk->add(currBlk);
        delete tempBlk;
      }
      
      
      
      oScreen = new Matrix(iScreen);
      oScreen->paste(tempBlk2, top, left);
      delete tempBlk2;
      
      drawScreen(oScreen, SCREEN_DW);
      top++;
          // int bottom = top + currBlk->get_dy();
          // if (bottom > 12) {
          //   bottom = 12;
          // }
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
      
      if (tempBlk2->anyGreaterThan(1)) {
        iScreen->paste(oScreen, 0, 0);
        delete oScreen;
        Matrix *temp_screen = deleteFullLines(iScreen);
        iScreen->paste(temp_screen, 0, 0);
        delete temp_screen;
        delete tempBlk2;
        
          top = 0;
          left = 4;
          blkType = rand() % MAX_BLK_TYPES;
          blkDegree = 0;
          currBlk = (setOfBlockObjects[blkType][blkDegree]);
          tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
          tempBlk2 = tempBlk->add(currBlk);
          delete tempBlk;
          
          oScreen = new Matrix(iScreen);
          oScreen->paste(tempBlk2, top, left);
          delete tempBlk2;
          drawScreen(oScreen, SCREEN_DW);
          delete oScreen;
          cout << "dnput" << endl;
        
      }
      else {
        delete oScreen;
        delete tempBlk2;
        top--;
      }
     
  }
  for (int i = 0; i < MAX_BLK_TYPES; i++) {
    for (int j = 0; j < MAX_BLK_DEGREES; j++) {
     delete  setOfBlockObjects[i][j];
    }
  }
  delete iScreen;
  
  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
  cout << "Program terminated!" << endl;

  return 0;
}